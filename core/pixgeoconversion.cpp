#include "pixgeoconversion.h"
#include "globals.h"
#include <QDebug>


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


