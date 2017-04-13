#include "dialogpreferences.h"
#include "ui_dialogpreferences.h"
#include <QDebug>
#include <QFileDialog>
#include "segmentimage.h"
#include "poi.h"

extern SegmentImage *imageptrs;
extern Options opts;
extern Poi poi;

DialogPreferences::DialogPreferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPreferences)
{
    ui->setupUi(this);

    ui->edtBackImage2D->setText(opts.backgroundimage2D);
    ui->edtBackImage3D->setText(opts.backgroundimage3D);

    ui->edtGshhsGlobe1->setText(opts.gshhsglobe1);
    ui->edtGshhsGlobe2->setText(opts.gshhsglobe2);
    ui->edtGshhsGlobe3->setText(opts.gshhsglobe3);
    ui->edtGshhsOverlay1->setText(opts.gshhsoverlay1);
    ui->edtGshhsOverlay2->setText(opts.gshhsoverlay2);
    ui->edtGshhsOverlay3->setText(opts.gshhsoverlay3);

    ui->edtSkyboxUp->setText(opts.skyboxup);
    ui->edtSkyboxDown->setText(opts.skyboxdown);
    ui->edtSkyboxLeft->setText(opts.skyboxleft);
    ui->edtSkyboxRight->setText(opts.skyboxright);
    ui->edtSkyboxFront->setText(opts.skyboxfront);
    ui->edtSkyboxBack->setText(opts.skyboxback);

    ui->btnSatHorizonColor->setText(opts.sathorizoncolor);
    ui->lblSatHorizonColor->setPalette(QPalette(QColor(opts.sathorizoncolor)));
    ui->lblSatHorizonColor->setAutoFillBackground(true);

    ui->btnSatTrackColor->setText(opts.sattrackcolor);
    ui->lblSatTrackColor->setPalette(QPalette(QColor(opts.sattrackcolor)));
    ui->lblSatTrackColor->setAutoFillBackground(true);


    ui->btnSatSegmentColor->setText(opts.satsegmentcolor);
    ui->lblSatSegmentColor->setPalette(QPalette(QColor(opts.satsegmentcolor)));
    ui->lblSatSegmentColor->setAutoFillBackground(true);

    ui->btnSatSegmentColorSel->setText(opts.satsegmentcolorsel);
    ui->lblSatSegmentColorSel->setPalette(QPalette(QColor(opts.satsegmentcolorsel)));
    ui->lblSatSegmentColorSel->setAutoFillBackground(true);

    ui->btnGlobeOverlayColor1->setText(opts.globeoverlaycolor1);
    ui->chkGshhs1->setPalette(QPalette(QColor(opts.globeoverlaycolor1)));
    ui->chkGshhs1->setAutoFillBackground(true);

    ui->btnGlobeOverlayColor2->setText(opts.globeoverlaycolor2);
    ui->chkGshhs2->setPalette(QPalette(QColor(opts.globeoverlaycolor2)));
    ui->chkGshhs2->setAutoFillBackground(true);

    ui->btnGlobeOverlayColor3->setText(opts.globeoverlaycolor3);
    ui->chkGshhs3->setPalette(QPalette(QColor(opts.globeoverlaycolor3)));
    ui->chkGshhs3->setAutoFillBackground(true);


    ui->btnGeoImageOverlayColor1->setText(opts.geoimageoverlaycolor1);
    ui->lblGeoImageOverlayColor1->setPalette(QPalette(QColor(opts.geoimageoverlaycolor1)));
    ui->lblGeoImageOverlayColor1->setAutoFillBackground(true);

    ui->btnGeoImageOverlayColor2->setText(opts.geoimageoverlaycolor2);
    ui->lblGeoImageOverlayColor2->setPalette(QPalette(QColor(opts.geoimageoverlaycolor2)));
    ui->lblGeoImageOverlayColor2->setAutoFillBackground(true);

    ui->btnGeoImageOverlayColor3->setText(opts.geoimageoverlaycolor3);
    ui->lblGeoImageOverlayColor3->setPalette(QPalette(QColor(opts.geoimageoverlaycolor3)));
    ui->lblGeoImageOverlayColor3->setAutoFillBackground(true);

    ui->btnOLCIImageOverlayColor->setText(opts.olciimageoverlaycolor);
    ui->lblOLCIImageOverlayColor->setPalette(QPalette(QColor(opts.olciimageoverlaycolor)));
    ui->lblOLCIImageOverlayColor->setAutoFillBackground(true);


    ui->btnGlobeLonLatColor->setText(opts.globelonlatcolor);
    ui->lblGlobeLonLatColor->setPalette(QPalette(QColor(opts.globelonlatcolor)));
    ui->lblGlobeLonLatColor->setAutoFillBackground(true);

    ui->btnMapLCCExtentColor->setText(opts.maplccextentcolor);
    ui->lblMapLCCExtentColor->setPalette(QPalette(QColor(opts.maplccextentcolor)));
    ui->lblMapLCCExtentColor->setAutoFillBackground(true);

    ui->btnMapGVPExtentColor->setText(opts.mapgvpextentcolor);
    ui->lblMapGVPExtentColor->setPalette(QPalette(QColor(opts.mapgvpextentcolor)));
    ui->lblMapGVPExtentColor->setAutoFillBackground(true);


    ui->btnProjectionOverlayColor1->setText(opts.projectionoverlaycolor1);
    ui->lblProjectionOverlayColor1->setPalette(QPalette(QColor(opts.projectionoverlaycolor1)));
    ui->lblProjectionOverlayColor1->setAutoFillBackground(true);

    ui->btnProjectionOverlayColor2->setText(opts.projectionoverlaycolor2);
    ui->lblProjectionOverlayColor2->setPalette(QPalette(QColor(opts.projectionoverlaycolor2)));
    ui->lblProjectionOverlayColor2->setAutoFillBackground(true);

    ui->btnProjectionOverlayColor3->setText(opts.projectionoverlaycolor3);
    ui->lblProjectionOverlayColor3->setPalette(QPalette(QColor(opts.projectionoverlaycolor3)));
    ui->lblProjectionOverlayColor3->setAutoFillBackground(true);

    ui->btnProjectionOverlayLonLatColor->setText(opts.projectionoverlaylonlatcolor);
    ui->lblProjectionOverlayLonLatColor->setPalette(QPalette(QColor(opts.projectionoverlaylonlatcolor)));
    ui->lblProjectionOverlayLonLatColor->setAutoFillBackground(true);

    ui->chkTexture->setChecked(opts.textureOn);
    ui->chkStations->setChecked(opts.stationnameOn);
    ui->chkLight->setChecked(opts.buttonPhong);
    ui->chkImageOnTextureMet->setChecked(opts.imageontextureOnMet);
    ui->chkImageOnTextureAVHRR->setChecked(opts.imageontextureOnAVHRR);
    ui->chkImageOnTextureVIIRS->setChecked(opts.imageontextureOnVIIRS);
    ui->chkImageOnTextureOLCI->setChecked(opts.imageontextureOnOLCI);
    ui->chkImageOnTextureSLSTR->setChecked(opts.imageontextureOnSLSTR);
    ui->chkWindowVectors->setChecked(opts.windowvectors);
    ui->chkUDPMessages->setChecked(opts.udpmessages);

    ui->chkGshhs1->setChecked(opts.gshhsglobe1On);
    ui->chkGshhs2->setChecked(opts.gshhsglobe2On);
    ui->chkGshhs3->setChecked(opts.gshhsglobe3On);
    ui->chkGray->setChecked(opts.graytextureOn);

    ui->ledLocalDirRemote->setText(opts.localdirremote);
    ui->ledDirRemote->setText(opts.dirremote);

    ui->ledLon->setText(QString("%1").arg(opts.obslon));
    ui->ledLat->setText(QString("%1").arg(opts.obslat));
    ui->ledAlt->setText(QString("%1").arg(opts.obsalt));

    ui->ledEquirectangularDirectory->setText(QString("%1").arg(opts.equirectangulardirectory));

    ui->rbSattrackOn->setChecked(opts.sattrackinimage);
    if(opts.smoothprojectiontype == 0)
        ui->rbNoSmoothing->setChecked(true);
    else if(opts.smoothprojectiontype == 1)
        ui->rbSmoothProjection->setChecked(true);
    else
        ui->rbLinearInterpolation->setChecked(true);

    ui->rbMagma->setChecked(opts.colormapMagma);
    ui->rbInferno->setChecked(opts.colormapInferno);
    ui->rbPlasma->setChecked(opts.colormapPlasma);
    ui->rbViridis->setChecked(opts.colormapViridis);

    ui->rdbRemoveOLCIDirs->setChecked(opts.remove_OLCI_dirs);
    ui->rdbRemoveSLSTRDirs->setChecked(opts.remove_SLSTR_dirs);
    ui->rdbSaturation->setChecked(opts.usesaturationmask);

    setupStationsTable();
    setupTLESourceTable();
    setupPOILCCTable();
    setupPOIGVPTable();
    setupPOISGTable();
    setupVIIRSMConfigTable();
    setupOLCIefrConfigTable();
    setupSLSTRConfigTable();
    setupDatahubConfig();

    POItablechanged = false;

    connect(ui->listWidget,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(dialogaccept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(dialogreject()));

    ui->stackedWidget->setCurrentIndex(0);
    ui->listWidget->item(0)->setSelected(true);

    QString htmlstring = "When checked the created OLCI ERR and EFR directories will be removed";
    ui->rdbRemoveOLCIDirs->setWhatsThis(htmlstring);
    htmlstring = "When checked the created SLSTR directories will be removed";
    ui->rdbRemoveSLSTRDirs->setWhatsThis(htmlstring);

}

DialogPreferences::~DialogPreferences()
{
    delete ui;
    qDebug() << "destructor DialogPreferences";
}

void DialogPreferences::setupStationsTable()
{
    myStationModel = new StationModel(this);

    ui->stationtableView->setModel(myStationModel);


    QHeaderView *hheader = ui->stationtableView->horizontalHeader();
    hheader->setStretchLastSection(true);
    //hheader->setMinimumSectionSize(-1);
    //hheader->->setResizeMode(0, QHeaderView::ResizeToContents);

    connect(ui->btnStationAdd, SIGNAL(clicked()), this, SLOT(addStationRow()));
    connect(ui->btnStationDelete, SIGNAL(clicked()), this, SLOT(deleteStationRow()));

}

void DialogPreferences::setupTLESourceTable()
{
    myTLESourceModel = new TLESourceModel(this);

    ui->tlesourcestableView->setModel(myTLESourceModel);


    QHeaderView *hheader = ui->tlesourcestableView->horizontalHeader();
    hheader->setStretchLastSection(true);
    //hheader->setMinimumSectionSize(-1);
    //hheader->setResizeMode(0, QHeaderView::ResizeToContents);

    connect(ui->btnTLESourceAdd, SIGNAL(clicked()), this, SLOT(addTLESourceRow()));
    connect(ui->btnTLESourceDelete, SIGNAL(clicked()), this, SLOT(deleteTLESourceRow()));

}

void DialogPreferences::setupPOILCCTable()
{
    myPOILCCModel = new POILCCModel(this);

    ui->tbvPOILCC->setModel(myPOILCCModel);


    QHeaderView *hheader = ui->tbvPOILCC->horizontalHeader();
    hheader->setStretchLastSection(true);
    //hheader->setMinimumSectionSize(-1);
    //hheader->->setResizeMode(0, QHeaderView::ResizeToContents);

    connect(ui->btnAddPOILCC, SIGNAL(clicked()), this, SLOT(addPOILCCRow()));
    connect(ui->btnDeletePOILCC, SIGNAL(clicked()), this, SLOT(deletePOILCCRow()));

}

void DialogPreferences::setupPOIGVPTable()
{
    myPOIGVPModel = new POIGVPModel(this);

    ui->tbvPOIGVP->setModel(myPOIGVPModel);


    QHeaderView *hheader = ui->tbvPOIGVP->horizontalHeader();
    hheader->setStretchLastSection(true);
    //hheader->setMinimumSectionSize(-1);
    //hheader->->setResizeMode(0, QHeaderView::ResizeToContents);

    connect(ui->btnAddPOIGVP, SIGNAL(clicked()), this, SLOT(addPOIGVPRow()));
    connect(ui->btnDeletePOIGVP, SIGNAL(clicked()), this, SLOT(deletePOIGVPRow()));

}

void DialogPreferences::setupPOISGTable()
{
    myPOISGModel = new POISGModel(this);

    ui->tbvPOISG->setModel(myPOISGModel);


    QHeaderView *hheader = ui->tbvPOISG->horizontalHeader();
    hheader->setStretchLastSection(true);
    //hheader->setMinimumSectionSize(-1);
    //hheader->->setResizeMode(0, QHeaderView::ResizeToContents);

    connect(ui->btnAddPOISG, SIGNAL(clicked()), this, SLOT(addPOISGRow()));
    connect(ui->btnDeletePOISG, SIGNAL(clicked()), this, SLOT(deletePOISGRow()));

}

void DialogPreferences::setupVIIRSMConfigTable()
{
    myVIIRSMConfigModel = new VIIRSMConfigModel(this);

    ui->tbvVIIRSMConfig->setModel(myVIIRSMConfigModel);
    ui->tbvVIIRSMConfig->setColumnWidth(0, 100);

    for(int i = 1; i < 18; i++)
        ui->tbvVIIRSMConfig->setColumnWidth(i, 50);

    QHeaderView *hheader = ui->tbvVIIRSMConfig->horizontalHeader();
    hheader->setStretchLastSection(true);
    //hheader->setMinimumSectionSize(-1);
    //hheader->->setResizeMode(0, QHeaderView::ResizeToContents);

    connect(ui->btnAddMConfig, SIGNAL(clicked()), this, SLOT(addVIIRSMConfigRow()));
    connect(ui->btnDeleteMConfig, SIGNAL(clicked()), this, SLOT(deleteVIIRSMConfigRow()));

}

void DialogPreferences::setupOLCIefrConfigTable()
{
    myOLCIefrConfigModel = new OLCIefrConfigModel(this);

    ui->tbvOLCIefrConfig->setModel(myOLCIefrConfigModel);
    ui->tbvOLCIefrConfig->setColumnWidth(0, 100);

    for(int i = 1; i < 23; i++)
        ui->tbvOLCIefrConfig->setColumnWidth(i, 50);

    QHeaderView *hheader = ui->tbvOLCIefrConfig->horizontalHeader();
    hheader->setStretchLastSection(true);
    //hheader->setMinimumSectionSize(-1);
    //hheader->->setResizeMode(0, QHeaderView::ResizeToContents);

    connect(ui->btnAddOLCIefrConfig, SIGNAL(clicked()), this, SLOT(addOLCIefrConfigRow()));
    connect(ui->btnDeleteOLCIefrConfig, SIGNAL(clicked()), this, SLOT(deleteOLCIefrConfigRow()));

}

void DialogPreferences::setupSLSTRConfigTable()
{
    mySLSTRConfigModel = new SLSTRConfigModel(this);

    ui->tbvSLSTRConfig->setModel(mySLSTRConfigModel);
    ui->tbvSLSTRConfig->setColumnWidth(0, 100);

    for(int i = 1; i < 13; i++)
        ui->tbvSLSTRConfig->setColumnWidth(i, 50);

    QHeaderView *hheader = ui->tbvSLSTRConfig->horizontalHeader();
    hheader->setStretchLastSection(true);
    //hheader->setMinimumSectionSize(-1);
    //hheader->->setResizeMode(0, QHeaderView::ResizeToContents);

    connect(ui->btnAddSLSTRConfig, SIGNAL(clicked()), this, SLOT(addSLSTRConfigRow()));
    connect(ui->btnDeleteSLSTRConfig, SIGNAL(clicked()), this, SLOT(deleteSLSTRConfigRow()));

}

void DialogPreferences::setupDatahubConfig()
{
    ui->leEsaUser->setText(opts.esauser);
    ui->leEsaPassword->setText(opts.esapassword);
    ui->leEumetsatUser->setText(opts.eumetsatuser);
    ui->leEumetsatPassword->setText(opts.eumetsatpassword);
    ui->leProductDirectory->setText(opts.productdirectory);
    if(opts.provideresaoreumetsat)
        ui->rdbUseScihub->setChecked(true);
    else
        ui->rdbUseCoda->setChecked(true);
    ui->rdbDownloadXMLOLCIEFR->setChecked(opts.downloadxmlolciefr);
    ui->rdbDownloadXMLOLCIERR->setChecked(opts.downloadxmlolcierr);
    ui->rdbDownloadXMLSLSTR->setChecked(opts.downloadxmlslstr);
    ui->rdbXMLlogging->setChecked(opts.xmllogging);
}

void DialogPreferences::addStationRow()
{
    myStationModel->insertRows(myStationModel->rowCount(), 1, QModelIndex());
}

void DialogPreferences::deleteStationRow()
{
    //ui->stationtableView->selectRow();
    //myModel->removeRows()
    //QTableView *temp = static_cast<QTableView*>(currentWidget());


    //QItemSelectionModel *selectionModel = ui->stationtableView->selectionModel();

    //QModelIndexList indexes = selectionModel->selectedRows();
    //QModelIndex index;

    //qDebug() << QString("QModelIndexList = %1 row = %2").arg(indexes.count()).arg(index.row());


    int row = ui->stationtableView->currentIndex().row();
    myStationModel->removeRow(row, QModelIndex());

/*    foreach (index, indexes) {
        int row =  proxy->mapToSource(index).row();
        table->removeRows(row, 1, QModelIndex());
    }
*/

}

void DialogPreferences::addTLESourceRow()
{
    myTLESourceModel->insertRows(myTLESourceModel->rowCount(), 1, QModelIndex());
}

void DialogPreferences::deleteTLESourceRow()
{

    int row = ui->tlesourcestableView->currentIndex().row();
    myTLESourceModel->removeRow(row, QModelIndex());

}

void DialogPreferences::addPOILCCRow()
{
    POItablechanged = true;

    myPOILCCModel->insertRows(myPOILCCModel->rowCount(), 1, QModelIndex());
}

void DialogPreferences::deletePOILCCRow()
{
    POItablechanged = true;

    int row = ui->tbvPOILCC->currentIndex().row();
    myPOILCCModel->removeRow(row, QModelIndex());
}

void DialogPreferences::addPOIGVPRow()
{
    POItablechanged = true;

    myPOIGVPModel->insertRows(myPOIGVPModel->rowCount(), 1, QModelIndex());
}

void DialogPreferences::deletePOIGVPRow()
{
    POItablechanged = true;

    int row = ui->tbvPOIGVP->currentIndex().row();
    myPOIGVPModel->removeRow(row, QModelIndex());
}

void DialogPreferences::addPOISGRow()
{
    POItablechanged = true;

    myPOISGModel->insertRows(myPOISGModel->rowCount(), 1, QModelIndex());
}

void DialogPreferences::deletePOISGRow()
{
    POItablechanged = true;

    int row = ui->tbvPOISG->currentIndex().row();
    myPOISGModel->removeRow(row, QModelIndex());
}

void DialogPreferences::addVIIRSMConfigRow()
{
    //POItablechanged = true;

    myVIIRSMConfigModel->insertRows(myVIIRSMConfigModel->rowCount(), 1, QModelIndex());
}

void DialogPreferences::deleteVIIRSMConfigRow()
{
    //POItablechanged = true;

    int row = ui->tbvVIIRSMConfig->currentIndex().row();
    myVIIRSMConfigModel->removeRow(row, QModelIndex());
}

void DialogPreferences::addOLCIefrConfigRow()
{
    //POItablechanged = true;

    myOLCIefrConfigModel->insertRows(myOLCIefrConfigModel->rowCount(), 1, QModelIndex());
}

void DialogPreferences::deleteOLCIefrConfigRow()
{
    //POItablechanged = true;

    int row = ui->tbvOLCIefrConfig->currentIndex().row();
    myOLCIefrConfigModel->removeRow(row, QModelIndex());
}

void DialogPreferences::addSLSTRConfigRow()
{
    //POItablechanged = true;

    mySLSTRConfigModel->insertRows(mySLSTRConfigModel->rowCount(), 1, QModelIndex());
}

void DialogPreferences::deleteSLSTRConfigRow()
{
    //POItablechanged = true;

    int row = ui->tbvSLSTRConfig->currentIndex().row();
    mySLSTRConfigModel->removeRow(row, QModelIndex());
}

void DialogPreferences::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    ui->stackedWidget->setCurrentIndex(ui->listWidget->row(current));
}




void DialogPreferences::dialogaccept()
{
    qDebug() << "in accept";
    opts.backgroundimage2D = ui->edtBackImage2D->text();

    if( opts.backgroundimage3D != ui->edtBackImage3D->text())
    {
        opts.texture_changed = true;
        opts.backgroundimage3D = ui->edtBackImage3D->text();
        QImage qim(opts.backgroundimage3D);
        delete imageptrs->pmOriginal;
        delete imageptrs->pmOut;
        imageptrs->pmOriginal = new QPixmap(QPixmap::fromImage(qim));
        imageptrs->pmOut = new QPixmap(QPixmap::fromImage(qim));
    }

    opts.gshhsglobe1 = ui->edtGshhsGlobe1->text();
    opts.gshhsglobe2 = ui->edtGshhsGlobe2->text();
    opts.gshhsglobe3 = ui->edtGshhsGlobe3->text();
    opts.gshhsoverlay1 = ui->edtGshhsOverlay1->text();
    opts.gshhsoverlay2 = ui->edtGshhsOverlay2->text();
    opts.gshhsoverlay3 = ui->edtGshhsOverlay3->text();

    opts.skyboxup = ui->edtSkyboxUp->text();
    opts.skyboxdown = ui->edtSkyboxDown->text();
    opts.skyboxright = ui->edtSkyboxRight->text();
    opts.skyboxleft = ui->edtSkyboxLeft->text();
    opts.skyboxfront = ui->edtSkyboxFront->text();
    opts.skyboxback = ui->edtSkyboxBack->text();

    opts.sathorizoncolor = ui->btnSatHorizonColor->text();
    opts.sattrackcolor = ui->btnSatTrackColor->text();
    opts.satsegmentcolor = ui->btnSatSegmentColor->text();
    opts.satsegmentcolorsel = ui->btnSatSegmentColorSel->text();
    opts.globeoverlaycolor1 = ui->btnGlobeOverlayColor1->text();
    opts.globeoverlaycolor2 = ui->btnGlobeOverlayColor2->text();
    opts.globeoverlaycolor3 = ui->btnGlobeOverlayColor3->text();
    opts.geoimageoverlaycolor1 = ui->btnGeoImageOverlayColor1->text();
    opts.geoimageoverlaycolor2 = ui->btnGeoImageOverlayColor2->text();
    opts.geoimageoverlaycolor3 = ui->btnGeoImageOverlayColor3->text();
    opts.olciimageoverlaycolor = ui->btnOLCIImageOverlayColor->text();

    opts.globelonlatcolor = ui->btnGlobeLonLatColor->text();

    opts.textureOn = ui->chkTexture->isChecked();
    opts.stationnameOn = ui->chkStations->isChecked();
    opts.buttonPhong = ui->chkLight->isChecked();
    opts.imageontextureOnMet = ui->chkImageOnTextureMet->isChecked();
    opts.imageontextureOnAVHRR = ui->chkImageOnTextureAVHRR->isChecked();
    opts.imageontextureOnVIIRS = ui->chkImageOnTextureVIIRS->isChecked();
    opts.imageontextureOnOLCI = ui->chkImageOnTextureOLCI->isChecked();
    opts.imageontextureOnSLSTR = ui->chkImageOnTextureSLSTR->isChecked();
    opts.windowvectors = ui->chkWindowVectors->isChecked();
    opts.udpmessages = ui->chkUDPMessages->isChecked();

    opts.sattrackinimage = ui->rbSattrackOn->isChecked();
    if(ui->rbNoSmoothing->isChecked())
        opts.smoothprojectiontype = 0;
    else if(ui->rbSmoothProjection->isChecked())
        opts.smoothprojectiontype = 1;
    else
        opts.smoothprojectiontype = 2;

    opts.gshhsglobe1On = ui->chkGshhs1->isChecked();
    opts.gshhsglobe2On = ui->chkGshhs2->isChecked();
    opts.gshhsglobe3On = ui->chkGshhs3->isChecked();
    opts.graytextureOn = ui->chkGray->isChecked();

    opts.localdirremote = ui->ledLocalDirRemote->text();
    opts.dirremote = ui->ledDirRemote->text();
    opts.equirectangulardirectory = ui->ledEquirectangularDirectory->text();

    opts.obslon = ui->ledLon->text().toDouble();
    opts.obslat = ui->ledLat->text().toDouble();
    opts.obsalt = ui->ledAlt->text().toDouble();

    opts.colormapMagma = ui->rbMagma->isChecked();
    opts.colormapInferno = ui->rbInferno->isChecked();
    opts.colormapPlasma = ui->rbPlasma->isChecked();
    opts.colormapViridis = ui->rbViridis->isChecked();

    opts.remove_OLCI_dirs = ui->rdbRemoveOLCIDirs->isChecked();
    opts.remove_SLSTR_dirs = ui->rdbRemoveSLSTRDirs->isChecked();
    opts.usesaturationmask = ui->rdbSaturation->isChecked();

    opts.esauser = ui->leEsaUser->text();
    opts.esapassword = ui->leEsaPassword->text();
    opts.eumetsatuser = ui->leEumetsatUser->text();
    opts.eumetsatpassword = ui->leEumetsatPassword->text();
    opts.productdirectory = ui->leProductDirectory->text();
    if(ui->rdbUseScihub->isChecked())
        opts.provideresaoreumetsat = true;
    else
        opts.provideresaoreumetsat = false;

    opts.downloadxmlolciefr = ui->rdbDownloadXMLOLCIEFR->isChecked();
    opts.downloadxmlolcierr = ui->rdbDownloadXMLOLCIERR->isChecked();
    opts.downloadxmlslstr = ui->rdbDownloadXMLSLSTR->isChecked();
    opts.xmllogging = ui->rdbXMLlogging->isChecked();


    if(POItablechanged)
        done(2);
    else
        done(QDialog::Accepted);  // value = 1 == accept()
}

void DialogPreferences::dialogreject()
{
    qDebug() << "in reject";
    if(POItablechanged)
        done(2);
    else
        done(QDialog::Rejected); // value = 0 == reject()

}



void DialogPreferences::on_listWidget_itemChanged(QListWidgetItem *item)
{

}

void DialogPreferences::on_btnLocalDirRemote_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(this,
                               tr("Directory"), QDir::currentPath());

    if (!directory.isEmpty()) {
        qDebug() << QString("directory = %1").arg(directory);
        ui->ledLocalDirRemote->setText(directory);
        opts.localdirremote = directory;
    }

}


void DialogPreferences::on_btnGshhsGlobe1_clicked()
{
    QString fn = QFileDialog::getOpenFileName( this,
                    tr("Select the Gshhs database file"),
                    ".",
                    tr("Gshhs files (*.b)"));

    if ( !fn.isEmpty() )
    {
        ui->edtGshhsGlobe1->setText(fn);
    }


}

void DialogPreferences::on_btnGshhsGlobe2_clicked()
{
    QString fn = QFileDialog::getOpenFileName( this,
                    tr("Select the Gshhs database file"),
                    ".",
                    tr("Gshhs files (*.b)"));

    if ( !fn.isEmpty() )
    {
        ui->edtGshhsGlobe2->setText(fn);
    }

}

void DialogPreferences::on_btnGshhsGlobe3_clicked()
{
    QString fn = QFileDialog::getOpenFileName( this,
                    tr("Select the Gshhs database file"),
                    ".",
                    tr("Gshhs files (*.b)"));

    if ( !fn.isEmpty() )
    {
        ui->edtGshhsGlobe3->setText(fn);
    }

}

void DialogPreferences::on_btnGshhsOverlay1_clicked()
{
    QString fn = QFileDialog::getOpenFileName( this,
                    tr("Select the Gshhs database file"),
                    ".",
                    tr("Gshhs files (*.b)"));

    if ( !fn.isEmpty() )
    {
        ui->edtGshhsOverlay1->setText(fn);
    }

}

void DialogPreferences::on_btnGshhsOverlay2_clicked()
{
    QString fn = QFileDialog::getOpenFileName( this,
                    tr("Select the Gshhs database file"),
                    ".",
                    tr("Gshhs files (*.b)"));

    if ( !fn.isEmpty() )
    {
        ui->edtGshhsOverlay2->setText(fn);
    }

}

void DialogPreferences::on_btnGshhsOverlay3_clicked()
{
    QString fn = QFileDialog::getOpenFileName( this,
                    tr("Select the Gshhs database file"),
                    ".",
                    tr("Gshhs files (*.b)"));

    if ( !fn.isEmpty() )
    {
        ui->edtGshhsOverlay3->setText(fn);
    }

}



void DialogPreferences::on_btnSkyboxUp_clicked()
{
    QString fn = QFileDialog::getOpenFileName( this,
                    tr("Select the 'Up' skybox file"),
                    ".",
                    tr("Image Files (*.png *.jpg *.bmp)"));

    if ( !fn.isEmpty() )
    {
        ui->edtSkyboxUp->setText(fn);
    }

}

void DialogPreferences::on_btnSkyboxDown_clicked()
{
    QString fn = QFileDialog::getOpenFileName( this,
                    tr("Select the 'Down' skybox file"),
                    ".",
                    tr("Image Files (*.png *.jpg *.bmp)"));

    if ( !fn.isEmpty() )
    {
        ui->edtSkyboxDown->setText(fn);
    }

}

void DialogPreferences::on_btnSkyboxLeft_clicked()
{
    QString fn = QFileDialog::getOpenFileName( this,
                    tr("Select the 'Left' skybox file"),
                    ".",
                    tr("Image Files (*.png *.jpg *.bmp)"));

    if ( !fn.isEmpty() )
    {
        ui->edtSkyboxLeft->setText(fn);
    }

}

void DialogPreferences::on_btnSkyboxRight_clicked()
{
    QString fn = QFileDialog::getOpenFileName( this,
                    tr("Select the 'Right' skybox file"),
                    ".",
                    tr("Image Files (*.png *.jpg *.bmp)"));

    if ( !fn.isEmpty() )
    {
        ui->edtSkyboxRight->setText(fn);
    }

}

void DialogPreferences::on_btnSkyboxFront_clicked()
{
    QString fn = QFileDialog::getOpenFileName( this,
                    tr("Select the 'Front' skybox file"),
                    ".",
                    tr("Image Files (*.png *.jpg *.bmp)"));

    if ( !fn.isEmpty() )
    {
        ui->edtSkyboxFront->setText(fn);
    }

}

void DialogPreferences::on_btnSkyboxBack_clicked()
{
    QString fn = QFileDialog::getOpenFileName( this,
                    tr("Select the 'Back' skybox file"),
                    ".",
                    tr("Image Files (*.png *.jpg *.bmp)"));

    if ( !fn.isEmpty() )
    {
        ui->edtSkyboxBack->setText(fn);
    }

}



void DialogPreferences::on_btnGlobeOverlayColor1_clicked()
{
    QColor color(opts.globeoverlaycolor1);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnGlobeOverlayColor1->setText(color.name());
        ui->chkGshhs1->setPalette(QPalette(color));
        ui->chkGshhs1->setAutoFillBackground(true);
    }
}

void DialogPreferences::on_btnGlobeOverlayColor2_clicked()
{
    QColor color(opts.globeoverlaycolor2);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnGlobeOverlayColor2->setText(color.name());
        ui->chkGshhs2->setPalette(QPalette(color));
        ui->chkGshhs2->setAutoFillBackground(true);
    }
}

void DialogPreferences::on_btnGlobeOverlayColor3_clicked()
{
    QColor color(opts.globeoverlaycolor3);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnGlobeOverlayColor3->setText(color.name());
        ui->chkGshhs3->setPalette(QPalette(color));
        ui->chkGshhs3->setAutoFillBackground(true);
    }
}


void DialogPreferences::on_btnBackImage2D_clicked()
{
    QString fn2D = QFileDialog::getOpenFileName( this,
                    tr("Select the 2D image file"),
                    ".",
                    tr("Image Files (*.png *.jpg *.bmp)"));

    if ( !fn2D.isEmpty() )
    {
        ui->edtBackImage2D->setText(fn2D);
    }

}

void DialogPreferences::on_btnBackImage3D_clicked()
{
    QFileInfo info(ui->edtBackImage3D->text());

    QString fn3D = QFileDialog::getOpenFileName( 0,
                    tr("Select the 3D image file"),
                    info.absoluteDir().absolutePath(),
                    tr("Image Files (*.png *.jpg *.bmp)"));

    if ( !fn3D.isEmpty() )
    {
        ui->edtBackImage3D->setText(fn3D);
    }

}

void DialogPreferences::on_btnMapLCCExtentColor_clicked()
{
    QColor color(opts.maplccextentcolor);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnMapLCCExtentColor->setText(color.name());
        ui->lblMapLCCExtentColor->setPalette(QPalette(color));
        ui->lblMapLCCExtentColor->setAutoFillBackground(true);
        opts.maplccextentcolor = ui->btnMapLCCExtentColor->text();
    }
}

void DialogPreferences::on_btnMapGVPExtentColor_clicked()
{
    QColor color(opts.mapgvpextentcolor);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnMapGVPExtentColor->setText(color.name());
        ui->lblMapGVPExtentColor->setPalette(QPalette(color));
        ui->lblMapGVPExtentColor->setAutoFillBackground(true);
        opts.mapgvpextentcolor = ui->btnMapGVPExtentColor->text();
    }
}

void DialogPreferences::on_btnProjectionOverlayColor1_clicked()
{
    QColor color(opts.projectionoverlaycolor1);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnProjectionOverlayColor1->setText(color.name());
        ui->lblProjectionOverlayColor1->setPalette(QPalette(color));
        ui->lblProjectionOverlayColor1->setAutoFillBackground(true);
        opts.projectionoverlaycolor1 = ui->btnProjectionOverlayColor1->text();
    }

}

void DialogPreferences::on_btnProjectionOverlayColor2_clicked()
{
    QColor color(opts.projectionoverlaycolor2);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnProjectionOverlayColor2->setText(color.name());
        ui->lblProjectionOverlayColor2->setPalette(QPalette(color));
        ui->lblProjectionOverlayColor2->setAutoFillBackground(true);
        opts.projectionoverlaycolor2 = ui->btnProjectionOverlayColor2->text();
    }

}

void DialogPreferences::on_btnProjectionOverlayColor3_clicked()
{
    QColor color(opts.projectionoverlaycolor3);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnProjectionOverlayColor3->setText(color.name());
        ui->lblProjectionOverlayColor3->setPalette(QPalette(color));
        ui->lblProjectionOverlayColor3->setAutoFillBackground(true);
        opts.projectionoverlaycolor3 = ui->btnProjectionOverlayColor3->text();
    }

}

void DialogPreferences::on_btnProjectionOverlayLonLatColor_clicked()
{
    QColor color(opts.projectionoverlaylonlatcolor);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnProjectionOverlayLonLatColor->setText(color.name());
        ui->lblProjectionOverlayLonLatColor->setPalette(QPalette(color));
        ui->lblProjectionOverlayLonLatColor->setAutoFillBackground(true);
        opts.projectionoverlaylonlatcolor = ui->btnProjectionOverlayLonLatColor->text();
    }
}



void DialogPreferences::on_btnSatHorizonColor_clicked()
{

    QColor color(opts.sathorizoncolor);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnSatHorizonColor->setText(color.name());
        ui->lblSatHorizonColor->setPalette(QPalette(color));
        ui->lblSatHorizonColor->setAutoFillBackground(true);
        opts.sathorizoncolor = ui->btnSatHorizonColor->text();
    }
}

void DialogPreferences::on_btnSatTrackColor_clicked()
{
    QColor color(opts.sattrackcolor);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnSatTrackColor->setText(color.name());
        ui->lblSatTrackColor->setPalette(QPalette(color));
        ui->lblSatTrackColor->setAutoFillBackground(true);
        opts.sattrackcolor = ui->btnSatTrackColor->text();
    }
}

void DialogPreferences::on_btnSatSegmentColor_clicked()
{
    QColor color(opts.satsegmentcolor);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnSatSegmentColor->setText(color.name());
        ui->lblSatSegmentColor->setPalette(QPalette(color));
        ui->lblSatSegmentColor->setAutoFillBackground(true);
        opts.satsegmentcolor = ui->btnSatSegmentColor->text();
    }
}

void DialogPreferences::on_btnSatSegmentColorSel_clicked()
{
    QColor color(opts.satsegmentcolorsel);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnSatSegmentColorSel->setText(color.name());
        ui->lblSatSegmentColorSel->setPalette(QPalette(color));
        ui->lblSatSegmentColorSel->setAutoFillBackground(true);
        opts.satsegmentcolorsel = ui->btnSatSegmentColorSel->text();
    }
}

void DialogPreferences::on_btnGlobeLonLatColor_clicked()
{
    QColor color(opts.globelonlatcolor);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnGlobeLonLatColor->setText(color.name());
        ui->lblGlobeLonLatColor->setPalette(QPalette(color));
        ui->lblGlobeLonLatColor->setAutoFillBackground(true);
        opts.globelonlatcolor = ui->btnGlobeLonLatColor->text();
    }
}

void DialogPreferences::on_btnEquirectangularDirectory_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    opts.equirectangulardirectory,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    if ( !dir.isEmpty() )
    {
        ui->ledEquirectangularDirectory->setText(dir);
    }

}

void DialogPreferences::on_btnGeoImageOverlayColor1_clicked()
{
    QColor color(opts.geoimageoverlaycolor1);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnGeoImageOverlayColor1->setText(color.name());
        ui->lblGeoImageOverlayColor1->setPalette(QPalette(color));
        ui->lblGeoImageOverlayColor1->setAutoFillBackground(true);
        opts.geoimageoverlaycolor1 = ui->btnGeoImageOverlayColor1->text();

    }
}

void DialogPreferences::on_btnGeoImageOverlayColor2_clicked()
{
    QColor color(opts.geoimageoverlaycolor2);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnGeoImageOverlayColor2->setText(color.name());
        ui->lblGeoImageOverlayColor2->setPalette(QPalette(color));
        ui->lblGeoImageOverlayColor2->setAutoFillBackground(true);
        opts.geoimageoverlaycolor2 = ui->btnGeoImageOverlayColor2->text();
    }
}

void DialogPreferences::on_btnGeoImageOverlayColor3_clicked()
{
    QColor color(opts.geoimageoverlaycolor3);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnGeoImageOverlayColor3->setText(color.name());
        ui->lblGeoImageOverlayColor3->setPalette(QPalette(color));
        ui->lblGeoImageOverlayColor3->setAutoFillBackground(true);
        opts.geoimageoverlaycolor3 = ui->btnGeoImageOverlayColor3->text();
    }
}


void DialogPreferences::on_btnOLCIImageOverlayColor_clicked()
{
    QColor color(opts.olciimageoverlaycolor);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnOLCIImageOverlayColor->setText(color.name());
        ui->lblOLCIImageOverlayColor->setPalette(QPalette(color));
        ui->lblOLCIImageOverlayColor->setAutoFillBackground(true);
        opts.olciimageoverlaycolor = ui->btnOLCIImageOverlayColor->text();

    }

}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

StationModel::StationModel(QObject *parent)
    :QAbstractTableModel(parent)
{

}

int StationModel::rowCount(const QModelIndex & /*parent*/) const
{
    return opts.stationlistname.count();
}

int StationModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant StationModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if(index.column() == 0)
            return opts.stationlistname.at(index.row());
        else if (index.column() == 1)
            return opts.stationlistlon.at(index.row());
        else
            return opts.stationlistlat.at(index.row());

    }
    return QVariant();
}



bool StationModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole)
    {
        // m_gridData[index.row()][index.column()] = value.toString();
        if(index.column() == 0)
            opts.stationlistname.replace(index.row(), value.toString());
        else if (index.column() == 1)
            opts.stationlistlon.replace(index.row(), value.toString());
        else
            opts.stationlistlat.replace(index.row(), value.toString());

        emit editCompleted();
    }



    return true;
}

QVariant StationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0:
                return tr("Station");

            case 1:
                return tr("Longitude");

            case 2:
                return tr("Latitude");

            default:
                return QVariant();
        }
    }
    return QVariant();
}

bool StationModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);

    opts.stationlistname.append( " " );
    opts.stationlistlon.append( " " );
    opts.stationlistlat.append( " " );

    endInsertRows();
    return true;
}

bool StationModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    opts.stationlistname.removeAt(position);
    opts.stationlistlon.removeAt(position);
    opts.stationlistlat.removeAt(position);

    endRemoveRows();
    return true;
}

Qt::ItemFlags StationModel::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}

//////////////////////////////////////////////////////////////////////////////

TLESourceModel::TLESourceModel(QObject *parent)
    :QAbstractTableModel(parent)
{

}

int TLESourceModel::rowCount(const QModelIndex & /*parent*/) const
{
    return opts.tlesources.count();
}

int TLESourceModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

QVariant TLESourceModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if(index.column() == 0)
            return opts.tlesources.at(index.row());

    }
    return QVariant();
}



bool TLESourceModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole)
    {
        // m_gridData[index.row()][index.column()] = value.toString();
        if(index.column() == 0)
            opts.tlesources.replace(index.row(), value.toString());

        emit editCompleted();
    }

    return true;
}

QVariant TLESourceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0:
                return tr("TLE Source");


            default:
                return QVariant();
        }
    }
    return QVariant();
}

bool TLESourceModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);

    opts.tlesources.append( " " );

    endInsertRows();
    return true;
}

bool TLESourceModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    opts.tlesources.removeAt(position);

    endRemoveRows();
    return true;
}

Qt::ItemFlags TLESourceModel::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}

//-----------------------------------------------------------------

POILCCModel::POILCCModel(QObject *parent)
    :QAbstractTableModel(parent)
{

}

int POILCCModel::rowCount(const QModelIndex & /*parent*/) const
{
    return poi.strlLCCName.count();
}

int POILCCModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 14;
}

QVariant POILCCModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        switch(index.column())
        {
        case 0:
            return poi.strlLCCName.at(index.row());
            break;
        case 1:
            return poi.strlLCCMapWidth.at(index.row());
            break;
        case 2:
            return poi.strlLCCMapHeight.at(index.row());
            break;
        case 3:
            return poi.strlLCCParallel1.at(index.row());
            break;
        case 4:
            return poi.strlLCCParallel2.at(index.row());
            break;
        case 5:
            return poi.strlLCCCentral.at(index.row());
            break;
        case 6:
            return poi.strlLCCLatOrigin.at(index.row());
            break;
        case 7:
            return poi.strlLCCNorth.at(index.row());
            break;
        case 8:
            return poi.strlLCCSouth.at(index.row());
            break;
        case 9:
            return poi.strlLCCEast.at(index.row());
            break;
        case 10:
            return poi.strlLCCWest.at(index.row());
            break;
        case 11:
            return poi.strlLCCScaleX.at(index.row());
            break;
        case 12:
            return poi.strlLCCScaleY.at(index.row());
            break;
        case 13:
            return poi.strlLCCGridOnProj.at(index.row());
            break;
        }
    }

    return QVariant();
}



bool POILCCModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole)
    {
        // m_gridData[index.row()][index.column()] = value.toString();
        switch(index.column())
        {
        case 0:
            poi.strlLCCName.replace(index.row(), value.toString());
            break;
        case 1:
            poi.strlLCCMapWidth.replace(index.row(), value.toString());
            break;
        case 2:
            poi.strlLCCMapHeight.replace(index.row(), value.toString());
            break;
        case 3:
            poi.strlLCCParallel1.replace(index.row(), value.toString());
            break;
        case 4:
            poi.strlLCCParallel2.replace(index.row(), value.toString());
            break;
        case 5:
            poi.strlLCCCentral.replace(index.row(), value.toString());
            break;
        case 6:
            poi.strlLCCLatOrigin.replace(index.row(), value.toString());
            break;
        case 7:
            poi.strlLCCNorth.replace(index.row(), value.toString());
            break;
        case 8:
            poi.strlLCCSouth.replace(index.row(), value.toString());
            break;
        case 9:
            poi.strlLCCEast.replace(index.row(), value.toString());
            break;
        case 10:
            poi.strlLCCWest.replace(index.row(), value.toString());
            break;
        case 11:
            poi.strlLCCScaleX.replace(index.row(), value.toString());
            break;
        case 12:
            poi.strlLCCScaleY.replace(index.row(), value.toString());
            break;
        case 13:
            poi.strlLCCGridOnProj.replace(index.row(), value.toString());
            break;
        }

        emit editCompleted();
    }

    return true;
}

QVariant POILCCModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("Name");

        case 1:
            return tr("Width");

        case 2:
            return tr("Height");

        case 3:
            return tr("Parallel1");

        case 4:
            return tr("Parallel2");

        case 5:
            return tr("Lon Origin");

        case 6:
            return tr("Lat Origin");

        case 7:
            return tr("North");

        case 8:
            return tr("South");

        case 9:
            return tr("East");

        case 10:
            return tr("West");

        case 11:
            return tr("Scale X");

        case 12:
            return tr("Scale Y");

        case 13:
            return tr("Grid on proj");

        default:
            return QVariant();
        }
    }
    return QVariant();
}

bool POILCCModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);

    poi.strlLCCName.append( " " );
    poi.strlLCCMapWidth.append( " " );
    poi.strlLCCMapHeight.append( " " );
    poi.strlLCCParallel1.append( " " );
    poi.strlLCCParallel2.append( " " );
    poi.strlLCCCentral.append( " " );
    poi.strlLCCLatOrigin.append( " " );
    poi.strlLCCNorth.append( " " );
    poi.strlLCCSouth.append( " " );
    poi.strlLCCEast.append( " " );
    poi.strlLCCWest.append( " " );
    poi.strlLCCScaleX.append( " " );
    poi.strlLCCScaleY.append( " " );
    poi.strlLCCGridOnProj.append( " " );

    endInsertRows();
    return true;
}

bool POILCCModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    poi.strlLCCName.removeAt(position);
    poi.strlLCCMapWidth.removeAt(position);
    poi.strlLCCMapHeight.removeAt(position);
    poi.strlLCCParallel1.removeAt(position);
    poi.strlLCCParallel2.removeAt(position);
    poi.strlLCCCentral.removeAt(position);
    poi.strlLCCLatOrigin.removeAt(position);
    poi.strlLCCNorth.removeAt(position);
    poi.strlLCCSouth.removeAt(position);
    poi.strlLCCEast.removeAt(position);
    poi.strlLCCWest.removeAt(position);
    poi.strlLCCScaleX.removeAt(position);
    poi.strlLCCScaleY.removeAt(position);
    poi.strlLCCGridOnProj.removeAt(position);

    endRemoveRows();
    return true;
}

Qt::ItemFlags POILCCModel::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}

//-----------------------------------------------------------------

POIGVPModel::POIGVPModel(QObject *parent)
    :QAbstractTableModel(parent)
{

}

int POIGVPModel::rowCount(const QModelIndex & /*parent*/) const
{
    return poi.strlGVPName.count();
}

int POIGVPModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 8;
}

QVariant POIGVPModel::data(const QModelIndex &index, int role) const
{

    if (role == Qt::DisplayRole)
    {
        switch(index.column())
        {
        case 0:
            return poi.strlGVPName.at(index.row());
            break;
        case 1:
            return poi.strlGVPMapWidth.at(index.row());
            break;
        case 2:
            return poi.strlGVPMapHeight.at(index.row());
            break;
        case 3:
            return poi.strlGVPLon.at(index.row());
            break;
        case 4:
            return poi.strlGVPLat.at(index.row());
            break;
        case 5:
            return poi.strlGVPHeight.at(index.row());
            break;
        case 6:
            return poi.strlGVPScale.at(index.row());
            break;
        case 7:
            return poi.strlGVPGridOnProj.at(index.row());
            break;
        }
    }

    return QVariant();
}



bool POIGVPModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole)
    {
        // m_gridData[index.row()][index.column()] = value.toString();
        switch(index.column())
        {
        case 0:
            poi.strlGVPName.replace(index.row(), value.toString());
            break;
        case 1:
            poi.strlGVPMapWidth.replace(index.row(), value.toString());
            break;
        case 2:
            poi.strlGVPMapHeight.replace(index.row(), value.toString());
            break;
        case 3:
            poi.strlGVPLon.replace(index.row(), value.toString());
            break;
        case 4:
            poi.strlGVPLat.replace(index.row(), value.toString());
            break;
        case 5:
            poi.strlGVPHeight.replace(index.row(), value.toString());
            break;
        case 6:
            poi.strlGVPScale.replace(index.row(), value.toString());
            break;
        case 7:
            poi.strlGVPGridOnProj.replace(index.row(), value.toString());
            break;
        }

        emit editCompleted();
    }

    return true;
}

QVariant POIGVPModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("Name");

        case 1:
            return tr("Map Width");

        case 2:
            return tr("Map Height");

        case 3:
            return tr("Longitude");

        case 4:
            return tr("Latitude");

        case 5:
            return tr("Height");

        case 6:
            return tr("Scale");

        case 7:
            return tr("Grid on Proj");

        default:
            return QVariant();
        }
    }
    return QVariant();
}

bool POIGVPModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);

    poi.strlGVPName.append( " " );
    poi.strlGVPMapWidth.append( " " );
    poi.strlGVPMapHeight.append( " " );
    poi.strlGVPLon.append( " " );
    poi.strlGVPLat.append( " " );
    poi.strlGVPHeight.append( " " );
    poi.strlGVPScale.append( " " );
    poi.strlGVPGridOnProj.append( " " );

    endInsertRows();
    return true;

}

bool POIGVPModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    poi.strlGVPName.removeAt(position);
    poi.strlGVPMapWidth.removeAt(position);
    poi.strlGVPMapHeight.removeAt(position);
    poi.strlGVPLon.removeAt(position);
    poi.strlGVPLat.removeAt(position);
    poi.strlGVPHeight.removeAt(position);
    poi.strlGVPScale.removeAt(position);
    poi.strlGVPGridOnProj.removeAt(position);

    endRemoveRows();
    return true;

}

Qt::ItemFlags POIGVPModel::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}

//-----------------------------------------------------------------

POISGModel::POISGModel(QObject *parent)
    :QAbstractTableModel(parent)
{

}

int POISGModel::rowCount(const QModelIndex & /*parent*/) const
{
    return poi.strlSGName.count();
}

int POISGModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 10;
}

QVariant POISGModel::data(const QModelIndex &index, int role) const
{

    if (role == Qt::DisplayRole)
    {
        switch(index.column())
        {
        case 0:
            return poi.strlSGName.at(index.row());
            break;
        case 1:
            return poi.strlSGMapWidth.at(index.row());
            break;
        case 2:
            return poi.strlSGMapHeight.at(index.row());
            break;
        case 3:
            return poi.strlSGLon.at(index.row());
            break;
        case 4:
            return poi.strlSGLat.at(index.row());
            break;
        case 5:
            return poi.strlSGRadius.at(index.row());
            break;
        case 6:
            return poi.strlSGScale.at(index.row());
            break;
        case 7:
            return poi.strlSGPanH.at(index.row());
            break;
        case 8:
            return poi.strlSGPanV.at(index.row());
            break;
        case 9:
            return poi.strlSGGridOnProj.at(index.row());
            break;
        }
    }

    return QVariant();


}



bool POISGModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole)
    {
        // m_gridData[index.row()][index.column()] = value.toString();
        switch(index.column())
        {
        case 0:
            poi.strlSGName.replace(index.row(), value.toString());
            break;
        case 1:
            poi.strlSGMapWidth.replace(index.row(), value.toString());
            break;
        case 2:
            poi.strlSGMapHeight.replace(index.row(), value.toString());
            break;
        case 3:
            poi.strlSGLon.replace(index.row(), value.toString());
            break;
        case 4:
            poi.strlSGLat.replace(index.row(), value.toString());
            break;
        case 5:
            poi.strlSGRadius.replace(index.row(), value.toString());
            break;
        case 6:
            poi.strlSGScale.replace(index.row(), value.toString());
            break;
        case 7:
            poi.strlSGPanH.replace(index.row(), value.toString());
            break;
        case 8:
            poi.strlSGPanV.replace(index.row(), value.toString());
            break;
        case 9:
            poi.strlSGGridOnProj.replace(index.row(), value.toString());
            break;
        }

        emit editCompleted();
    }

    return true;
}

QVariant POISGModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("Name");

        case 1:
            return tr("Map Width");

        case 2:
            return tr("Map Height");

        case 3:
            return tr("Longitude");

        case 4:
            return tr("Latitude");

        case 5:
            return tr("Radius");

        case 6:
            return tr("Scale");

        case 7:
            return tr("Pan Hor");

        case 8:
            return tr("Pan Vert");

        case 9:
            return tr("Grid on Proj");

        default:
            return QVariant();
        }
    }
    return QVariant();

}

bool POISGModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);

    poi.strlSGName.append( " " );
    poi.strlSGMapWidth.append( " " );
    poi.strlSGMapHeight.append( " " );
    poi.strlSGLon.append( " " );
    poi.strlSGLat.append( " " );
    poi.strlSGRadius.append( " " );
    poi.strlSGScale.append( " " );
    poi.strlSGPanH.append( " " );
    poi.strlSGPanV.append( " " );
    poi.strlSGGridOnProj.append( " " );

    endInsertRows();
    return true;

}

bool POISGModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    poi.strlSGName.removeAt(position);
    poi.strlSGMapWidth.removeAt(position);
    poi.strlSGMapHeight.removeAt(position);
    poi.strlSGLon.removeAt(position);
    poi.strlSGLat.removeAt(position);
    poi.strlSGRadius.removeAt(position);
    poi.strlSGScale.removeAt(position);
    poi.strlSGPanH.removeAt(position);
    poi.strlSGPanV.removeAt(position);
    poi.strlSGGridOnProj.removeAt(position);

    endRemoveRows();
    return true;

}

Qt::ItemFlags POISGModel::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
VIIRSMConfigModel::VIIRSMConfigModel(QObject *parent)
    :QAbstractTableModel(parent)
{

}

int VIIRSMConfigModel::rowCount(const QModelIndex & /*parent*/) const
{
    return poi.strlConfigNameM.count();
}

int VIIRSMConfigModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 18;
}

QVariant VIIRSMConfigModel::data(const QModelIndex &index, int role) const
{

    if (role == Qt::DisplayRole)
    {
        switch(index.column())
        {
        case 0:
            return poi.strlConfigNameM.at(index.row());
            break;
        case 1:
            return poi.strlColorBandM.at(index.row());
            break;
        case 2:
            return poi.strlComboM1.at(index.row());
            break;
        case 3:
            return poi.strlComboM2.at(index.row());
            break;
        case 4:
            return poi.strlComboM3.at(index.row());
            break;
        case 5:
            return poi.strlComboM4.at(index.row());
            break;
        case 6:
            return poi.strlComboM5.at(index.row());
            break;
        case 7:
            return poi.strlComboM6.at(index.row());
            break;
        case 8:
            return poi.strlComboM7.at(index.row());
            break;
        case 9:
            return poi.strlComboM8.at(index.row());
            break;
        case 10:
            return poi.strlComboM9.at(index.row());
            break;
        case 11:
            return poi.strlComboM10.at(index.row());
            break;
        case 12:
            return poi.strlComboM11.at(index.row());
            break;
        case 13:
            return poi.strlComboM12.at(index.row());
            break;
        case 14:
            return poi.strlComboM13.at(index.row());
            break;
        case 15:
            return poi.strlComboM14.at(index.row());
            break;
        case 16:
            return poi.strlComboM15.at(index.row());
            break;
        case 17:
            return poi.strlComboM16.at(index.row());
            break;
        }
    }

    return QVariant();


}



bool VIIRSMConfigModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole)
    {
        // m_gridData[index.row()][index.column()] = value.toString();
        switch(index.column())
        {
        case 0:
            poi.strlConfigNameM.replace(index.row(), value.toString());
            break;
        case 1:
            poi.strlColorBandM.replace(index.row(), value.toString());
            break;
        case 2:
            poi.strlComboM1.replace(index.row(), value.toString());
            break;
        case 3:
            poi.strlComboM2.replace(index.row(), value.toString());
            break;
        case 4:
            poi.strlComboM3.replace(index.row(), value.toString());
            break;
        case 5:
            poi.strlComboM4.replace(index.row(), value.toString());
            break;
        case 6:
            poi.strlComboM5.replace(index.row(), value.toString());
            break;
        case 7:
            poi.strlComboM6.replace(index.row(), value.toString());
            break;
        case 8:
            poi.strlComboM7.replace(index.row(), value.toString());
            break;
        case 9:
            poi.strlComboM8.replace(index.row(), value.toString());
            break;
        case 10:
            poi.strlComboM9.replace(index.row(), value.toString());
            break;
        case 11:
            poi.strlComboM10.replace(index.row(), value.toString());
            break;
        case 12:
            poi.strlComboM11.replace(index.row(), value.toString());
            break;
        case 13:
            poi.strlComboM12.replace(index.row(), value.toString());
            break;
        case 14:
            poi.strlComboM13.replace(index.row(), value.toString());
            break;
        case 15:
            poi.strlComboM14.replace(index.row(), value.toString());
            break;
        case 16:
            poi.strlComboM15.replace(index.row(), value.toString());
            break;
        case 17:
            poi.strlComboM16.replace(index.row(), value.toString());
            break;
        }

        emit editCompleted();
    }

    return true;
}

QVariant VIIRSMConfigModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("Name");

        case 1:
            return tr("Band");

        case 2:
            return tr("M1");
        case 3:
            return tr("M2");
        case 4:
            return tr("M3");
        case 5:
            return tr("M4");
        case 6:
            return tr("M5");
        case 7:
            return tr("M6");
        case 8:
            return tr("M7");
        case 9:
            return tr("M8");
        case 10:
            return tr("M9");
        case 11:
            return tr("M10");
        case 12:
            return tr("M11");
        case 13:
            return tr("M12");
        case 14:
            return tr("M13");
        case 15:
            return tr("M14");
        case 16:
            return tr("M15");
        case 17:
            return tr("M16");


        default:
            return QVariant();
        }
    }
    return QVariant();

}

bool VIIRSMConfigModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);

    poi.strlConfigNameM.append( " " );
    poi.strlColorBandM.append( "0" );
    poi.strlComboM1.append( "0" );
    poi.strlComboM2.append( "0" );
    poi.strlComboM3.append( "0" );
    poi.strlComboM4.append( "0" );
    poi.strlComboM5.append( "0" );
    poi.strlComboM6.append( "0" );
    poi.strlComboM7.append( "0" );
    poi.strlComboM8.append( "0" );
    poi.strlComboM9.append( "0" );
    poi.strlComboM10.append( "0" );
    poi.strlComboM11.append( "0" );
    poi.strlComboM12.append( "0" );
    poi.strlComboM13.append( "0" );
    poi.strlComboM14.append( "0" );
    poi.strlComboM15.append( "0" );
    poi.strlComboM16.append( "0" );

    endInsertRows();
    return true;

}

bool VIIRSMConfigModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position+rows-1);

//    poi.strlConfigNameM.removeAt(position);
//    poi.strlSGMapWidth.removeAt(position);
//    poi.strlSGMapHeight.removeAt(position);
//    poi.strlSGLon.removeAt(position);
//    poi.strlSGLat.removeAt(position);
//    poi.strlSGRadius.removeAt(position);
//    poi.strlSGScale.removeAt(position);
//    poi.strlSGPanH.removeAt(position);
//    poi.strlSGPanV.removeAt(position);
//    poi.strlSGGridOnProj.removeAt(position);

    endRemoveRows();
    return true;

}

Qt::ItemFlags VIIRSMConfigModel::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
OLCIefrConfigModel::OLCIefrConfigModel(QObject *parent)
    :QAbstractTableModel(parent)
{

}

int OLCIefrConfigModel::rowCount(const QModelIndex & /*parent*/) const
{
    return poi.strlConfigNameOLCI.count();
}

int OLCIefrConfigModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 23;
}

QVariant OLCIefrConfigModel::data(const QModelIndex &index, int role) const
{

    if (role == Qt::DisplayRole)
    {
        switch(index.column())
        {
        case 0:
            return poi.strlConfigNameOLCI.at(index.row());
            break;
        case 1:
            return poi.strlColorBandOLCI.at(index.row());
            break;
        case 2:
            return poi.strlComboOLCI01.at(index.row());
            break;
        case 3:
            return poi.strlComboOLCI02.at(index.row());
            break;
        case 4:
            return poi.strlComboOLCI03.at(index.row());
            break;
        case 5:
            return poi.strlComboOLCI04.at(index.row());
            break;
        case 6:
            return poi.strlComboOLCI05.at(index.row());
            break;
        case 7:
            return poi.strlComboOLCI06.at(index.row());
            break;
        case 8:
            return poi.strlComboOLCI07.at(index.row());
            break;
        case 9:
            return poi.strlComboOLCI08.at(index.row());
            break;
        case 10:
            return poi.strlComboOLCI09.at(index.row());
            break;
        case 11:
            return poi.strlComboOLCI10.at(index.row());
            break;
        case 12:
            return poi.strlComboOLCI11.at(index.row());
            break;
        case 13:
            return poi.strlComboOLCI12.at(index.row());
            break;
        case 14:
            return poi.strlComboOLCI13.at(index.row());
            break;
        case 15:
            return poi.strlComboOLCI14.at(index.row());
            break;
        case 16:
            return poi.strlComboOLCI15.at(index.row());
            break;
        case 17:
            return poi.strlComboOLCI16.at(index.row());
            break;
        case 18:
            return poi.strlComboOLCI17.at(index.row());
            break;
        case 19:
            return poi.strlComboOLCI18.at(index.row());
            break;
        case 20:
            return poi.strlComboOLCI19.at(index.row());
            break;
        case 21:
            return poi.strlComboOLCI20.at(index.row());
            break;
        case 22:
            return poi.strlComboOLCI21.at(index.row());
            break;
        }
    }

    return QVariant();


}



bool OLCIefrConfigModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole)
    {
        // m_gridData[index.row()][index.column()] = value.toString();
        switch(index.column())
        {
        case 0:
            poi.strlConfigNameOLCI.replace(index.row(), value.toString());
            break;
        case 1:
            poi.strlColorBandOLCI.replace(index.row(), value.toString());
            break;
        case 2:
            poi.strlComboOLCI01.replace(index.row(), value.toString());
            break;
        case 3:
            poi.strlComboOLCI02.replace(index.row(), value.toString());
            break;
        case 4:
            poi.strlComboOLCI03.replace(index.row(), value.toString());
            break;
        case 5:
            poi.strlComboOLCI04.replace(index.row(), value.toString());
            break;
        case 6:
            poi.strlComboOLCI05.replace(index.row(), value.toString());
            break;
        case 7:
            poi.strlComboOLCI06.replace(index.row(), value.toString());
            break;
        case 8:
            poi.strlComboOLCI07.replace(index.row(), value.toString());
            break;
        case 9:
            poi.strlComboOLCI08.replace(index.row(), value.toString());
            break;
        case 10:
            poi.strlComboOLCI09.replace(index.row(), value.toString());
            break;
        case 11:
            poi.strlComboOLCI10.replace(index.row(), value.toString());
            break;
        case 12:
            poi.strlComboOLCI11.replace(index.row(), value.toString());
            break;
        case 13:
            poi.strlComboOLCI12.replace(index.row(), value.toString());
            break;
        case 14:
            poi.strlComboOLCI13.replace(index.row(), value.toString());
            break;
        case 15:
            poi.strlComboOLCI14.replace(index.row(), value.toString());
            break;
        case 16:
            poi.strlComboOLCI15.replace(index.row(), value.toString());
            break;
        case 17:
            poi.strlComboOLCI16.replace(index.row(), value.toString());
            break;
        case 18:
            poi.strlComboOLCI17.replace(index.row(), value.toString());
            break;
        case 19:
            poi.strlComboOLCI18.replace(index.row(), value.toString());
            break;
        case 20:
            poi.strlComboOLCI19.replace(index.row(), value.toString());
            break;
        case 21:
            poi.strlComboOLCI20.replace(index.row(), value.toString());
            break;
        case 22:
            poi.strlComboOLCI21.replace(index.row(), value.toString());
            break;
        }

        emit editCompleted();
    }

    return true;
}

QVariant OLCIefrConfigModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Band");
        case 2:
            return tr("Oa01");
        case 3:
            return tr("Oa02");
        case 4:
            return tr("Oa03");
        case 5:
            return tr("Oa04");
        case 6:
            return tr("Oa05");
        case 7:
            return tr("Oa06");
        case 8:
            return tr("Oa07");
        case 9:
            return tr("Oa08");
        case 10:
            return tr("Oa09");
        case 11:
            return tr("Oa10");
        case 12:
            return tr("Oa11");
        case 13:
            return tr("Oa12");
        case 14:
            return tr("Oa13");
        case 15:
            return tr("Oa14");
        case 16:
            return tr("Oa15");
        case 17:
            return tr("Oa16");
        case 18:
            return tr("Oa17");
        case 19:
            return tr("Oa18");
        case 20:
            return tr("Oa19");
        case 21:
            return tr("Oa20");
        case 22:
            return tr("Oa21");

        default:
            return QVariant();
        }
    }
    return QVariant();

}

bool OLCIefrConfigModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);

    poi.strlConfigNameOLCI.append( " " );
    poi.strlColorBandOLCI.append( "0" );
    poi.strlComboOLCI01.append( "0" );
    poi.strlComboOLCI02.append( "0" );
    poi.strlComboOLCI03.append( "0" );
    poi.strlComboOLCI04.append( "0" );
    poi.strlComboOLCI05.append( "0" );
    poi.strlComboOLCI06.append( "0" );
    poi.strlComboOLCI07.append( "0" );
    poi.strlComboOLCI08.append( "0" );
    poi.strlComboOLCI09.append( "0" );
    poi.strlComboOLCI10.append( "0" );
    poi.strlComboOLCI11.append( "0" );
    poi.strlComboOLCI12.append( "0" );
    poi.strlComboOLCI13.append( "0" );
    poi.strlComboOLCI14.append( "0" );
    poi.strlComboOLCI15.append( "0" );
    poi.strlComboOLCI16.append( "0" );
    poi.strlComboOLCI17.append( "0" );
    poi.strlComboOLCI18.append( "0" );
    poi.strlComboOLCI19.append( "0" );
    poi.strlComboOLCI20.append( "0" );
    poi.strlComboOLCI21.append( "0" );

    poi.strlInverseOLCI01.append( "0" );
    poi.strlInverseOLCI02.append( "0" );
    poi.strlInverseOLCI03.append( "0" );
    poi.strlInverseOLCI04.append( "0" );
    poi.strlInverseOLCI05.append( "0" );
    poi.strlInverseOLCI06.append( "0" );
    poi.strlInverseOLCI07.append( "0" );
    poi.strlInverseOLCI08.append( "0" );
    poi.strlInverseOLCI09.append( "0" );
    poi.strlInverseOLCI10.append( "0" );
    poi.strlInverseOLCI11.append( "0" );
    poi.strlInverseOLCI12.append( "0" );
    poi.strlInverseOLCI13.append( "0" );
    poi.strlInverseOLCI14.append( "0" );
    poi.strlInverseOLCI15.append( "0" );
    poi.strlInverseOLCI16.append( "0" );
    poi.strlInverseOLCI17.append( "0" );
    poi.strlInverseOLCI18.append( "0" );
    poi.strlInverseOLCI19.append( "0" );
    poi.strlInverseOLCI20.append( "0" );
    poi.strlInverseOLCI21.append( "0" );


    endInsertRows();
    return true;

}

bool OLCIefrConfigModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    endRemoveRows();
    return true;

}

Qt::ItemFlags OLCIefrConfigModel::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}



//-----------------------------------------------------------------
//-----------------------------------------------------------------
SLSTRConfigModel::SLSTRConfigModel(QObject *parent)
    :QAbstractTableModel(parent)
{

}

int SLSTRConfigModel::rowCount(const QModelIndex & /*parent*/) const
{
    return poi.strlConfigNameSLSTR.count();
}

int SLSTRConfigModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 13;
}

QVariant SLSTRConfigModel::data(const QModelIndex &index, int role) const
{

    if (role == Qt::DisplayRole)
    {
        switch(index.column())
        {
        case 0:
            return poi.strlConfigNameSLSTR.at(index.row());
            break;
        case 1:
            return poi.strlColorBandSLSTR.at(index.row());
            break;
        case 2:
            return poi.strlComboSLSTRS1.at(index.row());
            break;
        case 3:
            return poi.strlComboSLSTRS2.at(index.row());
            break;
        case 4:
            return poi.strlComboSLSTRS3.at(index.row());
            break;
        case 5:
            return poi.strlComboSLSTRS4.at(index.row());
            break;
        case 6:
            return poi.strlComboSLSTRS5.at(index.row());
            break;
        case 7:
            return poi.strlComboSLSTRS6.at(index.row());
            break;
        case 8:
            return poi.strlComboSLSTRS7.at(index.row());
            break;
        case 9:
            return poi.strlComboSLSTRS8.at(index.row());
            break;
        case 10:
            return poi.strlComboSLSTRS9.at(index.row());
            break;
        case 11:
            return poi.strlComboSLSTRF1.at(index.row());
            break;
        case 12:
            return poi.strlComboSLSTRF2.at(index.row());
            break;
        }
    }

    return QVariant();


}



bool SLSTRConfigModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole)
    {
        // m_gridData[index.row()][index.column()] = value.toString();
        switch(index.column())
        {
        case 0:
            poi.strlConfigNameSLSTR.replace(index.row(), value.toString());
            break;
        case 1:
            poi.strlColorBandSLSTR.replace(index.row(), value.toString());
            break;
        case 2:
            poi.strlComboSLSTRS1.replace(index.row(), value.toString());
            break;
        case 3:
            poi.strlComboSLSTRS2.replace(index.row(), value.toString());
            break;
        case 4:
            poi.strlComboSLSTRS3.replace(index.row(), value.toString());
            break;
        case 5:
            poi.strlComboSLSTRS4.replace(index.row(), value.toString());
            break;
        case 6:
            poi.strlComboSLSTRS5.replace(index.row(), value.toString());
            break;
        case 7:
            poi.strlComboSLSTRS6.replace(index.row(), value.toString());
            break;
        case 8:
            poi.strlComboSLSTRS7.replace(index.row(), value.toString());
            break;
        case 9:
            poi.strlComboSLSTRS8.replace(index.row(), value.toString());
            break;
        case 10:
            poi.strlComboSLSTRS9.replace(index.row(), value.toString());
            break;
        case 11:
            poi.strlComboSLSTRF1.replace(index.row(), value.toString());
            break;
        case 12:
            poi.strlComboSLSTRF2.replace(index.row(), value.toString());
            break;
        }

        emit editCompleted();
    }

    return true;
}

QVariant SLSTRConfigModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Band");
        case 2:
            return tr("S1");
        case 3:
            return tr("S2");
        case 4:
            return tr("S3");
        case 5:
            return tr("S4");
        case 6:
            return tr("S5");
        case 7:
            return tr("S6");
        case 8:
            return tr("S7");
        case 9:
            return tr("S8");
        case 10:
            return tr("S9");
        case 11:
            return tr("F1");
        case 12:
            return tr("F2");

        default:
            return QVariant();
        }
    }
    return QVariant();

}

bool SLSTRConfigModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);

    poi.strlConfigNameSLSTR.append( " " );
    poi.strlColorBandSLSTR.append( "0" );
    poi.strlComboSLSTRS1.append( "0" );
    poi.strlComboSLSTRS2.append( "0" );
    poi.strlComboSLSTRS3.append( "0" );
    poi.strlComboSLSTRS4.append( "0" );
    poi.strlComboSLSTRS5.append( "0" );
    poi.strlComboSLSTRS6.append( "0" );
    poi.strlComboSLSTRS7.append( "0" );
    poi.strlComboSLSTRS8.append( "0" );
    poi.strlComboSLSTRS9.append( "0" );
    poi.strlComboSLSTRF1.append( "0" );
    poi.strlComboSLSTRF2.append( "0" );

    poi.strlInverseSLSTRS1.append( "0" );
    poi.strlInverseSLSTRS2.append( "0" );
    poi.strlInverseSLSTRS3.append( "0" );
    poi.strlInverseSLSTRS4.append( "0" );
    poi.strlInverseSLSTRS5.append( "0" );
    poi.strlInverseSLSTRS6.append( "0" );
    poi.strlInverseSLSTRS7.append( "0" );
    poi.strlInverseSLSTRS8.append( "0" );
    poi.strlInverseSLSTRS9.append( "0" );
    poi.strlInverseSLSTRF1.append( "0" );
    poi.strlInverseSLSTRF2.append( "0" );

    endInsertRows();
    return true;

}

bool SLSTRConfigModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    endRemoveRows();
    return true;

}

Qt::ItemFlags SLSTRConfigModel::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}



void DialogPreferences::on_btnSearchProductDirectory_clicked()
{
    QFileInfo info(ui->leProductDirectory->text());

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    ".",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
//    QString proddir = QFileDialog::geto ::getOpenFileName( 0,
//                    tr("Select the 3D image file"),
//                    info.absoluteDir().absolutePath(),
//                    tr("Image Files (*.png *.jpg *.bmp)"));

    if ( !dir.isEmpty() )
    {
        ui->leProductDirectory->setText(dir);
    }

}
