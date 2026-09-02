#ifndef SEGMENTVII_H
#define SEGMENTVII_H

#include <QObject>
#include <QFutureWatcher>
//#include <QtConcurrent>

#include "segment.h"
#include "archive.h"
#include "archive_entry.h"
#include "satellite.h"
#include "viil1breader.h"


class SegmentVII : public Segment
{
    Q_OBJECT

public:
    explicit SegmentVII(eSegmentType type, QFileInfo fileinfo, QObject *parent = 0);
    Segment *ReadSegmentInMemory();

    /**
     * Read the channels one RGB recipe asks for, convert them to the physical
     * units the recipe is written in - reflectance for the solar channels,
     * brightness temperature for the thermal ones - and lay the finished colours
     * down in ptrbaVII.
     *
     * The recipe carries its own stretch, so what is stored is already the
     * displayed brightness rather than a radiance waiting for a histogram. It is
     * held on the radiance scale all the same, with 65535 still meaning no data,
     * so the compose and projection paths reproduce it exactly at a 100 %
     * stretch and nothing downstream needs to know a recipe was involved.
     */
    Segment *ReadSegmentRecipeInMemory(int recipe);

    void ComposeSegmentImage(int histogrammethod, bool normalized);
    void ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentOMProjection(int inputchannel, int histogrammethod, bool normalized);


    void initializeMemory();
    void resetMemory();
    int getEarthViewsPerScanline() { return this->earth_views_per_scanline; }
    /* lines per scan, 24: the bow-tie repeats with it, so the seam between two
       scans sits every num_pixels_alt lines. */
    int getNumPixelsAlt() const { return this->num_pixels_alt; }

    void recalculateStatsInProjection(bool normalized);
    void RecalculateProjection(bool normalized);

    /* non-zero where the bow-tie overlap duplicates a pixel of the neighbouring
       scan. The mask is only num_pixels_alt rows tall and repeats every scan,
       so it is indexed by the line's position within its own scan. */
    bool isDuplicatedPixel(int line, int pixelx) const;

    ~SegmentVII();

    int stat_max_projection[3];
    int stat_min_projection[3];
    long active_pixels_projection;
    long nbrsaturatedpixels;

    /* whole-degree graticule, built from the reconstructed geolocation. VII has
       no per-pixel coastline flag of the kind SegmentOLCI draws its overlay
       from, so the lat/lon lines are all the product can offer. */
    QPolygon latlonline;

    /* geolatitude / geolongitude of the base class hold the full resolution
       geolocation in degrees, longitude in -180..180, reconstructed from the
       tie point grid. */

    void getCentralCoords(double *startlon, double *startlat, double *endlon, double *endlat);

private:
    void RenderSegmentlineInTextureVII( int nbrLine, QRgb *row );
    void CalcOverlayLatLon();

    /* Rebuild the full resolution geolocation from the tie point grid, publish
       it, derive the graticule from it and read the bow-tie duplication mask.
       Common to the band and the recipe read paths. */
    bool ReadGeolocation(const ViiGeometry &geom);

    /* Sun-normalise every solar channel of a recipe, in place, and with dehaze
       also remove the Rayleigh path reflectance. The thermal channels are left
       alone: a brightness temperature is neither divided by the sun nor
       scattered by the air. See the definition for why only one of the two is
       ever optional. */
    void ApplySolarCorrection(QList<QVector<float> > &bandbuf,
                              const QStringList &bandnames, bool dehaze,
                              const QVector<float> &sza, const QVector<float> &saa,
                              const QVector<float> &vza, const QVector<float> &vaa);

    QString getChannelNameFromColor(int colorindex);
    QString getChannelNameFromBand();

    void ComposeProjection(eProjections proj, int histogrammethod, bool normalized);
    void MapPixel(int lines, int views, double map_x, double map_y, bool iscolor, int histogrammethod, bool normalized);
    bool invertthissegment[3];

    ViiL1BReader reader;
    QVector<quint8> duplicationmask;
    int num_pixels_alt;

};

#endif // SEGMENTVII_H
