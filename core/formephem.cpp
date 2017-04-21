#include "formephem.h"

#include "ui_formephem.h"
#include <QDebug>

extern Options opts;


QTreeWidgetItem *item(QString name, QTreeWidgetItem* parent=0) {
     QTreeWidgetItem *retval = new QTreeWidgetItem(parent);
     retval->setFlags(Qt::ItemIsSelectable |
         Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable |
         Qt::ItemIsEnabled);
     retval->setText(0, name);
     return retval;
}

FormEphem::FormEphem(QWidget *parent, SatelliteList *satlist, AVHRRSatellite *seglist):
    QWidget(parent),
    ui(new Ui::FormEphem)
{
    ui->setupUi(this);

    if(ui->splitter->restoreState( opts.ephemsplittersizes ))
        qDebug() << "splitter ok";
    else
        qDebug() << "splitter not ok";


    sats = satlist;
    segs = seglist;
    segs->setXMLDate(ui->calendar->selectedDate());

    resetProgressBar(1, "    ");

    ui->realminutesslider->setFocusPolicy(Qt::StrongFocus);
    ui->realminutesslider->setTickPosition(QSlider::TicksBothSides);
    ui->realminutesslider->setTickInterval(3);
    ui->realminutesslider->setSingleStep(1);
    ui->realminutesslider->setMinimum(1);
    ui->realminutesslider->setMaximum(90);
    ui->realminutesslider->setValue(opts.realminutesshown);

    ui->realminutesspinbox->setMinimum(1);
    ui->realminutesspinbox->setMaximum(90);
    ui->realminutesspinbox->setValue(opts.realminutesshown);

    ui->sliNbrOfHours->setFocusPolicy(Qt::StrongFocus);
    ui->sliNbrOfHours->setTickPosition(QSlider::TicksBothSides);
    ui->sliNbrOfHours->setTickInterval(1);
    ui->sliNbrOfHours->setSingleStep(1);
    ui->sliNbrOfHours->setMinimum(0);
    ui->sliNbrOfHours->setMaximum(24);
    ui->sliNbrOfHours->setValue(opts.nbrofhours);

    ui->spbNbrOfHours->setMinimum(0);
    ui->spbNbrOfHours->setMaximum(24);
    ui->spbNbrOfHours->setValue(opts.nbrofhours);

    ui->segmentsslider->setFocusPolicy(Qt::StrongFocus);
    ui->segmentsslider->setTickPosition(QSlider::TicksBothSides);
    ui->segmentsslider->setTickInterval(3);
    ui->segmentsslider->setSingleStep(1);
    ui->segmentsslider->setMinimum(1);
    ui->segmentsslider->setMaximum(200);
    ui->segmentsslider->setValue(opts.nbrofvisiblesegments);


    ui->segmentsspinbox->setMinimum(1);
    ui->segmentsspinbox->setMaximum(200);
    ui->segmentsspinbox->setValue(opts.nbrofvisiblesegments);

    ui->calendar->setMinimumDate(QDate(1900, 1, 1));
    ui->calendar->setMaximumDate(QDate(3000, 1, 1));
    ui->calendar->setGridVisible(true);

    ui->tletreewidget->setColumnCount(1);
    ui->tletreewidget->setHeaderLabels( QStringList() << tr("TLE Files") );

    ui->satlisttreewidget->setColumnCount(10);
    ui->satlisttreewidget->setHeaderLabels( QStringList() << tr("name") << tr("Catnbr" ) << tr("Days Old")
                                  << tr("longitude" ) << tr("latitude" ) << tr("altitude[km]" )
                                  << tr( "azimuth" ) << tr( "elevation" ) << tr( "range[km]" )
                                  << tr( "rate[km/s]" ));
    ui->satlisttreewidget->setColumnWidth( 0, 100 );

    ui->segmentoverview->setColumnCount(3);
    ui->segmentoverview->setHeaderLabels( QStringList() << tr("Directory") << tr("Segment type") << tr("Nbr.in dir."));
    ui->segmentoverview->setColumnWidth( 0, 100 );

    ui->segmentdirectorywidget->clear();
    ui->segmentdirectorywidget->setEnabled(true);
    ui->segmentdirectorywidget->setColumnCount(1);
    ui->segmentdirectorywidget->setRootIsDecorated(false);
    ui->segmentdirectorywidget->setHeaderLabels(QStringList() << tr("Segment directories"));
    ui->segmentdirectorywidget->header()->setStretchLastSection(true);
    ui->segmentdirectorywidget->setMinimumHeight(100);

    ui->selectedsegmentwidget->clear();
    ui->selectedsegmentwidget->setEnabled(true);
    ui->selectedsegmentwidget->setColumnCount(2);
    ui->selectedsegmentwidget->setRootIsDecorated(false);
    ui->selectedsegmentwidget->setHeaderLabels(QStringList() << tr("Selected Segments") << tr("#Lines"));
    ui->selectedsegmentwidget->header()->setStretchLastSection(true);
    ui->selectedsegmentwidget->setColumnWidth(0, 300);

/*    QDate now =QDate::currentDate();
    ui->calendar->setSelectedDate(now);

    QTextCharFormat format;
    format.setBackground(QBrush(Qt::blue, Qt::SolidPattern));
    ui->calendar->setDateTextFormat(ui->calendar->selectedDate(),format);
*/
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(45454, QUdpSocket::ShareAddress);

    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));

    connect(ui->satlisttreewidget,SIGNAL(itemClicked(QTreeWidgetItem*, int)), this,SLOT(itemSelectedtreewidget(QTreeWidgetItem*)));
    connect(ui->segmentdirectorywidget,SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(itemSelectedsegmentdirectory(QTreeWidgetItem*)));

    connect( segs, SIGNAL(signalProgress(int)), this, SLOT(setProgressBar(int)));
    connect( segs, SIGNAL(signalResetProgressbar(int, QString)), this, SLOT(resetProgressBar(int,QString)));

    connect(ui->realminutesslider, SIGNAL(sliderReleased()), this, SLOT(setRealMinutesShownValue()));
    connect(ui->realminutesspinbox, SIGNAL(valueChanged(int)), this,SLOT(setRealMinutesShownValue()));
    //connect(ui->calendar, SIGNAL(selectionChanged()), this, SLOT(getSegmentsForCalendar()));
    //connect(ui->btnReload, SIGNAL(clicked()), this, SLOT(getSegmentsForCalendar()));
    connect(ui->sliNbrOfHours, SIGNAL(sliderReleased()), this, SLOT(setNbrOfHours()));
    connect(ui->spbNbrOfHours, SIGNAL(valueChanged(int)), this,SLOT(setNbrOfHours()));
    connect(ui->segmentsslider, SIGNAL(sliderReleased()), this, SLOT(setSegmentsShownValue()));
    connect(ui->segmentsspinbox, SIGNAL(valueChanged(int)), this,SLOT(setSegmentsShownValue()));

    showAvailSat();
    showSegmentDirectoryList();

}

void FormEphem::setProgressBar(int progress)
{
    ui->progressBar->setValue(progress);
}

void FormEphem::resetProgressBar(int maxprogress, const QString &mytext)
{
    ui->lblProgress->setText(mytext);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(maxprogress);
    ui->progressBar->setValue(0);
}

FormEphem::~FormEphem()
{
    opts.ephemsplittersizes = ui->splitter->saveState();
    qDebug() << "closing FormEphem";

}

void FormEphem::on_btnAdd_clicked()
{
    QString fn = QFileDialog::getOpenFileName( this,
                      tr("Select the tle file"),
                      ".",
                      tr("Text files (*.txt *.tle)"));

    if ( !fn.isEmpty() )
    {
        if(opts.addTleFile(fn))
        {
            sats->ReloadList();
            showAvailSat();
        }
    }

    foreach (const QString &str, opts.tlelist)
    {
        qDebug() << "---> " + str;
    }
}

void FormEphem::on_btnDel_clicked()
{
    QString sel;

    QTreeWidgetItemIterator it1( ui->tletreewidget );

    while (*it1)
    {

        if ( (*it1)->isSelected() )
        {
            if( (*it1)->parent() )
                QMessageBox::information( this, "QtTrack",
            "Only Tle files can be removed !" );
            else
                sel = (*it1)->text( 0 );
            break;

        }
        ++it1;
    }

    opts.deleteTleFile( sel);
    sats->ReloadList();
    showAvailSat();
}

void FormEphem::setSegmentsShownValue()
{
    if (opts.buttonMetop)
        segs->seglmetop->SetNbrOfVisibleSegments(ui->segmentsslider->value());
    else if (opts.buttonNoaa)
        segs->seglnoaa->SetNbrOfVisibleSegments(ui->segmentsslider->value());
    else if (opts.buttonHRP)
        segs->seglhrp->SetNbrOfVisibleSegments(ui->segmentsslider->value());
    else if (opts.buttonGAC)
        segs->seglgac->SetNbrOfVisibleSegments(ui->segmentsslider->value());
    else if (opts.buttonVIIRSM)
        segs->seglviirsm->SetNbrOfVisibleSegments(ui->segmentsslider->value());
    else if (opts.buttonVIIRSDNB)
        segs->seglviirsdnb->SetNbrOfVisibleSegments(ui->segmentsslider->value());
    else if (opts.buttonOLCIefr)
        segs->seglolciefr->SetNbrOfVisibleSegments(ui->segmentsslider->value());
    else if (opts.buttonOLCIerr)
        segs->seglolcierr->SetNbrOfVisibleSegments(ui->segmentsslider->value());
    else if (opts.buttonSLSTR)
        segs->seglslstr->SetNbrOfVisibleSegments(ui->segmentsslider->value());
    else if (opts.buttonDatahubOLCIefr)
        segs->segldatahubolciefr->SetNbrOfVisibleSegments(ui->segmentsslider->value());
    else if (opts.buttonDatahubOLCIerr)
        segs->segldatahubolcierr->SetNbrOfVisibleSegments(ui->segmentsslider->value());
    else if (opts.buttonDatahubSLSTR)
        segs->segldatahubslstr->SetNbrOfVisibleSegments(ui->segmentsslider->value());

    opts.nbrofvisiblesegments = ui->segmentsslider->value();

    //qDebug() << QString("setMinutesShownValue value = %1").arg(segs->segmentlistmetop->GetNbrOfVisibleSegments());
}

void FormEphem::getSegmentsForCalendar()
{

    segs->ReadDirectories(ui->calendar->selectedDate(), ui->sliNbrOfHours->value());

    ui->segmentoverview->clear();

    NewSegmentOverviewItem();

    emit signalDirectoriesRead();
}

void FormEphem::NewSegmentOverviewItem()
{
    QTreeWidgetItem *newitem;

    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsMetop(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsNoaa(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsHRP(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsGAC(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsMetopAhrpt(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsMetopBhrpt(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsNoaa19hrpt(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsM01hrpt(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsM02hrpt(), 0  );


    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsVIIRSM(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsVIIRSDNB(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsOLCIefr(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsOLCIerr(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsSLSTR(), 0  );

    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsDatahubOLCIefr(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsDatahubOLCIerr(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsDatahubSLSTR(), 0  );

    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsMeteosat(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsMeteosatRss(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsMeteosat8(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsGOES13(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsGOES15(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsGOES16(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsFY2E(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsFY2G(), 0  );
    newitem = new QTreeWidgetItem( ui->segmentoverview, segs->GetOverviewSegmentsH8(), 0  );

}

void FormEphem::showSegmentsAdded()
{
    ui->segmentoverview->clear();
    NewSegmentOverviewItem();
}

void FormEphem::setNbrOfHours()
{

    opts.nbrofhours = ui->sliNbrOfHours->value();
}

void FormEphem::showAvailSat()
{
    QString lleft;
    QStringList catout;

    ui->tletreewidget->clear();
    ui->satlisttreewidget->clear();

    ui->tletreewidget->setColumnCount(1);

    QList<QTreeWidgetItem *> tleItemList;
    QStringList::Iterator its = opts.tlelist.begin();

    QStringList outtle;

    while( its != opts.tlelist.end() )
    {
        QFile file(*its);
        if(file.exists())
            outtle.append(*its);
        ++its;
    }
    opts.tlelist = outtle;

    its = opts.tlelist.begin();
    while( its != opts.tlelist.end() )
    {
        tleItemList.append( item(*its) );
        qDebug() << "showavailsat : " << *its;
        ++its;
    }

    QTreeWidgetItem *titem = 0;
    unsigned int num = 1;
    QString line, line1, line2;

    bool ok;

    // go through the list of parent items...

    for ( QList<QTreeWidgetItem*>::Iterator it = tleItemList.begin(); it != tleItemList.end();
          ++it, num++ )
    {
        titem = *it;
        QFile file( titem->text(0) );
        //qDebug() << titem->text(0);

        if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
            QTextStream stream( &file );
            while ( !stream.atEnd() )
            {
                line = stream.readLine(); // line of text excluding '\n'
                lleft = line.left(1);
                if ((lleft!=QString("1")) && (lleft!=QString("2")))
                {
                    line1 = stream.readLine();
                    lleft = line1.left(1);
                    if (lleft!=QString("1")) break;
                    //qDebug() << line1;
                    // checkitem = new QCheckListItem( item, line1.mid(2, 5) + QString(" | ") + line.stripWhiteSpace() , QCheckListItem::CheckBox );
                    QTreeWidgetItem *det = item( line1.mid(2, 5) + QString(" | ") + line.trimmed(), titem );
                    det->setCheckState(0, Qt::Unchecked );
                    for ( QStringList::Iterator itc = opts.catnbrlist.begin(); itc != opts.catnbrlist.end(); ++itc )
                    {
                        if ( (*itc).toInt( &ok, 10) == line1.mid(2, 5).toInt( &ok, 10 ) )
                        {
                            det->setCheckState(0, Qt::Checked);
                            catout << line1.mid(2, 5);
                        }
                    }
                }
            }
            ui->tletreewidget->addTopLevelItem(titem);
            file.close();
        }
        ui->tletreewidget->expandAll();

    }

    opts.catnbrlist = catout;
    showActiveSatellites();

}

void FormEphem::setRealMinutesShownValue()
{
    //qDebug() << "in setRealMinutesShownValue";
    opts.realminutesshown = ui->realminutesslider->value();
}

void FormEphem::itemSelectedsegmentdirectory( QTreeWidgetItem *item)
{

    qDebug() << "void FormEphem::itemSelectedsegmentdirectory";

    QTreeWidgetItemIterator it1( ui->segmentdirectorywidget );

    opts.segmentdirectorylistinc.clear();

    while (*it1)
    {
        if ( (*it1)->checkState(0) == Qt::Checked )
        {
            opts.segmentdirectorylistinc << "1";
            qDebug() << "1";
        }
        else
        {
            opts.segmentdirectorylistinc << "0";
            qDebug() << "0";
        }
        ++it1;
    }

    opts.Save();
}

void FormEphem::showActiveSatellites(void)
{

  QStringList tlelist = sats->GetActiveSatList();
  QStringList::Iterator it2 = tlelist.begin();
  QString str1, str2, str3, str4, str5, str6;
  int nRes, st;
  bool ok;

  QTreeWidgetItem *item;

  ui->satlisttreewidget->clear();

  while( it2 != tlelist.end() )
  {
      qDebug() << (*it2);
    st=0;
    nRes = (*it2).indexOf( ",", st);
    str1 = (*it2).mid(st, nRes-st);
    st=nRes+1;
    nRes = (*it2).indexOf( ",", st);
    str2 = (*it2).mid(st, nRes-st);
    st=nRes+1;
    nRes = (*it2).indexOf( ",", st);
    str3 = (*it2).mid(st, nRes-st);
    st=nRes+1;
    nRes = (*it2).indexOf( ",", st);
    str4 = (*it2).mid(st, nRes-st);
    st=nRes+1;
    nRes = (*it2).indexOf( ",", st);
    str5 = (*it2).mid(st, nRes-st);
    st=nRes+1;
    str6 = (*it2).mid(st);

    QStringList actsat;
    actsat << str1.toLatin1() << str2.toLatin1() << str3.toLatin1() << str4.toLatin1() << str5.toLatin1() << str6.toLatin1();
    item = new QTreeWidgetItem( ui->satlisttreewidget, actsat, 0  );

    if(str2.toInt(&ok, 10) == sats->GetSelectedSat())
        item->setSelected(true);
    else
        item->setSelected(false);

    ++it2;
  }
}

void FormEphem::timerDone(void)
{
    updateSatelliteEphem();
}

void FormEphem::updateSatelliteEphem(void)
{
  double lon, lat, alt;
  double azimuth, elevatie, range, rate;
  QString strlon, strlat, stralt;
  QString straz, strel, strrange, strrate;
  QTreeWidgetItemIterator it2( ui->satlisttreewidget );

    while (*it2)
    {
        sats->GetSatelliteEphem(((*it2)->text(1)).toInt(), &lon, &lat, &alt, &azimuth, &elevatie, &range, &rate);
        (*it2)->setText(3, strlon.setNum(lon, 'f', 1));
        (*it2)->setText(4, strlat.setNum(lat, 'f', 1));
        (*it2)->setText(5, stralt.setNum(alt, 'f', 1));

        (*it2)->setText(6, straz.setNum(azimuth, 'f', 1));
        (*it2)->setText(7, strel.setNum(elevatie, 'f', 1));
        (*it2)->setText(8, strrange.setNum(range, 'f', 1));
        (*it2)->setText(9, strrate.setNum(rate, 'f', 1));

        if ((*it2)->text(1).toInt()==sats->GetSelectedSat())
            (*it2)->setSelected(true);
        else
            (*it2)->setSelected(false);
        ++it2;
    }

}

void FormEphem::on_tletreewidget_itemChanged(QTreeWidgetItem *item, int column)
{
    bool ok;

    ui->satlisttreewidget->clear();
    sats->ClearActive();

    QStringList fcatnbrs;
    QTreeWidgetItemIterator it1( ui->tletreewidget );

    while (*it1)
    {

        if ( (*it1)->checkState(0) == Qt::Checked )
        {
          sats->SetActive(((*it1)->text(0)).mid(0,5).toInt(&ok,10));
          fcatnbrs << ((*it1)->text(0)).mid(0,5);
          qDebug() << ((*it1)->text(0)).mid(0,5);
        }
        ++it1;
    }

    opts.catnbrlist = fcatnbrs;

    qDebug() << "addEntry";

    showActiveSatellites();
}

void FormEphem::itemSelectedtreewidget( QTreeWidgetItem *item )
{
  if ( !item )
        return;

  bool ok;

  qDebug() << "itemselected = " << (*item).text(0) << "  " << (*item).text(1) << item->text(1).toInt(&ok, 10);
  sats->SetSelectedSat( item->text(1).toInt(&ok, 10));

}


void FormEphem::on_btnAddsegmentdir_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(this,
                               tr("Find Files"), QDir::currentPath());

    if (!directory.isEmpty()) {
        qDebug() << QString("directory = %1").arg(directory);
        opts.segmentdirectorylist.append(directory);
        showSegmentDirectoryList();
    }

}

void FormEphem::on_btnDelsegmentdir_clicked()
{
    QString sel;

    QTreeWidgetItemIterator it1( ui->segmentdirectorywidget );

    while (*it1)
    {

        if ( (*it1)->isSelected() )
        {
            sel = (*it1)->text( 0 );
            break;

        }
        ++it1;
    }

    qDebug() << QString("delete segment directory : %1").arg(sel);
    opts.deleteSegmentDirectory( sel);
    showSegmentDirectoryList();

}

void FormEphem::showSegmentDirectoryList(void)
{

    //QList<QTreeWidgetItem *> dirItemList;
    QStringList::Iterator itd = opts.segmentdirectorylist.begin();
    QStringList::Iterator itc = opts.segmentdirectorylistinc.begin();

    ui->segmentdirectorywidget->clear();

    if (opts.segmentdirectorylist.count() != opts.segmentdirectorylistinc.count())
    {
        while( itd != opts.segmentdirectorylist.end() )
        {
            opts.segmentdirectorylistinc << "0";
            ++itd;
        }

    }

    itd = opts.segmentdirectorylist.begin();
    itc = opts.segmentdirectorylistinc.begin();

    while( itd != opts.segmentdirectorylist.end() )
    {
        if (itc != opts.segmentdirectorylistinc.end())
        {
            if (*itc == "1")
                AddRootDirectoryWidgetItem(*itd, Qt::Checked);
            else
                AddRootDirectoryWidgetItem(*itd, Qt::Unchecked);

            ++itc;
        }
        else
        {
            AddRootDirectoryWidgetItem(*itd, Qt::Unchecked);
         }

        ++itd;

    }

    ui->segmentdirectorywidget->expandAll();


}

void FormEphem::AddRootDirectoryWidgetItem(QString segname, Qt::CheckState checkstate)
{
    QTreeWidgetItem *itm = new QTreeWidgetItem(ui->segmentdirectorywidget);
    itm->setFlags(Qt::ItemIsSelectable |
        Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable |
        Qt::ItemIsEnabled);
    itm->setCheckState(0, checkstate);
    itm->setText(0, segname);

}

void FormEphem::showSelectedSegmentList(void)
{


    QList<Segment*> *slmetop = segs->seglmetop->GetSegmentlistptr();
    QList<Segment*> *slnoaa = segs->seglnoaa->GetSegmentlistptr();
    QList<Segment*> *slgac = segs->seglgac->GetSegmentlistptr();
    QList<Segment*> *slhrp = segs->seglhrp->GetSegmentlistptr();
    QList<Segment*> *slmetopAhrpt = segs->seglmetopAhrpt->GetSegmentlistptr();
    QList<Segment*> *slmetopBhrpt = segs->seglmetopBhrpt->GetSegmentlistptr();
    QList<Segment*> *slnoaa19hrpt = segs->seglnoaa19hrpt->GetSegmentlistptr();
    QList<Segment*> *slM01hrpt = segs->seglM01hrpt->GetSegmentlistptr();
    QList<Segment*> *slM02hrpt = segs->seglM02hrpt->GetSegmentlistptr();

    QList<Segment*> *slviirsm = segs->seglviirsm->GetSegmentlistptr();
    QList<Segment*> *slviirsdnb = segs->seglviirsdnb->GetSegmentlistptr();
    QList<Segment*> *slolciefr = segs->seglolciefr->GetSegmentlistptr();
    QList<Segment*> *slolcierr = segs->seglolcierr->GetSegmentlistptr();
    QList<Segment*> *slslstr = segs->seglslstr->GetSegmentlistptr();
    QList<QTreeWidgetItem *> items;

    ui->selectedsegmentwidget->clear();

    if (opts.buttonMetop)
    {
        QList<Segment*>::iterator segitmetop = slmetop->begin();
        while ( segitmetop != slmetop->end() )
        {
            if((*segitmetop)->IsSelected())
            {
                QStringList nl;
                nl << (*segitmetop)->fileInfo.fileName() << QString("%1").arg((*segitmetop)->GetNbrOfLines());
                items.append(new QTreeWidgetItem( (QTreeWidget*)0 , nl));
                ui->selectedsegmentwidget->setHeaderLabel((*segitmetop)->fileInfo.absolutePath());
            }
            ++segitmetop;
        }
    }
    else
    if (opts.buttonNoaa)
    {
        QList<Segment*>::iterator segitnoaa = slnoaa->begin();
        while ( segitnoaa != slnoaa->end() )
        {
            if((*segitnoaa)->IsSelected())
            {
                QStringList nl;
                nl << (*segitnoaa)->fileInfo.fileName() << QString("%1").arg((*segitnoaa)->GetNbrOfLines());

                items.append(new QTreeWidgetItem( (QTreeWidget*)0 , nl));
                ui->selectedsegmentwidget->setHeaderLabel((*segitnoaa)->fileInfo.absolutePath());

            }
            ++segitnoaa;
        }
    }
    else
    if (opts.buttonGAC)
    {
        QList<Segment*>::iterator segitgac = slgac->begin();
        while ( segitgac != slgac->end() )
        {
            if((*segitgac)->IsSelected())
            {
                QStringList nl;
                nl << (*segitgac)->fileInfo.fileName() << QString("%1").arg((*segitgac)->GetNbrOfLines());

                items.append(new QTreeWidgetItem( (QTreeWidget*)0 , nl));
                ui->selectedsegmentwidget->setHeaderLabel((*segitgac)->fileInfo.absolutePath());

            }
            ++segitgac;
        }
    }
    else
    if (opts.buttonHRP)
    {
        QList<Segment*>::iterator segithrp = slhrp->begin();
        while ( segithrp != slhrp->end() )
        {
            if((*segithrp)->IsSelected())
            {
                QStringList nl;
                nl << (*segithrp)->fileInfo.fileName() << QString("%1").arg((*segithrp)->GetNbrOfLines());

                items.append(new QTreeWidgetItem( (QTreeWidget*)0 , nl));
                ui->selectedsegmentwidget->setHeaderLabel((*segithrp)->fileInfo.absolutePath());
            }
            ++segithrp;
        }

    }
    else
    if (opts.buttonMetopAhrpt)
    {
        QList<Segment*>::iterator segit = slmetopAhrpt->begin();
        while ( segit != slmetopAhrpt->end() )
        {
            if((*segit)->IsSelected())
            {
                QStringList nl;
                nl << (*segit)->fileInfo.fileName() << QString("%1").arg((*segit)->GetNbrOfLines());

                items.append(new QTreeWidgetItem( (QTreeWidget*)0 , nl));
                ui->selectedsegmentwidget->setHeaderLabel((*segit)->fileInfo.absolutePath());
            }
            ++segit;
        }

    }
    else
    if (opts.buttonMetopBhrpt)
    {
        QList<Segment*>::iterator segit = slmetopBhrpt->begin();
        while ( segit != slmetopBhrpt->end() )
        {
            if((*segit)->IsSelected())
            {
                QStringList nl;
                nl << (*segit)->fileInfo.fileName() << QString("%1").arg((*segit)->GetNbrOfLines());

                items.append(new QTreeWidgetItem( (QTreeWidget*)0 , nl));
                ui->selectedsegmentwidget->setHeaderLabel((*segit)->fileInfo.absolutePath());
            }
            ++segit;
        }

    }
    else
    if (opts.buttonNoaa19hrpt)
    {
        QList<Segment*>::iterator segit = slnoaa19hrpt->begin();
        while ( segit != slnoaa19hrpt->end() )
        {
            if((*segit)->IsSelected())
            {
                QStringList nl;
                nl << (*segit)->fileInfo.fileName() << QString("%1").arg((*segit)->GetNbrOfLines());

                items.append(new QTreeWidgetItem( (QTreeWidget*)0 , nl));
                ui->selectedsegmentwidget->setHeaderLabel((*segit)->fileInfo.absolutePath());
            }
            ++segit;
        }

    }
    else
    if (opts.buttonM01hrpt)
    {
        QList<Segment*>::iterator segit = slM01hrpt->begin();
        while ( segit != slM01hrpt->end() )
        {
            if((*segit)->IsSelected())
            {
                QStringList nl;
                nl << (*segit)->fileInfo.fileName() << QString("%1").arg((*segit)->GetNbrOfLines());

                items.append(new QTreeWidgetItem( (QTreeWidget*)0 , nl));
                ui->selectedsegmentwidget->setHeaderLabel((*segit)->fileInfo.absolutePath());
            }
            ++segit;
        }

    }
    else
    if (opts.buttonM02hrpt)
    {
        QList<Segment*>::iterator segit = slM02hrpt->begin();
        while ( segit != slM02hrpt->end() )
        {
            if((*segit)->IsSelected())
            {
                QStringList nl;
                nl << (*segit)->fileInfo.fileName() << QString("%1").arg((*segit)->GetNbrOfLines());

                items.append(new QTreeWidgetItem( (QTreeWidget*)0 , nl));
                ui->selectedsegmentwidget->setHeaderLabel((*segit)->fileInfo.absolutePath());
            }
            ++segit;
        }

    }
    else
    if (opts.buttonVIIRSM)
    {
        QList<Segment*>::iterator segitviirsm = slviirsm->begin();
        while ( segitviirsm != slviirsm->end() )
        {
            if((*segitviirsm)->IsSelected())
            {
                QStringList nl;
                nl << (*segitviirsm)->fileInfo.fileName() << QString("%1").arg((*segitviirsm)->GetNbrOfLines());

                items.append(new QTreeWidgetItem( (QTreeWidget*)0 , nl));
                ui->selectedsegmentwidget->setHeaderLabel((*segitviirsm)->fileInfo.absolutePath());
            }
            ++segitviirsm;
        }

    }
    else
    if (opts.buttonVIIRSDNB)
    {
        QList<Segment*>::iterator segitviirsdnb = slviirsdnb->begin();
        while ( segitviirsdnb != slviirsdnb->end() )
        {
            if((*segitviirsdnb)->IsSelected())
            {
                QStringList nl;
                nl << (*segitviirsdnb)->fileInfo.fileName() << QString("%1").arg((*segitviirsdnb)->GetNbrOfLines());

                items.append(new QTreeWidgetItem( (QTreeWidget*)0 , nl));
                ui->selectedsegmentwidget->setHeaderLabel((*segitviirsdnb)->fileInfo.absolutePath());
            }
            ++segitviirsdnb;
        }

    }
    else
    if (opts.buttonOLCIefr)
    {
        QList<Segment*>::iterator segitolciefr = slolciefr->begin();
        while ( segitolciefr != slolciefr->end() )
        {
            if((*segitolciefr)->IsSelected())
            {
                QStringList nl;
                nl << (*segitolciefr)->fileInfo.fileName() << QString("%1").arg((*segitolciefr)->GetNbrOfLines());

                items.append(new QTreeWidgetItem( (QTreeWidget*)0 , nl));
                ui->selectedsegmentwidget->setHeaderLabel((*segitolciefr)->fileInfo.absolutePath());
            }
            ++segitolciefr;
        }

    }
    else
    if (opts.buttonOLCIerr)
    {
        QList<Segment*>::iterator segitolcierr = slolcierr->begin();
        while ( segitolcierr != slolcierr->end() )
        {
            if((*segitolcierr)->IsSelected())
            {
                QStringList nl;
                nl << (*segitolcierr)->fileInfo.fileName() << QString("%1").arg((*segitolcierr)->GetNbrOfLines());

                items.append(new QTreeWidgetItem( (QTreeWidget*)0 , nl));
                ui->selectedsegmentwidget->setHeaderLabel((*segitolcierr)->fileInfo.absolutePath());
            }
            ++segitolcierr;
        }

    }
    else
    if (opts.buttonSLSTR)
    {
        QList<Segment*>::iterator segitslstr = slslstr->begin();
        while ( segitslstr != slslstr->end() )
        {
            if((*segitslstr)->IsSelected())
            {
                QStringList nl;
                nl << (*segitslstr)->fileInfo.fileName() << QString("%1").arg((*segitslstr)->GetNbrOfLines());

                items.append(new QTreeWidgetItem( (QTreeWidget*)0 , nl));
                ui->selectedsegmentwidget->setHeaderLabel((*segitslstr)->fileInfo.absolutePath());
            }
            ++segitslstr;
        }

    }


    ui->selectedsegmentwidget->insertTopLevelItems(0, items);
}

void FormEphem::on_btnUpdateTLE_clicked()
{
    downloadmanager.clearqueue();

    ui->edtUpdateTLE->clear();

    ui->edtUpdateTLE->setPlainText("Start download \n");
    downloadmanager.append(opts.tlesources);
    QObject::connect(&downloadmanager, SIGNAL(finished(QString)), this, SLOT(tlefilesread(QString)));
    //QObject::connect(&downloadmanager, SIGNAL(progress(QString)), this, SLOT(showprogress(QString)));
    QObject::connect(&downloadmanager, SIGNAL(startnext(QString)), this, SLOT(showprogress(QString)));

}

void FormEphem:: tlefilesread(QString str)
{

    QObject::disconnect(&downloadmanager, SIGNAL(finished(QString)), this, SLOT(tlefilesread(QString)));
    QObject::disconnect(&downloadmanager, SIGNAL(startnext(QString)), this, SLOT(showprogress(QString)));


    ui->edtUpdateTLE->setPlainText(ui->edtUpdateTLE->toPlainText() + "\n" + str);
    sats->ReloadList();
    showAvailSat();

}

void FormEphem:: showprogress(QString str)
{
    ui->edtUpdateTLE->setPlainText(ui->edtUpdateTLE->toPlainText() + str);
}

void FormEphem::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());
        if (opts.udpmessages)
           emit signalDatagram(datagram.data());
    }
}


void FormEphem::on_btnReload_clicked()
{
    getSegmentsForCalendar();
}

void FormEphem::on_calendar_selectionChanged()
{
    segs->setXMLDate(ui->calendar->selectedDate());
    getSegmentsForCalendar();
}

void FormEphem::on_btnDownloadFromDatahub_clicked()
{
    segs->setXMLDate(ui->calendar->selectedDate());
    segs->LoadXMLfromDatahub();
}
