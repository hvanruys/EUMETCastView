// Unit tests for RayleighCorrector. Standalone main(), no test framework.
// Build with: cmake -DBUILD_TESTS=ON ..    Run: bin/rayleigh_test

#include "rayleigh.h"
#include "rayleigh_rt.h"

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
    const float below[] = { 0.0f, 30.0f, 60.0f, 80.0f, 82.9f };
    for (float sza : below)
        checkClose(RayleighCorrector::sunZenithFactor(sza),
                   1.0 / std::cos(sza * d2r), 1e-5,
                   "factor is 1/cos(SZA) below the limit");

    // Frozen at and beyond the limit, at exactly the value pathReflectance's
    // own mu0 floor corresponds to. If these two ever diverge, the difference
    // between the amplified signal and the subtracted rho stops meaning
    // anything and a bright stripe appears along the terminator.
    const double capped = 1.0 / std::cos(RayleighCorrector::SzaLimit * d2r);
    for (float sza = RayleighCorrector::SzaLimit; sza <= 120.0f; sza += 3.0f)
        checkClose(RayleighCorrector::sunZenithFactor(sza), capped, 1e-5,
                   "factor is frozen at and past the limit");

    checkClose(RayleighCorrector::sunZenithFactor(
                   RayleighCorrector::SzaLimit + 0.001f),
               RayleighCorrector::sunZenithFactor(
                   RayleighCorrector::SzaLimit - 0.001f),
               1e-3, "factor is continuous across the limit");   // 1/cos slope is 1.2/deg here

    // Never zero: night has to go black through the subtraction, not by
    // switching the normalisation off.
    check(RayleighCorrector::sunZenithFactor(179.0f) > 0.0f,
          "factor stays positive at every angle");

    // Monotonically non-decreasing, and bounded.
    float prev = 0.0f;
    for (float sza = 0.0f; sza <= 179.0f; sza += 0.5f) {
        const float f = RayleighCorrector::sunZenithFactor(sza);
        check(f >= prev - 1e-6f, "factor never decreases with SZA");
        check(f <= capped + 1e-6, "factor never exceeds the frozen value");
        prev = f;
    }
}

static void testSphericalSolarPath()
{
    // Below SzaLimit nothing has changed: the result is still the plane-parallel
    // BRF, because Chapman equals 1/cos there to a fraction of a percent.
    for (float sza = 0.0f; sza <= 70.0f; sza += 10.0f) {
        const double mu0 = std::cos(sza * M_PI / 180.0);
        const double muv = std::cos(40.0 * M_PI / 180.0);
        const double pp  = RayleighRT::reflectance(
            RayleighRT::forBand(0), mu0, muv, 90.0, false);
        checkClose(RayleighCorrector::pathReflectance(0, sza, 40.0f, 90.0f), pp,
                   0.01, "reduces to plane-parallel in daylight");
    }

    // Past the limit it keeps falling with the real illumination rather than
    // being frozen, and it is still non-zero *past the terminator*, which is
    // the whole point - the atmosphere above a point stays lit after the sun
    // has set on the ground beneath it.
    float prev = 1e9f;
    for (float sza = 84.0f; sza <= 96.0f; sza += 1.0f) {
        const float r = RayleighCorrector::pathReflectance(0, sza, 40.0f, 90.0f);
        // Non-increasing rather than strictly falling: it reaches exactly zero
        // at SzaMax and stays there.
        check(r >= 0.0f && r <= prev, "path reflectance keeps falling through twilight");
        prev = r;
    }
    check(RayleighCorrector::pathReflectance(0, 92.0f, 40.0f, 90.0f)
              < 0.5f * RayleighCorrector::pathReflectance(0, 90.0f, 40.0f, 90.0f),
          "twilight decays fast, not slowly");
    check(RayleighCorrector::pathReflectance(0, 91.0f, 40.0f, 90.0f) > 0.0f,
          "still correcting one degree past the terminator");
    check(RayleighCorrector::pathReflectance(0, RayleighCorrector::SzaMax + 1.0f,
                                             40.0f, 90.0f) == 0.0f,
          "nothing left to correct deep into night");

    // A spherical path lets more light in than a flat one at grazing incidence,
    // so the correction there must be stronger than plane-parallel, not weaker.
    // Under-removal here is what left the terminator blue.
    const double mu0 = std::cos(88.0 * M_PI / 180.0);
    const double muv = std::cos(40.0 * M_PI / 180.0);
    const double ppToa = mu0 * RayleighRT::reflectance(
        RayleighRT::forBand(0), mu0, muv, 90.0, false);
    const double sphToa = RayleighRT::toaPathReflectance(
        RayleighRT::forBand(0), 88.0, muv, 90.0, false);
    std::printf("info : sza 88 TOA path reflectance, spherical %.5f vs flat %.5f "
                "(%.2fx)\n", sphToa, ppToa, sphToa / ppToa);
    check(sphToa > 1.2 * ppToa, "spherical beats plane-parallel at sza 88");
}

static void testSurfaceReflectance()
{
    const double d2r = M_PI / 180.0;

    check(RayleighCorrector::surfaceReflectance(0, 40.0f, 40.0f, 0.0f) == 0.0f,
          "nothing in, nothing out");
    check(RayleighCorrector::surfaceReflectance(0, 40.0f, 40.0f, -0.1f) == 0.0f,
          "negative leftovers give zero, not a negative surface");

    // Round trip. Forward-model what the atmosphere would leave behind for a
    // known surface, invert it, and require the surface back. Exact by
    // construction, so it pins the algebra rather than a tolerance.
    for (int b = 0; b < RayleighCorrector::SolarBandCount; ++b) {
        const RayleighRT::Solution &sol = RayleighRT::forBand(b);
        for (float sza = 0.0f; sza <= 80.0f; sza += 20.0f) {
            for (float vza = 0.0f; vza <= 80.0f; vza += 20.0f) {
                const double t0 = RayleighRT::transmittance(sol, std::cos(sza * d2r));
                const double tv = RayleighRT::transmittance(sol, std::cos(vza * d2r));
                for (double rs = 0.02; rs <= 0.8; rs *= 2.0) {
                    const double x = t0 * tv * rs / (1.0 - sol.sphericalAlbedo * rs);
                    const double got = RayleighCorrector::surfaceReflectance(
                        b, sza, vza, (float)x);
                    if (std::fabs(got - rs) > 1e-4 * std::max(1.0, rs)) {
                        std::printf("FAIL : round trip band %d sza %.0f vza %.0f "
                                    "rho_s %.3f -> %.6f\n", b, sza, vza, rs, got);
                        ++g_failures;
                        return;
                    }
                }
            }
        }
    }
    check(true, "inverts the forward model exactly, every band and geometry");

    // It has to brighten - the surface signal was attenuated on the way out and
    // back - and brighten the blue band far more than the red, since that is
    // where the atmosphere is thick.
    const float x = 0.10f;
    const float blue = RayleighCorrector::surfaceReflectance(0, 40.0f, 40.0f, x);
    const float red  = RayleighCorrector::surfaceReflectance(2, 40.0f, 40.0f, x);
    check(blue > x && red > x, "recovering the surface brightens it");
    check(blue > 1.15f * red, "vis_04 is amplified much more than vis_06");
    std::printf("info : X = 0.10 recovers vis_04 %.4f, vis_06 %.4f\n", blue, red);

    // The amplification must grow toward the limb, where the path out is longest.
    check(RayleighCorrector::surfaceReflectance(0, 40.0f, 80.0f, x)
              > RayleighCorrector::surfaceReflectance(0, 40.0f, 20.0f, x),
          "amplification grows with viewing zenith angle");

    // Bright targets saturate: the ground-to-sky bounce term keeps the recovered
    // reflectance from running away as X approaches and passes 1.
    check(RayleighCorrector::surfaceReflectance(0, 40.0f, 40.0f, 1.0f) < 1.3f,
          "bright targets do not run away");
    float prev = 0.0f;
    for (float v = 0.01f; v <= 2.0f; v += 0.01f) {
        const float r = RayleighCorrector::surfaceReflectance(0, 40.0f, 40.0f, v);
        check(r > prev, "recovered surface increases with what was left behind");
        prev = r;
    }

    // An IR band has no atmosphere to undo, so it must pass straight through.
    check(RayleighCorrector::surfaceReflectance(13, 40.0f, 40.0f, 0.25f) == 0.25f,
          "IR passes through untouched");
}

static void testTwilightFade()
{
    for (float sza = 0.0f; sza <= RayleighCorrector::SzaLimit; sza += 10.0f)
        check(RayleighCorrector::twilightFade(sza) == 1.0f, "no fade in daylight");

    check(RayleighCorrector::twilightFade(RayleighCorrector::SzaMax) == 0.0f,
          "fully faded at SzaMax");
    check(RayleighCorrector::twilightFade(150.0f) == 0.0f, "fully faded at night");

    const float mid = 0.5f * (RayleighCorrector::SzaLimit + RayleighCorrector::SzaMax);
    checkClose(RayleighCorrector::twilightFade(mid), 0.5, 1e-6, "half faded at the midpoint");

    // A visible ramp, not an edge.
    int graded = 0;
    float prev = 1.0f;
    for (float sza = 0.0f; sza <= 100.0f; sza += 0.5f) {
        const float w = RayleighCorrector::twilightFade(sza);
        check(w >= 0.0f && w <= 1.0f, "fade stays within [0,1]");
        check(w <= prev + 1e-7f, "fade is monotonically decreasing");
        if (w > 0.02f && w < 0.98f) ++graded;
        prev = w;
    }
    check(graded > 15, "the fade spans many degrees rather than cutting");
    check(RayleighCorrector::twilightFade(92.0f) > 0.0f,
          "image survives past the terminator");
}

static void testPathReflectance()
{
    const double d2r = M_PI / 180.0;

    check(RayleighCorrector::pathReflectance(13, 30.0f, 20.0f, 60.0f) == 0.0f,
          "ir_105 gets no path reflectance");
    check(RayleighCorrector::pathReflectance(-1, 30.0f, 20.0f, 60.0f) == 0.0f,
          "out-of-range band gets no path reflectance");

    // Near-reciprocal under a sun/view swap. Exact reciprocity belongs to the
    // doubling solution and is tested there; here the solar path is spherical
    // while the view path stays plane-parallel, so the two directions are
    // deliberately no longer interchangeable. At these angles Chapman is within
    // a fraction of a percent of 1/cos, so they still agree to 1e-3.
    checkClose(RayleighCorrector::pathReflectance(0, 25.0f, 55.0f, 100.0f),
               RayleighCorrector::pathReflectance(0, 55.0f, 25.0f, 100.0f),
               1e-3, "near-reciprocal under sun/view swap in daylight");

    // Multiple scattering is included, so the correction must be strictly
    // stronger than the single-scattering value it replaced - and by a margin
    // that grows with air mass. This is the whole point of the LUT.
    for (float vza = 0.0f; vza <= 80.0f; vza += 20.0f) {
        const double tau  = RayleighCorrector::opticalDepthFCI(0);
        const double mu0  = std::cos(50.0 * d2r), muv = std::cos(vza * d2r);
        const double cosT = -mu0 * muv + std::sin(50.0 * d2r) * std::sin(vza * d2r)
                          * std::cos(120.0 * d2r);
        const double ss = RayleighCorrector::phaseFunction(cosT) / (4.0 * (mu0 + muv))
                        * (1.0 - std::exp(-tau * (1.0 / mu0 + 1.0 / muv)));
        check(RayleighCorrector::pathReflectance(0, 50.0f, vza, 120.0f) > 1.1 * ss,
              "exceeds single scattering by more than 10 %");
    }

    // The blue band must be corrected far more than the red one.
    const float blue = RayleighCorrector::pathReflectance(0, 40.0f, 50.0f, 90.0f);
    const float red  = RayleighCorrector::pathReflectance(2, 40.0f, 50.0f, 90.0f);
    check(blue > 3.0f * red, "vis_04 correction is much larger than vis_06");

    // Monotonically increasing in optical depth across the FCI solar bands,
    // which are ordered by decreasing tau.
    for (int b = 1; b < RayleighCorrector::SolarBandCount; ++b)
        check(RayleighCorrector::pathReflectance(b, 35.0f, 45.0f, 80.0f)
                  < RayleighCorrector::pathReflectance(b - 1, 35.0f, 45.0f, 80.0f),
              "path reflectance falls with band optical depth");

    // The ring regression, swept over every band and illumination. The disc
    // interior out to vza 70 is de-hazed; if the outer annulus from there to the
    // limb is corrected any *less* than that, the haze left behind draws a
    // bright blue ring. The taper this replaced made the ratio below exactly
    // zero over the outermost degrees.
    double worstRatio = 1e9;
    for (int b = 0; b < RayleighCorrector::SolarBandCount; ++b) {
        for (float sza = 0.0f; sza <= 88.0f; sza += 2.0f) {
            for (float raa = 0.0f; raa <= 180.0f; raa += 10.0f) {
                const float inner = RayleighCorrector::pathReflectance(b, sza, 70.0f, raa);
                if (inner <= 0.0f)
                    continue;
                for (float vza = 72.0f; vza <= 90.0f; vza += 2.0f) {
                    const float r = RayleighCorrector::pathReflectance(b, sza, vza, raa);
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
}

int main()
{
    testOpticalDepth();
    testPhaseFunction();
    testSunZenithFactor();
    testSphericalSolarPath();
    testSurfaceReflectance();
    testTwilightFade();
    testPathReflectance();

    if (g_failures) {
        std::printf("\n%d check(s) FAILED\n", g_failures);
        return 1;
    }
    std::printf("\nall checks passed\n");
    return 0;
}
