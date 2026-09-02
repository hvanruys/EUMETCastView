#include "segmentvii.h"
#include "segmentimage.h"
#include "options.h"
#include "rayleigh.h"
#include "rayleigh_rt.h"
#include "landseamask.h"
#include <QDebug>
#include <QtConcurrent>
#include <QFile>
#include <algorithm>
#include <cmath>

extern Options opts;
extern SegmentImage *imageptrs;
extern SatelliteList satellitelist;

// Where the land/sea mask of the Rayleigh correction comes from. The same two
// settings, in the same order, that the geostationary recipes resolve it from:
// the mask file if one is configured, otherwise the globe's own shoreline file.
static QString viiShorelineFile()
{
    auto resolve = [](const QString &p) {
        return (p.isEmpty() || opts.appdir_env.isEmpty()) ? p
                                                          : opts.appdir_env + "/" + p;
    };

    QString file = resolve(opts.gshhsmask);
    if(file.isEmpty() || !QFileInfo::exists(file))
        file = resolve(opts.gshhsglobe1);

    return file;
}

// VII scans from the port side: pixel 0 lies 90 degrees to the left of the
// flight direction, measured the same on ascending and descending passes alike.
// The composed image runs the flight direction down the page, which puts that
// edge on the right, so leaving the array order alone draws the swath mirrored.
// Reversing across track here rather than at draw time keeps the image, the
// globe texture, the graticule, searchLatLon and the 48-bit PNG on one indexing
// convention, and leaves the projections untouched, since the radiances and
// their geolocation turn together.
template<typename T>
static void reverseAcrossTrack(QVector<T> *v, int rows, int cols)
{
    T *p = v->data();
    for(int r = 0; r < rows; r++)
        std::reverse(p + (qsizetype)r * cols, p + (qsizetype)(r + 1) * cols);
}

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
    BZFILE* b;
    int     bzerror;
    int     nBuf;
    char    buf[ 32768 ];

    bool tempfileexist;

    bool iscolorimage = this->bandlist.at(0);

    QString basename = this->fileInfo.baseName() + ".nc";
    QFile tfile(basename);
    tempfileexist = tfile.exists();

    qDebug() << QString("file %1  tempfileexist = %2").arg(basename).arg(tempfileexist);
    qDebug() << "Segment *SegmentVII::ReadSegmentInMemory()";

    QDir dir(opts.temporarydir);
    if (!dir.mkpath(".")) {
        qWarning() << "Cannot create directory" << opts.temporarydir;
        return this;
    }

    if(this->fileInfo.completeSuffix() == "nc.bz2")
    {
        QFile fileout(dir.filePath(basename));
        if (!fileout.open(QIODevice::WriteOnly)) {
            qWarning() << "Cannot open" << fileout.fileName() << fileout.errorString();
            return this;
        }
        QDataStream streamout(&fileout);

        // QFile fileout(opts.temporarydir + basename);
        // fileout.open(QIODevice::WriteOnly);
        // QDataStream streamout(&fileout);


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
    else if(this->fileInfo.completeSuffix() == "nc")
    {
        QFile::copy(this->fileInfo.absoluteFilePath(), dir.filePath(basename));
    }


    if(!reader.open(dir.filePath(basename)))
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

    if(!ReadGeolocation(geom))
    {
        reader.close();
        return this;
    }

    // sec(SZA), the same normalization SegmentOLCI does from tie_geometries.nc
    QVector<float> sza;
    QScopedArrayPointer<float> secSZA(new float[npix]);
    if(reader.interpolateTiePointVariable(QStringLiteral("solar_zenith"), &sza))
    {
        reverseAcrossTrack(&sza, geom.nlines, geom.npixels);
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

        reverseAcrossTrack(&rad, geom.nlines, geom.npixels);

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

// Rebuild the full resolution geolocation from the tie point grid, publish it,
// derive the graticule from it and read the bow-tie duplication mask.
bool SegmentVII::ReadGeolocation(const ViiGeometry &geom)
{
    const int npix = geom.pixelCount();

    // The product only carries geolocation on the tie point grid, so every
    // full resolution position has to be reconstructed from it.
    QVector<float> lat, lon;
    if(!reader.interpolateGeolocation(&lat, &lon))
    {
        qCritical() << "SegmentVII::ReadGeolocation " << reader.lastError();
        return false;
    }

    if(opts.viidemorthorectify)
    {
        if(!reader.orthorectify(&lat, &lon))
            qDebug() << "SegmentVII::ReadGeolocation no DEM orthorectification : " << reader.lastError();
    }

    reverseAcrossTrack(&lat, geom.nlines, geom.npixels);
    reverseAcrossTrack(&lon, geom.nlines, geom.npixels);

    geolatitude.reset(new float[npix]);
    geolongitude.reset(new float[npix]);
    memcpy(geolatitude.data(), lat.constData(), npix * sizeof(float));
    memcpy(geolongitude.data(), lon.constData(), npix * sizeof(float));

    CalcOverlayLatLon();

    if(reader.readDuplicationMask(&duplicationmask))
        reverseAcrossTrack(&duplicationmask, geom.nd, geom.npixels);
    else
        qDebug() << "SegmentVII::ReadGeolocation " << reader.lastError();

    return true;
}

/**
 * Put every solar channel of a recipe into the unit it is stretched against, in
 * place, and optionally take the molecular haze off it.
 *
 * Two separable things, and the caller decides only the second:
 *
 * - The sun normalisation always runs. Amplify by 1/cos(sza) and carry the last
 *   degrees before the terminator to black. This is what turns a top of
 *   atmosphere reflectance into the bidirectional reflectance factor every
 *   recipe's 0..1 range is written for, so it is not optional for any of them.
 *
 * - The de-hazing runs when dehaze is set. Subtract the modelled Rayleigh path
 *   reflectance and undo the two-way transmittance the subtraction leaves
 *   behind. Worth doing only where the optical depth is large enough to see,
 *   which among these recipes means True Color and nothing else.
 *
 * Both halves freeze at RayleighCorrector::SzaLimit, so past the terminator the
 * amplification and the path reflectance keep describing the same sun.
 *
 * The chain and its constants are the ones the FCI and SEVIRI recipes use. What
 * differs is only where the geometry comes from - VII carries the four angles in
 * the product, on the same tie point grid as the geolocation, so none of it has
 * to be derived from an orbit model.
 *
 * The path term is not tapered with solar zenith. RayleighCorrector::pathTrust
 * exists for that and is calibrated on MSG, whose bluest channel sits at an
 * optical depth of 0.054; VII's is at 0.236, near FCI's, and on FCI that taper
 * turned the twilight blue by leaving in the very haze it was meant to remove.
 *
 * saa, vza and vaa are read only when dehaze is set and are not touched
 * otherwise; sza is always required.
 */
void SegmentVII::ApplySolarCorrection(QList<QVector<float> > &bandbuf,
                                      const QStringList &bandnames, bool dehaze,
                                      const QVector<float> &sza, const QVector<float> &saa,
                                      const QVector<float> &vza, const QVector<float> &vaa)
{
    // A brightness temperature is neither divided by the sun nor scattered by
    // the air, so the thermal channels sit this out.
    // Buffer pointers are resolved here rather than in the pixel loop: indexing a
    // QList or a QVector through its mutable operator[] detaches, and the loop
    // below runs on every thread at once.
    QVector<float *> solarData;
    QVector<const RayleighRT::Solution*> solarSol;

    const float *maskData  = nullptr;
    double maskLambda = 0.0;
    const RayleighRT::Solution *maskSol = nullptr;

    for(int k = 0; k < bandnames.count(); k++)
    {
        const QString &name = bandnames.at(k);
        if(!ViiL1BReader::isSolarChannel(name))
            continue;

        solarData.append(bandbuf[k].data());

        if(!dehaze)
            continue;

        const double lambda = ViiL1BReader::centreWavelength(name);
        const RayleighRT::Solution &s =
            RayleighRT::forTau(RayleighCorrector::opticalDepthAt(lambda));
        solarSol.append(&s);

        // Longest wavelength drives the water test: water is far darker than
        // land toward the red, and that is the term deciding whether the sea
        // surface reflects sky into the view.
        if(lambda > maskLambda)
        {
            maskLambda = lambda;
            maskData   = solarData.last();
            maskSol    = &s;
        }
    }

    if(solarData.isEmpty())
        return;

    bool useOcean = false;
    bool haveGeoMask = false;

    if(dehaze)
    {
        useOcean = maskLambda >= RayleighCorrector::MinWaterTestLambda;
        if(!useOcean)
            qWarning() << "VII Rayleigh: longest solar channel in this recipe is too blue"
                       << "to separate water from land; sea surface not modelled";

        // Geography decides land from sea; brightness is then left with only the
        // question it is good at, whether cloud is sitting on top of the sea.
        const QString gshhsFile = viiShorelineFile();
        const QByteArray gshhsPath = gshhsFile.toLocal8Bit();
        haveGeoMask = LandSeaMask::load(gshhsPath.constData());
        if(!haveGeoMask)
            qWarning() << "VII Rayleigh: no shoreline mask at" << gshhsFile
                       << "- falling back to the brightness test, which reads dark"
                       << "vegetation at high view angle as water";

        qDebug() << "VII solar correction: sun-normalising and de-hazing"
                 << solarData.size() << "channels, water test on" << maskLambda << "um";
    }
    else
        qDebug() << "VII solar correction: sun-normalising" << solarData.size()
                 << "channels, no de-hazing asked for";

    const int cols = earth_views_per_scanline;

    QVector<int> lines(NbrOfLines);
    for(int i = 0; i < NbrOfLines; i++)
        lines[i] = i;

    // This runs on a QtConcurrent worker already, so the pool is nested. That is
    // safe - waiting on a future from inside the pool makes the waiting thread
    // help run the remaining work rather than block on it - and it is worth
    // doing: the correction solves a radiative transfer problem per pixel, which
    // is by far the most expensive thing that happens to a granule.
    QtConcurrent::blockingMap(lines, [&](int line) {
        for(int pixelx = 0; pixelx < cols; pixelx++)
        {
            const int i = line * cols + pixelx;

            const float szaDeg = sza.at(i);
            if(std::isnan(szaDeg))
                continue;

            // The viewing geometry is only wanted by the path term, and is only
            // read when that is going to run.
            float vzaDeg = 0.0f;
            float raaDeg = 0.0f;
            if(dehaze)
            {
                vzaDeg = vza.at(i);
                if(std::isnan(vzaDeg) || std::isnan(saa.at(i)) || std::isnan(vaa.at(i)))
                    continue;

                // Relative azimuth folded into [0, 180]; the phase function is
                // even in it.
                raaDeg = fmodf(saa.at(i) - vaa.at(i), 360.0f);
                if(raaDeg < 0.0f)
                    raaDeg += 360.0f;
                if(raaDeg > 180.0f)
                    raaDeg = 360.0f - raaDeg;
            }

            // Both freeze at SzaLimit, so past the terminator the amplification
            // and the path reflectance keep describing the same sun.
            const float f = RayleighCorrector::sunZenithFactor(szaDeg);
            const float w = RayleighCorrector::twilightFade(szaDeg);

            // How watery the pixel is, decided before anything is corrected,
            // from the longest channel against a black lower boundary.
            float water = 0.0f;
            if(dehaze && useOcean && w > 0.0f && !std::isnan(maskData[i])
               && (!haveGeoMask
                   || (!std::isnan(geolatitude[i]) && !std::isnan(geolongitude[i])
                       && LandSeaMask::isWater(geolatitude[i], geolongitude[i]))))
            {
                const float mb = RayleighCorrector::surfaceReflectance(
                    *maskSol, szaDeg, vzaDeg,
                    maskData[i] * f
                        - RayleighCorrector::pathReflectance(
                              *maskSol, szaDeg, vzaDeg, raaDeg));
                water = haveGeoMask ? RayleighCorrector::cloudFreeFraction(mb)
                                    : RayleighCorrector::waterFraction(mb);
            }

            for(int k = 0; k < solarData.size(); k++)
            {
                float *buf = solarData.at(k);

                if(std::isnan(buf[i]))
                    continue;

                if(w == 0.0f)
                {
                    buf[i] = 0.0f;   // night
                    continue;
                }

                const float brf = buf[i] * f;

                if(!dehaze)
                {
                    buf[i] = w * brf;
                    continue;
                }

                const float rho = RayleighCorrector::pathReflectance(
                    *solarSol.at(k), szaDeg, vzaDeg, raaDeg, water);

                // Taking the path term off leaves the surface seen through the
                // atmosphere, not the surface. Undo the two-way transmittance
                // and the ground-to-sky bouncing to get there.
                buf[i] = w * RayleighCorrector::surfaceReflectance(
                    *solarSol.at(k), szaDeg, vzaDeg, brf - rho);
            }
        }
    });
}

Segment *SegmentVII::ReadSegmentRecipeInMemory(int recipe)
{
    BZFILE* b;
    int     bzerror;
    int     nBuf;
    char    buf[ 32768 ];

    bool tempfileexist;

    if(recipe < 0 || recipe >= imageptrs->vii_rgbrecipes.count())
    {
        qCritical() << "SegmentVII::ReadSegmentRecipeInMemory : no recipe" << recipe;
        return this;
    }

    const RGBRecipe &rec = imageptrs->vii_rgbrecipes.at(recipe);

    QString basename = this->fileInfo.baseName() + ".nc";
    QFile tfile(basename);
    tempfileexist = tfile.exists();

    qDebug() << QString("file %1  tempfileexist = %2").arg(basename).arg(tempfileexist);
    qDebug() << "Segment *SegmentVII::ReadSegmentInMemory()";

    QDir dir(opts.temporarydir);
    if (!dir.mkpath(".")) {
        qWarning() << "Cannot create directory" << opts.temporarydir;
        return this;
    }

    if(this->fileInfo.completeSuffix() == "nc.bz2")
    {
        QFile fileout(dir.filePath(basename));
        if (!fileout.open(QIODevice::WriteOnly)) {
            qWarning() << "Cannot open" << fileout.fileName() << fileout.errorString();
            return this;
        }
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
    else if(this->fileInfo.completeSuffix() == "nc")
    {
        QFile::copy(this->fileInfo.absoluteFilePath(), dir.filePath(basename));
    }

    if(!reader.open(dir.filePath(basename)))
    {
        qCritical() << "SegmentVII::ReadSegmentRecipeInMemory " << reader.lastError();
        return this;
    }

    const ViiGeometry geom = reader.geometry();
    this->earth_views_per_scanline = geom.npixels;
    this->NbrOfLines = geom.nlines;
    this->num_pixels_alt = geom.nd;

    // A recipe is always a colour image and carries its own stretch, so no
    // colour of it is ever inverted.
    for(int k = 0; k < 3; k++)
        invertthissegment[k] = false;

    this->initializeMemory();

    const int npix = geom.pixelCount();

    // 65535 is the no-data marker; setting it up front means an error further
    // down leaves a transparent segment instead of uninitialised memory
    for(int k = 0; k < 3; k++)
    {
        for(int i = 0; i < npix; i++)
        {
            ptrbaVII[k][i] = 65535;
            ptrbaVIInormalized[k][i] = 65535;
        }
    }

    if(!ReadGeolocation(geom))
    {
        reader.close();
        return this;
    }

    // Every channel the three colours name, plus the ones a composite depends on
    // without their being a colour of their own. Named once each: True Color
    // names three different channels, the O2-A index two, and a difference
    // recipe would name one of its windows twice.
    QStringList bandnames;
    for(int ci = 0; ci < 3; ci++)
    {
        const RGBRecipeColor &col = rec.Colorvector.at(ci);
        for(const QString &c : col.channels)
            if(!bandnames.contains(c))
                bandnames << c;
    }
    for(const QString &c : rec.auxchannels)
        if(!bandnames.contains(c))
            bandnames << c;

    qDebug() << QString("SegmentVII::ReadSegmentRecipeInMemory %1 x %2 recipe '%3' channels %4")
                .arg(earth_views_per_scanline).arg(NbrOfLines)
                .arg(rec.Name, bandnames.join(' '));

    QList<QVector<float> > bandbuf;
    bool hassolar = false;

    for(const QString &c : std::as_const(bandnames))
    {
        const bool solar = ViiL1BReader::isSolarChannel(c);
        hassolar = hassolar || solar;

        QVector<float> v;
        if(!(solar ? reader.readReflectance(c, &v)
                   : reader.readBrightnessTemperature(c, &v)))
        {
            qCritical() << "SegmentVII::ReadSegmentRecipeInMemory" << c
                        << reader.lastError();
            reader.close();
            return this;
        }

        reverseAcrossTrack(&v, geom.nlines, geom.npixels);
        bandbuf.append(v);
    }

    // Every recipe with a solar channel is sun-normalised, whatever the
    // preference says. That is not a correction but part of the unit the
    // recipes are written in - EUMETSAT's "0 to 100 %" is a bidirectional
    // reflectance factor - so skipping it would not make a hazier picture but
    // one darkened by the cosine of the solar zenith, which no recipe here is
    // stretched for.
    //
    // De-hazing is the optional half, and the preference governs only that.
    // What it is worth is not a matter of taste. Measured over a granule of
    // ocean and marine cloud, turning it on moves the finished image by this
    // many of the 255 levels it is drawn on, on average:
    //
    //     True Color 10.3, Cirrus 4.3, Natural Color 1.9,
    //     Day Land Cloud Fire 1.3, Fire Temperature 0.04
    //
    // which tracks the optical depth of each recipe's bluest channel: 0.236 at
    // 0.443 um, 0.044 at 0.668, 0.0003 at 2.25. Only True Color reaches deep
    // enough into the blue for the haze to decide anything - whether its ocean
    // is navy or milky - and it is the only one that asks for the removal. The
    // rest are left on top of atmosphere reflectance, which is how EUMETSAT
    // defines the ones that are standards.
    if(hassolar)
    {
        const bool dehaze = rec.rayleigh && opts.bViiRayleigh;

        // Without the de-hazing only the solar zenith is wanted, so the other
        // three interpolations - a full grid each - are not paid for.
        QVector<float> sza, saa, vza, vaa;
        bool havegeometry =
            reader.interpolateTiePointVariable(QStringLiteral("solar_zenith"), &sza);

        if(havegeometry && dehaze)
            havegeometry =
                reader.interpolateTiePointVariable(QStringLiteral("solar_azimuth"), &saa, true)
                && reader.interpolateTiePointVariable(QStringLiteral("observation_zenith"), &vza)
                && reader.interpolateTiePointVariable(QStringLiteral("observation_azimuth"), &vaa, true);

        if(havegeometry)
        {
            reverseAcrossTrack(&sza, geom.nlines, geom.npixels);
            if(dehaze)
            {
                reverseAcrossTrack(&saa, geom.nlines, geom.npixels);
                reverseAcrossTrack(&vza, geom.nlines, geom.npixels);
                reverseAcrossTrack(&vaa, geom.nlines, geom.npixels);
            }

            ApplySolarCorrection(bandbuf, bandnames, dehaze, sza, saa, vza, vaa);
        }
        else
            qWarning() << "SegmentVII::ReadSegmentRecipeInMemory no solar geometry ("
                       << reader.lastError()
                       << ") - composing the reflectance as it stands, which will"
                       << "be dark away from the subsolar point";
    }

    reader.close();

    // Combine the channels into the three colours.
    QVector<float> result[3];
    for(int ci = 0; ci < 3; ci++)
        result[ci].fill(qQNaN(), npix);

    if(rec.compose == RECIPE_NORMDIFF)
    {
        // Smallest denominator an index is still allowed to have, in the
        // reflectance units the channels are held in.
        //
        // A normalised difference is a ratio and its noise is set by the
        // denominator alone, so the sum is the right thing to gate on. Below
        // this there is nothing in either channel but the night side noise, and
        // their ratio lands arbitrarily on +1 or -1 - black and white speckle
        // over what should be an empty swath.
        constexpr float MinIndexSignal = 0.01f;

        for(int ci = 0; ci < 3; ci++)
        {
            const RGBRecipeColor &col = rec.Colorvector.at(ci);
            QVector<const float *> src;
            QVector<float> sign;
            for(int k = 0; k < col.channels.count(); k++)
            {
                src.append(bandbuf.at(bandnames.indexOf(col.channels.at(k))).constData());
                sign.append(col.subtract.at(k) ? -1.0f : 1.0f);
            }

            float *out = result[ci].data();
            for(int i = 0; i < npix; i++)
            {
                float num = 0.0f, den = 0.0f;
                bool ok = true;
                for(int k = 0; k < src.count(); k++)
                {
                    const float v = src.at(k)[i];
                    if(std::isnan(v)) { ok = false; break; }
                    num += sign.at(k) * v;
                    den += v;
                }
                out[i] = (ok && den > MinIndexSignal) ? num / den : qQNaN();
            }
        }
    }
    else
    {
        for(int ci = 0; ci < 3; ci++)
        {
            const RGBRecipeColor &col = rec.Colorvector.at(ci);
            float *out = result[ci].data();

            for(int k = 0; k < col.channels.count(); k++)
            {
                const float *src = bandbuf.at(bandnames.indexOf(col.channels.at(k))).constData();
                const bool subtract = col.subtract.at(k);

                for(int i = 0; i < npix; i++)
                {
                    if(std::isnan(src[i]))
                    {
                        out[i] = qQNaN();
                        continue;
                    }
                    if(std::isnan(out[i]))
                        out[i] = subtract ? -src[i] : src[i];
                    else
                        out[i] += subtract ? -src[i] : src[i];
                }
            }
        }
    }

    bandbuf.clear();

    // The recipe's own stretch. An index recipe stops one short of full scale so
    // that 255 stays free to mean no value, and rounds rather than truncates -
    // the number carries meaning of its own, it is not just how bright a pixel
    // looks.
    const float outmax = (rec.compose == RECIPE_NORMDIFF) ? 254.0f : 255.0f;
    const float bias   = (rec.compose == RECIPE_NORMDIFF) ? 0.5f : 0.0f;

    // What comes out is a brightness, but it is stored on the radiance scale the
    // statistics are pinned to, so the compose and projection paths hand it back
    // unchanged at a 100 % stretch instead of needing a path of their own.
    //
    // The factor is 4 * 65534 / 1023, not the 65534 / 255 that spans the same
    // range, and the difference is not rounding slack. Those paths go through a
    // 1024 level intermediate and then divide by a literal 4, so a level only
    // survives the trip if it lands on a multiple of 4 there - and 1023 / 255 is
    // 4.0118, not 4. Scaled the obvious way the drift reaches half a level by
    // the middle of the range and stays there: 127 of the 256 levels come back
    // one too bright. Scaled this way every one of the 256 is exact, at the cost
    // of stopping at 65342 rather than 65534.
    const double tostore = 4.0 * 65534.0 / 1023.0;

    for(int i = 0; i < npix; i++)
    {
        if(std::isnan(result[0].at(i)) || std::isnan(result[1].at(i))
           || std::isnan(result[2].at(i)))
            continue;   // stays at the no-data marker set above

        for(int k = 0; k < 3; k++)
        {
            const RGBRecipeColor &col = rec.Colorvector.at(k);
            const float from = col.rangefrom;
            const float to   = col.rangeto;

            float val = result[k].at(i);
            if(val < from) val = from;
            if(val > to)   val = to;

            const float norm = (to != from) ? (val - from) / (to - from) : 0.0f;
            float gv = outmax * powf(norm, 1.0f / col.gamma);
            if(!col.inverse.isEmpty() && col.inverse.at(0))
                gv = outmax - gv;

            const int v8 = (int)qBound(0.0f, gv + bias, outmax);
            ptrbaVII[k][i] = (quint16)qRound(v8 * tostore);
            ptrbaVIInormalized[k][i] = ptrbaVII[k][i];
        }
    }

    // The recipe fixed the stretch, so the statistics must not be allowed to
    // move it again. Pinning them to the full scale is what makes the 100 %
    // path in ComposeSegmentImage and in the projections the identity, and it
    // holds for every segment alike, so segments of one image stay comparable.
    for(int k = 0; k < 3; k++)
    {
        stat_min_ch[k] = 0;
        stat_max_ch[k] = 65534;
        stat_min_norm_ch[k] = 0;
        stat_max_norm_ch[k] = 65534;
        active_pixels[k] = 0;
    }

    nbrsaturatedpixels = 0;

    for(int k = 0; k < 3; k++)
        for(int i = 0; i < npix; i++)
            if(ptrbaVII[k][i] < 65535)
                active_pixels[k]++;

    qDebug() << QString("SegmentVII recipe '%1' : %2 of %3 pixels have a colour")
                .arg(rec.Name).arg(active_pixels[0]).arg(npix);

    return this;
}

// Mark every pixel where the latitude or the longitude crosses a multiple of
// graticulestep degrees, which traces the parallels and meridians across the swath.
void SegmentVII::CalcOverlayLatLon()
{
    const float graticulestep = 5.0f;

    latlonline.clear();

    if(geolatitude.isNull() || geolongitude.isNull())
        return;

    for (int line = 1; line < NbrOfLines; line++)
    {
        for (int pixelx = 1; pixelx < earth_views_per_scanline; pixelx++)
        {
            const int idx = line * earth_views_per_scanline + pixelx;
            const float lat = geolatitude[idx];
            const float lon = geolongitude[idx];
            if(std::isnan(lat) || std::isnan(lon))
                continue;

            const float latleft = geolatitude[idx - 1];
            const float lonleft = geolongitude[idx - 1];
            const float latup = geolatitude[idx - earth_views_per_scanline];
            const float lonup = geolongitude[idx - earth_views_per_scanline];
            if(std::isnan(latleft) || std::isnan(lonleft) || std::isnan(latup) || std::isnan(lonup))
                continue;

            // the dateline puts a 360 degree step between neighbours, which is
            // not a meridian crossing
            const bool lonwrap = fabs(lon - lonleft) > 180.0f || fabs(lon - lonup) > 180.0f;

            if(floor(lat / graticulestep) != floor(latleft / graticulestep)
               || floor(lat / graticulestep) != floor(latup / graticulestep)
               || (!lonwrap && (floor(lon / graticulestep) != floor(lonleft / graticulestep)
                                || floor(lon / graticulestep) != floor(lonup / graticulestep))))
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
            flat = geolatitude[nbrLine * earthviews + pix];
            flon = geolongitude[nbrLine * earthviews + pix];
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
                    else // 100%, and any method with no stretch of its own
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
                        else
                        {
                            // CLAHE arrives here: SegmentListVII composes a
                            // plain stretch first and replaces the whole image
                            // afterwards. Leaving colour unset would have been
                            // an uninitialised read.
                            color[k] = (quint16)qMin(qMax(qRound((float)indexout[k]/4), 0), 255);
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

void SegmentVII::ComposeSegmentOMProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(OM, histogrammethod, normalized);
}

// First and last usable centre pixel of the segment. The oblique mercator puts
// its central line through these, so they have to follow the ground track.
void SegmentVII::getCentralCoords(double *startlon, double *startlat, double *endlon, double *endlat)
{
    *startlon = 65535.0;
    *startlat = 65535.0;
    *endlon = 65535.0;
    *endlat = 65535.0;

    if(geolatitude.isNull() || geolongitude.isNull())
        return;

    const int centre = earth_views_per_scanline / 2;

    for(int i = 0; i < this->NbrOfLines; i++)
    {
        const float lo = geolongitude[i * earth_views_per_scanline + centre];
        const float la = geolatitude[i * earth_views_per_scanline + centre];
        if(!std::isnan(lo) && !std::isnan(la))
        {
            *startlon = lo;
            *startlat = la;
            break;
        }
    }

    for(int i = this->NbrOfLines - 1; i >= 0; i--)
    {
        const float lo = geolongitude[i * earth_views_per_scanline + centre];
        const float la = geolatitude[i * earth_views_per_scanline + centre];
        if(!std::isnan(lo) && !std::isnan(la))
        {
            *endlon = lo;
            *endlat = la;
            break;
        }
    }
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

            latpos1 = geolatitude[i * earth_views_per_scanline + j];
            lonpos1 = geolongitude[i * earth_views_per_scanline + j];

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
                else if(proj == OM) // Oblique Mercator
                {
                    if(imageptrs->om->map_forward(lonpos1 * PIE / 180.0, latpos1 * PIE / 180.0, map_x, map_y))
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
            else // 100%, and any method with no stretch of its own
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
                else
                {
                    color8[k] = 255 - (quint16)qMin(qMax(qRound((float)indexout[k]/4), 0), 255);
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
                else
                {
                    color8[k] = (quint16)qMin(qMax(qRound((float)indexout[k]/4), 0), 255);
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

    geolatitude.reset();
    geolongitude.reset();
    duplicationmask.clear();
    latlonline.clear();
}

SegmentVII::~SegmentVII()
{
    qDebug() << "Destructor SegmentVII " << this->fileInfo.baseName();
}
