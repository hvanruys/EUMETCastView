// Rayleigh scattering correction for MTG FCI solar bands.
//
// Refs:
//   Bodhaine et al. (1999), J. Atmos. Oceanic Technol. 16, 1854-1861
//   Hansen & Travis (1974), Space Sci. Rev. 16, 527-610
//   https://pyspectral.readthedocs.io/en/master/rayleigh_correction.html

#include "rayleigh.h"

#include <cmath>

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
