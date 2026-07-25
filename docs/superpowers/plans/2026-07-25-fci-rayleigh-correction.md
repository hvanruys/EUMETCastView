# FCI Rayleigh Correction Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove Rayleigh path reflectance from the solar bands of every MTG FCI RGB recipe, so FCI True Color renders without the blue cast and limb haze.

**Architecture:** Rewrite the currently-dead `core/rayleigh.cpp` into three pure, unit-testable functions. Split the time-only part of `snu_solar_params2` out of `core/nav_util.cpp` so it can be hoisted from a per-pixel loop. Add one method to `SegmentListGeostationary` that runs a single parallel pass over the disc between the file-read loop and the band-combination loop in `ComposeGeoRGBRecipeMTGInThread`, computing solar/viewing geometry per pixel and correcting every solar band in place. Gate it on a new INI option with a checkbox.

**Tech Stack:** C++20, Qt6 (Core, Concurrent, Widgets), CMake, netCDF. No new dependencies.

**Spec:** `docs/superpowers/specs/2026-07-25-fci-rayleigh-correction-design.md`

---

## Background you need

You are working in EUMETCastView, a Qt6 desktop viewer for EUMETSAT satellite imagery. The relevant path composes RGB images for Meteosat-12 (MTG-I1), whose FCI instrument has 16 bands: indices 0–7 are solar (VIS/NIR, reflectance) and 8–15 are thermal infrared (brightness temperature).

`SegmentListGeostationary::ComposeGeoRGBRecipeMTGInThread()` in `core/segmentlistgeostationary.cpp:9650` reads FCI netCDF segments into one `float*` buffer per unique band (`bandBuf`), then combines those buffers into R/G/B according to a recipe, then stretches to 8-bit. You are inserting a correction step between the read and the combine.

Three facts that will otherwise confuse you:

1. **`core/rayleigh.cpp` is dead code.** It compiles but nothing calls it. Its optical depths are wrong and it contains an unrelated SGP4/TLE parser. You are replacing the whole file; do not try to preserve any of it.
2. **The compose buffer is south-up.** `bandBuf` index `line` maps to display row `outRes - 1 - line`. Geolocation must use the display row.
3. **Two different lat/lon functions exist and disagree on sign.** `snu_line_column_to_lat_lon` (in `nav_util.cpp`) expects the negative CFAC/LFAC in `internal.cpp:82-83`; `pixgeoConversion::pixcoord2geocoord` expects the positive values in `GeoSatellites.ini`. Use `pixcoord2geocoord` — it is what `FormImage::DrawLongLat` already uses correctly for MET_12.

`D2R` and `R2D` are already available in `segmentlistgeostationary.cpp` (both `pixgeoconversion.h` and `internal.h` define them; internal.h wins, and they agree to 11 digits). `FILL_VALUE_F` is `-999.` from `internal.h:72`. `SAT_HEIGHT` is `42164.0` from `pixgeoconversion.h:17`.

## File structure

| File | Responsibility | Action |
|---|---|---|
| `core/rayleigh.h` | Public interface of the Rayleigh/sun-zenith maths | Replace entirely |
| `core/rayleigh.cpp` | Implementation — pure functions, no Qt types | Replace entirely |
| `core/rayleigh_test.cpp` | Unit tests for the above | Create |
| `core/nav_util_test.cpp` | Regression test proving the solar refactor is behaviour-preserving | Create |
| `core/nav_util.h` | Add `snu_solar_epoch` struct + two functions | Modify |
| `core/nav_util.cpp` | Split `snu_solar_params2`; keep it working | Modify |
| `core/options.h` | `bFciRayleigh` flag | Modify |
| `core/options.cpp` | Read/write the flag | Modify |
| `core/formtoolbox.ui` | Checkbox | Modify |
| `core/formtoolbox.cpp` | Wire checkbox to `opts` | Modify |
| `core/segmentlistgeostationary.h` | Declare `applyFCISolarCorrection` | Modify |
| `core/segmentlistgeostationary.cpp` | Implement it, call it from the MTG recipe path | Modify |
| `core/CMakeLists.txt` | `BUILD_TESTS` option + two test targets | Modify |

Tests are plain `main()` programs returning non-zero on failure — the repo has no test framework, and `bin/filter_test.cpp` sets the precedent for standalone test binaries. They are behind `-DBUILD_TESTS=ON`, off by default, so normal and AppImage builds are unaffected.

**One include-order trap.** `core/nav_util.h` uses `struct nav_scaling_factors` without declaring it, so it only ever compiles when reached *through* `core/internal.h`, which defines that struct at line 79 and then includes `nav_util.h` at line 106. Always include `internal.h` first; a bare `#include "nav_util.h"` after it is a harmless no-op thanks to the include guard. `core/segmentlistgeostationary.cpp` already includes `internal.h` at line 23.

**Verified during planning.** The `rayleigh.{h,cpp}` code and both test files in Tasks 1–5 were compiled and run before this plan was written: all checks pass, the `nav_util` golden values reproduce to 1e-11, and the epoch split matches the monolithic path to 1e-12 across 280 lat/lon points. `QtConcurrent::blockingMap` with a by-value lambda, `QAtomicInt`, and `QDateTime::fromString(..., "yyyyMMddhhmm")` were each confirmed against the project's Qt 6.9.2 at `/home/hugo/Qt/6.9.2/gcc_64`. The Julian date formula was checked against the fact that noon UTC must give a whole-number JD — it does.

---

### Task 1: Rayleigh optical depth

**Files:**
- Replace: `core/rayleigh.h`
- Replace: `core/rayleigh.cpp`
- Create: `core/rayleigh_test.cpp`
- Modify: `core/CMakeLists.txt` (append at end of file)

- [ ] **Step 1: Replace `core/rayleigh.h` entirely with this**

```cpp
#ifndef RAYLEIGH_H
#define RAYLEIGH_H

/**
 * Rayleigh scattering correction for MTG FCI solar bands.
 *
 * Pure, stateless functions with no Qt, netCDF or image dependencies, so they
 * can be unit-tested without satellite data.
 *
 * See docs/superpowers/specs/2026-07-25-fci-rayleigh-correction-design.md
 */
class RayleighCorrector
{
public:
    /** FCI solar (VIS/NIR) bands occupy indices 0..7; 8..15 are thermal IR. */
    static constexpr int SolarBandCount = 8;

    /**
     * Rayleigh optical depth at sea level for an FCI band index (0..15).
     * Returns 0.0 for IR bands and for out-of-range indices.
     */
    static double opticalDepthFCI(int bandIndex);

    /**
     * Rayleigh optical depth at sea level for an arbitrary wavelength.
     * @param lambdaMicron wavelength in micrometres; <= 0 returns 0.0
     */
    static double opticalDepthAt(double lambdaMicron);
};

#endif // RAYLEIGH_H
```

- [ ] **Step 2: Replace `core/rayleigh.cpp` entirely with this**

The old file's SGP4/TLE half and its `QVector`/`QFuture`/`QImage` batch APIs are deleted outright. Nothing references them, and `QSgp4/` already provides orbit propagation.

```cpp
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
```

- [ ] **Step 3: Create `core/rayleigh_test.cpp`**

```cpp
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

int main()
{
    testOpticalDepth();

    if (g_failures) {
        std::printf("\n%d check(s) FAILED\n", g_failures);
        return 1;
    }
    std::printf("\nall checks passed\n");
    return 0;
}
```

- [ ] **Step 4: Append the test target to the end of `core/CMakeLists.txt`**

```cmake

# ---------------------------------------------------------------------------
# Unit tests. Off by default; enable with: cmake -DBUILD_TESTS=ON ..
# These are standalone main() programs, not linked against Qt.
# ---------------------------------------------------------------------------
option(BUILD_TESTS "Build core unit tests" OFF)

if(BUILD_TESTS)
    add_executable(rayleigh_test rayleigh_test.cpp rayleigh.cpp)
    target_include_directories(rayleigh_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    set_target_properties(rayleigh_test PROPERTIES
        AUTOMOC OFF
        AUTOUIC OFF
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
endif()
```

- [ ] **Step 5: Configure and build the test, verify it passes**

```bash
cd /home/hugo/EUMETCastTools/EUMETCastView
cmake -S . -B build-test -DBUILD_TESTS=ON
cmake --build build-test --target rayleigh_test -j$(nproc)
./bin/rayleigh_test
```

Expected: every line starts `ok   :`, final line `all checks passed`, exit code 0.

If it fails on the tau values, the formula in `rayleigh.cpp` is wrong — do not adjust the expected values, they are independently computed from Bodhaine et al.

- [ ] **Step 6: Commit**

```bash
git add core/rayleigh.h core/rayleigh.cpp core/rayleigh_test.cpp core/CMakeLists.txt
git commit -m "Replace dead rayleigh.cpp with tested FCI optical depth

The previous file was compiled but never called, its optical depth table
was wrong by roughly a factor of two on vis_08 and nir_16, and it carried
an unrelated SGP4/TLE parser duplicating the QSgp4 submodule.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>"
```

---

### Task 2: Rayleigh phase function

**Files:**
- Modify: `core/rayleigh.h`
- Modify: `core/rayleigh.cpp`
- Modify: `core/rayleigh_test.cpp`

- [ ] **Step 1: Add the failing test to `core/rayleigh_test.cpp`**

Insert this function after `testOpticalDepth()`:

```cpp
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
```

Add the call in `main()`, before the failure check:

```cpp
    testOpticalDepth();
    testPhaseFunction();
```

- [ ] **Step 2: Build to verify it fails**

```bash
cmake --build build-test --target rayleigh_test -j$(nproc)
```

Expected: compile error, `'phaseFunction' is not a member of 'RayleighCorrector'`.

- [ ] **Step 3: Declare it in `core/rayleigh.h`**

Add inside the `public:` section, after `opticalDepthAt`:

```cpp
    /**
     * Rayleigh phase function, polarisation-corrected with the Young (1980)
     * depolarisation factor. Normalised so that (1/2)*integral P dmu == 1.
     * @param cosTheta cosine of the scattering angle
     */
    static double phaseFunction(double cosTheta);
```

- [ ] **Step 4: Implement it in `core/rayleigh.cpp`**

Add to the anonymous namespace, after `kFciSolarLambda`:

```cpp
// Depolarisation factor for air, Young (1980), Appl. Opt. 19, 3427.
constexpr double kDepolarization = 0.0279;
```

Add at the end of the file:

```cpp
double RayleighCorrector::phaseFunction(double cosTheta)
{
    const double gamma = kDepolarization / (2.0 - kDepolarization);
    const double norm  = 3.0 / (4.0 * (1.0 + 2.0 * gamma));
    return norm * ((1.0 + 3.0 * gamma) + (1.0 - gamma) * cosTheta * cosTheta);
}
```

- [ ] **Step 5: Build and run, verify it passes**

```bash
cmake --build build-test --target rayleigh_test -j$(nproc) && ./bin/rayleigh_test
```

Expected: `all checks passed`, exit code 0.

- [ ] **Step 6: Commit**

```bash
git add core/rayleigh.h core/rayleigh.cpp core/rayleigh_test.cpp
git commit -m "Add polarisation-corrected Rayleigh phase function

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>"
```

---

### Task 3: Sun-zenith correction factor

**Files:**
- Modify: `core/rayleigh.h`
- Modify: `core/rayleigh.cpp`
- Modify: `core/rayleigh_test.cpp`

This reproduces Satpy's `sunzen_corr_cos` from `satpy/utils.py`: divide by cos(SZA) up to 88°, then fall off logarithmically to exactly zero at 95°.

- [ ] **Step 1: Add the failing test to `core/rayleigh_test.cpp`**

Insert after `testPhaseFunction()`:

```cpp
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
```

Add `testSunZenithFactor();` to `main()` after `testPhaseFunction();`.

- [ ] **Step 2: Build to verify it fails**

```bash
cmake --build build-test --target rayleigh_test -j$(nproc)
```

Expected: compile error, `'sunZenithFactor' is not a member of 'RayleighCorrector'`.

- [ ] **Step 3: Declare it in `core/rayleigh.h`**

Add these constants inside `public:`, right after `SolarBandCount`:

```cpp
    /** Sun-zenith angle above which the 1/cos correction is capped, degrees. */
    static constexpr float SzaLimit = 88.0f;
    /** Sun-zenith angle at which the correction reaches zero, degrees. */
    static constexpr float SzaMax   = 95.0f;
```

And this declaration after `phaseFunction`:

```cpp
    /**
     * Sun-zenith normalisation factor, equivalent to Satpy sunzen_corr_cos.
     * Multiply a radiance-equivalent reflectance by this to get BRF.
     * Returns 0.0 for night, i.e. szaDeg >= maxSzaDeg.
     */
    static float sunZenithFactor(float szaDeg,
                                 float limitDeg  = SzaLimit,
                                 float maxSzaDeg = SzaMax);
```

- [ ] **Step 4: Implement it in `core/rayleigh.cpp`**

Add at the end of the file:

```cpp
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
```

- [ ] **Step 5: Build and run, verify it passes**

```bash
cmake --build build-test --target rayleigh_test -j$(nproc) && ./bin/rayleigh_test
```

Expected: `all checks passed`, exit code 0.

- [ ] **Step 6: Commit**

```bash
git add core/rayleigh.h core/rayleigh.cpp core/rayleigh_test.cpp
git commit -m "Add Satpy-equivalent sun-zenith correction factor

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>"
```

---

### Task 4: Rayleigh path reflectance

**Files:**
- Modify: `core/rayleigh.h`
- Modify: `core/rayleigh.cpp`
- Modify: `core/rayleigh_test.cpp`

- [ ] **Step 1: Add the failing test to `core/rayleigh_test.cpp`**

Insert after `testSunZenithFactor()`:

```cpp
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
```

Add `testPathReflectance();` to `main()` after `testSunZenithFactor();`.

- [ ] **Step 2: Build to verify it fails**

```bash
cmake --build build-test --target rayleigh_test -j$(nproc)
```

Expected: compile error, `'pathReflectance' is not a member of 'RayleighCorrector'`.

- [ ] **Step 3: Declare it in `core/rayleigh.h`**

Add after `sunZenithFactor`:

```cpp
    /**
     * Single-scattering Rayleigh path reflectance for a homogeneous layer.
     * Subtract this from a sun-normalised BRF to remove molecular scattering.
     *
     * Both cosines are floored at cos(SzaLimit): from geostationary orbit the
     * viewing zenith angle reaches 90 degrees at the visible disc edge, so the
     * 1/(mu0+muv) term would otherwise diverge at the limb.
     *
     * @param tau    Rayleigh optical depth; <= 0 returns 0
     * @param szaDeg solar zenith angle, degrees
     * @param vzaDeg viewing zenith angle, degrees
     * @param raaDeg relative azimuth angle, degrees, folded into [0, 180]
     */
    static float pathReflectance(double tau, float szaDeg,
                                 float vzaDeg, float raaDeg);
```

- [ ] **Step 4: Implement it in `core/rayleigh.cpp`**

Add `#include <algorithm>` below `#include <cmath>`, then add at the end of the file:

```cpp
float RayleighCorrector::pathReflectance(double tau, float szaDeg,
                                         float vzaDeg, float raaDeg)
{
    if (tau <= 0.0)
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

    const double rho = phaseFunction(cosScatter) / (4.0 * (mu0 + muv))
                     * (1.0 - std::exp(-tau * (1.0 / mu0 + 1.0 / muv)));

    return static_cast<float>(rho);
}
```

- [ ] **Step 5: Build and run, verify it passes**

```bash
cmake --build build-test --target rayleigh_test -j$(nproc) && ./bin/rayleigh_test
```

Expected: `all checks passed`, exit code 0.

- [ ] **Step 6: Commit**

```bash
git add core/rayleigh.h core/rayleigh.cpp core/rayleigh_test.cpp
git commit -m "Add single-scattering Rayleigh path reflectance

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>"
```

---

### Task 5: Hoist the solar ephemeris out of the per-pixel path

**Files:**
- Modify: `core/nav_util.h`
- Modify: `core/nav_util.cpp:402-459` (`snu_solar_params2`)
- Create: `core/nav_util_test.cpp`
- Modify: `core/CMakeLists.txt`

`snu_solar_params2()` recomputes the full solar ephemeris on every call, but solar declination and equation-of-time depend only on `jtime`, which is constant across an FCI disc. Calling it 124 million times would waste roughly 90 % of the geometry budget. This task splits it without changing any existing caller's behaviour — the golden values below were captured from the current implementation before the refactor.

- [ ] **Step 1: Create `core/nav_util_test.cpp`**

```cpp
// Regression test: the snu_solar_params2 refactor must be behaviour-preserving.
// Golden values were captured from the pre-refactor implementation.
//
// Build with: cmake -DBUILD_TESTS=ON ..    Run: bin/nav_util_test

#include "internal.h"
#include "nav_util.h"

#include <cmath>
#include <cstdio>

static int g_failures = 0;

static void checkClose(double got, double want, double absTol, const char *what)
{
    if (std::fabs(got - want) <= absTol) {
        std::printf("ok   : %s\n", what);
    } else {
        std::printf("FAIL : %s (got %.12f, want %.12f)\n", what, got, want);
        ++g_failures;
    }
}

struct GoldenCase {
    double jtime;
    double latDeg;
    double lonDeg;
    double mu0;
    double theta0;
    double phi0;
};

// Captured from snu_solar_params2 before the epoch split.
static const GoldenCase kGolden[] = {
    { 2460520.937500,   0.0000,    0.0000, 0.865147990000, 0.525350945282, 0.882808586559 },
    { 2460520.937500,  50.0000,    4.0000, 0.816074475102, 0.616210439899, 2.541995970279 },
    { 2460520.937500, -30.0000,  -45.0000, 0.133311171055, 1.437087099294, 1.105494731001 },
    { 2460521.250000,  60.0000,   30.0000, 0.049332356718, 1.521443938258, 5.294321500286 },
    { 2460704.500000, -70.0000,  170.0000, 0.605833089730, 0.919983744167, 0.278031756823 },
};

static const int kGoldenCount = sizeof(kGolden) / sizeof(kGolden[0]);

static void testSolarParams2Unchanged()
{
    for (int i = 0; i < kGoldenCount; ++i) {
        const GoldenCase &c = kGolden[i];
        double mu0, theta0, phi0;
        snu_solar_params2(c.jtime, c.latDeg * D2R, c.lonDeg * D2R,
                          &mu0, &theta0, &phi0, NULL);
        checkClose(mu0,    c.mu0,    1e-11, "snu_solar_params2 mu0 unchanged");
        checkClose(theta0, c.theta0, 1e-11, "snu_solar_params2 theta0 unchanged");
        checkClose(phi0,   c.phi0,   1e-11, "snu_solar_params2 phi0 unchanged");
    }
}

static void testEpochSplitMatches()
{
    // The split path must agree with the monolithic one to the last bit that
    // matters, for every golden case.
    for (int i = 0; i < kGoldenCount; ++i) {
        const GoldenCase &c = kGolden[i];

        double mu0a, theta0a, phi0a;
        snu_solar_params2(c.jtime, c.latDeg * D2R, c.lonDeg * D2R,
                          &mu0a, &theta0a, &phi0a, NULL);

        struct snu_solar_epoch e;
        snu_solar_epoch_init(c.jtime, &e);

        double mu0b, theta0b, phi0b;
        snu_solar_params_at(&e, c.jtime, c.latDeg * D2R, c.lonDeg * D2R,
                            &mu0b, &theta0b, &phi0b);

        checkClose(mu0b,    mu0a,    1e-12, "epoch split mu0 matches");
        checkClose(theta0b, theta0a, 1e-12, "epoch split theta0 matches");
        checkClose(phi0b,   phi0a,   1e-12, "epoch split phi0 matches");
    }
}

static void testEpochReuseAcrossPixels()
{
    // One epoch reused across many longitudes must still match per-call results.
    const double jtime = 2460520.937500;
    struct snu_solar_epoch e;
    snu_solar_epoch_init(jtime, &e);

    for (double lonDeg = -180.0; lonDeg <= 180.0; lonDeg += 15.0) {
        for (double latDeg = -75.0; latDeg <= 75.0; latDeg += 15.0) {
            double mu0a, theta0a, phi0a;
            snu_solar_params2(jtime, latDeg * D2R, lonDeg * D2R,
                              &mu0a, &theta0a, &phi0a, NULL);

            double mu0b, theta0b, phi0b;
            snu_solar_params_at(&e, jtime, latDeg * D2R, lonDeg * D2R,
                                &mu0b, &theta0b, &phi0b);

            checkClose(mu0b,    mu0a,    1e-12, "reused epoch mu0 matches");
            checkClose(theta0b, theta0a, 1e-12, "reused epoch theta0 matches");
            checkClose(phi0b,   phi0a,   1e-12, "reused epoch phi0 matches");
        }
    }
}

int main()
{
    testSolarParams2Unchanged();
    testEpochSplitMatches();
    testEpochReuseAcrossPixels();

    if (g_failures) {
        std::printf("\n%d check(s) FAILED\n", g_failures);
        return 1;
    }
    std::printf("\nall checks passed\n");
    return 0;
}
```

- [ ] **Step 2: Add the test target to `core/CMakeLists.txt`**

Inside the existing `if(BUILD_TESTS)` block added in Task 1, before the closing `endif()`:

```cmake
    add_executable(nav_util_test nav_util_test.cpp nav_util.cpp misc_util.cpp)
    target_include_directories(nav_util_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    set_target_properties(nav_util_test PROPERTIES
        AUTOMOC OFF
        AUTOUIC OFF
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
```

`nav_util.cpp` and `misc_util.cpp` link standalone with no Qt dependency — this has been verified.

- [ ] **Step 3: Build to verify it fails**

```bash
cmake -S . -B build-test -DBUILD_TESTS=ON
cmake --build build-test --target nav_util_test -j$(nproc)
```

Expected: compile error, `'snu_solar_epoch' was not declared` / `'snu_solar_epoch_init' was not declared`.

- [ ] **Step 4: Declare the new interface in `core/nav_util.h`**

Replace the whole file with:

```c
/******************************************************************************%
 *
 *    Copyright (C) 2014-2017 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
 *
 *    This source code is licensed under the GNU General Public License (GPL),
 *    Version 3.  See the file COPYING for more details.
 *
 ******************************************************************************/

#ifndef NAV_UTIL_H
#define NAV_UTIL_H

/* Solar quantities that depend only on time, not on position. Compute once per
   image with snu_solar_epoch_init(), then reuse across every pixel. */
struct snu_solar_epoch {
     double delta;              /* solar declination */
     double gw_mean_sol_time;
     double gw_appar_sol_time;
};

int snu_line_column_to_lat_lon(unsigned int l, unsigned int c, float *lat, float *lon,
                               double lon0, const struct nav_scaling_factors *nav);
int snu_lat_lon_to_line_column(float lat, float lon, unsigned int *line, unsigned int *column,
                               double lon0, const struct nav_scaling_factors *nav);
void snu_solar_epoch_init(double jtime, struct snu_solar_epoch *e);
void snu_solar_params_at(const struct snu_solar_epoch *e, double jtime,
                         double lat, double lon, double *mu0,
                         double *theta0, double *phi0);
void snu_solar_params2(double jtime, double lat, double lon, double *mu0,
                       double *theta0, double *phi0, double *solar_dist_fac);
int snu_vza_and_vaa(double lat, double lon, double height,
                    double X, double Y, double Z, float *vza, float *vaa);

#endif /* NAV_UTIL_H */
```

- [ ] **Step 5: Replace `snu_solar_params2` in `core/nav_util.cpp`**

Replace the entire function body at `core/nav_util.cpp:402-459` (from `void snu_solar_params2(double jtime, ...` down to and including its closing `}`) with:

```c
void snu_solar_epoch_init(double jtime, struct snu_solar_epoch *e)
{
     solar_coords_and_times(jtime, &e->delta,
                            &e->gw_mean_sol_time, &e->gw_appar_sol_time);
}



/*******************************************************************************
 * Compute the cosine of the solar zenith angle, the solar zenith angle, and the
 * solar azimuth angle for one position, reusing a precomputed epoch.
 *
 * e		: Input epoch from snu_solar_epoch_init()
 * jtime	: Input Julian Day Number (must match the one used for the epoch)
 * lat		: Input latitude (radians: -PI/2 -- PI/2)
 * lon		: Input longitude (radians: -PI -- PI)
 * mu0		: Output cosine of the solar zenith angle (-1.0 -- 1.0)
 * theta0	: Output solar zenith angle (radians: 0.0 -- PI)
 * phi0		: Output solar azimuth angle (radians: 0.0 -- 2PI)
 ******************************************************************************/
void snu_solar_params_at(const struct snu_solar_epoch *e, double jtime,
                         double lat, double lon, double *mu0,
                         double *theta0, double *phi0)
{
     double jfrac;

     double loc_mean_sol_time;
     double loc_appar_sol_time;

     double local_hour;

     double eot;

     loc_mean_sol_time  = greenwich_to_local_time(lon, e->gw_mean_sol_time);
     loc_appar_sol_time = greenwich_to_local_time(lon, e->gw_appar_sol_time);

     jfrac = jtime - (long) jtime;
     if (jfrac < .5)
          jfrac = jfrac + .5;
     else
          jfrac = jfrac - .5;

     local_hour = jfrac * 24. + lon / (15.*D2R);

     eot = fmod(loc_mean_sol_time - loc_appar_sol_time,  1.);

     solar_angles(e->delta, lat, local_hour, eot, mu0, theta0, phi0);
}



void snu_solar_params2(double jtime, double lat, double lon, double *mu0,
                       double *theta0, double *phi0, double *solar_dist_fac)
{
     struct snu_solar_epoch e;

     snu_solar_epoch_init(jtime, &e);

     snu_solar_params_at(&e, jtime, lat, lon, mu0, theta0, phi0);

     if (solar_dist_fac) {
          int year;
          int month;
          int day;

          long jwhole;

          double jfrac;

          double local_hour;

          double jday;

          jwhole = (int) jtime;
          jfrac  = jtime - jwhole;
          if (jfrac < .5)
               jfrac  = jfrac  + .5;
          else {
               jfrac  = jfrac  - .5;
               jwhole = jwhole + 1;
          }

          local_hour = jfrac * 24. + lon / (15.*D2R);

          snu_jul_to_cal_date(jwhole, &year, &month, &day);

          jday = (int) (jtime - (snu_cal_to_jul_day(year, 1, 1) - .5));

          *solar_dist_fac = snu_solar_distance_factor2(jday + local_hour / 24.);
     }
}
```

The `jwhole` increment that the original did in the `else` branch only ever fed `solar_dist_fac`, so it stays in `snu_solar_params2` and is correctly absent from `snu_solar_params_at`.

- [ ] **Step 6: Build and run, verify it passes**

```bash
cmake --build build-test --target nav_util_test -j$(nproc) && ./bin/nav_util_test
```

Expected: `all checks passed`, exit code 0. If the golden values fail, the refactor changed behaviour — fix the refactor, not the golden values.

- [ ] **Step 7: Verify the main application still builds**

The MSG recipe path calls `snu_solar_params2`; make sure nothing broke.

```bash
cmake --build build-test --target EUMETCastView -j$(nproc) 2>&1 | tail -20
```

Expected: builds without errors.

- [ ] **Step 8: Commit**

```bash
git add core/nav_util.h core/nav_util.cpp core/nav_util_test.cpp core/CMakeLists.txt
git commit -m "Split time-only solar ephemeris out of snu_solar_params2

Declination and equation-of-time depend only on jtime, which is constant
across a geostationary disc. Splitting them out lets the FCI correction
hoist them from a 124-million-pixel loop. snu_solar_params2 keeps its
exact previous behaviour, pinned by golden-value regression tests.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>"
```

---

### Task 6: Add the `bFciRayleigh` option

**Files:**
- Modify: `core/options.h:277`
- Modify: `core/options.cpp:236`, `core/options.cpp:614`

- [ ] **Step 1: Add the member to `core/options.h`**

Find this block at `core/options.h:274-277`:

```cpp
    bool remove_OLCI_dirs;
    bool remove_SLSTR_dirs;
    bool usesaturationmask;
    bool copyMTGfiles;
```

Change it to:

```cpp
    bool remove_OLCI_dirs;
    bool remove_SLSTR_dirs;
    bool usesaturationmask;
    bool copyMTGfiles;
    bool bFciRayleigh;
```

- [ ] **Step 2: Read it in `core/options.cpp`**

Find this line at `core/options.cpp:236`:

```cpp
    copyMTGfiles = settings.value("/parameters/copymtgfiles", false).toBool();
```

Add immediately after it:

```cpp
    bFciRayleigh = settings.value("/parameters/fcirayleigh", true).toBool();
```

- [ ] **Step 3: Write it back in `core/options.cpp`**

Find this line at `core/options.cpp:614`:

```cpp
    settings.setValue("/parameters/copymtgfiles", copyMTGfiles);
```

Add immediately after it:

```cpp
    settings.setValue("/parameters/fcirayleigh", bFciRayleigh);
```

- [ ] **Step 4: Build to verify it compiles**

```bash
cmake --build build-test --target EUMETCastView -j$(nproc) 2>&1 | tail -20
```

Expected: builds without errors.

- [ ] **Step 5: Commit**

```bash
git add core/options.h core/options.cpp
git commit -m "Add bFciRayleigh option, default on

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>"
```

---

### Task 7: Add the FCI Rayleigh checkbox

**Files:**
- Modify: `core/formtoolbox.ui` (the `horizontalLayout_fcirecipe` block, around line 6745)
- Modify: `core/formtoolbox.cpp:474`

- [ ] **Step 1: Add the checkbox to `core/formtoolbox.ui`**

Find this block (it is the layout holding the "MTG/FCI recipes" label and the "Make FCI recipe" button):

```xml
          <item>
           <layout class="QHBoxLayout" name="horizontalLayout_fcirecipe">
            <item>
             <widget class="QLabel" name="label_fcirecipe">
              <property name="font">
               <font>
                <pointsize>10</pointsize>
               </font>
              </property>
              <property name="text">
               <string>MTG/FCI recipes</string>
              </property>
             </widget>
            </item>
            <item>
```

Insert a new `<item>` between the label's closing `</item>` and the button's opening `<item>`, so the block becomes:

```xml
          <item>
           <layout class="QHBoxLayout" name="horizontalLayout_fcirecipe">
            <item>
             <widget class="QLabel" name="label_fcirecipe">
              <property name="font">
               <font>
                <pointsize>10</pointsize>
               </font>
              </property>
              <property name="text">
               <string>MTG/FCI recipes</string>
              </property>
             </widget>
            </item>
            <item>
             <widget class="QCheckBox" name="chkFciRayleigh">
              <property name="font">
               <font>
                <pointsize>10</pointsize>
               </font>
              </property>
              <property name="toolTip">
               <string>Remove Rayleigh (molecular) scattering from the solar bands. Also sun-normalises the reflectance, which brightens the image away from the subsolar point.</string>
              </property>
              <property name="text">
               <string>Rayleigh correction</string>
              </property>
              <property name="checked">
               <bool>true</bool>
              </property>
             </widget>
            </item>
            <item>
```

Leave the rest of the block (the `btnFCIRecipes` widget and the closing tags) untouched.

- [ ] **Step 2: Initialise and wire it in `core/formtoolbox.cpp`**

Find this block at `core/formtoolbox.cpp:472-475`:

```cpp
    for(int i = 0; i < imageptrs->fci_rgbrecipes.count(); i++)
    {
        new QListWidgetItem(imageptrs->fci_rgbrecipes.at(i).Name, ui->lstFCIRGB);
    }
```

Add immediately after the closing brace:

```cpp
    ui->chkFciRayleigh->setChecked(opts.bFciRayleigh);
    connect(ui->chkFciRayleigh, &QCheckBox::toggled, this, [](bool checked) {
        opts.bFciRayleigh = checked;
    });
```

- [ ] **Step 3: Build and verify the checkbox appears**

```bash
cmake --build build-test --target EUMETCastView -j$(nproc) 2>&1 | tail -20
```

Expected: builds without errors. `CMAKE_AUTOUIC` regenerates `ui_formtoolbox.h`, so `ui->chkFciRayleigh` resolves.

- [ ] **Step 4: Commit**

```bash
git add core/formtoolbox.ui core/formtoolbox.cpp
git commit -m "Add Rayleigh correction checkbox to the FCI recipe tab

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>"
```

---

### Task 8: Apply the correction in the FCI compose path

**Files:**
- Modify: `core/segmentlistgeostationary.h` (declare in the `private:` section, which starts at line 135)
- Modify: `core/segmentlistgeostationary.cpp` (includes near line 22; new method; call site at line ~9845)

This is the task that actually changes the images. Read the whole task before starting.

- [ ] **Step 1: Add the include to `core/segmentlistgeostationary.cpp`**

Find this line at `core/segmentlistgeostationary.cpp:22`:

```cpp
#include "misc_util.h"
```

Add immediately after it:

```cpp
#include "rayleigh.h"
#include <QTimeZone>
```

- [ ] **Step 2: Declare the method in `core/segmentlistgeostationary.h`**

Find the `private:` line at `core/segmentlistgeostationary.h:135` and add immediately after it:

```cpp
    void applyFCISolarCorrection(QVector<float*> &bandBuf,
                                 const QList<int> &bandIndices,
                                 int outRes);
```

- [ ] **Step 3: Implement the method in `core/segmentlistgeostationary.cpp`**

Add this complete function immediately **before** `void SegmentListGeostationary::ComposeGeoRGBRecipeMTGInThread(int recipe)` at line 9650:

```cpp
/*
 * Sun-normalise and Rayleigh-correct every solar band of an FCI recipe.
 *
 * Runs once over the full disc, in place, between reading the netCDF segments
 * and combining bands into R/G/B. Per-pixel geometry is computed and discarded
 * rather than stored: six full-disc float arrays would cost about 3 GB at
 * 11136 x 11136, on top of the ~3.5 GB the compose path already peaks at.
 *
 * See docs/superpowers/specs/2026-07-25-fci-rayleigh-correction-design.md
 */
void SegmentListGeostationary::applyFCISolarCorrection(QVector<float*> &bandBuf,
                                                       const QList<int> &bandIndices,
                                                       int outRes)
{
    // Which slots in bandBuf hold solar bands? IR bands get neither
    // sun-normalisation nor a Rayleigh correction.
    QList<int>    solarSlots;
    QList<double> solarTau;
    for (int bi = 0; bi < bandIndices.size(); ++bi) {
        const int bandIndex = bandIndices.at(bi);
        if (bandIndex >= 0 && bandIndex < RayleighCorrector::SolarBandCount) {
            solarSlots.append(bi);
            solarTau.append(RayleighCorrector::opticalDepthFCI(bandIndex));
        }
    }

    if (solarSlots.isEmpty())
        return;   // IR-only recipe

    // Navigation parameters. Same convention FormImage::DrawLongLat uses for the
    // MET_12 coastline overlay: positive INI factors with the display row.
    const bool   hires = (outRes == 11136);
    const long   coff  = hires ? opts.geosatellites.at(geoindex).coffhrv
                               : opts.geosatellites.at(geoindex).coff;
    const long   loff  = hires ? opts.geosatellites.at(geoindex).loffhrv
                               : opts.geosatellites.at(geoindex).loff;
    const double cfac  = hires ? opts.geosatellites.at(geoindex).cfachrv
                               : opts.geosatellites.at(geoindex).cfac;
    const double lfac  = hires ? opts.geosatellites.at(geoindex).lfachrv
                               : opts.geosatellites.at(geoindex).lfac;

    if (cfac == 0.0 || lfac == 0.0) {
        qWarning() << "FCI Rayleigh: cfac/lfac missing in GeoSatellites.ini for geoindex"
                   << geoindex << "- skipping correction";
        return;
    }

    QDateTime dt = QDateTime::fromString(filedatestring, "yyyyMMddhhmm");
    if (!dt.isValid()) {
        qWarning() << "FCI Rayleigh: cannot parse filedatestring" << filedatestring
                   << "- skipping correction";
        return;
    }
    // fromString yields a local-time QDateTime; setTimeZone reinterprets the
    // same wall-clock fields as UTC, which is what the slot name means.
    // Do NOT use setTimeSpec(Qt::UTC) - deprecated in Qt 6.9.
    dt.setTimeZone(QTimeZone::UTC);

    // Nominal slot time, used as mid-scan for the whole disc.
    const double jtime = dt.toMSecsSinceEpoch() / 86400000.0 + 2440587.5;

    // Nominal geostationary satellite position in ECEF (km). FCI segments carry
    // no orbit polynomials; station-keeping holds MTG-I1 within about 0.1 deg.
    const double subLon = opts.geosatellites.at(geoindex).longitude;
    const double satX   = SAT_HEIGHT * cos(subLon * D2R);
    const double satY   = SAT_HEIGHT * sin(subLon * D2R);
    const double satZ   = 0.0;

    struct snu_solar_epoch epoch;
    snu_solar_epoch_init(jtime, &epoch);

    qDebug() << "FCI Rayleigh: res" << outRes << "jtime" << jtime
             << "subLon" << subLon << "solar bands" << solarSlots.size();

    QVector<int> lines(outRes);
    for (int i = 0; i < outRes; ++i)
        lines[i] = i;

    QAtomicInt rowsDone(0);

    QtConcurrent::blockingMap(lines, [&](int line) {
        pixgeoConversion pixconv;

        // The compose buffer is south-up; geolocation wants the display row.
        const int display_row = outRes - 1 - line;

        for (int pixelx = 0; pixelx < outRes; ++pixelx) {
            double lat_deg = 0.0;
            double lon_deg = 0.0;

            if (pixconv.pixcoord2geocoord(subLon, pixelx, display_row,
                                          (int)coff, (int)loff, cfac, lfac,
                                          &lat_deg, &lon_deg) != 0)
                continue;   // off-disc, stays FILL_VALUE_F

            const long i_pix = (long)line * outRes + pixelx;

            double mu0 = 0.0, theta0 = 0.0, phi0 = 0.0;
            snu_solar_params_at(&epoch, jtime, lat_deg * D2R, lon_deg * D2R,
                                &mu0, &theta0, &phi0);

            const float szaDeg = (float)(theta0 * R2D);
            const float saaDeg = (float)(phi0 * R2D);

            float vzaDeg = 0.0f;
            float vaaDeg = 0.0f;
            snu_vza_and_vaa(lat_deg, lon_deg, 0.0, satX, satY, satZ,
                            &vzaDeg, &vaaDeg);

            // Relative azimuth folded into [0, 180]; the phase function is even
            // in it, so the MSG path's matching 180 deg offsets would cancel.
            float raaDeg = fmodf(saaDeg - vaaDeg, 360.0f);
            if (raaDeg < 0.0f)
                raaDeg += 360.0f;
            if (raaDeg > 180.0f)
                raaDeg = 360.0f - raaDeg;

            const float f = RayleighCorrector::sunZenithFactor(szaDeg);

            for (int k = 0; k < solarSlots.size(); ++k) {
                float *buf = bandBuf[solarSlots.at(k)];

                if (buf[i_pix] == FILL_VALUE_F)
                    continue;

                if (f == 0.0f) {
                    buf[i_pix] = 0.0f;   // night
                    continue;
                }

                const float brf = buf[i_pix] * f;
                const float rho = RayleighCorrector::pathReflectance(
                    solarTau.at(k), szaDeg, vzaDeg, raaDeg);

                buf[i_pix] = qMax(0.0f, brf - rho);
            }
        }

        const int done = rowsDone.fetchAndAddOrdered(1) + 1;
        if ((done & 0x1FF) == 0)
            emit progressCounter(70 + (8 * done) / outRes);
    });
}
```

- [ ] **Step 4: Call it from `ComposeGeoRGBRecipeMTGInThread`**

Find this block at `core/segmentlistgeostationary.cpp:9843-9849`:

```cpp
        nc_close(ncid);
        if (opts.copyMTGfiles) QFile::remove(filePath);
    }

    emit progressCounter(70);

    // Combine bands into R/G/B result channels per recipe formula
```

Change it to:

```cpp
        nc_close(ncid);
        if (opts.copyMTGfiles) QFile::remove(filePath);
    }

    emit progressCounter(70);

    // Sun-normalise and remove Rayleigh path reflectance from the solar bands,
    // per band, before they are combined into R/G/B.
    if (opts.bFciRayleigh)
        applyFCISolarCorrection(bandBuf, uniqueIdx, outRes);

    emit progressCounter(78);

    // Combine bands into R/G/B result channels per recipe formula
```

- [ ] **Step 5: Build**

```bash
cmake --build build-test --target EUMETCastView -j$(nproc) 2>&1 | tail -30
```

Expected: builds without errors.

If the compiler complains that `QAtomicInt` is undeclared, add `#include <QAtomicInt>` next to the other Qt includes at the top of `core/segmentlistgeostationary.cpp`. `QtConcurrent` and `QApplication` are already included at lines 14-16.

- [ ] **Step 6: Commit**

```bash
git add core/segmentlistgeostationary.h core/segmentlistgeostationary.cpp
git commit -m "Rayleigh-correct FCI solar bands before recipe combination

Sun-normalises with Satpy's sunzen_corr_cos falloff, then subtracts the
single-scattering Rayleigh path reflectance per band. Geometry is computed
per pixel and discarded; storing six full-disc angle arrays would cost
about 3 GB at 11136x11136.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>"
```

---

### Task 9: End-to-end verification on real data

**Files:** none modified.

There is no automated test for this — it needs real FCI segments and human eyes.

- [ ] **Step 1: Confirm both test binaries still pass**

```bash
cd /home/hugo/EUMETCastTools/EUMETCastView
cmake --build build-test --target rayleigh_test nav_util_test -j$(nproc)
./bin/rayleigh_test && ./bin/nav_util_test
```

Expected: both print `all checks passed` and exit 0.

- [ ] **Step 2: Build the release binary the normal way**

```bash
cmake -S . -B build
cmake --build build -j$(nproc) 2>&1 | tail -20
```

Expected: builds without errors, and **without** the test targets, since `BUILD_TESTS` defaults to OFF.

- [ ] **Step 3: Run the app and compose FCI True Color with the correction OFF**

Launch `bin/EUMETCastView`. In the toolbox, open the MTG/FCI tab, pick a date/time slot with good daytime coverage, **uncheck** "Rayleigh correction", select "FCI True Color RGB" in the recipe list, and click "Make FCI recipe". Save the image.

This is the baseline. It should look exactly like the current release output.

- [ ] **Step 4: Compose the same slot with the correction ON**

Re-check "Rayleigh correction", click "Make FCI recipe" again, save.

Expected differences, in order of how obvious they should be:

- The blue cast over ocean lifts; deep ocean goes from hazy blue-grey toward darker blue.
- Haze reduction is strongest near the limb and weakest near the sub-satellite point, because the air mass factor `1/μ₀ + 1/μ_v` grows toward the edge.
- The whole image is brighter away from the subsolar point — this is the sun-normalisation, and it is expected, not a bug.
- The terminator fades smoothly to black rather than cutting off.

If instead you see a **black ring at the disc edge**, the limb clamp in `pathReflectance` is not working — check the `muFloor` logic.

If you see the image go **uniformly darker with no colour change**, the correction is being applied to non-normalised values — check that `sunZenithFactor` is being multiplied in before `pathReflectance` is subtracted.

If the **night side turns grey instead of black**, `f == 0.0f` is not being detected — check `SzaMax`.

- [ ] **Step 5: Sanity-check a band-ratio expectation**

Compose "FCI Natural Colors RGB" (bands nir_16 / vis_08 / vis_06) with the correction on. Because τ for nir_16 is 0.00128 — about 180× smaller than vis_04's — its red channel should be essentially unchanged from the uncorrected version apart from the sun-normalisation brightening. Only the blue channel (vis_06) should visibly de-haze.

If all three channels change equally, the per-band τ lookup is wrong — check that `uniqueIdx` really holds FCI band indices 0–15 and not positions in `uniqueBands`.

- [ ] **Step 6: Check the timing is acceptable**

The correction pass should add roughly 3–5 seconds on a 12-thread machine at 11136×11136, visible as the progress bar moving from 70 to 78. If it takes substantially longer, confirm `QtConcurrent::blockingMap` is actually parallelising — a serial run would take about a minute.

- [ ] **Step 7: Commit nothing, report findings**

Report to the user: which slot was used, whether the expected visual differences appeared, the timing, and whether any of the failure signatures above showed up.

---

## Known follow-ups (deliberately not in this plan)

These are recorded in the spec's Risks section and are **not** to be implemented here:

1. **Recipe range re-tuning.** All seven solar recipes (4, 5, 6, 7, 10, 11, 12) get brighter away from the subsolar point. `rangefrom`/`rangeto`/`gamma` in `core/segmentimage.cpp:826-1010` were tuned against non-normalised values. The checkbox exists to let the user A/B this before deciding.
2. **Recipe 4, Day Severe Storms**, will be the most disrupted — its blue channel is the difference `nir_16 − vis_06` over a narrow −0.7 to 0.2 range, and sun-normalising both terms scales that difference by 1/cos(SZA).
3. **Multiple scattering.** Single-scattering underestimates by roughly 5–10 % of the Rayleigh signal at 444 nm. Adding a second-order term needs no interface change.
