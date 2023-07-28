#ifndef SEGMENTVIIRSDNB_H
#define SEGMENTVIIRSDNB_H

#include "satellite.h"
#include "segment.h"


class SegmentVIIRSDNB : public Segment
{
    Q_OBJECT

public:
    explicit SegmentVIIRSDNB(QFile *filesegment = 0, eSegmentType type = eSegmentType::SEG_NONE, QObject *parent = 0);
    ~SegmentVIIRSDNB();

    void initializeMemory();

    Segment *ReadSegmentInMemory();
    Segment *ReadDatasetsInMemory();

    int ReadNbrOfLines();

    void ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentOMProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeProjection(eProjections proj, int histogrammethod, bool normalized);
    void ComposeSegmentImageWindow(float lowerlimit, float upperlimit);
    void ComposeSegmentImageWindowFromCurve(QVector<double> *x, QVector<double> *y);

    void CalcGraph(QScopedArrayPointer<long> *graph);
    void getCentralCoords(double *startlon, double *startlat, double *endlon, double *endlat, int *startindex, int *endindex);

    int threshold[3];
    float stat_max;
    float stat_min;

private:

    void MapPixel(int lines, int views, double map_x, double map_y);

    void RenderSegmentlineInTextureVIIRS( int nbrTotalLine, QRgb *row );
    void LonLatMax();

    void CalcGraphPockets(int xzenith, float val, QScopedArrayPointer<long> *graph);
    float getRadianceFromCurve(int xzenith, QVector<double> *x, QVector<double> *y);

    void CalcInterpolationPerGroup(int itrack, int igroupscan, int indexfrom);
    void CalcInterpolationInTPZ(int itrack, int iscan, int indexfrom, int igroupscan);
    void GetAlpha(float &ascan, float &atrack, int rels, int relt, int index, int zscan);

    void interpolateLonLatViaVector(int itrack, int indexfrom, int igroupscan, float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D);
    void interpolateSolarViaVector(int itrack, int indexfrom, int igroupscan,
                  float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D,
                  float solar_zenith_A, float solar_zenith_B, float solar_zenith_C, float solar_zenith_D,
                  float solar_azimuth_A, float solar_azimuth_B, float solar_azimuth_C, float solar_azimuth_D);
    void interpolateLonLatDirect(int itrack, int indexfrom, int igroupscan, float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D);
    void interpolateSolarDirect(int itrack, int indexfrom, int igroupscan, float solar_zenith_A, float solar_zenith_B, float solar_zenith_C, float solar_zenith_D, float solar_azimuth_A, float solar_azimuth_B, float solar_azimuth_C, float solar_azimuth_D);
    int GetTotalDimensionspace(hid_t id);

    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;

    float latMax;
    float lonMax;
    float latMin;
    float lonMin;

    bool invertthissegment[3];

    QScopedArrayPointer<float> tiepoints_lat;
    QScopedArrayPointer<float> tiepoints_lon;
    QScopedArrayPointer<float> tiepoints_lunar_azimuth;
    QScopedArrayPointer<float> tiepoints_lunar_zenith;
    QScopedArrayPointer<float> tiepoints_solar_azimuth;
    QScopedArrayPointer<float> tiepoints_solar_zenith;
    QScopedArrayPointer<float> aligncoef;
    QScopedArrayPointer<float> expanscoef;
    QScopedArrayPointer<int> NumberOfTiePointZonesScan;
    QScopedArrayPointer<int> NumberOfTiePointZonesTrack;
    QScopedArrayPointer<int> NumberOfTiePointZoneGroupsScan;
    QScopedArrayPointer<int> NumberOfTiePointZoneGroupsTrack;
    QScopedArrayPointer<int> TiePointZoneGroupLocationScanCompact;
    QScopedArrayPointer<int> TiePointZoneGroupLocationTrackCompact;
    QScopedArrayPointer<int> TiePointZoneGroupLocationScan;
    QScopedArrayPointer<int> TiePointZoneGroupLocationTrack;
    QScopedArrayPointer<int> TiePointZoneSizeScan;
    QScopedArrayPointer<int> TiePointZoneSizeTrack;
    QScopedArrayPointer<int> Ptpzscan;

    float s2[2];
    float s8[8];
    float s14[14];
    float s16[16];
    float s20[20];
    float s22[22];
    float s24[24];
    int tiepointxdim;
};

#endif // SEGMENTVIIRSDNB_H
