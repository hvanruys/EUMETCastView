///////////////////////////////////////////////////////////////////////////
//
// TLE data format
//
// [Reference: T.S. Kelso]
//
// Two line element data consists of three lines in the following format:
//
//  AAAAAAAAAAAAAAAAAAAAAA
//  1 NNNNNU NNNNNAAA NNNNN.NNNNNNNN +.NNNNNNNN +NNNNN-N +NNNNN-N N NNNNN
//  2 NNNNN NNN.NNNN NNN.NNNN NNNNNNN NNN.NNNN NNN.NNNN NN.NNNNNNNNNNNNNN
//
//  Line 0 is a twenty-two-character name.
//
//   Lines 1 and 2 are the standard Two-Line Orbital Element Set Format identical
//   to that used by NORAD and NASA.  The format description is:
//
//     Line 1
//     Column    Description
//     01-01     Line Number of Element Data
//     03-07     Satellite Number
//     10-11     International Designator (Last two digits of launch year)
//     12-14     International Designator (Launch number of the year)
//     15-17     International Designator (Piece of launch)
//     19-20     Epoch Year (Last two digits of year)
//     21-32     Epoch (Julian Day and fractional portion of the day)
//     34-43     First Time Derivative of the Mean Motion
//               or Ballistic Coefficient (Depending on ephemeris type)
//     45-52     Second Time Derivative of Mean Motion (decimal point assumed;
//               blank if N/A)
//     54-61     BSTAR drag term if GP4 general perturbation theory was used.
//               Otherwise, radiation pressure coefficient.  (Decimal point assumed)
//     63-63     Ephemeris type
//     65-68     Element number
//     69-69     Check Sum (Modulo 10)
//               (Letters, blanks, periods, plus signs = 0; minus signs = 1)
//
//     Line 2
//     Column    Description
//     01-01     Line Number of Element Data
//     03-07     Satellite Number
//     09-16     Inclination [Degrees]
//     18-25     Right Ascension of the Ascending Node [Degrees]
//     27-33     Eccentricity (decimal point assumed)
//     35-42     Argument of Perigee [Degrees]
//     44-51     Mean Anomaly [Degrees]
//     53-63     Mean Motion [Revs per day]
//     64-68     Revolution number at epoch [Revs]
//     69-69     Check Sum (Modulo 10)
//
//     All other columns are blank or fixed.
//
// Example:
//
// NOAA 6
// 1 11416U          86 50.28438588 0.00000140           67960-4 0  5293
// 2 11416  98.5105  69.3305 0012788  63.2828 296.9658 14.24899292346978

#pragma once
#ifndef QTLE_H
#define QTLE_H


#include <QString>
#include <QMap>
#include "qsgp4date.h"
#include "qsgp4globals.h"
#include <cstdio>



///////////////////////////////////////////////////////////////////////  //////
class QTle
{
public:

    enum eOperationmode {
        opsmode_afspc_code,
        opsmode_smooth
    };

    enum eGravconsttype
    {
      wgs72old,
      wgs72,
      wgs84
    };

   QTle(QString, QString, QString, eGravconsttype whichconst);
   QTle(const QTle &tle);
   ~QTle();

   enum eTleLine
   {
      LINE_ZERO,
      LINE_ONE,
      LINE_TWO
   };

   enum eField
   {
      FLD_FIRST,
      FLD_NORADNUM = FLD_FIRST,
      FLD_INTLDESC,
      FLD_SET,       // TLE set number
      FLD_EPOCHYEAR, // Epoch: Last two digits of year
      FLD_EPOCHDAY,  // Epoch: Fractional Julian Day of year
      FLD_ORBITNUM,  // Orbit at epoch
      FLD_I,         // Inclination
      FLD_RAAN,      // R.A. ascending node
      FLD_E,         // Eccentricity
      FLD_ARGPER,    // Argument of perigee
      FLD_M,         // Mean anomaly
      FLD_MMOTION,   // Mean motion
      FLD_MMOTIONDT, // First time derivative of mean motion
      FLD_MMOTIONDT2,// Second time derivative of mean motion
      FLD_BSTAR,     // BSTAR Drag
      FLD_LAST       // MUST be last
   };


   void Initialize();
   
   static int    CheckSum(const QString);
   static bool   IsValidLine(QString, eTleLine);
   static QString ExpToDecimal(const QString);

   QString getName()  const { return m_strName; }
   QString getLine1() const { return m_strLine1;}
   QString getLine2() const { return m_strLine2;}
   static bool getgravconst(eGravconsttype whichconst,
           double& tumin,
           double& mu,
           double& radiusearthkm,
           double& xke,
           double& j2,
           double& j3,
           double& j4,
           double& j3oj2
           , double &radiusearthkmminor, double &flattening);
   void jday(int year, int mon, int day, int hr, int minute, double sec, double& jd );
   void days2mdhms(int year, double days, int& mon, int& day, int& hr, int& minute, double& sec);
   void invjday(double jd, int& year, int& mon, int& day, int& hr, int& minute, double& sec);
   double gstime(double jdut1);

   double MeanAnomaly() { return mo; } // in rad
   double AscendingNode() { return nodeo; } // in rad
   double ArgumentPerigee() { return argpo; } // in rad
   double Eccentricity() { return ecco; }
   double Inclination() { return inclo; }  // in rad
   double MeanMotion() { return no; }  // in rad/min
   double Period();
   QSgp4Date Epoch();

   double TPlusEpoch(QSgp4Date &gmt);
   double MeanAnomaly(QSgp4Date gmt);

protected:

private:

   // Satellite name and two data lines
   QString m_strName;
   QString m_strLine1;
   QString m_strLine2;

   // Converted fields, in atof()-readable form
   QString m_Field[FLD_LAST];
   double recovered_mean_motion;

public:

   eGravconsttype whichconst;
   double tumin, mu, radiusearthkm, xke, j2, j3, j4, j3oj2, radiusearthkmminor,flattening;


   long int  satnum;
   int       epochyr, epochtynumrev;
   int       error;
   QString   errormsg;
   eOperationmode  operationmode;
   char      init, method;

   /* Near Earth */
   int    isimp;
   double aycof  , con41  , cc1    , cc4      , cc5    , d2      , d3   , d4    ,
          delmo  , eta    , argpdot, omgcof   , sinmao , t       , t2cof, t3cof ,
          t4cof  , t5cof  , x1mth2 , x7thm1   , mdot   , nodedot, xlcof , xmcof ,
          nodecf;

   /* Deep Space */
   int    irez;
   double d2201  , d2211  , d3210  , d3222    , d4410  , d4422   , d5220 , d5232 ,
          d5421  , d5433  , dedt   , del1     , del2   , del3    , didt  , dmdt  ,
          dnodt  , domdt  , e3     , ee2      , peo    , pgho    , pho   , pinco ,
          plo    , se2    , se3    , sgh2     , sgh3   , sgh4    , sh2   , sh3   ,
          si2    , si3    , sl2    , sl3      , sl4    , gsto    , xfact , xgh2  ,
          xgh3   , xgh4   , xh2    , xh3      , xi2    , xi3     , xl2   , xl3   ,
          xl4    , xlamo  , zmol   , zmos     , atime  , xli     , xni;

   double a      , altp   , alta   , epochdays, jdsatepoch       , nddot , ndot  ,
          bstar  , rcse   , inclo  , nodeo    , ecco             , argpo , mo    ,
          no;


};


#endif
