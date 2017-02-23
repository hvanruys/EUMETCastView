#include "segmentslstr.h"
#include "options.h"
#include "segmentimage.h"
#include <QDebug>
#include <netcdf.h>
#include <QMutex>

extern Options opts;
extern SegmentImage *imageptrs;
extern QMutex g_mutex;


SegmentSLSTR::SegmentSLSTR(QFileInfo fileinfo, SatelliteList *satl, QObject *parent) :
  Segment(parent)
{

    bool ok;
    satlist = satl;
    this->fileInfo = fileinfo;
    segment_type = "SLSTR";
    segtype = eSegmentType::SEG_SLSTR;

    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
    //0         1         2         3         4         5         6         7         8         9         10
    //S3A_OL_1_EFR____20161026T161414_20161026T161714_20161026T175243_0179_010_168_4319_MAR_O_NR_002.SEN3.tar
    int sensing_start_year = fileInfo.fileName().mid(16, 4).toInt( &ok , 10);
    int sensing_start_month = fileInfo.fileName().mid(20, 2).toInt( &ok, 10);
    int sensing_start_day = fileInfo.fileName().mid(22, 2).toInt( &ok, 10);
    int sensing_start_hour = fileInfo.fileName().mid(25, 2).toInt( &ok, 10);
    int sensing_start_minute = fileInfo.fileName().mid(27, 2).toInt( &ok, 10);
    int sensing_start_second = fileInfo.fileName().mid(29, 2).toInt( &ok, 10);

    int sensing_end_year = fileInfo.fileName().mid(32, 4).toInt( &ok , 10);
    int sensing_end_month = fileInfo.fileName().mid(36, 2).toInt( &ok, 10);
    int sensing_end_day = fileInfo.fileName().mid(38, 2).toInt( &ok, 10);
    int sensing_end_hour = fileInfo.fileName().mid(41, 2).toInt( &ok, 10);
    int sensing_end_minute = fileInfo.fileName().mid(43, 2).toInt( &ok, 10);
    int sensing_end_second = fileInfo.fileName().mid(45, 2).toInt( &ok, 10);

    double d_sensing_start_second = (double)sensing_start_second;
    double d_sensing_end_second = (double)sensing_end_second;

    //this->sensing_start_year = sensing_start_year;
    qdatetime_start.setDate(QDate(sensing_start_year, sensing_start_month, sensing_start_day));
    qdatetime_start.setTime(QTime(sensing_start_hour,sensing_start_minute, sensing_start_second, 0));

    julian_sensing_start = Julian_Date_of_Year(sensing_start_year) +
            DOY( sensing_start_year, sensing_start_month, sensing_start_day ) +
            Fraction_of_Day( sensing_start_hour, sensing_start_minute, d_sensing_start_second )
            + 5.787037e-06; /* Round up to nearest 1 sec */

    julian_sensing_end = Julian_Date_of_Year(sensing_end_year) +
            DOY( sensing_end_year, sensing_end_month, sensing_end_day ) +
            Fraction_of_Day( sensing_end_hour, sensing_end_minute, d_sensing_end_second )
            + 5.787037e-06; /* Round up to nearest 1 sec */


    qsensingstart = QSgp4Date(sensing_start_year, sensing_start_month, sensing_start_day, sensing_start_hour, sensing_start_minute, d_sensing_start_second);
    qsensingend = QSgp4Date(sensing_end_year, sensing_end_month, sensing_end_day, sensing_end_hour, sensing_end_minute, d_sensing_end_second);


    this->earth_views_per_scanline = 4865;
    this->NbrOfLines = 4091;

    Satellite s3a;
    ok = satlist->GetSatellite(41335, &s3a);
    line1 = s3a.line1;
    line2 = s3a.line2;

    //line1 = "1 33591U 09005A   11039.40718334  .00000086  00000-0  72163-4 0  8568";
    //line2 = "2 33591  98.8157 341.8086 0013952 344.4168  15.6572 14.11126791103228";
    double epoch = line1.mid(18,14).toDouble(&ok);
    julian_state_vector = Julian_Date_of_Epoch(epoch);

    qtle.reset(new QTle(s3a.sat_name, line1, line2, QTle::wgs72));
    qsgp4.reset(new QSgp4( *qtle ));


    minutes_since_state_vector = ( julian_sensing_start - julian_state_vector ) * MIN_PER_DAY; //  + (1.0/12.0) / 60.0;
    minutes_sensing = ( julian_sensing_end - julian_sensing_start ) * MIN_PER_DAY;

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

    // qDebug() << QString("---> lon = %1 lat = %2  hours_since_state_vector = %3").arg(lon_start_deg).arg(lat_start_deg).arg( hours_since_state_vector);

    CalculateCornerPoints();

    invertthissegment[0] = false;
    invertthissegment[1] = false;
    invertthissegment[2] = false;


}

void SegmentSLSTR::initializeMemory()
{
    qDebug() << "Initializing SLSTR memory";

    bool color = this->bandlist.at(0);

    for(int k = 0; k < (color ? 3 : 1); k++)
    {
        //if(ptrbaSLSTR[k].isNull())
        {
            ptrbaSLSTR[k].reset(new qint16[earth_views_per_scanline * NbrOfLines]);
        }
    }
    qDebug() << "End Initializing SLSTR memory";

}

void SegmentSLSTR::ComposeSegmentImage(int histogrammethod)
{

    QRgb *row;
    quint16 indexout[3];
    bool iscolor = bandlist.at(0);

    qDebug() << QString("SegmentSLSTR::ComposeSegmentImage() segm->startLineNbr = %1").arg(this->startLineNbr);
    qDebug() << QString("SegmentSLSTR::ComposeSegmentImage() color = %1 ").arg(bandlist.at(0));
    qDebug() << QString("SegmentSLSTR::ComposeSegmentImage() invertthissegment[0] = %1").arg(invertthissegment[0]);
    qDebug() << QString("SegmentSLSTR::ComposeSegmentImage() invertthissegment[1] = %1").arg(invertthissegment[1]);
    qDebug() << QString("SegmentSLSTR::ComposeSegmentImage() invertthissegment[2] = %1").arg(invertthissegment[2]);

    qDebug() << QString("SegmentSLSTR::ComposeSegmentImage() imageptrs stat_min_ch[0] = %1 stat_max_ch[0] = %2").arg(imageptrs->stat_min_ch[0]).arg(imageptrs->stat_max_ch[0]);
    if(iscolor)
    {
        qDebug() << QString("SegmentSLSTR::ComposeSegmentImage() imageptrs stat_min_ch[1] = %1 stat_max_ch[1] = %2").arg(imageptrs->stat_min_ch[1]).arg(imageptrs->stat_max_ch[1]);
        qDebug() << QString("SegmentSLSTR::ComposeSegmentImage() imageptrs stat_min_ch[2] = %1 stat_max_ch[2] = %2").arg(imageptrs->stat_min_ch[2]).arg(imageptrs->stat_max_ch[2]);
    }


    int color[3];
    qint16 pixval[3];
    qint16 pixval1024[3];

    bool valuesok = false;

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        row = (QRgb*)imageptrs->ptrimageSLSTR->scanLine(this->startLineNbr + line);

        for (int pixelx = 0; pixelx < earth_views_per_scanline; pixelx++)
        {
            valuesok = false;
            pixval[0] = this->ptrbaSLSTR[0][line * earth_views_per_scanline + pixelx];

            if(iscolor)
            {
                pixval[1] = this->ptrbaSLSTR[1][line * earth_views_per_scanline + pixelx];
                pixval[2] = this->ptrbaSLSTR[2][line * earth_views_per_scanline + pixelx];
            }

            if(iscolor)
            {
                if(pixval[0] > -1 && pixval[1] > -1 && pixval[2] < 0 )
                {
                    valuesok = true;
                    pixval[2] = 0;
                }
                else if(pixval[0] > -1 && pixval[1] < 0 && pixval[2] > -1 )
                {
                    valuesok = true;
                    pixval[1] = 0;
                }
                else if(pixval[0] < 0 && pixval[1] > -1 && pixval[2] > -1 )
                {
                    valuesok = true;
                    pixval[0] = 0;
                }
                else if(pixval[0] > -1 && pixval[1] > -1 && pixval[2] > -1 )
                {
                    valuesok = true;
                }
            }
            else
            {
                if(pixval[0] > -1)
                    valuesok = true;
            }



            if( valuesok )
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

                    if(invertthissegment[k])
                    {
                        color[k] = 255 - imageptrs->lut_ch[k][indexout[k]]/4;
                    }
                    else
                    {
                        if(histogrammethod == CMB_HISTO_NONE_95 || histogrammethod == CMB_HISTO_NONE_100)
                        {

                            color[k] = (quint16)qMin(qMax(qRound((float)indexout[k]/4), 0), 255);
                        }
                        else if(histogrammethod == CMB_HISTO_EQUALIZE)
                        {
                            color[k] = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[k][pixval1024[k]]/4), 0), 255);
                        }
                    }
                }

                row[pixelx] = qRgba(color[0], iscolor ? color[1] : color[0], iscolor ? color[2] : color[0], 255 );

            }
//            else
//            {
//                row[pixelx] = qRgba(255, 0, 0, 0);
//            }

        }

        if(opts.imageontextureOnSLSTR && line % 2 == 0)
        {
            this->RenderSegmentlineInTextureSLSTR( line, row );
            opts.texture_changed = true;
        }

    }
}

Segment *SegmentSLSTR::ReadSegmentInMemory()
{
    QString radiancename1;
    QString radiancename2;
    QString radiancename3;
    QByteArray array1;
    QByteArray array2;
    QByteArray array3;
    const char* pfile1;
    const char* pfile2;
    const char* pfile3;
    QString radiancevar1;
    QString radiancevar2;
    QString radiancevar3;
    const char* pvar1;
    const char* pvar2;
    const char* pvar3;
    QString geoname;
    QString geolon;
    QString geolat;

    int retval;
    int ncgeofileid, ncfileid1, ncfileid2, ncfileid3;
    int radianceid1, radianceid2, radianceid3;
    int longitudeid, latitudeid;
    float scale_factor[3], add_offset[3];


    int columnsid, rowsid;
    size_t columnslength, rowslength;

    bool iscolorimage = this->bandlist.at(0);

    qDebug() << " in Segment *SegmentSLSTR::ReadSegmentInMemory()";

    for(int i = 0; i < 3; i++)
        this->active_pixels[i] = 0;

    if(iscolorimage)
    {
        qDebug() << "Starting SLSTR netCDF color";
        getDatasetNameFromColor(slstrview, 0, &radiancename1, &radiancevar1, &geoname, &geolat, &geolon);
        getDatasetNameFromColor(slstrview, 1, &radiancename2, &radiancevar2, &geoname, &geolat, &geolon);
        getDatasetNameFromColor(slstrview, 2, &radiancename3, &radiancevar3, &geoname, &geolat, &geolon);

        qDebug() << "getDatasetNameFromBand fname1 = " << radiancename1 << " var1 = " << radiancevar1 << " geoname = " << geoname << " geolon = " << geolon << " geolat = " << geolat;
        qDebug() << "getDatasetNameFromBand fname2 = " << radiancename2 << " var2 = " << radiancevar2 << " geoname = " << geoname << " geolon = " << geolon << " geolat = " << geolat;
        qDebug() << "getDatasetNameFromBand fname3 = " << radiancename3 << " var3 = " << radiancevar3 << " geoname = " << geoname << " geolon = " << geolon << " geolat = " << geolat;


        array1 = radiancename1.toUtf8();
        array2 = radiancename2.toUtf8();
        array3 = radiancename3.toUtf8();
        pfile1 = array1.constData();
        pfile2 = array2.constData();
        pfile3 = array3.constData();

        retval = nc_open(pfile1, NC_NOWRITE, &ncfileid1);
        if(retval != NC_NOERR) qDebug() << "error opening file1";
        retval = nc_open(pfile2, NC_NOWRITE, &ncfileid2);
        if(retval != NC_NOERR) qDebug() << "error opening file2";
        retval = nc_open(pfile3, NC_NOWRITE, &ncfileid3);
        if(retval != NC_NOERR) qDebug() << "error opening file3";

        retval = nc_inq_dimid(ncfileid1, "columns", &columnsid);
        if(retval != NC_NOERR) qDebug() << "error reading columns id file1";
        retval = nc_inq_dimlen(ncfileid1, columnsid, &columnslength);
        if(retval != NC_NOERR) qDebug() << "error reading columns length file1";

        retval = nc_inq_dimid(ncfileid1, "rows", &rowsid);
        if(retval != NC_NOERR) qDebug() << "error reading rows id file1";
        retval = nc_inq_dimlen(ncfileid1, rowsid, &rowslength);
        if(retval != NC_NOERR) qDebug() << "error reading rows length file1";

        this->earth_views_per_scanline = columnslength;
        this->NbrOfLines = rowslength;
        this->longitude.reset(new int[columnslength * rowslength]);
        this->latitude.reset(new int[columnslength * rowslength]);

        this->initializeMemory();

        array1 = radiancevar1.toUtf8();
        array2 = radiancevar2.toUtf8();
        array3 = radiancevar3.toUtf8();
        pvar1 = array1.constData();
        pvar2 = array2.constData();
        pvar3 = array3.constData();

        retval = nc_inq_varid(ncfileid1, pvar1, &radianceid1);
        if (retval != NC_NOERR) qDebug() << "error reading radiance1 id";
        retval = nc_get_var_short(ncfileid1, radianceid1, ptrbaSLSTR[0].data());
        if (retval != NC_NOERR) qDebug() << "error reading radiance1 values";

        retval = nc_inq_varid(ncfileid2, pvar2, &radianceid2);
        if (retval != NC_NOERR) qDebug() << "error reading radiance2 id";
        retval = nc_get_var_short(ncfileid2, radianceid2, ptrbaSLSTR[1].data());
        if (retval != NC_NOERR) qDebug() << "error reading radiance2 values";

        retval = nc_inq_varid(ncfileid3, pvar3, &radianceid3);
        if (retval != NC_NOERR) qDebug() << "error reading radiance3 id";
        retval = nc_get_var_short(ncfileid3, radianceid3, ptrbaSLSTR[2].data());
        if (retval != NC_NOERR) qDebug() << "error reading radiance3 values";

        retval = nc_get_att_float(ncfileid1, radianceid1, "scale_factor", &scale_factor[0]);
        if (retval != NC_NOERR) qDebug() << "error reading scale_factor[0]";

        retval = nc_get_att_float(ncfileid2, radianceid2, "scale_factor", &scale_factor[1]);
        if (retval != NC_NOERR) qDebug() << "error reading scale_factor[1]";

        retval = nc_get_att_float(ncfileid3, radianceid3, "scale_factor", &scale_factor[2]);
        if (retval != NC_NOERR) qDebug() << "error reading scale_factor[2]";

        retval = nc_get_att_float(ncfileid1, radianceid1, "add_offset", &add_offset[0]);
        if (retval != NC_NOERR) qDebug() << "error reading add_offset[0]";

        retval = nc_get_att_float(ncfileid2, radianceid2, "add_offset", &add_offset[1]);
        if (retval != NC_NOERR) qDebug() << "error reading add_offset[1]";

        retval = nc_get_att_float(ncfileid3, radianceid3, "add_offset", &add_offset[2]);
        if (retval != NC_NOERR) qDebug() << "error reading add_offset[2]";


        for(int i = 0; i < 3; i++)
            qDebug() << QString("scale_factor %1 = %2").arg(i).arg(scale_factor[i]);

        retval = nc_close(ncfileid1);
        if (retval != NC_NOERR) qDebug() << "error closing file1";

        retval = nc_close(ncfileid2);
        if (retval != NC_NOERR) qDebug() << "error closing file2";

        retval = nc_close(ncfileid3);
        if (retval != NC_NOERR) qDebug() << "error closing file3";

    }
    else
    {
        getDatasetNameFromBand(slstrview, &radiancename1, &radiancevar1, &geoname, &geolat, &geolon);
        qDebug() << "getDatasetNameFromBand fname1 = " << radiancename1 << " var1 = " << radiancevar1 << " geoname = " << geoname << " geolon = " << geolon << " geolat = " << geolat;

        // open radiance nc file
        array1 = radiancename1.toUtf8();
        pfile1 = array1.constData();

        retval = nc_open(pfile1, NC_NOWRITE, &ncfileid1);
        if(retval != NC_NOERR) qDebug() << "error opening file1 " << radiancename1;

        retval = nc_inq_dimid(ncfileid1, "columns", &columnsid);
        if(retval != NC_NOERR) qDebug() << "error reading columns id file1";
        retval = nc_inq_dimlen(ncfileid1, columnsid, &columnslength);
        if(retval != NC_NOERR) qDebug() << "error reading columns length file1";

        retval = nc_inq_dimid(ncfileid1, "rows", &rowsid);
        if(retval != NC_NOERR) qDebug() << "error reading rows id file1";
        retval = nc_inq_dimlen(ncfileid1, rowsid, &rowslength);
        if(retval != NC_NOERR) qDebug() << "error reading rows length file1";

        this->earth_views_per_scanline = columnslength;
        this->NbrOfLines = rowslength;
        this->longitude.reset(new int[columnslength * rowslength]);
        this->latitude.reset(new int[columnslength * rowslength]);

        this->initializeMemory();

        // open radiance variable
        array1 = radiancevar1.toUtf8();
        pvar1 = array1.constData();

        retval = nc_inq_varid(ncfileid1, pvar1, &radianceid1);
        if (retval != NC_NOERR) qDebug() << "error reading radiance1 id";

        retval = nc_get_var_short(ncfileid1, radianceid1, ptrbaSLSTR[0].data());
        if (retval != NC_NOERR) qDebug() << "error reading radiance1 values";

        retval = nc_get_att_float(ncfileid1, radianceid1, "scale_factor", &scale_factor[0]);
        if (retval != NC_NOERR) qDebug() << "error reading scale_factor[0]";

        retval = nc_get_att_float(ncfileid1, radianceid1, "add_offset", &add_offset[0]);
        if (retval != NC_NOERR) qDebug() << "error reading add_offset[0]";

        // close ncfile
        retval = nc_close(ncfileid1);
        if (retval != NC_NOERR) qDebug() << "error closing file1";
    }


    // open geodetic nc file
    array1 = geoname.toUtf8();
    pfile1 = array1.constData();

    retval = nc_open(pfile1, NC_NOWRITE, &ncgeofileid);
    if(retval != NC_NOERR) qDebug() << "error opening file1 " << geoname;

    // open longitude/latitude variable

    array1 = geolon.toUtf8();
    pvar1 = array1.constData();

    retval = nc_inq_varid(ncgeofileid, pvar1, &longitudeid);
    if (retval != NC_NOERR) qDebug() << "error reading longitude id";
    retval = nc_get_var_int(ncgeofileid, longitudeid, longitude.data());
    if (retval != NC_NOERR) qDebug() << "error reading longitude values";

    array1 = geolat.toUtf8();
    pvar1 = array1.constData();

    retval = nc_inq_varid(ncgeofileid, pvar1, &latitudeid);
    if (retval != NC_NOERR) qDebug() << "error reading latitude id";
    retval = nc_get_var_int(ncgeofileid, latitudeid, this->latitude.data());
    if (retval != NC_NOERR) qDebug() << "error reading latitude values";


    // close ncfile
    retval = nc_close(ncgeofileid);
    if (retval != NC_NOERR) qDebug() << "error closing geodetic";



    float val;
    qint16 pixval[3];

    for(int j=0; j < NbrOfLines; j++)
    {
        for(int i=0; i < earth_views_per_scanline; i++)
        {
            for(int k = 0; k < (iscolorimage ? 3 : 1); k++)
            {
                if(ptrbaSLSTR[k][j*earth_views_per_scanline + i] > 0)
                {
                    val = ((float)ptrbaSLSTR[k][j*earth_views_per_scanline + i] * scale_factor[k] + add_offset[k]) * 10;
                    ptrbaSLSTR[k][j*earth_views_per_scanline + i]  = (quint16)qMin(qMax(qRound(val),0),30000);
                    pixval[k] = ptrbaSLSTR[k][j*earth_views_per_scanline + i];
                }
//                else
//                {
//                    ptrbaSLSTR[k][j*earth_views_per_scanline + i] = 0;
//                }
            }

//            if(pixval[0] > -1 && pixval[1] > -1 && pixval[2] < 0 )
//            {
//                ptrbaSLSTR[2][j*earth_views_per_scanline + i] = 0;
//            }
//            else if(pixval[0] > -1 && pixval[1] < 0 && pixval[2] > -1 )
//            {
//                ptrbaSLSTR[1][j*earth_views_per_scanline + i] = 0;
//            }
//            else if(pixval[0] < 0 && pixval[1] > -1 && pixval[2] > -1 )
//            {
//                ptrbaSLSTR[0][j*earth_views_per_scanline + i] = 0;
//            }


        }
    }


    for(int k = 0; k < (iscolorimage ? 3 : 1); k++)
    {
        for(int j=0; j < NbrOfLines; j++)
        {
            for(int i=0; i < earth_views_per_scanline; i++)
            {
                if(ptrbaSLSTR[k][j*earth_views_per_scanline + i] > -1)
                {
                    if(ptrbaSLSTR[k][j*earth_views_per_scanline + i] > stat_max_ch[k])
                        stat_max_ch[k] = ptrbaSLSTR[k][j*earth_views_per_scanline + i];
                    if(ptrbaSLSTR[k][j*earth_views_per_scanline + i] < stat_min_ch[k])
                        stat_min_ch[k] = ptrbaSLSTR[k][j*earth_views_per_scanline + i];

                    active_pixels[k]++;
                }

            }
        }
    }


    qDebug() << QString("ptrbaSLSTR min_ch[0] = %1 max_ch[0] = %2").arg(stat_min_ch[0]).arg(stat_max_ch[0]);
    if(iscolorimage)
    {
        qDebug() << QString("ptrbaSLSTR min_ch[1] = %1 max_ch[1] = %2").arg(stat_min_ch[1]).arg(stat_max_ch[1]);
        qDebug() << QString("ptrbaSLSTR min_ch[2] = %1 max_ch[2] = %2").arg(stat_min_ch[2]).arg(stat_max_ch[2]);

    }
    qDebug() << QString("active_pixels[0] = %1").arg(active_pixels[0]);

    qDebug() << "einde Segment *SegmentSLSTR::ReadSegmentInMemory()";

    return this;
}

void SegmentSLSTR::getDatasetNameFromBand(eSLSTRImageView view, QString *radiancedataset, QString *radiancevariable, QString *geodeticdataset, QString *latitude, QString *longitude)
{
    QString ncRadiance;
    QString ncGeodetic;

    if(bandlist.at(1))
    {
        ncRadiance = view == OBLIQUE ? "/S1_radiance_ao.nc" : "/S1_radiance_an.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_ao.nc" : "/geodetic_an.nc";
        invertthissegment[0] = invertlist.at(0);
        *radiancevariable = view == OBLIQUE ? "S1_radiance_ao" : "S1_radiance_an";
        *latitude = view == OBLIQUE ? "latitude_ao" : "latitude_an";
        *longitude = view == OBLIQUE ? "longitude_ao" : "longitude_an";
    }
    else if(bandlist.at(2))
    {
        ncRadiance = view == OBLIQUE ? "/S2_radiance_ao.nc" : "/S2_radiance_an.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_ao.nc" : "/geodetic_an.nc";
        invertthissegment[0] = invertlist.at(1);
        *radiancevariable = view == OBLIQUE ? "S2_radiance_ao" : "S2_radiance_an";
        *latitude = view == OBLIQUE ? "latitude_ao" : "latitude_an";
        *longitude = view == OBLIQUE ? "longitude_ao" : "longitude_an";
    }
    else if(bandlist.at(3))
    {
        ncRadiance = view == OBLIQUE ? "/S3_radiance_ao.nc" : "/S3_radiance_an.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_ao.nc" : "/geodetic_an.nc";
        invertthissegment[0] = invertlist.at(2);
        *radiancevariable = view == OBLIQUE ? "S3_radiance_ao" : "S3_radiance_an";
        *latitude = view == OBLIQUE ? "latitude_ao" : "latitude_an";
        *longitude = view == OBLIQUE ? "longitude_ao" : "longitude_an";
    }
    else if(bandlist.at(4))
    {
        ncRadiance = view == OBLIQUE ? "/S4_radiance_ao.nc" : "/S4_radiance_an.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_ao.nc" : "/geodetic_an.nc";
        invertthissegment[0] = invertlist.at(3);
        *radiancevariable = view == OBLIQUE ? "S4_radiance_ao" : "S4_radiance_an";
        *latitude = view == OBLIQUE ? "latitude_ao" : "latitude_an";
        *longitude = view == OBLIQUE ? "longitude_ao" : "longitude_an";
    }
    else if(bandlist.at(5))
    {
        ncRadiance = view == OBLIQUE ? "/S5_radiance_ao.nc" : "/S5_radiance_an.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_ao.nc" : "/geodetic_an.nc";
        invertthissegment[0] = invertlist.at(4);
        *radiancevariable = view == OBLIQUE ? "S5_radiance_ao" : "S5_radiance_an";
        *latitude = view == OBLIQUE ? "latitude_ao" : "latitude_an";
        *longitude = view == OBLIQUE ? "longitude_ao" : "longitude_an";
    }
    else if(bandlist.at(6))
    {
        ncRadiance = view == OBLIQUE ? "/S6_radiance_ao.nc" : "/S6_radiance_an.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_ao.nc" : "/geodetic_an.nc";
        invertthissegment[0] = invertlist.at(5);
        *radiancevariable = view == OBLIQUE ? "S6_radiance_ao" : "S6_radiance_an";
        *latitude = view == OBLIQUE ? "latitude_ao" : "latitude_an";
        *longitude = view == OBLIQUE ? "longitude_ao" : "longitude_an";
    }
    else if(bandlist.at(7))
    {
        ncRadiance = view == OBLIQUE ? "/S7_BT_io.nc" : "/S7_BT_in.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_io.nc" : "/geodetic_in.nc";
        invertthissegment[0] = invertlist.at(6);
        *radiancevariable = view == OBLIQUE ? "S7_BT_io" : "S7_BT_in";
        *latitude = view == OBLIQUE ? "latitude_io" : "latitude_in";
        *longitude = view == OBLIQUE ? "longitude_io" : "longitude_in";
    }
    else if(bandlist.at(8))
    {
        ncRadiance = view == OBLIQUE ? "/S8_BT_io.nc" : "/S8_BT_in.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_io.nc" : "/geodetic_in.nc";
        invertthissegment[0] = invertlist.at(7);
        *radiancevariable = view == OBLIQUE ? "S8_BT_io" : "S8_BT_in";
        *latitude = view == OBLIQUE ? "latitude_io" : "latitude_in";
        *longitude = view == OBLIQUE ? "longitude_io" : "longitude_in";
    }
    else if(bandlist.at(9))
    {
        ncRadiance = view == OBLIQUE ? "/S9_BT_io.nc" : "/S9_BT_in.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_io.nc" : "/geodetic_in.nc";
        invertthissegment[0] = invertlist.at(8);
        *radiancevariable = view == OBLIQUE ? "S9_BT_io" : "S9_BT_in";
        *latitude = view == OBLIQUE ? "latitude_io" : "latitude_in";
        *longitude = view == OBLIQUE ? "longitude_io" : "longitude_in";
    }
    else if(bandlist.at(10))
    {
        ncRadiance = view == OBLIQUE ? "/F1_BT_io.nc" : "/F1_BT_in.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_io.nc" : "/geodetic_in.nc";
        invertthissegment[0] = invertlist.at(9);
        *radiancevariable = view == OBLIQUE ? "F1_BT_io" : "F1_BT_in";
        *latitude = view == OBLIQUE ? "latitude_io" : "latitude_in";
        *longitude = view == OBLIQUE ? "longitude_io" : "longitude_in";
    }
    else if(bandlist.at(11))
    {
        ncRadiance = view == OBLIQUE ? "/F2_BT_io.nc" : "/F2_BT_in.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_io.nc" : "/geodetic_in.nc";
        invertthissegment[0] = invertlist.at(10);
        *radiancevariable = view == OBLIQUE ? "F2_BT_io" : "F2_BT_in";
        *latitude = view == OBLIQUE ? "latitude_io" : "latitude_in";
        *longitude = view == OBLIQUE ? "longitude_io" : "longitude_in";
    }
    *radiancedataset = fileInfo.isDir() ? fileInfo.absoluteFilePath() + ncRadiance : this->fileInfo.baseName() + ".SEN3" + ncRadiance;
    *geodeticdataset = fileInfo.isDir() ? fileInfo.absoluteFilePath() + ncGeodetic : this->fileInfo.baseName() + ".SEN3" + ncGeodetic;

}

void SegmentSLSTR::getDatasetNameFromColor(eSLSTRImageView view, int colorindex, QString *radiancedataset, QString *radiancevariable, QString *geodeticdataset, QString *latitude, QString *longitude)
{
    QString ncRadiance;
    QString ncGeodetic;

    qDebug() << "getDatasetNameFromColor colorindex = " << colorindex;

    Q_ASSERT(colorindex >=0 && colorindex < 3);
    colorindex++; // 1, 2 or 3

    if(colorlist.at(0) == colorindex)
    {
        ncRadiance = view == OBLIQUE ? "/S1_radiance_ao.nc" : "/S1_radiance_an.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_ao.nc" : "/geodetic_an.nc";
        invertthissegment[colorindex-1] = invertlist.at(0);
        *radiancevariable = view == OBLIQUE ? "S1_radiance_ao" : "S1_radiance_an";
        *latitude = view == OBLIQUE ? "latitude_ao" : "latitude_an";
        *longitude = view == OBLIQUE ? "longitude_ao" : "longitude_an";
    }
    else if(colorlist.at(1) == colorindex)
    {
        ncRadiance = view == OBLIQUE ? "/S2_radiance_ao.nc" : "/S2_radiance_an.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_ao.nc" : "/geodetic_an.nc";
        invertthissegment[colorindex-1] = invertlist.at(1);
        *radiancevariable = view == OBLIQUE ? "S2_radiance_ao" : "S2_radiance_an";
        *latitude = view == OBLIQUE ? "latitude_ao" : "latitude_an";
        *longitude = view == OBLIQUE ? "longitude_ao" : "longitude_an";
    }
    else if(colorlist.at(2) == colorindex)
    {
        ncRadiance = view == OBLIQUE ? "/S3_radiance_ao.nc" : "/S3_radiance_an.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_ao.nc" : "/geodetic_an.nc";
        invertthissegment[colorindex-1] = invertlist.at(2);
        *radiancevariable = view == OBLIQUE ? "S3_radiance_ao" : "S3_radiance_an";
        *latitude = view == OBLIQUE ? "latitude_ao" : "latitude_an";
        *longitude = view == OBLIQUE ? "longitude_ao" : "longitude_an";
    }
    else if(colorlist.at(3) == colorindex)
    {
        ncRadiance = view == OBLIQUE ? "/S4_radiance_ao.nc" : "/S4_radiance_an.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_ao.nc" : "/geodetic_an.nc";
        invertthissegment[colorindex-1] = invertlist.at(3);
        *radiancevariable = view == OBLIQUE ? "S4_radiance_ao" : "S4_radiance_an";
        *latitude = view == OBLIQUE ? "latitude_ao" : "latitude_an";
        *longitude = view == OBLIQUE ? "longitude_ao" : "longitude_an";
    }
    else if(colorlist.at(4) == colorindex)
    {
        ncRadiance = view == OBLIQUE ? "/S5_radiance_ao.nc" : "/S5_radiance_an.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_ao.nc" : "/geodetic_an.nc";
        invertthissegment[colorindex-1] = invertlist.at(4);
        *radiancevariable = view == OBLIQUE ? "S5_radiance_ao" : "S5_radiance_an";
        *latitude = view == OBLIQUE ? "latitude_ao" : "latitude_an";
        *longitude = view == OBLIQUE ? "longitude_ao" : "longitude_an";
    }
    else if(colorlist.at(5) == colorindex)
    {
        ncRadiance = view == OBLIQUE ? "/S6_radiance_ao.nc" : "/S6_radiance_an.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_ao.nc" : "/geodetic_an.nc";
        invertthissegment[colorindex-1] = invertlist.at(5);
        *radiancevariable = view == OBLIQUE ? "S6_radiance_ao" : "S6_radiance_an";
        *latitude = view == OBLIQUE ? "latitude_ao" : "latitude_an";
        *longitude = view == OBLIQUE ? "longitude_ao" : "longitude_an";
    }
    else if(colorlist.at(6) == colorindex)
    {
        ncRadiance = view == OBLIQUE ? "/S7_BT_io.nc" : "/S7_BT_in.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_io.nc" : "/geodetic_in.nc";
        invertthissegment[colorindex-1] = invertlist.at(6);
        *radiancevariable = view == OBLIQUE ? "S7_BT_io" : "S7_BT_in";
        *latitude = view == OBLIQUE ? "latitude_io" : "latitude_in";
        *longitude = view == OBLIQUE ? "longitude_io" : "longitude_in";
    }
    else if(colorlist.at(7) == colorindex)
    {
        ncRadiance = view == OBLIQUE ? "/S8_BT_io.nc" : "/S8_BT_in.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_io.nc" : "/geodetic_in.nc";
        invertthissegment[colorindex-1] = invertlist.at(7);
        *radiancevariable = view == OBLIQUE ? "S8_BT_io" : "S8_BT_in";
        *latitude = view == OBLIQUE ? "latitude_io" : "latitude_in";
        *longitude = view == OBLIQUE ? "longitude_io" : "longitude_in";
    }
    else if(colorlist.at(8) == colorindex)
    {
        ncRadiance = view == OBLIQUE ? "/S9_BT_io.nc" : "/S9_BT_in.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_io.nc" : "/geodetic_in.nc";
        invertthissegment[colorindex-1] = invertlist.at(8);
        *radiancevariable = view == OBLIQUE ? "S9_BT_io" : "S9_BT_in";
        *latitude = view == OBLIQUE ? "latitude_io" : "latitude_in";
        *longitude = view == OBLIQUE ? "longitude_io" : "longitude_in";
    }
    else if(colorlist.at(9) == colorindex)
    {
        ncRadiance = view == OBLIQUE ? "/F1_BT_io.nc" : "/F1_BT_in.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_io.nc" : "/geodetic_in.nc";
        invertthissegment[colorindex-1] = invertlist.at(9);
        *radiancevariable = view == OBLIQUE ? "F1_BT_io" : "F1_BT_in";
        *latitude = view == OBLIQUE ? "latitude_io" : "latitude_in";
        *longitude = view == OBLIQUE ? "longitude_io" : "longitude_in";
    }
    else if(colorlist.at(10) == colorindex)
    {
        ncRadiance = view == OBLIQUE ? "/F2_BT_io.nc" : "/F2_BT_in.nc";
        ncGeodetic = view == OBLIQUE ? "/geodetic_io.nc" : "/geodetic_in.nc";
        invertthissegment[colorindex-1] = invertlist.at(10);
        *radiancevariable = view == OBLIQUE ? "F2_BT_io" : "F2_BT_in";
        *latitude = view == OBLIQUE ? "latitude_io" : "latitude_in";
        *longitude = view == OBLIQUE ? "longitude_io" : "longitude_in";
    }
    *radiancedataset = fileInfo.isDir() ? fileInfo.absoluteFilePath() + ncRadiance : this->fileInfo.baseName() + ".SEN3" + ncRadiance;
    *geodeticdataset = fileInfo.isDir() ? fileInfo.absoluteFilePath() + ncGeodetic : this->fileInfo.baseName() + ".SEN3" + ncGeodetic;

}

void SegmentSLSTR ::setBandandColorandView(QList<bool> band, QList<int> color, QList<bool> invert, eSLSTRImageView slstrview)
{
    bandlist = band;
    colorlist = color;
    invertlist = invert;
    this->slstrview = slstrview;

}

void SegmentSLSTR::RenderSegmentlineInTextureSLSTR( int nbrLine, QRgb *row )
{

    QColor rgb;
    int posx, posy;

    QMutexLocker locker(&g_mutex);

    QPainter fb_painter(imageptrs->pmOut);

    int devwidth = (fb_painter.device())->width();
    int devheight = (fb_painter.device())->height();

    fb_painter.setPen( Qt::black );
    fb_painter.setBrush( Qt::NoBrush );

    int earthviews = earth_views_per_scanline;

    int pixval[3];
    bool valok[3];
    bool color = bandlist.at(0);

    float flon, flat, fflon, fflat;

    for (int pix = 0 ; pix < earthviews; pix+=8)
    {
        pixval[0] = (int)ptrbaSLSTR[0][nbrLine * earthviews + pix];
        valok[0] = pixval[0] > -1;


        if(color)
        {
            pixval[1] = (int)ptrbaSLSTR[1][nbrLine * earthviews + pix];
            pixval[2] = (int)ptrbaSLSTR[2][nbrLine * earthviews + pix];
            valok[1] = pixval[1] > -1;
            valok[2] = pixval[2] > -1;
        }


        if( valok[0] && (color ? valok[1] && valok[2] : true))
        {
            fflon = (float)(this->longitude[nbrLine * earthviews + pix])/1000000.0;
            fflat = (float)(this->latitude[nbrLine * earthviews + pix])/1000000.0;
            flon = fflon * PI/180.0;
            flat = fflat * PI/180.0;


            sphericalToPixel( flon, flat, posx, posy, devwidth, devheight );
            rgb.setRgb(qRed(row[pix]), qGreen(row[pix]), qBlue(row[pix]));
            fb_painter.setPen(rgb);
            fb_painter.drawPoint( posx , posy );

        }
    }

    fb_painter.end();

}
