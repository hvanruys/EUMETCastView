#ifndef FORMTOOLBOX_H
#define FORMTOOLBOX_H

#include <QWidget>
#include <QDockWidget>
#include <QFileSystemModel>
#include "formimage.h"
//#include "formmapcyl.h"
#include "formgeostationary.h"
#include "formmovie.h"

namespace Ui {
    class FormToolbox;
}

class FormImage;
class FormGeostationary;
class FormMovie;

class FormToolbox : public QWidget
{
    Q_OBJECT

public:
    explicit FormToolbox(QWidget *parent = 0, FormImage *p_formimage = 0, FormGeostationary *p_formgeostationary = 0, AVHRRSatellite *seglist = 0);
    int getTabWidgetIndex();
    int getToolboxIndex();
    int getTabWidgetVIIRSIndex();
    int getTabWidgetSentinelIndex();
    eProjectionType getCurrentProjectionType();
    int getGeoIndex() { return geoindex; }
    void setGeoIndex(int geo) { this->geoindex = geo; }
    void getOMimagesize(int *width, int *height );
    void setOMimagesize(int width, int height);
    QVector<QString> getSpectrumVector() { return spectrumvector; };
    QVector<bool> getInverseVector() { return inversevector; };


    QList<bool> getVIIRSMBandList();
    QList<int> getVIIRSMColorList();
    QList<bool> getVIIRSMInvertList();

    QList<bool> getOLCIBandList();
    QList<int> getOLCIColorList();
    QList<bool> getOLCIInvertList();
    int getOLCIHistogrammethod();
    bool getOLCINormalized();

    QList<bool> getSLSTRBandList();
    QList<int> getSLSTRColorList();
    QList<bool> getSLSTRInvertList();

    QList<bool> getMERSIBandList();
    QList<int> getMERSIColorList();
    QList<bool> getMERSIInvertList();
    int getMERSIHistogrammethod();



    int getGVPMapWidth();
    int getGVPMapHeight();
    float getGVPLat();
    float getGVPLon();
    float getGVPScale();
    int getGVPHeight();
    bool getGVPGridOnProj();
    int getGVPFalseEasting();
    int getGVPFalseNorthing();


    eSLSTRImageView getSLSTRImageView();

    void setTabWidgetIndex(int index);
    void setTabWidgetVIIRSIndex(int index);
    void setTabWidgetSentinelIndex(int index);
    void writeInfoToTextEdit(QString info);
    void createImageFilenamestring(QString sat, QString d, QVector<QString> spectrum);
    QString returnImageFilenamestring() { return filenamecreated; }
    bool comboColVIIRSOK();
    bool comboColAVHRROK();
    bool comboColOLCIOK();
    bool comboColSLSTROK();
    bool comboColGeoOK();
    bool comboColMERSIOK();
    bool GridOnProjLCC();
    bool GridOnProjGVP();
    bool GridOnProjSG();
    bool GridOnProjOM();
    void setPOIsettings();
    void setMConfigsettings();
    void setOLCIefrConfigsettings();
    void setSLSTRConfigsettings();
    void setMERSIConfigsettings();
    void setComboGeo(int geoindex);
    void setFormMovie(FormMovie *formmovie);
    void setProgressMaximum(int max);
    void setProgressValue(int val);
    void setupChannelGeoCombo(int index);

    QStringList getRowchosen() { return rowchosen; }
    eProjectionType currentProjectionType;


    ~FormToolbox();

private:
    Ui::FormToolbox *ui;
    FormImage *formimage;
    FormGeostationary *formgeostationary;
    FormMovie *formmovie;
    QCPColorMap *colorMap;

    void setupChannelCombo();
    void setInverseCheckBoxes();
    void onButtonChannel(QString channel, bool bInverse);
    void onButtonColorHRV(QString type);
    bool eventFilter(QObject *target, QEvent *event);
    int searchResolution(int mapwidth, int mapheight);
    void setLCCParameters(int strlindex);
    void setGVPParameters(int strlindex);
    void setSGParameters(int strlindex);
    void setConfigMParameters(int strlindex);
    void setRadioButtonsMToFalse();
    void setRadioButtonsMERSIToFalse();

    void setConfigOLCIParameters(int strlindex);
    void setConfigSLSTRParameters(int strlindex);
    void setConfigMERSIParameters(int strlindex);

    void setRadioButtonsOLCIefrToFalse();
    void setRadioButtonsSLSTRToFalse();
    void copyProjectionImage();
    bool checkSegmentDateTime();
    void setLogValue(int deg, double rad);
    void fitCurve();
    void setAllWhatsThis();
    void blockSignalscomboGeo(bool state);
    void reinitProjectionCanvas();
    void resetSpectrumInverse();


    AVHRRSatellite *segs;

    QVector<QString> spectrumvector;
    QVector<bool> inversevector;
    int geoindex;
    QStringList rowchosen;
    QString filenamecreated;
    QVector<int> resolutionX;
    QVector<int> resolutionY;
    eImageType currentAVHRRimage; // from 1 to 6 , 6 color image
    double valueRangeDNBGraph;

public slots:
    void setChannelComboBoxes();
    void setChannelIndex();
    void geostationarysegmentsChosen(int geoindex, QStringList tex);
    void setToolboxButtons(bool state);
    void setToolboxButtonLabels(int geoindex);
    void setButtons(int geoindex, bool state);

    void slotDisplayDNBGraph();
    void slotChangeAspectRatio(QPoint);

signals:
    void getgeosatchannel(QString, QVector<QString>, QVector<bool>, int, bool);
    void overlaycorrection(int,int);
    void switchstackedwidget(int);
    void creatergbrecipe(int recipe);
    void colorValueRed(int red);


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
    void setValueProgressBar(int val);


    void on_btnGeoColor_clicked();
    void on_btnCLAHEGeostationary_clicked();
    void on_btnExpandImage_clicked();
    void on_btnRotate180_clicked();
    void on_btnOverlayMeteosat_clicked();
    void on_btnCLAHEavhhr_clicked();
    void on_btnGeo1_clicked();
    void on_btnGeo2_clicked();
    void on_btnGeo3_clicked();
    void on_btnGeo4_clicked();
    void on_btnGeo5_clicked();
    void on_btnGeo6_clicked();
    void on_btnGeo7_clicked();
    void on_btnGeo8_clicked();
    void on_btnGeo9_clicked();
    void on_btnGeo10_clicked();
    void on_btnGeo11_clicked();
    void on_btnGeo12_clicked();
    void on_btnGeo13_clicked();
    void on_btnGeo14_clicked();
    void on_btnGeo15_clicked();
    void on_btnGeo16_clicked();

    void on_btnTextureMet_clicked();
    void on_btnTextureAVHRR_clicked();
    void on_tabWidget_currentChanged(int index);
    void on_spbNorth_valueChanged(int arg1);
    void on_spbWest_valueChanged(int arg1);
    void on_spbEast_valueChanged(int arg1);
    void on_spbSouth_valueChanged(int arg1);
    void on_chkShowLambert_stateChanged(int arg1);
    void on_btnCreateLambert_clicked();
    void on_spbLCCMapWidth_valueChanged(int arg1);
    void on_spbLCCMapHeight_valueChanged(int arg1);
    void on_spbGVPMapWidth_valueChanged(int arg1);
    void on_spbGVPMapHeight_valueChanged(int arg1);
    void on_spbSGMapWidth_valueChanged(int arg1);
    void on_spbSGMapHeight_valueChanged(int arg1);
    void on_btnCreatePerspective_clicked();
    void on_spbGVPlat_valueChanged(double arg1);
    void on_spbGVPlon_valueChanged(double arg1);
    void on_spbGVPheight_valueChanged(int arg1);
//    void on_spbGVPFalseEasting_valueChanged(int arg1);
//    void on_spbGVPFalseNorthing_valueChanged(int arg1);

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
    void on_spbSGlat_valueChanged(double arg1);
    void on_spbSGlon_valueChanged(double arg1);
    void on_spbSGScale_valueChanged(double arg1);
    void on_btnCreateStereo_clicked();
    void on_btnGVPHome_clicked();
    void on_btnSGHome_clicked();

    void on_spbSGPanHorizon_valueChanged(int arg1);
    void on_spbSGPanVert_valueChanged(int arg1);
    void on_spbSGRadius_valueChanged(double arg1);
    void on_btnSGClearMap_clicked();
    void on_btnOverlayProjectionSG_clicked();
    void on_btnOverlayProjectionOM_clicked();
    void on_spbLCCCorrX_valueChanged(int arg1);
    void on_spbLCCCorrY_valueChanged(int arg1);
    void on_btnUpdateVIIRSImage_clicked();
    void on_btnUpdateOLCIImage_clicked();
    void on_rbtnAColor_clicked();
    void on_rbtnACh1_clicked();
    void on_rbtnACh2_clicked();
    void on_rbtnACh3_clicked();
    void on_rbtnACh4_clicked();
    void on_rbtnACh5_clicked();
    void on_btnTextureVIIRS_clicked();
    void on_btnTextureOLCI_clicked();
    void on_btnTextureSLSTR_clicked();
    void on_sliCLAHE_sliderMoved(int position);
    void on_sliCLAHEAVHRR_sliderMoved(int position);
    void on_sliCLAHE_RGBRecipe_sliderMoved(int position);

    void on_cbProjResolutions_currentIndexChanged(int index);

    void on_sbCentreBand_valueChanged(int value);
    void on_spbDnbWindow_valueChanged(int arg1);
    void on_comboPOI_currentIndexChanged(int index);
    void on_chkLCCGridOnProj_clicked();
    void on_chkGVPGridOnProj_clicked();
    void on_chkSGGridOnProj_clicked();
    void on_btnAddPOI_clicked();

    void on_comboMConfig_currentIndexChanged(int index);
    void on_comboOLCIConfig_currentIndexChanged(int index);
    void on_comboSLSTRConfig_currentIndexChanged(int index);
    void on_btnAddMConfig_clicked();

    void on_tabWidgetVIIRS_currentChanged(int index);
    void on_tabWidgetSentinel_currentChanged(int index);

//    void on_btnGVPFalseColor_clicked();
//    void on_btnLCCFalseColor_clicked();
//    void on_btnSGFalseColor_clicked();

    void on_rdbOLCINormalized_toggled(bool checked);
    //void on_cmbHistogram_activated(int index);
    void on_cmbHistogramSLSTR_activated(int index);
    void on_btnSaveAsPNG48bits_clicked();
    void on_btnOverlayOLCI_clicked();
    void on_btnUpdateSLSTRImage_clicked();
    void on_btnSaveProjectionAsPNG48bits_clicked();

    void on_comboGeo1_currentIndexChanged(int index);
    void on_comboGeo2_currentIndexChanged(int index);
    void on_comboGeo3_currentIndexChanged(int index);
    void on_comboGeo4_currentIndexChanged(int index);
    void on_comboGeo5_currentIndexChanged(int index);
    void on_comboGeo6_currentIndexChanged(int index);
    void on_comboGeo7_currentIndexChanged(int index);
    void on_comboGeo8_currentIndexChanged(int index);
    void on_comboGeo9_currentIndexChanged(int index);
    void on_comboGeo10_currentIndexChanged(int index);
    void on_comboGeo11_currentIndexChanged(int index);
    void on_comboGeo12_currentIndexChanged(int index);
    void on_comboGeo13_currentIndexChanged(int index);
    void on_comboGeo14_currentIndexChanged(int index);
    void on_comboGeo15_currentIndexChanged(int index);
    void on_comboGeo16_currentIndexChanged(int index);

    void on_cmbHistogramAVHRR_activated(int index);
    void on_cmbHistogramGeo_activated(int index);
    void on_btnRecipes_clicked();
    void on_btnCLAHE_RGBRecipe_clicked();
    void on_btnUpdateMERSIImage_clicked();
    void on_comboMERSIConfig_currentIndexChanged(int index);
    void on_btnAddMERSIConfig_clicked();
    void on_btnCreateOM_clicked();

    void on_rdbAVHRRin_clicked();
    void on_rdbVIIRSMin_clicked();
    void on_rdbVIIRSDNBin_clicked();
    void on_rdbOLCIefrin_clicked();
    void on_rdbOLCIerrin_clicked();
    void on_rdbMERSIin_clicked();
    void on_rdbMeteosatin_clicked();

    void on_btnOMClearMap_clicked();
    void on_chkOMGridOnProj_clicked();
    void on_spbOMwidth_valueChanged(int arg1);
    void on_spbOMheight_valueChanged(int arg1);
    void on_spbGVPFalseNorthing_valueChanged(double arg1);
    void on_spbGVPFalseEasting_valueChanged(double arg1);
    void on_cbProjResolutions_activated(int index);
    void on_hslRed_valueChanged(int value);
    void on_btnUpdateAVHRRImage_clicked();
    void on_btnOverlayMoon_clicked();
    void on_chkInverseGeo1_stateChanged(int arg1);
    void on_chkInverseGeo2_stateChanged(int arg1);
    void on_chkInverseGeo3_stateChanged(int arg1);
    void on_chkInverseGeo4_stateChanged(int arg1);
    void on_chkInverseGeo5_stateChanged(int arg1);
    void on_chkInverseGeo6_stateChanged(int arg1);
    void on_chkInverseGeo7_stateChanged(int arg1);
    void on_chkInverseGeo8_stateChanged(int arg1);
    void on_chkInverseGeo9_stateChanged(int arg1);
    void on_chkInverseGeo10_stateChanged(int arg1);
    void on_chkInverseGeo11_stateChanged(int arg1);
    void on_chkInverseGeo12_stateChanged(int arg1);
    void on_chkInverseGeo13_stateChanged(int arg1);
    void on_chkInverseGeo14_stateChanged(int arg1);
    void on_chkInverseGeo15_stateChanged(int arg1);
    void on_chkInverseGeo16_stateChanged(int arg1);
};


#endif // FORMTOOLBOX_H


