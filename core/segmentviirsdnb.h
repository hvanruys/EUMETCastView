#ifndef SEGMENTVIIRSDNB_H
#define SEGMENTVIIRSDNB_H

#include "satellite.h"
#include "segment.h"


class SegmentVIIRSDNB : public Segment
{
    Q_OBJECT

public:
    explicit SegmentVIIRSDNB(QFile *filesegment = 0, SatelliteList *satl = 0, QObject *parent = 0);
    ~SegmentVIIRSDNB();

    void initializeMemory();

    Segment *ReadSegmentInMemory();
    Segment *ReadDatasetsInMemory();
    int ReadNbrOfLines();

    //void ComposeSegmentImage();
    void ComposeSegmentLCCProjection(int inputchannel);
    void ComposeSegmentGVProjection(int inputchannel);
    void ComposeSegmentSGProjection(int inputchannel);
    void ComposeProjection(eProjections proj);
    void ComposeSegmentImageWindow(float lowerlimit, float upperlimit);


    int threshold[3];
    float stat_max;
    float stat_min;

private:

    void MapPixel(int lines, int views, double map_x, double map_y);

    void GetAlpha(float &ascan, float &atrack, int rels, int relt, int index, int zscan);
    void CalcGeoLocationsPerGroup(int itrack, int igroupscan, int indexfrom);
    void CalcGeoLocationsInTPZ(int itrack, int iscan, int indexfrom, int igroupscan);

    void RenderSegmentlineInTextureVIIRS( int nbrTotalLine, QRgb *row );
    void LonLatMax();
    float Minf(const float v11, const float v12, const float v21, const float v22);
    float Maxf(const float v11, const float v12, const float v21, const float v22);
    qint32 Min(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22);
    qint32 Max(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22);

    void interpolateViaVector(int itrack, int indexfrom, int igroupscan, float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D);
    void interpolateViaLonLat(int itrack, int indexfrom, int igroupscan, float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D);

    QScopedArrayPointer<float> tiepoints_lat;
    QScopedArrayPointer<float> tiepoints_lon;
    QScopedArrayPointer<float> aligncoef;
    QScopedArrayPointer<float> expanscoef;
    QScopedArrayPointer<int> NumberOfTiePointZonesScan;
    QScopedArrayPointer<int> TiePointZoneGroupLocationScanCompact;
    float MoonIllumFraction;

    float s8[8];
    float s14[14];
    float s16[16];
    float s20[20];
    float s22[22];
    float s24[24];
    int Zscan[64] { 16, 16, 16, 16, 16, 16, 24, 24,
                    20, 14, 20, 16, 16, 16, 16, 24,
                    24, 24, 16, 14, 16, 16, 16, 16,
                    16, 16, 24, 16, 24, 22, 24,  8,
                     8, 24, 22, 24, 16, 24, 16, 16,
                    16, 16, 16, 16, 14, 16, 24, 24,
                    24, 16, 16, 16, 16, 20, 14, 20,
                    24, 24, 16, 16, 16, 16, 16, 16 };
    int Pscan[64];
    int Ptpzscan[252];

    QScopedArrayPointer<float> geolatitude;
    QScopedArrayPointer<float> geolongitude;

    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;

    float latMax;
    float lonMax;
    float latMin;
    float lonMin;


    bool invertthissegment[3];

};

#endif // SEGMENTVIIRSDNB_H
