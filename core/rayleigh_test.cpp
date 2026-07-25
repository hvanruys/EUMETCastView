// Unit tests for RayleighCorrector. Standalone main(), no test framework.
// Build with: cmake -DBUILD_TESTS=ON ..    Run: bin/rayleigh_test

#include "rayleigh.h"

#include <cmath>
#include <cstdio>

static int g_failures = 0;

static void check(bool cond, const char *what)
{
    if (cond) {
        std::printf("ok   : %s\n", what);
    } else {
        std::printf("FAIL : %s\n", what);
        ++g_failures;
    }
}

static void checkClose(double got, double want, double relTol, const char *what)
{
    const double denom = (want == 0.0) ? 1.0 : std::fabs(want);
    if (std::fabs(got - want) / denom <= relTol) {
        std::printf("ok   : %s\n", what);
    } else {
        std::printf("FAIL : %s (got %.8g, want %.8g)\n", what, got, want);
        ++g_failures;
    }
}

static void testOpticalDepth()
{
    // Bodhaine et al. (1999) evaluated at the FCI solar band centres.
    const double want[RayleighCorrector::SolarBandCount] = {
        0.23387128, 0.13240915, 0.052523854, 0.015540855,
        0.012446874, 0.0023768326, 0.0012809263, 0.00033509703
    };
    const char *name[RayleighCorrector::SolarBandCount] = {
        "tau vis_04", "tau vis_05", "tau vis_06", "tau vis_08",
        "tau vis_09", "tau nir_13", "tau nir_16", "tau nir_22"
    };

    for (int i = 0; i < RayleighCorrector::SolarBandCount; ++i)
        checkClose(RayleighCorrector::opticalDepthFCI(i), want[i], 0.01, name[i]);

    check(RayleighCorrector::opticalDepthFCI(8)  == 0.0, "ir_38 tau is zero");
    check(RayleighCorrector::opticalDepthFCI(15) == 0.0, "ir_133 tau is zero");
    check(RayleighCorrector::opticalDepthFCI(-1) == 0.0, "negative index tau is zero");
    check(RayleighCorrector::opticalDepthFCI(99) == 0.0, "out-of-range index tau is zero");

    check(RayleighCorrector::opticalDepthAt(0.0)  == 0.0, "zero wavelength tau is zero");
    check(RayleighCorrector::opticalDepthAt(-1.0) == 0.0, "negative wavelength tau is zero");

    // Blue scatters far more than red: this ratio is the whole point.
    checkClose(RayleighCorrector::opticalDepthFCI(0) /
               RayleighCorrector::opticalDepthFCI(2),
               4.4527, 0.01, "vis_04 / vis_06 tau ratio is about 4.45");

    // Monotonically decreasing with wavelength across the solar bands.
    for (int i = 1; i < RayleighCorrector::SolarBandCount; ++i)
        check(RayleighCorrector::opticalDepthFCI(i) <
              RayleighCorrector::opticalDepthFCI(i - 1),
              "tau decreases with increasing wavelength");
}

static void testPhaseFunction()
{
    // Normalisation: (1/2) * integral of P(mu) dmu over mu in [-1, 1] == 1.
    const int n = 200000;
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        const double mu = -1.0 + (2.0 * (i + 0.5)) / n;
        sum += RayleighCorrector::phaseFunction(mu);
    }
    checkClose(0.5 * sum * (2.0 / n), 1.0, 1e-6,
               "phase function normalises to 1 over the sphere");

    // Even in cos(Theta).
    checkClose(RayleighCorrector::phaseFunction(0.5),
               RayleighCorrector::phaseFunction(-0.5), 1e-12,
               "phase function is even in cos(Theta)");

    // Peaks at forward and back scatter, minimum at 90 degrees.
    check(RayleighCorrector::phaseFunction(1.0) >
          RayleighCorrector::phaseFunction(0.0),
          "phase function peaks at forward/back scatter");
    check(RayleighCorrector::phaseFunction(-1.0) >
          RayleighCorrector::phaseFunction(0.0),
          "phase function peaks at back scatter too");

    // Always positive.
    for (double mu = -1.0; mu <= 1.0; mu += 0.1)
        check(RayleighCorrector::phaseFunction(mu) > 0.0,
              "phase function is positive");
}

static void testSunZenithFactor()
{
    const double d2r = M_PI / 180.0;

    // Plain 1/cos below the limit.
    const float below[] = { 0.0f, 30.0f, 60.0f, 85.0f, 87.9f };
    for (float sza : below)
        checkClose(RayleighCorrector::sunZenithFactor(sza),
                   1.0 / std::cos(sza * d2r), 1e-5,
                   "factor is 1/cos(SZA) below the 88 degree limit");

    // Continuous across the limit: both branches meet at 1/cos(88).
    checkClose(RayleighCorrector::sunZenithFactor(88.001f),
               RayleighCorrector::sunZenithFactor(87.999f), 1e-3,
               "factor is continuous across 88 degrees");
    checkClose(RayleighCorrector::sunZenithFactor(88.0f),
               1.0 / std::cos(88.0 * d2r), 1e-5,
               "factor equals 1/cos(88) at the limit");

    // Reaches exactly zero at max_sza and stays there.
    check(RayleighCorrector::sunZenithFactor(95.0f)  == 0.0f, "factor is zero at 95 degrees");
    check(RayleighCorrector::sunZenithFactor(110.0f) == 0.0f, "factor is zero past 95 degrees");
    check(RayleighCorrector::sunZenithFactor(180.0f) == 0.0f, "factor is zero at the antisolar point");

    // Monotonically decreasing through the falloff, never negative.
    float prev = RayleighCorrector::sunZenithFactor(88.0f);
    for (float sza = 88.5f; sza <= 95.0f; sza += 0.5f) {
        const float f = RayleighCorrector::sunZenithFactor(sza);
        check(f >= 0.0f, "falloff is never negative");
        check(f <= prev, "falloff is monotonically decreasing");
        prev = f;
    }
}

static void testPathReflectance()
{
    const double d2r = M_PI / 180.0;

    check(RayleighCorrector::pathReflectance(0.0, 30.0f, 20.0f, 60.0f) == 0.0f,
          "zero optical depth gives zero path reflectance");

    // As tau -> 0 the layer solution reduces to tau*P(Theta)/(4*mu0*muv).
    {
        const double tau = 0.001;
        const float sza = 30.0f, vza = 20.0f, raa = 60.0f;
        const double mu0  = std::cos(sza * d2r);
        const double muv  = std::cos(vza * d2r);
        const double sin0 = std::sin(sza * d2r);
        const double sinv = std::sin(vza * d2r);
        const double cosT = -mu0 * muv + sin0 * sinv * std::cos(raa * d2r);
        const double want = tau * RayleighCorrector::phaseFunction(cosT)
                          / (4.0 * mu0 * muv);
        checkClose(RayleighCorrector::pathReflectance(tau, sza, vza, raa),
                   want, 0.01, "matches the small-tau single-scattering limit");
    }

    // Reciprocity: swapping sun and view geometry must leave rho unchanged.
    checkClose(RayleighCorrector::pathReflectance(0.2339, 25.0f, 55.0f, 100.0f),
               RayleighCorrector::pathReflectance(0.2339, 55.0f, 25.0f, 100.0f),
               1e-6, "reciprocal under sun/view swap");

    // Finite at the limb, where vza reaches 90 degrees from geostationary orbit.
    const float limb = RayleighCorrector::pathReflectance(0.2339, 40.0f, 90.0f, 30.0f);
    check(std::isfinite(limb), "finite at vza = 90 degrees");
    check(limb > 0.0f, "positive at vza = 90 degrees");
    check(limb < 1.0f, "below 1 at vza = 90 degrees");

    // Monotonically increasing in tau across the FCI range.
    float prev = 0.0f;
    for (double tau = 0.0005; tau <= 0.24; tau *= 1.5) {
        const float r = RayleighCorrector::pathReflectance(tau, 35.0f, 45.0f, 80.0f);
        check(r > prev, "path reflectance increases with tau");
        prev = r;
    }

    // The blue band must be corrected far more than the red one.
    const float blue = RayleighCorrector::pathReflectance(
        RayleighCorrector::opticalDepthFCI(0), 40.0f, 50.0f, 90.0f);
    const float red = RayleighCorrector::pathReflectance(
        RayleighCorrector::opticalDepthFCI(2), 40.0f, 50.0f, 90.0f);
    check(blue > 3.0f * red, "vis_04 correction is much larger than vis_06");

    // IR bands get no correction at all.
    check(RayleighCorrector::pathReflectance(
              RayleighCorrector::opticalDepthFCI(13), 40.0f, 50.0f, 90.0f) == 0.0f,
          "ir_105 gets no path reflectance");
}

int main()
{
    testOpticalDepth();
    testPhaseFunction();
    testSunZenithFactor();
    testPathReflectance();

    if (g_failures) {
        std::printf("\n%d check(s) FAILED\n", g_failures);
        return 1;
    }
    std::printf("\nall checks passed\n");
    return 0;
}
