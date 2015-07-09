#ifndef FORMTOOLBOX_H
#define FORMTOOLBOX_H

#include <QWidget>
#include "formimage.h"
#include "formmapcyl.h"


namespace Ui {
    class FormToolbox;
}

class FormImage;

class FormToolbox : public QWidget
{
    Q_OBJECT

public:
    explicit FormToolbox(QWidget *parent = 0, FormImage *p_formimage = 0, AVHRRSatellite *seglist = 0);
    int getTabWidgetIndex();
    QList<bool> getVIIRSBandList();
    QList<int> getVIIRSColorList();
    void setTabWidgetIndex(int index);
    void writeInfoToAVHRR(QString info);
    void writeInfoToVIIRS(QString info);
    void writeInfoToGeo(QString info);

    ~FormToolbox();

private:
    Ui::FormToolbox *ui;
    FormImage *formimage;
    void setupChannelCombo();
    void setInverseCheckBoxes();
    void setParameters();
    void onButtonChannel(QString channel, bool bInverse);
    void onButtonColorHRV(QString type);
    bool eventFilter(QObject *target, QEvent *event);

    AVHRRSatellite *segs;

    QVector<QString> spectrumvector;
    QVector<bool> inversevector;
    SegmentListGeostationary::eGeoSatellite whichgeo;
    QStringList rowchosen;

public slots:
    void setChannelComboBoxes();
    void setChannelIndex();
    void geostationarysegmentsChosen(SegmentListGeostationary::eGeoSatellite geo, QStringList tex);
    void setToolboxButtons(bool state);

signals:
    void getmeteosatchannel(SegmentListGeostationary::eGeoSatellite, QString, QVector<QString>, QVector<bool>);
    void overlaycorrection(int,int);
    void switchstackedwidget(int);
    void emitShowVIIRSImage();
    void screenupdateprojection();


private slots:
    void on_btnCol_clicked();
    void on_btnCh1_clicked();
    void on_btnCh2_clicked();
    void on_btnCh3_clicked();
    void on_btnCh4_clicked();
    void on_btnCh5_clicked();

    void setChannelInverse();
    void setMeteosatGamma();
    void setMeteosatGamma(double gammaval);
    void addTopbGeostationary(int val);
    void setMaxpbGeostationary(int max);
    void setValueProgressBar(int val);


    void on_btnMeteosat_clicked();
    void on_btnCLAHEMeteosat_clicked();
    void on_btnExpandImage_clicked();
    void on_btnRotate180_clicked();
    void on_btnOverlayMeteosat_clicked();
    void on_btnCLAHEavhhr_clicked();
    void on_btnVIS006_clicked();
    void on_btnVIS008_clicked();
    void on_btnIR016_clicked();
    void on_btnIR039_clicked();
    void on_btnWV062_clicked();
    void on_btnWV073_clicked();
    void on_btnIR087_clicked();
    void on_btnIR097_clicked();
    void on_btnIR108_clicked();
    void on_btnIR120_clicked();
    void on_btnIR134_clicked();
    void on_btnTextureMet_clicked();
    void on_btnTextureAVHRR_clicked();
    void on_tabWidget_currentChanged(int index);
    void on_spbNorth_valueChanged(int arg1);
    void on_spbWest_valueChanged(int arg1);
    void on_spbEast_valueChanged(int arg1);
    void on_spbSouth_valueChanged(int arg1);
    void on_chkShowLambert_stateChanged(int arg1);
    void on_btnCreateLambert_clicked();
    void on_spbMapWidth_valueChanged(int arg1);
    void on_spbMapHeight_valueChanged(int arg1);
    void on_btnCreatePerspective_clicked();
    void on_spbGVPlat_valueChanged(double arg1);
    void on_spbGVPlon_valueChanged(double arg1);
    void on_spbGVPheight_valueChanged(int arg1);
    void on_spbGVPscale_valueChanged(double arg1);
    void on_btnOverlayProjectionGVP_clicked();
    void on_btnOverlayProjectionLCC_clicked();
    void on_chkShowPerspective_stateChanged(int arg1);
    void on_toolBox_currentChanged(int index);
    void on_spbParallel1_valueChanged(int arg1);
    void on_spbParallel2_valueChanged(int arg1);
    void on_spbCentral_valueChanged(int arg1);
    void on_spbLatOrigin_valueChanged(int arg1);
    void on_spbScaleX_valueChanged(double arg1);
    void on_spbScaleY_valueChanged(double arg1);
    void on_btnHRV_clicked();
    void on_btnGVPClearMap_clicked();
    void on_btnLCCClearMap_clicked();

    void on_btnLCCMapNorth_clicked();
    void on_btnLCCMapSouth_clicked();
    void on_btnLCCMapWest_clicked();
    void on_btnLCCMapEast_clicked();
    void on_scbLCCMapUpDown_valueChanged(int value);
    void on_scbLCCMapLeftRight_valueChanged(int value);
    void on_spbSGlat_valueChanged(double arg1);
    void on_spbSGlon_valueChanged(double arg1);
    void on_spbSGScale_valueChanged(double arg1);
    void on_btnCreateStereo_clicked();
    void on_btnGVPHome_clicked();
    void on_btnSGHome_clicked();

    void on_spbSGPanHorizon_valueChanged(int arg1);
    void on_spbSGPanVert_valueChanged(int arg1);
    void on_btnSGNorth_clicked();
    void on_btnSGSouth_clicked();
    void on_btnSGEquatorial_clicked();
    void on_spbSGRadius_valueChanged(double arg1);
    void on_btnSGClearMap_clicked();
    void on_btnOverlayProjectionSG_clicked();
    void on_tabWidget_tabBarClicked(int index);
    void on_spbLCCCorrX_valueChanged(int arg1);
    void on_spbLCCCorrY_valueChanged(int arg1);
    void on_btnSetTrueColors_clicked();
    void on_btnSetNaturalColors_clicked();
    void on_btnMakeVIIRSImage_clicked();
    void on_rbtnAColor_clicked();
    void on_rbtnACh1_clicked();
    void on_rbtnACh2_clicked();
    void on_rbtnACh3_clicked();
    void on_rbtnACh4_clicked();
    void on_rbtnACh5_clicked();
    void on_btnTextureVIIRS_clicked();
    void on_sliCLAHE_sliderMoved(int position);
};

#endif // FORMTOOLBOX_H


