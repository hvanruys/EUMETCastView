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

// SEVIRI solar channel centre wavelengths in micrometres, channel number 1..3.
// PDF_MSG_SEVIRI_RAD2REFL nominal band centres. The thermal channels and HRV
// are not in the table; see wavelengthSEVIRI.
const double kSeviriSolarLambda[3] = {
    0.635,  // 1  VIS006
    0.810,  // 2  VIS008
    1.640   // 3  IR_016
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

double RayleighCorrector::wavelengthSEVIRI(int channelNbr)
{
    if (channelNbr < 1 || channelNbr > 3)
        return 0.0;   // thermal channels, HRV, and anything out of range
    return kSeviriSolarLambda[channelNbr - 1];
}

double RayleighCorrector::opticalDepthSEVIRI(int channelNbr)
{
    return opticalDepthAt(wavelengthSEVIRI(channelNbr));
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

float RayleighCorrector::pathTrust(float szaDeg)
{
    if (szaDeg <= SzaTrustFull)
        return 1.0f;
    if (szaDeg >= SzaTrustNone)
        return 0.0f;

    const float t = (szaDeg - SzaTrustFull) / (SzaTrustNone - SzaTrustFull);
    return 1.0f - t * t * (3.0f - 2.0f * t);   // smoothstep, C1 at both ends
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

    return surfaceReflectance(RayleighRT::forBand(bandIndex),
                              szaDeg, vzaDeg, pathRemoved);
}

float RayleighCorrector::surfaceReflectance(const RayleighRT::Solution &s,
                                            float szaDeg, float vzaDeg,
                                            float pathRemoved, float trust)
{
    if (pathRemoved <= 0.0f)
        return 0.0f;
    if (s.tau <= 0.0)
        return pathRemoved;   // no atmosphere to see through

    const double d2r = M_PI / 180.0;

    // The solar leg is spherical, like the path term it is undoing. It used to
    // be a plane-parallel cosine frozen at SzaLimit, on the argument that the
    // mu0 cancels against the amplification - which is true of the geometric
    // cosine, but not of the transmittance, which never entered that
    // cancellation. Freezing it understated how much light reaches the ground
    // past 83 degrees, and understated it in proportion to optical depth: at
    // sza 88 the recovered VIS006 kept 83 % of its true value against 99 % for
    // IR_016. A blue deficit that grows with sun angle and shows up wherever
    // the scene is dark and neutral - which is why the terminator went red over
    // water and nowhere else.
    //
    // What survives is still only the grey twilight dimming, mu0_true/mu0_eff,
    // carried by the shrinking signal rather than applied here.
    const double muv = std::max(std::cos(vzaDeg * d2r), std::cos(VzaLimit * d2r));

    double tt = RayleighRT::transmittanceSpherical(s, szaDeg)
              * RayleighRT::transmittance(s, muv);

    // Both halves of the correction retreat together; see the declaration.
    tt = 1.0 + trust * (tt - 1.0);

    // Dividing by a transmittance is ill-conditioned once that transmittance
    // gets small, and past the terminator it goes to zero: nothing bounds the
    // recovery, and a dark twilight pixel would come back as a bright one. Cap
    // the amplification. In daylight, and well into twilight, this never binds
    // - the gain is 1.1 at nadir and about 3 at sza 90 in the worst band.
    tt = std::max(tt, 1.0 / MaxSurfaceGain);

    const double den = tt + trust * s.sphericalAlbedo * (double)pathRemoved;
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

    return pathReflectance(RayleighRT::forBand(bandIndex),
                           szaDeg, vzaDeg, raaDeg, water);
}

float RayleighCorrector::pathReflectance(const RayleighRT::Solution &s,
                                         float szaDeg, float vzaDeg,
                                         float raaDeg, float water, float trust)
{
    if (s.tau <= 0.0)
        return 0.0f;   // nothing to scatter

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

    return static_cast<float>(sunZenithFactor(szaDeg) * toa * trust);
}
