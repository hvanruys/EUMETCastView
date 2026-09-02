#include "viil1breader.h"

#include <QDebug>
#include <QFile>
#include <QtMath>

#include <netcdf.h>
#include <algorithm>
#include <cmath>

namespace {

/* WGS84, used by PFS Equations 7-10 */
constexpr double kWgs84A = 6378137.0;
constexpr double kWgs84F = 1.0 / 298.257223563;

/* PFS 4.2.4.1.3.4: the L1B processor uses this fixed radius for the
   delta_lat_N_dem / delta_lon_E_dem fields. */
constexpr double kFixedRadius = 6371000.0;

/* The 20 VII channels in the order they appear in the product, which is also
   the order of the band radio buttons on the VII tab. */
const char * const kChannels[20] = {
    "vii_443",  "vii_555",  "vii_668",   "vii_752",   "vii_763",
    "vii_865",  "vii_914",  "vii_1240",  "vii_1375",  "vii_1630",
    "vii_2250", "vii_3740", "vii_3959",  "vii_4050",  "vii_6725",
    "vii_7325", "vii_8540", "vii_10690", "vii_12020", "vii_13345"
};

/* The split the calibration arrays are indexed by: the first eleven channels
   are solar, the last nine thermal. */
constexpr int kSolarChannels   = 11;
constexpr int kThermalChannels = 9;

/* Planck constants for a radiance in W.m^-2.sr^-1.um^-1 and a wavelength in um:
   c1 = 2hc^2, c2 = hc/k. */
constexpr double kPlanckC1 = 1.191042e8;    /* W.um^4.m^-2.sr^-1 */
constexpr double kPlanckC2 = 1.4387752e4;   /* um.K              */

inline void geodeticToEcef(double latDeg, double lonDeg,
                           double *x, double *y, double *z)
{
    const double e2  = kWgs84F * (2.0 - kWgs84F);
    const double phi = qDegreesToRadians(latDeg);
    const double lam = qDegreesToRadians(lonDeg);
    const double s   = std::sin(phi);
    const double N   = kWgs84A / std::sqrt(1.0 - e2 * s * s);

    *x = N * std::cos(phi) * std::cos(lam);
    *y = N * std::cos(phi) * std::sin(lam);
    *z = (1.0 - e2) * N * s;
}

/* Bowring / Hofmann-Wellenhof non-iterative inverse (PFS Eq. 9/10).
   atan2(y,x) is the same value as the half-angle form printed in Eq. 9. */
inline void ecefToGeodetic(double x, double y, double z,
                           double *latDeg, double *lonDeg)
{
    const double a   = kWgs84A;
    const double e2  = kWgs84F * (2.0 - kWgs84F);
    const double b   = a * (1.0 - kWgs84F);
    const double ep2 = (a * a - b * b) / (b * b);

    const double p  = std::sqrt(x * x + y * y);
    const double th = std::atan2(z * a, p * b);
    const double st = std::sin(th), ct = std::cos(th);

    *lonDeg = qRadiansToDegrees(std::atan2(y, x));
    *latDeg = qRadiansToDegrees(std::atan2(z + ep2 * b * st * st * st,
                                           p - e2  * a  * ct * ct * ct));
}

} // namespace

// ---------------------------------------------------------------------------

bool ViiGeometry::isConsistent() const
{
    return nscans > 0 && nd > 0 && zact > 0 && zalt > 0
        && nlines  == nscans * nd
        && ntieAlt == 4 * nscans
        && ntieAct == npixels / zact + 1
        && nd      == 3 * zalt;
}

QString ViiGeometry::toString() const
{
    return QStringLiteral(
               "%1 scans x %2 lines = %3 lines, %4 pixels; "
               "tie grid %5 x %6, zones %7 x %8")
        .arg(nscans).arg(nd).arg(nlines).arg(npixels)
        .arg(ntieAlt).arg(ntieAct).arg(zalt).arg(zact);
}

// ---------------------------------------------------------------------------

ViiL1BReader::ViiL1BReader()
{
}

ViiL1BReader::~ViiL1BReader()
{
    close();
}

void ViiL1BReader::close()
{
    if (m_ncid >= 0) {
        nc_close(m_ncid);
        m_ncid = m_measGid = m_dataGid = m_flagsGid = -1;
        m_calGid = m_satGid = -1;
    }
    m_fileName.clear();
    m_geom = ViiGeometry();

    m_calRead = false;
    m_solarIrradiance.clear();
    m_thermalCw.clear();
    m_thermalA.clear();
    m_thermalB.clear();
    m_sunEarthRatio = 1.0;
}

bool ViiL1BReader::open(const QString &path)
{
    close();
    m_error.clear();

    const QByteArray p = QFile::encodeName(path);
    int rc = nc_open(p.constData(), NC_NOWRITE, &m_ncid);
    if (rc != NC_NOERR) {
        m_ncid  = -1;
        m_error = QStringLiteral("cannot open %1: %2")
                      .arg(path, QString::fromLatin1(nc_strerror(rc)));
        return false;
    }

    rc = nc_inq_grp_full_ncid(m_ncid, "/data", &m_dataGid);
    if (rc == NC_NOERR)
        rc = nc_inq_grp_full_ncid(m_ncid, "/data/measurement_data", &m_measGid);
    if (rc != NC_NOERR) {
        m_error = QStringLiteral("not a VII L1B product (no /data/measurement_data): %1")
                      .arg(QString::fromLatin1(nc_strerror(rc)));
        close();
        return false;
    }

    /* the processing flags are optional: a product without them still gives a
       usable image, it just cannot mask the bow-tie duplicates */
    if (nc_inq_grp_full_ncid(m_ncid, "/data/processing_flags", &m_flagsGid) != NC_NOERR)
        m_flagsGid = -1;

    /* likewise the calibration: without it the radiances still make a picture,
       only not one in reflectance or brightness temperature */
    if (nc_inq_grp_full_ncid(m_ncid, "/data/calibration_data", &m_calGid) != NC_NOERR)
        m_calGid = -1;
    if (nc_inq_grp_full_ncid(m_ncid, "/status/satellite", &m_satGid) != NC_NOERR)
        m_satGid = -1;

    if (!readGeometry()) {
        close();
        return false;
    }

    m_fileName = path;
    qDebug() << "ViiL1BReader:" << m_geom.toString();
    return true;
}

int ViiL1BReader::dimLength(const char *name, bool *ok) const
{
    int    dimid = -1;
    size_t len   = 0;
    /* nc_inq_dimid searches ancestor groups, so num_lines / num_pixels
       resolve from measurement_data even though they live in /data */
    if (nc_inq_dimid(m_measGid, name, &dimid) != NC_NOERR
        || nc_inq_dimlen(m_measGid, dimid, &len) != NC_NOERR) {
        *ok = false;
        return 0;
    }
    return static_cast<int>(len);
}

bool ViiL1BReader::readGeometry()
{
    bool ok = true;
    ViiGeometry g;
    g.nscans  = dimLength("num_scans",          &ok);
    g.nd      = dimLength("num_pixels_alt",     &ok);
    g.nlines  = dimLength("num_lines",          &ok);
    g.npixels = dimLength("num_pixels",         &ok);
    g.zact    = dimLength("zone_size_act",      &ok);
    g.zalt    = dimLength("zone_size_alt",      &ok);
    g.ntieAct = dimLength("num_tie_points_act", &ok);
    g.ntieAlt = dimLength("num_tie_points_alt", &ok);

    if (!ok) {
        m_error = QStringLiteral("missing one of the expected VII dimensions");
        return false;
    }
    if (!g.isConsistent()) {
        m_error = QStringLiteral("inconsistent geometry: %1").arg(g.toString());
        return false;
    }
    m_geom = g;
    return true;
}

QStringList ViiL1BReader::radianceChannels() const
{
    QStringList out;
    if (m_measGid < 0)
        return out;

    int nvars = 0;
    if (nc_inq_varids(m_measGid, &nvars, nullptr) != NC_NOERR)
        return out;

    QVector<int> ids(nvars);
    nc_inq_varids(m_measGid, &nvars, ids.data());
    for (int id : ids) {
        char name[NC_MAX_NAME + 1] = "";
        if (nc_inq_varname(m_measGid, id, name) != NC_NOERR)
            continue;
        const QString n = QString::fromLatin1(name);
        if (n.startsWith(QLatin1String("vii_")))
            out.append(n);
    }
    std::sort(out.begin(), out.end(), [](const QString &a, const QString &b) {
        return a.mid(4).toInt() < b.mid(4).toInt();
    });
    return out;
}

QString ViiL1BReader::channelVariableName(int band)
{
    if (band < 1 || band > 20)
        return QString();
    return QString::fromLatin1(kChannels[band - 1]);
}

int ViiL1BReader::channelIndex(const QString &name)
{
    for (int i = 0; i < 20; ++i)
        if (name == QLatin1String(kChannels[i]))
            return i;
    return -1;
}

double ViiL1BReader::centreWavelength(const QString &name)
{
    /* the name carries it: vii_443 is 0.443 um, vii_10690 is 10.690 um */
    return channelIndex(name) < 0 ? 0.0 : name.mid(4).toDouble() / 1000.0;
}

bool ViiL1BReader::isSolarChannel(const QString &name)
{
    const int i = channelIndex(name);
    return i >= 0 && i < kSolarChannels;
}

// ---------------------------------------------------------------------------
// Calibration: radiance into reflectance and brightness temperature
// ---------------------------------------------------------------------------

bool ViiL1BReader::readCalibration()
{
    if (m_calRead)
        return true;

    if (m_calGid < 0) {
        m_error = QStringLiteral("no /data/calibration_data in this product");
        return false;
    }

    auto readArray = [this](const char *name, int n, QVector<double> *out) {
        int varid = -1;
        if (nc_inq_varid(m_calGid, name, &varid) != NC_NOERR)
            return false;
        out->resize(n);
        return nc_get_var_double(m_calGid, varid, out->data()) == NC_NOERR;
    };

    /* The operational product spells the solar irradiance
       band_averaged_solar_irradiance; the pre-launch test products this reader
       was first written against capitalise the B. It is the only variable the
       reader needs that was renamed, so it is the only one that needs both
       spellings - and without it no channel calibrates, solar or thermal. */
    const bool haveSolar =
        readArray("band_averaged_solar_irradiance", kSolarChannels, &m_solarIrradiance)
        || readArray("Band_averaged_solar_irradiance", kSolarChannels, &m_solarIrradiance);

    if (!haveSolar
        || !readArray("channel_cw_thermal",  kThermalChannels, &m_thermalCw)
        || !readArray("bt_conversion_a",     kThermalChannels, &m_thermalA)
        || !readArray("bt_conversion_b",     kThermalChannels, &m_thermalB)) {
        m_error = QStringLiteral("incomplete /data/calibration_data");
        return false;
    }

    /* Ratio of the current earth-sun distance to the mean one. The band
       averaged irradiance is quoted at the mean distance, so the reflectance
       has to be scaled by its square. Missing is not fatal - it moves the
       reflectance by at most 3.4 % over a year - so fall back to 1. */
    m_sunEarthRatio = 1.0;
    if (m_satGid >= 0) {
        int varid = -1;
        double d = 0.0;
        if (nc_inq_varid(m_satGid, "earth_sun_distance_ratio", &varid) == NC_NOERR
            && nc_get_var_double(m_satGid, varid, &d) == NC_NOERR
            && d > 0.9 && d < 1.1)
            m_sunEarthRatio = d;
        else
            qWarning() << "ViiL1BReader: no usable earth_sun_distance_ratio,"
                       << "reflectance is quoted at the mean earth-sun distance";
    }

    m_calRead = true;
    return true;
}

bool ViiL1BReader::readReflectance(const QString &name, QVector<float> *out)
{
    const int idx = channelIndex(name);
    if (idx < 0 || idx >= kSolarChannels) {
        m_error = QStringLiteral("%1 is not a solar channel").arg(name);
        return false;
    }
    if (!readCalibration())
        return false;

    const double e0 = m_solarIrradiance.at(idx);
    if (!(e0 > 0.0)) {
        m_error = QStringLiteral("no solar irradiance for %1").arg(name);
        return false;
    }

    if (!readFullGridVariable(name, out))
        return false;

    const double f = M_PI * m_sunEarthRatio * m_sunEarthRatio / e0;
    float *v = out->data();
    const int n = out->size();
    for (int i = 0; i < n; ++i)
        v[i] = (float)(v[i] * f);   /* NaN stays NaN */

    return true;
}

bool ViiL1BReader::readBrightnessTemperature(const QString &name, QVector<float> *out)
{
    const int idx = channelIndex(name);
    if (idx < kSolarChannels) {
        m_error = QStringLiteral("%1 is not a thermal channel").arg(name);
        return false;
    }
    if (!readCalibration())
        return false;

    const int t = idx - kSolarChannels;
    const double lambda = m_thermalCw.at(t);
    const double a      = m_thermalA.at(t);
    const double b      = m_thermalB.at(t);
    if (!(lambda > 0.0) || !(a > 0.0)) {
        m_error = QStringLiteral("no usable thermal calibration for %1").arg(name);
        return false;
    }

    if (!readFullGridVariable(name, out))
        return false;

    /* Inverse Planck for a radiance per micrometre, then the product's own
       linear correction for the width of the channel:  T = (T_planck - B) / A. */
    const double l5 = lambda * lambda * lambda * lambda * lambda;
    float *v = out->data();
    const int n = out->size();
    for (int i = 0; i < n; ++i) {
        const double L = v[i];
        /* a radiance at or below zero is noise around a cold scene, not a
           temperature; it has no inverse Planck */
        v[i] = (L > 0.0)
                 ? (float)((kPlanckC2 / (lambda * std::log1p(kPlanckC1 / (l5 * L))) - b) / a)
                 : qQNaN();
    }

    return true;
}

// ---------------------------------------------------------------------------
// Equation 1: unpacking
// ---------------------------------------------------------------------------

bool ViiL1BReader::unpack(int varid, QVector<double> *values) const
{
    /* nc_get_var_double converts int, uint and ushort alike, so one path
       covers latitude (NC_INT), longitude (NC_UINT) and vii_* (NC_USHORT).
       The C API never applies CF packing itself. */
    const int rc = nc_get_var_double(m_measGid, varid, values->data());
    if (rc != NC_NOERR)
        return false;

    double fill = 0.0, scale = 1.0, offset = 0.0;
    const bool hasFill =
        nc_get_att_double(m_measGid, varid, "_FillValue", &fill) == NC_NOERR;
    nc_get_att_double(m_measGid, varid, "scale_factor", &scale);
    nc_get_att_double(m_measGid, varid, "add_offset",  &offset);

    double *v = values->data();
    const int n = values->size();
    for (int i = 0; i < n; ++i) {
        /* the fill test must happen on the raw value, before scaling */
        v[i] = (hasFill && v[i] == fill) ? qQNaN() : v[i] * scale + offset;
    }
    return true;
}

bool ViiL1BReader::readTiePointVariable(const QString &name,
                                        QVector<double> *out)
{
    int varid = -1;
    const QByteArray n = name.toLatin1();
    if (nc_inq_varid(m_measGid, n.constData(), &varid) != NC_NOERR) {
        m_error = QStringLiteral("no such variable: %1").arg(name);
        return false;
    }
    out->resize(m_geom.tieCount());
    if (!unpack(varid, out)) {
        m_error = QStringLiteral("cannot read %1").arg(name);
        return false;
    }
    return true;
}

bool ViiL1BReader::readFullGridVariable(const QString &name,
                                        QVector<float> *out,
                                        bool applyValidRange)
{
    int varid = -1;
    const QByteArray n = name.toLatin1();
    if (nc_inq_varid(m_measGid, n.constData(), &varid) != NC_NOERR) {
        m_error = QStringLiteral("no such variable: %1").arg(name);
        return false;
    }

    QVector<double> raw(m_geom.pixelCount());
    /* read raw first so valid_min/valid_max can be tested in packed space,
       which is how the PFS states them */
    if (nc_get_var_double(m_measGid, varid, raw.data()) != NC_NOERR) {
        m_error = QStringLiteral("cannot read %1").arg(name);
        return false;
    }

    double fill = 0.0, scale = 1.0, offset = 0.0, vmin = 0.0, vmax = 0.0;
    const bool hasFill =
        nc_get_att_double(m_measGid, varid, "_FillValue", &fill) == NC_NOERR;
    const bool hasMin = applyValidRange &&
        nc_get_att_double(m_measGid, varid, "valid_min", &vmin) == NC_NOERR;
    const bool hasMax = applyValidRange &&
        nc_get_att_double(m_measGid, varid, "valid_max", &vmax) == NC_NOERR;
    nc_get_att_double(m_measGid, varid, "scale_factor", &scale);
    nc_get_att_double(m_measGid, varid, "add_offset",  &offset);

    out->resize(raw.size());
    float *o = out->data();
    const double *r = raw.constData();
    for (int i = 0; i < raw.size(); ++i) {
        const bool bad = (hasFill && r[i] == fill)
                      || (hasMin  && r[i] <  vmin)
                      || (hasMax  && r[i] >  vmax);
        o[i] = bad ? qQNaN() : static_cast<float>(r[i] * scale + offset);
    }
    return true;
}

bool ViiL1BReader::radianceRange(const QString &name, double *lo, double *hi)
{
    int varid = -1;
    const QByteArray n = name.toLatin1();
    if (nc_inq_varid(m_measGid, n.constData(), &varid) != NC_NOERR) {
        m_error = QStringLiteral("no such variable: %1").arg(name);
        return false;
    }

    double vmin = 0.0, vmax = 0.0, scale = 1.0, offset = 0.0;
    if (nc_get_att_double(m_measGid, varid, "valid_min", &vmin) != NC_NOERR
        || nc_get_att_double(m_measGid, varid, "valid_max", &vmax) != NC_NOERR) {
        m_error = QStringLiteral("%1 has no valid_min/valid_max").arg(name);
        return false;
    }
    nc_get_att_double(m_measGid, varid, "scale_factor", &scale);
    nc_get_att_double(m_measGid, varid, "add_offset",  &offset);

    *lo = vmin * scale + offset;
    *hi = vmax * scale + offset;
    if (*hi <= *lo) {
        m_error = QStringLiteral("%1 has an empty valid range").arg(name);
        return false;
    }
    return true;
}

bool ViiL1BReader::readDuplicationMask(QVector<quint8> *out)
{
    out->clear();
    if (m_flagsGid < 0) {
        m_error = QStringLiteral("product has no /data/processing_flags group");
        return false;
    }

    int varid = -1;
    if (nc_inq_varid(m_flagsGid, "pixel_duplication_mask", &varid) != NC_NOERR) {
        m_error = QStringLiteral("no pixel_duplication_mask in this product");
        return false;
    }

    out->resize(m_geom.nd * m_geom.npixels);
    if (nc_get_var_ubyte(m_flagsGid, varid,
                         reinterpret_cast<unsigned char *>(out->data())) != NC_NOERR) {
        m_error = QStringLiteral("cannot read pixel_duplication_mask");
        out->clear();
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Equations 2-5: the tie point stencil
// ---------------------------------------------------------------------------

ViiTieStencil ViiL1BReader::stencilAt(int ialt, int iact) const
{
    const ViiGeometry &g = m_geom;

    const int iscan   = ialt / g.nd;      // Eq. 4
    const int zoneAlt = ialt / g.zalt;    // Eq. 2
    const int zoneAct = iact / g.zact;
    const int rAlt    = ialt % g.zalt;    // Eq. 5
    const int rAct    = iact % g.zact;

    /* Eq. 3. The ALT index is zoneAlt + iscan, not zoneAlt: the extra
       scan offset gives every scan its own pair of edge tie rows, so
       interpolation never crosses a scan boundary and the bowtie
       discontinuity is never smeared. Scan l uses tie rows 4l..4l+3. */
    const int row0 = zoneAlt + iscan;
    const int row1 = row0 + 1;
    const int col0 = zoneAct;
    const int col1 = col0 + 1;

    ViiTieStencil s;
    s.index[0] = row0 * g.ntieAct + col0;   // A
    s.index[1] = row0 * g.ntieAct + col1;   // B
    s.index[2] = row1 * g.ntieAct + col1;   // C
    s.index[3] = row1 * g.ntieAct + col0;   // D

    /* Eq. 6 written as four weights that sum to 1 */
    const double u = double(rAct) / double(g.zact);
    const double v = double(rAlt) / double(g.zalt);
    s.weight[0] = (1.0 - v) * (1.0 - u);
    s.weight[1] = (1.0 - v) * u;
    s.weight[2] = v * u;
    s.weight[3] = v * (1.0 - u);
    return s;
}

// ---------------------------------------------------------------------------
// Equations 6-10: geolocation reconstruction
// ---------------------------------------------------------------------------

bool ViiL1BReader::interpolateGeolocation(QVector<float> *latitude,
                                          QVector<float> *longitude,
                                          bool useCartesian)
{
    QVector<double> latTie, lonTie;
    if (!readTiePointVariable(QStringLiteral("latitude"),  &latTie))
        return false;
    if (!readTiePointVariable(QStringLiteral("longitude"), &lonTie))
        return false;

    const ViiGeometry g = m_geom;
    latitude->resize(g.pixelCount());
    longitude->resize(g.pixelCount());

    float *latOut = latitude->data();
    float *lonOut = longitude->data();

    /* Convert the tie points once (55 160 conversions) rather than four
       times per output pixel (10.5 million). */
    QVector<double> tx, ty, tz;
    if (useCartesian) {
        const int n = g.tieCount();
        tx.resize(n); ty.resize(n); tz.resize(n);
        for (int i = 0; i < n; ++i) {
            if (std::isnan(latTie[i]) || std::isnan(lonTie[i])) {
                tx[i] = ty[i] = tz[i] = qQNaN();
            } else {
                geodeticToEcef(latTie[i], lonTie[i], &tx[i], &ty[i], &tz[i]);
            }
        }
    }

    const double *px = tx.constData(), *py = ty.constData(), *pz = tz.constData();
    const double *pla = latTie.constData(), *plo = lonTie.constData();

    for (int ialt = 0; ialt < g.nlines; ++ialt) {
        for (int iact = 0; iact < g.npixels; ++iact) {
            const ViiTieStencil s = stencilAt(ialt, iact);
            const int o = ialt * g.npixels + iact;

            if (useCartesian) {
                double x = 0, y = 0, z = 0;
                bool bad = false;
                for (int k = 0; k < 4; ++k) {
                    const int j = s.index[k];
                    if (std::isnan(px[j])) { bad = true; break; }
                    x += s.weight[k] * px[j];
                    y += s.weight[k] * py[j];
                    z += s.weight[k] * pz[j];
                }
                if (bad) {
                    latOut[o] = lonOut[o] = qQNaN();
                } else {
                    double la, lo;
                    ecefToGeodetic(x, y, z, &la, &lo);
                    latOut[o] = float(la);
                    lonOut[o] = float(lo);
                }
            } else {
                double la = 0, lo = 0;
                bool bad = false;
                for (int k = 0; k < 4; ++k) {
                    const int j = s.index[k];
                    if (std::isnan(pla[j])) { bad = true; break; }
                    la += s.weight[k] * pla[j];
                    lo += s.weight[k] * plo[j];
                }
                latOut[o] = bad ? qQNaN() : float(la);
                lonOut[o] = bad ? qQNaN() : float(lo > 180.0 ? lo - 360.0 : lo);
            }
        }
    }
    return true;
}

bool ViiL1BReader::interpolateTiePointVariable(const QString &name,
                                               QVector<float> *out,
                                               bool isAzimuth)
{
    QVector<double> tie;
    if (!readTiePointVariable(name, &tie))
        return false;

    const ViiGeometry g = m_geom;
    out->resize(g.pixelCount());
    float *o = out->data();
    const double *t = tie.constData();

    for (int ialt = 0; ialt < g.nlines; ++ialt) {
        for (int iact = 0; iact < g.npixels; ++iact) {
            const ViiTieStencil s = stencilAt(ialt, iact);
            const int idx = ialt * g.npixels + iact;
            bool bad = false;

            if (isAzimuth) {
                /* azimuths wrap at 360, so average the unit vectors */
                double sx = 0, sy = 0;
                for (int k = 0; k < 4; ++k) {
                    const double a = t[s.index[k]];
                    if (std::isnan(a)) { bad = true; break; }
                    sx += s.weight[k] * std::cos(qDegreesToRadians(a));
                    sy += s.weight[k] * std::sin(qDegreesToRadians(a));
                }
                o[idx] = bad ? qQNaN()
                             : float(qRadiansToDegrees(std::atan2(sy, sx)));
            } else {
                double acc = 0;
                for (int k = 0; k < 4; ++k) {
                    const double a = t[s.index[k]];
                    if (std::isnan(a)) { bad = true; break; }
                    acc += s.weight[k] * a;
                }
                o[idx] = bad ? qQNaN() : float(acc);
            }
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// Equations 11/12: DEM orthorectification
// ---------------------------------------------------------------------------

bool ViiL1BReader::orthorectify(QVector<float> *latitude,
                                QVector<float> *longitude)
{
    QVector<float> dn, de;
    if (!readFullGridVariable(QStringLiteral("delta_lat_N_dem"), &dn, false))
        return false;
    if (!readFullGridVariable(QStringLiteral("delta_lon_E_dem"), &de, false))
        return false;

    float *lat = latitude->data();
    float *lon = longitude->data();
    const int n = latitude->size();

    for (int i = 0; i < n; ++i) {
        if (std::isnan(lat[i]) || std::isnan(dn[i]) || std::isnan(de[i]))
            continue;
        const double clat = std::cos(qDegreesToRadians(double(lat[i])));
        lat[i] += float(qRadiansToDegrees(dn[i] / kFixedRadius));
        if (std::fabs(clat) > 1e-9)
            lon[i] += float(qRadiansToDegrees(de[i] / (kFixedRadius * clat)));
    }
    return true;
}
