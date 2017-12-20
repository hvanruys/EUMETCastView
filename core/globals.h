//
// globals.h
//
#ifndef GLOBALS_H
#define GLOBALS_H

#pragma once

#include "math.h"
#include <QPoint>
#include <QVector3D>
#include <QtCore/qmath.h>
#include <QDate>

enum eSegmentType
{
    SEG_NONE = 0,
    SEG_METOP,
    SEG_NOAA,
    SEG_HRP,
    SEG_GAC,
    SEG_VIIRSM,
    SEG_VIIRSDNB,
    SEG_OLCIEFR,
    SEG_OLCIERR,
    SEG_SLSTR,
    SEG_DATAHUB_OLCIEFR,
    SEG_DATAHUB_OLCIERR,
    SEG_DATAHUB_SLSTR,
    SEG_HRPT_METOPA,
    SEG_HRPT_METOPB,
    SEG_HRPT_NOAA19,
    SEG_HRPT_M01,
    SEG_HRPT_M02
};

enum eImageType
{
    IMAGE_NONE = 0,
    IMAGE_AVHRR_CH1,
    IMAGE_AVHRR_CH2,
    IMAGE_AVHRR_CH3,
    IMAGE_AVHRR_CH4,
    IMAGE_AVHRR_CH5,
    IMAGE_AVHRR_COL,
    IMAGE_AVHRR_EXPAND,
    IMAGE_GEOSTATIONARY,
    IMAGE_PROJECTION,
    IMAGE_VIIRSM,
    IMAGE_VIIRSDNB,
    IMAGE_OLCI,
    IMAGE_SLSTR
};

enum eProjectionType
{
    PROJ_NONE = 0,
    PROJ_AVHRR,
    PROJ_GEOSTATIONARY,
    PROJ_VIIRSM,
    PROJ_VIIRSDNB,
    PROJ_OLCI_EFR,
    PROJ_OLCI_ERR,
    PROJ_SLSTR
};

enum class eGeoSatellite {
    MET_10 = 0,
    MET_9 = 1,
    MET_8 = 2,
    GOMS2 = 3,
    FY2E = 4,
    FY2G = 5,
    GOES_15 = 6,
    GOES_16 = 7,
    H8 = 8,
    NOGEO = 10
};

// structures
struct floatVector {
        float x, y, z;
};

//struct LonLatPair {
//        int lonmicro;
//        int latmicro;
//};

struct Mapping {
        float u, v;
};

// vector-file
//struct VxpFeature {
//        int	 nVerts;
//        floatVector	*pVerts;
//        LonLatPair *pLonLat;
//};

//struct Vxp {
//        int         nFeatures;
//        VxpFeature  *pFeatures;
//};

struct point { int x, y; char c; };
struct line { struct point p1, p2; };

struct SphericalVector {
        float lon, lat, radius;
};



#define R2D     57.2957795131
#define D2R     0.0174532925199
#define sign2(x) (( x > 0 ) - ( x < 0 ))

//const double PI           = 3.141592653589793238;
//const double TWOPI        = 2.0 * PI;
const double PIO2         = 1.5707963267949; /* Pi/2 */
const double RADS_PER_DEG = 3.141592653589793238 / 180.0;

const double GM           = 398601.2;   // Earth gravitational constant, km^3/sec^2
const double GEOSYNC_ALT  = 42241.892;  // km
const double EARTH_DIA    = 12800.0;    // km
const double DAY_SIDERAL  = (23 * 3600) + (56 * 60) + 4.09;  // sec
const double DAY_24HR     = (24 * 3600);   // sec
const double XKMPER       = 6.378137E3;      /* Earth radius km */
const double AE           = 1.0;
//const double AU           = 149597870.0;  // Astronomical unit (km) (IAU 76)
const double SR           = 696000.0;     // Solar radius (km)      (IAU 76)
const double TWOTHRD      = 2.0 / 3.0;
const double XKMPER_WGS72 = 6378.135;            // Earth equatorial radius - km (WGS '72)
const double XKMPER_WGS84 = 6378.137;            // Earth equatorial radius - km (WGS '84)
//const double F            = 1.0 / 298.257223563; // Earth flattening (WGS '84)
const double GE           = 398600.8;     // Earth gravitational constant (WGS '72)
const double J2           = 1.0826158E-3; // J2 harmonic (WGS '72)
const double J3           = -2.53881E-6;  // J3 harmonic (WGS '72)
const double J4           = -1.65597E-6;  // J4 harmonic (WGS '72)
const double CK2          = J2 / 2.0;
const double CK4          = -3.0 * J4 / 8.0;
const double XJ3          = J3;
const double E6A          = 1.0e-06;
const double QO           = AE + 120.0 / XKMPER_WGS72;
const double S            = AE + 78.0  / XKMPER_WGS72;
const double HR_PER_DAY   = 24.0;          // Hours per day   (solar)
const double MIN_PER_DAY  = 1440.0;        // Minutes per day (solar)
const double SEC_PER_DAY  = 86400.0;       // Seconds per day (solar)
//const double OMEGA_E      = 1.00273790934; // earth rotation per sideral day
const double XKE          = sqrt(3600.0 * GE /           //sqrt(ge) ER^3/min^2
                                (XKMPER_WGS72 * XKMPER_WGS72 * XKMPER_WGS72)); 
const double QOMS2T       = pow((QO - S), 4);            //(QO - S)^4 ER^4
const double R_MAJOR_A_WGS84 = 6378137;
const double R_MAJOR_B_WGS84 = 6356752.3142;


// Utility functions
double sqr   (const double x);
double Fmod2p(const double arg);
double AcTan (const double sinx, double cosx);


double rad2deg(const double);
double deg2rad(const double);
double ArcCos(double arg);
int Sign(double arg);
double ArcSin(double arg);
float ArcSinf(float arg);

void LonLat2Point(float lat, float lon, QVector3D *pos, float radius);
void LonLat2PointRad(float lat, float lon, QVector3D *pos, float radius);
void LonLat2Vector(float lat, float lon, QVector3D *pos, float radius);
void Point2LonLat(double *lat_rad, double *lon_rad, double *radius, QVector3D pos );
void Pos2LatLonAlt(double *lat_rad, double *lon_rad, double *radius, QVector3D pos );
double adjust_lon_rad(double x);
double adjust_lon_deg(double x);
double longitudediffdeg( double lon1, double lon2);
QDateTime julianDoubleToDateTime(double julian);



void sphericalToPixel(double lon, double lat, int &x, int &y, int devwidth, int devheight);
int ccw(struct point p0, struct point p1, struct point p2 );
int intersect(struct line l1, struct line l2);
int pnpoly(int nvert, const QPoint *points, int testx, int testy);
void sortSphericalVectorLon(SphericalVector arr[], int size);

int Min(const int *Numbers, const int Count);
int Max(const int *Numbers, const int Count);



#endif
