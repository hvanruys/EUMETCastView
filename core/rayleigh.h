#ifndef RAYLEIGH_H
#define RAYLEIGH_H

/**
 * Rayleigh scattering correction for MTG FCI solar bands.
 *
 * Pure, stateless functions with no Qt, netCDF or image dependencies, so they
 * can be unit-tested without satellite data.
 *
 * See docs/superpowers/specs/2026-07-25-fci-rayleigh-correction-design.md
 */
class RayleighCorrector
{
public:
    /** FCI solar (VIS/NIR) bands occupy indices 0..7; 8..15 are thermal IR. */
    static constexpr int SolarBandCount = 8;

    /** Sun-zenith angle above which the 1/cos correction is capped, degrees. */
    static constexpr float SzaLimit = 88.0f;
    /** Sun-zenith angle at which the correction reaches zero, degrees. */
    static constexpr float SzaMax   = 95.0f;

    /** Viewing zenith angle above which the correction starts tapering, degrees. */
    static constexpr float VzaTaperStart = 70.0f;
    /** Viewing zenith angle at which the correction reaches zero, degrees. */
    static constexpr float VzaTaperEnd   = 90.0f;

    /**
     * Rayleigh optical depth at sea level for an FCI band index (0..15).
     * Returns 0.0 for IR bands and for out-of-range indices.
     */
    static double opticalDepthFCI(int bandIndex);

    /**
     * Rayleigh optical depth at sea level for an arbitrary wavelength.
     * @param lambdaMicron wavelength in micrometres; <= 0 returns 0.0
     */
    static double opticalDepthAt(double lambdaMicron);

    /**
     * Rayleigh phase function, polarisation-corrected with the Young (1980)
     * depolarisation factor. Normalised so that (1/2)*integral P dmu == 1.
     * @param cosTheta cosine of the scattering angle
     */
    static double phaseFunction(double cosTheta);

    /**
     * Sun-zenith normalisation factor, equivalent to Satpy sunzen_corr_cos.
     * Multiply a radiance-equivalent reflectance by this to get BRF.
     * Returns 0.0 for night, i.e. szaDeg >= maxSzaDeg.
     */
    static float sunZenithFactor(float szaDeg,
                                 float limitDeg  = SzaLimit,
                                 float maxSzaDeg = SzaMax);

    /**
     * Single-scattering Rayleigh path reflectance for a homogeneous layer.
     * Subtract this from a sun-normalised BRF to remove molecular scattering.
     *
     * Both cosines are floored at cos(SzaLimit): from geostationary orbit the
     * viewing zenith angle reaches 90 degrees at the visible disc edge, so the
     * 1/(mu0+muv) term would otherwise diverge at the limb.
     *
     * @param tau    Rayleigh optical depth; <= 0 returns 0
     * @param szaDeg solar zenith angle, degrees
     * @param vzaDeg viewing zenith angle, degrees
     * @param raaDeg relative azimuth angle, degrees, folded into [0, 180]
     */
    static float pathReflectance(double tau, float szaDeg,
                                 float vzaDeg, float raaDeg);

    /**
     * Limb taper weight applied to the path reflectance, 1.0 below
     * VzaTaperStart falling smoothly to 0.0 at VzaTaperEnd.
     *
     * Single-scattering theory breaks down at large optical air mass: the
     * (1 - exp) factor saturates at 1 while the 1/(mu0+muv) prefactor keeps
     * growing, so the raw formula can return values above the physical bound of
     * 1 near the disc edge and over-subtract dark scenes to black. Uses a
     * smoothstep so there is no seam where the taper begins.
     */
    static double limbTaper(float vzaDeg);
};

#endif // RAYLEIGH_H
