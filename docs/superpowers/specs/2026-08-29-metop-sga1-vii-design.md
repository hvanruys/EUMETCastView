# Metop-SG A1 VII (METimage) L1B implementation — design

Date: 2026-08-29
Status: approved

## Goal

Finish `SegmentVII` / `SegmentListVII` so VII reaches feature parity with the
OLCI path: read radiances, reconstruct full-resolution geolocation from the
tie-point grid, compose the flat image, drive the globe texture, support every
histogram method, the three map projections, and 48-bit PNG export.

## Product facts (measured on the reference granule)

`/home/hugo/EUMETCastTools/ncstruct/W_XX-EUMETSAT-Darmstadt,SAT,SGA1-VII-1B-RAD_C_EUMT_20210422165742_G_D_20080223084603_20080223084703_T_B____.nc`

- `/data`: `num_scans=35`, `num_chan=20`, `num_pixels=3144`, `num_pixels_alt=24`,
  `num_lines=840`
- `/data/measurement_data`: `zone_size_act=8`, `zone_size_alt=8`,
  `num_tie_points_act=394`, `num_tie_points_alt=140` (= 4 x num_scans)
- 20 radiance variables `vii_443` … `vii_13345`, all `ushort(num_lines, num_pixels)`,
  each with its own `scale_factor` / `add_offset` / `_FillValue` / `valid_min` /
  `valid_max`. Ranges differ per channel (1021 … 16381 counts).
- Geolocation is on the tie grid only: `latitude` (NC_INT), `longitude` (NC_UINT).
- `delta_lat_N_dem` / `delta_lon_E_dem` are full-resolution DEM shifts in metres.
- `solar_zenith`, `solar_azimuth`, `observation_zenith`, `observation_azimuth`
  are on the tie grid.
- No per-pixel quality flags. `/data/quality_information` is per scan/channel;
  `/data/processing_flags/pixel_duplication_mask` is `ubyte(24, 3144)`.

Measurements:

- Serial tie-point reconstruction of the whole 840 x 3144 grid: **171 ms**
  (48 ms on 4 threads). Tie-coincident pixels reproduce their tie points to
  **3.7e-06 deg**.
- VII half-scan angle from the tie grid: **54.18 deg** (first pixel side) and
  **53.85 deg** (last pixel side); swath arc 2697 km; altitude 830 km.
  `Segment::CalculateCornerPoints` currently gives `SEG_METOPSGA1` the AVHRR
  default of 55.36 deg, ~65 km too wide on each side.

## Components

### 1. `core/viil1breader.h` / `.cpp` (new)

Port of `ncstruct/qt-version/ViiL1BReader`, implementing
EUM/LEO-EPSSG/SPE/14/777138 v5A section 4.2.4.1.3 (Eq. 1-12).

Kept: geometry consistency check; Eq. 2-6 stencil, including the
`zoneAlt + iscan` row offset that gives every scan its own pair of edge tie
rows so interpolation never crosses a bow-tie boundary; Eq. 7-10 ECEF
round-trip (WGS84 forward, Bowring inverse); Eq. 11/12 DEM shift about a fixed
6371 km radius; azimuth interpolation through sin/cos.

Changed:

- Serial, no QtConcurrent. `ReadSegmentInMemory` already runs on a
  `QtConcurrent::run` worker, so nesting `blockingMap` on the global pool is a
  stall hazard for no gain at 171 ms. Drops `PoolLimit`, `runOverLines`,
  `progressChanged`, and `QObject`.

Added:

- `readDuplicationMask(QVector<quint8> *)` — `/data/processing_flags/pixel_duplication_mask`
- `static QString channelVariableName(int band)` — band 1..20 -> `vii_443` … `vii_13345`
- `radianceRange(const QString &name, double *lo, double *hi)` —
  `valid_min/valid_max * scale_factor + add_offset`

### 2. `SegmentVII`

- Constructor: `earth_views_per_scanline = 3144`, `NbrOfLines = 840`
  (was OLCI's 4865 / 4091).
- `ReadSegmentInMemory()`: open -> geometry -> `initializeMemory()` ->
  `interpolateGeolocation()` -> `orthorectify()` when enabled ->
  `interpolateTiePointVariable("solar_zenith")` -> per-channel
  `readFullGridVariable()` -> `readDuplicationMask()` -> per-channel stats.
- Radiance packing: each channel maps to `0..65534` against its own **fixed**
  range from the file (`valid_min/valid_max * scale + offset`), `65535` = no
  data. Fixed rather than per-segment so the cross-segment stats merge in
  `ComposeVIIImageInThread` stays valid. Normalized = `radiance * sec(SZA)`
  packed with the same range, clamped at 65534 — not 65535, which would
  collide with the no-data sentinel (an existing OLCI defect not copied here).
- Geolocation stored as `QScopedArrayPointer<float> latitude/longitude` in
  degrees, longitude wrapped to -180..180. OLCI's packed int x 1e6 convention
  buys nothing when the reader already produces floats.
- Bow-tie mask applied in `ComposeProjection` / `MapPixel` and
  `RenderSegmentlineInTextureVII` only. Duplicated pixels are real
  observations: masking them in the flat 3144 x 840 image would punch black
  stripes through it, while in a projection they genuinely double-map.
- Ported from `SegmentOLCI`: `ComposeSegmentImage` (without the `quality_flags`
  block, which VII has no equivalent of), `RenderSegmentlineInTextureVII`,
  `ComposeSegment{GV,LCC,SG}Projection`, `ComposeProjection`, `MapPixel`,
  `recalculateStatsInProjection`, `RecalculateProjection`.

### 3. `SegmentListVII`

Mirrors `SegmentListOLCI`: `ComposeVIIImage(…, histogrammethod, normalized)`;
`setHistogramMethod` / `ChangeHistogramMethod`; `CalculateLUT`,
`CalculateLUTAlt`, `CalculateLUTFull`, `CalculateProjectionLUT` — and actually
calling `CalculateLUTAlt` / `CalculateLUTFull` from `ComposeVIIImageInThread`,
where they are currently commented out, which is why every LUT stays zero;
`Compose{GV,LCC,SG}Projection`; `ComposeSegments`; `Compose48bitPNG` and
`Compose48bitPNGSegment`; `RecalculateCLAHEVII`; `NbrOfSaturatedPixels`
counting pixels at the channel's `valid_max`; `searchLatLon`; `ShowWinvec`;
`SmoothVIIImage` / `SmoothVIIImage12bits`; `progressreadvalue`.

### 4. `Segment::CalculateCornerPoints`

Add a `SEG_METOPSGA1` branch: `delta1 = 54.18 deg`, `delta2 = 53.85 deg`.
The function already supports asymmetric deltas — OLCI uses them.

### 5. UI

Slots for the seven VII widgets that have no reference in `formtoolbox`:
`btnUpdateVIIImage`, `btnOverlayVII`, `cmbHistogramVII` (populated alongside
`cmbHistogramOLCI`), `btnSaveAsPNG48bitsVII`, `rdbMapTo65535VII`,
`rdbVIINormalized`, `btnAddVIIConfig`; plus `getVIIHistogramMethod()`.
`PROJ_VII` routed to `seglmetopsga1->Compose*Projection` everywhere
`PROJ_OLCI_EFR` is routed. `FormImage::ComposeImage` passes histogram method
and normalized through.

### 6. Options

`opts.viidemorthorectify`, INI key `/window/viidemorthorectify`, default true,
with a checkbox in `DialogPreferences` next to "VII Image on Texture".

## Pre-existing bugs fixed in passing

- `formtoolbox.cpp:1203` `getVIIColorList()` ends with `ui->cmbOLCI20`, and
  `formtoolbox.cpp:1217` `getVIIInvertList()` with `ui->chkInverseOLCI20`.
  Band 20 (VII_13345) reads OLCI's widgets.
- `options.cpp:545` writes `imageontextureOnVII` under the key
  `/window/imageontextureonslstr` while `options.cpp:148` reads it from
  `/window/imageontextureonvii`, so the setting never round-trips.

## Memory per segment

31.7 MB radiance (3 channels, raw + normalized), 21.1 MB lat/lon, 75 KB mask,
21 MB transient DEM, +47 MB of projection arrays while projecting.

## Verification

Against the ncstruct granule: tie-coincident pixels reproduce their tie points;
reconstructed corner lat/lon agree with `CalculateCornerPoints`; the GSHHS
coastline overlay lands on the coastlines of the composed image.

## As built: deviations from the design above

- **`ShowWinvec` was not added.** `SegmentList::ShowWinvec` in the base class
  already serves VII and `globe.cpp:717` already calls it through
  `seglmetopsga1`. `SegmentListOLCI`'s override is a byte-for-byte copy of the
  base; adding a third copy would change no behaviour.
- **The overlay is a lat/lon graticule, not a coastline.** `OverlayOLCI` draws
  the coastline that the OLCI product flags per pixel in `qualityFlags.nc`. VII
  carries no per-pixel flags at all, so `SegmentVII::CalcOverlayLatLon` marks
  the pixels where the whole-degree part of the reconstructed latitude or
  longitude changes, and `FormImage::OverlayVII` draws those in
  `opts.projectionoverlaylonlatcolor`. The dateline is excluded by rejecting
  neighbour steps over 180 degrees.
- **CLAHE uses 8 x 16 contextual regions,** not OLCI's 16 x 16: 3144 divides
  exactly by 8, so unlike OLCI the across-track direction needs no cropping.
  The packed radiances are stretched into CLAHE's 0..1023 range rather than
  clipped to it, which at 16 bits would leave nearly every pixel saturated.
- **Added beyond the design:** `SegmentVII::resetMemory()` overrides the base to
  release latitude/longitude (21 MB a segment), the duplication mask and the
  graticule when a segment is deselected; `ptrbaVII` and `ptrbaVIInormalized`
  are filled with the no-data marker immediately after `initializeMemory()` so
  any later read failure leaves a transparent segment rather than uninitialised
  memory; the LUT passes are skipped when a selection has no valid pixels, and
  every min/max denominator is floored at 1.

## Verified on the reference data

`/home/hugo/VII_L1B_CrossRef_TDP_V2/Test_Scenario_001/Output_Data/L1B_Output_Products/orbit3`
holds 110 consecutive one-minute granules and is already listed in
`bin/EUMETCastView.ini`.

- Tie-coincident pixels reproduce their tie points to 3.679e-06 deg
  (41 265 checked); reconstruction takes 156-171 ms a granule, serially.
- The nadir pixel lands 3.82 km from that scan's `latitude_ssp`/`longitude_ssp`,
  identically on all 35 scans - a fixed offset, since the nadir column still
  sits 0.468 deg off nadir, not a geolocation error.
- Five consecutive granules join with a 0.08-0.09 km gap between the last line
  of one and the first line of the next, a tenth of a pixel.
- `vii_668` packs against the same -0.475 .. 740.3 range in every granule, so
  the merged per-channel statistics are meaningful.
- One of the five granules has 816 lines rather than 840, which is why the
  dimensions are taken from the product instead of the constructor's nominals.
- DEM orthorectification moves pixels 274 m on average, 5.6 km at most,
  consistent with terrain parallax at 66 deg observation zenith.
- The band 1..20 name table matches the product's variable order exactly.

Confirmed in the running application: composing a VII image, and the Oblique
Mercator page drawing its borders and bounding box and then the projected
swath over them.

Not verified: the GVP, LCC and SG projection buttons, the histogram methods
including CLAHE, the overlay toggle and the 48-bit PNG export.

### CLAHE had no way to be selected

`CMB_HISTO_CLAHE` is 4, but every sensor histogram combo - OLCI, Geo, AVHRR,
MERSI and the VII one added here - is filled from a three item list, and only
`cmbHistogramProj` gets a fourth entry, `Equalize Projection` at 3. Index 4 was
therefore unreachable from anywhere in the UI, so `RecalculateCLAHEOLCI` has
always been dead and `RecalculateCLAHEVII`, written to mirror it, was dead the
moment it was added.

`cmbHistogramVII` now offers CLAHE. Row order cannot be the method, because 3
is `Equalize Projection` and means nothing for a flat image, so the combo
carries the `CMB_HISTO_*` value as item data and `getVIIHistogrammethod()`
reads `currentData()`.

The parameters check out against `SegmentImage::CLAHE`'s guard clauses: 8 x 16
regions against a `uiMAX_REG_X` of 16, 3144 % 8 and the reduced height % 16
both zero, `Max` of 1024 under `uiNR_OF_GREY` of 4096, and `MakeLut(0, 1024,
256)` giving a bin size of 5 and a top bin of 204 inside 256. The return value
is now logged, since CLAHE signals a refused geometry that way and leaves the
buffer alone, which would otherwise show up as a plain linear stretch with
nothing to explain it.

## Oblique Mercator (added after the original scope)

OM is a fourth projection that OLCI does not have, so it was outside the
"OLCI parity" scope agreed above. `obliquemercator.cpp` already carried
`PROJ_VII` through its guard clause, but the branch that fetches the central
line was commented out because `SegmentListVII` had no `GetCentralCoords`.

- `SegmentVII` now stores its reconstructed geolocation in the base class's
  `geolatitude` / `geolongitude` rather than in members of its own. Those are
  what `ObliqueMercator::GetMinMaxXBoundingBox` reads, and what VIIRS-M,
  VIIRS-DNB and MERSI already use for the same purpose, so VII needed one
  `SEG_METOPSGA1` branch there instead of a parallel bounding-box routine.
- `SegmentVII::getCentralCoords` returns the first and last usable centre
  pixel; `SegmentListVII::GetCentralCoords` spans them across the selection.
  The centre column is `earth_views_per_scanline / 2`, as in VIIRS-M - the true
  nadir column is 1576 rather than 1572, but four pixels do not matter to a
  central line thousands of kilometres long.
- `SegmentVII::ComposeSegmentOMProjection` plus an `OM` branch in
  `ComposeProjection` using `imageptrs->om->map_forward`;
  `SegmentListVII::ComposeOMProjection`; `ObliqueMercator::CreateMapFromVII`;
  `PROJ_VII` sizing and bounding box in `ObliqueMercator::Initialize`.
- `on_btnCreateOM_clicked` sets `currentProjectionType = PROJ_VII` in the guard
  block, before `Initialize`, because `Initialize` is handed that value and the
  branch that sets it for the other sensors runs afterwards.

Checked on the five consecutive granules: the central line runs
(62.372, 27.797) -> (45.188, 18.479), whose great circle has its pole at
latitude 9.770, implying an inclination near 99.8 deg against Metop-SG A1's
sun-synchronous 98.7 - the ~1 deg difference being the chord approximation a
great circle makes to a ground track. The track azimuth of -158.33 deg is a
descending pass, as expected at those latitudes.

### Oblique Mercator: the first attempt did nothing

Symptom: after composing a VII image, opening the Oblique Mercator page drew
neither the country borders nor the bounding box, and put up no message either.

Two causes, in series.

1. `opts.bellipsoid` is set to `true` in `options.cpp` and is never read from
   the INI or written by any control, so `ObliqueMercator::Initialize` always
   dispatches to `InitializeEllipsoid`. `InitializeSpherical` - the one that
   already carried a `PROJ_VII` mention, and the only one the first attempt
   patched - never runs. `InitializeEllipsoid` ends its projtype dispatch with
   a bare `else return`, so VII returned before the central line, the canvas
   size or the bounding box were set, silently. Fixed by giving it the same
   three `PROJ_VII` branches, using the OM spin boxes for the canvas as the
   other sensors do and `GetMinMaxXBoundingBox` for the extent, since VII fills
   `geolatitude` / `geolongitude`.

2. Nothing sets `currentProjectionType` when the projection input radio
   changes; only the create buttons do, each inline. The oblique mercator is
   the one projection whose `Initialize` is handed that value, so reaching its
   tool box page before pressing a create button passed whatever the previous
   projection had left there. `FormToolbox::inputProjectionType()` now derives
   it from the checked radio, and the two OM entry points and the OM create
   button all use it.

Checked by replaying `InitializeEllipsoid` and `omerfor` on the five granules:
the central line (62.372, 27.797) -> (45.188, 18.479) gives an azimuth of
16.881 deg, trips none of the three "Input data error" returns, and projects
8352 swath-edge points with none rejected as projecting to infinity. The extent
is 2751.4 km across track against the 2697 km swath arc measured from the tie
grid, and 2064.8 km along track against 5 minutes at roughly 6.7 km/s.
