#ifndef RAYLEIGH_H
#define RAYLEIGH_H

#include "rayleigh_rt.h"

/**
 * Rayleigh scattering correction for MTG FCI and MSG SEVIRI solar bands.
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

    /**
     * Solar zenith angle at which the geometry freezes, degrees.
     *
     * One number drives both halves of the chain: the 1/cos amplification and
     * the mu0 the path reflectance is evaluated at. They have to agree. When
     * they did not - amplification running on to 26x at 88 degrees while
     * pathReflectance sat frozen at its own floor - the shortfall between them
     * was amplified into a bright blue stripe down the whole terminator.
     * Measured over open ocean, B-R ran from -11 at sza 82 to +45 at sza 89,
     * for an ocean that should be near black.
     *
     * 83 degrees is where that measurement says the plane-parallel model stops
     * being trustworthy: B-R holds around -30 out to 82 and starts drifting
     * blue past 84. Freezing there caps the amplification at 8.2x, and past it
     * the scene darkens on its own as the real radiance falls away beneath a
     * fixed rho - no roll-off needed to force it.
     */
    static constexpr float SzaLimit = 83.0f;

    /** Solar zenith angle at which the twilight fade reaches zero, degrees. */
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
     * Centre wavelength of a SEVIRI solar channel, micrometres.
     *
     * SEVIRI has no blue channel, so there is far less to remove than on FCI:
     * its bluest channel, VIS006, sits at the wavelength of FCI's *least*
     * corrected true-colour band. Returns 0 for the thermal channels, for HRV -
     * which is broadband from 0.4 to 1.1 um and has no single tau - and for
     * channel numbers outside 1..12.
     *
     * @param channelNbr SEVIRI channel number as GetSpectralChannelNbr gives it
     */
    static double wavelengthSEVIRI(int channelNbr);

    /** Rayleigh optical depth at sea level for a SEVIRI channel number. */
    static double opticalDepthSEVIRI(int channelNbr);

    /**
     * Rayleigh phase function, polarisation-corrected with the Young (1980)
     * depolarisation factor. Normalised so that (1/2)*integral P dmu == 1.
     * @param cosTheta cosine of the scattering angle
     */
    static double phaseFunction(double cosTheta);

    /**
     * Sun-zenith normalisation factor: 1/cos of the effective solar zenith.
     * Multiply a radiance-equivalent reflectance by this to get BRF.
     *
     * Bounded at 1/cos(SzaLimit). pathReflectance converts its TOA-unit result
     * back through this same factor, so the amplification and the thing being
     * subtracted always describe the same sun. Never zero - night goes black
     * because the illumination collapses, not because the factor is switched off.
     *
     * This deliberately drops the logarithmic roll-off of Satpy's
     * sunzen_corr_cos. Satpy fades the normalisation because it has nothing to
     * subtract afterwards; here a fading factor against a frozen rho is exactly
     * the mismatch that drew the terminator stripe.
     */
    static float sunZenithFactor(float szaDeg);

    /**
     * Twilight fade, 1.0 up to SzaLimit falling smoothly to 0.0 at SzaMax.
     * Multiplies the corrected reflectance, so it dims every band alike and
     * cannot tint anything.
     *
     * Kept as a backstop now that the solar path is spherical. The reason it
     * existed - a plane-parallel model recovering only a third of the observed
     * radiance past sza 86, leaving blue error behind - is what the Chapman
     * treatment addresses directly. What remains is that nothing is trustworthy
     * within a few degrees of the terminator, so the image is still carried to
     * black rather than left to whatever the model last said.
     */
    static float twilightFade(float szaDeg);

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
     * Only the viewing cosine is floored, at cos(VzaLimit) - see that constant.
     * The solar angle is passed through untouched, because the spherical solar
     * path is defined at every angle including past the terminator.
     *
     * The correction is deliberately never tapered away toward the limb: fading
     * it out leaves the haze it was meant to remove in place, and since the disc
     * interior is de-hazed the leftover reads as a bright blue ring.
     *
     * @param bandIndex FCI band 0..7; IR and out-of-range indices return 0
     * @param szaDeg    solar zenith angle, degrees
     * @param vzaDeg    viewing zenith angle, degrees
     * @param raaDeg    relative azimuth angle, degrees, folded into [0, 180]
     * @param water     0 for a black lower boundary, 1 for a Fresnel sea
     *                  surface, blended in between
     *
     * Evaluated through RayleighRT::toaPathReflectance, so the solar path is
     * spherical (Chapman) rather than plane-parallel and the result stays
     * meaningful past the terminator.
     */
    static float pathReflectance(int bandIndex, float szaDeg,
                                 float vzaDeg, float raaDeg,
                                 float water = 0.0f);

    /**
     * As above, for a solution the caller has already resolved.
     *
     * This is the form to use for an instrument outside the FCI band table:
     * resolve once with RayleighRT::forTau, outside the pixel loop, and hand
     * the same reference to every pixel. The band-index form is a wrapper
     * around this one, so the physics is shared.
     */
    static float pathReflectance(const RayleighRT::Solution &s, float szaDeg,
                                 float vzaDeg, float raaDeg,
                                 float water = 0.0f, float trust = 1.0f);

    /**
     * Recover the surface reflectance from what is left after the path
     * reflectance has been taken off.
     *
     * Subtracting the path term does not leave the surface. It leaves the
     * surface seen *through* the atmosphere:
     *
     *     X = T(mu0)*T(muv) * rho_s / (1 - s*rho_s)
     *
     * with T the total (direct + diffuse) transmittance each way and s the
     * atmosphere's spherical albedo, which accounts for light bouncing between
     * ground and air. Inverting is closed-form, no iteration:
     *
     *     rho_s = X / (T(mu0)*T(muv) + s*X)
     *
     * Both T and s come out of the same doubling solution as the path term.
     *
     * The effect is a brightening that grows with air mass and with optical
     * depth, so it is much stronger in vis_04 than vis_06 - which is the point.
     * Skipping it leaves a residual blue attenuation that deepens toward the
     * limb, because that is where the surface signal is most attenuated.
     *
     * @param pathRemoved X above; <= 0 returns 0
     */
    static float surfaceReflectance(int bandIndex, float szaDeg,
                                    float vzaDeg, float pathRemoved);

    /**
     * As above, for a solution the caller has already resolved.
     *
     * @param trust how much of the model to believe, from pathTrust. Both
     *              halves of the correction have to retreat together: a pixel
     *              that keeps its full transmittance recovery after the
     *              subtraction has been faded is left too blue by exactly the
     *              amount the subtraction was going to remove. At trust 0 this
     *              is the identity. Default 1 leaves the physics untouched.
     */
    static float surfaceReflectance(const RayleighRT::Solution &s, float szaDeg,
                                    float vzaDeg, float pathRemoved,
                                    float trust = 1.0f);

    /**
     * How much a pixel should be treated as open water, from its reflectance in
     * the longest-wavelength solar band available.
     *
     * Water absorbs strongly toward the red and near-infrared, so it is far
     * darker there than land, cloud or snow. Blended rather than thresholded:
     * a hard mask would draw its own edges into the image, and a misjudged
     * pixel degrades gradually instead of jumping.
     *
     * Thresholds sit in the gap measured on a real disc - recovered vis_06 runs
     * about 0.03-0.05 over clear ocean against 0.18-0.32 over land.
     *
     * Only a fallback now, for when no shoreline file is available. It cannot
     * tell dark vegetation from ocean, which is what LandSeaMask is for.
     */
    static float waterFraction(float longBandReflectance);

    /**
     * How much of a pixel geography already calls water is actually open sea
     * rather than cloud on top of it.
     *
     * With a shoreline mask deciding land from water, brightness is left with
     * only the second question, and can be far more permissive: it no longer
     * has to hold a line against land at 0.18, only against cloud. Clear ocean
     * sits at 0.03-0.05 in vis_06 and stays fully water, while thick cloud
     * switches the sea surface off, because a cloud is what the satellite sees
     * instead of the sea, not as well as it.
     */
    static float cloudFreeFraction(float longBandReflectance);

    /** Shortest wavelength usable for waterFraction, micrometres. */
    static constexpr double MinWaterTestLambda = 0.6;

    /**
     * Sun angles between which the path term is progressively distrusted.
     *
     * Measured on real discs: the ratio of modelled path reflectance to
     * measured BRF sits around 0.3 while the sun is up, reaches 0.74 by sza 83
     * and passes 1.0 by sza 85 - the model claims more light than the satellite
     * recorded, so the whole measurement is consumed and the deepest band is
     * left at zero. Since the shallow bands are barely touched, that is a red
     * cast along the terminator, which is exactly what it looked like.
     *
     * The cure is to stop trusting the path term where it stops being credible,
     * and to stop trusting it in every band at once, so what remains keeps its
     * colour. This is deliberately not the roll-off the design rejected: that
     * one faded the correction in broad daylight, where the haze it left behind
     * stood next to corrected pixels and read as a ring. Here the twilight fade
     * is already carrying the same pixels to black.
     */
    static constexpr float SzaTrustFull = 80.0f;
    static constexpr float SzaTrustNone = 88.0f;

    /**
     * How much of the modelled path reflectance to believe, 1 down to 0 across
     * SzaTrustFull..SzaTrustNone.
     *
     * Calibrated on MSG discs and passed in explicitly by the SEVIRI path,
     * rather than applied inside pathReflectance where it would reach every
     * instrument. It does not transfer: what a taper leaves behind is the haze
     * itself, which scales with optical depth, and FCI's bluest band is at tau
     * 0.234 against SEVIRI's 0.054. Tapering FCI on these thresholds turned its
     * twilight blue - the very failure the no-roll-off rule in pathReflectance
     * was written to prevent. Do not enable it for another instrument without
     * measuring that instrument's own path-to-measured ratio first.
     */
    static float pathTrust(float szaDeg);

    /**
     * Largest factor by which surfaceReflectance may amplify what is left after
     * the path term has been taken off.
     *
     * Undoing the two-way transmittance is a division, and past the terminator
     * the divisor goes to zero - the ground is in the Earth's shadow, so no
     * amount of algebra recovers what it reflected. Without a cap a dark
     * twilight pixel comes back bright, which is worse than leaving it dark.
     *
     * 4 is chosen to be inert where the model is trustworthy: the gain is about
     * 1.1 at nadir, 1.7 at sza 88 and 3 at sza 90 in the deepest band, so this
     * only ever engages in the last couple of degrees before the shadow.
     */
    static constexpr double MaxSurfaceGain = 4.0;
};

#endif // RAYLEIGH_H
