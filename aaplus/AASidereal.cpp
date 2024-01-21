/*
Module : AASidereal.cpp
Purpose: Implementation for the algorithms which obtain sidereal time
Created: PJN / 29-12-2003
         PJN / 26-01-2007 1. Update to fit in with new layout of CAADate class
         PJN / 28-01-2007 1. Minor updates to fit in with new layout of CAADate class
         PJN / 18-08-2019 1. Fixed some further compiler warnings when using VC 2019 Preview v16.3.0 Preview 2.0
         PJN / 11-07-2022 1. Updated all the code in AASidereal.cpp to use C++ uniform initialization for all
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
#include "AASidereal.h"
#include "AACoordinateTransformation.h"
#include "AANutation.h"
#include "AADate.h"
#include <cmath>


//////////////////// Implementation ///////////////////////////////////////////

double CAASidereal::MeanGreenwichSiderealTime(double JD) noexcept
{
  //Get the Julian day for the same day at midnight
  long Year{0};
  long Month{0};
  long Day{0};
  long Hour{0};
  long Minute{0};
  double Second{0};

  CAADate date;
  date.Set(JD, CAADate::AfterPapalReform(JD));
  date.Get(Year, Month, Day, Hour, Minute, Second);
  date.Set(Year, Month, Day, 0, 0, 0, date.InGregorianCalendar());
  const double JDMidnight{date.Julian()};

  //Calculate the sidereal time at midnight
  const double T{(JDMidnight - 2451545)/36525};
  const double TSquared{T*T};
  const double TCubed{TSquared*T};
  double Value{100.46061837 + (36000.770053608*T) + (0.000387933*TSquared) - (TCubed/38710000)};

  //Adjust by the time of day
  Value += (((Hour*15.0) + (Minute*0.25) + (Second*0.0041666666666666666666666666666667))*1.00273790935);

  Value = CAACoordinateTransformation::DegreesToHours(Value);
  return CAACoordinateTransformation::MapTo0To24Range(Value);
}

double CAASidereal::ApparentGreenwichSiderealTime(double JD) noexcept
{
  const double MeanObliquity{CAANutation::MeanObliquityOfEcliptic(JD)};
  const double TrueObliquity{MeanObliquity + (CAANutation::NutationInObliquity(JD)/3600)};
  const double NutationInLongitude{CAANutation::NutationInLongitude(JD)};

  const double Value{MeanGreenwichSiderealTime(JD) + (NutationInLongitude*cos(CAACoordinateTransformation::DegreesToRadians(TrueObliquity))/54000)};
  return CAACoordinateTransformation::MapTo0To24Range(Value);
}
