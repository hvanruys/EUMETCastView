#ifndef SEGMENTVIIRSM_H
#define SEGMENTVIIRSM_H

#include "satellite.h"
#include "segment.h"
#include <hdf5/serial/hdf5.h>

class SegmentVIIRSM : public Segment
{
    Q_OBJECT

public:
    explicit SegmentVIIRSM(QFile *filesegment = 0, SatelliteList *satl = 0, QObject *parent = 0);
    ~SegmentVIIRSM();

    void initializeMemory();

    Segment *ReadSegmentInMemory();
    Segment *ReadDatasetsInMemory();
    int ReadNbrOfLines();

    void setBandandColor(QList<bool> band, QList<int> color, QList<bool> invert);
    void ComposeSegmentImage();
    void ComposeSegmentLCCProjection(int inputchannel);
    void ComposeSegmentGVProjection(int inputchannel);
    void ComposeSegmentSGProjection(int inputchannel);
    void ComposeProjection(eProjections proj);
    //void ComposeProjectionConcurrent();
    void RecalculateProjection();

    void recalculateStatsInProjection();
    QString getDatasetNameFromBand();
    QString getDatasetNameFromColor(int colorindex);
    bool composeColorImage();

    float getBrightnessTemp(int lines, int views);
    float getBrightnessTemp(int radiance);
    float getRadiance(int lines, int views);
    float getRadiance(int radiance);


    int stat_max_projection[3];
    int stat_min_projection[3];
    long active_pixels_projection;
    float minBrightnessTemp;
    float maxBrightnessTemp;

private:

    void MapPixel(int lines, int views, double map_x, double map_y, bool color);

    void GetAlpha( float &ascan, float &atrack, int rels, int relt, int scan);
    void CalcGeoLocations(int itrack, int iscan);

    void RenderSegmentlineInTextureVIIRS( int nbrTotalLine, QRgb *row );
    void LonLatMax();
    float Minf(const float v11, const float v12, const float v21, const float v22);
    float Maxf(const float v11, const float v12, const float v21, const float v22);
    qint32 Min(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22);
    qint32 Max(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22);

    void interpolateViaLonLat(int itrack, int iscan, float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D);
    void interpolateViaVector(int itrack, int iscan, float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D);


    void ReadVIIRSM_SDR_All(hid_t h5_file_id);
    void ReadVIIRSM_GEO_All(hid_t h5_file_id);

    QScopedArrayPointer<float> tiepoints_lat;
    QScopedArrayPointer<float> tiepoints_lon;
    QScopedArrayPointer<float> aligncoef;
    QScopedArrayPointer<float> expanscoef;
    float s[16];

    QScopedArrayPointer<float> geolatitude;
    QScopedArrayPointer<float> geolongitude;


    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;

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
