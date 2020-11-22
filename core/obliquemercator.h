#ifndef OBLIQUEMERCATOR_H
#define OBLIQUEMERCATOR_H

#include <QLabel>
#include "segmentimage.h"
#include "gshhsdata.h"
#include "avhrrsatellite.h"

#define EPSLN	1.0e-3
class AVHRRSatellite;

class ObliqueMercator : public QObject
{
    Q_OBJECT

public:
    explicit ObliqueMercator(QObject *parent = 0, AVHRRSatellite *seglist = 0);
    ~ObliqueMercator();

    void Initialize(double r_maj, double r_min, eProjectionType projtype);
    //void InitializeSpherical(eProjectionType projtype);

    void CreateMapFromVIIRS(eSegmentType type, bool combine);
    void CreateMapFromMERSI(eSegmentType type, bool combine);
    void CreateMapFromAVHRR(eSegmentType type, int inputchannel);


    bool map_forward(double lon_rad, double lat_rad, double &map_x, double &map_y);
    bool map_inverse(double map_x, double map_y, double &lon_rad, double &lat_rad);

    int getProjectionWidth() { return image_width; }
    int getProjectionHeight() { return image_height; }


private:
    double tsfnz(double eccent, double phi, double sinphi);
    bool omerfor(double lon, double lat, double *x, double *y);
    //bool omerforspherical(double lon, double lat, double *x, double *y);

    void GetMinMaxXBoundingBox(eSegmentType type, double *boundingbox_min_x, double *boundingbox_max_x, double *boundingbox_min_y, double *boundingbox_max_y);
    void GetMinMaxXBoundingBoxAVHRR(eSegmentType type, double *boundingbox_min_x, double *boundingbox_max_x, double *boundingbox_min_y, double *boundingbox_max_y);
    //void GetMinMaxXBoundingBoxAVHRRSpherical(eSegmentType type, double *boundingbox_min_x, double *boundingbox_max_x, double *boundingbox_min_y, double *boundingbox_max_y);

    AVHRRSatellite *segs;

    double azimuth;
    double r_major;		/* major axis 				*/
    double r_minor;		/* minor axis 				*/
    double scale_factor;	/* scale factor				*/
    double lon_origin;	/* center longitude			*/
    double lat_origin;	/* center latitude			*/
    double e,es;		/* eccentricity constants		*/
    double false_northing;	/* y offset in meters			*/
    double false_easting;	/* x offset in meters			*/
    double sin_p20,cos_p20;	/* sin and cos values			*/
    double bl;
    double al;
    double d;
    double el,u;
    double singam,cosgam;
    double sinaz,cosaz;

    // spherical
    double lon_p1, lat_p1;
    double sinazimuth, cosazimuth;

    QPixmap pmGeneral;
    QPixmap pmScaled_res;

    int map_width;
    int map_height;
    int image_width;
    int image_height;
    double map_extend_x, map_extend_y;
    double boundingbox_max_x;
    double boundingbox_min_x;
    double boundingbox_max_y;
    double boundingbox_min_y;


    double deltaboundingboxX;
    double deltaboundingboxY;

signals:
    
public slots:
    
};

#endif // OBLIQUEMERCATOR_H
