/*
Module : AAParallactic.cpp
Purpose: Implementation for the algorithms which calculate various celestial globe angles
Created: PJN / 29-12-2003
History: PJN / 18-08-2019 1. Fixed some further compiler warnings when using VC 2019 Preview v16.3.0 Preview 2.0
         PJN / 02-07-2022 1. Updated all the code in AAParallactic.cpp to use C++ uniform initialization for all
                          variable declarations.

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
#include "AAParallactic.h"
#include "AACoordinateTransformation.h"
#include <cmath>


//////////////////// Implementation ///////////////////////////////////////////

double CAAParallactic::ParallacticAngle(double HourAngle, double Latitude, double delta) noexcept
{
  HourAngle = CAACoordinateTransformation::HoursToRadians(HourAngle);
  Latitude = CAACoordinateTransformation::DegreesToRadians(Latitude);
  delta = CAACoordinateTransformation::DegreesToRadians(delta);

  return CAACoordinateTransformation::RadiansToDegrees(atan2(sin(HourAngle), (tan(Latitude)*cos(delta)) - (sin(delta)*cos(HourAngle))));
}

double CAAParallactic::EclipticLongitudeOnHorizon(double LocalSiderealTime, double ObliquityOfEcliptic, double Latitude) noexcept
{
  LocalSiderealTime = CAACoordinateTransformation::HoursToRadians(LocalSiderealTime);
  Latitude = CAACoordinateTransformation::DegreesToRadians(Latitude);
  ObliquityOfEcliptic = CAACoordinateTransformation::DegreesToRadians(ObliquityOfEcliptic);

  const double value{CAACoordinateTransformation::RadiansToDegrees(atan2(-cos(LocalSiderealTime), (sin(ObliquityOfEcliptic)*tan(Latitude)) + (cos(ObliquityOfEcliptic)*sin(LocalSiderealTime))))};
  return CAACoordinateTransformation::MapTo0To360Range(value);
}

double CAAParallactic::AngleBetweenEclipticAndHorizon(double LocalSiderealTime, double ObliquityOfEcliptic, double Latitude) noexcept
{
  LocalSiderealTime = CAACoordinateTransformation::HoursToRadians(LocalSiderealTime);
  Latitude = CAACoordinateTransformation::DegreesToRadians(Latitude);
  ObliquityOfEcliptic = CAACoordinateTransformation::DegreesToRadians(ObliquityOfEcliptic);

  const double value{CAACoordinateTransformation::RadiansToDegrees(acos((cos(ObliquityOfEcliptic)*sin(Latitude)) - (sin(ObliquityOfEcliptic)*cos(Latitude)*sin(LocalSiderealTime))))};
  return CAACoordinateTransformation::MapTo0To360Range(value);
}

double CAAParallactic::AngleBetweenNorthCelestialPoleAndNorthPoleOfEcliptic(double Lambda, double Beta, double ObliquityOfEcliptic) noexcept
{
  Lambda = CAACoordinateTransformation::DegreesToRadians(Lambda);
  Beta = CAACoordinateTransformation::DegreesToRadians(Beta);
  ObliquityOfEcliptic = CAACoordinateTransformation::DegreesToRadians(ObliquityOfEcliptic);

  const double value{CAACoordinateTransformation::RadiansToDegrees(atan2(cos(Lambda)*tan(ObliquityOfEcliptic), (sin(Beta)*sin(Lambda)*tan(ObliquityOfEcliptic)) - cos(Beta)))};
  return CAACoordinateTransformation::MapTo0To360Range(value);
}
