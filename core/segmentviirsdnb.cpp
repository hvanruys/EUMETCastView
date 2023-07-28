#include "segmentviirsdnb.h"
#include "segmentimage.h"

#ifdef _WIN32
#include <hdf5.h>
#else
#include <hdf5.h>
#endif
#include <QDebug>

extern Options opts;
extern SegmentImage *imageptrs;
extern SatelliteList satellitelist;
#include <QMutex>


SegmentVIIRSDNB::SegmentVIIRSDNB(QFile *filesegment, eSegmentType type, QObject *parent) :
    Segment(parent)
{
    bool ok;

    fileInfo.setFile(*filesegment);
    switch (type) {
    case eSegmentType::SEG_VIIRSDNB:
        segment_type = "VIIRSDNB";
        break;
    case eSegmentType::SEG_VIIRSDNBNOAA20:
        segment_type = "VIIRSDNBNOAA20";
        break;
    default:
        break;
    }
    segtype = type;

    //SVDNBC_npp_d20141117_t0837599_e0839241_b15833_c20141117084501709131_eum_ops
    //012345678901234567890
    int sensing_start_year = fileInfo.fileName().mid(12, 4).toInt( &ok , 10);
    int sensing_start_month = fileInfo.fileName().mid(16, 2).toInt( &ok, 10);
    int sensing_start_day = fileInfo.fileName().mid(18, 2).toInt( &ok, 10);
    int sensing_start_hour = fileInfo.fileName().mid(22, 2).toInt( &ok, 10);
    int sensing_start_minute = fileInfo.fileName().mid(24, 2).toInt( &ok, 10);
    int sensing_start_second = fileInfo.fileName().mid(26, 2).toInt( &ok, 10);
    int sensing_start_msecond = fileInfo.fileName().mid(28, 1).toInt( &ok, 10);
    double d_sensing_start_second = (double)sensing_start_second + (double)sensing_start_msecond / 10.0;

    //this->sensing_start_year = sensing_start_year;
    qdatetime_start.setDate(QDate(sensing_start_year, sensing_start_month, sensing_start_day));
    qdatetime_start.setTime(QTime(sensing_start_hour,sensing_start_minute, sensing_start_second,sensing_start_msecond * 100));

    julian_sensing_start = Julian_Date_of_Year(sensing_start_year) +
            DOY( sensing_start_year, sensing_start_month, sensing_start_day ) +
            Fraction_of_Day( sensing_start_hour, sensing_start_minute, d_sensing_start_second )
            + 5.787037e-06; /* Round up to nearest 1 sec */

    julian_sensing_end = julian_sensing_start + Fraction_of_Day( 0, 1, 0);

    qsensingstart = QSgp4Date(sensing_start_year, sensing_start_month, sensing_start_day, sensing_start_hour, sensing_start_minute, d_sensing_start_second);
    qsensingend = qsensingstart;
    qsensingend.AddMin(1.0);

    this->earth_views_per_scanline = 4064;
    this->NbrOfLines = 768;

    Satellite *viirssat;
    if(type == eSegmentType::SEG_VIIRSDNB)
    {
        //viirssat.sat_name = "NPP";
        viirssat = satellitelist.GetSatellite(37849, &ok);
    }
    else if(type == eSegmentType::SEG_VIIRSDNBNOAA20)
    {
        //viirssat.sat_name = "NOAA-20";
        viirssat = satellitelist.GetSatellite(43013, &ok);
    }
    if(!ok)
        qDebug() << "Sat not found!";

    line1 = viirssat->line1;
    line2 = viirssat->line2;

    //line1 = "1 33591U 09005A   11039.40718334  .00000086  00000-0  72163-4 0  8568";
    //line2 = "2 33591  98.8157 341.8086 0013952 344.4168  15.6572 14.11126791103228";
    double epoch = line1.mid(18,14).toDouble(&ok);
    julian_state_vector = Julian_Date_of_Epoch(epoch);

    qtle.reset(new QTle(viirssat->sat_name, line1, line2, QTle::wgs72));
    qsgp4.reset(new QSgp4( *qtle ));


    minutes_since_state_vector = ( julian_sensing_start - julian_state_vector ) * MIN_PER_DAY; //  + (1.0/12.0) / 60.0;
    minutes_sensing = 86.0/60.0;

    QEci qeci;
    qsgp4->getPosition(minutes_since_state_vector, qeci);
    QGeodetic qgeo = qeci.ToGeo();

    lon_start_rad = qgeo.longitude;
    lat_start_rad = qgeo.latitude;

    lon_start_deg = rad2deg(lon_start_rad);
    if (lon_start_deg > 180)
        lon_start_deg = - (360 - rad2deg(lon_start_rad));

    lat_start_deg = rad2deg(lat_start_rad);


    double hours_since_state_vector = ( julian_sensing_start - julian_state_vector ) * HOURS_PER_DAY;

    //qDebug() << QString("---> lon = %1 lat = %2  hours_since_state_vector = %3").arg(lon_start_deg).arg(lat_start_deg).arg( hours_since_state_vector);


    latMax = 0.0;
    lonMax = 0.0;
    latMin = 999.0;
    lonMin = 999.0;
    CalculateCornerPoints();
    invertthissegment[0] = false;
    invertthissegment[1] = false;
    invertthissegment[2] = false;

}

SegmentVIIRSDNB::~SegmentVIIRSDNB()
{
    resetMemory();
}

void SegmentVIIRSDNB::initializeMemory()
{
    qDebug() << "Initializing VIIRSDNB memory";
    if(ptrbaVIIRSDNB.isNull())
    {
        ptrbaVIIRSDNB.reset(new float[earth_views_per_scanline * NbrOfLines]);
        qDebug() << QString("Initializing VIIRSDNB memory earth views = %1 nbr of lines = %2").arg(earth_views_per_scanline).arg(NbrOfLines);
    }
}



Segment *SegmentVIIRSDNB::ReadDatasetsInMemory()
{
    qDebug() << "Segment *SegmentVIIRS::ReadDatasetsInMemory()";

    hid_t   h5_file_id, radiance_id;
    herr_t  h5_status;

    bool tempfileexist;

    QString basename = this->fileInfo.baseName() + ".h5";

    {
        QFile tfile(basename);
        tempfileexist = tfile.exists();
    }

    qDebug() << QString("trying H5Fopen basename = %1 exist = %2").arg(basename).arg(tempfileexist);

    if( (h5_file_id = H5Fopen(basename.toLatin1(), H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
        qDebug() << "File " << basename << " not open !!";
    else
        qDebug() << "File " << basename << " is open !! ------------";

    if((radiance_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-SDR_All/Radiance", H5P_DEFAULT)) < 0)
        qDebug() << "Dataset " << "/All_Data/VIIRS-DNB-SDR_All/Radiance" << " is not open !!";
    else
        qDebug() << "Dataset " << "/All_Data/VIIRS-DNB-SDR_All/Radiance" << " is open !!  ok ok ok ";

    if((h5_status = H5Dread (radiance_id, H5T_NATIVE_USHORT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, ptrbaVIIRSDNB.data())) < 0)
        qDebug() << "Unable to read radiance dataset";


    for(int k = 0; k < 3; k++)
    {
        stat_max_ch[k] = 0;
        stat_min_ch[k] = 9999999;
    }


    for (int j = 0; j < NbrOfLines; j++) {
        for (int i = 0; i < earth_views_per_scanline; i++)
        {
            if(ptrbaVIIRSDNB[j * earth_views_per_scanline + i] >= stat_max_ch[0])
                stat_max_ch[0] = ptrbaVIIRSDNB[j * earth_views_per_scanline + i];
            if(ptrbaVIIRSDNB[j * earth_views_per_scanline + i] < stat_min_ch[0])
                stat_min_ch[0] = ptrbaVIIRSDNB[j * earth_views_per_scanline + i];
        }
    }

    h5_status = H5Dclose (radiance_id);

    h5_status = H5Fclose (h5_file_id);

    return this;

}

int SegmentVIIRSDNB::ReadNbrOfLines()
{
    return NbrOfLines;
}

void SegmentVIIRSDNB::ComposeSegmentImageWindow(float lowerlimit, float upperlimit)
{

    QRgb *row;
    long indexout;

    long countneg = 0;

    float pixval;
    int r;


    //    upperlimit = 0.8E-2;
    //    lowerlimit = 2.0E-4;

    qDebug() << QString("lowerlimit = %1").arg(lowerlimit, 0, 'E', 2);
    qDebug() << QString("upperlimit = %1").arg(upperlimit, 0, 'E', 2);
    qDebug() << QString("NbrOfLines = %1").arg(this->NbrOfLines);

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        row = (QRgb*)imageptrs->ptrimageViirsDNB->scanLine(this->startLineNbr + line);
        for (int pixelx = 0; pixelx < earth_views_per_scanline; pixelx++)
        {
            float zenith = solar_zenith[line * earth_views_per_scanline + pixelx];
            pixval = *(this->ptrbaVIIRSDNB.data() + line * earth_views_per_scanline + pixelx);
            //            if(pixval > 0)
            //            {
            indexout =  (long)(255 * ( pixval - lowerlimit ) / (upperlimit - lowerlimit));
            indexout = indexout > 255 ? 255 : indexout;
            indexout = indexout < 0 ? 0 : indexout;
            r = indexout;
            //                if((zenith >= 95.0 && zenith < 95.01) || (zenith >= 100.0 && zenith < 100.01))
            //                    row[pixelx] = qRgb(0, 255, 0);
            //                else if(zenith >= 90.0 && zenith < 90.01)
            //                    row[pixelx] = qRgb(0, 255, 255);
            //                else if((zenith >= 80.0 && zenith < 80.01) || (zenith >= 85.0 && zenith < 85.01) )
            //                    row[pixelx] = qRgb(255, 0, 0);
            //                else
            row[pixelx] = qRgb(r, r, r );
            //            }
            //            else
            //                row[pixelx] = qRgb(255, 0, 0 );
        }
        if(opts.imageontextureOnVIIRS) // && ((line + 5 ) % 16 == 0 || (line + 10) % 16 == 0) )
        {
            this->RenderSegmentlineInTextureVIIRS( line, row );
            opts.texture_changed = true;
        }

    }

    qDebug() << QString("Count neg = %1").arg(countneg);


}

void SegmentVIIRSDNB::ComposeSegmentImageWindowFromCurve(QVector<double> *x, QVector<double> *y)
{

    QRgb *row;
    long indexout;
    float upperlimit;
    float lowerlimit;
    float radlimit;
    long countneg = 0;

    float pixval;
    int r;

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        row = (QRgb*)imageptrs->ptrimageViirsDNB->scanLine(this->startLineNbr + line);
        for (int pixelx = 0; pixelx < earth_views_per_scanline; pixelx++)
        {
            float zenith = solar_zenith[line * earth_views_per_scanline + pixelx];

            int xzenith = floor(zenith);

            radlimit = getRadianceFromCurve(xzenith, x, y);
            upperlimit = radlimit * 100.0;
            lowerlimit = radlimit / 100.0;


            pixval = *(this->ptrbaVIIRSDNB.data() + line * earth_views_per_scanline + pixelx);
            //            if(pixval > 0)
            //            {
            indexout =  (long)(255 * ( pixval - lowerlimit ) / (upperlimit - lowerlimit));
            indexout = indexout > 255 ? 255 : indexout;
            indexout = indexout < 0 ? 0 : indexout;
            r = indexout;
            if((zenith >= 95.0 && zenith < 95.01) || (zenith >= 100.0 && zenith < 100.01))
                row[pixelx] = qRgb(0, 255, 0);
            else if(zenith >= 90.0 && zenith < 90.01)
                row[pixelx] = qRgb(0, 255, 255);
            else if((zenith >= 80.0 && zenith < 80.01) || (zenith >= 85.0 && zenith < 85.01) )
                row[pixelx] = qRgb(255, 0, 0);
            else
                row[pixelx] = qRgb(r, r, r );
            //            }
            //            else
            //                row[pixelx] = qRgb(255, 0, 0 );
        }
    }

    qDebug() << QString("Count neg = %1").arg(countneg);


}

float SegmentVIIRSDNB::getRadianceFromCurve(int xzenith, QVector<double> *x, QVector<double> *y)
{

    if(x->length() == 0)
        return 0.0;

    for(int i = 0; i < x->length()-1; i++)
    {
        if(xzenith > x->at(i) && xzenith <= x->at(i+1))
            return y->at(i);
    }

    return 0.0;
}

void SegmentVIIRSDNB::CalcGraph(QScopedArrayPointer<long> *graph)
{

    QRgb *row;
    float radiance;

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        row = (QRgb*)imageptrs->ptrimageViirsDNB->scanLine(this->startLineNbr + line);
        for (int pixelx = 0; pixelx < earth_views_per_scanline; pixelx = pixelx + 10)
        {
            int xzenith = (int)solar_zenith[line * earth_views_per_scanline + pixelx];
            radiance = *(this->ptrbaVIIRSDNB.data() + line * earth_views_per_scanline + pixelx);
            if(radiance > 0)
                CalcGraphPockets(xzenith, radiance, graph);
        }
    }

}

void SegmentVIIRSDNB::CalcGraphPockets(int xzenith, float radiance, QScopedArrayPointer<long> *graph)
{

    for(int i = 14; i >= 0; i--)
    {
        double upperlimit1 = pow(10, -i);
        double lowerlimit1 = pow(10, -i - 1);

        if(radiance >= lowerlimit1 && radiance < upperlimit1)
        {
            for(int j = 0; j < 10; j++)
            {
                double lowind = (double)(j + 1)/10.0;
                double upperind = (double)(j)/10.0;
                double lowerlimit2 = pow(10, -lowind - i);
                double upperlimit2 = pow(10, -upperind - i);
                if( radiance >= lowerlimit2 && radiance < upperlimit2)
                {
                    int index = 149 - ((i*10) + j);
                    graph->operator [](index*180+xzenith) = graph->operator [](index*180+xzenith) + 1;
                }
            }
        }
    }
}

void SegmentVIIRSDNB::ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(LCC, histogrammethod, normalized);
}

void SegmentVIIRSDNB::ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(GVP, histogrammethod, normalized);
}

void SegmentVIIRSDNB::ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(SG, histogrammethod, normalized);
}

void SegmentVIIRSDNB::ComposeSegmentOMProjection(int inputchannel, int histogrammethod,  bool normalized)
{
    ComposeProjection(OM, histogrammethod, normalized);
}

void SegmentVIIRSDNB::ComposeProjection(eProjections proj, int histogrammethod, bool normalized)
{

    double map_x, map_y;

    float lonpos1, latpos1;

    float pixval;

    bool valok;

    projectionCoordX.reset(new qint32[NbrOfLines * earth_views_per_scanline]);
    projectionCoordY.reset(new qint32[NbrOfLines * earth_views_per_scanline]);
    projectionCoordValue.reset(new QRgb[NbrOfLines * earth_views_per_scanline]);

    for( int i = 0; i < NbrOfLines; i++)
    {
        for( int j = 0; j < earth_views_per_scanline ; j++ )
        {
            projectionCoordX[i * earth_views_per_scanline + j] = 65535;
            projectionCoordY[i * earth_views_per_scanline + j] = 65535;
            projectionCoordValue[i * earth_views_per_scanline + j] = qRgba(0, 0, 0, 0);
        }
    }
    qDebug() << "SegmentVIIRSDNB::ComposeProjection(eProjections proj)";

    for( int i = 0; i < this->NbrOfLines; i++)
    {
        for( int j = 0; j < this->earth_views_per_scanline ; j++ )
        {
            pixval = ptrbaVIIRSDNB[i * earth_views_per_scanline + j];

            latpos1 = geolatitude[i * earth_views_per_scanline + j];
            lonpos1 = geolongitude[i * earth_views_per_scanline + j];

            if(!(pixval >= -99.0 && pixval <= -92.0 ))
            {

                if(proj == LCC) //Lambert
                {
                    if(imageptrs->lcc->map_forward_neg_coord(lonpos1 * PI / 180.0, latpos1 * PI / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y);
                    }
                }
                else if(proj == GVP) // General Vertical Perspecitve
                {
                    if(imageptrs->gvp->map_forward_neg_coord(lonpos1 * PI / 180.0, latpos1 * PI / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y);
                    }

                }
                else if(proj == SG) // Stereographic
                {
                    if(imageptrs->sg->map_forward_neg_coord(lonpos1 * PI / 180.0, latpos1 * PI / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y);
                    }
                }
                else if(proj == OM) // Oblique Mercator
                {
                    if(imageptrs->om->map_forward(lonpos1 * PI / 180.0, latpos1 * PI / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y);
                    }
                }
            }

        }
    }

}


void SegmentVIIRSDNB::LonLatMax()
{

    lonMin = +180.0;
    lonMax = -180.0;
    latMin = +90.0;
    latMax = -90.0;

    for (int j = 0; j < NbrOfLines; j+=1)
    {
        for (int i = 0; i < earth_views_per_scanline; i+=1)
        {
            if(geolatitude[j * earth_views_per_scanline + i] != -999.0)
            {
                if(geolatitude[j * earth_views_per_scanline + i] > latMax)
                    latMax = geolatitude[j * earth_views_per_scanline + i];
                if(geolatitude[j * earth_views_per_scanline + i] <= latMin)
                    latMin = geolatitude[j * earth_views_per_scanline + i];
            }
            if(geolongitude[j * earth_views_per_scanline + i] != -999.0)
            {
                if(geolongitude[j * earth_views_per_scanline + i] > lonMax)
                    lonMax = geolongitude[j * earth_views_per_scanline + i];
                if(geolongitude[j * earth_views_per_scanline + i] <= lonMin)
                    lonMin = geolongitude[j * earth_views_per_scanline + i];
            }
        }
    }

    qDebug() << QString("Minimum Latitude = %1 ; Maximum Latitude = %2").arg(latMin).arg(latMax);
    qDebug() << QString("Minimum Longitude = %1 ; Maximum Longitude = %2").arg(lonMin).arg(lonMax);


}


void SegmentVIIRSDNB::RenderSegmentlineInTextureVIIRS( int nbrLine, QRgb *row )
{

    QColor rgb;
    int posx, posy;

    QPainter fb_painter(imageptrs->pmOut);

    int devwidth = (fb_painter.device())->width();
    int devheight = (fb_painter.device())->height();

    fb_painter.setPen( Qt::black );
    fb_painter.setBrush( Qt::NoBrush );

    int earthviews = earth_views_per_scanline;

    for (int pix = 0 ; pix < earthviews; pix+=2)
    {
        sphericalToPixel( this->geolongitude[nbrLine * earth_views_per_scanline + pix] * PI/180.0, this->geolatitude[nbrLine * earth_views_per_scanline + pix] * PI/180.0, posx, posy, devwidth, devheight );
        rgb.setRgb(qRed(row[pix]), qGreen(row[pix]), qBlue(row[pix]));
        fb_painter.setPen(rgb);
        fb_painter.drawPoint( posx , posy );
    }

    fb_painter.end();

}


void SegmentVIIRSDNB::MapPixel( int lines, int views, double map_x, double map_y)
{
    int indexout;
    float pixval;
    QRgb rgbvalue = qRgb(0,0,0);

    float lowerlimit = pow(10, opts.dnbsbvalue/20.0)/pow(10, opts.dnbspbwindowsvalue);
    float upperlimit = pow(10, opts.dnbsbvalue/20.0)*pow(10, opts.dnbspbwindowsvalue);

    pixval = ptrbaVIIRSDNB[lines * earth_views_per_scanline + views];


    if (map_x > -15 && map_x < imageptrs->ptrimageProjection->width() + 15 && map_y > -15 && map_y < imageptrs->ptrimageProjection->height() + 15)
    {

        projectionCoordX[lines * earth_views_per_scanline + views] = (qint32)map_x;
        projectionCoordY[lines * earth_views_per_scanline + views] = (qint32)map_y;

        indexout =  (int)(255 * ( pixval - lowerlimit ) / (upperlimit - lowerlimit));
        indexout = indexout > 255 ? 255 : indexout;
        indexout = indexout < 0 ? 0 : indexout;

        rgbvalue = qRgba(indexout, indexout, indexout, 255);

        if(opts.sattrackinimage)
        {
            if(views == 1598 || views == 1599 || views == 1600 || views == 1601 )
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
            projectionCoordValue[lines * earth_views_per_scanline + views] = rgbvalue;
        }

    }
}


// 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 ..............................303            308   310            315
// |                 |     |              |         TPZGroupLocationScanCompact   |              |     |              |
// *--+--+--+--+--*--*--*--*--+--+--+--*--*--+--+.................................*--+--+--+--*--*--*--*--+--+--+--+--*
// |//|//|//|//|//|  |//|  |//|//|//|//|  |//|//|                                 |//|//|//|//|  |//|  |//|//|//|//|//|
// *--+--+--+--+--*--*--*--*--+--+--+--*--*--+--+.................................*--+--+--+--*--*--*--*--+--+--+--+--*
// |              |  |  |  |           |  |                                       |           |  |  |  |              |
//         5          1          4              4   NumberOfTiePointsZonesScan          4          1           5
// |  |  |  |  |     |     |  |  |  |     |                                       |  |  |  |     |     |  |  |  |  |
// 0  1  2  3  4     5     6  7  8  9     10 11     index in align/expans coef                  246   247         251
//
Segment *SegmentVIIRSDNB::ReadSegmentInMemory()
{

    FILE*   f = NULL;
    BZFILE* b;
    int     nBuf;
    char    buf[ 32768 ];
    int     bzerror;
    hid_t   h5_file_id, radiance_id, latitude_id, longitude_id;
    hid_t   lunar_azimuth_id, solar_azimuth_id;
    hid_t   lunar_zenith_id, solar_zenith_id;
    hid_t   aligncoef_id, expanscoef_id;

    hid_t NumberOfTiePointZonesScan_id;
    hid_t NumberOfTiePointZonesTrack_id;
    hid_t NumberOfTiePointZoneGroupsScan_id;
    hid_t NumberOfTiePointZoneGroupsTrack_id;
    hid_t TiePointZoneGroupLocationScanCompact_id;
    hid_t TiePointZoneGroupLocationTrackCompact_id;
    hid_t TiePointZoneGroupLocationScan_id;
    hid_t TiePointZoneGroupLocationTrack_id;
    hid_t TiePointZoneSizeScan_id;
    hid_t TiePointZoneSizeTrack_id;


    hid_t   MoonIllumFraction_id;
    herr_t  h5_status;
    bool tempfileexist;
    int totdim;

    QString basename = this->fileInfo.baseName() + ".h5";
    QFile tfile(basename);
    tempfileexist = tfile.exists();

    qDebug() << QString("file %1  tempfileexist = %2").arg(basename).arg(tempfileexist);
    qDebug() << "Segment *Segment::ReadVIIRSSegmentInMemory(QString strDNB_MOD)";


    if(this->fileInfo.completeSuffix() == "h5.bz2")
    {
        QFile fileout(basename);
        fileout.open(QIODevice::WriteOnly);
        QDataStream streamout(&fileout);


        if((b = BZ2_bzopen(this->fileInfo.absoluteFilePath().toLatin1(),"rb"))==NULL)
        {
            qDebug() << "error in BZ2_bzopen";
        }

        bzerror = BZ_OK;
        while ( bzerror == BZ_OK )
        {
            nBuf = BZ2_bzRead ( &bzerror, b, buf, 32768 );
            if ( bzerror == BZ_OK || bzerror == BZ_STREAM_END)
            {
                streamout.writeRawData(buf, nBuf);
            }
        }

        BZ2_bzclose ( b );

        fileout.close();
    }
    else if(this->fileInfo.completeSuffix() == "h5")
    {
        QFile::copy(this->fileInfo.absoluteFilePath(), basename);
    }


    if( (h5_file_id = H5Fopen(basename.toLatin1(), H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
        qDebug() << "File " << basename << " not open !!";

    if((radiance_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-SDR_All/Radiance", H5P_DEFAULT)) < 0)
        qDebug() << "Dataset " << "/All_Data/VIIRS-DNB-SDR_All/Radiance" << " is not open !!";
    else
        qDebug() << "Dataset " << "/All_Data/VIIRS-DNB-SDR_All/Radiance" << " is open !!  ok ok ok ";

    totdim = GetTotalDimensionspace(radiance_id);

    geolongitude.reset(new float[totdim]);
    geolatitude.reset(new float[totdim]);
    lunar_azimuth.reset(new float[totdim]);
    lunar_zenith.reset(new float[totdim]);

    solar_azimuth.reset(new float[totdim]);
    solar_zenith.reset(new float[totdim]);

    for(int i = 0; i < totdim; i++)
    {
        geolongitude[i] = -999.0;
        geolatitude[i] = -999.0;
        lunar_azimuth[i] = -999.0;
        lunar_zenith[i] = -999.0;

        solar_azimuth[i] = -999.0;
        solar_zenith[i] = -999.0;

    }

    if((h5_status = H5Dread (radiance_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, ptrbaVIIRSDNB.data())) < 0)
        qDebug() << "Unable to read radiance dataset";


    latitude_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/Latitude", H5P_DEFAULT);
    longitude_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/Longitude", H5P_DEFAULT);
    solar_azimuth_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/SolarAzimuthAngle", H5P_DEFAULT);
    solar_zenith_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/SolarZenithAngle", H5P_DEFAULT);
    lunar_azimuth_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/LunarAzimuthAngle", H5P_DEFAULT);
    lunar_zenith_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/LunarZenithAngle", H5P_DEFAULT);

    totdim = GetTotalDimensionspace(latitude_id);
    tiepointxdim = totdim/96;

    tiepoints_lat.reset(new float[totdim]);
    tiepoints_lon.reset(new float[totdim]);
    tiepoints_lunar_azimuth.reset(new float[totdim]);
    tiepoints_lunar_zenith.reset(new float[totdim]);
    tiepoints_solar_azimuth.reset(new float[totdim]);
    tiepoints_solar_zenith.reset(new float[totdim]);


    aligncoef_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/AlignmentCoefficient", H5P_DEFAULT);
    expanscoef_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/ExpansionCoefficient", H5P_DEFAULT);

    totdim = GetTotalDimensionspace(aligncoef_id);

    aligncoef.reset(new float[totdim]);
    expanscoef.reset(new float[totdim]);
    Ptpzscan.reset(new int[totdim]);

    // M-bands
    //        NumberOfTiePointZoneGroupsTrack = 1;
    //        NumberOfTiePointZoneGroupsScan = 1;
    //        TiePointZoneGroupLocationTrackCompact = 0;
    //        TiePointZoneGroupLocationScanCompact = 0;
    //        TiePointZoneGroupLocationTrack = 0;
    //        TiePointZoneGroupLocationScan = 0;

    NumberOfTiePointZonesScan_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/NumberOfTiePointZonesScan", H5P_DEFAULT);
    NumberOfTiePointZonesTrack_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/NumberOfTiePointZonesTrack", H5P_DEFAULT);
    NumberOfTiePointZoneGroupsScan_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/NumberOfTiePointZoneGroupsScan", H5P_DEFAULT);
    NumberOfTiePointZoneGroupsTrack_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/NumberOfTiePointZoneGroupsTrack", H5P_DEFAULT);
    TiePointZoneGroupLocationScanCompact_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/TiePointZoneGroupLocationScanCompact", H5P_DEFAULT);
    TiePointZoneGroupLocationTrackCompact_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/TiePointZoneGroupLocationTrackCompact", H5P_DEFAULT);
    //        TiePointZoneGroupLocationScan_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/TiePointZoneGroupLocationScan", H5P_DEFAULT);
    //        TiePointZoneGroupLocationTrack_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/TiePointZoneGroupLocationTrack", H5P_DEFAULT);

    NumberOfTiePointZonesScan.reset(new int[GetTotalDimensionspace(NumberOfTiePointZonesScan_id)]);
    NumberOfTiePointZonesTrack.reset(new int[GetTotalDimensionspace(NumberOfTiePointZonesTrack_id)]);
    NumberOfTiePointZoneGroupsScan.reset(new int[GetTotalDimensionspace(NumberOfTiePointZoneGroupsScan_id)]);
    NumberOfTiePointZoneGroupsTrack.reset(new int[GetTotalDimensionspace(NumberOfTiePointZoneGroupsTrack_id)]);
    TiePointZoneGroupLocationScanCompact.reset(new int[GetTotalDimensionspace(TiePointZoneGroupLocationScanCompact_id)]);
    TiePointZoneGroupLocationTrackCompact.reset(new int[GetTotalDimensionspace(TiePointZoneGroupLocationTrackCompact_id)]);
    //TiePointZoneGroupLocationScan.reset(new int[GetTotalDimensionspace(TiePointZoneGroupLocationScan_id)]);
    //TiePointZoneGroupLocationTrack.reset(new int[GetTotalDimensionspace(TiePointZoneGroupLocationTrack_id)]);


    MoonIllumFraction_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-DNB-GEO_All/MoonIllumFraction", H5P_DEFAULT);
    if((h5_status = H5Dread (MoonIllumFraction_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, &MoonIllumFraction)) < 0)
        fprintf(stderr, "unable to read MoonIllumFraction dataset");

    if((h5_status = H5Dread (NumberOfTiePointZonesScan_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, NumberOfTiePointZonesScan.data())) < 0)
        fprintf(stderr, "unable to read NumberOfTiePointZonesScan dataset");
    if((h5_status = H5Dread (NumberOfTiePointZonesTrack_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, NumberOfTiePointZonesTrack.data())) < 0)
        fprintf(stderr, "unable to read NumberOfTiePointZonesTrack dataset");
    if((h5_status = H5Dread (NumberOfTiePointZoneGroupsScan_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, NumberOfTiePointZoneGroupsScan.data())) < 0)
        fprintf(stderr, "unable to read NumberOfTiePointZoneGroupsScan dataset");
    if((h5_status = H5Dread (NumberOfTiePointZoneGroupsTrack_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, NumberOfTiePointZoneGroupsTrack.data())) < 0)
        fprintf(stderr, "unable to read NumberOfTiePointZoneGroupsTrack dataset");

    if((h5_status = H5Dread (TiePointZoneGroupLocationScanCompact_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, TiePointZoneGroupLocationScanCompact.data())) < 0)
        fprintf(stderr, "unable to read TiePointZoneGroupLocationScanCompact dataset");
    if((h5_status = H5Dread (TiePointZoneGroupLocationTrackCompact_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, TiePointZoneGroupLocationTrackCompact.data())) < 0)
        fprintf(stderr, "unable to read TiePointZoneGroupLocationTrackCompact dataset");
    //        if((h5_status = H5Dread (TiePointZoneGroupLocationScan_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
    //                                 H5P_DEFAULT, TiePointZoneGroupLocationScan.data())) < 0)
    //            fprintf(stderr, "unable to read TiePointZoneGroupLocationScan dataset");
    //        if((h5_status = H5Dread (TiePointZoneGroupLocationTrack_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
    //                                 H5P_DEFAULT, TiePointZoneGroupLocationTrack.data())) < 0)
    //            fprintf(stderr, "unable to read TiePointZoneGroupLocationTrack dataset");

    TiePointZoneSizeScan.reset(new int[NumberOfTiePointZoneGroupsScan[0]]);
    TiePointZoneGroupLocationScan.reset(new int[NumberOfTiePointZoneGroupsScan[0]]);


    TiePointZoneSizeScan_id = H5Aopen_by_name(h5_file_id, "/All_Data/VIIRS-DNB-SDR_All", "TiePointZoneSizeScan",
                                              H5P_DEFAULT, H5P_DEFAULT);
    if((h5_status = H5Aread(TiePointZoneSizeScan_id, H5T_NATIVE_INT, TiePointZoneSizeScan.data())) < 0)
        fprintf(stderr, "unable to read TiePointZoneSizeScan attribute");

    h5_status=  H5Aclose(TiePointZoneSizeScan_id);
    TiePointZoneGroupLocationScan_id = H5Aopen_by_name(h5_file_id, "/All_Data/VIIRS-DNB-SDR_All", "TiePointZoneGroupLocationScan",
                                                       H5P_DEFAULT, H5P_DEFAULT);
    if((h5_status = H5Aread(TiePointZoneGroupLocationScan_id, H5T_NATIVE_INT, TiePointZoneGroupLocationScan.data())) < 0)
        fprintf(stderr, "unable to read TiePointZoneGroupLocationScan attribute");

    h5_status=  H5Aclose(TiePointZoneGroupLocationScan_id);




    if((h5_status = H5Dread (latitude_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, tiepoints_lat.data())) < 0)
        fprintf(stderr, "unable to read latitude dataset");

    if((h5_status = H5Dread (longitude_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, tiepoints_lon.data())) < 0)
        fprintf(stderr, "unable to read longitude dataset");


    if((h5_status = H5Dread (solar_azimuth_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, tiepoints_solar_azimuth.data())) < 0)
        fprintf(stderr, "unable to read solar azimuth dataset");

    if((h5_status = H5Dread (solar_zenith_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, tiepoints_solar_zenith.data())) < 0)
        fprintf(stderr, "unable to read solar zenith dataset");

    if((h5_status = H5Dread (aligncoef_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, aligncoef.data())) < 0)
        fprintf(stderr, "unable to read AlignmentCoefficient dataset");

    if((h5_status = H5Dread (expanscoef_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, expanscoef.data())) < 0)
        fprintf(stderr, "unable to read ExpansionCoefficient dataset");



    if((h5_status = H5Dread (lunar_azimuth_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, tiepoints_lunar_azimuth.data())) < 0)
        fprintf(stderr, "unable to read lunar azimuth dataset");

    if((h5_status = H5Dread (lunar_zenith_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, tiepoints_lunar_zenith.data())) < 0)
        fprintf(stderr, "unable to read lunar zenith dataset");

    h5_status = H5Dclose (radiance_id);
    h5_status = H5Dclose (latitude_id);
    h5_status = H5Dclose (longitude_id);
    h5_status = H5Dclose (solar_azimuth_id);
    h5_status = H5Dclose (solar_zenith_id);
    h5_status = H5Dclose (aligncoef_id);
    h5_status = H5Dclose (expanscoef_id);

    h5_status = H5Dclose (NumberOfTiePointZonesScan_id);
    h5_status = H5Dclose (NumberOfTiePointZonesTrack_id);
    h5_status = H5Dclose (NumberOfTiePointZoneGroupsScan_id);
    h5_status = H5Dclose (NumberOfTiePointZoneGroupsTrack_id);
    h5_status = H5Dclose (TiePointZoneGroupLocationScanCompact_id);
    h5_status = H5Dclose (TiePointZoneGroupLocationTrackCompact_id);

    h5_status = H5Dclose (lunar_azimuth_id);
    h5_status = H5Dclose (lunar_zenith_id);
    h5_status = H5Dclose (MoonIllumFraction_id);

    h5_status = H5Fclose (h5_file_id);

    int i, j;

    for( j = 0; j < 2; j++)
        s2[j] = (float)((j + 0.5)/2.0);
    for( j = 0; j < 8; j++)
        s8[j] = (float)((j + 0.5)/8.0);
    for( j = 0; j < 14; j++)
        s14[j] = (float)((j + 0.5)/14.0);
    for( j = 0; j < 16; j++)
        s16[j] = (float)((j + 0.5)/16.0);
    for( j = 0; j < 20; j++)
        s20[j] = (float)((j + 0.5)/20.0);
    for( j = 0; j < 22; j++)
        s22[j] = (float)((j + 0.5)/22.0);
    for( j = 0; j < 24; j++)
        s24[j] = (float)((j + 0.5)/24.0);


    /*
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 6; i++)
           std::cout << " " <<  ptrbaVIIRSDNB[j * earth_views_per_scanline + i];
        std::cout << std::endl;
    }
*/
    /*    std::cout  << "tie point latitude : " << std::endl;
    for (j = 0; j < 2; j++) {
        for (i = 100; i < 102; i++)
           std::cout << " " <<  tiepoints_lat[j * 201 + i];
        std::cout << std::endl;
    }

*/
    int ptpzscan = 0;

    int from = 0;
    int count = 0;
    for(int i = 0; i < NumberOfTiePointZoneGroupsScan[0] ; i++)
    {
        int to = NumberOfTiePointZonesScan[i] + from;
        for(int j = from; j < to; j++)
        {
            Ptpzscan[j] = ptpzscan;
            ptpzscan += TiePointZoneSizeScan[i];
            count++;
        }
        from = to;
    }


    for(int itrack = 0; itrack < 48; itrack++)
    {
        int indexfrom = 0;

        if(itrack == 0)
            qDebug() << QString("NumberOfTiePointZoneGroupsScan[0] = %1").arg(NumberOfTiePointZoneGroupsScan[0]);

        for(int igroupscan = 0; igroupscan < NumberOfTiePointZoneGroupsScan[0]; igroupscan++)
        {
            if(itrack == 0)
                qDebug() << QString("itrack = %1 igroupscan = %2 indexfrom = %3 ").arg(itrack).arg(igroupscan).arg(indexfrom);
            CalcInterpolationPerGroup(itrack, igroupscan, indexfrom);
            indexfrom += NumberOfTiePointZonesScan[igroupscan];
        }
    }

    float max_zenith = 0;
    float min_zenith = 999.0;

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        for (int pixelx = 0; pixelx < earth_views_per_scanline; pixelx++)
        {
            float zenith = solar_zenith[line * earth_views_per_scanline + pixelx];
            if(zenith > max_zenith)
                max_zenith = zenith;
            if(zenith < min_zenith)
                min_zenith = zenith;
        }
    }

    qDebug() << QString("min_zenith = %1   max_zenith = %2").arg(min_zenith).arg(max_zenith);

    //this->LonLatMax();

    /*    std::cout << "alpha voor iscan = 100 :" << std::endl;
    for (j = 0; j < 16; j++) {
        for (i = 0; i < 16; i++)
           std::cout << " " <<  alpha[j][i];
        std::cout << std::endl;
    }


    std::cout << "geolatitude  :" << std::endl;
    for (j = 0; j < 16; j++) {
        for (i = 1600; i < 1616; i++)
           std::cout << " " <<  geolatitude[j * earth_views_per_scanline + i];
        std::cout << std::endl;
    }

    std::cout << "geolatitude  :" << std::endl;
    for (j = 0; j < NbrOfLines; j+=767) {
        for (i = 0; i < earth_views_per_scanline; i+=3199)
           std::cout << " " <<  geolatitude[j * earth_views_per_scanline + i];
        std::cout << std::endl;
    }
    std::cout << "geolongitude  :" << std::endl;
    for (j = 0; j < NbrOfLines; j+=767) {
        for (i = 0; i < earth_views_per_scanline; i+=3199)
           std::cout << " " <<  geolongitude[j * earth_views_per_scanline + i];
        std::cout << std::endl;
    }

*/

    tiepoints_lat.reset();
    tiepoints_lon.reset();
    tiepoints_lunar_azimuth.reset();
    tiepoints_lunar_zenith.reset();
    tiepoints_solar_azimuth.reset();
    tiepoints_solar_zenith.reset();
    aligncoef.reset();
    expanscoef.reset();
    Ptpzscan.reset();

    NumberOfTiePointZonesScan.reset();
    NumberOfTiePointZonesTrack.reset();
    NumberOfTiePointZoneGroupsScan.reset();
    NumberOfTiePointZoneGroupsTrack.reset();
    TiePointZoneGroupLocationScanCompact.reset();
    TiePointZoneGroupLocationTrackCompact.reset();
    TiePointZoneGroupLocationScan.reset();
    TiePointZoneGroupLocationTrack.reset();

    return this;
}

int SegmentVIIRSDNB::GetTotalDimensionspace(hid_t id)
{
    hid_t id_space = H5Dget_space(id);
    const int ndims = H5Sget_simple_extent_ndims(id_space);
    hsize_t dims[ndims];
    H5Sget_simple_extent_dims(id_space, dims, NULL);
    int totdim = 1;
    for(int i = 0; i < ndims; i++)
        totdim *= dims[i];
    return totdim;

}

void SegmentVIIRSDNB::CalcInterpolationPerGroup(int itrack, int igroupscan, int indexfrom)
{


    int index = indexfrom;
    if(itrack == 0)
        qDebug() << QString("CalcInterpolationPerGroup from %1 to %2").arg(TiePointZoneGroupLocationScanCompact[igroupscan]).arg(TiePointZoneGroupLocationScanCompact[igroupscan] + NumberOfTiePointZonesScan[igroupscan]);
    for(int iscan = TiePointZoneGroupLocationScanCompact[igroupscan];
        iscan < TiePointZoneGroupLocationScanCompact[igroupscan] + NumberOfTiePointZonesScan[igroupscan]; iscan++)
    {
        CalcInterpolationInTPZ(itrack, iscan, index, igroupscan);
        index++;
    }
}




void SegmentVIIRSDNB::CalcInterpolationInTPZ(int itrack, int iscan, int indexfrom, int igroupscan)
{
    int iA, iB, iC, iD;
    int jA, jB, jC, jD;
    float lat_A, lat_B, lat_C, lat_D;
    float lon_A, lon_B, lon_C, lon_D;
    // float val_A, val_B, val_C, val_D;
    //float lunar_azimuth_A, lunar_azimuth_B, lunar_azimuth_C, lunar_azimuth_D;
    //float lunar_zenith_A, lunar_zenith_B, lunar_zenith_C, lunar_zenith_D;
    float solar_azimuth_A, solar_azimuth_B, solar_azimuth_C, solar_azimuth_D;
    float solar_zenith_A, solar_zenith_B, solar_zenith_C, solar_zenith_D;

    iA = 2 * itrack;
    jA = iscan;
    iB = 2 * itrack;
    jB = iscan + 1;
    iC = 2 * itrack + 1;
    jC = iscan + 1;
    iD = 2 * itrack + 1;
    jD = iscan;



    lat_A = tiepoints_lat[iA * tiepointxdim + jA];
    lat_B = tiepoints_lat[iB * tiepointxdim + jB];
    lat_C = tiepoints_lat[iC * tiepointxdim + jC];
    lat_D = tiepoints_lat[iD * tiepointxdim + jD];

    lon_A = tiepoints_lon[iA * tiepointxdim + jA];
    lon_B = tiepoints_lon[iB * tiepointxdim + jB];
    lon_C = tiepoints_lon[iC * tiepointxdim + jC];
    lon_D = tiepoints_lon[iD * tiepointxdim + jD];

    if((lat_A >= -999.9 && lat_A <= -999.2) ||
            (lat_B >= -999.9 && lat_A <= -999.2) ||
            (lat_C >= -999.9 && lat_A <= -999.2) ||
            (lat_D >= -999.9 && lat_A <= -999.2) ||
            (lon_A >= -999.9 && lon_A <= -999.2) ||
            (lon_B >= -999.9 && lon_B <= -999.2) ||
            (lon_C >= -999.9 && lon_C <= -999.2) ||
            (lon_D >= -999.9 && lon_D <= -999.2))
        return;

    if(itrack == 0 && igroupscan == 0)
        qDebug() << QString("--- itrack = %1 iscan = %2 A(%3, %4) B(%5, %6) C(%7, %8) D(%9, %10) Lat tiepoint A = %11 B = %12 C = %13 D = %14").arg(itrack).arg(iscan).arg(jA).arg(iA).arg(jB).arg(iB)
                    .arg(jC).arg(iC).arg(jD).arg(iD).arg(lat_A).arg(lat_B).arg(lat_C).arg(lat_D);


    //    lunar_azimuth_A = tiepoints_lunar_azimuth[iA * tiepointxdim + jA];
    //    lunar_azimuth_B = tiepoints_lunar_azimuth[iB * tiepointxdim + jB];
    //    lunar_azimuth_C = tiepoints_lunar_azimuth[iC * tiepointxdim + jC];
    //    lunar_azimuth_D = tiepoints_lunar_azimuth[iD * tiepointxdim + jD];

    //    lunar_zenith_A = tiepoints_lunar_zenith[iA * tiepointxdim + jA];
    //    lunar_zenith_B = tiepoints_lunar_zenith[iB * tiepointxdim + jB];
    //    lunar_zenith_C = tiepoints_lunar_zenith[iC * tiepointxdim + jC];
    //    lunar_zenith_D = tiepoints_lunar_zenith[iD * tiepointxdim + jD];

    solar_azimuth_A = tiepoints_solar_azimuth[iA * tiepointxdim + jA];
    solar_azimuth_B = tiepoints_solar_azimuth[iB * tiepointxdim + jB];
    solar_azimuth_C = tiepoints_solar_azimuth[iC * tiepointxdim + jC];
    solar_azimuth_D = tiepoints_solar_azimuth[iD * tiepointxdim + jD];

    solar_zenith_A = tiepoints_solar_zenith[iA * tiepointxdim + jA];
    solar_zenith_B = tiepoints_solar_zenith[iB * tiepointxdim + jB];
    solar_zenith_C = tiepoints_solar_zenith[iC * tiepointxdim + jC];
    solar_zenith_D = tiepoints_solar_zenith[iD * tiepointxdim + jD];

    //    val_A = ptrbaVIIRSDNB[((itrack * 16)) * earth_views_per_scanline + (iscan * zscan)];
    //    val_B = ptrbaVIIRSDNB[((itrack * 16)) * earth_views_per_scanline + (iscan * zscan) + zscan - 1];
    //    val_C = ptrbaVIIRSDNB[((itrack * 16) + 15) * earth_views_per_scanline + (iscan * zscan) + zscan - 1];
    //    val_D = ptrbaVIIRSDNB[((itrack * 16) + 15) * earth_views_per_scanline + (iscan * zscan)];

    //    float minval = Minf(val_A, val_B, val_C, val_D);

    //    for(int relt = 0; relt < 16; relt++)
    //    {
    //        for(int rels = 0; rels < zscan; rels++)
    //        {
    //            if(ptrbaVIIRSDNB[((itrack * 16) + relt) * earth_views_per_scanline + (iscan * zscan) + rels] == 0 || ptrbaVIIRSDNB[((itrack * 16) + relt) * earth_views_per_scanline + (iscan * zscan) + rels] >= 65528)
    //            {
    //                geolatitude[((itrack * 16) + relt) * earth_views_per_scanline + (iscan * 16) + rels] = 65535;
    //                geolongitude[((itrack * 16) + relt) * earth_views_per_scanline + (iscan * 16) + rels] = 65535;
    //            }
    //        }
    //    }

    ////    if(itrack == 0)
    ////        qDebug() << QString("itrack = %1 iscan = %2 Lon tiepoint A = %3 B = %4 C = %5 D = %6").arg(itrack).arg(iscan).arg(lon_A).arg(lon_B).arg(lon_C).arg(lon_D);

    float min_lon = Minf(lon_A, lon_B, lon_C, lon_D);
    float max_lon = Maxf(lon_A, lon_B, lon_C, lon_D);


    if (Maxf(abs(lat_A), abs(lat_B), abs(lat_C), abs(lat_D)) > 60.0 || (max_lon - min_lon) > 90.0)
        interpolateLonLatViaVector(itrack, indexfrom, igroupscan, lon_A, lon_B, lon_C, lon_D, lat_A, lat_B, lat_C, lat_D);
    else
        interpolateLonLatDirect(itrack, indexfrom, igroupscan, lon_A, lon_B, lon_C, lon_D, lat_A, lat_B, lat_C, lat_D);

    //    if(itrack == 0)
    //        qDebug() << QString("itrack = %1 iscan = %2 indexfrom = %3 igroupscan = 4").arg(itrack).arg(iscan).arg(indexfrom).arg(igroupscan);

    float min_solar_azimuth = Minf(solar_azimuth_A, solar_azimuth_B, solar_azimuth_C, solar_azimuth_D);
    float max_solar_azimuth = Maxf(solar_azimuth_A, solar_azimuth_B, solar_azimuth_C, solar_azimuth_D);

    if (Minf(solar_zenith_A, solar_zenith_B, solar_zenith_C, solar_zenith_D) < 10.0 || (max_solar_azimuth - min_solar_azimuth) > 5.0 ||
            Maxf(abs(lat_A), abs(lat_B), abs(lat_C), abs(lat_D)) > 80.0 )
        interpolateSolarViaVector(itrack, indexfrom, igroupscan, lon_A, lon_B, lon_C, lon_D, lat_A, lat_B, lat_C, lat_D, solar_zenith_A, solar_zenith_B, solar_zenith_C, solar_zenith_D, solar_azimuth_A, solar_azimuth_B, solar_azimuth_C, solar_azimuth_D);
    else
        interpolateSolarDirect(itrack, indexfrom, igroupscan, solar_zenith_A, solar_zenith_B, solar_zenith_C, solar_zenith_D, solar_azimuth_A, solar_azimuth_B, solar_azimuth_C, solar_azimuth_D);

}

void SegmentVIIRSDNB::interpolateLonLatDirect(int itrack, int indexfrom, int igroupscan, float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D)
{

    float ascan, atrack;
    float lat_1, lat_2, lat;
    float lon_1, lon_2, lon;

    int zscan = TiePointZoneSizeScan[igroupscan];
    int nscan = NumberOfTiePointZonesScan[igroupscan];
    int pscan = TiePointZoneGroupLocationScan[igroupscan]; //  Pscan[igroupscan];
    int ptpzscan = Ptpzscan[indexfrom];

    if(itrack == 0 && igroupscan == 0)
        qDebug() <<  QString("---- ptpzscan = %1").arg(ptpzscan);

    for(int relt = 0; relt < 16; relt++)
    {
        for(int rels = 0; rels < zscan; rels++)
        {
            GetAlpha(ascan, atrack, rels, relt, indexfrom, zscan);
            // 96 x 316 or 332
            // 48 * 16 = 768
            lat_1 = (1 - ascan) * lat_A + ascan * lat_B;
            lat_2 = (1 - ascan) * lat_D + ascan * lat_C;
            lat = (1 - atrack) * lat_1 + atrack * lat_2;

            lon_1 = (1 - ascan) * lon_A + ascan * lon_B;
            lon_2 = (1 - ascan) * lon_D + ascan * lon_C;
            lon = (1 - atrack) * lon_1 + atrack * lon_2;

            //            if(itrack == 0 && igroupscan < 5 && relt == 0)
            //                qDebug() <<  QString("---- ptpzscan + rels = %1 rels = %2 lon = %3 zscan = %4").arg(ptpzscan + rels).arg(rels).arg(lon).arg(zscan);

            geolatitude[((itrack * 16) + relt) * earth_views_per_scanline + ptpzscan + rels] = lat;
            geolongitude[((itrack * 16) + relt) * earth_views_per_scanline + ptpzscan + rels] = lon;
        }
    }


    if( itrack == 0 && igroupscan == 0)
    {
        qDebug() << "Lat A = " << lat_A << " Lat B = " << lat_B << " Lat C = " << lat_C << " Lat D = " << lat_D;
        for (int relt = 0; relt < 16; relt++)
        {
            QString str = "";
            for (int rels = 0; rels < zscan; rels++)
                str.append(QString("%1 ").arg(geolatitude[((itrack * 16) + relt) * earth_views_per_scanline + ptpzscan + rels]));
            qDebug() << str;
        }
    }

}


void SegmentVIIRSDNB::interpolateSolarDirect(int itrack, int indexfrom, int igroupscan, float solar_zenith_A, float solar_zenith_B, float solar_zenith_C, float solar_zenith_D, float solar_azimuth_A, float solar_azimuth_B, float solar_azimuth_C, float solar_azimuth_D)
{

    float ascan, atrack;
    float azimuth_1, azimuth_2, azimuth;
    float zenith_1, zenith_2, zenith;

    int zscan = TiePointZoneSizeScan[igroupscan];
    int ptpzscan = Ptpzscan[indexfrom];


    for(int relt = 0; relt < 16; relt++)
    {
        for(int rels = 0; rels < zscan; rels++)
        {
            GetAlpha(ascan, atrack, rels, relt, indexfrom, zscan);
            // 96 x 316
            // 48 * 16 = 768
            azimuth_1 = (1 - ascan) * solar_azimuth_A + ascan * solar_azimuth_B;
            azimuth_2 = (1 - ascan) * solar_azimuth_D + ascan * solar_azimuth_C;
            azimuth = (1 - atrack) * azimuth_1 + atrack * azimuth_2;

            zenith_1 = (1 - ascan) * solar_zenith_A + ascan * solar_zenith_B;
            zenith_2 = (1 - ascan) * solar_zenith_D + ascan * solar_zenith_C;
            zenith = (1 - atrack) * zenith_1 + atrack * zenith_2;

            solar_azimuth[((itrack * 16) + relt) * earth_views_per_scanline + ptpzscan + rels] = azimuth;
            solar_zenith[((itrack * 16) + relt) * earth_views_per_scanline + ptpzscan + rels] = zenith;
        }
    }

}



void SegmentVIIRSDNB::interpolateLonLatViaVector(int itrack, int indexfrom, int igroupscan, float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D)
{

    float ascan, atrack;

    int zscan = TiePointZoneSizeScan[igroupscan];
    int ptpzscan = Ptpzscan[indexfrom];

    float lat_A_rad = lat_A * PI / 180.0;
    float lon_A_rad = lon_A * PI / 180.0;
    float lat_B_rad = lat_B * PI / 180.0;
    float lon_B_rad = lon_B * PI / 180.0;
    float lat_C_rad = lat_C * PI / 180.0;
    float lon_C_rad = lon_C * PI / 180.0;
    float lat_D_rad = lat_D * PI / 180.0;
    float lon_D_rad = lon_D * PI / 180.0;

    float x_A_unit = cos(lat_A_rad) * cos(lon_A_rad);
    float y_A_unit = cos(lat_A_rad) * sin(lon_A_rad);
    float z_A_unit = sin(lat_A_rad);

    float x_B_unit = cos(lat_B_rad) * cos(lon_B_rad);
    float y_B_unit = cos(lat_B_rad) * sin(lon_B_rad);
    float z_B_unit = sin(lat_B_rad);

    float x_C_unit = cos(lat_C_rad) * cos(lon_C_rad);
    float y_C_unit = cos(lat_C_rad) * sin(lon_C_rad);
    float z_C_unit = sin(lat_C_rad);

    float x_D_unit = cos(lat_D_rad) * cos(lon_D_rad);
    float y_D_unit = cos(lat_D_rad) * sin(lon_D_rad);
    float z_D_unit = sin(lat_D_rad);


    float x1, y1, z1;
    float x2, y2, z2;
    float x, y, z;
    float lon_deg, lat_deg;

    for(int relt = 0; relt < 16; relt++)
    {
        for(int rels = 0; rels < zscan; rels++)
        {
            GetAlpha(ascan, atrack, rels, relt, indexfrom, zscan);
            // 96 x 201

            x1 = (1 - ascan) * x_A_unit + ascan * x_B_unit;
            y1 = (1 - ascan) * y_A_unit + ascan * y_B_unit;
            z1 = (1 - ascan) * z_A_unit + ascan * z_B_unit;

            x2 = (1 - ascan) * x_D_unit + ascan * x_C_unit;
            y2 = (1 - ascan) * y_D_unit + ascan * y_C_unit;
            z2 = (1 - ascan) * z_D_unit + ascan * z_C_unit;

            x = (1 - atrack) * x1 + atrack * x2;
            y = (1 - atrack) * y1 + atrack * y2;
            z = (1 - atrack) * z1 + atrack * z2;

            lon_deg = atan2(y, x) * 180.0/PI;
            lat_deg = atan2(z, sqrt(x * x + y * y)) * 180.0/PI;

            geolatitude[((itrack * 16) + relt) * earth_views_per_scanline + ptpzscan + rels] = lat_deg;
            geolongitude[((itrack * 16) + relt) * earth_views_per_scanline + ptpzscan + rels] = lon_deg;
        }
    }

    //    if( itrack == 0 && igroupscan == 0)
    //    {
    //        std::cout << "geolatitude" << std::endl;
    //        for (int relt = 0; relt < 16; relt++) {
    //            for (int rels = 0; rels < zscan; rels++)
    //               std::cout << " " <<  geolatitude[((itrack * 16) + relt) * earth_views_per_scanline + (iscan * zscan) + rels];
    //            std::cout << std::endl;
    //        }
    //        std::cout << "geolongitude" << std::endl;
    //        for (int relt = 0; relt < 16; relt++) {
    //            for (int rels = 0; rels < zscan; rels++)
    //               std::cout << " " <<  geolongitude[((itrack * 16) + relt) * earth_views_per_scanline + (iscan * zscan) + rels];
    //            std::cout << std::endl;
    //        }
    //    }

}

void SegmentVIIRSDNB::interpolateSolarViaVector(int itrack, int indexfrom, int igroupscan,
                                                float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D,
                                                float solar_zenith_A, float solar_zenith_B, float solar_zenith_C, float solar_zenith_D,
                                                float solar_azimuth_A, float solar_azimuth_B, float solar_azimuth_C, float solar_azimuth_D)
{

    float ascan, atrack;

    int zscan = TiePointZoneSizeScan[igroupscan];
    int ptpzscan = Ptpzscan[indexfrom];

    float solar_azimuth_A_rad = solar_azimuth_A * PI / 180.0;
    float solar_zenith_A_rad = solar_zenith_A * PI / 180.0;
    float solar_azimuth_B_rad = solar_azimuth_B * PI / 180.0;
    float solar_zenith_B_rad = solar_zenith_B * PI / 180.0;
    float solar_azimuth_C_rad = solar_azimuth_C * PI / 180.0;
    float solar_zenith_C_rad = solar_zenith_C * PI / 180.0;
    float solar_azimuth_D_rad = solar_azimuth_D * PI / 180.0;
    float solar_zenith_D_rad = solar_zenith_D * PI / 180.0;

    float x_A_unit = sin(solar_zenith_A_rad) * sin(solar_azimuth_A_rad);
    float y_A_unit = sin(solar_zenith_A_rad) * cos(solar_azimuth_A_rad);
    float z_A_unit = cos(solar_zenith_A_rad);

    float x_B_unit = sin(solar_zenith_B_rad) * sin(solar_azimuth_B_rad);
    float y_B_unit = sin(solar_zenith_B_rad) * cos(solar_azimuth_B_rad);
    float z_B_unit = cos(solar_zenith_B_rad);

    float x_C_unit = sin(solar_zenith_C_rad) * sin(solar_azimuth_C_rad);
    float y_C_unit = sin(solar_zenith_C_rad) * cos(solar_azimuth_C_rad);
    float z_C_unit = cos(solar_zenith_C_rad);

    float x_D_unit = sin(solar_zenith_D_rad) * sin(solar_azimuth_D_rad);
    float y_D_unit = sin(solar_zenith_D_rad) * cos(solar_azimuth_D_rad);
    float z_D_unit = cos(solar_zenith_D_rad);



    float m00_A = -sin(lon_A);
    float m01_A = cos(lon_A);
    float m02_A = 0;
    float m10_A = -sin(lat_A) * cos(lon_A);
    float m11_A = -sin(lat_A) * sin(lon_A);
    float m12_A = cos(lat_A);
    float m20_A = cos(lat_A) * cos(lon_A);
    float m21_A = cos(lat_A) * sin(lon_A);
    float m22_A = sin(lat_A);

    float m00_B = -sin(lon_B);
    float m01_B = cos(lon_B);
    float m02_B = 0;
    float m10_B = -sin(lat_B) * cos(lon_B);
    float m11_B = -sin(lat_B) * sin(lon_B);
    float m12_B = cos(lat_B);
    float m20_B = cos(lat_B) * cos(lon_B);
    float m21_B = cos(lat_B) * sin(lon_B);
    float m22_B = sin(lat_B);

    float m00_C = -sin(lon_C);
    float m01_C = cos(lon_C);
    float m02_C = 0;
    float m10_C = -sin(lat_C) * cos(lon_C);
    float m11_C = -sin(lat_C) * sin(lon_C);
    float m12_C = cos(lat_C);
    float m20_C = cos(lat_C) * cos(lon_C);
    float m21_C = cos(lat_C) * sin(lon_C);
    float m22_C = sin(lat_C);

    float m00_D = -sin(lon_D);
    float m01_D = cos(lon_D);
    float m02_D = 0;
    float m10_D = -sin(lat_D) * cos(lon_D);
    float m11_D = -sin(lat_D) * sin(lon_D);
    float m12_D = cos(lat_D);
    float m20_D = cos(lat_D) * cos(lon_D);
    float m21_D = cos(lat_D) * sin(lon_D);
    float m22_D = sin(lat_D);

    // From PC to EC
    float x_A_ec = m00_A * x_A_unit + m01_A * y_A_unit + m02_A * z_A_unit;
    float y_A_ec = m10_A * x_A_unit + m11_A * y_A_unit + m12_A * z_A_unit;
    float z_A_ec = m20_A * x_A_unit + m21_A * y_A_unit + m22_A * z_A_unit;

    float x_B_ec = m00_B * x_B_unit + m01_B * y_B_unit + m02_B * z_B_unit;
    float y_B_ec = m10_B * x_B_unit + m11_B * y_B_unit + m12_B * z_B_unit;
    float z_B_ec = m20_B * x_B_unit + m21_B * y_B_unit + m22_B * z_B_unit;

    float x_C_ec = m00_C * x_C_unit + m01_C * y_C_unit + m02_C * z_C_unit;
    float y_C_ec = m10_C * x_C_unit + m11_C * y_C_unit + m12_C * z_C_unit;
    float z_C_ec = m20_C * x_C_unit + m21_C * y_C_unit + m22_C * z_C_unit;

    float x_D_ec = m00_D * x_D_unit + m01_D * y_D_unit + m02_D * z_D_unit;
    float y_D_ec = m10_D * x_D_unit + m11_D * y_D_unit + m12_D * z_D_unit;
    float z_D_ec = m20_D * x_D_unit + m21_D * y_D_unit + m22_D * z_D_unit;


    float x1, y1, z1;
    float x2, y2, z2;
    float x_ec, y_ec, z_ec;
    float azimuth_deg, zenith_deg;

    for(int relt = 0; relt < 16; relt++)
    {
        for(int rels = 0; rels < zscan; rels++)
        {
            GetAlpha(ascan, atrack, rels, relt, indexfrom, zscan);
            // 96 x 201

            x1 = (1 - ascan) * x_A_ec + ascan * x_B_ec;
            y1 = (1 - ascan) * y_A_ec + ascan * y_B_ec;
            z1 = (1 - ascan) * z_A_ec + ascan * z_B_ec;

            x2 = (1 - ascan) * x_D_ec + ascan * x_C_ec;
            y2 = (1 - ascan) * y_D_ec + ascan * y_C_ec;
            z2 = (1 - ascan) * z_D_ec + ascan * z_C_ec;

            x_ec = (1 - atrack) * x1 + atrack * x2;
            y_ec = (1 - atrack) * y1 + atrack * y2;
            z_ec = (1 - atrack) * z1 + atrack * z2;

            float lat = geolatitude[((itrack * 16) + relt) * earth_views_per_scanline + ptpzscan + rels];
            float lon = geolongitude[((itrack * 16) + relt) * earth_views_per_scanline + ptpzscan + rels];

            float m00 = -sin(lon);
            float m01 = cos(lon);
            float m02 = 0;
            float m10 = -sin(lat) * cos(lon);
            float m11 = -sin(lat) * sin(lon);
            float m12 = cos(lat);
            float m20 = cos(lat) * cos(lon);
            float m21 = cos(lat) * sin(lon);
            float m22 = sin(lat);

            // from EC to PC
            float x_pc = m00 * x_ec + m10 * y_ec + m20 * z_ec;
            float y_pc = m01 * x_ec + m11 * y_ec + m21 * z_ec;
            float z_pc = m02 * x_ec + m12 * y_ec + m22 * z_ec;

            azimuth_deg = atan2(x_pc, y_pc) * 180.0/PI;
            zenith_deg = (PI/2 - atan2(z_pc, sqrt(x_pc * x_pc + y_pc * y_pc))) * 180.0/PI;

            solar_azimuth[((itrack * 16) + relt) * earth_views_per_scanline + ptpzscan + rels] = azimuth_deg;
            solar_zenith[((itrack * 16) + relt) * earth_views_per_scanline + ptpzscan + rels] = zenith_deg;
        }
    }

    //    if( itrack == 0)
    //    {
    //        std::cout << "solar_azimuth" << std::endl;
    //        for (int relt = 0; relt < 16; relt++) {
    //            for (int rels = 0; rels < zscan; rels++)
    //                std::cout << " " <<  solar_azimuth[((itrack * 16) + relt) * earth_views_per_scanline + rels]; // + (iscan * zscan)];
    //            std::cout << std::endl;
    //        }
    //        std::cout << "solar_zenith" << std::endl;
    //        for (int relt = 0; relt < 16; relt++) {
    //            for (int rels = 0; rels < zscan; rels++)
    //                std::cout << " " <<  solar_zenith[((itrack * 16) + relt) * earth_views_per_scanline + rels]; // + (iscan * zscan)];
    //            std::cout << std::endl;
    //        }
    //    }
}

void SegmentVIIRSDNB::GetAlpha( float &ascan, float &atrack, int rels, int relt, int index, int zscan)
{
    if(zscan == 2)
        ascan = s2[rels] + s2[rels] * (1 - s2[rels]) * expanscoef[index] + s16[relt] * (1 - s16[relt]) * aligncoef[index];
    else if(zscan == 8)
        ascan = s8[rels] + s8[rels] * (1 - s8[rels]) * expanscoef[index] + s16[relt] * (1 - s16[relt]) * aligncoef[index];
    else if(zscan == 14)
        ascan = s14[rels] + s14[rels] * (1 - s14[rels]) * expanscoef[index] + s16[relt] * (1 - s16[relt]) * aligncoef[index];
    else if(zscan == 16)
        ascan = s16[rels] + s16[rels] * (1 - s16[rels]) * expanscoef[index] + s16[relt] * (1 - s16[relt]) * aligncoef[index];
    else if(zscan == 20)
        ascan = s20[rels] + s20[rels] * (1 - s20[rels]) * expanscoef[index] + s16[relt] * (1 - s16[relt]) * aligncoef[index];
    else if(zscan == 22)
        ascan = s22[rels] + s22[rels] * (1 - s22[rels]) * expanscoef[index] + s16[relt] * (1 - s16[relt]) * aligncoef[index];
    else if(zscan == 24)
        ascan = s24[rels] + s24[rels] * (1 - s24[rels]) * expanscoef[index] + s16[relt] * (1 - s16[relt]) * aligncoef[index];
    atrack = s16[relt];
}

void SegmentVIIRSDNB::getCentralCoords(double *startlon, double *startlat, double *endlon, double *endlat, int *startindex, int *endindex)
{
    if(geolatitude.isNull())
    {
        *startlon = 0.0;
        *startlat = 0.0;
        *endlon = 0.0;
        *endlat = 0.0;
        return;
    }

    for(int i = 0; i < this->NbrOfLines; i++)
    {
        *startindex = i;
        *startlon = geolongitude[i * earth_views_per_scanline + (earth_views_per_scanline/2)];
        *startlat = geolatitude[i * earth_views_per_scanline + (earth_views_per_scanline/2)];
        if(abs(*startlon) <= 180.0 && abs(*startlat) <= 90.0)
            break;
    }

    for(int i = this->NbrOfLines - 1; i >= 0; i--)
    {
        *endindex  = i;
        *endlon = geolongitude[i * earth_views_per_scanline + (earth_views_per_scanline/2)];
        *endlat = geolatitude[i * earth_views_per_scanline + (earth_views_per_scanline/2)];
        if(abs(*endlon) <= 180.0 && abs(*endlat) <= 90.0)
            break;
    }

    std::cout << "=================LONGITUDE===========================" << std::endl;
    for(int i = 0; i < this->NbrOfLines;i++)
    {
        std::cout << geolongitude[i * earth_views_per_scanline + (int)(earth_views_per_scanline/2)] << " ";
    }
    std::cout << std::endl;

    std::cout << "=================LATITUDE===========================" << std::endl;
    for(int i = 0; i < this->NbrOfLines;i++)
    {
        std::cout << geolatitude[i * earth_views_per_scanline + (int)(earth_views_per_scanline/2)] << " ";
    }
    std::cout << std::endl;

    qDebug() << "SegmentVIIRSDNB::GetCentralCoords startindex = " << *startindex << " endindex = " << *endindex << " slon = " << *startlon <<
                " slat = " << *startlat << " elon = " << *endlon << " elat = " << *endlat;

}
