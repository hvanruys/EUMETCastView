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

    void ComposeSegmentImage();
    void ComposeSegmentGVProjection(int inputchannel);


    void initializeMemory();
    int getEarthViewsPerScanline() { return this->earth_views_per_scanline; }

    void CalculateDetailCornerPoints();
    void setupVector(double statevec, QSgp4Date sensing);

    ~SegmentOLCI();

    int UntarSegmentToTemp();

private:
    void RenderSegmentlineInTextureOLCIefr( int nbrLine, QRgb *row );
    void getDatasetNameFromColor(int colorindex, QString *datasetname, QString *variablename);
    void getDatasetNameFromBand(QString *datasetname, QString *variablename);

    void ComposeProjection(eProjections proj);
    void MapPixel( int lines, int views, double map_x, double map_y, bool color);

    bool invertthissegment[3];
    int copy_data(struct archive *ar, struct archive *aw);

    QScopedArrayPointer<int> latitude;
    QScopedArrayPointer<int> longitude;

};

#endif // SEGMENTOLCIEFR_H
