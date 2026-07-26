// Unit tests for the Rayleigh doubling solver.
//
// Build: cmake -DBUILD_TESTS=ON, then bin/rayleigh_rt_test

#include "rayleigh_rt.h"
#include "rayleigh.h"

#include <cstdio>
#include <cmath>
#include <algorithm>

static int g_failures = 0;

static void check(bool ok, const char *what)
{
    if (ok) std::printf("ok   : %s\n", what);
    else  { std::printf("FAIL : %s\n", what); ++g_failures; }
}

static void checkClose(double got, double want, double tol, const char *what)
{
    const double d = std::fabs(got - want);
    if (d <= tol) std::printf("ok   : %s\n", what);
    else {
        std::printf("FAIL : %s (got %.9g, want %.9g, off by %.3g)\n", what, got, want, d);
        ++g_failures;
    }
}

static void testQuadrature()
{
    RayleighRT::Solution s;
    RayleighRT::solve(0.0, s);

    double sum = 0.0, sumMu = 0.0, sumMu2 = 0.0;
    for (int i = 0; i < RayleighRT::Nodes; ++i) {
        sum    += s.wt[i];
        sumMu  += s.wt[i] * s.mu[i];
        sumMu2 += s.wt[i] * s.mu[i] * s.mu[i];
    }
    checkClose(sum,    1.0,       1e-14, "quadrature weights sum to 1 over (0,1)");
    checkClose(sumMu,  0.5,       1e-14, "quadrature integrates mu exactly");
    checkClose(sumMu2, 1.0 / 3.0, 1e-14, "quadrature integrates mu^2 exactly");

    for (int i = 1; i < RayleighRT::Nodes; ++i)
        check(s.mu[i] > s.mu[i - 1], "nodes are strictly ascending");
    check(s.mu[0] > 0.0 && s.mu[RayleighRT::Nodes - 1] < 1.0,
          "nodes lie strictly inside (0,1)");
}

static void testEnergyConservation()
{
    // Rayleigh scattering has no absorption, so every photon entering the layer
    // must leave it: plane albedo + total transmittance = 1, for every angle of
    // incidence. This is the check that catches an error in the quadrature, the
    // hat-matrix normalisation or the doubling equations - all three would
    // otherwise produce a plausible-looking but wrong reflectance.
    const double taus[] = { 0.001, 0.0125, 0.0525, 0.1324, 0.2339, 0.5 };

    for (double tau : taus) {
        RayleighRT::Solution s;
        RayleighRT::solve(tau, s);

        // Split the grazing node out. Everywhere else the balance closes to
        // roundoff; at mu = 0.0014 (vza 89.9) 32 streams cannot resolve a
        // radiance field that peaks within a degree of the horizon, and the
        // residual there is quadrature resolution rather than a solver error -
        // it grows with tau and leaves reciprocity untouched.
        double worst = 0.0, worstResolved = 0.0;
        int worstIdx = 0;
        for (int j = 0; j < RayleighRT::Nodes; ++j) {
            const double d = std::fabs(s.planeAlbedo[j] + s.Ttot[j] - 1.0);
            if (d > worst) { worst = d; worstIdx = j; }
            if (s.mu[j] > 0.01) worstResolved = std::max(worstResolved, d);
        }
        char msg[160];
        std::snprintf(msg, sizeof msg,
                      "energy conserved at tau %.4f (%.1e resolved, %.1e at mu %.4f)",
                      tau, worstResolved, worst, s.mu[worstIdx]);
        // Quadrature error scales linearly with tau, so the bound does too.
        check(worstResolved < 4e-5 * tau && worst < 4e-4 * tau, msg);
    }
}

static void testReciprocity()
{
    // R(mu0,muv,phi) = R(muv,mu0,phi) for any layer. The single-scattering code
    // this replaces had to break reciprocity at the limb to stay bounded; the
    // doubling solution must hold it everywhere, including there.
    RayleighRT::Solution s;
    RayleighRT::solve(RayleighCorrector::opticalDepthFCI(0), s);

    double worst = 0.0;
    for (int i = 0; i < RayleighRT::Nodes; ++i)
        for (int j = 0; j < RayleighRT::Nodes; ++j)
            for (int m = 0; m < 3; ++m)
                worst = std::max(worst, std::fabs(s.R[m][i][j] - s.R[m][j][i]));

    char msg[128];
    std::snprintf(msg, sizeof msg, "reflection function is reciprocal (worst %.2e)", worst);
    check(worst < 1e-10, msg);
}

static void testThinLimit()
{
    // As tau -> 0 the solution must collapse onto tau*P(Theta)/(4*mu0*muv),
    // which is exactly what the superseded single-scattering code computed.
    const double tau = 1e-5;
    RayleighRT::Solution s;
    RayleighRT::solve(tau, s);

    const double d2r = M_PI / 180.0;
    for (double szaD = 10.0; szaD <= 70.0; szaD += 20.0) {
        for (double vzaD = 10.0; vzaD <= 70.0; vzaD += 20.0) {
            for (double raa = 0.0; raa <= 180.0; raa += 60.0) {
                const double mu0 = std::cos(szaD * d2r), muv = std::cos(vzaD * d2r);
                const double cosT = -mu0 * muv
                                  + std::sin(szaD * d2r) * std::sin(vzaD * d2r)
                                    * std::cos(raa * d2r);
                const double want = tau * RayleighCorrector::phaseFunction(cosT)
                                  / (4.0 * mu0 * muv);
                const double got = RayleighRT::reflectance(s, mu0, muv, raa);
                if (std::fabs(got - want) > 0.02 * want) {
                    std::printf("FAIL : thin limit off at sza %.0f vza %.0f raa %.0f "
                                "(got %.6g want %.6g)\n", szaD, vzaD, raa, got, want);
                    ++g_failures;
                    return;
                }
            }
        }
    }
    check(true, "reduces to single scattering as tau -> 0");
}

static void testPhysicalBounds()
{
    // The bound that actually exists is on *flux*: a layer cannot reflect more
    // than it receives. The reflectance factor is not bounded by 1 and must not
    // be forced to be - a surface can concentrate radiance into a direction, and
    // this layer does exactly that at grazing incidence. (An earlier version of
    // this code clamped rho to 1 on the mistaken assumption that 1 was a
    // physical ceiling for BRF.)
    for (int b = 0; b < RayleighCorrector::SolarBandCount; ++b) {
        const RayleighRT::Solution &s = RayleighRT::forBand(b);
        for (int j = 0; j < RayleighRT::Nodes; ++j) {
            if (!(s.planeAlbedo[j] >= 0.0 && s.planeAlbedo[j] <= 1.0)) {
                std::printf("FAIL : plane albedo %.4f out of range at band %d mu %.4f\n",
                            s.planeAlbedo[j], b, s.mu[j]);
                ++g_failures;
                return;
            }
        }
        if (!(s.sphericalAlbedo >= 0.0 && s.sphericalAlbedo <= 1.0)) {
            std::printf("FAIL : spherical albedo %.4f out of range at band %d\n",
                        s.sphericalAlbedo, b);
            ++g_failures;
            return;
        }
    }
    check(true, "plane and spherical albedo stay within [0,1] for every band");

    // Non-negative and finite over the whole range the correction evaluates.
    double worst = 0.0;
    int worstBand = 0;
    for (int b = 0; b < RayleighCorrector::SolarBandCount; ++b) {
        const RayleighRT::Solution &s = RayleighRT::forBand(b);
        for (double sza = 0.0; sza <= RayleighCorrector::SzaLimit; sza += 1.0)
            for (double vza = 0.0; vza <= RayleighCorrector::VzaLimit; vza += 1.0)
                for (double raa = 0.0; raa <= 180.0; raa += 15.0) {
                    const double r = RayleighRT::reflectance(
                        s, std::cos(sza * M_PI / 180.0), std::cos(vza * M_PI / 180.0), raa);
                    if (!(r >= 0.0) || !std::isfinite(r)) {
                        std::printf("FAIL : negative or NaN at band %d sza %.0f vza %.0f\n",
                                    b, sza, vza);
                        ++g_failures;
                        return;
                    }
                    if (r > worst) { worst = r; worstBand = b; }
                }
    }
    std::printf("info : largest rho inside the evaluated range = %.4f (band %d)\n",
                worst, worstBand);
    check(std::isfinite(worst), "reflectance is finite and non-negative throughout");

    // Spherical albedo must rise with tau and stay modest for a thin atmosphere.
    for (int b = 1; b < RayleighCorrector::SolarBandCount; ++b)
        check(RayleighRT::forBand(b).sphericalAlbedo
                  < RayleighRT::forBand(b - 1).sphericalAlbedo,
              "spherical albedo decreases toward longer wavelengths");
}

static void testMultipleScatteringMatters()
{
    // The point of the exercise: at large air mass single scattering is not a
    // small correction away from the truth. Report the ratio so a regression
    // shows up as a number rather than as a silently different image.
    const RayleighRT::Solution &s = RayleighRT::forBand(0);
    const double tau = RayleighCorrector::opticalDepthFCI(0);
    const double d2r = M_PI / 180.0;

    std::printf("info : vis_04 multiple / single scattering, raa 120\n");
    double ratioAt75 = 0.0;
    for (double vza = 0.0; vza <= 85.0; vza += 17.0) {
        const double mu0 = std::cos(50.0 * d2r), muv = std::cos(vza * d2r);
        const double cosT = -mu0 * muv + std::sin(50.0 * d2r) * std::sin(vza * d2r)
                          * std::cos(120.0 * d2r);
        const double ss = RayleighCorrector::phaseFunction(cosT) / (4.0 * (mu0 + muv))
                        * (1.0 - std::exp(-tau * (1.0 / mu0 + 1.0 / muv)));
        const double ms = RayleighRT::reflectance(s, mu0, muv, 120.0);
        std::printf("info :   sza 50 vza %2.0f : single %.4f  full %.4f  ratio %.3f\n",
                    vza, ss, ms, ms / ss);
        if (vza > 60.0) ratioAt75 = ms / ss;
    }
    check(ratioAt75 > 1.05, "multiple scattering adds materially at large air mass");
}

static void testOceanSurface()
{
    // Fresnel reflectance of water: nearly flat and small until it turns up
    // steeply past 60 degrees. That turn-up is the whole mechanism.
    checkClose(RayleighRT::fresnelWater(1.0), 0.0209, 1e-3, "Fresnel at normal incidence");
    check(RayleighRT::fresnelWater(std::cos(80.0 * M_PI / 180.0)) > 0.3,
          "Fresnel exceeds 0.3 at 80 degrees");
    double prev = -1.0;
    for (double v = 0.0; v <= 89.0; v += 1.0) {
        const double r = RayleighRT::fresnelWater(std::cos(v * M_PI / 180.0));
        check(r >= prev && r <= 1.0, "Fresnel rises monotonically and stays physical");
        prev = r;
    }

    const RayleighRT::Solution &s = RayleighRT::forBand(0);
    const double d2r = M_PI / 180.0;

    // Adding a reflecting surface under the atmosphere can only send more light
    // back up, never less, at every geometry.
    double worstRatio = 1e9;
    for (double sza = 0.0; sza <= 80.0; sza += 5.0)
        for (double vza = 0.0; vza <= 85.0; vza += 5.0)
            for (double raa = 0.0; raa <= 180.0; raa += 30.0) {
                const double r  = RayleighRT::reflectance(s, std::cos(sza * d2r),
                                                          std::cos(vza * d2r), raa, false);
                const double ro = RayleighRT::reflectance(s, std::cos(sza * d2r),
                                                          std::cos(vza * d2r), raa, true);
                if (ro < r) {
                    std::printf("FAIL : sea surface removes light at sza %.0f vza %.0f raa %.0f\n",
                                sza, vza, raa);
                    ++g_failures;
                    return;
                }
                if (r > 0.0) worstRatio = std::min(worstRatio, ro / r);
            }
    check(worstRatio >= 1.0, "sea surface never reduces the reflectance");

    // And the extra must grow with view angle the way Fresnel does - this is
    // what flattens the ocean limb. Measured on a real disc, the residual the
    // black-surface model left behind was 0.0085 / 0.0184 / 0.0446 at vza
    // 60 / 70 / 80; the model must land in that region, not an order off.
    const double mu0 = std::cos(50.0 * d2r);
    const double d60 = RayleighRT::reflectance(s, mu0, std::cos(60.0 * d2r), 90.0, true)
                     - RayleighRT::reflectance(s, mu0, std::cos(60.0 * d2r), 90.0, false);
    const double d80 = RayleighRT::reflectance(s, mu0, std::cos(80.0 * d2r), 90.0, true)
                     - RayleighRT::reflectance(s, mu0, std::cos(80.0 * d2r), 90.0, false);
    std::printf("info : sea-surface term at vza 60 / 80 = %.4f / %.4f "
                "(measured residual 0.0085 / 0.0446)\n", d60, d80);
    check(d80 > 2.5 * d60, "sea-surface term ramps steeply with view angle");
    check(d60 > 0.004 && d60 < 0.02, "sea-surface term at vza 60 is the right size");
    check(d80 > 0.02  && d80 < 0.08, "sea-surface term at vza 80 is the right size");

    // A thinner atmosphere sends less skylight down, so there is less to reflect.
    const RayleighRT::Solution &red = RayleighRT::forBand(2);
    const double dRed = RayleighRT::reflectance(red, mu0, std::cos(80.0 * d2r), 90.0, true)
                      - RayleighRT::reflectance(red, mu0, std::cos(80.0 * d2r), 90.0, false);
    check(dRed < d80, "vis_06 gets less sky glint than vis_04");
}

int main()
{
    testQuadrature();
    testEnergyConservation();
    testReciprocity();
    testThinLimit();
    testPhysicalBounds();
    testMultipleScatteringMatters();
    testOceanSurface();

    std::printf("\n%s\n", g_failures ? "FAILED" : "all checks passed");
    return g_failures ? 1 : 0;
}
