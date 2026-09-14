# Changelog

## 2.1.7

Three things. The CLAHE button on the geostationary tab now runs on every core
and keeps a fraction of the memory it used to, which is what it takes to
equalise a Meteosat-12 image — and on the way a drift in the kernel that has
skewed every image with an odd region size came out. The banding that METimage
leaves over sunglint, a sawtooth at the scan period in every VII image and
projection, can be taken out at read time. And the logging box in the
preferences stays ticked from one run to the next, which is how an AppImage
started from the desktop gets a `logging.txt` at all.

### Geostationary CLAHE

- **The button runs on every core and keeps a fiftieth of the memory.** Both
  phases of the Zuiderveld kernel are independent per contextual region and per
  interpolation block, so each is a `QtConcurrent::blockingMap` now; the Lab
  round trip around it is one task per scanline. On a 5568² FDHSI image the
  button went from 3.5 s to 0.26 s here, the kernel alone from 73 ms to 9 ms,
  with output byte for byte what it was. The three `double` planes of L, a and
  b are gone : only the 8-bit L plane lives between the passes, and a and b are
  recomputed from the untouched pixel on the way back. Temporaries drop from 26
  to 2 bytes per pixel — HRFI from 3.2 GB to 250 MB, the 0.5 km sharpened
  composite from 13 GB to 1 GB, which is what makes CLAHE on that one possible
  at all. Every path that calls `SegmentImage::CLAHE` — the polar images, the
  recipes — gets the parallel kernel with it.
- **The kernel skewed every image whose region size is odd.** The serial code
  walked a running pointer through the image and advanced it by one pixel less
  than the row width per block row whenever the region size was odd, so each
  row of blocks started one pixel further left than the one above. By the
  bottom of the image the interpolation grid was 16 pixels off, one column per
  block row was mapped twice and the last columns of a block row's final line
  were mapped with the weights of the row below. GOES (5424/16 = 339), MSG RSS
  (87), HRV RSS (145), OLCI (293), FY-2 (143) and VII (393) all had it. A block's
  origin is computed from its index now, and the last border block in each
  direction takes the remainder, so those images are equalised to their last
  column and row. Even sizes — every FCI, MSG full disc, Himawari image — come
  out unchanged.
- **The kernel has a pool of its own.** Qt 6's `blockingMap` does not run its
  engine on the calling thread — that was Qt 5 — so a kernel reached from a
  compose worker on the global pool could only make progress while that pool
  still had a thread to spare, and the XRIT compose does reach it from a
  `QtConcurrent::run` worker. With every global thread busy the compose would
  hang. On a private pool no CLAHE task ever waits on another, so nothing can.
- **The pixel work lives next to the kernel.** `SegmentImage::CLAHELab` is the
  RGB → L → CLAHE → RGB pipeline that `recalculateCLAHEMeteosat1` used to do
  inline, and that function is now the GUI wrapper it was around it : cursor,
  progress, the 10×10 regions Himawari-9's 5500 px disc needs, the globe
  texture. It refuses a null pointer, a null image or anything but a 32-bit
  format with -9 — the scanline arithmetic assumes four bytes per pixel — and
  leaves the image untouched on any error, after which the globe upload is
  skipped rather than fed a picture that did not change. The intermediate
  progress steps are gone : there is no hook for them and little left to report.
  Both the kernel and the pipeline are asserted byte-identical to verbatim
  copies of the code they replaced, on synthetic images, by a probe outside the
  repository.

### Metop-SG VII

- **The scan banding over sunglint can be taken out.** The 24 detectors of a
  METimage scan look along track under angles 0.83 degrees apart, and over
  glint the sea answers that with a brightness that climbs through the scan and
  drops back at the next one : a sawtooth at the scan period, up to 10 % of the
  signal, in every image and projection. It is not the bow-tie — the
  duplication mask removes that — and not a calibration difference between the
  detectors : it is a clean ramp, only over sea, only in the glint half of the
  swath, and it is in the L1B radiances as they arrive. Filtering it out of the
  finished picture does not work : a comb of notches in the 2D spectrum, at the
  period measured to the pixel and ten harmonics deep, left two thirds of it in
  place while moving cloud pixels by up to 56 levels and ringing at the swath
  border, because the seams tilt, the period jitters with the projection and
  the amplitude follows the scene. So it is measured and removed in sensor
  geometry, where the detector of every line is known exactly, before the
  channels are packed or combined. Per band of 64 columns the ramp is the median
  over the granule of the within-scan slope less the scene's own along-track
  gradient, the latter read off the neighbouring scan means and the
  detector-to-scan ground spacing from the geolocation, and a band keeps its
  ramp only when the estimate stands four standard errors from zero, which is
  what silences cloud texture. The correction scales with the square root of
  the pixel's brightness in the reddest channel of the image relative to the
  band's typical — the same over ordinary sea, half in a cloud shadow, and
  capped so a cloud, which has no glint, gets no more than the sea would. On two
  granules of Mediterranean glint the step at the seams went from -3.2 to +0.07
  levels in red and green and from -5.5 to -1.6 in blue, whose ramp is the same
  in reflectance but sits on the steep end of the gamma curve; clouds and land
  move by at most one level, and it costs 45 ms per channel per granule. The
  **Destripe scans** box next to Rayleigh correction on the VII tab, off by
  default, for the single bands and the RGB recipes alike.

### Logging

- **The logging box sticks.** Since 2.1.4 only `-l` on the command line opened
  `logging.txt`; the box in the preferences switched the file on and off for
  the run, and `/debugging/dologging` in the ini was overwritten at exit with
  whatever the run had done, so it never decided anything. An AppImage started
  from the file manager has no command line, and so no way to log at all. Now
  the box is the preference : `ViewLog::install()` reads the key from
  `EUMETCastView.ini` in the working directory — next to where the logfile goes,
  and before the `QApplication`, so what Qt says on the way up is kept as it is
  with `-l` — and a run without `-l` does what the box was left at. `-l` is for
  the run it is on : it opens the file without turning the box on for the next
  one, and only a click on the box changes what is saved, which is why the slot
  moved from `toggled` to `clicked` — the dialog sets the box to what the run is
  doing when it opens, and that must not count. Verified with the `viewlog.cpp`
  probe, four ways, and with the built application started headless from a
  directory holding only the ini.

## 2.1.6

Meteosat-12 sends two FCI products and EUMETCastView only ever read one of them.
HRFI is the other : four channels sampled one step finer than FDHSI, two of them
on the 0.5 km reference grid. It gets its own tab and its own band buttons, and
because its vis_06 is FDHSI's vis_06 at double sampling, it also sharpens an
FDHSI composite to 0.5 km.

### Meteosat-12 HRFI

- **The HRFI settings were FDHSI's, copied across.** HRFI samples one step finer
  - 0.5 km for vis_06 and nir_22, 1 km for ir_38 and ir_105, against FDHSI's
  1 km and 2 km - so every grid constant was half what it should be. The channel
  names were the FDHSI spellings too, and since the reader opens
  `/data/<name>/measured`, `vis_06` does not exist in an HRFI file and the
  compose returned before it had read anything. `spectrumlist` is the netCDF
  group names now, `imagewidth` is the infrared full disc and `imagewidthhrv0`
  the solar one, and `coff`/`cfac` with `coffhrv`/`cfachrv` are the 1 km and
  0.5 km pair. Clicking one of the four band buttons composes that band.
- **Two tables of sixteen FDHSI channel names are gone.** `fciGridSize()` reads
  the grid off the channel's own name : the `_hr` suffix is HRFI, and within
  either product `vis_` and `nir_` get the finer of its two grids. The
  `== 11136 ? coffhrv : coff` tests became `== imagewidthhrv0`, so the
  navigation constants follow the satellite rather than a literal.
- **Three things laid down with the groundwork did not work.** `newGeoTab` tested
  `!(a != X || a != Y)`, false whatever `a` is, so every geostationary tab was
  being built as a 41 column MTG tree. `getGeostationarySegmentsMTG` read the
  FDHSI map whichever tab asked it. And an HRFI file was matched with
  `baseName().contains()`, true on every iteration of the satellite loop, so it
  set the FDHSI list's image path as well as its own.
- **No colour composite on the HRFI tab.** Four bands on two grids, of which only
  two are solar, make no combination worth offering, so `btnGeoColor` is off
  there.

### Sharpening FDHSI with HRFI

- **An FDHSI composite can be sharpened to 0.5 km.** A checkbox beside the recipe
  list, off by default. `vis_06_hr` is `vis_06` : same central wavelength, same
  spectral width, and the same `scale_factor`, `add_offset` and solar irradiance
  in the files, differing only in sampling distance. So the ratio of the two is a
  pure resolution term with no spectral mismatch behind it - nothing to unmix, no
  colour shift to correct, and no strength to choose. True Color and Natural
  Colors come out at 22272 square instead of 11136, with the mean of the disc
  preserved to a fifth of a display level.
- **It is a pass over the finished composite, not a change inside it.** Every
  band stays on the grid it was composed on, so the recipe machinery, the solar
  correction and their memory cost are untouched, and any recipe drawn on the
  1 km solar grid is sharpened by the same code. Composing at 0.5 km instead
  would have cost about 22 GB for one image against the 2.7 GB this adds. It runs
  before `signalcomposefinished`, because anything listening to that takes a
  pointer to the image the sharpener is about to replace.
- **The box greys itself out when it cannot help.** Not the FDHSI tab, a slot
  with no HRFI chunks, or a recipe that is not drawn on the 1 km solar grid - a
  2 km one has nothing for a 0.5 km numerator to divide into, and an index is a
  datum rather than a brightness. The tooltip says which, and how many of the 40
  HRFI chunks arrived; what is missing simply stays at 1 km.
- **Night is left alone.** FCI puts zero radiance at count 204, and on the unlit
  half both bands hold that zero plus a count or two of noise, whose ratio is
  arbitrary and would paint speckle across the disc. Below a floor of 0.1 the
  pixel keeps the 1 km value it had, which is what that part of the disc already
  looked like. At the 05:00 terminator 234 of 370 million pixels take that path
  and the boundary is invisible.

### Fixed

- **The histogram bins overflowed.** `mtg_histogram` counted into a `quint16`,
  and one chunk of one band can put millions of pixels into a single bin. A
  0.5 km HRFI chunk holds 12.4 million pixels and bin 204 - dark ocean - takes
  4.18 million of them. EUMETCastVideo had it worse : on the night side a whole
  FDHSI solar channel collapses into three or four bins, and at 2026-09-02 00:00
  the wrap threw away 96 % of the counts, which skews the cumulative LUT and with
  it the 95 % stretch the video is made with. In a finished video that reads as
  the day side drifting in brightness across the sequence, worst at the
  terminator and gone by midday. Both are `quint32` now.
- **A reloaded list kept the satellites the reload had dropped.**
  `segmentlistmapgeomtgi1_hrfi` was never emptied in `ReadDirectories`, so a
  second load merged into the first : two loads of different days gave one tab
  holding both. And both `PopulateTree` helpers return early when their satellite
  has no segments, and returned before clearing, so the tab kept the previous
  load's list on screen - readable, selectable, and pointing at files the reload
  had decided were not part of the selection. Every tab is emptied before any is
  filled now.

## 2.1.5

Everything here is the last step of making a video, the point where the frames
are on disk and ffmpeg is handed them. Two separate things stopped a video being
made at all — one of them on every single run — and the third is the holes a day
of MTG arrives with, which cut the video short at the first of them.

### Making a video

- **ffmpeg was pointed at frames that were never written.** The movie form held
  two mappings from the satellite radio buttons to a short name and they
  disagreed. `prepareVideoRun()` answers `MET_12`, which goes into
  `EUMETCastVideo.json` as the shortname and comes back out of it as
  `videooutputname` `PROJ_MET_12_`, so the video processes write
  `tempvideo/PROJ_MET_12_0000.png` and up; `on_btnffmpeg_clicked()` answered
  `MET12` and built `tempvideo/PROJ_MET12_%04d.png` out of it. ffmpeg said
  "Could find no file with path ... and index in the range 0-4" and stopped
  there, on a directory holding 144 frames. It was not only the button:
  `deleteManager()` calls it as soon as a render comes back, so a full run
  rendered every frame and then ended without a video. `selectedSatellite()` is
  that mapping, once, and both callers ask it. The mp4 is
  `PROJ_MET_12_<date>.mp4` now rather than `PROJ_MET12_<date>.mp4`, which is
  what the frames and the json were already saying.
- **A hole in the frame numbering ended the video at the hole.** A frame that was
  never composed — a cycle that was not received, a chunk that would not read, a
  process that died — leaves a number ffmpeg does not find, and the image2
  demuxer stops at the first one: 144 frames on disk with 0020 missing encode as
  a 20 frame video. Every number missing between the lowest and the highest frame
  present is now written as a copy of the last real frame before it, in front of
  the listing the ffmpeg button takes, so what is counted is what ffmpeg gets. A
  run of holes all copy that one frame rather than a copy of a copy. Only between
  them: image2 looks for its own start index, and a run that stopped early is a
  shorter video, not a broken one, so padding its tail with thirty copies of one
  frame would be worse than leaving it. Every frame filled in is named in the
  traffic list, and the frame carries the date overlay of the cycle it was copied
  from, so the clock stands still for it rather than advancing.
- **A chunk that was not received moved everything under it.** It does not leave
  a band out of the image: `compileImageMTG` stacks the chunks that did come in
  and adds up their rows, so everything below the hole moves up and is navigated
  as the wrong latitude. Chunk *n* covers a fixed band of the disc, so the same
  chunk of a neighbouring cycle puts those rows back, ten minutes stale, and the
  geometry is right again. This is done where `EUMETCastVideo.json` is written,
  so what the video processes are handed is already whole: every cycle from the
  first of the selection to the last gets an entry, and every entry every chunk
  the projection asks for, taken from the nearest cycle that has it — the one
  before by preference, the one after only for a hole in the first cycle.
  Sources are looked for in the selection as it was received and never in what
  was just filled in, so a run of holes all name the same real file. A cycle
  received not at all becomes an entry holding nothing: it is there for the frame
  number it takes, so the frames keep step with the clock, and the frame itself
  is filled in after the render.
- **A cycle with no files composed nothing, in silence.** `compileImageMTG`
  returned on an empty file list without a word, which is what an empty entry
  now hands it. It says which timestamp and why.
- **The commented out XML writer is gone.** `on_btnCreateXML_clicked()` wrote the
  `EUMETCastVideo.xml` that `CreateVideoJson` replaced, and had been left in
  place as 439 lines of comment in front of everything the form does.
  `XmlVideoReader` is untouched and still built into EUMETCastVideo, so a hand
  written xml keeps working.

## 2.1.4

Making a video worked on Linux and did nothing at all on Windows. It comes down
to one relative path, and a great deal of silence around it — and behind that, an
EUMETCastVideo that started and then died halfway through an MTG image, on a
stack it never initialised and a row it read past the end of. Both executables
write a logfile of their own now, which is what the silence was. Alongside that:
Kill and Test on the movie form, the toolbar says which view is on screen, a
composed image appears without a second click, and the qmake project files are
gone.

### Video tool

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
- **EUMETCastVideo died halfway through an MTG image, and differently on every
  run.** A `VideoMaker` is a megabyte and a half of object that `main()` put on
  the stack, and nothing initialised it. The MTG path then read a set of its
  members before anything wrote them: `total_rows` was summed with `+=` from
  whatever it started as, the loop over all 40 chunks read
  `mtg_start_position_row` for the ones that are not in the selection,
  `InitializeImageGeostationary` deleted `ptrimageGeostationary` before it
  assigned it, and `histogrammethod` was never set anywhere at all. Which stack
  the object landed on decided what all of that meant, and that depends on the
  environment block, the path and the arguments — so it came out right for one
  image and not for the next, and Windows lost the toss more often. They are
  initialised in the header now, and the object is built on the heap: 1.25 MB of
  its 1351600 bytes is `mtg_histogram`, and a MinGW executable reserves 2 MB of
  stack against Linux' 8 MB, so the object and the frames underneath it ran at
  about 1.5 MB of the 2 MB before `compileImage` did anything. Clamped with
  `ulimit -s`, the image was composed at 1600K and segfaulted at 1500K before;
  it is composed at 250K and segfaults at 200K now.
- **Nothing that read a chunk from netCDF checked its return.** A file that would
  not open left the `ncfileid` unset, and the four position values, the row
  counts, the size of the buffer and the height of the image were then built out
  of the stack in the same way. Every read is checked now, a chunk that does not
  come in complete is dropped with a message over the udp socket — a zero start
  position is what every loop below already reads as a chunk that is not there —
  and an image with no chunk left writes nothing instead of composing from an
  empty list. The image composed into is checked for having been allocated as
  well: `QImage` reports an allocation it could not make by staying null rather
  than by throwing, and every `scanLine()` taken off that is a read of address
  zero.
- **The night row ran off the end of the night image.** `CalculateImageMTG`
  takes the night pixel from `scanLine(line/2)` of an image half the height of
  the day image, and the day image does not always hold an even number of rows:
  the MTG chunks are 279 and 278 rows about turn, so the six of them in a
  selection come to 1671 rows, against 835 for the infrared at half the
  resolution. The first line composed is 1670, `line/2` is 835, and an image of
  835 rows ends at 834 — `scanLine(835)` is 22 kB past the end of the
  allocation. Linux hands back heap and composes the image; Windows answers with
  0xC0000005 on the first line of the first chunk, which is where the log
  stopped. The last day line takes the last night row now, which is what half of
  it means anyway, and the night image is checked for having been allocated,
  since every pixel written reads a row out of it.

### Logging

- **EUMETCastVideo writes `templogs/EUMETCastVideo_<image number>.log`.** It is a
  console application, but QProcess starts it with `CREATE_NO_WINDOW` when the
  parent has no console of its own, so on Windows everything it said through
  `qDebug()` was thrown away — and what it says is the only account there is of
  what it read and where it stopped. One file per process, so the ones running
  next to each other do not write over one another, and every line is flushed as
  it is written: the point of the file is that the last line in it is where the
  process was standing when it went. It opens with the arguments, the working
  and application directories, and the versions of Qt, netCDF and HDF5 that were
  actually loaded, with `PATH` and `HDF5_PLUGIN_PATH` behind them — the same
  executable composes an image under QtCreator and crashes when it is started
  from a bat file, and that is what a different set of dll's on `PATH` looks
  like. `compileImageMTG` says which phase it is in as well — the chunks that
  came in, the min/max, the histograms, the compose, the projection, the save —
  so a log that stops says where.
- **EUMETCastView writes `logging.txt`.** `-l` opened the file and wrote nothing
  into it: the handler that would have filled it is commented out, and the
  `myMessageOutput` standing next to it was never installed either. `ViewLog` is
  for EUMETCastView what the above is for EUMETCastVideo — installed before the
  `QApplication`, so that what Qt itself says on the way up is kept, flushed line
  by line, and chained onto the handler that was there, so stderr and the abort
  at the end of a `qFatal` keep doing what they did. Only `-l` or `--logging`
  turns it on; `/debugging/dologging` in the ini does not, and `opts.doLogging`
  follows what was actually done, so the box in the preferences shows the run
  rather than the setting. That box switches the file on and off while the
  application runs now, and switching it back on appends instead of emptying what
  was logged before it. It was a lone auto-exclusive radio button, which can be
  switched on and never off again, so it is the checkbox it always read as.
- **A missing FCIDECOMP filter is named.** Filter 32018 unpacks the JPEG-LS the
  FCI radiances are stored in. EUMETCastView does not need it — `mainwindow.cpp`
  asks `H5Zfilter_avail` for it, and when the answer is no,
  `segmentlistgeostationary` takes the chunk raw and decodes it itself.
  EUMETCastVideo has no such path: it reads `effective_radiance` through
  `nc_get_var_ushort`, which wants the filter. So an `HDF5_PLUGIN_PATH` that does
  not hold the plugin broke the video and left the program it was started from
  composing the same files perfectly, which does not read as a missing plugin at
  all. It is asked for once now, at the start of `compileImageMTG`, and named
  either way with the path it was looked for on.

### Interface

- **Kill and Test on the movie form.** `btnKillVideo` gives the run up:
  `ProcessManager::stopAll()` clears the task queue, so nothing new is spawned,
  and kills every `EUMETCastVideo` it started. The kills come back as
  `finished()`, so the processes retire along the path a run that ends by itself
  takes, and `deleteManager` reads `wasAborted()` to leave ffmpeg alone — an
  aborted run leaves part of the frames in `tempvideo/`, which is not a set of
  images to make a video out of. `btnRunTest` renders a single image, to see what
  the settings on the form give before spending a full run on them: it is queued
  under its own number, so the file it writes is the one a full run would have
  written for that slot, and a single bad frame can be rendered again without
  redoing the others. `processmanager` was never initialised, so clicking Kill
  before a render had run would have used whatever was on the stack; it is
  `nullptr` now, and both buttons refuse to start a second manager over a live
  one. `spbTestImageNbr` counts from 0 and up to 9999 — the QSpinBox default of
  99 puts frames 100 to 143 of an MTG day out of reach.
- **The 3D globe text follows the preferences.** The globe wrote everything in
  `QFont("Times", 12, Bold)`, built in `paintGL` and set on the painter the whole
  overlay is drawn with. The frame rate, the last selected segment, the
  satellite, station and segment names and the view distance now come from
  `fontfamily3D` and `fontsize3D`, so they follow `cmbFont3DGlobe` and
  `spbSize3DFont` on the 3D page of the preferences, and `paintGL` ends in
  `update()`, so a change shows on the next frame. `drawInstructions` measured
  its text rect with the widget font while drawing it with the painter font; the
  two were both 12pt-ish, so it did not show, and with a size to choose it would
  — it measures the font it writes in now. `spbSize3DFont` had no range of its
  own and so could write a point size of 0 to the ini, which draws nothing and
  warns once a frame; it goes from 6 to 72.
- **The toolbar says which view is showing.** `on_actionImage_triggered` already
  unchecked the other view actions, but none of them were checkable in the first
  place, so nothing marked the view on screen. All seven are checkable now, every
  handler checks its own action and clears the rest, and the constructor sets the
  state the application starts in.
- **A composed image appears without a second click.** The update buttons for
  VII, the VII recipes, VIIRS M and OLCI EFR/ERR composed the image and left the
  viewer on whatever it was showing before. `setPixmapToScene` ends by calling
  `displayImage(channelshown, true)`, so the scene is filled from one place, and
  selecting the METimage tab displays the VII image as the other sensor tabs
  already did for theirs. Its own switch, which repeated `displayImage`'s case
  for case, is down to what `displayImage` does not do: the Oblique Mercator
  image size, the cleared info panel for a projection, and the early return on
  `IMAGE_NONE`. The two disagreed here and there — the OLCI info was written for
  `SEG_OLCIEFR` in one and for `imageptrs->olcitype` in the other — and
  `displayImage`, running last, already had the last word.
- **A VII colour combination using channels 17 to 20 was refused.**
  `comboColVIIOK` summed the first sixteen channel combo boxes only, so a
  selection made in the last four counted as nothing selected. All twenty count.
- **The VII image info is written whatever was displayed before it.** The
  `segmenttype` test in front of it was copied from the VIIRS branches, where it
  dispatches between three satellites; VII flies on one, so the test could only
  ever skip the call — and `ShowVIIImage` calls `displayImage` before
  `setSegmentType`, so it did skip it on the first VII image after another
  sensor. `displayVIIImageInfo` never read the segment type it was handed, and
  no longer takes one.

### Build

- **The qmake project files are gone**, CMake is the build on both platforms.
  They had drifted out of use and out of date: `EUMETCastView.pro` named a
  `PublicDecompWT-2.8.1` subproject that has no project file of its own, so it
  could not be read at all, and did not list `video`; `video/video.pro` listed
  neither `videomaker.cpp` nor `jsonvideoreader.cpp`, and so not
  `compileImageMTG`, and named a `geoseglist.cpp` that no longer exists.
- Two `CMakeLists.txt.user` files are out of the repository as well. They were
  committed before `.gitignore` learned about them, and `.gitignore` has no say
  over a file git is already tracking.

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
