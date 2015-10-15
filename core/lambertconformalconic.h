#ifndef LAMBERTCONFORMALCONIC_H
#define LAMBERTCONFORMALCONIC_H

#include <QObject>
#include "avhrrsatellite.h"

class LambertConformalConic : public QObject
{
    Q_OBJECT

public:
    explicit LambertConformalConic(QObject *parent = 0, AVHRRSatellite *seglist = 0);
    ~LambertConformalConic();

    void Initialize(double r_maj, double r_min, double stdlat1, double stdlat2, double c_lon, double c_lat, int imagewidth, int imageheight, int corrX, int corrY);

    void CreateMapFromAVHRR(int inputchannel, eSegmentType type);
    void CreateMapFromVIIRS();
    void CreateMapFromGeostationary();


    bool map_forward(double lon_rad, double lat_rad, double &map_x, double &map_y);
    bool map_forward_viirs(double lon_rad, double lat_rad, double &map_x, double &map_y);
    bool map_inverse(double map_x, double map_y, double &lon_rad, double &lat_rad);
    bool lamccfor(double lon, double lat, double *x, double *y);
    bool lamccinv(double x, double y, double *lon, double *lat);

    void testmap();


    void calc_map_extents();


private:

    double tsfnz(double eccent, double phi, double sinphi);
    double msfnz (double eccent, double sinphi, double cosphi);
    double phi2z(double eccent, double ts);



    double min_x, max_x, min_y, max_y;

    AVHRRSatellite *segs;
    int map_width;
    int map_height;
    int image_width;
    int image_height;

    double mapdeltax;
    double mapdeltay;
    double *lon_array;
    double *lat_array;
    double Ay;
    double Ax;
    double Dy;
    double Dx;




    double r_major;                /* major axis                   */
    double r_minor;                /* minor axis                   */
    double es;                     /* eccentricity squared         */
    double e;                      /* eccentricity                 */
    double center_lon;             /* center longituted            */
    double center_lat;             /* center latitude              */
    double ns;                     /* ratio of angle between meridian*/
    double f0;                     /* flattening of ellipsoid      */
    double rh;                     /* height above ellipsoid       */
    double false_easting;          /* x offset in meters           */
    double false_northing;         /* y offset in meters           */

signals:
    
public slots:
    
};

#endif // LAMBERTCONFORMALCONIC_H
