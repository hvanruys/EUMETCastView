#include "formtoolbox.h"
#include "ui_formtoolbox.h"
#include "msgfileaccess.h"
#include "poi.h"
#include <cmath>

#include "FreeImage.h"

extern Options opts;
extern Poi poi;
extern SegmentImage *imageptrs;

#include <QMutex>

#define BIT_DEPTH 8
#define BYTES_PER_PIXEL ((3 * BIT_DEPTH)/8) + 1 //RGBA


class SignalsBlocker
{
public:
    SignalsBlocker(QObject* ptr):
    _ptr(ptr)
    {
        _b = ptr->blockSignals(true);
    }
    ~SignalsBlocker()
    {
        _ptr->blockSignals(_b);
    }

private:
    QObject* _ptr;
    bool _b;
};

FormToolbox::FormToolbox(QWidget *parent, FormImage *p_formimage, FormGeostationary *p_formgeostationary, FormInfraScales *p_forminfrascales, AVHRRSatellite *seglist) :
    QWidget(parent),
    ui(new Ui::FormToolbox)
{
    ui->setupUi(this);
    formimage = p_formimage;
    formgeostationary = p_formgeostationary;
    forminfrascales = p_forminfrascales;
    segs = seglist;
    filenamecreated = "";

    setupChannelCombo();
    setChannelComboBoxes();
    currentAVHRRimage = IMAGE_AVHRR_COL;


    if (opts.imageontextureOnAVHRR)
        ui->btnTextureAVHRR->setText("Texture On");
    else
        ui->btnTextureAVHRR->setText("Texture Off");

    if (opts.imageontextureOnMet)
        ui->btnTextureMet->setText("Texture On");
    else
        ui->btnTextureMet->setText("Texture Off");

    if (opts.imageontextureOnVIIRS)
        ui->btnTextureVIIRS->setText("Texture On");
    else
        ui->btnTextureVIIRS->setText("Texture Off");

    if (opts.imageontextureOnOLCI)
        ui->btnTextureOLCI->setText("Texture On");
    else
        ui->btnTextureOLCI->setText("Texture Off");

    ui->btnOverlayMeteosat->setText("Overlay On");
    ui->btnOverlayOLCI->setText("Overlay On");
    ui->btnOverlayProjectionGVP->setText("Overlay On");
    ui->btnOverlayProjectionLCC->setText("Overlay On");
    ui->btnOverlayProjectionSG->setText(("Overlay On"));

    if(opts.lastinputprojection == 0)
        ui->rdbAVHRRin->setChecked(true);
    else if(opts.lastinputprojection == 1)
        ui->rdbVIIRSMin->setChecked(true);
    else if(opts.lastinputprojection == 2)
        ui->rdbVIIRSDNBin->setChecked(true);
    else if(opts.lastinputprojection == 3)
        ui->rdbOLCIefrin->setChecked(true);
    else if(opts.lastinputprojection == 4)
        ui->rdbOLCIerrin->setChecked(true);
    else
        ui->rdbMeteosatin->setChecked(true);

    ui->cmbHRVtype->addItem("Europe");
    ui->cmbHRVtype->addItem("Full");


    connect(ui->sliMeteosatGamma, SIGNAL(sliderReleased()), this, SLOT(setMeteosatGamma()));
    connect(ui->spbMeteosatGamma, SIGNAL(valueChanged(double)), this,SLOT(setMeteosatGamma(double)));

    ui->sliMeteosatGamma->setSliderPosition(opts.meteosatgamma*100);
    ui->spbMeteosatGamma->setValue(opts.meteosatgamma);
    //setMeteosatGamma(opts.meteosatgamma);

    connect(ui->comboCh1, SIGNAL(activated(int)), this, SLOT(setChannelIndex()));
    connect(ui->comboCh2, SIGNAL(activated(int)), this, SLOT(setChannelIndex()));
    connect(ui->comboCh3, SIGNAL(activated(int)), this, SLOT(setChannelIndex()));
    connect(ui->comboCh4, SIGNAL(activated(int)), this, SLOT(setChannelIndex()));
    connect(ui->comboCh5, SIGNAL(activated(int)), this, SLOT(setChannelIndex()));

    connect(ui->chkInverseCh1, SIGNAL(stateChanged(int)), this, SLOT(setChannelInverse()));
    connect(ui->chkInverseCh2, SIGNAL(stateChanged(int)), this, SLOT(setChannelInverse()));
    connect(ui->chkInverseCh3, SIGNAL(stateChanged(int)), this, SLOT(setChannelInverse()));
    connect(ui->chkInverseCh4, SIGNAL(stateChanged(int)), this, SLOT(setChannelInverse()));
    connect(ui->chkInverseCh5, SIGNAL(stateChanged(int)), this, SLOT(setChannelInverse()));

    spectrumvector.append("");
    spectrumvector.append("");
    spectrumvector.append("");

    inversevector.append(false);
    inversevector.append(false);
    inversevector.append(false);

    formimage->channelshown = IMAGE_GEOSTATIONARY;
    ui->tabWidget->setCurrentIndex(opts.currenttabwidget);
    ui->tabWidgetVIIRS->setCurrentIndex(0);

    QStringList listResolution;
    listResolution << "User defined";
    listResolution << "4:3  SVGA   800x600";
    listResolution << "4:3  XGA   1024x768";
    listResolution << "4:3  XGA+  1152x864";
    listResolution << "4:3  SXGA  1280x960";
    listResolution << "4:3  SVGA+ 1400x1050";
    listResolution << "4:3  UXGA  1600x1200";
    listResolution << "4:3  ----  1000x750";
    listResolution << "4:3  ----  2000x1500";
    listResolution << "4:3  ----  3000x2250";
    listResolution << "4:3  ----  4000x3000";
    listResolution << "4:3  ----  5000x3750";


    listResolution << "16:9 WXGA  1280x720";
    listResolution << "16:9 HD    1360x768";
    listResolution << "16:9 HD+   1600x900";
    listResolution << "16:9 FHD   1920x1080";
    listResolution << "16:9 QWXGA 2048x1152";
    listResolution << "16:9 WQHD  2560x1440";
    listResolution << "16:9 UHD   3840x2160";

    listResolution << "1:1  ---   1000x1000";
    listResolution << "1:1  ---   2000x2000";
    listResolution << "1:1  ---   3000x3000";
    listResolution << "1:1  ---   4000x4000";
    listResolution << "1:1  ---   5000x5000";


    resolutionX.append(800);
    resolutionX.append(1024);
    resolutionX.append(1152);
    resolutionX.append(1280);
    resolutionX.append(1400);
    resolutionX.append(1600);
    resolutionX.append(1000);
    resolutionX.append(2000);
    resolutionX.append(3000);
    resolutionX.append(4000);
    resolutionX.append(5000);

    resolutionX.append(1280);
    resolutionX.append(1360);
    resolutionX.append(1600);
    resolutionX.append(1920);
    resolutionX.append(2048);
    resolutionX.append(2560);
    resolutionX.append(3840);

    resolutionX.append(1000);
    resolutionX.append(2000);
    resolutionX.append(3000);
    resolutionX.append(4000);
    resolutionX.append(5000);

    resolutionY.append(600);
    resolutionY.append(768);
    resolutionY.append(864);
    resolutionY.append(960);
    resolutionY.append(1050);
    resolutionY.append(1200);
    resolutionY.append(750);
    resolutionY.append(1500);
    resolutionY.append(2250);
    resolutionY.append(3000);
    resolutionY.append(3750);

    resolutionY.append(720);
    resolutionY.append(768);
    resolutionY.append(900);
    resolutionY.append(1080);
    resolutionY.append(1152);
    resolutionY.append(1440);
    resolutionY.append(2160);

    resolutionY.append(1000);
    resolutionY.append(2000);
    resolutionY.append(3000);
    resolutionY.append(4000);
    resolutionY.append(5000);

    ui->cbProjResolutions->addItems(listResolution);

    ui->pbProgress->setValue(0);
    ui->pbProgress->setMinimum(0);
    ui->pbProgress->setMaximum(100);

    setPOIsettings();
    setMConfigsettings();
    setOLCIefrConfigsettings();


    qDebug() << QString("Setting currenttoolbox = %1").arg(opts.currenttoolbox);
    ui->toolBox->setCurrentIndex(opts.currenttoolbox); // in projection tab LCC GVP or SG
    ui->comboPOI->setCurrentIndex(0);

    ui->btnLCCMapNorth->installEventFilter(this);
    ui->btnLCCMapSouth->installEventFilter(this);
    ui->btnLCCMapEast->installEventFilter(this);
    ui->btnLCCMapWest->installEventFilter(this);


    ui->rbtnAColor->setChecked(true);
    opts.channelontexture = 6; // color channel

    ui->teAVHRR->setReadOnly(true);
    ui->teAVHRR->append(formimage->txtInfo);

    ui->lblCLAHE->setText(QString("%1").arg(double(opts.clahecliplimit), 0, 'f', 1));
    ui->sliCLAHE->setSliderPosition(opts.clahecliplimit * 10);

    ui->sbCentreBand->blockSignals(true);

    ui->sbCentreBand->setMinimum(opts.dnbsblowerlimit);
    ui->sbCentreBand->setMaximum(opts.dnbsbupperlimit);
    ui->sbCentreBand->setValue(opts.dnbsbvalue);
    ui->sbCentreBand->setTracking(false);
    ui->sbCentreBand->blockSignals(false);

    ui->spbDnbWindow->setValue(opts.dnbspbwindowsvalue);

    float fval = pow(10, opts.dnbsbvalue/20.0);
    ui->lblCentreBand->setText(QString("%1").arg(fval, 0, 'E', 2));
    ui->lblTitleCentreBand->setText(QString("Centre Band from %1 to %2 [W/cm² sr]").arg(fval/pow(10, opts.dnbspbwindowsvalue), 0, 'E', 2).arg(fval*pow(10, opts.dnbspbwindowsvalue), 0, 'E', 2));



    ui->comboGeo1->setCurrentIndex(opts.lastcomboMet006);
    ui->comboGeo2->setCurrentIndex(opts.lastcomboMet008);
    ui->comboGeo3->setCurrentIndex(opts.lastcomboMet016);
    ui->comboGeo4->setCurrentIndex(opts.lastcomboMet039);
    ui->comboGeo5->setCurrentIndex(opts.lastcomboMet062);
    ui->comboGeo6->setCurrentIndex(opts.lastcomboMet073);
    ui->comboGeo7->setCurrentIndex(opts.lastcomboMet087);
    ui->comboGeo8->setCurrentIndex(opts.lastcomboMet097);
    ui->comboGeo9->setCurrentIndex(opts.lastcomboMet108);
    ui->comboGeo10->setCurrentIndex(opts.lastcomboMet120);
    ui->comboGeo11->setCurrentIndex(opts.lastcomboMet134);

    ui->chkInverseGeo1->setChecked(opts.lastinverseMet006);
    ui->chkInverseGeo2->setChecked(opts.lastinverseMet008);
    ui->chkInverseGeo3->setChecked(opts.lastinverseMet016);
    ui->chkInverseGeo4->setChecked(opts.lastinverseMet039);
    ui->chkInverseGeo5->setChecked(opts.lastinverseMet062);
    ui->chkInverseGeo6->setChecked(opts.lastinverseMet073);
    ui->chkInverseGeo7->setChecked(opts.lastinverseMet087);
    ui->chkInverseGeo8->setChecked(opts.lastinverseMet097);
    ui->chkInverseGeo9->setChecked(opts.lastinverseMet108);
    ui->chkInverseGeo10->setChecked(opts.lastinverseMet120);
    ui->chkInverseGeo11->setChecked(opts.lastinverseMet134);


    ui->lblGeo1->setText("0.635");
    ui->lblGeo2->setText("0.81");
    ui->lblGeo3->setText("1.64");
    ui->lblGeo4->setText("3.90");
    ui->lblGeo5->setText("6.25");
    ui->lblGeo6->setText("7.35");
    ui->lblGeo7->setText("8.70");
    ui->lblGeo8->setText("9.66");
    ui->lblGeo9->setText("10.80");
    ui->lblGeo10->setText("12.00");
    ui->lblGeo11->setText("13.40");
    ui->lblGeo12->setText("");
    ui->lblGeo13->setText("");

    ui->lblUpper->setVisible(false);
    ui->lblLower->setVisible(false);
    ui->spbLower->setVisible(false);
    ui->spbUpper->setVisible(false);

    ui->chkShowLambert->setChecked(opts.mapextentlamberton);
    ui->chkShowPerspective->setChecked(opts.mapextentperspectiveon);

    whichgeo = SegmentListGeostationary::eGeoSatellite::NOGEO;

    setToolboxButtons(true);

    double valuerange1;
    double valuerange2;

    //ui->graph->setInteraction(QCP::iRangeDrag|QCP::iRangeZoom);
    colorMap = new QCPColorMap(ui->graph->xAxis, ui->graph->yAxis);
    ui->graph->addPlottable(colorMap);

    colorMap->data()->setSize(180, 150);
    colorMap->data()->setRange(QCPRange(0, 179), QCPRange(1.0E-15, 1.0));
    valuerange1 = 1.0E-15;
    valuerange2 = 1.0;
    valueRangeDNBGraph = log10(valuerange2) - log10(valuerange1);

    ui->graph->xAxis->setLabel("Solar Zenith Angle (deg)");
    ui->graph->yAxis->setLabel("radiance");

    ui->graph->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->graph->yAxis->setScaleLogBase(10.0);
    ui->graph->yAxis->setAutoTicks(false);
    ui->graph->yAxis->setSubTickCount(8);
    QVector<double> tick;
    tick << 1.0E-15 << 1.0E-14 << 1.0E-13 << 1.0E-12 << 1.0E-11 << 1.0E-10
         << 1.0E-9 << 1.0E-8 << 1.0E-7 << 1.0E-6 << 1.0E-5 << 1.0E-4 << 1.0E-3 << 0.01 << 0.1 << 1 ;
    ui->graph->yAxis->setTickVector(tick);


    ui->graph->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    ui->graph->axisRect()->setupFullAxesBox(true);


    // add a color scale:
    QCPColorScale *colorScale = new QCPColorScale(ui->graph);
    ui->graph->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap->setColorScale(colorScale); // associate the color map with the color scale
    colorScale->axis()->setLabel("Radiance Distribution");


    QCPColorGradient gradient = QCPColorGradient::gpPolar;
    gradient.setLevelCount(12);

    colorMap->setGradient(gradient);


    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->graph);
    ui->graph->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);


    colorMap->rescaleDataRange(true);
    ui->graph->rescaleAxes();
    ui->graph->replot();

    QStringList lsthistogram;
    lsthistogram << "None 95%" << "None 100%" << "Equalize";
    ui->cmbHistogram->addItems(lsthistogram);
    ui->cmbHistogram->setCurrentIndex(CMB_HISTO_NONE_95);
    lsthistogram.clear();
    lsthistogram << "None 95%" << "None 100%" << "Equalize" << "Equalize Projection";
    ui->cmbHistogramProj->addItems(lsthistogram);
    ui->cmbHistogramProj->setCurrentIndex(CMB_HISTO_NONE_95);

    ui->rdbGridOnOLCIimage->setChecked(opts.gridonolciimage);

    setAllWhatsThis();

    qDebug() << "constructor formtoolbox";

}

void FormToolbox::setPOIsettings()
{
    qDebug() << "FormToolbox::setPOIsettings()";

    ui->comboPOI->blockSignals(true);
    ui->comboPOI->clear();

    if(ui->toolBox->currentIndex() == 0)
        ui->comboPOI->addItems(poi.strlLCCName);
    else if(ui->toolBox->currentIndex() == 1)
    {
        ui->comboPOI->addItems(poi.strlGVPName);
    }
    else if(ui->toolBox->currentIndex() == 2)
        ui->comboPOI->addItems(poi.strlSGName);

    setLCCParameters(0);
    setGVPParameters(0);
    setSGParameters(0);
    ui->comboPOI->blockSignals(false);
    on_comboPOI_currentIndexChanged(0);
}

void FormToolbox::setMConfigsettings()
{
    qDebug() << "FormToolbox::setMConfigsettings()";

    ui->comboMConfig->blockSignals(true);
    ui->comboMConfig->clear();

    ui->comboMConfig->addItems(poi.strlConfigNameM);

    setConfigMParameters(0);

    ui->comboMConfig->blockSignals(false);
}

void FormToolbox::setOLCIefrConfigsettings()
{
    qDebug() << "FormToolbox::setOLCIefrConfigsettings()";

    ui->comboOLCIConfig->blockSignals(true);
    ui->comboOLCIConfig->clear();

    ui->comboOLCIConfig->addItems(poi.strlConfigNameOLCI);

    setConfigOLCIParameters(0); // 0 = User Defines

    ui->comboOLCIConfig->blockSignals(false);
}

void FormToolbox::writeInfoToAVHRR(QString info)
{
    ui->teAVHRR->clear();
    ui->teAVHRR->append(info);
}

void FormToolbox::writeInfoToVIIRSM(QString info)
{
    ui->teVIIRSM->clear();
    ui->teVIIRSM->append(info);
}

void FormToolbox::writeInfoToVIIRSDNB(QString info)
{
    ui->teVIIRSDNB->clear();
    ui->teVIIRSDNB->append(info);
}

void FormToolbox::writeInfoToGeo(QString info)
{
    ui->teGeo->clear();
    ui->teGeo->append(info);
}

void FormToolbox::writeInfoToOLCI(QString info)
{
    ui->teOLCI->clear();
    ui->teOLCI->append(info);
}

bool FormToolbox::eventFilter(QObject *target, QEvent *event)
{
    if (target == ui->btnLCCMapNorth || target == ui->btnLCCMapSouth || target == ui->btnLCCMapEast || target == ui->btnLCCMapWest)
    {
        if (event->type() == QEvent::Wheel)
        {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
            QPoint numPixels = wheelEvent->pixelDelta();
            QPoint numDegrees = wheelEvent->angleDelta() / 8;

            if (!numPixels.isNull()) {
                qDebug() << QString("eventFilter numPixels x = %1 y = %2 target = %3").arg(numPixels.x()).arg(numPixels.y()).arg(target->objectName());
            } else if (!numDegrees.isNull()) {
                QPoint numSteps = numDegrees / 15;
                qDebug() << QString("eventFilter numSteps  x = %1 y = %2 target = %3").arg(numSteps.x()).arg(numSteps.y()).arg(target->objectName());
                if(target->objectName() == "btnLCCMapNorth" || target->objectName() == "btnLCCMapSouth")
                {
                    if(numSteps.y() > 0)
                        on_btnLCCMapNorth_clicked();
                    else
                        on_btnLCCMapSouth_clicked();
                }
                if(target->objectName() == "btnLCCMapEast" || target->objectName() == "btnLCCMapWest")
                {
                    if(numSteps.y() > 0)
                        on_btnLCCMapEast_clicked();
                    else
                        on_btnLCCMapWest_clicked();
                }
            }
        }
    }
    return QWidget::eventFilter(target, event);
}


void FormToolbox::setValueProgressBar(int val)
{
    qDebug() << "FormToolbox::setValueProgressBar(int val) val = " << val;
    ui->pbProgress->setValue(val);
    //this->update();
}

void FormToolbox::setupChannelCombo()
{
    qDebug() << "FormToolbox::setupChannelCombo()";

    ui->comboCh1->addItem(tr("-"));ui->comboCh1->addItem(tr("R"));ui->comboCh1->addItem(tr("G"));ui->comboCh1->addItem(tr("B"));
    ui->comboCh2->addItem(tr("-"));ui->comboCh2->addItem(tr("R"));ui->comboCh2->addItem(tr("G"));ui->comboCh2->addItem(tr("B"));
    ui->comboCh3->addItem(tr("-"));ui->comboCh3->addItem(tr("R"));ui->comboCh3->addItem(tr("G"));ui->comboCh3->addItem(tr("B"));
    ui->comboCh4->addItem(tr("-"));ui->comboCh4->addItem(tr("R"));ui->comboCh4->addItem(tr("G"));ui->comboCh4->addItem(tr("B"));
    ui->comboCh5->addItem(tr("-"));ui->comboCh5->addItem(tr("R"));ui->comboCh5->addItem(tr("G"));ui->comboCh5->addItem(tr("B"));
    ui->comboGeo1->addItem(tr("-"));ui->comboGeo1->addItem(tr("R"));ui->comboGeo1->addItem(tr("G"));ui->comboGeo1->addItem(tr("B"));
    ui->comboGeo2->addItem(tr("-"));ui->comboGeo2->addItem(tr("R"));ui->comboGeo2->addItem(tr("G"));ui->comboGeo2->addItem(tr("B"));
    ui->comboGeo3->addItem(tr("-"));ui->comboGeo3->addItem(tr("R"));ui->comboGeo3->addItem(tr("G"));ui->comboGeo3->addItem(tr("B"));
    ui->comboGeo4->addItem(tr("-"));ui->comboGeo4->addItem(tr("R"));ui->comboGeo4->addItem(tr("G"));ui->comboGeo4->addItem(tr("B"));
    ui->comboGeo5->addItem(tr("-"));ui->comboGeo5->addItem(tr("R"));ui->comboGeo5->addItem(tr("G"));ui->comboGeo5->addItem(tr("B"));
    ui->comboGeo6->addItem(tr("-"));ui->comboGeo6->addItem(tr("R"));ui->comboGeo6->addItem(tr("G"));ui->comboGeo6->addItem(tr("B"));
    ui->comboGeo7->addItem(tr("-"));ui->comboGeo7->addItem(tr("R"));ui->comboGeo7->addItem(tr("G"));ui->comboGeo7->addItem(tr("B"));
    ui->comboGeo8->addItem(tr("-"));ui->comboGeo8->addItem(tr("R"));ui->comboGeo8->addItem(tr("G"));ui->comboGeo8->addItem(tr("B"));
    ui->comboGeo9->addItem(tr("-"));ui->comboGeo9->addItem(tr("R"));ui->comboGeo9->addItem(tr("G"));ui->comboGeo9->addItem(tr("B"));
    ui->comboGeo10->addItem(tr("-"));ui->comboGeo10->addItem(tr("R"));ui->comboGeo10->addItem(tr("G"));ui->comboGeo10->addItem(tr("B"));
    ui->comboGeo11->addItem(tr("-"));ui->comboGeo11->addItem(tr("R"));ui->comboGeo11->addItem(tr("G"));ui->comboGeo11->addItem(tr("B"));
    ui->comboGeo12->addItem(tr("-"));ui->comboGeo12->addItem(tr("R"));ui->comboGeo12->addItem(tr("G"));ui->comboGeo12->addItem(tr("B"));
    ui->comboGeo13->addItem(tr("-"));ui->comboGeo13->addItem(tr("R"));ui->comboGeo13->addItem(tr("G"));ui->comboGeo13->addItem(tr("B"));

    ui->comboM1->addItem(tr("-"));ui->comboM1->addItem(tr("R"));ui->comboM1->addItem(tr("G"));ui->comboM1->addItem(tr("B"));
    ui->comboM2->addItem(tr("-"));ui->comboM2->addItem(tr("R"));ui->comboM2->addItem(tr("G"));ui->comboM2->addItem(tr("B"));
    ui->comboM3->addItem(tr("-"));ui->comboM3->addItem(tr("R"));ui->comboM3->addItem(tr("G"));ui->comboM3->addItem(tr("B"));
    ui->comboM4->addItem(tr("-"));ui->comboM4->addItem(tr("R"));ui->comboM4->addItem(tr("G"));ui->comboM4->addItem(tr("B"));
    ui->comboM5->addItem(tr("-"));ui->comboM5->addItem(tr("R"));ui->comboM5->addItem(tr("G"));ui->comboM5->addItem(tr("B"));
    ui->comboM6->addItem(tr("-"));ui->comboM6->addItem(tr("R"));ui->comboM6->addItem(tr("G"));ui->comboM6->addItem(tr("B"));
    ui->comboM7->addItem(tr("-"));ui->comboM7->addItem(tr("R"));ui->comboM7->addItem(tr("G"));ui->comboM7->addItem(tr("B"));
    ui->comboM8->addItem(tr("-"));ui->comboM8->addItem(tr("R"));ui->comboM8->addItem(tr("G"));ui->comboM8->addItem(tr("B"));
    ui->comboM9->addItem(tr("-"));ui->comboM9->addItem(tr("R"));ui->comboM9->addItem(tr("G"));ui->comboM9->addItem(tr("B"));
    ui->comboM10->addItem(tr("-"));ui->comboM10->addItem(tr("R"));ui->comboM10->addItem(tr("G"));ui->comboM10->addItem(tr("B"));
    ui->comboM11->addItem(tr("-"));ui->comboM11->addItem(tr("R"));ui->comboM11->addItem(tr("G"));ui->comboM11->addItem(tr("B"));
    ui->comboM12->addItem(tr("-"));ui->comboM12->addItem(tr("R"));ui->comboM12->addItem(tr("G"));ui->comboM12->addItem(tr("B"));
    ui->comboM13->addItem(tr("-"));ui->comboM13->addItem(tr("R"));ui->comboM13->addItem(tr("G"));ui->comboM13->addItem(tr("B"));
    ui->comboM14->addItem(tr("-"));ui->comboM14->addItem(tr("R"));ui->comboM14->addItem(tr("G"));ui->comboM14->addItem(tr("B"));
    ui->comboM15->addItem(tr("-"));ui->comboM15->addItem(tr("R"));ui->comboM15->addItem(tr("G"));ui->comboM15->addItem(tr("B"));
    ui->comboM16->addItem(tr("-"));ui->comboM16->addItem(tr("R"));ui->comboM16->addItem(tr("G"));ui->comboM16->addItem(tr("B"));

    ui->cmbOLCI01->addItem(tr("-"));ui->cmbOLCI01->addItem(tr("R"));ui->cmbOLCI01->addItem(tr("G"));ui->cmbOLCI01->addItem(tr("B"));
    ui->cmbOLCI02->addItem(tr("-"));ui->cmbOLCI02->addItem(tr("R"));ui->cmbOLCI02->addItem(tr("G"));ui->cmbOLCI02->addItem(tr("B"));
    ui->cmbOLCI03->addItem(tr("-"));ui->cmbOLCI03->addItem(tr("R"));ui->cmbOLCI03->addItem(tr("G"));ui->cmbOLCI03->addItem(tr("B"));
    ui->cmbOLCI04->addItem(tr("-"));ui->cmbOLCI04->addItem(tr("R"));ui->cmbOLCI04->addItem(tr("G"));ui->cmbOLCI04->addItem(tr("B"));
    ui->cmbOLCI05->addItem(tr("-"));ui->cmbOLCI05->addItem(tr("R"));ui->cmbOLCI05->addItem(tr("G"));ui->cmbOLCI05->addItem(tr("B"));
    ui->cmbOLCI06->addItem(tr("-"));ui->cmbOLCI06->addItem(tr("R"));ui->cmbOLCI06->addItem(tr("G"));ui->cmbOLCI06->addItem(tr("B"));
    ui->cmbOLCI07->addItem(tr("-"));ui->cmbOLCI07->addItem(tr("R"));ui->cmbOLCI07->addItem(tr("G"));ui->cmbOLCI07->addItem(tr("B"));
    ui->cmbOLCI08->addItem(tr("-"));ui->cmbOLCI08->addItem(tr("R"));ui->cmbOLCI08->addItem(tr("G"));ui->cmbOLCI08->addItem(tr("B"));
    ui->cmbOLCI09->addItem(tr("-"));ui->cmbOLCI09->addItem(tr("R"));ui->cmbOLCI09->addItem(tr("G"));ui->cmbOLCI09->addItem(tr("B"));
    ui->cmbOLCI10->addItem(tr("-"));ui->cmbOLCI10->addItem(tr("R"));ui->cmbOLCI10->addItem(tr("G"));ui->cmbOLCI10->addItem(tr("B"));
    ui->cmbOLCI11->addItem(tr("-"));ui->cmbOLCI11->addItem(tr("R"));ui->cmbOLCI11->addItem(tr("G"));ui->cmbOLCI11->addItem(tr("B"));
    ui->cmbOLCI12->addItem(tr("-"));ui->cmbOLCI12->addItem(tr("R"));ui->cmbOLCI12->addItem(tr("G"));ui->cmbOLCI12->addItem(tr("B"));
    ui->cmbOLCI13->addItem(tr("-"));ui->cmbOLCI13->addItem(tr("R"));ui->cmbOLCI13->addItem(tr("G"));ui->cmbOLCI13->addItem(tr("B"));
    ui->cmbOLCI14->addItem(tr("-"));ui->cmbOLCI14->addItem(tr("R"));ui->cmbOLCI14->addItem(tr("G"));ui->cmbOLCI14->addItem(tr("B"));
    ui->cmbOLCI15->addItem(tr("-"));ui->cmbOLCI15->addItem(tr("R"));ui->cmbOLCI15->addItem(tr("G"));ui->cmbOLCI15->addItem(tr("B"));
    ui->cmbOLCI16->addItem(tr("-"));ui->cmbOLCI16->addItem(tr("R"));ui->cmbOLCI16->addItem(tr("G"));ui->cmbOLCI16->addItem(tr("B"));
    ui->cmbOLCI17->addItem(tr("-"));ui->cmbOLCI17->addItem(tr("R"));ui->cmbOLCI17->addItem(tr("G"));ui->cmbOLCI17->addItem(tr("B"));
    ui->cmbOLCI18->addItem(tr("-"));ui->cmbOLCI18->addItem(tr("R"));ui->cmbOLCI18->addItem(tr("G"));ui->cmbOLCI18->addItem(tr("B"));
    ui->cmbOLCI19->addItem(tr("-"));ui->cmbOLCI19->addItem(tr("R"));ui->cmbOLCI19->addItem(tr("G"));ui->cmbOLCI19->addItem(tr("B"));
    ui->cmbOLCI20->addItem(tr("-"));ui->cmbOLCI20->addItem(tr("R"));ui->cmbOLCI20->addItem(tr("G"));ui->cmbOLCI20->addItem(tr("B"));
    ui->cmbOLCI21->addItem(tr("-"));ui->cmbOLCI21->addItem(tr("R"));ui->cmbOLCI21->addItem(tr("G"));ui->cmbOLCI21->addItem(tr("B"));

}

void FormToolbox::setChannelComboBoxes()
{
    qDebug() << "FormToolbox::setChannelComboBoxes()";

    disconnect(ui->chkInverseCh1, SIGNAL(stateChanged(int)), 0, 0);
    disconnect(ui->chkInverseCh2, SIGNAL(stateChanged(int)), 0, 0);
    disconnect(ui->chkInverseCh3, SIGNAL(stateChanged(int)), 0, 0);
    disconnect(ui->chkInverseCh4, SIGNAL(stateChanged(int)), 0, 0);
    disconnect(ui->chkInverseCh5, SIGNAL(stateChanged(int)), 0, 0);

    if (opts.buttonMetop || opts.buttonMetopAhrpt || opts.buttonMetopBhrpt || opts.buttonM01hrpt || opts.buttonM02hrpt)
    {
        qDebug() << "metop";
        ui->comboCh1->setCurrentIndex(opts.channellistmetop.at(0).toInt());
        ui->comboCh2->setCurrentIndex(opts.channellistmetop.at(1).toInt());
        ui->comboCh3->setCurrentIndex(opts.channellistmetop.at(2).toInt());
        ui->comboCh4->setCurrentIndex(opts.channellistmetop.at(3).toInt());
        ui->comboCh5->setCurrentIndex(opts.channellistmetop.at(4).toInt());

    } else
    if (opts.buttonNoaa || opts.buttonNoaa19hrpt)
    {
        qDebug() << "noaa";

        ui->comboCh1->setCurrentIndex(opts.channellistnoaa.at(0).toInt());
        ui->comboCh2->setCurrentIndex(opts.channellistnoaa.at(1).toInt());
        ui->comboCh3->setCurrentIndex(opts.channellistnoaa.at(2).toInt());
        ui->comboCh4->setCurrentIndex(opts.channellistnoaa.at(3).toInt());
        ui->comboCh5->setCurrentIndex(opts.channellistnoaa.at(4).toInt());

    } else
    if (opts.buttonGAC)
    {
        qDebug() << "GAC";

        ui->comboCh1->setCurrentIndex(opts.channellistgac.at(0).toInt());
        ui->comboCh2->setCurrentIndex(opts.channellistgac.at(1).toInt());
        ui->comboCh3->setCurrentIndex(opts.channellistgac.at(2).toInt());
        ui->comboCh4->setCurrentIndex(opts.channellistgac.at(3).toInt());
        ui->comboCh5->setCurrentIndex(opts.channellistgac.at(4).toInt());

    } else
    if (opts.buttonHRP)
    {
        qDebug() << "HRP";

        ui->comboCh1->setCurrentIndex(opts.channellisthrp.at(0).toInt());
        ui->comboCh2->setCurrentIndex(opts.channellisthrp.at(1).toInt());
        ui->comboCh3->setCurrentIndex(opts.channellisthrp.at(2).toInt());
        ui->comboCh4->setCurrentIndex(opts.channellisthrp.at(3).toInt());
        ui->comboCh5->setCurrentIndex(opts.channellisthrp.at(4).toInt());
    }

    setInverseCheckBoxes();

    connect(ui->chkInverseCh1, SIGNAL(stateChanged(int)), this, SLOT(setChannelInverse()));
    connect(ui->chkInverseCh2, SIGNAL(stateChanged(int)), this, SLOT(setChannelInverse()));
    connect(ui->chkInverseCh3, SIGNAL(stateChanged(int)), this, SLOT(setChannelInverse()));
    connect(ui->chkInverseCh4, SIGNAL(stateChanged(int)), this, SLOT(setChannelInverse()));
    connect(ui->chkInverseCh5, SIGNAL(stateChanged(int)), this, SLOT(setChannelInverse()));

    QStringList inpchannels;
    inpchannels << "Color   (avhrr)" << "Chan 1 (avhrr)" << "Chan 2 (avhrr)" << "Chan 3 (avhrr)" << "Chan 4 (avhrr)" << "Chan 5 (avhrr)";

    ui->cmbInputAVHRRChannel->clear();
    ui->cmbInputAVHRRChannel->addItems(inpchannels);


    qDebug() << QString("setChannelComboBoxes combo 1 channelindex = %1 %2").arg(ui->comboCh1->currentIndex()).arg(ui->comboCh1->currentText());
    qDebug() << QString("setChannelComboBoxes combo 2 channelindex = %1 %2").arg(ui->comboCh2->currentIndex()).arg(ui->comboCh2->currentText());
    qDebug() << QString("setChannelComboBoxes combo 3 channelindex = %1 %2").arg(ui->comboCh3->currentIndex()).arg(ui->comboCh3->currentText());
    qDebug() << QString("setChannelComboBoxes combo 4 channelindex = %1 %2").arg(ui->comboCh4->currentIndex()).arg(ui->comboCh4->currentText());
    qDebug() << QString("setChannelComboBoxes combo 5 channelindex = %1 %2").arg(ui->comboCh5->currentIndex()).arg(ui->comboCh5->currentText());

}

QList<bool> FormToolbox::getVIIRSMBandList()
{
    QList<bool> viirslist;
    viirslist << ui->rbColorVIIRS->isChecked() << ui->rbM1->isChecked() << ui->rbM2->isChecked() << ui->rbM3->isChecked() << ui->rbM4->isChecked() << ui->rbM5->isChecked() << ui->rbM6->isChecked()
                 << ui->rbM7->isChecked() << ui->rbM8->isChecked() << ui->rbM9->isChecked() << ui->rbM10->isChecked() << ui->rbM11->isChecked() << ui->rbM12->isChecked()
                    << ui->rbM13->isChecked() << ui->rbM14->isChecked() << ui->rbM15->isChecked() << ui->rbM16->isChecked();
    return(viirslist);
}

QList<int> FormToolbox::getVIIRSMColorList()
{
    QList<int> viirslist;
    viirslist << ui->comboM1->currentIndex() << ui->comboM2->currentIndex() << ui->comboM3->currentIndex() << ui->comboM4->currentIndex() << ui->comboM5->currentIndex()
                  << ui->comboM6->currentIndex() << ui->comboM7->currentIndex() << ui->comboM8->currentIndex() << ui->comboM9->currentIndex() << ui->comboM10->currentIndex()
                      << ui->comboM11->currentIndex() << ui->comboM12->currentIndex() << ui->comboM13->currentIndex() << ui->comboM14->currentIndex() << ui->comboM15->currentIndex()
                          << ui->comboM16->currentIndex();
    return(viirslist);
}

QList<bool> FormToolbox::getVIIRSMInvertList()
{
    QList<bool> viirslist;
    viirslist << ui->chkInverseM1->isChecked() << ui->chkInverseM2->isChecked() << ui->chkInverseM3->isChecked() << ui->chkInverseM4->isChecked() << ui->chkInverseM5->isChecked()
                  << ui->chkInverseM6->isChecked() << ui->chkInverseM7->isChecked() << ui->chkInverseM8->isChecked() << ui->chkInverseM9->isChecked() << ui->chkInverseM10->isChecked()
                      << ui->chkInverseM11->isChecked() << ui->chkInverseM12->isChecked() << ui->chkInverseM13->isChecked() << ui->chkInverseM14->isChecked() << ui->chkInverseM15->isChecked()
                          << ui->chkInverseM16->isChecked();
    return(viirslist);
}

QList<bool> FormToolbox::getOLCIBandList()
{
    QList<bool> olcilist;
    olcilist << ui->rbColorOLCI->isChecked() << ui->rbOLCI01->isChecked() << ui->rbOLCI02->isChecked() << ui->rbOLCI03->isChecked()
             << ui->rbOLCI04->isChecked() << ui->rbOLCI05->isChecked() << ui->rbOLCI06->isChecked() << ui->rbOLCI07->isChecked()
             << ui->rbOLCI08->isChecked() << ui->rbOLCI09->isChecked() << ui->rbOLCI10->isChecked() << ui->rbOLCI11->isChecked()
             << ui->rbOLCI12->isChecked() << ui->rbOLCI13->isChecked() << ui->rbOLCI14->isChecked() << ui->rbOLCI15->isChecked()
             << ui->rbOLCI16->isChecked() << ui->rbOLCI17->isChecked() << ui->rbOLCI18->isChecked() << ui->rbOLCI19->isChecked()
             << ui->rbOLCI20->isChecked() << ui->rbOLCI21->isChecked();


    Q_ASSERT(olcilist.count() == 22);

    return(olcilist);
}

QList<int> FormToolbox::getOLCIColorList()
{
    QList<int> olcilist;
    olcilist << ui->cmbOLCI01->currentIndex() << ui->cmbOLCI02->currentIndex() << ui->cmbOLCI03->currentIndex() << ui->cmbOLCI04->currentIndex()
             << ui->cmbOLCI05->currentIndex() << ui->cmbOLCI06->currentIndex() << ui->cmbOLCI07->currentIndex() << ui->cmbOLCI08->currentIndex()
             << ui->cmbOLCI09->currentIndex() << ui->cmbOLCI10->currentIndex() << ui->cmbOLCI11->currentIndex() << ui->cmbOLCI12->currentIndex()
             << ui->cmbOLCI13->currentIndex() << ui->cmbOLCI14->currentIndex() << ui->cmbOLCI15->currentIndex() << ui->cmbOLCI16->currentIndex()
             << ui->cmbOLCI17->currentIndex() << ui->cmbOLCI18->currentIndex() << ui->cmbOLCI19->currentIndex() << ui->cmbOLCI20->currentIndex()
             << ui->cmbOLCI21->currentIndex();
    Q_ASSERT(olcilist.count() == 21);

    return(olcilist);
}

QList<bool> FormToolbox::getOLCIInvertList()
{
    QList<bool> olcilist;
    olcilist << ui->chkInverseOLCI01->isChecked() << ui->chkInverseOLCI02->isChecked() << ui->chkInverseOLCI03->isChecked() << ui->chkInverseOLCI04->isChecked()
              << ui->chkInverseOLCI05->isChecked() << ui->chkInverseOLCI06->isChecked() << ui->chkInverseOLCI07->isChecked() << ui->chkInverseOLCI08->isChecked()
              << ui->chkInverseOLCI09->isChecked() << ui->chkInverseOLCI10->isChecked() << ui->chkInverseOLCI11->isChecked() << ui->chkInverseOLCI12->isChecked()
              << ui->chkInverseOLCI13->isChecked() << ui->chkInverseOLCI14->isChecked() << ui->chkInverseOLCI15->isChecked() << ui->chkInverseOLCI16->isChecked()
              << ui->chkInverseOLCI17->isChecked() << ui->chkInverseOLCI18->isChecked() << ui->chkInverseOLCI19->isChecked() << ui->chkInverseOLCI20->isChecked()
              << ui->chkInverseOLCI21->isChecked();
    Q_ASSERT(olcilist.count() == 21);
    return(olcilist);
}

int FormToolbox::searchResolution(int mapwidth, int mapheight)
{
   int index = -1;

   for(int i = 0; i < resolutionX.size(); i++)
   {
       if(resolutionX.at(i) == mapwidth && resolutionY.at(i) == mapheight )
       {
           index = i;
           break;
       }
   }

   return(index+1);

}

void FormToolbox::setMeteosatGamma()
{
    ui->spbMeteosatGamma->setValue((double)ui->sliMeteosatGamma->value()/100);
    opts.meteosatgamma = (double)ui->sliMeteosatGamma->value()/100;
}


void FormToolbox::setMeteosatGamma(double gammaval)
{
    double val = ui->spbMeteosatGamma->value() * 100;
    ui->sliMeteosatGamma->setValue((int)val);
    opts.meteosatgamma = gammaval;
    qDebug() << QString("opts.meteosatvalue = %1").arg(opts.meteosatgamma);

}

void FormToolbox::setInverseCheckBoxes()
{
    qDebug() << "FormToolbox::setInverseCheckBoxes()";

    if (opts.buttonMetop || opts.buttonMetopAhrpt || opts.buttonMetopBhrpt || opts.buttonM01hrpt || opts.buttonM02hrpt)
    {
        if (opts.metop_invlist.count() == 5)
        {
            ui->chkInverseCh1->setChecked(opts.metop_invlist.at(0) == "1" ? true : false);
            ui->chkInverseCh2->setChecked(opts.metop_invlist.at(1) == "1" ? true : false);
            ui->chkInverseCh3->setChecked(opts.metop_invlist.at(2) == "1" ? true : false);
            ui->chkInverseCh4->setChecked(opts.metop_invlist.at(3) == "1" ? true : false);
            ui->chkInverseCh5->setChecked(opts.metop_invlist.at(4) == "1" ? true : false);
        }
        else
        {
            ui->chkInverseCh1->setChecked(false);
            ui->chkInverseCh2->setChecked(false);
            ui->chkInverseCh3->setChecked(false);
            ui->chkInverseCh4->setChecked(false);
            ui->chkInverseCh5->setChecked(false);
        }

    } else
    if (opts.buttonNoaa || opts.buttonNoaa19hrpt)
    {
        if (opts.noaa_invlist.count() == 5)
        {
            ui->chkInverseCh1->setChecked(opts.noaa_invlist.at(0) == "1" ? true : false);
            ui->chkInverseCh2->setChecked(opts.noaa_invlist.at(1) == "1" ? true : false);
            ui->chkInverseCh3->setChecked(opts.noaa_invlist.at(2) == "1" ? true : false);
            ui->chkInverseCh4->setChecked(opts.noaa_invlist.at(3) == "1" ? true : false);
            ui->chkInverseCh5->setChecked(opts.noaa_invlist.at(4) == "1" ? true : false);
        }
        else
        {
            ui->chkInverseCh1->setChecked(false);
            ui->chkInverseCh2->setChecked(false);
            ui->chkInverseCh3->setChecked(false);
            ui->chkInverseCh4->setChecked(false);
            ui->chkInverseCh5->setChecked(false);
        }

    } else
    if (opts.buttonGAC)
    {
        if (opts.gac_invlist.count() == 5)
        {
            ui->chkInverseCh1->setChecked(opts.gac_invlist.at(0) == "1" ? true : false);
            ui->chkInverseCh2->setChecked(opts.gac_invlist.at(1) == "1" ? true : false);
            ui->chkInverseCh3->setChecked(opts.gac_invlist.at(2) == "1" ? true : false);
            ui->chkInverseCh4->setChecked(opts.gac_invlist.at(3) == "1" ? true : false);
            ui->chkInverseCh5->setChecked(opts.gac_invlist.at(4) == "1" ? true : false);
        }
        else
        {
            ui->chkInverseCh1->setChecked(false);
            ui->chkInverseCh2->setChecked(false);
            ui->chkInverseCh3->setChecked(false);
            ui->chkInverseCh4->setChecked(false);
            ui->chkInverseCh5->setChecked(false);
        }

    } else
    if (opts.buttonHRP)
    {
        if (opts.hrp_invlist.count() == 5)
        {
            ui->chkInverseCh1->setChecked(opts.hrp_invlist.at(0) == "1" ? true : false);
            ui->chkInverseCh2->setChecked(opts.hrp_invlist.at(1) == "1" ? true : false);
            ui->chkInverseCh3->setChecked(opts.hrp_invlist.at(2) == "1" ? true : false);
            ui->chkInverseCh4->setChecked(opts.hrp_invlist.at(3) == "1" ? true : false);
            ui->chkInverseCh5->setChecked(opts.hrp_invlist.at(4) == "1" ? true : false);
        }
        else
        {
            ui->chkInverseCh1->setChecked(false);
            ui->chkInverseCh2->setChecked(false);
            ui->chkInverseCh3->setChecked(false);
            ui->chkInverseCh4->setChecked(false);
            ui->chkInverseCh5->setChecked(false);
        }
    }
    qDebug() << QString("chkInverse = %1 %2 %3 %4 %5").arg(opts.noaa_invlist.at(0)).arg(opts.noaa_invlist.at(1)).arg(opts.noaa_invlist.at(2)).arg(opts.noaa_invlist.at(3)).arg(opts.noaa_invlist.at(4));
}

void FormToolbox::setChannelInverse()
{
    qDebug() << "FormToolbox::setChannelInverse()";

    if (opts.buttonMetop || opts.buttonMetopAhrpt || opts.buttonMetopBhrpt || opts.buttonM01hrpt || opts.buttonM02hrpt)
    {
        opts.metop_invlist.clear();

        opts.metop_invlist << (ui->chkInverseCh1->isChecked() ? "1" : "0");
        opts.metop_invlist << (ui->chkInverseCh2->isChecked() ? "1" : "0");
        opts.metop_invlist << (ui->chkInverseCh3->isChecked() ? "1" : "0");
        opts.metop_invlist << (ui->chkInverseCh4->isChecked() ? "1" : "0");
        opts.metop_invlist << (ui->chkInverseCh5->isChecked() ? "1" : "0");
    } else
    if (opts.buttonNoaa || opts.buttonNoaa19hrpt)
    {
        opts.noaa_invlist.clear();

        opts.noaa_invlist << (ui->chkInverseCh1->isChecked() ? "1" : "0");
        opts.noaa_invlist << (ui->chkInverseCh2->isChecked() ? "1" : "0");
        opts.noaa_invlist << (ui->chkInverseCh3->isChecked() ? "1" : "0");
        opts.noaa_invlist << (ui->chkInverseCh4->isChecked() ? "1" : "0");
        opts.noaa_invlist << (ui->chkInverseCh5->isChecked() ? "1" : "0");
    } else
    if (opts.buttonGAC)
    {
        opts.gac_invlist.clear();

        opts.gac_invlist << (ui->chkInverseCh1->isChecked() ? "1" : "0");
        opts.gac_invlist << (ui->chkInverseCh2->isChecked() ? "1" : "0");
        opts.gac_invlist << (ui->chkInverseCh3->isChecked() ? "1" : "0");
        opts.gac_invlist << (ui->chkInverseCh4->isChecked() ? "1" : "0");
        opts.gac_invlist << (ui->chkInverseCh5->isChecked() ? "1" : "0");
    } else
    if (opts.buttonHRP)
    {
        opts.hrp_invlist.clear();

        opts.hrp_invlist << (ui->chkInverseCh1->isChecked() ? "1" : "0");
        opts.hrp_invlist << (ui->chkInverseCh2->isChecked() ? "1" : "0");
        opts.hrp_invlist << (ui->chkInverseCh3->isChecked() ? "1" : "0");
        opts.hrp_invlist << (ui->chkInverseCh4->isChecked() ? "1" : "0");
        opts.hrp_invlist << (ui->chkInverseCh5->isChecked() ? "1" : "0");
    }

}

void FormToolbox::setChannelIndex()
{
    qDebug() << "FormToolbox::setChannelIndex()";

    if (opts.buttonMetop || opts.buttonMetopAhrpt || opts.buttonMetopBhrpt || opts.buttonM01hrpt || opts.buttonM02hrpt)
    {
        opts.channellistmetop.clear();
        opts.channellistmetop << QString("%1").arg(ui->comboCh1->currentIndex());
        opts.channellistmetop << QString("%1").arg(ui->comboCh2->currentIndex());
        opts.channellistmetop << QString("%1").arg(ui->comboCh3->currentIndex());
        opts.channellistmetop << QString("%1").arg(ui->comboCh4->currentIndex());
        opts.channellistmetop << QString("%1").arg(ui->comboCh5->currentIndex());
    }
    else
    if (opts.buttonNoaa || opts.buttonNoaa19hrpt)
    {
        opts.channellistnoaa.clear();
        opts.channellistnoaa << QString("%1").arg(ui->comboCh1->currentIndex());
        opts.channellistnoaa << QString("%1").arg(ui->comboCh2->currentIndex());
        opts.channellistnoaa << QString("%1").arg(ui->comboCh3->currentIndex());
        opts.channellistnoaa << QString("%1").arg(ui->comboCh4->currentIndex());
        opts.channellistnoaa << QString("%1").arg(ui->comboCh5->currentIndex());
    }
    else
    if (opts.buttonGAC)
    {
        opts.channellistgac.clear();
        opts.channellistgac << QString("%1").arg(ui->comboCh1->currentIndex());
        opts.channellistgac << QString("%1").arg(ui->comboCh2->currentIndex());
        opts.channellistgac << QString("%1").arg(ui->comboCh3->currentIndex());
        opts.channellistgac << QString("%1").arg(ui->comboCh4->currentIndex());
        opts.channellistgac << QString("%1").arg(ui->comboCh5->currentIndex());

    }
    else
    if (opts.buttonHRP)
    {
        opts.channellisthrp.clear();
        opts.channellisthrp << QString("%1").arg(ui->comboCh1->currentIndex());
        opts.channellisthrp << QString("%1").arg(ui->comboCh2->currentIndex());
        opts.channellisthrp << QString("%1").arg(ui->comboCh3->currentIndex());
        opts.channellisthrp << QString("%1").arg(ui->comboCh4->currentIndex());
        opts.channellisthrp << QString("%1").arg(ui->comboCh5->currentIndex());
    }

    qDebug() << QString("setChannelIndex combo 1 channelindex = %1 %2").arg(ui->comboCh1->currentIndex()).arg(ui->comboCh1->currentText());
    qDebug() << QString("setChannelIndex combo 2 channelindex = %1 %2").arg(ui->comboCh2->currentIndex()).arg(ui->comboCh2->currentText());
    qDebug() << QString("setChannelIndex combo 3 channelindex = %1 %2").arg(ui->comboCh3->currentIndex()).arg(ui->comboCh3->currentText());
    qDebug() << QString("setChannelIndex combo 4 channelindex = %1 %2").arg(ui->comboCh4->currentIndex()).arg(ui->comboCh4->currentText());
    qDebug() << QString("setChannelIndex combo 5 channelindex = %1 %2").arg(ui->comboCh5->currentIndex()).arg(ui->comboCh5->currentText());

}

FormToolbox::~FormToolbox()
{
    qDebug() << "closing FormToolbox";
    if(ui->rdbAVHRRin->isChecked())
        opts.lastinputprojection = 0;
    else if(ui->rdbVIIRSMin->isChecked())
        opts.lastinputprojection = 1;
    else if(ui->rdbVIIRSDNBin->isChecked())
        opts.lastinputprojection = 2;
    else if(ui->rdbOLCIefrin->isChecked())
        opts.lastinputprojection = 3;
    else if(ui->rdbOLCIerrin->isChecked())
        opts.lastinputprojection = 3;
    else
        opts.lastinputprojection = 5;


    opts.lastcomboMet006 = ui->comboGeo1->currentIndex();
    opts.lastcomboMet008 = ui->comboGeo2->currentIndex();
    opts.lastcomboMet016 = ui->comboGeo3->currentIndex();
    opts.lastcomboMet039 = ui->comboGeo4->currentIndex();
    opts.lastcomboMet062 = ui->comboGeo5->currentIndex();
    opts.lastcomboMet073 = ui->comboGeo6->currentIndex();
    opts.lastcomboMet087 = ui->comboGeo7->currentIndex();
    opts.lastcomboMet097 = ui->comboGeo8->currentIndex();
    opts.lastcomboMet108 = ui->comboGeo9->currentIndex();
    opts.lastcomboMet120 = ui->comboGeo10->currentIndex();
    opts.lastcomboMet134 = ui->comboGeo11->currentIndex();

    opts.lastinverseMet006 = ui->chkInverseGeo1->isChecked();
    opts.lastinverseMet008 = ui->chkInverseGeo2->isChecked();
    opts.lastinverseMet016 = ui->chkInverseGeo3->isChecked();
    opts.lastinverseMet039 = ui->chkInverseGeo4->isChecked();
    opts.lastinverseMet062 = ui->chkInverseGeo5->isChecked();
    opts.lastinverseMet073 = ui->chkInverseGeo6->isChecked();
    opts.lastinverseMet087 = ui->chkInverseGeo7->isChecked();
    opts.lastinverseMet097 = ui->chkInverseGeo8->isChecked();
    opts.lastinverseMet108 = ui->chkInverseGeo9->isChecked();
    opts.lastinverseMet120 = ui->chkInverseGeo10->isChecked();
    opts.lastinverseMet134 = ui->chkInverseGeo11->isChecked();


    opts.currenttabwidget = ui->tabWidget->currentIndex();
    opts.currenttoolbox = ui->toolBox->currentIndex();


    poi.strlLCCParallel1.replace(0, QString("%1").arg(ui->spbParallel1->value()));
    poi.strlLCCParallel2.replace(0, QString("%1").arg(ui->spbParallel2->value()));
    poi.strlLCCCentral.replace(0, QString("%1").arg(ui->spbCentral->value()));
    poi.strlLCCLatOrigin.replace(0, QString("%1").arg(ui->spbLatOrigin->value()));
    poi.strlLCCNorth.replace(0, QString("%1").arg(ui->spbNorth->value()));
    poi.strlLCCSouth.replace(0, QString("%1").arg(ui->spbSouth->value()));
    poi.strlLCCEast.replace(0, QString("%1").arg(ui->spbEast->value()));
    poi.strlLCCWest.replace(0, QString("%1").arg(ui->spbWest->value()));
    poi.strlLCCScaleX.replace(0, QString("%1").arg(ui->spbScaleX->value(), 0, 'f', 2));
    poi.strlLCCScaleY.replace(0, QString("%1").arg(ui->spbScaleY->value(), 0, 'f', 2));
    poi.strlLCCMapHeight.replace(0, QString("%1").arg(ui->spbLCCMapHeight->value()));
    poi.strlLCCMapWidth.replace(0, QString("%1").arg(ui->spbLCCMapWidth->value()));
    poi.strlLCCGridOnProj.replace(0, QString("%1").arg(ui->chkLCCGridOnProj->isChecked()));

    poi.strlGVPLat.replace(0, QString("%1").arg(ui->spbGVPlat->value(), 0, 'f', 2));
    poi.strlGVPLon.replace(0, QString("%1").arg(ui->spbGVPlon->value(), 0, 'f', 2));
    poi.strlGVPScale.replace(0, QString("%1").arg(ui->spbGVPscale->value(), 0, 'f', 2));
    poi.strlGVPHeight.replace(0, QString("%1").arg(ui->spbGVPheight->value()));
    poi.strlGVPMapHeight.replace(0, QString("%1").arg(ui->spbGVPMapHeight->value()));
    poi.strlGVPMapWidth.replace(0, QString("%1").arg(ui->spbGVPMapWidth->value()));
    poi.strlGVPGridOnProj.replace(0, QString("%1").arg(ui->chkGVPGridOnProj->isChecked()));


    poi.strlSGLat.replace(0, QString("%1").arg(ui->spbSGlat->value(), 0, 'f', 2));
    poi.strlSGLon.replace(0, QString("%1").arg(ui->spbSGlon->value(), 0, 'f', 2));
    poi.strlSGRadius.replace(0, QString("%1").arg(ui->spbSGRadius->value(), 0, 'f', 2));
    poi.strlSGScale.replace(0, QString("%1").arg(ui->spbSGScale->value(), 0, 'f', 2));
    poi.strlSGPanH.replace(0, QString("%1").arg(ui->spbSGPanHorizon->value()));
    poi.strlSGPanV.replace(0, QString("%1").arg(ui->spbSGPanVert->value()));
    poi.strlSGMapHeight.replace(0, QString("%1").arg(ui->spbSGMapHeight->value()));
    poi.strlSGMapWidth.replace(0, QString("%1").arg(ui->spbSGMapWidth->value()));
    poi.strlSGGridOnProj.replace(0, QString("%1").arg(ui->chkSGGridOnProj->isChecked()));

    if(ui->rbColorVIIRS->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("0"));
    else if(ui->rbM1->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("1"));
    else if(ui->rbM2->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("2"));
    else if(ui->rbM3->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("3"));
    else if(ui->rbM4->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("4"));
    else if(ui->rbM5->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("5"));
    else if(ui->rbM6->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("6"));
    else if(ui->rbM7->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("7"));
    else if(ui->rbM8->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("8"));
    else if(ui->rbM9->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("9"));
    else if(ui->rbM10->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("10"));
    else if(ui->rbM11->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("11"));
    else if(ui->rbM12->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("12"));
    else if(ui->rbM13->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("13"));
    else if(ui->rbM14->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("14"));
    else if(ui->rbM15->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("15"));
    else if(ui->rbM16->isChecked())
        poi.strlColorBandM.replace(0, QString("%1").arg("16"));


    poi.strlInverseM1.replace(0, QString("%1").arg(ui->chkInverseM1->isChecked()));
    poi.strlInverseM2.replace(0, QString("%1").arg(ui->chkInverseM2->isChecked()));
    poi.strlInverseM3.replace(0, QString("%1").arg(ui->chkInverseM3->isChecked()));
    poi.strlInverseM4.replace(0, QString("%1").arg(ui->chkInverseM4->isChecked()));
    poi.strlInverseM5.replace(0, QString("%1").arg(ui->chkInverseM5->isChecked()));
    poi.strlInverseM6.replace(0, QString("%1").arg(ui->chkInverseM6->isChecked()));
    poi.strlInverseM7.replace(0, QString("%1").arg(ui->chkInverseM7->isChecked()));
    poi.strlInverseM8.replace(0, QString("%1").arg(ui->chkInverseM8->isChecked()));
    poi.strlInverseM9.replace(0, QString("%1").arg(ui->chkInverseM9->isChecked()));
    poi.strlInverseM10.replace(0, QString("%1").arg(ui->chkInverseM10->isChecked()));
    poi.strlInverseM11.replace(0, QString("%1").arg(ui->chkInverseM11->isChecked()));
    poi.strlInverseM12.replace(0, QString("%1").arg(ui->chkInverseM12->isChecked()));
    poi.strlInverseM13.replace(0, QString("%1").arg(ui->chkInverseM13->isChecked()));
    poi.strlInverseM14.replace(0, QString("%1").arg(ui->chkInverseM14->isChecked()));
    poi.strlInverseM15.replace(0, QString("%1").arg(ui->chkInverseM15->isChecked()));
    poi.strlInverseM16.replace(0, QString("%1").arg(ui->chkInverseM16->isChecked()));

    poi.strlComboM1.replace(0, QString("%1").arg(ui->comboM1->currentIndex()));
    poi.strlComboM2.replace(0, QString("%1").arg(ui->comboM2->currentIndex()));
    poi.strlComboM3.replace(0, QString("%1").arg(ui->comboM3->currentIndex()));
    poi.strlComboM4.replace(0, QString("%1").arg(ui->comboM4->currentIndex()));
    poi.strlComboM5.replace(0, QString("%1").arg(ui->comboM5->currentIndex()));
    poi.strlComboM6.replace(0, QString("%1").arg(ui->comboM6->currentIndex()));
    poi.strlComboM7.replace(0, QString("%1").arg(ui->comboM7->currentIndex()));
    poi.strlComboM8.replace(0, QString("%1").arg(ui->comboM8->currentIndex()));
    poi.strlComboM9.replace(0, QString("%1").arg(ui->comboM9->currentIndex()));
    poi.strlComboM10.replace(0, QString("%1").arg(ui->comboM10->currentIndex()));
    poi.strlComboM11.replace(0, QString("%1").arg(ui->comboM11->currentIndex()));
    poi.strlComboM12.replace(0, QString("%1").arg(ui->comboM12->currentIndex()));
    poi.strlComboM13.replace(0, QString("%1").arg(ui->comboM13->currentIndex()));
    poi.strlComboM14.replace(0, QString("%1").arg(ui->comboM14->currentIndex()));
    poi.strlComboM15.replace(0, QString("%1").arg(ui->comboM15->currentIndex()));
    poi.strlComboM16.replace(0, QString("%1").arg(ui->comboM16->currentIndex()));


    if(ui->rbColorOLCI->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("0"));
    else if(ui->rbOLCI01->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("1"));
    else if(ui->rbOLCI02->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("2"));
    else if(ui->rbOLCI03->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("3"));
    else if(ui->rbOLCI04->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("4"));
    else if(ui->rbOLCI05->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("5"));
    else if(ui->rbOLCI06->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("6"));
    else if(ui->rbOLCI07->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("7"));
    else if(ui->rbOLCI08->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("8"));
    else if(ui->rbOLCI09->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("9"));
    else if(ui->rbOLCI10->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("10"));
    else if(ui->rbOLCI11->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("11"));
    else if(ui->rbOLCI12->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("12"));
    else if(ui->rbOLCI13->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("13"));
    else if(ui->rbOLCI14->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("14"));
    else if(ui->rbOLCI15->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("15"));
    else if(ui->rbOLCI16->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("16"));
    else if(ui->rbOLCI17->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("17"));
    else if(ui->rbOLCI18->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("18"));
    else if(ui->rbOLCI19->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("19"));
    else if(ui->rbOLCI20->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("20"));
    else if(ui->rbOLCI21->isChecked())
        poi.strlColorBandOLCI.replace(0, QString("%1").arg("21"));


    poi.strlInverseOLCI01.replace(0, QString("%1").arg(ui->chkInverseOLCI01->isChecked()));
    poi.strlInverseOLCI02.replace(0, QString("%1").arg(ui->chkInverseOLCI02->isChecked()));
    poi.strlInverseOLCI03.replace(0, QString("%1").arg(ui->chkInverseOLCI03->isChecked()));
    poi.strlInverseOLCI04.replace(0, QString("%1").arg(ui->chkInverseOLCI04->isChecked()));
    poi.strlInverseOLCI05.replace(0, QString("%1").arg(ui->chkInverseOLCI05->isChecked()));
    poi.strlInverseOLCI06.replace(0, QString("%1").arg(ui->chkInverseOLCI06->isChecked()));
    poi.strlInverseOLCI07.replace(0, QString("%1").arg(ui->chkInverseOLCI07->isChecked()));
    poi.strlInverseOLCI08.replace(0, QString("%1").arg(ui->chkInverseOLCI08->isChecked()));
    poi.strlInverseOLCI09.replace(0, QString("%1").arg(ui->chkInverseOLCI09->isChecked()));
    poi.strlInverseOLCI10.replace(0, QString("%1").arg(ui->chkInverseOLCI10->isChecked()));
    poi.strlInverseOLCI11.replace(0, QString("%1").arg(ui->chkInverseOLCI11->isChecked()));
    poi.strlInverseOLCI12.replace(0, QString("%1").arg(ui->chkInverseOLCI12->isChecked()));
    poi.strlInverseOLCI13.replace(0, QString("%1").arg(ui->chkInverseOLCI13->isChecked()));
    poi.strlInverseOLCI14.replace(0, QString("%1").arg(ui->chkInverseOLCI14->isChecked()));
    poi.strlInverseOLCI15.replace(0, QString("%1").arg(ui->chkInverseOLCI15->isChecked()));
    poi.strlInverseOLCI16.replace(0, QString("%1").arg(ui->chkInverseOLCI16->isChecked()));
    poi.strlInverseOLCI17.replace(0, QString("%1").arg(ui->chkInverseOLCI17->isChecked()));
    poi.strlInverseOLCI18.replace(0, QString("%1").arg(ui->chkInverseOLCI18->isChecked()));
    poi.strlInverseOLCI19.replace(0, QString("%1").arg(ui->chkInverseOLCI19->isChecked()));
    poi.strlInverseOLCI20.replace(0, QString("%1").arg(ui->chkInverseOLCI20->isChecked()));
    poi.strlInverseOLCI21.replace(0, QString("%1").arg(ui->chkInverseOLCI21->isChecked()));

    poi.strlComboOLCI01.replace(0, QString("%1").arg(ui->cmbOLCI01->currentIndex()));
    poi.strlComboOLCI02.replace(0, QString("%1").arg(ui->cmbOLCI02->currentIndex()));
    poi.strlComboOLCI03.replace(0, QString("%1").arg(ui->cmbOLCI03->currentIndex()));
    poi.strlComboOLCI04.replace(0, QString("%1").arg(ui->cmbOLCI04->currentIndex()));
    poi.strlComboOLCI05.replace(0, QString("%1").arg(ui->cmbOLCI05->currentIndex()));
    poi.strlComboOLCI06.replace(0, QString("%1").arg(ui->cmbOLCI06->currentIndex()));
    poi.strlComboOLCI07.replace(0, QString("%1").arg(ui->cmbOLCI07->currentIndex()));
    poi.strlComboOLCI08.replace(0, QString("%1").arg(ui->cmbOLCI08->currentIndex()));
    poi.strlComboOLCI09.replace(0, QString("%1").arg(ui->cmbOLCI09->currentIndex()));
    poi.strlComboOLCI10.replace(0, QString("%1").arg(ui->cmbOLCI10->currentIndex()));
    poi.strlComboOLCI11.replace(0, QString("%1").arg(ui->cmbOLCI11->currentIndex()));
    poi.strlComboOLCI12.replace(0, QString("%1").arg(ui->cmbOLCI12->currentIndex()));
    poi.strlComboOLCI13.replace(0, QString("%1").arg(ui->cmbOLCI13->currentIndex()));
    poi.strlComboOLCI14.replace(0, QString("%1").arg(ui->cmbOLCI14->currentIndex()));
    poi.strlComboOLCI15.replace(0, QString("%1").arg(ui->cmbOLCI15->currentIndex()));
    poi.strlComboOLCI16.replace(0, QString("%1").arg(ui->cmbOLCI16->currentIndex()));
    poi.strlComboOLCI17.replace(0, QString("%1").arg(ui->cmbOLCI17->currentIndex()));
    poi.strlComboOLCI18.replace(0, QString("%1").arg(ui->cmbOLCI18->currentIndex()));
    poi.strlComboOLCI19.replace(0, QString("%1").arg(ui->cmbOLCI19->currentIndex()));
    poi.strlComboOLCI20.replace(0, QString("%1").arg(ui->cmbOLCI20->currentIndex()));
    poi.strlComboOLCI21.replace(0, QString("%1").arg(ui->cmbOLCI21->currentIndex()));


    delete ui;
}

bool FormToolbox::GridOnProjLCC()
{
    return ui->chkLCCGridOnProj->isChecked();
}

bool FormToolbox::GridOnProjGVP()
{
    return ui->chkGVPGridOnProj->isChecked();
}

bool FormToolbox::GridOnProjSG()
{
    return ui->chkSGGridOnProj->isChecked();
}

void FormToolbox::on_btnCol_clicked()
{
    formimage->setKindOfImage("AVHRR Color");
    currentAVHRRimage = IMAGE_AVHRR_COL;
    formimage->displayImage(currentAVHRRimage);
}

void FormToolbox::on_btnCh1_clicked()
{
    formimage->setKindOfImage("Chan 1");
    currentAVHRRimage = IMAGE_AVHRR_CH1;
    formimage->displayImage(currentAVHRRimage);
}

void FormToolbox::on_btnCh2_clicked()
{
    formimage->setKindOfImage("Chan 2");
    currentAVHRRimage = IMAGE_AVHRR_CH2;
    formimage->displayImage(currentAVHRRimage);
}

void FormToolbox::on_btnCh3_clicked()
{
    formimage->setKindOfImage("Chan 3");
    currentAVHRRimage = IMAGE_AVHRR_CH3;
    formimage->displayImage(currentAVHRRimage);
}

void FormToolbox::on_btnCh4_clicked()
{
    formimage->setKindOfImage("Chan 4");
    currentAVHRRimage = IMAGE_AVHRR_CH4;
    formimage->displayImage(currentAVHRRimage);
}

void FormToolbox::on_btnCh5_clicked()
{
    formimage->setKindOfImage("Chan 5");
    currentAVHRRimage = IMAGE_AVHRR_CH5;
    formimage->displayImage(currentAVHRRimage);
}

void FormToolbox::on_btnExpandImage_clicked()
{
    if(formimage->channelshown == IMAGE_AVHRR_EXPAND)
        return;
    formimage->setKindOfImage("Expanded " + formimage->getKindOfImage());
    imageptrs->ExpandImage(formimage->channelshown);
    formimage->displayImage(IMAGE_AVHRR_EXPAND);
}

void FormToolbox::on_btnRotate180_clicked()
{
    imageptrs->RotateImage();
    formimage->returnimageLabelptr()->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_col)));
    formimage->adjustImage();
}

void FormToolbox::on_btnOverlayMeteosat_clicked()
{
    if(formimage->toggleOverlayMeteosat())
        ui->btnOverlayMeteosat->setText("Overlay On");
    else
        ui->btnOverlayMeteosat->setText("Overlay Off");
}

void FormToolbox::on_btnOverlayOLCI_clicked()
{
    if(formimage->toggleOverlayOLCI())
        ui->btnOverlayOLCI->setText("Overlay On");
    else
        ui->btnOverlayOLCI->setText("Overlay Off");

}

void FormToolbox::on_btnOverlayProjectionGVP_clicked()
{
    if(formimage->toggleOverlayProjection())
        ui->btnOverlayProjectionGVP->setText("Overlay On");
    else
        ui->btnOverlayProjectionGVP->setText("Overlay Off");

}

void FormToolbox::on_btnOverlayProjectionLCC_clicked()
{
    if(formimage->toggleOverlayProjection())
        ui->btnOverlayProjectionLCC->setText("Overlay On");
    else
        ui->btnOverlayProjectionLCC->setText("Overlay Off");
}

void FormToolbox::on_btnOverlayProjectionSG_clicked()
{
    if(formimage->toggleOverlayProjection())
        ui->btnOverlayProjectionSG->setText("Overlay On");
    else
        ui->btnOverlayProjectionSG->setText("Overlay Off");

}

void FormToolbox::geostationarysegmentsChosen(SegmentListGeostationary::eGeoSatellite geo, QStringList tex)
{
    whichgeo = geo;
    rowchosen = tex;
    ui->lblMeteosatChosen->setText(tex.at(0) + " " + tex.at(1) );
    ui->tabWidget->setCurrentIndex(TAB_GEOSTATIONARY);

    this->setToolboxButtons(true);

    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8 ||
        whichgeo == SegmentListGeostationary::MET_7 || whichgeo == SegmentListGeostationary::GOES_13 ||
        whichgeo == SegmentListGeostationary::GOES_15 )
    {
        ui->btnGeo1->setText("VIS006");
        ui->btnGeo2->setText("VIS008");
        ui->btnGeo3->setText("IR_016");
        ui->btnGeo4->setText("IR_039");
        ui->btnGeo5->setText("WV_062");
        ui->btnGeo6->setText("WV_073");
        ui->btnGeo7->setText("IR_087");
        ui->btnGeo8->setText("IR_097");
        ui->btnGeo9->setText("IR_108");
        ui->btnGeo10->setText("IR_120");
        ui->btnGeo11->setText("IR_134");
        ui->btnGeo12->setText("");
        ui->btnGeo13->setText("");
        ui->btnGeo14->setText("");
        ui->btnGeo15->setText("");
        ui->btnGeo16->setText("");
        ui->btnHRV->setText("HRV");
    }

    if(whichgeo == SegmentListGeostationary::MET_7)
    {
        if(rowchosen.at(3).toInt() == 0)
            ui->btnGeo2->setEnabled(false);
        if(rowchosen.at(4).toInt() == 0)
            ui->btnGeo5->setEnabled(false);
        if(rowchosen.at(5).toInt() == 0)
            ui->btnGeo10->setEnabled(false);
    }
    else if(whichgeo == SegmentListGeostationary::GOES_13 || whichgeo == SegmentListGeostationary::GOES_15)
    {
        if(rowchosen.at(3).toInt() == 0)
            ui->btnGeo2->setEnabled(false);
        if(rowchosen.at(4).toInt() == 0)
            ui->btnGeo4->setEnabled(false);
        if(rowchosen.at(5).toInt() == 0)
            ui->btnGeo5->setEnabled(false);
        if(rowchosen.at(6).toInt() == 0)
            ui->btnGeo9->setEnabled(false);
    }
    else if(whichgeo == SegmentListGeostationary::GOES_16)
    {

        if(rowchosen.at(15).toInt() == 0)
            ui->btnGeo1->setEnabled(false);
        if(rowchosen.at(7).toInt() == 0)
            ui->btnGeo2->setEnabled(false);
        if(rowchosen.at(8).toInt() == 0)
            ui->btnGeo3->setEnabled(false);
        if(rowchosen.at(9).toInt() == 0)
            ui->btnGeo4->setEnabled(false);
        if(rowchosen.at(6).toInt() == 0)
            ui->btnGeo5->setEnabled(false);
        if(rowchosen.at(5).toInt() == 0)
            ui->btnGeo6->setEnabled(false);
        if(rowchosen.at(10).toInt() == 0)
            ui->btnGeo7->setEnabled(false);
        if(rowchosen.at(11).toInt() == 0)
            ui->btnGeo8->setEnabled(false);
        if(rowchosen.at(12).toInt() == 0)
            ui->btnGeo9->setEnabled(false);
        if(rowchosen.at(3).toInt() == 0)
            ui->btnGeo10->setEnabled(false);
        if(rowchosen.at(13).toInt() == 0)
            ui->btnGeo11->setEnabled(false);
        if(rowchosen.at(4).toInt() == 0)
            ui->btnGeo12->setEnabled(false);
        if(rowchosen.at(14).toInt() == 0)
            ui->btnGeo13->setEnabled(false);
        if(rowchosen.at(16).toInt() == 0)
            ui->btnGeo14->setEnabled(false);
        if(rowchosen.at(17).toInt() == 0)
            ui->btnGeo15->setEnabled(false);
        if(rowchosen.at(18).toInt() == 0)
            ui->btnGeo16->setEnabled(false);

        ui->btnGeo1->setText("C01");
        ui->btnGeo2->setText("C02");
        ui->btnGeo3->setText("C03");
        ui->btnGeo4->setText("C04");
        ui->btnGeo5->setText("C05");
        ui->btnGeo6->setText("C06");
        ui->btnGeo7->setText("C07");
        ui->btnGeo8->setText("C08");
        ui->btnGeo9->setText("C09");
        ui->btnGeo10->setText("C10");
        ui->btnGeo11->setText("C11");
        ui->btnGeo12->setText("C12");
        ui->btnGeo13->setText("C13");
        ui->btnGeo14->setText("C14");
        ui->btnGeo15->setText("C15");
        ui->btnGeo16->setText("C16");
        ui->btnHRV->setText("");
    }
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
    {
        if(rowchosen.at(3).toInt() == 0)
            ui->btnGeo1->setEnabled(false);
        if(rowchosen.at(4).toInt() == 0)
            ui->btnGeo2->setEnabled(false);
        if(rowchosen.at(5).toInt() == 0)
            ui->btnGeo3->setEnabled(false);
        if(rowchosen.at(6).toInt() == 0)
            ui->btnGeo4->setEnabled(false);
        if(rowchosen.at(7).toInt() == 0)
            ui->btnGeo5->setEnabled(false);
        if(rowchosen.at(8).toInt() == 0)
            ui->btnHRV->setEnabled(false);
        ui->btnGeo1->setText("VIS");
        ui->btnGeo2->setText("IR4");
        ui->btnGeo3->setText("IR3");
        ui->btnGeo4->setText("IR1");
        ui->btnGeo5->setText("IR2");
        ui->btnGeo6->setText("");
        ui->btnGeo7->setText("");
        ui->btnGeo8->setText("");
        ui->btnGeo9->setText("");
        ui->btnGeo10->setText("");
        ui->btnGeo11->setText("");
        ui->btnGeo12->setText("");
        ui->btnGeo13->setText("");
        ui->btnHRV->setText("VIS1KM");
    }
    else if(whichgeo == SegmentListGeostationary::H8)
    {

        if(rowchosen.at(15).toInt() == 0)
            ui->btnGeo1->setEnabled(false);
        if(rowchosen.at(7).toInt() == 0)
            ui->btnGeo2->setEnabled(false);
        if(rowchosen.at(8).toInt() == 0)
            ui->btnGeo3->setEnabled(false);
        if(rowchosen.at(9).toInt() == 0)
            ui->btnGeo4->setEnabled(false);
        if(rowchosen.at(6).toInt() == 0)
            ui->btnGeo5->setEnabled(false);
        if(rowchosen.at(5).toInt() == 0)
            ui->btnGeo6->setEnabled(false);
        if(rowchosen.at(10).toInt() == 0)
            ui->btnGeo7->setEnabled(false);
        if(rowchosen.at(11).toInt() == 0)
            ui->btnGeo8->setEnabled(false);
        if(rowchosen.at(12).toInt() == 0)
            ui->btnGeo9->setEnabled(false);
        if(rowchosen.at(3).toInt() == 0)
            ui->btnGeo10->setEnabled(false);
        if(rowchosen.at(13).toInt() == 0)
            ui->btnGeo11->setEnabled(false);
        if(rowchosen.at(4).toInt() == 0)
            ui->btnGeo12->setEnabled(false);
        if(rowchosen.at(14).toInt() == 0)
            ui->btnGeo13->setEnabled(false);

        ui->btnGeo1->setText("VIS");
        ui->btnGeo2->setText("B04");
        ui->btnGeo3->setText("B05");
        ui->btnGeo4->setText("B06");
        ui->btnGeo5->setText("IR4");
        ui->btnGeo6->setText("IR3");
        ui->btnGeo7->setText("B09");
        ui->btnGeo8->setText("B10");
        ui->btnGeo9->setText("B11");
        ui->btnGeo10->setText("IR1");
        ui->btnGeo11->setText("B14");
        ui->btnGeo12->setText("IR2");
        ui->btnGeo13->setText("B16");
        ui->btnHRV->setText("");
    }

}

void FormToolbox::on_btnCLAHEMeteosat_clicked()
{

    if (segs->seglmeteosat->getKindofImage().length() == 0 )
       return;
    if(formimage->channelshown == IMAGE_GEOSTATIONARY)
    {
        this->setToolboxButtons(false);
        QApplication::processEvents();
        formimage->recalculateCLAHE(spectrumvector, inversevector);
        formimage->slotUpdateGeosat();
    }
}

void FormToolbox::on_btnCLAHEavhhr_clicked()
{
    formimage->recalculateCLAHEAvhrr(spectrumvector, inversevector);

    //segs->nb->p->ptrbaChannel[5].
    qDebug() << "CLAHE";
}

void FormToolbox::setTabWidgetIndex(int index)
{
    qDebug() << "FormToolbox::setTabWidgetIndex(int index)";
    ui->tabWidget->setCurrentIndex(index);
}

void FormToolbox::setTabWidgetVIIRSIndex(int index)
{
    qDebug() << "FormToolbox::setTabWidgetVIIRSIndex(int index)";
    ui->tabWidgetVIIRS->setCurrentIndex(index);
}

void FormToolbox::setToolboxButtons(bool state)
{
    qDebug() << QString("FormToolbox::setToolboxButtons state = %1").arg(state);

    ui->btnCh1->setEnabled(state);
    ui->btnCh2->setEnabled(state);
    ui->btnCh3->setEnabled(state);
    ui->btnCh4->setEnabled(state);
    ui->btnCh5->setEnabled(state);
    ui->btnCol->setEnabled(state);

    switch(whichgeo)
    {
    case SegmentListGeostationary::MET_7:
        ui->btnGeoColor->setEnabled(false);
        ui->btnGeo1->setEnabled(false);
        ui->btnGeo2->setEnabled(state);
        ui->btnGeo3->setEnabled(false);
        ui->btnGeo4->setEnabled(false);
        ui->btnGeo5->setEnabled(state);
        ui->btnGeo6->setEnabled(false);
        ui->btnGeo7->setEnabled(false);
        ui->btnGeo8->setEnabled(false);
        ui->btnGeo9->setEnabled(false);
        ui->btnGeo10->setEnabled(state);
        ui->btnGeo11->setEnabled(false);
        ui->btnGeo12->setEnabled(false);
        ui->btnGeo13->setEnabled(false);
        ui->btnGeo14->setEnabled(false);
        ui->btnGeo15->setEnabled(false);
        ui->btnGeo16->setEnabled(false);

        ui->btnHRV->setEnabled(false);
        ui->cmbHRVtype->setEnabled(false);
        ui->chkColorHRV->setEnabled(false);

        ui->lblGeo1->setText("   ");
        ui->lblGeo2->setText("   ");
        ui->lblGeo3->setText("   ");
        ui->lblGeo4->setText("   ");
        ui->lblGeo5->setText("   ");
        ui->lblGeo6->setText("   ");
        ui->lblGeo7->setText("   ");
        ui->lblGeo8->setText("   ");
        ui->lblGeo9->setText("   ");
        ui->lblGeo10->setText("   ");
        ui->lblGeo11->setText("   ");
        ui->lblGeo12->setText("   ");
        ui->lblGeo13->setText("   ");
        ui->lblGeo14->setText("   ");
        ui->lblGeo15->setText("   ");
        ui->lblGeo16->setText("   ");


        break;
    case SegmentListGeostationary::GOES_13:
    case SegmentListGeostationary::GOES_15:
        ui->btnGeoColor->setEnabled(false);
        ui->btnGeo1->setEnabled(false);
        ui->btnGeo2->setEnabled(state);
        ui->btnGeo3->setEnabled(false);
        ui->btnGeo4->setEnabled(state);
        ui->btnGeo5->setEnabled(state);
        ui->btnGeo6->setEnabled(false);
        ui->btnGeo7->setEnabled(false);
        ui->btnGeo8->setEnabled(false);
        ui->btnGeo9->setEnabled(state);
        ui->btnGeo10->setEnabled(false);
        ui->btnGeo11->setEnabled(false);
        ui->btnGeo12->setEnabled(false);
        ui->btnGeo13->setEnabled(false);
        ui->btnGeo14->setEnabled(false);
        ui->btnGeo15->setEnabled(false);
        ui->btnGeo16->setEnabled(false);

        ui->btnHRV->setEnabled(false);
        ui->cmbHRVtype->setEnabled(false);
        ui->chkColorHRV->setEnabled(false);

        ui->lblGeo1->setText("   ");
        ui->lblGeo2->setText("   ");
        ui->lblGeo3->setText("   ");
        ui->lblGeo4->setText("   ");
        ui->lblGeo5->setText("   ");
        ui->lblGeo6->setText("   ");
        ui->lblGeo7->setText("   ");
        ui->lblGeo8->setText("   ");
        ui->lblGeo9->setText("   ");
        ui->lblGeo10->setText("   ");
        ui->lblGeo11->setText("   ");
        ui->lblGeo12->setText("   ");
        ui->lblGeo13->setText("   ");
        ui->lblGeo14->setText("   ");
        ui->lblGeo15->setText("   ");
        ui->lblGeo16->setText("   ");


        break;
    case SegmentListGeostationary::FY2E:
    case SegmentListGeostationary::FY2G:
        ui->btnGeoColor->setEnabled(state);
        ui->btnGeo1->setEnabled(state);
        ui->btnGeo2->setEnabled(state);
        ui->btnGeo3->setEnabled(state);
        ui->btnGeo4->setEnabled(state);
        ui->btnGeo5->setEnabled(state);
        ui->btnGeo6->setEnabled(false);
        ui->btnGeo7->setEnabled(false);
        ui->btnGeo8->setEnabled(false);
        ui->btnGeo9->setEnabled(false);
        ui->btnGeo10->setEnabled(false);
        ui->btnGeo11->setEnabled(false);
        ui->btnGeo12->setEnabled(false);
        ui->btnGeo13->setEnabled(false);
        ui->btnGeo14->setEnabled(false);
        ui->btnGeo15->setEnabled(false);
        ui->btnGeo16->setEnabled(false);

        ui->btnHRV->setEnabled(state);
        ui->cmbHRVtype->setEnabled(false);
        ui->chkColorHRV->setEnabled(false);

        ui->lblGeo1->setText("   ");
        ui->lblGeo2->setText("   ");
        ui->lblGeo3->setText("   ");
        ui->lblGeo4->setText("   ");
        ui->lblGeo5->setText("   ");
        ui->lblGeo6->setText("   ");
        ui->lblGeo7->setText("   ");
        ui->lblGeo8->setText("   ");
        ui->lblGeo9->setText("   ");
        ui->lblGeo10->setText("   ");
        ui->lblGeo11->setText("   ");
        ui->lblGeo12->setText("   ");
        ui->lblGeo13->setText("   ");
        ui->lblGeo14->setText("   ");
        ui->lblGeo15->setText("   ");
        ui->lblGeo16->setText("   ");

        break;
    case SegmentListGeostationary::H8:
        ui->btnGeoColor->setEnabled(state);
        ui->btnGeo1->setEnabled(state);
        ui->btnGeo2->setEnabled(state);
        ui->btnGeo3->setEnabled(state);
        ui->btnGeo4->setEnabled(state);
        ui->btnGeo5->setEnabled(state);
        ui->btnGeo6->setEnabled(state);
        ui->btnGeo7->setEnabled(state);
        ui->btnGeo8->setEnabled(state);
        ui->btnGeo9->setEnabled(state);
        ui->btnGeo10->setEnabled(state);
        ui->btnGeo11->setEnabled(state);
        ui->btnGeo12->setEnabled(state);
        ui->btnGeo13->setEnabled(state);
        ui->btnGeo14->setEnabled(state);
        ui->btnGeo15->setEnabled(state);
        ui->btnGeo16->setEnabled(state);

        ui->btnHRV->setEnabled(false);
        ui->cmbHRVtype->setEnabled(false);
        ui->chkColorHRV->setEnabled(false);

        ui->lblGeo1->setText("0.64");
        ui->lblGeo2->setText("0.86");
        ui->lblGeo3->setText("1.6");
        ui->lblGeo4->setText("2.3");
        ui->lblGeo5->setText("3.9");
        ui->lblGeo6->setText("6.2");
        ui->lblGeo7->setText("6.9");
        ui->lblGeo8->setText("7.3");
        ui->lblGeo9->setText("8.6");
        ui->lblGeo10->setText("10.4");
        ui->lblGeo11->setText("11.2");
        ui->lblGeo12->setText("12.4");
        ui->lblGeo13->setText("13.3");
        ui->lblGeo14->setText("");
        ui->lblGeo15->setText("");
        ui->lblGeo16->setText("");

        break;

    case SegmentListGeostationary::GOES_16:
        ui->btnGeoColor->setEnabled(state);
        ui->btnGeo1->setEnabled(state);
        ui->btnGeo2->setEnabled(state);
        ui->btnGeo3->setEnabled(state);
        ui->btnGeo4->setEnabled(state);
        ui->btnGeo5->setEnabled(state);
        ui->btnGeo6->setEnabled(state);
        ui->btnGeo7->setEnabled(state);
        ui->btnGeo8->setEnabled(state);
        ui->btnGeo9->setEnabled(state);
        ui->btnGeo10->setEnabled(state);
        ui->btnGeo11->setEnabled(state);
        ui->btnGeo12->setEnabled(state);
        ui->btnGeo13->setEnabled(state);
        ui->btnGeo14->setEnabled(state);
        ui->btnGeo15->setEnabled(state);
        ui->btnGeo16->setEnabled(state);

        ui->btnHRV->setEnabled(false);
        ui->cmbHRVtype->setEnabled(false);
        ui->chkColorHRV->setEnabled(false);

        ui->lblGeo1->setText("0.47");
        ui->lblGeo2->setText("0.64");
        ui->lblGeo3->setText("0.86");
        ui->lblGeo4->setText("1.37");
        ui->lblGeo5->setText("1.61");
        ui->lblGeo6->setText("2.24");
        ui->lblGeo7->setText("3.89");
        ui->lblGeo8->setText("6.17");
        ui->lblGeo9->setText("6.93");
        ui->lblGeo10->setText("7.34");
        ui->lblGeo11->setText("8.44");
        ui->lblGeo12->setText("9.61");
        ui->lblGeo13->setText("10.33");
        ui->lblGeo14->setText("11.2");
        ui->lblGeo15->setText("12.3");
        ui->lblGeo16->setText("13.3");

        break;

    default:
        ui->btnGeoColor->setEnabled(state);
        ui->btnGeo1->setEnabled(state);
        ui->btnGeo2->setEnabled(state);
        ui->btnGeo3->setEnabled(state);
        ui->btnGeo4->setEnabled(state);
        ui->btnGeo5->setEnabled(state);
        ui->btnGeo6->setEnabled(state);
        ui->btnGeo7->setEnabled(state);
        ui->btnGeo8->setEnabled(state);
        ui->btnGeo9->setEnabled(state);
        ui->btnGeo10->setEnabled(state);
        ui->btnGeo11->setEnabled(state);
        ui->btnGeo12->setEnabled(false);
        ui->btnGeo13->setEnabled(false);
        ui->btnGeo14->setEnabled(false);
        ui->btnGeo15->setEnabled(false);
        ui->btnGeo16->setEnabled(false);

        ui->btnHRV->setEnabled(state);
        ui->cmbHRVtype->setEnabled(state);
        ui->chkColorHRV->setEnabled(state);

        ui->lblGeo1->setText("0.635");
        ui->lblGeo2->setText("0.81");
        ui->lblGeo3->setText("1.64");
        ui->lblGeo4->setText("3.90");
        ui->lblGeo5->setText("6.25");
        ui->lblGeo6->setText("7.35");
        ui->lblGeo7->setText("8.70");
        ui->lblGeo8->setText("9.66");
        ui->lblGeo9->setText("10.80");
        ui->lblGeo10->setText("12.00");
        ui->lblGeo11->setText("13.40");
        ui->lblGeo12->setText("");
        ui->lblGeo13->setText("");
        ui->lblGeo14->setText("");
        ui->lblGeo15->setText("");
        ui->lblGeo16->setText("");

    }

    ui->btnCLAHEavhhr->setEnabled(state);
    ui->btnCLAHEMeteosat->setEnabled(state);
    ui->btnExpandImage->setEnabled(state);
    ui->btnOverlayMeteosat->setEnabled(state);
    ui->btnOverlayProjectionGVP->setEnabled(state);
    ui->btnOverlayProjectionLCC->setEnabled(state);
    ui->btnOverlayProjectionSG->setEnabled(state);
    ui->btnRotate180->setEnabled(state);

    ui->btnUpdateVIIRSImage->setEnabled(state);
    ui->btnUpdateOLCIImage->setEnabled(state);
    ui->btnTextureVIIRS->setEnabled(state);

    ui->btnCreateLambert->setEnabled(state);
    ui->btnCreatePerspective->setEnabled(state);
    ui->btnCreateStereo->setEnabled(state);
    ui->btnLCCClearMap->setEnabled(state);
    ui->btnGVPClearMap->setEnabled(state);
    ui->btnSGClearMap->setEnabled(state);

    ui->rbColorVIIRS->setEnabled(state);
    ui->rbM1->setEnabled(state);
    ui->rbM2->setEnabled(state);
    ui->rbM3->setEnabled(state);
    ui->rbM4->setEnabled(state);
    ui->rbM5->setEnabled(state);
    ui->rbM6->setEnabled(state);
    ui->rbM7->setEnabled(state);
    ui->rbM8->setEnabled(state);
    ui->rbM9->setEnabled(state);
    ui->rbM10->setEnabled(state);
    ui->rbM11->setEnabled(state);
    ui->rbM12->setEnabled(state);
    ui->rbM13->setEnabled(state);
    ui->rbM14->setEnabled(state);
    ui->rbM15->setEnabled(state);
    ui->rbM16->setEnabled(state);

    ui->rbColorOLCI->setEnabled(state);
    ui->rbOLCI01->setEnabled(state);
    ui->rbOLCI02->setEnabled(state);
    ui->rbOLCI03->setEnabled(state);
    ui->rbOLCI04->setEnabled(state);
    ui->rbOLCI05->setEnabled(state);
    ui->rbOLCI06->setEnabled(state);
    ui->rbOLCI07->setEnabled(state);
    ui->rbOLCI08->setEnabled(state);
    ui->rbOLCI09->setEnabled(state);
    ui->rbOLCI10->setEnabled(state);
    ui->rbOLCI11->setEnabled(state);
    ui->rbOLCI12->setEnabled(state);
    ui->rbOLCI13->setEnabled(state);
    ui->rbOLCI14->setEnabled(state);
    ui->rbOLCI15->setEnabled(state);
    ui->rbOLCI16->setEnabled(state);
    ui->rbOLCI17->setEnabled(state);
    ui->rbOLCI18->setEnabled(state);
    ui->rbOLCI19->setEnabled(state);
    ui->rbOLCI20->setEnabled(state);
    ui->rbOLCI21->setEnabled(state);

}

void FormToolbox::on_btnGeo1_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
        onButtonChannel("VIS006", ui->chkInverseGeo1->isChecked());
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
        onButtonChannel("VIS", ui->chkInverseGeo1->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("VIS", ui->chkInverseGeo1->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C01", ui->chkInverseGeo1->isChecked());

}

void FormToolbox::on_btnGeo2_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
        onButtonChannel("VIS008", ui->chkInverseGeo2->isChecked());
    //else if(whichgeo == SegmentListGeostationary::ELECTRO_N1)
    //    onButtonChannel("00_9_0", ui->chkInverseGeo2->isChecked());
    else if(whichgeo == SegmentListGeostationary::MET_7)
        onButtonChannel("00_7_0", ui->chkInverseGeo2->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_13)
        onButtonChannel("00_7_0", ui->chkInverseGeo2->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_15)
        onButtonChannel("00_7_1", ui->chkInverseGeo2->isChecked());
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
        onButtonChannel("IR4", ui->chkInverseGeo2->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("B04", ui->chkInverseGeo2->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C02", ui->chkInverseGeo2->isChecked());


}

void FormToolbox::on_btnGeo3_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
        onButtonChannel("IR_016", ui->chkInverseGeo3->isChecked());
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
        onButtonChannel("IR3", ui->chkInverseGeo3->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("B05", ui->chkInverseGeo3->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C03", ui->chkInverseGeo3->isChecked());
}

void FormToolbox::on_btnGeo4_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
        onButtonChannel("IR_039", ui->chkInverseGeo4->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_13)
        onButtonChannel("03_9_0", ui->chkInverseGeo4->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_15)
        onButtonChannel("03_9_1", ui->chkInverseGeo4->isChecked());
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
        onButtonChannel("IR4", ui->chkInverseGeo4->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("B06", ui->chkInverseGeo4->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C04", ui->chkInverseGeo4->isChecked());


}

void FormToolbox::on_btnGeo5_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
        onButtonChannel("WV_062", ui->chkInverseGeo5->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_13)
        onButtonChannel("06_6_0", ui->chkInverseGeo5->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_15)
        onButtonChannel("06_6_1", ui->chkInverseGeo5->isChecked());
    else if(whichgeo == SegmentListGeostationary::MET_7)
        onButtonChannel("06_4_0", ui->chkInverseGeo5->isChecked());
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
        onButtonChannel("VIS", ui->chkInverseGeo5->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("IR4", ui->chkInverseGeo5->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C05", ui->chkInverseGeo5->isChecked());


}

void FormToolbox::on_btnGeo6_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
        onButtonChannel("WV_073", ui->chkInverseGeo6->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("IR3", ui->chkInverseGeo6->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C06", ui->chkInverseGeo6->isChecked());

}

void FormToolbox::on_btnGeo7_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
        onButtonChannel("IR_087", ui->chkInverseGeo7->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("B09", ui->chkInverseGeo7->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C07", ui->chkInverseGeo7->isChecked());

}

void FormToolbox::on_btnGeo8_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
        onButtonChannel("IR_097", ui->chkInverseGeo8->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("B10", ui->chkInverseGeo8->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C08", ui->chkInverseGeo8->isChecked());

}

void FormToolbox::on_btnGeo9_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
        onButtonChannel("IR_108", ui->chkInverseGeo9->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_13)
        onButtonChannel("10_7_0", ui->chkInverseGeo9->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_15)
        onButtonChannel("10_7_1", ui->chkInverseGeo9->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("B11", ui->chkInverseGeo9->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C09", ui->chkInverseGeo9->isChecked());

}

void FormToolbox::on_btnGeo10_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
        onButtonChannel("IR_120", ui->chkInverseGeo10->isChecked());
    else if(whichgeo == SegmentListGeostationary::MET_7)
        onButtonChannel("11_5_0", ui->chkInverseGeo10->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("IR1", ui->chkInverseGeo10->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C10", ui->chkInverseGeo10->isChecked());

}

void FormToolbox::on_btnGeo11_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
        onButtonChannel("IR_134", ui->chkInverseGeo11->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("B14", ui->chkInverseGeo11->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C11", ui->chkInverseGeo11->isChecked());

}

void FormToolbox::on_btnGeo12_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("IR2", ui->chkInverseGeo12->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C12", ui->chkInverseGeo12->isChecked());

}

void FormToolbox::on_btnGeo13_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("B16", ui->chkInverseGeo13->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C13", ui->chkInverseGeo13->isChecked());

}

void FormToolbox::on_btnGeo14_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C14", ui->chkInverseGeo14->isChecked());
}

void FormToolbox::on_btnGeo15_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C15", ui->chkInverseGeo15->isChecked());

}

void FormToolbox::on_btnGeo16_clicked()
{
    checkSegmentDateTime();
    if(whichgeo == SegmentListGeostationary::GOES_16)
        onButtonChannel("C16", ui->chkInverseGeo16->isChecked());

}



void FormToolbox::onButtonChannel( QString channel, bool bInverse)
{
    qDebug() << QString("onButtonChannel( QString channel, bool bInverse)  ; channel = %1").arg(channel);

    ui->pbProgress->reset();

    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_8)
        ui->pbProgress->setMaximum(8);
    else if(whichgeo == SegmentListGeostationary::MET_9)
        ui->pbProgress->setMaximum(3);
    else if(whichgeo == SegmentListGeostationary::MET_7)
    {
        if(channel == "00_7_0")
            ui->pbProgress->setMaximum(10);
        else
            ui->pbProgress->setMaximum(5);
    }
    else if(whichgeo == SegmentListGeostationary::GOES_13)
        ui->pbProgress->setMaximum(7);
    else if(whichgeo == SegmentListGeostationary::GOES_15)
        ui->pbProgress->setMaximum(7);
    else if(whichgeo == SegmentListGeostationary::GOES_16)
        ui->pbProgress->setMaximum(100);
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
        ui->pbProgress->setMaximum(100);
    else if(whichgeo == SegmentListGeostationary::H8)
        ui->pbProgress->setMaximum(10);

    segs->seglmeteosat->areatype = ui->cmbHRVtype->currentIndex();
    segs->seglmeteosatrss->areatype = ui->cmbHRVtype->currentIndex();
    segs->seglmet8->areatype = ui->cmbHRVtype->currentIndex();
    segs->seglmet7->areatype = 1;
    segs->seglgoes13dc3->areatype = 1;
    segs->seglgoes15dc3->areatype = 1;
    segs->seglgoes13dc4->areatype = 1;
    segs->seglgoes15dc4->areatype = 1;
    segs->seglgoes16->areatype = 1;
    segs->seglfy2e->areatype = 1;
    segs->seglfy2g->areatype = 1;
    segs->seglh8->areatype = 1;

    formimage->setKindOfImage(channel);
    segs->seglmeteosat->setKindofImage("VIS_IR");
    segs->seglmeteosatrss->setKindofImage("VIS_IR");
    segs->seglmet8->setKindofImage("VIS_IR");
    segs->seglmet7->setKindofImage("VIS_IR");
    segs->seglgoes13dc3->setKindofImage("VIS_IR");
    segs->seglgoes15dc3->setKindofImage("VIS_IR");
    segs->seglgoes13dc4->setKindofImage("VIS_IR");
    segs->seglgoes15dc4->setKindofImage("VIS_IR");
    segs->seglgoes16->setKindofImage("VIS_IR");
    segs->seglfy2e->setKindofImage("VIS_IR");
    segs->seglfy2g->setKindofImage("VIS_IR");
    segs->seglh8->setKindofImage("VIS_IR");

    setToolboxButtons(false);

    spectrumvector[0] = channel;
    spectrumvector[1] = "";
    spectrumvector[2] = "";

    inversevector[0] = bInverse;
    inversevector[1] = false;
    inversevector[2] = false;

    //formimage->displayImage(8);
    //formimage->adjustPicSize(true);
    emit switchstackedwidget(3);

    emit getmeteosatchannel("VIS_IR", spectrumvector, inversevector);
}

void FormToolbox::on_btnGeoColor_clicked()
{

    checkSegmentDateTime();

    if(!comboColGeoOK())
    {
        QMessageBox msgBox;
        msgBox.setText("Need color choices for 3 different bands in the Geostationary tab.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        int ret = msgBox.exec();

        switch (ret) {
        case QMessageBox::Ok:
            break;
        default:
            break;
        }

        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor); // restore in FormImage::slotUpdateHimawari() or slotUpdateGeosat()

    if(whichgeo == SegmentListGeostationary::NOGEO)
        return;
    ui->pbProgress->reset();
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_8)
        ui->pbProgress->setMaximum(24);
    else if(whichgeo == SegmentListGeostationary::MET_9)
        ui->pbProgress->setMaximum(9);
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G )
        ui->pbProgress->setMaximum(100);
    else if(whichgeo == SegmentListGeostationary::H8)
        ui->pbProgress->setMaximum(30);
    else if(whichgeo == SegmentListGeostationary::GOES_16)
        ui->pbProgress->setMaximum(100);

    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8 ||
            whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G || whichgeo == SegmentListGeostationary::H8 ||
            whichgeo == SegmentListGeostationary::GOES_16)
        onButtonColorHRV("VIS_IR Color");

}

void FormToolbox::checkSegmentDateTime()
{
    if(ui->lblMeteosatChosen->text() == "Image Date/Time")
    {
        QMessageBox msgBox;
        msgBox.setText("Select the Image Date and Time");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        int ret = msgBox.exec();

        switch (ret) {
        case QMessageBox::Ok:
            break;
        default:
            break;
        }

        return;
    }
}

void FormToolbox::on_btnHRV_clicked()
{

    checkSegmentDateTime();
    QApplication::setOverrideCursor(Qt::WaitCursor); // restore in FormImage::slotUpdateGeosat()

    ui->pbProgress->reset();

    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_8)
    {
        if(ui->cmbHRVtype->currentIndex() == 0 && ui->chkColorHRV->isChecked() == false)
            ui->pbProgress->setMaximum(5);
        if(ui->cmbHRVtype->currentIndex() == 0 && ui->chkColorHRV->isChecked() == true)
            ui->pbProgress->setMaximum(8+8+8+5);
        if(ui->cmbHRVtype->currentIndex() == 1 && ui->chkColorHRV->isChecked() == false)
            ui->pbProgress->setMaximum(24);
        if(ui->cmbHRVtype->currentIndex() == 1 && ui->chkColorHRV->isChecked() == true)
            ui->pbProgress->setMaximum(8+8+8+24);
    }

    if(whichgeo == SegmentListGeostationary::MET_9)
    {
        if(ui->cmbHRVtype->currentIndex() == 0 && ui->chkColorHRV->isChecked() == false)
            ui->pbProgress->setMaximum(5);
        if(ui->cmbHRVtype->currentIndex() == 0 && ui->chkColorHRV->isChecked() == true)
            ui->pbProgress->setMaximum(3+3+3+5);
        if(ui->cmbHRVtype->currentIndex() == 1 && ui->chkColorHRV->isChecked() == false)
            ui->pbProgress->setMaximum(0);
        if(ui->cmbHRVtype->currentIndex() == 1 && ui->chkColorHRV->isChecked() == true)
            ui->pbProgress->setMaximum(0);
    }

    if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
    {
        ui->pbProgress->setMaximum(100);
    }

    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
    {

        if (ui->chkColorHRV->isChecked())
        {
            onButtonColorHRV("HRV Color");
        }
        else
            onButtonColorHRV("HRV");
    }
    if (whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
    {
        onButtonColorHRV("HRV");
    }

}

void FormToolbox::onButtonColorHRV(QString type)
{

    segs->seglmeteosat->areatype = ui->cmbHRVtype->currentIndex();
    segs->seglmeteosatrss->areatype = ui->cmbHRVtype->currentIndex();
    segs->seglmet8->areatype = ui->cmbHRVtype->currentIndex();

    segs->seglmeteosat->setKindofImage(type);
    segs->seglmeteosatrss->setKindofImage(type);
    segs->seglmet8->setKindofImage(type);
    segs->seglfy2e->setKindofImage(type);
    segs->seglfy2g->setKindofImage(type);
    segs->seglh8->setKindofImage(type);
    segs->seglgoes16->setKindofImage(type);

    formimage->setKindOfImage(type);

    formimage->displayImage(IMAGE_GEOSTATIONARY);
    formimage->adjustPicSize(true);


    setToolboxButtons(false);

    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
    {
        if(ui->comboGeo1->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo1->currentIndex()-1] = "VIS006";
            inversevector[ui->comboGeo1->currentIndex()-1] = ui->chkInverseGeo1->isChecked();
        }
        if(ui->comboGeo2->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo2->currentIndex()-1] = "VIS008";
            inversevector[ui->comboGeo2->currentIndex()-1] = ui->chkInverseGeo2->isChecked();
        }
        if(ui->comboGeo3->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo3->currentIndex()-1] = "IR_016";
            inversevector[ui->comboGeo3->currentIndex()-1] = ui->chkInverseGeo3->isChecked();
        }
        if(ui->comboGeo4->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo4->currentIndex()-1] = "IR_039";
            inversevector[ui->comboGeo4->currentIndex()-1] = ui->chkInverseGeo4->isChecked();
        }
        if(ui->comboGeo5->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo5->currentIndex()-1] = "WV_062";
            inversevector[ui->comboGeo5->currentIndex()-1] = ui->chkInverseGeo5->isChecked();
        }
        if(ui->comboGeo6->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo6->currentIndex()-1] = "WV_073";
            inversevector[ui->comboGeo6->currentIndex()-1] = ui->chkInverseGeo6->isChecked();
        }
        if(ui->comboGeo7->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo7->currentIndex()-1] = "IR_087";
            inversevector[ui->comboGeo7->currentIndex()-1] = ui->chkInverseGeo7->isChecked();
        }
        if(ui->comboGeo8->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo8->currentIndex()-1] = "IR_097";
            inversevector[ui->comboGeo8->currentIndex()-1] = ui->chkInverseGeo8->isChecked();
        }
        if(ui->comboGeo9->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo9->currentIndex()-1] = "IR_108";
            inversevector[ui->comboGeo9->currentIndex()-1] = ui->chkInverseGeo9->isChecked();
        }
        if(ui->comboGeo10->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo10->currentIndex()-1] = "IR_120";
            inversevector[ui->comboGeo10->currentIndex()-1] = ui->chkInverseGeo10->isChecked();
        }
        if(ui->comboGeo11->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo11->currentIndex()-1] = "IR_134";
            inversevector[ui->comboGeo11->currentIndex()-1] = ui->chkInverseGeo11->isChecked();
        }
    }
    else if (whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
    {
        if(ui->comboGeo1->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo1->currentIndex()-1] = "IR1";
            inversevector[ui->comboGeo1->currentIndex()-1] = ui->chkInverseGeo1->isChecked();
        }
        if(ui->comboGeo2->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo2->currentIndex()-1] = "IR2";
            inversevector[ui->comboGeo2->currentIndex()-1] = ui->chkInverseGeo2->isChecked();
        }
        if(ui->comboGeo3->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo3->currentIndex()-1] = "IR3";
            inversevector[ui->comboGeo3->currentIndex()-1] = ui->chkInverseGeo3->isChecked();
        }
        if(ui->comboGeo4->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo4->currentIndex()-1] = "IR4";
            inversevector[ui->comboGeo4->currentIndex()-1] = ui->chkInverseGeo4->isChecked();
        }
        if(ui->comboGeo5->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo5->currentIndex()-1] = "VIS";
            inversevector[ui->comboGeo5->currentIndex()-1] = ui->chkInverseGeo5->isChecked();
        }

    }
    else if(whichgeo == SegmentListGeostationary::H8 )
    {
        if(ui->comboGeo1->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo1->currentIndex()-1] = "VIS";
            inversevector[ui->comboGeo1->currentIndex()-1] = ui->chkInverseGeo1->isChecked();
        }
        if(ui->comboGeo2->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo2->currentIndex()-1] = "B04";
            inversevector[ui->comboGeo2->currentIndex()-1] = ui->chkInverseGeo2->isChecked();
        }
        if(ui->comboGeo3->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo3->currentIndex()-1] = "B05";
            inversevector[ui->comboGeo3->currentIndex()-1] = ui->chkInverseGeo3->isChecked();
        }
        if(ui->comboGeo4->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo4->currentIndex()-1] = "IR4";
            inversevector[ui->comboGeo4->currentIndex()-1] = ui->chkInverseGeo4->isChecked();
        }
        if(ui->comboGeo5->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo5->currentIndex()-1] = "IR3";
            inversevector[ui->comboGeo5->currentIndex()-1] = ui->chkInverseGeo5->isChecked();
        }
        if(ui->comboGeo6->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo6->currentIndex()-1] = "B09";
            inversevector[ui->comboGeo6->currentIndex()-1] = ui->chkInverseGeo6->isChecked();
        }
        if(ui->comboGeo7->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo7->currentIndex()-1] = "B10";
            inversevector[ui->comboGeo7->currentIndex()-1] = ui->chkInverseGeo7->isChecked();
        }
        if(ui->comboGeo8->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo8->currentIndex()-1] = "B11";
            inversevector[ui->comboGeo8->currentIndex()-1] = ui->chkInverseGeo8->isChecked();
        }
        if(ui->comboGeo9->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo9->currentIndex()-1] = "IR1";
            inversevector[ui->comboGeo9->currentIndex()-1] = ui->chkInverseGeo9->isChecked();
        }
        if(ui->comboGeo10->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo10->currentIndex()-1] = "B14";
            inversevector[ui->comboGeo10->currentIndex()-1] = ui->chkInverseGeo10->isChecked();
        }
        if(ui->comboGeo11->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo11->currentIndex()-1] = "IR2";
            inversevector[ui->comboGeo11->currentIndex()-1] = ui->chkInverseGeo11->isChecked();
        }
        if(ui->comboGeo12->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo12->currentIndex()-1] = "B16";
            inversevector[ui->comboGeo12->currentIndex()-1] = ui->chkInverseGeo12->isChecked();
        }
    }
    else if(whichgeo == SegmentListGeostationary::GOES_16 )
    {
        if(ui->comboGeo1->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo1->currentIndex()-1] = "C01";
            inversevector[ui->comboGeo1->currentIndex()-1] = ui->chkInverseGeo1->isChecked();
        }
        if(ui->comboGeo2->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo2->currentIndex()-1] = "C02";
            inversevector[ui->comboGeo2->currentIndex()-1] = ui->chkInverseGeo2->isChecked();
        }
        if(ui->comboGeo3->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo3->currentIndex()-1] = "C03";
            inversevector[ui->comboGeo3->currentIndex()-1] = ui->chkInverseGeo3->isChecked();
        }
        if(ui->comboGeo4->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo4->currentIndex()-1] = "C04";
            inversevector[ui->comboGeo4->currentIndex()-1] = ui->chkInverseGeo4->isChecked();
        }
        if(ui->comboGeo5->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo5->currentIndex()-1] = "C05";
            inversevector[ui->comboGeo5->currentIndex()-1] = ui->chkInverseGeo5->isChecked();
        }
        if(ui->comboGeo6->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo6->currentIndex()-1] = "C06";
            inversevector[ui->comboGeo6->currentIndex()-1] = ui->chkInverseGeo6->isChecked();
        }
        if(ui->comboGeo7->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo7->currentIndex()-1] = "C07";
            inversevector[ui->comboGeo7->currentIndex()-1] = ui->chkInverseGeo7->isChecked();
        }
        if(ui->comboGeo8->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo8->currentIndex()-1] = "C08";
            inversevector[ui->comboGeo8->currentIndex()-1] = ui->chkInverseGeo8->isChecked();
        }
        if(ui->comboGeo9->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo9->currentIndex()-1] = "C09";
            inversevector[ui->comboGeo9->currentIndex()-1] = ui->chkInverseGeo9->isChecked();
        }
        if(ui->comboGeo10->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo10->currentIndex()-1] = "C10";
            inversevector[ui->comboGeo10->currentIndex()-1] = ui->chkInverseGeo10->isChecked();
        }
        if(ui->comboGeo11->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo11->currentIndex()-1] = "C11";
            inversevector[ui->comboGeo11->currentIndex()-1] = ui->chkInverseGeo11->isChecked();
        }
        if(ui->comboGeo12->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo12->currentIndex()-1] = "C12";
            inversevector[ui->comboGeo12->currentIndex()-1] = ui->chkInverseGeo12->isChecked();
        }
        if(ui->comboGeo13->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo13->currentIndex()-1] = "C13";
            inversevector[ui->comboGeo13->currentIndex()-1] = ui->chkInverseGeo13->isChecked();
        }
        if(ui->comboGeo14->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo14->currentIndex()-1] = "C14";
            inversevector[ui->comboGeo14->currentIndex()-1] = ui->chkInverseGeo14->isChecked();
        }
        if(ui->comboGeo15->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo15->currentIndex()-1] = "C15";
            inversevector[ui->comboGeo15->currentIndex()-1] = ui->chkInverseGeo15->isChecked();
        }
        if(ui->comboGeo16->currentIndex() > 0)
        {
            spectrumvector[ui->comboGeo16->currentIndex()-1] = "C16";
            inversevector[ui->comboGeo16->currentIndex()-1] = ui->chkInverseGeo16->isChecked();
        }
    }

    emit switchstackedwidget(3);
    emit getmeteosatchannel(type, spectrumvector, inversevector);
}

void FormToolbox::on_btnTextureMet_clicked()
{
    if (opts.imageontextureOnMet)
    {
        opts.imageontextureOnMet = false;
        ui->btnTextureMet->setText("Texture Off");
    }
    else
    {
        opts.imageontextureOnMet = true;
        ui->btnTextureMet->setText("Texture On");
    }

}

void FormToolbox::on_btnTextureAVHRR_clicked()
{
    if (opts.imageontextureOnAVHRR)
    {
        opts.imageontextureOnAVHRR = false;
        ui->btnTextureAVHRR->setText("Texture Off");
    }
    else
    {
        opts.imageontextureOnAVHRR = true;
        ui->btnTextureAVHRR->setText("Texture On");
    }

}

void FormToolbox::on_btnTextureVIIRS_clicked()
{
    if (opts.imageontextureOnVIIRS)
    {
        opts.imageontextureOnVIIRS = false;
        ui->btnTextureVIIRS->setText("Texture Off");
    }
    else
    {
        opts.imageontextureOnVIIRS = true;
        ui->btnTextureVIIRS->setText("Texture On");
    }

}

void FormToolbox::on_btnTextureOLCI_clicked()
{
    if (opts.imageontextureOnOLCI)
    {
        opts.imageontextureOnOLCI = false;
        ui->btnTextureOLCI->setText("Texture Off");
    }
    else
    {
        opts.imageontextureOnOLCI = true;
        ui->btnTextureOLCI->setText("Texture On");
    }

}

void FormToolbox::on_tabWidget_currentChanged(int index)
{
    qDebug() << "on_tabWidget_currentChanged(int index) index = " << index;

//    if(ui->tabWidgetVIIRS->currentIndex() == 0)
//        qDebug() << "begin ui->tabWidgetVIIRS->currentIndex() == 0";

    if(!forminfrascales->isHidden())
        forminfrascales->hide();

    ui->btnGVPFalseColor->setChecked(false);
    ui->btnSGFalseColor->setChecked(false);
    ui->btnLCCFalseColor->setChecked(false);

    qDebug() << "1";

    if (index == TAB_AVHRR) //AVHHR
    {
        formimage->displayImage(currentAVHRRimage);
    }
    else if (index == TAB_VIIRS) // VIIRS
    {
        if(ui->tabWidgetVIIRS->currentIndex() == 0)
        {
            formimage->displayImage(IMAGE_VIIRS_M);
        }
        else
        {
            formimage->displayImage(IMAGE_VIIRS_DNB);
        }
    }
    else if (index == TAB_OLCI) // OLCI
    {
        formimage->displayImage(IMAGE_OLCI);
    }
    else if (index == TAB_GEOSTATIONARY) // Geostationair
    {
        formimage->displayImage(IMAGE_GEOSTATIONARY);
    }
    else if (index == TAB_PROJECTION) // Projection
    {
        if( ui->toolBox->currentIndex() == 0)
            imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                       ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
        else if( ui->toolBox->currentIndex() == 1)
            imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), ui->spbGVPMapWidth->value(), ui->spbGVPMapHeight->value());
        else
            imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), ui->spbSGMapWidth->value(), ui->spbSGMapHeight->value(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
        formimage->UpdateProjection();
    }

//    opts.currenttabwidget = ui->tabWidget->currentIndex();
//    opts.currenttoolbox = ui->toolBox->currentIndex();
}

void FormToolbox::on_tabWidgetVIIRS_currentChanged(int index)
{

    qDebug() << "on_tabWidgetVIIRS_currentChanged(int index) index = " << index;

    if(!forminfrascales->isHidden())
        forminfrascales->hide();
    ui->btnGVPFalseColor->setChecked(false);
    ui->btnSGFalseColor->setChecked(false);
    ui->btnLCCFalseColor->setChecked(false);

    if (index == 0) //VIIRSM
        formimage->displayImage(IMAGE_VIIRS_M);
    else if (index == 1) // VIIRSDNB
        formimage->displayImage(IMAGE_VIIRS_DNB);


}

void FormToolbox::on_chkShowLambert_stateChanged(int arg1)
{
    opts.mapextentlamberton = ui->chkShowLambert->isChecked();
}

void FormToolbox::on_chkShowPerspective_stateChanged(int arg1)
{
    opts.mapextentperspectiveon = ui->chkShowPerspective->isChecked();
}

void FormToolbox::on_spbLCCMapWidth_valueChanged(int arg1)
{
    opts.mapwidth = arg1;
}

void FormToolbox::on_spbLCCMapHeight_valueChanged(int arg1)
{
    opts.mapheight = arg1;
}

void FormToolbox::on_spbGVPMapWidth_valueChanged(int arg1)
{
    opts.mapwidth = arg1;
}

void FormToolbox::on_spbGVPMapHeight_valueChanged(int arg1)
{
    opts.mapheight = arg1;
}
void FormToolbox::on_spbSGMapWidth_valueChanged(int arg1)
{
    opts.mapwidth = arg1;
}

void FormToolbox::on_spbSGMapHeight_valueChanged(int arg1)
{
    opts.mapheight = arg1;
}


void FormToolbox::on_spbScaleX_valueChanged(double arg1)
{
    qDebug() << "FormToolbox::on_spbScaleX_valueChanged(double arg1)";
    opts.maplccscalex = arg1;
    if(imageptrs->ptrimageProjection->width() > 0 && opts.currenttoolbox == TAB_LLC)
    {
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
        formimage->UpdateProjection();
    }
}

void FormToolbox::on_spbScaleY_valueChanged(double arg1)
{
    opts.maplccscaley = arg1;
    if(imageptrs->ptrimageProjection->width() > 0 && opts.currenttoolbox == TAB_LLC)
    {
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
        formimage->UpdateProjection();
    }

}

void FormToolbox::on_spbGVPlat_valueChanged(double arg1)
{
    opts.mapgvplat = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height());
        formimage->UpdateProjection();
    }

}

void FormToolbox::on_spbGVPlon_valueChanged(double arg1)
{
    opts.mapgvplon = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height());
        formimage->UpdateProjection();
    }

}

void FormToolbox::on_spbGVPheight_valueChanged(int arg1)
{
    opts.mapgvpheight = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height());
        formimage->UpdateProjection();
    }

}

void FormToolbox::on_spbGVPscale_valueChanged(double arg1)
{
    if( arg1 > 0.0)
    {
        opts.mapgvpscale = arg1;
        if(imageptrs->ptrimageProjection->width() > 0)
        {
            imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height());
            formimage->UpdateProjection();
        }
    }

}

void FormToolbox::on_btnCreatePerspective_clicked()
{
    if(ui->rdbCombine->isChecked() && ui->rdbVIIRSDNBin->isChecked())
    {
        QRgb *row, *rowcopy;
        imageptrs->ptrimageProjectionCopy = new QImage(imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height(), QImage::Format_ARGB32);
        for(int h = 0; h < imageptrs->ptrimageProjection->height(); h++ )
        {
            row = (QRgb*)imageptrs->ptrimageProjection->scanLine(h);
            rowcopy = (QRgb*)imageptrs->ptrimageProjectionCopy->scanLine(h);
            for(int w = 0; w < imageptrs->ptrimageProjection->width() ; w++)
            {
                rowcopy[w] = row[w];
            }
        }
    }

    if(ui->rdbAVHRRin->isChecked())
    {
        if(opts.buttonMetop || opts.buttonNoaa || opts.buttonHRP || opts.buttonGAC || opts.buttonMetopAhrpt || opts.buttonMetopBhrpt || opts.buttonNoaa19hrpt ||
                opts.buttonM01hrpt || opts.buttonM02hrpt)
        {
            if(!segs->SelectedAVHRRSegments())
            {
                QMessageBox::information( this, "AVHHR", "No selected AVHRR segments  !" );
                return;
            }
        }
    }
    else if(ui->rdbVIIRSMin->isChecked())
    {
        if(opts.buttonVIIRSM)
        {
            if(!segs->SelectedVIIRSMSegments())
            {
                QMessageBox::information( this, "VIIRS M", "No selected VIIRS M segments  !" );
                return;
            }
        }

    }
    else if(ui->rdbVIIRSDNBin->isChecked())
    {
        if(opts.buttonVIIRSDNB)
        {
            if(!segs->SelectedVIIRSDNBSegments())
            {
                QMessageBox::information( this, "VIIRS DNB", "No selected VIIRS DNB segments  !" );
                return;
            }
        }

    }
    else if(ui->rdbOLCIefrin->isChecked())
    {
        if(opts.buttonOLCIefr)
        {
            if(!segs->SelectedOLCIefrSegments())
            {
                QMessageBox::information( this, "OLCI EFR 1", "No selected OLCI EFR segments  !" );
                return;
            }
        }
        else
        {
            QMessageBox::information( this, "OLCI EFR 2", "No selected OLCI EFR segments  !" );
            return;
        }

    }
    else if(ui->rdbOLCIerrin->isChecked())
    {
        if(opts.buttonOLCIerr)
        {
            if(!segs->SelectedOLCIerrSegments())
            {
                QMessageBox::information( this, "OLCI ERR 1", "No selected OLCI ERR segments  !" );
                return;
            }
        }
        else
        {
            QMessageBox::information( this, "OLCI ERR 2", "No selected OLCI ERR segments  !" );
            return;
        }

    }

    QApplication::setOverrideCursor( Qt::WaitCursor );

    imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), ui->spbGVPMapWidth->value(), ui->spbGVPMapHeight->value());

    int width = imageptrs->ptrimageProjection->width();
    int height = imageptrs->ptrimageProjection->height();

    if(!forminfrascales->isHidden())
        forminfrascales->hide();
    ui->btnGVPFalseColor->setChecked(false);
    ui->btnSGFalseColor->setChecked(false);
    ui->btnLCCFalseColor->setChecked(false);


    if(ui->rdbAVHRRin->isChecked())
        imageptrs->gvp->CreateMapFromAVHRR(ui->cmbInputAVHRRChannel->currentIndex(), formimage->getSegmentType());
    else if(ui->rdbVIIRSMin->isChecked())
    {
        imageptrs->ptrProjectionBrightnessTemp.reset(new float[width * height]);

        for(int y = 0; y < height; y++)
        {
            for(int x = 0; x < width; x++)
            {
                imageptrs->ptrProjectionBrightnessTemp[y * width + x] = -1.0;
            }
        }

        ui->btnGVPFalseColor->setChecked(false);
        imageptrs->gvp->CreateMapFromVIIRS(eSegmentType::SEG_VIIRSM, false);
        initializeScales();

    }
    else if(ui->rdbVIIRSDNBin->isChecked())
        imageptrs->gvp->CreateMapFromVIIRS(eSegmentType::SEG_VIIRSDNB, ui->rdbCombine->isChecked());
    else if(ui->rdbOLCIefrin->isChecked())
    {
        qDebug() << "voor CreateMapFromOLCI currentindex = " << ui->cmbHistogramProj->currentIndex() << " " <<  ui->rdbOLCIprojNormalized->isChecked();
        imageptrs->gvp->CreateMapFromOLCI(eSegmentType::SEG_OLCIEFR, false, ui->cmbHistogramProj->currentIndex(), ui->rdbOLCIprojNormalized->isChecked());
    }
    else if(ui->rdbOLCIerrin->isChecked())
        imageptrs->gvp->CreateMapFromOLCI(eSegmentType::SEG_OLCIERR, false, ui->cmbHistogramProj->currentIndex(), ui->rdbOLCIprojNormalized->isChecked());
    else if(ui->rdbMeteosatin->isChecked())
        imageptrs->gvp->CreateMapFromGeoStationary();

    if(ui->rdbCombine->isChecked())
        delete imageptrs->ptrimageProjectionCopy;

    formimage->setPixmapToLabel(true);
    QApplication::restoreOverrideCursor();

}

void FormToolbox::initializeScales()
{
    QList<bool> blist = this->getVIIRSMBandList();
    if(blist.at(12) || blist.at(13) || blist.at(14) || blist.at(15) || blist.at(16))
    {
        copyProjectionImage();

        forminfrascales->initializeLowHigh();
        forminfrascales->setMinMaxTemp( segs->seglviirsm->getMinBrightnessTempProjection(), segs->seglviirsm->getMaxBrightnessTempProjection());

        qDebug() << QString("setMinMaxTemp %1 %2").arg(segs->seglviirsm->getMinBrightnessTempProjection()).arg(segs->seglviirsm->getMaxBrightnessTempProjection());

        QList<bool> ilist = this->getVIIRSMInvertList();
        if(blist.at(12))
            forminfrascales->setInverse(ilist.at(11));
        else if(blist.at(13))
            forminfrascales->setInverse(ilist.at(12));
        else if(blist.at(14))
            forminfrascales->setInverse(ilist.at(13));
        else if(blist.at(15))
            forminfrascales->setInverse(ilist.at(14));
        else if(blist.at(16))
            forminfrascales->setInverse(ilist.at(15));
    }

}

void FormToolbox::copyProjectionImage()
{
    int width = imageptrs->ptrimageProjection->width();
    int height = imageptrs->ptrimageProjection->height();
    QRgb *scanproj;
    QRgb rgb;

    imageptrs->ptrProjectionInfra.reset(new quint8[width * height]);
    for(int y = 0; y < height; y++)
    {
        scanproj = (QRgb *)imageptrs->ptrimageProjection->scanLine(y);
        for(int x = 0; x < width; x++)
        {
            rgb = scanproj[x];
            imageptrs->ptrProjectionInfra[y * width + x] = (quint8)(qRed(rgb));
        }

    }

}

void FormToolbox::on_btnCreateLambert_clicked()
{
    if(ui->rdbCombine->isChecked())
    {
        QRgb *row, *rowcopy;
        imageptrs->ptrimageProjectionCopy = new QImage(imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height(), QImage::Format_ARGB32);
        for(int h = 0; h < imageptrs->ptrimageProjection->height(); h++ )
        {
            row = (QRgb*)imageptrs->ptrimageProjection->scanLine(h);
            rowcopy = (QRgb*)imageptrs->ptrimageProjectionCopy->scanLine(h);
            for(int w = 0; w < imageptrs->ptrimageProjection->width() ; w++)
            {
                rowcopy[w] = row[w];
            }
        }
    }

    if(opts.buttonMetop || opts.buttonNoaa || opts.buttonHRP || opts.buttonGAC || opts.buttonMetopAhrpt || opts.buttonMetopBhrpt || opts.buttonNoaa19hrpt ||
            opts.buttonM01hrpt || opts.buttonM02hrpt)
    {
        if(ui->rdbAVHRRin->isChecked())
        {
            if(!segs->SelectedAVHRRSegments())
                return;
        }
    }
    else if(opts.buttonVIIRSM)
    {
        if(ui->rdbVIIRSMin->isChecked())
        {
            if(!segs->SelectedVIIRSMSegments())
                return;
        }

    }
    else if(opts.buttonVIIRSDNB)
    {
        if(ui->rdbVIIRSDNBin->isChecked())
        {
            if(!segs->SelectedVIIRSDNBSegments())
                return;
        }

    }
    else if(opts.buttonOLCIefr)
    {
        if(ui->rdbOLCIefrin->isChecked())
        {
            if(!segs->SelectedOLCIefrSegments())
                return;
        }

    }
    else if(opts.buttonOLCIerr)
    {
        if(ui->rdbOLCIerrin->isChecked())
        {
            if(!segs->SelectedOLCIerrSegments())
                return;
        }

    }

    QApplication::setOverrideCursor( Qt::WaitCursor );
    imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                               ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());

    int width = imageptrs->ptrimageProjection->width();
    int height = imageptrs->ptrimageProjection->height();

    if(!forminfrascales->isHidden())
        forminfrascales->hide();

    ui->btnGVPFalseColor->setChecked(false);
    ui->btnSGFalseColor->setChecked(false);
    ui->btnLCCFalseColor->setChecked(false);

    if(ui->rdbAVHRRin->isChecked())
        imageptrs->lcc->CreateMapFromAVHRR(ui->cmbInputAVHRRChannel->currentIndex(), formimage->getSegmentType());
    else if(ui->rdbVIIRSMin->isChecked())
    {
        imageptrs->ptrProjectionBrightnessTemp.reset(new float[width * height]);

        for(int y = 0; y < height; y++)
        {
            for(int x = 0; x < width; x++)
            {
                imageptrs->ptrProjectionBrightnessTemp[y * width + x] = -1.0;
            }
        }

        ui->btnLCCFalseColor->setChecked(false);
        imageptrs->lcc->CreateMapFromVIIRS(eSegmentType::SEG_VIIRSM, false);
        initializeScales();
    }
    else if(ui->rdbVIIRSDNBin->isChecked())
        imageptrs->lcc->CreateMapFromVIIRS(eSegmentType::SEG_VIIRSDNB, ui->rdbCombine->isChecked());
    else if(ui->rdbMeteosatin->isChecked())
        imageptrs->lcc->CreateMapFromGeostationary();
    else if(ui->rdbOLCIefrin->isChecked())
        imageptrs->lcc->CreateMapFromOLCI(eSegmentType::SEG_OLCIEFR, ui->rdbCombine->isChecked(), ui->cmbHistogramProj->currentIndex(), ui->rdbOLCIprojNormalized->isChecked());
    else if(ui->rdbOLCIerrin->isChecked())
        imageptrs->lcc->CreateMapFromOLCI(eSegmentType::SEG_OLCIERR, ui->rdbCombine->isChecked(), ui->cmbHistogramProj->currentIndex(), ui->rdbOLCIprojNormalized->isChecked());

    if(ui->rdbCombine->isChecked())
        delete imageptrs->ptrimageProjectionCopy;

    formimage->setPixmapToLabel(true);
    QApplication::restoreOverrideCursor();
}

void FormToolbox::on_btnCreateStereo_clicked()
{
    if(ui->rdbCombine->isChecked())
    {
        QRgb *row, *rowcopy;
        imageptrs->ptrimageProjectionCopy = new QImage(imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height(), QImage::Format_ARGB32);
        for(int h = 0; h < imageptrs->ptrimageProjection->height(); h++ )
        {
            row = (QRgb*)imageptrs->ptrimageProjection->scanLine(h);
            rowcopy = (QRgb*)imageptrs->ptrimageProjectionCopy->scanLine(h);
            for(int w = 0; w < imageptrs->ptrimageProjection->width() ; w++)
            {
                rowcopy[w] = row[w];
            }
        }
    }

    if(opts.buttonMetop || opts.buttonNoaa || opts.buttonHRP || opts.buttonGAC || opts.buttonMetopAhrpt || opts.buttonMetopBhrpt || opts.buttonNoaa19hrpt ||
            opts.buttonM01hrpt || opts.buttonM02hrpt)
    {
        if(ui->rdbAVHRRin->isChecked())
        {
            if(!segs->SelectedAVHRRSegments())
                return;
        }
    }
    else if(opts.buttonVIIRSM)
    {
        if(ui->rdbVIIRSMin->isChecked())
        {
            if(!segs->SelectedVIIRSMSegments())
                return;
        }
    }
    else if(opts.buttonVIIRSDNB)
    {
        if(ui->rdbVIIRSDNBin->isChecked())
        {
            if(!segs->SelectedVIIRSDNBSegments())
                return;
        }
    }
    else if(opts.buttonOLCIefr)
    {
        if(ui->rdbOLCIefrin->isChecked())
        {
            if(!segs->SelectedOLCIefrSegments())
                return;
        }

    }
    else if(opts.buttonOLCIerr)
    {
        if(ui->rdbOLCIerrin->isChecked())
        {
            if(!segs->SelectedOLCIerrSegments())
                return;
        }

    }
    QApplication::setOverrideCursor( Qt::WaitCursor );

    imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), ui->spbSGMapWidth->value(), ui->spbSGMapHeight->value(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());

    int width = imageptrs->ptrimageProjection->width();
    int height = imageptrs->ptrimageProjection->height();

    if(!forminfrascales->isHidden())
        forminfrascales->hide();
    ui->btnGVPFalseColor->setChecked(false);
    ui->btnSGFalseColor->setChecked(false);
    ui->btnLCCFalseColor->setChecked(false);

    if(ui->rdbAVHRRin->isChecked())
        imageptrs->sg->CreateMapFromAVHRR(ui->cmbInputAVHRRChannel->currentIndex(), formimage->getSegmentType());
    else if(ui->rdbVIIRSMin->isChecked())
    {
        imageptrs->ptrProjectionBrightnessTemp.reset(new float[width * height]);

        for(int y = 0; y < height; y++)
        {
            for(int x = 0; x < width; x++)
            {
                imageptrs->ptrProjectionBrightnessTemp[y * width + x] = -1.0;
            }
        }

        ui->btnSGFalseColor->setChecked(false);

        imageptrs->sg->CreateMapFromVIIRS(eSegmentType::SEG_VIIRSM, false);
        initializeScales();
    }
    else if(ui->rdbVIIRSDNBin->isChecked())
        imageptrs->sg->CreateMapFromVIIRS(eSegmentType::SEG_VIIRSDNB, ui->rdbCombine->isChecked());
    else if(ui->rdbMeteosatin->isChecked())
        imageptrs->sg->CreateMapFromGeostationary();
    else if(ui->rdbOLCIefrin->isChecked())
        imageptrs->sg->CreateMapFromOLCI(eSegmentType::SEG_OLCIEFR,  ui->rdbCombine->isChecked(), ui->cmbHistogramProj->currentIndex(),ui->rdbOLCIprojNormalized->isChecked());
    else if(ui->rdbOLCIerrin->isChecked())
        imageptrs->sg->CreateMapFromOLCI(eSegmentType::SEG_OLCIERR,  ui->rdbCombine->isChecked(), ui->cmbHistogramProj->currentIndex(), ui->rdbOLCIprojNormalized->isChecked());

    if(ui->rdbCombine->isChecked())
        delete imageptrs->ptrimageProjectionCopy;

    formimage->setPixmapToLabel(true);
    QApplication::restoreOverrideCursor();

}

void FormToolbox::on_spbParallel1_valueChanged(int arg1)
{
        opts.parallel1 = arg1;
        if(imageptrs->ptrimageProjection->width() > 0)
        {
            imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                       ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
            formimage->UpdateProjection();
        }
}

void FormToolbox::on_spbParallel2_valueChanged(int arg1)
{
        opts.parallel2 = arg1;

        if(imageptrs->ptrimageProjection->width() > 0)
        {
            imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                       ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
            formimage->UpdateProjection();
        }


}

void FormToolbox::on_spbCentral_valueChanged(int arg1)
{
    opts.centralmeridian = arg1;

    qDebug() << QString("FormToolbox::on_spbCentral_valueChanged(int arg1)  opts.centralmeridian = %1)").arg(opts.centralmeridian);

    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
        formimage->UpdateProjection();
    }

}

void FormToolbox::on_spbLatOrigin_valueChanged(int arg1)
{
        opts.latitudeoforigin = arg1;

        qDebug() << QString("FormToolbox::on_spbOrigin_valueChanged(int arg1) origin value = %1").arg(arg1);

        if(imageptrs->ptrimageProjection->width() > 0)
        {
            imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                       ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
            formimage->UpdateProjection();
        }

}

void FormToolbox::on_spbSouth_valueChanged(int arg1)
{
    SignalsBlocker block(ui->spbNorth);

    if(ui->spbSouth->value() > ui->spbNorth->value())
    {
        ui->spbSouth->setValue(arg1 - 1);
        return;
    }


    opts.mapextentsouth = arg1;

    qDebug() << QString("FormToolbox::on_spbSouth_valueChanged(int arg1) south value = %1").arg(arg1);

    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
        formimage->UpdateProjection();
    }
}

void FormToolbox::on_spbNorth_valueChanged(int arg1)
{
    SignalsBlocker block(ui->spbSouth);

    if(ui->spbSouth->value() > ui->spbNorth->value())
    {
        ui->spbNorth->setValue(arg1 + 1);
        return;
    }

    opts.mapextentnorth = arg1;

    qDebug() << QString("FormToolbox::on_spbNorth_valueChanged(int arg1) north value = %1").arg(arg1);

    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
        formimage->UpdateProjection();
    }

}

void FormToolbox::on_spbWest_valueChanged(int arg1)
{
    if(ui->spbWest->value() > ui->spbEast->value() )
    {
        ui->spbWest->setValue(arg1 - 1 );
        return;
    }

    opts.mapextentwest = arg1;

    int central = ui->spbWest->value() + longitudediffdeg( ui->spbEast->value(), ui->spbWest->value())/2;
    qDebug() << QString("FormToolbox::on_spbWest_valueChanged(int arg1) central = %1").arg(central);

    ui->spbCentral->setValue(adjust_lon_deg(central));

    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
        formimage->UpdateProjection();
    }

}

void FormToolbox::on_spbEast_valueChanged(int arg1)
{
    if(ui->spbWest->value() > ui->spbEast->value() )
    {
        ui->spbEast->setValue(arg1 + 1);
        return;
    }

   opts.mapextenteast = arg1;


   int central = ui->spbWest->value() + longitudediffdeg( ui->spbEast->value(), ui->spbWest->value())/2;
   qDebug() << QString("FormToolbox::on_spbEast_valueChanged(int arg1) central = %1").arg(central);


   ui->spbCentral->setValue(adjust_lon_deg(central));

   if(imageptrs->ptrimageProjection->width() > 0)
   {
       imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                  ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
       formimage->UpdateProjection();
   }

}



int FormToolbox::getTabWidgetIndex()
{
    return ui->tabWidget->currentIndex();
}

int FormToolbox::getTabWidgetVIIRSIndex()
{
    return ui->tabWidgetVIIRS->currentIndex();
}

void FormToolbox::on_toolBox_currentChanged(int index)
{
    qDebug() << QString("FormToolbox::on_toolBox_currentChanged(int index) index = %1").arg(index);

    ui->comboPOI->blockSignals(true);
    ui->comboPOI->clear();

//    if(!dockinfrascales->isHidden())
//        dockinfrascales->hide();

    imageptrs->ptrProjectionInfra.reset();
    imageptrs->ptrProjectionBrightnessTemp.reset();

    opts.currenttoolbox = index;
    if (index == 0)
    {
        ui->comboPOI->addItems(poi.strlLCCName);
//        on_comboPOI_currentIndexChanged(0);
        ui->cbProjResolutions->setCurrentIndex(searchResolution(ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value()));
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(),  ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
    }
    else if(index == 1)
    {
        ui->comboPOI->addItems(poi.strlGVPName);
//        on_comboPOI_currentIndexChanged(0);
        ui->cbProjResolutions->setCurrentIndex(searchResolution(ui->spbGVPMapWidth->value(), ui->spbGVPMapHeight->value()));
        imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), ui->spbGVPMapWidth->value(), ui->spbGVPMapHeight->value());
    }
    else
    {
        ui->comboPOI->addItems(poi.strlSGName);
//        on_comboPOI_currentIndexChanged(0);
        ui->cbProjResolutions->setCurrentIndex(searchResolution(ui->spbSGMapWidth->value(), ui->spbSGMapHeight->value()));
        imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), ui->spbSGMapWidth->value(), ui->spbSGMapHeight->value(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
    }

    ui->comboPOI->blockSignals(false);
    formimage->UpdateProjection();
}


void FormToolbox::on_btnGVPClearMap_clicked()
{
    if(!forminfrascales->isHidden())
        forminfrascales->hide();
    ui->btnGVPFalseColor->setChecked(false);
    ui->btnSGFalseColor->setChecked(false);
    ui->btnLCCFalseColor->setChecked(false);

    imageptrs->ptrimageProjection->fill(qRgba(0, 0, 0, 250));
    formimage->displayImage(IMAGE_PROJECTION);
}

void FormToolbox::on_btnLCCClearMap_clicked()
{
    if(!forminfrascales->isHidden())
        forminfrascales->hide();
    ui->btnGVPFalseColor->setChecked(false);
    ui->btnSGFalseColor->setChecked(false);
    ui->btnLCCFalseColor->setChecked(false);

    imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                               ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());

    imageptrs->ptrimageProjection->fill(qRgba(0, 0, 0, 250));
    formimage->displayImage(IMAGE_PROJECTION);
}


void FormToolbox::on_btnLCCMapNorth_clicked()
{
    qDebug() << QString("FormToolbox::on_btnLCCMapNorth_clicked() ui->spbNorth = %1").arg(ui->spbNorth->value());

    ui->spbNorth->blockSignals(true);
    ui->spbSouth->blockSignals(true);

    ui->spbNorth->setValue(ui->spbNorth->value() + 1);
    ui->spbSouth->setValue(ui->spbSouth->value() + 1);

    ui->spbNorth->blockSignals(false);
    ui->spbSouth->blockSignals(false);

    opts.mapextentnorth = ui->spbNorth->value();
    opts.mapextentsouth = ui->spbSouth->value();

    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
        formimage->UpdateProjection();
    }

}

void FormToolbox::on_btnLCCMapSouth_clicked()
{
    qDebug() << QString("FormToolbox::on_btnLCCMapSouth_clicked() ui->spbSouth = %1").arg(ui->spbSouth->value());

    ui->spbNorth->blockSignals(true);
    ui->spbSouth->blockSignals(true);

    ui->spbNorth->setValue(ui->spbNorth->value() - 1);
    ui->spbSouth->setValue(ui->spbSouth->value() - 1);

    ui->spbNorth->blockSignals(false);
    ui->spbSouth->blockSignals(false);

    opts.mapextentnorth = ui->spbNorth->value();
    opts.mapextentsouth = ui->spbSouth->value();

    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
        formimage->UpdateProjection();
    }

}

void FormToolbox::on_btnLCCMapWest_clicked()
{
    int valwest = ui->spbWest->value();
    if(valwest == -180)
        valwest = 180;

    ui->spbEast->blockSignals(true);
    ui->spbWest->blockSignals(true);

    ui->spbEast->setValue(ui->spbEast->value() - 1);
    ui->spbWest->setValue(valwest - 1);

    ui->spbEast->blockSignals(false);
    ui->spbWest->blockSignals(false);

    opts.mapextenteast = ui->spbEast->value();
    opts.mapextentwest = ui->spbWest->value();

    int central = ui->spbWest->value() + longitudediffdeg( ui->spbEast->value(), ui->spbWest->value())/2;
    qDebug() << QString("FormToolbox::on_btnLCCMapWest_clicked central = %1").arg(central);

    ui->spbCentral->blockSignals(true);
    ui->spbCentral->setValue(adjust_lon_deg(central));
    ui->spbCentral->blockSignals(false);

    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
        formimage->UpdateProjection();
    }

}

void FormToolbox::on_btnLCCMapEast_clicked()
{
    int valeast = ui->spbEast->value();
    if(valeast == 180)
        valeast = -180;

    ui->spbEast->blockSignals(true);
    ui->spbWest->blockSignals(true);

    ui->spbEast->setValue(valeast + 1);
    ui->spbWest->setValue(ui->spbWest->value() + 1);

    ui->spbEast->blockSignals(false);
    ui->spbWest->blockSignals(false);

    opts.mapextenteast = ui->spbEast->value();
    opts.mapextentwest = ui->spbWest->value();

    int central = ui->spbWest->value() + longitudediffdeg( ui->spbEast->value(), ui->spbWest->value())/2;
    qDebug() << QString("FormToolbox::on_btnLCCMapEast_clicked central = %1").arg(central);

    ui->spbCentral->blockSignals(true);
    ui->spbCentral->setValue(adjust_lon_deg(central));
    ui->spbCentral->blockSignals(false);

    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
        formimage->UpdateProjection();
    }

}

void FormToolbox::on_spbSGlat_valueChanged(double arg1)
{
    qDebug() << "FormToolbox::on_spbSGlat_valueChanged(double arg1)";
    opts.mapsglat = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
        formimage->displayImage(IMAGE_PROJECTION);
    }

}

void FormToolbox::on_spbSGlon_valueChanged(double arg1)
{
    opts.mapsglon = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
        formimage->displayImage(IMAGE_PROJECTION);
    }

}

void FormToolbox::on_spbSGScale_valueChanged(double arg1)
{
    opts.mapsgscale = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
        formimage->displayImage(IMAGE_PROJECTION);
    }

}



void FormToolbox::on_btnGVPHome_clicked()
{
    ui->spbGVPlon->setValue(opts.obslon);
    ui->spbGVPlat->setValue(opts.obslat);

}

void FormToolbox::on_btnSGHome_clicked()
{
    ui->spbSGlon->setValue(opts.obslon);
    ui->spbSGlat->setValue(opts.obslat);

}

void FormToolbox::on_spbSGPanHorizon_valueChanged(int arg1)
{
    opts.mapsgpanhorizon = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
        formimage->displayImage(IMAGE_PROJECTION);
    }

}

void FormToolbox::on_spbSGPanVert_valueChanged(int arg1)
{
    opts.mapsgpanvert = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
        formimage->displayImage(IMAGE_PROJECTION);
    }

}

void FormToolbox::on_spbSGRadius_valueChanged(double arg1)
{
    opts.mapsgradius = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
        formimage->displayImage(IMAGE_PROJECTION);
    }

}

void FormToolbox::on_btnSGClearMap_clicked()
{
    if(!forminfrascales->isHidden())
        forminfrascales->hide();
    ui->btnGVPFalseColor->setChecked(false);
    ui->btnSGFalseColor->setChecked(false);
    ui->btnLCCFalseColor->setChecked(false);

    imageptrs->ptrimageProjection->fill(qRgba(0, 0, 0, 250));
    formimage->displayImage(IMAGE_PROJECTION);

}


void FormToolbox::on_spbLCCCorrX_valueChanged(int arg1)
{

    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value()*1000, ui->spbLCCCorrY->value());
        formimage->UpdateProjection();
    }

}

void FormToolbox::on_spbLCCCorrY_valueChanged(int arg1)
{

    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value()*1000, ui->spbLCCCorrY->value());
        formimage->UpdateProjection();
    }

}


bool FormToolbox::comboColVIIRSOK()
{
    int cnt = 0;

    cnt += ui->comboM1->currentIndex();
    cnt += ui->comboM2->currentIndex();
    cnt += ui->comboM3->currentIndex();
    cnt += ui->comboM4->currentIndex();
    cnt += ui->comboM5->currentIndex();
    cnt += ui->comboM6->currentIndex();
    cnt += ui->comboM7->currentIndex();
    cnt += ui->comboM8->currentIndex();
    cnt += ui->comboM9->currentIndex();
    cnt += ui->comboM10->currentIndex();
    cnt += ui->comboM11->currentIndex();
    cnt += ui->comboM12->currentIndex();
    cnt += ui->comboM13->currentIndex();
    cnt += ui->comboM14->currentIndex();
    cnt += ui->comboM15->currentIndex();
    cnt += ui->comboM16->currentIndex();

    if(cnt == 6)
        return true;
    else
        return false;
}

bool FormToolbox::comboColOLCIOK()
{
    int cnt = 0;

    cnt += ui->cmbOLCI01->currentIndex();
    cnt += ui->cmbOLCI02->currentIndex();
    cnt += ui->cmbOLCI03->currentIndex();
    cnt += ui->cmbOLCI04->currentIndex();
    cnt += ui->cmbOLCI05->currentIndex();
    cnt += ui->cmbOLCI06->currentIndex();
    cnt += ui->cmbOLCI07->currentIndex();
    cnt += ui->cmbOLCI08->currentIndex();
    cnt += ui->cmbOLCI09->currentIndex();
    cnt += ui->cmbOLCI10->currentIndex();
    cnt += ui->cmbOLCI11->currentIndex();
    cnt += ui->cmbOLCI12->currentIndex();
    cnt += ui->cmbOLCI13->currentIndex();
    cnt += ui->cmbOLCI14->currentIndex();
    cnt += ui->cmbOLCI15->currentIndex();
    cnt += ui->cmbOLCI16->currentIndex();
    cnt += ui->cmbOLCI17->currentIndex();
    cnt += ui->cmbOLCI18->currentIndex();
    cnt += ui->cmbOLCI19->currentIndex();
    cnt += ui->cmbOLCI20->currentIndex();
    cnt += ui->cmbOLCI21->currentIndex();

    if(cnt == 6)
        return true;
    else
        return false;
}
bool FormToolbox::comboColGeoOK()
{
    int cnt = 0;

    cnt += ui->comboGeo1->currentIndex();
    cnt += ui->comboGeo2->currentIndex();
    cnt += ui->comboGeo3->currentIndex();
    cnt += ui->comboGeo4->currentIndex();
    cnt += ui->comboGeo5->currentIndex();
    cnt += ui->comboGeo6->currentIndex();
    cnt += ui->comboGeo7->currentIndex();
    cnt += ui->comboGeo8->currentIndex();
    cnt += ui->comboGeo9->currentIndex();
    cnt += ui->comboGeo10->currentIndex();
    cnt += ui->comboGeo11->currentIndex();

    if(cnt == 6)
        return true;
    else
        return false;
}

void FormToolbox::on_btnUpdateVIIRSImage_clicked()
{

    if(segs->seglviirsm->NbrOfSegmentsSelected() > 0)
    {
        if(!comboColVIIRSOK())
        {
            QMessageBox msgBox;
            msgBox.setText("Need color choices for 3 different bands in the VIIRS tab.");
            //msgBox.setInformativeText("Do you want to save your changes?");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Warning);
            int ret = msgBox.exec();

            switch (ret) {
            case QMessageBox::Ok:
                break;
            default:
                break;
            }

            return;
        }

        if(!forminfrascales->isHidden())
            forminfrascales->hide();
        ui->btnGVPFalseColor->setChecked(false);
        ui->btnSGFalseColor->setChecked(false);
        ui->btnLCCFalseColor->setChecked(false);

        ui->pbProgress->reset();
        formimage->ShowVIIRSMImage();
    }
}

void FormToolbox::on_btnUpdateOLCIImage_clicked()
{

    if(!comboColOLCIOK())
    {
        QMessageBox msgBox;
        msgBox.setText("Need color choices for 3 different bands in the OLCI efr tab.");
        //msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        int ret = msgBox.exec();

        switch (ret) {
        case QMessageBox::Ok:
            break;
        default:
            break;
        }

        return;
    }

    if(opts.buttonOLCIefr)
    {
        if(segs->seglolciefr->NbrOfSegmentsSelected() > 0)
        {

            ui->btnGVPFalseColor->setChecked(false);
            ui->btnSGFalseColor->setChecked(false);
            ui->btnLCCFalseColor->setChecked(false);

            ui->pbProgress->reset();
            formimage->ShowOLCIefrImage(ui->cmbHistogram->currentIndex(), ui->rdbOLCINormalized);
        }
    }
    else if(opts.buttonOLCIerr)
    {
        if(segs->seglolcierr->NbrOfSegmentsSelected() > 0)
        {

            ui->btnGVPFalseColor->setChecked(false);
            ui->btnSGFalseColor->setChecked(false);
            ui->btnLCCFalseColor->setChecked(false);

            ui->pbProgress->reset();
            formimage->ShowOLCIerrImage(ui->cmbHistogram->currentIndex(), ui->rdbOLCINormalized);
        }
    }
}

void FormToolbox::on_rbtnAColor_clicked()
{
    if(ui->rbtnAColor->isChecked())
        opts.channelontexture = 6;
}

void FormToolbox::on_rbtnACh1_clicked()
{
    if(ui->rbtnACh1->isChecked())
        opts.channelontexture = 1;
}

void FormToolbox::on_rbtnACh2_clicked()
{
    if(ui->rbtnACh2->isChecked())
        opts.channelontexture = 2;
}

void FormToolbox::on_rbtnACh3_clicked()
{
    if(ui->rbtnACh3->isChecked())
        opts.channelontexture = 3;
}

void FormToolbox::on_rbtnACh4_clicked()
{
    if(ui->rbtnACh4->isChecked())
        opts.channelontexture = 4;
}

void FormToolbox::on_rbtnACh5_clicked()
{
    if(ui->rbtnACh5->isChecked())
        opts.channelontexture = 5;
}

void FormToolbox::on_sliCLAHE_sliderMoved(int position)
{
    opts.clahecliplimit = float(position)/10;
    ui->lblCLAHE->setText(QString("%1").arg(double(opts.clahecliplimit), 0, 'f', 1));
}

void FormToolbox::createFilenamestring(QString sat, QString d, QVector<QString> spectrum)
{
    QString outstring;
    outstring.append(sat + "_" + d.mid(0, 4) + d.mid(5, 2) + d.mid(8, 2) + d.mid(13, 2) + d.mid(16, 2));

    if (spectrum.at(0) != "")
    {
        outstring.append("_");
        outstring.append(spectrum.at(0));
    }
    if (spectrum.at(1) != "")
    {
        outstring.append("_");
        outstring.append(spectrum.at(1));
    }
    if (spectrum.at(2) != "")
    {
        outstring.append("_");
        outstring.append(spectrum.at(2));
    }
    outstring.append(".png");

    filenamecreated = outstring;

}

void FormToolbox::on_cbProjResolutions_currentIndexChanged(int index)
{
    qDebug() << QString("on_cbProjResolutions_currentIndexChanged(int index) = %1").arg(index);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if(index > 0)
    {
        if(ui->toolBox->currentIndex() == 0)
        {
            ui->spbLCCMapWidth->setValue(resolutionX.at(index-1));
            ui->spbLCCMapHeight->setValue(resolutionY.at(index-1));
            imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(),  ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                       ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
            formimage->UpdateProjection();
        }
        else if(ui->toolBox->currentIndex() == 1)
        {
            ui->spbGVPMapWidth->setValue(resolutionX.at(index-1));
            ui->spbGVPMapHeight->setValue(resolutionY.at(index-1));
            imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), ui->spbGVPMapWidth->value(), ui->spbGVPMapHeight->value());
            formimage->UpdateProjection();
        }
        else if(ui->toolBox->currentIndex() == 2)
        {
            ui->spbSGMapWidth->setValue(resolutionX.at(index-1));
            ui->spbSGMapHeight->setValue(resolutionY.at(index-1));
            imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), ui->spbSGMapWidth->value(), ui->spbSGMapHeight->value(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
            formimage->UpdateProjection();
        }
    }
    QApplication::restoreOverrideCursor();
}

void FormToolbox::on_sbCentreBand_valueChanged(int value)
{
    qDebug() << "FormToolbox::on_sbCentreBand_valueChanged(int value)";

    float fval = value/20.0;
    opts.dnbsbvalue = value;

    ui->sbCentreBand->blockSignals(true);
    segs->seglviirsdnb->sliderCentreBandChanged(value);
    float fval1 = pow(10, fval);
    ui->lblCentreBand->setText(QString("%1").arg(fval1, 0, 'E', 2));
    ui->lblTitleCentreBand->setText(QString("Centre Band from %1 to %2 [W/cm² sr]").arg(fval1/pow(10, opts.dnbspbwindowsvalue), 0, 'E', 2).arg(fval1*pow(10, opts.dnbspbwindowsvalue), 0, 'E', 2));
    ui->sbCentreBand->blockSignals(false);
}



void FormToolbox::on_spbDnbWindow_valueChanged(int arg1)
{
    if(opts.dnbspbwindowsvalue == arg1)
        return;
    ui->spbDnbWindow->blockSignals(true);
    qDebug() << QString("---->on_spbDnbWindow_valueChanged(int arg1) arg1 = %1").arg(arg1);
    segs->seglviirsdnb->spbWindowValueChanged(arg1, ui->sbCentreBand->value());
    ui->spbDnbWindow->blockSignals(false);
}


void FormToolbox::setLCCParameters(int strlindex)
{
    ui->spbParallel1->blockSignals(true);
    ui->spbParallel2->blockSignals(true);
    ui->spbCentral->blockSignals(true);
    ui->spbLatOrigin->blockSignals(true);
    ui->spbNorth->blockSignals(true);
    ui->spbSouth->blockSignals(true);
    ui->spbEast->blockSignals(true);
    ui->spbWest->blockSignals(true);
    ui->spbScaleX->blockSignals(true);
    ui->spbScaleY->blockSignals(true);
    ui->spbLCCMapWidth->blockSignals(true);
    ui->spbLCCMapHeight->blockSignals(true);
    ui->cbProjResolutions->blockSignals(true);
    ui->chkLCCGridOnProj->blockSignals(true);

    ui->spbParallel1->setValue(poi.strlLCCParallel1.at(strlindex).toInt());
    ui->spbParallel2->setValue(poi.strlLCCParallel2.at(strlindex).toInt());
    ui->spbCentral->setValue(poi.strlLCCCentral.at(strlindex).toInt());
    ui->spbLatOrigin->setValue(poi.strlLCCLatOrigin.at(strlindex).toInt());
    ui->spbNorth->setValue(poi.strlLCCNorth.at(strlindex).toInt());
    ui->spbSouth->setValue(poi.strlLCCSouth.at(strlindex).toInt());
    ui->spbWest->setValue(poi.strlLCCWest.at(strlindex).toInt());
    ui->spbEast->setValue(poi.strlLCCEast.at(strlindex).toInt());
    ui->spbScaleX->setValue(poi.strlLCCScaleX.at(strlindex).toDouble());
    ui->spbScaleY->setValue(poi.strlLCCScaleY.at(strlindex).toDouble());
    ui->spbLCCMapHeight->setValue(poi.strlLCCMapHeight.at(strlindex).toInt());
    ui->spbLCCMapWidth->setValue(poi.strlLCCMapWidth.at(strlindex).toInt());
    ui->chkLCCGridOnProj->setChecked(poi.strlLCCGridOnProj.at(strlindex).toInt());

    ui->cbProjResolutions->setCurrentIndex(searchResolution(poi.strlLCCMapWidth.at(strlindex).toInt(), poi.strlLCCMapHeight.at(strlindex).toInt()));

    opts.centralmeridian = poi.strlLCCCentral.at(strlindex).toInt();
    opts.mapextentnorth = poi.strlLCCNorth.at(strlindex).toInt();
    opts.mapextentsouth = poi.strlLCCSouth.at(strlindex).toInt();
    opts.mapextentwest = poi.strlLCCWest.at(strlindex).toInt();
    opts.mapextenteast = poi.strlLCCEast.at(strlindex).toInt();
    opts.maplccscalex = poi.strlLCCScaleX.at(strlindex).toDouble();
    opts.maplccscaley = poi.strlLCCScaleY.at(strlindex).toDouble();


    ui->spbParallel1->blockSignals(false);
    ui->spbParallel2->blockSignals(false);
    ui->spbCentral->blockSignals(false);
    ui->spbLatOrigin->blockSignals(false);
    ui->spbNorth->blockSignals(false);
    ui->spbSouth->blockSignals(false);
    ui->spbEast->blockSignals(false);
    ui->spbWest->blockSignals(false);
    ui->spbScaleX->blockSignals(false);
    ui->spbScaleY->blockSignals(false);
    ui->spbLCCMapWidth->blockSignals(false);
    ui->spbLCCMapHeight->blockSignals(false);
    ui->cbProjResolutions->blockSignals(false);
    ui->chkLCCGridOnProj->blockSignals(false);

    opts.mapextentnorth = poi.strlLCCNorth.at(strlindex).toInt();
    opts.mapextentsouth = poi.strlLCCSouth.at(strlindex).toInt();
    opts.mapextenteast = poi.strlLCCEast.at(strlindex).toInt();
    opts.mapextentwest = poi.strlLCCWest.at(strlindex).toInt();

}

void FormToolbox::setGVPParameters(int strlindex)
{
    ui->spbGVPlat->blockSignals(true);
    ui->spbGVPlon->blockSignals(true);
    ui->spbGVPscale->blockSignals(true);
    ui->spbGVPheight->blockSignals(true);
    ui->spbGVPMapWidth->blockSignals(true);
    ui->spbGVPMapHeight->blockSignals(true);
    ui->cbProjResolutions->blockSignals(true);
    ui->chkGVPGridOnProj->blockSignals(true);

    ui->spbGVPlat->setValue(poi.strlGVPLat.at(strlindex).toDouble());
    ui->spbGVPlon->setValue(poi.strlGVPLon.at(strlindex).toDouble());
    ui->spbGVPscale->setValue(poi.strlGVPScale.at(strlindex).toDouble());
    ui->spbGVPheight->setValue(poi.strlGVPHeight.at(strlindex).toInt());
    ui->spbGVPMapWidth->setValue(poi.strlGVPMapWidth.at(strlindex).toInt());
    ui->spbGVPMapHeight->setValue(poi.strlGVPMapHeight.at(strlindex).toInt());
    ui->chkGVPGridOnProj->setChecked(poi.strlGVPGridOnProj.at(strlindex).toInt());

    ui->cbProjResolutions->setCurrentIndex(searchResolution(poi.strlGVPMapWidth.at(strlindex).toInt(), poi.strlGVPMapHeight.at(strlindex).toInt()));

    ui->spbGVPlat->blockSignals(false);
    ui->spbGVPlon->blockSignals(false);
    ui->spbGVPscale->blockSignals(false);
    ui->spbGVPheight->blockSignals(false);
    ui->spbGVPMapWidth->blockSignals(false);
    ui->spbGVPMapHeight->blockSignals(false);
    ui->cbProjResolutions->blockSignals(false);
    ui->chkGVPGridOnProj->blockSignals(false);

    opts.mapgvplat = poi.strlGVPLat.at(strlindex).toDouble();
    opts.mapgvplon = poi.strlGVPLon.at(strlindex).toDouble();


}

void FormToolbox::setSGParameters(int strlindex)
{
    ui->spbSGlat->blockSignals(true);
    ui->spbSGlon->blockSignals(true);
    ui->spbSGRadius->blockSignals(true);
    ui->spbSGScale->blockSignals(true);
    ui->spbSGPanHorizon->blockSignals(true);
    ui->spbSGPanVert->blockSignals(true);
    ui->spbSGMapWidth->blockSignals(true);
    ui->spbSGMapHeight->blockSignals(true);
    ui->cbProjResolutions->blockSignals(true);
    ui->chkSGGridOnProj->blockSignals(true);

    ui->spbSGlat->setValue(poi.strlSGLat.at(strlindex).toDouble());
    ui->spbSGlon->setValue(poi.strlSGLon.at(strlindex).toDouble());
    ui->spbSGRadius->setValue(poi.strlSGRadius.at(strlindex).toDouble());
    ui->spbSGScale->setValue(poi.strlSGScale.at(strlindex).toDouble());
    ui->spbSGPanHorizon->setValue(poi.strlSGPanH.at(strlindex).toDouble());
    ui->spbSGPanVert->setValue(poi.strlSGPanV.at(strlindex).toDouble());
    ui->spbSGMapWidth->setValue(poi.strlSGMapWidth.at(strlindex).toInt());
    ui->spbSGMapHeight->setValue(poi.strlSGMapHeight.at(strlindex).toInt());
    ui->chkSGGridOnProj->setChecked(poi.strlSGGridOnProj.at(strlindex).toInt());

    ui->cbProjResolutions->setCurrentIndex(searchResolution(poi.strlSGMapWidth.at(strlindex).toInt(), poi.strlSGMapHeight.at(strlindex).toInt()));

    ui->spbSGlat->blockSignals(false);
    ui->spbSGlon->blockSignals(false);
    ui->spbSGRadius->blockSignals(false);
    ui->spbSGScale->blockSignals(false);
    ui->spbSGPanHorizon->blockSignals(false);
    ui->spbSGPanVert->blockSignals(false);
    ui->spbSGMapWidth->blockSignals(false);
    ui->spbSGMapHeight->blockSignals(false);
    ui->cbProjResolutions->blockSignals(false);
    ui->chkSGGridOnProj->blockSignals(false);
}

void FormToolbox::setConfigMParameters(int strlindex)
{
    qDebug() << "FormToolbox::setConfigMParameters(int strlindex)";

    int theband = poi.strlColorBandM.at(strlindex).toInt();
    if( theband == 0) // is color
    {
        setRadioButtonsMToFalse();
        ui->rbColorVIIRS->setChecked(true);
    }
    else
    {
        ui->rbColorVIIRS->setChecked(false);
        setRadioButtonsMToFalse();
        switch (theband)
        {
        case 1:
            ui->rbM1->setChecked(true);
            break;
        case 2:
            ui->rbM2->setChecked(true);
            break;
        case 3:
            ui->rbM3->setChecked(true);
            break;
        case 4:
            ui->rbM4->setChecked(true);
            break;
        case 5:
            ui->rbM5->setChecked(true);
            break;
        case 6:
            ui->rbM6->setChecked(true);
            break;
        case 7:
            ui->rbM7->setChecked(true);
            break;
        case 8:
            ui->rbM8->setChecked(true);
            break;
        case 9:
            ui->rbM9->setChecked(true);
            break;
        case 10:
            ui->rbM10->setChecked(true);
            break;
        case 11:
            ui->rbM11->setChecked(true);
            break;
        case 12:
            ui->rbM12->setChecked(true);
            break;
        case 13:
            ui->rbM13->setChecked(true);
            break;
        case 14:
            ui->rbM14->setChecked(true);
            break;
        case 15:
            ui->rbM15->setChecked(true);
            break;
        case 16:
            ui->rbM16->setChecked(true);
            break;

        }

    }

    ui->chkInverseM1->setChecked(poi.strlInverseM1.at(strlindex).toInt());
    ui->chkInverseM2->setChecked(poi.strlInverseM2.at(strlindex).toInt());
    ui->chkInverseM3->setChecked(poi.strlInverseM3.at(strlindex).toInt());
    ui->chkInverseM4->setChecked(poi.strlInverseM4.at(strlindex).toInt());
    ui->chkInverseM5->setChecked(poi.strlInverseM5.at(strlindex).toInt());
    ui->chkInverseM6->setChecked(poi.strlInverseM6.at(strlindex).toInt());
    ui->chkInverseM7->setChecked(poi.strlInverseM7.at(strlindex).toInt());
    ui->chkInverseM8->setChecked(poi.strlInverseM8.at(strlindex).toInt());
    ui->chkInverseM9->setChecked(poi.strlInverseM9.at(strlindex).toInt());
    ui->chkInverseM10->setChecked(poi.strlInverseM10.at(strlindex).toInt());
    ui->chkInverseM11->setChecked(poi.strlInverseM11.at(strlindex).toInt());
    ui->chkInverseM12->setChecked(poi.strlInverseM12.at(strlindex).toInt());
    ui->chkInverseM13->setChecked(poi.strlInverseM13.at(strlindex).toInt());
    ui->chkInverseM14->setChecked(poi.strlInverseM14.at(strlindex).toInt());
    ui->chkInverseM15->setChecked(poi.strlInverseM15.at(strlindex).toInt());
    ui->chkInverseM16->setChecked(poi.strlInverseM16.at(strlindex).toInt());

    ui->comboM1->setCurrentIndex(poi.strlComboM1.at(strlindex).toInt());
    ui->comboM2->setCurrentIndex(poi.strlComboM2.at(strlindex).toInt());
    ui->comboM3->setCurrentIndex(poi.strlComboM3.at(strlindex).toInt());
    ui->comboM4->setCurrentIndex(poi.strlComboM4.at(strlindex).toInt());
    ui->comboM5->setCurrentIndex(poi.strlComboM5.at(strlindex).toInt());
    ui->comboM6->setCurrentIndex(poi.strlComboM6.at(strlindex).toInt());
    ui->comboM7->setCurrentIndex(poi.strlComboM7.at(strlindex).toInt());
    ui->comboM8->setCurrentIndex(poi.strlComboM8.at(strlindex).toInt());
    ui->comboM9->setCurrentIndex(poi.strlComboM9.at(strlindex).toInt());
    ui->comboM10->setCurrentIndex(poi.strlComboM10.at(strlindex).toInt());
    ui->comboM11->setCurrentIndex(poi.strlComboM11.at(strlindex).toInt());
    ui->comboM12->setCurrentIndex(poi.strlComboM12.at(strlindex).toInt());
    ui->comboM13->setCurrentIndex(poi.strlComboM13.at(strlindex).toInt());
    ui->comboM14->setCurrentIndex(poi.strlComboM14.at(strlindex).toInt());
    ui->comboM15->setCurrentIndex(poi.strlComboM15.at(strlindex).toInt());
    ui->comboM16->setCurrentIndex(poi.strlComboM16.at(strlindex).toInt());


}

void FormToolbox::setRadioButtonsMToFalse()
{
    ui->rbM1->setChecked(false);
    ui->rbM2->setChecked(false);
    ui->rbM3->setChecked(false);
    ui->rbM4->setChecked(false);
    ui->rbM5->setChecked(false);
    ui->rbM6->setChecked(false);
    ui->rbM7->setChecked(false);
    ui->rbM8->setChecked(false);
    ui->rbM9->setChecked(false);
    ui->rbM10->setChecked(false);
    ui->rbM11->setChecked(false);
    ui->rbM12->setChecked(false);
    ui->rbM13->setChecked(false);
    ui->rbM14->setChecked(false);
    ui->rbM15->setChecked(false);
    ui->rbM16->setChecked(false);
}

void FormToolbox::setConfigOLCIParameters(int strlindex)
{
    qDebug() << "FormToolbox::setConfigOLCIParameters(int strlindex)";

    int theband = poi.strlColorBandOLCI.at(strlindex).toInt();
    if( theband == 0) // is color
    {
        setRadioButtonsOLCIefrToFalse();
        ui->rbColorOLCI->setChecked(true);
    }
    else
    {
        ui->rbColorOLCI->setChecked(false);
        setRadioButtonsOLCIefrToFalse();
        switch (theband)
        {
        case 1:
            ui->rbOLCI01->setChecked(true);
            break;
        case 2:
            ui->rbOLCI02->setChecked(true);
            break;
        case 3:
            ui->rbOLCI03->setChecked(true);
            break;
        case 4:
            ui->rbOLCI04->setChecked(true);
            break;
        case 5:
            ui->rbOLCI05->setChecked(true);
            break;
        case 6:
            ui->rbOLCI06->setChecked(true);
            break;
        case 7:
            ui->rbOLCI07->setChecked(true);
            break;
        case 8:
            ui->rbOLCI08->setChecked(true);
            break;
        case 9:
            ui->rbOLCI09->setChecked(true);
            break;
        case 10:
            ui->rbOLCI10->setChecked(true);
            break;
        case 11:
            ui->rbOLCI11->setChecked(true);
            break;
        case 12:
            ui->rbOLCI12->setChecked(true);
            break;
        case 13:
            ui->rbOLCI13->setChecked(true);
            break;
        case 14:
            ui->rbOLCI14->setChecked(true);
            break;
        case 15:
            ui->rbOLCI15->setChecked(true);
            break;
        case 16:
            ui->rbOLCI16->setChecked(true);
            break;
        case 17:
            ui->rbOLCI17->setChecked(true);
            break;
        case 18:
            ui->rbOLCI18->setChecked(true);
            break;
        case 19:
            ui->rbOLCI19->setChecked(true);
            break;
        case 20:
            ui->rbOLCI20->setChecked(true);
            break;
        case 21:
            ui->rbOLCI21->setChecked(true);
            break;

        }

    }

    ui->chkInverseOLCI01->setChecked(poi.strlInverseOLCI01.at(strlindex).toInt());
    ui->chkInverseOLCI02->setChecked(poi.strlInverseOLCI02.at(strlindex).toInt());
    ui->chkInverseOLCI03->setChecked(poi.strlInverseOLCI03.at(strlindex).toInt());
    ui->chkInverseOLCI04->setChecked(poi.strlInverseOLCI04.at(strlindex).toInt());
    ui->chkInverseOLCI05->setChecked(poi.strlInverseOLCI05.at(strlindex).toInt());
    ui->chkInverseOLCI06->setChecked(poi.strlInverseOLCI06.at(strlindex).toInt());
    ui->chkInverseOLCI07->setChecked(poi.strlInverseOLCI07.at(strlindex).toInt());
    ui->chkInverseOLCI08->setChecked(poi.strlInverseOLCI08.at(strlindex).toInt());
    ui->chkInverseOLCI09->setChecked(poi.strlInverseOLCI09.at(strlindex).toInt());
    ui->chkInverseOLCI10->setChecked(poi.strlInverseOLCI10.at(strlindex).toInt());
    ui->chkInverseOLCI11->setChecked(poi.strlInverseOLCI11.at(strlindex).toInt());
    ui->chkInverseOLCI12->setChecked(poi.strlInverseOLCI12.at(strlindex).toInt());
    ui->chkInverseOLCI13->setChecked(poi.strlInverseOLCI13.at(strlindex).toInt());
    ui->chkInverseOLCI14->setChecked(poi.strlInverseOLCI14.at(strlindex).toInt());
    ui->chkInverseOLCI15->setChecked(poi.strlInverseOLCI15.at(strlindex).toInt());
    ui->chkInverseOLCI16->setChecked(poi.strlInverseOLCI16.at(strlindex).toInt());
    ui->chkInverseOLCI17->setChecked(poi.strlInverseOLCI17.at(strlindex).toInt());
    ui->chkInverseOLCI18->setChecked(poi.strlInverseOLCI18.at(strlindex).toInt());
    ui->chkInverseOLCI19->setChecked(poi.strlInverseOLCI19.at(strlindex).toInt());
    ui->chkInverseOLCI20->setChecked(poi.strlInverseOLCI20.at(strlindex).toInt());
    ui->chkInverseOLCI21->setChecked(poi.strlInverseOLCI21.at(strlindex).toInt());

    qDebug() << "FormToolbox::setConfigOLCIefrParameters(int strlindex) 3";

    ui->cmbOLCI01->setCurrentIndex(poi.strlComboOLCI01.at(strlindex).toInt());
    ui->cmbOLCI02->setCurrentIndex(poi.strlComboOLCI02.at(strlindex).toInt());
    ui->cmbOLCI03->setCurrentIndex(poi.strlComboOLCI03.at(strlindex).toInt());
    ui->cmbOLCI04->setCurrentIndex(poi.strlComboOLCI04.at(strlindex).toInt());
    ui->cmbOLCI05->setCurrentIndex(poi.strlComboOLCI05.at(strlindex).toInt());
    ui->cmbOLCI06->setCurrentIndex(poi.strlComboOLCI06.at(strlindex).toInt());
    ui->cmbOLCI07->setCurrentIndex(poi.strlComboOLCI07.at(strlindex).toInt());
    ui->cmbOLCI08->setCurrentIndex(poi.strlComboOLCI08.at(strlindex).toInt());
    ui->cmbOLCI09->setCurrentIndex(poi.strlComboOLCI09.at(strlindex).toInt());
    ui->cmbOLCI10->setCurrentIndex(poi.strlComboOLCI10.at(strlindex).toInt());
    ui->cmbOLCI11->setCurrentIndex(poi.strlComboOLCI11.at(strlindex).toInt());
    ui->cmbOLCI12->setCurrentIndex(poi.strlComboOLCI12.at(strlindex).toInt());
    ui->cmbOLCI13->setCurrentIndex(poi.strlComboOLCI13.at(strlindex).toInt());
    ui->cmbOLCI14->setCurrentIndex(poi.strlComboOLCI14.at(strlindex).toInt());
    ui->cmbOLCI15->setCurrentIndex(poi.strlComboOLCI15.at(strlindex).toInt());
    ui->cmbOLCI16->setCurrentIndex(poi.strlComboOLCI16.at(strlindex).toInt());
    ui->cmbOLCI17->setCurrentIndex(poi.strlComboOLCI17.at(strlindex).toInt());
    ui->cmbOLCI18->setCurrentIndex(poi.strlComboOLCI18.at(strlindex).toInt());
    ui->cmbOLCI19->setCurrentIndex(poi.strlComboOLCI19.at(strlindex).toInt());
    ui->cmbOLCI20->setCurrentIndex(poi.strlComboOLCI20.at(strlindex).toInt());
    ui->cmbOLCI21->setCurrentIndex(poi.strlComboOLCI21.at(strlindex).toInt());

    qDebug() << "FormToolbox::setConfigOLCIefrParameters(int strlindex) 4";

}

void FormToolbox::setRadioButtonsOLCIefrToFalse()
{
    ui->rbOLCI01->setChecked(false);
    ui->rbOLCI02->setChecked(false);
    ui->rbOLCI03->setChecked(false);
    ui->rbOLCI04->setChecked(false);
    ui->rbOLCI05->setChecked(false);
    ui->rbOLCI06->setChecked(false);
    ui->rbOLCI07->setChecked(false);
    ui->rbOLCI08->setChecked(false);
    ui->rbOLCI09->setChecked(false);
    ui->rbOLCI10->setChecked(false);
    ui->rbOLCI11->setChecked(false);
    ui->rbOLCI12->setChecked(false);
    ui->rbOLCI13->setChecked(false);
    ui->rbOLCI14->setChecked(false);
    ui->rbOLCI15->setChecked(false);
    ui->rbOLCI16->setChecked(false);
}

void FormToolbox::on_comboPOI_currentIndexChanged(int index)
{

    if(ui->toolBox->currentIndex() == 0) // LCC
    {
        setLCCParameters(index);
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(),  ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
        formimage->UpdateProjection();
    }
    else if(ui->toolBox->currentIndex() == 1) // GVP
    {
        setGVPParameters(index);
        imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), ui->spbGVPMapWidth->value(), ui->spbGVPMapHeight->value());
        formimage->UpdateProjection();
    }
    else if(ui->toolBox->currentIndex() == 2) // SG
    {
        setSGParameters(index);
        imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), ui->spbSGMapWidth->value(), ui->spbSGMapHeight->value(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
        formimage->UpdateProjection();
    }

}

void FormToolbox::on_chkLCCGridOnProj_clicked()
{
    formimage->UpdateProjection();
}

void FormToolbox::on_chkGVPGridOnProj_clicked()
{
    formimage->UpdateProjection();
}

void FormToolbox::on_chkSGGridOnProj_clicked()
{
    formimage->UpdateProjection();
}

void FormToolbox::on_btnAddPOI_clicked()
{
    if(ui->lePOI->text().length() == 0)
    {
        QMessageBox msgBox;
        msgBox.setText("Fill in a name for this POI.");
        msgBox.exec();
        return;
    }

    ui->comboPOI->blockSignals(true);

    if(ui->toolBox->currentIndex() == 0) // LCC
    {
        poi.strlLCCName.append(QString("%1").arg(ui->lePOI->text()));
        poi.strlLCCParallel1.append(QString("%1").arg(ui->spbParallel1->value()));
        poi.strlLCCParallel2.append(QString("%1").arg(ui->spbParallel2->value()));
        poi.strlLCCCentral.append(QString("%1").arg(ui->spbCentral->value()));
        poi.strlLCCLatOrigin.append(QString("%1").arg(ui->spbLatOrigin->value()));
        poi.strlLCCNorth.append(QString("%1").arg(ui->spbNorth->value()));
        poi.strlLCCSouth.append(QString("%1").arg(ui->spbSouth->value()));
        poi.strlLCCEast.append(QString("%1").arg(ui->spbEast->value()));
        poi.strlLCCWest.append(QString("%1").arg(ui->spbWest->value()));
        poi.strlLCCScaleX.append(QString("%1").arg(ui->spbScaleX->value(), 0, 'f', 2));
        poi.strlLCCScaleY.append(QString("%1").arg(ui->spbScaleY->value(), 0, 'f', 2));
        poi.strlLCCMapHeight.append(QString("%1").arg(ui->spbLCCMapHeight->value()));
        poi.strlLCCMapWidth.append(QString("%1").arg(ui->spbLCCMapWidth->value()));
        poi.strlLCCGridOnProj.append(QString("%1").arg(ui->chkLCCGridOnProj->isChecked()));

        ui->comboPOI->clear();
        ui->comboPOI->addItems(poi.strlLCCName);
        ui->comboPOI->setCurrentIndex(poi.strlLCCName.count()-1);
    }
    else if(ui->toolBox->currentIndex() == 1) // GVP
    {
        poi.strlGVPName.append(QString("%1").arg(ui->lePOI->text()));
        poi.strlGVPLat.append(QString("%1").arg(ui->spbGVPlat->value(), 0, 'f', 2));
        poi.strlGVPLon.append(QString("%1").arg(ui->spbGVPlon->value(), 0, 'f', 2));
        poi.strlGVPScale.append(QString("%1").arg(ui->spbGVPscale->value(), 0, 'f', 2));
        poi.strlGVPHeight.append(QString("%1").arg(ui->spbGVPheight->value()));
        poi.strlGVPMapHeight.append(QString("%1").arg(ui->spbGVPMapHeight->value()));
        poi.strlGVPMapWidth.append(QString("%1").arg(ui->spbGVPMapWidth->value()));
        poi.strlGVPGridOnProj.append(QString("%1").arg(ui->chkGVPGridOnProj->isChecked()));

        ui->comboPOI->clear();
        ui->comboPOI->addItems(poi.strlGVPName);
        ui->comboPOI->setCurrentIndex(poi.strlGVPName.count()-1);

    }
    else if(ui->toolBox->currentIndex() == 2) // SG
    {
        poi.strlSGName.append(QString("%1").arg(ui->lePOI->text()));
        poi.strlSGLat.append(QString("%1").arg(ui->spbSGlat->value(), 0, 'f', 2));
        poi.strlSGLon.append(QString("%1").arg(ui->spbSGlon->value(), 0, 'f', 2));
        poi.strlSGRadius.append(QString("%1").arg(ui->spbSGRadius->value(), 0, 'f', 2));
        poi.strlSGScale.append(QString("%1").arg(ui->spbSGScale->value(), 0, 'f', 2));
        poi.strlSGPanH.append(QString("%1").arg(ui->spbSGPanHorizon->value()));
        poi.strlSGPanV.append(QString("%1").arg(ui->spbSGPanVert->value()));
        poi.strlSGMapHeight.append(QString("%1").arg(ui->spbSGMapHeight->value()));
        poi.strlSGMapWidth.append(QString("%1").arg(ui->spbSGMapWidth->value()));
        poi.strlSGGridOnProj.append(QString("%1").arg(ui->chkSGGridOnProj->isChecked()));

        ui->comboPOI->clear();
        ui->comboPOI->addItems(poi.strlSGName);
        ui->comboPOI->setCurrentIndex(poi.strlSGName.count()-1);

    }

    ui->comboPOI->blockSignals(false);

    ui->lePOI->setText("");

}


void FormToolbox::on_comboMConfig_currentIndexChanged(int index)
{
    setConfigMParameters(index);
}

void FormToolbox::on_comboOLCIConfig_currentIndexChanged(int index)
{
    setConfigOLCIParameters(index);
}

void FormToolbox::on_btnAddMConfig_clicked()
{
    if(ui->leMConfig->text().length() == 0)
    {
        QMessageBox msgBox;
        msgBox.setText("Fill in a name for this Configuration.");
        msgBox.exec();
        return;
    }

    ui->comboMConfig->blockSignals(true);

    poi.strlConfigNameM.append(QString("%1").arg(ui->leMConfig->text()));

    if(ui->rbColorVIIRS->isChecked())
        poi.strlColorBandM.append("0");
    else if(ui->rbM1->isChecked())
        poi.strlColorBandM.append("1");
    else if(ui->rbM2->isChecked())
        poi.strlColorBandM.append("2");
    else if(ui->rbM3->isChecked())
        poi.strlColorBandM.append("3");
    else if(ui->rbM4->isChecked())
        poi.strlColorBandM.append("4");
    else if(ui->rbM5->isChecked())
        poi.strlColorBandM.append("5");
    else if(ui->rbM6->isChecked())
        poi.strlColorBandM.append("6");
    else if(ui->rbM7->isChecked())
        poi.strlColorBandM.append("7");
    else if(ui->rbM8->isChecked())
        poi.strlColorBandM.append("8");
    else if(ui->rbM9->isChecked())
        poi.strlColorBandM.append("9");
    else if(ui->rbM10->isChecked())
        poi.strlColorBandM.append("10");
    else if(ui->rbM11->isChecked())
        poi.strlColorBandM.append("11");
    else if(ui->rbM12->isChecked())
        poi.strlColorBandM.append("12");
    else if(ui->rbM13->isChecked())
        poi.strlColorBandM.append("13");
    else if(ui->rbM14->isChecked())
        poi.strlColorBandM.append("14");
    else if(ui->rbM15->isChecked())
        poi.strlColorBandM.append("15");
    else if(ui->rbM16->isChecked())
        poi.strlColorBandM.append("16");

//    poi.strlColorsM.append(QString("%1").arg(ui->rbColorVIIRS->isChecked()));

//    poi.strlBandM1.append(QString("%1").arg(ui->rbM1->isChecked()));
//    poi.strlBandM2.append(QString("%1").arg(ui->rbM2->isChecked()));
//    poi.strlBandM3.append(QString("%1").arg(ui->rbM3->isChecked()));
//    poi.strlBandM4.append(QString("%1").arg(ui->rbM4->isChecked()));
//    poi.strlBandM5.append(QString("%1").arg(ui->rbM5->isChecked()));
//    poi.strlBandM6.append(QString("%1").arg(ui->rbM6->isChecked()));
//    poi.strlBandM7.append(QString("%1").arg(ui->rbM7->isChecked()));
//    poi.strlBandM8.append(QString("%1").arg(ui->rbM8->isChecked()));
//    poi.strlBandM9.append(QString("%1").arg(ui->rbM9->isChecked()));
//    poi.strlBandM10.append(QString("%1").arg(ui->rbM10->isChecked()));
//    poi.strlBandM11.append(QString("%1").arg(ui->rbM11->isChecked()));
//    poi.strlBandM12.append(QString("%1").arg(ui->rbM12->isChecked()));
//    poi.strlBandM13.append(QString("%1").arg(ui->rbM13->isChecked()));
//    poi.strlBandM14.append(QString("%1").arg(ui->rbM14->isChecked()));
//    poi.strlBandM15.append(QString("%1").arg(ui->rbM15->isChecked()));
//    poi.strlBandM16.append(QString("%1").arg(ui->rbM16->isChecked()));

    poi.strlInverseM1.append(QString("%1").arg(ui->chkInverseM1->isChecked()));
    poi.strlInverseM2.append(QString("%1").arg(ui->chkInverseM2->isChecked()));
    poi.strlInverseM3.append(QString("%1").arg(ui->chkInverseM3->isChecked()));
    poi.strlInverseM4.append(QString("%1").arg(ui->chkInverseM4->isChecked()));
    poi.strlInverseM5.append(QString("%1").arg(ui->chkInverseM5->isChecked()));
    poi.strlInverseM6.append(QString("%1").arg(ui->chkInverseM6->isChecked()));
    poi.strlInverseM7.append(QString("%1").arg(ui->chkInverseM7->isChecked()));
    poi.strlInverseM8.append(QString("%1").arg(ui->chkInverseM8->isChecked()));
    poi.strlInverseM9.append(QString("%1").arg(ui->chkInverseM9->isChecked()));
    poi.strlInverseM10.append(QString("%1").arg(ui->chkInverseM10->isChecked()));
    poi.strlInverseM11.append(QString("%1").arg(ui->chkInverseM11->isChecked()));
    poi.strlInverseM12.append(QString("%1").arg(ui->chkInverseM12->isChecked()));
    poi.strlInverseM13.append(QString("%1").arg(ui->chkInverseM13->isChecked()));
    poi.strlInverseM14.append(QString("%1").arg(ui->chkInverseM14->isChecked()));
    poi.strlInverseM15.append(QString("%1").arg(ui->chkInverseM15->isChecked()));
    poi.strlInverseM16.append(QString("%1").arg(ui->chkInverseM16->isChecked()));

    poi.strlComboM1.append(QString("%1").arg(ui->comboM1->currentIndex()));
    poi.strlComboM2.append(QString("%1").arg(ui->comboM2->currentIndex()));
    poi.strlComboM3.append(QString("%1").arg(ui->comboM3->currentIndex()));
    poi.strlComboM4.append(QString("%1").arg(ui->comboM4->currentIndex()));
    poi.strlComboM5.append(QString("%1").arg(ui->comboM5->currentIndex()));
    poi.strlComboM6.append(QString("%1").arg(ui->comboM6->currentIndex()));
    poi.strlComboM7.append(QString("%1").arg(ui->comboM7->currentIndex()));
    poi.strlComboM8.append(QString("%1").arg(ui->comboM8->currentIndex()));
    poi.strlComboM9.append(QString("%1").arg(ui->comboM9->currentIndex()));
    poi.strlComboM10.append(QString("%1").arg(ui->comboM10->currentIndex()));
    poi.strlComboM11.append(QString("%1").arg(ui->comboM11->currentIndex()));
    poi.strlComboM12.append(QString("%1").arg(ui->comboM12->currentIndex()));
    poi.strlComboM13.append(QString("%1").arg(ui->comboM13->currentIndex()));
    poi.strlComboM14.append(QString("%1").arg(ui->comboM14->currentIndex()));
    poi.strlComboM15.append(QString("%1").arg(ui->comboM15->currentIndex()));
    poi.strlComboM16.append(QString("%1").arg(ui->comboM16->currentIndex()));

    ui->comboMConfig->clear();
    ui->comboMConfig->addItems(poi.strlConfigNameM);
    ui->comboMConfig->setCurrentIndex(poi.strlConfigNameM.count()-1);
    ui->comboMConfig->blockSignals(false);
    ui->leMConfig->setText("");

}


void FormToolbox::on_btnGVPFalseColor_clicked()
{

    if(imageptrs->ptrProjectionInfra.isNull())
    {
        QMessageBox msgBox;
        msgBox.setText("Only for VIIRS channels M12 to M16.");
        msgBox.exec();

        ui->btnGVPFalseColor->setChecked(false);
        return;
    }

    if(forminfrascales->isHidden())
    {
        QList<bool> blist = this->getVIIRSMBandList();
//        if(formimage->getSegmentType() == eSegmentType::SEG_VIIRSM &&
        if(ui->rdbVIIRSMin->isChecked() &&
                (blist.at(12) == true || blist.at(13) == true || blist.at(14) == true || blist.at(15) == true || blist.at(16) == true ))
        {
            ui->btnGVPFalseColor->setChecked(true);
            forminfrascales->show();
            forminfrascales->initializeLowHigh();
            formimage->ToInfraColorProjection();
            formimage->displayImage(IMAGE_PROJECTION);
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("Only for VIIRS channels M12 to M16.");
            msgBox.exec();
            ui->btnGVPFalseColor->setChecked(false);
        }
    }
    else
    {
        ui->btnGVPFalseColor->setChecked(false);
        if(!forminfrascales->isHidden())
            forminfrascales->hide();
        ui->btnGVPFalseColor->setChecked(false);
        ui->btnSGFalseColor->setChecked(false);
        ui->btnLCCFalseColor->setChecked(false);

        formimage->FromInfraColorProjection();
        formimage->displayImage(IMAGE_PROJECTION);
    }

}

void FormToolbox::on_btnLCCFalseColor_clicked()
{
    if(imageptrs->ptrProjectionInfra.isNull())
    {
        ui->btnLCCFalseColor->setChecked(false);
        return;
    }

    if(forminfrascales->isHidden())
    {
        QList<bool> blist = this->getVIIRSMBandList();
//        if(formimage->getSegmentType() == eSegmentType::SEG_VIIRSM &&
        if(ui->rdbVIIRSMin->isChecked() &&
                (blist.at(12) == true || blist.at(13) == true || blist.at(14) == true || blist.at(15) == true || blist.at(16) == true ))
        {
            ui->btnLCCFalseColor->setChecked(true);
            forminfrascales->show();
            forminfrascales->initializeLowHigh();
            formimage->ToInfraColorProjection();
            formimage->displayImage(IMAGE_PROJECTION);
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("Only for VIIRS channels M12 to M16.");
            msgBox.exec();
            ui->btnLCCFalseColor->setChecked(false);
        }
    }
    else
    {
        ui->btnLCCFalseColor->setChecked(false);
        if(!forminfrascales->isHidden())
            forminfrascales->hide();
        ui->btnGVPFalseColor->setChecked(false);
        ui->btnSGFalseColor->setChecked(false);
        ui->btnLCCFalseColor->setChecked(false);

        formimage->FromInfraColorProjection();
        formimage->displayImage(IMAGE_PROJECTION);
    }

}

void FormToolbox::on_btnSGFalseColor_clicked()
{
    if(imageptrs->ptrProjectionInfra.isNull())
    {
        ui->btnSGFalseColor->setChecked(false);
        return;
    }

    if(forminfrascales->isHidden())
    {
        QList<bool> blist = this->getVIIRSMBandList();
//        if(formimage->getSegmentType() == eSegmentType::SEG_VIIRSM &&
        if(ui->rdbVIIRSMin->isChecked() &&
                (blist.at(12) == true || blist.at(13) == true || blist.at(14) == true || blist.at(15) == true || blist.at(16) == true ))
        {
            ui->btnSGFalseColor->setChecked(true);
            forminfrascales->show();
            forminfrascales->initializeLowHigh();
            formimage->ToInfraColorProjection();
            formimage->displayImage(IMAGE_PROJECTION);
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("Only for VIIRS channels M12 to M16.");
            msgBox.exec();
            ui->btnSGFalseColor->setChecked(false);
        }
    }
    else
    {
        ui->btnSGFalseColor->setChecked(false);
        if(!forminfrascales->isHidden())
            forminfrascales->hide();
        ui->btnGVPFalseColor->setChecked(false);
        ui->btnSGFalseColor->setChecked(false);
        ui->btnLCCFalseColor->setChecked(false);

        formimage->FromInfraColorProjection();
        formimage->displayImage(IMAGE_PROJECTION);
    }

}

void FormToolbox::slotDisplayDNBGraph()
{

    qDebug() << QString("FormToolbox::slotDisplayDNBGraph() valuerange = %1").arg(valueRangeDNBGraph);

    long valmax = 0;

    for(int j = 0; j < 150; j++)
    {
        for(int xzenith = 0; xzenith < 180; xzenith++)
        {
            int index = j * 180 + xzenith;
            long val = segs->seglviirsdnb->graphvalues.operator [](index);
            if(val > valmax)
                valmax = val;
        }
    }

    qDebug() << QString("FormToolbox::slotDisplayDNBGraph() valmax = %1").arg(valmax);

    for(int j = 0; j < 150; j++)
    {
        for(int xzenith = 0; xzenith < 180; xzenith++)
        {
            colorMap->data()->setCell(xzenith, j, -valmax);
        }
    }

    for(int j = 0; j < 150; j++)
    {
        for(int xzenith = 0; xzenith < 180; xzenith++)
        {
            int index = j * 180 + xzenith;
            long val = segs->seglviirsdnb->graphvalues.operator [](index);
            if(val > 0)
                colorMap->data()->setCell(xzenith, j, val);
        }
    }



    ui->graph->addLayer("mylayer");
    ui->graph->setCurrentLayer("mylayer");
    fitCurve();

    colorMap->rescaleDataRange(true);
    ui->graph->rescaleAxes();
    ui->graph->replot();

    setValueProgressBar(100);
}

void FormToolbox::setLogValue(int deg, double rad)
{
    int y = floor((valueRangeDNBGraph + log10(rad)) * 149 / valueRangeDNBGraph);
    colorMap->data()->setCell(deg, y, 1);
}

void FormToolbox::fitCurve()
{


//    QVector<double> xmax(180), ymax(180); // initialize with entries 0..100
//    QVector<double> x, y;

////    for(int i = 149; i >= 0; i--)
////    {

////            double lowind = (double)(i % 10 + 1)/10.0;
////            double upperind = (double)(i % 10)/10.0;
////            double lowerlimit2 = pow(10, -lowind - (int)(i/10.0));
////            double upperlimit2 = pow(10, -upperind - (int)(i/10.0));
////            int index = 149 - i;
////            qDebug() << QString("---- index = %1 lowelimit2 = %2  upperlimit2 = %3")
////                        .arg(index).arg(lowerlimit2).arg(upperlimit2);
////    }


//    for(int xzenith = 0; xzenith < 180; xzenith++)
//    {
//        xmax[xzenith] = xzenith;
//        ymax[xzenith] = 0;
//        double radval = 0;
//        long maxval = 0;
//        int jmax = 0;

//        for(int j = 149; j >= 0; j--)
//        {
//            int index = j * 180 + xzenith;
//            long val = segs->seglviirsdnb->graphvalues.operator [](index);
//            if(val > maxval)
//            {
//                maxval = val;
//                jmax = 149 - j;
//            }
//        }
//        //qDebug() << QString("maxval = %1 jmax = %2").arg(maxval).arg(jmax);

//        if(maxval > 0)
//        {
//            double lowind = (double)((jmax % 10) + 1)/10.0;
//            double upperind = (double)(jmax % 10)/10.0;
//            double lowerlimit = pow(10, -lowind - (int)(jmax/10));
//            double upperlimit = pow(10, -upperind - (int)(jmax/10));

//            ymax[xzenith] = upperlimit;
//          //  qDebug() << QString("lowlimit = %1   upperlimit = %2").arg(lowerlimit, 0, 'E', 3).arg(upperlimit, 0, 'E', 3);
//        }
//    }

//    for(int i = 0; i < 180; i++)
//    {
//        if(ymax[i] > 0)
//        {
//            x.append(xmax.at(i));
//            y.append(ymax.at(i));
//        }
//    }
    // create graph and assign data to it:
    ui->graph->addGraph();
    ui->graph->graph(0)->setData(segs->seglviirsdnb->xDNBcurve, segs->seglviirsdnb->yDNBcurve);

    ui->graph->graph(0)->setLineStyle(QCPGraph::lsLine );

    QCPScatterStyle myScatter;
    myScatter.setShape(QCPScatterStyle::ssCross); // ssCircle);
    myScatter.setPen(QPen(Qt::blue));
    // myScatter.setBrush(Qt::red);
    myScatter.setSize(5);
    ui->graph->graph(0)->setScatterStyle(myScatter);


}

void FormToolbox::on_rdbOLCINormalized_toggled(bool checked)
{
    formimage->setHistogramMethod(ui->cmbHistogram->currentIndex(), checked);
}

void FormToolbox::on_cmbHistogram_activated(int index)
{
    formimage->setHistogramMethod(ui->cmbHistogram->currentIndex(), ui->rdbOLCINormalized->isChecked());

    if(opts.buttonOLCIefr)
    {
        if(segs->seglolciefr->NbrOfSegmentsSelected() > 0)
        {

            ui->btnGVPFalseColor->setChecked(false);
            ui->btnSGFalseColor->setChecked(false);
            ui->btnLCCFalseColor->setChecked(false);

            ui->pbProgress->reset();
            formimage->ShowHistogramImage(ui->cmbHistogram->currentIndex(), ui->rdbOLCINormalized->isChecked());
        }
    }
    else if(opts.buttonOLCIerr)
    {
        if(segs->seglolcierr->NbrOfSegmentsSelected() > 0)
        {

            ui->btnGVPFalseColor->setChecked(false);
            ui->btnSGFalseColor->setChecked(false);
            ui->btnLCCFalseColor->setChecked(false);

            ui->pbProgress->reset();
            formimage->ShowHistogramImage(ui->cmbHistogram->currentIndex(), ui->rdbOLCINormalized->isChecked());
        }
    }

}

void FormToolbox::setAllWhatsThis()
{

    const QString htmlText1 =
    "<b>None 95%</b><br>"
    "Linear mapping of 95% of the pixels to 0 - 255. The lower 2.5% of the pixel values are set to 0 and the higher 2.5% of the pixel values are set to 255. <br><br>"
    "<b>None 100%</b><br>"
    "Linear mapping of 100% of the pixels to 0 - 255<br><br>"
    "<b>Equalize%</b><br>"
    "Histogram equalization of the projected image.<br>The LUT is calculated over the complete input image.<br><br>"
    "<b>Equalize Projection</b><br>"
    "Histogram equalization of the projected image.<br>The LUT is calculated only over the pixels of the projected image.";


    ui->cmbHistogramProj->setWhatsThis(htmlText1);
    ui->cmbHistogram->setWhatsThis(htmlText1);

    const QString htmlText2 =
    "<b>Normalized</b><br>"
    "The values of the radiances are divided with the cosine of the Sun Zenith Angle.";
    ui->rdbOLCINormalized->setWhatsThis(htmlText2);
    ui->rdbOLCIprojNormalized->setWhatsThis(htmlText2);

    const QString htmlText3 =
    "A 48bit RGB PNG image as 16 bits per colorchannel. The RGB values of the PNG contains the radiances multiplied by 10";
    ui->btnSaveAsPNG48bits->setWhatsThis(htmlText3);

    const QString htmlText31 =
    "When set the radiances will be linear mapped to 0 - 65535.";
    ui->rdbMapTo65535->setWhatsThis(htmlText31);

    const QString htmlText4 =
    "A Lambert conformal conic projection (LCC) is a conic map projection used for aeronautical charts, portions of the State Plane Coordinate System, and many national and regional mapping systems.";
    ui->pageLambert->setWhatsThis(htmlText4);

    const QString htmlText5 =
    "The point of perspective for the General Vertical Perspective Projection (GVP) is a finite distance."
    "It depicts the earth as it appears from some relatively short distance above the surface, typically a few hundred to a few tens of thousands of kilometers.";
    ui->pagePerspective->setWhatsThis(htmlText5);

    const QString htmlText6 =
    "The stereographic projection was known to Hipparchus, Ptolemy and probably earlier to the Egyptians. It was originally known as the planisphere projection. One of its most important uses was the representation of celestial charts.";
    ui->pageStereographic->setWhatsThis(htmlText6);

    const QString htmlText7 =
    "For making a projection (LCC, GVP or SG) select one of the input images.<br><br>"
            "<b>AVHRR image</b><br>"
            "Input Images from the NOAA-19, Metop-A and B satellite<br><br>"
            "<b>VIIRS M and DNB image</b><br>"
            "Input Images from the Suomi NPP satellite<br><br>"
            "<b>OLCI EFR and ER</b><br>"
            "Input Images from the Sentinel-3A satellite<br><br>"
            "<b>Image from Geostationary satellite</b><br>"
            "Input Images from Meteosat-8,-9,-10, Fengyun 2E,2G and Himawari-8 ";

    ui->frameInputImages->setWhatsThis(htmlText7);

}

void FormToolbox::on_btnSaveAsPNG48bits_clicked()
{

    if(!formimage->SaveAsPNG48bits(ui->rdbMapTo65535->isChecked()))
    {
        QMessageBox::information( this, "Save 48bit PNG",
            "There is no input file !" );
    }

}


void FormToolbox::on_rdbGridOnOLCIimage_toggled(bool checked)
{
    opts.gridonolciimage = checked;
    formimage->toggleOverlayGridOnOLCI();
}

