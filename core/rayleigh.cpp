// Rayleigh scattering correction for MTG FCI solar bands.
//
// Refs:
//   Bodhaine et al. (1999), J. Atmos. Oceanic Technol. 16, 1854-1861
//   Hansen & Travis (1974), Space Sci. Rev. 16, 527-610
//   https://pyspectral.readthedocs.io/en/master/rayleigh_correction.html

#include "rayleigh.h"
#include "rayleigh_rt.h"

#include <cmath>
#include <algorithm>

namespace {

// Rayleigh optical depth at sea level (P0 = 1013.25 hPa), lambda in micrometres.
double rayleighTau(double lambda)
{
    const double inv2 = 1.0 / (lambda * lambda);
    const double inv4 = inv2 * inv2;
    return 0.008569 * inv4 * (1.0 + 0.0113 * inv2 + 0.00013 * inv4);
}

// FCI solar band centre wavelengths in micrometres, band index 0..7.
// Hardcoded rather than read from the GeoSatellites.ini spectrumvalueslist,
// which is rounded to two decimals (0.86 for a 0.865 um band, a 2.4 % tau
// error) and is user-editable.
const double kFciSolarLambda[RayleighCorrector::SolarBandCount] = {
    0.444,  // 0  vis_04
    0.510,  // 1  vis_05
    0.640,  // 2  vis_06
    0.865,  // 3  vis_08
    0.914,  // 4  vis_09
    1.380,  // 5  nir_13
    1.610,  // 6  nir_16
    2.250   // 7  nir_22
};

// Depolarisation factor for air, Young (1980), Appl. Opt. 19, 3427.
constexpr double kDepolarization = 0.0279;

} // namespace

double RayleighCorrector::opticalDepthAt(double lambdaMicron)
{
    if (lambdaMicron <= 0.0)
        return 0.0;
    return rayleighTau(lambdaMicron);
}

double RayleighCorrector::opticalDepthFCI(int bandIndex)
{
    if (bandIndex < 0 || bandIndex >= SolarBandCount)
        return 0.0;   // IR bands are not Rayleigh-corrected
    return rayleighTau(kFciSolarLambda[bandIndex]);
}

double RayleighCorrector::phaseFunction(double cosTheta)
{
    const double gamma = kDepolarization / (2.0 - kDepolarization);
    const double norm  = 3.0 / (4.0 * (1.0 + 2.0 * gamma));
    return norm * ((1.0 + 3.0 * gamma) + (1.0 - gamma) * cosTheta * cosTheta);
}

float RayleighCorrector::sunZenithFactor(float szaDeg)
{
    const double d2r = M_PI / 180.0;
    const double sza = std::min(static_cast<double>(szaDeg),
                                static_cast<double>(SzaLimit));
    return static_cast<float>(1.0 / std::cos(sza * d2r));
}

float RayleighCorrector::twilightFade(float szaDeg)
{
    if (szaDeg <= SzaLimit)
        return 1.0f;
    if (szaDeg >= SzaMax)
        return 0.0f;

    const float t = (szaDeg - SzaLimit) / (SzaMax - SzaLimit);
    return 1.0f - t * t * (3.0f - 2.0f * t);   // smoothstep, C1 at both ends
}

float RayleighCorrector::cloudFreeFraction(float longBandReflectance)
{
    constexpr float lo = 0.12f;   // clear sea, and thin haze over it
    constexpr float hi = 0.30f;   // solid cloud

    if (longBandReflectance <= lo) return 1.0f;
    if (longBandReflectance >= hi) return 0.0f;

    const float t = (longBandReflectance - lo) / (hi - lo);
    return 1.0f - t * t * (3.0f - 2.0f * t);   // smoothstep
}

float RayleighCorrector::surfaceReflectance(int bandIndex, float szaDeg,
                                            float vzaDeg, float pathRemoved)
{
    if (pathRemoved <= 0.0f)
        return 0.0f;
    if (bandIndex < 0 || bandIndex >= SolarBandCount)
        return pathRemoved;

    const RayleighRT::Solution &s = RayleighRT::forBand(bandIndex);
    const double d2r = M_PI / 180.0;

    // The solar cosine is frozen at SzaLimit, exactly as sunZenithFactor freezes
    // the amplification - and because that factor is 1/mu0_eff, the mu0 that
    // would otherwise appear here cancels against it. What survives is the
    // twilight dimming, carried by the shrinking signal rather than applied.
    const double mu0 = std::cos(std::min(szaDeg, SzaLimit) * d2r);
    const double muv = std::max(std::cos(vzaDeg * d2r), std::cos(VzaLimit * d2r));

    const double tt = RayleighRT::transmittance(s, mu0)
                    * RayleighRT::transmittance(s, muv);

    const double den = tt + s.sphericalAlbedo * (double)pathRemoved;
    if (!(den > 0.0))
        return pathRemoved;

    return static_cast<float>(pathRemoved / den);
}

float RayleighCorrector::waterFraction(float longBandReflectance)
{
    constexpr float lo = 0.05f;   // darker than this is certainly water
    constexpr float hi = 0.13f;   // brighter than this is certainly not

    if (longBandReflectance <= lo) return 1.0f;
    if (longBandReflectance >= hi) return 0.0f;

    const float t = (longBandReflectance - lo) / (hi - lo);
    return 1.0f - t * t * (3.0f - 2.0f * t);   // smoothstep
}

float RayleighCorrector::pathReflectance(int bandIndex, float szaDeg,
                                         float vzaDeg, float raaDeg,
                                         float water)
{
    if (bandIndex < 0 || bandIndex >= SolarBandCount)
        return 0.0f;

    // Deep night. The spherical path never returns exactly zero - there is
    // always some sunlit air somewhere above - but by SzaMax it is down eight
    // orders of magnitude and the pixel is being faded out anyway.
    if (twilightFade(szaDeg) <= 0.0f)
        return 0.0f;

    const double d2r = M_PI / 180.0;

    // Only the view cosine is floored. The solar angle is passed through
    // untouched, because the spherical treatment is defined at every angle -
    // including past the terminator, where a plane-parallel model has nothing
    // to say and twilight is exactly what we are trying to get right.
    const double muv = std::max(std::cos(vzaDeg * d2r), std::cos(VzaLimit * d2r));

    const RayleighRT::Solution &s = RayleighRT::forBand(bandIndex);

    // Work in TOA units and convert with the same frozen factor that scales the
    // signal. Below SzaLimit this is exactly the plane-parallel BRF; past it,
    // toaPathReflectance keeps falling as the real illumination does, which is
    // what the old pathReflectanceScale was approximating by hand.
    // Only blend where the water test is actually undecided. Almost every pixel
    // is plainly one or the other, and each branch costs a full solve lookup.
    double toa;
    if (water <= 0.0f) {
        toa = RayleighRT::toaPathReflectance(s, szaDeg, muv, raaDeg, false);
    } else if (water >= 1.0f) {
        toa = RayleighRT::toaPathReflectance(s, szaDeg, muv, raaDeg, true);
    } else {
        const double land  = RayleighRT::toaPathReflectance(s, szaDeg, muv, raaDeg, false);
        const double ocean = RayleighRT::toaPathReflectance(s, szaDeg, muv, raaDeg, true);
        toa = land + water * (ocean - land);
    }

    return static_cast<float>(sunZenithFactor(szaDeg) * toa);
}
