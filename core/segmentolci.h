#ifndef SEGMENTOLCIEFR_H
#define SEGMENTOLCIEFR_H

#include <QObject>
#include <QFutureWatcher>
#include <QtConcurrent>

#include "segment.h"
#include "archive.h"
#include "satellite.h"


class SegmentOLCI : public Segment
{
    Q_OBJECT

public:
    explicit SegmentOLCI(eSegmentType type, QFileInfo fileinfo, SatelliteList *satl = 0, QObject *parent = 0);
    Segment *ReadSegmentInMemory();

    void ComposeSegmentImage(int histogrammethod, bool normalized);
    void ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized);


    void initializeMemory();
    int getEarthViewsPerScanline() { return this->earth_views_per_scanline; }

    void recalculateStatsInProjection(bool normalized);
    void RecalculateProjection(bool normalized);
    void CalcOverlayLatLon(int columnslength, int rowslength);


    ~SegmentOLCI();

    int stat_max_projection[3];
    int stat_min_projection[3];
    long active_pixels_projection;
    QPolygon coastline;
    QPolygon latlonline;
    long nbrsaturatedpixels;
    long nbrcoastlinepixels;

    QScopedArrayPointer<int> latitude;
    QScopedArrayPointer<int> longitude;


private:
    void RenderSegmentlineInTextureOLCI( int nbrLine, QRgb *row );
    void getDatasetNameFromColor(int colorindex, QString *datasetname, QString *variablename, int *saturationindex);
    void getDatasetNameFromBand(QString *datasetname, QString *variablename, int *saturationindex);

    void ComposeProjection(eProjections proj, int histogrammethod, bool normalized);
    void MapPixel(int lines, int views, double map_x, double map_y, bool iscolor, int histogrammethod, bool normalized);
    float getSolarZenith(int *tieSZA, int navpoint, int intpoint, int nbrLine);
    bool invertthissegment[3];
    int copy_data(struct archive *ar, struct archive *aw);


    QScopedArrayPointer<quint32> masks;
    QStringList strlflagmeanings;
    int saturationindex[3];

};

#endif // SEGMENTOLCIEFR_H
