/*
Module : AAParallax.cpp
Purpose: Implementation for the algorithms which convert a geocentric set of coordinates to their topocentric equivalent
Created: PJN / 29-12-2003
History: PJN / 04-07-2010 Removed unnecessary "Longitude" parameter from method Ecliptic2Topocentric.
         PJN / 18-03-2012 1. All global "g_*" tables are now const. Thanks to Roger Dahl for reporting this 
                          issue when compiling AA+ on ARM.
         PJN / 18-08-2019 1. Fixed some further compiler warnings when using VC 2019 Preview v16.3.0 Preview 2.0
         PJN / 02-07-2022 1. Updated all the code in AAParallax.cpp to use C++ uniform initialization for all
                          variable declarations.
         PJN / 12-07-2023 1. Updated AAParallax.cpp to use a more accurate value of the Solar Parallax constant of 
                          8.794148" instead of 8.794". This newer value was adopted in 1977 by the IAU. Thanks to 
                          "Pavel" for reporting this issue.

Copyright (c) 2003 - 2023 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code.

*/


//////////////////// Includes /////////////////////////////////////////////////

#include "stdafx.h"
#include "AAParallax.h"
#include "AAGlobe.h"
#include "AACoordinateTransformation.h"
#include "AASidereal.h"
#include <cmath>


//////////////////// Macros / Defines /////////////////////////////////////////

constexpr double g_AAParallax_C1 = 4.2635232628103847e-05; //sin(CAACoordinateTransformation::DegreesToRadians(CAACoordinateTransformation::DMSToDegrees(0, 0, 8.794148)));


//////////////////// Implementation ///////////////////////////////////////////

double CAAParallax::DistanceToParallax(double Distance) noexcept
{
  const double pi{asin(g_AAParallax_C1 / Distance)};
  return CAACoordinateTransformation::RadiansToDegrees(pi);
}

double CAAParallax::ParallaxToDistance(double Parallax) noexcept
{
  return g_AAParallax_C1/sin(CAACoordinateTransformation::DegreesToRadians(Parallax));
}

CAA2DCoordinate CAAParallax::Equatorial2TopocentricDelta(double Alpha, double Delta, double Distance, double Longitude, double Latitude, double Height, double JD) noexcept
{
  const double RhoSinThetaPrime{CAAGlobe::RhoSinThetaPrime(Latitude, Height)};
  const double RhoCosThetaPrime{CAAGlobe::RhoCosThetaPrime(Latitude, Height)};

  //Calculate the Sidereal time
  const double theta{CAASidereal::ApparentGreenwichSiderealTime(JD)};

  //Convert to radians
  Delta = CAACoordinateTransformation::DegreesToRadians(Delta);
  const double cosDelta{cos(Delta)};

  //Calculate the Parallax
  const double pi{asin(g_AAParallax_C1 / Distance)};

  //Calculate the hour angle
  const double H{CAACoordinateTransformation::HoursToRadians(theta - Longitude/15 - Alpha)};
  const double cosH{cos(H)};
  const double sinH{sin(H)};

  CAA2DCoordinate DeltaTopocentric;
  DeltaTopocentric.X = CAACoordinateTransformation::RadiansToHours(-pi*RhoCosThetaPrime*sinH/cosDelta);
  DeltaTopocentric.Y = CAACoordinateTransformation::RadiansToDegrees(-pi*((RhoSinThetaPrime*cosDelta) - (RhoCosThetaPrime*cosH*sin(Delta))));
  return DeltaTopocentric;
}

CAA2DCoordinate CAAParallax::Equatorial2Topocentric(double Alpha, double Delta, double Distance, double Longitude, double Latitude, double Height, double JD) noexcept
{
  const double RhoSinThetaPrime{CAAGlobe::RhoSinThetaPrime(Latitude, Height)};
  const double RhoCosThetaPrime{CAAGlobe::RhoCosThetaPrime(Latitude, Height)};

  //Calculate the Sidereal time
  const double theta{CAASidereal::ApparentGreenwichSiderealTime(JD)};

  //Convert to radians
  Delta = CAACoordinateTransformation::DegreesToRadians(Delta);
  const double cosDelta{cos(Delta)};

  //Calculate the Parallax
  const double pi{asin(g_AAParallax_C1/Distance)};
  const double sinpi{sin(pi)};

  //Calculate the hour angle
  const double H{CAACoordinateTransformation::HoursToRadians(theta - (Longitude/15) - Alpha)};
  const double cosH{cos(H)};
  const double sinH{sin(H)};

  //Calculate the adjustment in right ascension
  const double DeltaAlpha{atan2(-RhoCosThetaPrime*sinpi*sinH, cosDelta - (RhoCosThetaPrime*sinpi*cosH))};

  CAA2DCoordinate Topocentric;
  Topocentric.X = CAACoordinateTransformation::MapTo0To24Range(Alpha + CAACoordinateTransformation::RadiansToHours(DeltaAlpha));
  Topocentric.Y = CAACoordinateTransformation::RadiansToDegrees(atan2((sin(Delta) - (RhoSinThetaPrime*sinpi))*cos(DeltaAlpha), cosDelta - (RhoCosThetaPrime*sinpi*cosH)));
  return Topocentric;
}

CAATopocentricEclipticDetails CAAParallax::Ecliptic2Topocentric(double Lambda, double Beta, double Semidiameter, double Distance, double Epsilon, double Latitude, double Height, double JD) noexcept
{
  const double S{CAAGlobe::RhoSinThetaPrime(Latitude, Height)};
  const double C{CAAGlobe::RhoCosThetaPrime(Latitude, Height)};

  //Convert to radians
  Lambda = CAACoordinateTransformation::DegreesToRadians(Lambda);
  Beta = CAACoordinateTransformation::DegreesToRadians(Beta);
  Epsilon = CAACoordinateTransformation::DegreesToRadians(Epsilon);
  Semidiameter = CAACoordinateTransformation::DegreesToRadians(Semidiameter);
  const double sine{sin(Epsilon)};
  const double cose{cos(Epsilon)};
  const double cosBeta{cos(Beta)};
  const double sinBeta{sin(Beta)};

  //Calculate the Sidereal time
  double theta{CAASidereal::ApparentGreenwichSiderealTime(JD)};
  theta = CAACoordinateTransformation::HoursToRadians(theta);
  const double sintheta{sin(theta)};

  //Calculate the Parallax
  const double pi{asin(g_AAParallax_C1/Distance)};
  const double sinpi{sin(pi)};

  const double N{(cos(Lambda)*cosBeta) - (C*sinpi*cos(theta))};

  CAATopocentricEclipticDetails Topocentric;
  Topocentric.Lambda = atan2((sin(Lambda)*cosBeta) - (sinpi*((S*sine) + (C*cose*sintheta))), N);
  const double cosTopocentricLambda{cos(Topocentric.Lambda)};
  Topocentric.Beta = atan(cosTopocentricLambda*(sinBeta - (sinpi*((S*cose) - (C*sine*sintheta))))/N);
  Topocentric.Semidiameter = asin(cosTopocentricLambda*cos(Topocentric.Beta)*sin(Semidiameter)/N);

  //Convert back to degrees
  Topocentric.Semidiameter = CAACoordinateTransformation::RadiansToDegrees(Topocentric.Semidiameter);
  Topocentric.Lambda = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(Topocentric.Lambda));
  Topocentric.Beta = CAACoordinateTransformation::RadiansToDegrees(Topocentric.Beta);

  return Topocentric;
}
