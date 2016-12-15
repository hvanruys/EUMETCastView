#ifndef SEGMENTGAC_H
#define SEGMENTGAC_H

#include <QObject>
#include <QFile>
#include <QImage>

#include "segment.h"

class SegmentGAC : public Segment
{
    Q_OBJECT

public:
    explicit SegmentGAC(QFile *filesegment = 0, SatelliteList *satl = 0, QObject *parent = 0);
    ~SegmentGAC();

    bool inspectMPHRrecord(QByteArray mphr_record);
    quint32 get_next_header( QByteArray ba );
    void RenderSegmentlineInTexture( int channel, int nbrLine, int nbrTotalLine );

    void ComposeProjection(int inputchannel, eProjections proj);
    void RenderSegmentlineInProjection( int channel, int nbrLine, int heightintotalimage, eProjections proj );
    void ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized);

    void RenderSegmentlineInSG( int channel, int nbrLine, int heightintotalimage);
    void RenderSegmentlineInGVP( int channel, int nbrLine, int heightintotalimage);
    void RenderSegmentlineInLCC( int channel, int nbrLine, int heightintotalimage);


    void initializeProjectionCoord();


    Segment *ReadSegmentInMemory();
    int ReadNbrOfLines();

    double earth_loc_lon_first[360], earth_loc_lat_first[360];
    double earth_loc_lon_last[360], earth_loc_lat_last[360];
    double earth_loc_altitude[360];

private:
    void inspectEarthLocations(QByteArray *mdr_record, int heightinsegment);
    void intermediatePoint(double lat1, double lng1, double lat2, double lng2, double f, double *lat, double *lng, double d);

    quint16 num_navigation_points;


signals:
    
public slots:
    
};

#endif // SEGMENTGAC_H
