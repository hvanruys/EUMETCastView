/******************************************************************************%
 *
 *    Copyright (C) 2014-2017 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
 *
 *    This source code is licensed under the GNU General Public License (GPL),
 *    Version 3.  See the file COPYING for more details.
 *
 ******************************************************************************/

#ifndef INTERNAL_H
#define INTERNAL_H

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

//#ifdef __cplusplus
//extern "C" {
//#endif

#define SEVIRI_N_BANDS 12

int getChannelNbr(std::string spectrum);


/*******************************************************************************
 * Configuration related
 ******************************************************************************/
#define VERBOSE	1


/*******************************************************************************
 * Some mathematical constants
 ******************************************************************************/
#ifdef M_PI
#define PI	M_PI
#else
#define PI	3.14159265358979323846264338327
#endif

#define D2R	(PI / 180.0)
#define R2D	(180.0 / PI)


/*******************************************************************************
 * Some macros
 ******************************************************************************/
/* Min/Max macros */
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define MAX(a,b) ((a) > (b) ? (a) : (b))


/*******************************************************************************
 * SEVIRI realated constants
 ******************************************************************************/
#define IMAGE_SIZE_VIR_LINES	3712
#define IMAGE_SIZE_VIR_COLUMNS	3712

#define IMAGE_SIZE_HRV_LINES	11136
#define IMAGE_SIZE_HRV_COLUMNS	5568


/*******************************************************************************
 * Fill values for various data types
 ******************************************************************************/
#define FILL_VALUE_US		USHRT_MAX
#define FILL_VALUE_I		INT_MAX
#define FILL_VALUE_F		-999.


/*******************************************************************************
 * Struct for scaling factors required to convert pixel corrdinates to
 * projection coordinates.
 ******************************************************************************/
struct nav_scaling_factors {
     double CFAC;
     double LFAC;
     double COFF;
     double LOFF;
};


/*******************************************************************************
 *
 ******************************************************************************/
extern const ushort satellite_ids[];
extern const ushort n_satellites;
extern const double channel_center_wavelength[];
extern const std::string channel_spectrum[SEVIRI_N_BANDS];
extern const double band_solar_irradiance[][SEVIRI_N_BANDS];
extern const double bt_nu_c[][SEVIRI_N_BANDS];
extern const double bt_A[][SEVIRI_N_BANDS];
extern const double bt_B[][SEVIRI_N_BANDS];
extern const struct nav_scaling_factors nav_scaling_factors_vir;
extern const struct nav_scaling_factors nav_scaling_factors_hrv;


/*******************************************************************************
 *
 ******************************************************************************/
#include "misc_util.h"
#include "nav_util.h"
//#include "read_write.h"


//#ifdef __cplusplus
//}
//#endif

#endif /* INTERNAL_H */
