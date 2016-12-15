#ifndef SEGMENTOLCIEFR_H
#define SEGMENTOLCIEFR_H

#include <QObject>
#include "segment.h"
#include "archive.h"
#include "satellite.h"


class SegmentOLCI : public Segment
{
    Q_OBJECT

public:
    explicit SegmentOLCI(eSegmentType type, QFile *filesegment = 0, SatelliteList *satl = 0, QObject *parent = 0);
    Segment *ReadSegmentInMemory();

    void ComposeSegmentImage(int histogrammethod, bool normalized);
    void ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized);


    void initializeMemory();
    int getEarthViewsPerScanline() { return this->earth_views_per_scanline; }

    void CalculateDetailCornerPoints();
    void setupVector(double statevec, QSgp4Date sensing);
    void recalculateStatsInProjection(bool normalized);
    void RecalculateProjection(bool normalized);


    ~SegmentOLCI();

    int UntarSegmentToTemp();
    int stat_max_projection[3];
    int stat_min_projection[3];
    long active_pixels_projection;

private:
    void RenderSegmentlineInTextureOLCI( int nbrLine, QRgb *row );
    void getDatasetNameFromColor(int colorindex, QString *datasetname, QString *variablename);
    void getDatasetNameFromBand(QString *datasetname, QString *variablename);

    void ComposeProjection(eProjections proj, int histogrammethod, bool normalized);
    void MapPixel(int lines, int views, double map_x, double map_y, bool iscolor, int histogrammethod, bool normalized);
    float getSolarZenith(int *tieSZA, int navpoint, int intpoint, int nbrLine);
    bool invertthissegment[3];
    int copy_data(struct archive *ar, struct archive *aw);


    QScopedArrayPointer<int> latitude;
    QScopedArrayPointer<int> longitude;


};

#endif // SEGMENTOLCIEFR_H
