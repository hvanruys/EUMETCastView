#include <QFile>
#include <QDebug>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <qregularexpression.h>
#include "rssvideo.h"
#include "qsun.h"
#include "qeci.h"
#include "qobserver.h"



#define CMB_HISTO_NONE_95 0
#define CMB_HISTO_NONE_100 1
#define CMB_HISTO_EQUALIZE 2
#define CMB_HISTO_CLAHE 3
#define uiNR_OF_GREY (4096)
const unsigned int uiMAX_REG_X = 16;	  /* max. # contextual regions in x-direction */
const unsigned int uiMAX_REG_Y = 16;	  /* max. # contextual regions in y-direction */

void RSSVideo::doCompileImage(RSSVideo *rv, QString date, QString path, int i)
{
    rv->compileImage(date, path, i);
}

RSSVideo::RSSVideo( QString xmlfile, QString argsingleimage, QObject *parent ) : QObject(parent)
{
    reader = new XMLVideoReader(xmlfile, argsingleimage);
    gshhs = new gshhsData(reader->gshhsoverlayfiles);
    gshhs->setupGeoOverlay(reader->satlon, reader->coff, reader->loff, reader->cfac, reader->lfac);
    this->SetupContrastStretch( 0, 0, 1023, 255);

    overlayimageProjection = QImage(reader->videowidth, reader->videoheight, QImage::Format_ARGB32);

    OverlayProjectionGVP();
    // overlayimageProjection.save("overlayproj.png");

    udpSocket = new QUdpSocket();


}

RSSVideo::~RSSVideo()
{

}

void RSSVideo::sendMessages(QString txt)
{

    QByteArray ba = txt.toLocal8Bit();
    this->udpSocket->writeDatagram(ba, QHostAddress::LocalHost, 7755);
    qDebug() << txt;

}


QVector<QString> RSSVideo::getDateVectorFromDir()
{
    QVector<QString> out;
    QFileInfoList fileinfolist;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmap;
    QString strspectrum;
    QString strdate;
    int filenbr;

    foreach (const QString &path, reader->rsspath)
    {
        QDir segmentdir(path);
        segmentdir.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        segmentdir.setSorting(QDir::Name);
        fileinfolist = segmentdir.entryInfoList();

        //            for(int i = 0; i < fileinfolist.count(); i++)
        //            {
        //                qDebug() << path << "  " << fileinfolist.at(i).absoluteFilePath();
        //            }


        foreach (const QFileInfo &fileInfo, fileinfolist)
        {
            getFilenameParameters(fileInfo.fileName(), strspectrum, strdate, filenbr);

            if (strspectrum != "___")
            {
                QMap<int, QFileInfo> hashfile;
                QMap<QString, QMap<int, QFileInfo> > hashspectrum;

                if (segmentlistmap.contains(strdate))
                {
                    hashspectrum = segmentlistmap.value(strdate);
                    if (hashspectrum.contains(strspectrum))
                        hashfile = hashspectrum.value(strspectrum);
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert( strspectrum, hashfile);
                    segmentlistmap.insert(strdate, hashspectrum);
                }
                else
                {
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert(strspectrum, hashfile);
                    segmentlistmap.insert( strdate, hashspectrum );
                }
            }
        }
    }

    QMap<QString, QMap<QString, QMap< int, QFileInfo > > >::const_iterator citdate = segmentlistmap.constBegin();

    while (citdate != segmentlistmap.constEnd())
    {
        strdate = citdate.key();
        out.append(strdate);
        //        qDebug() << "strdate = " << strdate;
        ++citdate;
    }
    qDebug() << "segmentlistmap count = " << segmentlistmap.count();

    return out;
}

void RSSVideo::getDatePathVectorFromDir( QStringList *datelist, QStringList *pathlist)
{
    QFileInfoList fileinfolist;
    QString strspectrum;
    QString strdate;
    int filenbr;

    QRegularExpression rx(this->reader->filepattern);
    //rx.setPatternSyntax(QRegExp::Wildcard);


    foreach (const QString &path, reader->rsspath)
    {
        QDir segmentdir(path);
        segmentdir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        segmentdir.setSorting(QDir::Name);
        fileinfolist = segmentdir.entryInfoList();

        //        qDebug() << "nbr fileinfolist = " << fileinfolist.count();

        //        for(int i = 0; i < fileinfolist.count(); i++)
        //        {
        //            //if(rx.exactMatch(fileinfolist.at(i).fileName()))
        //            {
        //                qDebug() << fileinfolist.at(i).absoluteFilePath() ;
        //            }
        //        }


        foreach (const QFileInfo &fileInfo, fileinfolist)
        {
            QRegularExpressionMatch match = rx.match(fileInfo.fileName());
            if(match.hasMatch())
            {
                getFilenameParameters(fileInfo.fileName(), strspectrum, strdate, filenbr);

                if (strspectrum != "___")
                {
                    if (!datelist->contains(strdate))
                    {
                        datelist->append(strdate);
                        pathlist->append(path);
                    }
                }
            }
        }
    }
}

void RSSVideo::compileImagesInBetween(QStringList datelist, QStringList pathlist)
{

    QString datefrom;
    QString dateto;
    quint16 *ptrDayRed1;
    quint16 *ptrDayRed2;
    quint16 *ptrDayRedfrom;
    quint16 *ptrDayRedto;
    bool even = false;

    ptrDayRed1 = new quint16[3712 * (reader->brss ? 1392 : 3712)];
    memset(ptrDayRed1, 0, 3712 * (reader->brss ? 1392 : 3712) * sizeof(quint16));
    ptrDayRed2 = new quint16[3712 * (reader->brss ? 1392 : 3712)];
    memset(ptrDayRed2, 0, 3712 * (reader->brss ? 1392 : 3712) * sizeof(quint16));

    for(int i = 0; i < datelist.size() - 1; i++)
    {
        datefrom = datelist.at(i);
        dateto = datelist.at(i+1);
        qDebug() << datefrom << dateto;
        // compile and do images in between
        if(i == 0)
        {
            ptrDayRedfrom = ptrDayRed1;
            ptrDayRedto = ptrDayRed2;

            //get data to ptrDayRed1(datefrom)
            //get data to ptrDayRed2(dateto)
            even = false;
        }
        else if(even == false)
        {
            ptrDayRedfrom = ptrDayRed2;
            ptrDayRedto = ptrDayRed1;
            // get data to ptrDayRed1/to(dateto)
        }
        else if(even == true)
        {
            ptrDayRedfrom = ptrDayRed1;
            ptrDayRedto = ptrDayRed2;
            // get data to ptrDayRed2/to(dateto)

        }

    }

    delete ptrDayRed1;
    delete ptrDayRed2;

}

void RSSVideo::compileImage(QString date, QString path, int imagenbr)
{
    QStringList llVIS_IR;
    QStringList llHRV;
    MsgFileAccess faVIS_IR;
    MsgFileAccess faHRV;
    MsgDataAccess da;

    MSG_data prodata;
    MSG_data epidata;
    QString prologuefile;
    QString epiloguefile;
    MSG_header epiheader;
    MSG_header proheader;
    MSG_header header;

    quint16 *ptrDayRed;
    quint16 *ptrDayGreen;
    quint16 *ptrDayBlue;
    quint16 *ptrNightRed;
    quint16 *ptrNightGreen;
    quint16 *ptrNightBlue;
    quint16 *ptrHRV;

    ptrDayRed = NULL;
    ptrDayGreen = NULL;
    ptrDayBlue = NULL;
    ptrNightRed = NULL;
    ptrNightGreen = NULL;
    ptrNightBlue = NULL;
    ptrHRV = NULL;

    QImage imagevisir;
    QImage imagehrv;

    int LECA = 0;
    int LNLA = 0;
    int LWCA = 0;
    int LSLA = 0;
    int UECA = 0;
    int USLA = 0;
    int UWCA = 0;
    int UNLA = 0;


    //sendMessages(QString("Start compileImage nbr %1").arg(imagenbr));

    qDebug() << "daykindofimage = " << reader->daykindofimage;
    qDebug() << "nightkindofimage = " << reader->nightkindofimage;

    if(reader->daykindofimage == "VIS_IR")
    {
        ptrDayRed = new quint16[3712 * (reader->brss ? 3*464 : 8*464)];
        memset(ptrDayRed, 0, 3712 * (reader->brss ? 3*464 : 8*464) * sizeof(quint16));
    }

    if(reader->daykindofimage == "VIS_IR Color" || reader->daykindofimage == "HRV Color")
    {
        ptrDayRed = new quint16[3712 * (reader->brss ? 1392 : (reader->bhrv ? 2*464 : 8*464))];
        memset(ptrDayRed, 0, 3712 * (reader->brss ? 1392 : (reader->bhrv ? 2*464 : 8*464)) * sizeof(quint16));
        ptrDayGreen = new quint16[3712 * (reader->brss ? 1392 : (reader->bhrv ? 2*464 : 8*464))];
        memset(ptrDayGreen, 0, 3712 * (reader->brss ? 1392 : (reader->bhrv ? 2*464 : 8*464)) * sizeof(quint16));
        ptrDayBlue = new quint16[3712 * (reader->brss ? 1392 : (reader->bhrv ? 2*464 : 8*464))];
        memset(ptrDayBlue, 0, 3712 * (reader->brss ? 1392 : (reader->bhrv ? 2*464 : 8*464)) * sizeof(quint16));
    }

    if(reader->daykindofimage == "HRV" || reader->daykindofimage == "HRV Color")
    {
        ptrHRV = new quint16[5568 * (reader->brss ? 9*464 : 6*464)];
        memset(ptrHRV, 0, 5568 * (reader->brss ? 9*464 : 6*464) * sizeof(quint16));
        qDebug() << "ptrHRV = " << (reader->brss ? "9 * 464" : "6 * 464");
    }

    if(reader->nightkindofimage == "VIS_IR")
    {
        ptrNightRed = new quint16[3712 * (reader->brss ? 3*464 : 8*464)];
        memset(ptrNightRed, 0, 3712 * (reader->brss ? 3*464 : 8*464) * sizeof(quint16));
    }

    if(reader->nightkindofimage == "VIS_IR Color")
    {
        ptrNightRed = new quint16[3712 * (reader->brss ? 3*464 : 8*464)];
        memset(ptrNightRed, 0, 3712 * (reader->brss ? 3*464 : 8*464) * sizeof(quint16));
        ptrNightGreen = new quint16[3712 * (reader->brss ? 3*464 : 8*464)];
        memset(ptrNightGreen, 0, 3712 * (reader->brss ? 3*464 : 8*464) * sizeof(quint16));
        ptrNightBlue = new quint16[3712 * (reader->brss ? 3*464 : 8*464)];
        memset(ptrNightBlue, 0, 3712 * (reader->brss ? 3*464 : 8*464) * sizeof(quint16));
    }

    QString fpattern = reader->filepattern.replace(46, 12, date);

    if((reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color" || reader->daykindofimage == "HRV Color") ||
            (reader->nightkindofimage == "VIS_IR" || reader->nightkindofimage == "VIS_IR Color"))
    {
        llVIS_IR = getGeostationarySegments("VIS_IR", path, reader->spectrum, fpattern);
        checkAvailableSegments(&llVIS_IR, date);

        if(llVIS_IR.count() == 0)
        {
            sendMessages("Warning : no segments found for 'VIS_IR'!");
            return;
        }
        else
        {
            faVIS_IR.parse(path + "/" + llVIS_IR.at(0));
            prologuefile = faVIS_IR.prologueFile();
            epiloguefile = faVIS_IR.epilogueFile();
        }
    }

    for(int i = 0; i < llVIS_IR.count(); i++)
    {
        qDebug() << "after checkavailableSegments " << llVIS_IR.at(i);
    }

    llVIS_IR.sort();

    if(reader->daykindofimage == "HRV" || reader->daykindofimage == "HRV Color")
    {
        llHRV = getGeostationarySegments("HRV", path, reader->spectrum, fpattern);
        if(llHRV.count() == 0)
        {
            sendMessages("Warning : no segments found for 'HRV'!");
            return;
        }
        else
        {
            faHRV.parse(path + "/" + llHRV.at(0));
            prologuefile = faHRV.prologueFile();
            epiloguefile = faHRV.epilogueFile();
        }
    }
    for(int i = 0; i < llHRV.count(); i++)
    {
        qDebug() << llHRV.at(i);
    }

    // Read prologue
    if (prologuefile.length() > 0)
    {
        try
        {
            da.read_file(prologuefile, proheader, prodata);
        }
        catch( std::runtime_error &run )
        {
            sendMessages(QString("Error : runtime error in reading prologue file : %1").arg(run.what()));
        }
    }

    // Read epilogue
    if (epiloguefile.length() > 0)
    {
        try
        {
            da.read_file(epiloguefile, epiheader, epidata);
        }
        catch( std::runtime_error &run )
        {
            sendMessages(QString("Error : runtime error in reading epilogue file : %1").arg(run.what()));
        }
        MSG_ActualL15CoverageHRV& cov = epidata.epilogue->product_stats.ActualL15CoverageHRV;
        LECA = cov.LowerEastColumnActual;
        LNLA = cov.LowerNorthLineActual;
        LWCA = cov.LowerWestColumnActual;
        LSLA = cov.LowerSouthLineActual;
        UECA = cov.UpperEastColumnActual;
        USLA = cov.UpperSouthLineActual;
        UWCA = cov.UpperWestColumnActual;
        UNLA = cov.UpperNorthLineActual;
        qDebug() << "Lower West : " << LWCA << " East : " << LECA << " North : " << LNLA << " South : " << LSLA;
        qDebug() << "Upper West : " << UWCA << " East : " << UECA << " North : " << UNLA << " South : " << USLA;
    }

    QString filespectrum;
    QString filedate;
    int filesequence;


    if(llVIS_IR.count() > 0)
        qDebug() << "getSegmentSamples for VIS_IR";

    qDebug()  << "0 " << reader->spectrum.at(0);
    qDebug()  << "1 " << reader->spectrum.at(1);
    qDebug()  << "2 " << reader->spectrum.at(2);
    qDebug()  << "3 " << reader->spectrum.at(3);
    qDebug()  << "4 " << reader->spectrum.at(4);
    qDebug()  << "5 " << reader->spectrum.at(5);

    for(int i = 0; i < llVIS_IR.count(); i++)
    {
        getFilenameParameters(llVIS_IR.at(i), filespectrum, filedate, filesequence);
        qDebug() << "filespectrum " << filespectrum << " filedate " << filedate << " sequence " << filesequence;
        bool sampleok = false;
        if(reader->brss)
        {
            if(filesequence >= 5)
                sampleok = true;

        }
        else
        {
            if(reader->bhrv)
            {
                if(filesequence >= 6)
                    sampleok = true;
            }
            else
            {
                sampleok = true;
            }

        }
        if(sampleok == true)
        {
            if(filespectrum == reader->spectrum.at(0))
                getSegmentSamples(path + "/" + llVIS_IR.at(i), ptrDayRed, filesequence, "VISIRList");
            else if(filespectrum == reader->spectrum.at(1))
                getSegmentSamples(path + "/" + llVIS_IR.at(i), ptrDayGreen, filesequence, "VISIRList");
            else if(filespectrum == reader->spectrum.at(2))
                getSegmentSamples(path + "/" + llVIS_IR.at(i), ptrDayBlue, filesequence, "VISIRList");
            else if(filespectrum == reader->spectrum.at(3))
                getSegmentSamples(path + "/" + llVIS_IR.at(i), ptrNightRed, filesequence, "VISIRList");
            else if(filespectrum == reader->spectrum.at(4))
                getSegmentSamples(path + "/" + llVIS_IR.at(i), ptrNightGreen, filesequence, "VISIRList");
            else if(filespectrum == reader->spectrum.at(5))
                getSegmentSamples(path + "/" + llVIS_IR.at(i), ptrNightBlue, filesequence, "VISIRList");
        }

    }

    if(llHRV.count() > 0)
        qDebug() << "getSegmentSamples for HRV";

    for(int i = 0; i < llHRV.count(); i++)
    {
        getFilenameParameters(llHRV.at(i), filespectrum, filedate, filesequence);
        bool sampleok = false;
        if(reader->brss)
        {
            if(filesequence >= 15)
                sampleok = true;
        }
        else
        {
            if(filesequence >= 18)
                sampleok = true;

        }
        if(sampleok == true)
        {
            if(filespectrum == "HRV")
            {
                if((reader->daykindofimage == "HRV" || reader->daykindofimage == "HRV Color") && filesequence >= 15 && reader->brss == true)
                    getSegmentSamples(path + "/" + llHRV.at(i), ptrHRV, filesequence, "HRVList");
                else if((reader->daykindofimage == "HRV" || reader->daykindofimage == "HRV Color") && filesequence >= 18 && reader->brss == false)
                    getSegmentSamples(path + "/" + llHRV.at(i), ptrHRV, filesequence, "HRVList");
            }
        }
    }


    QImage imageGeostationary;

    if(reader->daykindofimage == "HRV" || reader->daykindofimage == "HRV Color")
    {
        this->ComposeHRV1(ptrHRV, ptrDayRed, ptrDayGreen, ptrDayBlue, ptrNightRed, ptrNightGreen, ptrNightBlue, imagehrv, date,
                          LECA, LSLA, LWCA, LNLA, UECA, USLA, UWCA, UNLA, imagenbr);
        imageGeostationary = imagehrv;
        imagehrv.save(QString("tempimages/hrv%1.png").arg(imagenbr, 4, 10, QChar('0')));

        if(reader->projectiontype.length() == 0)
        {
            if(reader->boverlayborder)
                this->OverlayGeostationary(&imagehrv, true, LECA, LSLA, LWCA, LNLA, UECA, USLA, UWCA, UNLA);
            if(reader->boverlaydate)
                this->OverlayDate(&imagehrv, date);
        }

    }
    else
        if(reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color" || reader->nightkindofimage == "VIS_IR" || reader->nightkindofimage == "VIS_IR Color")
        {
            this->ComposeVISIR(ptrDayRed, ptrDayGreen, ptrDayBlue, ptrNightRed, ptrNightGreen, ptrNightBlue, imagevisir, date, imagenbr);
            imageGeostationary = imagevisir;
            imagevisir.save(QString("tempimages/visir%1.png").arg(imagenbr, 4, 10, QChar('0')));

            if(reader->projectiontype.length() == 0)
            {
                if(reader->boverlayborder)
                    this->OverlayGeostationary(&imagevisir, false, LECA, LSLA, LWCA, LNLA, UECA, USLA, UWCA, UNLA);
                if(reader->boverlaydate)
                    this->OverlayDate(&imagevisir, date);
            }
        }


    if(reader->projectiontype == "GVP")
    {
        GeneralVerticalPerspective *gvp = new GeneralVerticalPerspective(reader, this, &imageGeostationary);

        QPainter painter(gvp->imageProjection);
        gvp->CreateMapFromGeoStationary(&painter, LECA, LSLA, LWCA, LNLA, UECA, USLA, UWCA, UNLA);

        if(reader->boverlaydate)
        {
            QFont f("Courier", reader->overlaydatefontsize, QFont::Bold);
            painter.setFont(f);
            painter.setPen(Qt::yellow);
            painter.setBrush(Qt::NoBrush);

            QString year = date.mid(0, 4);
            QString month = date.mid(4, 2);
            QString day = date.mid(6, 2);
            QString hour = date.mid(8, 2);
            QString minute = date.mid(10, 2);

            painter.drawText(20, gvp->imageProjection->height() - 20, QString("%1 %2-%3-%4 %5:%6").arg(reader->satname).arg(year).arg(month).arg(day).arg(hour).arg(minute));
        }


        if(reader->boverlayborder)
        {
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.drawImage(0, 0, overlayimageProjection);
        }

        painter.end();

        QString prefixstr = reader->videooutputname;

        if(reader->singleimage.length() > 0)
            gvp->imageProjection->save("tempimages/" + QString(prefixstr + date + "_%1.png").arg(imagenbr, 4, 10, QChar('0')));
        else
            gvp->imageProjection->save("tempvideo/" + QString(prefixstr + "%1.png").arg(imagenbr, 4, 10, QChar('0')));


        delete gvp;
    }

    if(reader->daykindofimage == "VIS_IR")
    {
        delete ptrDayRed;
    }
    if(reader->daykindofimage == "VIS_IR Color" || reader->daykindofimage == "HRV Color")
    {
        delete ptrDayRed;
        delete ptrDayGreen;
        delete ptrDayBlue;
    }
    if(reader->daykindofimage == "HRV" || reader->daykindofimage == "HRV Color" ||
            reader->daykindofimage == "HRVFull" || reader->daykindofimage == "HRVFull Color")
    {
        delete ptrHRV;
    }
    if(reader->nightkindofimage == "VIS_IR")
    {
        delete ptrNightRed;
    }
    if(reader->nightkindofimage == "VIS_IR Color")
    {
        delete ptrNightRed;
        delete ptrNightGreen;
        delete ptrNightBlue;
    }


}

void RSSVideo::compileImageMTG(QString date, QString path, int imagenbr)
{

}

void RSSVideo::checkAvailableSegments(QStringList *segs, QString date)
{
    int countspectrum = 0;

    if(this->reader->bhrv)
        return;

    if(this->reader->brss)
    {
        for(int i = 0; i < this->reader->spectrum.count(); i++)
        {
            if(this->reader->spectrum.at(i).length() > 0)
                countspectrum++;
        }

        if(segs->count() == countspectrum * 3)
            return;
        else
        {
            qDebug() << "should be " << countspectrum * 3 << " got " << segs->count();
            replenishSegmentsRss(segs, date);
        }
    }
    else
    {
        for(int i = 0; i < this->reader->spectrum.count(); i++)
        {
            if(this->reader->spectrum.at(i).length() > 0)
                countspectrum++;
        }

        if(segs->count() == countspectrum * 8)
            return;
        else
        {
            qDebug() << "should be " << countspectrum * 3 << " got " << segs->count();
            replenishSegmentsFull(segs, date);
        }

    }
}

void RSSVideo::replenishSegmentsRss(QStringList *segs, QString datestr)
{
    QString fpattern = reader->filepattern.replace(46, 12, datestr);
    //H-000-MSG3__-MSG3_????___-??????___-??????___-????????????-?_
    //0         1         2         3         4         5         6
    //0123456789012345678901234567890123456789012345678901234567890
    //H-000-MSG3__-MSG3_RSS____-IR_016___-000006___-202204040740-C_
    //H-000-MSG4__-MSG4________-IR_016___-000001___-201807190700-C_

    QTime mytime(datestr.mid(8, 2).toInt(), datestr.mid(10, 2).toInt());
    for(int i = 0; i < this->reader->spectrum.count(); i++)
    {
        if(this->reader->spectrum.at(i).length() > 0)
        {
            fpattern = fpattern.replace(26, 6, this->reader->spectrum.at(i));
            fpattern = fpattern.replace(18, 4, "RSS_");
            fpattern = fpattern.replace(36, 6, "000006");
            fpattern = fpattern.replace(59, 1, "C");
            isSegmentAvailable(fpattern, segs, mytime);
            fpattern = fpattern.replace(36, 6, "000007");
            isSegmentAvailable(fpattern, segs, mytime);
            fpattern = fpattern.replace(36, 6, "000008");
            isSegmentAvailable(fpattern, segs, mytime);
        }
    }
}

void RSSVideo::replenishSegmentsFull(QStringList *segs, QString datestr)
{
    QString fpattern = reader->filepattern.replace(46, 12, datestr);
    //H-000-MSG3__-MSG3_????___-??????___-??????___-????????????-?_
    //0         1         2         3         4         5         6
    //0123456789012345678901234567890123456789012345678901234567890
    //H-000-MSG3__-MSG3_RSS____-IR_016___-000006___-202204040740-C_
    //H-000-MSG4__-MSG4________-IR_016___-000001___-201807190700-C_

    QTime mytime(datestr.mid(8, 2).toInt(), datestr.mid(10, 2).toInt());
    for(int i = 0; i < this->reader->spectrum.count(); i++)
    {
        if(this->reader->spectrum.at(i).length() > 0)
        {
            fpattern = fpattern.replace(26, 6, this->reader->spectrum.at(i));
            fpattern = fpattern.replace(18, 4, "____");
            fpattern = fpattern.replace(36, 6, "000001");
            fpattern = fpattern.replace(59, 1, "C");
            isSegmentAvailable(fpattern, segs, mytime);
            fpattern = fpattern.replace(36, 6, "000002");
            isSegmentAvailable(fpattern, segs, mytime);
            fpattern = fpattern.replace(36, 6, "000003");
            isSegmentAvailable(fpattern, segs, mytime);
            fpattern = fpattern.replace(36, 6, "000004");
            isSegmentAvailable(fpattern, segs, mytime);
            fpattern = fpattern.replace(36, 6, "000005");
            isSegmentAvailable(fpattern, segs, mytime);
            fpattern = fpattern.replace(36, 6, "000006");
            isSegmentAvailable(fpattern, segs, mytime);
            fpattern = fpattern.replace(36, 6, "000007");
            isSegmentAvailable(fpattern, segs, mytime);
            fpattern = fpattern.replace(36, 6, "000008");
            isSegmentAvailable(fpattern, segs, mytime);
        }
    }
}

bool RSSVideo::isSegmentAvailable(QString segmentstr, QStringList *segs, QTime time)
{
    if(!segs->contains(segmentstr, Qt::CaseInsensitive))
    {
        QTime newtime = time.addSecs(- (this->reader->brss ? 5 : 15) * 60) ;

        QString newsegmentstr = segmentstr.replace(54, 4, newtime.toString("HHmm"));
        segs->append(newsegmentstr);
        return true;
    }
    return false;
}

void RSSVideo::ComposeHRV(quint16 *ptrHRV, quint16 *ptrDayRed, quint16 *ptrDayGreen, quint16 *ptrDayBlue,
                          quint16 *ptrNightRed, quint16 *ptrNightGreen, quint16 *ptrNightBlue, QImage &imhrv, QString date,
                          int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla, int imagenbr)
{
    QRgb *row_col;
    QRgb *row_col_day;
    quint16 cred, cgreen, cblue, c, clum;
    quint16 crednight, cgreennight, cbluenight;
    quint16 rday, gday, bday;
    quint16 rnight, gnight, bnight;

    double gamma = reader->gamma;
    double gammafactor = 1023 / pow(1023, gamma);
    quint16 valgamma;
    quint8 valcontrast;

    long delta = 0;

    pixgeoConversion pixconv;
    double sub_lon = reader->satlon;


    long coff = reader->coffhrv;
    long loff = reader->loffhrv;
    double cfac = reader->cfachrv;
    double lfac = reader->lfachrv;
    double latitude, longitude;
    int ret;

    Vector3 solar_vector;
    Vector3 vel;
    QObserver observer;
    QSgp4Date dat;
    QGeodetic qgeo;
    QTopocentric qtopo;

    double elev;

    int year, month, day, hours, minutes;
    year = date.mid(0, 4).toInt();
    month = date.mid(4, 2).toInt();
    day = date.mid(6, 2).toInt();
    hours = date.mid(8, 2).toInt();
    minutes = date.mid(10, 2).toInt();


    imhrv = QImage(5568, (reader->brss ? 9*464 : 6*464), QImage::Format_ARGB32);
    imhrv.fill(Qt::black);


    this->CLAHE(ptrHRV, 5568, (reader->brss ? 9*464 : 6*464), 0, 1023, 16, 16, 256, 6);


    if(reader->nightkindofimage == "VIS_IR" || reader->nightkindofimage == "VIS_IR Color")
        this->CLAHE(ptrNightRed, 3712, (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464)), 0, 1023, 16, 16, 256, 6);

    if(reader->daykindofimage == "HRV Color")
    {
        this->CLAHE(ptrDayRed, 3712, (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464)), 0, 1023, 16, 16, 256, 6);
        this->CLAHE(ptrDayGreen, 3712, (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464)), 0, 1023, 16, 16, 256, 6);
        this->CLAHE(ptrDayBlue, 3712, (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464)), 0, 1023, 16, 16, 256, 6);
    }

    // test images
    //    if(reader->daykindofimage == "HRV Color")
    //    {
    //        QImage testimage(3712, (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464)), QImage::Format_ARGB32);
    //        for(int y = (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464))-1; y >= 0; y--)
    //        {
    //            row_col = (QRgb*)testimage.scanLine((reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464))-1-y);

    //            for(int x = 0; x < 3712; x++)
    //            {
    //                cred = *(ptrDayRed + y*3712 + x);
    //                cgreen = *(ptrDayGreen + y*3712 + x);
    //                cblue = *(ptrDayBlue + y*3712 + x);
    //                row_col[3712 - x - 1] = qRgb(ContrastStretch(cred), ContrastStretch(cgreen), ContrastStretch(cblue));
    //            }
    //        }

    //        testimage.save("ptrDayVIS.png");
    //    }

    //    if(reader->nightkindofimage == "VIS_IR")
    //    {
    //        QImage testimage(3712, (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464)), QImage::Format_ARGB32);
    //        for(int y = (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464))-1; y >= 0; y--)
    //        {
    //            row_col = (QRgb*)testimage.scanLine((reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464))-1-y);

    //            for(int x = 0; x < 3712; x++)
    //            {
    //                cred = *(ptrNightRed + y*3712 + x);
    //                quint16 c = ContrastStretch(cred);
    //                row_col[3712 - x - 1] = qRgb(c, c, c);
    //            }
    //        }

    //        //this->OverlayGeostationary(&testimage, false, leca, lsla, lwca, lnla, ueca, usla, uwca, unla);

    //        testimage.save("ptrNightIR.png");
    //    }

    //    if(reader->bhrv)
    //    {
    //        QImage testimage(5568, (reader->brss ? 9*464 : 6*464), QImage::Format_ARGB32);
    //        for(int y = (reader->brss ? 9*464 : 6*464)-1; y >= 0; y--)
    //        {
    //            row_col = (QRgb*)testimage.scanLine((reader->brss ? 9*464 : 6*464)-1-y);

    //            for(int x = 0; x < 5568; x++)
    //            {
    //                cred = *(ptrHRV + y*5568 + x);
    //                row_col[5568 - x - 1] = qRgb(ContrastStretch(cred), ContrastStretch(cred), ContrastStretch(cred));
    //            }
    //        }
    //        //this->OverlayGeostationary(&testimage, true, leca, lsla, lwca, lnla, ueca, usla, uwca, unla);

    //        testimage.save("ptrHRV.png");
    //    }


    for (int line = (reader->brss ? 9*464 : 6*464) - 1; line >= 0; line--)
    {
        row_col = (QRgb*)imhrv.scanLine((reader->brss ? 9*464 : 6*464) - 1 - line);

        for (int pixelx = 0; pixelx < 5568; pixelx++)
        {
            c = *(ptrHRV + line * 5568 + pixelx);
            if(reader->brss)
                delta = line/3 * 3712 + leca/3 + pixelx/3;
            else
                delta = line/3 * 3712 + ueca/3 + pixelx/3;
            if(reader->daykindofimage == "HRV Color")
            {
                cred = *(ptrDayRed + delta);
                cgreen = *(ptrDayGreen + delta);
                cblue = *(ptrDayBlue + delta);
                if(reader->nightkindofimage == "VIS_IR")
                    crednight = *(ptrNightRed + delta);
                clum = (cred+cgreen+cblue)/3;
                if( clum == 0)
                    clum = 1;


                valgamma = pow( c*cred/clum, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                rday = quint8(valcontrast);
                if (rday > 255)
                    rday = 255;

                valgamma = pow( c*cgreen/clum, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                gday = quint8(valcontrast);
                if (gday > 255)
                    gday = 255;

                valgamma = pow( c*cblue/clum, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                bday = quint8(valcontrast);
                if (bday > 255)
                    bday = 255;

                if(reader->nightkindofimage == "VIS_IR")
                {
                    crednight = quint16(reader->inverse.at(3) ? 1023 - crednight : crednight);
                    valgamma = pow( crednight, gamma) * gammafactor;
                    if (valgamma >= 1024)
                        valgamma = 1023;

                    valcontrast = ContrastStretch(valgamma);
                    rnight = quint8(valcontrast);
                    if (rnight > 255)
                        rnight = 255;
                }

                //////////////////
                //                r = ContrastStretch(cred);
                //                g = ContrastStretch(cgreen);
                //                b = ContrastStretch(cblue);
                ///////////////////////////

                if(reader->brss)
                {
                    ret = pixconv.pixcoord2geocoord(sub_lon, (5568 - 1) - pixelx +  leca, (9*464 - 1) - line, coff, loff, cfac, lfac, &latitude, &longitude);
                }
                else
                {
                    ret = 0;
                    //if(line < lnla)
                    //    ret = pixconv.pixcoord2geocoord(sub_lon, (5568 - 1) - pixelx + leca, (6*464 - 1) - line, coff, loff, cfac, lfac, &latitude, &longitude);
                    //else
                    ret = pixconv.pixcoord2geocoord(sub_lon, (5568 - 1) - pixelx +  ueca, (6*464 - 1) - line, coff, loff, cfac, lfac, &latitude, &longitude);


                }
                if(ret == -1)
                    //row_col[5568 - 1 - pixelx] = qRgb(0, 0, 0);
                    continue;
                else
                {
                    observer.SetLocation(latitude, longitude, 0.0);
                    dat.Set(year, month, day, hours, minutes, 0, true);
                    QSun::Calculate_Solar_Position(dat.Julian(), &solar_vector);
                    QEci qeci(solar_vector, vel, dat);
                    qtopo = observer.GetLookAngle(qeci);
                    elev = qtopo.elevation * 180.0/PIE;

                    if(reader->nightkindofimage == "VIS_IR")
                    {
                        if(elev < 0.0 )
                            row_col[5568 - 1 - pixelx] = qRgb(rnight, rnight, rnight);
                        else if(elev < 5.0 && elev >= 0.0)
                        {
                            int percentday = (int)(100*elev/5);
                            int percentnight = 100 - percentday;

                            int red = (percentday*rday + percentnight*rnight)/100;
                            red = (red > 255 ? 255 : red);

                            int green = (percentday*gday + percentnight*rnight)/100;
                            green = (green > 255 ? 255 : green);

                            int blue = (percentday*bday + percentnight*rnight)/100;
                            blue = (blue > 255 ? 255 : blue);
                            row_col[5568 - 1 - pixelx] = qRgb(red, green, blue);
                        }
                        else
                            row_col[5568 - 1 - pixelx] = qRgb(rday,gday,bday);
                    }
                    else
                    {
                        row_col[5568 - 1 - pixelx] = qRgb(rday,gday,bday);
                    }
                }
            }
            else if(reader->daykindofimage == "HRV")
            {
                valgamma = pow( c, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                rday = quint8(valcontrast);
                if (rday > 255)
                    rday = 255;

                row_col[5568 - 1 - pixelx] = qRgb(rday,rday,rday);

            }
        }
    }


}

void RSSVideo::ComposeHRV1(quint16 *ptrHRV, quint16 *ptrDayRed, quint16 *ptrDayGreen, quint16 *ptrDayBlue,
                           quint16 *ptrNightRed, quint16 *ptrNightGreen, quint16 *ptrNightBlue, QImage &imhrv, QString date,
                           int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla, int imagenbr)
{
    QRgb *row_col;
    QRgb *row_col_day;
    quint16 cred, cgreen, cblue, c, clum;
    quint16 crednight, cgreennight, cbluenight;
    quint16 rday, gday, bday;
    quint16 rnight, gnight, bnight;

    double gamma = reader->gamma;
    double gammafactor = 1023 / pow(1023, gamma);
    quint16 valgamma;
    quint8 valcontrast;

    long delta = 0;

    pixgeoConversion pixconv;
    double sub_lon = reader->satlon;


    long coff = reader->coffhrv;
    long loff = reader->loffhrv;
    double cfac = reader->cfachrv;
    double lfac = reader->lfachrv;
    double latitude, longitude;
    int ret;

    Vector3 solar_vector;
    Vector3 vel;
    QObserver observer;
    QSgp4Date dat;
    QGeodetic qgeo;
    QTopocentric qtopo;

    double elev;

    int year, month, day, hours, minutes;
    year = date.mid(0, 4).toInt();
    month = date.mid(4, 2).toInt();
    day = date.mid(6, 2).toInt();
    hours = date.mid(8, 2).toInt();
    minutes = date.mid(10, 2).toInt();


    imhrv = QImage(5568, (reader->brss ? 9*464 : 6*464), QImage::Format_ARGB32);
    imhrv.fill(Qt::black);


    this->CLAHE(ptrHRV, 5568, (reader->brss ? 9*464 : 6*464), 0, 1023, 16, 16, 256, 6);


    if(reader->nightkindofimage == "VIS_IR" || reader->nightkindofimage == "VIS_IR Color")
        this->CLAHE(ptrNightRed, 3712, (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464)), 0, 1023, 16, 16, 256, 6);

    if(reader->daykindofimage == "HRV Color")
    {
        this->CLAHE(ptrDayRed, 3712, (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464)), 0, 1023, 16, 16, 256, 6);
        this->CLAHE(ptrDayGreen, 3712, (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464)), 0, 1023, 16, 16, 256, 6);
        this->CLAHE(ptrDayBlue, 3712, (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464)), 0, 1023, 16, 16, 256, 6);
    }

    // test images
//#if 0
    if(reader->daykindofimage == "HRV Color")
    {
        QImage testimage(3712, (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464)), QImage::Format_ARGB32);
        for(int y = (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464))-1; y >= 0; y--)
        {
            row_col = (QRgb*)testimage.scanLine((reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464))-1-y);

            for(int x = 0; x < 3712; x++)
            {
                cred = *(ptrDayRed + y*3712 + x);
                cgreen = *(ptrDayGreen + y*3712 + x);
                cblue = *(ptrDayBlue + y*3712 + x);
                row_col[3712 - x - 1] = qRgb(ContrastStretch(cred), ContrastStretch(cgreen), ContrastStretch(cblue));
            }
        }

        testimage.save("tempimages/ptrDayVIS.png");
    }

    if(reader->nightkindofimage == "VIS_IR")
    {
        QImage testimage(3712, (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464)), QImage::Format_ARGB32);
        for(int y = (reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464))-1; y >= 0; y--)
        {
            row_col = (QRgb*)testimage.scanLine((reader->brss ? 3*464 : (reader->bhrv ? 2*464 : 8*464))-1-y);

            for(int x = 0; x < 3712; x++)
            {
                cred = *(ptrNightRed + y*3712 + x);
                quint16 c = ContrastStretch(cred);
                row_col[3712 - x - 1] = qRgb(c, c, c);
            }
        }

        this->OverlayGeostationary(&testimage, false, leca, lsla, lwca, lnla, ueca, usla, uwca, unla);

        testimage.save("tempimages/ptrNightIR.png");
    }

    if(reader->bhrv)
    {
        QImage testimage(5568, (reader->brss ? 9*464 : 6*464), QImage::Format_ARGB32);
        for(int y = (reader->brss ? 9*464 : 6*464)-1; y >= 0; y--)
        {
            row_col = (QRgb*)testimage.scanLine((reader->brss ? 9*464 : 6*464)-1-y);

            for(int x = 0; x < 5568; x++)
            {
                cred = *(ptrHRV + y*5568 + x);
                row_col[5568 - x - 1] = qRgb(ContrastStretch(cred), ContrastStretch(cred), ContrastStretch(cred));
            }
        }
        this->OverlayGeostationary(&testimage, true, leca, lsla, lwca, lnla, ueca, usla, uwca, unla);

        testimage.save("tempimages/ptrHRV.png");
    }
//#endif


    for (int line = (reader->brss ? 9*464 : 6*464) - 1; line >= 0; line--)
    {
        row_col = (QRgb*)imhrv.scanLine((reader->brss ? 9*464 : 6*464) - 1 - line);

        for (int pixelx = 0; pixelx < 5568; pixelx++)
        {
            c = *(ptrHRV + line * 5568 + pixelx);
            if(reader->brss)
                delta = line/3 * 3712 + leca/3 + pixelx/3;
            else
                delta = line/3 * 3712 + ueca/3 + pixelx/3;
            if(reader->daykindofimage == "HRV Color")
            {
                cred = *(ptrDayRed + delta);
                cgreen = *(ptrDayGreen + delta);
                cblue = *(ptrDayBlue + delta);
                if(reader->nightkindofimage == "VIS_IR")
                    crednight = *(ptrNightRed + delta);
                clum = (cred+cgreen+cblue)/3;
                if( clum == 0)
                    clum = 1;


                valgamma = pow( c*cred/clum, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                rday = quint8(valcontrast);
                if (rday > 255)
                    rday = 255;

                valgamma = pow( c*cgreen/clum, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                gday = quint8(valcontrast);
                if (gday > 255)
                    gday = 255;

                valgamma = pow( c*cblue/clum, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                bday = quint8(valcontrast);
                if (bday > 255)
                    bday = 255;

                if(reader->nightkindofimage == "VIS_IR")
                {
                    crednight = quint16(reader->inverse.at(3) ? 1023 - crednight : crednight);
                    valgamma = pow( crednight, gamma) * gammafactor;
                    if (valgamma >= 1024)
                        valgamma = 1023;

                    valcontrast = ContrastStretch(valgamma);
                    rnight = quint8(valcontrast);
                    if (rnight > 255)
                        rnight = 255;
                }
            }
            else if(reader->daykindofimage == "HRV")
            {
                if(reader->nightkindofimage == "VIS_IR")
                    crednight = *(ptrNightRed + delta);

                valgamma = pow( c, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                rday = quint8(valcontrast);
                if (rday > 255)
                    rday = 255;
                gday = rday;
                bday = rday;

                if(reader->nightkindofimage == "VIS_IR")
                {
                    crednight = quint16(reader->inverse.at(3) ? 1023 - crednight : crednight);
                    valgamma = pow( crednight, gamma) * gammafactor;
                    if (valgamma >= 1024)
                        valgamma = 1023;

                    valcontrast = ContrastStretch(valgamma);
                    rnight = quint8(valcontrast);
                    if (rnight > 255)
                        rnight = 255;
                }
            }

            if(reader->brss)
            {
                ret = pixconv.pixcoord2geocoord(sub_lon, (5568 - 1) - pixelx +  leca, (9*464 - 1) - line, coff, loff, cfac, lfac, &latitude, &longitude);
            }
            else
            {
                ret = 0;
                //if(line < lnla)
                //   ret = pixconv.pixcoord2geocoord(sub_lon, (5568 - 1) - pixelx + leca, (6*464 - 1) - line, coff, loff, cfac, lfac, &latitude, &longitude);
                //else
                   ret = pixconv.pixcoord2geocoord(sub_lon, 5567 - pixelx + uwca, (6*464 - 1) - line, coff, loff, cfac, lfac, &latitude, &longitude);


            }

            if(ret == -1)
                row_col[5568 - 1 - pixelx] = qRgb(255, 0, 0);
                //continue;
            else
            {
                observer.SetLocation(latitude, longitude, 0.0);
                dat.Set(year, month, day, hours, minutes, 0, true);
                QSun::Calculate_Solar_Position(dat.Julian(), &solar_vector);
                QEci qeci(solar_vector, vel, dat);
                qtopo = observer.GetLookAngle(qeci);
                elev = qtopo.elevation * 180.0/PIE;

                if(reader->nightkindofimage == "VIS_IR")
                {
                    if(elev < 0.0 )
                        row_col[5568 - 1 - pixelx] = qRgb(rnight, rnight, rnight);
                    else if(elev < 5.0 && elev >= 0.0)
                    {
                        int percentday = (int)(100*elev/5);
                        int percentnight = 100 - percentday;

                        int red = (percentday*rday + percentnight*rnight)/100;
                        red = (red > 255 ? 255 : red);

                        int green = (percentday*gday + percentnight*rnight)/100;
                        green = (green > 255 ? 255 : green);

                        int blue = (percentday*bday + percentnight*rnight)/100;
                        blue = (blue > 255 ? 255 : blue);
                        row_col[5568 - 1 - pixelx] = qRgb(red, green, blue);
                    }
                    else
                        row_col[5568 - 1 - pixelx] = qRgb(rday,gday,bday);
                }
                else
                {
                    row_col[5568 - 1 - pixelx] = qRgb(rday,gday,bday);
                }
            }
        }
    }

}

void RSSVideo::ComposeHRVFull(quint16 *ptrHRV, quint16 *ptrDayRed, quint16 *ptrDayGreen, quint16 *ptrDayBlue,
                              quint16 *ptrNightRed, quint16 *ptrNightGreen, quint16 *ptrNightBlue, QImage &imhrvfull, QString date,
                              int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla)
{
    QRgb *row_col;
    QRgb *row_col_day;
    quint16 cred, cgreen, cblue, c, clum;
    quint16 crednight, cgreennight, cbluenight;
    quint16 r,g, b;
    quint16 rnight, gnight, bnight;

    double gamma = reader->gamma;
    double gammafactor = 1023 / pow(1023, gamma);
    quint16 valgamma;
    quint8 valcontrast;

    long delta = 0;

    pixgeoConversion pixconv;
    double sub_lon = reader->satlon;


    long coff = reader->coffhrv;
    long loff = reader->loffhrv;
    double cfac = reader->cfachrv;
    double lfac = reader->lfachrv;
    double latitude, longitude;
    int ret;

    Vector3 solar_vector;
    Vector3 vel;
    QObserver observer;
    QSgp4Date dat;
    QGeodetic qgeo;
    QTopocentric qtopo;

    double elev;

    int year, month, day, hours, minutes;
    year = date.mid(0, 4).toInt();
    month = date.mid(4, 2).toInt();
    day = date.mid(6, 2).toInt();
    hours = date.mid(8, 2).toInt();
    minutes = date.mid(10, 2).toInt();



    imhrvfull = QImage(11136, 11136, QImage::Format_ARGB32);
    imhrvfull.fill(Qt::black);

    this->CLAHE(ptrHRV, 5568, 9*464, 0, 1023, 16, 16, 256, 6);


    if(reader->nightkindofimage == "VIS_IR" || reader->nightkindofimage == "VIS_IR Color")
        this->CLAHE(ptrNightRed, 3712, (reader->brss ? 1392 : 3712), 0, 1023, 16, 16, 256, 4);

    if(reader->daykindofimage == "HRV Color")
    {
        this->CLAHE(ptrDayRed, 3712, (reader->brss ? 1392 : 3712), 0, 1023, 16, 16, 256, 6);
        this->CLAHE(ptrDayGreen, 3712, (reader->brss ? 1392 : 3712), 0, 1023, 16, 16, 256, 6);
        this->CLAHE(ptrDayBlue, 3712, (reader->brss ? 1392 : 3712), 0, 1023, 16, 16, 256, 6);
    }

    // test images
    //    if(reader->daykindofimage == "HRV Color")
    //    {
    //        QImage testimage(3712, 3*464, QImage::Format_ARGB32);
    //        for(int y = 3*464-1; y >= 0; y--)
    //        {
    //            row_col = (QRgb*)testimage.scanLine(3*464-1-y);

    //            for(int x = 0; x < 3712; x++)
    //            {
    //                cred = *(ptrDayRed + y*3712 + x);
    //                cgreen = *(ptrDayGreen + y*3712 + x);
    //                cblue = *(ptrDayBlue + y*3712 + x);
    //                row_col[3712 - x - 1] = qRgb(ContrastStretch(cred), ContrastStretch(cgreen), ContrastStretch(cblue));
    //            }
    //        }

    //        testimage.save("ptrDayVIS.png");
    //    }


    //    {
    //        QImage testimage(5568, 9*464, QImage::Format_ARGB32);
    //        for(int y = 9*464-1; y >= 0; y--)
    //        {
    //            row_col = (QRgb*)testimage.scanLine(9*464-1-y);

    //            for(int x = 0; x < 5568; x++)
    //            {
    //                cred = *(ptrHRV + y*5568 + x);
    //                row_col[5568 - x - 1] = qRgb(ContrastStretch(cred), ContrastStretch(cred), ContrastStretch(cred));
    //            }
    //        }

    //        testimage.save("ptrHRV.png");
    //    }


    for (int line = 11136 - 1; line >= 0; line--)
    {
        row_col = (QRgb*)imhrvfull.scanLine(11136 - 1 - line);

        for (int pixelx = 0; pixelx < 5568; pixelx++)
        {
            c = *(ptrHRV + line * 5568 + pixelx);
            if(line <= lnla)
                delta = line/3 * 3712 + leca/3 + pixelx/3;
            else
                delta = line/3 * 3712 + ueca/3 + pixelx/3;
            if(reader->daykindofimage == "HRV Color")
            {
                cred = *(ptrDayRed + delta);
                cgreen = *(ptrDayGreen + delta);
                cblue = *(ptrDayBlue + delta);
                if(reader->nightkindofimage == "VIS_IR")
                    crednight = *(ptrNightRed + delta);
                clum = (cred+cgreen+cblue)/3;
                if( clum == 0)
                    clum = 1;


                valgamma = pow( c*cred/clum, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                r = quint8(valcontrast);
                if (r > 255)
                    r = 255;

                valgamma = pow( c*cgreen/clum, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                g = quint8(valcontrast);
                if (g > 255)
                    g = 255;

                valgamma = pow( c*cblue/clum, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                b = quint8(valcontrast);
                if (b > 255)
                    b = 255;

                if(reader->nightkindofimage == "VIS_IR")
                {
                    valgamma = pow( crednight, gamma) * gammafactor;
                    if (valgamma >= 1024)
                        valgamma = 1023;

                    valcontrast = ContrastStretch(valgamma);
                    rnight = quint8(valcontrast);
                    if (rnight > 255)
                        rnight = 255;
                }

                //////////////////
                //                r = ContrastStretch(cred);
                //                g = ContrastStretch(cgreen);
                //                b = ContrastStretch(cblue);
                ///////////////////////////
                ret = pixconv.pixcoord2geocoord(sub_lon, (5568 - 1) - pixelx +  leca, (9*464 - 1) - line, coff, loff, cfac, lfac, &latitude, &longitude);
                if(ret == -1)
                    row_col[5568 - 1 - pixelx] = qRgb(0, 0, 0);
                else
                {
                    observer.SetLocation(latitude, longitude, 0.0);
                    dat.Set(year, month, day, hours, minutes, 0, true);
                    QSun::Calculate_Solar_Position(dat.Julian(), &solar_vector);
                    QEci qeci(solar_vector, vel, dat);
                    qtopo = observer.GetLookAngle(qeci);
                    elev = qtopo.elevation * 180.0/PIE;

                    if(reader->nightkindofimage == "VIS_IR")
                    {
                        if(elev < 0.0 )
                            row_col[5568 - 1 - pixelx] = qRgb(rnight, rnight, rnight);
                        else if(elev < 5.0 && elev >= 0.0)
                        {
                            int percentday = (int)(100*elev/5);
                            int percentnight = 100 - percentday;

                            int red = (percentday*r + percentnight*rnight)/100;
                            red = (red > 255 ? 255 : red);

                            int green = (percentday*g + percentnight*rnight)/100;
                            green = (green > 255 ? 255 : green);

                            int blue = (percentday*b + percentnight*rnight)/100;
                            blue = (blue > 255 ? 255 : blue);
                            row_col[5568 - 1 - pixelx] = qRgb(red, green, blue);
                        }
                        else
                            row_col[5568 - 1 - pixelx] = qRgb(r,g,b);
                    }
                    else
                    {
                        row_col[5568 - 1 - pixelx] = qRgb(r,g,b);
                    }
                }
            }
            else if(reader->daykindofimage == "HRV")
            {
                valgamma = pow( c, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                r = quint8(valcontrast);
                if (r > 255)
                    r = 255;

                row_col[5568 - 1 - pixelx] = qRgb(r,r,r);

            }
        }
    }


}


void RSSVideo::getSegmentSamples(QString filepath, quint16 *ptr, int filesequence, QString typelist)
{
    MSG_header *header;
    MSG_data *msgdat;

    header = new MSG_header();
    msgdat = new MSG_data();


    qDebug() << "getSegmentSamples " << filepath << " seq " << filesequence << " type " << typelist;
    std::ifstream hrit(filepath.toStdString(), (std::ios::binary | std::ios::in) );
    if (hrit.fail())
    {
        std::cerr << "Cannot open input hrit file " << filepath.toStdString() << std::endl;
        delete  header;
        delete msgdat;
        return;
    }

    header->read_from(hrit);
    msgdat->read_from(hrit, *header);
    hrit.close();

    //cout << *header;

    if (header->segment_id->data_field_format == MSG_NO_FORMAT)
    {
        qDebug() << "Product dumped in binary format.";
        delete  header;
        delete msgdat;
        return;
    }

    int planned_end_segment = header->segment_id->planned_end_segment_sequence_number;

    int npix = header->image_structure->number_of_columns;
    int nlin = header->image_structure->number_of_lines;

    qDebug() << "getSegmentSamples npix = " << npix << " nlin = " << nlin << "planned end = " << planned_end_segment;

    size_t npixperseg = npix * nlin;


    MSG_SAMPLE *pixels = new MSG_SAMPLE[npixperseg];
    memset(pixels, 0, npixperseg*sizeof(MSG_SAMPLE));
    memcpy(pixels, msgdat->image->data, npixperseg*sizeof(MSG_SAMPLE));

    quint16 c;

    for(int line = 0; line < nlin; line++)
    {
        for (int pixelx = 0 ; pixelx < npix; pixelx++)
        {
            c = *(pixels + line * npix + pixelx);
            if(reader->brss )
                *(ptr + (filesequence - (typelist == "HRVList" ? 15 : 5)) * npix * nlin + line * npix + pixelx) = c;
            else
                *(ptr + (filesequence - (typelist == "HRVList" ? 18 : (reader->bhrv ? 6 : 0))) * npix * nlin + line * npix + pixelx) = c;

        }
    }

    delete header;
    delete msgdat;
    delete [ ] pixels;

}


void RSSVideo::ComposeVISIR(quint16 *ptrDayRed, quint16 *ptrDayGreen, quint16 *ptrDayBlue, quint16 *ptrNightRed, quint16 *ptrNightGreen,
                            quint16 *ptrNightBlue, QImage &imvisir, QString date, int imagenbr)
{

    QRgb *row_col_day;
    QRgb *row_col_night;
    QRgb *row_col;
    quint16 cred, cgreen, cblue;
    quint16 rday, gday, bday;
    quint16 rnight, gnight, bnight;
    quint16 indexoutrc, indexoutgc, indexoutbc;
    int dayhistogrammethod = CMB_HISTO_CLAHE;
    int nighthistogrammethod = CMB_HISTO_CLAHE;

    quint16 stat_min_day[3];
    quint16 stat_max_day[3];
    quint16 stat_min_night[3];
    quint16 stat_max_night[3];
    long active_pixels_day[3];
    long active_pixels_night[3];
    quint16 lut_ch_day[3][1024];
    quint16 lut_ch_night[3][1024];

    QImage imageday;
    QImage imagenight;

    imvisir = QImage(3712, reader->brss ? 1392 : 3712, QImage::Format_ARGB32);
    imvisir.fill(Qt::black);

    if(reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color")
    {
        imageday = QImage(3712, reader->brss ? 1392 : 3712, QImage::Format_ARGB32);
        imageday.fill(Qt::black);
    }

    if(reader->nightkindofimage == "VIS_IR" || reader->nightkindofimage == "VIS_IR Color")
    {
        imagenight = QImage(3712, reader->brss ? 1392 : 3712, QImage::Format_ARGB32);
        imagenight.fill(Qt::black);
    }

    int width = 3712;
    int height = reader->brss ? 1392 : 3712;

    int minRadianceIndexDay[3];
    int maxRadianceIndexDay[3];
    int minRadianceIndexNight[3];
    int maxRadianceIndexNight[3];

    for (int i=0; i < 3; i++)
    {
        stat_min_day[i] = 0;
        stat_max_day[i] = 0;
        stat_min_night[i] = 0;
        stat_max_night[i] = 0;
        active_pixels_day[i] = 0;
        active_pixels_night[i] = 0;
        for (int j=0; j < 1024; j++)
        {
            lut_ch_day[i][j] = 0;
            lut_ch_night[i][j] = 0;
        }
    }

    if(reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color")
        dayhistogrammethod = CMB_HISTO_CLAHE; //CMB_HISTO_NONE_95; //CMB_HISTO_EQUALIZE; //

    if(dayhistogrammethod == CMB_HISTO_NONE_95 || dayhistogrammethod == CMB_HISTO_EQUALIZE )
    {
        if(reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color" || reader->daykindofimage == "HRV Color")
        {
            CalculateMinMax(0, width, height, ptrDayRed, 0, stat_min_day, stat_max_day, active_pixels_day);
            CalculateLUTGeo(0, width, height, ptrDayRed, 0, stat_min_day, stat_max_day, active_pixels_day, lut_ch_day, minRadianceIndexDay, maxRadianceIndexDay);
        }
        if(reader->daykindofimage == "VIS_IR Color" || reader->daykindofimage == "HRV Color")
        {
            CalculateMinMax(1, width, height, ptrDayGreen, 0, stat_min_day, stat_max_day, active_pixels_day);
            CalculateLUTGeo(1, width, height, ptrDayGreen, 0, stat_min_day, stat_max_day, active_pixels_day, lut_ch_day, minRadianceIndexDay, maxRadianceIndexDay);
            CalculateMinMax(2, width, height, ptrDayBlue, 0, stat_min_day, stat_max_day, active_pixels_day);
            CalculateLUTGeo(2, width, height, ptrDayBlue, 0, stat_min_day, stat_max_day, active_pixels_day, lut_ch_day, minRadianceIndexDay, maxRadianceIndexDay);
        }
    }

    if(dayhistogrammethod == CMB_HISTO_CLAHE )
    {
        if(reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color" || reader->daykindofimage == "HRV Color")
        {
            this->CLAHE(ptrDayRed, 3712, reader->brss ? 3*464 : 8*464, 0, 1023, 16, 16, 256, 3.7);
        }
        if(reader->daykindofimage == "VIS_IR Color" || reader->daykindofimage == "HRV Color")
        {
            this->CLAHE(ptrDayGreen, 3712, reader->brss ? 3*464 : 8*464, 0, 1023, 16, 16, 256, 3.7);
            this->CLAHE(ptrDayBlue, 3712, reader->brss ? 3*464 : 8*464, 0, 1023, 16, 16, 256, 3.7);
        }
    }


    if(reader->nightkindofimage == "VIS_IR" || reader->nightkindofimage == "VIS_IR Color")
        nighthistogrammethod = CMB_HISTO_CLAHE; //CMB_HISTO_NONE_95;

    if(nighthistogrammethod == CMB_HISTO_NONE_95 || nighthistogrammethod == CMB_HISTO_EQUALIZE )
    {
        if(reader->nightkindofimage == "VIS_IR" || reader->nightkindofimage == "VIS_IR Color")
        {
            CalculateMinMax(0, width, height, ptrNightRed, 0, stat_min_night, stat_max_night, active_pixels_night);
            CalculateLUTGeo(0, width, height, ptrNightRed, 0, stat_min_night, stat_max_night, active_pixels_night, lut_ch_night, minRadianceIndexNight, maxRadianceIndexNight);
        }
        if(reader->nightkindofimage == "VIS_IR Color")
        {
            CalculateMinMax(1, width, height, ptrNightGreen, 0, stat_min_night, stat_max_night, active_pixels_night);
            CalculateLUTGeo(1, width, height, ptrNightGreen, 0, stat_min_night, stat_max_night, active_pixels_night, lut_ch_night, minRadianceIndexNight, maxRadianceIndexNight);
            CalculateMinMax(2, width, height, ptrNightBlue, 0, stat_min_night, stat_max_night, active_pixels_night);
            CalculateLUTGeo(2, width, height, ptrNightBlue, 0, stat_min_night, stat_max_night, active_pixels_night, lut_ch_night, minRadianceIndexNight, maxRadianceIndexNight);
        }
    }

    if(nighthistogrammethod == CMB_HISTO_CLAHE )
    {
        if(reader->nightkindofimage == "VIS_IR" || reader->nightkindofimage == "VIS_IR Color")
        {
            this->CLAHE(ptrNightRed, 3712, reader->brss ? 3*464 : 8*464, 0, 1023, 16, 16, 256, 8.7);
        }
        if(reader->nightkindofimage == "VIS_IR Color")
        {
            this->CLAHE(ptrNightGreen, 3712, reader->brss ? 3*464 : 8*464, 0, 1023, 16, 16, 256, 8.7);
            this->CLAHE(ptrNightBlue, 3712, reader->brss ? 3*464 : 8*464, 0, 1023, 16, 16, 256, 8.7);
        }
    }


    pixgeoConversion pixconv;

    double sub_lon = reader->satlon;

    long coff = reader->coff;
    long loff = reader->loff;
    double cfac = reader->cfac;
    double lfac = reader->lfac;

    double latitude, longitude;
    int ret;

    for (int line = reader->brss ? 3*464 - 1 : 3712 - 1; line >= 0; line--)
    {
        if(reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color")
            row_col_day = (QRgb*)imageday.scanLine((reader->brss ? 1392 - 1 : 3712 - 1) - line);
        if(reader->nightkindofimage == "VIS_IR" || reader->nightkindofimage == "VIS_IR Color")
            row_col_night = (QRgb*)imagenight.scanLine((reader->brss ? 1392 - 1 : 3712 - 1) - line);

        for (int pixelx = 3712 - 1 ; pixelx >= 0; pixelx--)
        {
            if(reader->daykindofimage == "VIS_IR Color" || reader->daykindofimage == "HRV Color")
            {
                cred = *(ptrDayRed + line * 3712 + pixelx);
                cgreen = *(ptrDayGreen + line * 3712 + pixelx);
                cblue = *(ptrDayBlue + line * 3712 + pixelx);

                if(dayhistogrammethod == CMB_HISTO_NONE_95)
                {
                    if(cred != 65535)
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cred - minRadianceIndexDay[0] ) / (float)(maxRadianceIndexDay[0] - minRadianceIndexDay[0])), 0), 1023);
                    if(cgreen != 65535)
                        indexoutgc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cgreen - minRadianceIndexDay[1] ) / (float)(maxRadianceIndexDay[1] - minRadianceIndexDay[1])), 0), 1023);
                    if(cblue != 65535)
                        indexoutbc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cblue - minRadianceIndexDay[2] ) / (float)(maxRadianceIndexDay[2] - minRadianceIndexDay[2])), 0), 1023);
                }
                else if(dayhistogrammethod == CMB_HISTO_NONE_100 || dayhistogrammethod == CMB_HISTO_CLAHE)
                {
                    if(cred != 65535)
                        indexoutrc = cred;
                    if(cgreen != 65535)
                        indexoutgc = cgreen;
                    if(cblue != 65535)
                        indexoutbc = cblue;
                }
                else if(dayhistogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( cred != 65535) indexoutrc = (quint16)qMin(qMax(qRound((float)lut_ch_day[0][cred]), 0), 1023);
                    if( cgreen != 65535) indexoutgc = (quint16)qMin(qMax(qRound((float)lut_ch_day[1][cgreen]), 0), 1023);
                    if( cblue != 65535) indexoutbc = (quint16)qMin(qMax(qRound((float)lut_ch_day[2][cblue]), 0), 1023);
                }

                //                if( (cred == 65535) || (cgreen == 65535) || (cblue == 65535))
                if( (cred == 0) || (cgreen == 0) || (cblue == 0))
                {
                    //row_col_day = (QRgb*)imvisir.scanLine((reader->brss ? 1392 - 1 : 3712 - 1) - line);

                    rday = 0;
                    gday = 0;
                    bday = 0;
                }
                else
                {
                    rday = quint16(reader->inverse.at(0) ? (1023 - indexoutrc)/4 : indexoutrc/4);
                    gday = quint16(reader->inverse.at(1) ? (1023 - indexoutgc)/4 : indexoutgc/4);
                    bday = quint16(reader->inverse.at(2) ? (1023 - indexoutbc)/4 : indexoutbc/4);
                }
            }
            else if(reader->daykindofimage == "VIS_IR")
            {
                cred = *(ptrDayRed + line * 3712 + pixelx);

                if(dayhistogrammethod == CMB_HISTO_NONE_95)
                {
                    if(cred != 65535)
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cred - minRadianceIndexDay[0] ) / (float)(maxRadianceIndexDay[0] - minRadianceIndexDay[0])), 0), 1023);
                }
                else if(dayhistogrammethod == CMB_HISTO_NONE_100 || dayhistogrammethod == CMB_HISTO_CLAHE)
                {
                    if(cred != 65535)
                        indexoutrc = cred;
                }
                else if(dayhistogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( cred != 65535) indexoutrc = (quint16)qMin(qMax(qRound((float)lut_ch_day[0][cred]), 0), 1023);
                }

                //                if( cred == 65535)
                if( cred == 0)
                {
                    rday = 0;
                    gday = 0;
                    bday = 0;
                }
                else
                {
                    rday = quint16(reader->inverse.at(0) ? (1023 - indexoutrc)/4 : indexoutrc/4);
                    gday = rday;
                    bday = rday;
                }
            }


            if(reader->nightkindofimage == "VIS_IR Color")
            {
                cred = *(ptrNightRed + line * 3712 + pixelx);
                cgreen = *(ptrNightGreen + line * 3712 + pixelx);
                cblue = *(ptrNightBlue + line * 3712 + pixelx);

                if(nighthistogrammethod == CMB_HISTO_NONE_95)
                {
                    if(cred != 65535)
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cred - minRadianceIndexNight[0] ) / (float)(maxRadianceIndexNight[0] - minRadianceIndexNight[0])), 0), 1023);
                    if(cgreen != 65535)
                        indexoutgc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cgreen - minRadianceIndexNight[1] ) / (float)(maxRadianceIndexNight[1] - minRadianceIndexNight[1])), 0), 1023);
                    if(cblue != 65535)
                        indexoutbc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cblue - minRadianceIndexNight[2] ) / (float)(maxRadianceIndexNight[2] - minRadianceIndexNight[2])), 0), 1023);
                }
                else if(nighthistogrammethod == CMB_HISTO_NONE_100 || nighthistogrammethod == CMB_HISTO_CLAHE)
                {
                    if(cred != 65535)
                        indexoutrc = cred;
                    if(cgreen != 65535)
                        indexoutgc = cgreen;
                    if(cblue != 65535)
                        indexoutbc = cblue;
                }
                else if(nighthistogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( cred != 65535) indexoutrc = (quint16)qMin(qMax(qRound((float)lut_ch_night[0][cred]), 0), 1023);
                    if( cgreen != 65535) indexoutgc = (quint16)qMin(qMax(qRound((float)lut_ch_night[1][cgreen]), 0), 1023);
                    if( cblue != 65535) indexoutbc = (quint16)qMin(qMax(qRound((float)lut_ch_night[2][cblue]), 0), 1023);
                }

                if( (cred == 65535) || (cgreen == 65535) || (cblue == 65535))
                {
                    rnight = 0;
                    gnight = 0;
                    bnight = 0;
                }
                else
                {
                    rnight = quint16(reader->inverse.at(3) ? (1023 - indexoutrc)/4 : indexoutrc/4);
                    gnight = quint16(reader->inverse.at(4) ? (1023 - indexoutgc)/4 : indexoutgc/4);
                    bnight = quint16(reader->inverse.at(5) ? (1023 - indexoutbc)/4 : indexoutbc/4);
                }
            }
            else if(reader->nightkindofimage == "VIS_IR")
            {
                cred = *(ptrNightRed + line * 3712 + pixelx);

                if(nighthistogrammethod == CMB_HISTO_NONE_95)
                {
                    if(cred != 65535)
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cred - minRadianceIndexNight[0] ) / (float)(maxRadianceIndexNight[0] - minRadianceIndexNight[0])), 0), 1023);
                }
                else if(nighthistogrammethod == CMB_HISTO_NONE_100 || nighthistogrammethod == CMB_HISTO_CLAHE)
                {
                    if(cred != 65535)
                        indexoutrc = cred;
                }
                else if(nighthistogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( cred != 65535) indexoutrc = (quint16)qMin(qMax(qRound((float)lut_ch_night[0][cred]), 0), 1023);
                }

                if( cred == 65535)
                {
                    rnight = 0;
                    gnight = 0;
                    bnight = 0;
                }
                else
                {
                    rnight = quint16(reader->inverse.at(3) ? (1023 - indexoutrc)/4 : indexoutrc/4);
                    gnight = rnight;
                    bnight = rnight;
                }
            }
            if(reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color")
                row_col_day[3712 - 1 - pixelx] = qRgb(rday, gday, bday);
            if(reader->nightkindofimage == "VIS_IR" || reader->nightkindofimage == "VIS_IR Color")
                row_col_night[3712 - 1 - pixelx] = qRgb(rnight, gnight, bnight);
        }
    }

    //imagenight.save(QString("imagenight.png"));

    Vector3 solar_vector;
    Vector3 vel;
    QObserver observer;
    QSgp4Date dat;
    QGeodetic qgeo;
    QTopocentric qtopo;

    double elev;
    double twilight = 12.0;

    int year, month, day, hours, minutes;
    year = date.mid(0, 4).toInt();
    month = date.mid(4, 2).toInt();
    day = date.mid(6, 2).toInt();
    hours = date.mid(8, 2).toInt();
    minutes = date.mid(10, 2).toInt();

    if((reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color") && (reader->nightkindofimage == "VIS_IR" || reader->nightkindofimage == "VIS_IR Color"))
    {
        for (int line = 0; line < (reader->brss ? 1392 : 3712); line++)
        {
            row_col = (QRgb*)imvisir.scanLine(line);
            if(reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color")
                row_col_day = (QRgb*)imageday.scanLine(line);
            if(reader->nightkindofimage == "VIS_IR" || reader->nightkindofimage == "VIS_IR Color")
                row_col_night = (QRgb*)imagenight.scanLine(line);

            for (int pixelx = 0 ; pixelx < 3712; pixelx++)
            {
                ret = pixconv.pixcoord2geocoord(sub_lon, pixelx, line, coff, loff, cfac, lfac, &latitude, &longitude);
                if(ret == -1)
                    row_col[pixelx] = qRgb(0, 0, 0);
                else
                {
                    observer.SetLocation(latitude, longitude, 0.0);
                    dat.Set(year, month, day, hours, minutes, 0, true);
                    QSun::Calculate_Solar_Position(dat.Julian(), &solar_vector);
                    QEci qeci(solar_vector, vel, dat);
                    qtopo = observer.GetLookAngle(qeci);
                    elev = qtopo.elevation * 180.0/PIE;

                    if(elev <= 0.0)
                    {
                        if(reader->brss)
                        {
                            if(line > 0 && line < 95 )
                                row_col[pixelx] = qRgb(0, 0, 0);
                            else if( (reader->brss ? 1392 : 3712) - 100 < line)
                                row_col[pixelx] = qRgb(0, 0, 0);
                            else
                                row_col[pixelx] = row_col_night[pixelx];
                        }
                        else
                        {
                            row_col[pixelx] = row_col_night[pixelx];
                        }
                    }
                    else if(elev < twilight && elev > 0.0)
                    {
                        int percentday = (int)(100.0 * elev / twilight);
                        int percentnight = 100 - percentday;
                        int redday = qRed(row_col_day[pixelx]);
                        int rednight = qRed(row_col_night[pixelx]);
                        int red = (percentday*redday + percentnight*rednight)/100;
                        red = (red > 255 ? 255 : red);

                        int greenday = qGreen(row_col_day[pixelx]);
                        int greennight = qGreen(row_col_night[pixelx]);
                        int green = (percentday*greenday + percentnight*greennight)/100;
                        green = (green > 255 ? 255 : green);

                        int blueday = qBlue(row_col_day[pixelx]);
                        int bluenight = qBlue(row_col_night[pixelx]);
                        int blue = (percentday*blueday + percentnight*bluenight)/100;
                        blue = (blue > 255 ? 255 : blue);
                        if(reader->brss)
                        {
                            if(line > 0 && line < 95)
                                row_col[pixelx] = qRgb(0, 0, 0);
                            else if( (reader->brss ? 1392 : 3712) - 100 < line)
                                row_col[pixelx] = qRgb(0, 0, 0);
                            else
                                row_col[pixelx] = qRgb(red, green, blue);
                        }
                        else
                        {
                            row_col[pixelx] = qRgb(red, green, blue);
                        }
                    }
                    else
                    {
                        if(reader->brss)
                        {
                            if(line > 0 && line < 95)
                                row_col[pixelx] = qRgb(0, 0, 0);
                            else if( (reader->brss ? 1392 : 3712) - 100 < line)
                                row_col[pixelx] = qRgb(0, 0, 0);
                            else
                                row_col[pixelx] = row_col_day[pixelx];
                        }
                        else
                        {
                            row_col[pixelx] = row_col_day[pixelx];
                        }
                    }

                    //                                                if(elev >= 0.0 && elev < 0.1)
                    //                                                {
                    //                                                    row_col[pixelx] = qRgb(255, 0, 0);
                    //                                                }
                    //                                                if(elev >= twilight && elev < twilight + 0.1)
                    //                                                {
                    //                                                    row_col[pixelx] = qRgb(0, 255, 0);
                    //                                                }

                }
            }

        }
    }
    else if((reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color") && reader->nightkindofimage == "")
    {
        for (int line = 0; line < (reader->brss ? 1392 : 3712); line++)
        {
            row_col = (QRgb*)imvisir.scanLine(line);
            row_col_day = (QRgb*)imageday.scanLine(line);

            for (int pixelx = 0 ; pixelx < 3712; pixelx++)
            {
                ret = pixconv.pixcoord2geocoord(sub_lon, pixelx, line, coff, loff, cfac, lfac, &latitude, &longitude);
                if(ret == -1)
                    row_col[pixelx] = qRgb(0, 0, 0);
                else
                {
                    if(reader->brss)
                    {
                        if(line > 0 && line < 95)
                            row_col[pixelx] = qRgb(0, 0, 0);
                        else if( (reader->brss ? 1392 : 3712) - 100 < line)
                            row_col[pixelx] = qRgb(0, 0, 0);
                        else
                            row_col[pixelx] = row_col_day[pixelx];
                    }
                    else
                    {
                        row_col[pixelx] = row_col_day[pixelx];
                    }
                }
            }
        }
    }
    else if(reader->daykindofimage == "" && (reader->nightkindofimage == "VIS_IR" || reader->nightkindofimage == "VIS_IR Color"))
    {
        for (int line = 0; line < (reader->brss ? 1392 : 3712); line++)
        {
            row_col = (QRgb*)imvisir.scanLine(line);
            row_col_night = (QRgb*)imagenight.scanLine(line);

            for (int pixelx = 0 ; pixelx < 3712; pixelx++)
            {
                ret = pixconv.pixcoord2geocoord(sub_lon, pixelx, line, coff, loff, cfac, lfac, &latitude, &longitude);
                if(ret == -1)
                    row_col[pixelx] = qRgb(0, 0, 0);
                else
                {
                    if(reader->brss)
                    {
                        if(line > 0 && line < 95)
                            row_col[pixelx] = qRgb(0, 0, 0);
                        else if( (reader->brss ? 1392 : 3712) - 100 < line)
                            row_col[pixelx] = qRgb(0, 0, 0);
                        else
                            row_col[pixelx] = row_col_night[pixelx];
                    }
                    else
                    {
                        row_col[pixelx] = row_col_night[pixelx];
                    }
                }
            }
        }

    }

}

void RSSVideo::CalculateMinMax(int colorindex, int width, int height, quint16 *ptr, quint16 fillvalue, quint16 stat_min[], quint16 stat_max[], long active_pixels[])
{
    long cnt = 0;
    stat_min[colorindex] = 65535;
    stat_max[colorindex] = 0;

    active_pixels[colorindex] = 0;

    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            quint16 val = ptr[j * width + i];
            if(val != fillvalue)
            {
                if(val >= stat_max[colorindex])
                    stat_max[colorindex] = val;
                if(val < stat_min[colorindex])
                    stat_min[colorindex] = val;
                active_pixels[colorindex]++;
            }
            else
                cnt++;
        }
    }

    qDebug() << QString("CalculateMinMax color = %1 stat_min = %2 stat_max = %3 active pixels = %4 pixels with fillvalue = %5")
                .arg(colorindex).arg(stat_min[colorindex]).arg(stat_max[colorindex]).arg(active_pixels[colorindex]).arg(cnt);

}

void RSSVideo::CalculateLUTGeo(int colorindex, int width, int height, quint16 *ptr, quint16 fillvalue, quint16 stat_min[], quint16 stat_max[],
                               long active_pixels[], quint16 lut_ch[3][1024], int minRadianceIndex[], int maxRadianceIndex[])
{
    long stats_ch[3][1024];

    for(int k = 0; k < 3; k++)
    {
        for (int j = 0; j < 1024; j++)
        {
            stats_ch[k][j] = 0;
        }
    }

    quint16 pixel;
    for (int line = 0; line < height; line++)
    {
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            pixel = ptr[line * width + pixelx];
            if(pixel != fillvalue)
            {
                quint16 indexout = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel - stat_min[colorindex])/(float)(stat_max[colorindex] - stat_min[colorindex])), 0), 1023);
                stats_ch[colorindex][indexout]++;
            }
        }
    }




    // float scale = 256.0 / (NbrOfSegmentLinesSelected() * earth_views);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
    double newscale = (double)(1024.0 / active_pixels[colorindex]);

    //qDebug() << QString("newscale = %1 active pixels = %2").arg(newscale).arg(active_pixels[colorindex]);

    unsigned long long sum_ch[3];

    for (int i=0; i < 3; i++)
    {
        sum_ch[i] = 0;
    }


    bool okmin, okmax;

    okmin = false;
    okmax = false;

    // min/maxRadianceIndex = index of 95% ( 2.5% of 1024 = 25, 97.5% of 1024 = 997 )
    for( int i = 0; i < 1024; i++)
    {
        sum_ch[colorindex] += stats_ch[colorindex][i];
        lut_ch[colorindex][i] = (quint16)((double)sum_ch[colorindex] * newscale);
        lut_ch[colorindex][i] = ( lut_ch[colorindex][i] > 1023 ? 1023 : lut_ch[colorindex][i]);
        //        qDebug() << QString("stats_ch[0][%1] = %2 lut_ch[0][%3] = %4").arg(i).arg(stats_ch[0][i]).arg(i).arg(imageptrs->lut_ch[0][i]);
        if(lut_ch[colorindex][i] > 25 && okmin == false)
        {
            okmin = true;
            minRadianceIndex[colorindex] = i;
        }
        if(lut_ch[colorindex][i] > 997 && okmax == false)
        {
            okmax = true;
            maxRadianceIndex[colorindex] = i;
        }
    }

    //    for(int i = 0; i < 1024; i++)
    //    {
    //        qDebug() << QString("stats_ch[0][%1] = %2").arg(i).arg(stats_ch[0][i]);
    //    }


    //        for(int i = 0; i < 1024; i++)
    //        {
    //            qDebug() << QString("stats_ch[0][%1] = %2 sum_ch[0][%3] = %4").arg(i).arg(stats_ch[0][i]).arg(i).arg(sum_ch[0][i]);
    //        }


    //    qDebug() << QString("minRadianceIndex [%1] = %2 maxRadianceIndex [%3] = %4").arg(colorindex).arg(minRadianceIndex[colorindex]).arg(colorindex).arg(maxRadianceIndex[colorindex]);
}

void RSSVideo::getFilenameParameters(QString filename, QString &filespectrum, QString &filedate, int &filesequence)
{

    int index = 26;
    int length = 3;
    QString spectrum = filename.mid(index, length);
    if(spectrum.length() > 0 && spectrum == "HRV")
    {
        filespectrum = spectrum;
        filedate = filename.mid(46, 12);
        filesequence = filename.mid(36, 6).toInt()-1;
    }
    else
    {
        spectrum = filename.mid(26, 6);
        filespectrum = spectrum;
        filedate = filename.mid(46, 12);
        filesequence = filename.mid(36, 6).toInt()-1;
    }

}

QStringList RSSVideo::getGeostationarySegments(const QString imagetype, QString path, QVector<QString> spectrumvector, QString filepattern)
{
    //    qDebug() << QString("getGeostationarySegments type = %1  Filepath = %2 filepattern = %3").arg(imagetype).arg(path).arg(filepattern);
    //    qDebug() << QString("getGeostationarySegments spectrumvector %1 %2 %3").arg(spectrumvector.at(0)).arg(spectrumvector.at(1)).arg(spectrumvector.at(2));

    QStringList strlistout;

    QDir meteosatdir(path);
    meteosatdir.setFilter(QDir::Files | QDir::NoSymLinks);
    meteosatdir.setSorting(QDir::Name);

    QStringList strlist = meteosatdir.entryList();


    QStringList::Iterator itc = strlist.begin();

    if(imagetype == "HRV" )
    {
        while( itc != strlist.end() )
        {
            QString st = *itc;
            if(meteosatdir.match(filepattern, *itc) && st.mid(26, 3) == "HRV")
                strlistout.append(*itc);
            itc++;
        }
    }
    else if(imagetype == "VIS_IR")
    {
        while( itc != strlist.end() )
        {
            QString st = *itc;
            QString filespectrum = st.mid(26, 6);
            if(meteosatdir.match(filepattern, *itc) &&
                    (filespectrum == spectrumvector.at(0) || filespectrum == spectrumvector.at(1) || filespectrum == spectrumvector.at(2) ||
                     filespectrum == spectrumvector.at(3) || filespectrum == spectrumvector.at(4) || filespectrum == spectrumvector.at(5)))
                strlistout.append(*itc);
            itc++;
        }
    }


    //    for (int j = 0; j < strlistout.size(); ++j)
    //    {
    //        qDebug() << QString("getGeostationarySegments out ======= %1  %2    %3").arg(imagetype).arg(j).arg(strlistout.at(j));
    //    }

    return strlistout;
}

// Contrast Limited Adaptive Histogram Equalization
int  RSSVideo::CLAHE (unsigned short* pImage, unsigned int uiXRes, unsigned int uiYRes,
                      unsigned short Min, unsigned short Max, unsigned int uiNrX, unsigned int uiNrY,
                      unsigned int uiNrBins, float fCliplimit)
/*   pImage - Pointer to the input/output image
                 *   uiXRes - Image resolution in the X direction
                 *   uiYRes - Image resolution in the Y direction
                 *   Min - Minimum greyvalue of input image (also becomes minimum of output image)
                 *   Max - Maximum greyvalue of input image (also becomes maximum of output image)
                 *   uiNrX - Number of contextial regions in the X direction (min 2, max uiMAX_REG_X)
                 *   uiNrY - Number of contextial regions in the Y direction (min 2, max uiMAX_REG_Y)
                 *   uiNrBins - Number of greybins for histogram ("dynamic range")
                 *   float fCliplimit - Normalized cliplimit (higher values give more contrast)
                 * The number of "effective" greylevels in the output image is set by uiNrBins; selecting
                 * a small value (eg. 128) speeds up processing and still produce an output image of
                 * good quality. The output image will have the same minimum and maximum value as the input
                 * image. A clip limit smaller than 1 results in standard (non-contrast limited) AHE.
                 */
{

    //qDebug() << "int  SegmentImage::CLAHE (unsigned short ............";

    unsigned int uiX, uiY;		  /* counters */
    unsigned int uiXSize, uiYSize, uiSubX, uiSubY; /* size of context. reg. and subimages */
    unsigned int uiXL, uiXR, uiYU, uiYB;  /* auxiliary variables interpolation routine */
    unsigned long ulClipLimit, ulNrPixels;/* clip limit and region pixel count */
    unsigned short* pImPointer;		   /* pointer to image */
    unsigned short aLUT[uiNR_OF_GREY];	    /* lookup table used for scaling of input image */
    unsigned long* pulHist, *pulMapArray; /* pointer to histogram and mappings*/
    unsigned long* pulLU, *pulLB, *pulRU, *pulRB; /* auxiliary pointers interpolation */

    if (uiNrX > uiMAX_REG_X) return -1;	   /* # of regions x-direction too large */
    if (uiNrY > uiMAX_REG_Y) return -2;	   /* # of regions y-direction too large */
    if (uiXRes % uiNrX) return -3;	  /* x-resolution no multiple of uiNrX */
    if (uiYRes % uiNrY) return -4;	  /* y-resolution no multiple of uiNrY */
    if (Max >= uiNR_OF_GREY) return -5;	   /* maximum too large */
    if (Min >= Max) return -6;		  /* minimum equal or larger than maximum */
    if (uiNrX < 2 || uiNrY < 2) return -7;/* at least 4 contextual regions required */
    if (fCliplimit == 1.0) return 0;	  /* is OK, immediately returns original image. */
    if (uiNrBins == 0) uiNrBins = 128;	  /* default value when not specified */

    pulMapArray=(unsigned long *)malloc(sizeof(unsigned long)*uiNrX*uiNrY*uiNrBins);
    if (pulMapArray == 0) return -8;	  /* Not enough memory! (try reducing uiNrBins) */

    uiXSize = uiXRes/uiNrX; uiYSize = uiYRes/uiNrY;  /* Actual size of contextual regions */
    ulNrPixels = (unsigned long)uiXSize * (unsigned long)uiYSize;

    if(fCliplimit > 0.0) {		  /* Calculate actual cliplimit	 */
        ulClipLimit = (unsigned long) (fCliplimit * (uiXSize * uiYSize) / uiNrBins);
        ulClipLimit = (ulClipLimit < 1UL) ? 1UL : ulClipLimit;
    }
    else ulClipLimit = 1UL<<14;		  /* Large value, do not clip (AHE) */
    MakeLut(aLUT, Min, Max, uiNrBins);	  /* Make lookup table for mapping of greyvalues */
    //qDebug() << "Calculate greylevel mappings for each contextual region";
    for (uiY = 0, pImPointer = pImage; uiY < uiNrY; uiY++)
    {
        for (uiX = 0; uiX < uiNrX; uiX++, pImPointer += uiXSize)
        {
            pulHist = &pulMapArray[uiNrBins * (uiY * uiNrX + uiX)];
            MakeHistogram(pImPointer,uiXRes,uiXSize,uiYSize,pulHist,uiNrBins,aLUT);
            ClipHistogram(pulHist, uiNrBins, ulClipLimit);
            MapHistogram(pulHist, Min, Max, uiNrBins, ulNrPixels);
        }
        pImPointer += (uiYSize - 1) * uiXRes;		  /* skip lines, set pointer */
    }

    //qDebug() << "Interpolate greylevel mappings to get CLAHE image";
    for (pImPointer = pImage, uiY = 0; uiY <= uiNrY; uiY++)
    {
        if (uiY == 0)       /* special case: top row */
        {
            uiSubY = uiYSize >> 1;  uiYU = 0; uiYB = 0;
        }
        else
        {
            if (uiY == uiNrY)				  /* special case: bottom row */
            {
                uiSubY = uiYSize >> 1;	uiYU = uiNrY-1;	 uiYB = uiYU;
            }
            else
            {					  /* default values */
                uiSubY = uiYSize; uiYU = uiY - 1; uiYB = uiYU + 1;
            }
        }

        for (uiX = 0; uiX <= uiNrX; uiX++)
        {
            if (uiX == 0)				  /* special case: left column */
            {
                uiSubX = uiXSize >> 1; uiXL = 0; uiXR = 0;
            }
            else
            {
                if (uiX == uiNrX)			  /* special case: right column */
                {
                    uiSubX = uiXSize >> 1;  uiXL = uiNrX - 1; uiXR = uiXL;
                }
                else
                {					  /* default values */
                    uiSubX = uiXSize; uiXL = uiX - 1; uiXR = uiXL + 1;
                }
            }

            pulLU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXL)];
            pulRU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXR)];
            pulLB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXL)];
            pulRB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXR)];
            Interpolate(pImPointer,uiXRes,pulLU,pulRU,pulLB,pulRB,uiSubX,uiSubY,aLUT);
            pImPointer += uiSubX;			  /* set pointer on next matrix */
        }
        pImPointer += (uiSubY - 1) * uiXRes;
    }

    free(pulMapArray);					  /* free space for histograms */
    return 0;						  /* return status OK */
}

void  RSSVideo::ClipHistogram (unsigned long* pulHistogram, unsigned int
                               uiNrGreylevels, unsigned long ulClipLimit)
/* This function performs clipping of the histogram and redistribution of bins.
                 * The histogram is clipped and the number of excess pixels is counted. Afterwards
                 * the excess pixels are equally redistributed across the whole histogram (providing
                 * the bin count is smaller than the cliplimit).
                 */
{
    unsigned long* pulBinPointer, *pulEndPointer, *pulHisto;
    unsigned long ulNrExcess, ulUpper, ulBinIncr, ulStepSize, i;
    long lBinExcess;

    ulNrExcess = 0;  pulBinPointer = pulHistogram;
    for (i = 0; i < uiNrGreylevels; i++) { /* calculate total number of excess pixels */
        lBinExcess = (long) pulBinPointer[i] - (long) ulClipLimit;
        if (lBinExcess > 0) ulNrExcess += lBinExcess;	  /* excess in current bin */
    };

    /* Second part: clip histogram and redistribute excess pixels in each bin */
    ulBinIncr = ulNrExcess / uiNrGreylevels;		  /* average binincrement */
    ulUpper =  ulClipLimit - ulBinIncr;	 /* Bins larger than ulUpper set to cliplimit */

    for (i = 0; i < uiNrGreylevels; i++)
    {
        if (pulHistogram[i] > ulClipLimit) pulHistogram[i] = ulClipLimit; /* clip bin */
        else
        {
            if (pulHistogram[i] > ulUpper)		/* high bin count */
            {
                ulNrExcess -= pulHistogram[i] - ulUpper; pulHistogram[i]=ulClipLimit;
            }
            else
            {					/* low bin count */
                ulNrExcess -= ulBinIncr; pulHistogram[i] += ulBinIncr;
            }
        }
    }

    while (ulNrExcess)       /* Redistribute remaining excess  */
    {
        pulEndPointer = &pulHistogram[uiNrGreylevels]; pulHisto = pulHistogram;

        while (ulNrExcess && pulHisto < pulEndPointer)
        {
            ulStepSize = uiNrGreylevels / ulNrExcess;
            if (ulStepSize < 1) ulStepSize = 1;		  /* stepsize at least 1 */
            for (pulBinPointer=pulHisto; pulBinPointer < pulEndPointer && ulNrExcess; pulBinPointer += ulStepSize)
            {
                if (*pulBinPointer < ulClipLimit)
                {
                    (*pulBinPointer)++;	 ulNrExcess--;	  /* reduce excess */
                }
            }
            pulHisto++;		  /* restart redistributing on other bin location */
        }
    }
}

void  RSSVideo::MakeHistogram (unsigned short* pImage, unsigned int uiXRes,
                               unsigned int uiSizeX, unsigned int uiSizeY,
                               unsigned long* pulHistogram,
                               unsigned int uiNrGreylevels, unsigned short* pLookupTable)
/* This function classifies the greylevels present in the array image into
                 * a greylevel histogram. The pLookupTable specifies the relationship
                 * between the greyvalue of the pixel (typically between 0 and 4095) and
                 * the corresponding bin in the histogram (usually containing only 128 bins).
                 */
{
    unsigned short* pImagePointer;
    unsigned int i;

    for (i = 0; i < uiNrGreylevels; i++) pulHistogram[i] = 0L; /* clear histogram */

    for (i = 0; i < uiSizeY; i++)
    {
        pImagePointer = &pImage[uiSizeX];
        while (pImage < pImagePointer) pulHistogram[pLookupTable[*pImage++]]++;
        pImagePointer += uiXRes;
        pImage = pImagePointer-uiSizeX;
    }
}

void  RSSVideo::MapHistogram (unsigned long* pulHistogram, unsigned short Min, unsigned short Max,
                              unsigned int uiNrGreylevels, unsigned long ulNrOfPixels)
/* This function calculates the equalized lookup table (mapping) by
                 * cumulating the input histogram. Note: lookup table is rescaled in range [Min..Max].
                 */
{
    unsigned int i;  unsigned long ulSum = 0;
    const float fScale = ((float)(Max - Min)) / ulNrOfPixels;
    const unsigned long ulMin = (unsigned long) Min;

    for (i = 0; i < uiNrGreylevels; i++) {
        ulSum += pulHistogram[i]; pulHistogram[i]=(unsigned long)(ulMin+ulSum*fScale);
        if (pulHistogram[i] > Max) pulHistogram[i] = Max;
    }
}

void  RSSVideo::MakeLut (unsigned short * pLUT, unsigned short Min, unsigned short Max, unsigned int uiNrBins)
/* To speed up histogram clipping, the input image [Min,Max] is scaled down to
                 * [0,uiNrBins-1]. This function calculates the LUT.
                 */
{
    int i;
    const unsigned short BinSize = (unsigned short) (1 + (Max - Min) / uiNrBins);

    for (i = Min; i <= Max; i++)  pLUT[i] = (i - Min) / BinSize;
}

void  RSSVideo::Interpolate (unsigned short *pImage, int uiXRes, unsigned long * pulMapLU,
                             unsigned long * pulMapRU, unsigned long * pulMapLB,  unsigned long * pulMapRB,
                             unsigned int uiXSize, unsigned int uiYSize, unsigned short *pLUT)
/* pImage      - pointer to input/output image
                 * uiXRes      - resolution of image in x-direction
                 * pulMap*     - mappings of greylevels from histograms
                 * uiXSize     - uiXSize of image submatrix
                 * uiYSize     - uiYSize of image submatrix
                 * pLUT	       - lookup table containing mapping greyvalues to bins
                 * This function calculates the new greylevel assignments of pixels within a submatrix
                 * of the image with size uiXSize and uiYSize. This is done by a bilinear interpolation
                 * between four different mappings in order to eliminate boundary artifacts.
                 * It uses a division; since division is often an expensive operation, I added code to
                 * perform a logical shift instead when feasible.
                 */
{
    const unsigned int uiIncr = uiXRes-uiXSize; /* Pointer increment after processing row */
    unsigned short GreyValue; unsigned int uiNum = uiXSize*uiYSize; /* Normalization factor */

    unsigned int uiXCoef, uiYCoef, uiXInvCoef, uiYInvCoef, uiShift = 0;

    if (uiNum & (uiNum - 1))   /* If uiNum is not a power of two, use division */
        for (uiYCoef = 0, uiYInvCoef = uiYSize; uiYCoef < uiYSize;  uiYCoef++, uiYInvCoef--,pImage+=uiIncr)
        {
            for (uiXCoef = 0, uiXInvCoef = uiXSize; uiXCoef < uiXSize; uiXCoef++, uiXInvCoef--)
            {
                GreyValue = pLUT[*pImage];		   /* get histogram bin value */
                *pImage++ = (unsigned short ) ((uiYInvCoef * (uiXInvCoef*pulMapLU[GreyValue] + uiXCoef * pulMapRU[GreyValue])
                                                + uiYCoef * (uiXInvCoef * pulMapLB[GreyValue] + uiXCoef * pulMapRB[GreyValue])) / uiNum);
            }
        }
    else
    {			   /* avoid the division and use a right shift instead */
        while (uiNum >>= 1) uiShift++;		   /* Calculate 2log of uiNum */
        for (uiYCoef = 0, uiYInvCoef = uiYSize; uiYCoef < uiYSize; uiYCoef++, uiYInvCoef--,pImage+=uiIncr)
        {
            for (uiXCoef = 0, uiXInvCoef = uiXSize; uiXCoef < uiXSize; uiXCoef++, uiXInvCoef--)
            {
                GreyValue = pLUT[*pImage];	  /* get histogram bin value */
                *pImage++ = (unsigned short)((uiYInvCoef* (uiXInvCoef * pulMapLU[GreyValue] + uiXCoef * pulMapRU[GreyValue])
                                              + uiYCoef * (uiXInvCoef * pulMapLB[GreyValue] + uiXCoef * pulMapRB[GreyValue])) >> uiShift);
            }
        }
    }
}

void RSSVideo::OverlayGeostationary(QImage *im, bool hrvimage, int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla)
{

    pixgeoConversion pixconv;

    int col, save_col;
    int row, save_row;
    bool first = true;

    double lat_deg;
    double lon_deg;
    int ret;

    long coff;
    long loff;
    double cfac;
    double lfac;


    double sub_lon = reader->satlon;
    lat_deg = reader->homelat;
    lon_deg = reader->homelon;
    if (lon_deg > 180.0)
        lon_deg -= 360.0;

    coff = hrvimage ? reader->coffhrv : reader->coff;
    loff = hrvimage ? reader->loffhrv : reader->loff;
    cfac = hrvimage ? reader->cfachrv : reader->cfac;
    lfac = hrvimage ? reader->lfachrv : reader->lfac;

    QPainter qPainter(im);
    qPainter.setBrush(Qt::SolidPattern);
    QPen pen(Qt::yellow, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    qPainter.setPen(pen);

    ret = pixconv.geocoord2pixcoord(sub_lon, lat_deg, lon_deg, coff, loff, cfac, lfac, &col, &row);
    if(ret == 0)
    {
        if(hrvimage)
        {
            if (row > 11136 - lnla ) //LOWER
            {
                col = col - (11136 - lwca);
            }
            else //UPPER
            {
                col = col - (11136 - uwca - 1);
            }
        }

        QPoint pt(col, row);
        QPoint ptleft(col-5, row);
        QPoint ptright(col+5, row);
        QPoint ptup(col, row-5);
        QPoint ptdown(col, row+5);

        qPainter.setPen(qRgb(255, 0, 0));
        qPainter.drawLine(ptleft,ptright);
        qPainter.drawLine(ptup,ptdown);
    }

    qPainter.setPen(Qt::yellow);
    if(!hrvimage)
    {
        for(int k = 0; k < 3; k++)
        {
            if(reader->bgshhsglobeOn[k] == true)
            {
                for(int i = 0; i < gshhs->geooverlay[k].count(); i++)
                {
                    if (gshhs->geooverlay[k].at(i).x() < 0)
                    {
                        first = true;
                    }
                    else if(first == true)
                    {
                        first = false;
                        save_col = (int)gshhs->geooverlay[k].at(i).x();
                        save_row = (int)gshhs->geooverlay[k].at(i).y();
                    }
                    else
                    {
                        qPainter.drawLine(save_col, save_row, (int)gshhs->geooverlay[k].at(i).x(), (int)gshhs->geooverlay[k].at(i).y());
                        save_col = (int)gshhs->geooverlay[k].at(i).x();
                        save_row = (int)gshhs->geooverlay[k].at(i).y();
                    }
                }
            }
        }
    }
    else
        OverlayGeostationaryHRV(&qPainter, leca, lsla, lwca, lnla, ueca, usla, uwca, unla);

    qPainter.end();

}

void RSSVideo::OverlayGeostationaryHRV(QPainter *paint, int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla)
{

    long coff;
    long loff;
    double cfac;
    double lfac;

    int col, save_col;
    int row, save_row;
    bool first = true;

    double lat_deg;
    double lon_deg;
    int ret;

    pixgeoConversion pixconv;

    coff = reader->coffhrv;
    loff = reader->loffhrv;
    cfac = reader->cfachrv;
    lfac = reader->lfachrv;

    double sub_lon = reader->satlon;


    //save_col = 0;
    //save_row = 0;

    if(reader->bgshhsglobeOn[0])
    {
        first = true;

        for (int i=0; i<gshhs->vxp_data[0]->nFeatures; i++)
        {
            for (int j=0; j<gshhs->vxp_data[0]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhs->vxp_data[0]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhs->vxp_data[0]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                if((lon_deg < 90.0 || lon_deg > -90.0))
                {
                    ret = pixconv.geocoord2pixcoord(sub_lon, lat_deg, lon_deg, coff, loff, cfac, lfac, &col, &row);
                    row+=5; //3;
                    col+=3; //2;

                    if(ret == 0)
                    {
                        if (row > 11136 - lnla ) //LOWER
                        {
                            if( save_row <= 11136 - lnla )
                                first = true;
                            col = col - (11136 - lwca);
                        }
                        else //UPPER
                        {
                            if( save_row > 11136 - lnla )
                                first = true;
                            col = col - (11136 - uwca - 1);
                        }

                        if (first)
                        {
                            first = false;
                            save_col = col;
                            save_row = row;
                        }
                        else
                        {
                            paint->setPen(Qt::yellow);
                            paint->drawLine(save_col, save_row, col, row);
                            save_col = col;
                            save_row = row;
                        }
                    }
                    else
                        first = true;
                }
            }
            first = true;
        }
    }

    if(reader->bgshhsglobeOn[1])
    {
        first = true;

        for (int i=0; i<gshhs->vxp_data[1]->nFeatures; i++)
        {
            for (int j=0; j<gshhs->vxp_data[1]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhs->vxp_data[1]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhs->vxp_data[1]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                if(lon_deg < 90.0 || lon_deg > -90.0)
                {
                    ret = pixconv.geocoord2pixcoord(sub_lon, lat_deg, lon_deg, coff, loff, cfac, lfac, &col, &row);
                    row+=5; //3;
                    col+=3; //2;

                    if(ret == 0)
                    {
                        if (row > 11136 - lnla ) //LOWER
                        {
                            if( save_row <= 11136 - lnla )
                                first = true;
                            col = col - (11136 - lwca);
                        }
                        else //UPPER
                        {
                            if( save_row > 11136 - lnla )
                                first = true;
                            col = col - (11136 - uwca - 1);
                        }

                        if (first)
                        {
                            first = false;
                            save_col = col;
                            save_row = row;
                        }
                        else
                        {
                            paint->setPen(Qt::yellow);
                            paint->drawLine(save_col, save_row, col, row);
                            save_col = col;
                            save_row = row;
                        }
                    }
                    else
                        first = true;
                }
            }
            first = true;
        }
    }

    if(reader->bgshhsglobeOn[2])
    {
        first = true;

        for (int i=0; i<gshhs->vxp_data[2]->nFeatures; i++)
        {
            for (int j=0; j<gshhs->vxp_data[2]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhs->vxp_data[2]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhs->vxp_data[2]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                if((lon_deg < 90.0 || lon_deg > -90.0))
                {
                    ret = pixconv.geocoord2pixcoord(sub_lon, lat_deg, lon_deg, coff, loff, cfac, lfac, &col, &row);
                    row+=5; //3;
                    col+=3; //2;

                    if(ret == 0)
                    {
                        if (row > 11136 - lnla ) //LOWER
                        {
                            if( save_row <= 11136 - lnla )
                                first = true;
                            col = col - (11136 - lwca);
                        }
                        else //UPPER
                        {
                            if( save_row > 11136 - lnla )
                                first = true;
                            col = col - (11136 - uwca - 1);
                        }

                        if (first)
                        {
                            first = false;
                            save_col = col;
                            save_row = row;
                        }
                        else
                        {
                            paint->setPen(Qt::yellow);
                            paint->drawLine(save_col, save_row, col, row);
                            save_col = col;
                            save_row = row;
                        }
                    }
                    else
                        first = true;
                }
            }
            first = true;
        }
    }



    //this->update();
}

quint16 RSSVideo::ContrastStretch(quint16 val)
{
    double res;
    res = double(val)*A1 + B1;
    return (res > 255.0 ? 255 : quint16(res));
}

void RSSVideo::SetupContrastStretch(quint16 x1, quint16 y1, quint16 x2, quint16 y2)
{
    //Q_ASSERT(val < 1023 && x1 < 1023 && x2 < 1023 && x3 < 1023 && x4 < 1023 && y1 < 255 && y2 < 255 && y3 < 255 && y4 < 255 && x1 < x2 < x3 < x4 && y1 < y2 < y3 < y4);

    this->d_x1 = (double)x1;
    this->d_x2 = (double)x2;
    this->d_y1 = (double)y1;
    this->d_y2 = (double)y2;

    A1 = (d_y2 - d_y1)/(d_x2 - d_x1);
    B1 = (d_y2 - (A1*d_x2));
    //qDebug() << QString("A1 = %1;B1 = %2;A2 = %3;B2 = %4;A3 = %5;B3 = %6").arg(A1).arg(B1).arg(A2).arg(B2).arg(A3).arg(B3);
}


void RSSVideo::OverlayProjectionGVP()
{
    double lat_deg;
    double lon_deg;
    bool bret;

    double map_x, map_y;
    double save_map_x, save_map_y;

    lat_deg = reader->homelat;
    lon_deg = reader->homelon;
    if (lon_deg > 180.0)
        lon_deg -= 360.0;

    GeneralVerticalPerspective *gvp = new GeneralVerticalPerspective(reader, this, &overlayimageProjection);
    QPainter paint(&overlayimageProjection);
    bret = gvp->map_forward( lon_deg*PIE/180, lat_deg*PIE/180, map_x, map_y) ;

    if(bret)
    {

        QPoint pt(map_x, map_y);
        QPoint ptleft(map_x-5, map_y);
        QPoint ptright(map_x+5, map_y);
        QPoint ptup(map_x, map_y-5);
        QPoint ptdown(map_x, map_y+5);

        paint.setPen(qRgb(255, 0, 0));
        paint.drawLine(ptleft,ptright);
        paint.drawLine(ptup,ptdown);

        //        QPoint pt(map_x-1, map_y-1);
        //        paint->setPen(qRgb(255, 0, 0));
        //        paint->drawEllipse(pt, 2, 2);
    }

    bool first = true;

    if( reader->bgshhsglobeOn[0])
    {
        for (int i=0; i<gshhs->vxp_data[0]->nFeatures; i++)
        {
            for (int j=0; j<gshhs->vxp_data[0]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhs->vxp_data[0]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhs->vxp_data[0]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                bret = gvp->map_forward( lon_deg*PIE/180, lat_deg*PIE/180, map_x, map_y);

                if(bret)
                {
                    if (first)
                    {
                        first = false;
                        save_map_x = map_x;
                        save_map_y = map_y;
                    }
                    else
                    {
                        if(abs(save_map_y - map_y) < 100 && (abs(save_map_x - map_x) < 100))
                        {
                            paint.setPen(QColor(reader->projectionoverlaycolor1));
                            paint.drawLine(save_map_x, save_map_y, map_x, map_y);
                        }
                        save_map_x = map_x;
                        save_map_y = map_y;
                    }
                }
                else
                    first = true;
            }
            first = true;
        }
    }


    first = true;

    if( reader->bgshhsglobeOn[1])
    {
        for (int i=0; i<gshhs->vxp_data[1]->nFeatures; i++)
        {
            for (int j=0; j<gshhs->vxp_data[1]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhs->vxp_data[1]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhs->vxp_data[1]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                bret = gvp->map_forward( lon_deg*PIE/180, lat_deg*PIE/180, map_x, map_y) ;

                if(bret)
                {
                    if (first)
                    {
                        first = false;
                        save_map_x = map_x;
                        save_map_y = map_y;
                    }
                    else
                    {
                        if(abs(save_map_y - map_y) < 100 && (abs(save_map_x - map_x) < 100))
                        {
                            paint.setPen(QColor(reader->projectionoverlaycolor2));
                            paint.drawLine(save_map_x, save_map_y, map_x, map_y);
                        }
                        save_map_x = map_x;
                        save_map_y = map_y;
                    }
                }
                else
                    first = true;
            }
            first = true;
        }
    }


    first = true;

    if( reader->bgshhsglobeOn[2])
    {
        for (int i=0; i<gshhs->vxp_data[2]->nFeatures; i++)
        {
            for (int j=0; j<gshhs->vxp_data[2]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhs->vxp_data[2]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhs->vxp_data[2]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                bret = gvp->map_forward( lon_deg*PIE/180, lat_deg*PIE/180, map_x, map_y) ;

                if(bret)
                {
                    if (first)
                    {
                        first = false;
                        save_map_x = map_x;
                        save_map_y = map_y;
                    }
                    else
                    {
                        if(abs(save_map_y - map_y) < 100 && (abs(save_map_x - map_x) < 100))
                        {
                            paint.setPen(QColor(reader->projectionoverlaycolor3));
                            paint.drawLine(save_map_x, save_map_y, map_x, map_y);
                        }
                        save_map_x = map_x;
                        save_map_y = map_y;
                    }
                }
                else
                    first = true;
            }
            first = true;
        }
    }


    if (reader->projectiontype == "GVP" && reader->gvpgridonproj) //GVP
    {
        for(double lon = -180.0; lon < 180.0; lon+=10.0)
        {
            first = true;
            {
                for(double lat = -90.0; lat < 90.0; lat+=0.5)
                {
                    bret = gvp->map_forward( lon*PIE/180, lat*PIE/180, map_x, map_y);

                    if(bret)
                    {
                        if (first)
                        {
                            first = false;
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                        else
                        {
                            paint.setPen(QColor(reader->projectionoverlaygridcolor));
                            paint.drawLine(save_map_x, save_map_y, map_x, map_y);
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                    }
                    else
                        first = true;

                }
            }
        }

        for(double lat = -80.0; lat < 81.0; lat+=10.0)
        {
            first = true;
            {
                for(double lon = -180.0; lon < 180.0; lon+=1.0)
                {
                    bret = gvp->map_forward( lon*PIE/180.0, lat*PIE/180.0, map_x, map_y);

                    if(bret)
                    {
                        if (first)
                        {
                            first = false;
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                        else
                        {
                            paint.setPen(QColor(reader->projectionoverlaygridcolor));
                            paint.drawLine(save_map_x, save_map_y, map_x, map_y);
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                    }
                    else
                        first = true;

                }
            }
        }
    }


    delete gvp;


}

void RSSVideo::OverlayDate(QImage *im, QString date)
{
    QPainter painter(im);

    QFont f("Courier", reader->overlaydatefontsize, QFont::Bold);
    painter.setFont(f);
    painter.setPen(Qt::yellow);
    painter.setBrush(Qt::NoBrush);

    QString year = date.mid(0, 4);
    QString month = date.mid(4, 2);
    QString day = date.mid(6, 2);
    QString hour = date.mid(8, 2);
    QString minute = date.mid(10, 2);

    painter.drawText(20, im->height() - 20, QString("%1-%2-%3 %4:%5").arg(year).arg(month).arg(day).arg(hour).arg(minute));

    painter.end();

}
