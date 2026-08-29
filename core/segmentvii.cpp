#include "segmentvii.h"
#include "segmentimage.h"
#include "options.h"
#include <QDebug>

#include <cmath>

extern Options opts;
extern SegmentImage *imageptrs;
extern SatelliteList satellitelist;

SegmentVII::SegmentVII(eSegmentType type, QFileInfo fileinfo, QObject *parent) :
  Segment(parent)
{

    bool ok;

    this->fileInfo = fileinfo;

    if(type == SEG_METOPSGA1)
    {
        segment_type = "METOP_SGA1";
        segtype = eSegmentType::SEG_METOPSGA1;
    }

    //012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
    //0         1         2         3         4         5         6         7         8         9         10
    //W_XX-EUMETSAT-Darmstadt,SAT,SGA1-VII-1B-RAD_C_EUMT_20210219013949_G_D_20070912084303_20070912084403_T_B____.nc

    const QString filename = fileInfo.fileName();
    QStringView fname{filename};
    int sensing_start_year = fname.mid(70, 4).toInt( &ok , 10);
    int sensing_start_month = fname.mid(74, 2).toInt( &ok, 10);
    int sensing_start_day = fname.mid(76, 2).toInt( &ok, 10);
    int sensing_start_hour = fname.mid(78, 2).toInt( &ok, 10);
    int sensing_start_minute = fname.mid(80, 2).toInt( &ok, 10);
    int sensing_start_second = fname.mid(82, 2).toInt( &ok, 10);

    int sensing_end_year = fname.mid(85, 4).toInt( &ok , 10);
    int sensing_end_month = fname.mid(89, 2).toInt( &ok, 10);
    int sensing_end_day = fname.mid(91, 2).toInt( &ok, 10);
    int sensing_end_hour = fname.mid(93, 2).toInt( &ok, 10);
    int sensing_end_minute = fname.mid(95, 2).toInt( &ok, 10);
    int sensing_end_second = fname.mid(97, 2).toInt( &ok, 10);

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


    // The real dimensions come from the product in ReadSegmentInMemory; these
    // are the nominal ones, needed before the file is ever opened because the
    // map and globe views draw the footprint of every segment in the list.
    this->earth_views_per_scanline = 3144;
    this->NbrOfLines = 840;
    this->num_pixels_alt = 24;

    this->nbrsaturatedpixels = 0;
    this->active_pixels_projection = 0;
    for(int k = 0; k < 3; k++)
    {
        this->invertthissegment[k] = false;
        this->stat_max_projection[k] = 0;
        this->stat_min_projection[k] = 0;
    }

    Satellite *sga1_sat;

    if(fileInfo.fileName().mid(28,4) == "SGA1")
        sga1_sat = satellitelist.GetSatellite(65159, &ok);

    if(!ok)
    {
        qInfo() << "EUMETCastView needs TLE's for SGA1";
        return;
    }

    line1 = sga1_sat->line1;
    line2 = sga1_sat->line2;

    //line1 = "1 33591U 09005A   11039.40718334  .00000086  00000-0  72163-4 0  8568";
    //line2 = "2 33591  98.8157 341.8086 0013952 344.4168  15.6572 14.11126791103228";
    double epoch = line1.mid(18,14).toDouble(&ok);
    julian_state_vector = Julian_Date_of_Epoch(epoch);

    qtle.reset(new QTle(sga1_sat->sat_name, line1, line2, QTle::wgs72));
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
    if(segtype == SEG_METOPSGA1)
    {
        CalculateDetailCornerPoints();
    }

}

// The band radio buttons and the colour combos both number the VII channels
// 1..20 in the order they appear in the product, so one loop replaces the
// twenty-way if chain the other sensors use.
QString SegmentVII::getChannelNameFromColor(int colorindex)
{
    Q_ASSERT(colorindex >= 0 && colorindex < 3);

    for(int band = 0; band < colorlist.count(); band++)
    {
        if(colorlist.at(band) == colorindex + 1)   // combo index 1, 2 or 3 = R, G or B
        {
            invertthissegment[colorindex] = invertlist.at(band);
            return ViiL1BReader::channelVariableName(band + 1);
        }
    }
    return QString();
}

QString SegmentVII::getChannelNameFromBand()
{
    for(int band = 1; band < bandlist.count(); band++)  // bandlist.at(0) = the colour radio button
    {
        if(bandlist.at(band))
        {
            invertthissegment[0] = invertlist.at(band - 1);
            return ViiL1BReader::channelVariableName(band);
        }
    }
    return QString();
}

bool SegmentVII::isDuplicatedPixel(int line, int pixelx) const
{
    if(duplicationmask.isEmpty())
        return false;
    return duplicationmask.at((line % num_pixels_alt) * earth_views_per_scanline + pixelx) != 0;
}

Segment *SegmentVII::ReadSegmentInMemory()
{
    bool iscolorimage = this->bandlist.at(0);

    if(!reader.open(fileInfo.absoluteFilePath()))
    {
        qCritical() << "SegmentVII::ReadSegmentInMemory " << reader.lastError();
        return this;
    }

    const ViiGeometry geom = reader.geometry();
    this->earth_views_per_scanline = geom.npixels;
    this->NbrOfLines = geom.nlines;
    this->num_pixels_alt = geom.nd;

    QString channel[3];
    if(iscolorimage)
    {
        for(int k = 0; k < 3; k++)
            channel[k] = getChannelNameFromColor(k);
    }
    else
        channel[0] = getChannelNameFromBand();

    if(channel[0].isEmpty() || (iscolorimage && (channel[1].isEmpty() || channel[2].isEmpty())))
    {
        qCritical() << "SegmentVII::ReadSegmentInMemory no VII band selected";
        reader.close();
        return this;
    }

    qDebug() << QString("SegmentVII::ReadSegmentInMemory %1 x %2 channels %3 %4 %5")
                .arg(earth_views_per_scanline).arg(NbrOfLines)
                .arg(channel[0]).arg(channel[1]).arg(channel[2]);

    this->initializeMemory();

    const int npix = geom.pixelCount();

    // 65535 is the no-data marker; setting it up front means an error further
    // down leaves a transparent segment instead of uninitialised memory
    for(int k = 0; k < (iscolorimage ? 3 : 1); k++)
    {
        for(int i = 0; i < npix; i++)
        {
            ptrbaVII[k][i] = 65535;
            ptrbaVIInormalized[k][i] = 65535;
        }
    }

    // The product only carries geolocation on the tie point grid, so every
    // full resolution position has to be reconstructed from it.
    QVector<float> lat, lon;
    if(!reader.interpolateGeolocation(&lat, &lon))
    {
        qCritical() << "SegmentVII::ReadSegmentInMemory " << reader.lastError();
        reader.close();
        return this;
    }

    if(opts.viidemorthorectify)
    {
        if(!reader.orthorectify(&lat, &lon))
            qDebug() << "SegmentVII::ReadSegmentInMemory no DEM orthorectification : " << reader.lastError();
    }

    latitude.reset(new float[npix]);
    longitude.reset(new float[npix]);
    memcpy(latitude.data(), lat.constData(), npix * sizeof(float));
    memcpy(longitude.data(), lon.constData(), npix * sizeof(float));

    CalcOverlayLatLon();

    // sec(SZA), the same normalization SegmentOLCI does from tie_geometries.nc
    QVector<float> sza;
    QScopedArrayPointer<float> secSZA(new float[npix]);
    if(reader.interpolateTiePointVariable(QStringLiteral("solar_zenith"), &sza))
    {
        for(int i = 0; i < npix; i++)
        {
            const float c = cos(sza.at(i) * PIE / 180.0);
            // beyond the terminator sec(SZA) runs away, so hold it at the
            // grazing value rather than letting it blow the stretch apart
            secSZA[i] = (std::isnan(sza.at(i)) || c < 0.01f) ? 100.0f : 1.0f / c;
        }
    }
    else
    {
        qDebug() << "SegmentVII::ReadSegmentInMemory no solar_zenith : " << reader.lastError();
        for(int i = 0; i < npix; i++)
            secSZA[i] = 1.0f;
    }

    if(!reader.readDuplicationMask(&duplicationmask))
        qDebug() << "SegmentVII::ReadSegmentInMemory " << reader.lastError();

    // Every channel is packed onto 0..65534 against its own valid range as the
    // product states it, with 65535 kept free as the no data marker. The range
    // is a property of the channel and not of this granule, so the per channel
    // statistics of different segments stay comparable and can be merged.
    QVector<float> rad;
    for(int k = 0; k < (iscolorimage ? 3 : 1); k++)
    {
        double radmin, radmax;
        if(!reader.radianceRange(channel[k], &radmin, &radmax)
           || !reader.readFullGridVariable(channel[k], &rad))
        {
            qCritical() << "SegmentVII::ReadSegmentInMemory " << reader.lastError();
            reader.close();
            return this;
        }

        const double scale = 65534.0 / (radmax - radmin);

        for(int i = 0; i < npix; i++)
        {
            const float val = rad.at(i);
            if(std::isnan(val))
            {
                ptrbaVII[k][i] = 65535;
                ptrbaVIInormalized[k][i] = 65535;
            }
            else
            {
                ptrbaVII[k][i] = (quint16)qBound(0, qRound((val - radmin) * scale), 65534);
                // clamped at 65534, not 65535, which is the no data marker
                ptrbaVIInormalized[k][i] = (quint16)qBound(0, qRound((val * secSZA[i] - radmin) * scale), 65534);
            }
        }
    }

    reader.close();

    for(int k = 0; k < 3; k++)
    {
        stat_max_ch[k] = 0;
        stat_min_ch[k] = 9999999;
        stat_max_norm_ch[k] = 0;
        stat_min_norm_ch[k] = 9999999;
        active_pixels[k] = 0;
    }

    nbrsaturatedpixels = 0;

    for(int k = 0; k < (iscolorimage ? 3 : 1); k++)
    {
        for(int i = 0; i < npix; i++)
        {
            const quint16 pix = ptrbaVII[k][i];
            if(pix < 65535)
            {
                if(pix > stat_max_ch[k])
                    stat_max_ch[k] = pix;
                if(pix < stat_min_ch[k])
                    stat_min_ch[k] = pix;
                if(ptrbaVIInormalized[k][i] > stat_max_norm_ch[k])
                    stat_max_norm_ch[k] = ptrbaVIInormalized[k][i];
                if(ptrbaVIInormalized[k][i] < stat_min_norm_ch[k])
                    stat_min_norm_ch[k] = ptrbaVIInormalized[k][i];
                if(pix == 65534)  // sitting on the channel's valid_max
                    nbrsaturatedpixels++;
                active_pixels[k]++;
            }
        }
    }

    qDebug() << QString("ptrbaVII min_ch[0] = %1 max_ch[0] = %2").arg(stat_min_ch[0]).arg(stat_max_ch[0]);
    if(iscolorimage)
    {
        qDebug() << QString("ptrbaVII min_ch[1] = %1 max_ch[1] = %2").arg(stat_min_ch[1]).arg(stat_max_ch[1]);
        qDebug() << QString("ptrbaVII min_ch[2] = %1 max_ch[2] = %2").arg(stat_min_ch[2]).arg(stat_max_ch[2]);
    }
    qDebug() << QString("ptrbaVIInormalized min_ch[0] = %1 max_ch[0] = %2").arg(stat_min_norm_ch[0]).arg(stat_max_norm_ch[0]);
    if(iscolorimage)
    {
        qDebug() << QString("ptrbaVIInormalized min_ch[1] = %1 max_ch[1] = %2").arg(stat_min_norm_ch[1]).arg(stat_max_norm_ch[1]);
        qDebug() << QString("ptrbaVIInormalized min_ch[2] = %1 max_ch[2] = %2").arg(stat_min_norm_ch[2]).arg(stat_max_norm_ch[2]);
    }
    qDebug() << QString("Nbr of saturated pixels = %1").arg(nbrsaturatedpixels);

    return this;

}

// Mark every pixel where the whole-degree part of the latitude or the longitude
// changes, which traces the parallels and meridians across the swath.
void SegmentVII::CalcOverlayLatLon()
{
    latlonline.clear();

    if(latitude.isNull() || longitude.isNull())
        return;

    for (int line = 1; line < NbrOfLines; line++)
    {
        for (int pixelx = 1; pixelx < earth_views_per_scanline; pixelx++)
        {
            const int idx = line * earth_views_per_scanline + pixelx;
            const float lat = latitude[idx];
            const float lon = longitude[idx];
            if(std::isnan(lat) || std::isnan(lon))
                continue;

            const float latleft = latitude[idx - 1];
            const float lonleft = longitude[idx - 1];
            const float latup = latitude[idx - earth_views_per_scanline];
            const float lonup = longitude[idx - earth_views_per_scanline];
            if(std::isnan(latleft) || std::isnan(lonleft) || std::isnan(latup) || std::isnan(lonup))
                continue;

            // the dateline puts a 360 degree step between neighbours, which is
            // not a meridian crossing
            const bool lonwrap = fabs(lon - lonleft) > 180.0f || fabs(lon - lonup) > 180.0f;

            if(floor(lat) != floor(latleft) || floor(lat) != floor(latup)
               || (!lonwrap && (floor(lon) != floor(lonleft) || floor(lon) != floor(lonup))))
            {
                latlonline << QPoint(pixelx, line);
            }
        }
    }

    qDebug() << QString("SegmentVII::CalcOverlayLatLon %1 graticule points").arg(latlonline.count());
}

void SegmentVII::RenderSegmentlineInTextureVII( int nbrLine, QRgb *row )
{

    QColor rgb;
    int posx, posy;

    QPainter fb_painter(imageptrs->pmOut);

    int devwidth = (fb_painter.device())->width();
    int devheight = (fb_painter.device())->height();

    fb_painter.setPen( Qt::black );
    fb_painter.setBrush( Qt::NoBrush );

    int earthviews = earth_views_per_scanline;

    int pixval[3];
    bool valok[3];
    bool color = bandlist.at(0);

    float flon, flat;

    for (int pix = 0 ; pix < earthviews; pix+=8)
    {
        pixval[0] = (int)ptrbaVII[0][nbrLine * earthviews + pix];
        valok[0] = pixval[0] < 65535;
        if(color)
        {
            pixval[1] = (int)ptrbaVII[1][nbrLine * earthviews + pix];
            pixval[2] = (int)ptrbaVII[2][nbrLine * earthviews + pix];
            valok[1] = pixval[1] < 65535;
            valok[2] = pixval[2] < 65535;
        }

        if(isDuplicatedPixel(nbrLine, pix))
            continue;

        if( valok[0] && (color ? valok[1] && valok[2] : true))
        {
            flat = latitude[nbrLine * earthviews + pix];
            flon = longitude[nbrLine * earthviews + pix];
            if(std::isnan(flat) || std::isnan(flon))
                continue;

            sphericalToPixel( flon * PIE/180.0, flat * PIE/180.0, posx, posy, devwidth, devheight );
            rgb.setRgb(qRed(row[pix]), qGreen(row[pix]), qBlue(row[pix]));
            fb_painter.setPen(rgb);
            fb_painter.drawPoint( posx , posy );
        }
    }

    fb_painter.end();

}

void SegmentVII::initializeMemory()
{
    qDebug() << "Initializing VII memory";

    bool color = this->bandlist.at(0);

    for(int k = 0; k < (color ? 3 : 1); k++)
    {
        if(ptrbaVII[k].isNull())
        {
            ptrbaVII[k].reset(new quint16[earth_views_per_scanline * NbrOfLines]);
            ptrbaVIInormalized[k].reset(new quint16[earth_views_per_scanline * NbrOfLines]);
            qDebug() << QString("Initializing VII memory earth views = %1 nbr of lines = %2").arg(earth_views_per_scanline).arg(NbrOfLines);
        }
    }
    qDebug() << "End Initializing VII memory";

}


void SegmentVII::ComposeSegmentImage(int histogrammethod, bool normalized)
{

    QRgb *row;
    quint16 indexout[3];

    qDebug() << QString("SegmentVII::ComposeSegmentImage() segm->startLineNbr = %1").arg(this->startLineNbr);
    qDebug() << QString("SegmentVII::ComposeSegmentImage() color = %1 ").arg(bandlist.at(0));

    int color[3];
    quint16 pixval[3];
    quint16 pixval1024[3];

    bool iscolor = bandlist.at(0);
    bool valok[3];

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        row = (QRgb*)imageptrs->ptrimageVII->scanLine(this->startLineNbr + line);
        for (int pixelx = 0; pixelx < earth_views_per_scanline; pixelx++)
        {
            if(normalized) pixval[0] = this->ptrbaVIInormalized[0][line * earth_views_per_scanline + pixelx];
            else pixval[0] = this->ptrbaVII[0][line * earth_views_per_scanline + pixelx];

            if(iscolor)
            {
                if(normalized) pixval[1] = this->ptrbaVIInormalized[1][line * earth_views_per_scanline + pixelx];
                else pixval[1] = this->ptrbaVII[1][line * earth_views_per_scanline + pixelx];
                if(normalized) pixval[2] = this->ptrbaVIInormalized[2][line * earth_views_per_scanline + pixelx];
                else pixval[2] = this->ptrbaVII[2][line * earth_views_per_scanline + pixelx];
            }

            valok[0] = pixval[0] < 65535;
            if(iscolor)
            {
                valok[1] = pixval[1] < 65535;
                valok[2] = pixval[2] < 65535;
            }

            if( valok[0] && (iscolor ? valok[1] && valok[2] : true))
            {
                for(int k = 0; k < (iscolor ? 3 : 1); k++)
                {
                    if(normalized) pixval1024[k] =  (quint16)qMin(qMax(qRound(1023.0 * (float)(pixval[k] - imageptrs->stat_min_norm_ch[k] ) / (float)(imageptrs->stat_max_norm_ch[k] - imageptrs->stat_min_norm_ch[k])), 0), 1023);
                    else pixval1024[k] =  (quint16)qMin(qMax(qRound(1023.0 * (float)(pixval[k] - imageptrs->stat_min_ch[k] ) / (float)(imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k])), 0), 1023);

                    if(histogrammethod == CMB_HISTO_NONE_95) // 95%
                    {
                            if(normalized) indexout[k] =  (quint16)qMin(qMax(qRound(1023.0 * (float)(pixval1024[k] - imageptrs->minRadianceIndexNormalized[k] ) / (float)(imageptrs->maxRadianceIndexNormalized[k] - imageptrs->minRadianceIndexNormalized[k])), 0), 1023);
                            else indexout[k] =  (quint16)qMin(qMax(qRound(1023.0 * (float)(pixval1024[k] - imageptrs->minRadianceIndex[k] ) / (float)(imageptrs->maxRadianceIndex[k] - imageptrs->minRadianceIndex[k])), 0), 1023);
                    }
                    else if(histogrammethod == CMB_HISTO_NONE_100) // 100%
                    {
                            indexout[k] =  pixval1024[k];
                    }

                    if(invertthissegment[k])
                    {
                        if(normalized) color[k] = 255 - imageptrs->lut_norm_ch[k][indexout[k]]/4;
                        else color[k] = 255 - imageptrs->lut_ch[k][indexout[k]]/4;
                    }
                    else
                    {
                        if(histogrammethod == CMB_HISTO_NONE_95 || histogrammethod == CMB_HISTO_NONE_100)
                        {
                            color[k] = (quint16)qMin(qMax(qRound((float)indexout[k]/4), 0), 255);
                        }
                        else if(histogrammethod == CMB_HISTO_EQUALIZE)
                        {
                            if(normalized) color[k] = (quint16)qMin(qMax(qRound((float)imageptrs->lut_norm_ch[k][pixval1024[k]]/4), 0), 255);
                            else color[k] = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[k][pixval1024[k]]/4), 0), 255);
                        }
                    }
                }

                row[pixelx] = qRgba(color[0], iscolor ? color[1] : color[0], iscolor ? color[2] : color[0], 255 );

            }
            else
            {
                row[pixelx] = qRgba(0, 0, 0, 0);
            }

        }
        if(opts.imageontextureOnVII && line % 2 == 0)
        {
            this->RenderSegmentlineInTextureVII( line, row );
            opts.texture_changed = true;
        }

    }
}



void SegmentVII::ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(GVP, histogrammethod, normalized);
}

void SegmentVII::ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(LCC, histogrammethod, normalized);
}

void SegmentVII::ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(SG, histogrammethod, normalized);
}

void SegmentVII::ComposeProjection(eProjections proj, int histogrammethod, bool normalized)
{

    qDebug() << "SegmentVII::ComposeProjection() hist = " << histogrammethod << " " << normalized;

    double map_x, map_y;

    float lonpos1, latpos1;

    quint16 pixval[3];

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

    for( int i = 0; i < this->NbrOfLines; i++)
    {
        for( int j = 0; j < this->earth_views_per_scanline ; j++ )
        {
            if(normalized) pixval[0] = ptrbaVIInormalized[0][i * earth_views_per_scanline + j];
            else pixval[0] = ptrbaVII[0][i * earth_views_per_scanline + j];
            valok[0] = pixval[0] < 65535;

            if(color)
            {
                if(normalized) pixval[1] = ptrbaVIInormalized[1][i * earth_views_per_scanline + j];
                else pixval[1] = ptrbaVII[1][i * earth_views_per_scanline + j];

                if(normalized) pixval[2] = ptrbaVIInormalized[2][i * earth_views_per_scanline + j];
                else pixval[2] = ptrbaVII[2][i * earth_views_per_scanline + j];

                valok[1] = pixval[1] < 65535;
                valok[2] = pixval[2] < 65535;
            }

            latpos1 = latitude[i * earth_views_per_scanline + j];
            lonpos1 = longitude[i * earth_views_per_scanline + j];

            // The bow-tie overlap hands the same ground twice; keeping both
            // copies would let the duplicate overwrite the pixel that the
            // scan actually owns.
            if( valok[0] && (color ? valok[1] && valok[2] : true)
                && !std::isnan(latpos1) && !std::isnan(lonpos1)
                && !isDuplicatedPixel(i, j))
            {
                if(proj == LCC) // Lambert
                {
                    if(imageptrs->lcc->map_forward_neg_coord(lonpos1 * PIE / 180.0, latpos1 * PIE / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y, color, histogrammethod, normalized);
                    }
                }
                else if(proj == GVP) // General Vertical Perspecitve
                {
                    if(imageptrs->gvp->map_forward_neg_coord(lonpos1 * PIE / 180.0, latpos1 * PIE / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y, color, histogrammethod, normalized);
                    }

                }
                else if(proj == SG) // Stereographic
                {
                    if(imageptrs->sg->map_forward_neg_coord(lonpos1 * PIE / 180.0, latpos1 * PIE / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y, color, histogrammethod, normalized);
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

void SegmentVII::MapPixel(int lines, int views, double map_x, double map_y, bool iscolor, int histogrammethod, bool normalized)
{
    int indexout[3];
    quint16 pixval[3];
    quint16 pixval1024[3];
    quint16 pixval4096[3];

    int color8[3];
    int color12[3];
    QRgb rgbvalue = qRgba(0,0,0,0);

    if(normalized) pixval[0] = ptrbaVIInormalized[0][lines * earth_views_per_scanline + views];
    else pixval[0] = ptrbaVII[0][lines * earth_views_per_scanline + views];

    if(iscolor)
    {
        if(normalized) pixval[1] = ptrbaVIInormalized[1][lines * earth_views_per_scanline + views];
        else pixval[1] = ptrbaVII[1][lines * earth_views_per_scanline + views];

        if(normalized) pixval[2] = ptrbaVIInormalized[2][lines * earth_views_per_scanline + views];
        else pixval[2] = ptrbaVII[2][lines * earth_views_per_scanline + views];
    }

    if (map_x > -15 && map_x < imageptrs->ptrimageProjection->width() + 15 && map_y > -15 && map_y < imageptrs->ptrimageProjection->height() + 15)
    {

        projectionCoordX[lines * earth_views_per_scanline + views] = (qint32)map_x;
        projectionCoordY[lines * earth_views_per_scanline + views] = (qint32)map_y;


        for(int k = 0; k < (iscolor ? 3 : 1); k++)
        {
            pixval4096[k] =  (quint16)qMin(qMax(qRound(4095.0 * (float)(pixval[k] - imageptrs->stat_min_ch[k] ) / (float)(imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k])), 0), 4095);

            if(normalized) pixval1024[k] =  (quint16)qMin(qMax(qRound(1023.0 * (float)(pixval[k] - imageptrs->stat_min_norm_ch[k] ) / (float)(imageptrs->stat_max_norm_ch[k] - imageptrs->stat_min_norm_ch[k])), 0), 1023);
            else pixval1024[k] =  (quint16)qMin(qMax(qRound(1023.0 * (float)(pixval[k] - imageptrs->stat_min_ch[k] ) / (float)(imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k])), 0), 1023);

            if(histogrammethod == CMB_HISTO_NONE_95) // 95%
            {
                    if(normalized) indexout[k] =  (quint16)qMin(qMax(qRound(1023.0 * (float)(pixval1024[k] - imageptrs->minRadianceIndexNormalized[k] ) / (float)(imageptrs->maxRadianceIndexNormalized[k] - imageptrs->minRadianceIndexNormalized[k])), 0), 1023);
                    else indexout[k] =  (quint16)qMin(qMax(qRound(1023.0 * (float)(pixval1024[k] - imageptrs->minRadianceIndex[k] ) / (float)(imageptrs->maxRadianceIndex[k] - imageptrs->minRadianceIndex[k])), 0), 1023);
            }
            else if(histogrammethod == CMB_HISTO_NONE_100) // 100%
            {
                    indexout[k] =  pixval1024[k];
            }

            if(invertthissegment[k])
            {
                color12[k] = 4095 - pixval4096[k];

                if(histogrammethod == CMB_HISTO_NONE_95 || histogrammethod == CMB_HISTO_NONE_100)
                {
                    color8[k] = 255 - (quint16)qMin(qMax(qRound((float)indexout[k]/4), 0), 255);
                }
                else if(histogrammethod == CMB_HISTO_EQUALIZE || histogrammethod == CMB_HISTO_EQUALIZE_PROJ)
                {
                    if(normalized)
                    {
                        color8[k] = 255 - (quint16)qMin(qMax(qRound((float)imageptrs->lut_norm_ch[k][pixval1024[k]]/4), 0), 255);
                    }
                    else
                    {
                        color8[k] = 255 - (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[k][pixval1024[k]]/4), 0), 255);
                    }
                }
            }
            else
            {
                color12[k] = pixval4096[k];

                if(histogrammethod == CMB_HISTO_NONE_95 || histogrammethod == CMB_HISTO_NONE_100)
                {
                    color8[k] = (quint16)qMin(qMax(qRound((float)indexout[k]/4), 0), 255);
                }
                else if(histogrammethod == CMB_HISTO_EQUALIZE || histogrammethod == CMB_HISTO_EQUALIZE_PROJ)
                {
                    if(normalized)
                    {
                        color8[k] = (quint16)qMin(qMax(qRound((float)imageptrs->lut_norm_ch[k][pixval1024[k]]/4), 0), 255);
                    }
                    else
                    {
                        color8[k] = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[k][pixval1024[k]]/4), 0), 255);
                    }
                }
            }
        }


        rgbvalue = qRgba(color8[0], iscolor ? color8[1] : color8[0], iscolor ? color8[2] : color8[0], 255 );

        if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
            imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
        projectionCoordValue[lines * earth_views_per_scanline + views] = rgbvalue;
        projectionCoordValueRed[lines * earth_views_per_scanline + views] = color12[0];
        if(iscolor)
        {
            projectionCoordValueGreen[lines * earth_views_per_scanline + views] = color12[1];
            projectionCoordValueBlue[lines * earth_views_per_scanline + views] = color12[2];
        }
        else
        {
            projectionCoordValueGreen[lines * earth_views_per_scanline + views] = color12[0];
            projectionCoordValueBlue[lines * earth_views_per_scanline + views] = color12[0];
        }
    }
}


void SegmentVII::recalculateStatsInProjection(bool normalized)
{
    int x, y;

    int statmax[3], statmin[3];
    long activepixels[3];
    quint16 pixval[3];

    qDebug() << "SegmentVII::recalculateStatsInProjection()";

    for(int k = 0; k < 3; k++)
    {
        statmax[k] = 0;
        statmin[k] = 999999;
        activepixels[k] = 0;
    }

    for(int k = 0; k < (this->bandlist.at(0) ? 3 : 1); k++)
    {
        for (int j = 0; j < this->NbrOfLines; j++)
        {
            for (int i = 0; i < this->earth_views_per_scanline; i++)
            {
                x = *(this->projectionCoordX.data() + j * this->earth_views_per_scanline + i);
                y = *(this->projectionCoordY.data() + j * this->earth_views_per_scanline + i);
                if(x >= 0 && x < imageptrs->ptrimageProjection->width() && y >= 0 && y < imageptrs->ptrimageProjection->height())
                {
                    if(normalized) pixval[k] = this->ptrbaVIInormalized[k][j * earth_views_per_scanline + i];
                    else pixval[k] = this->ptrbaVII[k][j * earth_views_per_scanline + i];

                    if(pixval[k] >= statmax[k])
                        statmax[k] = pixval[k];
                    if(pixval[k] < statmin[k])
                        statmin[k] = pixval[k];
                    activepixels[k]++;
                }
            }
        }
    }

    for(int k = 0; k < 3; k++)
    {
        stat_max_projection[k] = statmax[k];
        stat_min_projection[k] = statmin[k];
        qDebug() << QString("stat_min_projection[%1] = %2 stat_max_projection[%3] = %4").arg(k).arg(stat_min_projection[k]).arg(k).arg(stat_max_projection[k]);
    }
    active_pixels_projection = activepixels[0];
    qDebug() << QString("active_pixels_projection = %1").arg(active_pixels_projection);

}

void SegmentVII::RecalculateProjection(bool normalized)
{

    quint16 indexout[3];
    quint16 pixval[3];
    int r8, g8, b8;
    int r10, g10, b10;
    QRgb rgbvalue = qRgb(0,0,0);

    int map_x, map_y;

    bool iscolor = bandlist.at(0);

    for( int j = 0; j < this->NbrOfLines; j++)
    {
        for( int i = 0; i < this->earth_views_per_scanline ; i++ )
        {
            for(int k = 0; k < (iscolor ? 3 : 1); k++)
            {
                if(normalized) pixval[k] = this->ptrbaVIInormalized[k][j * earth_views_per_scanline + i];
                else pixval[k] = this->ptrbaVII[k][j * earth_views_per_scanline + i];
            }

            map_x = projectionCoordX[j * this->earth_views_per_scanline + i];
            map_y = projectionCoordY[j * this->earth_views_per_scanline + i];

            if (map_x > -15 && map_x < imageptrs->ptrimageProjection->width() + 15 && map_y > -15 && map_y < imageptrs->ptrimageProjection->height() + 15)
            {
                for(int k = 0; k < (iscolor ? 3 : 1); k++)
                {
                    indexout[k] =  (quint16)qMin(qMax((qRound(1023.0 * (float)( pixval[k] - imageptrs->stat_min_proj_ch[k] ) / (float)(imageptrs->stat_max_proj_ch[k] - imageptrs->stat_min_proj_ch[k]))), 0), 1023);
                }

                if(iscolor)
                {
                    if(invertthissegment[0])
                    {
                        r8 = 255 - imageptrs->lut_proj_ch[0][indexout[0]]/4;
                        r10 = 1023 - imageptrs->lut_proj_ch[0][indexout[0]];
                    }
                    else
                    {
                        r8 = imageptrs->lut_proj_ch[0][indexout[0]]/4;
                        r10 = imageptrs->lut_proj_ch[0][indexout[0]];
                    }

                    if(invertthissegment[1])
                    {
                        g8 = 255 - imageptrs->lut_proj_ch[1][indexout[1]]/4;
                        g10 = 1023 - imageptrs->lut_proj_ch[1][indexout[1]];
                    }
                    else
                    {
                        g8 = imageptrs->lut_proj_ch[1][indexout[1]]/4;
                        g10 = imageptrs->lut_proj_ch[1][indexout[1]];
                    }

                    if(invertthissegment[2])
                    {
                        b8 = 255 - imageptrs->lut_proj_ch[2][indexout[2]]/4;
                        b10 = 1023 - imageptrs->lut_proj_ch[2][indexout[2]];
                    }
                    else
                    {
                        b8 = imageptrs->lut_proj_ch[2][indexout[2]]/4;
                        b10 = imageptrs->lut_proj_ch[2][indexout[2]];
                    }

                    rgbvalue = qRgba(r8, g8, b8, 255);

                }
                else
                {
                    if(invertthissegment[0])
                    {
                        r8 = 255 - imageptrs->lut_proj_ch[0][indexout[0]]/4;
                        r10 = 1023 - imageptrs->lut_proj_ch[0][indexout[0]];
                    }
                    else
                    {
                        r8 = imageptrs->lut_proj_ch[0][indexout[0]]/4;
                        r10 = imageptrs->lut_proj_ch[0][indexout[0]];
                    }

                    g10 = r10;
                    b10 = r10;
                    rgbvalue = qRgba(r8, r8, r8, 255);
                }

                if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                projectionCoordValue[j * earth_views_per_scanline + i] = rgbvalue;
                projectionCoordValueRed[j * earth_views_per_scanline + i] = r10;
                projectionCoordValueGreen[j * earth_views_per_scanline + i] = g10;
                projectionCoordValueBlue[j * earth_views_per_scanline + i] = b10;
            }
        }
    }

}

void SegmentVII::resetMemory()
{
    Segment::resetMemory();

    latitude.reset();
    longitude.reset();
    duplicationmask.clear();
    latlonline.clear();
}

SegmentVII::~SegmentVII()
{
    qDebug() << "Destructor SegmentVII " << this->fileInfo.baseName();
}
