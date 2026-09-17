# Hide the image overlay while the image is being dragged — implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** While the left mouse button hand-drags the image in `FormImage`, the overlay is not drawn; on release it is drawn once more, so the drag no longer recomputes the overlay on every mouse move.

**Architecture:** `FormImage` (a `QGraphicsView` in `ScrollHandDrag` mode) gets one `bool m_handScrolling`, set by a `mousePressEvent` override on a left press while the drag mode is `ScrollHandDrag`, cleared by a `mouseReleaseEvent` override on any left release, which then calls `viewport()->update()`. `drawForeground` returns before `drawOverlays()` while the flag is set. Nothing else changes; `savePNGImage`, which calls `drawOverlays` directly, is not gated. A no-GUI probe drives a real `FormImage` with synthetic mouse events and counts what `drawForeground` paints.

**Tech Stack:** C++20, Qt 6.9 (`QGraphicsView`, `QMouseEvent`), CMake + Ninja debug tree at `build/Desktop_Qt_6_9_2-Debug`, probe harness at `/home/hugo/EUMETCastTools/viiprobes/`.

Spec: `docs/superpowers/specs/2026-09-17-hide-overlay-during-hand-drag-design.md`.

---

## File structure

| File | Responsibility |
|---|---|
| `core/formimage.h` | declare `m_handScrolling` and the two mouse overrides |
| `core/formimage.cpp` | initialise the flag; the two overrides after `wheelEvent`; the early return in `drawForeground` |
| `/home/hugo/EUMETCastTools/viiprobes/dragprobe.cpp` | new: a `FormImage` subclass that exposes `drawForeground` as a pixel count, synthetic press/move/release, the assertions |
| `/home/hugo/EUMETCastTools/viiprobes/build.sh` | add `dragprobe` to the loop |
| `/home/hugo/EUMETCastTools/viiprobes/README.md` | one section for `dragprobe` |

The probe directory is **not** a git repository; nothing there is committed. The user keeps the probes.

## Ground rules for every task

- The application build tree is the Ninja debug one: `cmake --build /home/hugo/EUMETCastTools/EUMETCastView/build/Desktop_Qt_6_9_2-Debug`. It builds both `EUMETCastView` and `EUMETCastVideo` and links into the shared `bin/`.
- **Rebuild the application before rebuilding the probe**, every time. The probe links the application's `.o` files; stale ones link fine and then test code that no longer exists.
- The probe **must run with the working directory set to `EUMETCastView/bin`** and `QT_QPA_PLATFORM=offscreen`: `opts.Initialize()` reads `./EUMETCastView.ini`, and `FormImage`'s constructor reads `opts.geosatellites` from it.
- The working tree has uncommitted changes in `core/formtoolbox.cpp` and `core/formtoolbox.ui` that belong to the user. **Never `git add -A`**; add only the files each task names.
- `FormImage` is the class the whole image view is; the change is a dozen lines in it and the plan does not restructure it.

---

### Task 1: The probe, failing against today's code

The probe pins the behaviour the spec asks for. Against the current code the "left button held: overlay not drawn" cases fail, because `drawForeground` draws the overlay whatever the mouse is doing.

How it observes the overlay: `drawForeground` is protected, so a subclass paints it into a transparent `QImage` the size of the projection and counts the non-transparent pixels. `OverlayProjection` always draws the observer cross at `opts.obslat/obslon`, which the probe puts at the projection centre, so the count is positive even when no shoreline file is found. A stereographic projection is used because `StereoGraphic::Initialize` is self-contained and allocates `imageptrs->ptrimageProjection` at the size it is given; `displayImage(IMAGE_PROJECTION, true)` then sets `m_image`, `m_ViewInitialized` and `ScrollHandDrag` exactly as the GUI does, and touches no `FormToolbox`.

How it drives the mouse: `QMouseEvent`s sent with `QApplication::sendEvent` to the **viewport** widget. `QAbstractScrollArea`'s viewport filter routes them to the view's `mousePressEvent` / `mouseMoveEvent` / `mouseReleaseEvent`, the same path a real click takes. No `QtTest` link is needed.

**Files:**
- Create: `/home/hugo/EUMETCastTools/viiprobes/dragprobe.cpp`
- Modify: `/home/hugo/EUMETCastTools/viiprobes/build.sh:44,50`

- [ ] **Step 1: Write the probe**

`/home/hugo/EUMETCastTools/viiprobes/dragprobe.cpp`:

```cpp
// Probe: FormImage leaves its overlay out while the image is hand-dragged
// with the left mouse button, and draws it again on release. Links the
// application's own objects, so the mousePressEvent / mouseReleaseEvent /
// drawForeground under test are exactly what the image view runs.
//
//   cd EUMETCastView/bin
//   QT_QPA_PLATFORM=offscreen /home/hugo/EUMETCastTools/viiprobes/dragprobe
//
// Exit status 0 when every case holds.

#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QMutex>

#include "options.h"
#include "segmentimage.h"
#include "gshhsdata.h"
#include "satellite.h"
#include "poi.h"
#include "avhrrsatellite.h"
#include "stereographic.h"
#include "formimage.h"

QMutex g_mutex;
Options opts;
Poi poi;
SegmentImage *imageptrs;
gshhsData *gshhsdata;
QFile loggingFile;
QTextStream outlogging(&loggingFile);
QNetworkAccessManager networkaccessmanager;
SatelliteList satellitelist;
bool ptrimagebusy;

// The view under test. drawForeground is protected; this paints it into an
// image of the probe's own and counts what came out.
class DragProbe : public FormImage
{
public:
    explicit DragProbe(AVHRRSatellite *segs) : FormImage(nullptr, segs) {}

    // Pixels drawForeground paints into a transparent image the size of the
    // projection. 0 means no overlay.
    int overlayPixels()
    {
        QImage img(imageptrs->ptrimageProjection->size(), QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QPainter p(&img);
        drawForeground(&p, img.rect());
        p.end();
        int n = 0;
        for(int y = 0; y < img.height(); y++)
            for(int x = 0; x < img.width(); x++)
                if(qAlpha(img.pixel(x, y)) != 0)
                    n++;
        return n;
    }
};

// Counts the paint events the viewport gets, to see that a release
// repaints it.
class PaintCounter : public QObject
{
public:
    int paints = 0;
protected:
    bool eventFilter(QObject *, QEvent *e) override
    {
        if(e->type() == QEvent::Paint)
            paints++;
        return false;
    }
};

// A mouse event delivered to w the way the window system would deliver it.
static void mouse(QWidget *w, QEvent::Type type, Qt::MouseButton button, Qt::MouseButtons held, const QPoint &pos)
{
    QMouseEvent ev(type, pos, w->mapToGlobal(pos), button, held, Qt::NoModifier);
    QApplication::sendEvent(w, &ev);
}

static int failures = 0;
static void check(bool ok, const char *what)
{
    qInfo() << (ok ? "ok  " : "FAIL") << what;
    if(!ok)
        failures++;
}

int main(int argc, char *argv[])
{
    ptrimagebusy = false;
    qputenv("HDF5_DISABLE_VERSION_CHECK", QByteArray("1"));
    QApplication app(argc, argv);

    opts.Initialize();
    poi.Initialize();
    satellitelist.Initialize();

    imageptrs = new SegmentImage();
    gshhsdata = new gshhsData();

    AVHRRSatellite *segs = new AVHRRSatellite();

    // A stereographic projection centred on 50N 0E, 800 x 600: Initialize
    // allocates ptrimageProjection at that size. The observer cross that
    // OverlayProjection draws goes to the centre, so something is drawn
    // even if no shoreline file was found.
    imageptrs->sg = new StereoGraphic(nullptr, segs);
    imageptrs->sg->Initialize(0.0, 50.0, 1.0, 800, 600, 0, 0);
    opts.currenttoolbox = 2;    // SG
    opts.obslat = 50.0;
    opts.obslon = 0.0;

    DragProbe view(segs);
    view.resize(400, 300);
    view.show();
    view.displayImage(IMAGE_PROJECTION, true);   // m_image, m_ViewInitialized, ScrollHandDrag
    QApplication::processEvents();

    QWidget *vp = view.viewport();
    PaintCounter counter;
    vp->installEventFilter(&counter);
    const QPoint a(100, 100), b(140, 120);

    const int baseline = view.overlayPixels();
    qInfo() << "overlay pixels with no button held :" << baseline;
    check(baseline > 0, "overlay is drawn before any mouse button");
    check(view.dragMode() == QGraphicsView::ScrollHandDrag, "view is in ScrollHandDrag after displayImage");

    // The case this probe exists for.
    mouse(vp, QEvent::MouseButtonPress, Qt::LeftButton, Qt::LeftButton, a);
    check(view.overlayPixels() == 0, "left button held: overlay not drawn");
    mouse(vp, QEvent::MouseMove, Qt::NoButton, Qt::LeftButton, b);
    check(view.overlayPixels() == 0, "left button held, after a move: overlay still not drawn");

    QApplication::processEvents();      // flush the repaints the move itself caused
    counter.paints = 0;
    mouse(vp, QEvent::MouseButtonRelease, Qt::LeftButton, Qt::NoButton, b);
    check(view.overlayPixels() == baseline, "left button released: overlay drawn again");
    QApplication::processEvents();
    check(counter.paints >= 1, "left button released: viewport repainted");

    // Other buttons leave it alone.
    mouse(vp, QEvent::MouseButtonPress, Qt::RightButton, Qt::RightButton, a);
    check(view.overlayPixels() == baseline, "right button held: overlay drawn");
    mouse(vp, QEvent::MouseButtonRelease, Qt::RightButton, Qt::NoButton, a);
    mouse(vp, QEvent::MouseButtonPress, Qt::MiddleButton, Qt::MiddleButton, a);
    check(view.overlayPixels() == baseline, "middle button held: overlay drawn");
    mouse(vp, QEvent::MouseButtonRelease, Qt::MiddleButton, Qt::NoButton, a);

    // NoDrag: a left press starts no drag, so there is nothing to hide.
    view.setDragMode(QGraphicsView::NoDrag);
    mouse(vp, QEvent::MouseButtonPress, Qt::LeftButton, Qt::LeftButton, a);
    check(view.overlayPixels() == baseline, "NoDrag, left button held: overlay drawn");
    mouse(vp, QEvent::MouseButtonRelease, Qt::LeftButton, Qt::NoButton, a);
    view.setDragMode(QGraphicsView::ScrollHandDrag);

    // Drag mode dropped mid-drag (an image recomposed under the mouse): the
    // release still brings the overlay back.
    mouse(vp, QEvent::MouseButtonPress, Qt::LeftButton, Qt::LeftButton, a);
    check(view.overlayPixels() == 0, "left button held (second drag): overlay not drawn");
    view.setDragMode(QGraphicsView::NoDrag);
    mouse(vp, QEvent::MouseButtonRelease, Qt::LeftButton, Qt::NoButton, b);
    check(view.overlayPixels() == baseline, "left button released after the drag mode changed: overlay drawn again");

    qInfo() << (failures ? "FAILED" : "PASSED") << "-" << failures << "failure(s)";
    return failures ? 1 : 0;
}
```

- [ ] **Step 2: Add the probe to `build.sh`**

In `/home/hugo/EUMETCastTools/viiprobes/build.sh`, the loop and the closing echo:

```bash
for probe in viitex_probe viitex_probe2 claheprobe viiom_probe dragprobe; do
```

```bash
echo "done: $HERE/viitex_probe $HERE/viitex_probe2 $HERE/claheprobe $HERE/viiom_probe $HERE/dragprobe"
```

- [ ] **Step 3: Build the application (so the objects are current), then the probe**

Run:
```bash
cmake --build /home/hugo/EUMETCastTools/EUMETCastView/build/Desktop_Qt_6_9_2-Debug 2>&1 | tail -3
/home/hugo/EUMETCastTools/viiprobes/build.sh 2>&1 | tail -4
```
Expected: the application build ends with `Linking CXX executable ../bin/EUMETCastView` (or `ninja: no work to do.`), and `build.sh` prints `compiling dragprobe`, `linking   dragprobe`, then the `done:` line. No compile errors: everything the probe uses (`FormImage::displayImage`, `dragMode()`, `viewport()`, `drawForeground` from a subclass) already exists.

- [ ] **Step 4: Run it — it must fail on the "not drawn" cases**

Run:
```bash
cd /home/hugo/EUMETCastTools/EUMETCastView/bin && QT_QPA_PLATFORM=offscreen /home/hugo/EUMETCastTools/viiprobes/dragprobe 2>&1 | grep -E '^(ok|FAIL|"?PASSED|"?FAILED|overlay pixels)'
```
Expected: `overlay pixels with no button held :` a positive number; `ok` for "overlay is drawn before any mouse button", "view is in ScrollHandDrag", "overlay drawn again", "right", "middle", "NoDrag"; **`FAIL`** for the three lines "left button held: overlay not drawn", "left button held, after a move: overlay still not drawn" and "left button held (second drag): overlay not drawn"; the last line `FAILED - 3 failure(s)` (4 if "viewport repainted" also fails — today nothing asks for a repaint on release, so that one is allowed to fail here too).

If instead "overlay is drawn before any mouse button" fails with `overlay pixels ... : 0`, the probe was not run from `bin/`: `opts.Initialize()` found no INI, and `m_image` or the projection is empty. Fix the working directory, not the probe.

No commit: the probe directory is not under git.

---

### Task 2: The flag, the two overrides and the gate in `drawForeground`

**Files:**
- Modify: `core/formimage.h:119-121` (private members) and `core/formimage.h:168-171` (protected overrides)
- Modify: `core/formimage.cpp:14` (include), `core/formimage.cpp:29-30` (constructor), after `core/formimage.cpp:321-351` (`wheelEvent`), `core/formimage.cpp:1513-1527` (`drawForeground`)

- [ ] **Step 1: Declare the flag and the overrides**

In `core/formimage.h`, the private members read today:

```cpp
    int m_rotateAngle;
    bool m_ViewInitialized;
    QString m_fileName;
```

Make them:

```cpp
    int m_rotateAngle;
    bool m_ViewInitialized;
    bool m_handScrolling;     // left button down in ScrollHandDrag: no overlay until it is released
    QString m_fileName;
```

and the protected block, today:

```cpp
protected:
    virtual void wheelEvent(QWheelEvent * event);
    virtual void resizeEvent(QResizeEvent * event);
    virtual void drawForeground(QPainter *painter, const QRectF &rect);
```

becomes:

```cpp
protected:
    virtual void wheelEvent(QWheelEvent * event);
    virtual void resizeEvent(QResizeEvent * event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void drawForeground(QPainter *painter, const QRectF &rect);
```

- [ ] **Step 2: Initialise the flag**

In `core/formimage.cpp`, the constructor's initialiser list, today:

```cpp
FormImage::FormImage(QWidget *parent, AVHRRSatellite *seglist) :
    QGraphicsView(parent), m_rotateAngle(0), m_ViewInitialized(false)
```

becomes:

```cpp
FormImage::FormImage(QWidget *parent, AVHRRSatellite *seglist) :
    QGraphicsView(parent), m_rotateAngle(0), m_ViewInitialized(false), m_handScrolling(false)
```

(`m_handScrolling` is declared right after `m_ViewInitialized`, so the list stays in declaration order.)

- [ ] **Step 3: Add the include and the two overrides**

In `core/formimage.cpp`, after `#include <QWheelEvent>` add:

```cpp
#include <QMouseEvent>
```

Then, directly after the closing brace of `FormImage::wheelEvent` (before `void FormImage::zoomIn()`), add:

```cpp
// The overlay is recomputed on every repaint, and a hand drag repaints on
// every mouse move, so it is left out while the left button is down and
// drawn once more when the button goes up. Only a left press on a
// ScrollHandDrag view starts a drag, so only that sets the flag; any left
// release clears it, whatever the drag mode has become in the meantime.
void FormImage::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && dragMode() == ScrollHandDrag)
        m_handScrolling = true;

    QGraphicsView::mousePressEvent(event);
}

void FormImage::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);

    if(event->button() == Qt::LeftButton && m_handScrolling)
    {
        m_handScrolling = false;
        viewport()->update();
    }
}
```

- [ ] **Step 4: Gate `drawForeground`**

In `core/formimage.cpp`, `FormImage::drawForeground` today:

```cpp
    if(m_image->isNull())
        return;

    drawOverlays(painter);
```

becomes:

```cpp
    if(m_image->isNull())
        return;

    if(m_handScrolling)
        return;

    drawOverlays(painter);
```

`savePNGImage` calls `drawOverlays` directly and is deliberately left alone.

- [ ] **Step 5: Build the application, then the probe**

Run:
```bash
cmake --build /home/hugo/EUMETCastTools/EUMETCastView/build/Desktop_Qt_6_9_2-Debug 2>&1 | grep -E 'error|Linking' | head
/home/hugo/EUMETCastTools/viiprobes/build.sh 2>&1 | tail -2
```
Expected: no `error` lines; both `Linking CXX executable ../bin/EUMETCastView` and `../bin/EUMETCastVideo` (the video tool does not compile `formimage.cpp`, so it only relinks if anything shared changed); `build.sh` ends with its `done:` line.

- [ ] **Step 6: Run the probe — every case passes**

Run:
```bash
cd /home/hugo/EUMETCastTools/EUMETCastView/bin && QT_QPA_PLATFORM=offscreen /home/hugo/EUMETCastTools/viiprobes/dragprobe 2>&1 | grep -E '^(ok|FAIL|"?PASSED|"?FAILED|overlay pixels)'
```
Expected: every line `ok`, the last line `PASSED - 0 failure(s)`, exit status 0.

If everything passes except "left button released: viewport repainted", the offscreen platform delivered no paint event for the `update()`; confirm the repaint in the application in Task 3 (the overlay coming back on release *is* that repaint) before deciding the check is wrong. Do not weaken the check without that.

- [ ] **Step 7: Commit**

```bash
cd /home/hugo/EUMETCastTools/EUMETCastView
git add core/formimage.h core/formimage.cpp
git commit -m "Leave the image overlay out while the image is being dragged

Every mouse move of a hand drag repaints the viewport, and drawForeground
recomputed the whole overlay on each of them - for a projection, one
map_forward per shoreline vertex. A left press in ScrollHandDrag now sets
m_handScrolling, drawForeground returns before drawOverlays while it is
set, and the release clears it and updates the viewport once, so the
overlay comes back at the final position.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01Wg9zd2gecWmso17m6MxJMe"
```

---

### Task 3: In-application check and the probe README

The probe covers the flag and the gate; the feel of the drag and the overlay coming back on the screen are only visible in the running application, on a real display.

**Files:**
- Modify: `/home/hugo/EUMETCastTools/viiprobes/README.md` (append a section after the `viiom_probe` one)

- [ ] **Step 1: Run the application from the debug build**

The debug tree linked last in Task 2, so `bin/EUMETCastView` is that binary. It runs from `bin/`, where the INI is:

```bash
cd /home/hugo/EUMETCastTools/EUMETCastView/bin && ./EUMETCastView
```

(This needs the real display; if the session cannot open one, hand the checklist below to the user and say so.)

- [ ] **Step 2: Walk the spec's checklist**

On a projection image (OM, since that is where it was noticed; any of the four will do) with the Overlay button reading "Overlay On":

1. Press the left button on the image: the shoreline overlay disappears at once; moving the mouse pans the image without the lag there was before.
2. Release: the overlay is back, at the new position.
3. Press, drag the pointer out of the image view, release outside: the overlay is back.
4. Right and middle button presses: the overlay stays.
5. Click Overlay to "Overlay Off": dragging pans as before and nothing appears on release.
6. With Overlay On, after a drag, save the image as PNG (the toolbox save button): the file has the overlay drawn.
7. Switch to a geostationary image with its grid on (Overlay On on the Meteosat tab): the grid and header text go while the left button is held and return on release.

Any item that does not behave this way is a defect in Task 2, not in the checklist — go back, do not commit around it.

- [ ] **Step 3: Document the probe**

Append to `/home/hugo/EUMETCastTools/viiprobes/README.md`, after the `viiom_probe` section:

```markdown
### `dragprobe` - the overlay during a hand drag of the image

    cd /home/hugo/EUMETCastTools/EUMETCastView/bin
    QT_QPA_PLATFORM=offscreen /home/hugo/EUMETCastTools/viiprobes/dragprobe

No arguments. Builds a `FormImage` on a stereographic projection (nothing to
compose: `StereoGraphic::Initialize` allocates the projection image, and the
observer cross `OverlayProjection` draws at `opts.obslat/obslon` is enough
for something to be on it), then sends it press / move / release
`QMouseEvent`s through the viewport - the path a real click takes - and
after each one paints `drawForeground` into a transparent image and counts
the pixels. Asserts that the count is 0 while the left button is down in
`ScrollHandDrag`, back to its baseline on release, and untouched by the
right and middle buttons or by `NoDrag`. Exit status 0 when every case
holds, one `ok`/`FAIL` line per case.

Must run from `bin/`, like the others: the `FormImage` constructor reads
`opts.geosatellites`, which comes from the INI.
```

No commit: the probe directory is not under git. The plan file's checkboxes are the only thing left in the repository to update; tick them and commit the plan with `git add docs/superpowers/plans/2026-09-17-hide-overlay-during-hand-drag.md`.
