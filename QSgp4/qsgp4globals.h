#ifndef QGLOBALS_H_
#define QGLOBALS_H_

#include <cmath>


/*
const double kAE = 1.0;
const double kQ0 = 120.0;
const double kS0 = 78.0;
const double kMU = 398600.8;
const double kXKMPER = 6378.135;
const double kXJ2 = 1.082616e-3;
const double kXJ3 = -2.53881e-6;
const double kXJ4 = -1.65597e-6;
*/
/*
 * alternative XKE
 * affects final results
 * aiaa-2006-6573
 * const double kXKE = 60.0 / sqrt(kXKMPER * kXKMPER * kXKMPER / kMU);
 * dundee
 * const double kXKE = 7.43669161331734132e-2;
 */
/*
const double kXKE = 60.0 / sqrt(kXKMPER * kXKMPER * kXKMPER / kMU);
const double kCK2 = 0.5 * kXJ2 * kAE * kAE;
const double kCK4 = -0.375 * kXJ4 * kAE * kAE * kAE * kAE;
*/
/*
 * alternative QOMS2T
 * affects final results
 * aiaa-2006-6573
 * #define QOMS2T   (pow(((Q0 - S0) / XKMPER), 4.0))
 * dundee
 * #define QOMS2T   (1.880279159015270643865e-9)
 */
/*
const double QOMS2T = pow(((kQ0 - kS0) / kXKMPER), 4.0);
const double kS = kAE * (1.0 + kS0 / kXKMPER);
*/


const double PI = 3.14159265358979323846264338327950288419716939937510582;
const double TWOPI = 2.0 * PI;
const double TWOTHIRD = 2.0 / 3.0;
const double THDT = 4.37526908801129966e-3;

const double NOAA_SWATH     = 55.3 * PI / 180.0;


/*
 * earth flattening
 */
const double F = 1.0 / 298.257;
/*
 * earth rotation per sideral day
 */
const double OMEGA_E = 1.00273790934;
const double AU = 1.49597870691e8;

const double SECONDS_PER_DAY = 86400.0;
const double MINUTES_PER_DAY = 1440.0;
const double HOURS_PER_DAY = 24.0;

// Jan 1.0 1900 = Jan 1 1900 00h UTC
const double EPOCH_JAN1_00H_1900 = 2415019.5;

// Jan 1.5 1900 = Jan 1 1900 12h UTC
const double EPOCH_JAN1_12H_1900 = 2415020.0;

// Jan 1.5 2000 = Jan 1 2000 12h UTC
const double EPOCH_JAN1_12H_2000 = 2451545.0;

#endif

