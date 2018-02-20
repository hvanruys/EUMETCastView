/******************************************************************************%
 *
 *    Copyright (C) 2014-2017 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
 *
 *    This source code is licensed under the GNU General Public License (GPL),
 *    Version 3.  See the file COPYING for more details.
 *
 ******************************************************************************/

#ifndef NAV_UTIL_H
#define NAV_UTIL_H


//#ifdef __cplusplus
//extern "C" {
//#endif


int snu_line_column_to_lat_lon(uint l, uint c, float *lat, float *lon,
                               double lon0, const struct nav_scaling_factors *nav);
int snu_lat_lon_to_line_column(float lat, float lon, uint *line, uint *column,
                               double lon0, const struct nav_scaling_factors *nav);
void snu_solar_params2(double jtime, double lat, double lon, double *mu0,
                       double *theta0, double *phi0, double *solar_dist_fac);
int snu_vza_and_vaa(double lat, double lon, double height,
                    double X, double Y, double Z, float *vza, float *vaa);

//#ifdef __cplusplus
//}
//#endif

#endif /* NAV_UTIL_H */
