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

float RayleighCorrector::pathReflectance(int bandIndex, float szaDeg,
                                         float vzaDeg, float raaDeg)
{
    if (bandIndex < 0 || bandIndex >= SolarBandCount)
        return 0.0f;

    const double d2r = M_PI / 180.0;

    // Floor both cosines. sza matches sunZenithFactor's limit so the two halves
    // of the chain agree; vza stops at VzaLimit because the model is not worth
    // trusting nearer the limb than that.
    const double mu0 = std::max(std::cos(szaDeg * d2r), std::cos(SzaLimit * d2r));
    const double muv = std::max(std::cos(vzaDeg * d2r), std::cos(VzaLimit * d2r));

    return static_cast<float>(
        RayleighRT::reflectance(RayleighRT::forBand(bandIndex), mu0, muv, raaDeg));
}
