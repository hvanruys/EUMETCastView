#ifndef GENERALVERTICALPERSPECTIVE_H
#define GENERALVERTICALPERSPECTIVE_H

#include <QLabel>
#include "gshhsdata.h"
#include "xmlvideoreader.h"
#include "rssvideo.h"

#define EPSLN	1.0e-3
class RSSVideo;

class GeneralVerticalPerspective : public QObject
{
    Q_OBJECT

public:
    explicit GeneralVerticalPerspective(XMLVideoReader *reader = 0, RSSVideo *video = 0, QImage *imGeostationary = 0, QObject *parent = 0);
    ~GeneralVerticalPerspective();

    double Initialize(double lonmapdeg, double latmapdeg, double heightmapkm, double scaling, double falseeasting, double falsenorthing, int imagewidth, int imageheight);

    void CreateMapFromGeoStationary(QPainter *painter, int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla);

    bool map_forward(double lon_rad, double lat_rad, double &map_x, double &map_y);
    bool map_forward_neg_coord(double lon_rad, double lat_rad, double &map_x, double &map_y);
    bool map_inverse(double map_x, double map_y, double &lon_rad, double &lat_rad);
    bool genpersfor(double lon, double lat, double *x, double *y);
    bool genpersinv(double x, double y, double *lon, double *lat);
    double asinz(double con);
    int getProjectionWidth() { return image_width; }
    int getProjectionHeight() { return image_height; }

    QImage *imageProjection;
    QImage *imageGeostationary;


private:
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

    int map_width;
    int map_height;
    int image_width;
    int image_height;
    double map_radius;
    //int *map_extents_min;
    //int *map_extents_max;
    XMLVideoReader *reader;
    RSSVideo *video;




signals:
    
public slots:
    
};

#endif // GENERALVERTICALPERSPECTIVE_H
