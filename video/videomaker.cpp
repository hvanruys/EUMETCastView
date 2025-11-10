#include "videomaker.h"
#include "Vectors.h"
#include "generalverticalperspective.h"
#include "MSG_data.h"
#include "msgfileaccess.h"
#include "msgdataaccess.h"
#include "pixgeoconversion.h"
#include "qobserver.h"
#include "qsun.h"
#include "qeci.h"

#include <qdir.h>
#include <qelapsedtimer.h>
#include <qpainter.h>

#ifdef _WIN32
#include <hdf5.h>
#else
#include <hdf5.h>
#endif
#include <netcdf.h>

#define ERR(e) {printf("Error: %s\n", nc_strerror(e));}

#define CMB_HISTO_NONE_95 0
#define CMB_HISTO_NONE_100 1
#define CMB_HISTO_EQUALIZE 2
#define CMB_HISTO_CLAHE 3
#define uiNR_OF_GREY (4096)
const unsigned int uiMAX_REG_X = 16;	  /* max. # contextual regions in x-direction */
const unsigned int uiMAX_REG_Y = 16;	  /* max. # contextual regions in y-direction */

VideoMaker::VideoMaker(QString jsonfile, QString timestamp, QObject *parent)
    : QObject{parent}
{

    reader = new JsonVideoReader(jsonfile, timestamp, parent);
    gshhs = new gshhsData(reader->gshhsoverlayfileslist);

    gshhs->setupGeoOverlay(reader->satlon, reader->coff, reader->loff, reader->cfac, reader->lfac);
    this->SetupContrastStretch( 0, 0, 1023, 255);

    overlayimageProjection = QImage(reader->videowidth, reader->videoheight, QImage::Format_ARGB32);

    this->OverlayProjectionGVP();

    //overlayimageProjection.save("tempvideo/overlayproj.png");

    udpSocket = new QUdpSocket();

}

void VideoMaker::SetupContrastStretch(quint16 x1, quint16 y1, quint16 x2, quint16 y2)
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

void VideoMaker::OverlayProjectionGVP()
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
    QPainter paint(&this->overlayimageProjection);
    bret = gvp->map_forward( lon_deg*M_PI/180, lat_deg*M_PI/180, map_x, map_y) ;

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

    if( reader->gshhsoverlayOnlist.at(0))
    {
        for (int i=0; i<gshhs->vxp_data[0]->nFeatures; i++)
        {
            for (int j=0; j<gshhs->vxp_data[0]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhs->vxp_data[0]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhs->vxp_data[0]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                bret = gvp->map_forward( lon_deg*M_PI/180, lat_deg*M_PI/180, map_x, map_y);

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
                            paint.setPen(QColor(reader->projectionoverlaycolorlist.at(0)));
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

    if( reader->gshhsoverlayOnlist.at(1))
    {
        for (int i=0; i<gshhs->vxp_data[1]->nFeatures; i++)
        {
            for (int j=0; j<gshhs->vxp_data[1]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhs->vxp_data[1]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhs->vxp_data[1]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                bret = gvp->map_forward( lon_deg*M_PI/180, lat_deg*M_PI/180, map_x, map_y) ;

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
                            paint.setPen(QColor(reader->projectionoverlaycolorlist.at(1)));
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

    if( reader->gshhsoverlayOnlist.at(2))
    {
        for (int i=0; i<gshhs->vxp_data[2]->nFeatures; i++)
        {
            for (int j=0; j<gshhs->vxp_data[2]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhs->vxp_data[2]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhs->vxp_data[2]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                bret = gvp->map_forward( lon_deg*M_PI/180, lat_deg*M_PI/180, map_x, map_y) ;

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
                            paint.setPen(QColor(reader->projectionoverlaycolorlist.at(2)));
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
                    bret = gvp->map_forward( lon*M_PI/180, lat*M_PI/180, map_x, map_y);

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
                            paint.setPen(QColor(reader->projectionoverlaycolorlist.at(3)));
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
                    bret = gvp->map_forward( lon*M_PI/180.0, lat*M_PI/180.0, map_x, map_y);

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
                            paint.setPen(QColor(reader->projectionoverlaycolorlist.at(3)));
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

    paint.end();

    delete gvp;
}

void VideoMaker::compileImage(QString date, int imagenbr)
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
    quint16 *ptrHRV;

    ptrDayRed = NULL;
    ptrDayGreen = NULL;
    ptrDayBlue = NULL;
    ptrNightRed = NULL;
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


    sendMessages(QString("Start compileImage nbr %1").arg(imagenbr));

    qDebug() << "daykindofimage = " << reader->daykindofimage;

    if(reader->daykindofimage == "VIS_IR")
    {
        ptrDayRed = new quint16[3712 * (reader->brss ? 3*464 : 8*464)];
        memset(ptrDayRed, 0, 3712 * (reader->brss ? 3*464 : 8*464) * sizeof(quint16));
    }
    else if(reader->daykindofimage == "VIS_IR Color" || reader->daykindofimage == "HRV Color")
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

    if(reader->spectrum.at(3).length() > 0)
    {
        ptrNightRed = new quint16[3712 * (reader->brss ? 3*464 : 8*464)];
        memset(ptrNightRed, 0, 3712 * (reader->brss ? 3*464 : 8*464) * sizeof(quint16));
    }

    if(reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color" || reader->daykindofimage == "HRV Color")
    {
        llVIS_IR = this->reader->segmentspathlist;
        //checkAvailableSegments(&llVIS_IR, date);

        if(llVIS_IR.count() == 0)
        {
            sendMessages("Warning : no segments found for 'VIS_IR'!");
            return;
        }
        else
        {
            faVIS_IR.parse(llVIS_IR.at(0));
            prologuefile = faVIS_IR.prologueFile();
            epiloguefile = faVIS_IR.epilogueFile();
        }
    }

    // for(int i = 0; i < llVIS_IR.count(); i++)
    // {
    //     qDebug() << "after checkavailableSegments " << llVIS_IR.at(i);
    // }

    llVIS_IR.sort();

    if(reader->daykindofimage == "HRV" || reader->daykindofimage == "HRV Color")
    {
        llHRV = this->reader->segmentspathlist;
        if(llHRV.count() == 0)
        {
            sendMessages("Warning : no segments found for 'HRV'!");
            return;
        }
        else
        {
            faHRV.parse(llHRV.at(0));
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
            qDebug() << QString("Error : runtime error in reading prologue file : %1").arg(run.what());
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
            qDebug() << QString("Error : runtime error in reading epilogue file : %1").arg(run.what());
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


    for(int i = 0; i < llVIS_IR.count(); i++)
    {
        QFileInfo fileinfo(llVIS_IR.at(i));
        QString filename = fileinfo.baseName();
        getFilenameParameters(filename, filespectrum, filedate, filesequence);
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
                getSegmentSamples(llVIS_IR.at(i), ptrDayRed, filesequence, "VISIRList");
            else if(filespectrum == reader->spectrum.at(1))
                getSegmentSamples(llVIS_IR.at(i), ptrDayGreen, filesequence, "VISIRList");
            else if(filespectrum == reader->spectrum.at(2))
                getSegmentSamples(llVIS_IR.at(i), ptrDayBlue, filesequence, "VISIRList");
            else if(filespectrum == reader->spectrum.at(3))
                getSegmentSamples(llVIS_IR.at(i), ptrNightRed, filesequence, "VISIRList");
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
                    getSegmentSamples(llHRV.at(i), ptrHRV, filesequence, "HRVList");
                else if((reader->daykindofimage == "HRV" || reader->daykindofimage == "HRV Color") && filesequence >= 18 && reader->brss == false)
                    getSegmentSamples(llHRV.at(i), ptrHRV, filesequence, "HRVList");
            }
        }
    }


    QImage imageGeostationary;

    if(reader->daykindofimage == "HRV" || reader->daykindofimage == "HRV Color")
    {
        this->ComposeHRV1(ptrHRV, ptrDayRed, ptrDayGreen, ptrDayBlue, ptrNightRed, imagehrv, date,
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
    else if(reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color")
    {
        this->ComposeVISIR(ptrDayRed, ptrDayGreen, ptrDayBlue, ptrNightRed, imagevisir, date, imagenbr);
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
        // GeneralVerticalPerspective(JsonVideoReader *reader = 0, VideoMaker *video = 0, QImage *imGeostationary = 0, QObject *parent = 0);
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

            painter.drawText(20, gvp->imageProjection->height() - 20, QString("%1 %2-%3-%4 %5:%6").arg(reader->shortname).arg(year).arg(month).arg(day).arg(hour).arg(minute));
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
    delete ptrNightRed;

    sendMessages(QString("Image %1 is finished").arg(imagenbr));


}

void VideoMaker::compileImageMTG(QString date, int imagenbr)
{
    // date = "001" "002" ..."144" for MET_12
    // date = "YYYYMMDDhhmm" for MET_11/10/9

    QString ncfile;
    QByteArray arrayncfile;
    const char* pncfile;
    int ncfileid;
    int grp_data;
    int grp_channel;
    int grp_measured;
    int grp_spectrum;

    int retval;
    int varid;

    float max_radiance_value_of_valid_pixels[4];
    float mean_radiance_value_of_valid_pixels[4];
    float min_radiance_value_of_valid_pixels[4];

    int ndimsp, nvarsp, ngattsp, unlimdimidp;
    //ndimsp	Pointer to location for returned number of dimensions defined for this netCDF dataset. Ignored if NULL.
    //nvarsp	Pointer to location for returned number of variables defined for this netCDF dataset. Ignored if NULL.
    //nattsp	Pointer to location for returned number of global attributes defined for this netCDF dataset. Ignored if NULL.
    //unlimdimidp	Pointer to location for returned ID of the unlimited dimension, if there is one for this netCDF dataset.
    //              If no unlimited length dimension has been defined, -1 is returned. Ignored if NULL.
    //              If there are multiple unlimited dimensions (possible only for netCDF-4 files), only a pointer to the first is returned,
    //              for backward compatibility. If you want them all, use nc_inq_unlimids().

    double geospatial_lat_min, geospatial_lat_max;


    nc_type rh_type;
    int rh_ndims;
    int  rh_dimids[NC_MAX_VAR_DIMS];
    int rh_natts;
    size_t xdim=0, ydim=0;
    float scale_factor[4];
    float add_offset[4];

    float nominal_satellite_subpoint_lon;
    bool trailfilefound = false;

    ushort end_position_row;
    ushort end_position_column;
    ushort start_position_row;
    ushort start_position_column;
    ushort total_number_of_rows;
    ushort total_number_of_columns;



    double gamma = this->reader->gamma;
    double gammafactor = 255 / pow(255, gamma);
    quint16 valgamma;
    quint16 valcontrast;


    int geoindex = this->reader->geoindex;

    QStringList spectrumlist = this->reader->spectrum;

    sendMessages(QString("Start compileImage nbr %1").arg(imagenbr));


    qDebug() << "Start compileImageMTG" << this->reader->segmentspathlist.size();
    qDebug() << "Spectrum vector count = " << this->reader->spectrum.count() << " kindofimage = " << this->reader->daykindofimage;

    QElapsedTimer timer;
    timer.start();



    for(int i = 0; i < 4; i++)
    {
        this->mtg_total_number_of_columns[i] = 0;
        this->mtg_total_number_of_rows[i] = 0;
    }


    trailfilefound = false;
    if(trailfilefound == false)
    {
        QString groupnames[16];
        int rows[16];
        int columns[16];

        groupnames[0] =  "vis_04";
        groupnames[1] =  "vis_05";
        groupnames[2] =  "vis_06";
        groupnames[3] =  "vis_08";
        groupnames[4] =  "vis_09";
        groupnames[5] =  "nir_13";
        groupnames[6] =  "nir_16";
        groupnames[7] =  "nir_22";
        groupnames[8] =  "ir_38";
        groupnames[9] =  "wv_63";
        groupnames[10] =  "wv_73";
        groupnames[11] =  "ir_87";
        groupnames[12] =  "ir_97";
        groupnames[13] =  "ir_105";
        groupnames[14] =  "ir_123";
        groupnames[15] =  "ir_133";

        rows[0] =  11136;  columns[0] =  11136;
        rows[1] =  11136;  columns[1] =  11136;
        rows[2] =  11136;  columns[2] =  11136;
        rows[3] =  11136;  columns[3] =  11136;
        rows[4] =  11136;  columns[4] =  11136;
        rows[5] =  11136;  columns[5] =  11136;
        rows[6] =  11136;  columns[6] =  11136;
        rows[7] =  11136;  columns[7] =  11136;
        rows[8] =  5568;  columns[8] =  5568;
        rows[9] =  5568;  columns[9] =  5568;
        rows[10] =  5568;  columns[10] =  5568;
        rows[11] =  5568;  columns[11] =  5568;
        rows[12] =  5568;  columns[12] =  5568;
        rows[13] =  5568;  columns[13] =  5568;
        rows[14] =  5568;  columns[14] =  5568;
        rows[15] =  5568;  columns[15] =  5568;

        for(int i = 0; i < (this->reader->daykindofimage == "VIS_IR Color" ? (this->reader->spectrum.at(3).length() > 0 ? 4 : 3) : 1); i++)
        {
            for(int j = 0; j < 16; j++ )
            {
                if(this->reader->spectrum.at(i) == groupnames[j])
                {
                    this->mtg_total_number_of_columns[i] = columns[j];
                    this->mtg_total_number_of_rows[i] = rows[j];
                }
            }
        }
    }


    QByteArray ba;


    for(int j = 0; j < this->reader->segmentspathlist.size(); j++)
    {
        if(this->reader->segmentspathlist.at(j).contains("BODY"))
        {
            ncfile = this->reader->segmentspathlist.at(j);
            arrayncfile = ncfile.toUtf8();
            pncfile = arrayncfile.constData();

            qDebug() << "Starting netCDF file " + ncfile;
            int ind = ncfile.indexOf(".nc");
            int findex = ncfile.mid(ind - 4, 4).toInt();

            this->vec.append(findex);

            retval = nc_open(pncfile, NC_NOWRITE, &ncfileid);
            if(retval != NC_NOERR) qDebug() << "error opening netCDF file " << this->reader->segmentspathlist.at(j);

            retval = nc_inq(ncfileid, &ndimsp, &nvarsp, &ngattsp, &unlimdimidp);
            if(retval != NC_NOERR) qDebug() << "error nc_inq " << this->reader->segmentspathlist.at(j);

            retval = nc_get_att_double(ncfileid, NC_GLOBAL, "geospatial_lat_min", &geospatial_lat_min);
            if(retval != NC_NOERR) qDebug() << "error nc_get_att_double for geospatial_lat_min";

            retval = nc_get_att_double(ncfileid, NC_GLOBAL, "geospatial_lat_max", &geospatial_lat_max);
            if(retval != NC_NOERR) qDebug() << "error nc_get_att_double for geospatial_lat_max";

            qDebug() << QString("index = %1 geospatial lat min = %2 lat max = %3 nbr of global att = %4").arg(j).arg(geospatial_lat_min)
                            .arg(geospatial_lat_max).arg(ngattsp);

            retval = nc_inq_ncid(ncfileid, "data", &grp_data);
            if(retval != NC_NOERR) qDebug() << "error opening data group";

            for(int i = 0; i < (this->reader->daykindofimage == "VIS_IR Color" ? (this->reader->spectrum.at(3).length() > 0 ? 4 : 3) : 1); i++)
            {
                qDebug() << "reading radiance from channel i = " << i << " spectrum = " << this->reader->spectrum.at(i);

                QString strspectrum = this->reader->spectrum.at(i);
                ba = strspectrum.toLocal8Bit();
                const char *c_channel = ba.data();
                retval = nc_inq_ncid(grp_data, c_channel, &grp_spectrum);
                if(retval != NC_NOERR) qDebug() << "error opening " << strspectrum;

                retval = nc_inq_ncid(grp_spectrum, "measured", &grp_measured);
                if(retval != NC_NOERR) qDebug() << "error opening " << strspectrum << "/measured";


                if ((retval = nc_inq_varid(grp_measured, "start_position_row", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_measured, varid, &start_position_row)))
                    ERR(retval);

                if ((retval = nc_inq_varid(grp_measured, "start_position_column", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_measured, varid, &start_position_column)))
                    ERR(retval);

                if ((retval = nc_inq_varid(grp_measured, "end_position_row", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_measured, varid, &end_position_row)))
                    ERR(retval);

                if ((retval = nc_inq_varid(grp_measured, "end_position_column", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_measured, varid, &end_position_column)))
                    ERR(retval);


                retval = nc_get_att_ushort(grp_measured, varid, "_FillValue", &fillvalue[i]);
                if (retval != NC_NOERR) qDebug() << "error reading _FillValue";
                this->fillvalue[i] = fillvalue[i];
                qDebug() << QString("FillValue for color %1 = %2").arg(i).arg(fillvalue[i]);


                //if(retval == 0 && i == 0)
                //{
                //    qDebug() << QString("start position row = %1 column = %2").arg(start_position_row).arg(start_position_column);
                //    qDebug() << QString("end position row = %1 column = %2").arg(end_position_row).arg(end_position_column);
                    //qDebug() << QString("j = %1 findex = %2 nbr of rows = %3 column = %4").arg(j).arg(findex).arg(end_position_row - start_position_row + 1).arg(
                    //                end_position_column - start_position_column + 1);
                //}

                this->mtg_start_position_row[i][findex - 1] = start_position_row;
                this->mtg_end_position_row[i][findex - 1] = end_position_row;

                this->mtg_start_position_column[i][findex -1] = start_position_column;
                this->mtg_end_position_column[i][findex - 1] = end_position_column;

                this->mtg_nbr_of_rows[i][findex - 1] = end_position_row - start_position_row + 1;
                this->mtg_nbr_of_columns[i][findex - 1] = end_position_column - start_position_column + 1;

                this->ptrMTG[i][findex - 1] = new quint16[mtg_nbr_of_rows[i][findex - 1] * mtg_nbr_of_columns[i][findex - 1]];

                retval = nc_inq_varid(grp_measured, "effective_radiance", &varid);
                if(retval != NC_NOERR) qDebug() << "error opening effective radiance from channel " << strspectrum << "/measured";


                retval = nc_get_var_ushort(grp_measured, varid, this->ptrMTG[i][findex - 1]);
                if(retval != NC_NOERR) qDebug() << "error reading effective radiance from channel " << strspectrum << "/measured" << " findex = " << findex << " error = " << retval;
            }
            retval = nc_close(ncfileid);
            if (retval != NC_NOERR) qDebug() << "error closing file " << ncfile;

        }
    }

    for(int i = 0; i < (this->reader->spectrum.at(3).length() > 0 ? 4 : 3); i++)
    {
        qDebug() << "for colour = " << i;
        for(int j = 0; j < 40; j++)
        {
            if(this->mtg_start_position_row[i][j] > 0)
            {
                qDebug() << QString("j = %1 start position row = %2 column = %3").arg(j).arg(this->mtg_start_position_row[i][j]).arg(this->mtg_start_position_column[i][j]);
                qDebug() << QString("j = %1 end position row = %2 column = %3").arg(j).arg(this->mtg_end_position_row[i][j]).arg(this->mtg_end_position_column[i][j]);
                qDebug() << QString("diff row = %1").arg(this->mtg_end_position_row[i][j] - this->mtg_start_position_row[i][j] + 1);
                this->total_rows[i] += this->mtg_end_position_row[i][j] - this->mtg_start_position_row[i][j] + 1;
                this->mtg_total_rows_per_segment[i][j] = this->mtg_end_position_row[i][j] - this->mtg_start_position_row[i][j] + 1;
            }
        }
    }

    for(int i = 0; i < (this->reader->spectrum.at(3).length() > 0 ? 4 : 3); i++)
    {
        qDebug() << "this->total_rows[" << i << "] = " << this->total_rows[i];
    }

    for(int i = 0; i < vec.length(); i++)
    {

        qDebug() << QString("findex ==> vec[%1] = %2").arg(i).arg(vec.at(i));
    }

    for(int i = 0; i < vec.length(); i++)
    {
        this->serialMinMaxMTG(vec[i]);
    }


    for(int i = 0; i < (this->reader->spectrum.at(3).length() > 0 ? 4 : 3); i++)
    {
        stat_min[i] = 65535;
        stat_max[i] = 0;
    }

    for(int i = 0; i < (this->reader->daykindofimage == "VIS_IR Color" ? (this->reader->spectrum.at(3).length() > 0 ? 4 : 3) : 1); i++) {
        for (int j = 0; j < vec.length(); j++)
        {
            quint16 val = this->mtg_stat_min[i][vec[j]-1];
            if(val != this->fillvalue[i])
            {
                if(val < this->stat_min[i])
                    this->stat_min[i] = val;
            }
            val = this->mtg_stat_max[i][vec[j]-1];
            if(val != 0)
            {
                if(val > this->stat_max[i])
                    this->stat_max[i] = val;
            }

        }
        qDebug() << QString("stat_min [%1] = %2 stat_max [%3] = %4")
                        .arg(i).arg(this->stat_min[i]).arg(i).arg(this->stat_max[i]);

    }


    for(int i = 0; i < vec.length(); i++)
    {
        this->serialLUTGeoMTG(vec[i]);
    }


    for(int colorindex = 0; colorindex < (this->reader->daykindofimage == "VIS_IR Color" ? (this->reader->spectrum.at(3).length() > 0 ? 4 : 3) : 1); colorindex++)
    {
        this->active_pixels[colorindex] = 0;
        for (int index = 0; index < vec.length(); index++)
        {
            this->active_pixels[colorindex] += this->mtg_active_pixels[colorindex][vec[index]-1];
        }
        qDebug() << QString("active_pixels[%1] = %2").arg(colorindex).arg(this->active_pixels[colorindex]);
    }

    //    for(int i = 0; i < (this->kindofimage == "VIS_IR Color" ? 3 : 1); i++)
    //    {
    //        qDebug() << QString("stat_min[%1] = %2 stat_max[%3] = %4 active_pixels[%5] = %6").arg(i).arg(stat_min[i]).arg(i).arg(stat_max[i]).arg(i).arg(this->active_pixels[i]);
    //    }

    double newscale;
    long histogram[4][4096];

    for(int colorindex = 0; colorindex < (this->reader->daykindofimage == "VIS_IR Color" ? (this->reader->spectrum.at(3).length() > 0 ? 4 : 3) : 1); colorindex++)
    {
        for (int i = 0; i < 4096; i++) {
            histogram[colorindex][i] = 0;
        }
    }

    long long totpixels[4];
    for(int i = 0; i < 4; i++)
        totpixels[i] = 0;

    for(int colorindex = 0; colorindex < (this->reader->daykindofimage == "VIS_IR Color" ? (this->reader->spectrum.at(3).length() > 0 ? 4 : 3) : 1); colorindex++)
    {
        for (int index = 0; index < vec.length(); index++) {
            for (int i = 0; i < 4096; i++) {
                histogram[colorindex][i] += this->mtg_histogram[colorindex][vec[index]-1][i];
                totpixels[colorindex] += this->mtg_histogram[colorindex][vec[index]-1][i];
            }
        }
        qDebug() << QString("totpixels[%1] = %2 active_pixels[%3] = %4").arg(colorindex).arg(totpixels[colorindex]).arg(colorindex).arg(this->active_pixels[colorindex]);
    }


    //    for (int j = 0; j < 4096; j++)
    //    {
    //        qDebug() << "histogram " << j << " " << histogram[0][j];
    //    }


    for(int colorindex = 0; colorindex < (this->reader->daykindofimage == "VIS_IR Color" ? (this->reader->spectrum.at(3).length() > 0 ? 4 : 3) : 1); colorindex++)
    {
        //        newscale = (double)(4095.0 / (double)(this->active_pixels[colorindex] - stat_min[colorindex]));
        newscale = (double)(4095.0 / (double)(totpixels[colorindex] - stat_min[colorindex]));

        qDebug() << QString("newscale = %1 active pixels = %2 11136*11136 = %3").arg(newscale).arg(this->active_pixels[colorindex]).arg(11136*11136);


        unsigned long long sum_ch = 0;
        bool okmin, okmax;

        okmin = false;
        okmax = false;

        this->minRadianceIndex[colorindex] = 65535;
        this->maxRadianceIndex[colorindex] = 65535;

        // min/maxRadianceIndex = index of 95% ( 2.5% of 1024 = 25, 97.5% of 1024 = 997 )
        // min/maxRadianceIndex = index of 95% ( 2.5% of 4096 = 102, 97.5% of 4096 = 3993 )

        for( int i = 0; i < 4096; i++)
        {
            sum_ch += histogram[colorindex][i];
            this->lut_mtg[colorindex][i] = qRound((sum_ch - stat_min[colorindex]) * newscale);
            this->lut_mtg[colorindex][i] = (this->lut_mtg[colorindex][i] > 4095 ? 4095 : this->lut_mtg[colorindex][i]);
            //        qDebug() << QString("stats_ch[0][%1] = %2 lut_ch[0][%3] = %4").arg(i).arg(stats_ch[0][i]).arg(i).arg(this->lut_ch[0][i]);
            if(this->lut_mtg[colorindex][i] > 102 && okmin == false)
            {
                okmin = true;
                this->minRadianceIndex[colorindex] = i;
            }
            if(this->lut_mtg[colorindex][i] > 3993 && okmax == false)
            {
                okmax = true;
                this->maxRadianceIndex[colorindex] = i;
            }
        }


        //        for (int j = 0; j < 4096; j++)
        //        {
        //            qDebug() << QString("histogram[%1][%2] = %3 LUT[%4][%5] = %6").arg(colorindex).arg(j).arg(histogram[colorindex][j])
        //                        .arg(colorindex).arg(j).arg(this->lut_mtg[colorindex][j]);
        //        }


        qDebug() << QString("minRadianceIndex [%1] = %2 maxRadianceIndex [%3] = %4 active_pixels = %5")
                        .arg(colorindex).arg(this->minRadianceIndex[colorindex]).arg(colorindex).arg(this->maxRadianceIndex[colorindex])
                        .arg(this->active_pixels[colorindex]);

    }

    this->InitializeImageGeostationary(this->mtg_total_number_of_columns[0], this->total_rows[0]);

    this->COFF = this->mtg_total_number_of_columns[0] == 11136 ? this->reader->coffhrv : this->reader->coff;
    this->LOFF = this->mtg_total_number_of_columns[0] == 11136 ? this->reader->loffhrv : this->reader->loff;
    this->CFAC = this->mtg_total_number_of_columns[0] == 11136 ? this->reader->cfachrv : this->reader->cfac;
    this->LFAC = this->mtg_total_number_of_columns[0] == 11136 ? this->reader->lfachrv : this->reader->lfac;


    ///////////////////////////////////////////////////////////////////////
    // tot_rows = 0;
    // tot_rest_rows = this->total_rows;

    // for(int i = vec.length() - 1; i >= 0; i--)
    // {
    //     TestCalculateImageMTG(vec.at(i));
    // }
    // return;
    ////////////////////////////////////////////////////////////////////////




    this->SetupContrastStretch( 0, 0, 1023, 255);

    // if(this->reader->spectrum.at(3).length() > 0)
    // {
    //     this->ptrimageGeoNight.reset(new quint16[ 5568 * 5568 ]);

    //     for(int i = 0; i < vec.length(); i++)
    //     {
    //         this->CalculateImageMTGNight(vec[i]);
    //     }
    // }

    for(int colorindex = 0; colorindex < (this->reader->daykindofimage == "VIS_IR Color" ? (this->reader->spectrum.at(3).length() > 0 ? 4 : 3) : 1); colorindex++)
    {
        tot_rows[colorindex] = 0;
        tot_rest_rows[colorindex] = this->total_rows[colorindex];
    }

    if(this->reader->spectrum.at(3).length() > 0)
    {
        ptrimageGeoNight = new QImage(this->mtg_total_number_of_columns[3], this->total_rows[3], QImage::Format_ARGB32);
        QColor nuts(0,0,0, 255);  //(alphazero == true ? 0 : 255 ));
        ptrimageGeoNight->fill(nuts);

        //ptrimageGeoNight = new QImage(this->mtg_total_number_of_columns[3], this->total_rows[3], QImage::Format_Grayscale8);

        for(int i = 0; i < vec.length(); i++)
        {
            this->CalculateImageMTGNight(vec[i]);
        }

        ptrimageGeoNight->save("ptrimagegeonight.png");
    }

    for(int colorindex = 0; colorindex < (this->reader->daykindofimage == "VIS_IR Color" ? (this->reader->spectrum.at(3).length() > 0 ? 4 : 3) : 1); colorindex++)
    {
        tot_rows[colorindex] = 0;
        tot_rest_rows[colorindex] = this->total_rows[colorindex];
    }

    for(int i = 0; i < vec.length(); i++)
    {
        this->CalculateImageMTG(vec.at(i));
    }

    this->ptrimageGeostationary->save("ptrimagegeo.png");

    QString year;
    QString month;
    QString day;
    QString hour;
    QString minute;

    if(reader->projectiontype == "GVP")
    {
        // GeneralVerticalPerspective(JsonVideoReader *reader = 0, VideoMaker *video = 0, QImage *imGeostationary = 0, QObject *parent = 0);
        GeneralVerticalPerspective *gvp = new GeneralVerticalPerspective(reader, this, ptrimageGeostationary);

        QPainter painter(gvp->imageProjection);
        gvp->CreateMapFromGeoStationary(&painter, mtg_end_position_row[0][vec.at(vec.length() - 1) - 1],0,0,0,0,0,0,0);

        if(reader->boverlaydate)
        {
            QFont f("Courier", reader->overlaydatefontsize, QFont::Bold);
            painter.setFont(f);
            painter.setPen(Qt::yellow);
            painter.setBrush(Qt::NoBrush);
            QString subscript;
            this->getTimeFromIndex(imagenbr, &subscript);
            painter.drawText(20, gvp->imageProjection->height() - 20, subscript);
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





    for(int i = 0; i < (this->reader->daykindofimage == "VIS_IR Color" ? (this->reader->spectrum.at(3).length() > 0 ? 4 : 3) : 1); i++)
    {
        for(int j = 0; j < vec.length(); j++)
        {
            if(this->ptrMTG[i][vec[j]-1] != NULL)
            {
                delete [] this->ptrMTG[i][vec[j]-1];
                this->ptrMTG[i][vec[j]-1] = NULL;
            }
        }
    }

    //this->ptrimageGeoNight->reset();

    sendMessages(QString("Image %1 is finished").arg(imagenbr));

    sendMessages(QString("===> The image process took %1 milliseconds").arg(timer.elapsed()));

    return;
}

void VideoMaker::TestCalculateImageMTG(int findex)
{
    qDebug() << "Total rows = " << this->total_rows[0];

    qDebug() << "Total rows per segment index = " << findex - 1 << " = " << this->mtg_total_rows_per_segment[0][findex - 1];
    tot_rows[0] += this->mtg_total_rows_per_segment[0][findex - 1];
    qDebug() << "scanline from line = " << tot_rest_rows[0] - 1 << " to " << this->total_rows[0] - tot_rows[0];
    tot_rest_rows[0] -= this->mtg_total_rows_per_segment[0][findex - 1];
}

void VideoMaker::CalculateImageMTG(int findex)
{
    quint16 pixel[4];
    quint16 indexoutpixel[4];
    quint8 pixelout[4];
    QRgb *row_col;
    QRgb *row_col_night;
    QImage *im;
    quint16 valgamma;
    quint8 valcontrast;
    double gamma = this->reader->gamma;
    double gammafactor = 1023 / pow(1023, gamma);

    im = this->ptrimageGeostationary;

    Vector3 solar_vector;
    Vector3 vel;
    QObserver observer;
    QSgp4Date dat;
    QGeodetic qgeo;
    QTopocentric qtopo;

    double elev;
    double twilight = 12.0;

    qDebug() << "start VideoMaker::CalculateImageMTG(int index) = " << findex;

    int year, month, day, hours, minutes;
    // timestamp for MTG = "001" "002"  ...  "144"
    year = this->reader->selectiondate.mid(0, 4).toInt();
    month = this->reader->selectiondate.mid(4, 2).toInt();
    day = this->reader->selectiondate.mid(6, 2).toInt();

    int ii = this->reader->timestamp.toUInt();
    int totmin = (ii - 1) * 10;
    hours = (totmin - (totmin % 60))/60;
    minutes = totmin - (hours * 60);

    pixgeoConversion pixconv;

    double sub_lon = this->reader->satlon;

    long coff = this->reader->coffhrv;
    long loff = this->reader->loffhrv;
    double cfac = this->reader->cfachrv;
    double lfac = this->reader->lfachrv;

    double latitude, longitude;
    int ret = 0;

    int linelocal = 0;

    //    2024-02-27 18:30:26.047 Debug: "start position row = 1 column = 1"
    //    2024-02-27 18:30:26.047 Debug: "end position row = 278 column = 11136"

    //    2024-02-27 18:33:14.782 Debug: "start position row = 1 column = 1"
    //    2024-02-27 18:33:14.782 Debug: "end position row = 139 column = 5568"

    long count_error = 0;


    for(int i = 0; i < (this->reader->daykindofimage == "VIS_IR Color" ? (this->reader->spectrum.at(3).length() > 0 ? 4 : 3) : 1); i++)
    {
        tot_rows[i] += this->mtg_total_rows_per_segment[i][findex - 1];
        qDebug() << "Total rows = " << this->total_rows[i] << "  total rest rows = " << this->tot_rest_rows[i] << " total_rows - tot_rows = " << this->total_rows[i] - tot_rows[i] <<
            "  this->mtg_total_rows_per_segment[" << i << "][findex - 1] = " << this->mtg_total_rows_per_segment[i][findex - 1];
    }

    for(int line = tot_rest_rows[0] - 1; line >= this->total_rows[0] - tot_rows[0]; line--)
    {
        row_col = (QRgb*)im->scanLine(line);

        for (int pixelx = this->mtg_start_position_column[0][findex-1] - 1; pixelx < this->mtg_end_position_column[0][findex-1]; pixelx++)
        {
            for(int colorindex = 0; colorindex < (this->reader->daykindofimage == "VIS_IR Color" ? 3 : 1); colorindex++)
            {
                //                if(colorindex < 3)
                pixel[colorindex] = *(this->ptrMTG[colorindex][findex-1] + (this->mtg_nbr_of_columns[colorindex][findex-1] * linelocal) + pixelx);
                //                else
                //                {
                //                    long tot = 5568 * (linelocal/2) + (int)(pixelx/2);
                //                    long nbr_rows_per_segment = this->mtg_end_position_row[3][index-1] - this->mtg_start_position_row[3][index-1] + 1;
                //                    if(nbr_rows_per_segment * 5568 < tot)
                //                    {
                //                        count_error++;
                //                    }
                //                }

                if(this->histogrammethod == CMB_HISTO_NONE_95)
                {
                    if(pixel[colorindex] != this->fillvalue[colorindex])
                    {
                        indexoutpixel[colorindex] = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel[colorindex] - this->minRadianceIndex[colorindex] ) / (float)(this->maxRadianceIndex[colorindex] - this->minRadianceIndex[colorindex])), 0), 1023);
                        valgamma = pow( indexoutpixel[colorindex], gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        pixelout[colorindex] = (valcontrast);
                    }
                }
                else if(this->histogrammethod == CMB_HISTO_NONE_100)
                {
                    if(pixel[colorindex] != this->fillvalue[colorindex])
                    {
                        indexoutpixel[colorindex] = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel[colorindex] - this->stat_min[colorindex] ) / (float)(this->stat_max[colorindex] - this->stat_min[colorindex])), 0), 1023);
                        valgamma = pow( indexoutpixel[colorindex], gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        pixelout[colorindex] = (valcontrast);
                    }
                }
                else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( pixel[colorindex] != this->fillvalue[colorindex])
                    {
                        quint16 val = (quint16)qMin(qMax(qRound(4095.0 * (float)(pixel[colorindex] - this->stat_min[colorindex] ) / (float)(this->stat_max[colorindex] - this->stat_min[colorindex])), 0), 1023);
                        indexoutpixel[colorindex] = (quint16)(qMin(qMax((int)this->lut_mtg[colorindex][val], 0), 4095));
                        valgamma = pow( indexoutpixel[colorindex]/4, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        pixelout[colorindex] = (valcontrast);
                    }
                }
            }

            //            ret = pixconv.pixcoord2geocoord(sub_lon, opts.geosatellites[geoindex].imagewidthhrv0 - 1 - pixelx,
            //                                            opts.geosatellites[geoindex].imageheighthrv0 - 1 - line, coff, loff, cfac, lfac, &latitude, &longitude);


            if(this->reader->spectrum.at(3).length() > 0)
            {
                row_col_night = (QRgb*)this->ptrimageGeoNight->scanLine(line/2);
                pixelout[3] = row_col_night[(int)(pixelx/2)];
            }

            if(this->reader->daykindofimage == "VIS_IR")
            {
                if(pixel[0] != this->fillvalue[0])
                {
                    pixelout[0] = quint16(this->reader->inverse[0] ? (255 - pixelout[0]) : pixelout[0]);
                    pixelout[1] = pixelout[0];
                    pixelout[2] = pixelout[0];
                    row_col[pixelx] = qRgb(pixelout[0], pixelout[1], pixelout[2]);
                }
                else
                {
                    row_col[pixelx] = qRgb(0, 0, 0);
                }
            }
            else if(this->reader->daykindofimage == "VIS_IR Color")
            {
                if(this->reader->spectrum.at(3).length() > 0)
                {
                    if(pixelout[3] != this->fillvalue[3])
                    {
                        //ret = pixconv.pixcoord2geocoord(sub_lon, pixelx/2, (11136 - 1 - line)/2 - this->mtg_end_position_row[3][vec.at(vec.length()-1) - 1], coff/2, loff/2, cfac/2, lfac/2, &latitude, &longitude);
                        //ret = pixconv.pixcoord2geocoord(sub_lon, pixelx/2, - (5568 - 1) + (line/2) + this->mtg_end_position_row[3][vec.at(vec.length()-1) - 1], coff/2, loff/2, cfac/2, lfac/2, &latitude, &longitude);
                        // ret = pixconv.pixcoord2geocoord(sub_lon, pixelx/2, - (5568 - 1) + (line/2)  + 5845, coff/2, loff/2, cfac/2, lfac/2, &latitude, &longitude);
                        ret = pixconv.pixcoord2geocoord(sub_lon, pixelx/2, 5568 - this->mtg_end_position_row[3][vec.at(vec.length()-1) - 1] + (line/2), coff/2, loff/2, cfac/2, lfac/2, &latitude, &longitude);

                        if(ret > -1)
                        {
                            observer.SetLocation(latitude, longitude, 0.0);
                            dat.Set(year, month, day, hours, minutes, 0, true);
                            QSun::Calculate_Solar_Position(dat.Julian(), &solar_vector);
                            QEci qeci(solar_vector, vel, dat);
                            qtopo = observer.GetLookAngle(qeci);
                            elev = qtopo.elevation * 180.0/PIE;


                            if(elev <= 0.0)
                            {
                                row_col[pixelx] = qRgb(pixelout[3], pixelout[3], pixelout[3]);
                                //row_col[pixelx] = qRgb(255, 0, 0);
                            }
                            else if(elev > 0.0 && elev < twilight)
                            {
                                int percentday = (int)(100.0 * elev / twilight);
                                int percentnight = 100 - percentday;
                                int red = (percentday*pixelout[0] + percentnight*pixelout[3])/100;
                                red = (red > 255 ? 255 : red);

                                int green = (percentday*pixelout[1] + percentnight*pixelout[3])/100;
                                green = (green > 255 ? 255 : green);

                                int blue = (percentday*pixelout[2] + percentnight*pixelout[3])/100;
                                blue = (blue > 255 ? 255 : blue);

                                row_col[pixelx] = qRgb(red, green, blue);
                            }
                            else
                            {
                                pixelout[0] = quint16(this->reader->inverse[0] ? (255 - pixelout[0]) : pixelout[0]);
                                pixelout[1] = quint16(this->reader->inverse[1] ? (255 - pixelout[1]) : pixelout[1]);
                                pixelout[2] = quint16(this->reader->inverse[2] ? (255 - pixelout[2]) : pixelout[2]);
                                row_col[pixelx] = qRgb(pixelout[0], pixelout[1], pixelout[2]);
                            }

                        }
                        else
                        {
                            row_col[pixelx] = qRgb(0,255,255);
                        }
                    }
                }
                else
                {
                    if(pixel[0] != this->fillvalue[0])
                    {
                        pixelout[0] = quint16(this->reader->inverse[0] ? (255 - pixelout[0]) : pixelout[0]);
                        pixelout[1] = quint16(this->reader->inverse[1] ? (255 - pixelout[1]) : pixelout[1]);
                        pixelout[2] = quint16(this->reader->inverse[2] ? (255 - pixelout[2]) : pixelout[2]);
                        row_col[pixelx] = qRgb(pixelout[0], pixelout[1], pixelout[2]);
                    }
                }
            }
        }

        linelocal++;
    }



    for(int i = 0; i < (this->reader->daykindofimage == "VIS_IR Color" ? (this->reader->spectrum.at(3).length() > 0 ? 4 : 3) : 1); i++)
    {
        tot_rest_rows[i] -= this->mtg_total_rows_per_segment[i][findex - 1];
    }


    // if(this->reader->spectrum.at(3).length() > 0)
    // {
    //     tot_rows[3] += this->mtg_total_rows_per_segment[3][findex - 1];

    //     for(int line = tot_rest_rows[3] - 1; line >= this->total_rows[3] - tot_rows[3]; line--)
    //     {
    //         row_col = (QRgb*)im->scanLine(line);

    //         for (int pixelx = this->mtg_start_position_column[3][findex-1] - 1; pixelx < this->mtg_end_position_column[3][findex-1]; pixelx++)
    //         {
    //             ret = pixconv.pixcoord2geocoord(sub_lon, pixelx, 11136 - 1 - line, coff, loff, cfac, lfac, &latitude, &longitude);
    //             pixelout[3] = this->ptrimageGeoNight[5568 * line + pixelx];
    //         }
    //     }
    // }
}

// void VideoMaker::CalculateImageMTG(int findex)
// {
//     quint16 pixel[4];
//     quint16 indexoutpixel[4];
//     quint8 pixelout[4];
//     QRgb *row_col;
//     QImage *im;
//     quint16 valgamma;
//     quint8 valcontrast;
//     double gamma = this->reader->gamma;
//     double gammafactor = 1023 / pow(1023, gamma);

//     im = this->ptrimageGeostationary;

//     Vector3 solar_vector;
//     Vector3 vel;
//     QObserver observer;
//     QSgp4Date dat;
//     QGeodetic qgeo;
//     QTopocentric qtopo;

//     double elev;
//     double twilight = 12.0;

//     qDebug() << "start VideoMaker::CalculateImageMTG(int index) = " << findex;

//     int year, month, day, hours, minutes;
//     // timestamp for MTG = "001" "002"  ...  "144"
//     year = this->reader->selectiondate.mid(0, 4).toInt();
//     month = this->reader->selectiondate.mid(4, 2).toInt();
//     day = this->reader->selectiondate.mid(6, 2).toInt();

//     int ii = this->reader->timestamp.toUInt();
//     int totmin = (ii - 1) * 10;
//     hours = (totmin - (totmin % 60))/60;
//     minutes = totmin - (hours * 60);

//     pixgeoConversion pixconv;

//     double sub_lon = this->reader->satlon;

//     long coff = this->reader->coffhrv;
//     long loff = this->reader->loffhrv;
//     double cfac = this->reader->cfachrv;
//     double lfac = this->reader->lfachrv;

//     double latitude, longitude;
//     int ret = 0;

//     int linelocal = 0;

//     //    2024-02-27 18:30:26.047 Debug: "start position row = 1 column = 1"
//     //    2024-02-27 18:30:26.047 Debug: "end position row = 278 column = 11136"

//     //    2024-02-27 18:33:14.782 Debug: "start position row = 1 column = 1"
//     //    2024-02-27 18:33:14.782 Debug: "end position row = 139 column = 5568"

//     long count_error = 0;

//     qDebug() << "Total rows = " << this->total_rows;
//     for(int line = 0 ; line < this->mtg_total_rows_per_segment[0][findex- 1]; line++)
//     {
//         qDebug() << "scanline = " << this->total_rows - (this->mtg_end_position_row[0][findex-1] -  this->mtg_start_position_row[0][findex-1]) - line << " start_pos = "
//                  <<  this->mtg_start_position_row[0][findex-1] << " end_pos " << this->mtg_end_position_row[0][findex-1] << " line = " << line;
//         row_col = (QRgb*)im->scanLine(this->total_rows - (this->mtg_end_position_row[0][findex-1] -  this->mtg_start_position_row[0][findex-1]) - line);

//         for (int pixelx = this->mtg_start_position_column[0][findex-1] - 1; pixelx < this->mtg_end_position_column[0][findex-1]; pixelx++)
//         {
//             for(int colorindex = 0; colorindex < (this->reader->daykindofimage == "VIS_IR Color" ? 3 : 1); colorindex++)
//             {
//                 //                if(colorindex < 3)
//                 pixel[colorindex] = *(this->ptrMTG[colorindex][findex-1] + (this->mtg_nbr_of_columns[colorindex][findex-1] * linelocal) + pixelx);
//                 //                else
//                 //                {
//                 //                    long tot = 5568 * (linelocal/2) + (int)(pixelx/2);
//                 //                    long nbr_rows_per_segment = this->mtg_end_position_row[3][index-1] - this->mtg_start_position_row[3][index-1] + 1;
//                 //                    if(nbr_rows_per_segment * 5568 < tot)
//                 //                    {
//                 //                        count_error++;
//                 //                    }
//                 //                }

//                 if(this->histogrammethod == CMB_HISTO_NONE_95)
//                 {
//                     if(pixel[colorindex] != this->fillvalue[colorindex])
//                     {
//                         indexoutpixel[colorindex] = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel[colorindex] - this->minRadianceIndex[colorindex] ) / (float)(this->maxRadianceIndex[colorindex] - this->minRadianceIndex[colorindex])), 0), 1023);
//                         valgamma = pow( indexoutpixel[colorindex], gamma) * gammafactor;
//                         if (valgamma > 1023)
//                             valgamma = 1023;

//                         valcontrast = ContrastStretch(valgamma);
//                         pixelout[colorindex] = (valcontrast);
//                     }
//                 }
//                 else if(this->histogrammethod == CMB_HISTO_NONE_100)
//                 {
//                     if(pixel[colorindex] != this->fillvalue[colorindex])
//                     {
//                         indexoutpixel[colorindex] = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel[colorindex] - this->stat_min[colorindex] ) / (float)(this->stat_max[colorindex] - this->stat_min[colorindex])), 0), 1023);
//                         valgamma = pow( indexoutpixel[colorindex], gamma) * gammafactor;
//                         if (valgamma > 1023)
//                             valgamma = 1023;

//                         valcontrast = ContrastStretch(valgamma);
//                         pixelout[colorindex] = (valcontrast);
//                     }
//                 }
//                 else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
//                 {
//                     if( pixel[colorindex] != this->fillvalue[colorindex])
//                     {
//                         quint16 val = (quint16)qMin(qMax(qRound(4095.0 * (float)(pixel[colorindex] - this->stat_min[colorindex] ) / (float)(this->stat_max[colorindex] - this->stat_min[colorindex])), 0), 1023);
//                         indexoutpixel[colorindex] = (quint16)(qMin(qMax((int)this->lut_mtg[colorindex][val], 0), 4095));
//                         valgamma = pow( indexoutpixel[colorindex]/4, gamma) * gammafactor;
//                         if (valgamma > 1023)
//                             valgamma = 1023;

//                         valcontrast = ContrastStretch(valgamma);
//                         pixelout[colorindex] = (valcontrast);
//                     }
//                 }
//             }

//             //            ret = pixconv.pixcoord2geocoord(sub_lon, opts.geosatellites[geoindex].imagewidthhrv0 - 1 - pixelx,
//             //                                            opts.geosatellites[geoindex].imageheighthrv0 - 1 - line, coff, loff, cfac, lfac, &latitude, &longitude);


//             if(this->reader->spectrum.at(3).length() > 0)
//             {
//                 ret = pixconv.pixcoord2geocoord(sub_lon, pixelx, 11136 - 1 - line, coff, loff, cfac, lfac, &latitude, &longitude);
//                 pixelout[3] = this->ptrimageGeoNight[5568 * (line/2) + (int)(pixelx/2)];
//             }

//             if(this->reader->daykindofimage == "VIS_IR")
//             {
//                 if(pixel[0] != this->fillvalue[0])
//                 {
//                     pixelout[0] = quint16(this->reader->inverse[0] ? (255 - pixelout[0]) : pixelout[0]);
//                     pixelout[1] = pixelout[0];
//                     pixelout[2] = pixelout[0];
//                     row_col[pixelx] = qRgb(pixelout[0], pixelout[1], pixelout[2]);
//                 }
//                 else
//                 {
//                     row_col[pixelx] = qRgb(0, 0, 0);
//                 }
//             }
//             else if(this->reader->daykindofimage == "VIS_IR Color")
//             {
//                 if(this->reader->spectrum.at(3).length() > 0)
//                 {
//                     if(pixel[0] != this->fillvalue[0])
//                     {
//                         if(ret > -1)
//                         {
//                             observer.SetLocation(latitude, longitude, 0.0);
//                             dat.Set(year, month, day, hours, minutes, 0, true);
//                             QSun::Calculate_Solar_Position(dat.Julian(), &solar_vector);
//                             QEci qeci(solar_vector, vel, dat);
//                             qtopo = observer.GetLookAngle(qeci);
//                             elev = qtopo.elevation * 180.0/PIE;


//                             if(elev <= 0.0)
//                             {
//                                 row_col[pixelx] = qRgb(pixelout[3], pixelout[3], pixelout[3]);
//                                 //row_col[pixelx] = qRgb(255, 0, 0);
//                             }
//                             else if(elev > 0.0 && elev < twilight)
//                             {
//                                 int percentday = (int)(100.0 * elev / twilight);
//                                 int percentnight = 100 - percentday;
//                                 int red = (percentday*pixelout[0] + percentnight*pixelout[3])/100;
//                                 red = (red > 255 ? 255 : red);

//                                 int green = (percentday*pixelout[1] + percentnight*pixelout[3])/100;
//                                 green = (green > 255 ? 255 : green);

//                                 int blue = (percentday*pixelout[2] + percentnight*pixelout[3])/100;
//                                 blue = (blue > 255 ? 255 : blue);

//                                 row_col[pixelx] = qRgb(red, green, blue);
//                             }
//                             else
//                             {
//                                 pixelout[0] = quint16(this->reader->inverse[0] ? (255 - pixelout[0]) : pixelout[0]);
//                                 pixelout[1] = quint16(this->reader->inverse[1] ? (255 - pixelout[1]) : pixelout[1]);
//                                 pixelout[2] = quint16(this->reader->inverse[2] ? (255 - pixelout[2]) : pixelout[2]);
//                                 row_col[pixelx] = qRgb(pixelout[0], pixelout[1], pixelout[2]);
//                             }

//                         }
//                     }
//                 }
//                 else
//                 {
//                     if(pixel[0] != this->fillvalue[0])
//                     {
//                         pixelout[0] = quint16(this->reader->inverse[0] ? (255 - pixelout[0]) : pixelout[0]);
//                         pixelout[1] = quint16(this->reader->inverse[1] ? (255 - pixelout[1]) : pixelout[1]);
//                         pixelout[2] = quint16(this->reader->inverse[2] ? (255 - pixelout[2]) : pixelout[2]);
//                         if(pixelout[0] < 256 && pixelout[1] < 256 && pixelout[2] < 256)
//                             row_col[pixelx] = qRgb(pixelout[0], pixelout[1], pixelout[2]);
//                         else
//                             row_col[pixelx] = qRgb(255, 0, 0);
//                     }
//                 }
//             }
//         }

//         linelocal++;
//     }

//     //    qDebug() << QString("====> count_error for index %1 = %2 start_pos_row[0] = %3 end_pos_row[0] = %4 start_pos_row[3] = %5 end_pos_row[3] = %6  tot[0] = %7 tot[3] = %8")
//     //                .arg(index).arg(count_error).arg(this->mtg_start_position_row[0][index - 1]).arg(this->mtg_end_position_row[0][index - 1])
//     //            .arg(this->mtg_start_position_row[3][index - 1]).arg(this->mtg_end_position_row[3][index - 1])
//     //            .arg(this->mtg_end_position_row[0][index - 1] - this->mtg_start_position_row[0][index - 1] + 1)
//     //            .arg(this->mtg_end_position_row[3][index - 1] - this->mtg_start_position_row[3][index - 1] + 1);

// }

void VideoMaker::CalculateImageMTGNight(int findex)
{
    quint16 pixel;
    quint16 indexoutpixel;
    int linelocal = 0;
    quint16 valgamma;
    quint8 pixelout;
    QImage *im;
    QRgb *row_col;

    im = this->ptrimageGeoNight;

    double gamma = this->reader->gamma;
    double gammafactor = 1023 / pow(1023, gamma);
    int countfillvalue = 0;
    int count = 0;

    tot_rows[3] += this->mtg_total_rows_per_segment[3][findex - 1];

    qDebug() << "start VideoMaker::CalculateImageMTGNight(int index) = " << findex;

    if(this->reader->spectrum.at(3).length() > 0)
    {
        qDebug() << "From " << tot_rest_rows[3] - 1 << " to line >= " << this->total_rows[3] - tot_rows[3];
        for(int line = tot_rest_rows[3] - 1; line >= this->total_rows[3] - tot_rows[3]; line--)
        {
            row_col = (QRgb*)im->scanLine(line);

            for (int pixelx = this->mtg_start_position_column[3][findex-1] - 1; pixelx < this->mtg_end_position_column[3][findex-1]; pixelx++)
            {
                count++;
                pixel = *(this->ptrMTG[3][findex-1] + ((this->mtg_nbr_of_columns[3][findex-1]) * linelocal) + pixelx);

                if(pixel != this->fillvalue[3])
                {
                    if(this->histogrammethod == CMB_HISTO_NONE_95)
                    {
                        indexoutpixel = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel - this->minRadianceIndex[3] ) / (float)(this->maxRadianceIndex[3] - this->minRadianceIndex[3])), 0), 1023);
                        valgamma = pow( indexoutpixel, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        pixelout = ContrastStretch(valgamma);
                    }
                    else if(this->histogrammethod == CMB_HISTO_NONE_100)
                    {
                        indexoutpixel = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel - this->stat_min[3] ) / (float)(this->stat_max[3] - this->stat_min[3])), 0), 1023);
                        valgamma = pow( indexoutpixel, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        pixelout = ContrastStretch(valgamma);
                    }
                    else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
                    {
                        quint16 val = (quint16)qMin(qMax(qRound(4095.0 * (float)(pixel - this->stat_min[3] ) / (float)(this->stat_max[3] - this->stat_min[3])), 0), 4095);
                        indexoutpixel = (quint16)(qMin(qMax((int)this->lut_mtg[3][val], 0), 4095));
                        valgamma = pow( indexoutpixel/4, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        pixelout = ContrastStretch(valgamma);
                    }

                    pixelout = quint16(this->reader->inverse[3] ? (255 - pixelout) : pixelout);
                    row_col[pixelx] = qRgb(pixelout, pixelout, pixelout);
                }
                else {
                    countfillvalue++;
                }
            }
            linelocal++;
        }
        tot_rest_rows[3] -= this->mtg_total_rows_per_segment[3][findex - 1];

    }


    //QByteArray rawData((const char*)this->ptrimageGeoNight->constBits(), this->ptrimageGeoNight->sizeInBytes());
    qDebug() << "count countfillvalue = " << countfillvalue << " count = " << count;
}

int VideoMaker::serialMinMaxMTG(const int &findex)
{

    qDebug() << "serialMinMaxMTG for findex = " << findex;

    for(int j = 0; j < (this->reader->daykindofimage == "VIS_IR Color" ? (this->reader->spectrum.at(3).length() > 0 ? 4 : 3) : 1); j++)
    {
        this->CalculateMinMaxMTG(j, findex);
    }


    return(findex);

}

void VideoMaker::CalculateMinMaxMTG(int colorindex, int findex)
{
    qDebug() << QString("CalculateMinMaxMTG colorindex = %1 findex-1 = %2").arg(colorindex).arg(findex-1);

    this->mtg_active_pixels[colorindex][findex-1] = 0;

    this->mtg_stat_min[colorindex][findex-1] = this->fillvalue[colorindex];
    this->mtg_stat_max[colorindex][findex-1] = 0;

    quint16 *ptr = this->ptrMTG[colorindex][findex-1];
    for (int j = 0; j < this->mtg_nbr_of_rows[colorindex][findex-1]; j++) {
        for (int i = 0; i < this->mtg_nbr_of_columns[colorindex][findex-1]; i++)
        {
            quint16 val = ptr[j * this->mtg_nbr_of_columns[colorindex][findex-1] + i];
            if(val != this->fillvalue[colorindex])
            {
                if(val >= this->mtg_stat_max[colorindex][findex-1])
                    this->mtg_stat_max[colorindex][findex-1] = val;
                if(val < this->mtg_stat_min[colorindex][findex-1])
                    this->mtg_stat_min[colorindex][findex-1] = val;

                this->mtg_active_pixels[colorindex][findex-1]++;
            }
        }
    }

    //qDebug() << QString("this->mtg_active_pixels[%1][%2] = %3").arg(colorindex).arg(index).arg(this->mtg_active_pixels[colorindex][index]);

}

// void VideoMaker::CalculateImageMTGConcurrentNight(int index)
// {
//     quint16 pixel;
//     quint16 indexoutpixel;
//     int linelocal = 0;
//     quint16 valgamma;
//     quint8 pixelout;
//     double gamma = this->reader->gamma;
//     double gammafactor = 1023 / pow(1023, gamma);

//     if(this->reader->spectrum.at(3).length() > 0)
//     {
//         for(int line = this->mtg_start_position_row[3][index-1] - 1; line < this->mtg_end_position_row[3][index-1]; line++)
//         {
//             for (int pixelx = this->mtg_start_position_column[3][index-1] - 1; pixelx < this->mtg_end_position_column[3][index-1]; pixelx++)
//             {
//                 pixel = *(this->ptrMTG[3][index-1] + ((this->mtg_nbr_of_columns[3][index-1]) * linelocal) + pixelx);

//                 if(pixel != this->fillvalue[3])
//                 {
//                     if(this->histogrammethod == CMB_HISTO_NONE_95)
//                     {
//                         if(pixel != this->fillvalue[3])
//                         {
//                             indexoutpixel = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel - this->minRadianceIndex[3] ) / (float)(this->maxRadianceIndex[3] - this->minRadianceIndex[3])), 0), 1023);
//                             valgamma = pow( indexoutpixel, gamma) * gammafactor;
//                             if (valgamma > 1023)
//                                 valgamma = 1023;

//                             pixelout = ContrastStretch(valgamma);
//                         }
//                     }
//                     else if(this->histogrammethod == CMB_HISTO_NONE_100)
//                     {
//                         if(pixel != this->fillvalue[3])
//                         {
//                             indexoutpixel = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel - this->stat_min[3] ) / (float)(this->stat_max[3] - this->stat_min[3])), 0), 1023);
//                             valgamma = pow( indexoutpixel, gamma) * gammafactor;
//                             if (valgamma > 1023)
//                                 valgamma = 1023;

//                             pixelout = ContrastStretch(valgamma);
//                         }
//                     }
//                     else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
//                     {
//                         if( pixel != this->fillvalue[3])
//                         {
//                             quint16 val = (quint16)qMin(qMax(qRound(4095.0 * (float)(pixel - this->stat_min[3] ) / (float)(this->stat_max[3] - this->stat_min[3])), 0), 4095);
//                             indexoutpixel = (quint16)(qMin(qMax((int)this->lut_mtg[3][val], 0), 4095));
//                             valgamma = pow( indexoutpixel/4, gamma) * gammafactor;
//                             if (valgamma > 1023)
//                                 valgamma = 1023;

//                             pixelout = ContrastStretch(valgamma);
//                         }
//                     }

//                     pixelout = quint16(this->reader->inverse[3] ? (255 - pixelout) : pixelout);
//                     this->ptrimageGeoNight[line * 5568 + pixelx] = pixelout;
//                 }

//             }
//             linelocal++;
//         }
//     }
// }

void VideoMaker::InitializeImageGeostationary( int imagewidth, int imageheight) // , long stat_min_ch[], long stat_max_ch[] )
{

    if(ptrimageGeostationary != NULL)
        delete ptrimageGeostationary;

    qDebug() << QString("Total nbr of pixels = %1").arg(imagewidth*imageheight);
    qDebug() << QString("width %1  height %2 alphazero = %3").arg(imagewidth).arg(imageheight).arg(alphazero);

    ptrimageGeostationary = new QImage(imagewidth, imageheight, QImage::Format_ARGB32);
    QColor nuts(0,0,0, 255);  //(alphazero == true ? 0 : 255 ));
    ptrimageGeostationary->fill(nuts);

}

int VideoMaker::serialLUTGeoMTG(const int &findex)
{
    for(int j = 0; j < (this->reader->daykindofimage == "VIS_IR Color" ? (this->reader->spectrum.at(3).length() > 0 ? 4 : 3) : 1); j++)
    {
        this->CalculateLUTGeoMTG(j, findex);
    }

    return(findex);
}

void VideoMaker::CalculateLUTGeoMTG(int colorindex, int findex)
{

    qDebug() << "start VideoMaker::CalculateLUTGeoMTG() colorindex = " << colorindex << " findex = " << findex;

    //assert(colorindex == 0);

    quint16 *ptr = this->ptrMTG[colorindex][findex-1];

    for (int j = 0; j < 4096; j++)
    {
        this->mtg_histogram[colorindex][findex-1][j] = 0;
    }

    quint16 pixel;
    for (int line = 0; line < this->mtg_nbr_of_rows[colorindex][findex-1]; line++)
    {
        for (int pixelx = 0; pixelx < this->mtg_nbr_of_columns[colorindex][findex-1]; pixelx++)
        {
            pixel = ptr[line * this->mtg_nbr_of_columns[colorindex][findex-1] + pixelx];
            if(pixel != this->fillvalue[colorindex])
            {
                //quint16 indexout = (quint16)qMin(qMax(qRound(4095.0 * (float)(pixel - this->stat_min[colorindex]) /
                //                      (float)(this->stat_max[colorindex] - this->stat_min[colorindex])), 0), 4095);
                quint16 indexout = qMin(pixel, (quint16)4095);
                this->mtg_histogram[colorindex][findex-1][indexout]++;
            }
        }
    }

    return;

}

void VideoMaker::sendMessages(QString txt)
{

    QByteArray ba = txt.toLocal8Bit();
    this->udpSocket->writeDatagram(ba, QHostAddress::LocalHost, 7755);
    qDebug() << txt;

}

void VideoMaker::checkAvailableSegments(QStringList *segs, QString date)
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

void VideoMaker::replenishSegmentsRss(QStringList *segs, QString datestr)
{
    //QString fpattern = reader->filepattern.replace(46, 12, datestr);

    //H-000-MSG3__-MSG3_????___-??????___-??????___-????????????-?_
    //0         1         2         3         4         5         6
    //0123456789012345678901234567890123456789012345678901234567890
    //H-000-MSG3__-MSG3_RSS____-IR_016___-000006___-202204040740-C_
    //H-000-MSG4__-MSG4________-IR_016___-000001___-201807190700-C_

    // QTime mytime(datestr.mid(8, 2).toInt(), datestr.mid(10, 2).toInt());
    // for(int i = 0; i < this->reader->spectrum.count(); i++)
    // {
    //     if(this->reader->spectrum.at(i).length() > 0)
    //     {
    //         fpattern = fpattern.replace(26, 6, this->reader->spectrum.at(i));
    //         fpattern = fpattern.replace(18, 4, "RSS_");
    //         fpattern = fpattern.replace(36, 6, "000006");
    //         fpattern = fpattern.replace(59, 1, "C");
    //         isSegmentAvailable(fpattern, segs, mytime);
    //         fpattern = fpattern.replace(36, 6, "000007");
    //         isSegmentAvailable(fpattern, segs, mytime);
    //         fpattern = fpattern.replace(36, 6, "000008");
    //         isSegmentAvailable(fpattern, segs, mytime);
    //     }
    // }
}

void VideoMaker::replenishSegmentsFull(QStringList *segs, QString datestr)
{
    // QString fpattern = reader->filepattern.replace(46, 12, datestr);
    // //H-000-MSG3__-MSG3_????___-??????___-??????___-????????????-?_
    // //0         1         2         3         4         5         6
    // //0123456789012345678901234567890123456789012345678901234567890
    // //H-000-MSG3__-MSG3_RSS____-IR_016___-000006___-202204040740-C_
    // //H-000-MSG4__-MSG4________-IR_016___-000001___-201807190700-C_

    // QTime mytime(datestr.mid(8, 2).toInt(), datestr.mid(10, 2).toInt());
    // for(int i = 0; i < this->reader->spectrum.count(); i++)
    // {
    //     if(this->reader->spectrum.at(i).length() > 0)
    //     {
    //         fpattern = fpattern.replace(26, 6, this->reader->spectrum.at(i));
    //         fpattern = fpattern.replace(18, 4, "____");
    //         fpattern = fpattern.replace(36, 6, "000001");
    //         fpattern = fpattern.replace(59, 1, "C");
    //         isSegmentAvailable(fpattern, segs, mytime);
    //         fpattern = fpattern.replace(36, 6, "000002");
    //         isSegmentAvailable(fpattern, segs, mytime);
    //         fpattern = fpattern.replace(36, 6, "000003");
    //         isSegmentAvailable(fpattern, segs, mytime);
    //         fpattern = fpattern.replace(36, 6, "000004");
    //         isSegmentAvailable(fpattern, segs, mytime);
    //         fpattern = fpattern.replace(36, 6, "000005");
    //         isSegmentAvailable(fpattern, segs, mytime);
    //         fpattern = fpattern.replace(36, 6, "000006");
    //         isSegmentAvailable(fpattern, segs, mytime);
    //         fpattern = fpattern.replace(36, 6, "000007");
    //         isSegmentAvailable(fpattern, segs, mytime);
    //         fpattern = fpattern.replace(36, 6, "000008");
    //         isSegmentAvailable(fpattern, segs, mytime);
    //     }
    // }
}

bool VideoMaker::isSegmentAvailable(QString segmentstr, QStringList *segs, QTime time)
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

void VideoMaker::getFilenameParameters(QString filename, QString &filespectrum, QString &filedate, int &filesequence)
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

void VideoMaker::getSegmentSamples(QString filepath, quint16 *ptr, int filesequence, QString typelist)
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

void VideoMaker::ComposeVISIR(quint16 *ptrDayRed, quint16 *ptrDayGreen, quint16 *ptrDayBlue, quint16 *ptrNightRed, QImage &imvisir, QString date, int imagenbr)
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

    if(reader->spectrum.at(3).length() > 0)
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


    nighthistogrammethod = CMB_HISTO_CLAHE; //CMB_HISTO_NONE_95;

    if(nighthistogrammethod == CMB_HISTO_NONE_95 || nighthistogrammethod == CMB_HISTO_EQUALIZE )
    {
        CalculateMinMax(0, width, height, ptrNightRed, 0, stat_min_night, stat_max_night, active_pixels_night);
        CalculateLUTGeo(0, width, height, ptrNightRed, 0, stat_min_night, stat_max_night, active_pixels_night, lut_ch_night, minRadianceIndexNight, maxRadianceIndexNight);
    }

    if(nighthistogrammethod == CMB_HISTO_CLAHE )
    {
        this->CLAHE(ptrNightRed, 3712, reader->brss ? 3*464 : 8*464, 0, 1023, 16, 16, 256, 8.7);
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

    if((reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color"))
    {
        for (int line = 0; line < (reader->brss ? 1392 : 3712); line++)
        {
            row_col = (QRgb*)imvisir.scanLine(line);
            if(reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color")
                row_col_day = (QRgb*)imageday.scanLine(line);
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
    else if((reader->daykindofimage == "VIS_IR" || reader->daykindofimage == "VIS_IR Color"))
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
    else if(reader->daykindofimage == "" && reader->spectrum.at(3).length() > 0)
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

void VideoMaker::ComposeHRV1(quint16 *ptrHRV, quint16 *ptrDayRed, quint16 *ptrDayGreen, quint16 *ptrDayBlue,
                             quint16 *ptrNightRed, QImage &imhrv, QString date,
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


    if(reader->spectrum.at(3).length() > 0)
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

    if(reader->spectrum.at(3).length() > 0)
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
                if(reader->spectrum.at(3).length() > 0)
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

                if(reader->spectrum.at(3).length() > 0)
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
                if(reader->spectrum.at(3).length() > 0)
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

                if(reader->spectrum.at(3).length() > 0)
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

                if(reader->spectrum.at(3).length() > 0)
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

quint16 VideoMaker::ContrastStretch(quint16 val)
{
    double res;
    res = double(val)*A1 + B1;
    return (res > 255.0 ? 255 : quint16(res));
}

void VideoMaker::CalculateMinMax(int colorindex, int width, int height, quint16 *ptr, quint16 fillvalue, quint16 stat_min[], quint16 stat_max[], long active_pixels[])
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

void VideoMaker::CalculateLUTGeo(int colorindex, int width, int height, quint16 *ptr, quint16 fillvalue, quint16 stat_min[], quint16 stat_max[],
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
        //        qDebug() << QString("stats_ch[0][%1] = %2 lut_ch[0][%3] = %4").arg(i).arg(stats_ch[0][i]).arg(i).arg(this->lut_ch[0][i]);
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

// Contrast Limited Adaptive Histogram Equalization
int  VideoMaker::CLAHE (unsigned short* pImage, unsigned int uiXRes, unsigned int uiYRes,
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

void  VideoMaker::ClipHistogram (unsigned long* pulHistogram, unsigned int
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

void  VideoMaker::MakeHistogram (unsigned short* pImage, unsigned int uiXRes,
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

void  VideoMaker::MapHistogram (unsigned long* pulHistogram, unsigned short Min, unsigned short Max,
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

void  VideoMaker::MakeLut (unsigned short * pLUT, unsigned short Min, unsigned short Max, unsigned int uiNrBins)
/* To speed up histogram clipping, the input image [Min,Max] is scaled down to
                 * [0,uiNrBins-1]. This function calculates the LUT.
                 */
{
    int i;
    const unsigned short BinSize = (unsigned short) (1 + (Max - Min) / uiNrBins);

    for (i = Min; i <= Max; i++)  pLUT[i] = (i - Min) / BinSize;
}

void  VideoMaker::Interpolate (unsigned short *pImage, int uiXRes, unsigned long * pulMapLU,
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

void VideoMaker::OverlayGeostationary(QImage *im, bool hrvimage, int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla)
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
            if(reader->gshhsoverlayOnlist.at(k) == true)
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

void VideoMaker::OverlayGeostationaryHRV(QPainter *paint, int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla)
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

    if(reader->gshhsoverlayOnlist.at(0))
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

    if(reader->gshhsoverlayOnlist.at(1))
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

    if(reader->gshhsoverlayOnlist.at(2))
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

void VideoMaker::OverlayDate(QImage *im, QString date)
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

void VideoMaker::getTimeFromIndex(int index, QString *strtime)
{
    QString mydate = reader->selectiondate;
    QString yeardir = mydate.mid(0, 4);
    QString monthdir = mydate.mid(4, 2);
    QString daydir = mydate.mid(6, 2);

    index--;
    int m = index % 6;
    int h = index / 6;
    int hours = h;
    int minutes = m*10;
    *strtime = QString("%1-%2-%3 %4:%5").arg(yeardir).arg(monthdir).arg(daydir).arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0'));
}
