#include "formtoolbox.h"
#include "ui_formtoolbox.h"
#include "msgfileaccess.h"
#include "poi.h"

extern Options opts;
extern Poi poi;
extern SegmentImage *imageptrs;

#include <QMutex>

extern QMutex g_mutex;

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

FormToolbox::FormToolbox(QWidget *parent, FormImage *p_formimage, FormGeostationary *p_formgeostationary, AVHRRSatellite *seglist) :
    QWidget(parent),
    ui(new Ui::FormToolbox)
{
    ui->setupUi(this);
    formimage = p_formimage;
    formgeostationary = p_formgeostationary;
    segs =seglist;
    filenamecreated = "";

    setupChannelCombo();
    setChannelComboBoxes();
    currentAVHRRimage = 1;


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

    ui->btnOverlayMeteosat->setText("Overlay On");
    ui->btnOverlayProjectionGVP->setText("Overlay On");
    ui->btnOverlayProjectionLCC->setText("Overlay On");
    ui->btnOverlayProjectionSG->setText(("Overlay On"));

    if(opts.lastinputprojection == 0)
        ui->rdbAVHRRin->setChecked(true);
    else if(opts.lastinputprojection == 1)
        ui->rdbVIIRSin->setChecked(true);
    else
        ui->rdbMeteosatin->setChecked(true);

    ui->cmbHRVtype->addItem("Europe");
    ui->cmbHRVtype->addItem("Full");

    connect(ui->sliMeteosatGamma, SIGNAL(sliderReleased()), this, SLOT(setMeteosatGamma()));
    connect(ui->spbMeteosatGamma, SIGNAL(valueChanged(double)), this,SLOT(setMeteosatGamma(double)));

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

    formimage->channelshown = 8;
    spectrumvector.append("");
    spectrumvector.append("");
    spectrumvector.append("");

    inversevector.append(false);
    inversevector.append(false);
    inversevector.append(false);

    ui->tabWidget->setCurrentIndex(opts.currenttabwidget);

    QStringList listResolution;
    listResolution << "User defined";
    listResolution << "4:3  SVGA   800x600";
    listResolution << "4:3  XGA   1024x768";
    listResolution << "4:3  XGA+  1152x864";
    listResolution << "4:3  SXGA  1280x960";
    listResolution << "4:3  SVGA+ 1400x1050";
    listResolution << "4:3  UXGA  1600x1200";
    listResolution << "4:3  ----  3200x2400";
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
    resolutionX.append(3200);
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
    resolutionY.append(2400);
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


    if(opts.lastVIIRSband == 0)
        ui->rbColorVIIRS->setChecked(true);
    else if(opts.lastVIIRSband == 1)
        ui->rbM1->setChecked(true);
    else if(opts.lastVIIRSband == 2)
        ui->rbM2->setChecked(true);
    else if(opts.lastVIIRSband == 3)
        ui->rbM3->setChecked(true);
    else if(opts.lastVIIRSband == 4)
        ui->rbM4->setChecked(true);
    else if(opts.lastVIIRSband == 5)
        ui->rbM5->setChecked(true);
    else if(opts.lastVIIRSband == 6)
        ui->rbM6->setChecked(true);
    else if(opts.lastVIIRSband == 7)
        ui->rbM7->setChecked(true);
    else if(opts.lastVIIRSband == 8)
        ui->rbM8->setChecked(true);
    else if(opts.lastVIIRSband == 9)
        ui->rbM9->setChecked(true);
    else if(opts.lastVIIRSband == 10)
        ui->rbM10->setChecked(true);
    else if(opts.lastVIIRSband == 11)
        ui->rbM11->setChecked(true);
    else if(opts.lastVIIRSband == 12)
        ui->rbM12->setChecked(true);
    else if(opts.lastVIIRSband == 13)
        ui->rbM13->setChecked(true);
    else if(opts.lastVIIRSband == 14)
        ui->rbM14->setChecked(true);
    else if(opts.lastVIIRSband == 15)
        ui->rbM15->setChecked(true);
    else if(opts.lastVIIRSband == 16)
        ui->rbM16->setChecked(true);


    ui->comboMet006->setCurrentIndex(opts.lastcomboMet006);
    ui->comboMet008->setCurrentIndex(opts.lastcomboMet008);
    ui->comboMet016->setCurrentIndex(opts.lastcomboMet016);
    ui->comboMet039->setCurrentIndex(opts.lastcomboMet039);
    ui->comboMet062->setCurrentIndex(opts.lastcomboMet062);
    ui->comboMet073->setCurrentIndex(opts.lastcomboMet073);
    ui->comboMet087->setCurrentIndex(opts.lastcomboMet087);
    ui->comboMet097->setCurrentIndex(opts.lastcomboMet097);
    ui->comboMet108->setCurrentIndex(opts.lastcomboMet108);
    ui->comboMet120->setCurrentIndex(opts.lastcomboMet120);
    ui->comboMet134->setCurrentIndex(opts.lastcomboMet134);

    ui->chkInverseVIS006->setChecked(opts.lastinverseMet006);
    ui->chkInverseVIS008->setChecked(opts.lastinverseMet008);
    ui->chkInverseIR_016->setChecked(opts.lastinverseMet016);
    ui->chkInverseIR_039->setChecked(opts.lastinverseMet039);
    ui->chkInverseWV_062->setChecked(opts.lastinverseMet062);
    ui->chkInverseWV_073->setChecked(opts.lastinverseMet073);
    ui->chkInverseIR_087->setChecked(opts.lastinverseMet087);
    ui->chkInverseIR_097->setChecked(opts.lastinverseMet097);
    ui->chkInverseIR_108->setChecked(opts.lastinverseMet108);
    ui->chkInverseIR_120->setChecked(opts.lastinverseMet120);
    ui->chkInverseIR_134->setChecked(opts.lastinverseMet134);

    ui->comboM1->setCurrentIndex(opts.lastcomboM1);
    ui->comboM2->setCurrentIndex(opts.lastcomboM2);
    ui->comboM3->setCurrentIndex(opts.lastcomboM3);
    ui->comboM4->setCurrentIndex(opts.lastcomboM4);
    ui->comboM5->setCurrentIndex(opts.lastcomboM5);
    ui->comboM6->setCurrentIndex(opts.lastcomboM6);
    ui->comboM7->setCurrentIndex(opts.lastcomboM7);
    ui->comboM8->setCurrentIndex(opts.lastcomboM8);
    ui->comboM9->setCurrentIndex(opts.lastcomboM9);
    ui->comboM10->setCurrentIndex(opts.lastcomboM10);
    ui->comboM11->setCurrentIndex(opts.lastcomboM11);
    ui->comboM12->setCurrentIndex(opts.lastcomboM12);
    ui->comboM13->setCurrentIndex(opts.lastcomboM13);
    ui->comboM14->setCurrentIndex(opts.lastcomboM14);
    ui->comboM15->setCurrentIndex(opts.lastcomboM15);
    ui->comboM16->setCurrentIndex(opts.lastcomboM16);

    ui->chkInverseM1->setChecked(opts.lastinverseM1);
    ui->chkInverseM2->setChecked(opts.lastinverseM2);
    ui->chkInverseM3->setChecked(opts.lastinverseM3);
    ui->chkInverseM4->setChecked(opts.lastinverseM4);
    ui->chkInverseM5->setChecked(opts.lastinverseM5);
    ui->chkInverseM6->setChecked(opts.lastinverseM6);
    ui->chkInverseM7->setChecked(opts.lastinverseM7);
    ui->chkInverseM8->setChecked(opts.lastinverseM8);
    ui->chkInverseM9->setChecked(opts.lastinverseM9);
    ui->chkInverseM10->setChecked(opts.lastinverseM10);
    ui->chkInverseM11->setChecked(opts.lastinverseM11);
    ui->chkInverseM12->setChecked(opts.lastinverseM12);
    ui->chkInverseM13->setChecked(opts.lastinverseM13);
    ui->chkInverseM14->setChecked(opts.lastinverseM14);
    ui->chkInverseM15->setChecked(opts.lastinverseM15);
    ui->chkInverseM16->setChecked(opts.lastinverseM16);

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


    ui->lblUpper->setVisible(false);
    ui->lblLower->setVisible(false);
    ui->spbLower->setVisible(false);
    ui->spbUpper->setVisible(false);

    ui->chkShowLambert->setChecked(opts.mapextentlamberton);
    ui->chkShowPerspective->setChecked(opts.mapextentperspectiveon);

    whichgeo = SegmentListGeostationary::eGeoSatellite::NOGEO;

    setToolboxButtons(true);

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

void FormToolbox::writeInfoToAVHRR(QString info)
{
    ui->teAVHRR->clear();
    ui->teAVHRR->append(info);
}

void FormToolbox::writeInfoToVIIRS(QString info)
{
    ui->teVIIRS->clear();
    ui->teVIIRS->append(info);
}

void FormToolbox::writeInfoToGeo(QString info)
{
    ui->teGeo->clear();
    ui->teGeo->append(info);
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
    ui->pbProgress->setValue(val);
    //this->update();
}

void FormToolbox::setupChannelCombo()
{
    qDebug() << "FormToolbox::setupChannelCombo()";

    ui->comboCh1->addItem(tr("-"));
    ui->comboCh1->addItem(tr("R"));
    ui->comboCh1->addItem(tr("G"));
    ui->comboCh1->addItem(tr("B"));

    ui->comboCh2->addItem(tr("-"));
    ui->comboCh2->addItem(tr("R"));
    ui->comboCh2->addItem(tr("G"));
    ui->comboCh2->addItem(tr("B"));

    ui->comboCh3->addItem(tr("-"));
    ui->comboCh3->addItem(tr("R"));
    ui->comboCh3->addItem(tr("G"));
    ui->comboCh3->addItem(tr("B"));

    ui->comboCh4->addItem(tr("-"));
    ui->comboCh4->addItem(tr("R"));
    ui->comboCh4->addItem(tr("G"));
    ui->comboCh4->addItem(tr("B"));

    ui->comboCh5->addItem(tr("-"));
    ui->comboCh5->addItem(tr("R"));
    ui->comboCh5->addItem(tr("G"));
    ui->comboCh5->addItem(tr("B"));

    ui->comboMet006->addItem(tr("-"));
    ui->comboMet006->addItem(tr("R"));
    ui->comboMet006->addItem(tr("G"));
    ui->comboMet006->addItem(tr("B"));

    ui->comboMet008->addItem(tr("-"));
    ui->comboMet008->addItem(tr("R"));
    ui->comboMet008->addItem(tr("G"));
    ui->comboMet008->addItem(tr("B"));

    ui->comboMet016->addItem(tr("-"));
    ui->comboMet016->addItem(tr("R"));
    ui->comboMet016->addItem(tr("G"));
    ui->comboMet016->addItem(tr("B"));

    ui->comboMet039->addItem(tr("-"));
    ui->comboMet039->addItem(tr("R"));
    ui->comboMet039->addItem(tr("G"));
    ui->comboMet039->addItem(tr("B"));

    ui->comboMet062->addItem(tr("-"));
    ui->comboMet062->addItem(tr("R"));
    ui->comboMet062->addItem(tr("G"));
    ui->comboMet062->addItem(tr("B"));

    ui->comboMet073->addItem(tr("-"));
    ui->comboMet073->addItem(tr("R"));
    ui->comboMet073->addItem(tr("G"));
    ui->comboMet073->addItem(tr("B"));

    ui->comboMet087->addItem(tr("-"));
    ui->comboMet087->addItem(tr("R"));
    ui->comboMet087->addItem(tr("G"));
    ui->comboMet087->addItem(tr("B"));

    ui->comboMet097->addItem(tr("-"));
    ui->comboMet097->addItem(tr("R"));
    ui->comboMet097->addItem(tr("G"));
    ui->comboMet097->addItem(tr("B"));

    ui->comboMet108->addItem(tr("-"));
    ui->comboMet108->addItem(tr("R"));
    ui->comboMet108->addItem(tr("G"));
    ui->comboMet108->addItem(tr("B"));

    ui->comboMet120->addItem(tr("-"));
    ui->comboMet120->addItem(tr("R"));
    ui->comboMet120->addItem(tr("G"));
    ui->comboMet120->addItem(tr("B"));

    ui->comboMet134->addItem(tr("-"));
    ui->comboMet134->addItem(tr("R"));
    ui->comboMet134->addItem(tr("G"));
    ui->comboMet134->addItem(tr("B"));

    ui->comboM1->addItem(tr("-"));
    ui->comboM1->addItem(tr("R"));
    ui->comboM1->addItem(tr("G"));
    ui->comboM1->addItem(tr("B"));

    ui->comboM2->addItem(tr("-"));
    ui->comboM2->addItem(tr("R"));
    ui->comboM2->addItem(tr("G"));
    ui->comboM2->addItem(tr("B"));

    ui->comboM3->addItem(tr("-"));
    ui->comboM3->addItem(tr("R"));
    ui->comboM3->addItem(tr("G"));
    ui->comboM3->addItem(tr("B"));

    ui->comboM4->addItem(tr("-"));
    ui->comboM4->addItem(tr("R"));
    ui->comboM4->addItem(tr("G"));
    ui->comboM4->addItem(tr("B"));

    ui->comboM5->addItem(tr("-"));
    ui->comboM5->addItem(tr("R"));
    ui->comboM5->addItem(tr("G"));
    ui->comboM5->addItem(tr("B"));

    ui->comboM6->addItem(tr("-"));
    ui->comboM6->addItem(tr("R"));
    ui->comboM6->addItem(tr("G"));
    ui->comboM6->addItem(tr("B"));

    ui->comboM7->addItem(tr("-"));
    ui->comboM7->addItem(tr("R"));
    ui->comboM7->addItem(tr("G"));
    ui->comboM7->addItem(tr("B"));

    ui->comboM8->addItem(tr("-"));
    ui->comboM8->addItem(tr("R"));
    ui->comboM8->addItem(tr("G"));
    ui->comboM8->addItem(tr("B"));

    ui->comboM9->addItem(tr("-"));
    ui->comboM9->addItem(tr("R"));
    ui->comboM9->addItem(tr("G"));
    ui->comboM9->addItem(tr("B"));

    ui->comboM10->addItem(tr("-"));
    ui->comboM10->addItem(tr("R"));
    ui->comboM10->addItem(tr("G"));
    ui->comboM10->addItem(tr("B"));

    ui->comboM11->addItem(tr("-"));
    ui->comboM11->addItem(tr("R"));
    ui->comboM11->addItem(tr("G"));
    ui->comboM11->addItem(tr("B"));

    ui->comboM12->addItem(tr("-"));
    ui->comboM12->addItem(tr("R"));
    ui->comboM12->addItem(tr("G"));
    ui->comboM12->addItem(tr("B"));

    ui->comboM13->addItem(tr("-"));
    ui->comboM13->addItem(tr("R"));
    ui->comboM13->addItem(tr("G"));
    ui->comboM13->addItem(tr("B"));

    ui->comboM14->addItem(tr("-"));
    ui->comboM14->addItem(tr("R"));
    ui->comboM14->addItem(tr("G"));
    ui->comboM14->addItem(tr("B"));

    ui->comboM15->addItem(tr("-"));
    ui->comboM15->addItem(tr("R"));
    ui->comboM15->addItem(tr("G"));
    ui->comboM15->addItem(tr("B"));

    ui->comboM16->addItem(tr("-"));
    ui->comboM16->addItem(tr("R"));
    ui->comboM16->addItem(tr("G"));
    ui->comboM16->addItem(tr("B"));

}

void FormToolbox::setChannelComboBoxes()
{
    qDebug() << "FormToolbox::setChannelComboBoxes()";

    disconnect(ui->chkInverseCh1, SIGNAL(stateChanged(int)), 0, 0);
    disconnect(ui->chkInverseCh2, SIGNAL(stateChanged(int)), 0, 0);
    disconnect(ui->chkInverseCh3, SIGNAL(stateChanged(int)), 0, 0);
    disconnect(ui->chkInverseCh4, SIGNAL(stateChanged(int)), 0, 0);
    disconnect(ui->chkInverseCh5, SIGNAL(stateChanged(int)), 0, 0);

    if (opts.buttonMetop)
    {
        qDebug() << "metop";
        ui->comboCh1->setCurrentIndex(opts.channellistmetop.at(0).toInt());
        ui->comboCh2->setCurrentIndex(opts.channellistmetop.at(1).toInt());
        ui->comboCh3->setCurrentIndex(opts.channellistmetop.at(2).toInt());
        ui->comboCh4->setCurrentIndex(opts.channellistmetop.at(3).toInt());
        ui->comboCh5->setCurrentIndex(opts.channellistmetop.at(4).toInt());

    } else
    if (opts.buttonNoaa)
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

QList<bool> FormToolbox::getVIIRSBandList()
{
    QList<bool> viirslist;
    viirslist << ui->rbColorVIIRS->isChecked() << ui->rbM1->isChecked() << ui->rbM2->isChecked() << ui->rbM3->isChecked() << ui->rbM4->isChecked() << ui->rbM5->isChecked() << ui->rbM6->isChecked()
                 << ui->rbM7->isChecked() << ui->rbM8->isChecked() << ui->rbM9->isChecked() << ui->rbM10->isChecked() << ui->rbM11->isChecked() << ui->rbM12->isChecked()
                    << ui->rbM13->isChecked() << ui->rbM14->isChecked() << ui->rbM15->isChecked() << ui->rbM16->isChecked();
    return(viirslist);
}

QList<int> FormToolbox::getVIIRSColorList()
{
    QList<int> viirslist;
    viirslist << ui->comboM1->currentIndex() << ui->comboM2->currentIndex() << ui->comboM3->currentIndex() << ui->comboM4->currentIndex() << ui->comboM5->currentIndex()
                  << ui->comboM6->currentIndex() << ui->comboM7->currentIndex() << ui->comboM8->currentIndex() << ui->comboM9->currentIndex() << ui->comboM10->currentIndex()
                      << ui->comboM11->currentIndex() << ui->comboM12->currentIndex() << ui->comboM13->currentIndex() << ui->comboM14->currentIndex() << ui->comboM15->currentIndex()
                          << ui->comboM16->currentIndex();
    return(viirslist);
}

QList<bool> FormToolbox::getVIIRSInvertList()
{
    QList<bool> viirslist;
    viirslist << ui->chkInverseM1->isChecked() << ui->chkInverseM2->isChecked() << ui->chkInverseM3->isChecked() << ui->chkInverseM4->isChecked() << ui->chkInverseM5->isChecked()
                  << ui->chkInverseM6->isChecked() << ui->chkInverseM7->isChecked() << ui->chkInverseM8->isChecked() << ui->chkInverseM9->isChecked() << ui->chkInverseM10->isChecked()
                      << ui->chkInverseM11->isChecked() << ui->chkInverseM12->isChecked() << ui->chkInverseM13->isChecked() << ui->chkInverseM14->isChecked() << ui->chkInverseM15->isChecked()
                          << ui->chkInverseM16->isChecked();
    return(viirslist);
}

//void FormToolbox::setParameters()
//{
//    qDebug() << "FormToolbox::setParameters()";

//    ui->spbParallel1->blockSignals(true);
//    ui->spbParallel2->blockSignals(true);
//    ui->spbCentral->blockSignals(true);
//    ui->spbLatOrigin->blockSignals(true);

//    ui->spbParallel1->setValue(opts.parallel1);
//    ui->spbParallel2->setValue(opts.parallel2);
//    ui->spbCentral->setValue(opts.centralmeridian);
//    ui->spbLatOrigin->setValue(opts.latitudeoforigin);

//    ui->spbParallel1->blockSignals(false);
//    ui->spbParallel2->blockSignals(false);
//    ui->spbCentral->blockSignals(false);
//    ui->spbLatOrigin->blockSignals(false);

//    ui->spbNorth->blockSignals(true);
//    ui->spbSouth->blockSignals(true);
//    ui->spbEast->blockSignals(true);
//    ui->spbWest->blockSignals(true);

//    ui->spbNorth->setValue(opts.mapextentnorth);
//    ui->spbSouth->setValue(opts.mapextentsouth);
//    ui->spbWest->setValue(opts.mapextentwest);
//    ui->spbEast->setValue(opts.mapextenteast);

//    ui->spbNorth->blockSignals(false);
//    ui->spbSouth->blockSignals(false);
//    ui->spbEast->blockSignals(false);
//    ui->spbWest->blockSignals(false);

//    qDebug() << QString("setParameters() north = %1").arg(ui->spbNorth->value());
//    qDebug() << QString("setParameters() south = %1").arg(ui->spbSouth->value());
//    qDebug() << QString("setParameters() west = %1").arg(ui->spbWest->value());
//    qDebug() << QString("setParameters() east = %1").arg(ui->spbEast->value());

//    ui->chkShowLambert->setChecked(opts.mapextentlamberton);
//    ui->chkShowPerspective->setChecked(opts.mapextentperspectiveon);
//    ui->spbMapHeight->setValue(opts.mapheight);
//    ui->spbMapWidth->setValue(opts.mapwidth);

//    ui->cbProjResolutions->setCurrentIndex(searchResolution(opts.mapwidth, opts.mapheight));

//    ui->spbGVPlon->setValue(opts.mapgvplon);
//    ui->spbGVPlat->setValue(opts.mapgvplat);
//    ui->spbGVPheight->setValue(opts.mapgvpheight);
//    ui->spbGVPscale->setValue(opts.mapgvpscale);
//    ui->spbScaleX->setValue(opts.maplccscalex);
//    ui->spbScaleY->setValue(opts.maplccscaley);
//    ui->spbMeteosatGamma->setValue(opts.meteosatgamma);
//    int val = opts.meteosatgamma * 100;
//    ui->sliMeteosatGamma->setValue(val);
//    ui->spbSGlat->setValue(opts.mapsglat);
//    ui->spbSGlon->setValue(opts.mapsglon);
//    ui->spbSGScale->setValue(opts.mapsgscale);
//    ui->spbSGPanHorizon->setValue(opts.mapsgpanhorizon);
//    ui->spbSGPanVert->setValue(opts.mapsgpanvert);
//    ui->spbSGRadius->setValue(opts.mapsgradius);

//}

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

}

void FormToolbox::setInverseCheckBoxes()
{
    qDebug() << "FormToolbox::setInverseCheckBoxes()";

    if (opts.buttonMetop)
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
    if (opts.buttonNoaa)
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

    if (opts.buttonMetop)
    {
        opts.metop_invlist.clear();

        opts.metop_invlist << (ui->chkInverseCh1->isChecked() ? "1" : "0");
        opts.metop_invlist << (ui->chkInverseCh2->isChecked() ? "1" : "0");
        opts.metop_invlist << (ui->chkInverseCh3->isChecked() ? "1" : "0");
        opts.metop_invlist << (ui->chkInverseCh4->isChecked() ? "1" : "0");
        opts.metop_invlist << (ui->chkInverseCh5->isChecked() ? "1" : "0");
    } else
    if (opts.buttonNoaa)
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

    if (opts.buttonMetop)
    {
        opts.channellistmetop.clear();
        opts.channellistmetop << QString("%1").arg(ui->comboCh1->currentIndex());
        opts.channellistmetop << QString("%1").arg(ui->comboCh2->currentIndex());
        opts.channellistmetop << QString("%1").arg(ui->comboCh3->currentIndex());
        opts.channellistmetop << QString("%1").arg(ui->comboCh4->currentIndex());
        opts.channellistmetop << QString("%1").arg(ui->comboCh5->currentIndex());
    }
    else
    if (opts.buttonNoaa)
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
    else if(ui->rdbVIIRSin->isChecked())
        opts.lastinputprojection = 1;
    else
        opts.lastinputprojection = 2;

    if(ui->rbColorVIIRS->isChecked())
        opts.lastVIIRSband = 0;
    else if(ui->rbM1->isChecked())
        opts.lastVIIRSband = 1;
    else if(ui->rbM2->isChecked())
        opts.lastVIIRSband = 2;
    else if(ui->rbM3->isChecked())
        opts.lastVIIRSband = 3;
    else if(ui->rbM4->isChecked())
        opts.lastVIIRSband = 4;
    else if(ui->rbM5->isChecked())
        opts.lastVIIRSband = 5;
    else if(ui->rbM6->isChecked())
        opts.lastVIIRSband = 6;
    else if(ui->rbM7->isChecked())
        opts.lastVIIRSband = 7;
    else if(ui->rbM8->isChecked())
        opts.lastVIIRSband = 8;
    else if(ui->rbM9->isChecked())
        opts.lastVIIRSband = 9;
    else if(ui->rbM10->isChecked())
        opts.lastVIIRSband = 10;
    else if(ui->rbM11->isChecked())
        opts.lastVIIRSband = 11;
    else if(ui->rbM12->isChecked())
        opts.lastVIIRSband = 12;
    else if(ui->rbM13->isChecked())
        opts.lastVIIRSband = 13;
    else if(ui->rbM14->isChecked())
        opts.lastVIIRSband = 14;
    else if(ui->rbM15->isChecked())
        opts.lastVIIRSband = 15;
    else if(ui->rbM16->isChecked())
        opts.lastVIIRSband = 16;

    opts.lastcomboMet006 = ui->comboMet006->currentIndex();
    opts.lastcomboMet008 = ui->comboMet008->currentIndex();
    opts.lastcomboMet016 = ui->comboMet016->currentIndex();
    opts.lastcomboMet039 = ui->comboMet039->currentIndex();
    opts.lastcomboMet062 = ui->comboMet062->currentIndex();
    opts.lastcomboMet073 = ui->comboMet073->currentIndex();
    opts.lastcomboMet087 = ui->comboMet087->currentIndex();
    opts.lastcomboMet097 = ui->comboMet097->currentIndex();
    opts.lastcomboMet108 = ui->comboMet108->currentIndex();
    opts.lastcomboMet120 = ui->comboMet120->currentIndex();
    opts.lastcomboMet134 = ui->comboMet134->currentIndex();

    opts.lastinverseMet006 = ui->chkInverseVIS006->isChecked();
    opts.lastinverseMet008 = ui->chkInverseVIS008->isChecked();
    opts.lastinverseMet016 = ui->chkInverseIR_016->isChecked();
    opts.lastinverseMet039 = ui->chkInverseIR_039->isChecked();
    opts.lastinverseMet062 = ui->chkInverseWV_062->isChecked();
    opts.lastinverseMet073 = ui->chkInverseWV_073->isChecked();
    opts.lastinverseMet087 = ui->chkInverseIR_087->isChecked();
    opts.lastinverseMet097 = ui->chkInverseIR_097->isChecked();
    opts.lastinverseMet108 = ui->chkInverseIR_108->isChecked();
    opts.lastinverseMet120 = ui->chkInverseIR_120->isChecked();
    opts.lastinverseMet134 = ui->chkInverseIR_134->isChecked();

    opts.lastcomboM1 = ui->comboM1->currentIndex();
    opts.lastcomboM2 = ui->comboM2->currentIndex();
    opts.lastcomboM3 = ui->comboM3->currentIndex();
    opts.lastcomboM4 = ui->comboM4->currentIndex();
    opts.lastcomboM5 = ui->comboM5->currentIndex();
    opts.lastcomboM6 = ui->comboM6->currentIndex();
    opts.lastcomboM7 = ui->comboM7->currentIndex();
    opts.lastcomboM8 = ui->comboM8->currentIndex();
    opts.lastcomboM9 = ui->comboM9->currentIndex();
    opts.lastcomboM10 = ui->comboM10->currentIndex();
    opts.lastcomboM11 = ui->comboM11->currentIndex();
    opts.lastcomboM12 = ui->comboM12->currentIndex();
    opts.lastcomboM13 = ui->comboM13->currentIndex();
    opts.lastcomboM14 = ui->comboM14->currentIndex();
    opts.lastcomboM15 = ui->comboM15->currentIndex();
    opts.lastcomboM16 = ui->comboM16->currentIndex();

    opts.lastinverseM1 = ui->chkInverseM1->isChecked();
    opts.lastinverseM2 = ui->chkInverseM2->isChecked();
    opts.lastinverseM3 = ui->chkInverseM3->isChecked();
    opts.lastinverseM4 = ui->chkInverseM4->isChecked();
    opts.lastinverseM5 = ui->chkInverseM5->isChecked();
    opts.lastinverseM6 = ui->chkInverseM6->isChecked();
    opts.lastinverseM7 = ui->chkInverseM7->isChecked();
    opts.lastinverseM8 = ui->chkInverseM8->isChecked();
    opts.lastinverseM9 = ui->chkInverseM9->isChecked();
    opts.lastinverseM10 = ui->chkInverseM10->isChecked();
    opts.lastinverseM11 = ui->chkInverseM11->isChecked();
    opts.lastinverseM12 = ui->chkInverseM12->isChecked();
    opts.lastinverseM13 = ui->chkInverseM13->isChecked();
    opts.lastinverseM14 = ui->chkInverseM14->isChecked();
    opts.lastinverseM15 = ui->chkInverseM15->isChecked();
    opts.lastinverseM16 = ui->chkInverseM16->isChecked();

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
    currentAVHRRimage = 6;
    formimage->displayImage(currentAVHRRimage);
}

void FormToolbox::on_btnCh1_clicked()
{
    formimage->setKindOfImage("Chan 1");
    currentAVHRRimage = 1;
    formimage->displayImage(currentAVHRRimage);
}

void FormToolbox::on_btnCh2_clicked()
{
    formimage->setKindOfImage("Chan 2");
    currentAVHRRimage = 2;
    formimage->displayImage(currentAVHRRimage);
}

void FormToolbox::on_btnCh3_clicked()
{
    formimage->setKindOfImage("Chan 3");
    currentAVHRRimage = 3;
    formimage->displayImage(currentAVHRRimage);
}

void FormToolbox::on_btnCh4_clicked()
{
    formimage->setKindOfImage("Chan 4");
    currentAVHRRimage = 4;
    formimage->displayImage(currentAVHRRimage);
}

void FormToolbox::on_btnCh5_clicked()
{
    formimage->setKindOfImage("Chan 5");
    currentAVHRRimage = 5;
    formimage->displayImage(currentAVHRRimage);
}

void FormToolbox::on_btnExpandImage_clicked()
{
    if(formimage->channelshown == 7)
        return;
    formimage->setKindOfImage("Expanded " + formimage->getKindOfImage());
    imageptrs->ExpandImage(formimage->channelshown);
    formimage->displayImage(7);
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

    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 ||
        whichgeo == SegmentListGeostationary::MET_7 || whichgeo == SegmentListGeostationary::GOES_13 ||
        whichgeo == SegmentListGeostationary::GOES_15 || whichgeo == SegmentListGeostationary::MTSAT )

    {
        ui->btnVIS006->setText("VIS006");
        ui->btnVIS008->setText("VIS008");
        ui->btnIR016->setText("IR_016");
        ui->btnIR039->setText("IR_039");
        ui->btnWV062->setText("WV_062");
        ui->btnWV073->setText("WV_073");
        ui->btnIR087->setText("IR_087");
        ui->btnIR097->setText("IR_097");
        ui->btnIR108->setText("IR_108");
        ui->btnIR120->setText("IR_120");
        ui->btnIR134->setText("IR_134");
        ui->btnHRV->setText("HRV");
    }
    else if(whichgeo == SegmentListGeostationary::MET_7)
    {
        if(rowchosen.at(3).toInt() == 0)
            ui->btnVIS008->setEnabled(false);
        else if(rowchosen.at(4).toInt() == 0)
            ui->btnWV062->setEnabled(false);
        else if(rowchosen.at(5).toInt() == 0)
            ui->btnIR120->setEnabled(false);
    }
    else if(whichgeo == SegmentListGeostationary::GOES_13 || whichgeo == SegmentListGeostationary::GOES_15)
    {
        if(rowchosen.at(3).toInt() == 0)
            ui->btnVIS008->setEnabled(false);
        else if(rowchosen.at(4).toInt() == 0)
            ui->btnIR039->setEnabled(false);
        else if(rowchosen.at(5).toInt() == 0)
            ui->btnWV062->setEnabled(false);
        else if(rowchosen.at(6).toInt() == 0)
            ui->btnIR108->setEnabled(false);
    }
    else if(whichgeo == SegmentListGeostationary::MTSAT)
    {
        if(rowchosen.at(3).toInt() == 0)
            ui->btnVIS008->setEnabled(false);
        else if(rowchosen.at(4).toInt() == 0)
            ui->btnIR039->setEnabled(false);
        else if(rowchosen.at(5).toInt() == 0)
            ui->btnWV062->setEnabled(false);
        else if(rowchosen.at(6).toInt() == 0)
            ui->btnIR108->setEnabled(false);
        else if(rowchosen.at(7).toInt() == 0)
            ui->btnIR120->setEnabled(false);
    }
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
    {
        if(rowchosen.at(3).toInt() == 0)
            ui->btnVIS006->setEnabled(false);
        else if(rowchosen.at(4).toInt() == 0)
            ui->btnVIS008->setEnabled(false);
        else if(rowchosen.at(5).toInt() == 0)
            ui->btnIR016->setEnabled(false);
        else if(rowchosen.at(6).toInt() == 0)
            ui->btnIR039->setEnabled(false);
        else if(rowchosen.at(7).toInt() == 0)
            ui->btnWV062->setEnabled(false);
        else if(rowchosen.at(8).toInt() == 0)
            ui->btnHRV->setEnabled(false);
        ui->btnVIS006->setText("VIS");
        ui->btnVIS008->setText("IR4");
        ui->btnIR016->setText("IR3");
        ui->btnIR039->setText("IR1");
        ui->btnWV062->setText("IR2");
        ui->btnWV073->setText("");
        ui->btnIR087->setText("");
        ui->btnIR097->setText("");
        ui->btnIR108->setText("");
        ui->btnIR120->setText("");
        ui->btnIR134->setText("");
        ui->btnHRV->setText("VIS1KM");
    }
    else if(whichgeo == SegmentListGeostationary::H8)
    {
        if(rowchosen.at(13).toInt() == 0)
            ui->btnVIS006->setEnabled(false);
        else if(rowchosen.at(7).toInt() == 0)
            ui->btnVIS008->setEnabled(false);
        else if(rowchosen.at(8).toInt() == 0)
            ui->btnIR016->setEnabled(false);
        else if(rowchosen.at(6).toInt() == 0)
            ui->btnIR039->setEnabled(false);
        else if(rowchosen.at(5).toInt() == 0)
            ui->btnWV062->setEnabled(false);
        else if(rowchosen.at(9).toInt() == 0)
            ui->btnWV073->setEnabled(false);
        else if(rowchosen.at(10).toInt() == 0)
            ui->btnIR087->setEnabled(false);
        else if(rowchosen.at(3).toInt() == 0)
            ui->btnIR097->setEnabled(false);
        else if(rowchosen.at(11).toInt() == 0)
            ui->btnIR108->setEnabled(false);
        else if(rowchosen.at(4).toInt() == 0)
            ui->btnIR120->setEnabled(false);
        else if(rowchosen.at(12).toInt() == 0)
            ui->btnIR134->setEnabled(false);

        ui->btnVIS006->setText("VIS");
        ui->btnVIS008->setText("B04");
        ui->btnIR016->setText("B05");
        ui->btnIR039->setText("IR4");
        ui->btnWV062->setText("IR3");
        ui->btnWV073->setText("B10");
        ui->btnIR087->setText("B11");
        ui->btnIR097->setText("IR1");
        ui->btnIR108->setText("B14");
        ui->btnIR120->setText("IR2");
        ui->btnIR134->setText("B16");
        ui->btnHRV->setText("");
    }

}

void FormToolbox::on_btnCLAHEMeteosat_clicked()
{

    if (segs->seglmeteosat->getKindofImage().length() == 0 )
       return;
    if(formimage->channelshown == 8)
    {
        this->setToolboxButtons(false);
        QApplication::processEvents();
        formimage->recalculateCLAHE(spectrumvector, inversevector);
        formimage->slotUpdateMeteosat();
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
        ui->btnVIS006->setEnabled(false);
        ui->btnVIS008->setEnabled(state);
        ui->btnIR016->setEnabled(false);
        ui->btnIR039->setEnabled(false);
        ui->btnWV062->setEnabled(state);
        ui->btnWV073->setEnabled(false);
        ui->btnIR087->setEnabled(false);
        ui->btnIR097->setEnabled(false);
        ui->btnIR108->setEnabled(false);
        ui->btnIR120->setEnabled(state);
        ui->btnIR134->setEnabled(false);
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

        break;
    case SegmentListGeostationary::GOES_13:
    case SegmentListGeostationary::GOES_15:
        ui->btnGeoColor->setEnabled(false);
        ui->btnVIS006->setEnabled(false);
        ui->btnVIS008->setEnabled(state);
        ui->btnIR016->setEnabled(false);
        ui->btnIR039->setEnabled(state);
        ui->btnWV062->setEnabled(state);
        ui->btnWV073->setEnabled(false);
        ui->btnIR087->setEnabled(false);
        ui->btnIR097->setEnabled(false);
        ui->btnIR108->setEnabled(state);
        ui->btnIR120->setEnabled(false);
        ui->btnIR134->setEnabled(false);
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


        break;
    case SegmentListGeostationary::MTSAT:
        ui->btnGeoColor->setEnabled(false);
        ui->btnVIS006->setEnabled(false);
        ui->btnVIS008->setEnabled(state);
        ui->btnIR016->setEnabled(false);
        ui->btnIR039->setEnabled(state);
        ui->btnWV062->setEnabled(state);
        ui->btnWV073->setEnabled(false);
        ui->btnIR087->setEnabled(false);
        ui->btnIR097->setEnabled(false);
        ui->btnIR108->setEnabled(state);
        ui->btnIR120->setEnabled(state);
        ui->btnIR134->setEnabled(false);
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

        break;
    case SegmentListGeostationary::FY2E:
    case SegmentListGeostationary::FY2G:
        ui->btnGeoColor->setEnabled(state);
        ui->btnVIS006->setEnabled(state);
        ui->btnVIS008->setEnabled(state);
        ui->btnIR016->setEnabled(state);
        ui->btnIR039->setEnabled(state);
        ui->btnWV062->setEnabled(state);
        ui->btnWV073->setEnabled(false);
        ui->btnIR087->setEnabled(false);
        ui->btnIR097->setEnabled(false);
        ui->btnIR108->setEnabled(false);
        ui->btnIR120->setEnabled(false);
        ui->btnIR134->setEnabled(false);
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

        break;
    case SegmentListGeostationary::H8:
        ui->btnGeoColor->setEnabled(state);
        ui->btnVIS006->setEnabled(state);
        ui->btnVIS008->setEnabled(state);
        ui->btnIR016->setEnabled(state);
        ui->btnIR039->setEnabled(state);
        ui->btnWV062->setEnabled(state);
        ui->btnWV073->setEnabled(state);
        ui->btnIR087->setEnabled(state);
        ui->btnIR097->setEnabled(state);
        ui->btnIR108->setEnabled(state);
        ui->btnIR120->setEnabled(state);
        ui->btnIR134->setEnabled(state);
        ui->btnHRV->setEnabled(false);
        ui->cmbHRVtype->setEnabled(false);
        ui->chkColorHRV->setEnabled(false);

        ui->lblGeo1->setText("0.64");
        ui->lblGeo2->setText("0.86");
        ui->lblGeo3->setText("1.6");
        ui->lblGeo4->setText("3.9");
        ui->lblGeo5->setText("6.2");
        ui->lblGeo6->setText("7.3");
        ui->lblGeo7->setText("8.6");
        ui->lblGeo8->setText("10.4");
        ui->lblGeo9->setText("11.2");
        ui->lblGeo10->setText("12.4");
        ui->lblGeo11->setText("13.3");

        break;

    default:
        ui->btnGeoColor->setEnabled(state);
        ui->btnVIS006->setEnabled(state);
        ui->btnVIS008->setEnabled(state);
        ui->btnIR016->setEnabled(state);
        ui->btnIR039->setEnabled(state);
        ui->btnWV062->setEnabled(state);
        ui->btnWV073->setEnabled(state);
        ui->btnIR087->setEnabled(state);
        ui->btnIR097->setEnabled(state);
        ui->btnIR108->setEnabled(state);
        ui->btnIR120->setEnabled(state);
        ui->btnIR134->setEnabled(state);
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

    }

    /*    ui->btnGeoColor->setEnabled(state ? (whichgeo == SegmentListMeteosat::ELECTRO_N1 ? false : state) : state );
    ui->btnVIS006->setEnabled(state ? (whichgeo == SegmentListMeteosat::ELECTRO_N1 ? false : state) : state );
    ui->btnVIS008->setEnabled(state);
    ui->btnIR016->setEnabled(state ? (whichgeo == SegmentListMeteosat::ELECTRO_N1 ? false : state) : state );
    ui->btnIR039->setEnabled(state ? (whichgeo == SegmentListMeteosat::ELECTRO_N1 ? false : state) : state );
    ui->btnWV062->setEnabled(state ? (whichgeo == SegmentListMeteosat::ELECTRO_N1 ? false : state) : state );
    ui->btnWV073->setEnabled(state ? (whichgeo == SegmentListMeteosat::ELECTRO_N1 ? false : state) : state );
    ui->btnIR087->setEnabled(state);
    ui->btnIR097->setEnabled(state);
    ui->btnIR108->setEnabled(state);
    ui->btnIR120->setEnabled(state ? (whichgeo == SegmentListMeteosat::ELECTRO_N1 ? false : state) : state );
    ui->btnIR134->setEnabled(state ? (whichgeo == SegmentListMeteosat::ELECTRO_N1 ? false : state) : state );
    ui->btnHRV->setEnabled(state ? (whichgeo == SegmentListMeteosat::ELECTRO_N1 ? false : state) : state );
*/
    ui->btnCLAHEavhhr->setEnabled(state);
    ui->btnCLAHEMeteosat->setEnabled(state);
    ui->btnExpandImage->setEnabled(state);
    ui->btnOverlayMeteosat->setEnabled(state);
    ui->btnOverlayProjectionGVP->setEnabled(state);
    ui->btnOverlayProjectionLCC->setEnabled(state);
    ui->btnOverlayProjectionSG->setEnabled(state);
    ui->btnRotate180->setEnabled(state);

    ui->btnSetTrueColors->setEnabled(state);
    ui->btnSetNaturalColors->setEnabled(state);
    ui->btnMakeVIIRSImage->setEnabled(state);
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

//    if(state)
//    {
//        QApplication::restoreOverrideCursor();
//        ui->pbProgress->setValue(100);
//    }
//    else
//        QApplication::setOverrideCursor( Qt::WaitCursor );

}

void FormToolbox::on_btnVIS006_clicked()
{
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
        onButtonChannel("VIS006", ui->chkInverseVIS006->isChecked());
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
        onButtonChannel("VIS", ui->chkInverseVIS006->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("VIS", ui->chkInverseVIS006->isChecked());

}

void FormToolbox::on_btnVIS008_clicked()
{
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
        onButtonChannel("VIS008", ui->chkInverseVIS008->isChecked());
    //else if(whichgeo == SegmentListGeostationary::ELECTRO_N1)
    //    onButtonChannel("00_9_0", ui->chkInverseVIS008->isChecked());
    else if(whichgeo == SegmentListGeostationary::MET_7)
        onButtonChannel("00_7_0", ui->chkInverseVIS008->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_13)
        onButtonChannel("00_7_0", ui->chkInverseVIS008->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_15)
        onButtonChannel("00_7_1", ui->chkInverseVIS008->isChecked());
    else if(whichgeo == SegmentListGeostationary::MTSAT)
        onButtonChannel("00_7_1", ui->chkInverseVIS008->isChecked());
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
        onButtonChannel("IR4", ui->chkInverseVIS008->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("B04", ui->chkInverseVIS008->isChecked());

}

void FormToolbox::on_btnIR016_clicked()
{
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
        onButtonChannel("IR_016", ui->chkInverseIR_016->isChecked());
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
        onButtonChannel("IR3", ui->chkInverseIR_016->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("B05", ui->chkInverseIR_016->isChecked());


}

void FormToolbox::on_btnIR039_clicked()
{
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
        onButtonChannel("IR_039", ui->chkInverseIR_039->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_13)
        onButtonChannel("03_9_0", ui->chkInverseIR_039->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_15)
        onButtonChannel("03_9_1", ui->chkInverseIR_039->isChecked());
    else if(whichgeo == SegmentListGeostationary::MTSAT)
        onButtonChannel("03_8_1", ui->chkInverseIR_039->isChecked());
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
        onButtonChannel("IR4", ui->chkInverseIR_039->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("IR1", ui->chkInverseIR_039->isChecked());


}

void FormToolbox::on_btnWV062_clicked()
{
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
        onButtonChannel("WV_062", ui->chkInverseWV_062->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_13)
        onButtonChannel("06_6_0", ui->chkInverseWV_062->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_15)
        onButtonChannel("06_6_1", ui->chkInverseWV_062->isChecked());
    else if(whichgeo == SegmentListGeostationary::MTSAT)
        onButtonChannel("06_8_1", ui->chkInverseWV_062->isChecked());
    else if(whichgeo == SegmentListGeostationary::MET_7)
        onButtonChannel("06_4_0", ui->chkInverseWV_062->isChecked());
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
        onButtonChannel("VIS", ui->chkInverseWV_062->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("IR2", ui->chkInverseWV_062->isChecked());


}

void FormToolbox::on_btnWV073_clicked()
{
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
        onButtonChannel("WV_073", ui->chkInverseWV_073->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("B10", ui->chkInverseWV_073->isChecked());

}

void FormToolbox::on_btnIR087_clicked()
{
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
        onButtonChannel("IR_087", ui->chkInverseIR_087->isChecked());
    //else if(whichgeo == SegmentListGeostationary::ELECTRO_N1)
    //    onButtonChannel("08_0_0", ui->chkInverseIR_087->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("B11", ui->chkInverseIR_087->isChecked());

}

void FormToolbox::on_btnIR097_clicked()
{
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
        onButtonChannel("IR_097", ui->chkInverseIR_097->isChecked());
    //else if(whichgeo == SegmentListGeostationary::ELECTRO_N1)
    //    onButtonChannel("09_7_0", ui->chkInverseIR_097->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("IR1", ui->chkInverseIR_097->isChecked());

}

void FormToolbox::on_btnIR108_clicked()
{
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
        onButtonChannel("IR_108", ui->chkInverseIR_108->isChecked());
    //else if(whichgeo == SegmentListGeostationary::ELECTRO_N1)
    //    onButtonChannel("10_7_0", ui->chkInverseIR_108->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_13)
        onButtonChannel("10_7_0", ui->chkInverseIR_108->isChecked());
    else if(whichgeo == SegmentListGeostationary::GOES_15)
        onButtonChannel("10_7_1", ui->chkInverseIR_108->isChecked());
    else if(whichgeo == SegmentListGeostationary::MTSAT)
        onButtonChannel("10_8_1", ui->chkInverseIR_108->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("B14", ui->chkInverseIR_108->isChecked());

}

void FormToolbox::on_btnIR120_clicked()
{
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
        onButtonChannel("IR_120", ui->chkInverseIR_120->isChecked());
    else if(whichgeo == SegmentListGeostationary::MTSAT)
        onButtonChannel("12_0_1", ui->chkInverseIR_120->isChecked());
    else if(whichgeo == SegmentListGeostationary::MET_7)
        onButtonChannel("11_5_0", ui->chkInverseIR_120->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("IR2", ui->chkInverseIR_120->isChecked());

}

void FormToolbox::on_btnIR134_clicked()
{
    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
        onButtonChannel("IR_134", ui->chkInverseIR_134->isChecked());
    else if(whichgeo == SegmentListGeostationary::H8)
        onButtonChannel("B16", ui->chkInverseIR_134->isChecked());

}



void FormToolbox::onButtonChannel( QString channel, bool bInverse)
{
    qDebug() << QString("onButtonChannel( QString channel, bool bInverse)  ; channel = %1").arg(channel);

    ui->pbProgress->reset();

    if(whichgeo == SegmentListGeostationary::MET_10)
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
    //else if(whichgeo == SegmentListGeostationary::ELECTRO_N1)
    //    ui->pbProgress->setMaximum(6);
    else if(whichgeo == SegmentListGeostationary::GOES_13)
        ui->pbProgress->setMaximum(7);
    else if(whichgeo == SegmentListGeostationary::GOES_15)
        ui->pbProgress->setMaximum(7);
    else if(whichgeo == SegmentListGeostationary::MTSAT)
        ui->pbProgress->setMaximum(6);
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
        ui->pbProgress->setMaximum(100);
    else if(whichgeo == SegmentListGeostationary::H8)
        ui->pbProgress->setMaximum(10);

    segs->seglmeteosat->areatype = ui->cmbHRVtype->currentIndex();
    segs->seglmeteosatrss->areatype = ui->cmbHRVtype->currentIndex();
    //segs->seglelectro->areatype = 1; // europe = 0 ;full earth = 1
    segs->seglmet7->areatype = 1;
    segs->seglgoes13dc3->areatype = 1;
    segs->seglgoes15dc3->areatype = 1;
    segs->seglmtsatdc3->areatype = 1;
    segs->seglgoes13dc4->areatype = 1;
    segs->seglgoes15dc4->areatype = 1;
    segs->seglmtsatdc4->areatype = 1;
    segs->seglfy2e->areatype = 1;
    segs->seglfy2g->areatype = 1;
    segs->seglh8->areatype = 1;

    formimage->setKindOfImage(channel);
    segs->seglmeteosat->setKindofImage("VIS_IR");
    segs->seglmeteosatrss->setKindofImage("VIS_IR");
    //segs->seglelectro->setKindofImage("VIS_IR");
    segs->seglmet7->setKindofImage("VIS_IR");
    segs->seglgoes13dc3->setKindofImage("VIS_IR");
    segs->seglgoes15dc3->setKindofImage("VIS_IR");
    segs->seglmtsatdc3->setKindofImage("VIS_IR");
    segs->seglgoes13dc4->setKindofImage("VIS_IR");
    segs->seglgoes15dc4->setKindofImage("VIS_IR");
    segs->seglmtsatdc4->setKindofImage("VIS_IR");
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

    QApplication::setOverrideCursor(Qt::WaitCursor); // restore in FormImage::slotUpdateHimawari() or slotUpdateMeteosat()

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


    if(whichgeo == SegmentListGeostationary::NOGEO)
        return;
    ui->pbProgress->reset();
    if(whichgeo == SegmentListGeostationary::MET_10)
        ui->pbProgress->setMaximum(24);
    else if(whichgeo == SegmentListGeostationary::MET_9)
        ui->pbProgress->setMaximum(9);
    else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G )
        ui->pbProgress->setMaximum(100);
    else if(whichgeo == SegmentListGeostationary::H8)
        ui->pbProgress->setMaximum(30);

    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 ||
            whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G || whichgeo == SegmentListGeostationary::H8)
        onButtonColorHRV("VIS_IR Color");

}

void FormToolbox::on_btnHRV_clicked()
{

    QApplication::setOverrideCursor(Qt::WaitCursor); // restore in FormImage::slotUpdateMeteosat()

    ui->pbProgress->reset();

    if(whichgeo == SegmentListGeostationary::MET_10)
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

    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
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


    formimage->setKindOfImage(type);
    segs->seglmeteosat->setKindofImage(type);
    segs->seglmeteosatrss->setKindofImage(type);
    segs->seglfy2e->setKindofImage(type);
    segs->seglfy2g->setKindofImage(type);
    segs->seglh8->setKindofImage(type);

    formimage->displayImage(8);
    formimage->adjustPicSize(true);


    setToolboxButtons(false);

    if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
    {
        if(ui->comboMet006->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet006->currentIndex()-1] = "VIS006";
            inversevector[ui->comboMet006->currentIndex()-1] = ui->chkInverseVIS006->isChecked();
        }
        if(ui->comboMet008->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet008->currentIndex()-1] = "VIS008";
            inversevector[ui->comboMet008->currentIndex()-1] = ui->chkInverseVIS008->isChecked();
        }
        if(ui->comboMet016->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet016->currentIndex()-1] = "IR_016";
            inversevector[ui->comboMet016->currentIndex()-1] = ui->chkInverseIR_016->isChecked();
        }
        if(ui->comboMet039->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet039->currentIndex()-1] = "IR_039";
            inversevector[ui->comboMet039->currentIndex()-1] = ui->chkInverseIR_039->isChecked();
        }
        if(ui->comboMet062->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet062->currentIndex()-1] = "WV_062";
            inversevector[ui->comboMet062->currentIndex()-1] = ui->chkInverseWV_062->isChecked();
        }
        if(ui->comboMet073->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet073->currentIndex()-1] = "WV_073";
            inversevector[ui->comboMet073->currentIndex()-1] = ui->chkInverseWV_073->isChecked();
        }
        if(ui->comboMet087->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet087->currentIndex()-1] = "IR_087";
            inversevector[ui->comboMet087->currentIndex()-1] = ui->chkInverseIR_087->isChecked();
        }
        if(ui->comboMet097->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet097->currentIndex()-1] = "IR_097";
            inversevector[ui->comboMet097->currentIndex()-1] = ui->chkInverseIR_097->isChecked();
        }
        if(ui->comboMet108->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet108->currentIndex()-1] = "IR_108";
            inversevector[ui->comboMet108->currentIndex()-1] = ui->chkInverseIR_108->isChecked();
        }
        if(ui->comboMet120->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet120->currentIndex()-1] = "IR_120";
            inversevector[ui->comboMet120->currentIndex()-1] = ui->chkInverseIR_120->isChecked();
        }
        if(ui->comboMet134->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet134->currentIndex()-1] = "IR_134";
            inversevector[ui->comboMet134->currentIndex()-1] = ui->chkInverseIR_134->isChecked();
        }
    }
    else if (whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
    {
        if(ui->comboMet006->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet006->currentIndex()-1] = "IR1";
            inversevector[ui->comboMet006->currentIndex()-1] = ui->chkInverseVIS006->isChecked();
        }
        if(ui->comboMet008->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet008->currentIndex()-1] = "IR2";
            inversevector[ui->comboMet008->currentIndex()-1] = ui->chkInverseVIS008->isChecked();
        }
        if(ui->comboMet016->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet016->currentIndex()-1] = "IR3";
            inversevector[ui->comboMet016->currentIndex()-1] = ui->chkInverseIR_016->isChecked();
        }
        if(ui->comboMet039->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet039->currentIndex()-1] = "IR4";
            inversevector[ui->comboMet039->currentIndex()-1] = ui->chkInverseIR_039->isChecked();
        }
        if(ui->comboMet062->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet062->currentIndex()-1] = "VIS";
            inversevector[ui->comboMet062->currentIndex()-1] = ui->chkInverseWV_062->isChecked();
        }

    }
    else if(whichgeo == SegmentListGeostationary::H8 )
    {
        if(ui->comboMet006->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet006->currentIndex()-1] = "VIS";
            inversevector[ui->comboMet006->currentIndex()-1] = ui->chkInverseVIS006->isChecked();
        }
        if(ui->comboMet008->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet008->currentIndex()-1] = "B04";
            inversevector[ui->comboMet008->currentIndex()-1] = ui->chkInverseVIS008->isChecked();
        }
        if(ui->comboMet016->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet016->currentIndex()-1] = "B05";
            inversevector[ui->comboMet016->currentIndex()-1] = ui->chkInverseIR_016->isChecked();
        }
        if(ui->comboMet039->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet039->currentIndex()-1] = "IR4";
            inversevector[ui->comboMet039->currentIndex()-1] = ui->chkInverseIR_039->isChecked();
        }
        if(ui->comboMet062->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet062->currentIndex()-1] = "IR3";
            inversevector[ui->comboMet062->currentIndex()-1] = ui->chkInverseWV_062->isChecked();
        }
        if(ui->comboMet073->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet073->currentIndex()-1] = "B10";
            inversevector[ui->comboMet073->currentIndex()-1] = ui->chkInverseWV_073->isChecked();
        }
        if(ui->comboMet087->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet087->currentIndex()-1] = "B11";
            inversevector[ui->comboMet087->currentIndex()-1] = ui->chkInverseIR_087->isChecked();
        }
        if(ui->comboMet097->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet097->currentIndex()-1] = "IR1";
            inversevector[ui->comboMet097->currentIndex()-1] = ui->chkInverseIR_097->isChecked();
        }
        if(ui->comboMet108->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet108->currentIndex()-1] = "B14";
            inversevector[ui->comboMet108->currentIndex()-1] = ui->chkInverseIR_108->isChecked();
        }
        if(ui->comboMet120->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet120->currentIndex()-1] = "IR2";
            inversevector[ui->comboMet120->currentIndex()-1] = ui->chkInverseIR_120->isChecked();
        }
        if(ui->comboMet134->currentIndex() > 0)
        {
            spectrumvector[ui->comboMet134->currentIndex()-1] = "B16";
            inversevector[ui->comboMet134->currentIndex()-1] = ui->chkInverseIR_134->isChecked();
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

void FormToolbox::on_tabWidget_currentChanged(int index)
{
    qDebug() << "on_tabWidget_currentChanged(int index) index = " << index;

    if (index == 0) //AVHHR
    {
        formimage->displayImage(currentAVHRRimage);
    }
    else if (index == 1) // VIIRS
    {
        formimage->displayImage(10);
    }
    else if (index == 2) // Geostationair
    {
        formimage->displayImage(8);
    }
    else if (index == 3) // Projection
    {
        if( ui->toolBox->currentIndex() == 0)
            imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                       ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
        else if( ui->toolBox->currentIndex() == 1)
            imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), ui->spbGVPMapWidth->value(), ui->spbGVPMapHeight->value());
        else
            imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), ui->spbSGMapWidth->value(), ui->spbSGMapHeight->value(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
        //formimage->displayImage(9);
        //emit screenupdateprojection();
        formimage->slotUpdateProjection();
    }

//    opts.currenttabwidget = ui->tabWidget->currentIndex();
//    opts.currenttoolbox = ui->toolBox->currentIndex();
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
        //formimage->displayImage(9);
        //emit screenupdateprojection();
        formimage->slotUpdateProjection();
    }
}

void FormToolbox::on_spbScaleY_valueChanged(double arg1)
{
    opts.maplccscaley = arg1;
    if(imageptrs->ptrimageProjection->width() > 0 && opts.currenttoolbox == TAB_LLC)
    {
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
        //formimage->displayImage(9);
        //emit screenupdateprojection();
        formimage->slotUpdateProjection();
    }

}

void FormToolbox::on_spbGVPlat_valueChanged(double arg1)
{
    opts.mapgvplat = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height());
//        formimage->displayImage(9);
//        emit screenupdateprojection();
        formimage->slotUpdateProjection();
    }

}

void FormToolbox::on_spbGVPlon_valueChanged(double arg1)
{
    opts.mapgvplon = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height());
        //formimage->displayImage(9);
        //emit screenupdateprojection();
        formimage->slotUpdateProjection();
    }

}

void FormToolbox::on_spbGVPheight_valueChanged(int arg1)
{
    opts.mapgvpheight = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height());
  //      formimage->displayImage(9);
  //      emit screenupdateprojection();
        formimage->slotUpdateProjection();
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
            //formimage->displayImage(9);
            //emit screenupdateprojection();
            formimage->slotUpdateProjection();
        }
    }

}

void FormToolbox::on_btnCreatePerspective_clicked()
{

    if(opts.buttonMetop || opts.buttonNoaa || opts.buttonHRP || opts.buttonGAC)
    {
        if(ui->rdbAVHRRin->isChecked())
        {
            if(!segs->SelectedAVHRRSegments())
                return;
        }
    }
    else if(opts.buttonVIIRSM)
    {
        if(ui->rdbVIIRSin->isChecked())
        {
            if(!segs->SelectedVIIRSMSegments())
                return;
        }

    }
    else if(opts.buttonVIIRSDNB)
    {
        if(ui->rdbVIIRSin->isChecked())
        {
            if(!segs->SelectedVIIRSDNBSegments())
                return;
        }

    }

    QApplication::setOverrideCursor( Qt::WaitCursor );

    imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), ui->spbGVPMapWidth->value(), ui->spbGVPMapHeight->value());
    if(ui->rdbAVHRRin->isChecked())
    {
        imageptrs->gvp->CreateMapFromAVHRR(ui->cmbInputAVHRRChannel->currentIndex(), formimage->getSegmentType());
    }
    else if(ui->rdbVIIRSin->isChecked())
    {
        imageptrs->gvp->CreateMapFromVIIRS(formimage->getSegmentType());
    }
    else
        imageptrs->gvp->CreateMapFromGeoStationary();


    formimage->setPixmapToLabel(true);
    QApplication::restoreOverrideCursor();

}

void FormToolbox::on_btnCreateLambert_clicked()
{
    if(opts.buttonMetop || opts.buttonNoaa || opts.buttonHRP || opts.buttonGAC)
    {
        if(ui->rdbAVHRRin->isChecked())
        {
            if(!segs->SelectedAVHRRSegments())
                return;
        }
    }
    else if(opts.buttonVIIRSM)
    {
        if(ui->rdbVIIRSin->isChecked())
        {
            if(!segs->SelectedVIIRSMSegments())
                return;
        }

    }
    else if(opts.buttonVIIRSDNB)
    {
        if(ui->rdbVIIRSin->isChecked())
        {
            if(!segs->SelectedVIIRSDNBSegments())
                return;
        }

    }


    QApplication::setOverrideCursor( Qt::WaitCursor );

    imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                               ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
    if(ui->rdbAVHRRin->isChecked())
    {
        imageptrs->lcc->CreateMapFromAVHRR(ui->cmbInputAVHRRChannel->currentIndex(), formimage->getSegmentType());
    }
    else if(ui->rdbVIIRSin->isChecked())
    {
        imageptrs->lcc->CreateMapFromVIIRS(formimage->getSegmentType());
    }
    else
        imageptrs->lcc->CreateMapFromGeostationary();

    formimage->setPixmapToLabel(true);
    QApplication::restoreOverrideCursor();
}

void FormToolbox::on_btnCreateStereo_clicked()
{
    if(opts.buttonMetop || opts.buttonNoaa || opts.buttonHRP || opts.buttonGAC)
    {
        if(ui->rdbAVHRRin->isChecked())
        {
            if(!segs->SelectedAVHRRSegments())
                return;
        }
    }
    else if(opts.buttonVIIRSM)
    {
        if(ui->rdbVIIRSin->isChecked())
        {
            if(!segs->SelectedVIIRSMSegments())
                return;
        }

    }
    else if(opts.buttonVIIRSDNB)
    {
        if(ui->rdbVIIRSin->isChecked())
        {
            if(!segs->SelectedVIIRSDNBSegments())
                return;
        }

    }

    QApplication::setOverrideCursor( Qt::WaitCursor );

    imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), ui->spbSGMapWidth->value(), ui->spbSGMapHeight->value(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());

    if(ui->rdbAVHRRin->isChecked())
    {
        imageptrs->sg->CreateMapFromAVHRR(ui->cmbInputAVHRRChannel->currentIndex(), formimage->getSegmentType());
    }
    else if(ui->rdbVIIRSin->isChecked())
    {
        imageptrs->sg->CreateMapFromVIIRS(formimage->getSegmentType());
    }
    else
        imageptrs->sg->CreateMapFromGeostationary();

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
            //formimage->displayImage(9);
            //emit screenupdateprojection();
            formimage->slotUpdateProjection();
        }
}

void FormToolbox::on_spbParallel2_valueChanged(int arg1)
{
        opts.parallel2 = arg1;

        if(imageptrs->ptrimageProjection->width() > 0)
        {
            imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                       ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
            //formimage->displayImage(9);
            //emit screenupdateprojection();
            formimage->slotUpdateProjection();
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
        //formimage->displayImage(9);
        //emit screenupdateprojection();
        formimage->slotUpdateProjection();
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
            //formimage->displayImage(9);
            //emit screenupdateprojection();
            formimage->slotUpdateProjection();
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
        //formimage->displayImage(9);
        //emit screenupdateprojection();
        formimage->slotUpdateProjection();
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
        //formimage->displayImage(9);
        //emit screenupdateprojection();
        formimage->slotUpdateProjection();
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
        //formimage->displayImage(9);
        //emit screenupdateprojection();
        formimage->slotUpdateProjection();
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
       //formimage->displayImage(9);
       //emit screenupdateprojection();
       formimage->slotUpdateProjection();
   }

}



int FormToolbox::getTabWidgetIndex()
{
    return ui->tabWidget->currentIndex();
}


void FormToolbox::on_toolBox_currentChanged(int index)
{
    qDebug() << QString("FormToolbox::on_toolBox_currentChanged(int index) index = %1").arg(index);

    ui->comboPOI->blockSignals(true);
    ui->comboPOI->clear();

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
    formimage->slotUpdateProjection();
}


void FormToolbox::on_btnGVPClearMap_clicked()
{
    imageptrs->ptrimageProjection->fill(qRgba(0, 0, 0, 250));
    formimage->displayImage(9);
}

void FormToolbox::on_btnLCCClearMap_clicked()
{
    imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                               ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());

    imageptrs->ptrimageProjection->fill(qRgba(0, 0, 0, 250));
    formimage->displayImage(9);
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
        //formimage->displayImage(9);
        //emit screenupdateprojection();
        formimage->slotUpdateProjection();
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
        //formimage->displayImage(9);
        //emit screenupdateprojection();
        formimage->slotUpdateProjection();
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
        //formimage->displayImage(9);
        //emit screenupdateprojection();
        formimage->slotUpdateProjection();
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
        //formimage->displayImage(9);
        //emit screenupdateprojection();
        formimage->slotUpdateProjection();
    }

}

//void FormToolbox::on_scbLCCMapUpDown_valueChanged(int value)
//{
//    qDebug() << "on_scbLCCMapUpDown_valueChanged(int value)";

//    ui->spbNorth->setValue(ui->spbNorth->value() - value);
//    ui->spbSouth->setValue(ui->spbSouth->value() - value);

//    opts.mapextentnorth = ui->spbNorth->value();
//    opts.mapextentsouth = ui->spbSouth->value();

//    if(imageptrs->ptrimageProjection->width() > 0)
//    {
//        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
//                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
//        //formimage->displayImage(9);
//        //emit screenupdateprojection();
//        formimage->slotUpdateProjection();
//    }

//}

//void FormToolbox::on_scbLCCMapLeftRight_valueChanged(int value)
//{
//    qDebug() << "on_scbLCCMapLeftRight_valueChanged(int value)";

//    ui->spbEast->setValue(ui->spbEast->value() - value);
//    ui->spbWest->setValue(ui->spbWest->value() - value);

//    opts.mapextenteast = ui->spbEast->value();
//    opts.mapextentwest = ui->spbWest->value();

//    int central = ui->spbWest->value() + longitudediffdeg( ui->spbEast->value(), ui->spbWest->value())/2;
//    qDebug() << QString("FormToolbox::on_spbEast_valueChanged(int arg1) central = %1").arg(central);

//    ui->spbCentral->setValue(adjust_lon_deg(central));

//    if(imageptrs->ptrimageProjection->width() > 0)
//    {
//        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
//                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
//        //formimage->displayImage(9);
//        //emit screenupdateprojection();
//        formimage->slotUpdateProjection();
//    }
//}



void FormToolbox::on_spbSGlat_valueChanged(double arg1)
{
    qDebug() << "FormToolbox::on_spbSGlat_valueChanged(double arg1)";
    opts.mapsglat = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
        formimage->displayImage(9);
    }

}

void FormToolbox::on_spbSGlon_valueChanged(double arg1)
{
    opts.mapsglon = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
        formimage->displayImage(9);
    }

}

void FormToolbox::on_spbSGScale_valueChanged(double arg1)
{
    opts.mapsgscale = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
        formimage->displayImage(9);
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
        formimage->displayImage(9);
    }

}

void FormToolbox::on_spbSGPanVert_valueChanged(int arg1)
{
    opts.mapsgpanvert = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
        formimage->displayImage(9);
    }

}

void FormToolbox::on_spbSGRadius_valueChanged(double arg1)
{
    opts.mapsgradius = arg1;
    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), imageptrs->ptrimageProjection->width(), imageptrs->ptrimageProjection->height(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
        formimage->displayImage(9);
    }

}

void FormToolbox::on_btnSGClearMap_clicked()
{
    imageptrs->ptrimageProjection->fill(qRgba(0, 0, 0, 250));
    formimage->displayImage(9);

}


void FormToolbox::on_spbLCCCorrX_valueChanged(int arg1)
{

    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value()*1000, ui->spbLCCCorrY->value());
        //formimage->displayImage(9);
        //emit screenupdateprojection();
        formimage->slotUpdateProjection();
    }

}

void FormToolbox::on_spbLCCCorrY_valueChanged(int arg1)
{

    if(imageptrs->ptrimageProjection->width() > 0)
    {
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(), ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value()*1000, ui->spbLCCCorrY->value());
        //formimage->displayImage(9);
        //emit screenupdateprojection();
        formimage->slotUpdateProjection();
    }

}

void FormToolbox::on_btnSetTrueColors_clicked()
{
    ui->rbColorVIIRS->setChecked(true);
    ui->comboM1->setCurrentIndex(0);
    ui->comboM2->setCurrentIndex(0);
    ui->comboM3->setCurrentIndex(3);
    ui->comboM4->setCurrentIndex(2);
    ui->comboM5->setCurrentIndex(1);
    ui->comboM6->setCurrentIndex(0);
    ui->comboM7->setCurrentIndex(0);
    ui->comboM8->setCurrentIndex(0);
    ui->comboM9->setCurrentIndex(0);
    ui->comboM10->setCurrentIndex(0);
    ui->comboM11->setCurrentIndex(0);
    ui->comboM12->setCurrentIndex(0);
    ui->comboM13->setCurrentIndex(0);
    ui->comboM14->setCurrentIndex(0);
    ui->comboM15->setCurrentIndex(0);
    ui->comboM16->setCurrentIndex(0);

}

void FormToolbox::on_btnSetNaturalColors_clicked()
{
    ui->rbColorVIIRS->setChecked(true);
    ui->comboM1->setCurrentIndex(0);
    ui->comboM2->setCurrentIndex(0);
    ui->comboM3->setCurrentIndex(0);
    ui->comboM4->setCurrentIndex(0);
    ui->comboM5->setCurrentIndex(3);
    ui->comboM6->setCurrentIndex(0);
    ui->comboM7->setCurrentIndex(2);
    ui->comboM8->setCurrentIndex(0);
    ui->comboM9->setCurrentIndex(0);
    ui->comboM10->setCurrentIndex(1);
    ui->comboM11->setCurrentIndex(0);
    ui->comboM12->setCurrentIndex(0);
    ui->comboM13->setCurrentIndex(0);
    ui->comboM14->setCurrentIndex(0);
    ui->comboM15->setCurrentIndex(0);
    ui->comboM16->setCurrentIndex(0);

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

bool FormToolbox::comboColGeoOK()
{
    int cnt = 0;

    cnt += ui->comboMet006->currentIndex();
    cnt += ui->comboMet008->currentIndex();
    cnt += ui->comboMet016->currentIndex();
    cnt += ui->comboMet039->currentIndex();
    cnt += ui->comboMet062->currentIndex();
    cnt += ui->comboMet073->currentIndex();
    cnt += ui->comboMet087->currentIndex();
    cnt += ui->comboMet097->currentIndex();
    cnt += ui->comboMet108->currentIndex();
    cnt += ui->comboMet120->currentIndex();
    cnt += ui->comboMet134->currentIndex();

    if(cnt == 6)
        return true;
    else
        return false;
}

void FormToolbox::on_btnMakeVIIRSImage_clicked()
{

    if(formimage->getSegmentType() == eSegmentType::SEG_VIIRSM)
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

        ui->pbProgress->reset();
        emit emitShowVIIRSImage();
    }
    else if(formimage->getSegmentType() == eSegmentType::SEG_VIIRSDNB)
    {

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
            formimage->slotUpdateProjection();
        }
        else if(ui->toolBox->currentIndex() == 1)
        {
            ui->spbGVPMapWidth->setValue(resolutionX.at(index-1));
            ui->spbGVPMapHeight->setValue(resolutionY.at(index-1));
            imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), ui->spbGVPMapWidth->value(), ui->spbGVPMapHeight->value());
            formimage->slotUpdateProjection();
        }
        else if(ui->toolBox->currentIndex() == 2)
        {
            ui->spbSGMapWidth->setValue(resolutionX.at(index-1));
            ui->spbSGMapHeight->setValue(resolutionY.at(index-1));
            imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), ui->spbSGMapWidth->value(), ui->spbSGMapHeight->value(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
            formimage->slotUpdateProjection();
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
    qDebug() << QString("---->on_spbDnbWindow_valueChanged(int arg1) arg1 = %1").arg(arg1);
    segs->seglviirsdnb->spbWindowValueChanged(arg1, ui->sbCentreBand->value());
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

void FormToolbox::on_comboPOI_currentIndexChanged(int index)
{

    if(ui->toolBox->currentIndex() == 0) // LCC
    {
        setLCCParameters(index);
        imageptrs->lcc->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, ui->spbParallel1->value(), ui->spbParallel2->value(),  ui->spbCentral->value(), ui->spbLatOrigin->value(),
                                   ui->spbLCCMapWidth->value(), ui->spbLCCMapHeight->value(), ui->spbLCCCorrX->value(), ui->spbLCCCorrY->value());
        formimage->slotUpdateProjection();
    }
    else if(ui->toolBox->currentIndex() == 1) // GVP
    {
        setGVPParameters(index);
        imageptrs->gvp->Initialize(ui->spbGVPlon->value(), ui->spbGVPlat->value(), ui->spbGVPheight->value(), ui->spbGVPscale->value(), ui->spbGVPMapWidth->value(), ui->spbGVPMapHeight->value());
        formimage->slotUpdateProjection();
    }
    else if(ui->toolBox->currentIndex() == 2) // SG
    {
        setSGParameters(index);
        imageptrs->sg->Initialize(ui->spbSGlon->value(), ui->spbSGlat->value(), ui->spbSGScale->value(), ui->spbSGMapWidth->value(), ui->spbSGMapHeight->value(), ui->spbSGPanHorizon->value(), ui->spbSGPanVert->value());
        formimage->slotUpdateProjection();
    }

}

void FormToolbox::on_chkLCCGridOnProj_clicked()
{
    formimage->slotUpdateProjection();
}

void FormToolbox::on_chkGVPGridOnProj_clicked()
{
    formimage->slotUpdateProjection();
}

void FormToolbox::on_chkSGGridOnProj_clicked()
{
    formimage->slotUpdateProjection();
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
