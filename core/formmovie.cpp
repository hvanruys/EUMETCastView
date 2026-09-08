#include "formmovie.h"
#include "videogapfiller.h"
#include "generalverticalperspective.h"
#include "poi.h"
#include "ui_formmovie.h"
#include "options.h"
#include <QDomDocument>
#include <QtGlobal>

extern Options opts;
extern SegmentImage *imageptrs;
extern Poi poi;

FormMovie::FormMovie(QWidget *parent, AVHRRSatellite *seglist) :
    QWidget(parent),
    ui(new Ui::FormMovie)
{
    ui->setupUi(this);

    segs = seglist;

    ui->spbProcesscount->setValue(opts.processcount);


    setupSpectrumMeteosat();

    ui->leGshhsOverlay1->setText(opts.videogshhsoverlayfile1);
    ui->leGshhsOverlay2->setText(opts.videogshhsoverlayfile2);
    ui->leGshhsOverlay3->setText(opts.videogshhsoverlayfile3);
    ui->rbGshhs1->setChecked(opts.videogshhsglobe1On);
    ui->rbGshhs2->setChecked(opts.videogshhsglobe2On);
    ui->rbGshhs3->setChecked(opts.videogshhsglobe3On);
    ui->spbGamma->setValue(opts.videogamma);
    //ui->cmbRss->set

    QColor color(opts.videooverlaycolor1);

    //    QPalette palette = ui->lblOverlayColor1->palette();
    //     palette.setColor(ui->lblOverlayColor1->backgroundRole(), Qt::yellow);
    //     palette.setColor(ui->lblOverlayColor1->foregroundRole(), Qt::yellow);
    //     ui->lblOverlayColor1->setPalette(palette);

    ui->btnOverlayColor1->setText(opts.videooverlaycolor1);
    //    color.setNamedColor(opts.videooverlaycolor1);
    //    ui->lblOverlayColor1->setPalette(QPalette(color));
    //    ui->lblOverlayColor1->setAutoFillBackground(true);
    ui->lblOverlayColor1->setStyleSheet("QLabel { background-color : " + QString(opts.videooverlaycolor1) + "; color : black; }");

    ui->btnOverlayColor2->setText(opts.videooverlaycolor2);
    //    color.setNamedColor(opts.videooverlaycolor2);
    //    ui->lblOverlayColor2->setPalette(QPalette(color));
    //    ui->lblOverlayColor2->setAutoFillBackground(true);
    ui->lblOverlayColor2->setStyleSheet("QLabel { background-color : " + QString(opts.videooverlaycolor2) + "; color : black; }");

    ui->btnOverlayColor3->setText(opts.videooverlaycolor3);
    //    color.setNamedColor(opts.videooverlaycolor3);
    //    ui->lblOverlayColor3->setPalette(QPalette(color));
    //    ui->lblOverlayColor3->setAutoFillBackground(true);
    ui->lblOverlayColor3->setStyleSheet("QLabel { background-color : " + QString(opts.videooverlaycolor3) + "; color : black; }");

    ui->btnOverlayGridColor->setText(opts.videooverlaygridcolor);
    //    color.setNamedColor(opts.videooverlaygridcolor);
    //    ui->lblOverlayGridColor->setPalette(QPalette(color));
    //    ui->lblOverlayGridColor->setAutoFillBackground(true);
    ui->lblOverlayGridColor->setStyleSheet("QLabel { background-color : " + QString(opts.videooverlaygridcolor) + "; color : black; }");

    ui->spbFontSize->setValue(opts.videooverlaydatefontsize);


    ui->leVideoHeight->setText(poi.strlGVPMapHeight.at(0));
    ui->leVideoWidth->setText(poi.strlGVPMapWidth.at(0));
    ui->chkOverlayBorder->setChecked(opts.videooverlayborder);
    ui->chkOverlayDate->setChecked(opts.videooverlaydate);

    ui->leScale->setText(poi.strlGVPScale.at(0));
    ui->leHeight->setText(poi.strlGVPHeight.at(0));
    ui->leLatitude->setText(poi.strlGVPLat.at(0));
    ui->leLongitude->setText(poi.strlGVPLon.at(0));
    ui->chkDisplayGrid->setChecked(poi.strlGVPGridOnProj.at(0).toInt());
    ui->leFalseEasting->setText(poi.strlGVPFalseEasting.at(0));
    ui->leFalseNorthing->setText(poi.strlGVPFalseNorthing.at(0));

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::LocalHost, 7755);

    connect(udpSocket, &QUdpSocket::readyRead, this, &FormMovie::readPendingDatagrams);
//    connect(this->processmanager,SIGNAL(signalDeleteManager()), this, SLOT(deleteManager()));

    for(int i = 0; i < opts.ffmpeg_options.count() ; i++)
    {
        ui->lwffmpeg->addItem(opts.ffmpeg_options.at(i));
    }

    if(ui->lwffmpeg->count() > 0)
    {
        ui->lwffmpeg->setCurrentRow(0);
        ui->leffmpegoptions->setText(ui->lwffmpeg->currentItem()->text());
    }


    opts.globalChangeFonts(this, opts.fontsize);

    VideoMinMaxLat minmax;

    minmax.maxlat =  -69.2419 ; minmax.minlat =  -80.0814; minmaxlist.append(minmax);
    minmax.maxlat =  -59.8475 ; minmax.minlat =  -74.2498; minmaxlist.append(minmax);
    minmax.maxlat =  -53.5037 ; minmax.minlat =  -65.8096; minmaxlist.append(minmax);
    minmax.maxlat =  -48.3907 ; minmax.minlat =  -59.5561; minmaxlist.append(minmax);
    minmax.maxlat =  -43.9502 ; minmax.minlat =  -54.2379; minmaxlist.append(minmax);
    minmax.maxlat =  -39.9802 ; minmax.minlat =  -49.5518; minmaxlist.append(minmax);
    minmax.maxlat =  -36.3348 ; minmax.minlat =  -45.3278; minmaxlist.append(minmax);
    minmax.maxlat =  -32.9216 ; minmax.minlat =  -41.3692; minmaxlist.append(minmax);
    minmax.maxlat =  -29.7108 ; minmax.minlat =  -37.5932; minmaxlist.append(minmax);
    minmax.maxlat =  -26.6401 ; minmax.minlat =  -34.0258; minmaxlist.append(minmax);
    minmax.maxlat =  -23.7023 ; minmax.minlat =  -30.5877; minmaxlist.append(minmax);
    minmax.maxlat =  -20.8637 ; minmax.minlat =  -27.2897; minmaxlist.append(minmax);
    minmax.maxlat =  -18.0964 ; minmax.minlat =  -24.0395; minmaxlist.append(minmax);
    minmax.maxlat =  -15.4051 ; minmax.minlat =  -20.8972; minmaxlist.append(minmax);
    minmax.maxlat =  -12.7579 ; minmax.minlat =  -17.8043; minmaxlist.append(minmax);
    minmax.maxlat =  -10.1626 ; minmax.minlat =  -14.759; minmaxlist.append(minmax);
    minmax.maxlat =  -7.60002 ; minmax.minlat =  -11.7664; minmaxlist.append(minmax);
    minmax.maxlat =  -5.05215 ; minmax.minlat =  -8.80622; minmaxlist.append(minmax);
    minmax.maxlat =  -2.529 ; minmax.minlat =  -5.85302; minmaxlist.append(minmax);
    minmax.maxlat =  -0.00452185 ; minmax.minlat =  -2.92396; minmaxlist.append(minmax);
    minmax.maxlat =  2.91328 ; minmax.minlat =  0.00452185; minmaxlist.append(minmax);
    minmax.maxlat =  5.84166 ; minmax.minlat =  2.51995; minmaxlist.append(minmax);
    minmax.maxlat =  8.79377 ; minmax.minlat =  5.04305; minmaxlist.append(minmax);
    minmax.maxlat =  11.7528 ; minmax.minlat =  7.59085; minmaxlist.append(minmax);
    minmax.maxlat =  14.759 ; minmax.minlat =  10.1533; minmaxlist.append(minmax);
    minmax.maxlat =  18.0173 ; minmax.minlat =  12.7579; minmaxlist.append(minmax);
    minmax.maxlat =  21.3152 ; minmax.minlat =  15.5872; minmaxlist.append(minmax);
    minmax.maxlat =  24.7405 ; minmax.minlat =  18.4789; minmaxlist.append(minmax);
    minmax.maxlat =  27.9595 ; minmax.minlat =  21.469; minmaxlist.append(minmax);
    minmax.maxlat =  31.0826 ; minmax.minlat =  24.3168; minmaxlist.append(minmax);
    minmax.maxlat =  34.2672 ; minmax.minlat =  27.0613; minmaxlist.append(minmax);
    minmax.maxlat =  37.5885 ; minmax.minlat =  29.9137; minmaxlist.append(minmax);
    minmax.maxlat =  41.3187 ; minmax.minlat =  32.9097; minmaxlist.append(minmax);
    minmax.maxlat =  45.3114 ; minmax.minlat =  36.3221; minmaxlist.append(minmax);
    minmax.maxlat =  49.5518 ; minmax.minlat =  39.9666; minmaxlist.append(minmax);
    minmax.maxlat =  54.2458 ; minmax.minlat =  43.9502; minmaxlist.append(minmax);
    minmax.maxlat =  59.5467 ; minmax.minlat =  48.3737; minmaxlist.append(minmax);
    minmax.maxlat =  65.8148 ; minmax.minlat =  53.4836; minmaxlist.append(minmax);
    minmax.maxlat =  74.0858 ; minmax.minlat =  59.8213; minmaxlist.append(minmax);
    minmax.maxlat =  80.1184 ; minmax.minlat =  69.1945; minmaxlist.append(minmax);
}

// void FormMovie::listWidgets()
// {
//     qDebug() << "--- Listing Widgets in FormMapCyl ---";

//     QFont new_font = this->font();
//     new_font.setPointSize(16); //your option
//     new_font.setWeight(QFont::Medium); //your option


//     // 1. Find all Buttons (using the base class QAbstractButton)
//     // qDebug() << "\n[Buttons (QAbstractButton subclasses)]";
//     // // Use 'this' to search within the MainWindow instance
//     // QList<QAbstractButton *> allButtons = this->findChildren<QAbstractButton *>();
//     // if (allButtons.isEmpty()) {
//     //     qDebug() << "  No buttons found.";
//     // } else {
//     //     for (QAbstractButton *button : allButtons) {
//     //         // Print object name (if set) and class name
//     //         qDebug() << "  - Object Name:" << button->objectName()
//     //                  << ", Class:" << button->metaObject()->className()
//     //                  << ", Text:" << button->text(); // Text might be empty for some
//     //         //button->setFont(new_font);

//     //     }
//     // }

//     // 2. Find all Item Views (using the base class QAbstractItemView)
//     // qDebug() << "\n[Item Views (QAbstractItemView subclasses)]";
//     // QList<QAbstractItemView *> allItemViews = this->findChildren<QAbstractItemView *>();
//     //  if (allItemViews.isEmpty()) {
//     //     qDebug() << "  No item views found.";
//     // } else {
//     //     for (QAbstractItemView *view : allItemViews) {
//     //         qDebug() << "  - Object Name:" << view->objectName()
//     //                  << ", Class:" << view->metaObject()->className();
//     //     }
//     // }


//     // 3. Find ALL Widgets (using the base class QWidget)
//     //    This will find *everything*, including layouts if they derive from QWidget (QVBoxLayout doesn't)
//     //    and potentially internal widgets of complex controls.
//     qDebug() << "\n[All Widgets (QWidget subclasses)]";
//     QList<QWidget *> allWidgets = this->findChildren<QWidget *>();
//     if (allWidgets.isEmpty()) {
//         qDebug() << "  No widgets found.";
//     } else {
//         for (QWidget *widget : allWidgets) {
//             // You could add filters here if needed, e.g., ignore widgets with empty object names
//             // or only show specific types not covered above.
//             qDebug() << "  - Object Name:" << widget->objectName()
//                      << ", Class:" << widget->metaObject()->className();
//             widget->setFont(new_font);
//         }
//     }

//     qDebug() << "\n--- End of Listing ---";
// }

void FormMovie::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        QString replyData = QString(datagram.data());
        writeTolistwidget(replyData);
    }
}

void FormMovie::saveOverlayColorsToOptions()
{

    opts.videooverlaycolor1 = ui->btnOverlayColor1->text();
    opts.videooverlaycolor2 = ui->btnOverlayColor2->text();
    opts.videooverlaycolor3 = ui->btnOverlayColor3->text();
    opts.videooverlaygridcolor = ui->btnOverlayGridColor->text();

}

void FormMovie::saveSpectrumToOptions()
{
    if(ui->rdbMeteosat_12->isChecked())
    {
        opts.dayredMTG = ui->cmbDayRed->currentText();
        opts.daygreenMTG = ui->cmbDayGreen->currentText();
        opts.dayblueMTG = ui->cmbDayBlue->currentText();
        opts.nightredMTG = ui->cmbNightRed->currentText();
        opts.dayredinverseMTG = ui->rbDayRedInverse->isChecked();
        opts.daygreeninverseMTG = ui->rbDayGreenInverse->isChecked();
        opts.dayblueinverseMTG = ui->rbDayBlueInverse->isChecked();
        opts.nightredinverseMTG = ui->rbNightRedInverse->isChecked();
    }
    else
    {
        opts.dayred = ui->cmbDayRed->currentText();
        opts.daygreen = ui->cmbDayGreen->currentText();
        opts.dayblue = ui->cmbDayBlue->currentText();
        opts.nightred = ui->cmbNightRed->currentText();
        opts.dayredinverse = ui->rbDayRedInverse->isChecked();
        opts.daygreeninverse = ui->rbDayGreenInverse->isChecked();
        opts.dayblueinverse = ui->rbDayBlueInverse->isChecked();
        opts.nightredinverse = ui->rbNightRedInverse->isChecked();
    }

}

void FormMovie::setupSpectrumMeteosat()
{
    QStringList spectrum;
    spectrum << "------" << "VIS006" << "VIS008" << "IR_016" << "IR_039" << "WV_062" << "WV_073" << "IR_087" << "IR_097" << "IR_108" << "IR_120" << "IR134";

    ui->cmbDayRed->clear();
    ui->cmbDayGreen->clear();
    ui->cmbDayBlue->clear();
    ui->cmbNightRed->clear();

    ui->cmbDayRed->addItems(spectrum);
    ui->cmbDayGreen->addItems(spectrum);
    ui->cmbDayBlue->addItems(spectrum);
    ui->cmbNightRed->addItems(spectrum);

    ui->cmbDayRed->setCurrentText(opts.dayred);
    ui->cmbDayGreen->setCurrentText(opts.daygreen);
    ui->cmbDayBlue->setCurrentText(opts.dayblue);
    ui->cmbNightRed->setCurrentText(opts.nightred);

    ui->rbDayRedInverse->setChecked(opts.dayredinverse);
    ui->rbDayGreenInverse->setChecked(opts.daygreeninverse);
    ui->rbDayBlueInverse->setChecked(opts.dayblueinverse);

    ui->rbNightRedInverse->setChecked(opts.nightredinverse);

}

void FormMovie::setupSpectrumMTG()
{
    QStringList spectrum;
    spectrum << "------" << "vis_04" << "vis_05" << "vis_06" << "vis_08" << "vis_09" << "nir_13" << "nir_16" << "nir_22"
             << "ir_38" << "wv_63" << "wv_73" << "ir_87" << "ir_97" << "ir_105" << "ir_123" << "ir_133";

    ui->cmbDayRed->clear();
    ui->cmbDayGreen->clear();
    ui->cmbDayBlue->clear();
    ui->cmbNightRed->clear();

    ui->cmbDayRed->addItems(spectrum);
    ui->cmbDayGreen->addItems(spectrum);
    ui->cmbDayBlue->addItems(spectrum);
    ui->cmbNightRed->addItems(spectrum);

    ui->cmbDayRed->setCurrentText(opts.dayredMTG);
    ui->cmbDayGreen->setCurrentText(opts.daygreenMTG);
    ui->cmbDayBlue->setCurrentText(opts.dayblueMTG);
    ui->cmbNightRed->setCurrentText(opts.nightredMTG);


    ui->rbDayRedInverse->setChecked(opts.dayredinverseMTG);
    ui->rbDayGreenInverse->setChecked(opts.daygreeninverseMTG);
    ui->rbDayBlueInverse->setChecked(opts.dayblueinverseMTG);

    ui->rbNightRedInverse->setChecked(opts.nightredinverseMTG);

}

bool FormMovie::saveFormToOptions()
{

    opts.videooverlaydatefontsize = ui->spbFontSize->value();
    saveOverlayColorsToOptions();
    saveSpectrumToOptions();
    opts.videoresolutionheight = ui->leVideoHeight->text().toInt();
    opts.videoresolutionwidth = ui->leVideoWidth->text().toInt();
    opts.videogamma = ui->spbGamma->value();
    opts.videooverlayborder = ui->chkOverlayBorder->isChecked();
    opts.videooverlaydate = ui->chkOverlayDate->isChecked();
    opts.videooverlaydatefontsize = ui->spbFontSize->value();
    opts.processcount = ui->spbProcesscount->value();


    return(true);

}

FormMovie::~FormMovie()
{
    saveFormToOptions();
    delete ui;
}

void FormMovie::getProjectionData()
{
    double lon_rad;
    double lat_rad;
    double x, y;

    int tabwidgetindex = formtoolbox->getTabWidgetIndex();
    eProjectionType projtype = formtoolbox->getCurrentProjectionType();
    int toolboxindex = formtoolbox->getToolboxIndex();
    QString lbl = QString("tabwidgetindex = %1 toolboxindex = %2 height = %3 width = %4").
                  arg(tabwidgetindex).arg(toolboxindex).arg(formtoolbox->getGVPMapHeight()).arg(formtoolbox->getGVPMapWidth());

}

void FormMovie::writeTolistwidget(QString txt)
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    ui->lwTraffic->appendPlainText(QString(now.toString(Qt::ISODate)) + " " + txt);
    ui->lwTraffic->verticalScrollBar()->setValue(ui->lwTraffic->verticalScrollBar()->maximum());
}

void FormMovie::on_btnOverlayColor1_clicked()
{
    QColor color(opts.videooverlaycolor1);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnOverlayColor1->setText(color.name());
        ui->lblOverlayColor1->setStyleSheet("QLabel { background-color : " + QString(color.name()) + "; color : black; }");

        //ui->lblOverlayColor1->setPalette(QPalette(color));
        //ui->lblOverlayColor1->setAutoFillBackground(true);
        opts.videooverlaycolor1 = ui->btnOverlayColor1->text();

    }
}

void FormMovie::on_btnOverlayColor2_clicked()
{
    QColor color(opts.videooverlaycolor2);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnOverlayColor2->setText(color.name());
        ui->lblOverlayColor2->setStyleSheet("QLabel { background-color : " + QString(color.name()) + "; color : black; }");

        //        ui->lblOverlayColor2->setPalette(QPalette(color));
        //        ui->lblOverlayColor2->setAutoFillBackground(true);
        opts.videooverlaycolor2 = ui->btnOverlayColor2->text();

    }
}


void FormMovie::on_btnOverlayColor3_clicked()
{
    QColor color(opts.videooverlaycolor3);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnOverlayColor3->setText(color.name());
        ui->lblOverlayColor3->setStyleSheet("QLabel { background-color : " + QString(color.name()) + "; color : black; }");

        //        ui->lblOverlayColor3->setPalette(QPalette(color));
        //        ui->lblOverlayColor3->setAutoFillBackground(true);
        opts.videooverlaycolor3 = ui->btnOverlayColor3->text();

    }
}


void FormMovie::on_btnOverlayGridColor_clicked()
{
    QColor color(opts.videooverlaygridcolor);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnOverlayGridColor->setText(color.name());
        ui->lblOverlayGridColor->setStyleSheet("QLabel { background-color : " + QString(color.name()) + "; color : black; }");

        //        ui->lblOverlayGridColor->setPalette(QPalette(color));
        //        ui->lblOverlayGridColor->setAutoFillBackground(true);
        opts.videooverlaygridcolor = ui->btnOverlayGridColor->text();

    }
}


void FormMovie::setGVPlat(double latitude)
{
    ui->leLatitude->setText(QString("%1").arg(latitude));
}

void FormMovie::setGVPlon(double longitude)
{
    ui->leLongitude->setText(QString("%1").arg(longitude));
}

void FormMovie::setGVPscale(double scale)
{
    ui->leScale->setText(QString("%1").arg(scale));
}

void FormMovie::setGVPheight(int height)
{
    ui->leHeight->setText(QString("%1").arg(height));
}

void FormMovie::setGVPMapHeight(int height)
{
    ui->leVideoHeight->setText(QString("%1").arg(height));
}

void FormMovie::setGVPMapWidth(int width)
{
    ui->leVideoWidth->setText(QString("%1").arg(width));
}

void FormMovie::setGVPFalseEasting(double easting)
{
    ui->leFalseEasting->setText(QString("%1").arg(easting));
}

void FormMovie::setGVPFalseNorthing(double northing)
{
    ui->leFalseNorthing->setText(QString("%1").arg(northing));
}

void FormMovie::setGVPDisplayGrid(bool grid)
{
    ui->chkDisplayGrid->setChecked(grid);
}


void FormMovie::on_btnClear_clicked()
{
    ui->lwTraffic->clear();
}

void FormMovie::on_btnffmpeg_clicked()
{
    // selectedSatellite() and not a spelling of its own : the frames ffmpeg is
    // about to be pointed at are named after the one this form wrote into
    // EUMETCastVideo.json.
    this->shortname = selectedSatellite();
    if(this->shortname.isEmpty())
    {
        writeTolistwidget("One of the input satellite radiobuttons must be set");
        return;
    }

    writeTolistwidget("Starting creating video with FFMPEG");
    QString datevideo = this->selectiondate.toString("yyyyMMdd");

    QString frameprefix = QString(ui->chkHRV->isChecked() ? "PROJHRV_" : "PROJ_") + this->shortname + "_";
    QString inputimagename = "tempvideo/" + frameprefix + "%04d.png";
    QString outputvideoname = frameprefix + datevideo + ".mp4";

    QCoreApplication::processEvents();

    // A frame that was not composed - a cycle that was not received, a chunk
    // that could not be read, a process that died - is a number ffmpeg does not
    // find, and it stops at the first one. The holes are filled with the frame
    // before them here, in front of the listing below, so that what is counted
    // is what ffmpeg is going to be handed.
    QStringList framereport;
    int framesfilled = fillMissingFrames("tempvideo", frameprefix, framereport);

    for(const QString &line : std::as_const(framereport))
        writeTolistwidget(line);

    if(framesfilled > 0)
        writeTolistwidget(QString("%1 frame(s) were missing and are copies of the frame before them.")
                              .arg(framesfilled));

    QDir dir("tempvideo");
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();

    QProcess process;
    process.setProgram("ffmpeg");

    writeTolistwidget(QString("=== Start creation video %1 ! ===").arg(outputvideoname));

    if(opts.ffmpeg_options.contains("-i INPUTFILES"))
    {
        if(list.size() == 0)
        {
            qDebug() << "The directory 'tempvideo' doesn't contains any files !";
            writeTolistwidget(QString("No images were written to '%1/tempvideo' : no video is created.")
                                  .arg(QDir::currentPath()));
            return;
        }
    }

    QStringList mylistin = opts.ffmpeg_options;
    QStringList mylistout;
    mylistin.replaceInStrings(QString("INPUTFILES"), inputimagename);
    mylistin.replaceInStrings("OUPUTFILE", outputvideoname);

    for(int i = 0; i < mylistin.count(); i++)
    {
        QStringList list = mylistin.at(i).split(QLatin1Char(' ')); //, Qt::SkipEmptyParts);
        mylistout.append(list);
    }

    for(int i = 0; i < mylistout.count(); i++)
    {
        qDebug() << mylistout.at(i);
    }


    process.setArguments(mylistout);

    process.setStandardOutputFile("ffmpegouput.txt");
    process.setStandardErrorFile("ffmpegoutputerror.txt"); //QProcess::nullDevice());

    process.start();
    process.waitForFinished(-1);
    writeTolistwidget(QString("=== The video %1 is created ! ===").arg(outputvideoname));
}


void FormMovie::on_lwffmpeg_itemSelectionChanged()
{
    // Get the pointer to the currently selected item.
    item = ui->lwffmpeg->currentItem();

    // Set the text color and its background color using the pointer to the item.
    //    item->setForeground(Qt::white);
    //    item->setBackground(Qt::blue);
    ui->leffmpegoptions->clear();
    ui->leffmpegoptions->insert(item->text());

}


void FormMovie::on_leffmpegoptions_textEdited(const QString &arg1)
{
    if(item != NULL)
    {
        item->setText(arg1);
        opts.ffmpeg_options.clear();
        for(int i = 0; i < ui->lwffmpeg->count(); i++)
        {
            if(!ui->lwffmpeg->item(i)->text().isEmpty())
                opts.ffmpeg_options.append(ui->lwffmpeg->item(i)->text());
        }

    }
}


void FormMovie::on_btnAdd_clicked()
{
    ui->lwffmpeg->addItem("");
    ui->lwffmpeg->setCurrentRow(ui->lwffmpeg->count()-1);

}


void FormMovie::on_btnDelete_clicked()
{
    if(ui->lwffmpeg->currentRow() > 0)
    {
        QListWidgetItem *item = ui->lwffmpeg->takeItem(ui->lwffmpeg->currentRow());
        delete item;
    }
}


void FormMovie::on_btnUp_clicked()
{
    if(ui->lwffmpeg->count() < 2)
        return;
    if(ui->lwffmpeg->currentRow() == 0)
        return;
    int row = ui->lwffmpeg->currentRow();
    QString text_before = ui->lwffmpeg->item(ui->lwffmpeg->currentRow() - 1)->text();
    QString text_current = ui->lwffmpeg->item(ui->lwffmpeg->currentRow())->text();
    ui->lwffmpeg->item(ui->lwffmpeg->currentRow() - 1)->setText(text_current);
    ui->lwffmpeg->item(ui->lwffmpeg->currentRow())->setText(text_before);
    ui->lwffmpeg->setCurrentRow(row - 1);
}


void FormMovie::on_btnDown_clicked()
{
    if(ui->lwffmpeg->count() < 2)
        return;
    if(ui->lwffmpeg->currentRow() == ui->lwffmpeg->count() - 1)
        return;
    int row = ui->lwffmpeg->currentRow();
    QString text_after = ui->lwffmpeg->item(ui->lwffmpeg->currentRow() + 1)->text();
    QString text_current = ui->lwffmpeg->item(ui->lwffmpeg->currentRow())->text();
    ui->lwffmpeg->item(ui->lwffmpeg->currentRow() + 1)->setText(text_current);
    ui->lwffmpeg->item(ui->lwffmpeg->currentRow())->setText(text_after);
    ui->lwffmpeg->setCurrentRow(row + 1);

}


void FormMovie::on_btnDefault_clicked()
{
    opts.ffmpeg_options.clear();
    opts.ffmpeg_options << "-framerate 5" << "-i INPUTFILES" << "-vf minterpolate=fps=60:mi_mode=blend";
    opts.ffmpeg_options << "-c:v libx264" << "-pix_fmt yuv420p" << "-y OUPUTFILE";

    qDebug() << opts.ffmpeg_options;
    qDebug() << "ffmpeg_options.count = " << opts.ffmpeg_options.count();

    ui->lwffmpeg->blockSignals(true);
    ui->lwffmpeg->clear();

    for(int i = 0; i < opts.ffmpeg_options.count() ; i++)
    {
        qDebug() << "adding " << i << " " << opts.ffmpeg_options.at(i);
        ui->lwffmpeg->addItem(opts.ffmpeg_options.at(i));
        ui->lwffmpeg->setCurrentRow(0);
    }
    ui->lwffmpeg->blockSignals(false);


}

void FormMovie::PopulateSelectionList(QDate seldate)
{

    qDebug() << "FormMovie::PopulateSelectionList() selection date = " << seldate.toString();

    // QString txt= QString("for 0 = %1").arg(segs->segmentlistmapgeomtgi1.count());
    // ui->rdbMeteosat_12->setText( txt);

    this->selectiondate = segs->selectiondate;

    QString txt = segs->GetOverviewSegmentsGeo(opts.GetGeoIndex("MET_12")).join(" ");
    ui->rdbMeteosat_12->setText( txt);
    txt = segs->GetOverviewSegmentsGeo(opts.GetGeoIndex("MET_11")).join(" ");
    ui->rdbMeteosat_11->setText( txt);
    txt = segs->GetOverviewSegmentsGeo(opts.GetGeoIndex("MET_10")).join(" ");
    ui->rdbMeteosat_10->setText( txt);
    txt = segs->GetOverviewSegmentsGeo(opts.GetGeoIndex("MET_9")).join(" ");
    ui->rdbMeteosat_9->setText( txt);

    // for(int i = 0; i < opts.geosatellites.count(); i++)
    // {
    //     if(opts.geosatellites.at(i).shortname != "MET_12")
    //         PopulateTreeGeo(i);
    //     else if(opts.geosatellites.at(i).shortname == "MET_12")
    //         PopulateTreeGeoMTGI1(i);
    // }

}


// The satellite the form is set to, in the one spelling the whole video path
// uses : it goes into EUMETCastVideo.json as the shortname, and from there into
// the names of the frames the video processes write - which is what ffmpeg is
// handed afterwards. An empty string means no radio button is set ; reporting
// that is left to the caller, they do it differently.
QString FormMovie::selectedSatellite()
{
    if(ui->rdbMeteosat_12->isChecked())
        return "MET_12";
    else if(ui->rdbMeteosat_11->isChecked())
        return "MET_11";
    else if(ui->rdbMeteosat_10->isChecked())
        return "MET_10";
    else if(ui->rdbMeteosat_9->isChecked())
        return "MET_9";

    return QString();
}

// The part of a render that a full run and a single test image have in common :
// pick the satellite off the radio buttons, write EUMETCastVideo.json for it,
// and hand back the timestamps the video processes are indexed by. Empty means
// there is nothing to render, and the reason has been reported.
QStringList FormMovie::prepareVideoRun()
{
    this->geoindex = 99;

    QString satellite = selectedSatellite();
    if(satellite.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("Select a satellite list.(MET-9/-10/-11/-12");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        int ret = msgBox.exec();

        switch (ret) {
        case QMessageBox::Ok:
            break;
        default:
            break;
        }

        return QStringList();
    }

    this->geoindex = opts.GetGeoIndex(satellite);
    this->shortname = satellite;

    QApplication::setOverrideCursor( Qt::WaitCursor ); // this might take time
    QJsonObject root = CreateVideoJson(satellite);
    QApplication::restoreOverrideCursor();

    // get datelist ("001","002", ...) from Json files
    QStringList datelist;

    if (root.contains("files") && root["files"].isObject()) {
        QJsonObject filesObj = root["files"].toObject();

        for (const QString &timestampKey : filesObj.keys()) {
            datelist << timestampKey;
        }
    }

    qDebug() << "datelist count = " << datelist.count();

    for(int i = 0; i < datelist.count(); i++)
    {
        qDebug() << datelist.at(i);
    }

    if(datelist.isEmpty())
        writeTolistwidget("Nothing to render : the selection holds no images.");

    return datelist;
}

void FormMovie::on_btnJson_clicked()
{
    if(this->processmanager != nullptr)
    {
        writeTolistwidget("A render is still busy : kill it first.");
        return;
    }

    QDir temp;
    temp.mkdir("tempvideo");
    temp.mkdir("tempimages");

    QDir tempimages("./tempimages");
    QDir tempvideo("./tempvideo");

    QStringList filters;
    filters << "*.png";

    tempvideo.setNameFilters(filters);
    tempvideo.setFilter(QDir::Files | QDir::NoSymLinks);
    tempimages.setNameFilters(filters);
    tempimages.setFilter(QDir::Files | QDir::NoSymLinks);


    // delete the images in 'tempvideo'
    foreach(QFileInfo item, tempvideo.entryInfoList() )
    {
        QFile file(item.absoluteFilePath());
        if(file.exists())
            file.remove();
    }

    // delete the images in 'tempimages'
    foreach(QFileInfo item, tempimages.entryInfoList() )
    {
        QFile file(item.absoluteFilePath());
        if(file.exists())
            file.remove();
    }

    QStringList datelist = prepareVideoRun();
    if(datelist.isEmpty())
        return;

    this->testrun = false;

    processmanager = new ProcessManager(datelist, ui->spbProcesscount->value(), this->shortname);
    connect(this->processmanager,SIGNAL(signalDeleteManager()), this, SLOT(deleteManager()));
    connect(this->processmanager, &ProcessManager::signalMessage, this, &FormMovie::writeTolistwidget);

    processmanager->start();
}

// One image out of the selection, to see what the settings on this form give
// before spending a full run on them. tempvideo is left alone : the test image
// takes the place of the frame with the same number, so a single bad frame can
// be rendered again without redoing the others.
void FormMovie::on_btnRunTest_clicked()
{
    if(this->processmanager != nullptr)
    {
        writeTolistwidget("A render is still busy : kill it first.");
        return;
    }

    QDir temp;
    temp.mkdir("tempvideo");
    temp.mkdir("tempimages");

    QStringList datelist = prepareVideoRun();
    if(datelist.isEmpty())
        return;

    int imagenbr = ui->spbTestImageNbr->value();

    if(imagenbr >= datelist.count())
    {
        writeTolistwidget(QString("There is no image %1 : the selection holds %2 image(s), numbered 0 to %3.")
                              .arg(imagenbr)
                              .arg(datelist.count())
                              .arg(datelist.count() - 1));
        return;
    }

    this->testrun = true;

    processmanager = new ProcessManager(datelist, 1, this->shortname, imagenbr);
    connect(this->processmanager,SIGNAL(signalDeleteManager()), this, SLOT(deleteManager()));
    connect(this->processmanager, &ProcessManager::signalMessage, this, &FormMovie::writeTolistwidget);

    writeTolistwidget(QString("Rendering test image %1 : %2").arg(imagenbr).arg(datelist.at(imagenbr)));

    processmanager->start();
}

void FormMovie::on_btnKillVideo_clicked()
{
    if(this->processmanager == nullptr)
    {
        writeTolistwidget("Nothing to kill : no EUMETCastVideo process is running.");
        return;
    }

    // stopAll() can finish the manager off from here - through
    // signalDeleteManager - so processmanager may not be touched afterwards.
    this->processmanager->stopAll();
}

void FormMovie::deleteManager()
{
    bool killed = this->processmanager->wasAborted();

    writeTolistwidget(killed ? "== All processes are killed ! ==" : "== All processes are finished ! ==");
    qDebug() << "deleting processmanager";
    // deleteLater(), not delete: this slot runs on a signal the manager emits
    // about itself, and the path taken when a process never starts still has
    // ProcessManager frames on the stack underneath it.
    this->processmanager->deleteLater();
    this->processmanager = nullptr;

    // A killed run leaves part of the frames behind and a test run leaves one :
    // neither is a set of images to hand to ffmpeg.
    if(!killed && !this->testrun)
        this->on_btnffmpeg_clicked();
}

QJsonObject FormMovie::CreateVideoJson(QString shortname)
{
    QJsonObject rootObject;
    rootObject["shortname"] = shortname;

    int geoindex = opts.GetGeoIndex(shortname);
    rootObject["geoindex"] = QString::number(geoindex);
    rootObject["rss"] = opts.geosatellites.at(geoindex).rss;
    rootObject["gamma"] = ui->spbGamma->value();
    rootObject["maxprocesscount"] = ui->spbProcesscount->value();
    QDate seldate = this->selectiondate;
    rootObject["selectiondate"] = seldate.toString("yyyyMMdd");

    QJsonObject object;
    object["gshhsoverlayfile1"] = ui->leGshhsOverlay1->text();
    object["gshhsoverlayfile2"] = ui->leGshhsOverlay2->text();
    object["gshhsoverlayfile3"] = ui->leGshhsOverlay3->text();
    object["gshhsoverlayOn1"] = ui->rbGshhs1->isChecked();
    object["gshhsoverlayOn2"] = ui->rbGshhs2->isChecked();
    object["gshhsoverlayOn3"] = ui->rbGshhs3->isChecked();
    object["projectionoverlaycolor1"] = ui->btnOverlayColor1->text();
    object["projectionoverlaycolor2"] = ui->btnOverlayColor2->text();
    object["projectionoverlaycolor3"] = ui->btnOverlayColor3->text();
    object["projectionoverlaygridcolor"] = ui->btnOverlayGridColor->text();

    rootObject["gshhs"] = object;

    object = QJsonObject();
    object["height"] = ui->leVideoHeight->text().toInt();
    object["width"] = ui->leVideoWidth->text().toInt();
    rootObject["resolution"] = object;

    object = QJsonObject();
    object["dayred"] = ui->cmbDayRed->currentText() == "------" ? "" : ui->cmbDayRed->currentText();
    object["daygreen"] = ui->cmbDayGreen->currentText() == "------" ? "" : ui->cmbDayGreen->currentText();
    object["dayblue"] = ui->cmbDayBlue->currentText() == "------" ? "" : ui->cmbDayBlue->currentText();
    object["dayredinverse"] = ui->rbDayRedInverse->isChecked();
    object["daygreeninverse"] = ui->rbDayGreenInverse->isChecked();
    object["daybluenverse"] = ui->rbDayBlueInverse->isChecked();
    object["bhrv"] = ui->chkHRV->isChecked();
    object["nightred"] = ui->cmbNightRed->currentText() == "------" ? "" : ui->cmbNightRed->currentText();
    object["nightredinverse"] = ui->rbNightRedInverse->isChecked();
    rootObject["spectrum"] = object;

    object = QJsonObject();
    object["coff"] = opts.geosatellites.at(geoindex).coff;
    object["loff"] = opts.geosatellites.at(geoindex).loff;
    object["cfac"] = opts.geosatellites.at(geoindex).cfac;
    object["lfac"] = opts.geosatellites.at(geoindex).lfac;
    object["coffhrv"] = opts.geosatellites.at(geoindex).coffhrv;
    object["loffhrv"] = opts.geosatellites.at(geoindex).loffhrv;
    object["cfachrv"] = opts.geosatellites.at(geoindex).cfachrv;
    object["lfachrv"] = opts.geosatellites.at(geoindex).lfachrv;

    if(shortname == "MET_12")
    {
        object["satlon"] = 0.0;
    }
    else if(shortname == "MET_11")
    {
        object["satlon"] = 9.5;
    }
    else if(shortname == "MET_10")
    {
        object["satlon"] = 0.0;
    }
    else if(shortname == "MET_9")
    {
        object["satlon"] = 45.5;
    }

    object["homelon"] = opts.obslon;
    object["homelat"] = opts.obslat;
    object["overlayborder"] = ui->chkOverlayBorder->isChecked();
    object["overlaydate"] = ui->chkOverlayDate->isChecked();
    object["overlaydatefontsize"] = ui->spbFontSize->value();
    rootObject["overlay"] = object;


    rootObject["projectiontype"] = "GVP";

    object = QJsonObject();

    object["latitude"] = ui->leLatitude->text().toDouble();
    object["longitude"] = ui->leLongitude->text().toDouble();
    object["scale"] = ui->leScale->text().toDouble();
    object["height"] = ui->leHeight->text().toDouble();
    object["gridonprojection"] = ui->chkDisplayGrid->isChecked();
    object["falseeasting"] = ui->leFalseEasting->text().toDouble();
    object["falsenorthing"] = ui->leFalseNorthing->text().toDouble();

    rootObject["gvpprojectionparameters"] = object;


    if(ui->chkHRV->isChecked())
    {
        rootObject["videooutputname"] = "PROJHRV_" + shortname + "_";
    }
    else
    {
        rootObject["videooutputname"] = "PROJ_" + shortname + "_";
    }



    //QMap<int, QMap<int, QFileInfo>> filterByKeys( const QMap<int, QMap<int, QFileInfo>>& input, const QSet<int>& allowedKeys)
    if(shortname == "MET_12")
    {
        QMap<int, QMap<int, QFileInfo>> filteredmap;
        QSet<int> allowedsegments = this->getFilteredSet();
        // QSetIterator<int> i(allowedsegments);
        // while (i.hasNext()) {
        //     int w = i.next();
        //     qDebug() << w;
        // }
        filteredmap = filterByKeys(segs->segmentlistmapgeomtgi1, allowedsegments);

        // A chunk that was not received does not leave a band out of the image :
        // compileImageMTG adds up the rows of the chunks that did come in, so
        // everything under the hole moves up and is navigated as the wrong
        // latitude. The same chunk of the cycle before covers the same band of
        // the disc, so it puts those rows back, ten minutes stale. A cycle that
        // was not received at all is kept in the numbering as an empty entry and
        // filled in as a frame after the render.
        QStringList chunkreport;
        filteredmap = fillMTGChunkGaps(filteredmap, allowedsegments, chunkreport);

        for(const QString &line : std::as_const(chunkreport))
            writeTolistwidget(line);

        rootObject["files"] = getJasonObjectFromMap(filteredmap);
    }
    else if(shortname == "MET_11")
        rootObject["files"] = getJasonObjectFromMap(segs->segmentlistmapgeo.at(geoindex));
    else if(shortname == "MET_10")
        rootObject["files"] = getJasonObjectFromMap(segs->segmentlistmapgeo.at(geoindex));
    else if(shortname == "MET_9")
        rootObject["files"] = getJasonObjectFromMap(segs->segmentlistmapgeo.at(geoindex));


    // ffmpeg parameters
    QString inputimagename = QString("tempvideo/%1").arg(ui->chkHRV->isChecked() ? "PROJHRV_" + shortname + "_%04d.png" : "PROJ_" + shortname + "_%04d.png");
    QString outputvideoname = QString("%1").arg(ui->chkHRV->isChecked() ? "PROJHRV_"  + shortname : "PROJ_" + shortname + ".mp4");

    QStringList mylistin = opts.ffmpeg_options;
    QStringList mylistout;
    mylistin.replaceInStrings(QString("INPUTFILES"), inputimagename);
    mylistin.replaceInStrings("OUPUTFILE", outputvideoname);

    for(int i = 0; i < mylistin.count(); i++)
    {
        QStringList list = mylistin.at(i).split(QLatin1Char(' ')); //, Qt::SkipEmptyParts);
        mylistout.append(list);
    }

    QString myopt;
    for(int i = 0; i < mylistout.count(); i++)
    {
        myopt.append(mylistout.at(i) + (i == mylistout.count() - 1 ? "" : ","));
    }
    rootObject["ffmpegparameters"] = myopt;

    // Create JSON document
    QJsonDocument jsonDoc(rootObject);

    // Write to file
    QFile jsonFile("EUMETCastVideo.json");
    if (!jsonFile.open(QIODevice::WriteOnly))
    {
        qWarning() << "Could not open file for writing output.json";
    }
    else
    {
        jsonFile.write(jsonDoc.toJson(QJsonDocument::Indented));
        jsonFile.close();
    }

    return rootObject;

}



QSet<int> FormMovie::getFilteredSet()
{
    QSet<int> set;
    double lon_rad, lat_rad;
    double lon_deg_1, lat_deg_1;
    double lon_deg_2, lat_deg_2;
    double lon_deg_3, lat_deg_3;
    double lon_deg_4, lat_deg_4;
    double lon_deg_5, lat_deg_5;
    double lon_deg_6, lat_deg_6;
    double lat_deg_max, lat_deg_min;


    GeneralVerticalPerspective gvp(segs);
    gvp.Initialize(ui->leLongitude->text().toDouble(), ui->leLatitude->text().toDouble(), ui->leHeight->text().toDouble(), ui->leScale->text().toDouble(),
                   ui->leFalseEasting->text().toDouble(), ui->leFalseNorthing->text().toDouble(), ui->leVideoWidth->text().toUInt(), ui->leVideoHeight->text().toUInt());

    bool ok = gvp.map_inverse( 0, 0, lon_rad, lat_rad);
    if(ok)
    {
        lat_deg_1 = 360 * lat_rad/TWOPI;
        qDebug() << "ok = " << ok << "lon = " << lon_deg_1 << " lat = " << lat_deg_1;
    }
    else
    {
        lat_deg_1 = 90.0;
    }

    ok = gvp.map_inverse( ui->leVideoWidth->text().toUInt()/2, 0, lon_rad, lat_rad);
    if(ok)
    {
        lat_deg_2 = 360 * lat_rad/TWOPI;
        qDebug() << "ok = " << ok << "lon = " << lon_deg_2 << " lat = " << lat_deg_2;
    }
    else
    {
        lat_deg_2 = 90.0;
    }

    ok = gvp.map_inverse( ui->leVideoWidth->text().toUInt(), 0, lon_rad, lat_rad);
    if(ok)
    {
        lat_deg_3 = 360 * lat_rad/TWOPI;
        qDebug() << "ok = " << ok << "lon = " << lon_deg_3 << " lat = " << lat_deg_3;
    }
    else
    {
        lat_deg_3 = 90.0;
    }

    lat_deg_max = qMax(lat_deg_1, lat_deg_2);
    lat_deg_max = qMax(lat_deg_max, lat_deg_3);


    ok = gvp.map_inverse( 0, ui->leVideoHeight->text().toUInt(), lon_rad, lat_rad);
    if(ok)
    {
        lat_deg_4 = 360 * lat_rad/TWOPI;
        qDebug() << "ok = " << ok << "lon = " << lon_deg_4 << " lat = " << lat_deg_4;
    }
    else
    {
        lat_deg_4 = -90.0;
    }

    ok = gvp.map_inverse( ui->leVideoWidth->text().toUInt()/2, ui->leVideoHeight->text().toUInt(), lon_rad, lat_rad);
    if(ok)
    {
        lat_deg_5 = 360 * lat_rad/TWOPI;
        qDebug() << "ok = " << ok << "lon = " << lon_deg_5 << " lat = " << lat_deg_5;
    }
    else
    {
        lat_deg_5 = -90.0;
    }

    ok = gvp.map_inverse( ui->leVideoWidth->text().toUInt(), ui->leVideoHeight->text().toUInt(), lon_rad, lat_rad);
    if(ok)
    {
        lat_deg_6 = 360 * lat_rad/TWOPI;
        qDebug() << "ok = " << ok << "lon = " << lon_deg_6 << " lat = " << lat_deg_6;
    }
    else
    {
        lat_deg_6 = -90.0;
    }

    lat_deg_min = qMin(lat_deg_4, lat_deg_5);
    lat_deg_min = qMin(lat_deg_min, lat_deg_6);


    VideoMinMaxLat mm;
    for(int i = 0; i < minmaxlist.size(); i++)
    {
        mm = minmaxlist.at(i);
        qDebug() << "mm.maxlat = " << mm.maxlat << " mm.minlat = " << mm.minlat;
        if(lat_deg_max > lat_deg_min)
        {
            if(lat_deg_max > mm.minlat && lat_deg_min < mm.maxlat)
            {
                qDebug() << "bingo";
                set << i + 1;
            }
        }
        else
        {
            if(lat_deg_min > mm.minlat && lat_deg_max < mm.maxlat)
            {
                qDebug() << "bingo";
                set << i + 1;
            }
        }
    }

    return set;
}

QJsonObject FormMovie::getJsonFileList()
{
    QJsonObject obj;
    // QMap<int, QMap< int, QFileInfo > > segmentlistmapgeomtgi1;
    for (auto seqnbr = segs->segmentlistmapgeomtgi1.cbegin(), end = segs->segmentlistmapgeomtgi1.cend(); seqnbr != end; ++seqnbr)
    {
        for (auto fileseqnbr = seqnbr.value().cbegin(), end = seqnbr.value().cend(); fileseqnbr != end; ++fileseqnbr)
        {
            QJsonValue val(fileseqnbr.key());
            obj.insert(QString::number(seqnbr.key()), val);

            qDebug() << seqnbr.key() << ": " << fileseqnbr.key();
        }
    }

    return obj;
}

bool FormMovie::convertToJson(const QMap<int, QMap<int, QFileInfo>>& segmentlistmap, const QString& outputFilePath)
{
    QJsonObject rootObject;

    // Iterate through outer map
    for (auto outerIt = segmentlistmap.constBegin();
         outerIt != segmentlistmap.constEnd(); ++outerIt)
    {
        int outerKey = outerIt.key();
        const QMap<int, QFileInfo>& innerMap = outerIt.value();

        QJsonObject innerObject;

        // Iterate through inner map
        for (auto innerIt = innerMap.constBegin();
             innerIt != innerMap.constEnd(); ++innerIt)
        {
            int innerKey = innerIt.key();
            const QFileInfo& fileInfo = innerIt.value();

            // Create JSON object for QFileInfo
            QJsonObject fileInfoObject;
            fileInfoObject["filePath"] = fileInfo.filePath();
            fileInfoObject["fileName"] = fileInfo.fileName();
            fileInfoObject["absolutePath"] = fileInfo.absolutePath();
            fileInfoObject["absoluteFilePath"] = fileInfo.absoluteFilePath();
            fileInfoObject["size"] = static_cast<qint64>(fileInfo.size());
            fileInfoObject["exists"] = fileInfo.exists();
            fileInfoObject["isFile"] = fileInfo.isFile();
            fileInfoObject["isDir"] = fileInfo.isDir();
            fileInfoObject["isReadable"] = fileInfo.isReadable();
            fileInfoObject["isWritable"] = fileInfo.isWritable();
            fileInfoObject["suffix"] = fileInfo.suffix();
            fileInfoObject["completeSuffix"] = fileInfo.completeSuffix();
            fileInfoObject["lastModified"] = fileInfo.lastModified().toString(Qt::ISODate);

            // Add to inner object with inner key as string
            innerObject[QString("%1").arg(innerKey, 2, 10, '0')] = fileInfoObject;
        }

        // Add to root object with outer key as string
        rootObject[QString("%1").arg(outerKey, 3, 10, '0')] = innerObject;
    }

    // Create JSON document
    QJsonDocument jsonDoc(rootObject);

    // Write to file
    QFile jsonFile(outputFilePath);
    if (!jsonFile.open(QIODevice::WriteOnly))
    {
        qWarning() << "Could not open file for writing:" << outputFilePath;
        return false;
    }

    jsonFile.write(jsonDoc.toJson(QJsonDocument::Indented));
    jsonFile.close();

    qDebug() << "JSON file created successfully:" << outputFilePath;
    return true;
}

// Filter by specific set of inner keys
QMap<int, QMap<int, QFileInfo>> FormMovie::filterByKeys( const QMap<int, QMap<int, QFileInfo>>& input,
    const QSet<int>& allowedKeys)
{
    QMap<int, QMap<int, QFileInfo>> output;

    for (auto outerIt = input.constBegin(); outerIt != input.constEnd(); ++outerIt) {
        QMap<int, QFileInfo> filteredInner;
        const QMap<int, QFileInfo>& innerMap = outerIt.value();

        for (auto innerIt = innerMap.constBegin(); innerIt != innerMap.constEnd(); ++innerIt) {
            if (allowedKeys.contains(innerIt.key())) {
                filteredInner.insert(innerIt.key(), innerIt.value());
            }
        }

        if (!filteredInner.isEmpty()) {
            output.insert(outerIt.key(), filteredInner);
        }
    }

    return output;
}

QJsonObject FormMovie::getJasonObjectFromMap(const QMap<int, QMap<int, QFileInfo>> &segmentlistmap)
{
    QJsonObject rootObject;

    // Iterate through outer map
    for (auto outerIt = segmentlistmap.constBegin();
         outerIt != segmentlistmap.constEnd(); ++outerIt)
    {
        int outerKey = outerIt.key();
        const QMap<int, QFileInfo>& innerMap = outerIt.value();

        QJsonObject innerObject;

        // Iterate through inner map
        for (auto innerIt = innerMap.constBegin();
             innerIt != innerMap.constEnd(); ++innerIt)
        {
            int innerKey = innerIt.key();
            const QFileInfo& fileInfo = innerIt.value();

            // Create JSON object for QFileInfo
            QJsonObject fileInfoObject;
            fileInfoObject["absoluteFilePath"] = fileInfo.absoluteFilePath();
            fileInfoObject["size"] = static_cast<qint64>(fileInfo.size());

            // Add to inner object with inner key as string
            innerObject[QString("%1").arg(innerKey, 2, 10, '0')] = fileInfoObject;
        }

        // Add to root object with outer key as string
        rootObject[QString("%1").arg(outerKey, 3, 10, '0')] = innerObject;
    }

    return rootObject;
}

QJsonObject FormMovie::getJasonObjectFromMap(const QMap<QString, QMap<QString, QMap<int, QFileInfo> > > &segmentlistmap)
{
    QJsonObject rootObject;

    // Iterate through outer map
    for (auto dateIt = segmentlistmap.constBegin(); dateIt != segmentlistmap.constEnd(); ++dateIt)
    {
        QString dateKey = dateIt.key();
        const QMap<QString, QMap<int, QFileInfo> >& spectrumMap = dateIt.value();

        QJsonObject spectrumObject;

        for (auto spectrumIt = spectrumMap.constBegin(); spectrumIt != spectrumMap.constEnd(); ++spectrumIt)
        {
            QString spectrumKey = spectrumIt.key();
            const QMap<int, QFileInfo>& filenbrMap = spectrumIt.value();

            QJsonObject filenbrObject;

            for (auto filenbrIt = filenbrMap.constBegin(); filenbrIt != filenbrMap.constEnd(); ++filenbrIt)
            {
                int filenbrKey = filenbrIt.key();
                const QFileInfo& fileInfo = filenbrIt.value();

                QJsonObject fileInfoObject;
                fileInfoObject["absoluteFilePath"] = fileInfo.absoluteFilePath();
                fileInfoObject["size"] = static_cast<qint64>(fileInfo.size());

                filenbrObject[QString("%1").arg(filenbrKey, 2, 10, '0')] = fileInfoObject;
            }

            spectrumObject[QString("%1").arg(spectrumKey)] = filenbrObject;
         }

        rootObject[QString("%1").arg(dateKey)] = spectrumObject;
    }


    return rootObject;
}

void FormMovie::on_rdbMeteosat_12_clicked()
{
    this->setupSpectrumMTG();
}


void FormMovie::on_rdbMeteosat_11_clicked()
{
    this->setupSpectrumMeteosat();
}

void FormMovie::on_rdbMeteosat_10_clicked()
{
    this->setupSpectrumMeteosat();
}

void FormMovie::on_rdbMeteosat_9_clicked()
{
    this->setupSpectrumMeteosat();
}



