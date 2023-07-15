/*******************************************************************************
 *
 *    Copyright (C) 2014-2017 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
 *
 *    This source code is licensed under the GNU General Public License (GPL),
 *    Version 3.  See the file COPYING for more details.
 *
 ******************************************************************************/
#include "internal.h"
#include "nav_util.h"


#define T_U 2451545.

/*******************************************************************************
 * Convert SEVIRI line and column to latitude and longitude.
 *
 * line		: Input SEVIRI line number
 * column	: Input SEVIRI column number
 * lat		: Output latitude (degrees: -90.0 -- 90.0)
 * lon		: Output longitude (degrees: -180.0 -- 180.0)
 * lon0		: Projection longitude origin (degrees: -180.0 -- 180.0)
 * nav		: struct containing the navigation scaling factors defined in
 * nav		: Input struct containing the navigation scaling factors
 *                defined in the reference
 *
 * returns	: Non-zero on error
 *
 * Ref: PDF_CGMS_LRIT_HRIT_2_6, Section 4.4
 ******************************************************************************/
int snu_line_column_to_lat_lon(unsigned int line, unsigned int column, float *lat, float *lon,
          double lon0, const struct nav_scaling_factors *nav)
{
     double x;
     double y;

     double cos_x;
     double sin_x;
     double cos_y;
     double sin_y;
     double cos_y2;
     double sin_y2;

     double s_1;
     double s_2;
     double s_3;
     double s_xy;
     double s_n;
     double s_d;

     x = (column - nav->COFF) / (pow(2, -16) * nav->CFAC);
     y = (line   - nav->LOFF) / (pow(2, -16) * nav->LFAC);

     cos_x  = cos(x);
     sin_x  = sin(x);
     cos_y  = cos(y);
     sin_y  = sin(y);
     cos_y2 = cos_y * cos_y;
     sin_y2 = sin_y * sin_y;

//     s_d  = pow(42164. * cos_x * cos_y, 2) -
//           (cos_y2 + 1.006803 * sin_y2) * 1737121856.;

     s_d  = pow(42164. * cos_x * cos_y, 2) -
           (cos_y2 + 1.006739501 * sin_y2) * 1737122264.;

     if (s_d < 0.)
          return -1;

     s_d  = sqrt(s_d);

//     s_n  = (42164. * cos_x * cos_y - s_d) / (cos_y2 + 1.006803 * sin_y2);
     s_n  = (42164. * cos_x * cos_y - s_d) / (cos_y2 + 1.006739501 * sin_y2);
     s_1  =  42164. - s_n * cos_x * cos_y;
     s_2  =  s_n * sin_x * cos_y;
     s_3  = -s_n * sin_y;
     s_xy = sqrt(s_1 * s_1 + s_2 * s_2);

//     *lat = atan(1.006803 * s_3 / s_xy);
     *lat = atan(1.006739501 * s_3 / s_xy);
     *lon = atan(s_2 / s_1) + lon0*D2R;

     *lat *= R2D;
     *lon *= R2D;

     return 0;
}



/*******************************************************************************
 * Convert latitude and longitude to SEVIRI line and column.
 *
 * lat		: Input latitude (degrees: -90.0 -- 90.0)
 * lon		: Input longitude (degrees: -180.0 -- 180.0)
 * line		: Output SEVIRI line number
 * column	: Output SEVIRI column number
 * lon0		: Input projection longitude origin (radians: -PI -- PI)
 * nav		: Input struct containing the navigation scaling factors
 *                defined in the reference
 *
 * returns	: Non-zero on error
 *
 * Ref: PDF_CGMS_LRIT_HRIT_2_6, Section 4.4
 ******************************************************************************/
int snu_lat_lon_to_line_column(float lat, float lon, unsigned int *line, unsigned int *column,
          double lon0, const struct nav_scaling_factors *nav)
{
     double x;
     double y;

     double c_lat;

     double cos_c_lat;
     double sin_c_lat;
     double cos_lon_2;
     double sin_lon_2;

     double r_l;
     double r_1;
     double r_2;
     double r_3;
     double r_n;

     const double r_pol = 6356.5838;
/*
     const double r_eq  = 6378.1690;
*/
     if (lat < -90. || lat > 90.) {
          fprintf(stderr, "ERROR: Latitude out of valid range: %e\n", lat);
          return -1;
     }

     if (lon < -180. || lon > 180.) {
          fprintf(stderr, "ERROR: Longitude out of valid range: %e\n", lon);
          return -1;
     }

     *column = FILL_VALUE_I;
     *line   = FILL_VALUE_I;

     c_lat = atan(.993243 * tan(lat * D2R));

     cos_c_lat = cos(c_lat);
     sin_c_lat = sin(c_lat);
     cos_lon_2 = cos((lon - lon0) * D2R);
     sin_lon_2 = sin((lon - lon0) * D2R);

     r_l =  r_pol / sqrt(1. - .00675701 * cos_c_lat * cos_c_lat);
     r_1 =  42164. - r_l * cos_c_lat * cos_lon_2;
     r_2 = -r_l * cos_c_lat * sin_lon_2;
     r_3 =  r_l * sin_c_lat;
     r_n =  sqrt(r_1*r_1 + r_2*r_2 + r_3*r_3);

     x = atan(-r_2 / r_1);
     y = asin(-r_3 / r_n);

     *column = nav->COFF + snu_rint(x * pow(2, -16) * nav->CFAC);
     *line   = nav->LOFF + snu_rint(y * pow(2, -16) * nav->LFAC);

     return 0;
}



/*******************************************************************************
 * Compute the Greenwich mean sidereal time given Julian Day Number.
 *
 * jtime	: Input Julian Day Number
 *
 * returns	: Greenwich mean sidereal time
 *
 * Ref: The Astronomical Almanac, 2003, B6
 ******************************************************************************/
static double calc_gmst(double jtime)
{
     double a;
     double jdelta;
     double gmst;

     jdelta = (jtime - T_U) / 36525.;

     /* 43200. is added to the first constant for julian time */
     a = (24110.54841 + 43200. + jdelta*(8640184.812866 +
          jdelta*(0.093104 + jdelta * (-.0000062)))) / (60.*60.*24.);

     /* a is for hour 0. but we want gmst for the input jtime */
     gmst = fmod(jtime, 1.) + fmod(a, 1.);
     if (gmst > 1.)
          gmst -= 1.;

     return gmst;
}



/*******************************************************************************
 * Compute the solar declination, Greenwich mean solar time, and Greenwich local
 * solar time given Julian Day Number.
 *
 * jtime		: Input Julian Day Number
 * delta		: Output solar declination
 * gw_mean_sol_time	: Output Greenwich mean solar time
 * gw_appar_sol_time	: Output Greenwich local solar
 *
 * Ref: The Astronomical Almanac, 2003, C24
 ******************************************************************************/
static void solar_coords_and_times(double jtime, double *delta,
          double *gw_mean_sol_time, double *gw_appar_sol_time)
{
     double a;

     double jdelta;
     double L;
     double g;
     double lambda;
     double epsilon;
/*
     double t;
*/
     double alpha;
     double gmst;
     double E;

     jdelta = jtime - T_U;

     /* mean longitude of, corrected for aberration */
     L = (280.466 + .9856474 * jdelta)*D2R;
     L = fmod(L, (PI * 2.));

     /* mean anamoly */
     g = (357.528 + .9856003 * jdelta)*D2R;
     g = fmod(g, (PI * 2.));

     /* ecliptic longitude */
     a = 1.915 * sin(g) + .020 * sin(2.0*g);

     lambda = L + a*D2R;
/*
     if ((lambda = fmod(lambda, (PI * 2.))) < 0.)
          lambda += (PI * 2.);
*/
     /* obliquity of ecliptic */
     epsilon = (23.44 - .0000004 * jdelta)*D2R;

     /* right ascension */
/*
     alpha = atan(cos(epsilon)*cos(lambda));
*/
/*
     t  = tan(epsilon / 2.);
     t *= t;
     alpha = lambda - R2D * t * sin(2. * lambda) +
                     (R2D / 2.) * t*t * sin(4. * lambda);
*/
     alpha = atan2(cos(epsilon)*sin(lambda), cos(epsilon)*cos(lambda));
     if (alpha <= 0.)
          alpha += (PI * 2.);

     /* solar declination */
     *delta = asin(sin(epsilon)*sin(lambda));

     /* greenwich mean sidereal time */
     gmst = calc_gmst(jtime);

     /* greenwich mean solar time */
     *gw_mean_sol_time  = (gmst - .5 - alpha / (PI * 2.));

     if ((*gw_mean_sol_time  = fmod(*gw_mean_sol_time,   1.)) < 0.)
          *gw_mean_sol_time += 1.;

     /* equation of time (apparent time  - mean time) */
/*
     E = (L - alpha) * 4. / (60. * 24.);
*/
     /* equation of time from:
        Explanatory Supplement to the Astronomical Almanac, 1992 */

     E = (-a + 2.466 * sin(2*lambda) - .053 * sin(4*lambda)) / 360.;

     /* greenwich apparent solar time */
     *gw_appar_sol_time = *gw_mean_sol_time + E;

     if ((*gw_appar_sol_time  = fmod(*gw_appar_sol_time,  1.)) < 0.)
          *gw_appar_sol_time += 1.;
}



/*******************************************************************************
 * Convert GMT/UTC to local time.
 *
 * gw_time	: input GMT/UTC
 * lon		: input local longitude (radians: -PI -- PI)
 *
 * returns	: local time
 ******************************************************************************/
static double greenwich_to_local_time(double lon, double gw_time)
{
     double loc_time;

     loc_time = gw_time + lon / (PI * 2.);
     if ((loc_time  = fmod(loc_time,  1.)) < 0.)
          loc_time += 1.;

     return loc_time;
}



/*******************************************************************************
 * Convert local time to GMT/UTC.
 *
 * loc_time	: Input local time
 * lon		: Input local longitude (radians: -PI -- PI)
 *
 * returns	: GMT/UTC
 ******************************************************************************/
static double local_to_greenwich_time(double lon, double loc_time)
{
     double gw_time;

     gw_time = loc_time - lon / (PI * 2.);
     if ((gw_time  = fmod(gw_time,  1.)) < 0.)
          gw_time += 1.;

     return gw_time;
}



/*******************************************************************************
 * Compute the cosine of the solar zenith angle, the solar zenith angle, and the
 * solar azimuth angle given the solar declination, latitude, hour of day, and
 * the "equation of time" (apparent time - mean time)
 *
 * delta	: Input solar declination
 * lat		: Input latitude (radians: -PI/2 -- PI/2)
 * hour		: Input hour of day
 * eot		: Input equation of time (apparent time - mean time)
 * mu0		: Output solar zenith angle (-1.0 -- 1.0)
 * theta0	: Output solar zenith angle (radians: 0.0 -- PI)
 * phi0		: Output solar azimuth angle (radians: 0.0 -- 2PI)
 ******************************************************************************/
static void solar_angles(double delta, double lat, double hour, double eot,
                         double *mu0, double *theta0, double *phi0)
{
     double a;

     double h;

     double coslat;
     double sinlat;
     double sindelta;
     double cos_h;

     double costheta0;
     double cosphi0;

     h = (hour - eot * 24. - 12.) * 15. * D2R;

     coslat   = cos(lat);
     sinlat   = sin(lat);
     sindelta = sin(delta);
     cos_h    = cos(h);

     a = cos(delta)*cos_h;

     costheta0 = (sinlat*sindelta + coslat*a);

     *mu0    = costheta0;
     *theta0 = acos(costheta0);
/*
     sinphi0 = -cos(delta)*sin(h) / sin(acos(costheta0));
     *phi0   = asin(sinphi0);
*/
     cosphi0 = (coslat*sindelta - sinlat*a) / sqrt(1.-costheta0*costheta0);
/*
     cosphi0 = (costheta0 * sinlat - sindelta) / (sqrt(1.-costheta0*costheta0) * coslat);
*/
     *phi0   = acos(cosphi0);
     if (h > 0.)
          *phi0 = 2. * PI - *phi0;
}



/*******************************************************************************
 * Compute the cosine of the solar zenith angle, the solar zenith angle, the
 * solar azimuth angle, and the solar distance factor given Julian Day Number
 * latitude and longitude
 *
 * jtime	: Input Julian Day Number
 * lat		: Input latitude (radians: -PI/2 -- PI/2)
 * lon		: Input longitude (radians: -PI -- PI)
 * mu0		: Output solar zenith angle (-1.0 -- 1.0)
 * theta0	: Output solar zenith angle (radians: 0.0 -- PI)
 * phi0		: Output solar azimuth angle (radians: 0.0 -- 2PI)
 *
 * Ref: The Astronomical Almanac, 2003
 ******************************************************************************/
void snu_solar_params2(double jtime, double lat, double lon, double *mu0,
                       double *theta0, double *phi0, double *solar_dist_fac)
{
     int year;
     int month;
     int day;

     long jwhole;

     double jfrac;

     double delta;

     double gw_mean_sol_time;
     double gw_appar_sol_time;
     double loc_mean_sol_time;
     double loc_appar_sol_time;

     double local_hour;

     double eot;

     double jday;


     solar_coords_and_times(jtime, &delta,
                            &gw_mean_sol_time, &gw_appar_sol_time);
     loc_mean_sol_time  = greenwich_to_local_time(lon, gw_mean_sol_time);
     loc_appar_sol_time = greenwich_to_local_time(lon, gw_appar_sol_time);


     jwhole = (int) jtime;
     jfrac  = jtime - jwhole;
     if (jfrac < .5)
          jfrac  = jfrac  + .5;
     else {
          jfrac  = jfrac  - .5;
          jwhole = jwhole + 1;
     }


     local_hour = jfrac * 24. + lon / (15.*D2R);


     eot = fmod(loc_mean_sol_time - loc_appar_sol_time,  1.);

     solar_angles(delta, lat, local_hour, eot, mu0, theta0, phi0);


     if (solar_dist_fac) {
          snu_jul_to_cal_date(jwhole, &year, &month, &day);

          jday = (int) (jtime - (snu_cal_to_jul_day(year, 1, 1) - .5));

          *solar_dist_fac = snu_solar_distance_factor2(jday + local_hour / 24.);
     }
}



/*******************************************************************************
 * Compute the SEVIRI viewing zenith and azimuth angles.
 *
 * lat		: Input latitude of the observer(degrees: -90.0 -- 90.0)
 * lon		: Input longitude of the observer (degrees: -180.0 -- 180.0)
 * height	: Input height of the observer above the ref ellipsoid (km)
 * X, Y, Z	: Input satellite position vector in Cartesian coordinates (km)
 * vza		: Output SEVIRI viewing zenith angle (degrees: 0.0 -- 180.0)
 * vaa		: Output SEVIRI viewing azimuth angle (degrees: 0.0 -- 360.0)
 *
 * Ref: GIESKE_A_S_M, Page 6
 ******************************************************************************/
int snu_vza_and_vaa(double lat, double lon, double height,
                    double X, double Y, double Z, float *vza, float *vaa)
{
     /* Below are the values given by Gieske et. al. */
/*
     const double a = 6378.1370;
     const double b = 6356.7523;
*/
     /* Instead, to be consistent, we used the values used for the line/column
        <-> lat/lon functions. */
     const double a = 6378.1690;
     const double b = 6356.5838;

     double e2;

     double N;

     double x;
     double y;
     double z;

     double cos_lat;
     double sin_lat;
     double cos_lon;
     double sin_lon;

     double qv[3];
     double u [3];

     cos_lat = cos(lat * D2R);
     sin_lat = sin(lat * D2R);
     cos_lon = cos(lon * D2R);
     sin_lon = sin(lon * D2R);

     e2 = 1. - (b * b) / (a * a);

     N = a / sqrt(1. - e2 * sin_lat * sin_lat);

     x = (N + height) * cos_lat * cos_lon;
     y = (N + height) * cos_lat * sin_lon;
     z = ((b * b) / (a * a) * N + height) * sin_lat;

     qv[0] = X - x;
     qv[1] = Y - y;
     qv[2] = Z - z;

     u [0] = -sin_lat * cos_lon * qv[0] + -sin_lat * sin_lon * qv[1] + cos_lat * qv[2];
     u [1] = -sin_lon *           qv[0] +  cos_lon           * qv[1];
     u [2] =  cos_lat * cos_lon * qv[0] + -cos_lat * sin_lon * qv[1] + sin_lat * qv[2];

     *vza = acos(u[2] / sqrt(u[0]*u[0] + u[1]*u[1] + u[2]*u[2])) * R2D;

     *vaa = atan2(u[1], u[0]) * R2D;
     if (*vaa < 0.)
          *vaa += 360.;

     return 0;
}
