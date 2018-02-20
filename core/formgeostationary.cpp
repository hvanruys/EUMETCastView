#include "formgeostationary.h"
#include "ui_formgeostationary.h"
#include <MSG_HRIT.h>
#include "pixgeoconversion.h"
#include "qsgp4date.h"

#include <QByteArray>
#include "msgfileaccess.h"
#include "msgdataaccess.h"
#include "internal.h"
#include "nav_util.h"
#include "qsgp4date.h"

#define REDCHANNEL 0
#define GREENCHANNEL 1
#define BLUECHANNEL 2

extern SegmentImage *imageptrs;
extern Options opts;

FormGeostationary::FormGeostationary(QWidget *parent, SatelliteList *satlist, AVHRRSatellite *seglist) :
    QWidget(parent),
    ui(new Ui::FormGeostationary)
{
    ui->setupUi(this);
    segs = seglist;
    sats = satlist;

    qDebug() << "in constructor FormGeostationary";

    for(int i = 0; i < opts.geosatellites.count(); i++)
    {
        QWidget *mywidget = new QWidget();
        ui->tabGeostationary->addTab(mywidget,opts.geosatellites.at(i).fullname + " : " + QString("%1").arg(opts.geosatellites.at(i).longitude) + "°");
        QTreeWidget *treeWidget = new QTreeWidget;

        geotreewidgetlist.append(treeWidget);
        treeWidget->setRootIsDecorated(false);
        treeWidget->header()->setStretchLastSection(true);
        treeWidget->setColumnCount(opts.geosatellites.at(i).spectrumlist.count() + 2);
        QStringList header;
        int columnsheader;
        if(opts.geosatellites.at(i).spectrumhrv.length() == 0)
        {
            header << opts.geosatellites.at(i).spectrumvalueslist;
            columnsheader = opts.geosatellites.at(i).spectrumlist.count() + 2;
        }
        else
        {
            header << opts.geosatellites.at(i).spectrumhrv << opts.geosatellites.at(i).spectrumvalueslist;
            columnsheader = opts.geosatellites.at(i).spectrumlist.count() + 3;
        }

        treeWidget->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << header );
        treeWidget->setColumnWidth(0, 150);
        treeWidget->setColumnWidth(1, 150);

        for(int i = 2; i < columnsheader; i++)
        {
            treeWidget->setColumnWidth(i, 40);
        }

        for(int i = 0; i < columnsheader; i++)
        {
            treeWidget->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
        }


        connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(ontreeWidgetitemClicked(QTreeWidgetItem *, int)));
        QVBoxLayout *mylayout = new QVBoxLayout;
        mylayout->addWidget(treeWidget);
        mywidget->setLayout(mylayout);
    }

    if(opts.geosatellites.count() > 0)
        ui->tabGeostationary->setCurrentIndex(0);

}

QStringList FormGeostationary::getGeostationarySegments(int geoindex, const QString imagetype, const QString filepath, QVector<QString> spectrumvector, QString filepattern)
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
            if(meteosatdir.match(filepattern, *itc) &&
                    st.mid(opts.geosatellites.at(geoindex).indexspectrumhrv, opts.geosatellites.at(geoindex).spectrumhrv.length()) == opts.geosatellites.at(geoindex).spectrumhrv)
                strlistout.append(*itc);
            itc++;
        }
    }
    else if(imagetype == "VIS_IR")
    {
        while( itc != strlist.end() )
        {
            QString st = *itc;
            QString filespectrum = st.mid(opts.geosatellites.at(geoindex).indexspectrum, opts.geosatellites.at(geoindex).spectrumlist.at(0).length());
            if(meteosatdir.match(filepattern, *itc) &&
                    (filespectrum == spectrumvector.at(0) || filespectrum == spectrumvector.at(1) || filespectrum == spectrumvector.at(2)))
                strlistout.append(*itc);
            itc++;
        }
    }

    for (int j = 0; j < strlistout.size(); ++j)
    {
        qDebug() << QString("getGeostationarySegments out ======= %1  %2    %3").arg(imagetype).arg(j).arg(strlistout.at(j));
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

void FormGeostationary::PopulateTree()
{

    qDebug() << "FormGeostationary::PopulateTree()";

    for(int i = 0; i < opts.geosatellites.count(); i++)
        PopulateTreeGeo(i);

}

void FormGeostationary::PopulateTreeGeo(int geoindex)
{

    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > map;
    map = segs->segmentlistmapgeo.at(geoindex);
    QTreeWidget *widget;
    widget = geotreewidgetlist.at(geoindex);

    QStringList strlist;
    //QString strnbrlist;
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
            if (strspectrum == "HRV")
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
            else if (strspectrum == "03_8_0")
                strspectrumlist += "I";
            else if (strspectrum == "08_0_0")
                strspectrumlist += "I";
            else if (strspectrum == "09_7_0")
                strspectrumlist += "I";
            else if (strspectrum == "10_7_0")
                strspectrumlist += "I";
            else if (strspectrum == "11_9_0")
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
            //strnbrlist.clear();
            while (citfile != mapfile.constEnd())
            {
                filenbr = citfile.key();
                //strnbrlist.append(filenbr);
                // MET-10, MET-9, MET-8
                if (strspectrum == "HRV")
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
                else if (strspectrum == "00_7")
                    cnt_vis008++;
                else if (strspectrum == "03_9")
                    cnt_ir039++;
                else if (strspectrum == "06_6")
                    cnt_ir087++;
                else if (strspectrum == "10_7")
                    cnt_ir108++;
                // GOES15
                else if (strspectrum == "00_7")
                    cnt_vis008++;
                else if (strspectrum == "03_9")
                    cnt_ir039++;
                else if (strspectrum == "06_6")
                    cnt_ir087++;
                else if (strspectrum == "10_7")
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
                // GOMS2
                else if (strspectrum == "00_9_0")
                    cnt_vis008++;
                else if (strspectrum == "03_8_0")
                    cnt_ir039++;
                else if (strspectrum == "08_0_0")
                    cnt_ir087++;
                else if (strspectrum == "09_7_0")
                    cnt_ir097++;
                else if (strspectrum == "10_7_0")
                    cnt_ir108++;
                else if (strspectrum == "11_9_0")
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

        if(geoindex == (int)eGeoSatellite::MET_11 || geoindex == (int)eGeoSatellite::MET_10 || geoindex == (int)eGeoSatellite::MET_9 || geoindex == (int)eGeoSatellite::MET_8)
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 2) << strspectrumlist <<
                   QString("%1").arg(cnt_hrv) << QString("%1").arg(cnt_vis006) << QString("%1").arg(cnt_vis008) <<  QString("%1").arg(cnt_ir016) << QString("%1").arg(cnt_ir039) <<
                   QString("%1").arg(cnt_wv062) << QString("%1").arg(cnt_wv073) << QString("%1").arg(cnt_ir087) << QString("%1").arg(cnt_ir097) <<
                   QString("%1").arg(cnt_ir108) << QString("%1").arg(cnt_ir120) << QString("%1").arg(cnt_ir134);
        }
        else if(geoindex == (int)eGeoSatellite::GOMS2)
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 2) << strspectrumlist <<
                   QString("%1").arg(cnt_vis008) << QString("%1").arg(cnt_ir039) << QString("%1").arg(cnt_ir087) << QString("%1").arg(cnt_ir097) << QString("%1").arg(cnt_ir108) << QString("%1").arg(cnt_ir120);
        }
        else if(geoindex == (int)eGeoSatellite::FY2E || geoindex == (int)eGeoSatellite::FY2G)
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 2) << strspectrumlist <<
                   QString("%1").arg(cnt_VIS1KM) << QString("%1").arg(cnt_VIS) << QString("%1").arg(cnt_IR4) << QString("%1").arg(cnt_IR3) << QString("%1").arg(cnt_IR1) << QString("%1").arg(cnt_IR2);

        }
        else if(geoindex == (int)eGeoSatellite::GOES_15)
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 2) << strspectrumlist <<
                   QString("%1").arg(cnt_vis008) << QString("%1").arg(cnt_ir039) << QString("%1").arg(cnt_ir087) << QString("%1").arg(cnt_ir108);
        }
        else if(geoindex == (int)eGeoSatellite::GOES_16)
        {
            // GOES-16
//            int yyyy = strdate.mid(0, 4).toInt();
//            int day = strdate.mid(4, 3).toInt();
//            QDate now(yyyy, 1, 1);
//            QDate newnow = now.addDays(day - 1);

//            strlist << strdate.mid(0,4) + "-" + QString("%1").arg(newnow.month()).rightJustified(2, '0') + "-" + QString("%1").arg(newnow.day()).rightJustified(2, '0') + "   " + strdate.mid(7,2) + ":" + strdate.mid(9, 2) << strspectrumlist <<
            strlist << strdate.mid(0, 4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8, 2) + ":" + strdate.mid(10, 2) << strspectrumlist <<
                   QString("%1").arg(cnt_C01) << QString("%1").arg(cnt_C02) << QString("%1").arg(cnt_C03) << QString("%1").arg(cnt_C04) <<
                   QString("%1").arg(cnt_C05) << QString("%1").arg(cnt_C06) << QString("%1").arg(cnt_C07) << QString("%1").arg(cnt_C08) <<
                   QString("%1").arg(cnt_C09) << QString("%1").arg(cnt_C10) << QString("%1").arg(cnt_C11) << QString("%1").arg(cnt_C12) <<
                   QString("%1").arg(cnt_C13) << QString("%1").arg(cnt_C14) << QString("%1").arg(cnt_C15) << QString("%1").arg(cnt_C16);
        }
        else if(geoindex == (int)eGeoSatellite::H8)
        {
            strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 1) + "0" << strspectrumlist <<
                   QString("%1").arg(cnt_IR1) << QString("%1").arg(cnt_IR2) << QString("%1").arg(cnt_IR3) << QString("%1").arg(cnt_IR4)
                       << QString("%1").arg(cnt_B04) << QString("%1").arg(cnt_B05) << QString("%1").arg(cnt_B06) << QString("%1").arg(cnt_B09) << QString("%1").arg(cnt_B10) << QString("%1").arg(cnt_B11)
                       << QString("%1").arg(cnt_B12) << QString("%1").arg(cnt_B14) << QString("%1").arg(cnt_B16) << QString("%1").arg(cnt_VIS);
        }

        newitem = new QTreeWidgetItem( widget, strlist, 0  );
        if(geoindex == (int)eGeoSatellite::MET_11 || geoindex == (int)eGeoSatellite::MET_10 || geoindex == (int)eGeoSatellite::MET_8)
        {
            if (cnt_hrv == 24 && cnt_ir016 == 8 && cnt_ir039 == 8 && cnt_ir087 == 8 && cnt_ir097 == 8 && cnt_ir108 == 8 && cnt_ir120 == 8 && cnt_ir134 == 8 && cnt_vis006 == 8
                && cnt_vis008 == 8 && cnt_wv062 == 8 && cnt_wv073 == 8)
                col.setRgb(174, 225, 184);
            else
                col.setRgb(225, 171, 196);
        }
        else if(geoindex == (int)eGeoSatellite::MET_9)
        {
            if (cnt_hrv == 9 && cnt_ir016 == 3 && cnt_ir039 == 3 && cnt_ir087 == 3 && cnt_ir097 == 3 && cnt_ir108 == 3 && cnt_ir120 == 3 && cnt_ir134 == 3 && cnt_vis006 == 3
                && cnt_vis008 == 3 && cnt_wv062 == 3 && cnt_wv073 == 3)
                col.setRgb(174, 225, 184);
            else
                col.setRgb(225, 171, 196);
        }
        else if (geoindex == (int)eGeoSatellite::GOMS2)
        {
            if (cnt_vis008 == 6 && cnt_ir039 == 6 && cnt_ir087 == 6 && cnt_ir097 == 6 && cnt_ir108 == 6 && cnt_ir120 == 6 )
                col.setRgb(174, 225, 184);
            else
                col.setRgb(225, 171, 196);
        }
        else if (geoindex == (int)eGeoSatellite::FY2E || geoindex == (int)eGeoSatellite::FY2G)
        {
            if (cnt_IR1 == 1 && cnt_IR2 == 1 && cnt_IR3 == 1 && cnt_IR4 == 1 && cnt_VIS == 1 && cnt_VIS1KM == 1)
                col.setRgb(174, 225, 184);
            else
                col.setRgb(225, 171, 196);
        }
        else if (geoindex == (int)eGeoSatellite::GOES_15)
        {
            if (cnt_vis008 == 7 && cnt_ir039 == 7 && cnt_ir087 == 7 && cnt_ir108 == 7 )
                col.setRgb(174, 225, 184);
            else
                col.setRgb(225, 171, 196);
        }
        else if (geoindex == (int)eGeoSatellite::GOES_16)
        {
            if (cnt_C02 == 1 && cnt_C03 == 1 && cnt_C04 == 1 && cnt_C05 == 1 && cnt_C06 == 1 && cnt_C07 == 1 &&
                cnt_C08 == 1 && cnt_C09 == 1 && cnt_C10 == 1 && cnt_C11 == 1 && cnt_C12 == 1 && cnt_C13 == 1 && cnt_C14 == 1 &&
                cnt_C15 == 1 && cnt_C16 == 1 )
                col.setRgb(174, 225, 184);
            else
                col.setRgb(225, 171, 196);
        }
        else if (geoindex == (int)eGeoSatellite::H8)
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


FormGeostationary::~FormGeostationary()
{
    delete ui;
}

void FormGeostationary::slotCreateGeoImage(QString type, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor)
{
    // segs->seglgeo[0]->areatype == 1 ==> full
    // segs->seglgeo[0]->areatype == 0 ==> europe


    // Red =(HRV x IR_016) / LUMlo
    // Green=(HRV x VIS008) / LUMlo
    // Blue =(HRV x VIS006) / LUMlo
    // LUMlo=IR_016 + VIS008 + VIS006

    QString tex;


    SegmentListGeostationary *sl;

    for(int i = 0; i < opts.geosatellites.count(); i++)
    {
        QList<QTreeWidgetItem *> treewidgetselected = geotreewidgetlist.at(i)->selectedItems();
        if(treewidgetselected.count() > 0)
        {
            sl = setActiveSegmentList(i);
            QTreeWidgetItem *it = treewidgetselected.at(0);
            tex = it->text(0);
            break;
        }
    }

//    int geoindex = formtoolbox->getGeoIndex();
    int geoindex = sl->getGeoSatelliteIndex();

    formtoolbox->createFilenamestring(opts.geosatellites.at(geoindex).shortname, tex, spectrumvector);

    sl = setActiveSegmentList(geoindex);
    sl->ResetSegments();

    qDebug() << "FormGeostationary::slotCreateGeoImage(eGeoSatellite, QString, QVector<QString>, QVector<bool>, int) geoindex  " << geoindex;

    imageptrs->ResetPtrImage();

    qDebug() << QString(" CreateGeoImage ; kind of image = %1 spectrumvector = %2 ; %3 ; %4").arg(sl->getKindofImage()).arg(spectrumvector.at(0)).arg(spectrumvector.at(1)).arg(spectrumvector.at(2));
    qDebug() << QString(" CreateGeoImage ; imagecreated = %1").arg(tex);

    if (spectrumvector.at(0) == "" &&  spectrumvector.at(1) == "" && spectrumvector.at(1) == "")
        return;


    if (type == "HRV" || type == "HRV Color")
    {
            if (sl->areatype == 1)
            {
                if(opts.geosatellites.at(geoindex).imageheighthrv1 > 0 && opts.geosatellites.at(geoindex).imagewidthhrv1)
                    imageptrs->InitializeImageGeostationary(opts.geosatellites.at(geoindex).imagewidthhrv1, opts.geosatellites.at(geoindex).imageheighthrv1);
                else
                    return;
            }
            else
            {
                if(opts.geosatellites.at(geoindex).imageheighthrv0 > 0 && opts.geosatellites.at(geoindex).imagewidthhrv0 > 0)
                    imageptrs->InitializeImageGeostationary(opts.geosatellites.at(geoindex).imagewidthhrv0, opts.geosatellites.at(geoindex).imageheighthrv0);
                else
                    return;
            }
    }
    else
    {
        if(opts.geosatellites.at(geoindex).imageheight > 0 && opts.geosatellites.at(geoindex).imagewidth > 0)
            imageptrs->InitializeImageGeostationary(opts.geosatellites.at(geoindex).imagewidth, opts.geosatellites.at(geoindex).imageheight);
        else
            return;
    }


    formimage->displayImage(IMAGE_GEOSTATIONARY);

    //formimage->adjustPicSize(true);

    qDebug() << QString("FormGeostationary::CreateGeoImage kind = %1 areatype = %2").arg(type).arg(sl->areatype);

    if(opts.geosatellites.at(sl->getGeoSatelliteIndex()).protocol == "HDF" )
        CreateGeoImageHDF(sl, type, tex, spectrumvector, inversevector);
    else if(opts.geosatellites.at(sl->getGeoSatelliteIndex()).protocol == "netCDF")
        CreateGeoImagenetCDF(sl, type, tex, spectrumvector, inversevector, histogrammethod, pseudocolor);
    else
        CreateGeoImageXRIT(sl, type, tex, spectrumvector, inversevector, histogrammethod);

}

SegmentListGeostationary *FormGeostationary::setActiveSegmentList(int geoindex)
{
    SegmentListGeostationary *sl;
    for(int i = 0; i < opts.geosatellites.count(); i++)
    {
        if(geoindex == i)
        {
            sl = segs->seglgeo.at(i);
            sl->bActiveSegmentList = true;
        }
        else
            segs->seglgeo[i]->bActiveSegmentList = false;

    }
    return sl;
}

SegmentListGeostationary *FormGeostationary::getActiveSegmentList()
{
    SegmentListGeostationary *sl = NULL;
    for(int i = 0; i < opts.geosatellites.count(); i++)
    {
        if(segs->seglgeo[i]->bActiveSegmentList == true)
        {
            sl = segs->seglgeo.at(i);
            break;
        }
    }
    return sl;
}

void FormGeostationary::CreateGeoImageXRIT(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod)
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

    QApplication::setOverrideCursor(Qt::WaitCursor);
    int geoindex = sl->getGeoSatelliteIndex();

    filetiming = tex.mid(0, 4) + tex.mid(5, 2) + tex.mid(8, 2) + tex.mid(13, 2) + tex.mid(16, 2);

    filepattern = QString(opts.geosatellites.at(geoindex).filepattern).arg(filetiming.mid(0, opts.geosatellites.at(geoindex).lengthdate));

    qDebug() << "FormGeostationary::CreateGeoImage filepattern = " << filepattern;

    if(type == "VIS_IR" || type == "VIS_IR Color")
    {
        llVIS_IR = this->getGeostationarySegments(geoindex, "VIS_IR", sl->getImagePath(), spectrumvector, filepattern);
        if(llVIS_IR.size() == 0)
        {
            qDebug() << "FormGeostationary::CreateGeoImage : VIS_IR : ===> No segments selected";
            emit enabletoolboxbuttons(true);
            QApplication::restoreOverrideCursor();
            return;
        }
        if(sl->getGeoSatellite() != eGeoSatellite::H8)
            faVIS_IR.parse(sl->getImagePath() + "/" + llVIS_IR.at(0));
    }
    else if(type == "HRV Color")
    {
        llVIS_IR = this->getGeostationarySegments(geoindex, "VIS_IR", sl->getImagePath(), spectrumvector, filepattern);
        llHRV = this->getGeostationarySegments(geoindex, "HRV", sl->getImagePath(), spectrumvector, filepattern);
        if(llVIS_IR.size() == 0 || llHRV.size() == 0)
        {
            qDebug() << "FormGeostationary::CreateGeoImage : HRV Color : ===> No segments selected";
            emit enabletoolboxbuttons(true);
            QApplication::restoreOverrideCursor();
            return;
        }
        if(sl->getGeoSatellite() != eGeoSatellite::H8)
        {
            faVIS_IR.parse(sl->getImagePath() + "/" + llVIS_IR.at(0));
            faHRV.parse(sl->getImagePath() + "/" + llHRV.at(0));
        }
    }
    else if(type == "HRV")
    {
        llHRV = this->getGeostationarySegments(geoindex, "HRV", sl->getImagePath(), spectrumvector, filepattern);
        if(llHRV.size() == 0)
        {
            qDebug() << "FormGeostationary::CreateGeoImage : HRV : ===> No segments selected";
            emit enabletoolboxbuttons(true);
            QApplication::restoreOverrideCursor();
            return;
        }
        if(sl->getGeoSatellite() != eGeoSatellite::H8)
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

        MSG_data prodata;
        MSG_data epidata;
        MsgFileAccess fa;
        QString prologuefile;
        QString epiloguefile;
        MSG_header epiheader;
        MSG_header proheader;

        if(sl->getGeoSatellite() != eGeoSatellite::H8)
        {
            if(opts.geosatellites.at(geoindex).prologfile)
            {

                fa = (type == "VIS_IR" || type == "VIS_IR Color" ? faVIS_IR : faHRV);

                // Read prologue

                prologuefile = fa.prologueFile();

                qDebug() << QString("Reading prologue file = %1").arg(prologuefile);

                if (prologuefile.length() > 0)
                {
                    try
                    {
                        da.read_file(prologuefile, proheader, prodata);
                    }
                    catch( std::runtime_error &run )
                    {
                        qDebug() << QString("Error : runtime error in reading prologue file : %1").arg(run.what());
                    }
                }


            }

            if(opts.geosatellites.at(geoindex).epilogfile)
            {
                // Read epilogue

                epiloguefile = fa.epilogueFile();

                qDebug() << QString("Reading epilogue file = %1").arg(epiloguefile);

                try
                {
                    da.read_file(epiloguefile, epiheader, epidata);
                    if (type == "HRV" || type == "HRV Color")
                    {
                        MSG_ActualL15CoverageHRV& cov = epidata.epilogue->product_stats.ActualL15CoverageHRV;
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

//                    qDebug()<< epidata.epilogue->product_stats.ActualScanningSummary.ForwardScanStart.get_timestring().c_str();
//                    double JD_start = epidata.epilogue->product_stats.ActualScanningSummary.ForwardScanStart.get_jtime();
//                    double JD_end = epidata.epilogue->product_stats.ActualScanningSummary.ForwardScanEnd.get_jtime();

//                    double jtime = (JD_start + JD_end) / 2.;

//                    struct tm cdate;
//                    Calendar_Date(jtime, &cdate);
//                    double day_of_year = jtime - (QSgp4Date::DateToJD(cdate.tm_year, 1, 0, true) - .5);

//                    qDebug() << "day_of_year = " << day_of_year;

//                    double JD_start2;
//                    double JD_end2;

//                    /*-------------------------------------------------------------------------
//                     * Compute the satellite position vector in Cartesian coordinates (km).
//                     *-----------------------------------------------------------------------*/
//                    int i;
//                    for (i = 0; i < 100; ++i) {
//                         JD_start2 = prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].StartTime.get_jtime();
//                         JD_end2   = prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].EndTime.get_jtime();
//                         if (jtime >= JD_start2 && jtime <= JD_end2)
//                              break;
//                    }

//                    if (i == 100) {
//                         fprintf(stderr, "ERROR: Image time is out of range of supplied orbit "
//                                 "polynomials\n");
//                         return;
//                    }

//                    double t, X, Y, Z;
//                    t = (jtime - (JD_start2 + JD_end2) / 2.) / ((JD_end2   - JD_start2) / 2.);

//                    X = prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].X[0] +
//                        prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].X[1] * t;
//                    Y = prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].Y[0] +
//                        prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].Y[1] * t;
//                    Z = prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].Z[0] +
//                        prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].Z[1] * t;

//                    qDebug() << "i = " << i;
//                    qDebug() << "X = " << X;
//                    qDebug() << "Y = " << Y;
//                    qDebug() << "Z = " << Z;

//                    /*-------------------------------------------------------------------------
//                     * Compute latitude and longitude and solar and sensor zenith and azimuth
//                     * angles.
//                     *-----------------------------------------------------------------------*/

//                    double lon0 = prodata.prologue->image_description.ProjectionDescription.LongitudeOfSSP;
//                    qDebug() << "longitude SSP = " << lon0;

//                     for (i = 0; i < d->image.n_lines; ++i) {
//                         ii = d->image.i_line + i;

//                         jtime2 = jtime_start + (double) ii / (double) (IMAGE_SIZE_VIR_LINES - 1) *
//                                  (jtime_end - jtime_start);

//                         for (j = 0; j < d->image.n_columns; ++j) {
//                              i_image = i * d->image.n_columns + j;

//                              snu_line_column_to_lat_lon(ii + 1 + nav_off, d->image.i_column + j + 1,
//                                                         &d2->lat[i_image], &d2->lon[i_image],
//                                                         lon0, &nav_scaling_factors_vir);

//                              if (d2->lat[i_image] != FILL_VALUE_F && d2->lon[i_image] != FILL_VALUE_F) {
//                                   d2->time[i_image] = jtime2;

//                                   snu_solar_params2(jtime2, d2->lat[i_image] * D2R,
//                                                     d2->lon[i_image] * D2R, &mu0, &theta0,
//                                                     &phi0, NULL);
//                                   d2->sza[i_image] = theta0 * R2D;
//                                   d2->saa[i_image] = phi0   * R2D;

//                                   d2->saa[i_image] = d2->saa[i_image] + 180.;
//                                   if (d2->saa[i_image] > 360.)
//                                        d2->saa[i_image] = d2->saa[i_image] - 360.;

//                                   snu_vza_and_vaa(d2->lat[i_image], d2->lon[i_image], 0.,
//                                                   X, Y, Z, &d2->vza[i_image], &d2->vaa[i_image]);

//                                   d2->vaa[i_image] = d2->vaa[i_image] + 180.;
//                                   if (d2->vaa[i_image] > 360.)
//                                        d2->vaa[i_image] = d2->vaa[i_image] - 360.;
//                              }
//                         }
//                    }



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
        }

//        if(whichgeo == eGeoSatellite::GOMS2)
//        {
//            try
//            {
//                //            da.read_file(fa.directory + "/" + fa.segmentFiles().at(0),header);
//                da.read_file(fa.directory + "/" + (type == "VIS_IR" || type == "VIS_IR Color" ? llVIS_IR.at(0) : llHRV.at(0)),header);
//                lCFAC = (long)header.image_navigation->CFAC*180.0/PI;
//                lLFAC = (long)header.image_navigation->LFAC*180.0/PI;
//                sl->CFAC = abs(lCFAC);
//                sl->COFF = abs(header.image_navigation->COFF);
//                sl->LFAC = abs(lLFAC);
//                sl->LOFF = abs(header.image_navigation->LOFF);
//                qDebug() << QString("CFAC = %1 COFF = %2 LFAC = %3 LOFF = %4").arg(sl->CFAC, 0, 'f').arg(sl->COFF).arg(sl->LFAC, 0, 'f').arg(sl->LOFF);

//            }
//            catch( std::runtime_error &run )
//            {
//                qDebug() << QString("Error : runtime error in reading first segmentfile : %1").arg(run.what());
//            }
//        }



        qDebug() << "ForwardScanStart = " << epidata.epilogue->product_stats.ActualScanningSummary.ForwardScanStart.get_timestring().c_str();
        //    cout << epi.epilogue->product_stats.ActualScanningSummary.ForwardScanStart;
        qDebug() << "ForwardScanEnd = " << epidata.epilogue->product_stats.ActualScanningSummary.ForwardScanEnd.get_timestring().c_str();
        //    cout << epi.epilogue->product_stats.ActualScanningSummary.ForwardScanEnd;

        //epidata.epilogue->product_stats.ActualScanningSummary.ForwardScanEnd.get_unixtime().get_timestruct()

        if(type == "HRV" || type == "HRV Color")
        {
            sl->COFF = opts.geosatellites.at(geoindex).coffhrv;
            sl->LOFF = opts.geosatellites.at(geoindex).loffhrv;
            sl->CFAC = opts.geosatellites.at(geoindex).cfachrv;
            sl->LFAC = opts.geosatellites.at(geoindex).lfachrv;
        }
        else
        {
            sl->COFF = opts.geosatellites.at(geoindex).coff;
            sl->LOFF = opts.geosatellites.at(geoindex).loff;
            sl->CFAC = opts.geosatellites.at(geoindex).cfac;
            sl->LFAC = opts.geosatellites.at(geoindex).lfac;
        }


        if(type == "VIS_IR" || type == "VIS_IR Color" || type == "HRV Color")
        {
            for (int j =  0; j < llVIS_IR.size(); ++j)
            {
                QFile file(sl->getImagePath() + "/" + llVIS_IR.at(j));
                QFileInfo fileinfo(file);
                filesequence = fileinfo.fileName().mid(opts.geosatellites.at(geoindex).indexfilenbr, opts.geosatellites.at(geoindex).lengthfilenbr).toInt()-1;
                filespectrum = fileinfo.fileName().mid(opts.geosatellites.at(geoindex).indexspectrum, opts.geosatellites.at(geoindex).spectrumlist.at(0).length());
                filedate = fileinfo.fileName().mid(opts.geosatellites.at(geoindex).indexdate, opts.geosatellites.at(geoindex).lengthdate);
                filedate.leftJustified(12, '0');

                sl->InsertPresent( spectrumvector, filespectrum, filesequence);

                sl->ComposeImageXRIT(fileinfo.filePath(), spectrumvector, inversevector, histogrammethod);

                qDebug() << QString("CreateGeoImageXRIT VIS_IR || VIS_IR Color || HRV Color ----> %1 filesequence = %2").arg(fileinfo.filePath()).arg(filesequence);
            }
        }

        if( type == "HRV" || type == "HRV Color")
        {
            for (int j = 0; j < llHRV.size(); ++j)
            {
                QFile file(sl->getImagePath() + "/" + llHRV.at(j));
                QFileInfo fileinfo(file);
                filesequence = fileinfo.fileName().mid(opts.geosatellites.at(geoindex).indexfilenbrhrv, opts.geosatellites.at(geoindex).lengthfilenbrhrv).toInt()-1;
                filespectrum = fileinfo.fileName().mid(opts.geosatellites.at(geoindex).indexspectrumhrv, opts.geosatellites.at(geoindex).spectrumhrv.length());
                filedate = fileinfo.fileName().mid(opts.geosatellites.at(geoindex).indexdatehrv, opts.geosatellites.at(geoindex).lengthdatehrv);
                filedate.leftJustified(12, '0');

                sl->InsertPresent( spectrumvector, filespectrum, filesequence);

                if(sl->areatype == 1)
                    sl->ComposeImageXRIT(fileinfo.filePath(), spectrumvector, inversevector, histogrammethod);
                else if(filesequence >= opts.geosatellites.at(geoindex).startsegmentnbrhrvtype0)
                    sl->ComposeImageXRIT(fileinfo.filePath(), spectrumvector, inversevector, histogrammethod);

                qDebug() << QString("CreateGeoImageXRIT HRV || HRV Color ----> %1").arg(fileinfo.filePath());
            }
        }
}

void FormGeostationary::CreateGeoImageHDF(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector)
{

    QString filetiming;
    QString filedate;
    QStringList llVIS_IRgz;
    QStringList llHRVgz;
    QStringList llVIS_IR;
    QStringList llHRV;
    QString filepattern;
    QString filepatterngz;

    eGeoSatellite whichgeo = sl->getGeoSatellite();
    int geoindex = sl->getGeoSatelliteIndex();

    if(type == "HRV" || type == "HRV Color")
    {
        sl->COFF = opts.geosatellites.at(geoindex).coffhrv;
        sl->LOFF = opts.geosatellites.at(geoindex).loffhrv;
        sl->CFAC = opts.geosatellites.at(geoindex).cfachrv;
        sl->LFAC = opts.geosatellites.at(geoindex).lfachrv;
    }
    else
    {
        sl->COFF = opts.geosatellites.at(geoindex).coff;
        sl->LOFF = opts.geosatellites.at(geoindex).loff;
        sl->CFAC = opts.geosatellites.at(geoindex).cfac;
        sl->LFAC = opts.geosatellites.at(geoindex).lfac;
     }

    filetiming = tex.mid(0, 4) + tex.mid(5, 2) + tex.mid(8, 2) + tex.mid(13, 2) + tex.mid(16, 2) + "00";
    filedate = tex.mid(0, 4) + tex.mid(5, 2) + tex.mid(8, 2);


    if(whichgeo == eGeoSatellite::FY2E && (type == "VIS_IR" || type == "VIS_IR Color"))
    {
        filepatterngz = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2E_FDI_???") + QString("_001_NOM.HDF.gz");
        filepattern = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2E_FDI_???") + QString("_001_NOM.HDF");
    }
    else if(whichgeo == eGeoSatellite::FY2G && (type == "VIS_IR" || type == "VIS_IR Color"))
    {
        filepatterngz = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2G_FDI_???") + QString("_001_NOM.HDF.gz");
        filepattern = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2G_FDI_???") + QString("_001_NOM.HDF");
    }
    else if(whichgeo == eGeoSatellite::FY2E && type == "HRV")
    {
        filepatterngz = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2E_FDI_VIS1KM") + QString("_001_NOM.HDF.gz");
        filepattern = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2E_FDI_VIS1KM") + QString("_001_NOM.HDF");
    }
    else if(whichgeo == eGeoSatellite::FY2G && type == "HRV")
    {
        filepatterngz = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2G_FDI_VIS1KM") + QString("_001_NOM.HDF.gz");
        filepattern = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2G_FDI_VIS1KM") + QString("_001_NOM.HDF");
    }

    if(type == "VIS_IR" || type == "VIS_IR Color")
    {
        llVIS_IRgz = this->getGeostationarySegments(geoindex, "VIS_IR", sl->getImagePath(), spectrumvector, filepatterngz);
        qDebug() << QString("llVIS_IR count = %1").arg(llVIS_IRgz.count());
        for (int j =  0; j < llVIS_IRgz.size(); ++j)
        {
            qDebug() << QString("llVIS_IR at %1 = %2 spectrumvector = %3").arg(j).arg(llVIS_IRgz.at(j)).arg(spectrumvector.at(j));
        }
        if(llVIS_IRgz.count() == 0)
        {
            llVIS_IR = this->getGeostationarySegments(geoindex, "VIS_IR", sl->getImagePath(), spectrumvector, filepattern);
            if(llVIS_IR.count() == 0)
            {
                QApplication::restoreOverrideCursor();
                emit enabletoolboxbuttons(true);
                return;
            }
            else
                sl->ComposeImageHDFInThread(llVIS_IR, spectrumvector, inversevector);
        }
        else
            sl->ComposeImageHDFInThread(llVIS_IRgz, spectrumvector, inversevector);
    }
    else if(type == "HRV")
    {
        llHRVgz = this->getGeostationarySegments(geoindex, "HRV", sl->getImagePath(), spectrumvector, filepatterngz);
        if(llHRVgz.count() == 0)
        {
            llHRV = this->getGeostationarySegments(geoindex, "HRV", sl->getImagePath(), spectrumvector, filepattern);
            if(llHRV.count() == 0)
            {
                QApplication::restoreOverrideCursor();
                emit enabletoolboxbuttons(true);
                return;
            }
            else
                sl->ComposeImageHDFInThread(llHRV, spectrumvector, inversevector);
        }
        else
            sl->ComposeImageHDFInThread(llHRVgz, spectrumvector, inversevector);
    }

}

void FormGeostationary::CreateGeoImagenetCDF(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor)
{

    QString filetiming;
    QStringList llVIS_IR;
    QString filepattern;

    eGeoSatellite whichgeo = sl->getGeoSatellite();
    int geoindex = sl->getGeoSatelliteIndex();

    sl->COFF = opts.geosatellites.at(geoindex).coff;
    sl->LOFF = opts.geosatellites.at(geoindex).loff;
    sl->CFAC = opts.geosatellites.at(geoindex).cfac;
    sl->LFAC = opts.geosatellites.at(geoindex).lfac;

    qDebug() << "====> tex = " << tex;
    //"2017-08-10   19:45"
    QDate now(tex.mid(0, 4).toInt(), tex.mid(5, 2).toInt(), tex.mid(8, 2).toInt());
    int DDD = now.dayOfYear();
    filetiming = tex.mid(0, 4) + QString("%1").arg(DDD).rightJustified(3, '0') + tex.mid(13, 2) + tex.mid(16, 2);

    //OR_ABI-L1b-RadF-M4C01_G16_s20161811455312_e20161811500122_c20161811500175.nc
    //01234567890123456789012345678901234567890123456789
    if(whichgeo == eGeoSatellite::GOES_16 && (type == "VIS_IR" || type == "VIS_IR Color"))
        filepattern = QString("OR_ABI-L1b-RadF-M????_G16_s") + filetiming + QString("*.nc");
    else
        return;

    //sl->InsertPresent( spectrumvector, "", 0);

    if(type == "VIS_IR" || type == "VIS_IR Color")
    {
        llVIS_IR = this->getGeostationarySegments(geoindex, "VIS_IR", sl->getImagePath(), spectrumvector, filepattern);
        qDebug() << QString("llVIS_IR count = %1").arg(llVIS_IR.count());
        if(llVIS_IR.count() == 0 || (type == "VIS_IR Color" && llVIS_IR.count() != 3))
        {
            QApplication::restoreOverrideCursor();
            QMessageBox msgBox;
            msgBox.setText("Color band segment not found");
            msgBox.exec();
            emit enabletoolboxbuttons(true);
            return;
        }
        else
            sl->setThreadParameters(llVIS_IR, spectrumvector, inversevector, histogrammethod, pseudocolor);
            sl->ComposeImagenetCDFInThread(llVIS_IR, spectrumvector, inversevector, histogrammethod, pseudocolor);
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

void FormGeostationary::SelectGeoWidgetItem(int geoindex, QTreeWidgetItem *item, int column )
{

    if ( !item )
          return;

    for(int i = 0; i < opts.geosatellites.count(); i++)
        setTreeWidget( geotreewidgetlist.at(i), i == geoindex ? true : false);

    qDebug() << opts.geosatellites.at(geoindex).shortname + " " + (*item).text(0);

    imageptrs->ptrimageGeostationary->fill(Qt::black);
    formimage->setupGeoOverlay(geoindex);

    //012345678901234567
    //2017-07-04   08:00
    QString strdate = (*item).text(0).mid(0, 4) + (*item).text(0).mid(5, 2) + (*item).text(0).mid(8, 2) + (*item).text(0).mid(13, 2) + (*item).text(0).mid(16, 2);
    segs->setAbsolutePathFromMap(geoindex, strdate);

    QStringList tex;
    for(int i = 0; i < item->columnCount(); i++)
        tex << (*item).text(i);
    emit geostationarysegmentschosen(geoindex, tex);
}

void FormGeostationary::ontreeWidgetitemClicked(QTreeWidgetItem *item, int column)
{
    qDebug() << QString("FormGeostationary::ontreeWidgetitemClicked(QTreeWidgetItem *item, int column) = %1 %2").arg(column).arg(ui->tabGeostationary->currentIndex());

    SelectGeoWidgetItem(ui->tabGeostationary->currentIndex(), item, column);
}

void FormGeostationary::on_tabGeostationary_tabBarClicked(int index)
{
//    int activeindex = this->getActiveSegmentList()->getGeoSatelliteIndex();
//    formimage->setupGeoOverlay(index);

    emit setbuttonlabels(index, false);
}

int FormGeostationary::getTabWidgetGeoIndex()
{
    return ui->tabGeostationary->currentIndex();
}

void FormGeostationary::slotCreateRGBrecipe(int recipe)
{
    qDebug() << "CreateRGBrecipe = " << recipe;
    CreateRGBrecipeImage(recipe);
}

void FormGeostationary::CreateRGBrecipeImage(int recipe)
{

    QString tex;
    QStringList redbandlist = imageptrs->rgbrecipes[recipe].Colorvector.at(0).channels;
    QStringList greenbandlist = imageptrs->rgbrecipes[recipe].Colorvector.at(1).channels;
    QStringList bluebandlist = imageptrs->rgbrecipes[recipe].Colorvector.at(2).channels;
    QStringList uniquebandlist;
    QList<seviri_units> uniqueunitlist;
    seviri_units redunits = imageptrs->rgbrecipes[recipe].Colorvector.at(0).units;
    seviri_units greenunits = imageptrs->rgbrecipes[recipe].Colorvector.at(1).units;
    seviri_units blueunits = imageptrs->rgbrecipes[recipe].Colorvector.at(2).units;
    QList<int> uniquechannelnbrlist;

    for(int i = 0; i < redbandlist.length(); i++)
    {
        if(!uniquebandlist.contains(redbandlist.at(i)))
        {
            uniquebandlist.append(redbandlist.at(i));
            uniqueunitlist.append(redunits);
            uniquechannelnbrlist.append(imageptrs->rgbrecipes[recipe].Colorvector.at(0).spectral_channel_nbr.at(i));
        }
    }
    for(int i = 0; i < greenbandlist.length(); i++)
    {
        if(!uniquebandlist.contains(greenbandlist.at(i)))
        {
            uniquebandlist.append(greenbandlist.at(i));
            uniqueunitlist.append(greenunits);
            uniquechannelnbrlist.append(imageptrs->rgbrecipes[recipe].Colorvector.at(1).spectral_channel_nbr.at(i));
        }
    }
    for(int i = 0; i < bluebandlist.length(); i++)
    {
        if(!uniquebandlist.contains(bluebandlist.at(i)))
        {
            uniquebandlist.append(bluebandlist.at(i));
            uniqueunitlist.append(blueunits);
            uniquechannelnbrlist.append(imageptrs->rgbrecipes[recipe].Colorvector.at(2).spectral_channel_nbr.at(i));
        }
    }

    QString recipename = imageptrs->rgbrecipes[recipe].Name;
    qDebug() << "recipename = " << recipename;
    qDebug() << "uniquebandlist contains = " << uniquebandlist.count();
    for(int i = 0; i < uniquebandlist.length(); i++)
    {
        qDebug() << "channels at " << uniquechannelnbrlist.at(i) << " = " << uniquebandlist.at(i);
    }

    SegmentListGeostationary *sl;

    for(int i = 0; i < opts.geosatellites.count(); i++)
    {
        QList<QTreeWidgetItem *> treewidgetselected = geotreewidgetlist.at(i)->selectedItems();
        if(treewidgetselected.count() > 0)
        {
            sl = setActiveSegmentList(i);
            QTreeWidgetItem *it = treewidgetselected.at(0);
            tex = it->text(0);
            break;
        }
    }

    qDebug() << "tex = " << tex << " imagepath = " << sl->getImagePath();
    //tex =  "2018-02-13   14:45"

    int geoindex = sl->getGeoSatelliteIndex();
    sl = setActiveSegmentList(geoindex);
    sl->ResetSegments();
    imageptrs->ResetPtrImage();

    QString directory = sl->getImagePath();
    QString productid1 = opts.geosatellites.at(geoindex).searchstring.mid(6, 4);
    QString productid2 = "IR_016";
    QString timing = tex.mid(0, 4) + tex.mid(5, 2) + tex.mid(8, 2) + tex.mid(13, 2) + tex.mid(16, 2);

    qDebug() << "directory = " << directory;
    qDebug() << "productid1 = " << productid1;
    qDebug() << "productid2 = " << productid2;
    qDebug() << "timing = " << timing;

    int totalpixels = 3712 * 3712;

    double *time = new double[totalpixels];
    float *lat = new float[totalpixels];
    float *lon = new float[totalpixels];
    float *sza = new float[totalpixels];
//    float *saa = new float[totalpixels];
//    float *vza = new float[totalpixels];
//    float *vaa = new float[totalpixels];

    QList<float*> result;
    result.append(new float[totalpixels]);
    result.append(new float[totalpixels]);
    result.append(new float[totalpixels]);
    float resultmax[3];
    float resultmin[3];

    for(int i = 0; i < 3; i++)
    {
        resultmin[i] = std::numeric_limits<float>::max();
        resultmax[i] = std::numeric_limits<float>::min();
    }

    snu_init_array_d(time, totalpixels, FILL_VALUE_F);
    snu_init_array_f(lat,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(lon,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(sza,  totalpixels, FILL_VALUE_F);
//    snu_init_array_f(saa,  totalpixels, FILL_VALUE_F);
//    snu_init_array_f(vza,  totalpixels, FILL_VALUE_F);
//    snu_init_array_f(vaa,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(result[0],   totalpixels, FILL_VALUE_F);
    snu_init_array_f(result[1], totalpixels, FILL_VALUE_F);
    snu_init_array_f(result[2],  totalpixels, FILL_VALUE_F);

//    snu_init_array_f(d2->data2, d->image.n_bands * length, d2->fill_value);

    typedef struct {
        int spectral_channel_nbr;
        QString spectrum;
        float min;
        float max;
        float *data;
        seviri_units units;
    } bandstorage;

    QList<bandstorage> bands;

    for(int i = 0; i < uniquebandlist.length(); i++)
    {
        bandstorage newband;
        newband.spectral_channel_nbr = uniquechannelnbrlist.at(i);
        newband.data = new float[totalpixels];
        snu_init_array_f(newband.data,  totalpixels, FILL_VALUE_F);
        newband.min = 0; newband.max = 0;
        newband.spectrum = uniquebandlist.at(i);
        newband.units = uniqueunitlist.at(i);
        bands.append(newband);
    }

    qDebug() << "number of bands = " << bands.length();


    MsgFileAccess fa(directory, "H", productid1, productid2, timing);
    MsgDataAccess da;
    MSG_data prodata;
    MSG_data epidata;
    MSG_header header;


    da.scan(fa, prodata, epidata, header);

    //cout << header;

    qDebug()<< epidata.epilogue->product_stats.ActualScanningSummary.ForwardScanStart.get_timestring().c_str();
    double jtime_start = epidata.epilogue->product_stats.ActualScanningSummary.ForwardScanStart.get_jtime();
    double jtime_end = epidata.epilogue->product_stats.ActualScanningSummary.ForwardScanEnd.get_jtime();

    double jtime2;
    double jtime = (jtime_start + jtime_end) / 2.;

    struct tm cdate;
    Calendar_Date(jtime, &cdate);
    double day_of_year = jtime - QSgp4Date::DateToJD(cdate.tm_year, 1, 0, true);

    qDebug() << "day_of_year = " << day_of_year;

    double jtime_start2;
    double jtime_end2;

    /*-------------------------------------------------------------------------
     * Compute the satellite position vector in Cartesian coordinates (km).
     *-----------------------------------------------------------------------*/
    int i;
    for (i = 0; i < 100; ++i) {
         jtime_start2 = prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].StartTime.get_jtime();
         jtime_end2   = prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].EndTime.get_jtime();
         if (jtime >= jtime_start2 && jtime <= jtime_end2)
              break;
    }

    if (i == 100) {
         fprintf(stderr, "ERROR: Image time is out of range of supplied orbit "
                 "polynomials\n");
         return;
    }

    double t, X, Y, Z;
    t = (jtime - (jtime_start2 + jtime_end2) / 2.) / ((jtime_end2   - jtime_start2) / 2.);

    X = prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].X[0] +
        prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].X[1] * t;
    Y = prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].Y[0] +
        prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].Y[1] * t;
    Z = prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].Z[0] +
        prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].Z[1] * t;

    qDebug() << "i = " << i;
    qDebug() << "X = " << X;
    qDebug() << "Y = " << Y;
    qDebug() << "Z = " << Z;

    /*-------------------------------------------------------------------------
     * Compute latitude and longitude and solar and sensor zenith and azimuth
     * angles.
     *-----------------------------------------------------------------------*/

    double lon0 = prodata.prologue->image_description.ProjectionDescription.LongitudeOfSSP;
    qDebug() << "longitude SSP = " << lon0;

    double mu0;
    double theta0;
    double phi0;

    long countF = 0;

    quint16 seglines = header.image_structure->number_of_lines;
    quint16 columns = header.image_structure->number_of_columns;

    qDebug() << "Start calculations";
    unsigned int i_image;


    for(int i = 0; i < uniquebandlist.length(); i++)
    {
        MsgFileAccess fac(fa, uniquebandlist.at(i));
        da.scan(fac, header);
        bands[i].spectral_channel_nbr = header.segment_id->spectral_channel_id;
        MSG_SAMPLE compmin = 0xffff, compmax = 0;
        for (size_t j = 0; j < da.segnames.size(); ++j)
        {
            cout << "Segment " << j << ": ";
            MSG_data* d = da.segment(j);
            MSG_SAMPLE min = 0xffff, max = 0;
            if (!d)
            {
                cout << "missing.\n";
                continue;
            } else {
                for (size_t k = 0; k < da.npixperseg; ++k)
                {
                    if (d->image->data[k] < min) min = d->image->data[k];
                    if (d->image->data[k] > max) max = d->image->data[k];
                    bands[i].data[j*da.npixperseg + k] = d->image->data[k];
                }
                cout << "min " << min << " max " << max << endl;
            }
            if (min < compmin) compmin = min;
            if (max > compmax) compmax = max;
        }
        cout << "compmin = " << compmin << " compmax = " << compmax << endl;
        bands[i].min = (float)compmin;
        bands[i].max = (float)compmax;
    }


    for (int i = 0; i < 3712; ++i)
    {
        jtime2 = jtime_start + (double) i / (double) (3712 - 1) * (jtime_end - jtime_start);
        for (int j = 0; j < 3712; ++j)
        {
            i_image = i * 3712 + j;

            snu_line_column_to_lat_lon(i, j, &lat[i_image], &lon[i_image], lon0, &nav_scaling_factors_vir);

            if (lat[i_image] != FILL_VALUE_F && lon[i_image] != FILL_VALUE_F)
            {
                time[i_image] = jtime2;

                snu_solar_params2(jtime2, lat[i_image] * D2R, lon[i_image] * D2R, &mu0, &theta0, &phi0, NULL);
                sza[i_image] = theta0 * R2D;
//                saa[i_image] = phi0   * R2D;

//                saa[i_image] = saa[i_image] + 180.;
//                if (saa[i_image] > 360.)
//                    saa[i_image] = saa[i_image] - 360.;

//                snu_vza_and_vaa(lat[i_image], lon[i_image], 0., X, Y, Z, &vza[i_image], &vaa[i_image]);

//                vaa[i_image] = vaa[i_image] + 180.;
//                if (vaa[i_image] > 360.)
//                    vaa[i_image] = vaa[i_image] - 360.;
            }
            else
            {
                countF++;
                for(int k = 0; k < bands.length(); k++)
                    bands[k].data[i_image] = FILL_VALUE_F;
            }
        }
    }
    qDebug() << "End calculations countF = " << countF;


    for(int i = 0; i < bands.length(); i++)
    {
        qDebug() << "spectral channel = " << bands[i].spectral_channel_nbr;
        qDebug() << "spectrum = " << bands[i].spectrum;
        qDebug() << "min = " << bands[i].min << " max = " << bands[i].max;
        qDebug() << "units = " << (int)bands[i].units;
    }

    /*-------------------------------------------------------------------------
     * Compute radiance for the bands requested.
     *
     * Ref: PDF_TEN_05105_MSG_IMG_DATA, Page 26
     *-----------------------------------------------------------------------*/

    double slope, offset;
    bool toint;


    for(int i = 0; i < bands.length(); i++)
    {
        if(bands[i].units == SEVIRI_UNIT_RAD) // mW*pow(m,-2)*pow(sr,-1)*pow(pow(cm,-1)), -1)
        {
            prodata.prologue->radiometric_proc.get_slope_offset(bands[i].spectral_channel_nbr, slope, offset, toint);
            bands[i].min = std::numeric_limits<float>::max();
            bands[i].max = std::numeric_limits<float>::min();
            for (int j = 0; j < 3712; ++j)
            {
                for (int k = 0; k < 3712; ++k)
                {
                    i_image = j * 3712 + k;

                    if (bands[i].data[i_image] != FILL_VALUE_US && bands[i].data[i_image] > 0)
                    {
                        bands[i].data[i_image] = bands[i].data[i_image] * slope + offset;
                        if(bands[i].data[i_image] < bands[i].min) bands[i].min = bands[i].data[i_image];
                        if(bands[i].data[i_image] > bands[i].max) bands[i].max = bands[i].data[i_image];
                    }
                }
            }
        }
    }

    /*-------------------------------------------------------------------------
     * Compute reflectance or bidirectional reflectance factor (BRF) for the
     * bands requested.
     *
     * Ref: PDF_MSG_SEVIRI_RAD2REFL, Page 8
     *-----------------------------------------------------------------------*/
    double dd = 1. / sqrt(snu_solar_distance_factor2(day_of_year));

    double a = PI * dd * dd;

    int satid =  (int)header.segment_id->spacecraft_id - 321 ;

    for (int i = 0; i < bands.length(); ++i)
    {
        if (bands[i].units == SEVIRI_UNIT_REF || bands[i].units == SEVIRI_UNIT_BRF)
        {
            prodata.prologue->radiometric_proc.get_slope_offset(bands[i].spectral_channel_nbr, slope, offset, toint);
            bands[i].min = std::numeric_limits<float>::max();
            bands[i].max = std::numeric_limits<float>::min();

            double b = a / band_solar_irradiance[satid][bands[i].spectral_channel_nbr - 1];

            for (int j = 0; j < 3712; ++j)
            {
                for (int k = 0; k < 3712; ++k)
                {
                    i_image = j * 3712 + k;

                    if (bands[i].units == SEVIRI_UNIT_BRF && bands[i].data[i_image] != FILL_VALUE_US && bands[i].data[i_image] > 0 &&
                            sza[i_image] >= 0. && sza[i_image] < 80.)
                    {
                        double R = bands[i].data[i_image] * slope + offset;

                        bands[i].data[i_image] = b * R;

                        if (bands[i].units == SEVIRI_UNIT_BRF)
                            bands[i].data[i_image] /= cos(sza[i_image] * D2R);

                        if(bands[i].data[i_image] < bands[i].min) bands[i].min = bands[i].data[i_image];
                        if(bands[i].data[i_image] > bands[i].max) bands[i].max = bands[i].data[i_image];
                    }
                    else if (bands[i].units == SEVIRI_UNIT_REF && bands[i].data[i_image] != FILL_VALUE_US && bands[i].data[i_image] > 0)
                    {
                        double R = bands[i].data[i_image] * slope + offset;

                        bands[i].data[i_image] = b * R;

                        if(bands[i].data[i_image] < bands[i].min) bands[i].min = bands[i].data[i_image];
                        if(bands[i].data[i_image] > bands[i].max) bands[i].max = bands[i].data[i_image];
                    }
                }
            }
        }
    }

    const double c1 = 1.19104e-5;
    const double c2 = 1.43877;

    /*-------------------------------------------------------------------------
     * Compute brightness temperature for the bands requested.
     *
     * Ref: PDF_TEN_05105_MSG_IMG_DATA, Page 26
     * Ref: The Conversion from Effective Radiances
     *      to Equivalent Brightness Temperatures (EUM/MET/TEN/11/0569)
     *-----------------------------------------------------------------------*/
    for (int i = 0; i < bands.length(); ++i)
    {
         if (bands[i].units == SEVIRI_UNIT_BT)
         {
             prodata.prologue->radiometric_proc.get_slope_offset(bands[i].spectral_channel_nbr, slope, offset, toint);
             bands[i].min = std::numeric_limits<float>::max();
             bands[i].max = std::numeric_limits<float>::min();

             qDebug() << bands[i].spectral_channel_nbr << " " << bands[i].spectrum << " slope = " << slope << " offset = " << offset;
/*
              nu = 1.e4 / channel_center_wavelength[d->image.band_ids[i] - 1];
*/
              double nu = bt_nu_c[satid][bands[i].spectral_channel_nbr - 1];
              double nu3 = nu * nu * nu;

              double a = bt_A[satid][bands[i].spectral_channel_nbr - 1];
              double b = bt_B[satid][bands[i].spectral_channel_nbr - 1];

              for (int j = 0; j < 3712; ++j)
              {
                   for (int k = 0; k < 3712; ++k)
                   {
                        i_image = j * 3712 + k;

                        if (bands[i].data[i_image] != FILL_VALUE_F && bands[i].data[i_image] > 0)
                        {
                             double L = bands[i].data[i_image] * slope + offset;
                             bands[i].data[i_image] =  (c2 * nu / log(1. + nu3 * c1 / L) - b) / a;
                             if(bands[i].data[i_image] < bands[i].min) bands[i].min = bands[i].data[i_image];
                             if(bands[i].data[i_image] > bands[i].max) bands[i].max = bands[i].data[i_image];
                        }
                   }
              }
         }
    }

    QRgb *row_col;
    float red, green, blue;

//    for (int line = 0; line < 3712; line++)
//    {
//        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(3711 - line);
//        for (int pixelx = 0; pixelx < 3712; pixelx++)
//        {
//            i_image = line * 3712 + pixelx;
//            if(bands[0].data[i_image] != FILL_VALUE_F )
//            {
//                red = 255.0 * pow((bands[0].data[i_image] - bands[0].min) / (bands[0].max - bands[0].min), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(0).gamma);
//                green = 255.0 * pow((bands[1].data[i_image] - bands[1].min) / (bands[1].max - bands[1].min), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(1).gamma);
//                blue = 255.0 * pow((bands[2].data[i_image] - bands[2].min) / (bands[2].max - bands[2].min), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(2).gamma);
//            }
//            else
//            {
//                red = 0.0;
//                green = 0.0;
//                blue = 0.0;
//            }
//            row_col[3711 - pixelx] = qRgb((int)red, (int)green, (int)blue);
//        }
//    }

    countF = 0;
    for(int colorindex = 0; colorindex < 3; colorindex++)
    {
        for(int i = 0; i < imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).channels.length(); i++)
        {
            for(int j = 0; j < bands.length(); j++)
            {
                if(bands[j].spectral_channel_nbr == imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).spectral_channel_nbr.at(i))
                {
                    qDebug() << colorindex << " " << imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).channels.at(i) << " " <<
                                imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).spectral_channel_nbr.at(i) << " " <<
                                imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).subtract.at(i);
                    for (int line = 0; line < 3712; line++)
                    {
                        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(3711 - line);
                        for (int pixelx = 0; pixelx < 3712; pixelx++)
                        {
                            i_image = line * 3712 + pixelx;
                            if(bands[j].data[i_image] != FILL_VALUE_F )
                            {
                                if(imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).subtract.at(i))
                                {
                                    if(result[colorindex][i_image] == FILL_VALUE_F)
                                        result[colorindex][i_image] = - bands[j].data[i_image];
                                    else
                                        result[colorindex][i_image] -= bands[j].data[i_image];
                                }
                                else
                                {
                                    if(result[colorindex][i_image] == FILL_VALUE_F)
                                        result[colorindex][i_image] = bands[j].data[i_image];
                                    else
                                        result[colorindex][i_image] += bands[j].data[i_image];
                                }
                               if(imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).dimension == "K")
                                {
                                    if(result[colorindex][i_image] > imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).rangeto )
                                        result[colorindex][i_image] = imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).rangeto;
                                    if(result[colorindex][i_image] < imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).rangefrom )
                                        result[colorindex][i_image] = imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).rangefrom;
                                }
                               if(result[colorindex][i_image] < resultmin[colorindex]) resultmin[colorindex] = result[colorindex][i_image];
                               if(result[colorindex][i_image] > resultmax[colorindex]) resultmax[colorindex] = result[colorindex][i_image];

                             }
                            else
                            {
                                countF++;
                                result[colorindex][i_image] = FILL_VALUE_F;
                            }
                        }
                    }
                }
            }
        }
    }

    qDebug() << "countF = " << countF;

    for(int colorindex = 0; colorindex < 3; colorindex++ )
        qDebug() << QString("%1 resultmin = %2 resultmax = %3").arg(colorindex).arg(resultmin[colorindex]).arg(resultmax[colorindex]);

    for (int line = 0; line < 3712; line++)
    {
        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(3711 - line);
        for (int pixelx = 0; pixelx < 3712; pixelx++)
        {
            i_image = line * 3712 + pixelx;

            if(result[0][i_image] != FILL_VALUE_F) //  && result[1][i_image] != FILL_VALUE_F && result[2][i_image] != FILL_VALUE_F )
            {
                red   = 255.0 * pow((result[0][i_image] - resultmin[0]) / (resultmax[0] - resultmin[0]), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(0).gamma);
                green = 255.0 * pow((result[1][i_image] - resultmin[1]) / (resultmax[1] - resultmin[1]), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(1).gamma);
                blue  = 255.0 * pow((result[2][i_image] - resultmin[2]) / (resultmax[2] - resultmin[2]), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(2).gamma);
            }
            else
            {
                red = 0.0;
                green = 0.0;
                blue = 0.0;
            }
            row_col[3711 - pixelx] = qRgb((int)red, (int)green, (int)blue);
        }
    }




    int i_pixel = 1856 * 3712 + 1856;

    qDebug() << QString("time = %1").arg(time[1856 * 3712 + 1856], 16, 'f', 5);
    qDebug() << QString("lat = %1").arg(lat[1856 * 3712 + 1856], 16, 'f', 5);
    qDebug() << QString("lon = %1").arg(lon[1856 * 3712 + 1856], 16, 'f', 5);
    qDebug() << QString("sza = %1").arg(sza[1856 * 3712 + 1856], 16, 'f', 5);
//    qDebug() << QString("saa = %1").arg(saa[1856 * 3712 + 1856], 16, 'f', 5);
//    qDebug() << QString("vza = %1").arg(vza[1856 * 3712 + 1856], 16, 'f', 5);
//    qDebug() << QString("vaa = %1").arg(vaa[1856 * 3712 + 1856], 16, 'f', 5);

//    printf("Julian Day Number:            % .8e\n", time[i_pixel]);
//    printf("latitude:                     % .8e\n", lat [i_pixel]);
//    printf("longitude:                    % .8e\n", lon [i_pixel]);
//    printf("solar zenith angle:           % .8e\n", sza [i_pixel]);
//    printf("solar azimuth angle:          % .8e\n", saa [i_pixel]);
//    printf("viewing zenith angle:         % .8e\n", vza [i_pixel]);
//    printf("viewing azimuth angle:        % .8e\n", vaa [i_pixel]);

    qDebug() << "min float = " << std::numeric_limits<float>::min();
    qDebug() << "max float = " << std::numeric_limits<float>::max();

//    qDebug() << QString("bands[0] = %1  [1856][1856] =  %2 units = %3 min = %4 max = %5")
//                .arg(bands[0].spectrum).arg(bands[0].data[1856 * 3712 + 1856], 9, 'f', 3).arg((int)bands[0].units).arg(bands[0].min).arg(bands[0].max);
//    qDebug() << QString("bands[1] = %1  [1856][1856] =  %2 units = %3 min = %4 max = %5")
//                .arg(bands[1].spectrum).arg(bands[1].data[1856 * 3712 + 1856], 9, 'f', 3).arg((int)bands[1].units).arg(bands[1].min).arg(bands[1].max);
//    qDebug() << QString("bands[2] = %1  [1856][1856] =  %2 units = %3 min = %4 max = %5")
//                .arg(bands[2].spectrum).arg(bands[2].data[1856 * 3712 + 1856], 9, 'f', 3).arg((int)bands[2].units).arg(bands[2].min).arg(bands[2].max);

    qDebug() << "CreateRGBrecipeImage(int recipe) Finished !!";

//    cout << "Columns: " << da.columns << endl;
//    cout << "Lines: " << da.lines << endl;
//    cout << "Segments: " << da.segnames.size() << endl;
//    cout << "Pixels per segment: " << da.npixperseg << endl;
//    cout << "Scanlines per segment: " << da.seglines << endl;
//    cout << "East is: " << (da.swapX ? "left" : "right") << endl;
//    cout << "North is: " << (da.swapY ? "down" : "up") << endl;
//    cout << "HRV: " << (da.hrv ? "yes" : "no") << endl;


    for(int i = 0; i < uniquebandlist.length(); i++)
    {
        delete [] bands[i].data;
    }

    delete [] time;
    delete [] lat;
    delete [] lon;
    delete [] sza;
//    delete [] saa;
//    delete [] vza;
//    delete [] vaa;
    delete [] result[0];
    delete [] result[1];
    delete [] result[2];


}

