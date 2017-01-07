#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QDebug>
#include <QDockWidget>
#include <QScrollBar>
#include <QScrollArea>

#include "formephem.h"
#include "formmapcyl.h"
#include "formtoolbox.h"
#include "dialogpreferences.h"
#include "dialogsaveimage.h"
#include "satellite.h"
#include "formgeostationary.h"
#include "forminfrascales.h"

#include "mapcyl.h"
#include "globe.h"
#include "formimage.h"
#include "imagescrollarea.h"
#include "segmentimage.h"
#include "segmentlistgeostationary.h"

#include "options.h"
#include "poi.h"

namespace Ui {
class MainWindow;
}
class InfraScales;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    SatelliteList *satlist;
    AVHRRSatellite *seglist;

    ~MainWindow();
    
private:

    void setupStatusBar();
    void createDockWidget();
    void closeEvent(QCloseEvent *event);
    void loadLayout();
    void saveLayout();

    Ui::MainWindow *ui;
    FormEphem *formephem;
    FormMapCyl *formglobecyl;
    FormImage *formimage;
    ImageScrollArea *imagescrollarea;
    QDockWidget *dockwidget;
    FormToolbox *formtoolbox;
    FormGeostationary *formgeostationary;

    CylEquiDist *cylequidist;
    MapFieldCyl *mapcyl;
    Globe *globe;

    FormInfraScales *forminfrascales;

    QTimer *timer;
    QLabel *timeLabel;
    QLabel *formulaLabel;
    QLabel *coordinateLabel;
    //ZoomObject *zoomobject;


protected:


private slots:
    void timerDone(void);
    void on_actionPreferences_triggered();
    void on_actionAbout_triggered();


    void on_actionSatSelection_triggered();
    void on_actionCylindricalEquidistant_triggered();
    void on_action3DGlobe_triggered();
    void on_actionImage_triggered();

    void on_actionShowToolbox_triggered();

    void on_actionCreatePNG_triggered();

    void on_actionMeteosat_triggered();
    void on_actionNormalSize_triggered();
    void on_actionFitWindow_triggered();
    void on_actionFitWindowWidth_triggered();
    void on_actionZoomin_triggered();
    void on_actionZoomout_triggered();
    void moveImage(QPoint, QPoint);
    void slotSwitchStackedWindow(int);
    void slotPreferencesFinished(int result);

    void on_actionWhatsthis_triggered();

public slots:
    void updateStatusBarIndicator(const QString &text);
    void updateStatusBarCoordinate(const QString &text);
    void updateWindowTitle();

};

#endif // MAINWINDOW_H
