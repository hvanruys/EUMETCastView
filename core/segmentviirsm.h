#ifndef SEGMENTVIIRSM_H
#define SEGMENTVIIRSM_H

#include "satellite.h"
#include "segment.h"

#ifdef _WIN32
#include <hdf5.h>
#else
#include <hdf5.h>
#endif


class SegmentVIIRSM : public Segment
{
    Q_OBJECT

public:
    explicit SegmentVIIRSM(QFile *filesegment = 0, eSegmentType type = eSegmentType::SEG_NONE, QObject *parent = 0);
    ~SegmentVIIRSM();

    void initializeMemory();

    Segment *ReadSegmentInMemory();
    Segment *ReadDatasetsInMemory();
    int ReadNbrOfLines();

    void ComposeSegmentImage();
    void ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentOMProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeProjection(eProjections proj, int histogrammethod, bool normalized);
    //void ComposeProjectionConcurrent();
    void RecalculateProjection();

    void recalculateStatsInProjection();
    QString getDatasetNameFromBand();
    QString getDatasetNameFromColor(int colorindex);

    float getBrightnessTemp(int lines, int views);
    float getBrightnessTemp(int radiance);
    float getRadiance(int lines, int views);
    float getRadiance(int radiance);
    void getCentralCoords(double *startlon, double *startlat, double *endlon, double *endlat, int *startindex, int *endindex);
    void getStartCornerCoords(double *cornerlon1, double *cornerlat1, double *cornerlon2, double *cornerlat2,
                                            int *Xstartindex1, int *Xstartindex2, int *Ystartindex12);
    void getEndCornerCoords(double *cornerlon3, double *cornerlat3, double *cornerlon4, double *cornerlat4,
                                            int *Xstartindex3, int *Xstartindex4, int *Ystartindex34);

    int stat_max_projection[3];
    int stat_min_projection[3];
    long active_pixels_projection;
    float minBrightnessTemp;
    float maxBrightnessTemp;

private:

    void MapPixel(int lines, int views, double map_x, double map_y, bool iscolor);

    void GetAlpha( float &ascan, float &atrack, int rels, int relt, int scan);
    void CalcGeoLocations(int itrack, int iscan);

    void RenderSegmentlineInTextureVIIRS( int nbrTotalLine, QRgb *row );
    void LonLatMax();
    float Minf(const float v11, const float v12, const float v21, const float v22);
    float Maxf(const float v11, const float v12, const float v21, const float v22);
    qint32 Min(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22);
    qint32 Max(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22);

    void interpolateLonLatDirect(int itrack, int iscan, float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D);
    void interpolateLonLatViaVector(int itrack, int iscan, float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D);


    void ReadVIIRSM_SDR_All(hid_t h5_file_id);
    void ReadVIIRSM_GEO_All(hid_t h5_file_id);

    QScopedArrayPointer<float> tiepoints_lat;
    QScopedArrayPointer<float> tiepoints_lon;
    QScopedArrayPointer<float> aligncoef;
    QScopedArrayPointer<float> expanscoef;
    float s16[16];

    float latMax;
    float lonMax;
    float latMin;
    float lonMin;

    int threshold[3];
    float radianceoffsethigh[3];
    float radianceoffsetlow[3];
    float radiancescalehigh[3];
    float radiancescalelow[3];
    double bandcorrectioncoefficientA[3];
    double bandcorrectioncoefficientB[3];
    double centralwavelength[3];
    bool invertthissegment[3];

};

#endif // SEGMENTVIIRSM_H
