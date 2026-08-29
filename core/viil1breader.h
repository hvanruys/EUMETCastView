#ifndef VIIL1BREADER_H
#define VIIL1BREADER_H

/*
 * ViiL1BReader -- EPS-SG VII (METimage) L1B RAD reader with tie-point
 *                 geolocation reconstruction.
 *
 * Implements EUM/LEO-EPSSG/SPE/14/777138 v5A section 4.2.4.1.3 (Equations
 * 1-12): the product only carries latitude/longitude on a coarse tie-point
 * grid (140 x 394 for a one minute granule), and every full resolution
 * position has to be rebuilt from it.
 *
 * Everything runs serially: reconstructing a whole 840 x 3144 granule takes
 * ~170 ms, and SegmentVII::ReadSegmentInMemory already runs on a
 * QtConcurrent worker, so a nested thread pool would only risk starving the
 * global one.
 *
 * Errors are reported through lastError() rather than by exiting, so a bad
 * segment cannot take the application down with it.
 */

#include <QString>
#include <QStringList>
#include <QVector>
#include <QtGlobal>

struct ViiGeometry
{
    int nscans   = 0;   // Nl        num_scans
    int nd       = 0;   // N_D       num_pixels_alt      (24)
    int nlines   = 0;   // N_ALT     num_lines           (nscans * nd)
    int npixels  = 0;   // N_ACT     num_pixels          (3144)
    int zact     = 0;   // Z_ACT     zone_size_act       (8)
    int zalt     = 0;   // Z_ALT     zone_size_alt       (8)
    int ntieAct  = 0;   // N_TIE_ACT num_tie_points_act  (394)
    int ntieAlt  = 0;   // N_TIE_ALT num_tie_points_alt  (4 * nscans)

    bool    isConsistent() const;
    QString toString() const;

    int tieCount()   const { return ntieAlt * ntieAct; }
    int pixelCount() const { return nlines  * npixels; }
};

/* The four tie points surrounding one pixel, with their bilinear weights.
   Order is A, B, C, D as defined in PFS Equation 3. */
struct ViiTieStencil
{
    int    index[4];
    double weight[4];
};

class ViiL1BReader
{

public:
    ViiL1BReader();
    ~ViiL1BReader();

    bool open(const QString &path);
    void close();
    bool isOpen() const { return m_ncid >= 0; }

    QString        fileName() const { return m_fileName; }
    ViiGeometry    geometry() const { return m_geom; }
    QString        lastError() const { return m_error; }

    /* names of the vii_* radiance variables present in the product */
    QStringList    radianceChannels() const;

    /* band 1..20 as numbered by the VII tab of the toolbox, in the order the
       channels appear in the product. Returns an empty string out of range. */
    static QString channelVariableName(int band);

    /* Read a packed variable and apply Equation 1
       (value = packed * scale_factor + add_offset).
       _FillValue and, when requested, out-of-valid-range samples become NaN. */
    bool readTiePointVariable(const QString &name, QVector<double> *out);
    bool readFullGridVariable(const QString &name, QVector<float> *out,
                              bool applyValidRange = true);

    /* valid_min/valid_max of a variable put through Equation 1, i.e. the range
       the physical values of that channel can occupy. */
    bool radianceRange(const QString &name, double *lo, double *hi);

    /* /data/processing_flags/pixel_duplication_mask, num_pixels_alt x
       num_pixels: non-zero where the bow-tie overlap duplicates a pixel of the
       neighbouring scan. It is indexed by the line's position within its scan,
       so it is only nd rows tall and applies to every scan alike. */
    bool readDuplicationMask(QVector<quint8> *out);

    /* PFS Eq. 2-10: reconstruct latitude/longitude on the full
       nlines x npixels grid. Longitude is returned in -180..180.
       Set useCartesian=false to interpolate degrees directly -- faster, but
       wrong across the dateline and near the poles. */
    bool interpolateGeolocation(QVector<float> *latitude,
                                QVector<float> *longitude,
                                bool useCartesian = true);

    /* Same stencil for the other tie-point variables. Azimuths must set
       isAzimuth=true so they are interpolated through sin/cos. */
    bool interpolateTiePointVariable(const QString &name, QVector<float> *out,
                                     bool isAzimuth = false);

    /* PFS Eq. 11/12: shift geolocation using the full-resolution
       delta_lat_N_dem / delta_lon_E_dem fields. */
    bool orthorectify(QVector<float> *latitude, QVector<float> *longitude);

    /* Stencil for one pixel; public so callers can geolocate a subset
       (a visible tile, a single scan) without reconstructing the whole grid. */
    ViiTieStencil stencilAt(int ialt, int iact) const;

private:
    bool readGeometry();
    int  dimLength(const char *name, bool *ok) const;
    bool unpack(int varid, QVector<double> *values) const;

    int         m_ncid      = -1;
    int         m_measGid   = -1;
    int         m_dataGid   = -1;
    int         m_flagsGid  = -1;
    QString     m_fileName;
    QString     m_error;
    ViiGeometry m_geom;
};

#endif // VIIL1BREADER_H
