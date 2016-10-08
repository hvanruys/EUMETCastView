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
    void ComposeSegmentImageWindowFromCurve(QVector<double> *x, QVector<double> *y);

    void CalcGraph(QScopedArrayPointer<long> *graph);


    float MoonIllumFraction;

    int threshold[3];
    float stat_max;
    float stat_min;

private:

    void MapPixel(int lines, int views, double map_x, double map_y);

    void GetAlpha(float &ascan, float &atrack, int rels, int relt, int index, int zscan);
    void CalcInterpolationPerGroup(int itrack, int igroupscan, int indexfrom);
    void CalcInterpolationInTPZ(int itrack, int iscan, int indexfrom, int igroupscan);

    void RenderSegmentlineInTextureVIIRS( int nbrTotalLine, QRgb *row );
    void LonLatMax();
    float Minf(const float v11, const float v12, const float v21, const float v22);
    float Maxf(const float v11, const float v12, const float v21, const float v22);
    qint32 Min(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22);
    qint32 Max(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22);

    void interpolateLonLatViaVector(int itrack, int indexfrom, int igroupscan, float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D);
    void interpolateSolarViaVector(int itrack, int indexfrom, int igroupscan,
                  float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D,
                  float solar_zenith_A, float solar_zenith_B, float solar_zenith_C, float solar_zenith_D,
                  float solar_azimuth_A, float solar_azimuth_B, float solar_azimuth_C, float solar_azimuth_D);
    void interpolateLonLatDirect(int itrack, int indexfrom, int igroupscan, float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D);
    void interpolateSolarDirect(int itrack, int indexfrom, int igroupscan, float solar_zenith_A, float solar_zenith_B, float solar_zenith_C, float solar_zenith_D, float solar_azimuth_A, float solar_azimuth_B, float solar_azimuth_C, float solar_azimuth_D);

    void CalcGraphPockets(int xzenith, float val, QScopedArrayPointer<long> *graph);
    float getRadianceFromCurve(int xzenith, QVector<double> *x, QVector<double> *y);

    QScopedArrayPointer<float> tiepoints_lat;
    QScopedArrayPointer<float> tiepoints_lon;
    QScopedArrayPointer<float> tiepoints_lunar_azimuth;
    QScopedArrayPointer<float> tiepoints_lunar_zenith;
    QScopedArrayPointer<float> tiepoints_solar_azimuth;
    QScopedArrayPointer<float> tiepoints_solar_zenith;
    QScopedArrayPointer<float> aligncoef;
    QScopedArrayPointer<float> expanscoef;
    QScopedArrayPointer<int> NumberOfTiePointZonesScan;
    QScopedArrayPointer<int> TiePointZoneGroupLocationScanCompact;

    float s8[8];
    float s14[14];
    float s16[16];
    float s20[20];
    float s22[22];
    float s24[24];
    int Zscan[64];

//    int Zscan[64] { 16, 16, 16, 16, 16, 16, 24, 24,
//                    20, 14, 20, 16, 16, 16, 16, 24,
//                    24, 24, 16, 14, 16, 16, 16, 16,
//                    16, 16, 24, 16, 24, 22, 24,  8,
//                     8, 24, 22, 24, 16, 24, 16, 16,
//                    16, 16, 16, 16, 14, 16, 24, 24,
//                    24, 16, 16, 16, 16, 20, 14, 20,
//                    24, 24, 16, 16, 16, 16, 16, 16 };
    int Pscan[64];
    int Ptpzscan[252];

    QScopedArrayPointer<float> geolatitude;
    QScopedArrayPointer<float> geolongitude;
    QScopedArrayPointer<float> lunar_zenith;
    QScopedArrayPointer<float> solar_zenith;
    QScopedArrayPointer<float> lunar_azimuth;
    QScopedArrayPointer<float> solar_azimuth;

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
