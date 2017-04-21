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

enum eProjections {
    LCC = 0,
    GVP,
    SG
};

enum eSLSTRImageView
{
    OBLIQUE = 0,
    NADIR
};

class Segment : public QObject
{
    Q_OBJECT

public:
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

    virtual void ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    virtual void ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized);
    virtual void ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized);

    virtual void RecalculateProjection();

    //void RenderSegmentContourline(float lat_first, float lon_first, float lat_last, float lon_last);
    void RenderSegmentlineInTextureRad(int channel, double earth_loc_lat_first,double earth_loc_lon_first, double earth_loc_lat_last,
                                    double earth_loc_lon_last, double earth_loc_altitude, int segmentheight);
    // void RenderSegmentInTexture(int channel, int nbrTotalLine);

    void NormalizeSegment(bool channel_3_select);

    void setBandandColor(QList<bool> band, QList<int> color, QList<bool> invert);
    bool composeColorImage();
    bool IsSelected() { return segmentselected; }
    QString GetTle_line1() { return line1; }
    QString GetTle_line2() { return line2; }
    int GetNbrOfLines() { return NbrOfLines; }
    //void setImageReady() { image_ready = true; }
    //void resetImageReady() { image_ready = false; }
    //bool isImageReady() { return image_ready; }
    void setStartLineNbr(int nbr) { startLineNbr = nbr; endLineNbr = nbr + NbrOfLines; }
    int getStartLineNbr() { return startLineNbr; }
    qint32 getProjectionX(int line, int pixelx);
    qint32 getProjectionY(int line, int pixelx);
    QRgb getProjectionValue(int line, int pixelx);
    quint16 getProjectionValueRed(int line, int pixelx);
    quint16 getProjectionValueGreen(int line, int pixelx);
    quint16 getProjectionValueBlue(int line, int pixelx);
    int DecompressSegmentToTemp();
    int copy_data(struct archive *ar, struct archive *aw);


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

    QScopedArrayPointer<quint16> ptrbaChannel[5];
    QScopedArrayPointer<quint16> ptrbaChannelNormalized[5];
    QScopedArrayPointer<quint16> ptrbaVIIRS[3];
    QScopedArrayPointer<float> ptrbaVIIRSDNB;
    QScopedArrayPointer<quint16> ptrbaOLCI[3];
    QScopedArrayPointer<quint16> ptrbaOLCInormalized[3];
    QScopedArrayPointer<qint16> ptrbaSLSTR[3];
    QScopedArrayPointer<qint16> ptrbaSLSTRnormalized[3];

    QString line1;
    QString line2;
    long stat_max_ch[5];
    long stat_min_ch[5];
    long stat_max_norm_ch[5];
    long stat_min_norm_ch[5];
    long list_stat_max_ch[5];
    long list_stat_min_ch[5];

    long active_pixels[5];

    unsigned long segment_stats_ch[5][1024];
    quint16 lut_ch[5][1024];

    int NbrOfLines;
    int startLineNbr;
    int endLineNbr;

    int earth_views_per_scanline;

    QScopedArrayPointer<float> earthloc_lon;
    QScopedArrayPointer<float> earthloc_lat;

    QScopedArrayPointer<float> solar_zenith_angle;
    QScopedArrayPointer<float> satellite_zenith_angle;
    QScopedArrayPointer<float> solar_azimuth_angle;
    QScopedArrayPointer<float> satellite_azimuth_angle;
    QScopedArrayPointer<float> cos_solar_zenith_angle;
    QScopedArrayPointer<quint32> quality_flags;

    QScopedPointer<QTle> qtle;
    QScopedPointer<QSgp4> qsgp4;

    double minutes_since_state_vector;
    double minutes_sensing;

    QGeodetic cornerpointfirst1;
    QGeodetic cornerpointlast1;
    QGeodetic cornerpointcenter1;
    QGeodetic cornerpointfirst2;
    QGeodetic cornerpointlast2;
    QGeodetic cornerpointcenter2;



    bool channel_3a_3b[1080];

    QVector<QGeodetic> vectorfirst;
    QVector<QGeodetic> vectorlast;

    QVector<QVector2D> winvectorfirst;
    QVector<QVector2D> winvectorlast;
    QVector<QVector3D> vecvector;

protected:

    void CalculateCornerPoints();
    void CalculateDetailCornerPoints();
    void initializeProjectionCoord();
    void setupVector(double statevec, QSgp4Date sensing);


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
    QScopedArrayPointer<quint16> projectionCoordValueRed;
    QScopedArrayPointer<quint16> projectionCoordValueGreen;
    QScopedArrayPointer<quint16> projectionCoordValueBlue;

    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;



signals:
    //void segmentimagecomposed();
    
public slots:
    
};

#endif // SEGMENT_H
