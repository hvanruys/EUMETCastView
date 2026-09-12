# Parallel CLAHE on the geostationary button path — design

Date: 2026-09-12
Status: approved

## Goal

Make the CLAHE button on the geostationary toolbox tab fast enough and small
enough for Meteosat-12 images — FDHSI (5568²), HRFI (11136²) and the 0.5 km
sharpened composite (22272²) — by running the work on every core and by
cutting its temporary memory by an order of magnitude. Every geostationary
satellite goes through the same button path, so all of them get the change;
the output stays byte-for-byte what it is today.

## Where the time and memory go today

`FormToolbox::on_btnCLAHEGeostationary_clicked` → `FormImage::recalculateCLAHEGeo`
→ `FormImage::recalculateCLAHEMeteosat1` (`core/formimage.cpp:3618`), on the
GUI thread, single-threaded, four passes over the whole image:

1. RGB → Lab for every pixel through `ColorSpace` (three `pow()` and three
   `cbrt()` per pixel), stored as three `double` arrays `L`, `a`, `b`
2. `L` quantised to an 8-bit `ushort` array `pixelsL`
3. `SegmentImage::CLAHE(pixelsL, …, 16, 16, 256, opts.clahecliplimit)`
   (10×10 regions for Himawari-9)
4. Lab → RGB for every pixel (three `pow()`), written back in place

| Image | Pixels | Temporaries today (24 B/px Lab + 2 B/px L) |
|---|---|---|
| FDHSI 5568² | 31 M | ~0.8 GB |
| HRFI 11136² | 124 M | ~3.2 GB |
| 0.5 km sharpened 22272² | 496 M | ~13 GB |

The colour-space passes dominate the wall-clock time; the memory footprint is
what puts the sharpened composite out of reach.

`SegmentImage::CLAHE` (`core/segmentimage.cpp:1373`) is the Zuiderveld kernel:
phase 1 builds, clips and maps one histogram per contextual region; phase 2
bilinearly interpolates the four surrounding mappings over every sub-block.
Both phases are embarrassingly parallel — each region writes only its own
slot of `pulMapArray`, each sub-block writes only its own pixels — but they
run serially, walking a running pointer through the image.

## Components

### 1. `SegmentImage::CLAHE` — parallel kernel, same interface

Signature, return codes, the argument checks, the `fCliplimit == 1.0` early
return, and the five helpers (`MakeLut`, `MakeHistogram`, `ClipHistogram`,
`MapHistogram`, `Interpolate`) are unchanged. The helpers already touch only
their arguments, never member state, so they are safe to call from worker
threads.

Both phases become `QtConcurrent::blockingMap` over a `QVector<int>` of
indices, the pattern `SegmentListGeostationary` already uses for the MTG
compose (`concurrentMinMaxMTG`, `concurrentLUTGeoMTG`, …):

- **Phase 1**: one index per contextual region, `0 … uiNrX*uiNrY-1`. The task
  derives `uiX = idx % uiNrX`, `uiY = idx / uiNrX`, the tile origin
  `pImage + uiY*uiYSize*uiXRes + uiX*uiXSize`, and its histogram slot
  `&pulMapArray[uiNrBins*idx]`, then runs `MakeHistogram`, `ClipHistogram`,
  `MapHistogram` exactly as now.
- **Phase 2**: one index per sub-block, `0 … (uiNrX+1)*(uiNrY+1)-1`. The
  task derives `uiX`, `uiY`, the existing `uiSubX/uiSubY/uiXL/uiXR/uiYU/uiYB`
  border cases, and the block origin as an explicit formula instead of the
  running pointer:
  `x0 = (uiX == 0) ? 0 : uiXSize/2 + (uiX-1)*uiXSize`, same for `y0`. It
  calls `Interpolate` on `pImage + y0*uiXRes + x0`. `pulMapArray` and `aLUT`
  are read-only by then.

Integer arithmetic is untouched, so the result is bit-identical to the serial
kernel regardless of thread count or scheduling — for even region sizes,
which every FCI, MSG full-disc, Himawari and FY image has. The serial
kernel's running pointer advances by `2*(uiXSize>>1) + (uiNrX-1)*uiXSize`
per block row, one pixel short of `uiXRes` when `uiXSize` is odd, so on
those images each block row starts one pixel further left than the one
above: the interpolation grid is skewed by up to `uiNrY` pixels at the
bottom, one column per block row is mapped twice and the last columns of a
block row's final line are mapped with the row below's weights. GOES
(5424/16 = 339), MSG RSS (1392/16 = 87), HRV RSS (2320/16 = 145) and OLCI
(4688/16 = 293) are affected today. The explicit block origin puts every
block where the region grid says it belongs, so those images come out
slightly different, and correct. 256 and 289 tasks spread well over any
core count; no `#ifdef CONC` switch — that define is local to
`segmentlistgeostationary.cpp` and the kernel has no debugging need for a
serial fallback.

The `qDebug()` lines announcing the two phases stay.

### 2. `SegmentImage::CLAHELab` — the pipeline, out of `FormImage`

New method next to `CLAHE` in `core/segmentimage.h` / `.cpp`:

```cpp
int CLAHELab(QImage *image, unsigned int uiNrX, unsigned int uiNrY, float fCliplimit);
```

It holds the RGB → L → CLAHE → RGB work that `recalculateCLAHEMeteosat1` does
today, and returns what `CLAHE` returns (0, or its negative error code, in
which case the image is left as it was — the L pass has not written anything
back yet).

`FormImage::recalculateCLAHEMeteosat1` becomes the GUI wrapper it really is:

```
sl = active segment list; set wait cursor; progress 10
imageptrs->CLAHELab(imageptrs->ptrimageGeostationary,
                    H9 ? 10 : 16, H9 ? 10 : 16, opts.clahecliplimit)
progress 100
render3dgeo unless HRV / HRV Color (as now)
restore cursor
```

The intermediate progress values 30/60/80 go: the GUI thread is inside the
work and never repaints them.

The point of the split is that the pixel work is callable from a no-GUI probe
(section 5). `recalculateCLAHEGeo`, its commented-out MTG dispatch, and the
empty `recalculateCLAHEMTG` stub are left alone.

### 3. Memory: keep only L, recompute a and b

`CLAHELab` keeps one temporary, `pixelsL` (`ushort`, 2 B/px):

- **Pass 1**, one task per scanline: RGB → Lab via `ColorSpace::Rgb::To<Lab>`,
  clamp `l` to `[0, 100]`, `qRound(l * 255.0 / 100.0)`, clamp to 255, store in
  `pixelsL`. Same expression chain as today; `a` and `b` are discarded.
- **`CLAHE`** on `pixelsL` with the caller's region counts and clip limit,
  `Min = 0`, `Max = 255`, `uiNrBins = 256`.
- **Pass 2**, one task per scanline: for every pixel read the still-original
  RGB from the image, convert to Lab again for `a` and `b`, substitute
  `l = pixelsL * 100.0 / 255.0` clamped to `[0, 100]`, convert back with
  `Lab::To<Rgb>`, clamp each channel to `[0, 255]`, write `qRgb((int)r, (int)g, (int)b)`.

The second RGB → Lab conversion is one extra parallel pass; in exchange `a`
and `b` are never stored and stay `double`, so the pipeline's output is
bit-identical to today's. Temporaries: HRFI ~250 MB instead of ~3.2 GB, the
0.5 km composite ~1 GB instead of ~13 GB.

### 4. Threading details

- `blockingMap` runs on the global `QThreadPool` and the calling thread; it
  is called from the GUI thread as today, so the button flow,
  `slotUpdateGeosat` and the wait cursor are unchanged. Moving the work to a
  worker thread with a `QFutureWatcher` is out of scope.
- Scanline pointers come from a `bits()` base plus `bytesPerLine()`, fetched
  once on the calling thread before each parallel pass. `QImage::scanLine()`
  from worker threads is not safe: it goes through the detach bookkeeping,
  which is not atomic.
- The `ColorSpace` converters are static, stateless functions over their
  arguments (`core/Conversion.cpp`); concurrent calls are safe.
- One task per scanline (5568 … 22272 tasks) load-balances well and needs no
  band arithmetic. Per-row lambdas capture `this`, the base pointer, the
  width and `pixelsL`.

### 5. Verification

No unit test framework exists in the repository; verification is a probe that
links the application's own objects, the technique documented in
`/home/hugo/EUMETCastTools/viiprobes/README.md`.

- **`claheprobe`** in `/home/hugo/EUMETCastTools/viiprobes/`, added to the loop
  in that directory's `build.sh`. It carries verbatim copies of today's serial
  `CLAHE` (with its five helpers) and of today's `recalculateCLAHEMeteosat1`
  pixel loops (operating on a `QImage*` instead of `ptrimageGeostationary`)
  as reference implementations, and on synthetic images asserts byte-for-byte
  equality with the new code:
  - kernel: 5568×5568 `ushort` gradient + noise, ranges 0–255 and 0–1023,
    regions 16×16 and 10×10 (5500×5500 for the latter), clip limits 1.0
    (the early-return path), 3.0 and 6.9 (the OLCI value) — asserted equal;
    plus one odd-region case, 5424×5424 at 16×16 (GOES), where the serial
    kernel's drift makes a difference expected: reported, not asserted;
  - pipeline: 5568×5568 `QImage` (`Format_ARGB32`) gradient + colour noise,
    with a band of alpha-0 black pixels the way a space-filled disc has,
    against the reference loops.
  It prints old-vs-new timings for both. It runs under
  `QT_QPA_PLATFORM=offscreen` with cwd = `EUMETCastView/bin`, like the other
  probes.
- **In the application**: compose an HRFI image, press CLAHE, compare the
  elapsed time in the log before and after, and look at the result.
- **Build** both targets from the top-level `CMakeLists.txt`. `EUMETCastVideo`
  carries its own copies of the CLAHE kernel in `video/videomaker.cpp` and
  `video/rssvideo.cpp` and does not compile `core/segmentimage.cpp`; it is
  not affected and not changed.

## Out of scope

- The video tool's private CLAHE copies.
- The polar CLAHE paths (`recalculateCLAHEAVHRR`, `RecalculateCLAHEOLCI`,
  `RecalculateCLAHEVII`), `CLAHERGBRecipe`, and the recipe-time CLAHE calls
  inside `SegmentListGeostationary`. They all call `SegmentImage::CLAHE` and
  so get the parallel kernel for free, but their own per-pixel loops are not
  touched.
- Running the button's work off the GUI thread.
- Any change to the CLAHE parameters (region counts, clip limit, bins) or to
  the Lab round trip.
