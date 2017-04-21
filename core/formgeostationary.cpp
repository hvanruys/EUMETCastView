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
    ui->tabGeostationary->setTabText( 0, "MET-10 : " + opts.geostationarylistlon.at(SegmentListGeostationary::MET_10) + "°");
    ui->tabGeostationary->setTabText( 1, "MET-9 : " + opts.geostationarylistlon.at(SegmentListGeostationary::MET_9) + "°");
    ui->tabGeostationary->setTabText( 2, "MET-8 : " + opts.geostationarylistlon.at(SegmentListGeostationary::MET_8) + "°");
    ui->tabGeostationary->setTabText( 3, "FY2E : " + opts.geostationarylistlon.at(SegmentListGeostationary::FY2E) + "°");
    ui->tabGeostationary->setTabText( 4, "FY2G : " + opts.geostationarylistlon.at(SegmentListGeostationary::FY2G) + "°");
    ui->tabGeostationary->setTabText( 5, "GOES-13 (DC-3) : " + opts.geostationarylistlon.at(SegmentListGeostationary::GOES_13) + "°");
    ui->tabGeostationary->setTabText( 6, "GOES-13 (DC-4) : " + opts.geostationarylistlon.at(SegmentListGeostationary::GOES_13) + "°");
    ui->tabGeostationary->setTabText( 7, "GOES-15 (DC-3) : " + opts.geostationarylistlon.at(SegmentListGeostationary::GOES_15) + "°");
    ui->tabGeostationary->setTabText( 8, "GOES-15 (DC-4) : " + opts.geostationarylistlon.at(SegmentListGeostationary::GOES_15) + "°");
    ui->tabGeostationary->setTabText( 9, "GOES-16 : " + opts.geostationarylistlon.at(SegmentListGeostationary::GOES_16) + "°");
    ui->tabGeostationary->setTabText( 10, "Himawari-8 : " + opts.geostationarylistlon.at(SegmentListGeostationary::H8) + "°");

    ui->SegmenttreeWidget->setRootIsDecorated(false);
    ui->SegmenttreeWidget->header()->setStretchLastSection(true);
    ui->SegmenttreeWidget->setColumnCount(14);
    for(int i = 0; i < 14; i++)
    {
        ui->SegmenttreeWidget->setColumnWidth(i, 200);
        ui->SegmenttreeWidget->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    ui->SegmenttreeWidget->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "HRV" << "0.6" << "0.8" << "1.6" << "3.9" << "6.2" << "7.3" << "8.7" << "9.7" << "10.8" << "12.8" << "13.4" );


    ui->SegmenttreeWidgetRSS->setRootIsDecorated(false);
    ui->SegmenttreeWidgetRSS->header()->setStretchLastSection(true);
    ui->SegmenttreeWidgetRSS->setColumnCount(14);
    for(int i = 0; i < 14; i++)
    {
        ui->SegmenttreeWidgetRSS->setColumnWidth(i, 200);
        ui->SegmenttreeWidgetRSS->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    ui->SegmenttreeWidgetRSS->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "HRV" << "0.6" << "0.8" << "1.6" << "3.9" << "6.2" << "7.3" << "8.7" << "9.7" << "10.8" << "12.8" << "13.4" );


    ui->SegmenttreeWidgetMet8->setRootIsDecorated(false);
    ui->SegmenttreeWidgetMet8->header()->setStretchLastSection(true);
    ui->SegmenttreeWidgetMet8->setColumnCount(14);
    for(int i = 0; i < 14; i++)
    {
        ui->SegmenttreeWidgetMet8->setColumnWidth(i, 200);
        ui->SegmenttreeWidgetMet8->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    ui->SegmenttreeWidgetMet8->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "HRV" << "0.6" << "0.8" << "1.6" << "3.9" << "6.2" << "7.3" << "8.7" << "9.7" << "10.8" << "12.8" << "13.4" );


    ui->SegmenttreeWidgetGOES13dc3->setRootIsDecorated(false);
    ui->SegmenttreeWidgetGOES13dc3->header()->setStretchLastSection(true);
    ui->SegmenttreeWidgetGOES13dc3->setColumnCount(4);
    for(int i = 0; i < 4; i++)
    {
        ui->SegmenttreeWidgetGOES13dc3->setColumnWidth(i, 200);
        ui->SegmenttreeWidgetGOES13dc3->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    ui->SegmenttreeWidgetGOES13dc3->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "0.7" << "3.9" << "6.6" << "10.7" );

    ui->SegmenttreeWidgetGOES13dc4->setRootIsDecorated(false);
    ui->SegmenttreeWidgetGOES13dc4->header()->setStretchLastSection(true);
    ui->SegmenttreeWidgetGOES13dc4->setColumnCount(4);
    for(int i = 0; i < 4; i++)
    {
        ui->SegmenttreeWidgetGOES13dc4->setColumnWidth(i, 200);
        ui->SegmenttreeWidgetGOES13dc4->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    ui->SegmenttreeWidgetGOES13dc4->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "0.7" << "3.9" << "6.6" << "10.7" );


    ui->SegmenttreeWidgetGOES15dc3->setRootIsDecorated(false);
    ui->SegmenttreeWidgetGOES15dc3->header()->setStretchLastSection(true);
    ui->SegmenttreeWidgetGOES15dc3->setColumnCount(4);
    for(int i = 0; i < 4; i++)
    {
        ui->SegmenttreeWidgetGOES15dc3->setColumnWidth(i, 200);
        ui->SegmenttreeWidgetGOES15dc3->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    ui->SegmenttreeWidgetGOES15dc3->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "0.7" << "3.9" << "6.9" << "10.7" );

    ui->SegmenttreeWidgetGOES15dc4->setRootIsDecorated(false);
    ui->SegmenttreeWidgetGOES15dc4->header()->setStretchLastSection(true);
    ui->SegmenttreeWidgetGOES15dc4->setColumnCount(4);
    for(int i = 0; i < 4; i++)
    {
        ui->SegmenttreeWidgetGOES15dc4->setColumnWidth(i, 200);
        ui->SegmenttreeWidgetGOES15dc4->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    ui->SegmenttreeWidgetGOES15dc4->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "0.7" << "3.9" << "6.9" << "10.7" );


    ui->SegmenttreeWidgetGOES16->setRootIsDecorated(false);
    ui->SegmenttreeWidgetGOES16->header()->setStretchLastSection(true);
    ui->SegmenttreeWidgetGOES16->setColumnCount(18);
    for(int i = 0; i < 18; i++)
    {
        ui->SegmenttreeWidgetGOES16->setColumnWidth(i, 200);
        ui->SegmenttreeWidgetGOES16->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    ui->SegmenttreeWidgetGOES16->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "0.47" << "0.64" << "0.86" << "1.37" << "1.6" << "2.2"
                                                  << "3.9" << "6.2" << "6.9" << "7.3" << "8.4" << "9.6" << "10.3" << "11.2" << "12.3" << "13.3");


    ui->SegmenttreeWidgetFY2E->setRootIsDecorated(false);
    ui->SegmenttreeWidgetFY2E->header()->setStretchLastSection(true);
    ui->SegmenttreeWidgetFY2E->setColumnCount(6);
    for(int i = 0; i < 6; i++)
    {
        ui->SegmenttreeWidgetFY2E->setColumnWidth(i, 200);
        ui->SegmenttreeWidgetFY2E->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    ui->SegmenttreeWidgetFY2E->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "IR1" << "IR2" << "IR3" << "IR4" << "VIS" << "VIS1KM" );

    ui->SegmenttreeWidgetFY2G->setRootIsDecorated(false);
    ui->SegmenttreeWidgetFY2G->header()->setStretchLastSection(true);
    ui->SegmenttreeWidgetFY2G->setColumnCount(6);
    for(int i = 0; i < 6; i++)
    {
        ui->SegmenttreeWidgetFY2G->setColumnWidth(i, 200);
        ui->SegmenttreeWidgetFY2G->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    ui->SegmenttreeWidgetFY2G->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "IR1" << "IR2" << "IR3" << "IR4" << "VIS" << "VIS1KM" );

    ui->SegmenttreeWidgetH8->setRootIsDecorated(false);
    ui->SegmenttreeWidgetH8->header()->setStretchLastSection(true);
    ui->SegmenttreeWidgetH8->setColumnCount(16);
    for(int i = 0; i < 16; i++)
    {
        ui->SegmenttreeWidgetH8->setColumnWidth(i, 200);
        ui->SegmenttreeWidgetH8->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    ui->SegmenttreeWidgetH8->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << "IR1" << "IR2" << "IR3" << "IR4"
          << "B04" << "B05" << "B06" << "B09" << "B10" << "B11" << "B12" << "B14" << "B16" << "VIS");

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
                //if(meteosatdir.match(filepattern, *itc))
                //qDebug() << "meteosatdir.match => " << st << " pattern = " << filepattern;

                if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G)
                {
                    if(meteosatdir.match(filepattern, *itc) && (st.mid(40, 3) == spectrumvector.at(0) || st.mid(40, 3) == spectrumvector.at(1) || st.mid(40, 3) == spectrumvector.at(2)))
                        strlistout.append(*itc);
                } else if(whichgeo == SegmentListGeostationary::H8)
                {
                    if(meteosatdir.match(filepattern, *itc) && (st.mid(8, 3) == spectrumvector.at(0) || st.mid(8, 3) == spectrumvector.at(1) || st.mid(8, 3) == spectrumvector.at(2)))
                    {
                        QString spectrum = st.mid(8, 3);
                        strlistout.append(*itc);
                    }
                } else if(whichgeo == SegmentListGeostationary::GOES_16)
                {
                    //01234567890123456789012345678901234567890123456789
                    //OR_ABI-L1b-RadF-M4C01_G16_s20161811455312_e20161811500122_c20161811500175.nc
                    if(meteosatdir.match(filepattern, st) && (st.mid(18, 3) == spectrumvector.at(0) || st.mid(18, 3) == spectrumvector.at(1) || st.mid(18, 3) == spectrumvector.at(2)))
                    {
                        strlistout.append(st);
                    }
                }
                else
                {
                    if(meteosatdir.match(filepattern, *itc) && (st.mid(26, 6) == spectrumvector.at(0) || st.mid(26, 6) == spectrumvector.at(1) || st.mid(26, 6) == spectrumvector.at(2)))
                        strlistout.append(*itc);
                }
                itc++;
            }
        }

    //IMG_DK01B04_201510090000_001.bz2
    //L-???-??????-GOES13______-?????????-0?????___-201404181000-C_


    for (int j = 0; j < strlistout.size(); ++j)
    {
        qDebug() << QString("getGeostationarySegments out ======= %1  %2    %3").arg(imagetype).arg(j).arg(strlistout.at(j));
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
    int cnt_B04 = 0;
    int cnt_B05 = 0;
    int cnt_B06 = 0;
    int cnt_B09 = 0;
    int cnt_B10 = 0;
    int cnt_B11 = 0;
    int cnt_B12 = 0;
    int cnt_B14 = 0;
    int cnt_B16 = 0;

    int cnt_C01 = 0;
    int cnt_C02 = 0;
    int cnt_C03 = 0;
    int cnt_C04 = 0;
    int cnt_C05 = 0;
    int cnt_C06 = 0;
    int cnt_C07 = 0;
    int cnt_C08 = 0;
    int cnt_C09 = 0;
    int cnt_C10 = 0;
    int cnt_C11 = 0;
    int cnt_C12 = 0;
    int cnt_C13 = 0;
    int cnt_C14 = 0;
    int cnt_C15 = 0;
    int cnt_C16 = 0;

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
        cnt_B04 = 0;
        cnt_B05 = 0;
        cnt_B06 = 0;
        cnt_B09 = 0;
        cnt_B10 = 0;
        cnt_B11 = 0;
        cnt_B12 = 0;
        cnt_B14 = 0;
        cnt_B16 = 0;

        cnt_C01 = 0;
        cnt_C02 = 0;
        cnt_C03 = 0;
        cnt_C04 = 0;
        cnt_C05 = 0;
        cnt_C06 = 0;
        cnt_C07 = 0;
        cnt_C08 = 0;
        cnt_C09 = 0;
        cnt_C10 = 0;
        cnt_C11 = 0;
        cnt_C12 = 0;
        cnt_C13 = 0;
        cnt_C14 = 0;
        cnt_C15 = 0;
        cnt_C16 = 0;



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
            // Himawari-8
            else if (strspectrum == "IR1")
                strspectrumlist += "I";
            else if (strspectrum == "IR2")
                strspectrumlist += "I";
            else if (strspectrum == "IR3")
                strspectrumlist += "I";
            else if (strspectrum == "IR4")
                strspectrumlist += "I";
            else if (strspectrum == "B04")
                strspectrumlist += "B";
            else if (strspectrum == "B05")
                strspectrumlist += "B";
            else if (strspectrum == "B06")
                strspectrumlist += "B";
            else if (strspectrum == "B09")
                strspectrumlist += "B";
            else if (strspectrum == "B10")
                strspectrumlist += "B";
            else if (strspectrum == "B11")
                strspectrumlist += "B";
            else if (strspectrum == "B12")
                strspectrumlist += "B";
            else if (strspectrum == "B14")
                strspectrumlist += "B";
            else if (strspectrum == "B16")
                strspectrumlist += "B";
            else if (strspectrum == "VIS")
                strspectrumlist += "V";
            //GOES-16
            else if (strspectrum == "C01")
                strspectrumlist += "V";
            else if (strspectrum == "C02")
                strspectrumlist += "V";
            else if (strspectrum == "C03")
                strspectrumlist += "I";
            else if (strspectrum == "C04")
                strspectrumlist += "I";
            else if (strspectrum == "C05")
                strspectrumlist += "I";
            else if (strspectrum == "C06")
                strspectrumlist += "I";
            else if (strspectrum == "C07")
                strspectrumlist += "I";
            else if (strspectrum == "C08")
                strspectrumlist += "I";
            else if (strspectrum == "C09")
                strspectrumlist += "I";
            else if (strspectrum == "C10")
                strspectrumlist += "I";
            else if (strspectrum == "C11")
                strspectrumlist += "I";
            else if (strspectrum == "C12")
                strspectrumlist += "I";
            else if (strspectrum == "C13")
                strspectrumlist += "I";
            else if (strspectrum == "C14")
                strspectrumlist += "I";
            else if (strspectrum == "C15")
                strspectrumlist += "I";
            else if (strspectrum == "C16")
                strspectrumlist += "I";



            QMap< int, QFileInfo > mapfile;
            mapfile = mapspectrum.value(strspectrum);
            QMap< int, QFileInfo >::const_iterator citfile = mapfile.constBegin();
            strnbrlist.clear();
            while (citfile != mapfile.constEnd())
            {
                filenbr = citfile.key();
                strnbrlist.append(filenbr);
                // MET-10, MET-9, MET-8
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
                // GOES16
                else if (strspectrum == "C01")
                    cnt_C01++;
                else if (strspectrum == "C02")
                    cnt_C02++;
                else if (strspectrum == "C03")
                    cnt_C03++;
                else if (strspectrum == "C04")
                    cnt_C04++;
                else if (strspectrum == "C05")
                    cnt_C05++;
                else if (strspectrum == "C06")
                    cnt_C06++;
                else if (strspectrum == "C07")
                    cnt_C07++;
                else if (strspectrum == "C08")
                    cnt_C08++;
                else if (strspectrum == "C09")
                    cnt_C09++;
                else if (strspectrum == "C10")
                    cnt_C10++;
                else if (strspectrum == "C11")
                    cnt_C11++;
                else if (strspectrum == "C12")
                    cnt_C12++;
                else if (strspectrum == "C13")
                    cnt_C13++;
                else if (strspectrum == "C14")
                    cnt_C14++;
                else if (strspectrum == "C15")
                    cnt_C15++;
                else if (strspectrum == "C16")
                    cnt_C16++;
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
                // Himawari
                else if (strspectrum == "IR1")
                    cnt_IR1++;
                else if (strspectrum == "IR2")
                    cnt_IR2++;
                else if (strspectrum == "IR3")
                    cnt_IR3++;
                else if (strspectrum == "IR4")
                    cnt_IR4++;
                else if (strspectrum == "B04")
                    cnt_B04++;
                else if (strspectrum == "B05")
                    cnt_B05++;
                else if (strspectrum == "B06")
                    cnt_B06++;
                else if (strspectrum == "B09")
                    cnt_B09++;
                else if (strspectrum == "B10")
                    cnt_B10++;
                else if (strspectrum == "B11")
                    cnt_B11++;
                else if (strspectrum == "B12")
                    cnt_B12++;
                else if (strspectrum == "B14")
                    cnt_B14++;
                else if (strspectrum == "B16")
                    cnt_B16++;
                else if (strspectrum == "VIS")
                    cnt_VIS++;




                ++citfile;
            }
            ++citspectrum;

        }

        strlist.clear();
        //strnbrlist = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12").arg(cnt_hrv).arg(cnt_ir016).arg(cnt_ir039).arg(cnt_ir087).arg(cnt_ir097).arg(cnt_ir108).
        //        arg(cnt_ir120).arg(cnt_ir134).arg(cnt_vis006).arg(cnt_vis008).arg(cnt_wv062).arg(cnt_wv073);

        if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 2) << strspectrumlist <<
                   QString("%1").arg(cnt_hrv) << QString("%1").arg(cnt_vis006) << QString("%1").arg(cnt_vis008) <<  QString("%1").arg(cnt_ir016) << QString("%1").arg(cnt_ir039) <<
                   QString("%1").arg(cnt_wv062) << QString("%1").arg(cnt_wv073) << QString("%1").arg(cnt_ir087) << QString("%1").arg(cnt_ir097) <<
                   QString("%1").arg(cnt_ir108) << QString("%1").arg(cnt_ir120) << QString("%1").arg(cnt_ir134);
        }
        else if(whichgeo == SegmentListGeostationary::GOES_13 || whichgeo == SegmentListGeostationary::GOES_15)
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 2) << strspectrumlist <<
                   QString("%1").arg(cnt_vis008) << QString("%1").arg(cnt_ir039) << QString("%1").arg(cnt_ir087) << QString("%1").arg(cnt_ir108);
        }
        else if(whichgeo == SegmentListGeostationary::GOES_16)
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 2) << strspectrumlist <<
                   QString("%1").arg(cnt_C01) << QString("%1").arg(cnt_C02) << QString("%1").arg(cnt_C03) << QString("%1").arg(cnt_C04) <<
                   QString("%1").arg(cnt_C05) << QString("%1").arg(cnt_C06) << QString("%1").arg(cnt_C07) << QString("%1").arg(cnt_C08) <<
                   QString("%1").arg(cnt_C09) << QString("%1").arg(cnt_C10) << QString("%1").arg(cnt_C11) << QString("%1").arg(cnt_C12) <<
                   QString("%1").arg(cnt_C13) << QString("%1").arg(cnt_C14) << QString("%1").arg(cnt_C15) << QString("%1").arg(cnt_C16);
        }
        else if(whichgeo == SegmentListGeostationary::FY2E || whichgeo == SegmentListGeostationary::FY2G )
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 2) << strspectrumlist <<
                   QString("%1").arg(cnt_IR1) << QString("%1").arg(cnt_IR2) << QString("%1").arg(cnt_IR3) << QString("%1").arg(cnt_IR4)
                    << QString("%1").arg(cnt_VIS) << QString("%1").arg(cnt_VIS1KM);
        }
        else if(whichgeo == SegmentListGeostationary::H8 )
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 1) + "0" << strspectrumlist <<
                   QString("%1").arg(cnt_IR1) << QString("%1").arg(cnt_IR2) << QString("%1").arg(cnt_IR3) << QString("%1").arg(cnt_IR4)
                       << QString("%1").arg(cnt_B04) << QString("%1").arg(cnt_B05) << QString("%1").arg(cnt_B06) << QString("%1").arg(cnt_B09) << QString("%1").arg(cnt_B10) << QString("%1").arg(cnt_B11)
                       << QString("%1").arg(cnt_B12) << QString("%1").arg(cnt_B14) << QString("%1").arg(cnt_B16) << QString("%1").arg(cnt_VIS);
        }

        newitem = new QTreeWidgetItem( widget, strlist, 0  );
        if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_8)
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
        else if (whichgeo == SegmentListGeostationary::GOES_13 || whichgeo == SegmentListGeostationary::GOES_15)
        {
            if (cnt_vis008 == 7 && cnt_ir039 == 7 && cnt_ir087 == 7 && cnt_ir108 == 7 )
                col.setRgb(174, 225, 184);
            else
                col.setRgb(225, 171, 196);
        }
        else if (whichgeo == SegmentListGeostationary::GOES_16)
        {
            if (cnt_C01 == 1 && cnt_C02 == 1 && cnt_C03 == 1 && cnt_C04 == 1 && cnt_C05 == 1 && cnt_C06 == 1 && cnt_C07 == 1 &&
                cnt_C08 == 1 && cnt_C09 == 1 && cnt_C10 == 1 && cnt_C11 == 1 && cnt_C12 == 1 && cnt_C13 == 1 && cnt_C14 == 1 &&
                cnt_C15 == 1 && cnt_C16 == 1 )
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
        else if (whichgeo == SegmentListGeostationary::H8)
        {
            if (cnt_IR1 == 10 && cnt_IR2 == 10 && cnt_IR3 == 10 && cnt_IR4 == 10 && cnt_VIS == 10
                    && cnt_B04 == 10 && cnt_B05 == 10 && cnt_B06 == 10 && cnt_B09 == 10 && cnt_B10 == 10 && cnt_B11 == 10 && cnt_B12 == 10 && cnt_B14 == 10 && cnt_B16 == 10)
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
        newitem->setBackgroundColor( 14, col );
        newitem->setBackgroundColor( 15, col );
        newitem->setBackgroundColor( 16, col );
        newitem->setBackgroundColor( 17, col );

        ++citdate;
    }


}

void FormGeostationary::PopulateTree()
{

    qDebug() << "FormGeostationary::PopulateTree()";
    PopulateTreeGeo(SegmentListGeostationary::MET_10, segs->segmentlistmapmeteosat, ui->SegmenttreeWidget);
    PopulateTreeGeo(SegmentListGeostationary::MET_9, segs->segmentlistmapmeteosatrss, ui->SegmenttreeWidgetRSS);
    PopulateTreeGeo(SegmentListGeostationary::MET_8, segs->segmentlistmapmet8, ui->SegmenttreeWidgetMet8);
    PopulateTreeGeo(SegmentListGeostationary::GOES_13, segs->segmentlistmapgoes13dc3, ui->SegmenttreeWidgetGOES13dc3);
    PopulateTreeGeo(SegmentListGeostationary::GOES_13, segs->segmentlistmapgoes13dc4, ui->SegmenttreeWidgetGOES13dc4);
    PopulateTreeGeo(SegmentListGeostationary::GOES_15, segs->segmentlistmapgoes15dc3, ui->SegmenttreeWidgetGOES15dc3);
    PopulateTreeGeo(SegmentListGeostationary::GOES_15, segs->segmentlistmapgoes15dc4, ui->SegmenttreeWidgetGOES15dc4);
    PopulateTreeGeo(SegmentListGeostationary::GOES_16, segs->segmentlistmapgoes16, ui->SegmenttreeWidgetGOES16);
    PopulateTreeGeo(SegmentListGeostationary::FY2E, segs->segmentlistmapfy2e, ui->SegmenttreeWidgetFY2E);
    PopulateTreeGeo(SegmentListGeostationary::FY2G, segs->segmentlistmapfy2g, ui->SegmenttreeWidgetFY2G);
    PopulateTreeGeo(SegmentListGeostationary::H8, segs->segmentlistmaph8, ui->SegmenttreeWidgetH8);

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
    QList<QTreeWidgetItem *> treewidgetselectedmet8 = ui->SegmenttreeWidgetMet8->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedgoes13dc3 = ui->SegmenttreeWidgetGOES13dc3->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedgoes13dc4 = ui->SegmenttreeWidgetGOES13dc4->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedgoes15dc3 = ui->SegmenttreeWidgetGOES15dc3->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedgoes15dc4 = ui->SegmenttreeWidgetGOES15dc4->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedgoes16 = ui->SegmenttreeWidgetGOES16->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedfy2e = ui->SegmenttreeWidgetFY2E->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedfy2g = ui->SegmenttreeWidgetFY2G->selectedItems();
    QList<QTreeWidgetItem *> treewidgetselectedh8 = ui->SegmenttreeWidgetH8->selectedItems();

    SegmentListGeostationary *sl;

    if(treewidgetselected.size() > 0)
    {
        sl = segs->seglmeteosat;
        sl->bActiveSegmentList = true;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        segs->seglmet8->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglgoes16->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        segs->seglh8->bActiveSegmentList = false;

        tex = (*treewidgetselected[0]).text(0);
        formtoolbox->createFilenamestring("MET-10", tex, spectrumvector);
    }
    else if(treewidgetselectedrss.size() > 0)
    {
        sl = segs->seglmeteosatrss;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmet8->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglgoes16->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        segs->seglh8->bActiveSegmentList = false;

        tex = (*treewidgetselectedrss[0]).text(0);
        formtoolbox->createFilenamestring("MET-9", tex, spectrumvector);
    }
    else if(treewidgetselectedmet8.size() > 0)
    {
        sl = segs->seglmet8;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglgoes16->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        segs->seglh8->bActiveSegmentList = false;

        tex = (*treewidgetselectedmet8[0]).text(0);
        formtoolbox->createFilenamestring("MET-8", tex, spectrumvector);
    }
    else if(treewidgetselectedgoes13dc3.size() > 0)
    {
        sl = segs->seglgoes13dc3;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        segs->seglmet8->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglgoes16->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        segs->seglh8->bActiveSegmentList = false;

        tex = (*treewidgetselectedgoes13dc3[0]).text(0);
        formtoolbox->createFilenamestring("GOES-13", tex, spectrumvector);
    }
    else if(treewidgetselectedgoes15dc3.size() > 0)
    {
        sl = segs->seglgoes15dc3;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        segs->seglmet8->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglgoes16->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        segs->seglh8->bActiveSegmentList = false;

        tex = (*treewidgetselectedgoes15dc3[0]).text(0);
        formtoolbox->createFilenamestring("GOES-15", tex, spectrumvector);
    }
    else if(treewidgetselectedgoes13dc4.size() > 0)
    {
        sl = segs->seglgoes13dc4;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        segs->seglmet8->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglgoes16->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        segs->seglh8->bActiveSegmentList = false;

        tex = (*treewidgetselectedgoes13dc4[0]).text(0);
        formtoolbox->createFilenamestring("GOES-13", tex, spectrumvector);
    }
    else if(treewidgetselectedgoes15dc4.size() > 0)
    {
        sl = segs->seglgoes15dc4;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        segs->seglmet8->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglgoes16->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        segs->seglh8->bActiveSegmentList = false;

        tex = (*treewidgetselectedgoes15dc4[0]).text(0);
        formtoolbox->createFilenamestring("GOES-15", tex, spectrumvector);
    }
    else if(treewidgetselectedgoes16.size() > 0)
    {
        sl = segs->seglgoes16;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        segs->seglmet8->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        segs->seglh8->bActiveSegmentList = false;

        tex = (*treewidgetselectedgoes16[0]).text(0);
        formtoolbox->createFilenamestring("GOES-16", tex, spectrumvector);
    }
    else if(treewidgetselectedfy2e.size() > 0)
    {
        sl = segs->seglfy2e;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        segs->seglmet8->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglgoes16->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;
        segs->seglh8->bActiveSegmentList = false;

        tex = (*treewidgetselectedfy2e[0]).text(0);
        formtoolbox->createFilenamestring("FY2E", tex, spectrumvector);
    }
    else if(treewidgetselectedfy2g.size() > 0)
    {
        sl = segs->seglfy2g;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        segs->seglmet8->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglgoes16->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglh8->bActiveSegmentList = false;

        tex = (*treewidgetselectedfy2g[0]).text(0);
        formtoolbox->createFilenamestring("FY2G", tex, spectrumvector);
    }
    else if(treewidgetselectedh8.size() > 0)
    {
        sl = segs->seglh8;
        sl->bActiveSegmentList = true;
        segs->seglmeteosat->bActiveSegmentList = false;
        segs->seglmeteosatrss->bActiveSegmentList = false;
        segs->seglmet8->bActiveSegmentList = false;
        segs->seglgoes13dc3->bActiveSegmentList = false;
        segs->seglgoes15dc3->bActiveSegmentList = false;
        segs->seglgoes13dc4->bActiveSegmentList = false;
        segs->seglgoes15dc4->bActiveSegmentList = false;
        segs->seglgoes16->bActiveSegmentList = false;
        segs->seglfy2e->bActiveSegmentList = false;
        segs->seglfy2g->bActiveSegmentList = false;

        tex = (*treewidgetselectedh8[0]).text(0);
        formtoolbox->createFilenamestring("H8", tex, spectrumvector);
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
        if(sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_8)
            imageptrs->InitializeImageGeostationary(3712, 3712);
        else if(sl->getGeoSatellite() == SegmentListGeostationary::MET_9)
            imageptrs->InitializeImageGeostationary(3712, 464*3);
        else if(sl->getGeoSatellite() == SegmentListGeostationary::GOES_13)
            imageptrs->InitializeImageGeostationary(2816, 464*7);
        else if(sl->getGeoSatellite() == SegmentListGeostationary::GOES_15)
            imageptrs->InitializeImageGeostationary(2816, 464*7);
//        else if(sl->getGeoSatellite() == SegmentListGeostationary::GOES_16)
//            imageptrs->InitializeImageGeostationary(10848, 10848);
        else if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
            imageptrs->InitializeImageGeostationary(2288, 2288);
        else if(sl->getGeoSatellite() == SegmentListGeostationary::H8)
            imageptrs->InitializeImageGeostationary(5500, 5500);
    }

    formimage->displayImage(IMAGE_GEOSTATIONARY);
    //formimage->adjustPicSize(true);

    qDebug() << QString("FormGeostationary::CreateGeoImage kind = %1 areatype = %2").arg(type).arg(sl->areatype);

    if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
        CreateGeoImageHDF(sl, type, tex, spectrumvector, inversevector);
    else if(sl->getGeoSatellite() == SegmentListGeostationary::GOES_16)
        CreateGeoImagenetCDF(sl, type, tex, spectrumvector, inversevector);
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


    QApplication::setOverrideCursor(( Qt::WaitCursor));

    SegmentListGeostationary::eGeoSatellite whichgeo = sl->getGeoSatellite();

    filetiming = tex.mid(0, 4) + tex.mid(5, 2) + tex.mid(8, 2) + tex.mid(13, 2) + tex.mid(16, 2);
    filedate = tex.mid(0, 4) + tex.mid(5, 2) + tex.mid(8, 2);
    //filepattern1 = "H-???-??????-MSG?_???____-??????___-0?????___-" + filetiming + "-C_";
    //filepattern1 = "H-000-MSG1__-MSG1_RSS____-IR_016___-000007___-201310271420-C_";
    //filepattern1 = "H-000-MSG1__-MSG1_RSS____-_________-EPI______-201204101155-__"
    //H-000-MSG1__-MSG1_RSS____-IR_016___-000007___-201310271420-C_
    //H-000-GOMS1_-GOMS1_4_____-00_9_076E-000001___-201312231100-C_
    //L-000-MSG3__-GOES13______-00_7_075W-000001___-201404041200-C_
    //IMG_DK01B04_201510090000_001.bz2
    //0123456789012345678901234567890

    //    filepattern1 = QString("H-???-??????-????????____-?????????-0?????___-") + filetiming + QString("-C_");
    //  filepattern1 = QString("H-???-??????-MSG?_???____-??????___-0?????___-") + filetiming + QString("-C_");

    if(whichgeo == SegmentListGeostationary::GOES_13)
        filepattern = QString("L-???") + QString("-??????") + QString("-GOES13*") + filetiming + QString("-C_");
    else if(whichgeo == SegmentListGeostationary::GOES_15 )
        filepattern = QString("L-???") + QString("-??????") + QString("-GOES15*") + filetiming + QString("-C_");
    else if(whichgeo == SegmentListGeostationary::H8 )
        filepattern = QString("IMG_DK01") + QString("???_") + filetiming.mid(0, 11) + "?*";
    else
        filepattern = QString("H-???") + QString("-??????") + QString("-?????????___-?????????") + QString("-0?????___-") + filetiming + QString("-C_");
    qDebug() << "FormGeostationary::CreateGeoImage filepattern = " << filepattern;

    //H-000-MSG1__-MSG1_IODC___-WV_073___-000005___-201610061245-C_
    //H-???-??????-?????????___-?????????-0?????___-201610061100-C_


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
        MsgFileAccess fa;
        QString epiloguefile;
        MSG_header EPI_head;


        if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8 ||
                whichgeo == SegmentListGeostationary::GOES_13 || whichgeo == SegmentListGeostationary::GOES_15 )
        {

            fa = (type == "VIS_IR" || type == "VIS_IR Color" ? faVIS_IR : faHRV);

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

            epiloguefile = fa.epilogueFile();

            qDebug() << QString("Reading epilogue file = %1").arg(epiloguefile);

        }

        if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
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

//        try
//        {
////            da.read_file(fa.directory + "/" + fa.segmentFiles().at(0),header);
//            da.read_file(fa.directory + "/" + (type == "VIS_IR" || type == "VIS_IR Color" ? llVIS_IR.at(0) : llHRV.at(0)),header);
//            lCFAC = (long)header.image_navigation->CFAC*180.0/PI;
//            lLFAC = (long)header.image_navigation->LFAC*180.0/PI;
//            sl->CFAC = abs(lCFAC);
//            sl->COFF = abs(header.image_navigation->COFF);
//            sl->LFAC = abs(lLFAC);
//            sl->LOFF = abs(header.image_navigation->LOFF);
//            qDebug() << QString("CFAC = %1 COFF = %2 LFAC = %3 LOFF = %4").arg(sl->CFAC).arg(sl->COFF).arg(sl->LFAC).arg(sl->LOFF);

//        }
//        catch( std::runtime_error &run )
//        {
//            qDebug() << QString("Error : runtime error in reading first segmentfile : %1").arg(run.what());
//        }


        if(whichgeo == SegmentListGeostationary::MET_10 || whichgeo == SegmentListGeostationary::MET_9 || whichgeo == SegmentListGeostationary::MET_8)
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
        else if(whichgeo == SegmentListGeostationary::GOES_13 || whichgeo == SegmentListGeostationary::GOES_15)
        {
            sl->COFF = COFF_NONHRV_GOES;
            sl->LOFF = LOFF_NONHRV_GOES;
            sl->CFAC = CFAC_NONHRV_GOES;
            sl->LFAC = LFAC_NONHRV_GOES;
        }
        else if(whichgeo == SegmentListGeostationary::H8)
        {
            sl->COFF = COFF_NONHRV_H8;
            sl->LOFF = LOFF_NONHRV_H8;
            sl->CFAC = CFAC_NONHRV_H8;
            sl->LFAC = LFAC_NONHRV_H8;
        }

        if(type == "VIS_IR" || type == "VIS_IR Color" || type == "HRV Color")
        {
            for (int j =  0; j < llVIS_IR.size(); ++j)
            {
                QFile file(sl->getImagePath() + "/" + llVIS_IR.at(j));
                QFileInfo fileinfo(file);
                //IMG_DK01B04_201510090000_001.bz2
                //0123456789012345678901234567890

                if(sl->getGeoSatellite() == SegmentListGeostationary::H8)
                {
                    filesequence = fileinfo.fileName().mid(25, 3).toInt()-1;
                    filespectrum = fileinfo.fileName().mid(8, 3);
                    filedate = fileinfo.fileName().mid(12, 11) + "0";
                }
                else
                {
                    filesequence = fileinfo.fileName().mid(36, 6).toInt()-1;
                    filespectrum = fileinfo.fileName().mid(26, 6);
                    filedate = fileinfo.fileName().mid(46, 12);

                }
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

    sl->InsertPresent( spectrumvector, "", 0);

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

void FormGeostationary::CreateGeoImagenetCDF(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector)
{
//    QString filepath = "OR_ABI" + tex;
//    if(type == "VIS_IR")
//    {
//        sl->ComposeImagenetCDFInThread(filepath, spectrumvector, inversevector);
//    }


    QString filetiming;
    QString filedate;
    QStringList llVIS_IR;
    QString filepattern;

    SegmentListGeostationary::eGeoSatellite whichgeo = sl->getGeoSatellite();

    sl->COFF = COFF_NONHRV_GOES16;
    sl->LOFF = LOFF_NONHRV_GOES16;
    sl->CFAC = CFAC_NONHRV_GOES16;
    sl->LFAC = LFAC_NONHRV_GOES16;

    qDebug() << "====> tex = " << tex;

    filetiming = tex.mid(0, 4) + tex.mid(8, 2) + tex.mid(5, 2);
    filedate = tex.mid(0, 4) + tex.mid(5, 2) + tex.mid(8, 2);

    //OR_ABI-L1b-RadF-M4C01_G16_s20161811455312_e20161811500122_c20161811500175.nc
    //01234567890123456789012345678901234567890123456789
    if(whichgeo == SegmentListGeostationary::GOES_16 && (type == "VIS_IR" || type == "VIS_IR Color"))
        filepattern = QString("OR_ABI-L1b-RadF-M4???_G16_s") + filetiming + QString("*.nc");
    else
        return;

    sl->InsertPresent( spectrumvector, "", 0);

    if(type == "VIS_IR" || type == "VIS_IR Color")
    {
        llVIS_IR = this->getGeostationarySegments(whichgeo, "VIS_IR", sl->getImagePath(), spectrumvector, filepattern);
        qDebug() << QString("llVIS_IR count = %1").arg(llVIS_IR.count());
        for (int j =  0; j < llVIS_IR.size(); ++j)
        {
            qDebug() << QString("llVIS_IR at %1 = %2 spectrumvector = %3").arg(j).arg(llVIS_IR.at(j)).arg(spectrumvector.at(j));
        }
        sl->ComposeImagenetCDFInThread(llVIS_IR, spectrumvector, inversevector);
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

void FormGeostationary::SelectGeoWidgetItem(SegmentListGeostationary::eGeoTreeWidget geowidget, QTreeWidgetItem *item, int column )
{

    SegmentListGeostationary::eGeoSatellite geosat;
    QString geostring;

    if ( !item )
          return;

    setTreeWidget(ui->SegmenttreeWidget, geowidget == SegmentListGeostationary::TREEWIDGET_MET_10 ? true : false);
    setTreeWidget(ui->SegmenttreeWidgetRSS, geowidget == SegmentListGeostationary::TREEWIDGET_MET_9 ? true : false);
    setTreeWidget(ui->SegmenttreeWidgetMet8, geowidget == SegmentListGeostationary::TREEWIDGET_MET_8 ? true : false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc3, geowidget == SegmentListGeostationary::TREEWIDGET_GOES_13DC3 ? true : false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc3, geowidget == SegmentListGeostationary::TREEWIDGET_GOES_15DC3 ? true : false);
    setTreeWidget(ui->SegmenttreeWidgetGOES13dc4, geowidget == SegmentListGeostationary::TREEWIDGET_GOES_13DC4 ? true : false);
    setTreeWidget(ui->SegmenttreeWidgetGOES15dc4, geowidget == SegmentListGeostationary::TREEWIDGET_GOES_15DC4 ? true : false);
    setTreeWidget(ui->SegmenttreeWidgetGOES16, geowidget == SegmentListGeostationary::TREEWIDGET_GOES_16 ? true : false);
    setTreeWidget(ui->SegmenttreeWidgetFY2E, geowidget == SegmentListGeostationary::TREEWIDGET_FY2E ? true : false);
    setTreeWidget(ui->SegmenttreeWidgetFY2G, geowidget == SegmentListGeostationary::TREEWIDGET_FY2G ? true : false);
    setTreeWidget(ui->SegmenttreeWidgetH8, geowidget == SegmentListGeostationary::TREEWIDGET_H8 ? true : false);

    switch(geowidget)
    {
    case SegmentListGeostationary::TREEWIDGET_MET_10:
        geosat = SegmentListGeostationary::MET_10;
        geostring = "MET-10";
        break;
    case SegmentListGeostationary::TREEWIDGET_MET_9:
        geosat = SegmentListGeostationary::MET_9;
        geostring = "MET-9";
        break;
    case SegmentListGeostationary::TREEWIDGET_MET_8:
        geosat = SegmentListGeostationary::MET_8;
        geostring = "MET-8";
        break;
    case SegmentListGeostationary::TREEWIDGET_GOES_13DC3:
        geosat = SegmentListGeostationary::GOES_13;
        geostring = "GOES-13";
        break;
    case SegmentListGeostationary::TREEWIDGET_GOES_13DC4:
        geosat = SegmentListGeostationary::GOES_13;
        geostring = "GOES-13";
        break;
    case SegmentListGeostationary::TREEWIDGET_GOES_15DC3:
        geosat = SegmentListGeostationary::GOES_15;
        geostring = "GOES-15";
        break;
    case SegmentListGeostationary::TREEWIDGET_GOES_15DC4:
        geosat = SegmentListGeostationary::GOES_15;
        geostring = "GOES-15";
        break;
    case SegmentListGeostationary::TREEWIDGET_GOES_16:
        geosat = SegmentListGeostationary::GOES_16;
        geostring = "GOES-16";
        break;
    case SegmentListGeostationary::TREEWIDGET_FY2E:
        geosat = SegmentListGeostationary::FY2E;
        geostring = "FY2E";
        break;
    case SegmentListGeostationary::TREEWIDGET_FY2G:
        geosat = SegmentListGeostationary::FY2G;
        geostring = "FY2G";
        break;
    case SegmentListGeostationary::TREEWIDGET_H8:
        geosat = SegmentListGeostationary::H8;
        geostring = "H8";
        break;
    }

    qDebug() << geostring + " " + (*item).text(0);

    segs->seglmeteosat->setKindofImage("");

    QStringList tex;
    tex << geostring;
    for(int i = 0; i < item->columnCount(); i++)
        tex << (*item).text(i);
    emit geostationarysegmentschosen(geosat, tex);

}

void FormGeostationary::on_SegmenttreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    SelectGeoWidgetItem(SegmentListGeostationary::TREEWIDGET_MET_10, item, column);
}

void FormGeostationary::on_SegmenttreeWidgetRSS_itemClicked(QTreeWidgetItem *item, int column)
{
    SelectGeoWidgetItem(SegmentListGeostationary::TREEWIDGET_MET_9, item, column);
}

void FormGeostationary::on_SegmenttreeWidgetMet8_itemClicked(QTreeWidgetItem *item, int column)
{
    SelectGeoWidgetItem(SegmentListGeostationary::TREEWIDGET_MET_8, item, column);
}

void FormGeostationary::on_SegmenttreeWidgetGOES13dc3_itemClicked(QTreeWidgetItem *item, int column)
{
    SelectGeoWidgetItem(SegmentListGeostationary::TREEWIDGET_GOES_13DC3, item, column);
}

void FormGeostationary::on_SegmenttreeWidgetGOES15dc3_itemClicked(QTreeWidgetItem *item, int column)
{
    SelectGeoWidgetItem(SegmentListGeostationary::TREEWIDGET_GOES_15DC3, item, column);
}

void FormGeostationary::on_SegmenttreeWidgetGOES13dc4_itemClicked(QTreeWidgetItem *item, int column)
{
    SelectGeoWidgetItem(SegmentListGeostationary::TREEWIDGET_GOES_13DC4, item, column);
}

void FormGeostationary::on_SegmenttreeWidgetGOES15dc4_itemClicked(QTreeWidgetItem *item, int column)
{
    SelectGeoWidgetItem(SegmentListGeostationary::TREEWIDGET_GOES_15DC4, item, column);
}

void FormGeostationary::on_SegmenttreeWidgetGOES16_itemClicked(QTreeWidgetItem *item, int column)
{
    SelectGeoWidgetItem(SegmentListGeostationary::TREEWIDGET_GOES_16, item, column);
}

void FormGeostationary::on_SegmenttreeWidgetFY2E_itemClicked(QTreeWidgetItem *item, int column)
{
    SelectGeoWidgetItem(SegmentListGeostationary::TREEWIDGET_FY2E, item, column);
}

void FormGeostationary::on_SegmenttreeWidgetFY2G_itemClicked(QTreeWidgetItem *item, int column)
{
    SelectGeoWidgetItem(SegmentListGeostationary::TREEWIDGET_FY2G, item, column);
}

void FormGeostationary::on_SegmenttreeWidgetH8_itemClicked(QTreeWidgetItem *item, int column)
{
    SelectGeoWidgetItem(SegmentListGeostationary::TREEWIDGET_H8, item, column);
}
