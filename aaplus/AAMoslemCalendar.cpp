/*
Module : AAMoslemCalendar.cpp
Purpose: Implementation for the algorithms which convert between the Julian and Moslem calendars
Created: PJN / 04-02-2004
History: PJN / 12-02-2004 1. Replaced all calls to the macro "INT" with the function CAADate::INT which 
                          is what they should have been.
         PJN / 26-01-2007 1. Update to fit in with new layout of CAADate class
         PJN / 28-01-2007 1. Minor updates to fit in with new layout of CAADate class
         PJN / 29-06-2022 1. Updated all the code in AAMoslemCalendar.cpp to use C++ uniform initialization
                          for all variable declarations.

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
#include "AAMoslemCalendar.h"
#include <cmath>


//////////////////// Implementation ///////////////////////////////////////////

CAACalendarDate CAAMoslemCalendar::MoslemToJulian(long Year, long Month, long Day) noexcept
{
  //What will be the return value
  CAACalendarDate JulianDate;

  const long N{Day + CAADate::INT(29.5001*(Month - 1.0) + 0.99)};
  const long Q{CAADate::INT(Year/30.0)};
  const long R{Year % 30};
  const long A{CAADate::INT(((11.0*R) + 3)/30.0)};
  const long W{(404*Q) + (354*R) + 208 + A};
  const long Q1{CAADate::INT(W/1461.0)};
  const long Q2{W % 1461};
  const long G{621 + (4*CAADate::INT((7.0*Q) + Q1))};
  const long K{CAADate::INT(Q2/365.2422)};
  const long E{CAADate::INT(365.2422*K)};
  long J{Q2 - E + N - 1};
  long X{G + K};

  const long XMod4{X % 4};
  if ((J > 366) && (XMod4 == 0))
  {
    J -= 366;
    X++;
  }
  if ((J > 365) && (XMod4 > 0))
  {
    J -= 365;
    X++;
  }

  JulianDate.Year = X;
  CAADate::DayOfYearToDayAndMonth(J, CAADate::IsLeap(X, false), JulianDate.Day, JulianDate.Month);

  return JulianDate;
}

CAACalendarDate CAAMoslemCalendar::JulianToMoslem(long Year, long Month, long Day) noexcept
{
  //What will be the return value
  CAACalendarDate MoslemDate;

  const long W{(Year % 4) ? 2 : 1};
  const long N{CAADate::INT((275.0*Month)/9.0) - (W*CAADate::INT((Month + 9.0)/12.0)) + Day - 30};
  const long A{Year - 623};
  const long B{CAADate::INT(A/4.0)};
  const long C{A % 4};
  const double C1{365.2501*C};
  long C2{CAADate::INT(C1)};
  if ((C1 - C2) > 0.5)
    C2++;

  const long Ddash{(1461*B) + 170 + C2};
  const long Q{CAADate::INT(Ddash/10631.0)};
  const long R{Ddash % 10631};
  const long J{CAADate::INT(R/354.0)};
  const long K{R % 354};
  const long O{CAADate::INT(((11*J) + 14)/30)};
  long H{(30*Q) + J + 1};
  long JJ{K - O + N - 1};

  if (JJ > 354)
  {
    const long CL{H % 30};
    const long DL{((11*CL) + 3) % 30};
    if (DL < 19)
    {
      JJ -= 354;
      H++;
    }
    else
    {
      JJ -= 355;
      H++;
    }
    if (JJ == 0)
    {
      JJ = 355;
      H--;
    }
  }

  const long S{CAADate::INT((JJ - 1.0)/29.5)};
  MoslemDate.Month = 1 + S;
  MoslemDate.Day = CAADate::INT(JJ - (29.5*S));
  MoslemDate.Year = H;

  if (JJ == 355)
  {
    MoslemDate.Month = 12;
    MoslemDate.Day = 30;
  }

  return MoslemDate;
}
