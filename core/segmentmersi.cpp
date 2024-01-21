
// File name convention
// FY3D_<startDate>_<startTime>_<endTime>_<orbitNumber>_MERSI_1000M_L1B.HDF (for the scientific data).
// FY3D_<startDate>_<startTime>_<endTime>_<orbitNumber>_MERSI_GEO1K_L1B.HDF (for the geolocation data)
#include "segmentmersi.h"
#include "globals.h"
#include "segmentimage.h"


#include <QDebug>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QString>
#include <QColor>
#include <QPen>
#include <QPixmap>
#include <QDate>
#include <QFileInfo>
#include <QDebug>
#include <QBitArray>
#include <QByteArray>



extern Options opts;
extern SegmentImage *imageptrs;
#include <QMutex>

#define DIMX 410
#define DIMY 80
#define MULT 5


//   y0 ---- Q00-------R2---------------Q01
//            |         |                |
//   y  ----------------P-----------------
//            |         |                |
//            |         |                |
//   y1 -----Q10-------R1---------------Q11
//            |         |                |
//            x0        x                x1

inline float
BilinearInterpolation(float q10, float q00, float q11, float q01, float x0, float x1, float y1, float y0, float x, float y)
{
    float x1x0, y0y1, x1x, y0y, yy1, xx0;
    x1x0 = x1 - x0;
    y0y1 = y0 - y1;
    x1x = x1 - x;
    y0y = y0 - y;
    yy1 = y - y1;
    xx0 = x - x0;
    return 1.0 / (x1x0 * y0y1) * (
                q10 * x1x * y0y +
                q11 * xx0 * y0y +
                q00 * x1x * yy1 +
                q01 * xx0 * yy1
                );
}


SegmentMERSI::SegmentMERSI(QFileInfo fileinfo, QObject *parent) :
    Segment(parent)
{
    bool ok;
    this->fileInfo = fileinfo;

    this->satname = fileInfo.baseName().mid(0, 4);

    segment_type = "MERSI";
    segtype = eSegmentType::SEG_MERSI;
    this->earth_views_per_scanline = 2048;
    this->NbrOfLines = 400;

    //012345678901234567890123456789012345678901234567890
    //FY3D_20200113_113000_113100_11206_MERSI_1000M_L1B.HDF
    //FY3D_20200113_113000_113100_11206_MERSI_GEO1K_L1B.HDF

    int sensing_start_year = fileInfo.fileName().mid(5, 4).toInt( &ok , 10);
    int sensing_start_month = fileInfo.fileName().mid(9, 2).toInt( &ok, 10);
    int sensing_start_day = fileInfo.fileName().mid(11, 2).toInt( &ok, 10);
    int sensing_start_hour = fileInfo.fileName().mid(14, 2).toInt( &ok, 10);
    int sensing_start_minute = fileInfo.fileName().mid(16, 2).toInt( &ok, 10);
    int sensing_start_second = fileInfo.fileName().mid(18, 2).toInt( &ok, 10);

    qdatetime_start.setDate(QDate(sensing_start_year, sensing_start_month, sensing_start_day));
    qdatetime_start.setTime(QTime(sensing_start_hour,sensing_start_minute, sensing_start_second));

    qsensingstart = QSgp4Date(sensing_start_year, sensing_start_month, sensing_start_day, sensing_start_hour, sensing_start_minute, sensing_start_second);
    qsensingend = qsensingstart;
    qsensingend.AddMin(3.0);

    julian_sensing_start = qsensingstart.Julian();
    julian_sensing_end = qsensingend.Julian();

    Satellite *fy3d_sat;

    if(fileInfo.fileName().mid(0,4) == "FY3D")
        fy3d_sat = satellitelist.GetSatellite(43010, &ok);

    if(!ok)
    {
        qInfo() << "EUMETCastView needs TLE's";
        return;
    }

    line1 = fy3d_sat->line1;
    line2 = fy3d_sat->line2;

    qtle.reset(new QTle(fileInfo.fileName().mid(0,4), line1, line2, QTle::wgs72));
    qsgp4.reset(new QSgp4( *qtle ));

    julian_state_vector = qtle->Epoch();

    minutes_since_state_vector = ( julian_sensing_start - julian_state_vector ) * MINUTES_PER_DAY;
    minutes_sensing = 1;

    QEci eci;

    qsgp4->getPosition(minutes_since_state_vector, eci);
    QGeodetic geo = eci.ToGeo();

    lon_start_rad = geo.longitude;
    lat_start_rad = geo.latitude;
    lon_start_deg = lon_start_rad * 180.0 / PIE;
    lat_start_deg = lat_start_rad * 180.0 /PIE;



    CalculateCornerPoints();
}

void SegmentMERSI::initializeMemory()
{
    qDebug() << "Initializing MERSI memory";
    if(ptrbaMERSI.isNull())
    {
        ptrbaMERSI.reset(new quint16[15 * earth_views_per_scanline * NbrOfLines]);
        qDebug() << QString("Initializing MERSI memory earth views = %1 nbr of lines = %2").arg(earth_views_per_scanline).arg(NbrOfLines);
    }
}

void SegmentMERSI::ComposeSegmentImage(int bandindex, int colorarrayindex[], bool invertarrayindex[], int histogrammethod, bool normalized, int totallines)
{

    QRgb *row;

    qDebug() << QString("SegmentMERSI::ComposeSegmentImage() segm->startLineNbr = %1").arg(this->startLineNbr);
    qDebug() << QString("SegmentMERSI::ComposeSegmentImage() color = %1 ").arg(bandlist.at(0));
    qDebug() << QString("SegmentMERSI::ComposeSegmentImage() colorarrayindex[0] = %1").arg(colorarrayindex[0]);
    qDebug() << QString("SegmentMERSI::ComposeSegmentImage() colorarrayindex[1] = %1").arg(colorarrayindex[1]);
    qDebug() << QString("SegmentMERSI::ComposeSegmentImage() colorarrayindex[2] = %1").arg(colorarrayindex[2]);

    quint16 pixval[3];
    quint16 pixval1024[3];
    quint16 indexout[3];

    int rgbcolor[3];

    bool iscolor = bandlist.at(0);
    bool valok[3];
    int oneblock = 400 * 2048;

    for(int i = 0; i < 3; i++)
    {
        this->colorarrayindex[i] = colorarrayindex[i];
        this->invertarrayindex[i] = invertarrayindex[i];
    }
    this->bandindex = bandindex;

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        row = (QRgb*)imageptrs->ptrimageMERSI->scanLine(totallines - 1 - this->startLineNbr - line);
        for (int pixelx = 0; pixelx < earth_views_per_scanline; pixelx++)
        {
            if(iscolor)
            {
                pixval[0] = *(this->ptrbaMERSI.data() + this->colorarrayindex[0] * oneblock + line * earth_views_per_scanline + earth_views_per_scanline - 1 - pixelx);
                pixval[1] = *(this->ptrbaMERSI.data() + this->colorarrayindex[1] * oneblock + line * earth_views_per_scanline + earth_views_per_scanline - 1 - pixelx);
                pixval[2] = *(this->ptrbaMERSI.data() + this->colorarrayindex[2] * oneblock + line * earth_views_per_scanline + earth_views_per_scanline - 1 - pixelx);

                valok[0] = pixval[0] < 65528 && pixval[0] > 0;
                valok[1] = pixval[1] < 65528 && pixval[1] > 0;
                valok[2] = pixval[2] < 65528 && pixval[2] > 0;
            }
            else
            {
                pixval[0] = *(this->ptrbaMERSI.data() + (this->bandindex - 1) * oneblock + line * earth_views_per_scanline + earth_views_per_scanline - 1 - pixelx);
                pixval[1] = pixval[0];
                pixval[2] = pixval[0];

                valok[0] = pixval[0] < 65528 && pixval[0] > 0;
                valok[1] = valok[0];
                valok[2] = valok[0];

            }

            if( valok[0] && (iscolor ? valok[1] && valok[2] : true))
            {
                for(int k = 0; k < (iscolor ? 3 : 1); k++)
                {
                    pixval1024[k] =  (quint16)qMin(qMax(qRound(1023.0 * (float)(pixval[k] - imageptrs->stat_min_ch[k] ) / (float)(imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k])), 0), 1023);

                    if(histogrammethod == CMB_HISTO_NONE_95) // 95%
                    {
                        indexout[k] =  (quint16)qMin(qMax(qRound(1023.0 * (float)(pixval1024[k] - imageptrs->minRadianceIndex[k] ) / (float)(imageptrs->maxRadianceIndex[k] - imageptrs->minRadianceIndex[k])), 0), 1023);
                    }
                    else if(histogrammethod == CMB_HISTO_NONE_100) // 100%
                    {
                        indexout[k] =  pixval1024[k];
                    }

                    if(invertarrayindex[k])
                    {
                        rgbcolor[k] = 255 - imageptrs->lut_ch[k][indexout[k]]/4;
                    }
                    else
                    {
                        if(histogrammethod == CMB_HISTO_NONE_95 || histogrammethod == CMB_HISTO_NONE_100)
                        {
                            rgbcolor[k] = (quint16)qMin(qMax(qRound((float)indexout[k]/4), 0), 255);
                        }
                        else if(histogrammethod == CMB_HISTO_EQUALIZE)
                        {
                            rgbcolor[k] = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[k][pixval1024[k]]/4), 0), 255);
                        }
                    }
                }

                row[pixelx] = qRgba(rgbcolor[0], iscolor ? rgbcolor[1] : rgbcolor[0], iscolor ? rgbcolor[2] : rgbcolor[0], 255 );
            }
            else
            {
                if(pixval[0] >= 65528 && pixval[1] >= 65528 && pixval[2] >= 65528)
                    row[pixelx] = qRgba(0, 0, 150, 250);
                else if(pixval[0] == 0 || pixval[1] == 0 || pixval[2] == 0)
                    row[pixelx] = qRgba(150, 0, 0, 250);
                else
                {
                    row[pixelx] = qRgba(0, 150, 0, 250);
                }
            }

        }

        if(opts.imageontextureOnMERSI) // && ((line + 5 ) % 16 == 0 || (line + 10) % 16 == 0) )
        {
            this->RenderSegmentlineInTextureMERSI( line, row );
            opts.texture_changed = true;
        }

    }

}

//void SegmentMERSI::ComposeSegmentImage(int bandindex, int colorarrayindex[], bool invertarrayindex[], int histogrammethod, bool normalized, int totallines)
//{

//    QRgb *row;

//    qDebug() << QString("SegmentMERSI::ComposeSegmentImage() segm->startLineNbr = %1").arg(this->startLineNbr);
//    qDebug() << QString("SegmentMERSI::ComposeSegmentImage() color = %1 ").arg(bandlist.at(0));
//    qDebug() << QString("SegmentMERSI::ComposeSegmentImage() colorarrayindex[0] = %1").arg(colorarrayindex[0]);
//    qDebug() << QString("SegmentMERSI::ComposeSegmentImage() colorarrayindex[1] = %1").arg(colorarrayindex[1]);
//    qDebug() << QString("SegmentMERSI::ComposeSegmentImage() colorarrayindex[2] = %1").arg(colorarrayindex[2]);

//    quint16 pixval[3];
//    quint16 pixval256[3];
//    int r, g, b;

//    bool color = bandlist.at(0);
//    bool valok[3];
//    int oneblock = 400 * 2048;

//    for(int i = 0; i < 3; i++)
//    {
//        this->colorarrayindex[i] = colorarrayindex[i];
//        this->invertarrayindex[i] = invertarrayindex[i];
//    }
//    this->bandindex = bandindex;

//    for (int line = 0; line < this->NbrOfLines; line++)
//    {
//        row = (QRgb*)imageptrs->ptrimageMERSI->scanLine(totallines - 1 - this->startLineNbr - line);
//        for (int pixelx = 0; pixelx < earth_views_per_scanline; pixelx++)
//        {
//            if(color)
//            {
//                pixval[0] = *(this->ptrbaMERSI.data() + this->colorarrayindex[0] * oneblock + line * earth_views_per_scanline + earth_views_per_scanline - 1 - pixelx);
//                pixval[1] = *(this->ptrbaMERSI.data() + this->colorarrayindex[1] * oneblock + line * earth_views_per_scanline + earth_views_per_scanline - 1 - pixelx);
//                pixval[2] = *(this->ptrbaMERSI.data() + this->colorarrayindex[2] * oneblock + line * earth_views_per_scanline + earth_views_per_scanline - 1 - pixelx);

//                valok[0] = pixval[0] < 65528 && pixval[0] > 0;
//                valok[1] = pixval[1] < 65528 && pixval[1] > 0;
//                valok[2] = pixval[2] < 65528 && pixval[2] > 0;
//            }
//            else
//            {
//                pixval[0] = *(this->ptrbaMERSI.data() + (this->bandindex - 1) * oneblock + line * earth_views_per_scanline + earth_views_per_scanline - 1 - pixelx);
//                pixval[1] = pixval[0];
//                pixval[2] = pixval[0];

//                valok[0] = pixval[0] < 65528 && pixval[0] > 0;
//                valok[1] = valok[0];
//                valok[2] = valok[0];

//            }

//            if( valok[0] && (color ? valok[1] && valok[2] : true))
//            {
//                for(int i = 0; i < (color ? 3 : 1); i++)
//                {
//                    pixval256[i] =  (quint16)qMin(qMax(qRound(255.0 * (float)(pixval[i] - imageptrs->stat_min_ch[i] ) / (float)(imageptrs->stat_max_ch[i] - imageptrs->stat_min_ch[i])), 0), 255);

////                    indexout[i] =  (int)(255 * ( pixval[i] - imageptrs->stat_min_ch[i] ) / (imageptrs->stat_max_ch[i] - imageptrs->stat_min_ch[i]));
////                    indexout[i] = ( indexout[i] > 255 ? 255 : indexout[i] );
//                }

//                if(color)
//                {
//                    //                    if(invertarrayindex[0])
//                    //                    {
//                    //                        r = 255 - imageptrs->lut_ch[0][indexout[0]];
//                    //                    }
//                    //                    else
//                    //                        r = imageptrs->lut_ch[0][indexout[0]];
//                    //                    if(invertarrayindex[1])
//                    //                    {
//                    //                        g = 255 - imageptrs->lut_ch[1][indexout[1]];
//                    //                    }
//                    //                    else
//                    //                        g = imageptrs->lut_ch[1][indexout[1]];
//                    //                    if(invertarrayindex[2])
//                    //                    {
//                    //                        b = 255 - imageptrs->lut_ch[2][indexout[2]];
//                    //                    }
//                    //                    else
//                    //                        b = imageptrs->lut_ch[2][indexout[2]];
//                    if(invertarrayindex[0])
//                    {
//                        r = 255 - indexout[0];
//                    }
//                    else
//                        r = indexout[0];
//                    if(invertarrayindex[1])
//                    {
//                        g = 255 - indexout[1];
//                    }
//                    else
//                        g = indexout[1];
//                    if(invertarrayindex[2])
//                    {
//                        b = 255 - indexout[2];
//                    }
//                    else
//                        b = indexout[2];


//                    row[pixelx] = qRgb(r, g, b );
//                }
//                else
//                {
//                    //                    if(invertarrayindex[0])
//                    //                    {
//                    //                        r = 255 - imageptrs->lut_ch[0][indexout[0]];
//                    //                    }
//                    //                    else
//                    //                        r = imageptrs->lut_ch[0][indexout[0]];

//                    if(invertarrayindex[0])
//                    {
//                        r = 255 - indexout[0];
//                    }
//                    else
//                        r = indexout[0];

//                    row[pixelx] = qRgb(r, r, r );
//                }

//            }
//            else
//            {
//                if(pixval[0] >= 65528 && pixval[1] >= 65528 && pixval[2] >= 65528)
//                    row[pixelx] = qRgba(0, 0, 150, 250);
//                else if(pixval[0] == 0 || pixval[1] == 0 || pixval[2] == 0)
//                    row[pixelx] = qRgba(150, 0, 0, 250);
//                else
//                {
//                    row[pixelx] = qRgba(0, 150, 0, 250);
//                }
//            }

//        }

//        if(opts.imageontextureOnMERSI) // && ((line + 5 ) % 16 == 0 || (line + 10) % 16 == 0) )
//        {
//            this->RenderSegmentlineInTextureMERSI( line, row );
//            opts.texture_changed = true;
//        }

//    }

//}

void SegmentMERSI::RenderSegmentlineInTextureMERSI( int nbrLine, QRgb *row )
{

    QColor rgb;
    int posx, posy;

    int oneblock = 400 * 2048;

    QPainter fb_painter(imageptrs->pmOut);

    int devwidth = (fb_painter.device())->width();
    int devheight = (fb_painter.device())->height();

    fb_painter.setPen( Qt::black );
    fb_painter.setBrush( Qt::NoBrush );

    int earthviews = earth_views_per_scanline;

    int pixval[3];
    bool valok[3];
    bool color = bandlist.at(0);


    for (int pix = 0 ; pix < earthviews; pix+=2)
    {
        pixval[0] = *(this->ptrbaMERSI.data() + this->colorarrayindex[0] * oneblock + nbrLine * earth_views_per_scanline + pix);

        valok[0] = pixval[0] < 65528 && pixval[0] > 0;
        if(color)
        {
            pixval[1] = *(this->ptrbaMERSI.data() + this->colorarrayindex[1] * oneblock + nbrLine * earth_views_per_scanline + pix);
            pixval[2] = *(this->ptrbaMERSI.data() + this->colorarrayindex[2] * oneblock + nbrLine * earth_views_per_scanline + pix);

            valok[1] = pixval[1] < 65528 && pixval[1] > 0;
            valok[2] = pixval[2] < 65528 && pixval[2] > 0;
        }


        if( valok[0] && (color ? valok[1] && valok[2] : true))
        {
            sphericalToPixel( this->geolongitude[nbrLine * earth_views_per_scanline + earth_views_per_scanline - 1 - pix] * PIE/180.0, this->geolatitude[nbrLine * earth_views_per_scanline + earth_views_per_scanline - 1 - pix] * PIE/180.0, posx, posy, devwidth, devheight );
            rgb.setRgb(qRed(row[pix]), qGreen(row[pix]), qBlue(row[pix]));
            fb_painter.setPen(rgb);
            fb_painter.drawPoint( posx , posy );
        }
    }

    fb_painter.end();

}

Segment *SegmentMERSI::ReadSegmentInMemory(int bandindex, int colorarrayindex[])
{
    QScopedArrayPointer<float> ptrbaLongitude;
    QScopedArrayPointer<float> ptrbaLatitude;

    int geoindex;

    qDebug() << "*SegmentMERSI::ReadSegmentInMemory() for " << fileInfo.filePath();
    qDebug() << "bandindex = " << bandindex << " colorarrayindex[0] = " << colorarrayindex[0]<< " colorarrayindex[1] = " << colorarrayindex[1]<< " colorarrayindex[2] = " << colorarrayindex[2];
    hid_t   h5_file_id, h5_filegeo_id, h5_Longitude_id, h5_Latitude_id;
    herr_t  h5_status;

    if( (h5_file_id = H5Fopen(fileInfo.filePath().toLatin1(), H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
        qDebug() << "File " << fileInfo.filePath().toLatin1() << " not open !!";


    if(fileInfo.fileName().indexOf("1000M") == 40)
        strgeofile = fileInfo.fileName().replace(40, 5, "GEO1K");
    else if(fileInfo.fileName().indexOf("1000M") == 39)
        strgeofile = fileInfo.fileName().replace(39, 5, "GEO1K");
    else
        qDebug() << "Wrong filename = " << fileInfo.fileName();

    QString strgeofilepath = fileInfo.path() + "/" + strgeofile;
    QFile geofile(strgeofilepath);
    if (geofile.exists())
    {
        if( (h5_filegeo_id = H5Fopen( strgeofilepath.toLatin1(), H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
            qDebug() << "File " << strgeofilepath << " not open !!";
        else
            qDebug() << "File " << strgeofilepath << " found !!";
    }
    else
    {
        qDebug() << "File " << strgeofilepath << " not found !!";
        h5_filegeo_id = -1;
    }

    if(h5_filegeo_id < 0)
    {
        ptrbaLongitude.reset(new float[80 * 410]);
        if( (h5_Longitude_id = H5Dopen2(h5_file_id, "/Geolocation/Longitude", H5P_DEFAULT)) < 0)
            qDebug() << "/Geolocation/Longitude does not exist";
        else
            qDebug() << "Dataset '" << "/Geolocation/Longitude" << "' is open";
        if((h5_status = H5Dread (h5_Longitude_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                                 H5P_DEFAULT, ptrbaLongitude.data())) < 0)
            qDebug() << "Unable to read Longitude dataset";


        ptrbaLatitude.reset(new float[80 * 410]);
        if( (h5_Latitude_id = H5Dopen2(h5_file_id, "/Geolocation/Latitude", H5P_DEFAULT)) < 0)
            qDebug() << "/Geolocation/Latitude does not exist";
        else
            qDebug() << "Dataset '" << "/Geolocation/Latitude" << "' is open";
        if((h5_status = H5Dread (h5_Latitude_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                                 H5P_DEFAULT, ptrbaLatitude.data())) < 0)
            qDebug() << "Unable to read Latitude dataset";

        geolatitude.reset(new float[400 * 2050]);
        geolongitude.reset(new float[400  * 2050]);

        // BilinearInterpolation(float q10, float q00, float q11, float q01, float x0, float x1, float y1, float y0, float x, float y)

        for(int i = 0; i < DIMY-1; i++)
        {
            for(int j = 0; j < DIMX-1; j++)
            {
                int index10 = (i+1) * DIMX + j;
                int index11 = (i+1) * DIMX + j + 1;
                int index00 = i * DIMX + j;
                int index01 = i * DIMX + j + 1;
                //            qDebug() << index00 << " " << index01;
                //            qDebug() << index10 << " " << index11;
                //            qDebug() << "--------------------";


                for(int k = 0; k < MULT; k++)
                {
                    for(int l = 0; l < MULT; l++)
                    {
                        //                    qDebug() << "[" << i * MULT + k << ", " << j * MULT + l << "] = " << (DIMX * MULT - 2) * (i * MULT + k) + j * MULT + l;
                        geoindex = (DIMX * MULT - 2) * (i * MULT + k) + j * MULT + l;
                        geolatitude[geoindex] = BilinearInterpolation(ptrbaLatitude[index10], ptrbaLatitude[index00], ptrbaLatitude[index11], ptrbaLatitude[index01],
                                                                      (float)(j * MULT), (float)((j+1) * MULT), (float)((i+1) * MULT), (float)(i * MULT),
                                                                      (float)(j * MULT + l), (float)(i * MULT + k));
                        geolongitude[geoindex] = BilinearInterpolation(ptrbaLongitude[index10], ptrbaLongitude[index00], ptrbaLongitude[index11], ptrbaLongitude[index01],
                                                                       (float)(j * MULT), (float)((j+1) * MULT), (float)((i+1) * MULT), (float)(i * MULT),
                                                                       (float)(j * MULT + l), (float)(i * MULT + k));

                    }
                }
                //            qDebug() << "====================";
            }
        }
        h5_status = H5Dclose (h5_Longitude_id);
        h5_status = H5Dclose (h5_Latitude_id);

    }
    else
    {
        geolatitude.reset(new float[400 * 2048]);
        geolongitude.reset(new float[400  * 2048]);
        ReadGeoFile(h5_filegeo_id);

    }

    //    for(int i = 0; i < 5; i++)
    //    {
    //        for(int j = 0; j < 10; j++)
    //        {
    //            qDebug() << "latitude [" << i << " ," << j  << "] = " << geolatitude[i*2050 + j];
    //        }
    //    }

    //    for(int i = 0; i < 5; i++)
    //    {
    //        for(int j = 0; j < 10; j++)
    //        {
    //            qDebug() << "longitude [" << i << " ," << j  << "] = " << geolongitude[i*2050 + j];
    //        }
    //    }

    ReadMERSI_1KM(h5_file_id);
    for(int k = 0; k < 3; k++)
    {
        stat_max_ch[k] = 0;
        stat_min_ch[k] = 9999999;
        active_pixels[k] = 0;
    }

    int oneblock = 400 * 2048;

    if(bandindex == 0) // color
    {
        for(int k = 0; k < 3; k++)
        {
            for (int j = 0; j < NbrOfLines; j++)
            {
                for (int i = 0; i < earth_views_per_scanline; i++)
                {
                    if(ptrbaMERSI[oneblock * colorarrayindex[k] + j * earth_views_per_scanline + i] > 0 && ptrbaMERSI[oneblock * colorarrayindex[k] + j * earth_views_per_scanline + i] < 65528)
                    {
                        if(ptrbaMERSI[oneblock * colorarrayindex[k] + j * earth_views_per_scanline + i] >= stat_max_ch[k])
                            stat_max_ch[k] = ptrbaMERSI[oneblock * colorarrayindex[k] + j * earth_views_per_scanline + i];
                        if(ptrbaMERSI[oneblock * colorarrayindex[k] + j * earth_views_per_scanline + i] < stat_min_ch[k])
                            stat_min_ch[k] = ptrbaMERSI[oneblock * colorarrayindex[k] + j * earth_views_per_scanline + i];
                        active_pixels[k]++;
                    }
                }
            }
        }
    }
    else
    {
        for (int j = 0; j < NbrOfLines; j++)
        {
            for (int i = 0; i < earth_views_per_scanline; i++)
            {
                if(ptrbaMERSI[oneblock * (bandindex-1) + j * earth_views_per_scanline + i] > 0 && ptrbaMERSI[oneblock * (bandindex-1) + j * earth_views_per_scanline + i] < 65528)
                {
                    if(ptrbaMERSI[oneblock * (bandindex-1) + j * earth_views_per_scanline + i] >= stat_max_ch[0])
                        stat_max_ch[0] = ptrbaMERSI[oneblock * (bandindex-1) + j * earth_views_per_scanline + i];
                    if(ptrbaMERSI[oneblock * (bandindex-1) + j * earth_views_per_scanline + i] < stat_min_ch[0])
                        stat_min_ch[0] = ptrbaMERSI[oneblock * (bandindex-1) + j * earth_views_per_scanline + i];
                    active_pixels[0]++;
                }
            }
        }

    }

    qDebug() << QString("ptrbaMERSI min_ch[0] = %1 max_ch[0] = %2").arg(stat_min_ch[0]).arg(stat_max_ch[0]);
    if(this->bandlist.at(0))
    {
        qDebug() << QString("ptrbaMERSI min_ch[1] = %1 max_ch[1] = %2").arg(stat_min_ch[1]).arg(stat_max_ch[1]);
        qDebug() << QString("ptrbaMERSI min_ch[2] = %1 max_ch[2] = %2").arg(stat_min_ch[2]).arg(stat_max_ch[2]);

    }

    //    this->cornerpointfirst1 = QGeodetic(geolatitude[0]*PI/180.0, geolongitude[0]*PI/180.0, 0 );
    //    this->cornerpointlast1 = QGeodetic(geolatitude[3199]*PI/180.0, geolongitude[3199]*PI/180.0, 0 );
    //    this->cornerpointfirst2 = QGeodetic(geolatitude[767*earth_views_per_scanline]*PI/180.0, geolongitude[767*earth_views_per_scanline]*PI/180.0, 0 );
    //    this->cornerpointlast2 = QGeodetic(geolatitude[767*earth_views_per_scanline + 3199]*PI/180.0, geolongitude[767*earth_views_per_scanline + 3199]*PI/180.0, 0 );
    //    this->cornerpointcenter1 = QGeodetic(geolatitude[1600]*PI/180.0, geolongitude[1600]*PI/180.0, 0);
    //    this->cornerpointcenter2 = QGeodetic(geolatitude[767*earth_views_per_scanline + 1600]*PI/180.0, geolongitude[767*earth_views_per_scanline + 1600]*PI/180.0, 0);

    qDebug() << "first1 = (" << geolatitude[0] << "," << geolongitude[0] << ") last1 = (" << geolatitude[2000] << "," << geolongitude[2000] << ")";
    qDebug() << "first2 = (" << geolatitude[390*2050] << "," << geolongitude[390*2050] << ") last2 = (" << geolatitude[390*2050 + 2000] << "," << geolongitude[390*2050 + 2000] << ")";


    h5_status = H5Fclose (h5_file_id);
    return this;
}

void SegmentMERSI::ReadMERSI_1KM(hid_t h5_file_id)
{
    hid_t   h5_EV_1KM_RefSB_id;
    herr_t  h5_status;

    if( (h5_EV_1KM_RefSB_id = H5Dopen2(h5_file_id, "/Data/EV_1KM_RefSB", H5P_DEFAULT)) < 0)
        qDebug() << "/Data/EV_1KM_RefSB does not exist";
    else
        qDebug() << "Dataset '" << "/Data/EV_1KM_RefSB" << "' is open";



    if((h5_status = H5Dread (h5_EV_1KM_RefSB_id, H5T_NATIVE_USHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, ptrbaMERSI.data())) < 0)
        qDebug() << "Unable to read radiance dataset";
    h5_status = H5Dclose (h5_EV_1KM_RefSB_id);


}

void SegmentMERSI::ReadGeoFile(hid_t h5_geofile_id)
{
    qDebug() << "SegmentMERSI::ReadGeoFile(hid_t h5_geofile_id)";

    hid_t   h5_Longitude_id, h5_Latitude_id;
    herr_t  h5_status;

    if( (h5_Longitude_id = H5Dopen2(h5_geofile_id, "/Geolocation/Longitude", H5P_DEFAULT)) < 0)
        qDebug() << "/Geolocation/Longitude does not exist";
    else
        qDebug() << "Dataset '" << "/Geolocation/Longitude" << "' is open";

    if( (h5_Latitude_id = H5Dopen2(h5_geofile_id, "/Geolocation/Latitude", H5P_DEFAULT)) < 0)
        qDebug() << "/Geolocation/Latitude does not exist";
    else
        qDebug() << "Dataset '" << "/Geolocation/Latitude" << "' is open";

    if((h5_status = H5Dread (h5_Longitude_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, geolongitude.data())) < 0)
        qDebug() << "Unable to read Longitude dataset";
    if((h5_status = H5Dread (h5_Latitude_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, geolatitude.data())) < 0)
        qDebug() << "Unable to read Latitude dataset";

    h5_status = H5Dclose (h5_Longitude_id);
    h5_status = H5Dclose (h5_Latitude_id);


}

SegmentMERSI::~SegmentMERSI()
{
}

void SegmentMERSI::ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(LCC, histogrammethod, normalized);
}

void SegmentMERSI::ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(GVP, histogrammethod, normalized);
}

void SegmentMERSI::ComposeSegmentSGProjection(int inputchannel, int histogrammethod,  bool normalized)
{
    ComposeProjection(SG, histogrammethod, normalized);
}

void SegmentMERSI::ComposeSegmentOMProjection(int inputchannel, int histogrammethod,  bool normalized)
{
    ComposeProjection(OM, histogrammethod, normalized);
}

void SegmentMERSI::ComposeProjection(eProjections proj, int histogrammethod, bool normalized)
{

    double map_x, map_y;

    float lonpos1, latpos1;


    int pixval[3];

    bool color = bandlist.at(0);
    bool valok[3];

    projectionCoordX.reset(new qint32[NbrOfLines * earth_views_per_scanline]);
    projectionCoordY.reset(new qint32[NbrOfLines * earth_views_per_scanline]);
    projectionCoordValue.reset(new QRgb[NbrOfLines * earth_views_per_scanline]);
    projectionCoordValueRed.reset(new quint16[NbrOfLines * earth_views_per_scanline]);
    projectionCoordValueGreen.reset(new quint16[NbrOfLines * earth_views_per_scanline]);
    projectionCoordValueBlue.reset(new quint16[NbrOfLines * earth_views_per_scanline]);

    for( int i = 0; i < NbrOfLines; i++)
    {
        for( int j = 0; j < earth_views_per_scanline; j++ )
        {
            projectionCoordX[i * earth_views_per_scanline + j] = 65535;
            projectionCoordY[i * earth_views_per_scanline + j] = 65535;
            projectionCoordValue[i * earth_views_per_scanline + j] = qRgba(0, 0, 0, 0);
            projectionCoordValueRed[i * earth_views_per_scanline + j] = 0;
            projectionCoordValueGreen[i * earth_views_per_scanline + j] = 0;
            projectionCoordValueBlue[i * earth_views_per_scanline + j] = 0;
        }
    }

    int oneblock = 400 * 2048;

    for( int i = 0; i < this->NbrOfLines; i++)
    {
        for( int j = 0; j < this->earth_views_per_scanline ; j++ )
        {
            pixval[0] = ptrbaMERSI[oneblock * this->colorarrayindex[0] + i * earth_views_per_scanline + j];
            valok[0] = pixval[0] > 0 && pixval[0] < 65528;

            if(color)
            {
                pixval[1] = ptrbaMERSI[oneblock * this->colorarrayindex[1] + i * earth_views_per_scanline + j];
                pixval[2] = ptrbaMERSI[oneblock * this->colorarrayindex[2] + i * earth_views_per_scanline + j];
                valok[1] = pixval[1] > 0 && pixval[1] < 65528;
                valok[2] = pixval[2] > 0 && pixval[2] < 65528;
            }

            if( valok[0] && (color ? valok[1] && valok[2] : true))
            {
                latpos1 = geolatitude[i * earth_views_per_scanline + j];
                lonpos1 = geolongitude[i * earth_views_per_scanline + j];

                if(proj == LCC) //Lambert
                {
                    if(imageptrs->lcc->map_forward_neg_coord(lonpos1 * PIE / 180.0, latpos1 * PIE / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y, color);
                    }
                }
                else if(proj == GVP) // General Vertical Perspecitve
                {
                    if(imageptrs->gvp->map_forward_neg_coord(lonpos1 * PIE / 180.0, latpos1 * PIE / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y, color);
                    }

                }
                else if(proj == SG) // Stereographic
                {
                    if(imageptrs->sg->map_forward_neg_coord(lonpos1 * PIE / 180.0, latpos1 * PIE / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y, color);
                    }
                }
                else if(proj == OM) // Oblique Mercator
                {
                    if(imageptrs->om->map_forward(lonpos1 * PIE / 180.0, latpos1 * PIE / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y, color);
                    }
                }
            } else
            {
                projectionCoordX[i * earth_views_per_scanline + j] = 65535;
                projectionCoordY[i * earth_views_per_scanline + j] = 65535;
                projectionCoordValue[i * earth_views_per_scanline + j] = qRgba(0, 0, 0, 0);
                projectionCoordValueRed[i * earth_views_per_scanline + j] = 0;
                projectionCoordValueGreen[i * earth_views_per_scanline + j] = 0;
                projectionCoordValueBlue[i * earth_views_per_scanline + j] = 0;
            }
        }
    }

}

void SegmentMERSI::MapPixel(int lines, int views, double map_x, double map_y, bool iscolor)
{
    quint16 indexout[3];
    quint16 pixval[3];
    quint16 pixval1024[3];

    int rgbcolor[3];
    bool valok[3];

    QRgb rgbvalue = qRgba(0,0,0,0);

    int histogrammethod = this->histogrammethod;


    int oneblock = 400 * 2048;

    if(iscolor)
    {
        pixval[0] = *(this->ptrbaMERSI.data() + this->colorarrayindex[0] * oneblock + lines * earth_views_per_scanline + views);
        pixval[1] = *(this->ptrbaMERSI.data() + this->colorarrayindex[1] * oneblock + lines * earth_views_per_scanline + views);
        pixval[2] = *(this->ptrbaMERSI.data() + this->colorarrayindex[2] * oneblock + lines * earth_views_per_scanline + views);

        valok[0] = pixval[0] < 65528 && pixval[0] > 0;
        valok[1] = pixval[1] < 65528 && pixval[1] > 0;
        valok[2] = pixval[2] < 65528 && pixval[2] > 0;
    }
    else
    {
        pixval[0] = *(this->ptrbaMERSI.data() + (this->bandindex - 1) * oneblock + lines * earth_views_per_scanline + views);
        pixval[1] = pixval[0];
        pixval[2] = pixval[0];

        valok[0] = pixval[0] < 65528 && pixval[0] > 0;
        valok[1] = valok[0];
        valok[2] = valok[0];

    }


    if (map_x > -15 && map_x < imageptrs->ptrimageProjection->width() + 15 && map_y > -15 && map_y < imageptrs->ptrimageProjection->height() + 15)
    {

        projectionCoordX[lines * earth_views_per_scanline + views] = (qint32)map_x;
        projectionCoordY[lines * earth_views_per_scanline + views] = (qint32)map_y;

        if( valok[0] && (iscolor ? valok[1] && valok[2] : true))
        {
            for(int k = 0; k < (iscolor ? 3 : 1); k++)
            {
                pixval1024[k] =  (quint16)qMin(qMax(qRound(1023.0 * (float)(pixval[k] - imageptrs->stat_min_ch[k] ) / (float)(imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k])), 0), 1024);

                if(histogrammethod == CMB_HISTO_NONE_95) // 95%
                {
                    indexout[k] =  (quint16)qMin(qMax(qRound(1023.0 * (float)(pixval1024[k] - imageptrs->minRadianceIndex[k] ) / (float)(imageptrs->maxRadianceIndex[k] - imageptrs->minRadianceIndex[k])), 0), 1023);
                }
                else if(histogrammethod == CMB_HISTO_NONE_100) // 100%
                {
                    indexout[k] =  pixval1024[k];
                }

                if(invertarrayindex[k])
                {
                    rgbcolor[k] = 255 - imageptrs->lut_ch[k][indexout[k]]/4;
                }
                else
                {
                    if(histogrammethod == CMB_HISTO_NONE_95 || histogrammethod == CMB_HISTO_NONE_100)
                    {
                        rgbcolor[k] = (quint16)qMin(qMax(qRound((float)indexout[k]/4), 0), 255);
                    }
                    else if(histogrammethod == CMB_HISTO_EQUALIZE)
                    {
                        rgbcolor[k] = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[k][pixval1024[k]]/4), 0), 255);
                    }
                }
            }


            //        rgbvalue = qRgba(color8[0], iscolor ? color8[1] : color8[0], iscolor ? color8[2] : color8[0], 255 );
            rgbvalue = qRgba(rgbcolor[0], iscolor ? rgbcolor[1] : rgbcolor[0], iscolor ? rgbcolor[2] : rgbcolor[0], 255 );


            if(opts.sattrackinimage)
            {
                if(views == 1023 || views == 1024 || views == 1025 )
                {
                    rgbvalue = qRgb(250, 0, 0);
                    if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
                        imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                }
                else
                {
                    if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
                        imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                    projectionCoordValue[lines * earth_views_per_scanline + views] = rgbvalue;

                }
            }
            else
            {
                if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
            }

            projectionCoordValue[lines * earth_views_per_scanline + views] = rgbvalue;
            projectionCoordValueRed[lines * earth_views_per_scanline + views] = rgbcolor[0];
            if(iscolor)
            {
                projectionCoordValueGreen[lines * earth_views_per_scanline + views] = rgbcolor[1];
                projectionCoordValueBlue[lines * earth_views_per_scanline + views] = rgbcolor[2];
            }
            else
            {
                projectionCoordValueGreen[lines * earth_views_per_scanline + views] = rgbcolor[0];
                projectionCoordValueBlue[lines * earth_views_per_scanline + views] = rgbcolor[0];
            }
        }
    }
}

//void SegmentMERSI::MapPixel(int lines, int views, double map_x, double map_y, bool iscolor)
//{
//    int indexout[3];
//    quint16 pixval[3];
//    quint16 pixval256[3];
//    quint16 pixval4096[3];

//    int color8[3];
//    int color12[3];
//    QRgb rgbvalue = qRgba(0,0,0,0);

//    int histogrammethod = this->histogrammethod;


//    int oneblock = 400 * 2048;

//    pixval[0] = ptrbaMERSI[oneblock * colorarrayindex[0] + lines * earth_views_per_scanline + views];

//    if(iscolor)
//    {
//        pixval[1] = ptrbaMERSI[oneblock * colorarrayindex[1] + lines * earth_views_per_scanline + views];
//        pixval[2] = ptrbaMERSI[oneblock * colorarrayindex[2] + lines * earth_views_per_scanline + views];
//    }

//    if (map_x > -15 && map_x < imageptrs->ptrimageProjection->width() + 15 && map_y > -15 && map_y < imageptrs->ptrimageProjection->height() + 15)
//    {

//        projectionCoordX[lines * earth_views_per_scanline + views] = (qint32)map_x;
//        projectionCoordY[lines * earth_views_per_scanline + views] = (qint32)map_y;


//        for(int k = 0; k < (iscolor ? 3 : 1); k++)
//        {
//            pixval4096[k] =  (quint16)qMin(qMax(qRound(4095.0 * (float)(pixval[k] - imageptrs->stat_min_ch[k] ) / (float)(imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k])), 0), 4095);
//            pixval256[k] =  (quint16)qMin(qMax(qRound(255.0 * (float)(pixval[k] - imageptrs->stat_min_ch[k] ) / (float)(imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k])), 0), 255);

//            indexout[k] =  pixval256[k];


//            if(invertarrayindex[k])
//            {
//                color12[k] = 4095 - pixval4096[k];
//                color8[k] = 255 - (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[k][pixval256[k]]), 0), 255);
//            }
//            else
//            {
//                color12[k] = pixval4096[k];
//                color8[k] = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[k][pixval256[k]]), 0), 255);
//            }
//        }


//        //        rgbvalue = qRgba(color8[0], iscolor ? color8[1] : color8[0], iscolor ? color8[2] : color8[0], 255 );
//        rgbvalue = qRgba(pixval256[0], iscolor ? pixval256[1] : pixval256[0], iscolor ? pixval256[2] : pixval256[0], 255 );


//        if(opts.sattrackinimage)
//        {
//            if(views == 1023 || views == 1024 || views == 1025 )
//            {
//                rgbvalue = qRgb(250, 0, 0);
//                if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
//                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
//            }
//            else
//            {
//                if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
//                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
//                projectionCoordValue[lines * earth_views_per_scanline + views] = rgbvalue;

//            }
//        }
//        else
//        {
//            if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
//                imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
//        }

//        projectionCoordValue[lines * earth_views_per_scanline + views] = rgbvalue;
//        projectionCoordValueRed[lines * earth_views_per_scanline + views] = color12[0];
//        if(iscolor)
//        {
//            projectionCoordValueGreen[lines * earth_views_per_scanline + views] = color12[1];
//            projectionCoordValueBlue[lines * earth_views_per_scanline + views] = color12[2];
//        }
//        else
//        {
//            projectionCoordValueGreen[lines * earth_views_per_scanline + views] = color12[0];
//            projectionCoordValueBlue[lines * earth_views_per_scanline + views] = color12[0];
//        }
//    }
//}

void SegmentMERSI::GetCentralCoords(double *startlon, double *startlat, double *endlon, double *endlat, int *startindex, int *endindex)
{
    if(geolatitude.isNull())
    {
        *startlon = 0.0;
        *startlat = 0.0;
        *endlon = 0.0;
        *endlat = 0.0;
        return;
    }

    for(int i = 0; i < NbrOfLines; i++)
    {
        *startindex = i;
        *startlon = geolongitude[i * earth_views_per_scanline + (int)(earth_views_per_scanline/2)];
        *startlat = geolatitude[i * earth_views_per_scanline + (int)(earth_views_per_scanline/2)];
        if(*startlon != 65535 && *startlat != 65535)
            break;
    }

    for(int i = NbrOfLines - 1; i >= 0; i--)
    {
        *endindex  = i;
        *endlon = geolongitude[i * earth_views_per_scanline + (int)(earth_views_per_scanline/2)];
        *endlat = geolatitude[i * earth_views_per_scanline + (int)(earth_views_per_scanline/2)];
        if(*endlon != 65535 && *endlat != 65535)
            break;
    }

    //    qDebug() << "SegmentMERSI::GetCentralCoords startindex = " << *startindex << " endindex = " << *endindex << " slon = " << *startlon <<
    //                " slat = " << *startlat << " elon = " << *endlon << " elat = " << *endlat;

}

void SegmentMERSI::GetStartCornerCoords(double *cornerlon1, double *cornerlat1, double *cornerlon2, double *cornerlat2,
                                        int *Xstartindex1, int *Xstartindex2, int *Ystartindex12)
{
    if(geolatitude.isNull())
    {
        *cornerlon1 = 999.0;
        *cornerlat1 = 999.0;
        *cornerlon2 = 999.0;
        *cornerlat2 = 999.0;
        *Xstartindex1 = 0;
        *Xstartindex2 = 0;
        *Ystartindex12 = 0;
        return;
    }

    for(int i = 0; i < NbrOfLines; i++)
    {
        *Ystartindex12 = i;

        for(int j = 0; j < earth_views_per_scanline; j++)
        {
            *Xstartindex1 = j;
            *cornerlon1 = geolongitude[i * earth_views_per_scanline + j];
            *cornerlat1 = geolatitude[i * earth_views_per_scanline + j];
            if(abs(*cornerlon1) < 180.0 && abs(*cornerlat1) < 90.0)
                break;
        }

        for(int j = earth_views_per_scanline - 1; j >= 0; j--)
        {
            *Xstartindex2 = j;
            *cornerlon2 = geolongitude[i * earth_views_per_scanline + j];
            *cornerlat2 = geolatitude[i * earth_views_per_scanline + j];
            if(abs(*cornerlon2) < 180.0 && abs(*cornerlat2) < 90.0)
                break;
        }

        if(abs(*cornerlon1) < 180.0 && abs(*cornerlat1) < 90.0 && abs(*cornerlon2) < 180.0 && abs(*cornerlat2) < 90.0)
            break;
    }

    qDebug() << "SegmentMERSI::GetStartCornerCoords " << " cornerlon1 = " << *cornerlon1 << " cornerlat1 = " << *cornerlat1 << " cornerlon2 = " << *cornerlon2 <<
                " cornerlat2 = " << *cornerlat2 << " Xstartindex1 = " << *Xstartindex1 << " Xstartindex2 = " << *Xstartindex2 << " Ystartindex12 = " << *Ystartindex12;

}
void SegmentMERSI::GetEndCornerCoords(double *cornerlon3, double *cornerlat3, double *cornerlon4, double *cornerlat4,
                                      int *Xstartindex3, int *Xstartindex4, int *Ystartindex34)
{
    if(geolatitude.isNull())
    {
        *cornerlon3 = 999.0;
        *cornerlat3 = 999.0;
        *cornerlon4 = 999.0;
        *cornerlat4 = 999.0;
        *Xstartindex3 = 0;
        *Xstartindex4 = 0;
        *Ystartindex34 = 0;
        return;
    }

    for(int i = 399; i >= 0; i--)
    {
        *Ystartindex34 = i;

        for(int j = 0; j < 2048; j++)
        {
            *Xstartindex3 = j;
            *cornerlon3 = geolongitude[i * earth_views_per_scanline + j];
            *cornerlat3 = geolatitude[i * earth_views_per_scanline + j];
            if(abs(*cornerlon3) < 180.0 && abs(*cornerlat3) < 90.0)
                break;
        }

        for(int j = 2047; j >= 0; j--)
        {
            *Xstartindex4 = j;
            *cornerlon4 = geolongitude[i * earth_views_per_scanline + j];
            *cornerlat4 = geolatitude[i * earth_views_per_scanline + j];
            if(abs(*cornerlon4) < 180.0 && abs(*cornerlat4) < 90.0)
                break;
        }

        if(abs(*cornerlon3) < 180.0 && abs(*cornerlat3) < 90.0 && abs(*cornerlon4) < 180.0 && abs(*cornerlat4) < 90.0)
            break;
    }

    qDebug() << "SegmentMERSI::GetEndCornerCoords " << " cornerlon3 = " << *cornerlon3 << " cornerlat3 = " << *cornerlat3 << " cornerlon4 = " << *cornerlon4 << " cornerlat4 = " << *cornerlat4 <<
                " Xstartindex3 = " << *Xstartindex3 << " Xstartindex4 = " << *Xstartindex4 << " Ystartindex34 = " << *Ystartindex34;

}
