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
     *
     * No longer a numerical guard - the doubling solution is finite and
     * reciprocal all the way to the horizon. It is a statement about trust: the
     * layer is plane-parallel and the solver is scalar with 32 streams, and
     * within a few degrees of the limb none of those hold well enough to justify
     * subtracting what the model returns (rho exceeds 3 in the twilight corner).
     * Past this the correction plateaus at its VzaLimit value.
     */
    static constexpr float VzaLimit = 85.0f;

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
     * Rayleigh path reflectance for an FCI solar band, all scattering orders.
     * Subtract this from a sun-normalised BRF to remove molecular scattering.
     *
     * Evaluates the doubling solution in RayleighRT, which supersedes the
     * single-scattering formula this used to compute. Multiple scattering is not
     * a small correction here: at tau 0.234 it adds 32 % at nadir and 44 % at
     * grazing view, so the old formula under-removed haze everywhere and most at
     * the limb.
     *
     * The solar cosine is floored at cos(SzaLimit) to match sunZenithFactor's
     * own limit, and the viewing cosine at cos(VzaLimit) - see that constant.
     * The correction is deliberately never tapered away toward the limb: fading
     * it out leaves the haze it was meant to remove in place, and since the disc
     * interior is de-hazed the leftover reads as a bright blue ring.
     *
     * @param bandIndex FCI band 0..7; IR and out-of-range indices return 0
     * @param szaDeg    solar zenith angle, degrees
     * @param vzaDeg    viewing zenith angle, degrees
     * @param raaDeg    relative azimuth angle, degrees, folded into [0, 180]
     */
    static float pathReflectance(int bandIndex, float szaDeg,
                                 float vzaDeg, float raaDeg);
};

#endif // RAYLEIGH_H
