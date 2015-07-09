#include "qsgp4globals.h"
#include <QString>
#include <cassert>
#include <cmath>

#include "qtle.h"
#include "qsgp4utilities.h"


// Note: The column offsets are ZERO based.

// Name
const int TLE_LEN_LINE_DATA      = 69; const int TLE_LEN_LINE_NAME      = 22;

// Line 1
const int TLE1_COL_SATNUM        =  2; const int TLE1_LEN_SATNUM        =  5;
const int TLE1_COL_INTLDESC_A    =  9; const int TLE1_LEN_INTLDESC_A    =  2;
const int TLE1_COL_INTLDESC_B    = 11; const int TLE1_LEN_INTLDESC_B    =  3;
const int TLE1_COL_INTLDESC_C    = 14; const int TLE1_LEN_INTLDESC_C    =  3;
const int TLE1_COL_EPOCH_A       = 18; const int TLE1_LEN_EPOCH_A       =  2;
const int TLE1_COL_EPOCH_B       = 20; const int TLE1_LEN_EPOCH_B       = 12;
const int TLE1_COL_MEANMOTIONDT  = 33; const int TLE1_LEN_MEANMOTIONDT  = 10;
const int TLE1_COL_MEANMOTIONDT2 = 44; const int TLE1_LEN_MEANMOTIONDT2 =  8;
const int TLE1_COL_BSTAR         = 53; const int TLE1_LEN_BSTAR         =  8;
const int TLE1_COL_EPHEMTYPE     = 62; const int TLE1_LEN_EPHEMTYPE     =  1;
const int TLE1_COL_ELNUM         = 64; const int TLE1_LEN_ELNUM         =  4;

// Line 2
const int TLE2_COL_SATNUM        = 2;  const int TLE2_LEN_SATNUM        =  5;
const int TLE2_COL_INCLINATION   = 8;  const int TLE2_LEN_INCLINATION   =  8;
const int TLE2_COL_RAASCENDNODE  = 17; const int TLE2_LEN_RAASCENDNODE  =  8;
const int TLE2_COL_ECCENTRICITY  = 26; const int TLE2_LEN_ECCENTRICITY  =  7;
const int TLE2_COL_ARGPERIGEE    = 34; const int TLE2_LEN_ARGPERIGEE    =  8;
const int TLE2_COL_MEANANOMALY   = 43; const int TLE2_LEN_MEANANOMALY   =  8;
const int TLE2_COL_MEANMOTION    = 52; const int TLE2_LEN_MEANMOTION    = 11;
const int TLE2_COL_REVATEPOCH    = 63; const int TLE2_LEN_REVATEPOCH    =  5;

/////////////////////////////////////////////////////////////////////////////
QTle::QTle(QString strName, QString strLine1, QString strLine2, eGravconsttype gravcnst = QTle::wgs72)
{
   m_strName  = strName;
   m_strLine1 = strLine1;
   m_strLine2 = strLine2;
   whichconst = gravcnst;
   Initialize();
}

/////////////////////////////////////////////////////////////////////////////
QTle::QTle(const QTle &tle)
{
    m_strName  = tle.m_strName;
    m_strLine1 = tle.m_strLine1;
    m_strLine2 = tle.m_strLine2;

    for (int fld = FLD_FIRST; fld < FLD_LAST; fld++)
    {
        m_Field[fld] = tle.m_Field[fld];
    }


    satnum = tle.satnum;
    bstar = tle.bstar;
    ecco = tle.ecco;


    whichconst = tle.whichconst;
    tumin = tle.tumin;
    mu = tle.mu;
    radiusearthkm = tle.radiusearthkm;
    xke = tle.xke;
    j2 = tle.j2;
    j3 = tle.j3;
    j4 = tle.j4;
    j3oj2 = tle.j3oj2;
    no = tle.no;
    a = tle.a;
    ndot = tle.ndot;
    nddot = tle.nddot;
    inclo = tle.inclo;
    nodeo = tle.nodeo;
    argpo = tle.argpo;
    mo = tle.mo;
    alta = tle.alta;
    altp = tle.altp;
    epochyr = tle.epochyr;
    epochdays = tle.epochdays;
    jdsatepoch = tle.jdsatepoch;


}

/////////////////////////////////////////////////////////////////////////////
QTle::~QTle()
{
}


/////////////////////////////////////////////////////////////////////////////
// ExpToDecimal()
// Converts TLE-style exponential notation of the form [ |-]00000[+|-]0 to
// decimal notation. Assumes implied decimal point to the left of the first
// number in the string, i.e., 
//       " 12345-3" =  0.00012345
//       "-23429-5" = -0.0000023429   
//       " 40436+1" =  4.0436
QString QTle::ExpToDecimal(const QString str)
{

   QString strnumber = "0." + str.mid(1, 5);
   double number = strnumber.toDouble();
   int nExp = str.mid(6, 2).toInt();
   
   double dblVal = number * pow(10.0, nExp);
   if(str.mid(0,1) == "-")
       dblVal *= -1;
   QString strVal = QString("%1").arg(dblVal, 0, 'f', abs(nExp) + 5);

   
   return strVal;
   
} // ExpToDecimal()

/////////////////////////////////////////////////////////////////////////////
// Initialize()
// Initialize the string array.
void QTle::Initialize()
{
   const double deg2rad  =   PI / 180.0;         //   0.0174532925199433
   const double xpdotp   =  1440.0 / (2.0 * PI);  // 229.1831180523293
   int year, mon, day, hr, minute;
   double sec;


   assert(!m_strName.isEmpty());
   assert(!m_strLine1.isEmpty());
   assert(!m_strLine2.isEmpty());
   

   m_Field[FLD_NORADNUM] = m_strLine1.mid(TLE1_COL_SATNUM, TLE1_LEN_SATNUM);
   m_Field[FLD_INTLDESC] = m_strLine1.mid(TLE1_COL_INTLDESC_A,
                                             TLE1_LEN_INTLDESC_A +
                                             TLE1_LEN_INTLDESC_B +   
                                             TLE1_LEN_INTLDESC_C);   
   m_Field[FLD_EPOCHYEAR] = 
         m_strLine1.mid(TLE1_COL_EPOCH_A, TLE1_LEN_EPOCH_A);

   m_Field[FLD_EPOCHDAY] = 
         m_strLine1.mid(TLE1_COL_EPOCH_B, TLE1_LEN_EPOCH_B);
   
   if (m_strLine1[TLE1_COL_MEANMOTIONDT] == '-')
   {
      // value is negative
      m_Field[FLD_MMOTIONDT] = "-0";
   }
   else
      m_Field[FLD_MMOTIONDT] = "0";
   
   m_Field[FLD_MMOTIONDT] += m_strLine1.mid(TLE1_COL_MEANMOTIONDT + 1,
                                               TLE1_LEN_MEANMOTIONDT);
   
   // decimal point assumed; exponential notation
   m_Field[FLD_MMOTIONDT2] = ExpToDecimal(
                                 m_strLine1.mid(TLE1_COL_MEANMOTIONDT2,
                                                   TLE1_LEN_MEANMOTIONDT2));
   // decimal point assumed; exponential notation
   m_Field[FLD_BSTAR] = ExpToDecimal(
                                m_strLine1.mid(TLE1_COL_BSTAR,
                                                       TLE1_LEN_BSTAR));
   //TLE1_COL_EPHEMTYPE      
   //TLE1_LEN_EPHEMTYPE   
   m_Field[FLD_SET] = m_strLine1.mid(TLE1_COL_ELNUM, TLE1_LEN_ELNUM);
   
   //TrimLeft(m_Field[FLD_SET].tr);
   
   //TLE2_COL_SATNUM         
   //TLE2_LEN_SATNUM         
   
   m_Field[FLD_I] = m_strLine2.mid(TLE2_COL_INCLINATION,
                                      TLE2_LEN_INCLINATION).trimmed();
   
   m_Field[FLD_RAAN] = m_strLine2.mid(TLE2_COL_RAASCENDNODE,
                                         TLE2_LEN_RAASCENDNODE).trimmed();
   // decimal point is assumed
   m_Field[FLD_E]   = "0.";
   m_Field[FLD_E]   += m_strLine2.mid(TLE2_COL_ECCENTRICITY,
                                       TLE2_LEN_ECCENTRICITY);
   
   m_Field[FLD_ARGPER] = m_strLine2.mid(TLE2_COL_ARGPERIGEE,
                                           TLE2_LEN_ARGPERIGEE).trimmed();
   
   m_Field[FLD_M]   = m_strLine2.mid(TLE2_COL_MEANANOMALY,
                                      TLE2_LEN_MEANANOMALY).trimmed();
   
   m_Field[FLD_MMOTION]   = m_strLine2.mid(TLE2_COL_MEANMOTION,
                                            TLE2_LEN_MEANMOTION).trimmed();

   m_Field[FLD_ORBITNUM] = m_strLine2.mid(TLE2_COL_REVATEPOCH,
                                             TLE2_LEN_REVATEPOCH).trimmed();

   satnum = m_Field[FLD_NORADNUM].toLong();
   epochyr = m_Field[FLD_EPOCHYEAR].toInt();
   epochdays = m_Field[FLD_EPOCHDAY].toDouble();
   ndot = m_Field[FLD_MMOTIONDT].toDouble();        // mean motion dt
   nddot = m_Field[FLD_MMOTIONDT2].toDouble();      // mean motion dt2
   bstar = m_Field[FLD_BSTAR].toDouble();           // bstar
   inclo = m_Field[FLD_I].toDouble();               // inclination
   nodeo = m_Field[FLD_RAAN].toDouble();            // right ascending node
   ecco = m_Field[FLD_E].toDouble();                // eccenticity
   argpo = m_Field[FLD_ARGPER].toDouble();          // argument of perigee
   mo = m_Field[FLD_M].toDouble();                  // mean anomaly
   no = m_Field[FLD_MMOTION].toDouble();            // mean motion

   getgravconst( whichconst, tumin, mu, radiusearthkm, xke, j2, j3, j4, j3oj2, radiusearthkmminor, flattening );

   // ---- find no ----
   no   = no / xpdotp; //* rad/min
   // ---- convert to sgp4 units ----
   a    = pow( no*tumin , (-2.0/3.0) );
   ndot = ndot  / (xpdotp*1440.0);  //* ? * minperday
   nddot= nddot / (xpdotp*1440.0*1440);

   // ---- find standard orbital elements ----
   inclo = inclo  * deg2rad;
   nodeo = nodeo  * deg2rad;
   argpo = argpo  * deg2rad;
   mo    = mo     * deg2rad;

   alta = a*(1.0 + ecco) - 1.0;
   altp = a*(1.0 - ecco) - 1.0;

   // ----------------------------------------------------------------
   // find sgp4epoch time of element set
   // remember that sgp4 uses units of days from 0 jan 1950 (sgp4epoch)
   // and minutes from the epoch (time)
   // ----------------------------------------------------------------

   // ---------------- temp fix for years from 1957-2056 -------------------
   // --------- correct fix will occur when year is 4-digit in tle ---------
   if (epochyr < 57)
       year= epochyr + 2000;
     else
       year= epochyr + 1900;

   days2mdhms ( year, epochdays, mon, day, hr, minute, sec );
   jday( year, mon, day, hr, minute, sec, jdsatepoch );

   const double a1 = pow( xke / no, 2/3);
   const double cosio = cos(inclo);
   const double theta2 = cosio * cosio;
   const double x3thm1 = 3.0 * theta2 - 1.0;
   const double eosq = ecco * ecco;
   const double betao2 = 1.0 - eosq;
   const double betao = sqrt(betao2);
   const double temp = (1.5 * (j2/2)) * x3thm1 / (betao * betao2);
   const double del1 = temp / (a1 * a1);
   const double a0 = a1 * (1.0 - del1 * (1.0 / 3.0 + del1 * (1.0 + del1 * 134.0 / 81.0)));
   const double del0 = temp / (a0 * a0);

   recovered_mean_motion = no/(1.0 + del0);

} // InitStrVars()

double QTle::Period()
{
    double period;
    if (recovered_mean_motion  == 0)
       period= 0.0;
    else
       period = TWOPI / recovered_mean_motion * 60.0;
    return period;
}

QSgp4Date QTle::Epoch()
{
    QSgp4Date ep(jdsatepoch, true);
    return ep;
}

//////////////////////////////////////////////////////////////////////////////
// Returns elapsed number of seconds from epoch to given time.
// Note: "Predicted" TLEs can have epochs in the future.
double QTle::TPlusEpoch(QSgp4Date &gmt)
{
   return gmt.spanSec(Epoch());
}

//////////////////////////////////////////////////////////////////////////////
// Returns the mean anomaly in radians at given GMT.
// At epoch, the mean anomaly is given by the elements data.
double QTle::MeanAnomaly(QSgp4Date gmt)
{
   double span = TPlusEpoch(gmt);
   double P    = Period();

   assert(P != 0.0);

   return fmod(MeanAnomaly() + (TWOPI * (span / P)), TWOPI);
}
/////////////////////////////////////////////////////////////////////////////
// IsTleFormat()
// Returns true if "str" is a valid data line of a two-line element set,
//   else false.
//
// To be valid a line must:
//      Have as the first character the line number
//      Have as the second character a blank
//      Be TLE_LEN_LINE_DATA characters long
//      Have a valid checksum (note: no longer required as of 12/96)
//      
bool QTle::IsValidLine(QString str, eTleLine line)
{
   str.trimmed();
   
   int nLen = str.length();
   
   if (nLen != TLE_LEN_LINE_DATA)
      return false;
   
   // First char in string must be line number
   if (str.at(0) != line)
      return false;
   
   // Second char in string must be blank
   if (str.at(1) != ' ')
      return false;
   
   /*
      NOTE: 12/96 
      The requirement that the last char in the line data must be a valid 
      checksum is too restrictive. 
      
      // Last char in string must be checksum
      int nSum = CheckSum(str);
     
      if (nSum != (str[TLE_LEN_LINE_DATA - 1] - '0'))
         return false;
   */
   
   return true;
   
} // IsTleFormat()

/////////////////////////////////////////////////////////////////////////////
// CheckSum()
// Calculate the check sum for a given line of TLE data, the last character
// of which is the current checksum. (Although there is no check here,
// the current checksum should match the one we calculate.)
// The checksum algorithm: 
//      Each number in the data line is summed, modulo 10.
//    Non-numeric characters are zero, except minus signs, which are 1.
//
int QTle::CheckSum(const QString str)
{
   // The length is "- 1" because we don't include the current (existing)
   // checksum character in the checksum calculation.
   size_t len = str.length() - 1;
   int xsum = 0;
   
   for (size_t i = 0; i < len; i++)
   {
      QChar ch = str.at(i);
      if (ch.isDigit())
         xsum += ch.digitValue();
      else if (ch == '-')
         xsum++;
   }
   
   return (xsum % 10);
   
} // CheckSum()

/*
For the Earth modelled by the WGS84 ellipsoid the defining values are[7]
a (equatorial radius): 6 378 137.0 m
1/f (inverse flattening): 298.257 223 563
from which one derives
b (polar radius): 6 356 752.3142 m,
*/

/* -----------------------------------------------------------------------------
*
*                           function getgravconst
*
*  this function gets constants for the propagator. note that mu is identified to
*    facilitiate comparisons with newer models. the common useage is wgs72.
*
*  author        : david vallado                  719-573-2600   21 jul 2006
*
*  inputs        :
*    whichconst  - which set of constants to use  wgs72old, wgs72, wgs84
*
*  outputs       :
*    tumin       - minutes in one time unit
*    mu          - earth gravitational parameter
*    radiusearthkm - radius of the earth in km
*    xke         - reciprocal of tumin
*    j2, j3, j4  - un-normalized zonal harmonic values
*    j3oj2       - j3 divided by j2
*
*  locals        :
*
*  coupling      :
*    none
*
*  references    :
*    norad spacetrack report #3
*    vallado, crawford, hujsak, kelso  2006
  --------------------------------------------------------------------------- */

bool QTle::getgravconst
(eGravconsttype whichconst,
        double& tumin,
        double& mu,
        double& radiusearthkm,
        double& xke,
        double& j2,
        double& j3,
        double& j4,
        double& j3oj2,
        double& radiusearthkmminor,
        double& flattening
        )
{

    switch (whichconst)
    {
    // -- wgs-72 low precision str#3 constants --
    case wgs72old:
        mu     = 398600.79964;        // in km3 / s2
        radiusearthkm = 6378.135;     // km
        radiusearthkmminor = 6356.75052;
        flattening = 1 / 298.257;
        xke    = 0.0743669161;
        tumin  = 1.0 / xke;
        j2     =   0.001082616;
        j3     =  -0.00000253881;
        j4     =  -0.00000165597;
        j3oj2  =  j3 / j2;
        break;
        // ------------ wgs-72 constants ------------
    case wgs72:
        mu     = 398600.8;            // in km3 / s2
        radiusearthkm = 6378.135;     // km
        radiusearthkmminor = 6356.75052;
        flattening = 1 / 298.257;
        xke    = 60.0 / sqrt(radiusearthkm*radiusearthkm*radiusearthkm/mu);
        tumin  = 1.0 / xke;
        j2     =   0.001082616;
        j3     =  -0.00000253881;
        j4     =  -0.00000165597;
        j3oj2  =  j3 / j2;
        break;
    case wgs84:
        // ------------ wgs-84 constants ------------
        mu     = 398600.5;            // in km3 / s2
        radiusearthkm = 6378.1370;     // km
        radiusearthkmminor = 6356.752314245;
        flattening = 1 / 298.257223563;
        xke    = 60.0 / sqrt(radiusearthkm*radiusearthkm*radiusearthkm/mu);
        tumin  = 1.0 / xke;
        j2     =   0.00108262998905;
        j3     =  -0.00000253215306;
        j4     =  -0.00000161098761;
        j3oj2  =  j3 / j2;
        break;
    default:
        return false;
        break;
    }
    return true;
}   // end getgravconst

/* -----------------------------------------------------------------------------
*
*                           procedure jday
*
*  this procedure finds the julian date given the year, month, day, and time.
*    the julian date is defined by each elapsed day since noon, jan 1, 4713 bc.
*
*  algorithm     : calculate the answer in one step for efficiency
*
*  author        : david vallado                  719-573-2600    1 mar 2001
*
*  inputs          description                    range / units
*    year        - year                           1900 .. 2100
*    mon         - month                          1 .. 12
*    day         - day                            1 .. 28,29,30,31
*    hr          - universal time hour            0 .. 23
*    min         - universal time min             0 .. 59
*    sec         - universal time sec             0.0 .. 59.999
*
*  outputs       :
*    jd          - julian date                    days from 4713 bc
*
*  locals        :
*    none.
*
*  coupling      :
*    none.
*
*  references    :
*    vallado       2007, 189, alg 14, ex 3-14
*
* --------------------------------------------------------------------------- */

void QTle::jday(
        int year, int mon, int day, int hr, int minute, double sec,
        double& jd
        )
{
    jd = 367.0 * year -
            floor((7 * (year + floor((mon + 9) / 12.0))) * 0.25) +
            floor( 275 * mon / 9.0 ) +
            day + 1721013.5 +
            ((sec / 60.0 + minute) / 60.0 + hr) / 24.0;  // ut in days
    // - 0.5*sgn(100.0*year + mon - 190002.5) + 0.5;
}  // end jday


/* -----------------------------------------------------------------------------
*
*                           procedure days2mdhms
*
*  this procedure converts the day of the year, days, to the equivalent month
*    day, hour, minute and second.
*
*  algorithm     : set up array for the number of days per month
*                  find leap year - use 1900 because 2000 is a leap year
*                  loop through a temp value while the value is < the days
*                  perform int conversions to the correct day and month
*                  convert remainder into h m s using type conversions
*
*  author        : david vallado                  719-573-2600    1 mar 2001
*
*  inputs          description                    range / units
*    year        - year                           1900 .. 2100
*    days        - julian day of the year         0.0  .. 366.0
*
*  outputs       :
*    mon         - month                          1 .. 12
*    day         - day                            1 .. 28,29,30,31
*    hr          - hour                           0 .. 23
*    min         - minute                         0 .. 59
*    sec         - second                         0.0 .. 59.999
*
*  locals        :
*    dayofyr     - day of year
*    temp        - temporary extended values
*    inttemp     - temporary int value
*    i           - index
*    lmonth[12]  - int array containing the number of days per month
*
*  coupling      :
*    none.
* --------------------------------------------------------------------------- */

void    QTle::days2mdhms
(
        int year, double days,
        int& mon, int& day, int& hr, int& minute, double& sec
        )
{
    int i, inttemp, dayofyr;
    double    temp;
    int lmonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    dayofyr = (int)floor(days);
    /* ----------------- find month and day of month ---------------- */
    if ( (year % 4) == 0 )
        lmonth[1] = 29;

    i = 1;
    inttemp = 0;
    while ((dayofyr > inttemp + lmonth[i-1]) && (i < 12))
    {
        inttemp = inttemp + lmonth[i-1];
        i++;
    }
    mon = i;
    day = dayofyr - inttemp;

    /* ----------------- find hours minutes and seconds ------------- */
    temp = (days - dayofyr) * 24.0;
    hr   = (int)floor(temp);
    temp = (temp - hr) * 60.0;
    minute  = (int)floor(temp);
    sec  = (temp - minute) * 60.0;
}  // end days2mdhms

/* -----------------------------------------------------------------------------
*
*                           procedure invjday
*
*  this procedure finds the year, month, day, hour, minute and second
*  given the julian date. tu can be ut1, tdt, tdb, etc.
*
*  algorithm     : set up starting values
*                  find leap year - use 1900 because 2000 is a leap year
*                  find the elapsed days through the year in a loop
*                  call routine to find each individual value
*
*  author        : david vallado                  719-573-2600    1 mar 2001
*
*  inputs          description                    range / units
*    jd          - julian date                    days from 4713 bc
*
*  outputs       :
*    year        - year                           1900 .. 2100
*    mon         - month                          1 .. 12
*    day         - day                            1 .. 28,29,30,31
*    hr          - hour                           0 .. 23
*    min         - minute                         0 .. 59
*    sec         - second                         0.0 .. 59.999
*
*  locals        :
*    days        - day of year plus fractional
*                  portion of a day               days
*    tu          - julian centuries from 0 h
*                  jan 0, 1900
*    temp        - temporary double values
*    leapyrs     - number of leap years from 1900
*
*  coupling      :
*    days2mdhms  - finds month, day, hour, minute and second given days and year
*
*  references    :
*    vallado       2007, 208, alg 22, ex 3-13
* --------------------------------------------------------------------------- */

void    QTle::invjday
(
        double jd,
        int& year, int& mon, int& day,
        int& hr, int& minute, double& sec
        )
{
    int leapyrs;
    double    days, tu, temp;

    /* --------------- find year and days of the year --------------- */
    temp    = jd - 2415019.5;
    tu      = temp / 365.25;
    year    = 1900 + (int)floor(tu);
    leapyrs = (int)floor((year - 1901) * 0.25);

    // optional nudge by 8.64x10-7 sec to get even outputs
    days    = temp - ((year - 1900) * 365.0 + leapyrs) + 0.00000000001;

    /* ------------ check for case of beginning of a year ----------- */
    if (days < 1.0)
    {
        year    = year - 1;
        leapyrs = (int)floor((year - 1901) * 0.25);
        days    = temp - ((year - 1900) * 365.0 + leapyrs);
    }

    /* ----------------- find remaing data  ------------------------- */
    days2mdhms(year, days, mon, day, hr, minute, sec);
    sec = sec - 0.00000086400;
}  // end invjday


/* -----------------------------------------------------------------------------
*
*                           function gstime
*
*  this function finds the greenwich sidereal time.
*
*  author        : david vallado                  719-573-2600    1 mar 2001
*
*  inputs          description                    range / units
*    jdut1       - julian date in ut1             days from 4713 bc
*
*  outputs       :
*    gstime      - greenwich sidereal time        0 to 2pi rad
*
*  locals        :
*    temp        - temporary variable for doubles   rad
*    tut1        - julian centuries from the
*                  jan 1, 2000 12 h epoch (ut1)
*
*  coupling      :
*    none
*
*  references    :
*    vallado       2004, 191, eq 3-45
* --------------------------------------------------------------------------- */

double  QTle::gstime(double jdut1)
{

    double       temp, tut1;

    tut1 = (jdut1 - 2451545.0) / 36525.0;
    temp = -6.2e-6* tut1 * tut1 * tut1 + 0.093104 * tut1 * tut1 +
            (876600.0*3600 + 8640184.812866) * tut1 + 67310.54841;  // sec
    temp = fmod(Util::DegreesToRadians(temp) / 240.0, TWOPI); //360/86400 = 1/240, to deg, to rad

    // ------------------------ check quadrants ---------------------
    if (temp < 0.0)
        temp += TWOPI;

    return temp;
}  // end gstime



