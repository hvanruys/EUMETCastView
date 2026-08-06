#ifndef PIXGEOCONVERSION_H
#define PIXGEOCONVERSION_H

#include "qsgp4globals.h"
#include <qtypes.h>
#include <QtCore/qmath.h>
#include <QtCore/QPointF>
#include <QList>

#define HALF_PI PIE*0.5
#define TWO_PI 	PIE*2.0
#define EPSLN	1.0e-3
#define R2D     57.2957795131
#define D2R     0.0174532925199
#define sign2(x) (( x > 0 ) - ( x < 0 ))

const double  SAT_HEIGHT = 42164.0;     /* distance from Earth centre to satellite     */
//const double  SAT_HEIGHT = 42164.16;     /* distance from Earth centre to satellite     */
const double  R_EQ       =  6378.169;   /* radius from Earth centre to equator         */
const double  R_POL      =  6356.5838;  /* radius from Earth centre to pol             */
//const double  R_EQ       =  6378.1370;   /* radius from Earth centre to equator         */
//const double  R_POL      =  6356.7523;  /* radius from Earth centre to pol             */
//const double  SUB_LON    =     0.0;     /* longitude of sub-satellite point in radiant */
                                            /* SUB_LON = 0.16572   9.5° MET-9 (MSG-2) */
                                            /* SUB_LON = 0.0       0.0° MET-10 (MSG-3) */

const double  CFAC_NONHRV  =  781648343;      /* scaling coefficients (see note above)  */
const double  LFAC_NONHRV  =  781648343;      /* scaling coefficients (see note above)  */

const double  CFAC_HRV     =   2344945030.;   /* scaling coefficients (see note above)  */
const double  LFAC_HRV     =   2344945030.;   /* scaling coefficients (see note above)  */

const double  CFAC_NONHRV_GOMS2     =   586236263.;   /* scaling coefficients (see note above)  */
const double  LFAC_NONHRV_GOMS2     =   586236263.;   /* scaling coefficients (see note above)  */

const double  CFAC_HRV_FENGYUN     =   1872000000;   /* scaling coefficients (see note above)  */
const double  LFAC_HRV_FENGYUN     =   1872000000;   /* scaling coefficients (see note above)  */

const double  CFAC_NONHRV_FENGYUN     =   468000000;   /* scaling coefficients (see note above)  */
const double  LFAC_NONHRV_FENGYUN     =   468000000;   /* scaling coefficients (see note above)  */

const double  CFAC_NONHRV_MET7     =   1043037809.;   /* scaling coefficients (see note above)  */
const double  LFAC_NONHRV_MET7     =   1043037809.;   /* scaling coefficients (see note above)  */

const double  CFAC_NONHRV_GOES     =   585352820.;   /* scaling coefficients (see note above)  */
const double  LFAC_NONHRV_GOES     =   585352820.;   /* scaling coefficients (see note above)  */

const double  CFAC_NONHRV_GOES16   =   1474382050.*1.5;   /* scaling coefficients (see note above)  */
const double  LFAC_NONHRV_GOES16   =   1474382050.*1.5;   /* scaling coefficients (see note above)  */

const double  CFAC_NONHRV_MTSAT     =   586315045.;   /* scaling coefficients (see note above)  */
const double  LFAC_NONHRV_MTSAT     =   586315045.;   /* scaling coefficients (see note above)  */

const double  CFAC_NONHRV_H9     =   1172050000.; //20466275.;   /* scaling coefficients (see note above)  */
const double  LFAC_NONHRV_H9     =   1172050000.; //20466275.;   /* scaling coefficients (see note above)  */

const long    COFF_NONHRV  =        1856;      /* scaling coefficients (see note above)  */
const long    LOFF_NONHRV  =        1856;      /* scaling coefficients (see note above)  */

const long    COFF_HRV     =        5566;      /* scaling coefficients (see note above)  */
const long    LOFF_HRV     =        5566;      /* scaling coefficients (see note above)  */

const long    COFF_NONHRV_GOMS2  =        1392;      /* scaling coefficients (see note above)  */
const long    LOFF_NONHRV_GOMS2  =        1392;      /* scaling coefficients (see note above)  */

const long    COFF_HRV_FENGYUN  =        4576;      /* scaling coefficients (see note above)  */
const long    LOFF_HRV_FENGYUN  =        4576;      /* scaling coefficients (see note above)  */

const long    COFF_NONHRV_FENGYUN  =        1144;      /* scaling coefficients (see note above)  */
const long    LOFF_NONHRV_FENGYUN  =        1144;      /* scaling coefficients (see note above)  */

const long    COFF_NONHRV_MET7  =        2500;      /* scaling coefficients (see note above)  */
const long    LOFF_NONHRV_MET7  =        2500;      /* scaling coefficients (see note above)  */

const long    COFF_NONHRV_GOES  =        1408;      /* scaling coefficients (see note above)  */
const long    LOFF_NONHRV_GOES  =        1408;      /* scaling coefficients (see note above)  */

const long    COFF_NONHRV_GOES16 =       5424;      /* scaling coefficients (see note above)  */
const long    LOFF_NONHRV_GOES16 =       5424;      /* scaling coefficients (see note above)  */

const long    COFF_NONHRV_H9 =        2750;      /* scaling coefficients (see note above)  */
const long    LOFF_NONHRV_H9 =        2750;      /* scaling coefficients (see note above)  */

enum SCAN_GEOMETRIES {
  GOES,
  GEOS
};

struct GridParams
{
    double lambda_0;
    double phi_0;
    double azimuth_sampling;
    double elevation_sampling;
};

// Structure to hold lat/lon/row/col conversion result
struct ConversionResult
{
    std::vector<double> values;
    std::vector<bool> valid;
};

// Get grid parameters for given SSD

// Convert latitude/longitude to FCI grid row/column
struct LatLonToGridResult
{
    std::vector<double> rows;
    std::vector<double> columns;
};

// Structure for grid to lat/lon conversion result
struct GridToLatLonResult
{
    std::vector<double> lons;
    std::vector<double> lats;
    std::vector<bool> earth_mask;
};

const double NAN_VALUE = std::numeric_limits<double>::quiet_NaN();

class pixgeoConversion
{
public:
    pixgeoConversion();

    int pixcoord2geocoord(double sub_lon_deg, int column, int row, int coff, int loff, double cfac, double lfac, double *latitude, double *longitude);
    double calc_sa(double sub_lon_deg, int column, int row, int coff, int loff, double cfac, double lfac, double *latitude, double *longitude);
    int geocoord2pixcoord(double sub_lon_deg, double latitude, double longitude, int coff, int loff, double cfac, double lfac, int *column, int *row);
    /**
     * FCI grid column/row to latitude/longitude - the exact inverse of
     * geocoord2pixcoordFCI, and the only correct way to geolocate an FCI pixel.
     *
     * Both indices are 1-based, as the files count them: column 1 is the west
     * limb, row 1 the south. That is the same convention start_position_row and
     * start_position_column arrive in, so the reader's placement and this agree
     * without a flip.
     *
     * Do not reach for the generic pixcoord2geocoord here. It describes the MSG
     * grid on the MSG ellipsoid, and driving it from the INI's COFF/CFAC lands
     * two to four kilometres away from where FCI actually puts a pixel - which
     * is what once put the land/sea mask a few pixels off every shoreline.
     *
     * @param ssd sampling in km: 1.0 for the 11136 grid, 2.0 for the 5568 one
     * @return 0 on the disc, -1 behind the limb (lat/lon set to -999.999)
     */
    int pixcoord2geocoordFCI(double sub_lon_deg, int column, int row, double *latitude, double *longitude, double ssd = 1.0);
    int geocoord2pixcoordFCI(double sub_lon_deg, double latitude, double longitude, int *column, int *row, double ssd = 1.0);


    int nint(double val);
    void earth_to_fgf_(const int  *sat, const double *lon_degrees, const double *lat_degrees, const double *scale_x, const double *offset_x,
                       const double *scale_y, const double *offset_y, const double *sub_lon_degrees, double *fgf_x, double *fgf_y);
    void fgf_to_earth_(const int *sat, const double *fgf_x, const double *fgf_y, const double *scale_x,
                        const double *offset_x, const double *scale_y, const double *offset_y,
                        const double *sub_lon_degrees, double *lon_degrees, double *lat_degrees);
private:
    void earth_to_sat(const enum SCAN_GEOMETRIES scan_geom, const double lon_degrees, const double lat_degrees, const double sub_lon_degrees, double *lamda, double *theta);
    void sat_to_fgf_abi(const double lamda, const double theta, const double scale_x, const double offset_x, const double scale_y,
                    const double offset_y, double *fgf_x, double *fgf_y);
    void fgf_to_sat_abi(const double fgf_x, const double fgf_y, const double scale_x,
                    const double offset_x, const double scale_y, const double offset_y,
                    double *lamda, double *theta);
    void seviriidx_to_sat(const double fgf_x, const double fgf_y, const double scale_x,
                    const double offset_x, const double scale_y, const double offset_y,
                    double *lamda, double *theta);
    void jmaidx_to_sat(const double fgf_x, const double fgf_y, const double scale_x,
                    const double offset_x, const double scale_y, const double offset_y,
                    double *lamda, double *theta);
    void goes_to_geos(double *lamda, double *theta);
    void sat_to_seviriidx(const double lamda, const double theta, const double scale_x, const double offset_x, const double scale_y,
                    const double offset_y, double *fgf_x, double *fgf_y);
    void sat_to_jmaidx(const double lamda, const double theta, const double scale_x, const double offset_x, const double scale_y,
                    const double offset_y, double *fgf_x, double *fgf_y);
    void sat_to_earth(const enum SCAN_GEOMETRIES scan_geom, const double lamda, const double theta, const double sub_lon_degrees,
                      double *lon_degrees, double *lat_degrees);



    GridParams getGridParams(double ssd);
    LatLonToGridResult fci_latlon_to_grid(const std::vector<double> &lats,
                                          const std::vector<double> &lons,
                                          double ssd = 1.0);
    GridToLatLonResult fci_grid_to_latlon(const std::vector<double> &rows,
                                                            const std::vector<double> &cols,
                                                            double ssd = 1.0);


};

// Constants for MTG FCI navigation
namespace MTGFCIConstants {
// Earth ellipsoid parameters
//static constexpr double R_EQ = 6378.137;           // Equatorial radius (km)
//static constexpr double R_POL = 6356.7523142;      // Polar radius (km)
static constexpr double FLATTENING = 1.0 / 298.257223563;

// Geostationary orbit parameters
static constexpr double H = 42164.0;               // Geostationary radius (km)
static constexpr double LAMBDA_D = 0.0;            // Sub-satellite longitude (radians)

// Grid sampling angles (radians) for different SSDs
static constexpr double GRID_SAMPLING_05KM = 1.3971788e-5;
static constexpr double GRID_SAMPLING_1KM = 2.7943576e-5;
static constexpr double GRID_SAMPLING_2KM = 5.5887153e-5;

// Full disc dimensions and center positions
struct GridParams {
    int columns;
    int rows;
    double coff;     // Column offset (center position)
    double loff;     // Line offset (center position)
    double lambda0;  // Azimuth angle at first column (radians)
    double phi0;     // Elevation angle at first row (radians)
    double grid_sampling; // Grid sampling angle (radians)
};

// Grid parameters for 0.5 km SSD
static GridParams get_grid_05km() {
    return {22272, 22272, 11136.5, 11136.5,
            0.1555828471, -0.1555828471, GRID_SAMPLING_05KM};
}

// Grid parameters for 1 km SSD
static GridParams get_grid_1km() {
    return {11136, 11136, 5568.5, 5568.5,
            0.1555758612, -0.1555758612, GRID_SAMPLING_1KM};
}

// Grid parameters for 2 km SSD
static GridParams get_grid_2km() {
    return {5568, 5568, 2784.5, 2784.5,
            0.1555618893, -0.1555618893, GRID_SAMPLING_2KM};
}
};

// Each FCI sample separation distance has exactly one full disc size, so the
// width of a composed FCI image tells which grid its columns and rows belong
// to : 5568 columns is the 2 km grid, 11136 the 1 km grid, 22272 the 0.5 km one.
inline double fciSsdFromImageWidth(int imagewidth)
{
    if (imagewidth >= 22272)
        return 0.5;
    else if (imagewidth >= 11136)
        return 1.0;
    else
        return 2.0;
}

// Structure to hold geographic coordinates
struct GeoCoord {
    double lon;  // Longitude in degrees
    double lat;  // Latitude in degrees
};

// Structure to hold pixel coordinates (1-indexed as per MTG convention)
struct PixCoord {
    double col;  // Column (1-based)
    double row;  // Row (1-based)
};



class MTGLatMinMax {
public:

    MTGLatMinMax();
    qreal getLatMin(int index);
    qreal getLatMax(int index);
private:
    // Structure to hold the max and min in degrees of MTG segments (40)
    struct ListLatMinMax {
        qreal max;
        qreal min;
    };

    QList<ListLatMinMax> listlatminmax;
};

class GeostationaryConverter {
private:
    // Image dimensions
    const int imageWidth = 11136;
    const int imageHeight = 11136;

    // Earth parameters
    const qreal EARTH_RADIUS = 6378137.0;  // WGS84 equatorial radius in meters
    const qreal EARTH_FLATTENING = 1.0 / 298.257223563;  // WGS84 flattening
    const qreal SATELLITE_HEIGHT = 35786400.0;  // Geostationary orbit height in meters

    // Satellite position (default: 0° longitude, equator)
    qreal satelliteLongitude;  // in radians

    // Projection parameters (typical for full disk)
    qreal pixelSizeX;  // radians per pixel
    qreal pixelSizeY;  // radians per pixel

    // Convert degrees to radians
    qreal deg2rad(qreal deg) const {
        return qDegreesToRadians(deg);
    }

    // Convert radians to degrees
    qreal rad2deg(qreal rad) const {
        return qRadiansToDegrees(rad);
    }

public:
    // Constructor with satellite longitude in degrees
    GeostationaryConverter(qreal satLonDeg);
    QPointF pixelToGeo(qreal pixelX, qreal pixelY) const;
    QPointF geoToPixel(qreal lon, qreal lat) const;
    QPointF geoToPixel(const QPointF& geoCoord) const;
};


#endif // PIXGEOCONVERSION_H
