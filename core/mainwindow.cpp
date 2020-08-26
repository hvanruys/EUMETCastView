#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#ifdef _WIN32
#include <hdf5.h>
#else
#include <hdf5/serial/hdf5.h>
#endif


extern Options opts;
extern Poi poi;
extern SegmentImage *imageptrs;
extern QFile loggingFile;

class SegmentListGeostationary;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupStatusBar();

    satlist = new SatelliteList();
    seglist = new AVHRRSatellite(this, satlist);

    formephem = new FormEphem(this, satlist, seglist);
    ui->stackedWidget->addWidget(formephem); // index 0

    //formtoolbox = NULL;

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

    for(int i = 0; i < opts.geosatellites.count(); i++)
        connect(seglist->seglgeo[i], SIGNAL(signalcomposefinished(QString)), formimage, SLOT(slotcomposefinished(QString)));

    imageptrs->gvp = new GeneralVerticalPerspective(this, seglist);
    imageptrs->lcc = new LambertConformalConic(this, seglist);
    imageptrs->sg = new StereoGraphic(this, seglist);

    QMainWindow::setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    QMainWindow::setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    QMainWindow::setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    QMainWindow::setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    forminfrascales = new FormInfraScales();
    formtoolbox = new FormToolbox(this, formimage, formgeostationary, forminfrascales, seglist);

    formimage->SetFormToolbox(formtoolbox);
    formgeostationary->SetFormToolBox(formtoolbox);

    for(int i = 0; i < opts.geosatellites.count(); i++)
        connect(seglist->seglgeo[i], SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));

    connect(seglist->seglviirsm, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglviirsdnb, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglviirsmnoaa20, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglviirsdnbnoaa20, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglolciefr, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglolcierr, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglmetop, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglnoaa, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglhrp, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglgac, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglmetopAhrpt, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglmetopBhrpt, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglnoaa19hrpt, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglM01hrpt, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglM02hrpt, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));
    connect(seglist->seglmersi, SIGNAL(progressCounter(int)), formtoolbox, SLOT(setValueProgressBar(int)));


    connect(seglist->seglviirsdnb, SIGNAL(displayDNBGraph()), formtoolbox, SLOT(slotDisplayDNBGraph()));
    connect(seglist->seglviirsdnbnoaa20, SIGNAL(displayDNBGraph()), formtoolbox, SLOT(slotDisplayDNBGraph()));


    formglobecyl = new FormMapCyl( this, mapcyl, globe, formtoolbox, satlist, seglist);

    connect(seglist, SIGNAL(signalShowSegmentCount()), formglobecyl, SLOT(slotShowSegmentCount()));

    createDockWidget();

    forminfrascales->setFormImage(formimage);

    addDockWidget(Qt::BottomDockWidgetArea, forminfrascales);
    forminfrascales->hide();

    formimage->SetDockWidgetInfraScales(forminfrascales);

    ui->stackedWidget->addWidget(formglobecyl);  // index 2

    ui->stackedWidget->addWidget(imagescrollarea);  // index 3
    ui->stackedWidget->setCurrentIndex(0);

    connect(seglist->seglmetop, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglnoaa, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglhrp, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglgac, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));

    connect(seglist->seglmetopAhrpt, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglmetopBhrpt, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglnoaa19hrpt, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglM01hrpt, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglM02hrpt, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));

    connect(seglist->seglviirsm, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglviirsdnb, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglviirsmnoaa20, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglviirsdnbnoaa20, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglolciefr, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglolcierr, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglslstr, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglmersi, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));

    connect(seglist->seglmetop, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglnoaa, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglhrp, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglgac, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));

    connect(seglist->seglmetopAhrpt, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglmetopBhrpt, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglnoaa19hrpt, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglM01hrpt, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglM02hrpt, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));

    connect(seglist->seglviirsm, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglviirsdnb, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglolciefr, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglolcierr, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));
    connect(seglist->seglmersi, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToLabel(bool)));

    connect(seglist, SIGNAL(signalXMLProgress(QString, int, bool)), formglobecyl, SLOT(slotShowXMLProgress(QString, int, bool)));

    connect( formglobecyl, SIGNAL(signalSegmentChanged(QString)), this, SLOT(updateStatusBarIndicator(QString)) );
    connect( ui->stackedWidget, SIGNAL(currentChanged(int)),formglobecyl, SLOT(updatesatmap(int)) );
    connect( formephem,SIGNAL(signalDirectoriesRead()), formgeostationary, SLOT(PopulateTree()) );
    connect( seglist,SIGNAL(signalAddedSegmentlist()), formephem, SLOT(showSegmentsAdded()));
    connect( seglist,SIGNAL(signalAddedSegmentlist()), formglobecyl, SLOT(slotShowSegmentCount()));

    connect( formephem,SIGNAL(signalDirectoriesRead()), formglobecyl, SLOT(setScrollBarMaximum()));
    connect( formglobecyl, SIGNAL(signalMakeImage()), formimage, SLOT(slotMakeImage()));

    connect( globe , SIGNAL(mapClicked()), formephem, SLOT(showSelectedSegmentList()));
    connect( globe , SIGNAL(mapClicked()), formglobecyl, SLOT(createSelectedSegmentToDownloadList()));
    connect( mapcyl , SIGNAL(mapClicked()), formephem, SLOT(showSelectedSegmentList()));

    connect( formephem, SIGNAL(signalDatagram(QByteArray)), seglist, SLOT(AddSegmentsToListFromUdp(QByteArray)));

    connect( formimage, SIGNAL(render3dgeo(int)), globe, SLOT(Render3DGeo(int)));
    connect( formimage, SIGNAL(allsegmentsreceivedbuttons(bool)), formtoolbox, SLOT(setToolboxButtons(bool)));
    connect( formimage, SIGNAL(setmapcylbuttons(bool)), formglobecyl, SLOT(slotSetMapCylButtons(bool)));
    connect( formimage->imageLabel, SIGNAL(coordinateChanged(QString)), this, SLOT(updateStatusBarCoordinate(QString)));

    connect( globe, SIGNAL(renderingglobefinished(bool)), formtoolbox, SLOT(setToolboxButtons(bool)));

    connect( formgeostationary, SIGNAL(geostationarysegmentschosen(int, QStringList)), formtoolbox, SLOT(geostationarysegmentsChosen(int, QStringList)));
    connect( formgeostationary, SIGNAL(setbuttonlabels(int, bool)), formtoolbox, SLOT(setButtons(int, bool)));

    connect( formtoolbox, SIGNAL(getgeosatchannel(QString, QVector<QString>, QVector<bool>, int, bool)), formgeostationary, SLOT(slotCreateGeoImage(QString, QVector<QString>, QVector<bool>, int, bool)));
    connect( formtoolbox, SIGNAL(switchstackedwidget(int)), this, SLOT(slotSwitchStackedWindow(int)));
    connect( formtoolbox, SIGNAL(creatergbrecipe(int)), formgeostationary, SLOT(slotCreateRGBrecipe(int)));

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

    restoreGeometry(opts.mainwindowgeometry);
    restoreState(opts.mainwindowstate);
    QMainWindow::resizeDocks({dockwidget}, {1000}, Qt::Horizontal);

    seglist->ReadXMLfiles();

    qDebug() << QString("ideal threadcount = %1  max threadcount = %2 active threadcount = %3").
                arg(QThread::idealThreadCount()).
                arg(QThreadPool::globalInstance()->maxThreadCount()).
                arg(QThreadPool::globalInstance()->activeThreadCount());

    qDebug() << "currentthreadid = " << QThread::currentThreadId() << " dockwidget width = " << dockwidget->width();

    QList<QThread*> mainwindowthreadslist = this->findChildren <QThread*> ();
    for(int i = 0; i < mainwindowthreadslist.count(); i++)
        qDebug() << mainwindowthreadslist.at(i)->currentThread()->currentThreadId();

}

void MainWindow::slotSwitchStackedWindow(int ind)
{
    qDebug() << QString("MainWindow::slotSwitchStackedWindow ind = %1").arg(ind);
    ui->stackedWidget->setCurrentIndex(ind);
    QApplication::processEvents();

}

void MainWindow::createDockWidget()
{
    dockwidget = new QDockWidget(tr("Toolbox"),this,Qt::Widget|Qt::WindowStaysOnTopHint|Qt::X11BypassWindowManagerHint);
    dockwidget->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
    dockwidget->setObjectName("Toolbox");
    QScrollArea * scrollArea = new QScrollArea(this);
    scrollArea->setWidget(formtoolbox);
    scrollArea->setWidgetResizable(true);
    dockwidget->setWidget(scrollArea);
    addDockWidget(Qt::LeftDockWidgetArea,dockwidget);
//    QMainWindow::resizeDocks({dockwidget}, {opts.toolboxwidth}, Qt::Horizontal);
    QMainWindow::resizeDocks({dockwidget}, {1000}, Qt::Horizontal);

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
    qDebug() << "in MainWindow::closeEvent(QCloseEvent *event)";

    opts.mainwindowgeometry = saveGeometry();
    opts.mainwindowstate = saveState(0);
    opts.toolboxwidth = formtoolbox->width();

    forminfrascales->close();

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

    QDir workingdir1(".");
    filters.clear();
    filters << "S3A_OL_1_*" << "S3B_OL_1_*";
    workingdir1.setNameFilters(filters);
    workingdir1.setFilter(QDir::Dirs | QDir::NoSymLinks);
    QStringList infolist = workingdir1.entryList();

    if(opts.remove_OLCI_dirs)
    {
        for (int i = 0; i < infolist.size(); ++i)
        {
            QDir deletedir(infolist.at(i));
            bool gelukt = deletedir.removeRecursively();
            if(gelukt)
                qDebug() << "removing OLCI dir : " << infolist.at(i);
        }
    }

    QDir workingdir2(".");
    filters.clear();
    filters << "S3A_SL_1_*" << "S3B_SL_1_*";
    workingdir2.setNameFilters(filters);
    workingdir2.setFilter(QDir::Dirs | QDir::NoSymLinks);
    infolist = workingdir2.entryList();

    if(opts.remove_SLSTR_dirs)
    {
        for (int i = 0; i < infolist.size(); ++i)
        {
            QDir deletedir(infolist.at(i));
            bool gelukt = deletedir.removeRecursively();
            if(gelukt)
                qDebug() << "removing SLSTR dir : " << infolist.at(i);
        }
    }



    qDebug() << "currentThreadId() = " << QThread::currentThreadId();

    QList<QThread*> mainwindowthreadslist = this->findChildren <QThread*> ();
    for(int i = 0; i < mainwindowthreadslist.count(); i++)
        qDebug() << mainwindowthreadslist.at(i)->currentThread()->currentThreadId();

    QMainWindow::closeEvent(event);

}

MainWindow::~MainWindow()
{
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
    qDebug() << "Dialog finished with result = " << result;
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
    "<br><b>Polar satellites :</b>"
    "<br>AVHHR images from Metop-A/-B/-C and NOAA-19"
    "<br>VIIRS images from SUOMI NPP and NOAA-20 (M-Band and Day/Night Band)"
    "<br>OLCI EFR/ERR and SLSTR from Sentinel-3A/-3B"
    "<br>MERSI from FengYun 3D"
    "<br><br><b>Geostationary satellites :</b>"
    "<br>XRIT from Meteosat-11, Meteosat-10, Meteosat-8"
    "<br>Electro L2, FengYun 2H, FengYun 2G"
    "<br>GOES-16, GOES-17 and Himawari-8"
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

void MainWindow::on_actionWhatsthis_triggered()
{
    QWhatsThis::enterWhatsThisMode();
}

void MainWindow::on_actionFitWindow_triggered()
{
    formimage->adjustPicSize(false);
    qDebug() << QString("mainwindow x = %1 y = %2").arg(this->width()).arg(this->height());
    qDebug() << QString("formtoolbox x = %1 y = %2").arg(formtoolbox->width()).arg(formtoolbox->height());
    qDebug() << QString("=======> totaal x = %1 y = %2").arg(this->width() - formtoolbox->width()).arg(this->height());
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
    formulaLabel->setAlignment(Qt::AlignRight);
    coordinateLabel = new QLabel;
    coordinateLabel->setAlignment(Qt::AlignRight);
    statusBar()->addWidget(timeLabel);
    statusBar()->addWidget(formulaLabel);
    statusBar()->addWidget(coordinateLabel,1);
}


void MainWindow::on_actionSatSelection_triggered()
{
    ui->actionSatSelection->setChecked(true);
    ui->stackedWidget->setCurrentIndex(0);

}

void MainWindow::on_actionMeteosat_triggered()
{
     ui->stackedWidget->setCurrentIndex(1);
     formtoolbox->setTabWidgetIndex(TAB_GEOSTATIONARY);
     //formtoolbox->setButtons(formtoolbox->getGeoIndex(), true);
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
    int indexsentinel = formtoolbox->getTabWidgetSentinelIndex();
    int indexgeostationary = formgeostationary->getTabWidgetGeoIndex();

    Q_ASSERT(index < 6);

    if(index == TAB_HISTOGRAM)
        return;

    if(index == -1 || index == TAB_AVHRR)
    {
        formimage->displayImage(IMAGE_AVHRR_COL);
    }
    else if(index == TAB_VIIRS)
    {
        if(indexviirs == 0)
            formimage->displayImage(IMAGE_VIIRSM); //VIIRSM image
        else
            formimage->displayImage(IMAGE_VIIRSDNB); //VIIRSDNB image
    }
    else if(index == TAB_SENTINEL)
    {
        if(indexsentinel == 0)
            formimage->displayImage(IMAGE_OLCI); //OLCI image
        else
            formimage->displayImage(IMAGE_SLSTR); //SLSTR image
    }
    else if(index == TAB_MERSI)
    {
        formimage->displayImage(IMAGE_MERSI); //MERSI image
    }
    else if(index == TAB_GEOSTATIONARY)
    {
        formimage->displayImage(IMAGE_GEOSTATIONARY); //Geostationary image
    }
    else if(index == TAB_PROJECTION)
        formimage->displayImage(IMAGE_PROJECTION); //Projection image

    ui->actionSatSelection->setChecked(false);
    ui->actionMeteosat->setChecked(false);
    ui->actionCylindricalEquidistant->setChecked(false);
    ui->action3DGlobe->setChecked(false);

    qDebug() << " MainWindow::on_actionImage_triggered() einde";


}

void MainWindow::updateStatusBarIndicator(const QString &text)
{
   formulaLabel->setText(text);
}

void MainWindow::updateStatusBarCoordinate(const QString &text)
{
   coordinateLabel->setText(text);
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


    if (formimage->channelshown == IMAGE_GEOSTATIONARY)
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

        if(!forminfrascales->isHidden())
        {
            QImage imresult(pm->width(), pm->height() + 80, QImage::Format_RGB32);

            QImage im = pm->toImage();
            QPainter painter(&imresult);

            QImage scales = forminfrascales->getScalesImage(im.width());
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

void MainWindow::on_actionXml_triggered()
{

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




