# Preferences dialog opens at its minimum, top-left — design

Date: 2026-09-14
Status: approved

## Goal

The Preferences dialog opens as small as its layout allows, with its top-left
corner at the top-left corner of the main window, so it covers as little of
the image as possible while a setting is being tried. It can still be dragged
larger; nothing about its size or place is remembered between runs — every
open is the minimum.

## Today

`Ui::DialogPreferences::setupUi` ends with `resize(1434, 1230)`, the geometry
saved in `dialogpreferences.ui`, and nothing after it changes the size. A
`QDialog` with a parent centres itself on that parent in `setVisible()` unless
`Qt::WA_Moved` is set. So the dialog opens 1434×1230 in the middle of the
window.

The layout's minimum, measured by loading the `.ui` with `QUiLoader` at 12 pt
Medium (what `main.cpp` sets), is 1088×909. A `QStackedWidget` is as big as
its largest page, and the 3D page needs 759×851, the MERSI page 804 wide; the
2D page needs only 361×88. The user chose one fixed minimum for every page
over a dialog that resizes to the page shown.

## Change

At the end of the `DialogPreferences` constructor, after
`opts.globalChangeFonts(this, opts.fontsize)` — the hints are only right once
the fonts are the ones the dialog will draw with:

- `resize(minimumSizeHint())` — opens at the layout's minimum. Qt already
  sets a top-level's minimum size from its layout, so the window could never
  be dragged below this; it now opens *at* it instead of above it.
- `move(parent->mapToGlobal(QPoint(0, 0)))` when there is a parent widget —
  the dialog's frame goes to the top-left of the main window's client area,
  its title bar just under the main window's, over the menu bar. `move()`
  before `show()` sets `WA_Moved`, which is what stops `QDialog` centring it.

`mainwindow.cpp` and the `.ui` file do not change; Designer keeps showing the
dialog at a comfortable size.

## Not done

- Sizing to the current page (the user chose against it).
- Saving a dragged size or position to the ini.
- Following the main window when it is moved afterwards — the dialog stays
  where it opened, as it does today.

## Testing

- The `.ui` probe (`QUiLoader`, offscreen) gives the expected minimum,
  1088×909 at 12 pt.
- The built application, opened on the real display: the dialog's geometry
  read with `xdotool`/`xwininfo` matches the main window's client-area origin
  and the minimum size hint.
