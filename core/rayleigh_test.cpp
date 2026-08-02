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
    // These two used to assert that the correction is never tapered and is
    // still working past the terminator. Measured on real MSG discs, that is
    // where it does damage: by sza 85 the modelled path reflectance exceeds the
    // measured BRF, so the subtraction consumes the whole signal and leaves the
    // deepest band at zero while the shallow ones are untouched - a red band
    // along the terminator. pathTrust now retires the path term over
    // SzaTrustFull..SzaTrustNone, before it can claim more light than arrived.
    // The taper is opt-in and reaches only the caller that asks for it. FCI,
    // which uses the band-index form, must still be correcting here - fading it
    // leaves tau 0.234 of haze in place and turns its twilight blue.
    check(RayleighCorrector::pathReflectance(0, RayleighCorrector::SzaTrustNone,
                                             40.0f, 90.0f) > 0.0f,
          "the untapered path is untouched past the trust limit");
    {
        const RayleighRT::Solution &b0 = RayleighRT::forBand(0);
        const float sza = RayleighCorrector::SzaTrustNone;
        check(RayleighCorrector::pathReflectance(b0, sza, 40.0f, 90.0f, 0.0f,
                  RayleighCorrector::pathTrust(sza)) == 0.0f,
              "a caller that asks for the taper gets it");
        const float lit = RayleighCorrector::SzaTrustFull - 0.1f;
        checkClose(RayleighCorrector::pathReflectance(b0, lit, 40.0f, 90.0f, 0.0f,
                       RayleighCorrector::pathTrust(lit)),
                   RayleighCorrector::pathReflectance(b0, lit, 40.0f, 90.0f),
                   1e-6, "and below the trust limit the two agree exactly");
    }
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
    //
    // The solar leg of the forward model is spherical, because that is the leg
    // the inversion undoes. It used to be a plane-parallel cosine on both sides,
    // which agreed with itself but not with the path term being subtracted.
    for (int b = 0; b < RayleighCorrector::SolarBandCount; ++b) {
        const RayleighRT::Solution &sol = RayleighRT::forBand(b);
        for (float sza = 0.0f; sza <= 80.0f; sza += 20.0f) {
            for (float vza = 0.0f; vza <= 80.0f; vza += 20.0f) {
                const double t0 = RayleighRT::transmittanceSpherical(sol, sza);
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

static void testSeviri()
{
    // SEVIRI's bluest channel sits where FCI's least-corrected true-colour band
    // sits, which is the whole story of how much there is to remove here.
    checkClose(RayleighCorrector::opticalDepthSEVIRI(1),
               RayleighCorrector::opticalDepthAt(0.635), 1e-12, "VIS006 tau is at 0.635 um");
    checkClose(RayleighCorrector::opticalDepthSEVIRI(1), 0.0542, 0.01, "VIS006 tau near 0.054");
    checkClose(RayleighCorrector::opticalDepthSEVIRI(2), 0.0203, 0.01, "VIS008 tau near 0.020");
    checkClose(RayleighCorrector::opticalDepthSEVIRI(3), 0.0012, 0.05, "IR_016 tau near 0.001");

    check(RayleighCorrector::opticalDepthSEVIRI(1) < RayleighCorrector::opticalDepthFCI(0),
          "VIS006 has less to remove than FCI vis_04");
    check(RayleighCorrector::opticalDepthSEVIRI(1) > RayleighCorrector::opticalDepthSEVIRI(2) &&
          RayleighCorrector::opticalDepthSEVIRI(2) > RayleighCorrector::opticalDepthSEVIRI(3),
          "tau falls with wavelength across the three solar channels");

    // Thermal channels, HRV and nonsense channel numbers are not corrected.
    check(RayleighCorrector::wavelengthSEVIRI(4)  == 0.0, "IR_039 has no Rayleigh wavelength");
    check(RayleighCorrector::wavelengthSEVIRI(9)  == 0.0, "IR_108 has no Rayleigh wavelength");
    check(RayleighCorrector::wavelengthSEVIRI(12) == 0.0, "broadband HRV is not corrected");
    check(RayleighCorrector::opticalDepthSEVIRI(0)  == 0.0, "channel 0 is not corrected");
    check(RayleighCorrector::opticalDepthSEVIRI(99) == 0.0, "out-of-range channel is not corrected");

    // The solution-keyed overloads have to agree with the band-keyed ones, or
    // the two instruments are silently running different physics. FCI vis_06 at
    // 0.640 um is close enough to VIS006 to compare directly.
    const RayleighRT::Solution &s6 = RayleighRT::forTau(RayleighCorrector::opticalDepthFCI(2));
    checkClose(RayleighCorrector::pathReflectance(s6, 40.0f, 30.0f, 70.0f),
               RayleighCorrector::pathReflectance(2, 40.0f, 30.0f, 70.0f),
               1e-6, "solution-keyed path reflectance matches the band-keyed one");
    checkClose(RayleighCorrector::surfaceReflectance(s6, 40.0f, 30.0f, 0.12f),
               RayleighCorrector::surfaceReflectance(2, 40.0f, 30.0f, 0.12f),
               1e-6, "solution-keyed surface reflectance matches the band-keyed one");

    // forTau caches by value: the same tau must hand back the same object, or
    // every band would pay a 12 Mflop solve on every lookup.
    check(&RayleighRT::forTau(0.0542) == &RayleighRT::forTau(0.0542),
          "forTau returns the cached solution for a repeated tau");
    check(&RayleighRT::forTau(0.0542) != &RayleighRT::forTau(0.0203),
          "forTau keeps distinct taus apart");

    // A zero-tau solution must leave a reflectance exactly as it found it, so
    // that a channel with no Rayleigh term is a no-op rather than a small edit.
    const RayleighRT::Solution &s0 = RayleighRT::forTau(0.0);
    check(RayleighCorrector::pathReflectance(s0, 40.0f, 30.0f, 70.0f) == 0.0f,
          "no optical depth means no path reflectance");
    check(RayleighCorrector::surfaceReflectance(s0, 40.0f, 30.0f, 0.2f) == 0.2f,
          "no optical depth means no surface recovery");

    // Ordering that decides whether the correction helps or hurts: what is
    // removed from VIS006 must exceed what is removed from VIS008, and IR_016
    // must be left essentially untouched.
    const RayleighRT::Solution &v6 = RayleighRT::forTau(RayleighCorrector::opticalDepthSEVIRI(1));
    const RayleighRT::Solution &v8 = RayleighRT::forTau(RayleighCorrector::opticalDepthSEVIRI(2));
    const RayleighRT::Solution &i16 = RayleighRT::forTau(RayleighCorrector::opticalDepthSEVIRI(3));
    const float r6  = RayleighCorrector::pathReflectance(v6,  50.0f, 45.0f, 90.0f);
    const float r8  = RayleighCorrector::pathReflectance(v8,  50.0f, 45.0f, 90.0f);
    const float r16 = RayleighCorrector::pathReflectance(i16, 50.0f, 45.0f, 90.0f);
    check(r6 > r8 && r8 > r16, "the correction is strongest in the bluest channel");
    check(r16 < 0.002f, "IR_016 is left essentially untouched");
    check(r6 > 0.01f && r6 < 0.10f, "VIS006 path reflectance is a few percent");
}

// The terminator must not acquire a colour. Recovering the surface divides by a
// two-way transmittance, and that divisor has to describe the same sun the path
// term was integrated along - if it does not, the two disagree by more in the
// blue than in the red, which is a cast rather than a brightness error.
//
// Tested as a round trip: synthesise the measurement from a known surface with
// the forward model, push it through the correction, and ask for the surface
// back. Any per-channel drift is exactly the cast.
static void testTerminatorNeutrality()
{
    const double d2r = M_PI / 180.0;

    auto roundTrip = [&](const RayleighRT::Solution &s, double rs, double sza,
                         double vza, double raa, bool ocean) {
        const double mu0 = std::cos(sza * d2r);
        const double muv = std::max(std::cos(vza * d2r),
                                    std::cos((double)RayleighCorrector::VzaLimit * d2r));
        const double Ts  = RayleighRT::transmittanceSpherical(s, sza);
        const double Tv  = RayleighRT::transmittance(s, muv);
        const double toa = RayleighRT::toaPathReflectance(s, sza, muv, raa, ocean)
                         + mu0 * Ts * Tv * rs / (1.0 - s.sphericalAlbedo * rs);
        const float  f   = RayleighCorrector::sunZenithFactor(sza);
        const float  rho = RayleighCorrector::pathReflectance(s, sza, vza, raa,
                                                              ocean ? 1.0f : 0.0f);
        const float  rec = RayleighCorrector::surfaceReflectance(s, sza, vza,
                                                                 (float)(toa * f) - rho);
        // Divide out the grey twilight dimming, mu0_true/mu0_frozen, which is
        // intended and identical in every channel.
        return rec / (mu0 * f) / rs;
    };

    // The two extremes of optical depth in play: FCI's blue at 0.234 and
    // SEVIRI's near infrared at 0.001. If the chain is neutral for these it is
    // neutral for everything between.
    const RayleighRT::Solution &deep    = RayleighRT::forBand(0);
    const RayleighRT::Solution &shallow =
        RayleighRT::forTau(RayleighCorrector::opticalDepthSEVIRI(3));

    // Exact only where the correction is applied in full. Past SzaTrustFull it
    // deliberately retreats toward the identity, which is tested separately.
    int worst = 0;
    double worstTilt = 1.0;
    for (double sza = 20.0; sza <= RayleighCorrector::SzaTrustFull; sza += 1.0) {
        const double b = roundTrip(deep,    0.03, sza, 65.0, 90.0, true);
        const double r = roundTrip(shallow, 0.02, sza, 65.0, 90.0, true);
        if (std::fabs(b / r - 1.0) > std::fabs(worstTilt - 1.0)) {
            worstTilt = b / r;
            worst = (int)sza;
        }
    }
    std::printf("info : worst blue/red tilt is %.4f, at sza %d\n", worstTilt, worst);
    check(std::fabs(worstTilt - 1.0) < 0.01,
          "no blue/red tilt anywhere the correction is applied in full");

    // Same over land, and at a view angle near the limb where the air mass is
    // longest and a mismatch would show first.
    //
    // 2 % rather than 1 % because a residual of that size is real and belongs
    // here: the ground-to-sky bounce couples the spherical albedo to the
    // twilight dimming, as 1/(1 - s*rho_s*(1-dimming)), and s depends on optical
    // depth. It is second order in s*rho_s - about 1 % over bright land at sza
    // 88 - against the 17 % the mismatched transmittance was costing.
    for (double sza = 60.0; sza <= RayleighCorrector::SzaTrustFull; sza += 4.0) {
        const double b = roundTrip(deep,    0.10, sza, 80.0, 30.0, false);
        const double r = roundTrip(shallow, 0.30, sza, 80.0, 30.0, false);
        checkClose(b / r, 1.0, 0.02, "no tilt over land near the limb");
    }

    // Past SzaTrustNone both halves of the correction have retired, so what
    // comes back is what went in - sun-normalised and dimmed, but not recoloured.
    // This is the property that keeps the terminator grey rather than red.
    for (double sza = RayleighCorrector::SzaTrustNone; sza <= 94.0; sza += 2.0) {
        const float t = RayleighCorrector::pathTrust(sza);
        check(RayleighCorrector::pathReflectance(deep, sza, 70.0f, 90.0f, 0.0f, t) == 0.0f,
              "nothing is subtracted past the trust limit");
        checkClose(RayleighCorrector::surfaceReflectance(deep, sza, 70.0f, 0.20f, t), 0.20,
                   1e-6, "nothing is amplified past the trust limit");
        checkClose(RayleighCorrector::surfaceReflectance(shallow, sza, 70.0f, 0.20f, t), 0.20,
                   1e-6, "and the shallow band is left alone identically");
    }

    // The retreat has to be gradual in both halves, or it draws its own edge.
    double prevGain = 0.0;
    for (double sza = 70.0; sza <= 92.0; sza += 0.5) {
        const float x = 0.05f;
        const double g = RayleighCorrector::surfaceReflectance(
            deep, sza, 70.0f, x, RayleighCorrector::pathTrust(sza)) / x;
        if (sza > 70.0)
            check(std::fabs(g - prevGain) < 0.20, "the recovery retires smoothly");   // slope peaks at 0.125
        prevGain = g;
    }

    // The recovery is a division by a transmittance that goes to zero past the
    // terminator, so it has to stay bounded, and it has to stay continuous -
    // a step in the gain draws a ring along the shadow line.
    double prev = 0.0;
    for (double sza = 0.0; sza <= 110.0; sza += 0.5) {
        const float x = 0.02f;
        const double gain = RayleighCorrector::surfaceReflectance(deep, sza, 65.0f, x) / x;
        check(gain <= RayleighCorrector::MaxSurfaceGain + 1e-6, "recovery gain stays capped");
        // The gain climbs by at most 0.07 per half degree through the steepest
        // part of twilight, so a step twice that is a discontinuity, not a slope.
        if (sza > 0.0)
            check(gain - prev < 0.14, "recovery gain has no step across the terminator");
        prev = gain;
    }

    // Daylight must be untouched by any of this: the spherical and the flat
    // solar path agree to a fraction of a percent while the sun is up.
    for (double sza = 0.0; sza <= 75.0; sza += 5.0) {
        checkClose(RayleighRT::transmittanceSpherical(deep, sza),
                   RayleighRT::transmittance(deep, std::cos(sza * d2r)),
                   1e-2, "spherical and flat transmittance agree in daylight");
    }

    // Past local sunset the ground is in the Earth's shadow and the direct-beam
    // transmittance is frozen rather than allowed to collapse.
    checkClose(RayleighRT::transmittanceSpherical(deep, 95.0),
               RayleighRT::transmittanceSpherical(deep, 90.0),
               1e-9, "solar transmittance freezes at local sunset");
    check(RayleighRT::transmittanceSpherical(deep, 90.0) > 0.0,
          "solar transmittance is still positive at local sunset");
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
    testSeviri();
    testTerminatorNeutrality();

    if (g_failures) {
        std::printf("\n%d check(s) FAILED\n", g_failures);
        return 1;
    }
    std::printf("\nall checks passed\n");
    return 0;
}
