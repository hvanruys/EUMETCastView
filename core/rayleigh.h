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

    /**
     * Viewing zenith angle beyond which the geometry is frozen, degrees.
     * Plane-parallel single scattering stops describing a real, curved
     * atmosphere near grazing view; past this the correction plateaus at its
     * VzaLimit value instead of following the diverging 1/muv.
     *
     * 80 degrees is the largest angle at which this model still agrees with 6S
     * (0.098 against about 0.10 for vis_06 at sza 40, backscatter), and it is
     * r/R = 0.985 on the disc, so the whole visible limb annulus is corrected at
     * full strength. Freezing rather than extrapolating past it also limits how
     * far the correction can over-redden the outermost pixels: it removes
     * molecular scattering only, while at grazing view the unmodelled aerosol
     * contribution is largest.
     */
    static constexpr float VzaLimit = 80.0f;

    /**
     * Relative air mass at the horizon, Kasten & Young (1989). The plane-
     * parallel 1/cos diverges at 90 degrees; the real spherical-shell value
     * saturates near 38.
     */
    static constexpr double HorizonAirMass = 38.0;

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
     * The solar cosine is floored at cos(SzaLimit) and the viewing cosine at
     * cos(VzaLimit): from geostationary orbit the viewing zenith angle reaches
     * 90 degrees at the visible disc edge, so 1/(mu0+muv) would otherwise
     * diverge at the limb. The result is then eased smoothly into
     * maxPathReflectance(tau) so it can never exceed what the layer is able to
     * reflect. Both bounds leave the sub-VzaLimit result untouched to well
     * within the model's own accuracy.
     *
     * The correction is deliberately *not* tapered away toward the limb.
     * Fading it out leaves the haze it was meant to remove in place, and since
     * the disc interior is de-hazed the leftover reads as a bright blue ring.
     *
     * @param tau    Rayleigh optical depth; <= 0 returns 0
     * @param szaDeg solar zenith angle, degrees
     * @param vzaDeg viewing zenith angle, degrees
     * @param raaDeg relative azimuth angle, degrees, folded into [0, 180]
     */
    static float pathReflectance(double tau, float szaDeg,
                                 float vzaDeg, float raaDeg);

    /**
     * Largest path reflectance a Rayleigh layer of this optical depth can
     * produce: the conservative two-stream plane albedo at horizon air mass,
     * R = (3/4)*tau*M / (1 + (3/4)*tau*M), with M = HorizonAirMass.
     *
     * Single scattering has no such bound - its (1 - exp) factor saturates at 1
     * while the 1/(mu0+muv) prefactor keeps growing - so at very large air mass
     * it returns reflectances above 1. Rayleigh scattering is conservative
     * (no absorption), so this is the true ceiling, and being derived from tau
     * it is per-band rather than a tuned constant.
     */
    static double maxPathReflectance(double tau);
};

#endif // RAYLEIGH_H
