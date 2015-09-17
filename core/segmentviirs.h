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
    bool lookupLonLat(double lon_deg, double lat_deg, int &col, int &row);
    bool testlookupLonLat(double lon_deg, double lat_deg, int &col, int &row);
    int ReadNbrOfLines();

    void setBandandColor(QList<bool> band, QList<int> color, QList<bool> invert);
    void ComposeSegmentImage();
    void ComposeSegmentLCCProjection(int inputchannel);
    void ComposeSegmentGVProjection(int inputchannel);
    void ComposeSegmentSGProjection(int inputchannel);
    void ComposeProjection(eProjections proj);
    void ComposeProjectionConcurrent();

    QString getDatasetNameFromBand();
    QString getDatasetNameFromColor(int colorindex);
    bool composeColorImage();
    int threshold[3];


private:

    void MapPixel(int lines, int views, double map_x, double map_y, bool color);

    void GetAlpha( float &ascan, float &atrack, int rels, int relt, int scan);
    void CalcGeoLocations(int itrack, int iscan);

    void RenderSegmentlineInTextureVIIRS( int nbrTotalLine, QRgb *row );
    void LonLatMax();
    float *tiepoints_lat;
    float *tiepoints_lon;
    float *aligncoef;
    float *expanscoef;
    float s[16];
    float *geolatitude;
    float *geolongitude;
    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;

    float latMax;
    float lonMax;
    float latMin;
    float lonMin;
    QMap<int, QList<lonlatdata>> viirsmap;

    bool invertthissegment[3];


};

#endif // SEGMENTVIIRS_H
