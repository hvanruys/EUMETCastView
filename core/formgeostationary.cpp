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
#include "AA+.h"
#include "moon.h"

#include <QtConcurrent/QtConcurrent>

#include <cmath>

#define REDCHANNEL 0
#define GREENCHANNEL 1
#define BLUECHANNEL 2

extern SegmentImage *imageptrs;
extern Options opts;
extern SatelliteList satellitelist;

int moonCalc::moonVisible[144];
int moonCalc::moonCoordX[144];
int moonCalc::moonCoordY[144];

FormGeostationary::FormGeostationary(QWidget *parent, AVHRRSatellite *seglist) :
    QWidget(parent),
    ui(new Ui::FormGeostationary)
{
    ui->setupUi(this);
    segs = seglist;

    qDebug() << "in constructor FormGeostationary opts.geosatellites.count() = " << opts.geosatellites.count();

    for(int i = 0; i < opts.geosatellites.count(); i++)
    {
        if(opts.geosatellites.at(i).shortname != "MTG-I1")
        {
            QWidget *mywidget = new QWidget();
            ui->tabGeostationary->addTab(mywidget,opts.geosatellites.at(i).fullname + " : " + QString("%1").arg(opts.geosatellites.at(i).longitude) + "°");
            QTreeWidget *treeWidget = new QTreeWidget;

            geotreewidgetlist.append(treeWidget);
            treeWidget->setRootIsDecorated(false);
            treeWidget->header()->setStretchLastSection(true);
            treeWidget->setColumnCount(opts.geosatellites.at(i).spectrumlist.count() + 2);
            QStringList header;
            int columnsheadercount;
            if(opts.geosatellites.at(i).spectrumhrv.length() == 0)
            {
                header << opts.geosatellites.at(i).spectrumvalueslist;
                columnsheadercount = opts.geosatellites.at(i).spectrumlist.count() + 2;
                if(opts.geosatellites.at(i).shortname == "H9")
                {
                    columnsheadercount++;
                }

            }
            else
            {
                header << opts.geosatellites.at(i).spectrumhrv << opts.geosatellites.at(i).spectrumvalueslist;
                columnsheadercount = opts.geosatellites.at(i).spectrumlist.count() + 3;
            }

            treeWidget->setColumnWidth(0, 150);
            treeWidget->setColumnWidth(1, 150);

            if(opts.geosatellites.at(i).shortname == "H9")
                treeWidget->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << header << "Moon illumination (%)" );
            else
                treeWidget->setHeaderLabels( QStringList() << "Date/Time" << "Channels" << header );


            for(int i = 2; i < columnsheadercount; i++)
            {
                treeWidget->setColumnWidth(i, 40);
            }

            for(int i = 0; i < columnsheadercount; i++)
            {
                treeWidget->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
            }

            connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(ontreeWidgetitemClicked(QTreeWidgetItem *, int)));
            QVBoxLayout *mylayout = new QVBoxLayout;
            mylayout->addWidget(treeWidget);
            mywidget->setLayout(mylayout);
        }
        else
        {
            QWidget *mywidget = new QWidget();
            ui->tabGeostationary->addTab(mywidget,opts.geosatellites.at(i).fullname + " : " + QString("%1").arg(opts.geosatellites.at(i).longitude) + "°");
                QTreeWidget *treeWidget = new QTreeWidget;

            geotreewidgetlist.append(treeWidget);
            treeWidget->setRootIsDecorated(false);
            treeWidget->header()->setStretchLastSection(false);
            treeWidget->setColumnCount(41 + 2);
            QStringList header;

            for(int i = 1; i < 42; i++)
            {
                QString tel = QString("%1").arg(i, 3, 10);
                header.append(tel);
            }

            treeWidget->setHeaderLabels( QStringList() << "Date/Time" << "Seq." << header );

            treeWidget->header()->setMinimumSectionSize(5);
            treeWidget->header()->resizeSections(QHeaderView::ResizeToContents);
            treeWidget->setColumnWidth(0, 250);

            connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(ontreeWidgetitemClicked(QTreeWidgetItem *, int)));
            QVBoxLayout *mylayout = new QVBoxLayout;
            mylayout->addWidget(treeWidget);
            mywidget->setLayout(mylayout);
        }
    }

    if(opts.geosatellites.count() > 0)
    {
        ui->tabGeostationary->setCurrentIndex(0);
    }

}

QStringList FormGeostationary::getGeostationarySegments(int geoindex, const QString imagetype, const QString filepath, QVector<QString> spectrumvector, QString filepattern)
{
    qDebug() << QString("getGeostationarySegments type = %1  Filepath = %2 filepattern = %3").arg(imagetype).arg(filepath).arg(filepattern);
    qDebug() << QString("getGeostationarySegments spectrumvector %1 %2 %3 %4").arg(spectrumvector.at(0))
                .arg(spectrumvector.at(1)).arg(spectrumvector.at(2)).arg(spectrumvector.at(3));
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
                (filespectrum == spectrumvector.at(0) || filespectrum == spectrumvector.at(1) || filespectrum == spectrumvector.at(2) || filespectrum == spectrumvector.at(3)))
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

QStringList FormGeostationary::getGeostationarySegmentsMTG(int geoindex, const QString imagetype, const QString filepath, int filenbr)
{
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    //          1         2         3         4         5         6         7         8         9         10        11        12        13        14        15
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    //W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-BODY---NC4E_C_EUMT_20170920113515_GTT_DEV_20170920113008_20170920113015_N__T_0070_0001.nc
    //W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-TRAIL---NC4E_C_EUMT_20170920114422_GTT_DEV_20170920113008_20170920113922_N__T_0070_0041.nc

    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    //          1         2         3         4         5         6         7         8         9         10        11        12        13        14        15
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    //W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-BODY---NC4E_C_EUMT_20170920113515_GTT_DEV_20170920113008_20170920113015_N_JLS_T_0070_0001.nc
    //W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-TRAIL---NC4E_C_EUMT_20170920114422_GTT_DEV_20170920113008_20170920113922_N_JLS_T_0070_0041.nc
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    //          1         2         3         4         5         6         7         8         9         10        11        12        13        14        15
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    //W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-BODY--DIS-NC4E_C_EUMT_20231127115648_IDPFI_OPE_20231127115154_20231127115249_N_JLS_C_0072_0012.nc
    //W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-TRAIL--DIS-NC4E_C_EUMT_20231127112342_IDPFI_OPE_20231127112007_20231127112924_N_JLS_C_0069_0041.nc

    qDebug() << QString("getGeostationarySegments type = %1  Filepath = %2 filenbr = %3").arg(imagetype).arg(filepath).arg(filenbr);



    QDir meteosatdir(filepath);
    meteosatdir.setFilter(QDir::Files | QDir::NoSymLinks);
    meteosatdir.setSorting(QDir::Name);

    QStringList strlist = meteosatdir.entryList();
    QStringList strlistout;

    QStringList::Iterator itc = strlist.begin();

    int inpfilenbr = 0;

    while( itc != strlist.end() )
    {
        QString st = *itc;
        if(st.contains("JLS", Qt::CaseSensitive))
        {
            if(st.contains("DIS", Qt::CaseSensitive))
            {
                if(st.contains("BODY", Qt::CaseSensitive))
                {
                    inpfilenbr = st.mid(148, 4).toInt();
                }
                else if(st.contains("TRAIL", Qt::CaseSensitive))
                {
                    inpfilenbr = st.mid(149, 4).toInt();
                }
            }
            else
            {
                if(st.contains("BODY", Qt::CaseSensitive))
                {
                    inpfilenbr = st.mid(143, 4).toInt();
                }
                else if(st.contains("TRAIL", Qt::CaseSensitive))
                {
                    inpfilenbr = st.mid(144, 4).toInt();
                }
            }
        } else
        {
            if(st.contains("BODY", Qt::CaseSensitive))
            {
                inpfilenbr = st.mid(140, 4).toInt();
            }
            else if(st.contains("TRAIL", Qt::CaseSensitive))
            {
                inpfilenbr = st.mid(141, 4).toInt();
            }
        }

        if(inpfilenbr == filenbr)
        {
            strlistout.append(*itc);
        }
        itc++;
    }

    // If you want to sort your strings in an arbitrary order, consider using the QMap class.
    // For example, you could use a QMap<QString, QString> to create a case-insensitive ordering
    // (e.g. with the keys being lower-case versions of the strings, and the values being the strings),
    // or a QMap<int, QString> to sort the strings by some integer index.

//    for (int j = 0; j < strlistout.size(); ++j)
//    {
//        qDebug() << QString("getGeostationarySegmentsMTG out ======= %1  %2    %3").arg(imagetype).arg(j).arg(strlistout.at(j));
//    }

    return strlistout;
}

QStringList FormGeostationary::getGeostationarySegmentsMTGAlt(int geoindex, const QString imagetype, const QString filepath, int filenbr)
{
    QStringList outlist;
    QMap<int, QMap< int, QFileInfo > > mapmtgi1;
    mapmtgi1 = segs->segmentlistmapgeomtgi1;

    QMap< int, QFileInfo > mapfilenbr;
    mapfilenbr = mapmtgi1.value(filenbr);

    QMap<int, QFileInfo>::const_iterator i = mapfilenbr.constBegin();
    while (i != mapfilenbr.constEnd()) {
        outlist << i.value().fileName();
        if(i.value().fileName().contains("BODY"))
            qDebug() << "key = " << i.key() << " filename = " << i.value().fileName().midRef(85);
        else
            qDebug() << "key = " << i.key() << " filename = " << i.value().fileName().midRef(86);

        ++i;
    }
    return outlist;
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

void FormGeostationary::PopulateTree(QDate seldate)
{

    qDebug() << "FormGeostationary::PopulateTree() selection date = " << seldate.toString();

    for(int i = 0; i < opts.geosatellites.count(); i++)
    {
        if(opts.geosatellites.at(i).shortname != "MTG-I1")
            PopulateTreeGeo(i);
        else if(opts.geosatellites.at(i).shortname == "MTG-I1")
            PopulateTreeGeoMTGI1(i);
    }

}

void FormGeostationary::PopulateTreeGeoMTGI1(int geoindex)
{

    int filenbr;// 1 --> 144
    int seqnbr; // 1 --> 41
    QStringList strlist;
    int seqarray[41];
    QTreeWidgetItem *newitem;
    QColor col;
    bool seqok;

    QMap<int, QMap< int, QFileInfo > > mapmtgi1;
    mapmtgi1 = segs->segmentlistmapgeomtgi1;

    QTreeWidget *widget;
    widget = geotreewidgetlist.at(geoindex);
    widget->clear();

    QMap<int, QMap< int, QFileInfo > >::const_iterator citfilenbr = mapmtgi1.constBegin();

    while (citfilenbr != mapmtgi1.constEnd())
    {
        filenbr = citfilenbr.key();
        //qDebug() << filenbr;

        for(int i = 0; i < 41; i++)
            seqarray[i] = 0;
        seqok = true;

        QMap<int, QFileInfo> mapseqnbr;
        mapseqnbr = mapmtgi1.value(filenbr);
        QMap< int, QFileInfo >::const_iterator citseqnbr = mapseqnbr.constBegin();
        while (citseqnbr != mapseqnbr.constEnd())
        {
            seqnbr = citseqnbr.key();
            seqarray[seqnbr-1] = 1;
            QFileInfo fileinfo = citseqnbr.value();
            //qDebug() << fileinfo.absoluteFilePath();
            ++citseqnbr;
        }

        for(int i = 0; i < 41; i++)
        {
            if(seqarray[i] == 0)
            {
                seqok = false;
                break;
            }
        }

        strlist.clear();
        QString strtime;
        getTimeFromFilenbr(filenbr, &strtime);

        strlist << strtime << QString("%1").arg(filenbr);

        for(int i = 0; i < 41; i++)
        {
            strlist << QString("%1").arg(seqarray[i]);
        }

        newitem = new QTreeWidgetItem( widget, strlist, 0  );

        if(seqok)
            col.setRgb(174, 225, 184);
        else
            col.setRgb(225, 171, 196);


        for(int i = 0; i < 43; i++)
        {
            if(i == 1 && !seqok)
            {
                col.setRgb(225, 171, 196);
            }
            else if(i > 1 && seqarray[i - 2] == 0 )
            {
                col.setRgb(225, 171, 196);
            }
            else
            {
                col.setRgb(174, 225, 184);
            }
            newitem->setBackground( i, QBrush(col) );
        }
        ++citfilenbr;
    }
}

void FormGeostationary::getTimeFromFilenbr(int filenbr, QString *strtime)
{
    double intpart;
    double hrs = filenbr/6.0;
    int hour;
    double fractpart = modf(hrs, &intpart);

    QDate mydate = segs->selectiondate;
    QString yeardir = mydate.toString("yyyyMMdd").mid(0, 4);
    QString monthdir = mydate.toString("yyyyMMdd").mid(4, 2);
    QString daydir = mydate.toString("yyyyMMdd").mid(6, 2);

    hour = (int)intpart;
    double dmin = fractpart * 60.0;
    int min = round(dmin);

    *strtime = QString("%1-%2-%3 %4:%5").arg(yeardir).arg(monthdir).arg(daydir).arg(hour, 2, 10, QChar('0')).arg(min, 2, 10, QChar('0'));
}

void FormGeostationary::PopulateTreeGeo(int geoindex)
{

    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > map;
    map = segs->segmentlistmapgeo.at(geoindex);

    QDate seldate = segs->selectiondate;

    QTreeWidget *widget;
    widget = geotreewidgetlist.at(geoindex);

    QStringList strlist;
    QString strspectrumlist;
    QList<QTreeWidgetItem *> items;

    QTreeWidgetItem *newitem;
    QString strdate;
    QString strspectrum;
    QString filenbr;
    QColor col;

    moonCalc obj;
    if(opts.geosatellites.at(geoindex).shortname == "H9")
    {
        obj.CalcMoon(seldate, geoindex);
    }

    int nbr_spectrum = opts.geosatellites.at(geoindex).spectrumlist.count();
    if(opts.geosatellites.at(geoindex).spectrumhrv.length() > 0)
        nbr_spectrum++;

    QVector<int> cnt_spectrum(nbr_spectrum);

    widget->clear();

    QMap<QString, QMap<QString, QMap< int, QFileInfo > > >::const_iterator citdate = map.constBegin();

    while (citdate != map.constEnd())
    {
        for(int i = 0; i < nbr_spectrum; i++)
            cnt_spectrum[i] = 0;

        strlist.clear();
        strspectrumlist.clear();
        strdate = citdate.key();
        QMap<QString, QMap< int, QFileInfo > > mapspectrum;
        mapspectrum = map.value(strdate);
        QMap<QString, QMap< int, QFileInfo > >::const_iterator citspectrum = mapspectrum.constBegin();
        while (citspectrum != mapspectrum.constEnd())
        {
            strspectrum = citspectrum.key();

            QMap< int, QFileInfo > mapfile;
            mapfile = mapspectrum.value(strspectrum);

            QMap< int, QFileInfo >::const_iterator citfile = mapfile.constBegin();
            while (citfile != mapfile.constEnd())
            {
                filenbr = citfile.key();
                QFileInfo fileinfo = citfile.value();

                //qDebug() << "strdate = " << strdate << " strspectrum = " << strspectrum << " filenbr = " << filenbr << " info = " << fileinfo.completeBaseName();

                if(opts.geosatellites.at(geoindex).spectrumhrv.length() > 0)
                {
                    if(strspectrum == opts.geosatellites.at(geoindex).spectrumhrv)
                        cnt_spectrum[0]++;
                    for(int i = 1; i < nbr_spectrum ; i++)
                    {
                        if(strspectrum == opts.geosatellites.at(geoindex).spectrumlist.at(i-1))
                            cnt_spectrum[i]++;
                    }
                }
                else
                {
                    for(int i = 0; i < nbr_spectrum; i++)
                    {
                        if(strspectrum == opts.geosatellites.at(geoindex).spectrumlist.at(i))
                            cnt_spectrum[i]++;
                    }
                }

                ++citfile;
            }


            ++citspectrum;
        }

        strlist.clear();

        strlist << strdate.mid(0,4) + "-" + strdate.mid(4, 2) + "-" + strdate.mid(6, 2) + "   " + strdate.mid(8,2) + ":" + strdate.mid(10, 2) << strspectrumlist;
        for(int i = 0; i < nbr_spectrum; i++)
        {
            strlist << QString("%1").arg(cnt_spectrum.at(i));
            //            qDebug() << "cnt_spectrum " << i << " " << strlist;
        }



        if(opts.geosatellites.at(geoindex).shortname == "H9")
        {
            int timeindex = obj.getTimeIndex(strdate.mid(8,2), strdate.mid(10, 2));
            if(obj.moonIsVisible(timeindex) > 0)
                strlist << QString("%1").arg(obj.moonIsVisible(timeindex));
        }

        newitem = new QTreeWidgetItem( widget, strlist, 0  );

        bool spectrumok = true;

        if(opts.geosatellites.at(geoindex).spectrumhrv.length() > 0)
        {
            if(cnt_spectrum[0] != opts.geosatellites.at(geoindex).maxsegmentshrv)
            {
                spectrumok = false;
            }
            else
            {
                for(int i = 1; i < nbr_spectrum ; i++)
                {
                    if(cnt_spectrum[i] != opts.geosatellites.at(geoindex).maxsegments)
                    {
                        spectrumok = false;
                        break;
                    }
                }
            }
        }
        else
        {
            for(int i = 0; i < nbr_spectrum ; i++)
            {
                if(cnt_spectrum[i] != opts.geosatellites.at(geoindex).maxsegments)
                {
                    spectrumok = false;
                    break;
                }
            }
        }
        if(spectrumok)
            col.setRgb(174, 225, 184);
        else
            col.setRgb(225, 171, 196);


        for(int i = 0; i < newitem->columnCount(); i++)
            newitem->setBackground( i, QBrush(col) );

        if(opts.geosatellites.at(geoindex).shortname == "H9")
        {
            int timeindex = obj.getTimeIndex(strdate.mid(8,2), strdate.mid(10, 2));
            if(obj.moonIsVisible(timeindex) > 0)
                newitem->setBackground( newitem->columnCount() - 1, QBrush(Qt::yellow) );
            else
                newitem->setBackground( newitem->columnCount() - 1, QBrush(col) );

        }

        ++citdate;
    }


}

//int FormGeostationary::getTimeIndex(QString h, QString m)
//{
//    int hours = h.toInt();
//    int minutes = m.toInt();

//    return (hours * 6) + (minutes/10);

//}

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
    QString tex1;

    SegmentListGeostationary *sl;

    for(int i = 0; i < opts.geosatellites.count(); i++)
    {
        QList<QTreeWidgetItem *> treewidgetselected = geotreewidgetlist.at(i)->selectedItems();
        if(treewidgetselected.count() > 0)
        {
            sl = setActiveSegmentList(i);
            qDebug() << "getGeoSatelliteIndex = " << sl->getGeoSatelliteIndex();
            QTreeWidgetItem *it = treewidgetselected.at(0);
            tex = it->text(0);
            tex1 = it->text(1);
            break;
        }
    }

    int geoindex = sl->getGeoSatelliteIndex();

    if(opts.geosatellites.at(geoindex).shortname == "MTG-I1") {
        CreateGeoImageMTG(type, spectrumvector, inversevector, histogrammethod, pseudocolor, tex + ";" + tex1, geoindex);
    }
    else {
        CreateGeoImages(type, spectrumvector, inversevector, histogrammethod, pseudocolor, tex, geoindex);
    }


}

void FormGeostationary::CreateGeoImages(QString type, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor, QString tex, int geoindex)
{

    SegmentListGeostationary *sl;
    sl = setActiveSegmentList(geoindex);

    sl->setKindofImage(type);

    formtoolbox->createImageFilenamestring(opts.geosatellites.at(geoindex).shortname, tex, spectrumvector);
    imageptrs->ResetPtrImage();

    qDebug() << QString(" CreateGeoImages ; kind of image = %1 spectrumvector = %2 ; %3 ; %4").arg(sl->getKindofImage()).arg(spectrumvector.at(0)).arg(spectrumvector.at(1)).arg(spectrumvector.at(2));
    qDebug() << QString(" CreateGeoImages ; imagecreated = %1").arg(tex);

    if (spectrumvector.at(0) == "" &&  spectrumvector.at(1) == "" && spectrumvector.at(1) == "")
        return;

    sl->setSpectrumVector(spectrumvector);

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

    qDebug() << QString("FormGeostationary::CreateGeoImages kind = %1 areatype = %2").arg(type).arg(sl->areatype);

    if(opts.geosatellites.at(geoindex).protocol == "HDF" )
        CreateGeoImageHDF(sl, type, tex, spectrumvector, inversevector);
    else if(opts.geosatellites.at(geoindex).protocol == "netCDF")
        CreateGeoImagenetCDF(sl, type, tex, spectrumvector, inversevector, histogrammethod, pseudocolor);
    else
        CreateGeoImageXRIT(sl, type, tex, spectrumvector, inversevector, histogrammethod);

}

void FormGeostationary::CreateGeoImageMTG(QString type, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor, QString tex, int geoindex)
{
    SegmentListGeostationary *sl;
    sl = setActiveSegmentList(geoindex);

    sl->setSpectrumVector(spectrumvector);

    imageptrs->ResetPtrImage();

    CreateGeoImagenetCDFMTG(sl, type, tex, spectrumvector, inversevector, histogrammethod, pseudocolor);


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

    QApplication::setOverrideCursor(( Qt::WaitCursor));

    formtoolbox->setProgressValue(10);

    int geoindex = sl->getGeoSatelliteIndex();

    filetiming = tex.mid(0, 4) + tex.mid(5, 2) + tex.mid(8, 2) + tex.mid(13, 2) + tex.mid(16, 2);

    filepattern = QString(opts.geosatellites.at(geoindex).filepattern).arg(filetiming.mid(0, opts.geosatellites.at(geoindex).lengthdate));

    qDebug() << "FormGeostationary::CreateGeoImageXRIT filepattern = " << filepattern;

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
        if(sl->getGeoSatellite() != eGeoSatellite::H9)
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
        if(sl->getGeoSatellite() != eGeoSatellite::H9)
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
        if(sl->getGeoSatellite() != eGeoSatellite::H9)
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
    MSG_header header;
    float subsatpoint;

    if(sl->getGeoSatellite() != eGeoSatellite::H9)
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
                    //subsatpoint = proheader.image_navigation->subsatellite_longitude;
                    //qDebug() << "------> subsatpoint = " << subsatpoint;
                    //qDebug() << "reading prologue";
                    //qDebug() << proheader.;
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

            if(epiloguefile == "")
            {
                QApplication::restoreOverrideCursor();
                QMessageBox msgBox;
                msgBox.setText("There is no epilogue file.");
                msgBox.exec();


                return;
            }
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


    if(sl->getGeoSatellite() == eGeoSatellite::MET_11)
    {
        qDebug() << "ForwardScanStart = " << epidata.epilogue->product_stats.ActualScanningSummary.ForwardScanStart.get_timestring().c_str();
        //    cout << epi.epilogue->product_stats.ActualScanningSummary.ForwardScanStart;
        qDebug() << "ForwardScanEnd = " << epidata.epilogue->product_stats.ActualScanningSummary.ForwardScanEnd.get_timestring().c_str();
        //    cout << epi.epilogue->product_stats.ActualScanningSummary.ForwardScanEnd;

        da.scan(fa, header);

        qDebug() << "COFF = " << header.image_navigation->COFF;
        qDebug() << "LOFF = " << header.image_navigation->LOFF;
        qDebug() << "CFAC = " << header.image_navigation->CFAC;
        qDebug() << "LFAC = " << header.image_navigation->LFAC;

        qDebug() << QString("column scaling factor = %1").arg(header.image_navigation->column_scaling_factor); //, 16, 'f', 2);
        qDebug() << QString("line scaling factor = %1").arg(header.image_navigation->line_scaling_factor); //, 16, 'f', 2);
        qDebug() << QString("column offset = %1").arg(header.image_navigation->column_offset); //, 16, 'f', 2);
        qDebug() << QString("line offset = %1").arg(header.image_navigation->line_offset); //, 16, 'f', 2);


    }
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


    if(type == "VIS_IR" || type == "VIS_IR Color")
    {
        sl->ComposeImageXRITMSGInThread(llVIS_IR, QStringList(), spectrumvector, inversevector, histogrammethod);
    }
    else if( type == "HRV Color")
    {
        sl->ComposeImageXRITMSGInThread(llVIS_IR, llHRV, spectrumvector, inversevector, histogrammethod);
    }
    else if( type == "HRV")
    {
        sl->ComposeImageXRITMSGInThread(QStringList(), llHRV, spectrumvector, inversevector, histogrammethod);
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


    if(whichgeo == eGeoSatellite::FY2H && (type == "VIS_IR" || type == "VIS_IR Color"))
    {
        filepatterngz = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2H_FDI_???") + QString("_001_NOM.HDF.gz");
        filepattern = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2H_FDI_???") + QString("_001_NOM.HDF");
    }
    else if(whichgeo == eGeoSatellite::FY2G && (type == "VIS_IR" || type == "VIS_IR Color"))
    {
        filepatterngz = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2G_FDI_???") + QString("_001_NOM.HDF.gz");
        filepattern = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2G_FDI_???") + QString("_001_NOM.HDF");
    }
    else if(whichgeo == eGeoSatellite::FY2H && type == "HRV")
    {
        filepatterngz = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2H_FDI_VIS1KM") + QString("_001_NOM.HDF.gz");
        filepattern = QString("Z_SATE_C_BABJ_") + filetiming + QString("_O_FY2H_FDI_VIS1KM") + QString("_001_NOM.HDF");
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

    QString filetiming_goes;
    QString filetiming_mtg;
    QStringList llVIS_IR;
    QString filepattern;

    QApplication::setOverrideCursor(( Qt::WaitCursor));

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
    filetiming_goes = tex.mid(0, 4) + QString("%1").arg(DDD).rightJustified(3, '0') + tex.mid(13, 2) + tex.mid(16, 2);

    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    //          1         2         3         4         5         6         7         8         9         10        11        12        13        14        15
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    //W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-BODY---NC4E_C_EUMT_20170920113515_GTT_DEV_20170920113008_20170920113015_N__T_0070_0001.nc
    //W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-TRAIL---NC4E_C_EUMT_20170920114422_GTT_DEV_20170920113008_20170920113922_N__T_0070_0041.nc

    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    //          1         2         3         4         5         6         7         8         9         10        11        12        13        14        15
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    //W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-BODY---NC4E_C_EUMT_20170920113515_GTT_DEV_20170920113008_20170920113015_N_JLS_T_0070_0001.nc
    //W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-TRAIL---NC4E_C_EUMT_20170920114422_GTT_DEV_20170920113008_20170920113922_N_JLS_T_0070_0041.nc

    //OR_ABI-L1b-RadF-M4C01_G16_s20161811455312_e20161811500122_c20161811500175.nc
    //01234567890123456789012345678901234567890123456789

    if((whichgeo == eGeoSatellite::GOES_16) && (type == "VIS_IR" || type == "VIS_IR Color"))
        filepattern = QString("OR_ABI-L1b-RadF-M????_G16_s") + filetiming_goes + QString("*.nc");
    else if((whichgeo == eGeoSatellite::GOES_17) && (type == "VIS_IR" || type == "VIS_IR Color"))
        filepattern = QString("OR_ABI-L1b-RadF-M????_G17_s") + filetiming_goes + QString("*.nc");
    else if((whichgeo == eGeoSatellite::GOES_18) && (type == "VIS_IR" || type == "VIS_IR Color"))
        filepattern = QString("OR_ABI-L1b-RadF-M????_G18_s") + filetiming_goes + QString("*.nc");
    else if(whichgeo == eGeoSatellite::MTG_I1)
        filepattern = QString("W_XX") + filetiming_mtg + QString("*.nc");
    else
        return;


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
        {
            sl->setThreadParametersnetCDF(llVIS_IR, spectrumvector, inversevector, histogrammethod, pseudocolor);
            sl->ComposeImagenetCDFInThread(llVIS_IR, spectrumvector, inversevector, histogrammethod, pseudocolor);
        }
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

void FormGeostationary::CreateGeoImagenetCDFMTG(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor)
{

    QString filetiming_mtg;
    QStringList llVIS_IR;

    eGeoSatellite whichgeo = sl->getGeoSatellite();
    int geoindex = sl->getGeoSatelliteIndex();

    formtoolbox->setProgressMaximum(100);

    qDebug() << "====> tex = " << tex;
    //"2017-08-10   19:45"
    //              012345678901234567890
    //====> tex =  "2017-09-20 11:00;66"
    QDate now(tex.mid(0, 4).toInt(), tex.mid(5, 2).toInt(), tex.mid(8, 2).toInt());
    int filenbr = tex.mid(17).toInt();

    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    //          1         2         3         4         5         6         7         8         9         10        11        12        13        14        15
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    //W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-BODY---NC4E_C_EUMT_20170920113515_GTT_DEV_20170920113008_20170920113015_N__T_0070_0001.nc
    //W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-TRAIL---NC4E_C_EUMT_20170920114422_GTT_DEV_20170920113008_20170920113922_N__T_0070_0041.nc

    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    //          1         2         3         4         5         6         7         8         9         10        11        12        13        14        15
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    //W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-BODY---NC4E_C_EUMT_20170920113515_GTT_DEV_20170920113008_20170920113015_N_JLS_T_0070_0001.nc
    //W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-TRAIL---NC4E_C_EUMT_20170920114422_GTT_DEV_20170920113008_20170920113922_N_JLS_T_0070_0041.nc

    sl->COFF = opts.geosatellites.at(geoindex).coffhrv;
    sl->LOFF = opts.geosatellites.at(geoindex).loffhrv;
    sl->CFAC = opts.geosatellites.at(geoindex).cfachrv;
    sl->LFAC = opts.geosatellites.at(geoindex).lfachrv;

    if(type == "VIS_IR" || type == "VIS_IR Color")
    {
        llVIS_IR = this->getGeostationarySegmentsMTGAlt(geoindex, type, sl->getImagePath(), filenbr);
        qDebug() << QString("llVIS_IR count = %1").arg(llVIS_IR.count());
        if(llVIS_IR.count() == 0)
        {
            QApplication::restoreOverrideCursor();
            QMessageBox msgBox;
            msgBox.setText("No segments found !");
            msgBox.exec();
            emit enabletoolboxbuttons(true);
            return;
        }
        else
        {
            sl->setThreadParametersnetCDF(llVIS_IR, spectrumvector, inversevector, histogrammethod, pseudocolor);
            sl->ComposeImagenetCDFMTGInThread(llVIS_IR, spectrumvector, inversevector, histogrammethod, pseudocolor);
        }
    }
}

void FormGeostationary::SelectGeoWidgetItem(int geoindex, QTreeWidgetItem *item, int column )
{

    if ( !item )
        return;

    qDebug() << "FormGeostationary::SelectGeoWidgetItem";

    for(int i = 0; i < opts.geosatellites.count(); i++)
        if(opts.geosatellites.at(i).shortname != "MTG-I1")
            setTreeWidget( geotreewidgetlist.at(i), i == geoindex ? true : false);

    qDebug() << opts.geosatellites.at(geoindex).shortname + " " + (*item).text(0);

    imageptrs->ptrimageGeostationary->fill(Qt::black);

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
    qDebug() << "FormGeostationary::on_tabGeostationary_tabBarClicked(int index) index = " << index;

    formimage->setupGeoOverlay(index);

    emit setbuttonlabels(index, false);
}

int FormGeostationary::getTabWidgetGeoIndex()
{
    return ui->tabGeostationary->currentIndex();
}

void FormGeostationary::slotCreateRGBrecipe(int recipe)
{
    QRgb *row_col;

    qDebug() << "CreateRGBrecipe = " << recipe;

    SegmentListGeostationary *sl;
    QString tex;

    for(int i = 0; i < opts.geosatellites.count(); i++)
    {
        QList<QTreeWidgetItem *> treewidgetselected = geotreewidgetlist.at(i)->selectedItems();
        if(treewidgetselected.count() > 0)
        {
            QTreeWidgetItem *it = treewidgetselected.at(0);
            tex = it->text(0);
            sl = setActiveSegmentList(i);
            break;
        }
    }

    sl->ComposeGeoRGBRecipe(recipe, tex);

}

//void FormGeostationary::CalcMoon(QDate selected, int geosatindex)
//{
//    double hours, minutes;
//    int day, year, month;
//    int min = 10;
//    int timeindex;
//    year = selected.year();
//    month = selected.month();
//    day = selected.day();

//    double opposietlong = MapToMinus180To180Range(180 - opts.geosatellites.at(geosatindex).longitude);


//    QBrush moonBrush(Qt::yellow);
//    QPen blackpen(Qt::black);
//    blackpen.setWidth(1);

////    while (!moonlist.isEmpty())
////        scene->removeItem(moonlist.takeFirst());
////    moonlist.clear();

////    while (!textlist.isEmpty())
////        scene->removeItem(textlist.takeFirst());
////    scene->update();
////    textlist.clear();

////    qDebug() << QString("Calculating for satellite at %1°").arg(geolonlist.at(geosatindex));

////    ui->listWidget->clear();

//    for( int hours = 0; hours < 24; hours++)
//    {
//        for( int minutes = 0; minutes < 60; minutes += min)
//        {
//            timeindex = (hours * 6) + (minutes/min);
//            double dday =  static_cast<double>(day) + static_cast<double>(hours)/24.0 + static_cast<double>(minutes)/(24.0*60.0);
//            double JD = CAADate::DateToJD(year, month, dday, true);

//            double JDMoon = CAADynamicalTime::UTC2TT(JD);
//            double MoonLong = CAAELP2000::EclipticLongitude(JDMoon);
//            double MoonLat = CAAELP2000::EclipticLatitude(JDMoon);

//            int MoonLongdeg, MoonLongmin, MoonLongsec;
//            int MoonLatdeg, MoonLatmin, MoonLatsec;
//            HoursTohms(MoonLong, MoonLongdeg, MoonLongmin, MoonLongsec);
//            HoursTohms(MoonLat, MoonLatdeg, MoonLatmin, MoonLatsec);
//            CAA2DCoordinate Equatorial = CAACoordinateTransformation::Ecliptic2Equatorial(MoonLong, MoonLat, CAANutation::TrueObliquityOfEcliptic(JDMoon));
//            double MoonRad = CAAELP2000::RadiusVector(JDMoon);
//            //MoonRad /= 149597870.691; //Convert KM to AU
//            double AST = CAASidereal::ApparentGreenwichSiderealTime(JDMoon);
//            double LongtitudeAsHourAngle = CAACoordinateTransformation::DegreesToHours( opposietlong );
//            double LocalHourAngle = AST - LongtitudeAsHourAngle - Equatorial.X;

//            double LHA = CAACoordinateTransformation::MapTo0To24Range(LocalHourAngle);
//            int LHAhour, LHAmin, LHAsec;
//            HoursTohms(LHA, LHAhour, LHAmin, LHAsec);
//            int DEdeg, DEmin, DEsec;
//            HoursTohms(Equatorial.Y, DEdeg, DEmin, DEsec);
//            int RAhour, RAmin, RAsec;
//            HoursTohms(Equatorial.X, RAhour, RAmin, RAsec);
//            //printf("%d/%d/%d %2d:%02d UTC Ecliptic long = %02d°%02d'%02d\" lat = %02d°%02d'%02d\" RA = %02dh%02dm%02ds localHourAngle = %02dh%02dm%02ds DE = %02d°%02d'%02d\" MoonRad = %f\n",
//            //   year, month, day, hours, minutes, MoonLongdeg, MoonLongmin, MoonLongsec, MoonLatdeg, MoonLatmin, MoonLatsec, RAhour, RAmin, RAsec, LHAhour, LHAmin, LHAsec, DEdeg, DEmin, DEsec, MoonRad);

//            double diffangle = 9.0;

//            double HourAngleDegrees = MapToMinus180To180Range(CAACoordinateTransformation::HoursToDegrees(LHA));
//            double DecLHA = sqrt(HourAngleDegrees * HourAngleDegrees + Equatorial.Y * Equatorial.Y);
//            double parallax = 5.92 * sin(CAACoordinateTransformation::DegreesToRadians(DecLHA));
//            double deltaY = Equatorial.Y*parallax/DecLHA;
//            double deltaX = HourAngleDegrees*parallax/DecLHA;

//            double illuminated_fraction = 0;
//            double position_angle = 0;
//            double phase_angle = 0;

//            GetMoonIllumination(JD, true, illuminated_fraction, position_angle, phase_angle);

//            if(HourAngleDegrees - deltaX > -diffangle && HourAngleDegrees - deltaX < diffangle && Equatorial.Y - deltaY < diffangle && Equatorial.Y - deltaY > -diffangle)
//            {

//                printf("==%d/%d/%d %2d:%02d UTC Ecliptic long = %f° lat = %f° RA = %fh DE = %f° localHourAngle = %f° AST = %f MoonRad = %f parallax = %f\n",
//                       year, month, day, hours, minutes, MoonLong, MoonLat, Equatorial.X, Equatorial.Y, HourAngleDegrees, AST, MoonRad, parallax);
//                fflush(stdout);
//                int ilfraction = static_cast<int>((illuminated_fraction * 100) + 0.5);
//                moonVisible[timeindex] = ilfraction;

////                int mooncoordX = (int)((HourAngleDegrees - deltaX) * scale);
////                int mooncoordY = - (int)((Equatorial.Y - deltaY) * scale);

////                moonlist.append(scene->addEllipse(mooncoordX - 0.25*scale, mooncoordY - 0.25*scale, 0.5*scale, 0.5*scale, blackpen, moonBrush));
////                QGraphicsTextItem *text = scene->addText(QString("%1:%2").arg(hours, 2, 'f', 0, '0').arg(minutes, 2, 'f', 0, '0'));
////                textlist.append(text);
////                text->setPos(mooncoordX-scale, mooncoordY+0.5*scale);
////                int ilfraction = static_cast<int>((illuminated_fraction * 100) + 0.5);

////                ui->listWidget->addItem(new QListWidgetItem(QString("%1:%2 illumination = %3 ").arg(hours, 2, 'f', 0, '0').arg(minutes, 2, 'f', 0, '0').arg(ilfraction), ui->listWidget));

//            }
//            else
//                moonVisible[timeindex] = 0;
//        }
//    }

//    for(int i = 0; i < 144; i++)
//    {
//        qDebug() << QString("%1 illumated fraction = %2").arg(i).arg(moonVisible[i]);
//    }
//}

//void FormGeostationary::GetMoonIllumination(double JD, bool bHighPrecision, double& illuminated_fraction, double& position_angle, double& phase_angle)
//{
//  double moon_alpha{0};
//  double moon_delta{0};
//  GetLunarRaDecByJulian(JD, moon_alpha, moon_delta);
//  double sun_alpha{0};
//  double sun_delta{0};
//  GetSolarRaDecByJulian(JD, bHighPrecision, sun_alpha, sun_delta);
//  const double geo_elongation{CAAMoonIlluminatedFraction::GeocentricElongation(moon_alpha, moon_delta, sun_alpha, sun_delta)};

//  position_angle = CAAMoonIlluminatedFraction::PositionAngle(sun_alpha, sun_delta, moon_alpha, moon_delta);
//  phase_angle = CAAMoonIlluminatedFraction::PhaseAngle(geo_elongation, 368410.0, 149971520.0);
//  illuminated_fraction = CAAMoonIlluminatedFraction::IlluminatedFraction(phase_angle);
//}

//void FormGeostationary::GetLunarRaDecByJulian(double JD, double& RA, double& Dec)
//{
//  const double JDMoon{CAADynamicalTime::UTC2TT(JD)};
//  const double lambda{CAAMoon::EclipticLongitude(JDMoon)};
//  const double beta{CAAMoon::EclipticLatitude(JDMoon)};
//  const double epsilon{CAANutation::TrueObliquityOfEcliptic(JDMoon)};
//  CAA2DCoordinate Lunarcoord{CAACoordinateTransformation::Ecliptic2Equatorial(lambda, beta, epsilon)};
//  RA = Lunarcoord.X;
//  Dec = Lunarcoord.Y;
//}

//void FormGeostationary::GetSolarRaDecByJulian(double JD, bool bHighPrecision, double& RA, double& Dec)
//{
//  const double JDSun{CAADynamicalTime::UTC2TT(JD)};
//  const double lambda{CAASun::ApparentEclipticLongitude(JDSun, bHighPrecision)};
//  const double beta{CAASun::ApparentEclipticLatitude(JDSun, bHighPrecision)};
//  const double epsilon{CAANutation::TrueObliquityOfEcliptic(JDSun)};
//  CAA2DCoordinate Solarcoord{CAACoordinateTransformation::Ecliptic2Equatorial(lambda, beta, epsilon)};
//  RA = Solarcoord.X;
//  Dec = Solarcoord.Y;
//}


//inline double FormGeostationary::MapToMinus180To180Range(double Degrees)
//  {
//    double fResult = CAACoordinateTransformation::MapTo0To360Range(Degrees);

//    if (fResult > 180)
//      fResult = fResult - 360;

//    return fResult;
//  }
