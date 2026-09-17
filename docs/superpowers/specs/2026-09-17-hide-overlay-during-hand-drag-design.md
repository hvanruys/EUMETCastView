# Hide the image overlay while the image is being dragged — design

Date: 2026-09-17
Status: approved

## Goal

Dragging the image in the image view with the left mouse button ("grab"
mode, the closed-hand cursor) is sluggish when the Overlay button is on,
because the overlay is recomputed and redrawn on every mouse move. While the
left button is held, the overlay is not drawn at all; the moment it is
released, the overlay is drawn once more at the new position. The drag then
costs no more than moving the pixmap.

This applies to every image type that has an overlay: the projection images
(LCC, GVP, SG, OM), the geostationary lon/lat grid with its header text, and
the OLCI and VII overlays. The user chose all of them over just the
projections.

## Today

`FormImage` is a `QGraphicsView` in `ScrollHandDrag` mode with
`FullViewportUpdate`. Every mouse move during a grab scrolls the view and
repaints the whole viewport. `QGraphicsView` calls `drawForeground()` on
every such repaint, and `FormImage::drawForeground()` calls `drawOverlays()`,
which for a projection image runs `OverlayProjection()`: for each vertex of
the three GSHHS overlay files it calls the projection's `map_forward()` and
draws a line. Nothing is cached between paints, so a drag of fifty mouse
moves projects the whole shoreline fifty times. The user noticed it on the
OM projection, but the LCC, GVP and SG projections and the geostationary,
OLCI and VII overlays are computed from scratch in the same way.

`savePNGImage()` calls `drawOverlays()` directly, not through
`drawForeground()`.

## Change

All in `FormImage` (`core/formimage.h`, `core/formimage.cpp`).

- A new `bool m_handScrolling`, false in the constructor.
- `mousePressEvent(QMouseEvent *)` override: if the button is the left one
  and `dragMode()` is `ScrollHandDrag`, set `m_handScrolling`. Then call the
  base class, which starts the drag and shows the closed hand as it does
  today. Other buttons, and a view still in `NoDrag` (before an image is
  shown, or after `resetView()`), do not set the flag.
- `mouseReleaseEvent(QMouseEvent *)` override: call the base class first, so
  the drag ends where Qt ends it. Then, if the button is the left one and
  `m_handScrolling` is set, clear it and call `viewport()->update()` — one
  repaint at the final position brings the overlay back. The flag is cleared
  on any left release, whatever the drag mode is by then, so an image
  recomposed mid-drag cannot leave it stuck.
- `drawForeground()`: after its existing early returns, return before
  `drawOverlays()` when `m_handScrolling` is set.

Qt's implicit mouse grab delivers the release to the view even when the
pointer has left it, so the flag is always cleared. The wheel zoom during a
held button repaints without the overlay, like the moves, and the release
restores it.

`savePNGImage()` is not gated by the flag; a save started while dragging
would be a save of the overlay-on image, as now.

## Not done

- Caching the projected overlay so it can stay visible during the drag.
  Rejected: every place a projection or geo image is recomposed would have
  to invalidate it, and a missed one shows a stale overlay silently.
- Any change to the Overlay buttons or to what the overlays draw.
- Throttling repaints during the drag by other means.

## Testing

The change is in mouse handling of a live `QGraphicsView`; it is verified in
the built application, on a projection image with Overlay on:

- Press the left button on the image: the overlay disappears; moving the
  mouse pans the image without the earlier lag.
- Release: the overlay is drawn again at the new position, once.
- Release with the pointer outside the view: same.
- Right or middle button: nothing changes from today.
- Overlay button off: dragging is unaffected, nothing appears on release.
- Save the image to PNG after a drag: the file has the overlay.
- The same on a geostationary image with its grid on.
