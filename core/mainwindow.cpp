#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#ifdef _WIN32
#include <hdf5.h>
#else
//#include <hdf5/serial/hdf5.h>
#include <hdf5.h>
#endif


extern Options opts;
extern Poi poi;
extern SegmentImage *imageptrs;
extern QFile loggingFile;
extern QTextStream outlogging;
extern SatelliteList satellitelist;
class SegmentListGeostationary;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupStatusBar();

    seglist = new AVHRRSatellite(this);

    formephem = new FormEphem(this, seglist);
    ui->stackedWidget->addWidget(formephem); // index 0

    //formtoolbox = NULL;

    formgeostationary = new FormGeostationary(this, seglist);
    ui->stackedWidget->addWidget(formgeostationary); // index 1

    cylequidist = new CylEquiDist( opts.appdir_env == "" ? opts.backgroundimage2D : opts.appdir_env + "/" + opts.backgroundimage2D );
    mapcyl = new MapFieldCyl(this, cylequidist, seglist);
    globe = new Globe(this, seglist);

    formimage = new FormImage(this, seglist);
    formgeostationary->SetFormImage(formimage);

    for(int i = 0; i < opts.geosatellites.count(); i++)
        connect(seglist->seglgeo[i], SIGNAL(signalcomposefinished(QString, int)), formimage, SLOT(slotcomposefinished(QString, int)));

    imageptrs->gvp = new GeneralVerticalPerspective(this, seglist);
    imageptrs->lcc = new LambertConformalConic(this, seglist);
    imageptrs->sg = new StereoGraphic(this, seglist);
    imageptrs->om = new ObliqueMercator(this, seglist);

    QMainWindow::setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    QMainWindow::setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    QMainWindow::setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    QMainWindow::setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    formtoolbox = new FormToolbox(this, formimage, formgeostationary, seglist);

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


    formglobecyl = new FormMapCyl( this, mapcyl, globe, formtoolbox, seglist);

    connect(seglist, SIGNAL(signalShowSegmentCount()), formglobecyl, SLOT(slotShowSegmentCount()));

    createDockWidget();


    ui->stackedWidget->addWidget(formglobecyl);  // index 2

    ui->stackedWidget->addWidget(formimage); //imagescrollarea);  // index 3

    formmovie = new FormMovie(this);
    formmovie->SetFormToolbox(formtoolbox);
    formtoolbox->setFormMovie(formmovie);

    ui->stackedWidget->addWidget(formmovie); // index 4

    ui->stackedWidget->setCurrentIndex(0);

    connect(seglist->seglmetop, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglnoaa, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglhrp, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglgac, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));

    connect(seglist->seglmetopAhrpt, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglmetopBhrpt, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglnoaa19hrpt, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglM01hrpt, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglM02hrpt, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));

    connect(seglist->seglviirsm, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglviirsdnb, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglviirsmnoaa20, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglviirsdnbnoaa20, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglolciefr, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglolcierr, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglslstr, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglmersi, SIGNAL(segmentlistfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));

    connect(seglist->seglmetop, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglnoaa, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglhrp, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglgac, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));

    connect(seglist->seglmetopAhrpt, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglmetopBhrpt, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglnoaa19hrpt, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglM01hrpt, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglM02hrpt, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));

    connect(seglist->seglviirsm, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglviirsdnb, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglviirsmnoaa20, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglviirsdnbnoaa20, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglolciefr, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglolcierr, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));
    connect(seglist->seglmersi, SIGNAL(segmentprojectionfinished(bool)), formimage, SLOT(setPixmapToScene(bool)));

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
    //connect( formimage, SIGNAL(coordinateChanged(QString)), this, SLOT(updateStatusBarCoordinate(QString)));

    connect( globe, SIGNAL(renderingglobefinished(bool)), formtoolbox, SLOT(setToolboxButtons(bool)));

    connect( formgeostationary, SIGNAL(geostationarysegmentschosen(int, QStringList)), formtoolbox, SLOT(geostationarysegmentsChosen(int, QStringList)));
    connect( formgeostationary, SIGNAL(setbuttonlabels(int, bool)), formtoolbox, SLOT(setButtons(int, bool)));

    connect( formtoolbox, SIGNAL(getgeosatchannel(QString, QVector<QString>, QVector<bool>, int, bool)), formgeostationary, SLOT(slotCreateGeoImage(QString, QVector<QString>, QVector<bool>, int, bool)));
    connect( formtoolbox, SIGNAL(switchstackedwidget(int)), this, SLOT(slotSwitchStackedWindow(int)));
    connect( formtoolbox, SIGNAL(creatergbrecipe(int)), formgeostationary, SLOT(slotCreateRGBrecipe(int)));

    connect( formgeostationary, SIGNAL(enabletoolboxbuttons(bool)), formtoolbox, SLOT(setToolboxButtons(bool)));

    connect( imageptrs->om, SIGNAL(aspectratioChanged(QPoint)), formtoolbox, SLOT(slotChangeAspectRatio(QPoint)));

    connect( formimage, SIGNAL(signalMainWindowTitleChanged(QString)), this, SLOT(slotMainWindowTitleChanged(QString)));

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

    {
        unsigned size, mask;
        char     buf[255];

        // retrieve the number of entries in the plugin path list
        if (H5PLsize(&size) < 0) {
            qDebug() << "error retrieve the number of entries in the plugin path list";
            goto fail_read;
        }
        else
            qDebug() << "Number of stored plugin paths: " << size;

        // check the plugin state mask
        if (H5PLget_loading_state(&mask) < 0) {
            qDebug() << "error check the plugin state mask";
            goto fail_read;
        }
        else
        {
            QString canit = (mask & H5PL_FILTER_PLUGIN) == 1 ? "can" : "can't";
            qDebug() << "Filter plugins " << canit << " be loaded.";

            // print the paths in the plugin path list
            for (unsigned i = 0; i < size; ++i) {
                if (H5PLget(i, buf, 255) < 0) {
                    break;
                }
                qDebug() << "the plugin path list = " << buf;
            }
        }

    fail_read:;
    }

    //char envvar[] = "HDF5_PLUGIN_PATH";
#ifdef WIN32
    char envvar[] = "HDF5_PLUGIN_PATH=.\\";
#else
    char envvar[] = "HDF5_PLUGIN_PATH=./";
#endif

    // Make sure envvar actually exists
    if(!getenv("HDF5_PLUGIN_PATH")){
        fprintf(stderr, "The environment variable HDF5_PLUGIN_PATH was not found.\n");
        fprintf(stderr, "setting environment variable.\n");
        //setenv(envvar, ".", 1);
        putenv(envvar);
    }

    char *myptr = getenv("HDF5_PLUGIN_PATH");
    QString str = QString::fromLocal8Bit(myptr);

    qDebug() << QString("getenv = %1").arg(str);

    int avail = H5Zfilter_avail(32018);
    if (avail == 0) {
        qDebug() << "FCIDECOMP filter is not available.";
        opts.bFciDecomp = false;
    }
    else
    {
        qDebug() << "FCIDECOMP filter is available.";
        opts.bFciDecomp = true;
    }


    restoreGeometry(opts.mainwindowgeometry);
    restoreState(opts.mainwindowstate);

    ui->toolBar->setVisible(true);
    ui->mainToolBar->setVisible(true);
    //    dockwidget->setVisible(true);


    QMainWindow::resizeDocks({dockwidget}, {1000}, Qt::Horizontal);
    //bool restored = QMainWindow::restoreDockWidget(dockwidget);
    //qDebug() << "restoredockwidget = " << restored << " width of toolbox = " << formtoolbox->width();


    seglist->ReadXMLfiles();

    qDebug() << QString("ideal threadcount = %1  max threadcount = %2 active threadcount = %3").
                arg(QThread::idealThreadCount()).
                arg(QThreadPool::globalInstance()->maxThreadCount()).
                arg(QThreadPool::globalInstance()->activeThreadCount());

    qDebug() << "currentthreadid = " << QThread::currentThreadId() << " dockwidget width = " << dockwidget->width();

    QList<QThread*> mainwindowthreadslist = this->findChildren <QThread*> ();
    for(int i = 0; i < mainwindowthreadslist.count(); i++)
        qDebug() << mainwindowthreadslist.at(i)->currentThread()->currentThreadId();

    if(!QFile::exists("weather.txt"))
        formephem->downloadTLE();



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
    QScrollArea * scrollArea = new QScrollArea;
    scrollArea->setWidget(formtoolbox);
    scrollArea->setWidgetResizable(true);
    scrollArea->resize(700, MainWindow::height());
    dockwidget->setWidget(scrollArea);
    dockwidget->resize(700, MainWindow::height());
    addDockWidget(Qt::LeftDockWidgetArea,dockwidget);
//    QMainWindow::resizeDocks({dockwidget}, {opts.toolboxwidth}, Qt::Horizontal);
    //QMainWindow::resizeDocks({dockwidget}, {1000}, Qt::Horizontal);

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

    qDebug() << "Width of toolbox = " << formtoolbox->width();

    delete timer;
    delete formtoolbox;
    delete formephem;
    delete formglobecyl;
    delete cylequidist;
    delete seglist;
    delete formmovie;

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

//    QFile segfile("Segments.xml");
//    segfile.remove();


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
    outlogging.flush();
    loggingFile.close();
    qInstallMessageHandler(nullptr);

}

void MainWindow::on_actionPreferences_triggered()
{
    DialogPreferences *pref=new DialogPreferences(this);
    pref->setAttribute(Qt::WA_DeleteOnClose);
    pref->show();
    //connect(pref,SIGNAL(finished(int)), formimage, SLOT(slotRefreshOverlay()));
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
    "<br>AVHRR images from Metop-A/-B/-C and NOAA-19"
    "<br>VIIRS images from SUOMI NPP and NOAA-20 (M-Band and Day/Night Band)"
    "<br>OLCI EFR/ERR and SLSTR from Sentinel-3A/-3B"
    "<br>MERSI from FengYun 3D"
    "<br><br><b>Geostationary satellites :</b>"
    "<br>XRIT from Meteosat-11, Meteosat-10, Meteosat-8"
    "<br>FCI from MTG-I1"
    "<br>Electro L3, FengYun 2H, FengYun 2G"
    "<br>GOES-16, GOES-17 and Himawari-8"
    "<ul>"
    "<li>Made by Hugo Van Ruyskensvelde.</li>"
    "</HTML>";

    QString title = QString("About EUMETCastView Version %1").arg(QApplication::applicationVersion());
    QMessageBox::about(this, title, htmlText);
}

void MainWindow::on_actionNormalSize_triggered()
{
    formimage->originalSize();
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
    formimage->fitWindow();
    qDebug() << QString("mainwindow x = %1 y = %2").arg(this->width()).arg(this->height());
    qDebug() << QString("formtoolbox x = %1 y = %2").arg(formtoolbox->width()).arg(formtoolbox->height());
    qDebug() << QString("=======> totaal x = %1 y = %2").arg(this->width() - formtoolbox->width()).arg(this->height());
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
    statusBar()->addWidget(coordinateLabel);
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

void MainWindow::on_actionSettingsMovie_triggered()
{
    ui->stackedWidget->setCurrentIndex(4);
    formmovie->getProjectionData();

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
        formimage->displayImage(IMAGE_AVHRR_COL, true);
    }
    else if(index == TAB_VIIRS)
    {
        if(indexviirs == 0)
            formimage->displayImage(IMAGE_VIIRSM, true); //VIIRSM image
        else
            formimage->displayImage(IMAGE_VIIRSDNB, true); //VIIRSDNB image
    }
    else if(index == TAB_SENTINEL)
    {
        if(indexsentinel == 0)
            formimage->displayImage(IMAGE_OLCI, true); //OLCI image
        else
            formimage->displayImage(IMAGE_SLSTR, true); //SLSTR image
    }
    else if(index == TAB_MERSI)
    {
        formimage->displayImage(IMAGE_MERSI, true); //MERSI image
    }
    else if(index == TAB_GEOSTATIONARY)
    {
        formimage->displayImage(IMAGE_GEOSTATIONARY, true); //Geostationary image
    }
    else if(index == TAB_PROJECTION)
    {
        if( formtoolbox->getToolboxIndex() == 3)
        {
            int w, h;
            formtoolbox->getOMimagesize(&w, &h);
            imageptrs->om->Initialize(R_MAJOR_A_WGS84, R_MAJOR_B_WGS84, formtoolbox->getCurrentProjectionType(), w, h);
        }
//        formimage->UpdateProjection();

       // formimage->displayImage(IMAGE_PROJECTION); //Projection image
    }

    ui->actionSatSelection->setChecked(false);
    ui->actionMeteosat->setChecked(false);
    ui->actionCylindricalEquidistant->setChecked(false);
    ui->action3DGlobe->setChecked(false);

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
    QString filestr = "";

    filestr.append("./");


    if (formimage->channelshown == IMAGE_GEOSTATIONARY)
    {
        filestr += formtoolbox->returnImageFilenamestring();
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

//        QPixmap *pixmapVal = formimage->getPixmap();
//        pixmapVal->save(fileName);

//        QImage im(3712, 3712, QImage::Format_ARGB32);
//        QPainter painter(&im);
//        painter.setRenderHint(QPainter::SmoothPixmapTransform);


//        painter.begin(&im);
//        formimage->getScene()->render(&painter, im.rect() ); //, rectbuffer());
//        formimage->drawOverlays(&painter);
//        painter.end();

//        im.save(fileName);

//        QGraphicsView* view = new QGraphicsView(formimage->getScene(),this);
//        QString fileName = "file_name.png";
//        QPixmap pixMap = view->grab(view->sceneRect().toRect());
//        pixMap.save(fileName);
        //Uses QWidget::grab function to create a pixmap and paints the QGraphicsView inside it.

        formimage->savePNGImage(fileName);
        QApplication::restoreOverrideCursor();

    }
}

//void MainWindow::moveImage(QPoint d, QPoint e)
//{
//    int width = imagescrollarea->width();
//    int height = imagescrollarea->height();

//    QPoint mousePos = d;

//    int deltaX = e.x() - mousePos.x();
//    int deltaY = e.y() - mousePos.y();


//    if (mousePos.y() <= 4 && imagescrollarea->verticalScrollBar()->value() < imagescrollarea->verticalScrollBar()->maximum() - 10) {
//            // wrap mouse from top to bottom
//            mousePos.setY(height - 5);
//    } else if (mousePos.y() >= height - 4 && imagescrollarea->verticalScrollBar()->value() > 10) {
//            // wrap mouse from bottom to top
//            mousePos.setY(5);
//    }

//    if (mousePos.x() <= 4 && imagescrollarea->horizontalScrollBar()->value() < imagescrollarea->horizontalScrollBar()->maximum() - 10) {
//            // wrap mouse from left to right
//            mousePos.setX(width - 5);
//    } else if (mousePos.x() >= width - 4 && imagescrollarea->horizontalScrollBar()->value() > 10) {
//            // wrap mouse from right to left
//            mousePos.setX(5);
//    }

//    imagescrollarea->horizontalScrollBar()->setValue(imagescrollarea->horizontalScrollBar()->value() + deltaX);
//    imagescrollarea->verticalScrollBar()->setValue(imagescrollarea->verticalScrollBar()->value() + deltaY);

//}

void MainWindow::updateWindowTitle()
{
        QString windowTitleFormat = QString("EUMETCastView zoomLevel");
        //windowTitleFormat.replace("imageName", fileUtils->getFileName());
        //windowTitleFormat.replace("pos", QString("%1").arg(fileUtils->getCurrentPosition()+1));
        //windowTitleFormat.replace("amount", QString("%1").arg(fileUtils->getFilesAmount()));
//        windowTitleFormat.replace("zoomLevel", QString("%1%").arg(formimage->getZoomValue()));
        this->setWindowTitle(windowTitleFormat);
}




