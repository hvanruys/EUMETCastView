#include "dialogpreferences.h"
#include "ui_dialogpreferences.h"
#include <QDebug>
#include <QFileDialog>

#include "segmentimage.h"
extern SegmentImage *imageptrs;
extern Options opts;

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


    ui->btnImageOverlayColor1->setText(opts.imageoverlaycolor1);
    ui->lblImageOverlayColor1->setPalette(QPalette(QColor(opts.imageoverlaycolor1)));
    ui->lblImageOverlayColor1->setAutoFillBackground(true);

    ui->btnImageOverlayColor2->setText(opts.imageoverlaycolor2);
    ui->lblImageOverlayColor2->setPalette(QPalette(QColor(opts.imageoverlaycolor2)));
    ui->lblImageOverlayColor2->setAutoFillBackground(true);

    ui->btnImageOverlayColor3->setText(opts.imageoverlaycolor3);
    ui->lblImageOverlayColor3->setPalette(QPalette(QColor(opts.imageoverlaycolor3)));
    ui->lblImageOverlayColor3->setAutoFillBackground(true);


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
    ui->chkLight->setChecked(opts.bPhongModel);
    ui->chkImageOnTextureMet->setChecked(opts.imageontextureOnMet);
    ui->chkImageOnTextureAVHRR->setChecked(opts.imageontextureOnAVHRR);
    ui->chkImageOnTextureVIIRS->setChecked(opts.imageontextureOnVIIRS);
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

    ui->rbSattrackOn->setChecked(opts.sattrackinimage);
    ui->rbSmoothProjection->setChecked(opts.smoothprojectionimage);
    ui->rbGridOnProjection->setChecked(opts.gridonprojection);

    setupStationsTable();
    setupTLESourceTable();


    connect(ui->listWidget,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(dialogaccept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(dialogreject()));

    ui->stackedWidget->setCurrentIndex(0);
    ui->listWidget->item(0)->setSelected(true);

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
    opts.imageoverlaycolor1 = ui->btnImageOverlayColor1->text();
    opts.imageoverlaycolor2 = ui->btnImageOverlayColor2->text();
    opts.imageoverlaycolor3 = ui->btnImageOverlayColor3->text();

    opts.globelonlatcolor = ui->btnGlobeLonLatColor->text();

    opts.textureOn = ui->chkTexture->isChecked();
    opts.stationnameOn = ui->chkStations->isChecked();
    opts.bPhongModel = ui->chkLight->isChecked();
    opts.imageontextureOnMet = ui->chkImageOnTextureMet->isChecked();
    opts.imageontextureOnAVHRR = ui->chkImageOnTextureAVHRR->isChecked();
    opts.imageontextureOnVIIRS = ui->chkImageOnTextureVIIRS->isChecked();
    opts.udpmessages = ui->chkUDPMessages->isChecked();

    opts.sattrackinimage = ui->rbSattrackOn->isChecked();
    opts.smoothprojectionimage = ui->rbSmoothProjection->isChecked();
    opts.gridonprojection = ui->rbGridOnProjection->isChecked();

    opts.gshhsglobe1On = ui->chkGshhs1->isChecked();
    opts.gshhsglobe2On = ui->chkGshhs2->isChecked();
    opts.gshhsglobe3On = ui->chkGshhs3->isChecked();
    opts.graytextureOn = ui->chkGray->isChecked();

    opts.localdirremote = ui->ledLocalDirRemote->text();
    opts.dirremote = ui->ledDirRemote->text();


    opts.obslon = ui->ledLon->text().toDouble();
    opts.obslat = ui->ledLat->text().toDouble();
    opts.obsalt = ui->ledAlt->text().toDouble();

    accept();
}

void DialogPreferences::dialogreject()
{
    qDebug() << "in reject";
    reject();
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


void DialogPreferences::on_btnImageOverlayColor1_clicked()
{
    QColor color(opts.imageoverlaycolor1);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnImageOverlayColor1->setText(color.name());
        ui->lblImageOverlayColor1->setPalette(QPalette(color));
        ui->lblImageOverlayColor1->setAutoFillBackground(true);
    }
}

void DialogPreferences::on_btnImageOverlayColor2_clicked()
{
    QColor color(opts.imageoverlaycolor2);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnImageOverlayColor2->setText(color.name());
        ui->lblImageOverlayColor2->setPalette(QPalette(color));
        ui->lblImageOverlayColor2->setAutoFillBackground(true);
    }
}

void DialogPreferences::on_btnImageOverlayColor3_clicked()
{
    QColor color(opts.imageoverlaycolor3);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnImageOverlayColor3->setText(color.name());
        ui->lblImageOverlayColor3->setPalette(QPalette(color));
        ui->lblImageOverlayColor3->setAutoFillBackground(true);
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
