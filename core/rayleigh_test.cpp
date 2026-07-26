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

    // Reciprocity: swapping sun and view geometry leaves rho unchanged, as long
    // as both angles are below VzaLimit (the view-angle clamp is a function of
    // vza only and so deliberately breaks strict reciprocity above it).
    checkClose(RayleighCorrector::pathReflectance(0.2339, 25.0f, 55.0f, 100.0f),
               RayleighCorrector::pathReflectance(0.2339, 55.0f, 25.0f, 100.0f),
               1e-6, "reciprocal under sun/view swap below the view clamp");

    // Still correcting at the limb. Tapering the correction away toward the disc
    // edge leaves the haze it was meant to remove in place, and because the
    // interior around it *is* de-hazed the leftover shows up as a bright blue
    // ring. rho must therefore keep rising with vza, not collapse.
    const float limb = RayleighCorrector::pathReflectance(0.2339, 40.0f, 90.0f, 30.0f);
    check(std::isfinite(limb), "finite at vza = 90 degrees");
    check(limb > 0.0f, "still corrects at vza = 90 degrees");
    check(limb >= RayleighCorrector::pathReflectance(0.2339, 40.0f, 80.0f, 30.0f),
          "limb correction is at least as strong as at vza = 80");

    // The ring regression, swept over every band and illumination. The disc
    // interior out to vza 70 is de-hazed; if the outer annulus from there to the
    // limb is corrected any *less* than that, the haze left behind draws a
    // bright blue ring. The taper this replaced made the ratio below exactly
    // zero over the outermost degrees.
    double worstRatio = 1e9;
    for (int b = 0; b < RayleighCorrector::SolarBandCount; ++b) {
        const double tau = RayleighCorrector::opticalDepthFCI(b);
        for (float sza = 0.0f; sza <= 88.0f; sza += 2.0f) {
            for (float raa = 0.0f; raa <= 180.0f; raa += 10.0f) {
                const float inner = RayleighCorrector::pathReflectance(tau, sza, 70.0f, raa);
                if (inner <= 0.0f)
                    continue;
                for (float vza = 72.0f; vza <= 90.0f; vza += 2.0f) {
                    const float r = RayleighCorrector::pathReflectance(tau, sza, vza, raa);
                    if (r < inner) {
                        std::printf("FAIL : limb under-corrected at band %d sza %.0f "
                                    "vza %.0f raa %.0f (%.5f at vza 70 -> %.5f)\n",
                                    b, sza, vza, raa, inner, r);
                        ++g_failures;
                        return;
                    }
                    worstRatio = std::min(worstRatio, (double)(r / inner));
                }
            }
        }
    }
    std::printf("info : weakest limb correction is %.3fx the vza 70 value\n", worstRatio);
    check(true, "the limb is never corrected less than the disc interior");

    // The regime where single scattering is trustworthy must be left alone: no
    // clamp or ceiling may perturb it, or the fix would darken the whole disc.
    for (float sza = 0.0f; sza <= 70.0f; sza += 10.0f) {
        for (float vza = 0.0f; vza <= 70.0f; vza += 10.0f) {
            const double tau  = RayleighCorrector::opticalDepthFCI(0);
            const double mu0  = std::cos(sza * d2r);
            const double muv  = std::cos(vza * d2r);
            const double cosT = -mu0 * muv
                              + std::sin(sza * d2r) * std::sin(vza * d2r)
                                * std::cos(120.0 * d2r);
            const double raw  = RayleighCorrector::phaseFunction(cosT)
                              / (4.0 * (mu0 + muv))
                              * (1.0 - std::exp(-tau * (1.0 / mu0 + 1.0 / muv)));
            checkClose(RayleighCorrector::pathReflectance(tau, sza, vza, 120.0f),
                       raw, 0.02, "untouched where single scattering holds");
        }
    }

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

static void testMaxPathReflectance()
{
    check(RayleighCorrector::maxPathReflectance(0.0) == 0.0,
          "zero optical depth has a zero ceiling");

    // Conservative two-stream plane albedo of the layer at horizon air mass:
    // R = (3/4)*tau*M / (1 + (3/4)*tau*M).
    for (int b = 0; b < RayleighCorrector::SolarBandCount; ++b) {
        const double tau = RayleighCorrector::opticalDepthFCI(b);
        const double x   = 0.75 * tau * RayleighCorrector::HorizonAirMass;
        checkClose(RayleighCorrector::maxPathReflectance(tau), x / (1.0 + x), 1e-9,
                   "ceiling matches the conservative two-stream albedo");
    }

    // Strictly increasing in tau and always a physical reflectance.
    double prev = 0.0;
    for (double tau = 0.0002; tau <= 0.25; tau *= 1.3) {
        const double m = RayleighCorrector::maxPathReflectance(tau);
        check(m > prev, "ceiling increases with tau");
        check(m > 0.0 && m < 1.0, "ceiling stays a physical reflectance");
        prev = m;
    }

    // A thin-atmosphere band can barely scatter; the blue band scatters a lot.
    check(RayleighCorrector::maxPathReflectance(
              RayleighCorrector::opticalDepthFCI(7)) < 0.05,
          "nir_22 ceiling is negligible");
    check(RayleighCorrector::maxPathReflectance(
              RayleighCorrector::opticalDepthFCI(0)) > 0.5,
          "vis_04 ceiling is substantial");
}

static void testPhysicalBound()
{
    // A Rayleigh atmosphere cannot reflect more than it receives. Sweep the
    // entire FCI geometry range and confirm rho never exceeds 1 - the single-
    // scattering formula diverges at large air mass without the limb taper.
    double worst = 0.0;
    float worstSza = 0.0f, worstVza = 0.0f, worstRaa = 0.0f;
    int worstBand = 0;

    for (int b = 0; b < RayleighCorrector::SolarBandCount; ++b) {
        const double tau = RayleighCorrector::opticalDepthFCI(b);
        for (float sza = 0.0f; sza <= 95.0f; sza += 1.0f) {
            for (float vza = 0.0f; vza <= 90.0f; vza += 1.0f) {
                for (float raa = 0.0f; raa <= 180.0f; raa += 15.0f) {
                    const double r = RayleighCorrector::pathReflectance(tau, sza, vza, raa);
                    if (!(r >= 0.0)) {
                        std::printf("FAIL : negative or NaN rho at band %d sza %.0f vza %.0f raa %.0f\n",
                                    b, sza, vza, raa);
                        ++g_failures;
                        return;
                    }
                    if (r > RayleighCorrector::maxPathReflectance(tau) + 1e-6) {
                        std::printf("FAIL : rho %.4f above the band ceiling %.4f "
                                    "at band %d sza %.0f vza %.0f raa %.0f\n",
                                    r, RayleighCorrector::maxPathReflectance(tau),
                                    b, sza, vza, raa);
                        ++g_failures;
                        return;
                    }
                    if (r > worst) {
                        worst = r; worstBand = b;
                        worstSza = sza; worstVza = vza; worstRaa = raa;
                    }
                }
            }
        }
    }

    std::printf("info : worst-case rho = %.4f (band %d, sza %.0f, vza %.0f, raa %.0f)\n",
                worst, worstBand, worstSza, worstVza, worstRaa);
    check(worst <= 1.0, "path reflectance never exceeds the physical bound of 1");
}

int main()
{
    testOpticalDepth();
    testPhaseFunction();
    testSunZenithFactor();
    testPathReflectance();
    testMaxPathReflectance();
    testPhysicalBound();

    if (g_failures) {
        std::printf("\n%d check(s) FAILED\n", g_failures);
        return 1;
    }
    std::printf("\nall checks passed\n");
    return 0;
}
