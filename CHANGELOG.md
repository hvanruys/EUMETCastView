# Changelog

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
