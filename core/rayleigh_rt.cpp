// Doubling solution for a conservative Rayleigh layer. See rayleigh_rt.h.
//
// Refs:
//   Hansen (1971), J. Atmos. Sci. 28, 1400-1426 (doubling)
//   van de Hulst (1980), Multiple Light Scattering, vol. 1 (adding equations)
//   Chandrasekhar (1960), Radiative Transfer

#include "rayleigh_rt.h"
#include "rayleigh.h"

#include <cmath>
#include <algorithm>
#include <mutex>

namespace {

constexpr int N = RayleighRT::Nodes;

// Depolarisation factor for air, Young (1980); must match rayleigh.cpp.
constexpr double kDepolarization = 0.0279;

/**
 * Number of doublings. The initial layer must be optically thin compared with
 * the smallest quadrature cosine or its single-scattering seed is wrong: the
 * smallest 32-point node is 0.0014, and tau/2^20 is at most 2.3e-7.
 */
constexpr int kDoublings = 20;

inline double legP2 (double x) { return 0.5 * (3.0 * x * x - 1.0); }
inline double legP21(double x) { return -3.0 * x * std::sqrt(std::max(0.0, 1.0 - x * x)); }
inline double legP22(double x) { return 3.0 * (1.0 - x * x); }

/** Gauss-Legendre nodes and weights mapped from [-1,1] onto (0,1), ascending. */
void gaussLegendre01(double *x, double *w, int n)
{
    for (int i = 0; i < n; ++i) {
        double z  = std::cos(M_PI * (i + 0.75) / (n + 0.5));
        double pp = 0.0;
        for (int it = 0; it < 100; ++it) {
            double p0 = 1.0, p1 = 0.0;
            for (int j = 0; j < n; ++j) {
                const double p2 = p1;
                p1 = p0;
                p0 = ((2.0 * j + 1.0) * z * p1 - j * p2) / (j + 1.0);
            }
            pp = n * (z * p0 - p1) / (z * z - 1.0);
            const double dz = p0 / pp;
            z -= dz;
            if (std::fabs(dz) < 1e-15)
                break;
        }
        x[i] = 0.5 * (1.0 - z);
        w[i] = 1.0 / ((1.0 - z * z) * pp * pp);
    }
}

/** In-place Gauss-Jordan inverse with partial pivoting. */
void invert(double *a, int n)
{
    int piv[N];
    for (int i = 0; i < n; ++i) piv[i] = i;

    for (int col = 0; col < n; ++col) {
        int best = col;
        for (int r = col + 1; r < n; ++r)
            if (std::fabs(a[r * n + col]) > std::fabs(a[best * n + col])) best = r;
        if (best != col) {
            for (int c = 0; c < n; ++c) std::swap(a[col * n + c], a[best * n + c]);
            std::swap(piv[col], piv[best]);
        }
        const double d = 1.0 / a[col * n + col];
        a[col * n + col] = 1.0;
        for (int c = 0; c < n; ++c) a[col * n + c] *= d;
        for (int r = 0; r < n; ++r) {
            if (r == col) continue;
            const double f = a[r * n + col];
            if (f == 0.0) continue;
            a[r * n + col] = 0.0;
            for (int c = 0; c < n; ++c) a[r * n + c] -= f * a[col * n + c];
        }
    }

    // Undo the column permutation the row swaps induced.
    double tmp[N * N];
    for (int r = 0; r < n; ++r)
        for (int c = 0; c < n; ++c) tmp[r * n + piv[c]] = a[r * n + c];
    std::copy(tmp, tmp + n * n, a);
}

inline void matmul(const double *a, const double *b, double *out, int n)
{
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            double s = 0.0;
            for (int k = 0; k < n; ++k) s += a[i * n + k] * b[k * n + j];
            out[i * n + j] = s;
        }
}

/**
 * Everything below works with "hat" matrices, A^_ij = A_ij * 2*mu_j*w_j, which
 * turns the angular integral operator (A**B)_ij = sum_k A_ik*2*mu_k*w_k*B_kj
 * into an ordinary matrix product and the identity operator into I.
 */
void doubleLayer(double *Rh, double *Th, int n)
{
    double A[N * N], Q[N * N], X[N * N], Y[N * N];

    matmul(Rh, Rh, A, n);
    for (int i = 0; i < n * n; ++i) Q[i] = -A[i];
    for (int i = 0; i < n; ++i) Q[i * n + i] += 1.0;
    invert(Q, n);                       // Q = (I - R^ R^)^-1

    matmul(Th, Q, X, n);                // X = T^ Q
    matmul(X, Rh, Y, n);
    matmul(Y, Th, A, n);                // A = T^ Q R^ T^
    matmul(X, Th, Q, n);                // Q = T^ Q T^   (new transmission)

    for (int i = 0; i < n * n; ++i) Rh[i] += A[i];
    std::copy(Q, Q + n * n, Th);
}

double fresnelUnpolarised(double mu, double n)
{
    mu = std::max(0.0, std::min(1.0, mu));
    const double sin_i = std::sqrt(std::max(0.0, 1.0 - mu * mu));
    const double sin_t = sin_i / n;
    if (sin_t >= 1.0) return 1.0;                     // cannot happen entering water
    const double cos_t = std::sqrt(std::max(0.0, 1.0 - sin_t * sin_t));

    const double rs = (mu - n * cos_t) / (mu + n * cos_t);
    const double rp = (n * mu - cos_t) / (n * mu + cos_t);
    return 0.5 * (rs * rs + rp * rp);
}

const double *bandTauTable()
{
    static double t[RayleighCorrector::SolarBandCount];
    for (int b = 0; b < RayleighCorrector::SolarBandCount; ++b)
        t[b] = RayleighCorrector::opticalDepthFCI(b);
    return t;
}

} // namespace

void RayleighRT::solve(double tau, Solution &out)
{
    out.tau = tau;
    gaussLegendre01(out.mu, out.wt, N);

    const double gamma = kDepolarization / (2.0 - kDepolarization);
    const double c     = (1.0 - gamma) / (2.0 * (1.0 + 2.0 * gamma));

    if (tau <= 0.0) {
        for (int m = 0; m < 3; ++m)
            for (int i = 0; i < N; ++i)
                for (int j = 0; j < N; ++j) {
                    out.R[m][i][j] = 0.0;
                    // No atmosphere, but the sea surface still reflects the sun.
                    out.Rocean[m][i][j] = 0.0;
                }
        for (int i = 0; i < N; ++i) { out.Ttot[i] = 1.0; out.planeAlbedo[i] = 0.0; }
        out.sphericalAlbedo = 0.0;
        return;
    }

    const double dtau = tau / std::pow(2.0, kDoublings);

    double Rh[N * N], Th[N * N];

    for (int m = 0; m < 3; ++m) {
        for (int i = 0; i < N; ++i) {
            const double mui = out.mu[i];
            const double dir = std::exp(-dtau / mui);
            for (int j = 0; j < N; ++j) {
                const double muj = out.mu[j];

                // Phase function Fourier modes. Reflection pairs the downward
                // incident direction -muj with the upward outgoing +mui, which
                // flips the sign of the odd associated Legendre term.
                double pr, pt;
                if (m == 0) {
                    pr = pt = 1.0 + c * legP2(mui) * legP2(muj);
                } else if (m == 1) {
                    const double q = c * (1.0 / 6.0) * legP21(mui) * legP21(muj);
                    pr = -q; pt = q;
                } else {
                    pr = pt = c * (1.0 / 24.0) * legP22(mui) * legP22(muj);
                }

                // Thin-layer seed R_ij = dtau*P/(4*mui*muj), in hat form.
                const double f = dtau * out.wt[j] / (2.0 * mui);
                Rh[i * N + j] = f * pr;
                Th[i * N + j] = f * pt + (i == j ? dir : 0.0);
            }
        }

        for (int k = 0; k < kDoublings; ++k)
            doubleLayer(Rh, Th, N);

        // Add the sea surface as a lower boundary. A flat Fresnel interface
        // reflects each direction into its mirror and nowhere else, so as an
        // operator it is exactly diagonal - and a diagonal is the same in every
        // Fourier mode, which means it drops into the adding equations here
        // with no truncation error at all.
        //
        //   R_ocean = R + T_up (I - Rs R)^-1 Rs T_down,   Rs = diag(rF)
        //
        // T_down deliberately excludes the direct solar beam. A flat surface
        // reflects that beam specularly, and a delta in azimuth cannot be
        // represented in three Fourier modes - it would smear an oscillating
        // ghost across the whole disc. So sun glint is not modelled; it is a
        // separate analytic Cox-Munk term. Skylight, which is smooth and is
        // what draws the limb ramp, is handled exactly.
        {
            double Rs[N], Td[N * N], B[N * N], Q[N * N], S[N * N], X[N * N];
            for (int i = 0; i < N; ++i)
                Rs[i] = fresnelWater(out.mu[i]);

            for (int i = 0; i < N; ++i)
                for (int j = 0; j < N; ++j)
                    Td[i * N + j] = Th[i * N + j]
                                  - (i == j ? std::exp(-tau / out.mu[i]) : 0.0);

            for (int i = 0; i < N; ++i)
                for (int j = 0; j < N; ++j) {
                    B[i * N + j] = -Rs[i] * Rh[i * N + j];
                    S[i * N + j] =  Rs[i] * Td[i * N + j];
                }
            for (int i = 0; i < N; ++i) B[i * N + i] += 1.0;
            invert(B, N);                       // B = (I - Rs R)^-1

            matmul(Th, B, X, N);
            matmul(X, S, Q, N);                 // Q = T_up (I - Rs R)^-1 Rs T_down

            for (int i = 0; i < N; ++i)
                for (int j = 0; j < N; ++j)
                    out.Rocean[m][i][j] = (Rh[i * N + j] + Q[i * N + j])
                                        / (2.0 * out.mu[j] * out.wt[j]);
        }

        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                out.R[m][i][j] = Rh[i * N + j] / (2.0 * out.mu[j] * out.wt[j]);

        if (m == 0) {
            // Fluxes come from the azimuth-averaged mode alone. The hat form
            // already carries the incoming-side weight, so divide it back out.
            for (int j = 0; j < N; ++j) {
                double r = 0.0, t = 0.0;
                for (int i = 0; i < N; ++i) {
                    const double up = 2.0 * out.mu[i] * out.wt[i];
                    r += Rh[i * N + j] * up;
                    t += Th[i * N + j] * up;
                }
                const double sc = 1.0 / (2.0 * out.mu[j] * out.wt[j]);
                out.planeAlbedo[j] = r * sc;
                out.Ttot[j]        = t * sc;
            }
            double s = 0.0;
            for (int j = 0; j < N; ++j)
                s += out.planeAlbedo[j] * 2.0 * out.mu[j] * out.wt[j];
            out.sphericalAlbedo = s;
        }
    }
}

const RayleighRT::Solution &RayleighRT::forBand(int bandIndex)
{
    static Solution sols[RayleighCorrector::SolarBandCount + 1];
    static std::once_flag once;
    std::call_once(once, [] {
        const double *t = bandTauTable();
        for (int b = 0; b < RayleighCorrector::SolarBandCount; ++b)
            solve(t[b], sols[b]);
        solve(0.0, sols[RayleighCorrector::SolarBandCount]);
    });

    if (bandIndex < 0 || bandIndex >= RayleighCorrector::SolarBandCount)
        return sols[RayleighCorrector::SolarBandCount];
    return sols[bandIndex];
}

double RayleighRT::fresnelWater(double mu)
{
    return fresnelUnpolarised(mu, WaterRefractiveIndex);
}

double RayleighRT::reflectance(const Solution &s, double mu0, double muv,
                               double raaDeg, bool ocean)
{
    if (s.tau <= 0.0)
        return 0.0;

    // Bracket both cosines in the node array. The reflection function is finite
    // and smooth as mu -> 0, so clamping into the outermost cell is safe.
    auto locate = [&s](double mu, int &i0, double &f) {
        mu = std::max(s.mu[0], std::min(s.mu[N - 1], mu));
        i0 = 0;
        while (i0 < N - 2 && s.mu[i0 + 1] < mu) ++i0;
        f = (mu - s.mu[i0]) / (s.mu[i0 + 1] - s.mu[i0]);
    };

    int iv, i0v;
    double fv, f0;
    locate(muv, iv, fv);
    locate(mu0, i0v, f0);

    const double raa = raaDeg * M_PI / 180.0;
    const double az[3] = { 1.0, 2.0 * std::cos(raa), 2.0 * std::cos(2.0 * raa) };

    double rho = 0.0;
    for (int m = 0; m < 3; ++m) {
        const double (*T)[Nodes] = ocean ? s.Rocean[m] : s.R[m];
        const double a = T[iv][i0v]     * (1 - fv) + T[iv + 1][i0v]     * fv;
        const double b = T[iv][i0v + 1] * (1 - fv) + T[iv + 1][i0v + 1] * fv;
        rho += az[m] * (a * (1 - f0) + b * f0);
    }
    return std::max(0.0, rho);
}

double RayleighRT::transmittance(const Solution &s, double mu)
{
    if (s.tau <= 0.0)
        return 1.0;
    mu = std::max(s.mu[0], std::min(s.mu[N - 1], mu));
    int i = 0;
    while (i < N - 2 && s.mu[i + 1] < mu) ++i;
    const double f = (mu - s.mu[i]) / (s.mu[i + 1] - s.mu[i]);
    return s.Ttot[i] * (1 - f) + s.Ttot[i + 1] * f;
}
