#ifndef GENERALVERTICALPERSPECTIVE_H
#define GENERALVERTICALPERSPECTIVE_H

#include <QLabel>
#include "segmentimage.h"
#include "gshhsdata.h"
#include "avhrrsatellite.h"

#define EPSLN	1.0e-3
class AVHRRSatellite;
class GenVertNSP;

class GeneralVerticalPerspective : public QObject
{
    Q_OBJECT

public:
    explicit GeneralVerticalPerspective(QObject *parent = 0, AVHRRSatellite *seglist = 0);
    ~GeneralVerticalPerspective();

    double Initialize(double lonmapdeg, double latmapdeg, double heightmapkm, double scaling, int imagewidth, int imageheight);

    void CreateMapFromAVHRR(int inputchannel, eSegmentType type);
    void CreateMapFromVIIRS();
    void CreateMapFromGeoStationary();

    bool map_forward(double lon_rad, double lat_rad, double &map_x, double &map_y);
    bool map_forward_viirs(double lon_rad, double lat_rad, double &map_x, double &map_y);
    bool map_inverse(double map_x, double map_y, double &lon_rad, double &lat_rad);
    bool genpersfor(double lon, double lat, double *x, double *y);
    bool genpersinv(double x, double y, double *lon, double *lat);
    double asinz(double con);
    int getProjectionWidth() { return image_width; }
    int getProjectionHeight() { return image_height; }


private:
    AVHRRSatellite *segs;

    double ul_x, ul_y;
    int nl, ns;

    double lon_center;      /* Center longitude (projection center) */
    double lat_center;      /* Center latitude (projection center) 	*/

    double R;               /* Radius of the earth (sphere) */
    double false_easting;	/* x offset in meters			*/
    double false_northing;	/* y offset in meters			*/
    double sin_lat_o;       /* Sine of the center latitude 		*/
    double cos_lat_o;       /* Cosine of the center latitude 	*/
    double p;       		/* Height above sphere			*/
    double scale;
    int mapdeltax;
    int mapdeltay;

    QPixmap pmGeneral;
    QPixmap pmScaled_res;

    int map_width;
    int map_height;
    int image_width;
    int image_height;
    double map_radius;
    //int *map_extents_min;
    //int *map_extents_max;




signals:
    
public slots:
    
};

#endif // GENERALVERTICALPERSPECTIVE_H
