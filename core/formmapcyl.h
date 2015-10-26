#ifndef FORMMAPCYL_H
#define FORMMAPCYL_H

#include "options.h"
#include "cylequidist.h"
#include "satellite.h"
#include "avhrrsatellite.h"
#include "mapcyl.h"
#include "globe.h"
#include "formtoolbox.h"

#include <QWidget>

namespace Ui {
    class FormMapCyl;
}

class FormToolbox;

class FormMapCyl : public QWidget
{
    Q_OBJECT

public:
    explicit FormMapCyl(QWidget *parent = 0, MapFieldCyl *p_mapcyl = 0, Globe *p_globe = 0, FormToolbox *p_formtoolbox = 0, SatelliteList *satlist=0, AVHRRSatellite *seglist=0 );

    void setCylOrGlobe(int ind);
    bool ComposeImage();

private:
    Ui::FormMapCyl *ui;
    SatelliteList *sats;
    AVHRRSatellite *segs;

    MapFieldCyl *mapcyl;
    Globe *globe;
    FormToolbox *formtoolbox;
    void RemoveAllSelected();

    //void ShowSegment(int value);




private slots:
    void toggleButtonMetop();
    void toggleButtonNoaa();
    void toggleButtonGAC();
    void toggleButtonHRP();
    void toggleButtonVIIRSM();
    void toggleButtonVIIRSDNB();
    void toggleButtonRealTime();
    void updatesatmap(int);
    void setScrollBarMaximum();
    void slotNothingSelected();


    void on_btnRemoveSelected_clicked();

    void on_btnMakeImage_clicked();

    void on_btnSaveTexture_clicked();

    void on_btnClearTexture_clicked();

    void on_verticalScrollBar_valueChanged(int value);

    void on_btnNoaa_clicked();

    void on_btnMetop_clicked();

    void on_btnHRP_clicked();

    void on_btnGAC_clicked();

    void on_btnRealTime_clicked();

    void on_btnVIIRSM_clicked();

    void on_btnAllSegments_clicked();

    void on_btnPhong_clicked();

    void on_btnVIIRSDNB_clicked();

public slots:
      void showSegmentList(int);
      void showSegmentcount();
      void changeScrollBar(int);

signals:
    void signalSegmentChanged(QString);
    void emitMakeImage();

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;


};

#endif // FORMMAPCYL_H

