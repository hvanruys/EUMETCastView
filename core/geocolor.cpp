// GeoColor composite maths for MTG FCI. See geocolor.h.
//
// Ref: Miller et al. (2020), BAMS 101, E1715-E1743.

#include "geocolor.h"

#include <cmath>
#include <algorithm>

namespace {

/** Hermite ramp, 0 at or below lo, 1 at or above hi, C1 at both ends. */
inline float smoothstep(float lo, float hi, float x)
{
    if (!(hi > lo))
        return x >= hi ? 1.0f : 0.0f;
    if (x <= lo) return 0.0f;
    if (x >= hi) return 1.0f;

    const float t = (x - lo) / (hi - lo);
    return t * t * (3.0f - 2.0f * t);
}

/** Linear mix; t == 0 returns a bit-for-bit, which several layers rely on. */
inline float mix(float a, float b, float t)
{
    return a + t * (b - a);
}

inline GeoColorRGB mix(GeoColorRGB a, GeoColorRGB b, float t)
{
    return { mix(a.r, b.r, t), mix(a.g, b.g, t), mix(a.b, b.b, t) };
}

inline float clamp01(float v)
{
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

// Unlit surface colours. Dark enough that cloud and lights carry the night
// side, bright enough that the coastline is still readable in a dark room.
constexpr GeoColorRGB kSeaBase  = { 0.020f, 0.045f, 0.095f };
constexpr GeoColorRGB kLandBase = { 0.058f, 0.052f, 0.043f };

// Cloud hues, normalised so the brighter of the two peaks at 1 and scaled by
// GeoColor::NightCloudGain. High cloud is near-white with a cool cast, the way
// a moonlit anvil reads; low cloud is the cyan the night fog product has used
// since SEVIRI, and keeping it means the same scene is recognisable in both.
// Low sits at about four fifths of high, which is what separates a shallow
// stratus deck from an anvil once both are fully opaque.
constexpr GeoColorRGB kHighCloudHue = { 0.887f, 0.928f, 1.000f };
constexpr GeoColorRGB kLowCloudHue  = { 0.433f, 0.742f, 0.804f };

constexpr GeoColorRGB scaled(GeoColorRGB c, float k)
{
    return { c.r * k, c.g * k, c.b * k };
}

const GeoColorRGB kHighCloud = scaled(kHighCloudHue, GeoColor::NightCloudGain);
const GeoColorRGB kLowCloud  = scaled(kLowCloudHue,  GeoColor::NightCloudGain);

} // namespace

// ---------------------------------------------------------------- day side

float GeoColor::vegetationFraction(float ndvi)
{
    return smoothstep(VegNdviLo, VegNdviHi, ndvi);
}

GeoColorRGB GeoColor::greenVegetation(GeoColorRGB refl, float nir, float veg)
{
    // Only ever brightens. Where the near infrared sits below the green - which
    // is not vegetation by any reading, whatever the index said - the sensible
    // enhancement is none, not a darkening.
    const float headroom = std::max(0.0f, nir - refl.g);

    return { refl.r, refl.g + veg * VegGreenGain * headroom, refl.b };
}

// -------------------------------------------------------------- night side

GeoColorRGB GeoColor::nightBase(bool water)
{
    return water ? kSeaBase : kLandBase;
}

float GeoColor::highCloudFraction(float bt105)
{
    // Falls as the pixel warms, so the ramp is written cold-to-warm and turned
    // around. Doubles as the greyscale that shades mid-level cloud.
    return 1.0f - smoothstep(HighCloudColdK, HighCloudWarmK, bt105);
}

float GeoColor::lowCloudFraction(float bt105, float bt38)
{
    const float btd = bt105 - bt38;

    // A cold anvil shows the same positive difference as fog, since its ice
    // particles are also poor shortwave emitters. What tells them apart is not
    // the difference but the temperature underneath it.
    return smoothstep(LowCloudBtdLo, LowCloudBtdHi, btd)
         * (1.0f - highCloudFraction(bt105));
}

GeoColorRGB GeoColor::nightClouds(GeoColorRGB base, float bt105, float bt38)
{
    // Low first, then high over the top of it: whatever is highest is what the
    // satellite actually sees.
    GeoColorRGB c = mix(base, kLowCloud,  lowCloudFraction(bt105, bt38));
    return       mix(c,    kHighCloud, highCloudFraction(bt105));
}

GeoColorRGB GeoColor::addCityLights(GeoColorRGB base, float lum)
{
    const float a = LightsGain * clamp01(lum);

    return { clamp01(base.r + a * LightsR),
             clamp01(base.g + a * LightsG),
             clamp01(base.b + a * LightsB) };
}

// -------------------------------------------------------------- terminator

float GeoColor::nightWeight(float twilightFade, float dayGamma)
{
    const float w = clamp01(twilightFade);

    if (!(dayGamma > 0.0f))
        return 1.0f - w;

    return 1.0f - std::pow(w, 1.0f / dayGamma);
}

GeoColorRGB GeoColor::blend(GeoColorRGB dayDisplay, GeoColorRGB night, float nightW)
{
    const float n = clamp01(nightW);

    return { clamp01(dayDisplay.r + n * night.r),
             clamp01(dayDisplay.g + n * night.g),
             clamp01(dayDisplay.b + n * night.b) };
}
