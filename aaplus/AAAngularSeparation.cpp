/*
Module : AAAngularSeparation.cpp
Purpose: Implementation for the algorithms which obtain various separation distances between celestial objects
Created: PJN / 29-12-2003
History: PJN / 16-05-2022 1. Updated all the code in AAAngularSeparation.cpp to use C++ uniform initialization
                          for all variable declarations.

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
#include "AAAngularSeparation.h"
#include "AACoordinateTransformation.h"
#include <cmath>


//////////////////// Implementation ///////////////////////////////////////////

double CAAAngularSeparation::Separation(double Alpha1, double Delta1, double Alpha2, double Delta2) noexcept
{
  Delta1 = CAACoordinateTransformation::DegreesToRadians(Delta1);
  Delta2 = CAACoordinateTransformation::DegreesToRadians(Delta2);
  Alpha1 = CAACoordinateTransformation::HoursToRadians(Alpha1);
  Alpha2 = CAACoordinateTransformation::HoursToRadians(Alpha2);

  const double x{(cos(Delta1)*sin(Delta2)) - (sin(Delta1)*cos(Delta2)*cos(Alpha2 - Alpha1))};
  const double y{cos(Delta2)*sin(Alpha2 - Alpha1)};
  const double z{(sin(Delta1)*sin(Delta2)) + (cos(Delta1)*cos(Delta2)*cos(Alpha2 - Alpha1))};

  double value{atan2(sqrt((x*x) + (y*y)), z)};
  value = CAACoordinateTransformation::RadiansToDegrees(value);
  if (value < 0)
    value += 180;

  return value;
}

double CAAAngularSeparation::PositionAngle(double Alpha1, double Delta1, double Alpha2, double Delta2) noexcept
{
  Delta1 = CAACoordinateTransformation::DegreesToRadians(Delta1);
  Delta2 = CAACoordinateTransformation::DegreesToRadians(Delta2);
  Alpha1 = CAACoordinateTransformation::HoursToRadians(Alpha1);
  Alpha2 = CAACoordinateTransformation::HoursToRadians(Alpha2);

  const double DeltaAlpha{Alpha1 - Alpha2};
  double value{atan2(sin(DeltaAlpha), (cos(Delta2)*tan(Delta1)) - (sin(Delta2)*cos(DeltaAlpha)))};
  value = CAACoordinateTransformation::RadiansToDegrees(value);
  if (value < 0)
    value += 180;

  return value;
}

double CAAAngularSeparation::DistanceFromGreatArc(double Alpha1, double Delta1, double Alpha2, double Delta2, double Alpha3, double Delta3) noexcept
{
  Delta1 = CAACoordinateTransformation::DegreesToRadians(Delta1);
  Delta2 = CAACoordinateTransformation::DegreesToRadians(Delta2);
  Delta3 = CAACoordinateTransformation::DegreesToRadians(Delta3);
  Alpha1 = CAACoordinateTransformation::HoursToRadians(Alpha1);
  Alpha2 = CAACoordinateTransformation::HoursToRadians(Alpha2);
  Alpha3 = CAACoordinateTransformation::HoursToRadians(Alpha3);

  const double X1{cos(Delta1)*cos(Alpha1)};
  const double X2{cos(Delta2)*cos(Alpha2)};
  const double Y1{cos(Delta1)*sin(Alpha1)};
  const double Y2{cos(Delta2)*sin(Alpha2)};
  const double Z1{sin(Delta1)};
  const double Z2{sin(Delta2)};

  const double A{(Y1*Z2) - (Z1*Y2)};
  const double B{(Z1*X2) - (X1*Z2)};
  const double C{(X1*Y2) - (Y1*X2)};

  const double m{tan(Alpha3)};
  const double n{tan(Delta3)/cos(Alpha3)};

  double value{asin((A + (B*m) + (C*n)) / (sqrt((A*A) + (B*B) + (C*C)) * sqrt(1 + (m*m) + (n*n))))};
  value = CAACoordinateTransformation::RadiansToDegrees(value);
  if (value < 0)
    value = fabs(value);

  return value;
}

double CAAAngularSeparation::SmallestCircle(double Alpha1, double Delta1, double Alpha2, double Delta2, double Alpha3, double Delta3, bool& bType1) noexcept
{
  const double d1{Separation(Alpha1, Delta1, Alpha2, Delta2)};
  const double d2{Separation(Alpha1, Delta1, Alpha3, Delta3)};
  const double d3{Separation(Alpha2, Delta2, Alpha3, Delta3)};

  double a{d1};
  double b{d2};
  double c{d3};
  if (b > a)
  {
    a = d2;
    b = d1;
    c = d3;
  }
  if (c > a)
  {
    a = d3;
    b = d1;
    c = d2;
  }

  double value{0};
  if (a > sqrt((b*b) + (c*c)))
  {
    bType1 = true;
    value = a;
  }
  else
  {
    bType1 = false;
    value = (2*a*b*c)/(sqrt((a+b+c)*(a+b-c)*(b+c-a)*(a+c-b)));
  }

  return value;
}
