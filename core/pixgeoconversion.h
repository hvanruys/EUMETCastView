#ifndef PIXGEOCONVERSION_H
#define PIXGEOCONVERSION_H

#include "qsgp4globals.h"

#define HALF_PI PI*0.5
#define TWO_PI 	PI*2.0
#define EPSLN	1.0e-3
#define R2D     57.2957795131
#define D2R     0.0174532925199
#define sign2(x) (( x > 0 ) - ( x < 0 ))

const double  SAT_HEIGHT = 42164.0;     /* distance from Earth centre to satellite     */
const double  R_EQ       =  6378.169;   /* radius from Earth centre to equator         */
const double  R_POL      =  6356.5838;  /* radius from Earth centre to pol             */
//const double  SUB_LON    =     0.0;     /* longitude of sub-satellite point in radiant */
                                            /* SUB_LON = 0.16572   9.5° MET-9 (MSG-2) */
                                            /* SUB_LON = 0.0       0.0° MET-10 (MSG-3) */

const double  CFAC_NONHRV  =  781648343;      /* scaling coefficients (see note above)  */
const double  LFAC_NONHRV  =  781648343;      /* scaling coefficients (see note above)  */

const double  CFAC_HRV     =   2344945030.;   /* scaling coefficients (see note above)  */
const double  LFAC_HRV     =   2344945030.;   /* scaling coefficients (see note above)  */

const double  CFAC_NONHRV_ELECTRO     =   586236263.;   /* scaling coefficients (see note above)  */
const double  LFAC_NONHRV_ELECTRO     =   586236263.;   /* scaling coefficients (see note above)  */

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

const double  CFAC_NONHRV_H8     =   1172050000.; //20466275.;   /* scaling coefficients (see note above)  */
const double  LFAC_NONHRV_H8     =   1172050000.; //20466275.;   /* scaling coefficients (see note above)  */

const long    COFF_NONHRV  =        1856;      /* scaling coefficients (see note above)  */
const long    LOFF_NONHRV  =        1856;      /* scaling coefficients (see note above)  */

const long    COFF_HRV     =        5566;      /* scaling coefficients (see note above)  */
const long    LOFF_HRV     =        5566;      /* scaling coefficients (see note above)  */

const long    COFF_NONHRV_ELECTRO  =        1392;      /* scaling coefficients (see note above)  */
const long    LOFF_NONHRV_ELECTRO  =        1392;      /* scaling coefficients (see note above)  */

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

const long    COFF_NONHRV_H8 =        2750;      /* scaling coefficients (see note above)  */
const long    LOFF_NONHRV_H8 =        2750;      /* scaling coefficients (see note above)  */

class pixgeoConversion
{
public:
    pixgeoConversion();
    int pixcoord2geocoord(double sub_lon_deg, int column, int row, long coff, long loff, long long cfac, long long lfac, double *latitude, double *longitude);
    int geocoord2pixcoord(double sub_lon_deg, double latitude, double longitude, long coff, long loff, long long cfac, long long lfac, int *column, int *row);
    int geocoord2pixcoordrad(double sub_lon_deg, double lat_rad, double lon_rad, long coff, long loff, long long cfac, long long lfac, int *column, int *row);
    int nint(double val);

};

#endif // PIXGEOCONVERSION_H
