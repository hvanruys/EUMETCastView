# FCI Rayleigh Correction — Design

**Date:** 2026-07-25
**Status:** Approved, ready for implementation planning
**Scope:** Meteosat-12 (MTG-I1) FCI RGB recipe composition in `core/`

## Problem

`SegmentListGeostationary::ComposeGeoRGBRecipeMTGInThread()` composes FCI RGB
recipes directly from top-of-atmosphere radiance. No atmospheric correction is
applied, so molecular (Rayleigh) scattering leaves a blue cast over ocean and a
haze that thickens toward the limb. The effect is strongest in the shortest
bands — `vis_04` at 0.444 µm has 4.5× the Rayleigh optical depth of `vis_06` at
0.640 µm — which is precisely the band set the True Color recipe uses, so True
Color is the worst-affected product.

Two pre-existing facts shape the design:

1. **`core/rayleigh.cpp` is dead code.** It is compiled (`core/CMakeLists.txt:26`)
   but never called from anywhere in the tree. Its optical-depth table
   (`rayleigh.cpp:17-25`) is wrong for the bands it names — it gives `vis_08`
   (0.865 µm) a τ of 0.0318, roughly a 740 nm value, and `nir_16` (1.61 µm) a τ
   of 0.0087, roughly a 950 nm value. Its `correctPixel()` path-reflectance term
   (`rayleigh.cpp:155`) divides by `4π·μs·μv` *and* multiplies by
   `(1 − exp(−τ(1/μs+1/μv)))`, double-counting τ and introducing a spurious 1/π,
   so the correction it computes is around two orders of magnitude too small.
   The file also carries an unrelated hand-rolled SGP4/TLE parser
   (`rayleigh.cpp:207-410`) that duplicates the `QSgp4/` submodule.

2. **FCI reflectance is not sun-normalized.** `segmentlistgeostationary.cpp:9819`
   computes `physVal = π·L / solar_irradiance`, which is BRF × cos(SZA), not BRF.
   Rayleigh path reflectance is defined in BRF space, so the correction cannot be
   applied to these values as they stand.

## Goals

- Remove the Rayleigh path reflectance from every solar band of every FCI RGB
  recipe, per band, before the recipe combines bands into R/G/B.
- Match the Satpy/EUMETSAT processing chain: sun-normalize, then de-Rayleigh.
- Keep the app offline and dependency-free — no LUT downloads, no new libraries.
- Stay within the existing memory envelope of the FCI compose path.

## Non-goals

- Aerosol correction.
- Re-tuning `rangefrom`/`rangeto`/`gamma` for the affected recipes (see Risks).
- Any change to MSG/SEVIRI, HDF5, or other geostationary paths.
- Any change to `core/good_segmentlistgeostationary.cpp` (untracked working-tree
  copy, not in the build).

## Decisions

| Decision | Choice | Rationale |
|---|---|---|
| Which recipes | All solar-band recipes | Per-band τ makes NIR corrections automatically negligible, so no special-casing is needed and the treatment is physically uniform. Affects 7 of the 13 FCI recipes: 4 (Day Severe Storms), 5 (Natural Colors), 6 (Snow), 7 (Cloud Phase), 10 (True Color), 11 (Natural Colors Enhanced), 12 (True Color Enhanced). Recipes 0–3, 8, 9 are IR-only and untouched. |
| Normalization | Full Satpy chain: sun-normalize, then de-Rayleigh | Makes the 0–1 recipe ranges mean actual reflectance and matches how EUMETSAT defines these products. |
| Physics model | Analytic single-scattering, homogeneous layer | No data files, no network, ~10 flops/px. Underestimates by 5–10 % of the Rayleigh signal at 444 nm and less at longer λ — below the perceptual threshold after a 2.2 gamma stretch. |
| User control | INI setting + checkbox, default ON | The checkbox is what makes the brightness change from sun-normalization auditable via A/B comparison. |

Rejected: pyspectral LUTs (needs external HDF5 files fetched over the network and
a new sensor-specific read path); single-scattering plus a second-order term
(adds tuning constants with no in-repo validation data). Both remain open later —
neither changes the `RayleighCorrector` interface.

## Physics

### Rayleigh optical depth

Bodhaine et al. (1999) / Hansen & Travis (1974), sea level, P₀ = 1013.25 hPa,
λ in µm:

```
τ_r(λ) = 0.008569 λ⁻⁴ (1 + 0.0113 λ⁻² + 0.00013 λ⁻⁴)
```

Evaluated at the FCI solar band centres:

| Index | Band | λ (µm) | τ_r |
|---|---|---|---|
| 0 | vis_04 | 0.444 | 0.2339 |
| 1 | vis_05 | 0.510 | 0.1324 |
| 2 | vis_06 | 0.640 | 0.0525 |
| 3 | vis_08 | 0.865 | 0.0155 |
| 4 | vis_09 | 0.914 | 0.0124 |
| 5 | nir_13 | 1.380 | 0.00238 |
| 6 | nir_16 | 1.610 | 0.00128 |
| 7 | nir_22 | 2.250 | 0.000335 |
| 8–15 | IR bands | — | 0 (not corrected) |

Wavelengths are hardcoded rather than read from the INI's `spectrumvalueslist`,
which is rounded to 2 decimals (`0.86` for a 0.865 µm band, a 2.4 % τ error) and
is user-editable.

### Phase function

Polarization-corrected, depolarization factor δ = 0.0279 (Young 1980),
γ = δ/(2−δ) = 0.0141474:

```
P(Θ) = 3/(4(1+2γ)) · [(1+3γ) + (1−γ)cos²Θ]
     = 0.7293684 · [1.0424420 + 0.9858527 cos²Θ]
```

This normalizes to 1 over the sphere: ½∫₋₁¹ P dμ = 0.7293684 × (1.0424420 +
0.9858527/3) = 1.0000.

Scattering angle from the three geometry angles:

```
cos Θ = −μ₀ μ_v + sin θ₀ sin θ_v cos(Δφ)
```

Δφ is the relative azimuth, computed as `saa − vaa` folded into [0°, 180°]:
take the difference modulo 360°, then map values above 180° to 360° − x. Both
`snu_solar_params2` and `snu_vza_and_vaa` return azimuths in [0°, 360°) in the
same reference frame, and the cosine is even in Δφ, so no 180° offset is needed
(the MSG path adds 180° to both `saa` and `vaa`, which cancels in the difference).

### Path reflectance

Single-scattering solution for a homogeneous Rayleigh layer, single-scattering
albedo ω = 1:

```
ρ_ray = P(Θ) / (4(μ₀ + μ_v)) · [1 − exp(−τ (1/μ₀ + 1/μ_v))]
```

Both μ₀ and μ_v are floored at cos(88°) = 0.0348995 inside this function. This
matters: from geostationary orbit vza runs from 0° at nadir to **90° at the
visible disc edge**, so μ_v → 0 and the 1/(μ₀+μ_v) term would diverge at the
limb. Clamping saturates the correction over the last degree or two of an
already-smeared limb; masking instead would leave a visible black ring.

### Sun-zenith correction

Satpy `sunzen_corr_cos` with `limit = 88°`, `max_sza = 95°`:

```
SZA < 88°:   f = 1 / cos(SZA)
SZA ≥ 88°:   f = max(0, 1 − log₂(1 + (SZA − 88)/7)) / cos(88°)
```

Continuous at 88° (both branches give 1/cos 88° = 28.65) and reaching exactly 0
at 95°, so the terminator fades smoothly rather than cutting hard.

### Per-band chain

```
BRF        = (π·L / E₀) · f(SZA)
BRF_corr   = max(0, BRF − ρ_ray(τ_band, SZA, VZA, RAA))
```

`BRF_corr` then feeds the existing recipe stretch unchanged.

## Architecture

### `core/rayleigh.{h,cpp}` — rewritten

The SGP4/TLE half (`rayleigh.cpp:207-410`) is deleted; nothing references it and
`QSgp4/` already provides orbit propagation. The `QVector`/`QFuture`/`QImage`
batch APIs are dropped too — the caller owns raw `float*` buffers and its own
parallel loop, so those are unused indirection. What remains is three pure,
stateless functions with no Qt-image and no netCDF dependency:

```cpp
class RayleighCorrector
{
public:
    // Sea-level Rayleigh optical depth for FCI band index 0..15.
    // Returns 0.0 for IR bands (index >= 8).
    static double opticalDepthFCI(int bandIndex);

    // Satpy sunzen_corr_cos equivalent. Returns 0.0 for night (SZA > maxSza).
    static float  sunZenithFactor(float szaDeg,
                                  float limitDeg  = 88.0f,
                                  float maxSzaDeg = 95.0f);

    // Single-scattering Rayleigh path reflectance.
    static float  pathReflectance(double tau, float szaDeg,
                                  float vzaDeg, float raaDeg);
};
```

Testable in isolation with no satellite data.

### `core/nav_util.{h,cpp}` — one addition

`snu_solar_params2()` recomputes the full solar ephemeris on every call, but
declination and equation-of-time depend only on `jtime`, which is constant across
an FCI disc. Splitting the time-only part out lets it be hoisted from the
per-pixel loop:

```cpp
struct snu_solar_epoch { double delta, gw_mean_sol_time, gw_appar_sol_time; };

void snu_solar_epoch_init(double jtime, struct snu_solar_epoch *e);
void snu_solar_params_at(const struct snu_solar_epoch *e, double jtime,
                         double lat, double lon,
                         double *mu0, double *theta0, double *phi0);
```

`snu_solar_params2()` remains, reimplemented in terms of both. Existing MSG
callers are untouched.

### `core/segmentlistgeostationary.cpp` — one new private method

`applyFCISolarCorrection()`, called from `ComposeGeoRGBRecipeMTGInThread()`
between the file-read loop and the band-combination loop — that is, after all
bands are in `bandBuf` and before R/G/B are assembled. Correcting there means it
happens once per band, which is what the physics requires (each band has its own
τ) and what a band shared between two recipe channels needs.

## Geometry: computed per pixel, stored nowhere

The MSG recipe path stores six full-disc `float` arrays (`lat`, `lon`, `sza`,
`saa`, `vza`, `vaa`). At 3712² = 13.8 M px that is 331 MB. At FCI 11136² =
124 M px the same six arrays are **2.97 GB**, on top of the ~3.5 GB the compose
function already peaks at (`bandBuf` 1.5 GB + `result` 1.5 GB + output QImage
496 MB). That is not viable.

Geometry is therefore computed inline and discarded — one pass over the disc,
parallelized with `QtConcurrent::map` over row blocks, computing each pixel's
angles once and applying them to every solar band present:

```
epoch = snu_solar_epoch_init(jtime)
Xsat, Ysat, Zsat = 42164·cos λs, 42164·sin λs, 0

for each row block (parallel):
  for each line, pixelx:
    display_row = outRes - 1 - line          // compose buffer is south-up
    if pixcoord2geocoord(subLon, pixelx, display_row,
                         coff, loff, cfac, lfac, &lat, &lon) != 0:
        continue                             // off-disc, stays FILL_VALUE_F
    snu_solar_params_at(&epoch, jtime, lat·D2R, lon·D2R, &mu0, &sza, &saa)
    snu_vza_and_vaa(lat, lon, 0.0, Xsat, Ysat, Zsat, &vza, &vaa)
    raa = normalize180(saa - vaa)
    f = sunZenithFactor(sza)
    for each solar band bi (bandIndex < 8):
      if bandBuf[bi][p] == FILL_VALUE_F: continue
      if f == 0.0f:                          // night
          bandBuf[bi][p] = 0.0f
      else:
          brf = bandBuf[bi][p] * f
          bandBuf[bi][p] = max(0.0f, brf - pathReflectance(tau[bi], sza, vza, raa))
```

Extra memory: zero. Cost: roughly 300 ns/px for the shared geometry, so about
3–5 s on a 12-thread machine — small next to the CharLS decompression of 41 FCI
segments the function already performs.

### Why `pixcoord2geocoord` and not `snu_line_column_to_lat_lon`

Both implement the same CGMS LRIT/HRIT inverse projection, but they are called
with opposite conventions. `nav_scaling_factors_vir` (`internal.cpp:82-83`)
hardcodes **negative** CFAC/LFAC (−781648343), while `GeoSatellites.ini` stores
them **positive** for every satellite including MET_12 (+1172050000 / +2344100000).
Negating CFAC mirrors the column about COFF and negating LFAC mirrors the row
about LOFF, so the two forms are equivalent under a row/column flip.

The INI + `pixcoord2geocoord` combination is already proven correct in the field:
it is what `FormImage::DrawLongLat` uses to place the MET_12 coastline overlay
(`formimage.cpp:1681-1686`). Reusing it inherits a known-good sign convention
rather than re-deriving one, at the cost of passing `display_row = outRes − 1 −
line` because the compose buffer is south-up while the overlay works in display
coordinates.

MET_12 nav parameters from `GeoSatellites.ini`: `coffhrv`/`loffhrv` = 5568,
`cfachrv`/`lfachrv` = 2344100000 for the 11136 grid; `coff`/`loff` = 2784,
`cfac`/`lfac` = 1172050000 for 5568. Sanity check: 2344100000 / 2¹⁶ = 35766.6,
giving 27.96 µrad/px — 1 km at nadir from 35786 km. Correct.

### Satellite position

Nominal geostationary vector at the sub-satellite longitude from the INI
(`longitude = 0` for MET_12): `X = 42164·cos λs, Y = 42164·sin λs, Z = 0`.

FCI segment files carry no orbit polynomials the way the MSG prologue does.
Station-keeping holds MTG-I1 within about 0.1° of nominal, which moves vza by
under 0.1°.

### Acquisition time

`jtime` is derived from the nominal slot in `filedatestring` ("YYYYMMDDHHMM"),
converted to Julian Date and used as mid-scan for the whole disc:

```cpp
QDateTime dt = QDateTime::fromString(filedatestring, "yyyyMMddhhmm");
dt.setTimeSpec(Qt::UTC);
double jtime = dt.toMSecsSinceEpoch() / 86400000.0 + 2440587.5;
```

The MSG path varies scan time per line; FCI does not here. Over a 10-minute
repeat cycle the extreme rows carry up to ±2.5° of solar hour angle error, which
perturbs the scattering angle slightly and is invisible after the gamma stretch.

## Error handling

| Case | Handling |
|---|---|
| Off-disc (`pixcoord2geocoord` returns −1) | Leave `FILL_VALUE_F`; renders black as today |
| Pixel already `FILL_VALUE_F` in `bandBuf` | Skipped, stays fill |
| Night, SZA > 95° | Solar bands set to `0.0f`, not fill — keeps the dark limb continuous instead of punching a hole through difference channels |
| Terminator, 88° ≤ SZA ≤ 95° | Satpy log falloff, factor decreasing to 0 |
| μ₀ or μ_v below cos(88°) | Floored inside `pathReflectance` |
| Subtraction goes negative | Clamped to 0 |
| IR band (index ≥ 8) | Skipped entirely; τ = 0, no sun-normalization |
| `cfachrv`/`lfachrv` zero or missing in `GeoSatellites.ini` | Skip the whole correction, `qWarning()`, emit the current uncorrected output |
| `filedatestring` unparseable | Same as above — skip, warn, uncorrected output |

The `cfachrv` guard is not hypothetical: several satellites in `GeoSatellites.ini`
have `cfachrv=0` (H9 among them). MET_12 has valid values, but the guard means a
bad INI degrades to today's behaviour rather than producing a garbage image.

## Options and UI

| File | Change |
|---|---|
| `core/options.h` | `bool bFciRayleigh;` alongside `copyMTGfiles` (~line 277) |
| `core/options.cpp` | Read `/parameters/fcirayleigh`, default `true` (~line 236); write back (~line 614) |
| `core/formtoolbox.ui` | `QCheckBox chkFciRayleigh` in `horizontalLayout_fcirecipe`, next to `btnFCIRecipes`, checked by default |
| `core/formtoolbox.cpp` | Initialise from `opts` where `lstFCIRGB` is populated (~line 472); write back on toggle |

## Testing

`core/` has no test infrastructure, which is the failure mode that let the
existing wrong τ table sit unnoticed — nothing ever called it. The three new
functions are pure and dependency-free, so they are cheap to pin down. There is
precedent for standalone test binaries in `bin/filter_test.cpp`.

`core/rayleigh_test.cpp`, an optional CMake target (`-DBUILD_TESTS=ON`, off by
default so normal and AppImage builds are unaffected):

- τ for all eight solar bands against the reference values tabulated above, 1 %
  tolerance — the check that would have caught the existing bug
- `sunZenithFactor`: equals 1/cos(SZA) below 88°; continuous across 88°; exactly
  0 at 95° and beyond; never negative
- `pathReflectance`: → 0 as τ → 0; symmetric under μ₀ ↔ μ_v swap; agrees with the
  small-τ limit τ·P(Θ)/(4μ₀μ_v) to within 1 % at τ = 0.001; finite at vza = 90°
- phase function normalizes to 1 over the sphere

Acceptance is visual: compose True Color on a real FCI slot with the checkbox on
and off. A correct result shows the blue cast lifting over ocean and haze
thinning most at the limb and least near nadir, with `vis_04` changing far more
than `vis_06` since τ differs by 4.5×.

## Risks and follow-ups

**All seven solar recipes get brighter away from the subsolar point.** This is
inherent to sun-normalization, not a defect — the existing `rangefrom`/`rangeto`/
`gamma` values in `segmentimage.cpp:826-1010` were tuned against BRF×cos(SZA).
Re-tuning is deliberately out of scope; the checkbox exists so the two can be
compared before deciding whether to re-tune.

**Recipe 4, Day Severe Storms, will be the most disrupted.** Its blue channel is
a *difference*, `nir_16 − vis_06` (`segmentimage.cpp:890-897`), stretched over a
narrow −0.7 to 0.2 range. Sun-normalizing both terms scales the difference by
1/cos(SZA), so that range no longer means what it did — the channel will wash out
away from the subsolar point far more visibly than the single-band recipes do.
The other two channels are IR and unaffected. Physically the change is correct,
but this recipe is the strongest candidate for range re-tuning as a follow-up.

**Single-scattering underestimates at 444 nm** by roughly 5–10 % of the Rayleigh
signal. Adding a second-order term later requires no interface change.

**`core/good_segmentlistgeostationary.cpp`** is an untracked working-tree copy of
`segmentlistgeostationary.cpp`, not in the build. It is not modified and will
drift further out of sync.

## References

- Bodhaine, B. A. et al. (1999), *On Rayleigh Optical Depth Calculations*,
  J. Atmos. Oceanic Technol. 16, 1854–1861
- Hansen, J. E. & Travis, L. D. (1974), *Light Scattering in Planetary
  Atmospheres*, Space Sci. Rev. 16, 527–610
- Young, A. T. (1980), *Revised depolarization corrections for atmospheric
  extinction*, Appl. Opt. 19, 3427
- pyspectral Rayleigh correction: https://pyspectral.readthedocs.io/en/master/rayleigh_correction.html
- Satpy `sunzen_corr_cos`, `satpy/utils.py`
- `remotesensing-10-00560.pdf` (repo root)
- `S3TBX_Rayleigh_Correction_Tutorial.pdf` (repo root)
