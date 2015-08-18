#ifndef SEGMENTVIIRS_H
#define SEGMENTVIIRS_H

#include "satellite.h"
#include "segment.h"

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

    void setBandandColor(QList<bool> band, QList<int> color);
    void ComposeSegmentImage();
    void ComposeSegmentLCCProjection(int inputchannel);
    void ComposeSegmentGVProjection(int inputchannel);
    void ComposeSegmentSGProjection(int inputchannel);
    void ComposeProjection(eProjections proj);

    QString getDatasetNameFromBand();
    QString getDatasetNameFromColor(int colorindex);
    bool composeColorImage();
    bool lookupLonLat(double lon_rad, double lat_rad, int &col, int &row);
    bool testLonLat();
    //Segment *ComposeGVProjection();
    int threshold[3];


private:

    void MapPixel(int lines, int views, double map_x, double map_y, bool color);

    void GetAlpha( float &ascan, float &atrack, int rels, int relt, int scan);
    void CalcGeoLocations(int itrack, int iscan);

    void RenderSegmentlineInTextureVIIRS( int nbrTotalLine, QRgb *row );


    float *tiepoints_lat;
    float *tiepoints_lon;
    float *aligncoef;
    float *expanscoef;
    float s[16];
    float *geolatitude;
    float *geolongitude;
    QList<bool> bandlist;
    QList<int> colorlist;

};

#endif // SEGMENTVIIRS_H
