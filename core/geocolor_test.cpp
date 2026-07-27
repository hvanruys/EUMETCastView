// Unit tests for the GeoColor composite maths.
//
// No satellite data needed: every input is an angle, a reflectance or a
// brightness temperature, and the properties asserted are the ones that make
// the composite correct rather than merely plausible - the terminator being a
// partition of unity, the enhancement leaving everything but vegetation alone,
// and every layer staying inside the display range.

#include "geocolor.h"
#include "rayleigh.h"

#include <cstdio>
#include <cmath>
#include <algorithm>

static int g_failures = 0;
static int g_checks = 0;

static void check(bool ok, const char *what)
{
    ++g_checks;
    if (ok) std::printf("ok   : %s\n", what);
    else  { std::printf("FAIL : %s\n", what); ++g_failures; }
}

static void checkNear(float got, float want, float tol, const char *what)
{
    ++g_checks;
    if (std::fabs(got - want) <= tol) std::printf("ok   : %s\n", what);
    else {
        std::printf("FAIL : %s (got %.6f, want %.6f, tol %.6f)\n",
                    what, got, want, tol);
        ++g_failures;
    }
}

static bool inUnit(const GeoColorRGB &c)
{
    return c.r >= 0.0f && c.r <= 1.0f
        && c.g >= 0.0f && c.g <= 1.0f
        && c.b >= 0.0f && c.b <= 1.0f;
}

// ---------------------------------------------------------------- day side

static void testVegetationFraction()
{
    std::printf("\n--- vegetation fraction ---\n");

    checkNear(GeoColor::vegetationFraction(-1.0f), 0.0f, 0.0f,
              "water NDVI gives no vegetation");
    checkNear(GeoColor::vegetationFraction(0.0f), 0.0f, 0.0f,
              "cloud NDVI gives no vegetation");
    checkNear(GeoColor::vegetationFraction(0.05f), 0.0f, 0.0f,
              "desert NDVI gives no vegetation");
    checkNear(GeoColor::vegetationFraction(GeoColor::VegNdviLo), 0.0f, 0.0f,
              "exactly at the low threshold is still zero");
    checkNear(GeoColor::vegetationFraction(GeoColor::VegNdviHi), 1.0f, 0.0f,
              "exactly at the high threshold is fully vegetated");
    checkNear(GeoColor::vegetationFraction(0.83f), 1.0f, 0.0f,
              "rainforest NDVI is fully vegetated");

    const float mid = 0.5f * (GeoColor::VegNdviLo + GeoColor::VegNdviHi);
    checkNear(GeoColor::vegetationFraction(mid), 0.5f, 1.0e-5f,
              "midpoint is half vegetated");

    // Monotone and inside range across the whole index.
    bool mono = true, bounded = true;
    float prev = -1.0f;
    for (int i = 0; i <= 2000; ++i) {
        const float ndvi = -1.0f + 2.0f * (float)i / 2000.0f;
        const float v = GeoColor::vegetationFraction(ndvi);
        if (v < 0.0f || v > 1.0f) bounded = false;
        if (v < prev - 1.0e-6f) mono = false;
        prev = v;
    }
    check(mono, "vegetation fraction never decreases with NDVI");
    check(bounded, "vegetation fraction stays in 0..1");
}

static void testGreenVegetation()
{
    std::printf("\n--- vegetation greening ---\n");

    const GeoColorRGB forest = { 0.040f, 0.060f, 0.035f };
    const float forestNir = 0.42f;

    // Nothing at all must happen where there is no vegetation. Exact equality,
    // not near: desert and ocean have to come through the composite untouched.
    const GeoColorRGB none = GeoColor::greenVegetation(forest, forestNir, 0.0f);
    check(none.r == forest.r && none.g == forest.g && none.b == forest.b,
          "veg = 0 leaves the colour bit-for-bit unchanged");

    const GeoColorRGB full = GeoColor::greenVegetation(forest, forestNir, 1.0f);
    check(full.r == forest.r && full.b == forest.b,
          "red and blue are never touched");
    check(full.g > forest.g, "green rises over vegetation");
    check(full.g < forestNir,
          "green never reaches the near infrared - it is a nudge, not a swap");
    checkNear(full.g, forest.g + GeoColor::VegGreenGain * (forestNir - forest.g),
              1.0e-6f, "green moves by exactly the gain fraction");

    // The look this exists to produce.
    std::printf("info : forest %.3f/%.3f/%.3f -> %.3f/%.3f/%.3f\n",
                forest.r, forest.g, forest.b, full.r, full.g, full.b);
    check(full.g > full.r && full.g > full.b,
          "greened forest is dominated by its green channel");

    // Monotone in veg, so a soft vegetation edge cannot band.
    bool mono = true;
    float prev = -1.0f;
    for (int i = 0; i <= 1000; ++i) {
        const float veg = (float)i / 1000.0f;
        const float g = GeoColor::greenVegetation(forest, forestNir, veg).g;
        if (g < prev - 1.0e-7f) mono = false;
        prev = g;
    }
    check(mono, "green rises monotonically with vegetation fraction");

    // Cloud: bright in both bands, so NDVI is near zero and nothing happens.
    const GeoColorRGB cloud = { 0.88f, 0.87f, 0.86f };
    const float cloudVeg = GeoColor::vegetationFraction(-0.017f);
    const GeoColorRGB cloudOut = GeoColor::greenVegetation(cloud, 0.85f, cloudVeg);
    check(cloudOut.g == cloud.g, "cloud is not greened");

    // Desert: bright, warm, and barely vegetated.
    const GeoColorRGB sand = { 0.38f, 0.33f, 0.26f };
    const float sandVeg = GeoColor::vegetationFraction(0.05f);
    const GeoColorRGB sandOut = GeoColor::greenVegetation(sand, 0.42f, sandVeg);
    check(sandOut.g == sand.g, "desert is not greened");

    // A dark surface must not be pushed out of range by the gain.
    const GeoColorRGB brightVeg = { 0.30f, 0.35f, 0.25f };
    check(inUnit(GeoColor::greenVegetation(brightVeg, 0.95f, 1.0f)),
          "greening a bright vegetated pixel stays in range");
}

// -------------------------------------------------------------- night side

static void testCloudFractions()
{
    std::printf("\n--- night cloud detection ---\n");

    checkNear(GeoColor::highCloudFraction(300.0f), 0.0f, 0.0f,
              "warm ground is not high cloud");
    checkNear(GeoColor::highCloudFraction(GeoColor::HighCloudWarmK), 0.0f, 0.0f,
              "at the warm bound there is no high cloud");
    checkNear(GeoColor::highCloudFraction(GeoColor::HighCloudColdK), 1.0f, 0.0f,
              "at the cold bound high cloud is total");
    checkNear(GeoColor::highCloudFraction(200.0f), 1.0f, 0.0f,
              "an overshooting top is total high cloud");

    bool mono = true, bounded = true;
    float prev = 2.0f;
    for (int k = 180; k <= 320; ++k) {
        const float f = GeoColor::highCloudFraction((float)k);
        if (f < 0.0f || f > 1.0f) bounded = false;
        if (f > prev + 1.0e-6f) mono = false;      // must fall as it warms
        prev = f;
    }
    check(mono, "high cloud fraction falls monotonically with temperature");
    check(bounded, "high cloud fraction stays in 0..1");

    // Low cloud: the shortwave window reads colder over water droplets.
    checkNear(GeoColor::lowCloudFraction(285.0f, 285.0f), 0.0f, 0.0f,
              "no difference means clear ground, not fog");
    checkNear(GeoColor::lowCloudFraction(285.0f, 289.0f), 0.0f, 0.0f,
              "a negative difference is not fog either");
    check(GeoColor::lowCloudFraction(283.0f, 279.0f) > 0.9f,
          "a 4 K difference over warm ground is solid low cloud");
    check(GeoColor::lowCloudFraction(285.0f, 282.5f) > 0.1f
       && GeoColor::lowCloudFraction(285.0f, 282.5f) < 0.9f,
          "a 2.5 K difference is partial low cloud");

    // The gate that matters: a cold anvil also shows a positive difference,
    // but there is no low cloud being seen through it.
    const float anvil = GeoColor::lowCloudFraction(215.0f, 211.0f);
    check(anvil < 0.05f, "a cold anvil is not reported as fog");
    std::printf("info : anvil low-cloud fraction %.4f\n", anvil);

    bounded = true;
    for (int k = 190; k <= 310; k += 2)
        for (int d = -10; d <= 10; ++d) {
            const float f = GeoColor::lowCloudFraction((float)k, (float)k - (float)d);
            if (f < 0.0f || f > 1.0f) bounded = false;
        }
    check(bounded, "low cloud fraction stays in 0..1 over the whole grid");
}

static void testNightLayer()
{
    std::printf("\n--- night layer ---\n");

    const GeoColorRGB sea  = GeoColor::nightBase(true);
    const GeoColorRGB land = GeoColor::nightBase(false);
    check(inUnit(sea) && inUnit(land), "night bases are in range");
    check(sea.b > sea.r && sea.b > sea.g, "unlit sea is blue-dominant");
    check(land.r >= land.b, "unlit land is warm rather than blue");
    check(sea.r + sea.g + sea.b < 0.5f && land.r + land.g + land.b < 0.5f,
          "both bases are dark - this is the night side");

    // Clear night sky must show the bare surface, untouched.
    const GeoColorRGB clear = GeoColor::nightClouds(land, 290.0f, 290.0f);
    check(clear.r == land.r && clear.g == land.g && clear.b == land.b,
          "clear warm ground returns the night base exactly");

    // Deep convection must read as bright cloud.
    const GeoColorRGB deep = GeoColor::nightClouds(sea, 205.0f, 203.0f);
    check(deep.r > 0.7f && deep.g > 0.7f && deep.b > 0.7f,
          "a cold cloud top is bright");
    check(deep.b >= deep.r, "high cloud carries a cool cast");

    // Fog must be distinguishable from both, and cyan.
    const GeoColorRGB fog = GeoColor::nightClouds(sea, 283.0f, 278.0f);
    check(fog.g > fog.r && fog.b > fog.r, "fog is cyan, not white");
    check(fog.g > sea.g, "fog is brighter than the sea it sits on");
    std::printf("info : sea %.3f/%.3f/%.3f  fog %.3f/%.3f/%.3f  deep %.3f/%.3f/%.3f\n",
                sea.r, sea.g, sea.b, fog.r, fog.g, fog.b, deep.r, deep.g, deep.b);

    bool bounded = true;
    for (int k = 190; k <= 310; k += 1)
        for (int d = -8; d <= 8; ++d) {
            if (!inUnit(GeoColor::nightClouds(sea,  (float)k, (float)k - (float)d))) bounded = false;
            if (!inUnit(GeoColor::nightClouds(land, (float)k, (float)k - (float)d))) bounded = false;
        }
    check(bounded, "night layer stays in 0..1 over the whole grid");

    // City lights.
    const GeoColorRGB unlit = GeoColor::addCityLights(land, 0.0f);
    check(unlit.r == land.r && unlit.g == land.g && unlit.b == land.b,
          "zero luminance adds nothing");
    const GeoColorRGB lit = GeoColor::addCityLights(land, 1.0f);
    check(lit.r > land.r && lit.g > land.g && lit.b > land.b, "lights brighten");
    check(lit.r > lit.b, "city light is warm");
    check(inUnit(lit), "fully lit stays in range");

    bool litMono = true;
    float prev = -1.0f;
    for (int i = 0; i <= 1000; ++i) {
        const float v = GeoColor::addCityLights(land, (float)i / 1000.0f).r;
        if (v < prev - 1.0e-7f) litMono = false;
        prev = v;
    }
    check(litMono, "lights brighten monotonically with luminance");
}

// -------------------------------------------------------------- terminator

static void testTerminator()
{
    std::printf("\n--- terminator ---\n");

    const float gamma = 2.2f;

    checkNear(GeoColor::nightWeight(1.0f, gamma), 0.0f, 1.0e-6f,
              "full day gives the night layer no weight");
    checkNear(GeoColor::nightWeight(0.0f, gamma), 1.0f, 1.0e-6f,
              "full night gives the night layer all the weight");

    bool mono = true;
    float prev = 2.0f;
    for (int i = 0; i <= 1000; ++i) {
        const float w = GeoColor::nightWeight((float)i / 1000.0f, gamma);
        if (w > prev + 1.0e-6f) mono = false;
        prev = w;
    }
    check(mono, "night weight falls monotonically as day takes over");

    // The property the whole design turns on. For a scene of uniform unfaded
    // day colour D and night colour N, the composite must be an exact convex
    // combination of the two at every solar zenith angle - never brighter than
    // both, never darker than both. That is what stops the terminator showing
    // as a bright or dark seam.
    float worstOver = 0.0f, worstUnder = 0.0f;
    float worstSza = 0.0f;
    for (float D = 0.0f; D <= 1.0f; D += 0.1f) {
        for (float N = 0.0f; N <= 1.0f; N += 0.1f) {
            for (float sza = 70.0f; sza <= 100.0f; sza += 0.1f) {
                const float fade = RayleighCorrector::twilightFade(sza);

                // What the pipeline actually hands the compositor: the day
                // reflectance carries the fade, then goes through the gamma.
                const float dayDisplay = std::pow(fade * D, 1.0f / gamma);
                const float nw = GeoColor::nightWeight(fade, gamma);
                const GeoColorRGB out = GeoColor::blend({dayDisplay, dayDisplay, dayDisplay},
                                                        {N, N, N}, nw);

                const float unfadedDay = std::pow(D, 1.0f / gamma);
                const float lo = std::min(unfadedDay, N) - 1.0e-5f;
                const float hi = std::max(unfadedDay, N) + 1.0e-5f;

                if (out.r > hi && out.r - hi > worstOver)  { worstOver  = out.r - hi; worstSza = sza; }
                if (out.r < lo && lo - out.r > worstUnder) { worstUnder = lo - out.r; worstSza = sza; }
            }
        }
    }
    std::printf("info : worst terminator overshoot %.2e, undershoot %.2e (near sza %.1f)\n",
                worstOver, worstUnder, worstSza);
    check(worstOver < 1.0e-4f,
          "the terminator never brightens beyond the brighter of the two layers");
    check(worstUnder < 1.0e-4f,
          "the terminator never darkens below the darker of the two layers");

    // And it has to actually get all the way to each end.
    {
        const float fadeDay = RayleighCorrector::twilightFade(30.0f);
        const float D = 0.6f;
        const float dayDisplay = std::pow(fadeDay * D, 1.0f / gamma);
        const GeoColorRGB out = GeoColor::blend({dayDisplay, dayDisplay, dayDisplay},
                                                {0.05f, 0.05f, 0.05f},
                                                GeoColor::nightWeight(fadeDay, gamma));
        checkNear(out.r, std::pow(D, 1.0f / gamma), 1.0e-5f,
                  "well inside the day the night layer is absent");
    }
    {
        const float fadeNight = RayleighCorrector::twilightFade(100.0f);
        const GeoColorRGB out = GeoColor::blend({0.0f, 0.0f, 0.0f},
                                                {0.05f, 0.07f, 0.12f},
                                                GeoColor::nightWeight(fadeNight, gamma));
        checkNear(out.b, 0.12f, 1.0e-5f,
                  "well past the terminator the night layer is all there is");
    }

    // Smoothness: no visible step anywhere across the handover. Sampled at
    // 0.01 deg, a step larger than one 8-bit level would be a seam.
    float worstStep = 0.0f, stepSza = 0.0f;
    const float D = 0.5f, N = 0.09f;
    float prevOut = -1.0f;
    for (float sza = 75.0f; sza <= 100.0f; sza += 0.01f) {
        const float fade = RayleighCorrector::twilightFade(sza);
        const float dayDisplay = std::pow(fade * D, 1.0f / gamma);
        const float out = GeoColor::blend({dayDisplay, dayDisplay, dayDisplay},
                                          {N, N, N},
                                          GeoColor::nightWeight(fade, gamma)).r;
        if (prevOut >= 0.0f && std::fabs(out - prevOut) > worstStep) {
            worstStep = std::fabs(out - prevOut);
            stepSza = sza;
        }
        prevOut = out;
    }
    std::printf("info : worst step per 0.01 deg is %.5f (%.3f of an 8-bit level) near sza %.2f\n",
                worstStep, worstStep * 255.0f, stepSza);
    check(worstStep * 255.0f < 1.0f, "no visible step anywhere across the terminator");

    bool bounded = true;
    for (float sza = 0.0f; sza <= 120.0f; sza += 0.5f) {
        const float fade = RayleighCorrector::twilightFade(sza);
        const float dayDisplay = std::pow(fade * 1.0f, 1.0f / gamma);
        if (!inUnit(GeoColor::blend({dayDisplay, dayDisplay, dayDisplay},
                                    {1.0f, 1.0f, 1.0f},
                                    GeoColor::nightWeight(fade, gamma))))
            bounded = false;
    }
    check(bounded, "the composite stays in 0..1 at every solar zenith angle");
}

int main()
{
    testVegetationFraction();
    testGreenVegetation();
    testCloudFractions();
    testNightLayer();
    testTerminator();

    std::printf("\n%d checks, %s\n", g_checks,
                g_failures ? "FAILED" : "all checks passed");
    return g_failures ? 1 : 0;
}
