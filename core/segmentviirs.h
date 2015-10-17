#ifndef SEGMENTVIIRS_H
#define SEGMENTVIIRS_H

#include "satellite.h"
#include "segment.h"

typedef struct
{
    float lon;
    float lat;
    int i;
    int j;
} lonlatdata;


class SegmentVIIRS : public Segment
{
    Q_OBJECT

public:
    explicit SegmentVIIRS(QFile *filesegment = 0, SatelliteList *satl = 0, QObject *parent = 0);
    ~SegmentVIIRS();

    void initializeMemory();
    void resetMemory();
    void cleanupMemory();

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

    QString getDatasetNameFromBand();
    QString getDatasetNameFromColor(int colorindex);
    bool composeColorImage();
    qint32 getProjectionX(int line, int pixelx) { return projectionCoordX[line * 3200 + pixelx]; }
    qint32 getProjectionY(int line, int pixelx) { return projectionCoordY[line * 3200 + pixelx]; }
    QRgb getProjectionValue(int line, int pixelx) { return projectionCoordValue[line * 3200 + pixelx]; }
    int threshold[3];

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

    float *tiepoints_lat;
    float *tiepoints_lon;
    float *aligncoef;
    float *expanscoef;
    float s[16];
    float *geolatitude;
    float *geolongitude;

    qint32 *projectionCoordX;
    qint32 *projectionCoordY;
    QRgb *projectionCoordValue;


    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;

    float latMax;
    float lonMax;
    float latMin;
    float lonMin;

    bool invertthissegment[3];

};

#endif // SEGMENTVIIRS_H
