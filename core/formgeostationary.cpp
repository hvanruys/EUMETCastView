#include "formgeostationary.h"
#include "ui_formgeostationary.h"
#include <MSG_HRIT.h>
#include "pixgeoconversion.h"

#include <QByteArray>

extern SegmentImage *imageptrs;
extern Options opts;

FormGeostationary::FormGeostationary(QWidget *parent, SatelliteList *satlist, AVHRRSatellite *seglist) :
    QWidget(parent),
    ui(new Ui::FormGeostationary)
{
    ui->setupUi(this);
    segs = seglist;
    sats = satlist;

    ui->tabGeostationary->setCurrentIndex(0);
    ui->tabGeostationary->setTabText(0, "MET-10 : " + opts.geostationarylistlon.at(0) + "°");
    ui->tabGeostationary->setTabText(1, "MET-9 : " + opts.geostationarylistlon.at(1) + "°");
    ui->tabGeostationary->setTabText(2, "MET-7 : " + opts.geostationarylistlon.at(2) + "°");
    ui->tabGeostationary->setTabText(3, "FY2E : " + opts.geostationarylistlon.at(3) + "°");
    ui->tabGeostationary->setTabText(4, "FY2G : " + opts.geostationarylistlon.at(4) + "°");
    ui->tabGeostationary->setTabText(5, "GOES-13 (DC-3) : " + opts.geostationarylistlon.at(5) + "°");
    ui->tabGeostationary->setTabText(6, "GOES-13 (DC-4) : " + opts.geostationarylistlon.at(5) + "°");
    ui->tabGeostationary->setTabText(7, "GOES-15 (DC-3) : " + opts.geostationarylistlon.at(6) + "°");
    ui->tabGeostationary->setTabText(8, "GOES-15 (DC-4) : " + opts.geostationarylistlon.at(6) + "°");
    ui->tabGeostationary->setTabText(9, "MTSAT2 (DC-3) : " + opts.geostationarylistlon.at(7) + "°");
    ui->tabGeostationary->setTabText(10, "MTSAT2 (DC-4) : " + opts.geostationarylistlon.at(7) + "°");

    ui->SegmenttreeWidget->setRootIsDecorated(false);
    ui->SegmenttreeWidget->header()->setStretchLastSection(true);
    //for(int i = 2; i < 14; i++)
    //    ui->SegmenttreeWidget->header()->setColumnWidth(i, 200);
    for(int i = 2; i < 14; i++)
        ui->SegmenttreeWidget->setColumnWidth(i, 200);

    ui->SegmenttreeWidget->setColumnCount(14);
    ui->SegmenttreeWidget->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "HRV" << "0.6" << "0.8" << "1.6" << "3.9" << "6.2" << "7.3" << "8.7" << "9.7" << "10.8" << "12.8" << "13.4" );
    ui->SegmenttreeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidget->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidget->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidget->header()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidget->header()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidget->header()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidget->header()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidget->header()->setSectionResizeMode(9, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidget->header()->setSectionResizeMode(10, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidget->header()->setSectionResizeMode(11, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidget->header()->setSectionResizeMode(12, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidget->header()->setSectionResizeMode(13, QHeaderView::ResizeToContents);

    //ui->lblMeteosatlist->setText(QString("Satellite %1  Longitude %2").arg(opts.msgcurrentname).arg(opts.msgcurrentlon));

    ui->SegmenttreeWidgetRSS->setRootIsDecorated(false);
    ui->SegmenttreeWidgetRSS->header()->setStretchLastSection(true);
    for(int i = 0; i < 14; i++)
        ui->SegmenttreeWidgetRSS->setColumnWidth(i, 200);

    ui->SegmenttreeWidgetRSS->setColumnCount(14);
    ui->SegmenttreeWidgetRSS->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "HRV" << "0.6" << "0.8" << "1.6" << "3.9" << "6.2" << "7.3" << "8.7" << "9.7" << "10.8" << "12.8" << "13.4" );
    ui->SegmenttreeWidgetRSS->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetRSS->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetRSS->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetRSS->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetRSS->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetRSS->header()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetRSS->header()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetRSS->header()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetRSS->header()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetRSS->header()->setSectionResizeMode(9, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetRSS->header()->setSectionResizeMode(10, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetRSS->header()->setSectionResizeMode(11, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetRSS->header()->setSectionResizeMode(12, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetRSS->header()->setSectionResizeMode(13, QHeaderView::ResizeToContents);

    //ui->lblMeteosatlistRSS->setText(QString("RSS Satellite %1  Longitude %2").arg(opts.msgrssname).arg(opts.msgrsslon));

    ui->SegmenttreeWidgetMet7->setRootIsDecorated(false);
    ui->SegmenttreeWidgetMet7->header()->setStretchLastSection(true);
    for(int i = 0; i < 3; i++)
        ui->SegmenttreeWidgetMet7->setColumnWidth(i, 200);

    ui->SegmenttreeWidgetMet7->setColumnCount(3);
    ui->SegmenttreeWidgetMet7->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "0.7" << "6.4" << "11.5" );
    ui->SegmenttreeWidgetMet7->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetMet7->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetMet7->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    ui->SegmenttreeWidgetGOES13dc3->setRootIsDecorated(false);
    ui->SegmenttreeWidgetGOES13dc3->header()->setStretchLastSection(true);
    for(int i = 0; i < 4; i++)
        ui->SegmenttreeWidgetGOES13dc3->setColumnWidth(i, 200);

    ui->SegmenttreeWidgetGOES13dc3->setColumnCount(4);
    ui->SegmenttreeWidgetGOES13dc3->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "0.7" << "3.9" << "6.6" << "10.7" );
    ui->SegmenttreeWidgetGOES13dc3->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetGOES13dc3->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetGOES13dc3->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetGOES13dc3->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    ui->SegmenttreeWidgetGOES13dc4->setRootIsDecorated(false);
    ui->SegmenttreeWidgetGOES13dc4->header()->setStretchLastSection(true);
    for(int i = 0; i < 4; i++)
        ui->SegmenttreeWidgetGOES13dc4->setColumnWidth(i, 200);

    ui->SegmenttreeWidgetGOES13dc4->setColumnCount(4);
    ui->SegmenttreeWidgetGOES13dc4->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "0.7" << "3.9" << "6.6" << "10.7" );
    ui->SegmenttreeWidgetGOES13dc4->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetGOES13dc4->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetGOES13dc4->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetGOES13dc4->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);


    ui->SegmenttreeWidgetGOES15dc3->setRootIsDecorated(false);
    ui->SegmenttreeWidgetGOES15dc3->header()->setStretchLastSection(true);
    for(int i = 0; i < 4; i++)
        ui->SegmenttreeWidgetGOES15dc3->setColumnWidth(i, 200);

    ui->SegmenttreeWidgetGOES15dc3->setColumnCount(4);
    ui->SegmenttreeWidgetGOES15dc3->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "0.7" << "3.9" << "6.9" << "10.7" );
    ui->SegmenttreeWidgetGOES15dc3->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetGOES15dc3->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetGOES15dc3->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetGOES15dc3->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    ui->SegmenttreeWidgetGOES15dc4->setRootIsDecorated(false);
    ui->SegmenttreeWidgetGOES15dc4->header()->setStretchLastSection(true);
    for(int i = 0; i < 4; i++)
        ui->SegmenttreeWidgetGOES15dc4->setColumnWidth(i, 200);

    ui->SegmenttreeWidgetGOES15dc4->setColumnCount(4);
    ui->SegmenttreeWidgetGOES15dc4->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "0.7" << "3.9" << "6.9" << "10.7" );
    ui->SegmenttreeWidgetGOES15dc4->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetGOES15dc4->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetGOES15dc4->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetGOES15dc4->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);


    ui->SegmenttreeWidgetMTSATdc3->setRootIsDecorated(false);
    ui->SegmenttreeWidgetMTSATdc3->header()->setStretchLastSection(true);
    for(int i = 0; i < 5; i++)
        ui->SegmenttreeWidgetMTSATdc3->setColumnWidth(i, 200);

    ui->SegmenttreeWidgetMTSATdc3->setColumnCount(5);
    ui->SegmenttreeWidgetMTSATdc3->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "0.7" << "3.8" << "6.8" << "10.8" << "12.0" );
    ui->SegmenttreeWidgetMTSATdc3->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetMTSATdc3->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetMTSATdc3->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetMTSATdc3->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetMTSATdc3->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);


    ui->SegmenttreeWidgetMTSATdc4->setRootIsDecorated(false);
    ui->SegmenttreeWidgetMTSATdc4->header()->setStretchLastSection(true);
    for(int i = 0; i < 5; i++)
        ui->SegmenttreeWidgetMTSATdc4->setColumnWidth(i, 200);

    ui->SegmenttreeWidgetMTSATdc4->setColumnCount(5);
    ui->SegmenttreeWidgetMTSATdc4->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "0.7" << "3.8" << "6.8" << "10.8" << "12.0" );
    ui->SegmenttreeWidgetMTSATdc4->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetMTSATdc4->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetMTSATdc4->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetMTSATdc4->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetMTSATdc4->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);


    ui->SegmenttreeWidgetFY2E->setRootIsDecorated(false);
    ui->SegmenttreeWidgetFY2E->header()->setStretchLastSection(true);
    for(int i = 0; i < 6; i++)
        ui->SegmenttreeWidgetFY2E->setColumnWidth(i, 200);

    ui->SegmenttreeWidgetFY2E->setColumnCount(6);
    ui->SegmenttreeWidgetFY2E->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "IR1" << "IR2" << "IR3" << "IR4" << "VIS" << "VIS1KM" );
    ui->SegmenttreeWidgetFY2E->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetFY2E->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetFY2E->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetFY2E->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetFY2E->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetFY2E->header()->setSectionResizeMode(5, QHeaderView::ResizeToContents);

    ui->SegmenttreeWidgetFY2G->setRootIsDecorated(false);
    ui->SegmenttreeWidgetFY2G->header()->setStretchLastSection(true);
    for(int i = 0; i < 6; i++)
        ui->SegmenttreeWidgetFY2G->setColumnWidth(i, 200);

    ui->SegmenttreeWidgetFY2G->setColumnCount(6);
    ui->SegmenttreeWidgetFY2G->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "IR1" << "IR2" << "IR3" << "IR4" << "VIS" << "VIS1KM" );
    ui->SegmenttreeWidgetFY2G->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetFY2G->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetFY2G->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetFY2G->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetFY2G->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->SegmenttreeWidgetFY2G->header()->setSectionResizeMode(5, QHeaderView::ResizeToContents);

}

QStringList FormGeostationary::getGeostationarySegments(SegmentListGeostationary::eGeoSatellite whichgeo, const QString imagetype, const QString filepath, QVector<QString> spectrumvector, QString filepattern)
{

    qDebug() << QString("getGeostationarySegments type = %1  Filepath = %2 filepattern = %3").arg(imagetype).arg(filepath).arg(filepattern);
    qDebug() << QString("getGeostationarySegments spectrumvector %1 %2 %3").arg(spectrumvector.at(0)).arg(spectrumvector.at(1)).arg(spectrumvector.at(2));

    QDir meteosatdir(filepath);
    meteosatdir.setFilter(QDir::Files | QDir::NoSymLinks);
    meteosatdir.setSorting(QDir::Name);

    QStringList strlist = meteosatdir.entryList();
    QStringList strlistout;


    QStringList::Iterator itc = strlist.begin();

    if(imagetype == "HRV" )
    {
        while( itc != strlist.end() )
        {
            QString st = *itc;
            if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
            {
                if(meteosatdir.match(filepattern, *itc) && st.mid(40, 6) == "VIS1KM")
                    strlistout.append(*itc);
            }
            else
            {
                if(meteosatdir.match(filepattern, *itc) && st.mid(26, 6) == "HRV___")
                    strlistout.append(*itc);
            }
            itc++;
        }
    }
    else
        if(imagetype == "VIS_IR")
        {
            while( itc != strlist.end() )
            {
                QString st = *itc;
                //qDebug() << "getGeostationarySegments in " << st;
                if(meteosatdir.match(filepattern, *itc))
                    qDebug() << "meteosatdir.match => " << st;

                if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
                {
                    if(meteosatdir.match(filepattern, *itc) && (st.mid(40, 3) == spectrumvector.at(0) || st.mid(40, 3) == spectrumvector.at(1) || st.mid(40, 3) == spectrumvector.at(2)))
                        strlistout.append(*itc);
                } else
                {
                    if(meteosatdir.match(filepattern, *itc) && (st.mid(26, 6) == spectrumvector.at(0) || st.mid(26, 6) == spectrumvector.at(1) || st.mid(26, 6) == spectrumvector.at(2)))
                        strlistout.append(*itc);
                }
                itc++;
            }
        }

    //L-???-??????-GOES13______-?????????-0?????___-201404181000-C_

    for (int j = 0; j < strlistout.size(); ++j)
    {
        qDebug() << QString("getMeteosatSegment out ======= %1  %2    %3").arg(imagetype).arg(j).arg(strlistout.at(j));
    }

    return strlistout;
}

QStringList FormGeostationary::getGeostationarySegmentsFengYun(SegmentListGeostationary::eGeoSatellite whichgeo, const QString imagetype, const QString filepath, QVector<QString> spectrumvector, QString filepattern)
{

    qDebug() << QString("getGeostationarySegmentsFengYun type = %1  Filepath = %2 filepattern = %3").arg(imagetype).arg(filepath).arg(filepattern);
    qDebug() << QString("getGeostationarySegmentsFengYun spectrumvector %1 %2 %3").arg(spectrumvector.at(0)).arg(spectrumvector.at(1)).arg(spectrumvector.at(2));

    QDir fydir(filepath);
    fydir.setFilter(QDir::Files | QDir::NoSymLinks);
    fydir.setSorting(QDir::Name);

    QStringList strlist = fydir.entryList();
    QStringList strlistout;


    QStringList::Iterator itc = strlist.begin();

    if(imagetype == "HRV" )
    {
        while( itc != strlist.end() )
        {
            QString st = *itc;
            if(fydir.match(filepattern, *itc) && st.mid(40, 6) == "VIS1KM")
                    strlistout.append(*itc);
            itc++;
        }
    }
    else
        if(imagetype == "VIS_IR")
        {
            for( int j = 0; j < spectrumvector.size(); j++)
            {
                while( itc != strlist.end() )
                {
                    QString st = *itc;

                    if(fydir.match(filepattern, *itc) && (st.mid(40, 3) == spectrumvector.at(j)))
                        strlistout.append(*itc);
                    itc++;
                }
                itc = strlist.begin();
            }
        }

    //L-???-??????-GOES13______-?????????-0?????___-201404181000-C_

    for (int j = 0; j < strlistout.size(); ++j)
    {
        qDebug() << QString("getMeteosatSegment out ======= %1  %2    %3").arg(imagetype).arg(j).arg(strlistout.at(j));
    }

    return strlistout;
}

QStringList FormGeostationary::globit(const QString filepath, const QString filepattern)
{

    QDir meteosatdir(filepath);
    meteosatdir.setFilter(QDir::Files | QDir::NoSymLinks);
    //meteosatdir.setSorting(QDir::Name);


    QStringList strlist = meteosatdir.entryList();
    QStringList strlistout;
    QStringList ret;

    QStringList::Iterator itc = strlist.begin();

    while( itc != strlist.end() )
    {
        if(meteosatdir.match(filepattern, *itc))
            strlistout.append(*itc);
        itc++;
    }

    return strlistout;

}

int FormGeostationary::wildcmp(const char *wild, const char *string)
{
  const char *cp = NULL, *mp = NULL;

  while ((*string) && (*wild != '*')) {
    if ((*wild != *string) && (*wild != '?')) {
      return 0;
    }
    wild++;
    string++;
  }

  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return 1;
      }
      mp = wild;
      cp = string+1;
    } else if ((*wild == *string) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }

  while (*wild == '*') {
    wild++;
  }
  return !*wild;
}

void FormGeostationary::PopulateTreeGeo(SegmentListGeostationary::eGeoSatellite whichgeo, QMap<QString, QMap<QString, QMap< int, QFileInfo > > > map, QTreeWidget *widget)
{
    QStringList strlist;
    QString strnbrlist;
    QString strspectrumlist;
    QList<QTreeWidgetItem *> items;

    QTreeWidgetItem *newitem;
    QString strdate;
    QString strspectrum;
    QString filenbr;
    QColor col;
    int cnt_hrv = 0;
    int cnt_ir016 = 0;
    int cnt_ir039 = 0;
    int cnt_ir087 = 0;
    int cnt_ir097 = 0;
    int cnt_ir108 = 0;
    int cnt_ir120 = 0;
    int cnt_ir134 = 0;
    int cnt_vis006 = 0;
    int cnt_vis008 = 0;
    int cnt_wv062 = 0;
    int cnt_wv073 = 0;
    int cnt_IR1 = 0;
    int cnt_IR2 = 0;
    int cnt_IR3 = 0;
    int cnt_IR4 = 0;
    int cnt_VIS = 0;
    int cnt_VIS1KM = 0;

    widget->clear();

    QMap<QString, QMap<QString, QMap< int, QFileInfo > > >::const_iterator citdate = map.constBegin();

     while (citdate != map.constEnd())
    {
        cnt_hrv = 0;
        cnt_ir016 = 0;
        cnt_ir039 = 0;
        cnt_ir087 = 0;
        cnt_ir097 = 0;
        cnt_ir108 = 0;
        cnt_ir120 = 0;
        cnt_ir134 = 0;
        cnt_vis006 = 0;
        cnt_vis008 = 0;
        cnt_wv062 = 0;
        cnt_wv073 = 0;
        cnt_IR1 = 0;
        cnt_IR2 = 0;
        cnt_IR3 = 0;
        cnt_IR4 = 0;
        cnt_VIS = 0;
        cnt_VIS1KM = 0;

        strlist.clear();
        strspectrumlist.clear();
        strdate = citdate.key();
        QMap<QString, QMap< int, QFileInfo > > mapspectrum;
        mapspectrum = map.value(strdate);
        QMap<QString, QMap< int, QFileInfo > >::const_iterator citspectrum = mapspectrum.constBegin();
        while (citspectrum != mapspectrum.constEnd())
        {
            strspectrum = citspectrum.key();
            //MET-10, MET-9
            if (strspectrum == "HRV___")
                strspectrumlist += "H";
            else if (strspectrum == "IR_016")
                strspectrumlist += "I";
            else if (strspectrum == "IR_039")
                strspectrumlist += "I";
            else if (strspectrum == "IR_087")
                strspectrumlist += "I";
            else if (strspectrum == "IR_097")
                strspectrumlist += "I";
            else if (strspectrum == "IR_108")
                strspectrumlist += "I";
            else if (strspectrum == "IR_120")
                strspectrumlist += "I";
            else if (strspectrum == "IR_134")
                strspectrumlist += "I";
            else if (strspectrum == "VIS006")
                strspectrumlist += "V";
            else if (strspectrum == "VIS008")
                strspectrumlist += "V";
            else if (strspectrum == "WV_062")
                strspectrumlist += "W";
            else if (strspectrum == "WV_073")
                strspectrumlist += "W";
            //MET-7
            else if (strspectrum == "00_7_0")
                strspectrumlist += "V";
            else if (strspectrum == "06_4_0")
                strspectrumlist += "I";
            else if (strspectrum == "11_5_0")
                strspectrumlist += "W";
            // Electro
            else if (strspectrum == "00_9_0")
                strspectrumlist += "V";
            else if (strspectrum == "08_0_0")
                strspectrumlist += "I";
            else if (strspectrum == "09_7_0")
                strspectrumlist += "I";
            else if (strspectrum == "10_7_0")
                strspectrumlist += "I";
            // GOES13
            else if (strspectrum == "00_7_0")
                strspectrumlist += "V";
            else if (strspectrum == "03_9_0")
                strspectrumlist += "I";
            else if (strspectrum == "06_6_0")
                strspectrumlist += "I";
            else if (strspectrum == "10_7_0")
                strspectrumlist += "I";
            // GOES15
            else if (strspectrum == "00_7_1")
                strspectrumlist += "V";
            else if (strspectrum == "03_9_1")
                strspectrumlist += "I";
            else if (strspectrum == "06_6_1")
                strspectrumlist += "I";
            else if (strspectrum == "10_7_1")
                strspectrumlist += "I";
            // MTSAT
            else if (strspectrum == "00_7_1")
                strspectrumlist += "V";
            else if (strspectrum == "03_8_1")
                strspectrumlist += "I";
            else if (strspectrum == "06_8_1")
                strspectrumlist += "I";
            else if (strspectrum == "10_8_1")
                strspectrumlist += "I";
            else if (strspectrum == "12_0_1")
                strspectrumlist += "I";
            // FengYun
            else if (strspectrum == "IR1")
                strspectrumlist += "I";
            else if (strspectrum == "IR2")
                strspectrumlist += "I";
            else if (strspectrum == "IR3")
                strspectrumlist += "I";
            else if (strspectrum == "IR4")
                strspectrumlist += "I";
            else if (strspectrum == "VIS")
                strspectrumlist += "V";
            else if (strspectrum == "VIS1KM")
                strspectrumlist += "V";


            QMap< int, QFileInfo > mapfile;
            mapfile = mapspectrum.value(strspectrum);
            QMap< int, QFileInfo >::const_iterator citfile = mapfile.constBegin();
            strnbrlist.clear();
            while (citfile != mapfile.constEnd())
            {
                filenbr = citfile.key();
                strnbrlist.append(filenbr);
                // MET-10, MET-9
                if (strspectrum == "HRV___")
                    cnt_hrv++;
                else if (strspectrum == "IR_016")
                    cnt_ir016++;
                else if (strspectrum == "IR_039")
                    cnt_ir039++;
                else if (strspectrum == "IR_087")
                    cnt_ir087++;
                else if (strspectrum == "IR_097")
                    cnt_ir097++;
                else if (strspectrum == "IR_108")
                    cnt_ir108++;
                else if (strspectrum == "IR_120")
                    cnt_ir120++;
                else if (strspectrum == "IR_134")
                    cnt_ir134++;
                else if (strspectrum == "VIS006")
                    cnt_vis006++;
                else if (strspectrum == "VIS008")
                    cnt_vis008++;
                else if (strspectrum == "WV_062")
                    cnt_wv062++;
                else if (strspectrum == "WV_073")
                    cnt_wv073++;
                //MET-7
                else if (strspectrum == "00_7_0")
                    cnt_vis008++;
                else if (strspectrum == "06_4_0")
                    cnt_wv062++;
                else if (strspectrum == "11_5_0")
                    cnt_ir108++;
                // Electro
                else if (strspectrum == "00_9_0")
                    cnt_vis008++;
                else if (strspectrum == "08_0_0")
                    cnt_ir087++;
                else if (strspectrum == "09_7_0")
                    cnt_ir097++;
                else if (strspectrum == "10_7_0")
                    cnt_ir108++;
                // GOES13
                else if (strspectrum == "00_7_0")
                    cnt_vis008++;
                else if (strspectrum == "03_9_0")
                    cnt_ir039++;
                else if (strspectrum == "06_6_0")
                    cnt_ir087++;
                else if (strspectrum == "10_7_0")
                    cnt_ir108++;
                // GOES15
                else if (strspectrum == "00_7_1")
                    cnt_vis008++;
                else if (strspectrum == "03_9_1")
                    cnt_ir039++;
                else if (strspectrum == "06_6_1")
                    cnt_ir087++;
                else if (strspectrum == "10_7_1")
                    cnt_ir108++;
                // MTSAT
                else if (strspectrum == "00_7_1")
                    cnt_vis008++;
                else if (strspectrum == "03_8_1")
                    cnt_ir039++;
                else if (strspectrum == "06_8_1")
                    cnt_ir087++;
                else if (strspectrum == "10_8_1")
                    cnt_ir108++;
                else if (strspectrum == "12_0_1")
                    cnt_ir120++;
                // FengYun
                else if (strspectrum == "IR1")
                    cnt_IR1++;
                else if (strspectrum == "IR2")
                    cnt_IR2++;
                else if (strspectrum == "IR3")
                    cnt_IR3++;
                else if (strspectrum == "IR4")
                    cnt_IR4++;
                else if (strspectrum == "VIS")
                    cnt_VIS++;
                else if (strspectrum == "VIS1KM")
                    cnt_VIS1KM++;




                ++citfile;
            }
            ++citspectrum;

        }

        strlist.clear();
        //strnbrlist = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12").arg(cnt_hrv).arg(cnt_ir016).arg(cnt_ir039).arg(cnt_ir087).arg(cnt_ir097).arg(cnt_ir108).
        //        arg(cnt_ir120).arg(cnt_ir134).arg(cnt_vis006).arg(cnt_vis008).arg(cnt_wv062).arg(cnt_wv073);

        if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 2) << strspectrumlist <<
                   QString("%1").arg(cnt_hrv) << QString("%1").arg(cnt_vis006) << QString("%1").arg(cnt_vis008) <<  QString("%1").arg(cnt_ir016) << QString("%1").arg(cnt_ir039) <<
                   QString("%1").arg(cnt_wv062) << QString("%1").arg(cnt_wv073) << QString("%1").arg(cnt_ir087) << QString("%1").arg(cnt_ir097) <<
                   QString("%1").arg(cnt_ir108) << QString("%1").arg(cnt_ir120) << QString("%1").arg(cnt_ir134);
        }
        else if(whichgeo == SegmentListGeostationary::MET_7)
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 2) << strspectrumlist <<
                   QString("%1").arg(cnt_vis008) << QString("%1").arg(cnt_wv062) << QString("%1").arg(cnt_ir108);
        }
/*        else if(whichgeo == SegmentListGeostationary::ELECTRO_N1)
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 2) << strspectrumlist <<
                   QString("%1").arg(cnt_vis008) << QString("%1").arg(cnt_ir087) << QString("%1").arg(cnt_ir097) << QString("%1").arg(cnt_ir108);
        }
*/
        else if(whichgeo == SegmentListGeostationary::GOES_13 || whichgeo == SegmentListGeostationary::GOES_15)
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 2) << strspectrumlist <<
                   QString("%1").arg(cnt_vis008) << QString("%1").arg(cnt_ir039) << QString("%1").arg(cnt_ir087) << QString("%1").arg(cnt_ir108);
        }
        else if(whichgeo == SegmentListGeostationary::MTSAT)
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 2) << strspectrumlist <<
                   QString("%1").arg(cnt_vis008) << QString("%1").arg(cnt_ir039) << QString("%1").arg(cnt_ir087) << QString("%1").arg(cnt_ir108) << QString("%1").arg(cnt_ir120);
        }
        else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G )
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 2) << strspectrumlist <<
                   QString("%1").arg(cnt_IR1) << QString("%1").arg(cnt_IR2) << QString("%1").arg(cnt_IR3) << QString("%1").arg(cnt_IR4)
                    << QString("%1").arg(cnt_VIS) << QString("%1").arg(cnt_VIS1KM);
        }

        newitem = new QTreeWidgetItem( widget, strlist, 0  );
        if(whichgeo == SegmentListGeostationary::MET_10)
        {
            if (cnt_hrv == 24 && cnt_ir016 == 8 && cnt_ir039 == 8 && cnt_ir087 == 8 && cnt_ir097 == 8 && cnt_ir108 == 8 && cnt_ir120 == 8 && cnt_ir134 == 8 && cnt_vis006 == 8
                && cnt_vis008 == 8 && cnt_wv062 == 8 && cnt_wv073 == 8)
                col.setRgb(174, 225, 184);
            else
                col.setRgb(225, 171, 196);
        }
        else if(whichgeo == SegmentListGeostationary::MET_9)
        {
            if (cnt_hrv == 9 && cnt_ir016 == 3 && cnt_ir039 == 3 && cnt_ir087 == 3 && cnt_ir097 == 3 && cnt_ir108 == 3 && cnt_ir120 == 3 && cnt_ir134 == 3 && cnt_vis006 == 3
                && cnt_vis008 == 3 && cnt_wv062 == 3 && cnt_wv073 == 3)
                col.setRgb(174, 225, 184);
            else
                col.setRgb(225, 171, 196);
        }
        else if(whichgeo == SegmentListGeostationary::MET_7)
        {
            if (cnt_vis008 == 10 && cnt_wv062 == 5 && cnt_ir108 == 5 )
                col.setRgb(174, 225, 184);
            else
                col.setRgb(225, 171, 196);
        }
        else if (whichgeo == SegmentListGeostationary::ELECTRO_N1)
        {
            if (cnt_vis008 == 6 && cnt_ir087 == 6 && cnt_ir097 == 6 && cnt_ir108 == 6 )
                col.setRgb(174, 225, 184);
            else
                col.setRgb(225, 171, 196);
        }
        else if (whichgeo == SegmentListGeostationary::GOES_13 || whichgeo == SegmentListGeostationary::GOES_15)
        {
            if (cnt_vis008 == 7 && cnt_ir039 == 7 && cnt_ir087 == 7 && cnt_ir108 == 7 )
                col.setRgb(174, 225, 184);
            else
                col.setRgb(225, 171, 196);
        }
        else if (whichgeo == SegmentListGeostationary::MTSAT)
        {
            if (cnt_vis008 == 6 && cnt_ir039 && cnt_ir087 == 6 && cnt_ir108 == 6 && cnt_ir120 == 6 )
                col.setRgb(174, 225, 184);
            else
                col.setRgb(225, 171, 196);
        }
        else if (whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
        {
            if (cnt_IR1 == 1 && cnt_IR2 == 1 && cnt_IR3 == 1 && cnt_IR4 == 1 && cnt_VIS == 1 && cnt_VIS1KM == 1)
                col.setRgb(174, 225, 184);
            else
                col.setRgb(225, 171, 196);
        }


        newitem->setBackgroundColor( 0, col );
        newitem->setBackgroundColor( 1, col );
        newitem->setBackgroundColor( 2, col );
        newitem->setBackgroundColor( 3, col );
        newitem->setBackgroundColor( 4, col );
        newitem->setBackgroundColor( 5, col );
        newitem->setBackgroundColor( 6, col );
        newitem->setBackgroundColor( 7, col );
        newitem->setBackgroundColor( 8, col );
        newitem->setBackgroundColor( 9, col );
        newitem->setBackgroundColor( 10, col );
        newitem->setBackgroundColor( 11, col );
        newitem->setBackgroundColor( 12, col );
        newitem->setBackgroundColor( 13, col );

        ++citdate;
    }


}

void FormGeostationary::PopulateTree()
{

    PopulateTreeGeo(SegmentListGeostationary::MET_10, segs->segmentlistmapmeteosat, ui->SegmenttreeWidget);
    PopulateTreeGeo(SegmentListGeostationary::MET_9, segs->segmentlistmapmeteosatrss, ui->SegmenttreeWidgetRSS);
    PopulateTreeGeo(SegmentListGeostationary::MET_7, segs->segmentlistmapmet7, ui->SegmenttreeWidgetMet7);
    PopulateTreeGeo(SegmentListGeostationary::GOES_13, segs->segmentlistmapgoes13dc3, ui->SegmenttreeWidgetGOES13dc3);
    PopulateTreeGeo(SegmentListGeostationary::GOES_13, segs->segmentlistmapgoes13dc4, ui->SegmenttreeWidgetGOES13dc4);
    PopulateTreeGeo(SegmentListGeostationary::GOES_15, segs->segmentlistmapgoes15dc3, ui->SegmenttreeWidgetGOES15dc3);
    PopulateTreeGeo(SegmentListGeostationary::GOES_15, segs->segmentlistmapgoes15dc4, ui->SegmenttreeWidgetGOES15dc4);
    PopulateTreeGeo(SegmentListGeostationary::MTSAT, segs->segmentlistmapmtsatdc3, ui->SegmenttreeWidgetMTSATdc3);
    PopulateTreeGeo(SegmentListGeostationary::MTSAT, segs->segmentlistmapmtsatdc4, ui->SegmenttreeWidgetMTSATdc4);
    PopulateTreeGeo(SegmentListGeostationary::FY2E, segs->segmentlistmapfy2e, ui->SegmenttreeWidgetFY2E);
    PopulateTreeGeo(SegmentListGeostationary::FY2G, segs->segmentlistmapfy2g, ui->SegmenttreeWidgetFY2G);

}

FormGeostationary::~FormGeostationary()
{
    delete ui;
}

void FormGeostationary::CreateGeoImage(QString type, QVector<QString> spectrumvector, QVector<bool> inversevector)
{
     //H-000-MSG2__-MSG2________-VIS008___-000003___-201211131115-C_
     //H-???-??????-MSG2________-VIS008___-0?????___-201211131300-?_

    // segs->seglmeteosat->areatype == 1 ==> full
    // segs->seglmeteosat->areatype == 0 ==> europe


    // Red =(HRV x IR_016) / LUMlo
    // Green=(HRV x VIS008) / LUMlo
    // Blue =(HRV x VIS006) / LUMlo
    // LUMlo=IR_016 + VIS008 + VIS006

    QString tex;

    qDebug() << "FormGeostationary::CreateGeoImage(SegmentListGeostationary::eGeoSatellite whichgeo, QString type, QVector<QString> spectrumvector, QVector<bool> inversevector)";

    QList<QTreeWidgetItem *> treewidgetselected = ui->SegmenttreeWidget->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedrss = ui->SegmenttreeWidgetRSS->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedmet7 = ui->SegmenttreeWidgetMet7->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedgoes13dc3 = ui->SegmenttreeWidgetGOES13dc3->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedgoes13dc4 = ui->SegmenttreeWidgetGOES13dc4->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedgoes15dc3 = ui->SegmenttreeWidgetGOES15dc3->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedgoes15dc4 = ui->SegmenttreeWidgetGOES15dc4->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedmtsatdc3 = ui->SegmenttreeWidgetMTSATdc3->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedmtsatdc4 = ui->SegmenttreeWidgetMTSATdc4->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedfy2e = ui->SegmenttreeWidgetFY2E->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedfy2g = ui->SegmenttreeWidgetFY2G->selectedItems();

    SegmentListGeostationary *sl;

    if(treewidgetselected.size() > 0)
    {
        sl = segs->seglmeteosat;
        sl->bActiveSegmentList = true;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        //segs->seglelectro->bActiveSegmentList = false;
        segs->seglmet7->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglmtsatdc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglmtsatdc4->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        tex = (*treewidgetselected[0]).text(0);
        formtoolbox->createFilenamestring("MET-10", tex, spectrumvector);
    }
    else if(treewidgetselectedrss.size() > 0)
    {
        sl = segs->seglmeteosatrss;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        //segs->seglelectro->bActiveSegmentList = false;
        segs->seglmet7->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglmtsatdc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglmtsatdc4->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        tex = (*treewidgetselectedrss[0]).text(0);
        formtoolbox->createFilenamestring("MET-9", tex, spectrumvector);
    }
    else if(treewidgetselectedmet7.size() > 0)
    {
        sl = segs->seglmet7;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        //segs->seglelectro->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglmtsatdc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglmtsatdc4->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        tex = (*treewidgetselectedmet7[0]).text(0);
        formtoolbox->createFilenamestring("MET-7", tex, spectrumvector);
    }
    else if(treewidgetselectedgoes13dc3.size() > 0)
    {
        sl = segs->seglgoes13dc3;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        //segs->seglelectro->bActiveSegmentList = false;
        segs->seglmet7->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglmtsatdc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglmtsatdc4->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        tex = (*treewidgetselectedgoes13dc3[0]).text(0);
        formtoolbox->createFilenamestring("GOES-13", tex, spectrumvector);
    }
    else if(treewidgetselectedgoes15dc3.size() > 0)
    {
        sl = segs->seglgoes15dc3;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        //segs->seglelectro->bActiveSegmentList = false;
        segs->seglmet7->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglmtsatdc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglmtsatdc4->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        tex = (*treewidgetselectedgoes15dc3[0]).text(0);
        formtoolbox->createFilenamestring("GOES-15", tex, spectrumvector);
    }
    else if(treewidgetselectedmtsatdc3.size() > 0)
    {
        sl = segs->seglmtsatdc3;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        //segs->seglelectro->bActiveSegmentList = false;
        segs->seglmet7->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglmtsatdc4->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        tex = (*treewidgetselectedmtsatdc3[0]).text(0);
        formtoolbox->createFilenamestring("MTSAT2", tex, spectrumvector);
    }
    else if(treewidgetselectedgoes13dc4.size() > 0)
    {
        sl = segs->seglgoes13dc4;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        //segs->seglelectro->bActiveSegmentList = false;
        segs->seglmet7->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglmtsatdc3->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglmtsatdc4->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        tex = (*treewidgetselectedgoes13dc4[0]).text(0);
        formtoolbox->createFilenamestring("GOES-13", tex, spectrumvector);
    }
    else if(treewidgetselectedgoes15dc4.size() > 0)
    {
        sl = segs->seglgoes15dc4;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        //segs->seglelectro->bActiveSegmentList = false;
        segs->seglmet7->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglmtsatdc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglmtsatdc4->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        tex = (*treewidgetselectedgoes15dc4[0]).text(0);
        formtoolbox->createFilenamestring("GOES-15", tex, spectrumvector);
    }
    else if(treewidgetselectedmtsatdc4.size() > 0)
    {
        sl = segs->seglmtsatdc4;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        //segs->seglelectro->bActiveSegmentList = false;
        segs->seglmet7->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglmtsatdc3->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        tex = (*treewidgetselectedmtsatdc4[0]).text(0);
        formtoolbox->createFilenamestring("MTSAT2", tex, spectrumvector);
    }
    else if(treewidgetselectedfy2e.size() > 0)
    {
        sl = segs->seglfy2e;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        //segs->seglelectro->bActiveSegmentList = false;
        segs->seglmet7->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglmtsatdc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglmtsatdc4->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        tex = (*treewidgetselectedfy2e[0]).text(0);
        formtoolbox->createFilenamestring("FY2E", tex, spectrumvector);
    }
    else if(treewidgetselectedfy2g.size() > 0)
    {
        sl = segs->seglfy2g;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        //segs->seglelectro->bActiveSegmentList = false;
        segs->seglmet7->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglmtsatdc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglmtsatdc4->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        tex = (*treewidgetselectedfy2g[0]).text(0);
        formtoolbox->createFilenamestring("FY2G", tex, spectrumvector);
    }

    sl->ResetSegments();

    imageptrs->ResetPtrImage();

    qDebug() << QString(" CreateGeoImage ; kind of image = %1 spectrumvector = %2 ; %3 ; %4").arg(sl->getKindofImage()).arg(spectrumvector.at(0)).arg(spectrumvector.at(1)).arg(spectrumvector.at(2));
    qDebug() << QString(" CreateGeoImage ; imagecreated = %1").arg(tex);

    if (spectrumvector.at(0) == "" &&  spectrumvector.at(1) == "" && spectrumvector.at(1) == "")
        return;

    if (type == "HRV" || type == "HRV Color")
    {
        if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
            imageptrs->InitializeImageGeostationary(9152, 9152);
        else
        {
            if (sl->areatype == 1)
                imageptrs->InitializeImageGeostationary(5568, 11136);
            else
                imageptrs->InitializeImageGeostationary(5568, 464*5);
        }
    }
    else
    {
        if(sl->getGeoSatellite() == SegmentListGeostationary::MET_10)
            imageptrs->InitializeImageGeostationary(3712, 3712);
        else if(sl->getGeoSatellite() == SegmentListGeostationary::MET_9)
            imageptrs->InitializeImageGeostationary(3712, 464*3);
        //else if(sl->getGeoSatellite() == SegmentListGeostationary::ELECTRO_N1)
        //    imageptrs->InitializeImageGeostationary(2784, 2784);
        else if(sl->getGeoSatellite() == SegmentListGeostationary::MET_7)
        {
            if(spectrumvector.at(0) == "00_7_0")
                imageptrs->InitializeImageGeostationary(5032, 500*10);
            else
                imageptrs->InitializeImageGeostationary(2532, 500*5);
        }
        else if(sl->getGeoSatellite() == SegmentListGeostationary::GOES_13)
            imageptrs->InitializeImageGeostationary(2816, 464*7);
        else if(sl->getGeoSatellite() == SegmentListGeostationary::GOES_15)
            imageptrs->InitializeImageGeostationary(2816, 464*7);
        else if(sl->getGeoSatellite() == SegmentListGeostationary::MTSAT)
            imageptrs->InitializeImageGeostationary(2752, 464*6);
        else if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
            imageptrs->InitializeImageGeostationary(2288, 2288);
    }

    formimage->displayImage(8);
    //formimage->adjustPicSize(true);

    qDebug() << QString("FormGeostationary::CreateGeoImage kind = %1 areatype = %2").arg(type).arg(sl->areatype);

    if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
        CreateGeoImageHDF(sl, type, tex, spectrumvector, inversevector);
    else
        CreateGeoImageXRIT(sl, type, tex, spectrumvector, inversevector);

}

void FormGeostationary::CreateGeoImageXRIT(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector)
{

    QString filetiming;
    QString filedate;
    QStringList llVIS_IR;
    QStringList llHRV;
    MsgFileAccess faVIS_IR;
    MsgFileAccess faHRV;
    QString filepattern;
    int filesequence;
    QString filespectrum;

    long lCFAC;
    long lLFAC;

    SegmentListGeostationary::eGeoSatellite whichgeo = sl->getGeoSatellite();

    filetiming = tex.mid(0, 4) + tex.mid(5, 2) + tex.mid(8, 2) + tex.mid(13, 2) + tex.mid(16, 2);
    filedate = tex.mid(0, 4) + tex.mid(5, 2) + tex.mid(8, 2);
    //filepattern1 = "H-???-??????-MSG?_???____-??????___-0?????___-" + filetiming + "-C_";
    //filepattern1 = "H-000-MSG1__-MSG1_RSS____-IR_016___-000007___-201310271420-C_";
    //filepattern1 = "H-000-MSG1__-MSG1_RSS____-_________-EPI______-201204101155-__"
    //H-000-MSG1__-MSG1_RSS____-IR_016___-000007___-201310271420-C_
    //H-000-GOMS1_-GOMS1_4_____-00_9_076E-000001___-201312231100-C_
    //L-000-MSG3__-GOES13______-00_7_075W-000001___-201404041200-C_

    //    filepattern1 = QString("H-???-??????-????????____-?????????-0?????___-") + filetiming + QString("-C_");
    //  filepattern1 = QString("H-???-??????-MSG?_???____-??????___-0?????___-") + filetiming + QString("-C_");

    if(whichgeo == SegmentListGeostationary::MET_7)
        filepattern = QString("L-???") + QString("-??????") + QString("-MET7*") + filetiming + QString("-C_");
    else if(whichgeo == SegmentListGeostationary::GOES_13)
        filepattern = QString("L-???") + QString("-??????") + QString("-GOES13*") + filetiming + QString("-C_");
    else if(whichgeo == SegmentListGeostationary::GOES_15 )
        filepattern = QString("L-???") + QString("-??????") + QString("-GOES15*") + filetiming + QString("-C_");
    else if(whichgeo == SegmentListGeostationary::MTSAT )
        filepattern = QString("L-???") + QString("-??????") + QString("-MTSAT2*") + filetiming + QString("-C_");
    else
        filepattern = QString("H-???") + QString("-??????") + QString("-????????____-?????????") + QString("-0?????___-") + filetiming + QString("-C_");
    qDebug() << "FormGeostationary::CreateGeoImage filepattern = " << filepattern;

    if(type == "VIS_IR" || type == "VIS_IR Color")
    {
        llVIS_IR = this->getGeostationarySegments(whichgeo, "VIS_IR", sl->getImagePath(), spectrumvector, filepattern);
        if(llVIS_IR.size() == 0)
        {
            qDebug() << "FormGeostationary::CreateGeoImage : VIS_IR : ===> No segments selected";
            emit enabletoolboxbuttons(true);
            QApplication::restoreOverrideCursor();
            return;
        }
        faVIS_IR.parse(sl->getImagePath() + "/" + llVIS_IR.at(0));
    }
    else if(type == "HRV Color")
    {
        llVIS_IR = this->getGeostationarySegments(whichgeo, "VIS_IR", sl->getImagePath(), spectrumvector, filepattern);
        llHRV = this->getGeostationarySegments(whichgeo, "HRV", sl->getImagePath(), spectrumvector, filepattern);
        if(llVIS_IR.size() == 0 || llHRV.size() == 0)
        {
            qDebug() << "FormGeostationary::CreateGeoImage : HRV Color : ===> No segments selected";
            emit enabletoolboxbuttons(true);
            QApplication::restoreOverrideCursor();
            return;
        }
        faVIS_IR.parse(sl->getImagePath() + "/" + llVIS_IR.at(0));
        faHRV.parse(sl->getImagePath() + "/" + llHRV.at(0));
    }
    else if(type == "HRV")
    {
        llHRV = this->getGeostationarySegments(whichgeo, "HRV", sl->getImagePath(), spectrumvector, filepattern);
        if(llHRV.size() == 0)
        {
            qDebug() << "FormGeostationary::CreateGeoImage : HRV : ===> No segments selected";
            emit enabletoolboxbuttons(true);
            QApplication::restoreOverrideCursor();
            return;
        }
        faHRV.parse(sl->getImagePath() + "/" + llHRV.at(0));
    }


#if 0
        for (int j = 0; j < llVIS_IR.size(); ++j)
        {
            qDebug() << QString("VIS_IR ======= %1    %2").arg(j).arg(llVIS_IR.at(j));
        }
        for (int j = 0; j < llHRV.size(); ++j)
        {
            qDebug() << QString("HRV ======= %1    %2").arg(j).arg(llHRV.at(j));
        }

#endif

        MsgDataAccess da;

        MSG_data pro;
        MSG_data epi;
        MSG_header header;
        MsgFileAccess fa = (type == "VIS_IR" || type == "VIS_IR Color" ? faVIS_IR : faHRV);

        // Read prologue
        MSG_header PRO_head;

        QString prologuefile = fa.prologueFile();

        qDebug() << QString("Reading prologue file = %1").arg(prologuefile);

        if (prologuefile.length() > 0)
        {
            try
            {
                da.read_file(fa.directory + "/" + prologuefile, PRO_head, pro);
            }
            catch( std::runtime_error &run )
            {
                qDebug() << QString("Error : runtime error in reading prologue file : %1").arg(run.what());
            }
        }

        // Read epilogue
        MSG_header EPI_head;

        QString epiloguefile = fa.epilogueFile();

        qDebug() << QString("Reading epilogue file = %1").arg(epiloguefile);

        if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
        {
            try
            {
                da.read_file(fa.directory + "/" + epiloguefile, EPI_head, epi);
                if (type == "HRV" || type == "HRV Color")
                {
                    MSG_ActualL15CoverageHRV& cov = epi.epilogue->product_stats.ActualL15CoverageHRV;
                    sl->LowerEastColumnActual = cov.LowerEastColumnActual;
                    sl->LowerNorthLineActual = cov.LowerNorthLineActual;
                    sl->LowerWestColumnActual = cov.LowerWestColumnActual;
                    sl->LowerSouthLineActual = cov.LowerSouthLineActual;
                    sl->UpperEastColumnActual = cov.UpperEastColumnActual;
                    sl->UpperSouthLineActual = cov.UpperSouthLineActual;
                    sl->UpperWestColumnActual = cov.UpperWestColumnActual;
                    sl->UpperNorthLineActual = cov.UpperNorthLineActual;

                    qDebug() << QString("sl->LowerEastColumnActual = %1").arg(sl->LowerEastColumnActual);
                    qDebug() << QString("sl->LowerNorthLineActual = %1").arg(sl->LowerNorthLineActual);
                    qDebug() << QString("sl->LowerWestColumnActual = %1").arg(sl->LowerWestColumnActual);
                    qDebug() << QString("sl->LowerSouthLineActual = %1").arg(sl->LowerSouthLineActual);
                    qDebug() << QString("sl->UpperEastColumnActual = %1").arg(sl->UpperEastColumnActual);
                    qDebug() << QString("sl->UpperSouthLineActual = %1").arg(sl->UpperSouthLineActual);
                    qDebug() << QString("sl->UpperWestColumnActual = %1").arg(sl->UpperWestColumnActual);
                    qDebug() << QString("sl->UpperNorthLineActual = %1").arg(sl->UpperNorthLineActual);
                }
                else
                {
                    sl->WestColumnActual = 1;
                    sl->SouthLineActual = 1;
                }
            }
            catch( std::runtime_error &run )
            {
                qDebug() << QString("Error : runtime error in reading epilogue file : %1").arg(run.what());
                sl->LowerEastColumnActual = 0;
                sl->LowerNorthLineActual = 0;
                sl->LowerWestColumnActual = 0;
                sl->LowerSouthLineActual = 0;
                sl->UpperEastColumnActual = 0;
                sl->UpperSouthLineActual = 0;
                sl->UpperWestColumnActual = 0;
                sl->UpperNorthLineActual = 0;

            }
        }

        try
        {
//            da.read_file(fa.directory + "/" + fa.segmentFiles().at(0),header);
            da.read_file(fa.directory + "/" + (type == "VIS_IR" || type == "VIS_IR Color" ? llVIS_IR.at(0) : llHRV.at(0)),header);
            lCFAC = (long)header.image_navigation->CFAC*180.0/PI;
            lLFAC = (long)header.image_navigation->LFAC*180.0/PI;
            sl->CFAC = abs(lCFAC);
            sl->COFF = abs(header.image_navigation->COFF);
            sl->LFAC = abs(lLFAC);
            sl->LOFF = abs(header.image_navigation->LOFF);
            qDebug() << QString("CFAC = %1 COFF = %2 LFAC = %3 LOFF = %4").arg(sl->CFAC).arg(sl->COFF).arg(sl->LFAC).arg(sl->LOFF);

        }
        catch( std::runtime_error &run )
        {
            qDebug() << QString("Error : runtime error in reading first segmentfile : %1").arg(run.what());
        }


        if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9)
        {
            if(type == "HRV" || type == "HRV Color")
            {
                sl->COFF = COFF_HRV;
                sl->LOFF = LOFF_HRV;
                sl->CFAC = CFAC_HRV;
                sl->LFAC = LFAC_HRV;
            }
            else
            {
                sl->COFF = COFF_NONHRV;
                sl->LOFF = LOFF_NONHRV;
                sl->CFAC = CFAC_NONHRV;
                sl->LFAC = LFAC_NONHRV;
             }
        }
        else if(whichgeo == SegmentListGeostationary::MET_7)
        {
            if(spectrumvector.at(0) == "00_7_0")
            {
                sl->COFF = COFF_NONHRV_MET7;
                sl->LOFF = LOFF_NONHRV_MET7;
                sl->CFAC = CFAC_NONHRV_MET7;
                sl->LFAC = LFAC_NONHRV_MET7;
            }
            else
            {
                sl->COFF = COFF_NONHRV_MET7/2;
                sl->LOFF = LOFF_NONHRV_MET7/2;
                sl->CFAC = CFAC_NONHRV_MET7/2;
                sl->LFAC = LFAC_NONHRV_MET7/2;
            }

        }
        else if(whichgeo == SegmentListGeostationary::GOES_13 || whichgeo == SegmentListGeostationary::GOES_15)
        {
            sl->COFF = COFF_NONHRV_GOES;
            sl->LOFF = LOFF_NONHRV_GOES;
            sl->CFAC = CFAC_NONHRV_GOES;
            sl->LFAC = LFAC_NONHRV_GOES;
        }
        else if(whichgeo == SegmentListGeostationary::MTSAT)
        {
            sl->COFF = COFF_NONHRV_MTSAT;
            sl->LOFF = LOFF_NONHRV_MTSAT;
            sl->CFAC = CFAC_NONHRV_MTSAT;
            sl->LFAC = LFAC_NONHRV_MTSAT;
        }

        if(type == "VIS_IR" || type == "VIS_IR Color" || type == "HRV Color")
        {
            for (int j =  0; j < llVIS_IR.size(); ++j)
            {
                QFile file(sl->getImagePath() + "/" + llVIS_IR.at(j));
                QFileInfo fileinfo(file);
                filesequence = fileinfo.fileName().mid(36, 6).toInt()-1;
                filespectrum = fileinfo.fileName().mid(26, 6);
                filedate = fileinfo.fileName().mid(46, 12);
                sl->InsertPresent( spectrumvector, filespectrum, filesequence);
                if(sl->getGeoSatellite() == SegmentListGeostationary::MET_9)
                {
                    if(filesequence >= 5)
                        sl->ComposeImageXRIT(fileinfo.filePath(), spectrumvector, inversevector);
                }
                else
                    sl->ComposeImageXRIT(fileinfo.filePath(), spectrumvector, inversevector);

                qDebug() << QString("ComposeImageXRIT VIS_IR ----> %1").arg(fileinfo.filePath());
            }
        }

        if( type == "HRV" || type == "HRV Color")
        {
            for (int j = 0; j < llHRV.size(); ++j)
            {
                QFile file(sl->getImagePath() + "/" + llHRV.at(j));
                QFileInfo fileinfo(file);
                filesequence = fileinfo.fileName().mid(36, 6).toInt()-1;
                filespectrum = fileinfo.fileName().mid(26, 6);
                filedate = fileinfo.fileName().mid(46, 12);
                sl->InsertPresent( spectrumvector, filespectrum, filesequence);
                if(sl->getGeoSatellite() == SegmentListGeostationary::MET_9)
                {
                    if(filesequence >= 19)
                        sl->ComposeImageXRIT(fileinfo.filePath(), spectrumvector, inversevector);
                }
                else
                {
                    if(sl->areatype == 1)
                        sl->ComposeImageXRIT(fileinfo.filePath(), spectrumvector, inversevector);
                    else if(filesequence >= 19)
                        sl->ComposeImageXRIT(fileinfo.filePath(), spectrumvector, inversevector);
                }
                qDebug() << QString("ComposeImageXRIT HRV ----> %1").arg(fileinfo.filePath());
            }
        }

}

void FormGeostationary::CreateGeoImageHDF(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector)
{

    QString filetiming;
    QString filedate;
    QStringList llVIS_IR;
    QStringList llHRV;
    QString filepattern;

    SegmentListGeostationary::eGeoSatellite whichgeo = sl->getGeoSatellite();

    if(type == "HRV")
    {
        sl->COFF = COFF_HRV_FENGYUN;
        sl->LOFF = LOFF_HRV_FENGYUN;
        sl->CFAC = CFAC_HRV_FENGYUN;
        sl->LFAC = LFAC_HRV_FENGYUN;
    }
    else
    {
        sl->COFF = COFF_NONHRV_FENGYUN;
        sl->LOFF = LOFF_NONHRV_FENGYUN;
        sl->CFAC = CFAC_NONHRV_FENGYUN;
        sl->LFAC = LFAC_NONHRV_FENGYUN;
     }

    filetiming = tex.mid(0, 4) + tex.mid(5, 2) + tex.mid(8, 2) + tex.mid(13, 2) + tex.mid(16, 2) + "00";
    filedate = tex.mid(0, 4) + tex.mid(5, 2) + tex.mid(8, 2);

    if(whichgeo == SegmentListGeostationary::FY2E && (type == "VIS_IR" || type == "VIS_IR Color"))
        filepattern = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2E_FDI_???") + QString("_001_NOM.HDF.gz");
    else if(whichgeo == SegmentListGeostationary::FY2G && (type == "VIS_IR" || type == "VIS_IR Color"))
        filepattern = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2G_FDI_???") + QString("_001_NOM.HDF.gz");
    else if(whichgeo == SegmentListGeostationary::FY2E && type == "HRV")
        filepattern = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2E_FDI_VIS1KM") + QString("_001_NOM.HDF.gz");
    else if(whichgeo == SegmentListGeostationary::FY2G && type == "HRV")
        filepattern = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2G_FDI_VIS1KM") + QString("_001_NOM.HDF.gz");

    if(type == "VIS_IR" || type == "VIS_IR Color")
    {
        llVIS_IR = this->getGeostationarySegmentsFengYun(whichgeo, "VIS_IR", sl->getImagePath(), spectrumvector, filepattern);
        qDebug() << QString("llVIS_IR count = %1").arg(llVIS_IR.count());
        for (int j =  0; j < llVIS_IR.size(); ++j)
        {
            qDebug() << QString("llVIS_IR at %1 = %2 spectrumvector = %3").arg(j).arg(llVIS_IR.at(j)).arg(spectrumvector.at(j));
        }
        sl->ComposeImageHDFInThread(llVIS_IR, spectrumvector, inversevector);
    }
    else if(type == "HRV")
    {
        llHRV = this->getGeostationarySegmentsFengYun(whichgeo, "HRV", sl->getImagePath(), spectrumvector, filepattern);
        sl->ComposeImageHDFInThread(llHRV, spectrumvector, inversevector);
    }

}


void FormGeostationary::setTreeWidget(QTreeWidget *widget, bool state)
{
    QList<QTreeWidgetItem *> treewidgetselected = widget->selectedItems();
    if(treewidgetselected.size() > 0)
    {
        for(int i = 0; i < treewidgetselected.size(); i++)
        {
            treewidgetselected[i]->setSelected(state);
        }
    }
}

void FormGeostationary::on_SegmenttreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{

    setTreeWidget(ui->SegmenttreeWidgetRSS, false);
    setTreeWidget(ui->SegmenttreeWidgetMet7, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc4, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2E, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2G, false);

    qDebug() << "MET-10 " + (*item).text(0);  // << "  " << (*item).text(1); // << item->text(1).toInt(&ok, 10);

    segs->seglmeteosat->setKindofImage("");

    if ( !item )
          return;

    QStringList tex;
    tex << "MET-10";
    for(int i = 0; i < item->columnCount(); i++)
        tex << (*item).text(i);
    emit geostationarysegmentschosen(SegmentListGeostationary::MET_10, tex);
}





void FormGeostationary::on_SegmenttreeWidgetRSS_itemClicked(QTreeWidgetItem *item, int column)
{
    setTreeWidget(ui->SegmenttreeWidget, false);
    setTreeWidget(ui->SegmenttreeWidgetMet7, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc4, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2E, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2G, false);

    segs->seglmeteosatrss->setKindofImage("");

    if ( !item )
          return;

    qDebug() << "MET-9 " << (*item).text(0);  // << "  " << (*item).text(1); // << item->text(1).toInt(&ok, 10);
    QStringList tex;
    tex << "MET-9";
    for(int i = 0; i < item->columnCount(); i++)
        tex << (*item).text(i);
    emit geostationarysegmentschosen(SegmentListGeostationary::MET_9, tex);

}


void FormGeostationary::on_SegmenttreeWidgetMet7_itemClicked(QTreeWidgetItem *item, int column)
{
    setTreeWidget(ui->SegmenttreeWidget, false);
    setTreeWidget(ui->SegmenttreeWidgetRSS, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc4, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2E, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2G, false);

    segs->seglmet7->setKindofImage("");

    if ( !item )
          return;

    qDebug() << "MET-7 " << (*item).text(0);  // << "  " << (*item).text(1); // << item->text(1).toInt(&ok, 10);
    QStringList tex;
    tex << "MET-7";
    for(int i = 0; i < item->columnCount(); i++)
        tex << (*item).text(i);
    emit geostationarysegmentschosen(SegmentListGeostationary::MET_7, tex);

}

void FormGeostationary::on_SegmenttreeWidgetGOES13dc3_itemClicked(QTreeWidgetItem *item, int column)
{
    setTreeWidget(ui->SegmenttreeWidget, false);
    setTreeWidget(ui->SegmenttreeWidgetRSS, false);
    setTreeWidget(ui->SegmenttreeWidgetMet7, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc4, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2E, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2G, false);

    segs->seglgoes13dc3->setKindofImage("");

    if ( !item )
          return;

    qDebug() << "GOES-13 " << (*item).text(0) << ";" << (*item).text(1) << ";" <<  (*item).text(2) << ";" << (*item).text(3) << ";" << (*item).text(4) << ";" << (*item).text(4);
    QStringList tex;
    tex << "GOES-13";
    for(int i = 0; i < item->columnCount(); i++)
        tex << (*item).text(i);
    emit geostationarysegmentschosen(SegmentListGeostationary::GOES_13, tex);


}

void FormGeostationary::on_SegmenttreeWidgetGOES15dc3_itemClicked(QTreeWidgetItem *item, int column)
{
    setTreeWidget(ui->SegmenttreeWidget, false);
    setTreeWidget(ui->SegmenttreeWidgetRSS, false);
    setTreeWidget(ui->SegmenttreeWidgetMet7, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc4, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2E, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2G, false);

    segs->seglgoes15dc3->setKindofImage("");

    if ( !item )
          return;

    qDebug() << "GOES-15 " << (*item).text(0);  // << "  " << (*item).text(1); // << item->text(1).toInt(&ok, 10);
    QStringList tex;
    tex << "GOES-15";
    for(int i = 0; i < item->columnCount(); i++)
        tex << (*item).text(i);
    emit geostationarysegmentschosen(SegmentListGeostationary::GOES_15, tex);

}

void FormGeostationary::on_SegmenttreeWidgetMTSATdc3_itemClicked(QTreeWidgetItem *item, int column)
{
    setTreeWidget(ui->SegmenttreeWidget, false);
    setTreeWidget(ui->SegmenttreeWidgetRSS, false);
    setTreeWidget(ui->SegmenttreeWidgetMet7, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc4, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2E, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2G, false);

    segs->seglmtsatdc3->setKindofImage("");

    if ( !item )
          return;

    qDebug() << "MTSAT2 " << (*item).text(0);  // << "  " << (*item).text(1); // << item->text(1).toInt(&ok, 10);
    QStringList tex;
    tex << "MTSAT2";
    for(int i = 0; i < item->columnCount(); i++)
        tex << (*item).text(i);
    emit geostationarysegmentschosen(SegmentListGeostationary::MTSAT, tex);


}

void FormGeostationary::on_SegmenttreeWidgetGOES13dc4_itemClicked(QTreeWidgetItem *item, int column)
{
    setTreeWidget(ui->SegmenttreeWidget, false);
    setTreeWidget(ui->SegmenttreeWidgetRSS, false);
    setTreeWidget(ui->SegmenttreeWidgetMet7, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc4, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2E, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2G, false);

    segs->seglgoes13dc4->setKindofImage("");

    if ( !item )
          return;

    qDebug() << "GOES-13 " << (*item).text(0) << ";" << (*item).text(1) << ";" <<  (*item).text(2) << ";" << (*item).text(3) << ";" << (*item).text(4) << ";" << (*item).text(4);
    QStringList tex;
    tex << "GOES-13";
    for(int i = 0; i < item->columnCount(); i++)
        tex << (*item).text(i);
    emit geostationarysegmentschosen(SegmentListGeostationary::GOES_13, tex);


}

void FormGeostationary::on_SegmenttreeWidgetGOES15dc4_itemClicked(QTreeWidgetItem *item, int column)
{
    setTreeWidget(ui->SegmenttreeWidget, false);
    setTreeWidget(ui->SegmenttreeWidgetRSS, false);
    setTreeWidget(ui->SegmenttreeWidgetMet7, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc4, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2E, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2G, false);

    segs->seglgoes15dc4->setKindofImage("");

    if ( !item )
          return;

    qDebug() << "GOES-15 " << (*item).text(0);  // << "  " << (*item).text(1); // << item->text(1).toInt(&ok, 10);
    QStringList tex;
    tex << "GOES-15";
    for(int i = 0; i < item->columnCount(); i++)
        tex << (*item).text(i);
    emit geostationarysegmentschosen(SegmentListGeostationary::GOES_15, tex);

}

void FormGeostationary::on_SegmenttreeWidgetMTSATdc4_itemClicked(QTreeWidgetItem *item, int column)
{
    setTreeWidget(ui->SegmenttreeWidget, false);
    setTreeWidget(ui->SegmenttreeWidgetRSS, false);
    setTreeWidget(ui->SegmenttreeWidgetMet7, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc3, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2E, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2G, false);

    segs->seglmtsatdc4->setKindofImage("");

    if ( !item )
          return;

    qDebug() << "MTSAT2 " << (*item).text(0);  // << "  " << (*item).text(1); // << item->text(1).toInt(&ok, 10);
    QStringList tex;
    tex << "MTSAT2";
    for(int i = 0; i < item->columnCount(); i++)
        tex << (*item).text(i);
    emit geostationarysegmentschosen(SegmentListGeostationary::MTSAT, tex);


}


void FormGeostationary::on_SegmenttreeWidgetFY2E_itemClicked(QTreeWidgetItem *item, int column)
{
    setTreeWidget(ui->SegmenttreeWidget, false);
    setTreeWidget(ui->SegmenttreeWidgetRSS, false);
    setTreeWidget(ui->SegmenttreeWidgetMet7, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc4, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2G, false);

    qDebug() << "FY2E " + (*item).text(0);  // << "  " << (*item).text(1); // << item->text(1).toInt(&ok, 10);

    segs->seglfy2e->setKindofImage("");

    if ( !item )
          return;

    QStringList tex;
    tex << "FY2E";
    for(int i = 0; i < item->columnCount(); i++)
        tex << (*item).text(i);
    emit geostationarysegmentschosen(SegmentListGeostationary::FY2E, tex);


}

void FormGeostationary::on_SegmenttreeWidgetFY2G_itemClicked(QTreeWidgetItem *item, int column)
{
    setTreeWidget(ui->SegmenttreeWidget, false);
    setTreeWidget(ui->SegmenttreeWidgetRSS, false);
    setTreeWidget(ui->SegmenttreeWidgetMet7, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc3, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc3, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc4, false);
    setTreeWidget(ui->SegmenttreeWidgetMTSATdc4, false);
    setTreeWidget(ui->SegmenttreeWidgetFY2E, false);

    qDebug() << "FY2G " + (*item).text(0);  // << "  " << (*item).text(1); // << item->text(1).toInt(&ok, 10);

    segs->seglfy2g->setKindofImage("");

    if ( !item )
          return;

    QStringList tex;
    tex << "FY2G";
    for(int i = 0; i < item->columnCount(); i++)
        tex << (*item).text(i);
    emit geostationarysegmentschosen(SegmentListGeostationary::FY2G, tex);

}
