#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hdf5.h"

extern Options opts;
extern Poi poi;
extern SegmentImage *imageptrs;

class SegmentListGeostationary;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    restoreGeometry( opts.mainwindowgeometry);
    restoreState( opts.windowstate );

    setupStatusBar();

    satlist = new SatelliteList();
    seglist = new AVHRRSatellite(this, satlist);


    formephem = new FormEphem(this, satlist, seglist);
    ui->stackedWidget->addWidget(formephem); // index 0

    formtoolbox = NULL;

    formgeostationary = new FormGeostationary(this, satlist, seglist);
    ui->stackedWidget->addWidget(formgeostationary); // index 1

    cylequidist = new CylEquiDist( opts.backgroundimage2D );
    mapcyl = new MapFieldCyl(this, cylequidist, satlist, seglist);
    globe = new Globe(this, satlist, seglist);

    formimage = new FormImage(this, satlist, seglist);
    imagescrollarea = new  ImageScrollArea();
    imagescrollarea->setBackgroundRole(QPalette::Dark);
    imagescrollarea->setWidget(formimage);

    formgeostationary->SetFormImage(formimage);
    connect(formimage, SIGNAL(moveImage(QPoint, QPoint)), this, SLOT(moveImage(QPoint, QPoint)));

    for( int i = 0; i < 8; i++)
    {
        connect(&seglist->seglmeteosat->watcherRed[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
        connect(&seglist->seglmeteosat->watcherGreen[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
        connect(&seglist->seglmeteosat->watcherBlue[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
    }

    for( int i = 0; i < 24; i++)
    {
        connect(&seglist->seglmeteosat->watcherHRV[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
    }

    for( int i = 0; i < 8; i++)
    {
        connect(&seglist->seglmeteosatrss->watcherRed[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
        connect(&seglist->seglmeteosatrss->watcherGreen[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
        connect(&seglist->seglmeteosatrss->watcherBlue[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
    }

    for( int i = 0; i < 24; i++)
    {
        connect(&seglist->seglmeteosatrss->watcherHRV[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
    }

/*    for( int i = 0; i < 6; i++)
    {
        connect(&seglist->seglelectro->watcherMono[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
    }
*/
    for( int i = 0; i < 10; i++)
    {
        connect(&seglist->seglmet7->watcherMono[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
    }

    for( int i = 0; i < 7; i++)
    {
        connect(&seglist->seglgoes13dc3->watcherMono[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
    }
    for( int i = 0; i < 7; i++)
    {
        connect(&seglist->seglgoes15dc3->watcherMono[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
    }
    for( int i = 0; i < 6; i++)
    {
        connect(&seglist->seglmtsatdc3->watcherMono[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
    }

    for( int i = 0; i < 7; i++)
    {
        connect(&seglist->seglgoes13dc4->watcherMono[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
    }
    for( int i = 0; i < 7; i++)
    {
        connect(&seglist->seglgoes15dc4->watcherMono[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
    }
    for( int i = 0; i < 6; i++)
    {
        connect(&seglist->seglmtsatdc4->watcherMono[i], SIGNAL(finished()), formimage, SLOT(slotUpdateMeteosat()));
    }

    connect(seglist->seglfy2e, SIGNAL(imagefinished()), formimage, SLOT(slotUpdateMeteosat()));
    connect(seglist->seglfy2g, SIGNAL(imagefinished()), formimage, SLOT(slotUpdateMeteosat()));

    for( int i = 0; i < 10; i++)
    {
        connect(&seglist->seglh8->watcherRed[i], SIGNAL(finished()), formimage, SLOT(slotUpdateHimawari()));
        connect(&seglist->seglh8->watcherGreen[i], SIGNAL(finished()), formimage, SLOT(slotUpdateHimawari()));
        connect(&seglist->seglh8->watcherBlue[i], SIGNAL(finished()), formimage, SLOT(slotUpdateHimawari()));
    }

    imageptrs->gvp = new GeneralVerticalPerspective(this, seglist);
    imageptrs->lcc = new LambertConformalConic(this, seglist);
    imageptrs->sg = new StereoGraphic(this, seglist);

    formtoolbox = new FormToolbox(this, formimage, formgeostationary, seglist);


    QVBoxLayout *verticalLayout;
    imageWidget = new QWidget(this);
    verticalLayout = new QVBoxLayout(imageWidget);
    verticalLayout->addWidget(imagescrollarea);
    this->infrascales = new InfraScales(this);
    this->infrascales->setMaximumHeight(80);
    this->infrascales->hide();
    verticalLayout->addWidget(infrascales);

    formtoolbox->SetInfraScales(infrascales);
    formimage->SetFormToolbox(formtoolbox);
    formimage->SetInfraScales(infrascales);
    formgeostationary->SetFormToolBox(formtoolbox);

    connect(infrascales, SIGNAL(repaintprojectionimage()), formimage, SLOT(slotRepaintProjectionImage()));
    connect(seglist->seglmeteosat, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglmeteosatrss, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    //connect(seglist->seglelectro, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglmet7, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglgoes13dc3, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglgoes15dc3, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglmtsatdc3, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglgoes13dc4, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglgoes15dc4, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglmtsatdc4, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglfy2e, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglfy2g, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglh8, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglviirsm, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglviirsdnb, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglmetop, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglnoaa, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglhrp, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglgac, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));


    formglobecyl = new FormMapCyl( this, mapcyl, globe, formtoolbox, satlist, seglist);

    connect(seglist, SIGNAL(signalNothingSelected()), formglobecyl, SLOT(slotNothingSelected()));

    createDockWindows();

    ui->stackedWidget->addWidget(formglobecyl);  // index 2

//    ui->stackedWidget->addWidget(imagescrollarea);  // index 3
    ui->stackedWidget->addWidget(imageWidget);  // index 3
    ui->stackedWidget->setCurrentIndex(0);

    connect(seglist->seglmetop, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglnoaa, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglhrp, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglgac, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglviirsm, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglviirsdnb, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabelDNB(bool)));

    connect(seglist->seglmetop, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglnoaa, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglhrp, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglgac, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglviirsm, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglviirsdnb, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));


    connect( formglobecyl, SIGNAL(signalSegmentChanged(QString)), this, SLOT(updateStatusBarIndicator(QString)) );
    connect( ui->stackedWidget, SIGNAL(currentChanged(int)),formglobecyl, SLOT(updatesatmap(int)) );
    connect( formephem,SIGNAL(signalDirectoriesRead()), formgeostationary, SLOT(PopulateTree()) );
    connect( seglist,SIGNAL(signalAddedSegmentlist()), formephem, SLOT(showSegmentsAdded()));

    connect( formephem,SIGNAL(signalDirectoriesRead()), formglobecyl, SLOT(setScrollBarMaximum()));
    connect( formglobecyl, SIGNAL(emitMakeImage()), formimage, SLOT(slotMakeImage()));
    connect( formtoolbox, SIGNAL(emitShowVIIRSImage()), formimage, SLOT(slotShowVIIRSMImage()));

    connect( globe , SIGNAL(mapClicked()), formephem, SLOT(showSelectedSegmentList()));
    connect( mapcyl , SIGNAL(mapClicked()), formephem, SLOT(showSelectedSegmentList()));

    connect( formephem, SIGNAL(signalDatagram(QByteArray)), seglist, SLOT(AddSegmentsToListFromUdp(QByteArray)));

    connect( formimage, SIGNAL(render3dgeo(SegmentListGeostationary::eGeoSatellite)), globe, SLOT(Render3DGeo(SegmentListGeostationary::eGeoSatellite)));
    connect( formimage, SIGNAL(allsegmentsreceivedbuttons(bool)), formtoolbox, SLOT(setToolboxButtons(bool)));
    connect( globe, SIGNAL(renderingglobefinished(bool)), formtoolbox, SLOT(setToolboxButtons(bool)));

    connect( formgeostationary, SIGNAL(geostationarysegmentschosen(SegmentListGeostationary::eGeoSatellite, QStringList)), formtoolbox, SLOT(geostationarysegmentsChosen(SegmentListGeostationary::eGeoSatellite, QStringList)));
    connect( formtoolbox, SIGNAL(getmeteosatchannel(QString, QVector<QString>, QVector<bool>)), formgeostationary, SLOT(CreateGeoImage(QString, QVector<QString>, QVector<bool>)));
    connect( formtoolbox, SIGNAL(screenupdateprojection()), formimage, SLOT(slotUpdateProjection()));
    connect( formtoolbox, SIGNAL(switchstackedwidget(int)), this, SLOT(slotSwitchStackedWindow(int)));

    connect( formgeostationary, SIGNAL(enabletoolboxbuttons(bool)), formtoolbox, SLOT(setToolboxButtons(bool)));

    formtoolbox->setChannelIndex();

    setWindowTitle(tr("EUMETCast Viewer"));
    timer = new QTimer( this );
    timer->start( 1000);
    connect(timer, SIGNAL(timeout()), formephem, SLOT(timerDone()));
    connect(timer, SIGNAL(timeout()), this, SLOT(timerDone()));

    herr_t  h5_status;
    unsigned int majnum;
    unsigned int minnum;
    unsigned int relnum;

    h5_status = H5get_libversion(&majnum, &minnum, &relnum);

    qDebug() << QString("HDF5 library %1.%2.%3").arg(majnum).arg(minnum).arg(relnum);


}

void MainWindow::slotSwitchStackedWindow(int ind)
{
    ui->stackedWidget->setCurrentIndex(ind);
}

void MainWindow::createDockWindows()
{
    dockwidget = new QDockWidget(tr("Toolbox"),this,Qt::Widget|Qt::WindowStaysOnTopHint|Qt::X11BypassWindowManagerHint);
    dockwidget->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
    dockwidget->setObjectName("Toolbox");
    QScrollArea * scrollArea = new QScrollArea(this);
    scrollArea->setWidget(formtoolbox);
    scrollArea->setWidgetResizable(true);
    dockwidget->setWidget(scrollArea);
    dockwidget->setMinimumWidth(400);
    //dockwidget->close();
    addDockWidget(Qt::LeftDockWidgetArea,dockwidget);
}



void MainWindow::timerDone(void)
{
    char tempstr[150];
    struct tm utc;
    time_t t;

    t = time(0);
    utc = *gmtime(&t);
    strcpy(tempstr, "GMT : ");
    strncat(tempstr, asctime(&utc), 20 );
    strcat(tempstr, "      Local : ");
    utc = *localtime(&t);
    strncat(tempstr, asctime(&utc), 20 );
    timeLabel->setText(QString(tempstr));
}





void MainWindow::closeEvent(QCloseEvent *event)
{
    opts.mainwindowgeometry = saveGeometry();
    opts.windowstate = saveState(0);
    QMainWindow::closeEvent(event);
}

MainWindow::~MainWindow()
{

    delete ui;
    delete timer;

    delete formtoolbox;
    delete formephem;
    delete formglobecyl;
    delete cylequidist;
    delete satlist;
    delete seglist;

    opts.Save();
    poi.Save();

    imageptrs->DeleteImagePtrs();

    // Deleting h5 files in program directory

    QDir workingdir(".");
    QStringList filters;
    filters << "*.h5" << "*.HDF" << "IMG_DK01*";
    workingdir.setNameFilters(filters);
    QFileInfoList fileinfolist;

    workingdir.setFilter(QDir::Files | QDir::NoSymLinks);
    workingdir.setSorting(QDir::Name);
    fileinfolist = workingdir.entryInfoList();
    for (int i = 0; i < fileinfolist.size(); ++i)
    {
        QFile h5file(fileinfolist.at(i).fileName());
        if(h5file.remove())
            qDebug() << QString("Deleting h5 and Himawari files : %1").arg(fileinfolist.at(i).fileName());
    }

    qDebug() << "================closing MainWindow================";

}

void MainWindow::on_actionPreferences_triggered()
{
    DialogPreferences *pref=new DialogPreferences(this);
    pref->setAttribute(Qt::WA_DeleteOnClose);
    pref->show();
    connect(pref,SIGNAL(finished(int)), formimage, SLOT(slotRefreshOverlay()));
    connect(pref,SIGNAL(finished(int)), this, SLOT(slotPreferencesFinished(int)));
}

void MainWindow::slotPreferencesFinished(int result)
{
    if(result == 2)
        formtoolbox->setPOIsettings();
}

void MainWindow::on_actionAbout_triggered()
{
    const QString htmlText =
    "<HTML>"
    "<center><p><b>===EUMETCastView===</b></p></center>"
    "<b>Image Viewer for the EUMETCast transmissions</b>"
    "<center><b>Version " + QApplication::applicationVersion() + "</b></center>"
    "<p>supports the following satellites</p>"
    "<p>Metop-A, Metop-B </p>"
    "<p>Noaa, SUOMI NPP (M-Band and Day/Night Band)</p>"
    "<p>Meteosat-10, Meteosat-9, Meteosat7</p>"
    "<p>FengYun 2E, FengYun 2G</p>"
    "<p>GOES-13, GOES-15</p>"
    "<p>MTSAT-2, Himawari-8</p>"
    "<ul>"
    "<li>Made by Hugo Van Ruyskensvelde.</li>"
    "</HTML>";

    QString title = QString("About EUMETCastView Version %1").arg(QApplication::applicationVersion());
    QMessageBox::about(this, title, htmlText);
}

void MainWindow::on_actionNormalSize_triggered()
{
       formimage->normalSize();
}

void MainWindow::on_actionZoomin_triggered()
{
       formimage->zoomIn();
}

void MainWindow::on_actionZoomout_triggered()
{
       formimage->zoomOut();
}


void MainWindow::on_actionFitWindow_triggered()
{
    formimage->adjustPicSize(false);
       //qDebug() << QString("mainwindow x = %1 y = %2").arg(this->width()).arg(this->height());
       //qDebug() << QString("formtoolbox x = %1 y = %2").arg(formtoolbox->width()).arg(formtoolbox->height());
       //qDebug() << QString("=======> totaal x = %1 y = %2").arg(this->width() - formtoolbox->width()).arg(this->height());
}

void MainWindow::on_actionFitWindowWidth_triggered()
{
    formimage->adjustPicSize(true);
}

void MainWindow::setupStatusBar()
{
    timeLabel = new QLabel;
    timeLabel->setAlignment(Qt::AlignHCenter);
    timeLabel->setMinimumSize(timeLabel->sizeHint());
    formulaLabel = new QLabel;
    formulaLabel->setIndent(3);
    statusBar()->addWidget(timeLabel);
    statusBar()->addWidget(formulaLabel, 1);
}


void MainWindow::on_actionSatSelection_triggered()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_actionMeteosat_triggered()
{
     ui->stackedWidget->setCurrentIndex(1);
}


void MainWindow::on_actionCylindricalEquidistant_triggered()
{
    ui->stackedWidget->setCurrentIndex(2);
    formglobecyl->setCylOrGlobe(0);
}

void MainWindow::on_action3DGlobe_triggered()
{
    ui->stackedWidget->setCurrentIndex(2);
    formglobecyl->setCylOrGlobe(1);
}

void MainWindow::on_actionImage_triggered()
{
    ui->stackedWidget->setCurrentIndex(3);
    int index = formtoolbox->getTabWidgetIndex();
    int indexviirs = formtoolbox->getTabWidgetVIIRSIndex();

    qDebug() << " MainWindow::on_actionImage_triggered() index = " << index;
    if(index == -1)
        formimage->displayImage(IMAGE_AVHRR_COL);
    else if(index == 0)
        formimage->displayImage(IMAGE_AVHRR_COL); // AVHRR Color image
    else if(index == 1)
    {
        if(indexviirs == 0)
            formimage->displayImage(IMAGE_VIIRS_M); //VIIRSM image
        else
            formimage->displayImage(IMAGE_VIIRS_DNB); //VIIRSDNB image
    }
    else if(index == 2)  //Geostationary image
        formimage->displayImage(IMAGE_GEOSTATIONARY);
    else if(index == 3)
        formimage->displayImage(IMAGE_EQUIRECTANGLE); // Equirectangular
    else
        formimage->displayImage(IMAGE_PROJECTION); //Projection image

}

void MainWindow::updateStatusBarIndicator(const QString &text)
{
   formulaLabel->setText(text);
}


void MainWindow::on_actionShowToolbox_triggered()
{
    if (dockwidget->isHidden())
        dockwidget->show();
    else
        dockwidget->hide();

}


void MainWindow::on_actionCreatePNG_triggered()
{
    const QPixmap *pm;
    QString filestr;

    filestr.append("./");


    if (formimage->channelshown == 8)
    {
        filestr += formtoolbox->returnFilenamestring();
    }


    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save image"), filestr,
            tr("*.png;;*.jpg"));
    if (fileName.isEmpty())
             return;
    else
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        if(fileName.mid(fileName.length()-4) != ".jpg" && fileName.mid(fileName.length()-4) != ".jpg" &&
                fileName.mid(fileName.length()-4) != ".png" && fileName.mid(fileName.length()-4) != ".PNG")
            fileName.append(".jpg");
        pm = formimage->returnimageLabelptr()->pixmap();

        if(!infrascales->isHidden())
        {
            QImage imresult(pm->width(), pm->height() + 80, QImage::Format_RGB32);

            QImage im = pm->toImage();
            QPainter painter(&imresult);

            QImage scales = infrascales->getScalesImage(im.width());
            //QImage scales(im.width(), 80, im.format());
            //scales.fill(Qt::blue);

            painter.drawImage(0, 0, im);
            painter.drawImage(0, imresult.height()-80, scales);

            painter.end();
            imresult.save(fileName);
        }
        else
        {
            pm->save(fileName);
        }
        QApplication::restoreOverrideCursor();
    }

}

void MainWindow::moveImage(QPoint d, QPoint e)
{
    int width = imagescrollarea->width();
    int height = imagescrollarea->height();

    QPoint mousePos = d;

    int deltaX = e.x() - mousePos.x();
    int deltaY = e.y() - mousePos.y();


    if (mousePos.y() <= 4 && imagescrollarea->verticalScrollBar()->value() < imagescrollarea->verticalScrollBar()->maximum() - 10) {
            // wrap mouse from top to bottom
            mousePos.setY(height - 5);
    } else if (mousePos.y() >= height - 4 && imagescrollarea->verticalScrollBar()->value() > 10) {
            // wrap mouse from bottom to top
            mousePos.setY(5);
    }

    if (mousePos.x() <= 4 && imagescrollarea->horizontalScrollBar()->value() < imagescrollarea->horizontalScrollBar()->maximum() - 10) {
            // wrap mouse from left to right
            mousePos.setX(width - 5);
    } else if (mousePos.x() >= width - 4 && imagescrollarea->horizontalScrollBar()->value() > 10) {
            // wrap mouse from right to left
            mousePos.setX(5);
    }

    imagescrollarea->horizontalScrollBar()->setValue(imagescrollarea->horizontalScrollBar()->value() + deltaX);
    imagescrollarea->verticalScrollBar()->setValue(imagescrollarea->verticalScrollBar()->value() + deltaY);

}

void MainWindow::updateWindowTitle()
{
        QString windowTitleFormat = QString("EUMETCastView zoomLevel");
        //windowTitleFormat.replace("imageName", fileUtils->getFileName());
        //windowTitleFormat.replace("pos", QString("%1").arg(fileUtils->getCurrentPosition()+1));
        //windowTitleFormat.replace("amount", QString("%1").arg(fileUtils->getFilesAmount()));
        windowTitleFormat.replace("zoomLevel", QString("%1%").arg(formimage->getZoomValue()));
        this->setWindowTitle(windowTitleFormat);
}

