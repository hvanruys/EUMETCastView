#ifndef SEGMENTMETOP_H
#define SEGMENTMETOP_H
#include <QApplication>
#include "segment.h"
#include <QFile>
#include <QImage>


class SegmentMetop : public Segment
{
    Q_OBJECT

public:
    explicit SegmentMetop(QFile *filesegment = 0, QObject *parent = 0);
    ~SegmentMetop();

    void SegmentMetopBz2(QFile&);
    void inspectMPHRrecord(QByteArray mphr_record);
    void inspectSolarAngle(QByteArray *mdr_record,int heightinsegment);
    void inspectEarthLocations(QByteArray *mdr_record, int heightinsegment);
    quint32 get_next_header( QByteArray ba );

    //    void SetEarthLocations( QByteArray mdr_record, int i );
    //    void ResetEarthLocations();

    void RenderSegmentlineInTexture( int channel, int nbrLine, int nbrTotalLine );

    Segment *ReadSegmentInMemory();
    //void RenderEarthLocationsGL();
    void ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized);
    void RenderSegmentlineInGVP( int channel, int nbrLine, int heightintotalimage, bool normalized);
   // void RenderSegmentlineInGVPRad(int channel, double lat_first, double lon_first, double lat_last, double lon_last, double altitude, int heightintotalimage);

    void ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    void RenderSegmentlineInLCC(int channel, int nbrLine, int heightintotalimage, bool normalized);

    void ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized);
    void RenderSegmentlineInSG( int channel, int nbrLine, int heightintotalimage, bool normalized);

    void ComposeSegmentOMProjection(int inputchannel, int histogrammethod, bool normalized);
    void RenderSegmentlineInOM( int channel, int nbrLine, int heightintotalimage, bool normalized);


    void intermediatePoint(double lat1, double lng1, double lat2, double lng2, double f, double *lat, double *lng, double d);
    int ReadNbrOfLines();
    float getSolarZenith(int navpoint, int intpoint, int nbrLine, int startnavindex, int deltaint, int lengthintindex, int lengthnavindex );
    void GetCentralCoords(double *startlon, double *startlat, double *endlon, double *endlat, int *startindex, int *endindex);

    double earth_loc_lon_first[1080], earth_loc_lat_first[1080];
    double earth_loc_lon_last[1080], earth_loc_lat_last[1080];
    double earth_loc_altitude[1080];



    double subsat_latitude_start;
    double subsat_longitude_start;
    double subsat_latitude_end;
    double subsat_longitude_end;

    //QString satname;

private:

    char saveheader;
    bool get_next_header(QByteArray ba, quint32 *reclength);

    quint32 state_vector_year;
    quint32 state_vector_month;
    quint32 state_vector_day;
    quint32 state_vector_hour;
    quint32 state_vector_minute;
    quint32 state_vector_second;

    quint16 num_navigation_points;

    double wcirkel;

    int x_start, y_start;
    int x_end, y_end;

    
signals:

    
public slots:
    
};

#endif // SEGMENTMETOP_H
