#ifndef SEGMENT_H
#define SEGMENT_H

#include <QObject>
#include <QVector3D>
#include <QVector2D>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

#include "globals.h"
#include "satellite.h"
#include "bzlib.h"

#include "qtle.h"
#include "qsgp4.h"
#include "qeci.h"

class Segment : public QObject
{
    Q_OBJECT

public:

    enum eProjections {
        LCC = 0,
        GVP,
        SG
    };

    explicit Segment(QObject *parent = 0);
    ~Segment();
    virtual Segment *ReadSegmentInMemory();
    virtual Segment *ReadDatasetsInMemory();
    virtual void ComposeSegmentImage();

    virtual void RenderSegmentlineInTexture( int channel, int nbrLine, int nbrTotalLine ); //not pure virtual : exception Metop ,GAC and VIIRS

    virtual void initializeMemory();
    virtual void resetMemory();
//    virtual void cleanupMemory();
    void RenderSatPath(QPainter *painter, QColor color);
    void sphericalToPixel(double lon, double lat, int &x, int &y, int devwidth, int devheight);
    void drawLineCyl(double lon1, double lat1, double lon2, double lat2, QPainter *painter);
    void RenderPosition(QPainter *painter);
    int pnpoly(int nvert, QPoint points[], int testx, int testy);
    bool ToggleSelected();

    virtual void RenderEarthLocationsGL();
    virtual int ReadNbrOfLines();

    virtual void ComposeSegmentLCCProjection(int inputchannel);
    virtual void ComposeSegmentGVProjection(int inputchannel);
    virtual void ComposeSegmentSGProjection(int inputchannel);

    virtual void RecalculateProjection();

    //void RenderSegmentContourline(float lat_first, float lon_first, float lat_last, float lon_last);
    void RenderSegmentlineInTextureRad(int channel, double earth_loc_lat_first,double earth_loc_lon_first, double earth_loc_lat_last,
                                    double earth_loc_lon_last, double earth_loc_altitude, int segmentheight);
    // void RenderSegmentInTexture(int channel, int nbrTotalLine);

    void NormalizeSegment(bool channel_3_select);

    bool IsSelected() { return segmentselected; }
    QString GetTle_line1() { return line1; }
    QString GetTle_line2() { return line2; }
    int GetNbrOfLines() { return NbrOfLines; }
    //void setImageReady() { image_ready = true; }
    //void resetImageReady() { image_ready = false; }
    //bool isImageReady() { return image_ready; }
    void setStartLineNbr(int nbr) { startLineNbr = nbr; }
    int getStartLineNbr() { return startLineNbr; }
    qint32 getProjectionX(int line, int pixelx);
    qint32 getProjectionY(int line, int pixelx);
    QRgb getProjectionValue(int line, int pixelx);

    bool segmentok;   // check if segment is read
    bool segmentselected; // selected for display
    bool segmentshow;     // show the segment in the map

    double julian_sensing_start, julian_sensing_end;
    double julian_state_vector;
    QDateTime qdatetime_start;
    QSgp4Date qsensingstart;
    QSgp4Date qsensingend;

    QFileInfo fileInfo;
    eSegmentType segtype;
    QString segment_type;

    double lat_start_deg, lon_start_deg, lat_end_deg, lon_end_deg;
    double lat_start_rad, lon_start_rad, lat_end_rad, lon_end_rad;

    QVector3D vec1, vec2;
    QVector2D winvec1, winvec2;
    QVector2D winvecend1, winvecend2;
    QVector2D winvecend3, winvecend4;

    QScopedArrayPointer<unsigned short> ptrbaChannel[5];
    QScopedArrayPointer<unsigned short> ptrbaVIIRS[3];
    QScopedArrayPointer<float> ptrbaVIIRSDNB;

    QString line1;
    QString line2;
    long stat_max_ch[5];
    long stat_min_ch[5];
//    long stat_3_0_max_ch;
//    long stat_3_1_max_ch;
//    long stat_3_0_min_ch;
//    long stat_3_1_min_ch;
    long list_stat_max_ch[5];
    long list_stat_min_ch[5];
//    long list_stat_3_0_max_ch;
//    long list_stat_3_1_max_ch;
//    long list_stat_3_0_min_ch;
//    long list_stat_3_1_min_ch;

    long active_pixels[5];

    unsigned long segment_stats_ch[5][1024];
    quint16 lut_ch[5][1024];

    int NbrOfLines;
    int startLineNbr;

    int earth_views_per_scanline;

    QScopedArrayPointer<float> earthloc_lon;
    QScopedArrayPointer<float> earthloc_lat;

    QScopedArrayPointer<float> solar_zenith_angle; // 1080 X 103
    QScopedArrayPointer<float> satellite_zenith_angle;
    QScopedArrayPointer<float> solar_azimuth_angle;
    QScopedArrayPointer<float> satellite_azimuth_angle;

    QScopedPointer<QTle> qtle;
    QScopedPointer<QSgp4> qsgp4;

    double minutes_since_state_vector;
    double minutes_sensing;

    QGeodetic cornerpointfirst1;
    QGeodetic cornerpointlast1;
    QGeodetic cornerpointfirst2;
    QGeodetic cornerpointlast2;

    bool channel_3a_3b[1080];


protected:

    void CalculateCornerPoints();

    quint32 cnt_mphr;
    quint32 cnt_sphr;
    quint32 cnt_ipr;
    quint32 cnt_geadr;
    quint32 cnt_giadr;
    quint32 cnt_veadr;
    quint32 cnt_viadr;
    quint32 cnt_mdr;
    quint32 cnt_unknown;


    SatelliteList *satlist;


    qint32 earth_location_lon_first[2];
    qint32 earth_location_lat_first[2];
    qint32 earth_location_lon_last[2];
    qint32 earth_location_lat_last[2];
    quint32 earth_location_altitude[2];

    //bool image_ready;

    QString satname;
    
    QScopedArrayPointer<int> projectionCoordX;
    QScopedArrayPointer<int> projectionCoordY;
    QScopedArrayPointer<QRgb> projectionCoordValue;

signals:
    //void segmentimagecomposed();
    
public slots:
    
};

#endif // SEGMENT_H
