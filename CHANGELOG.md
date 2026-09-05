# Changelog

## 2.1.3

Metop-SG A1 VII (METimage) is what this release is for. `SegmentVII` used to open
the product, print its dimensions and close it again, with everything else left as
commented-out OLCI code. It now reads the twenty channels, reconstructs
full-resolution geolocation from the tie-point grid, composes the flat image and
the globe texture, offers six RGB recipes and reaches all four map projections.

Alongside it: guards in the projection dialogs that could not run, OLCI products
unpacked in a temporary directory instead of next to the executable, and a video
tool that builds and debugs as a project of its own.

### Metop-SG A1 VII (METimage)

**Geolocation.** The product carries latitude and longitude on a coarse tie-point
grid only, so the new `viil1breader` reconstructs the full 840 × 3144 grid from
it, implementing PFS EUM/LEO-EPSSG/SPE/14/777138 § 4.2.4.1.3. It runs serially —
171 ms a granule — because `ReadSegmentInMemory` is already on a QtConcurrent
worker and nesting a second pool only risks starving the global one. Verified
against the 110 granules of Test_Scenario_001 orbit 3: tie-coincident pixels
reproduce their tie points to **3.7e-06°**, the nadir pixel lands a constant
**3.82 km** from `latitude_ssp`/`longitude_ssp` on every scan, and consecutive
granules join with a **0.08 km** gap. Dimensions come from the product rather
than from the constructor's nominals — one of those 110 granules has 816 lines
instead of 840. DEM orthorectification (Eq. 11/12) runs by default, under a new
preference.

**Orientation.** VII scans from the port side. Measured with great-circle
bearings over 13 granules spread across a full orbit, pixel 0 lies 90.1–90.6° to
the *left* of the flight direction, the same on ascending and descending passes.
The composed image puts line 0 at the top, so that edge belongs on the right, and
writing array column 0 to image x = 0 drew the swath mirrored — a descending
granule ran 48.8 E on the left to 0.8 E on the right, east on the left under a
north-up image. The across-track axis of the radiances, the geolocation, the
interpolated solar zenith and the duplication mask is now reversed as it is read,
rather than at draw time, because the flat image, the globe texture, the
graticule, `searchLatLon` and the 48-bit PNG all index those arrays. The
projections are unaffected: radiances and geolocation turn together, so every
(lat, lon, value) triple is the same and only the visiting order differs.

**Image and export.** Each channel packs onto 0..65534 against its own valid
range as the product states it — a property of the channel, not of the granule,
so per-channel statistics merged across segments stay comparable, and 65535 is
left free as the no-data marker. The bow-tie duplication mask applies to the
projections and the globe texture but not to the flat image, where the duplicated
pixels are real observations and masking them would punch black stripes through
the picture. VII has no per-pixel coastline flag of the kind OLCI draws, so the
image overlay is a lat/lon graticule built from the reconstructed geolocation,
spaced **5°** apart: at 0.75 km sampling a parallel every whole degree drew a mesh
dense enough to hide the image under it. Update VII Image, Overlay, the histogram
combo, Normalized, *Save 48bit RGB PNG* and *Add Configuration* were widgets with
no slot at all and are now wired up.

**Projections.** All four run for VII. The **oblique mercator** takes its central
line from `getCentralCoords` along the ground track; over five consecutive
granules that line runs (62.372, 27.797) to (45.188, 18.479), giving an azimuth
of 16.881°, no *Input data error* returns, and 8352 swath-edge points projected
with none rejected. The extent comes out 2751.4 km across track against the
2697 km swath arc measured from the tie grid, and 2064.8 km along track against
five minutes at roughly 6.7 km/s. Two faults had to be fixed before it drew
anything: `opts.bellipsoid` is set true in code and never read from the INI, so
`InitializeSpherical` never runs and the ellipsoid path — which ended its
dispatch with a bare `else return` — needed the VII branches; and nothing set
`currentProjectionType` when the projection input radio changed, so reaching the
oblique mercator page before pressing a create button used whatever the previous
projection had left behind. The OM overlay now also draws the **contour of the
projected swath** for VII, along columns 0 and 3143 — the real swath edges, not
the reduced extent the duplication mask leaves behind.

**Bow-tie seams are closed.** `pixel_duplication_mask` blanks the bow-tie overlap
at both swath edges in a staircase 1018 pixels wide on the first line of a scan,
narrowing to nothing over the twelve middle lines and widening to 1253 on the
last. That leaves each scan an hourglass, so the line continuing the ground past
the edge of one scan belongs to the next and sits up to 13 lines further on in
the array — and bilinear interpolation, which pairs a line only with line + 1,
never draws that quad. Measured over a granule the unreachable pairs are 0.24 km
apart on average and never more than one array step: a seam one pixel high along
the left and right edge of an oblique mercator image, and deep inside a zoomed
general vertical perspective one. At nadir, where the mask marks nothing, there
was no seam. Those quads are now bridged per column, using the same construction
`BilinearBetweenSegments` already draws at a segment join; where nothing is
masked the search returns line and line + 1 and the bridge is skipped. Rasterised
at 0.75 km/pixel, destination pixels left empty with covered neighbours all
round: **1743 before, 0 after**.

**Six RGB recipes**, chosen for what twenty channels can do that the
geostationary instruments cannot. A recipe names its channels, wants them in
physical units rather than packed radiance, and carries its own fixed stretch, so
the same scene renders the same way whichever granules are selected.

| Recipe | Channels | What it is for |
|---|---|---|
| **True Color** | 668 / 555 / 443 | VII has a real green, so three measured colours rather than a synthesised one |
| **Natural Color** | 1630 / 865 / 668 | the EUMETSAT standard |
| **Cirrus** | 1375 / 865 / 668 | 1.375 µm sits inside a water-vapour absorption band, so it sees only what is above the lower troposphere — thin cirrus a true-colour image misses entirely |
| **Fire Temperature** | 3959 BT / 2250 / 1630 | Planck's law as a colour ramp: how far up the spectrum a subpixel fire lifts radiance says how hot it is |
| **Day Land Cloud Fire** | 2250 / 865 / 668 | the EUMETSAT standard |
| **Cloud Top Height (O₂-A)** | (752 − 763)/(752 + 763) | the O₂-A pair measures how much air lies above whatever reflected the light, separating a thin high cloud from a bright low one |

`ViiL1BReader` grew the two conversions these are written against: reflectance
from `Band_averaged_solar_irradiance` and the sun–earth distance, and brightness
temperature through the inverse Planck function at the product's own centre
wavelengths and A/B coefficients. The finished brightness is stored on the
radiance scale with statistics pinned to the full range, so the compose and the
four projections reproduce it exactly at a 100 % stretch and none of them needs a
recipe-specific path. The two ranges with no standard behind them were measured
rather than guessed: over a granule split between deep cloud and warm surface the
O₂-A index runs 0.10 to 0.39 — pixels colder than 240 K at a median of 0.173,
warmer than 280 K at 0.291 — hence 0.15 to 0.33; the cirrus channel sits at 0.001
to 0.004 over cloud-free ground, hence 0 to 0.12 with a brightening gamma.
Rendered against real granules, Cirrus and the O₂-A index light up the same cloud
tops from two unrelated measurements.

**Rayleigh correction.** The checkbox did two jobs at once, copied from the FCI
path: sun-normalise the solar channels, and take the molecular haze off them.
Only the second is what the label names. Sun-normalisation is part of the unit
the recipes are stretched against — EUMETSAT's "0 to 100 %" is a bidirectional
reflectance factor — so unchecking the box did not give a hazier picture, it gave
one darkened by the cosine of the solar zenith, for all six recipes at once. Sun
normalisation now always runs and the preference governs the de-hazing alone.
Measured over a granule of ocean and marine cloud, turning the removal on moves
the finished image by this many of its 255 levels: True Color 10.3, Cirrus 4.3,
Natural Color 1.9, Day Land Cloud Fire 1.3, Fire Temperature 0.04 — following the
optical depth of each recipe's bluest channel, 0.236 at 0.443 µm against 0.0003
at 2.25 µm, so True Color is the only one where it decides anything. Cloud Top
Height is deliberately left uncorrected: the index reads the atmosphere above the
reflector, so removing a modelled atmosphere from both channels before dividing
takes away part of the signal. Three of the four tie-point interpolations, the
shoreline mask and the per-pixel radiative transfer are skipped when the de-hazing
is not going to run.

**CLAHE** is now reachable and works. `CMB_HISTO_CLAHE` is 4, but every sensor
histogram combo was filled from a three-item list, so index 4 could not be
selected from anywhere in the UI — `RecalculateCLAHEOLCI` has always been dead
code and `RecalculateCLAHEVII` was dead from the start. The VII combo now carries
the `CMB_HISTO_` value as item data instead of relying on row order. Reaching
`ComposeSegmentImage` and `MapPixel`, CLAHE matched no branch and left `colour`
read but never written; since it is a whole-image operation and the compose
worker runs a segment at a time, segments are composed with a plain 100 % stretch
and the CLAHE pass runs afterwards on the GUI thread. The projections follow the
image, degrading CLAHE to Equalize because there is no per-pixel form of it; an
explicit *Equalize Projection* still wins, since it re-equalises on the pixels
that land inside the projection.

**Fixes found on the way.**

- **Sensing times were read from the wrong offsets.** They were taken from
  offsets 16 and 32 of the file name, which are `rmst` and `-VII`; every field
  parsed as 0, so every VII segment carried a sensing time at the start of the
  epoch. They sit at 70 and 85. The timestamp at offset 51, which those offsets
  were presumably reaching for, is the EUMT processing time rather than the
  sensing window.
- **201 of 1540 granules never reached the segment list.** `ReadDirectories`
  collects a directory's files into a `QMap`, and VII files were keyed on the
  sensing start down to the minute. Granules last about a minute but do not start
  on one, so two regularly fall in the same minute and the second insert
  overwrote the first. The key is now the whole sensing window, which collides
  only for a genuine retransmission, and the list is sorted on
  `julian_sensing_start` once every directory has been read.
- **Footprints were drawn some 65 km too wide on each side.**
  `CalculateCornerPoints` gave `SEG_METOPSGA1` the AVHRR half-scan angle of
  55.36°; measured off the tie-point grid, VII is 54.18° on the first-pixel side
  and 53.85° on the last.
- **Equalized and 95 %-stretched images were black.** `CalculateLUTAlt` and
  `CalculateLUTFull` had been commented out of the compose path, which left every
  LUT zero.
- **A granule was decompressed once per band, colour combination or recipe.** The
  check meant to catch this looked in the process working directory instead of
  the temporary one, so it never matched and its result was only printed. A
  zero-length file counts as absent, since that is the leftover of an aborted run.
- `getVIIColorList` and `getVIIInvertList` read `cmbOLCI20` and
  `chkInverseOLCI20`, so band 20 (VII_13345) took OLCI's settings.
- `imageontextureOnVII` was saved under `/window/imageontextureonslstr` and
  loaded from `/window/imageontextureonvii`, so it never round-tripped.
- The `IMAGE_VII` case passed `SEG_MERSI` to `displayVIIImageInfo`.

### Projection dialogs

- **The AVHRR guard in the perspective dialog had its closing parenthesis one
  term too late**, which made it `!buttonMetop && !buttonHRP && SelectedAVHRRSegments()`
  — close to the opposite of what was meant. It warned only when neither
  satellite was enabled *and* segments were selected, and stayed quiet in exactly
  the case it exists for: Metop on, nothing selected, straight into the
  projection with an empty segment list.
- **Six of the seven stereographic guards were unreachable.** They tested
  `opts.buttonXXX` and only then the input radio button, and an `if`/`else if`
  chain stops at the first branch it *enters*, not the first that matches — so
  with Metop enabled the chain took the Metop branch, found `rdbAVHRRin`
  unchecked, did nothing and ended. Projecting VIIRS, OLCI or MERSI with nothing
  selected walked straight into the projection. Each is now keyed on its radio
  button, which is what says the input was chosen, and a refused projection says
  why instead of returning silently. Two of them widen while being moved: VIIRS M
  and DNB accept NOAA-20 and NOAA-21 as well as SUOMI NPP, where this chain named
  only the SUOMI NPP button.
- **VII with nothing selected is refused** in the perspective, Lambert and
  stereographic dialogs, which checked the other sensors but not VII.

### OLCI

- **Products are unpacked in the temporary directory.** libarchive resolves the
  relative entry paths in an OLCI tar against the working directory, so the
  `.SEN3` tree was written next to the executable and every reader built its file
  names relative to wherever the program happened to run. `SegmentOLCI` now has a
  `productDir()` — the product directory itself when the segment is delivered as
  a directory, the unpacked `.SEN3` under the temporary directory otherwise — and
  the 45 hand-built paths, radiances and geolocation alike, go through it. Exit
  cleanup still follows the existing *remove OLCI dirs* preference, but works
  from the temporary directory and keys on the `.SEN3` suffix every product
  carries rather than on the S3A and S3B prefixes.

### Video tool

- **Making a video stopped as soon as one ten-minute slot had no imagery behind
  it.** The GUI wrote `EUMETCastVideo.json` and then asked `GetDatestampsList`
  which frames to spawn, and for MET-12 those two disagree: the JSON's `files`
  object holds only the segments the chosen projection covers, while
  `GetDatestampsList` walks the unfiltered list and returns every slot on disc.
  The date list now comes from the `files` object of the JSON that was just
  written, so the frames asked for and the frames described are the same list by
  construction. `compileImageMTG` returns early on an empty segment path list
  rather than decoding a disc from nothing.
- **The timestamp overlay drifted in the same situation.** `getTimeFromIndex`
  converts a 1-based ten-minute slot to a wall-clock time and was being handed
  the sequential frame counter; drop slot 7 and every later frame was stamped ten
  minutes early, cumulatively.
- **`video/` configures as a project of its own**, which is what you want when
  the thing being debugged is the video tool and not the GUI. Every include path
  outside `video/` was written against `CMAKE_SOURCE_DIR`, which names whichever
  project is top level, and the same assumption ran through `meteosatlib`,
  `QSgp4` and all of `PublicDecompWT`; they now derive from
  `CMAKE_CURRENT_SOURCE_DIR`. Standing alone, `video/` adds those libraries
  itself and asks for hdf5 and netcdf on its own account — in the full build they
  were found only because `core/` happens to be configured first and
  `pkg_check_modules` leaves its results in the cache. The standalone build
  deliberately keeps its executable in its own build tree, so a debug build
  cannot replace the `bin/EUMETCastVideo` the GUI spawns.
- **The video tool was never reached on Windows.** `ProcessManager` spawned it as
  `./EUMETCastVideo`, a name resolved against the working directory rather than
  against the application. That works only when the program is started from its
  own directory, which is what happens on Linux and not what happens on Windows,
  where whatever launches EUMETCastView — a shortcut, QtCreator — chooses the
  working directory. The child never started and `tempvideo/` stayed empty for
  the whole run. The path now comes from
  `QCoreApplication::applicationDirPath()`; the `.exe` suffix is not needed,
  CreateProcess appends it when it parses the command line.
- **None of it was visible.** A process that would not start was reported with
  `qDebug()` only, and both executables are GUI subsystem applications on
  Windows, which have no console for `qDebug()` to reach — every diagnostic in
  the run was discarded. Process errors, a non-zero exit code, an empty
  selection and an empty `tempvideo/` are now written to the traffic list, the
  one place already being watched, and the run announces which binary it is
  about to spawn. `EUMETCastVideo` itself is no longer built as a GUI subsystem
  application, so running it by hand prints what it read and what it wrote;
  nothing pops up when the GUI spawns it, because QProcess passes
  `CREATE_NO_WINDOW` whenever the parent has no console of its own.
- **A process that failed to start stalled the queue.** No `finished()` is
  emitted for one, so it was never taken out of `activeProcesses` and no further
  task was ever started: the run neither progressed nor ended. Both endings now
  retire the process the same way. A process count of 0 — which the tooltip
  promises means "all" — started nothing at all, and is clamped to one.
- **`QImage::save()` is checked.** A `tempvideo/` that is not where the child is
  looking, and a full-disc image that could not be allocated — 11136 × 11136
  ARGB32 is 496 MB, and eight of these run at once — both end as `save()`
  returning false, which was ignored.
- **`video/video.pro` builds the video tool again.** It listed neither
  `videomaker.cpp` nor `jsonvideoreader.cpp`, and so not `compileImageMTG`,
  named a `geoseglist.cpp` that no longer exists, and had no Windows
  configuration at all. It now mirrors `core/core.pro`, down to writing its
  executable to the directory that one writes to. The top-level
  `EUMETCastView.pro` still cannot be used — it names a `PublicDecompWT-2.8.1`
  subproject that has no `.pro` file — so both executables come from the
  top-level CMake build, on Windows as well.

### Interface

- **Segment names are drawn on the globe in yellow.** `setPen` was given
  `Qt::Key_Yellow`, a key code rather than a colour; as an unscoped enum
  0x01000116 converted to `QRgb`, whose low three bytes gave a near-black
  `#000116` against the globe.
- A **VII Config** page in Preferences, for the band/colour configurations the
  other sensors already had, alongside *VII Image on Texture* and the DEM
  orthorectification checkbox. The toolbox gained a VII texture button.

### Settings

- **`/window/temporarydir`** (default `.`), with a field in Preferences. OLCI
  products are unpacked here and VII granules decompressed here, instead of
  beside the executable.
- **`/window/viidemorthorectify`** (default true) — apply the DEM shifts the VII
  product carries to the reconstructed geolocation.
- **`/parameters/viirayleigh`** (default true) — the molecular-haze removal for
  VII recipes, now separate from sun normalisation.
- **`/parameters/removeviifiles`** (default false) replaces
  `/parameters/removeslstrdirs`: delete the decompressed VII granules from the
  temporary directory at exit.
- **`/window/imageontextureonvii`** replaces `/window/imageontextureonslstr`.

## 2.1.2

One fix: the 3D globe would not start on systems whose default OpenGL context
is a compatibility profile below 3.3, failing with *GLSL 3.30 is not
supported*. This affected the 2.1.1 AppImage on Ubuntu 20.04.

The application asked for no particular OpenGL version or profile, so the
driver handed out its default — on Linux a compatibility profile, which Mesa
capped at OpenGL 3.0 and GLSL 1.30 for years. Every shader in `core/shader` is
`#version 330`. Newer distributions offer a high enough compatibility profile
that it worked there, which is why it went unnoticed.

It now requests **3.3 core** explicitly. This was never a hardware or driver
limit: the machine that failed reports core profile 4.5. Nothing in the drawing
code needs the fixed-function pipeline — every class already builds its geometry
from VAOs, VBOs and shaders.

`Globe` and `SkyBox` move to `QOpenGLFunctions_3_3_Core` with the build's new
`-DOPENGL33`, since the old `QOpenGLFunctions_3_0` is a compatibility class that
cannot be initialised on a core context. `SkyBox` had been pinned to it
regardless of the build setting; left alone it would have failed silently and
drawn no sky.

If you build with a different `OPENGL*` define, `Globe` and `SkyBox` must use
the same one, and the profile requested in `main.cpp` has to match.

## 2.1.1

Mostly MTG FCI: the imagery was being navigated on a grid that was not quite
the one it is composed on, in four separate places. Also a Rayleigh-corrected
SEVIRI Natural Colours, three compose-path bugs, and an AppImage that runs on
older distributions again.

### MTG FCI navigation

Four places geolocated FCI pixels with the CGMS scaling constants from the
settings, which approximate the FCI grid rather than describe it. Each is now
driven by the grid definition the files themselves carry — the `x`/`y`
`scale_factor` and `add_offset` in every BODY chunk — so the imagery, the
overlays and the projections all agree by construction instead of being tuned
against each other.

- **Reprojection** used the 1 km grid unconditionally and flipped rows against
  a hardcoded 11136. Every recipe containing an infrared band composes at 5568,
  so those images were navigated on a grid twice their size; the column and row
  error reached **5500 pixels**. GVP, LCC and SG now follow the composed image.
- **Land/sea mask and solar geometry** ran on the generic CGMS routine with the
  INI's `COFF`/`CFAC` and the MSG ellipsoid, landing **2–4 km** out across the
  disc. That put the mask a few pixels off every shoreline, which showed as a
  fringe of coastal land treated as sea. `pixcoord2geocoordFCI` had been
  declared for this and left as a stub returning without touching its outputs;
  it is now implemented.
- **Graticule and observer marker** sat up to **3 pixels** off on the 1 km grid
  and 2 on the 2 km one, drifting with latitude, with no constants at all for a
  0.5 km disc.
- **Coastline overlay** was drawn one pixel east of the imagery: it used the
  1-based grid column directly as an image column.

Points behind the limb are now refused rather than returned. `geocoord2pixcoordFCI`
previously returned success unconditionally with a visibility test that accepted
anything within 90° of nadir, so lat 0 lon 85 came back as a real column drawing
imagery from nowhere, while the far side survived on integer overflow of
`round(NaN)`.

### New recipe

- **FCI True Color NDVI RGB** — True Color RGB plus the NDVI vegetation
  enhancement from the GeoColor composite, which lifts dense forest out of the
  dark olive a strictly true-colour render gives it. Desert, ocean and cloud sit
  at an index near zero and are drawn exactly as True Color RGB draws them.

### Imagery

- **SEVIRI Natural Colours** is now Rayleigh corrected. Three geometry faults
  had to be fixed first, all measured against a full day of Meteosat-9 and -10:
  a sign error on the east component of the local vertical that reported viewing
  zenith angles of 114–123° and annihilated blue; the prologue orbit polynomials
  being read as a linear fit rather than the Chebyshev series they are, which
  put the satellite at 84 316 km instead of 42 164 km; and the solar leg of the
  surface reflectance being frozen too early, understating the light reaching
  the ground in proportion to optical depth — at a solar zenith of 88° the
  recovered VIS006 kept 83 % of its true value against 99 % for IR_016, a blue
  deficit growing with sun angle. Past 80° the modelled path reflectance
  approaches and then exceeds the measured signal, so both halves of the
  correction taper to nothing by 88°. The taper is opt-in and only SEVIRI uses
  it: FCI's optical depth is over four times SEVIRI's, and sparing that much
  haze turns its twilight blue. The diagnostic RGBs are deliberately left uncorrected —
  they are defined on top-of-atmosphere reflectance and correcting them would
  move the colours away from the reference images they are read against.
- **Land/sea mask** reads its own shoreline file and runs on a finer grid. See
  *Settings* below.

### Interface

- **One RGB recipe list.** The toolbox showed separate lists; it now shows a
  single one that follows the geostationary tab — FCI recipes for Meteosat-12,
  SEVIRI recipes for Meteosat-11/-10/-9, empty for anything else. That makes the
  two "wrong satellite" message boxes unreachable. One Rayleigh checkbox serves
  both, writing to whichever of `bFciRayleigh` / `bSeviriRayleigh` applies.

### Performance

- **FCI recipe segments are read concurrently.** Reading through netCDF runs the
  JPEG-LS decode inside HDF5, and a thread-safe libhdf5 wraps every entry point
  in one global mutex, so the obvious parallelisation bought nothing — sixteen
  threads burned 47 s of CPU to do 4.6 s of work. The read is now split: a
  serial header pass builds a task list, then a concurrent pass takes each
  chunk still compressed and decodes it outside the lock.
- **The bundled CharLS is built with optimisation.** It set no optimisation
  level of its own and did not inherit the project's, so the JPEG-LS decoder was
  compiled at `-O0`. Decoding three visible bands of one FCI disc took 18.9 s at
  `-O0` and **3.7 s** with the flags upstream's own Release build uses. Output
  verified bit-identical across builds.

### Fixes

- **Composing a VIIRS M image crashed** on NPP, NOAA-20 and NOAA-21 alike. The
  width and height spinboxes were set one at a time and each `setValue` called
  into `ObliqueMercator::Initialize`, so it always ran once on a half-updated
  pair — on the first image of a session, the new width with a height of zero.
  That reallocated the projection buffers at zero length while leaving the
  geometry looking valid, and the next pixel plotted ran off the end of the heap
  block.
- **The compose watcher was connected once per composed image**, so the finished
  slot ran once for every image composed so far in the session — the fifth
  compose called it five times, each one driving a full reprojection and redraw,
  and unbalancing Qt's cursor stack.
- **Compose workers no longer touch the GUI thread.** They called
  `setOverrideCursor`, `restoreOverrideCursor` and `processEvents` from a
  QtConcurrent worker; the cursor override stack is not thread safe, so these
  were undefined behaviour that happened to work. They were also redundant.
- **METIMAGE segments are reachable from the cylindrical map.** Metop-SG A1 had
  a segment list but no way to get at it: the sensor button did not toggle, the
  scrollbar had no maximum for it, and the segment list, window title and
  `RemoveAllSelected` all skipped it.

### Build and packaging

- **The AppImage is built in an Ubuntu 20.04 container.** Built natively it only
  ran on glibc 2.39 and newer, failing on GLIBC_2.32–2.38 and
  GLIBCXX_3.4.29–3.4.32. The container gives glibc 2.31 and GLIBCXX_3.4.28; the
  host's Qt 6.9.2 is bind mounted, since its libraries only ask for glibc 2.28
  and were never the problem. `build-appimage.sh` now takes `BUILD_DIR`,
  `APPDIR` and `OUTPUT` from the environment so a container build keeps its
  CMake cache apart from a native one.

### Settings

- **New key `gshhsmask`** (`[window]`), defaulting to `./gshhs2_3_7/gshhs_h.b`,
  with a *Land/sea mask* field in Preferences. The mask previously shared
  `gshhsglobe1` with the globe and map overlays, and the two want opposite
  things: the overlay is redrawn as vectors every frame so it wants few points,
  while the mask is rasterised once into a bitmap whose size does not depend on
  the polygon count. Moving to the high-resolution shoreline changes the
  land/sea answer for 0.10 % of the MTG-12 view — all of it on coasts and small
  islands, roughly 240 000 km² strung along every shore in sight. If the file is
  missing it falls back to `gshhsglobe1` rather than losing the mask.
- **The mask grid is now 0.01°** (about 1.1 km, against FCI's 1 km pixels), so a
  coastline lands within half a cell. This costs **77 MB** rather than 19 MB,
  held for the life of the process, and about 2.2 s to build once per run.

### Tests

`fcinav_test` pins the FCI grid constants against the values the files publish,
the north–south and east–west orientation of the grid, and the round trip
between the forward and inverse transforms. It fails on the old code.
