/*
Module : AADiameters.cpp
Purpose: Implementation for the algorithms for the semi diameters of the Sun, Moon, Planets and Asteroids
Created: PJN / 15-01-2004
History: PJN / 18-06-2022 1. Updated all the code in AADiameters.cpp to use C++ uniform initialization 
                          for all variable declarations.
         PJN / 11-07-2023 1. Updated CAADiameters::GeocentricMoonSemidiameter method to use the more 
                          rigorous formula on page 390 as presented in Meeus's book. Also updated the "k"
                          constant used in this method from 0.272481 to the more modern value of k = 
                          0.2725076. Please see section "1.9 Mean Lunar Radius" of 
                          https://umbra.nascom.nasa.gov/eclipse/20060329/text/chapter_1.html for the 
                          reason why the new value was adopted in 1982. Thanks to "Pavel" for reporting 
                          this issue.
                          2. Improved the k value in the CAADiameters::ApparentSaturnPolarSemidiameterA
                          method. Thanks to "Pavel" for reporting this issue.
                          3. Improved the k value in the CAADiameters::ApparentSaturnPolarSemidiameterB
                          method. Thanks to "Pavel" for reporting this issue.

Copyright (c) 2004 - 2023 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

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
#include "AADiameters.h"
#include "AAGlobe.h"
#include <cmath>


//////////////////// Implementation ///////////////////////////////////////////

double CAADiameters::GeocentricMoonSemidiameter(double Delta) noexcept
{
  return CAACoordinateTransformation::RadiansToDegrees(asin(0.2725076*6378.14/Delta)) * 3600;
}

double CAADiameters::ApparentSaturnPolarSemidiameterA(double Delta, double B) noexcept
{
  const double cosB{cos(CAACoordinateTransformation::DegreesToRadians(B))};
  return SaturnPolarSemidiameterA(Delta)*sqrt(1 - (0.19919731146620168*cosB*cosB));
}

double CAADiameters::ApparentSaturnPolarSemidiameterB(double Delta, double B) noexcept
{
  const double cosB{cos(CAACoordinateTransformation::DegreesToRadians(B))};
  return SaturnPolarSemidiameterB(Delta)*sqrt(1 - (0.20380025700102367*cosB*cosB));
}

double CAADiameters::TopocentricMoonSemidiameter(double DistanceDelta, double Delta, double H, double Latitude, double Height) noexcept
{
  //Convert to radians
  H = CAACoordinateTransformation::HoursToRadians(H);
  Delta = CAACoordinateTransformation::DegreesToRadians(Delta);

  const double pi{asin(6378.14/DistanceDelta)};
  const double cosDelta{cos(Delta)};
  const double A{cosDelta*sin(H)};
  const double sinPi{sin(pi)};
  const double B{(cosDelta*cos(H)) - (CAAGlobe::RhoCosThetaPrime(Latitude, Height)*sinPi)};
  const double C{sin(Delta) - (CAAGlobe::RhoSinThetaPrime(Latitude, Height)*sinPi)};
  const double q{sqrt((A*A) + (B*B) + (C*C))};

  const double s{CAACoordinateTransformation::DegreesToRadians(GeocentricMoonSemidiameter(DistanceDelta) / 3600)};
  return CAACoordinateTransformation::RadiansToDegrees(asin(sin(s)/q))*3600;
}

double CAADiameters::AsteroidDiameter(double H, double A) noexcept
{
  const double x{3.12 - (H / 5) - (0.217147 * log(A))};
  return pow(10.0, x);
}
