// Rayleigh scattering correction for MTG FCI solar bands.
//
// Refs:
//   Bodhaine et al. (1999), J. Atmos. Oceanic Technol. 16, 1854-1861
//   Hansen & Travis (1974), Space Sci. Rev. 16, 527-610
//   https://pyspectral.readthedocs.io/en/master/rayleigh_correction.html

#include "rayleigh.h"

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

float RayleighCorrector::sunZenithFactor(float szaDeg, float limitDeg, float maxSzaDeg)
{
    const double d2r      = M_PI / 180.0;
    const double limitRad = limitDeg * d2r;
    const double limitCos = std::cos(limitRad);

    if (szaDeg < limitDeg)
        return static_cast<float>(1.0 / std::cos(szaDeg * d2r));

    // Satpy sunzen_corr_cos: logarithmic falloff from limitDeg to maxSzaDeg,
    // reaching exactly zero at maxSzaDeg so the terminator fades smoothly
    // instead of cutting hard at the limit.
    const double maxRad = maxSzaDeg * d2r;
    double grad = (szaDeg * d2r - limitRad) / (maxRad - limitRad);
    grad = 1.0 - std::log(grad + 1.0) / std::log(2.0);
    if (grad < 0.0)
        grad = 0.0;

    return static_cast<float>(grad / limitCos);
}

double RayleighCorrector::limbTaper(float vzaDeg)
{
    if (vzaDeg <= VzaTaperStart)
        return 1.0;
    if (vzaDeg >= VzaTaperEnd)
        return 0.0;

    const double t = (vzaDeg - VzaTaperStart) / (VzaTaperEnd - VzaTaperStart);
    return 1.0 - t * t * (3.0 - 2.0 * t);   // smoothstep: C1 continuous at both ends
}

float RayleighCorrector::pathReflectance(double tau, float szaDeg,
                                         float vzaDeg, float raaDeg)
{
    if (tau <= 0.0)
        return 0.0f;

    const double taper = limbTaper(vzaDeg);
    if (taper == 0.0)
        return 0.0f;

    const double d2r = M_PI / 180.0;

    // Floor both cosines. vza reaches 90 degrees at the visible disc edge, so
    // 1/(mu0+muv) would diverge there. Clamping saturates the correction over
    // the last degree or two of an already-smeared limb; masking instead would
    // leave a visible black ring around the disc.
    const double muFloor = std::cos(SzaLimit * d2r);
    const double mu0 = std::max(std::cos(szaDeg * d2r), muFloor);
    const double muv = std::max(std::cos(vzaDeg * d2r), muFloor);

    const double sin0 = std::sqrt(std::max(0.0, 1.0 - mu0 * mu0));
    const double sinv = std::sqrt(std::max(0.0, 1.0 - muv * muv));

    const double cosScatter = -mu0 * muv + sin0 * sinv * std::cos(raaDeg * d2r);

    double rho = phaseFunction(cosScatter) / (4.0 * (mu0 + muv))
               * (1.0 - std::exp(-tau * (1.0 / mu0 + 1.0 / muv)));

    // An atmosphere cannot reflect more than it receives. At very large air mass
    // - deep twilight seen at a slant, roughly SZA > 85 with VZA > 70 - the
    // single-scattering formula exceeds 1 because its (1 - exp) factor saturates
    // while the 1/(mu0+muv) prefactor keeps growing. Bound it before tapering.
    rho = std::min(rho, 1.0);

    // Taper out toward the disc edge, where single scattering over-predicts.
    return static_cast<float>(rho * taper);
}
