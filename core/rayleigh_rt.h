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

    /** Solar-zenith sampling of the spherical single-scattering table. */
    static constexpr int    SzaGridPoints = 201;
    static constexpr double SzaGridStep   = 0.5;   /**< degrees; covers 0..100 */

    static constexpr double EarthRadiusKm  = 6371.0;
    /** Rayleigh scale height. Sets how fast the air thins, hence Chapman's X. */
    static constexpr double ScaleHeightKm  = 8.0;

    /**
     * Solar zenith beyond which the plane-parallel multiple-scattering term is
     * frozen and merely scaled by how much light still gets in. Single
     * scattering stays spherical at every angle; it is the part that matters
     * near the terminator, and the part a flat atmosphere gets badly wrong.
     */
    static constexpr double MsSzaLimit = 85.0;

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

        /**
         * Same, but with a Fresnel-reflecting water surface as the lower
         * boundary instead of a black one. The difference between this and R is
         * skylight reflected off the sea and transmitted back out - the term
         * that ramps with view angle and brightens the ocean limb.
         *
         * The water body below the interface is treated as black, so
         * subtracting this leaves the water-leaving reflectance, which is the
         * ocean colour one actually wants to see.
         */
        double Rocean[3][Nodes][Nodes];

        /**
         * Spherical single-scattering integral, Iss[sza][muv]:
         *
         *     Iss = integral over t of exp(-t*Ch(z(t),sza) - t/muv) dt, 0..tau
         *
         * with Ch the Chapman air mass at the altitude whose vertical optical
         * depth is t. Plane-parallel would use t/mu0 in place of t*Ch, which
         * diverges at the terminator and shadows the whole column past it; the
         * Chapman path stays finite and keeps the upper atmosphere lit, which
         * is what twilight is.
         *
         * Tabulated because the integral is far too costly per pixel. Sampled
         * every SzaGridStep degrees from 0 to the last row, at the quadrature
         * nodes in muv.
         */
        double Iss[SzaGridPoints][Nodes];

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

    /**
     * Cached solution for an arbitrary optical depth, for instruments whose
     * bands are not in the FCI table.
     *
     * Thread-safe, but it takes a lock and a miss costs a full solve. Resolve
     * once per band outside the pixel loop and hold the reference, which stays
     * valid for the lifetime of the process.
     */
    static const Solution &forTau(double tau);

    /** Refractive index of sea water in the visible. */
    static constexpr double WaterRefractiveIndex = 1.34;

    /** Fresnel reflectance of the water surface for unpolarised light. */
    static double fresnelWater(double mu);

    /**
     * Bidirectional reflectance, bilinear in both cosines, exact in azimuth.
     * @param ocean subtract over water: adds the sea surface as the lower
     *              boundary rather than a black one
     */
    static double reflectance(const Solution &s, double mu0, double muv,
                              double raaDeg, bool ocean = false);

    /** Total transmittance along one path. */
    static double transmittance(const Solution &s, double mu);

    /**
     * Total transmittance along the solar path, spherical.
     *
     * The same quantity as transmittance(), evaluated at the effective cosine
     * 1/Chapman rather than at cos(sza), so it describes the same sun that
     * toaPathReflectance already integrates along. Below about 80 degrees the
     * two agree to a fraction of a percent; past that the flat-atmosphere
     * cosine understates how much light gets through, and it understates it
     * most at large optical depth - which makes the disagreement a colour cast
     * rather than a brightness error.
     *
     * @return 0 where the Earth blocks the path to the sun at ground level
     */
    static double transmittanceSpherical(const Solution &s, double szaDeg);

    /**
     * Chapman air mass: the slant column to the sun through a spherical,
     * exponentially stratified atmosphere, in units of the vertical column
     * above the same point. Replaces the plane-parallel 1/cos(sza).
     *
     * Equal to 1/cos to within a fraction of a percent below 80 degrees, 0.65
     * of it at 88, and finite - about 35 - at 90, where 1/cos is infinite.
     * Past 90 it stays finite for altitudes the sunlight still clears, and
     * returns infinity below that, where the solid Earth blocks the path. That
     * transition is what makes the upper atmosphere glow after local sunset.
     *
     * @return HUGE_VAL when the path to the sun is blocked by the Earth
     */
    static double chapman(double altitudeKm, double szaDeg);

    /**
     * Atmospheric path reflectance in TOA units, pi*L/E0, *without* the 1/mu0
     * normalisation.
     *
     * This is the quantity to work in near the terminator. A BRF carries a
     * 1/mu0 that blows up as the sun sets and is meaningless past it, whereas
     * this stays finite and well defined at every angle - the mu0 cancels out
     * of the single-scattering term exactly.
     *
     * Pseudo-spherical: single scattering integrated along the true Chapman
     * path, multiple scattering taken from the plane-parallel solution at
     * min(sza, MsSzaLimit) and scaled by the ratio of the spherical
     * single-scattering integrals, so it follows the light that actually gets
     * in. Reduces to exactly mu0 * reflectance() wherever Chapman equals 1/cos.
     */
    static double toaPathReflectance(const Solution &s, double szaDeg,
                                     double muv, double raaDeg, bool ocean);
};

#endif // RAYLEIGH_RT_H
