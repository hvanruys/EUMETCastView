# Hiding the map/globe side panel from the main toolbar — design

Date: 2026-09-14
Status: approved

## Goal

A button in `mainToolBar` that hides and shows the column of buttons to the
right of the cylindrical map and the globe (`verticalLayout_6` in
`formmapcyl.ui`: the sensor tab widget and the Clear Texture … Show All
Segments buttons), so the map or globe can have the whole width. The toolbox
button that already exists in `toolBar` (`actionShowToolbox`) appears in
`mainToolBar` as well, next to it. Whether the panel is hidden is remembered
across runs.

## Today

- `FormMapCyl` lays out `stackedWidget` (map or globe), `verticalScrollBar`
  and `verticalLayout_6` side by side. A layout cannot be hidden.
- `actionShowToolbox` toggles the dock widget, but its checked state means
  nothing: every view-switch handler (`on_actionSatSelection_triggered` and
  the five others) sets it unchecked, and its own handler unchecks every view
  action and checks itself, so the button reads "checked" only until the next
  view switch. The dock's own visibility survives restarts through
  `saveState()`/`restoreState()`.

## Change

### formmapcyl.ui

`verticalLayout_6` is wrapped in a `QWidget` named `sidePanel`, with the
layout's four margins set to 0 so the contents stay where they are. Hiding the
wrapper removes its width, margins and spacing from the row, and the stacked
widget (Expanding) takes the space.

### FormMapCyl

- `void setSidePanelVisible(bool on)` (public slot): `ui->sidePanel->setVisible(on)`,
  `opts.sidePanelOn = on`.
- `bool isSidePanelVisible() const`.
- The constructor applies `opts.sidePanelOn` to the wrapper, the way it applies
  the other `/window/button…` states.

### Options

`bool sidePanelOn`, key `/window/sidepanelon`, default `true`, read in
`Initialize()` and written in `Save()` beside the other `/window/button…` keys.

### mainwindow.ui

- New action `actionShowSidePanel`: checkable, icon `:/icons/sidepanel.png`,
  text "Show Map Panel", tooltip "Show or hide the buttons next to the map and
  globe".
- `mainToolBar` gets, after `actionFitWindow`: a separator, `actionShowToolbox`,
  `actionShowSidePanel`, and the existing separator before `actionWhatsthis`
  stays. `actionShowToolbox` also stays in `toolBar`.

### MainWindow

- `on_actionShowSidePanel_triggered(bool checked)` →
  `formglobecyl->setSidePanelVisible(checked)`. After `formglobecyl` is
  built, `ui->actionShowSidePanel->setChecked(formglobecyl->isSidePanelVisible())`.
- `actionShowToolbox` checked ⇔ dock visible. After `restoreState()`:
  `setChecked(!dockwidget->isHidden())`; `dockwidget->visibilityChanged(bool)`
  is connected to `actionShowToolbox->setChecked`, so the dock's own close
  button unchecks it too. `on_actionShowToolbox_triggered()` only toggles the
  dock; the six view-switch handlers and the constructor no longer touch
  `actionShowToolbox`'s checked state. The view actions keep their radio
  behaviour among themselves as it is.

### Icon

`core/icons/sidepanel.png`, 48×48 like `options.png` and `image.png`: a window
outline with the right-hand column filled, drawn by a small Qt program kept
outside the repository. Added to `EUMETCastView.qrc`.

## Not done

- A per-view panel state (the same panel serves map and globe).
- Moving `actionShowToolbox` out of `toolBar`.
- Changing the toolbox icon.

## Testing

A probe linking the application's objects, on the offscreen platform:

1. `FormMapCyl` with `opts.sidePanelOn = true`: `sidePanel` visible; after
   `setSidePanelVisible(false)` it is hidden, the stacked widget is wider by the
   panel's width plus spacing, and `opts.sidePanelOn` is false; back on
   restores both.
2. With `opts.sidePanelOn = false` at construction the panel starts hidden.
3. `Options::Save()`/`Initialize()` round-trip `/window/sidepanelon` in a
   scratch ini.
4. `MainWindow` (if it can be built headless): after a view switch
   `actionShowToolbox` is still checked while the dock is visible; hiding the
   dock unchecks it; `actionShowSidePanel` follows `opts.sidePanelOn`.

Then a look on screen.
