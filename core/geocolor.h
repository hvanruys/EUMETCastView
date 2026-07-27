#ifndef GEOCOLOR_H
#define GEOCOLOR_H

/**
 * GeoColor composite for MTG FCI - the colour maths, and nothing else.
 *
 * Pure and stateless, with no Qt, netCDF or image dependencies, so it can be
 * unit-tested without satellite data. The satellite-facing half - reading
 * bands, geolocating pixels, sampling the lights image - lives in
 * SegmentListGeostationary.
 *
 * After Miller et al. (2020), "A Sight for Sore Eyes: The Return of True Color
 * to Geostationary Satellites", BAMS 101, E1715-E1743. Two departures from the
 * published recipe, both forced by what this instrument and this machine have:
 *
 *   - No hybrid green. GeoColor synthesises one for ABI, which has no band
 *     between 0.47 and 0.64 um. FCI has vis_05 at 0.51, so the green channel is
 *     measured rather than inferred, and only the vegetation enhancement below
 *     is needed.
 *   - City lights are optional. The published product paints the night side
 *     with a VIIRS DNB annual composite. None is bundled here, so the night
 *     base is drawn from the shoreline mask and lights are added only if the
 *     user supplies an image.
 */

struct GeoColorRGB {
    float r, g, b;
};

class GeoColor
{
public:

    // ------------------------------------------------------------------
    //  Day side
    // ------------------------------------------------------------------

    /**
     * NDVI at which vegetation starts, and at which it is fully established.
     *
     * Below the low end sit desert, rock and sand - the Sahara runs about 0.05
     * - and cloud, which lands near zero because it is bright in both bands.
     * That is what keeps the enhancement off everything it should not touch,
     * without needing a cloud mask: an index near zero is already the answer.
     */
    static constexpr float VegNdviLo = 0.15f;
    static constexpr float VegNdviHi = 0.55f;

    /**
     * How far the green channel is pulled toward the near infrared over fully
     * vegetated ground.
     *
     * Chlorophyll absorbs at 0.51 um as well as in the red - less, but enough
     * that a strictly true-colour render puts dense forest at a dark olive
     * around RGB (60, 71, 56). This is the one deliberately cosmetic step in
     * the composite: it exists because a forest looks green to a person
     * standing in it, not because 0.51 um says so. 0.12 lifts that same forest
     * to about (60, 91, 56) - unmistakably green, while leaving desert, ocean
     * and cloud exactly where they were, since all three have veg = 0.
     */
    static constexpr float VegGreenGain = 0.12f;

    /** How vegetated a pixel is, 0..1, smoothstepped over the NDVI band above. */
    static float vegetationFraction(float ndvi);

    /**
     * Pull the green channel of a corrected surface reflectance toward the near
     * infrared, in proportion to how vegetated the pixel is.
     *
     * Red and blue are left untouched. Saturating the greens by pushing the
     * other two down would reach the same look with a second knob doing the
     * first one's job, and it would also darken the scene.
     *
     * @param refl corrected surface reflectance, red/green/blue
     * @param nir  reflectance in vis_08, the near-infrared band NDVI uses
     * @param veg  vegetation fraction from vegetationFraction, 0..1
     */
    static GeoColorRGB greenVegetation(GeoColorRGB refl, float nir, float veg);

    // ------------------------------------------------------------------
    //  Night side
    // ------------------------------------------------------------------

    /**
     * Brightness temperatures bounding the high-cloud ramp, kelvin.
     *
     * Warm end is near a mild land surface, cold end is high cloud well above
     * the freezing level. Between them the ramp doubles as the greyscale that
     * gives mid-level cloud its shading.
     */
    static constexpr float HighCloudWarmK = 275.0f;
    static constexpr float HighCloudColdK = 235.0f;

    /**
     * Brightness temperature difference ir_105 - ir_38 bounding the low-cloud
     * ramp, kelvin.
     *
     * Small water droplets emit less efficiently at 3.8 um than at 10.5, so a
     * night-time water cloud reads colder in the shortwave window and the
     * difference goes positive. Clear ground sits near zero, fog and stratus
     * run 2 to 6 K. This is the standard night fog/low-stratus test and it only
     * works at night, which is the only place it is used.
     */
    static constexpr float LowCloudBtdLo = 1.0f;
    static constexpr float LowCloudBtdHi = 4.0f;

    /** Unlit surface colour: deep navy sea, very dark warm grey land. */
    static GeoColorRGB nightBase(bool water);

    /** How much high or thick cloud, 0..1, from the 10.5 um window. */
    static float highCloudFraction(float bt105);

    /**
     * How much low cloud or fog, 0..1, from the 10.5 minus 3.8 um difference.
     *
     * Gated by the high-cloud fraction: the test is about what sits nearest the
     * ground, and once a cold anvil is in the way the satellite is not seeing
     * the ground or anything just above it.
     */
    static float lowCloudFraction(float bt105, float bt38);

    /** Lay low cloud then high cloud over a night base, in that order. */
    static GeoColorRGB nightClouds(GeoColorRGB base, float bt105, float bt38);

    /**
     * Add city lights over the night base.
     *
     * Warm-tinted, because sodium and most street LED is, and additive rather
     * than blended: a lit city does not hide the ground, it emits on top of it.
     *
     * @param lum luminance sampled from the lights image, 0..1
     */
    static GeoColorRGB addCityLights(GeoColorRGB base, float lum);

    /** Colour of the light added at full luminance. */
    static constexpr float LightsR = 1.00f;
    static constexpr float LightsG = 0.82f;
    static constexpr float LightsB = 0.55f;

    /**
     * Brightness of a fully lit pixel.
     *
     * Below 1 on purpose: a city core should read as bright, not as a hole
     * punched in the disc, and leaving headroom means cloud lit from below
     * still separates from the city under it.
     */
    static constexpr float LightsGain = 0.85f;

    // ------------------------------------------------------------------
    //  Terminator
    // ------------------------------------------------------------------

    /**
     * How much the night layer counts, from the twilight fade already applied
     * to the day reflectances.
     *
     * The day layer arrives pre-multiplied by RayleighCorrector::twilightFade,
     * in reflectance, before the display gamma. Gamma is not linear, so a fade
     * of w does not dim the displayed pixel to w - it dims it to w^(1/gamma).
     * Weighting the night side by 1-w against that would brighten the whole
     * terminator, because the two would sum to more than one.
     *
     * Taking the fade through the same gamma first makes the pair exact: the
     * displayed day colour is w^(1/gamma) times what it would have been unfaded,
     * so 1 - w^(1/gamma) is precisely what is left for the night. The two
     * layers then form a partition of unity and the terminator neither
     * brightens nor darkens as it crosses.
     */
    static float nightWeight(float twilightFade, float dayGamma);

    /**
     * Lay the night layer under an already-faded day layer.
     *
     * An add, not a mix, and that is the whole point: the day term is
     * w^(1/gamma) * D and the night term is (1 - w^(1/gamma)) * N, so the sum is
     * an exact convex combination of the unfaded day colour and the night one.
     * Mixing here would apply the fade twice.
     *
     * @param dayDisplay day layer after range and gamma, 0..1, fade included
     * @param night      night layer, 0..1
     * @param nightW     from nightWeight()
     */
    static GeoColorRGB blend(GeoColorRGB dayDisplay, GeoColorRGB night, float nightW);
};

#endif // GEOCOLOR_H
