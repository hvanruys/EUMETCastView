#ifndef STEREOGRAPHIC_H
#define STEREOGRAPHIC_H

#include <QObject>
#include "avhrrsatellite.h"

//! This is the object used for the Polar Stereographic projection.
class PolarStereo;

class StereoGraphic : public QObject
{
    Q_OBJECT

public:

    explicit StereoGraphic(QObject *parent = 0, AVHRRSatellite *seglist = 0);
    void Initialize(double center_lon, double center_lat, double inscale, int imagewidth, int imageheight, int easting, int northing);
    bool inverse(double x, double y, double &lon_rad, double &lat_rad);
    bool forward(double lon_rad, double lat_rad, double &x, double &y);
    bool map_forward(double lon_rad, double lat_rad, double &map_x, double &map_y);
    bool map_inverse(double map_x, double map_y, double &lon_rad, double &lat_rad);
    void CreateMapFromAVHRR(int inputchannel, eSegmentType type);
    void CreateMapFromVIIRS();
    void CreateMapFromGeostationary();

protected:

private:

    AVHRRSatellite *segs;
    double r_major;		/* major axis 				*/
    double lon_center;	/* Center longitude (projection center) */
    double lat_origin;	/* center latitude			*/
    int false_northing;
    int false_easting;
    double sin_p10;		/* sin of center latitude		*/
    double cos_p10;		/* cos of center latitude		*/

    double scale;
    int mapdeltax;
    int mapdeltay;
    double map_radius;
    int map_width;
    int map_height;
    int image_width;
    int image_height;

};

#endif

