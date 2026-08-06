#include "pixgeoconversion.h"
#include "globals.h"
#include <QDebug>
#include <cmath>



#define DEG_TO_RAD (PIE/180.0)
#define RAD_TO_DEG (180.0/PIE)

//#define R_POL (6356.7523)     /* semi-minor axis (polar radius km) */
//#define R_EQ (6378.1370)       /* semi-major axis (equatorial radius km) */
#define R_EQ (6378.169)      /* radius from Earth centre to equator         */
#define R_POL (6356.5838)    /* radius from Earth centre to pol             */

#define F (1.0/298.257222101)  /* flattening */
#define FP (1.0/((1.0-F)*(1.0-F)))

#define H_MSG (42164.0)
#define H_GOESR (42164.16)
/*#define H (H_GOESR) */ // Geostationary Orbit Radius (spacecraft to barycenter distance) (km)
/*#define D (H*H - R_EQ*R_EQ)*/

/* FUTURE: accept as an input if needed */
/*#define ORIGINAL_SCAN_GEOM (GOES)*/

/* not easy to generate a NaN in c, so using the Geocat double missing value */
#define MISSING_VALUE_DOUBLE (-999.0)



pixgeoConversion::pixgeoConversion()
{
}

/* this function returns the nearest integer to the value val */
/* and is used in function geocoord2pixcoord */

int pixgeoConversion::nint(double val)
{
  double a=0.0; /* integral  part of val */
  double b=0.0; /* frational part of val */

  b = modf(val,&a);

  if ( b > 0.5 ){
    val = ceil(val);
  }
  else{
    val = floor(val);
  }

  return (int)val;

}


/**************************************************************
 * function pixcoord2geocoord                                 *
 *                                                            *
 * PURPOSE:                                                   *
 *   return the latitude and longitude of an MSG image        *
 *   for a given pair of pixel column and row.                *
 *   (based on the formulas given in Ref. [1])                *
 *                                                            *
 *                                                            *
 * DEPENDENCIES:                                              *
 *   none                                                     *
 *                                                            *
 *                                                            *
 * REFERENCE:                                                 *
 * [1] LRIT/HRIT Global Specification                         *
 *     (CGMS 03, Issue 2.6, 12.08.1999)                       *
 *     for the parameters used in the program.                *
 * [2] MSG Ground Segment LRIT/HRIT Mission Specific          *
 *     Implementation, EUMETSAT Document,                     *
 *     (EUM/MSG/SPE/057, Issue 6, 21. June 2006).             *
 *                                                            *
 *                                                            *
 * MODIFICATION HISTORY:                                      *
 *  Version 1.02                                              *
 *  08.08.2008 removed a bug in longi = atan(s2/s1) statement *
 *  30.11.2011 added HRV to the calculation                   *
 *             Implemented with introducing CFAC/LFAC in      *
 *             function call                                  *
 *   Copyright(c) EUMETSAT 2005, 2009, 2011                   *
 *                                                            *
 *                                                            *
 * INPUT:                                                     *
 *   row    (int) row-value of the pixel                      *
 *   column (int) line-value of the pixel                     *
 *   coff   (int) coefficient of the scalling function        *
 *                (see page 28, Ref [1])                      *
 *   loff   (int) coefficient of the scalling function        *
 *                (see page 28, Ref [1])                      *
 *   cfac  (real) image "spread" in the EW direction          *
 *   lfac  (real) image "spread" in the NS direction          *
 *                                                            *
 * OUTPUT:                                                    *
 *   latitude  (double) geographic Latitude of the given      *
 *                      pixel [Degrees]                       *
 *   longitude (double) geographic Longitude of the given     *
 *                      pixel [Degrees]                       *
 *                                                            *
 *                                                            *
 *************************************************************/

int pixgeoConversion::pixcoord2geocoord(double sub_lon_deg, int column, int row, int coff, int loff,
              double cfac, double lfac,
              double *latitude, double *longitude)
{

  double s1=0.0, s2=0.0, s3=0.0, sn=0.0, sd=0.0, sxy=0.0, sa=0.0;
  double x=0.0, y=0.0;
  double longi=0.0, lati=0.0;
  double sub_lon = sub_lon_deg*PIE/180.0;

  int c=0, l=0;

  c=column;
  l=row;


  /*  calculate viewing angle of the satellite by use of the equation  */
  /*  on page 28, Ref [1]. */

  x = pow(2,16) * ( (double)c - (double)coff) / (double)cfac ;
  y = pow(2,16) * ( (double)l - (double)loff) / (double)lfac ;

  /*  now calculate the inverse projection */

  /* first check for visibility, whether the pixel is located on the Earth   */
  /* surface or in space. 						     */
  /* To do this calculate the argument to sqrt of "sd", which is named "sa". */
  /* If it is negative then the sqrt will return NaN and the pixel will be   */
  /* located in space, otherwise all is fine and the pixel is located on the */
  /* Earth surface.                                                          */

  sa =  pow(SAT_HEIGHT * cos(x) * cos(y),2 )  - (cos(y)*cos(y) + (double)1.006803 * sin(y)*sin(y)) * (double)1737121856. ;

  /* produce error values */
//  if ( sa <= 407828.0 ) {
//    *latitude = -999.999;
//    *longitude = -999.999;
//    return (-1);
//  }
  if( sa < 0) {
    *latitude = -999.999;
    *longitude = -999.999;
    return (-1);
  }


  /* now calculate the rest of the formulas using equations on */
  /* page 25, Ref. [1]  */

  sd = sqrt( pow((SAT_HEIGHT * cos(x) * cos(y)),2)
         - (cos(y)*cos(y) + (double)1.006803 * sin(y)*sin(y)) * (double)1737121856. );
  sn = (SAT_HEIGHT * cos(x) * cos(y) - sd)
    / ( cos(y)*cos(y) + (double)1.006803 * sin(y)*sin(y) ) ;

  s1 = SAT_HEIGHT - sn * cos(x) * cos(y);
  s2 = sn * sin(x) * cos(y);
  s3 = -sn * sin(y);

  sxy = sqrt( s1*s1 + s2*s2 );

  /* using the previous calculations the inverse projection can be  */
  /* calculated now, which means calculating the lat./long. from    */
  /* the pixel row and column by equations on page 25, Ref [1].     */

  longi = atan(s2/s1) + sub_lon;
  lati  = atan(((double)1.006803*s3)/sxy);
  /* convert from radians into degrees */
  *latitude = lati*180./PIE;
  *longitude = longi*180./PIE;

  return (0);

}

double pixgeoConversion::calc_sa(double sub_lon_deg, int column, int row, int coff, int loff,
              double cfac, double lfac,
              double *latitude, double *longitude)
{

  double s1=0.0, s2=0.0, s3=0.0, sn=0.0, sd=0.0, sxy=0.0, sa=0.0;
  double x=0.0, y=0.0;
  double longi=0.0, lati=0.0;
  //double sub_lon = sub_lon_deg*PI/180.0;

  int c=0, l=0;

  c=column;
  l=row;


  /*  calculate viewing angle of the satellite by use of the equation  */
  /*  on page 28, Ref [1]. */

  x = pow(2,16) * ( (double)c - (double)coff) / (double)cfac ;
  y = pow(2,16) * ( (double)l - (double)loff) / (double)lfac ;

  /*  now calculate the inverse projection */

  /* first check for visibility, whether the pixel is located on the Earth   */
  /* surface or in space. 						     */
  /* To do this calculate the argument to sqrt of "sd", which is named "sa". */
  /* If it is negative then the sqrt will return NaN and the pixel will be   */
  /* located in space, otherwise all is fine and the pixel is located on the */
  /* Earth surface.                                                          */

  sa =  pow(SAT_HEIGHT * cos(x) * cos(y),2 )  - (cos(y)*cos(y) + (double)1.006803 * sin(y)*sin(y)) * (double)1737121856. ;

  /* produce error values */
  if ( sa <= 400000.0 ) {
    *latitude = -999.999;
    *longitude = -999.999;
    return (-1.0);
  }

  return sa;

}

/**************************************************************
 * function geocoord2pixcoord                                 *
 *                                                            *
 * PURPOSE:                                                   *
 *   return the pixel column and line of an MSG image         *
 *   for a given pair of latitude/longitude.                  *
 *   (based on the formulas given in Ref. [1])                *
 *                                                            *
 *                                                            *
 * DEPENDENCIES:                                              *
 *   none                                                     *
 *                                                            *
 *                                                            *
 * REFERENCE:                                                 *
 * [1] LRIT/HRIT Global Specification                         *
 *     (CGMS 03, Issue 2.6, 12.08.1999)                       *
 *     for the parameters used in the program                 *
 * [2] MSG Ground Segment LRIT/HRIT Mission Specific          *
 *     Implementation, EUMETSAT Document,                     *
 *     (EUM/MSG/SPE/057, Issue 6, 21. June 2006).             *
 *                                                            *
 *                                                            *
 * MODIFICATION HISTORY:                                      *
 *   Version 1.02                                             *
 *  30.11.2011 added HRV to the calculation                   *
 *             Implemented with introducing CFAC/LFAC in      *
 *             function call                                  *
 *   Copyright(c) EUMETSAT 2005, 2009, 2011                   *
 *                                                            *
 *                                                            *
 * INPUT:                                                     *
 *   latitude  (double) geographic Latitude of a point        *
 *                      [Degrees]                             *
 *   longitude (double) geographic Longitude of a point       *
 *                      [Degrees]                             *
 *   coff (int)   coefficient of the scalling function        *
 *                (see page 28, Ref [1])                      *
 *   loff (int)   coefficient of the scalling function        *
 *                (see page 28, Ref [1])                      *
 *   cfac  (real) image "spread" in the EW direction          *
 *   lfac  (real) image "spread" in the NS direction          *
 *                                                            *
 *                                                            *
 * OUTPUT:                                                    *
 *   row    (int) row-value of the wanted pixel               *
 *   column (int) column-value of the wanted pixel            *
 *                                                            *
 *************************************************************/

int pixgeoConversion::geocoord2pixcoord(double sub_lon_deg, double latitude, double longitude, int coff, int loff,
              double cfac, double lfac, int *column, int *row)
{
  int ccc=0, lll=0;

  double lati=0.0, longi=0.0;
  double c_lat=0.0;
  double lat=0.0;
  double lon=0.0;
  double r1=0.0, r2=0.0, r3=0.0, rn=0.0, re=0.0, rl=0.0;
  double xx=0.0, yy=0.0;
  double cc=0.0, ll=0.0;
  double dotprod=0.0;

  double sub_lon = sub_lon_deg*PIE/180.0;
  lati= latitude;
  longi= longitude;

  /* check if the values are sane, otherwise return error values */
  if (lati < -90.0 || lati > 90.0 || longi < -180.0 || longi > 180.0 ){
    *row = -999;
    *column = -999;
    return (-1);
  }


  /* convert them to radiants */
  lat = lati*PIE / (double)180.;
  lon = longi *PIE / (double)180.;

  /* calculate the geocentric latitude from the          */
  /* geograhpic one using equations on page 24, Ref. [1] */

  c_lat = atan ( ((double)0.993243*(sin(lat)/cos(lat)) ));


  /* using c_lat calculate the length form the Earth */
  /* centre to the surface of the Earth ellipsoid    */
  /* equations on page 23, Ref. [1]                  */

  re = R_POL / sqrt( ((double)1.0 - (double)0.00675701 * cos(c_lat) * cos(c_lat) ) );


  /* calculate the forward projection using equations on */
  /* page 24, Ref. [1]                                        */

  rl = re;
  r1 = SAT_HEIGHT - rl * cos(c_lat) * cos(lon - sub_lon);
  r2 = - rl *  cos(c_lat) * sin(lon - sub_lon);
  r3 = rl * sin(c_lat);
  rn = sqrt( r1*r1 + r2*r2 +r3*r3 );


  /* check for visibility, whether the point on the Earth given by the */
  /* latitude/longitude pair is visible from the satellte or not. This */
  /* is given by the dot product between the vectors of:               */
  /* 1) the point to the spacecraft,			               */
  /* 2) the point to the centre of the Earth.			       */
  /* If the dot product is positive the point is visible otherwise it  */
  /* is invisible.						       */

  dotprod = r1*(rl * cos(c_lat) * cos(lon - sub_lon)) - r2*r2 - r3*r3*(pow((R_EQ/R_POL),2));


//  if (dotprod <= 30000000 ){
  if (dotprod < 0.0 ){
    *column = -999;
    *row = -999;
    return (-1);
  }


  /* the forward projection is x and y */

  xx = atan( (-r2/r1) );
  yy = asin( (-r3/rn) );


  /* convert to pixel column and row using the scaling functions on */
  /* page 28, Ref. [1]. And finding nearest integer value for them. */


  cc = coff + xx *  pow(2,-16) * cfac ;
  ll = loff + yy *  pow(2,-16) * lfac ;


  ccc=nint(cc);
  lll=nint(ll);

  *column = ccc;
  *row = lll;

//  if(*row == 1850)
//       qDebug() << "column = " << *column << "  dotprod = " << dotprod;


  return (0);

}




/**
 * Transform fractional FGF coordinates to (lamda, theta) radians for ABI.
 *
 * @param[in] fgf_x fractional FGF coordinate, zero-based
 * @param[in] fgf_y fractional FGF coordinate, zero-based
 * @param[in] scale_x scaleFactor from the x coordinate variable
 * @param[in] offset_x addOffset from the x coordinate variable
 * @param[in] scale_y scaleFactor from the y coordinate variable
 * @param[in] offset_y addOffset from the y coordinate variable
 * @param[out] lamda units: radians
 * @param[out] theta units: radians
 */
void pixgeoConversion::fgf_to_sat_abi(const double fgf_x, const double fgf_y, const double scale_x,
                const double offset_x, const double scale_y, const double offset_y,
                double *lamda, double *theta) {
      *lamda = fgf_x*scale_x + offset_x;
      *theta = fgf_y*scale_y + offset_y;
}

/**
 * Transform fractional SEVIRI coordinates to (lamda, theta) radians.
 *
 * @param[in] SEVIRI fractional FGF coordinate, zero-based
 * @param[in] SEVIRI fractional FGF coordinate, zero-based
 * @param[in] offset_x is the COFF variable from HRIT Header
 * @param[in] scale_x is the CFAC variable from HRIT Header
 * @param[in] offset_y is the LOFF variable from HRIT Header
 * @param[in] scale_y is the LFAC variable from HRIT Header
 * @param[out] lamda units: radians
 * @param[out] theta units: radians
 *
 * REMINDER - EUMETSAT ASSUMES LOWER RIGHT as 0,0)
 */
void pixgeoConversion::seviriidx_to_sat(const double fgf_x, const double fgf_y, const double scale_x,
                const double offset_x, const double scale_y, const double offset_y,
                double *lamda, double *theta) {

       double frnt_factor =  pow( 2, 16 );

      *lamda = (frnt_factor * ( fgf_x - offset_x )) / scale_x;
      *theta = (frnt_factor * ( fgf_y - offset_y )) / scale_y;
}


/**
 * Transform fractional JMA coordinates to (lamda, theta) radians.
 *
 * @param[in] SEVIRI fractional FGF coordinate, zero-based
 * @param[in] SEVIRI fractional FGF coordinate, zero-based
 * @param[in] offset_x is the COFF variable from HRIT Header
 * @param[in] scale_x is the CFAC variable from HRIT Header
 * @param[in] offset_y is the LOFF variable from HRIT Header
 * @param[in] scale_y is the LFAC variable from HRIT Header
 * @param[out] lamda units: radians
 * @param[out] theta units: radians
 *
 * REMINDER - JMA ASSUMES UPPER LEFT as (0,0)
 * recall that lamda and theta are in radians, but JMA uses deg constants
 */
void pixgeoConversion::jmaidx_to_sat(const double fgf_x, const double fgf_y, const double scale_x,
                const double offset_x, const double scale_y, const double offset_y,
                double *lamda, double *theta) {

       double frnt_factor =  pow( 2, 16 );

      *lamda = ((frnt_factor * ( fgf_x - offset_x )) / scale_x) * DEG_TO_RAD;
      *theta = ((frnt_factor * ( fgf_y - offset_y )) / scale_y) * DEG_TO_RAD;
}


/**
 * Transform view angle coordinates in the GOES scan geometry frame to view angle coordinates
 * in the GEOS scan geometry frame.
 *
 *  @param[in, out] lamda units: radians
 *  @param[in, out] theta units: radians
 */
void pixgeoConversion::goes_to_geos(double *lamda, double *theta) {
     double theta_geos = asin( sin(*theta)*cos(*lamda) );
     double lamda_geos = atan( tan(*lamda)/cos(*theta) );
     *lamda = lamda_geos;
     *theta = theta_geos;
}

/**
 * Transform satellite view angle coordinates, known as the "intermediate" coordinates in the
 * CGMS Normalized Geostationary Projection, to geographic Earth coordinates.
 *
 * @param[in] lamda lamda (East-West) angle, units: radians
 * @param[in] theta theta (North-South) angle, units: radians
 * @param[in] sub_lon_degrees satellite subpoint longitude, units: degrees
 * @param[out] lon_degrees units: degrees
 * @param[out] lat_degrees units: degrees
 */
void pixgeoConversion::sat_to_earth(const enum SCAN_GEOMETRIES scan_geom, const double lamda, const double theta, const double sub_lon_degrees,
                  double *lon_degrees, double *lat_degrees) {

  double sub_lon_radians = sub_lon_degrees * DEG_TO_RAD;

  double x = lamda;
  double y = theta;
  enum SCAN_GEOMETRIES scan = scan_geom;


  double h = H_MSG;

  if (scan_geom == GOES) { /* convert from GOES to GEOS for transfrom below */
    goes_to_geos(&x, &y);
    h = H_GOESR;
  }


  double d = (h*h - R_EQ*R_EQ);
  double c1 = (h * cos(x) * cos(y)) * (h * cos(x) * cos(y));
  double c2 = (cos(y) * cos(y) + FP * sin(y) * sin(y)) * d;

  if (c1<c2) {
    *lon_degrees = MISSING_VALUE_DOUBLE;
    *lat_degrees = MISSING_VALUE_DOUBLE;
    return;
  }

  double s_d = sqrt(c1 - c2);

  double s_n = (h * cos(x) * cos(y) - s_d) / (cos(y) * cos(y) + FP * sin(y) * sin(y));

  double s_1 = h - s_n * cos(x) * cos(y);
  double s_2 = s_n * sin(x) * cos(y);
  double s_3 = -s_n * sin(y);


  double s_xy = sqrt(s_1*s_1 + s_2*s_2);
  double geographic_lon = atan(s_2/s_1) + sub_lon_radians;

  //WCS3
  double geographic_lat = atan(FP*(s_3/s_xy));

  if (scan_geom == GOES) { /* GOES has this flip of FP, while SEVIRI/MTSAT doesn't */
    geographic_lat = atan(-FP*(s_3/s_xy));
  }



  *lon_degrees = (RAD_TO_DEG*geographic_lon);
  *lat_degrees = RAD_TO_DEG*geographic_lat;


  // force output longitude to -180 to 180 range
  if (*lon_degrees < -180.0) *lon_degrees += 360.0;
  if (*lon_degrees > 180.0) *lon_degrees -= 360.0;
}

/**
 *  Transform fractional line/element coordinates to (longitude, latitude).
 *
 ***********************************
 * @param[in] sat - fixed grid identifier
 * @param[in] fgf_x fractional FGF coordinate, zero-based
 * @param[in] fgf_y fractional FGF coordinate, zero-based
 * For ABI
 * @param[in] scale_x scaleFactor from the x coordinate variable
 * @param[in] offset_x addOffset from the x coordinate variable
 * @param[in] scale_y scaleFactor from the y coordinate variable
 * @param[in] offset_y addOffset from the y coordinate variable
 * For EUMETSAT/JMA
 * @param[in] scale_x is the CFAC variable from HRIT Header
 * @param[in] offset_x is the COFF variable from HRIT Header
 * @param[in] scale_y is the LFAC variable from HRIT Header
 * @param[in] offset_y is the LOFF variable from HRIT Header
 * @param[in] sub_lon_degrees satellite subpoint longitude, units: degrees
 ***********************************
 * @param[out] lon_degrees units: degrees
 * @param[out] lat_degrees units: degrees
 */
void pixgeoConversion::fgf_to_earth_(const int *sat, const double *fgf_x, const double *fgf_y, const double *scale_x,
                    const double *offset_x, const double *scale_y, const double *offset_y,
                    const double *sub_lon_degrees, double *lon_degrees, double *lat_degrees) {


  double lamda, theta;
  enum SCAN_GEOMETRIES scan_geom;

  if (*sat == 1) {
    fgf_to_sat_abi(*fgf_x, *fgf_y, *scale_x, *offset_x, *scale_y, *offset_y, &lamda, &theta);
    scan_geom = GOES;
  }
  else if (*sat == 2) {
    seviriidx_to_sat(*fgf_x, *fgf_y, *scale_x, *offset_x, *scale_y, *offset_y, &lamda, &theta);
    scan_geom = GEOS;
  }
  else if (*sat == 3) {
    jmaidx_to_sat(*fgf_x, *fgf_y, *scale_x, *offset_x, *scale_y, *offset_y, &lamda, &theta);
    scan_geom = GEOS;
  }

  sat_to_earth(scan_geom, lamda, theta, *sub_lon_degrees, lon_degrees, lat_degrees);


}





/**
 * Transform Earth coordinates (lon,lat) to fractional line/element coordinates.
 *
 ***********************************
 * @param[in] sat - fixed grid identifier
 * @param[in] lon_degrees Longitude, units: degrees
 * @param[in] lat_degrees Latitude, units: degrees
 * For ABI
 * @param[in] scale_x scaleFactor from the x coordinate variable
 * @param[in] offset_x addOffset from the x coordinate variable
 * @param[in] scale_y scaleFactor from the y coordinate variable
 * @param[in] offset_y addOffset from the y coordinate variable
 * For EUMETSAT/JMA
 * @param[in] scale_x is the CFAC variable from HRIT Header
 * @param[in] offset_x is the COFF variable from HRIT Header
 * @param[in] scale_y is the LFAC variable from HRIT Header
 * @param[in] offset_y is the LOFF variable from HRIT Header
 ***********************************
 * @param[in] sub_lon_degrees satellite subpoint longitude, units: degrees
 * @param[out] fgf_x fractional fgf x coordinate
 * @param[out] fgf_y fractional fgf y coordinate



 */
void pixgeoConversion::earth_to_fgf_(const int  *sat, const double *lon_degrees, const double *lat_degrees, const double *scale_x, const double *offset_x,
                   const double *scale_y, const double *offset_y, const double *sub_lon_degrees, double *fgf_x, double *fgf_y)
{
  double lamda, theta;
  enum SCAN_GEOMETRIES scan_geom;

  if (*sat == 1) {
    scan_geom = GOES;
  }
  else if (*sat == 2) {
    scan_geom = GEOS;
  }
  else if (*sat == 3) {
    scan_geom = GEOS;
  }

  earth_to_sat(scan_geom, *lon_degrees, *lat_degrees, *sub_lon_degrees, &lamda, &theta);

  if (*sat == 1) {
    sat_to_fgf_abi(lamda, theta, *scale_x, *offset_x, *scale_y, *offset_y, fgf_x, fgf_y);
  }
  else if (*sat == 2) {
    sat_to_seviriidx(lamda, theta, *scale_x, *offset_x, *scale_y, *offset_y, fgf_x, fgf_y);
  }
  else if (*sat == 3) {
    sat_to_jmaidx(lamda, theta, *scale_x, *offset_x, *scale_y, *offset_y, fgf_x, fgf_y);
  }

}

/**
 * Transform geographic Earth coordinates to satellite view angle coordinate system
 * also known as the "intermediate" coordinate system in CGMS Normalized Geostationary Projection.
 *
 * @param[in] lon_degrees longitude, units: degrees
 * @param[in] lat_degrees latitude, units: degrees
 * @param[in] sub_lon_degrees satellite subpoint longitude, units: degrees
 * @param[out] lamda the x or East-West view angle, units: radians
 * @param[out] theta the y or North_South view angle, units: radians
 */

void pixgeoConversion::earth_to_sat(const enum SCAN_GEOMETRIES scan_geom, const double lon_degrees, const double lat_degrees, const double sub_lon_degrees, double *lamda, double *theta) {


  double h = H_MSG;
  enum SCAN_GEOMETRIES scan = scan_geom;

  if (scan_geom == GOES) { /* USE GOES-R Height */
    double h = H_GOESR;
  }

  double geographic_lat = lat_degrees*DEG_TO_RAD;
  double geographic_lon = lon_degrees*DEG_TO_RAD;
  double sub_lon = sub_lon_degrees * DEG_TO_RAD;

  double geocentric_lat = atan(((R_POL*R_POL)/(R_EQ*R_EQ))*tan(geographic_lat));

  double r_earth = R_POL/sqrt(1.0 -((R_EQ*R_EQ - R_POL*R_POL)/(R_EQ*R_EQ))*cos(geocentric_lat)*cos(geocentric_lat));

  double r_1 = h - r_earth*cos(geocentric_lat)*cos(geographic_lon - sub_lon);
  double r_2 = -r_earth*cos(geocentric_lat)*sin(geographic_lon - sub_lon);
  double r_3 = r_earth*sin(geocentric_lat);

  *lamda = MISSING_VALUE_DOUBLE;
  *theta = MISSING_VALUE_DOUBLE;



  if (r_1 > h) { // often two geoid intersect points, use the closer one.
    return;
  }

  if (scan_geom == GEOS) { // GEOS (eg. SEVIRI, MSG)  CGMS 03, 4.4.3.2, Normalized Geostationary Projection
  //WCS3 - theta was -r_3 in the original EUMETSAT/JMA code

    *lamda = atan(-r_2/r_1);
    *theta = asin(-r_3/sqrt(r_1*r_1 + r_2*r_2 + r_3*r_3));
  }
  else if (scan_geom == GOES) { // GOES (eg. GOES-R ABI)
    *lamda = asin(-r_2/sqrt(r_1*r_1 + r_2*r_2 + r_3*r_3));
    *theta = atan(r_3/r_1);
  }

/*  printf("%f\n",*lamda);
  printf("%f\n",*theta);*/

}

/**
 *  Transform (lamda, theta) in radians to fractional FGF coordinates for ABI.
 * @param[in] lamda the x or East-West view angle, units: radians
 * @param[in] theta the y or North_South view angle, units: radians
 * @param[in] scale_x scaleFactor from the x coordinate variable
 * @param[in] offset_x addOffset from the x coordinate variable
 * @param[in] scale_y scaleFactor from the y coordinate variable
 * @param[in] offset_y addOffset from the y coordinate variable
 * @param[out] fgf_x fractional fgf x coordinate
 * @param[out] fgf_y fractional fgf y coordinate
 */
void pixgeoConversion::sat_to_fgf_abi(const double lamda, const double theta, const double scale_x, const double offset_x, const double scale_y,
                const double offset_y, double *fgf_x, double *fgf_y) {
     *fgf_x = (lamda - offset_x)/scale_x;
     *fgf_y = (theta - offset_y)/scale_y;
}

/**
 *  Transform (lamda, theta) in radians to fractional EUMETSAT coordinates.
 * @param[in] lamda the x or East-West view angle, units: radians
 * @param[in] theta the y or North_South view angle, units: radians
 * @param[in] offset_x is the COFF variable from HRIT Header
 * @param[in] scale_x is the CFAC variable from HRIT Header
 * @param[in] offset_y is the LOFF variable from HRIT Header
 * @param[in] scale_y is the LFAC variable from HRIT Header
 * @param[out] fgf_x fractional EUMETSAT coordinate
 * @param[out] fgf_y fractional EUMETSAT coordinate
 */
void pixgeoConversion::sat_to_seviriidx(const double lamda, const double theta, const double scale_x, const double offset_x, const double scale_y,
                const double offset_y, double *fgf_x, double *fgf_y) {
     double frnt_factor =  pow( 2, -16 );

     *fgf_x = offset_x + (lamda *  frnt_factor * scale_x);
     *fgf_y = offset_y + (theta *  frnt_factor * scale_y);

/*     printf("fgf_x=%f  fgf_y=%f \n", *fgf_x, *fgf_y);*/


}

/**
 *  Transform (lamda, theta) in radians to fractional JMA coordinates.
 * @param[in] lamda the x or East-West view angle, units: radians
 * @param[in] theta the y or North_South view angle, units: radians
 * @param[in] offset_x is the COFF variable from HRIT Header
 * @param[in] scale_x is the CFAC variable from HRIT Header
 * @param[in] offset_y is the LOFF variable from HRIT Header
 * @param[in] scale_y is the LFAC variable from HRIT Header
 * @param[out] fgf_x fractional EUMETSAT coordinate
 * @param[out] fgf_y fractional EUMETSAT coordinate
 *
 * recall that lamda and theta are in radians, but JMA uses deg constants
 */
void pixgeoConversion::sat_to_jmaidx(const double lamda, const double theta, const double scale_x, const double offset_x, const double scale_y,
                const double offset_y, double *fgf_x, double *fgf_y) {
     double frnt_factor =  pow( 2, -16 );

     *fgf_x = offset_x + (lamda *  frnt_factor * scale_x * RAD_TO_DEG);
     *fgf_y = offset_y + (theta *  frnt_factor * scale_y * RAD_TO_DEG);

/*     printf("fgf_x=%f  fgf_y=%f \n", *fgf_x, *fgf_y);*/


}




/**
 * Convert pixel coordinates to geographic coordinates for MTG FCI
 * @param pix Pixel coordinates (1-based indexing: col and row start at 1)
 * @param grid Grid parameters for the specific SSD
 * @return Geographic coordinates (lon, lat in degrees)
 */
GeoCoord pixcoord2geocoord(const PixCoord& pix,
                           const MTGFCIConstants::GridParams& grid) {
    using namespace MTGFCIConstants;

    // Convert pixel coordinates to viewing angles
    // Note: columns are numbered 1 to N from west to east
    // rows are numbered 1 to N from south to north
    double lambda_s = grid.lambda0 - (pix.col - 1.0) * grid.grid_sampling;
    double phi_s = grid.phi0 + (pix.row - 1.0) * grid.grid_sampling;

    // Calculate intermediate values
    double cos_lambda = cos(lambda_s);
    double cos_phi = cos(phi_s);
    double sin_lambda = sin(lambda_s);
    double sin_phi = sin(phi_s);

    // Calculate viewing vector components
    double cos_lambda_cos_phi = cos_lambda * cos_phi;

    // Calculate sn (distance to Earth surface along viewing direction)
    double r_pol_sq = R_POL * R_POL;
    double r_eq_sq = R_EQ * R_EQ;
    double s4 = r_pol_sq / r_eq_sq;
    double sd = sqrt((H * cos_lambda_cos_phi) * (H * cos_lambda_cos_phi) -
                     (cos_phi * cos_phi + s4 * sin_phi * sin_phi) *
                         (H * H - r_eq_sq));

    double sn = (H * cos_lambda_cos_phi - sd) /
                (cos_phi * cos_phi + s4 * sin_phi * sin_phi);

    // Calculate satellite viewing vector components
    double s1 = H - sn * cos_lambda_cos_phi;
    double s2 = sn * sin_lambda * cos_phi;
    double s3 = -sn * sin_phi;

    // Calculate geographic coordinates
    double s_xy = sqrt(s1 * s1 + s2 * s2);

    GeoCoord geo;
    geo.lon = atan2(s2, s1) + LAMBDA_D;
    geo.lat = atan(s4 * s3 / s_xy);

    // Convert to degrees
    geo.lon *= 180.0 / M_PI;
    geo.lat *= 180.0 / M_PI;

    return geo;
}

/**
 * Convert geographic coordinates to pixel coordinates for MTG FCI
 * @param geo Geographic coordinates (lon, lat in degrees)
 * @param grid Grid parameters for the specific SSD
 * @return Pixel coordinates (1-based indexing) or {-1, -1} if point not visible
 */
PixCoord geocoord2pixcoord(const GeoCoord& geo,
                           const MTGFCIConstants::GridParams& grid) {
    using namespace MTGFCIConstants;

    // Convert to radians
    double lon_rad = geo.lon * M_PI / 180.0;
    double lat_rad = geo.lat * M_PI / 180.0;

    // Calculate geocentric latitude
    double c_lat = atan(R_POL * R_POL / (R_EQ * R_EQ) * tan(lat_rad));

    // Calculate position on Earth surface
    double r_l = R_POL / sqrt(1.0 - (1.0 - R_POL * R_POL / (R_EQ * R_EQ)) *
                                        cos(c_lat) * cos(c_lat));

    double r1 = H - r_l * cos(c_lat) * cos(lon_rad - LAMBDA_D);
    double r2 = -r_l * cos(c_lat) * sin(lon_rad - LAMBDA_D);
    double r3 = r_l * sin(c_lat);

    // Check visibility
    if (r1 * (H - R_EQ) - r2 * r2 - r3 * r3 < 0) {
        // Point not visible from satellite
        return {-1.0, -1.0};
    }

    // Calculate viewing angles
    double lambda_s = atan2(-r2, r1);
    double phi_s = atan(r3 / sqrt(r1 * r1 + r2 * r2));

    // Convert viewing angles to pixel coordinates
    PixCoord pix;
    pix.col = (grid.lambda0 - lambda_s) / grid.grid_sampling + 1.0;
    pix.row = (phi_s - grid.phi0) / grid.grid_sampling + 1.0;

    return pix;
}

// Example usage
// #include <iostream>

// int main() {
//     // Get grid parameters for 1 km SSD
//     auto grid = MTGFCIConstants::get_grid_1km();

//     // Example 1: Convert pixel to geographic coordinates
//     PixCoord pix1 = {5568.5, 5568.5};  // Center pixel
//     GeoCoord geo1 = pixcoord2geocoord(pix1, grid);
//     std::cout << "Pixel (" << pix1.col << ", " << pix1.row << ") -> "
//               << "Geo (" << geo1.lon << "°, " << geo1.lat << "°)\n";

//     // Example 2: Convert geographic to pixel coordinates
//     GeoCoord geo2 = {10.0, 45.0};  // Somewhere over Italy
//     PixCoord pix2 = geocoord2pixcoord(geo2, grid);
//     std::cout << "Geo (" << geo2.lon << "°, " << geo2.lat << "°) -> "
//               << "Pixel (" << pix2.col << ", " << pix2.row << ")\n";

//     // Example 3: Round trip test
//     GeoCoord geo3 = pixcoord2geocoord(pix2, grid);
//     std::cout << "Round trip: Pixel (" << pix2.col << ", " << pix2.row << ") -> "
//               << "Geo (" << geo3.lon << "°, " << geo3.lat << "°)\n";

//     return 0;
// }

// Constructor with satellite longitude in degrees
GeostationaryConverter::GeostationaryConverter(qreal satLonDeg) {
    satelliteLongitude = deg2rad(satLonDeg);

    // Calculate pixel size based on image dimensions
    // For full disk, angular extent is approximately ±81.3 degrees
    qreal maxScanAngle = deg2rad(81.3);
    pixelSizeX = (2.0 * maxScanAngle) / imageWidth;
    pixelSizeY = (2.0 * maxScanAngle) / imageHeight;
}

// Convert pixel coordinates (x, y) to geographic coordinates (lon, lat)
// Returns QPointF(longitude, latitude) in degrees
// Origin (0,0) is at top-left corner
// Returns QPointF(NaN, NaN) if point not visible
QPointF GeostationaryConverter::pixelToGeo(qreal pixelX, qreal pixelY) const {
    // Convert pixel to scanning angles (radians)
    // Center of image corresponds to (0, 0) scanning angles
    qreal x = (pixelX - imageWidth / 2.0) * pixelSizeX;
    qreal y = (imageHeight / 2.0 - pixelY) * pixelSizeY;

    // Distance from satellite to Earth center
    qreal H = SATELLITE_HEIGHT + EARTH_RADIUS;

    // Calculate intermediate values
    qreal cosx = qCos(x);
    qreal cosy = qCos(y);
    qreal sinx = qSin(x);
    qreal siny = qSin(y);

    // Calculate the distance to the Earth surface point
    qreal cosxcosy = cosx * cosy;
    qreal sd = H * H - EARTH_RADIUS * EARTH_RADIUS * (1.0 - cosxcosy * cosxcosy);

    // Check if point is visible from satellite
    if (sd < 0) {
        return QPointF(qQNaN(), qQNaN());  // Point not visible (beyond Earth limb)
    }

    qreal sn = (H * cosxcosy - qSqrt(sd)) / EARTH_RADIUS;

    // Calculate Cartesian coordinates
    qreal s1 = H - sn * EARTH_RADIUS * cosxcosy;
    qreal s2 = sn * EARTH_RADIUS * cosy * sinx;
    qreal s3 = -sn * EARTH_RADIUS * siny;

    // Convert to geodetic coordinates
    qreal lon = satelliteLongitude + qAtan2(s2, s1);
    qreal lat = qAtan2(s3, qSqrt(s1 * s1 + s2 * s2));

    // Apply ellipsoid correction for latitude
    qreal e2 = 2.0 * EARTH_FLATTENING - EARTH_FLATTENING * EARTH_FLATTENING;
    lat = qAtan(qTan(lat) / (1.0 - e2));

    return QPointF(rad2deg(lon), rad2deg(lat));
}

// Convert geographic coordinates (lon, lat) to pixel coordinates (x, y)
// Input: QPointF(longitude, latitude) in degrees
// Returns QPointF(pixelX, pixelY)
// Returns QPointF(NaN, NaN) if point not visible
QPointF GeostationaryConverter::geoToPixel(qreal lon, qreal lat) const {
    // Convert to radians
    qreal lonRad = deg2rad(lon);
    qreal latRad = deg2rad(lat);

    // Apply ellipsoid correction
    qreal e2 = 2.0 * EARTH_FLATTENING - EARTH_FLATTENING * EARTH_FLATTENING;
    qreal latGeocentric = qAtan((1.0 - e2) * qTan(latRad));

    // Calculate Cartesian coordinates on Earth surface
    qreal cosLat = qCos(latGeocentric);
    qreal sinLat = qSin(latGeocentric);
    qreal cosLon = qCos(lonRad - satelliteLongitude);
    qreal sinLon = qSin(lonRad - satelliteLongitude);

    // Distance from satellite to Earth center
    qreal H = SATELLITE_HEIGHT + EARTH_RADIUS;

    // Calculate if point is visible
    qreal cosc = cosLat * cosLon;
    if (H * (H - EARTH_RADIUS * cosc) < 0) {
        return QPointF(qQNaN(), qQNaN());  // Point not visible from satellite
    }

    // Calculate scanning angles
    qreal rl = EARTH_RADIUS / qSqrt(H * H - 2.0 * H * EARTH_RADIUS * cosc + EARTH_RADIUS * EARTH_RADIUS);

    qreal x = qAtan2(rl * cosLat * sinLon, H - rl * EARTH_RADIUS * cosc);
    qreal y = -qAtan2(rl * sinLat, qSqrt((H - rl * EARTH_RADIUS * cosc) *
                                             (H - rl * EARTH_RADIUS * cosc) +
                                         rl * rl * cosLat * cosLat * sinLon * sinLon));

    // Convert scanning angles to pixel coordinates
    qreal pixelX = imageWidth / 2.0 + x / pixelSizeX;
    qreal pixelY = imageHeight / 2.0 - y / pixelSizeY;

    return QPointF(pixelX, pixelY);
}

// Convenience overload using QPointF for geographic coordinates
QPointF GeostationaryConverter::geoToPixel(const QPointF& geoCoord) const {
    return geoToPixel(geoCoord.x(), geoCoord.y());
}

/* Maximum and minimum latidudes for MTG
veclatminmax  0  ; max =  -69.2419  min =  -80.0814
veclatminmax  1  ; max =  -59.8475  min =  -74.2498
veclatminmax  2  ; max =  -53.5037  min =  -65.8096
veclatminmax  3  ; max =  -48.3907  min =  -59.5561
veclatminmax  4  ; max =  -43.9502  min =  -54.2379
veclatminmax  5  ; max =  -39.9802  min =  -49.5518
veclatminmax  6  ; max =  -36.3348  min =  -45.3278
veclatminmax  7  ; max =  -32.9216  min =  -41.3692
veclatminmax  8  ; max =  -29.7108  min =  -37.5932
veclatminmax  9  ; max =  -26.6401  min =  -34.0258
veclatminmax  10  ; max =  -23.7023  min =  -30.5877
veclatminmax  11  ; max =  -20.8637  min =  -27.2897
veclatminmax  12  ; max =  -18.0964  min =  -24.0395
veclatminmax  13  ; max =  -15.4051  min =  -20.8972
veclatminmax  14  ; max =  -12.7579  min =  -17.8043
veclatminmax  15  ; max =  -10.1626  min =  -14.759
veclatminmax  16  ; max =  -7.60002  min =  -11.7664
veclatminmax  17  ; max =  -5.05215  min =  -8.80622
veclatminmax  18  ; max =  -2.529  min =  -5.85302
veclatminmax  19  ; max =  -0.00452185  min =  -2.92396
veclatminmax  20  ; max =  2.91328  min =  0.00452185
veclatminmax  21  ; max =  5.84166  min =  2.51995
veclatminmax  22  ; max =  8.79377  min =  5.04305
veclatminmax  23  ; max =  11.7528  min =  7.59085
veclatminmax  24  ; max =  14.759  min =  10.1533
veclatminmax  25  ; max =  18.0173  min =  12.7579
veclatminmax  26  ; max =  21.3152  min =  15.5872
veclatminmax  27  ; max =  24.7405  min =  18.4789
veclatminmax  28  ; max =  27.9595  min =  21.469
veclatminmax  29  ; max =  31.0826  min =  24.3168
veclatminmax  30  ; max =  34.2672  min =  27.0613
veclatminmax  31  ; max =  37.5885  min =  29.9137
veclatminmax  32  ; max =  41.3187  min =  32.9097
veclatminmax  33  ; max =  45.3114  min =  36.3221
veclatminmax  34  ; max =  49.5518  min =  39.9666
veclatminmax  35  ; max =  54.2458  min =  43.9502
veclatminmax  36  ; max =  59.5467  min =  48.3737
veclatminmax  37  ; max =  65.8148  min =  53.4836
veclatminmax  38  ; max =  74.0858  min =  59.8213
veclatminmax  39  ; max =  80.1184  min =  69.1945
*/
MTGLatMinMax::MTGLatMinMax()
{

    ListLatMinMax llmm;
    llmm.max =  -69.2419;  llmm.min =  -80.0814; listlatminmax.append(llmm);
    llmm.max =  -59.8475; llmm.min =  -74.2498; listlatminmax.append(llmm);
    llmm.max =  -53.5037; llmm.min =  -65.8096; listlatminmax.append(llmm);
    llmm.max =  -48.3907; llmm.min =  -59.5561; listlatminmax.append(llmm);
    llmm.max =  -43.9502; llmm.min =  -54.2379; listlatminmax.append(llmm);
    llmm.max =  -39.9802; llmm.min =  -49.5518; listlatminmax.append(llmm);
    llmm.max =  -36.3348; llmm.min =  -45.3278; listlatminmax.append(llmm);
    llmm.max =  -32.9216; llmm.min =  -41.3692; listlatminmax.append(llmm);
    llmm.max =  -29.7108; llmm.min =  -37.5932; listlatminmax.append(llmm);
    llmm.max =  -26.6401; llmm.min =  -34.0258; listlatminmax.append(llmm);
    llmm.max =  -23.7023; llmm.min =  -30.5877; listlatminmax.append(llmm);
    llmm.max =  -20.8637; llmm.min =  -27.2897; listlatminmax.append(llmm);
    llmm.max =  -18.0964; llmm.min =  -24.0395; listlatminmax.append(llmm);
    llmm.max =  -15.4051; llmm.min =  -20.8972; listlatminmax.append(llmm);
    llmm.max =  -12.7579; llmm.min =  -17.8043; listlatminmax.append(llmm);
    llmm.max =  -10.1626; llmm.min =  -14.759; listlatminmax.append(llmm);
    llmm.max =  -7.60002; llmm.min =  -11.7664; listlatminmax.append(llmm);
    llmm.max =  -5.05215; llmm.min =  -8.80622; listlatminmax.append(llmm);
    llmm.max =  -2.529; llmm.min =  -5.85302; listlatminmax.append(llmm);
    llmm.max =  -0.00452185; llmm.min =  -2.92396; listlatminmax.append(llmm);
    llmm.max =  2.91328; llmm.min =  0.00452185; listlatminmax.append(llmm);
    llmm.max =  5.84166; llmm.min =  2.51995; listlatminmax.append(llmm);
    llmm.max =  8.79377; llmm.min =  5.04305; listlatminmax.append(llmm);
    llmm.max =  11.7528; llmm.min =  7.59085; listlatminmax.append(llmm);
    llmm.max =  14.759; llmm.min =  10.1533; listlatminmax.append(llmm);
    llmm.max =  18.0173; llmm.min =  12.7579; listlatminmax.append(llmm);
    llmm.max =  21.3152; llmm.min =  15.5872; listlatminmax.append(llmm);
    llmm.max =  24.7405; llmm.min =  18.4789; listlatminmax.append(llmm);
    llmm.max =  27.9595; llmm.min =  21.469; listlatminmax.append(llmm);
    llmm.max =  31.0826; llmm.min =  24.3168; listlatminmax.append(llmm);
    llmm.max =  34.2672; llmm.min =  27.0613; listlatminmax.append(llmm);
    llmm.max =  37.5885; llmm.min =  29.9137; listlatminmax.append(llmm);
    llmm.max =  41.3187; llmm.min =  32.9097; listlatminmax.append(llmm);
    llmm.max =  45.3114; llmm.min =  36.3221; listlatminmax.append(llmm);
    llmm.max =  49.5518; llmm.min =  39.9666; listlatminmax.append(llmm);
    llmm.max =  54.2458; llmm.min =  43.9502; listlatminmax.append(llmm);
    llmm.max =  59.5467; llmm.min =  48.3737; listlatminmax.append(llmm);
    llmm.max =  65.8148; llmm.min =  53.4836; listlatminmax.append(llmm);
    llmm.max =  74.0858; llmm.min =  59.8213; listlatminmax.append(llmm);
    llmm.max =  80.1184; llmm.min =  69.1945; listlatminmax.append(llmm);
}

qreal MTGLatMinMax::getLatMax(int index)
{
    if (index <= 0)
        return 999;
    else
        return listlatminmax.at(index-1).max;
}

qreal MTGLatMinMax::getLatMin(int index)
{
    if (index <= 0)
        return 999;
    else
       return listlatminmax.at(index-1).min;
}

GridParams pixgeoConversion::getGridParams(double ssd)
{
    if (std::abs(ssd - 0.5) < 1e-6)
    {
        return {
                8.9142405037 * PIE / 180,
                -8.9142405037 * PIE / 180,
                0.000800524494 * PIE / 180,
                0.000800524494 * PIE / 180};
    }
    else if (std::abs(ssd - 1.0) < 1e-6)
    {
        return {
                8.9138402398 * PIE / 180,
                -8.9138402398 * PIE / 180,
                0.001601048988 * PIE / 180,
                0.001601048988 * PIE / 180};
    }
    else if (std::abs(ssd - 2.0) < 1e-6)
    {
        return {
                8.9130397083 * PIE / 180,
                -8.9130397083 * PIE / 180,
                0.003202097973 * PIE / 180,
                0.003202097973 * PIE / 180};
    }
    throw std::invalid_argument("SSD must be 0.5, 1.0, or 2.0 km");
}

LatLonToGridResult pixgeoConversion::fci_latlon_to_grid(const std::vector<double> &lats,
                                      const std::vector<double> &lons, double ssd)
{
    GridParams params = getGridParams(ssd);

    // Earth ellipsoid parameters
    double r_eq = 6378.137;                          // km
    double r_pol = r_eq * (1 - 1.0 / 298.257223563); // km
    double h = 35786.4 + r_eq;                       // km

    std::vector<double> result_rows, result_cols;

    for (size_t i = 0; i < lats.size(); ++i)
    {
        double lat_rad = lats[i] * PIE / 180.0;
        double lon_rad = lons[i] * PIE / 180.0;

        double cos_lat = std::cos(lat_rad);
        double sin_lat = std::sin(lat_rad);
        double cos_lon = std::cos(lon_rad);
        double sin_lon = std::sin(lon_rad);

        // Calculate geocentric coordinates
        double c = 1.0 / std::sqrt(cos_lat * cos_lat +
                                   (r_pol / r_eq) * (r_pol / r_eq) * sin_lat * sin_lat);

        double x = r_eq * c * cos_lat * cos_lon;
        double y = r_eq * c * cos_lat * sin_lon;
        double z = r_pol * c * (r_pol / r_eq) * sin_lat;

        // Satellite position
        double x_sat = h;
        double y_sat = 0.0;
        double z_sat = 0.0;

        // Vector from satellite to point
        double dx = x - x_sat;
        double dy = y - y_sat;
        double dz = z - z_sat;

        // Check visibility : dot product between the vector from the point to
        // the satellite and the ellipsoid normal in that point. Positive means
        // the point faces the satellite, so it lies on the visible disc. Same
        // criterion as geocoord2pixcoord, page 24 of the CGMS LRIT/HRIT global
        // specification.
        double dotprod = -dx * x - y * y - z * z * (r_eq / r_pol) * (r_eq / r_pol);
        bool visible = dotprod > 0.0;

        // Calculate viewing angles
        double rho_c = std::sqrt(dx * dx + dy * dy);
        double phi_s = std::atan(dz / rho_c);
        double lambda_s = std::atan(dy / dx);

        // Convert to grid coordinates
        double col = (params.lambda_0 - lambda_s) / params.azimuth_sampling + 1.0;
        double row = (phi_s - params.phi_0) / params.elevation_sampling + 1.0;

        result_rows.push_back(visible ? row : NAN_VALUE);
        result_cols.push_back(visible ? col : NAN_VALUE);
    }

    return {result_rows, result_cols};
}

// Convert FCI grid row/column to latitude/longitude
GridToLatLonResult pixgeoConversion::fci_grid_to_latlon(const std::vector<double> &rows,
                                      const std::vector<double> &cols, double ssd)
{
    GridParams params = getGridParams(ssd);

    // Earth ellipsoid parameters
    double r_eq = 6378.137;                          // km
    double r_pol = r_eq * (1 - 1.0 / 298.257223563); // km
    double h = 35786.4 + r_eq;                       // km
    double lambda_D = 0.0;

    std::vector<double> result_lons, result_lats;
    std::vector<bool> result_earth_mask;

    for (size_t i = 0; i < rows.size(); ++i)
    {
        // Calculate viewing angles
        double lambda_s = params.lambda_0 - (cols[i] - 1.0) * params.azimuth_sampling;
        double phi_s = params.phi_0 + (rows[i] - 1.0) * params.elevation_sampling;

        double cos_lambda_s = std::cos(lambda_s);
        double sin_lambda_s = std::sin(lambda_s);
        double cos_phi_s = std::cos(phi_s);
        double sin_phi_s = std::sin(phi_s);

        double s5 = h * h - r_eq * r_eq;
        double s4 = (r_eq / r_pol) * (r_eq / r_pol);

        double s_d_2 = (h * cos_lambda_s * cos_phi_s) * (h * cos_lambda_s * cos_phi_s) -
                       s5 * (cos_phi_s * cos_phi_s + s4 * sin_phi_s * sin_phi_s);

        bool is_earth = s_d_2 >= 0;

        if (!is_earth)
        {
            result_lons.push_back(NAN_VALUE);
            result_lats.push_back(NAN_VALUE);
            result_earth_mask.push_back(false);
            continue;
        }

        double s_d = std::sqrt(s_d_2);

        // Calculate s_n
        double s_n = (h * cos_lambda_s * cos_phi_s - s_d) /
                     (cos_phi_s * cos_phi_s + s4 * sin_phi_s * sin_phi_s);

        // Calculate s1, s2, s3
        double s1 = h - s_n * cos_lambda_s * cos_phi_s;
        double s2 = -s_n * sin_lambda_s * cos_phi_s;
        double s3 = s_n * sin_phi_s;

        // Calculate s_xy
        double s_xy = std::sqrt(s1 * s1 + s2 * s2);

        // Calculate latitude and longitude
        double lat = std::atan((s3 / s_xy) * s4);
        double lon = std::atan(s2 / s1) + lambda_D * PIE / 180.0;

        // Convert to degrees
        result_lons.push_back(lon * 180.0 / PIE);
        result_lats.push_back(lat * 180.0 / PIE);
        result_earth_mask.push_back(true);
    }

    return {result_lons, result_lats, result_earth_mask};
}

int pixgeoConversion::pixcoord2geocoordFCI(double sub_lon_deg, int column, int row, double *latitude, double *longitude, double ssd)
{
    std::vector<double> rows = { (double)row };
    std::vector<double> cols = { (double)column };
    auto res = fci_grid_to_latlon(rows, cols, ssd);

    // Behind the limb there is no surface to name, so this has to be reported
    // rather than returned as a plausible-looking number - the same contract
    // geocoord2pixcoordFCI keeps for points it cannot see.
    if (!res.earth_mask[0] || std::isnan(res.lats[0]) || std::isnan(res.lons[0]))
    {
        *latitude  = -999.999;
        *longitude = -999.999;
        return (-1);
    }

    // fci_grid_to_latlon works in the projection's own frame, where the
    // sub-satellite point is the zero of longitude.
    *latitude  = res.lats[0];
    *longitude = res.lons[0] + sub_lon_deg;
    return 0;
}

int pixgeoConversion::geocoord2pixcoordFCI(double sub_lon_deg, double latitude, double longitude, int *column, int *row, double ssd)
{
    std::vector<double> lats = {latitude};
    std::vector<double> lons = {longitude};
    auto res = fci_latlon_to_grid(lats, lons, ssd);

    // Points behind the limb come back as NaN. Rounding those to an int is
    // undefined behaviour, so they have to be rejected here, the same way
    // geocoord2pixcoord rejects invisible points.
    if(std::isnan(res.columns[0]) || std::isnan(res.rows[0]))
    {
        *column = -999;
        *row = -999;
        return (-1);
    }

    *column = std::round(res.columns[0]);
    *row = std::round(res.rows[0]);
    return 0;
}
