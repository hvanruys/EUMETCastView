#ifndef RAYLEIGH_RT_H
#define RAYLEIGH_RT_H

/**
 * Radiative transfer for a homogeneous, conservatively scattering Rayleigh
 * layer, solved by doubling.
 *
 * This replaces the single-scattering formula that preceded it. Single
 * scattering has no upper bound - its (1 - exp) factor saturates while the
 * 1/(mu0+muv) prefactor keeps growing - so it needed a view-angle clamp and a
 * reflectance ceiling bolted on to stay physical. The doubling solution is
 * bounded, reciprocal and energy-conserving by construction, so both are gone.
 *
 * Scalar, i.e. polarisation is neglected. For pure Rayleigh that costs a few
 * percent; single scattering was costing tens of percent at large air mass.
 *
 * Pure and Qt-free so it can be unit-tested without satellite data. See
 * docs/superpowers/plans/2026-07-26-fci-coupled-atmospheric-correction.md
 */
class RayleighRT
{
public:
    /** Gauss-Legendre quadrature points per hemisphere. */
    static constexpr int Nodes = 32;

    struct Solution
    {
        double tau;
        double mu[Nodes];              /**< quadrature nodes, ascending in (0,1) */
        double wt[Nodes];              /**< quadrature weights                   */

        /**
         * Fourier modes of the reflection function, R[m][outgoing][incoming].
         * The Rayleigh phase function is exactly 1 + c*P2(cos T), so modes
         * above 2 vanish and the azimuth dependence is exact:
         *     rho = R[0] + 2*R[1]*cos(raa) + 2*R[2]*cos(2*raa)
         */
        double R[3][Nodes][Nodes];

        double Ttot[Nodes];            /**< total (direct + diffuse) transmittance */
        double planeAlbedo[Nodes];     /**< reflected flux fraction per incidence  */
        double sphericalAlbedo;        /**< albedo for isotropic illumination      */
    };

    /** Solve from scratch. Roughly 12 Mflop; use forBand() in hot paths. */
    static void solve(double tau, Solution &out);

    /**
     * Cached solution for an FCI solar band (0..7). Built once on first use and
     * safe to call from multiple threads. Returns the tau = 0 solution for IR
     * and out-of-range indices.
     */
    static const Solution &forBand(int bandIndex);

    /** Bidirectional reflectance, bilinear in both cosines, exact in azimuth. */
    static double reflectance(const Solution &s, double mu0, double muv, double raaDeg);

    /** Total transmittance along one path. */
    static double transmittance(const Solution &s, double mu);
};

#endif // RAYLEIGH_RT_H
