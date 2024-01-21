/*
Module : AAPlanetaryPhenomena.cpp
Purpose: Implementation for the algorithms which obtain the dates of various planetary phenomena
Created: PJN / 29-12-2003
History: PJN / 08-05-2011 1. Fixed a compilation issue on GCC where size_t was undefined in various methods. Thanks to 
                          Carsten A. Arnholm and Andrew Hammond for reporting this bug.
         PJN / 18-03-2012 1. All global "g_*" tables are now const. Thanks to Roger Dahl for reporting this 
                          issue when compiling AA+ on ARM.
         PJN / 22-07-2018 1. Fixed a number of gcc warnings in CAAPlanetaryPhenomena::K, CAAPlanetaryPhenomena::Mean,
                          CAAPlanetaryPhenomena::True &  CAAPlanetaryPhenomena::ElongationValue. Thanks to Todd Carnes 
                          for reporting this issue.
         PJN / 24-07-2018 1. Fixed a number of GCC warnings in the methods CAAPlanetaryPhenomena::K, 
                          CAAPlanetaryPhenomena::Mean, CAAPlanetaryPhenomena::True & 
                          CAAPlanetaryPhenomena::ElongationValue. Thanks to Todd Carnes for reporting this issue.
         PJN / 18-08-2019 1. Fixed some further compiler warnings when using VC 2019 Preview v16.3.0 Preview 2.0
         PJN / 15-04-2020 1. Reworked C arrays to use std::array
         PJN / 12-03-2021 1. Changed the behavior of the CAAPlanetaryPhenomena::K method to now return the K value before 
                          it is rounded. This new behaviour is now consistent with all the other methods in the AA+ 
                          framework which return so called "K" values. This means that client code must round this value 
                          to an integer before calling other methods in this class with this K value.
         PJN / 03-10-2021 1. Renamed CAAPlanetaryPhenomena::PlanetaryObject type to Planet.
                          2. Renamed CAAPlanetaryPhenomena::EventType type to Type.
         PJN / 04-07-2022 1. Updated all the code in AAPlanetaryPhenomena.cpp to use C++ uniform initialization for all 
                          variable declarations.
                          2. Fixed a transcription error in the lookup tables used to calculate the Station 1 values for
                          Mars.

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
#include "AAPlanetaryPhenomena.h"
#include "AACoordinateTransformation.h"
#include <cmath>
#include <cassert>
#include <cstddef>
#include <array>


//////////////////// Macros / Defines /////////////////////////////////////////

struct PlanetaryPhenomenaCoefficient1
{
  double A;
  double B;
  double M0;
  double M1;
};

constexpr std::array<PlanetaryPhenomenaCoefficient1, 14> g_PlanetaryPhenomenaCoefficient1
{ {
  { 2451612.023, 115.8774771, 63.5867,  114.2088742 },
  { 2451554.084, 115.8774771, 6.4822,   114.2088742 },
  { 2451996.706, 583.921361,  82.7311,  215.513058  },
  { 2451704.746, 583.921361,  154.9745, 215.513058  },
  { 2452097.382, 779.936104,  181.9573, 48.705244   },
  { 2451707.414, 779.936104,  157.6047, 48.705244   },
  { 2451870.628, 398.884046,  318.4681, 33.140229   },
  { 2451671.186, 398.884046,  121.8980, 33.140229   },
  { 2451870.170, 378.091904,  318.0172, 12.647487   },
  { 2451681.124, 378.091904,  131.6934, 12.647487   },
  { 2451764.317, 369.656035,  213.6884, 4.333093    },
  { 2451579.489, 369.656035,  31.5219,  4.333093    },
  { 2451753.122, 367.486703,  202.6544, 2.194998    },
  { 2451569.379, 367.486703,  21.5569,  2.194998    }
} };


/////////////////////////// Implementation ////////////////////////////////////

double CAAPlanetaryPhenomena::K(double Year, Planet planet, Type type) noexcept
{
  size_t nCoefficient{0};
  if (planet >= Planet::MARS)
  {
    assert((type == Type::OPPOSITION) || (type == Type::CONJUNCTION));

    if (type == Type::OPPOSITION)
      nCoefficient = static_cast<size_t>(planet)*2;
    else
      nCoefficient = (static_cast<size_t>(planet)*2) + 1;
  }
  else
  {
    assert((type == Type::INFERIOR_CONJUNCTION) || (type == Type::SUPERIOR_CONJUNCTION));

    if (type == Type::INFERIOR_CONJUNCTION)
      nCoefficient = static_cast<size_t>(planet)*2;
    else
      nCoefficient = (static_cast<size_t>(planet)*2) + 1;
  }
  assert(nCoefficient < g_PlanetaryPhenomenaCoefficient1.size());
#ifdef _MSC_VER
  #pragma warning(suppress : 26446 26482)
#endif //#ifdef _MSC_VER
  return ((365.2425*Year) + 1721060 - g_PlanetaryPhenomenaCoefficient1[nCoefficient].A)/g_PlanetaryPhenomenaCoefficient1[nCoefficient].B;
}

double CAAPlanetaryPhenomena::Mean(double k, Planet planet, Type type) noexcept
{
  size_t nCoefficient{0};
  if (planet >= Planet::MARS)
  {
    assert((type == Type::OPPOSITION) || (type == Type::CONJUNCTION));

    if (type == Type::OPPOSITION)
      nCoefficient = static_cast<size_t>(planet)*2;
    else
      nCoefficient = (static_cast<size_t>(planet)*2) + 1;
  }
  else
  {
    assert((type == Type::INFERIOR_CONJUNCTION) || (type == Type::SUPERIOR_CONJUNCTION));

    if (type == Type::INFERIOR_CONJUNCTION)
      nCoefficient = static_cast<size_t>(planet)*2;
    else
      nCoefficient = (static_cast<size_t>(planet)*2) + 1;
  }
  assert(nCoefficient < g_PlanetaryPhenomenaCoefficient1.size());
#ifdef _MSC_VER
  #pragma warning(suppress : 26446 26482)
#endif //#ifdef _MSC_VER
  return g_PlanetaryPhenomenaCoefficient1[nCoefficient].A + (g_PlanetaryPhenomenaCoefficient1[nCoefficient].B*k);
}

double CAAPlanetaryPhenomena::True(double k, Planet planet, Type type) noexcept
{
  double JDE0{0};
  if ((type == Type::WESTERN_ELONGATION) || (type == Type::EASTERN_ELONGATION) || (type == Type::STATION1) || (type == Type::STATION2))
  {
    if (planet >= Planet::MARS)
      JDE0 = Mean(k, planet, Type::OPPOSITION);
    else
      JDE0 = Mean(k, planet, Type::INFERIOR_CONJUNCTION);
  }
  else
    JDE0 = Mean(k, planet, type);

  size_t nCoefficient{0};
  if (planet >= Planet::MARS)
  {
    assert((type == Type::OPPOSITION) || (type == Type::CONJUNCTION) || (type == Type::STATION1) || (type == Type::STATION2));

    if (type == Type::OPPOSITION || type == Type::STATION1 || type == Type::STATION2)
      nCoefficient = static_cast<size_t>(planet)*2;
    else
      nCoefficient = (static_cast<size_t>(planet)*2) + 1;
  }
  else
  {
    assert((type == Type::INFERIOR_CONJUNCTION) || (type == Type::SUPERIOR_CONJUNCTION) || (type == Type::EASTERN_ELONGATION) ||
           (type == Type::WESTERN_ELONGATION) || (type == Type::STATION1) || (type == Type::STATION2));

    if ((type == Type::INFERIOR_CONJUNCTION) || (type == Type::EASTERN_ELONGATION) || (type == Type::WESTERN_ELONGATION) || (type == Type::STATION1) || (type == Type::STATION2))
      nCoefficient = static_cast<size_t>(planet)*2;
    else
      nCoefficient = (static_cast<size_t>(planet)*2) + 1;
  }
  assert(nCoefficient < g_PlanetaryPhenomenaCoefficient1.size());
#ifdef _MSC_VER
  #pragma warning(suppress : 26482 26446)
#endif //#ifdef _MSC_VER
  double M{CAACoordinateTransformation::MapTo0To360Range(g_PlanetaryPhenomenaCoefficient1[nCoefficient].M0 + (g_PlanetaryPhenomenaCoefficient1[nCoefficient].M1*k))};
  M = CAACoordinateTransformation::DegreesToRadians(M); //convert M to radians
  const double twoM{2*M};
  const double threeM{3*M};
  const double fourM{4*M};
  const double fiveM{5*M};

  const double T{(JDE0 - 2451545)/36525};
  const double T2{T*T};

  double a{0};
  double b{0};
  double c{0};
  double d{0};
  double e{0};
  double f{0};
  double g{0};

  if (planet == Planet::JUPITER)
  {
    a = CAACoordinateTransformation::MapTo0To360Range(82.74 + (40.76*T));
    a = CAACoordinateTransformation::DegreesToRadians(a);
  }
  else if (planet == Planet::SATURN)
  {
    a = CAACoordinateTransformation::MapTo0To360Range(82.74 + (40.76*T));
    a = CAACoordinateTransformation::DegreesToRadians(a);
    b = CAACoordinateTransformation::MapTo0To360Range(29.86 + (1181.36*T));
    b = CAACoordinateTransformation::DegreesToRadians(b);
    c = CAACoordinateTransformation::MapTo0To360Range(14.13 + (590.68*T));
    c = CAACoordinateTransformation::DegreesToRadians(c);
    d = CAACoordinateTransformation::MapTo0To360Range(220.02 + (1262.87*T));
    d = CAACoordinateTransformation::DegreesToRadians(d);
  }
  else if (planet == Planet::URANUS)
  {
    e = CAACoordinateTransformation::MapTo0To360Range(207.83 + (8.51*T));
    e = CAACoordinateTransformation::DegreesToRadians(e);
    f = CAACoordinateTransformation::MapTo0To360Range(108.84 + (419.96*T));
    f = CAACoordinateTransformation::DegreesToRadians(f);
  }
  else if (planet == Planet::NEPTUNE)
  {
    e = CAACoordinateTransformation::MapTo0To360Range(207.83 + (8.51*T));
    e = CAACoordinateTransformation::DegreesToRadians(e);
    g = CAACoordinateTransformation::MapTo0To360Range(276.74 + (209.98*T));
    g = CAACoordinateTransformation::DegreesToRadians(g);
  }

  double delta{0};
  if (planet == Planet::MERCURY)
  {
    if (type == Type::INFERIOR_CONJUNCTION)
    {
      delta = (0.0545 + (0.0002*T)) + 
              (sin(M)*(-6.2008 + (0.0074*T) + (0.00003*T2))) +
              (cos(M)*(-3.2750 - (0.0197*T) + (0.00001*T2))) +
              (sin(twoM)*(0.4737 - (0.0052*T) - (0.00001*T2))) +
              (cos(twoM)*(0.8111 + (0.0033*T) - (0.00002*T2))) +
              (sin(threeM)*(0.0037 + (0.0018*T))) +
              (cos(threeM)*(-0.1768 + (0.00001*T2))) +
              (sin(fourM)*(-0.0211 - (0.0004*T))) +
              (cos(fourM)*(0.0326 - (0.0003*T))) +
              (sin(fiveM)*(0.0083 + (0.0001*T))) +
              (cos(fiveM)*(-0.0040 + (0.0001*T)));
    }
    else if (type == Type::SUPERIOR_CONJUNCTION)
    {
      delta = (-0.0548 - (0.0002*T)) +
              (sin(M)*(7.3894 - (0.0100*T) - (0.00003*T2))) +
              (cos(M)*(3.2200 + (0.0197*T) - (0.00001*T2))) +
              (sin(twoM)*(0.8383 - (0.0064*T) - (0.00001*T2))) +
              (cos(twoM)*(0.9666 + (0.0039*T) - (0.00003*T2))) +
              (sin(threeM)*(0.0770 - (0.0026*T))) +
              (cos(threeM)*(0.2758 + (0.0002*T) - (0.00002*T2))) +
              (sin(fourM)*(-0.0128 - (0.0008*T))) +
              (cos(fourM)*(0.0734 - (0.0004*T) - (0.00001*T2))) +
              (sin(fiveM)*(-0.0122 - (0.0002*T))) +
              (cos(fiveM)*(0.0173 - (0.0002*T)));
    }
    else if (type == Type::EASTERN_ELONGATION)
    {
      delta = (-21.6101 + (0.0002*T)) +
              (sin(M)*(-1.9803 - (0.0060*T) + (0.00001*T2))) +
              (cos(M)*(1.4151 - (0.0072*T) - (0.00001*T2))) +
              (sin(twoM)*(0.5528 - (0.0005*T) - (0.00001*T2))) +
              (cos(twoM)*(0.2905 + (0.0034*T) + (0.00001*T2))) +
              (sin(threeM)*(-0.1121 - (0.0001*T) + (0.00001*T2))) +
              (cos(threeM)*(-0.0098 - (0.0015*T))) +
              (sin(fourM)*0.0192) +
              (cos(fourM)*(0.0111 + (0.0004*T))) +
              (sin(fiveM)*-0.0061) +
              (cos(fiveM)*(-0.0032 - (0.0001*T2)));
    }
    else if (type == Type::WESTERN_ELONGATION)
    {
      delta = (21.6249 - (0.0002*T)) +
              (sin(M)*(0.1306 + 0.0065*T)) +
              (cos(M)*(-2.7661 - (0.0011*T) + (0.00001*T2))) +
              (sin(twoM)*(0.2438 - (0.0024*T) - (0.00001*T2))) +
              (cos(twoM)*(0.5767 + (0.0023*T))) +
              (sin(threeM)*0.1041) +
              (cos(threeM)*(-0.0184 + (0.0007*T))) +
              (sin(fourM)*(-0.0051 - (0.0001*T))) +
              (cos(fourM)*(0.0048 + (0.0001*T))) +
              (sin(fiveM)*0.0026) +
              (cos(fiveM)*0.0037);
    }
    else if (type == Type::STATION1)
    {
      delta = (-11.0761 + (0.0003*T)) +
              (sin(M)*(-4.7321 + (0.0023*T) + (0.00002*T2))) +
              (cos(M)*(-1.3230 - (0.0156*T))) +
              (sin(twoM)*(0.2270 - (0.0046*T))) +
              (cos(twoM)*(0.7184 + (0.0013*T) - (0.00002*T2))) +
              (sin(threeM)*(0.0638 + (0.0016*T))) +
              (cos(threeM)*(-0.1655 + (0.0007*T))) +
              (sin(fourM)*(-0.0395 - (0.0003*T))) +
              (cos(fourM)*(0.0247 - (0.0006*T))) +
              (sin(fiveM)*0.0131) +
              (cos(fiveM)*(0.0008 + (0.0002*T)));
    }
    else
    {
      assert(type == Type::STATION2);

      delta = (11.1343 - (0.0001*T)) +
              (sin(M)*(-3.9137 + (0.0073*T) + (0.00002*T2))) +
              (cos(M)*(-3.3861 - (0.0128*T) + (0.00001*T2))) +
              (sin(twoM)*(0.5222 - (0.0040*T) - (0.00002*T2))) +
              (cos(twoM)*(0.5929 + (0.0039*T) - (0.00002*T2))) +
              (sin(threeM)*(-0.0593 + (0.0018*T))) +
              (cos(threeM)*(-0.1733 - (0.0007*T) + (0.00001*T2))) +
              (sin(fourM)*(-0.0053 - (0.0006*T))) +
              (cos(fourM)*(0.0476 - (0.0001*T))) +
              (sin(fiveM)*(0.0070 + (0.0002*T))) +
              (cos(fiveM)*(-0.0115 + (0.0001*T)));
    }
  }
  else if (planet == Planet::VENUS)
  {
    if (type == Type::INFERIOR_CONJUNCTION)
    {
      delta = (-0.0096 + (0.0002*T) - (0.00001*T2)) +
              (sin(M)*(2.0009 - (0.0033*T) - (0.00001*T2))) +
              (cos(M)*(0.5980 - (0.0104*T) + (0.00001*T2))) +
              (sin(twoM)*(0.0967 - (0.0018*T) - (0.00003*T2))) +
              (cos(twoM)*(0.0913 + (0.0009*T) - (0.00002*T2))) +
              (sin(threeM)*(0.0046 - (0.0002*T))) +
              (cos(threeM)*(0.0079 + (0.0001*T)));
    }
    else if (type == Type::SUPERIOR_CONJUNCTION)
    {
      delta = (0.0099 - (0.0002*T) - (0.00001*T2)) +
              (sin(M)*(4.1991 - (0.0121*T) - (0.00003*T2))) +
              (cos(M)*(-0.6095 + (0.0102*T) - (0.00002*T2))) +
              (sin(twoM)*(0.2500 - (0.0028*T) - (0.00003*T2))) +
              (cos(twoM)*(0.0063 + (0.0025*T) - (0.00002*T2))) +
              (sin(threeM)*(0.0232 - (0.0005*T) - (0.00001*T2))) +
              (cos(threeM)*(0.0031 + (0.0004*T)));
    }
    else if (type == Type::EASTERN_ELONGATION)
    {
      delta = (-70.7600 + (0.0002*T) - (0.00001*T2)) +
              (sin(M)*(1.0282 - (0.0010*T) - (0.00001*T2))) +
              (cos(M)*(0.2761 - (0.0060*T))) +
              (sin(twoM)*(-0.0438 - (0.0023*T) + (0.00002*T2))) +
              (cos(twoM)*(0.1660 - (0.0037*T) - (0.00004*T2))) +
              (sin(threeM)*(0.0036 + (0.0001*T))) +
              (cos(threeM)*(-0.0011 + (0.00001*T2)));
    }
    else if (type == Type::WESTERN_ELONGATION)
    {
      delta = (70.7462 - (0.00001*T2)) +
              (sin(M)*(1.1218 - (0.0025*T) - (0.00001*T2))) +
              (cos(M)*(0.4538 - (0.0066*T))) +
              (sin(twoM)*(0.1320 + (0.0020*T) - (0.00003*T2))) +
              (cos(twoM)*(-0.0702 + (0.0022*T) + (0.00004*T2))) +
              (sin(threeM)*(0.0062 - (0.0001*T))) +
              (cos(threeM)*(0.0015 - (0.00001*T2)));
    }
    else if (type == Type::STATION1)
    {
      delta = (-21.0672 + (0.0002*T) - (0.00001*T2)) +
              (sin(M)*(1.9396 - (0.0029*T) - (0.00001*T2))) +
              (cos(M)*(1.0727 - (0.0102*T))) +
              (sin(twoM)*(0.0404 - (0.0023*T) - (0.00001*T2))) +
              (cos(twoM)*(0.1305 - (0.0004*T) - (0.00003*T2))) +
              (sin(threeM)*(-0.0007 - (0.0002*T))) +
              (cos(threeM)*0.0098);
    }
    else
    {
      assert(type == Type::STATION2);

      delta = (21.0623 - (0.00001*T2)) +
              (sin(M)*(1.9913 - (0.0040*T) - (0.00001*T2))) +
              (cos(M)*(-0.0407 - (0.0077*T))) +
              (sin(twoM)*(0.1351 - (0.0009*T) - (0.00004*T2))) +
              (cos(twoM)*(0.0303 + (0.0019*T))) +
              (sin(threeM)*(0.0089 - (0.0002*T))) +
              (cos(threeM)*(0.0043 + (0.0001*T)));
    }
  }
  else if (planet == Planet::MARS)
  {
    if (type == Type::OPPOSITION)
    {
      delta = (-0.3088 + (0.00002*T2)) +
              (sin(M)*(-17.6965 + (0.0363*T) + (0.00005*T2))) +
              (cos(M)*(18.3131 + (0.0467*T) - (0.00006*T2))) +
              (sin(twoM)*(-0.2162 - (0.0198*T) - (0.00001*T2))) +
              (cos(twoM)*(-4.5028 - (0.0019*T) + (0.00007*T2))) +
              (sin(threeM)*(0.8987 + (0.0058*T) - (0.00002*T2))) +
              (cos(threeM)*(0.7666 - (0.0050*T) - (0.00003*T2))) +
              (sin(fourM)*(-0.3636 - (0.0001*T) + (0.00002*T2))) +
              (cos(fourM)*(0.0402 + (0.0032*T))) +
              (sin(fiveM)*(0.0737 - (0.0008*T))) +
              (cos(fiveM)*(-0.0980 - (0.0011*T)));
    }
    else if (type == Type::CONJUNCTION)
    {
      delta = (0.3102 - (0.0001*T) + (0.00001*T2)) +
              (sin(M)*(9.7273 - (0.0156*T) + (0.00001*T2))) +
              (cos(M)*(-18.3195 - (0.0467*T) + (0.00009*T2))) +
              (sin(twoM)*(-1.6488 - (0.0133*T) + (0.00001*T2))) +
              (cos(twoM)*(-2.6117 - (0.0020*T) + (0.00004*T2))) +
              (sin(threeM)*(-0.6827 - (0.0026*T) + (0.00001*T2))) +
              (cos(threeM)*(0.0281 + (0.0035*T) + (0.00001*T2))) +
              (sin(fourM)*(-0.0823 + (0.0006*T) + (0.00001*T2))) +
              (cos(fourM)*(0.1584 + (0.0013*T))) +
              (sin(fiveM)*(0.0270 + (0.0005*T))) +
              (cos(fiveM)*0.0433);
    }
    else if (type == Type::STATION1)
    {
      delta = (-37.0790 - (0.0009*T) + (0.00002*T2)) +
              (sin(M)*(-20.0651 + (0.0228*T) + (0.00004*T2))) +
              (cos(M)*(14.5205 + (0.0504*T) - (0.00001*T2))) +
              (sin(twoM)*(1.1737 - (0.0169*T))) +
              (cos(twoM)*(-4.2550 - (0.0075*T) + (0.00008*T2))) +
              (sin(threeM)*(0.4897 + (0.0074*T) - (0.00001*T2))) +
              (cos(threeM)*(1.1151 - (0.0021*T) - (0.00005*T2))) +
              (sin(fourM)*(-0.3636 - (0.0020*T) + (0.00001*T2))) +
              (cos(fourM)*(-0.1769 + (0.0028*T) + (0.00002*T2))) +
              (sin(fiveM)*(0.1437 - (0.0004*T))) +
              (cos(fiveM)*(-0.0383 - (0.0016*T)));
    }
    else
    {
      assert(type == Type::STATION2);

      delta = (36.7191 + (0.0016*T) + (0.00003*T2)) +
              (sin(M)*(-12.6163 + (0.0417*T) - (0.00001*T2))) +
              (cos(M)*(20.1218 + (0.0379*T) - (0.00006*T2))) +
              (sin(twoM)*(-1.6360 - (0.0190*T))) +
              (cos(twoM)*(-3.9657 + (0.0045*T) + (0.00007*T2))) +
              (sin(threeM)*(1.1546 + (0.0029*T) - (0.00003*T2))) +
              (cos(threeM)*(0.2888 - (0.0073*T) - (0.00002*T2))) +
              (sin(fourM)*(-0.3128 + (0.0017*T) + (0.00002*T2))) +
              (cos(fourM)*(0.2513 + (0.0026*T) - (0.00002*T2))) +
              (sin(fiveM)*(-0.0021 - (0.0016*T))) +
              (cos(fiveM)*(-0.1497 - (0.0006*T)));
    }
  }
  else if (planet == Planet::JUPITER)
  {
    if (type == Type::OPPOSITION)
    {
      delta = (-0.1029 - (0.00009*T2)) +
              (sin(M)*(-1.9658 - (0.0056*T) + (0.00007*T2))) +
              (cos(M)*(6.1537 + (0.0210*T) - (0.00006*T2))) +
              (sin(twoM)*(-0.2081 - (0.0013*T))) +
              (cos(twoM)*(-0.1116 - (0.0010*T))) +
              (sin(threeM)*(0.0074 + (0.0001*T))) +
              (cos(threeM)*(-0.0097 - (0.0001*T))) +
              (sin(a)*(0.0144*T - (0.00008*T2))) +
              (cos(a)*(0.3642 - (0.0019*T) - (0.00029*T2)));
    }
    else if (type == Type::CONJUNCTION)
    {
      delta = (0.1027 + (0.0002*T) - (0.00009*T2)) +
              (sin(M)*(-2.2637 + (0.0163*T) - (0.00003*T2))) +
              (cos(M)*(-6.1540 - (0.0210*T) + (0.00008*T2))) +
              (sin(twoM)*(-0.2021 - (0.0017*T) + (0.00001*T2))) +
              (cos(twoM)*(0.1310 - (0.0008*T))) +
              (sin(threeM)*0.0086) +
              (cos(threeM)*(0.0087 + (0.0002*T))) +
              (sin(a)*(0.0144*T - (0.00008*T2))) +
              (cos(a)*(0.3642 - (0.0019*T) - (0.00029*T2)));
    }
    else if (type == Type::STATION1)
    {
      delta = (-60.3670 - (0.0001*T) - (0.00009*T2)) +
              (sin(M)*(-2.3144 - (0.0124*T) + (0.00007*T2))) +
              (cos(M)*(6.7439 + (0.0166*T) - (0.00006*T2))) +
              (sin(twoM)*(-0.2259 - (0.0010*T))) +
              (cos(twoM)*(-0.1497 - (0.0014*T))) +
              (sin(threeM)*(0.0105 + (0.0001*T))) +
              (cos(threeM)*-0.0098) +
              (sin(a)*(0.0144*T - (0.00008*T2))) +
              (cos(a)*(0.3642 - (0.0019*T) - (0.00029*T2)));
    }
    else
    {
      assert(type == Type::STATION2);

      delta = (60.3023 + (0.0002*T) - (0.00009*T2)) +
              (sin(M)*(0.3506 - (0.0034*T) + (0.00004*T2))) +
              (cos(M)*(5.3635 + (0.0247*T) - (0.00007*T2))) +
              (sin(twoM)*(-0.1872 - (0.0016*T))) +
              (cos(twoM)*(-0.0037 - (0.0005*T))) +
              (sin(threeM)*(0.0012 + (0.0001*T))) +
              (cos(threeM)*(-0.0096 - (0.0001*T))) +
              (sin(a)*((0.0144*T) - (0.00008*T2))) +
              (cos(a)*(0.3642 - (0.0019*T) - (0.00029*T2)));
    }
  }
  else if (planet == Planet::SATURN)
  {
    if (type == Type::OPPOSITION)
    {
      delta = (-0.0209 + (0.0006*T) + (0.00023*T2)) +
              (sin(M)*(4.5795 - (0.0312*T) - (0.00017*T2))) +
              (cos(M)*(1.1462 - (0.0351*T) + (0.00011*T2))) +
              (sin(twoM)*(0.0985 - (0.0015*T))) +
              (cos(twoM)*(0.0733 - (0.0031*T) + (0.00001*T2))) +
              (sin(threeM)*(0.0025 - (0.0001*T))) +
              (cos(threeM)*(0.0050 - (0.0002*T))) +
              (sin(a)*(-0.0337*T + (0.00018*T2))) +
              (cos(a)*(-0.8510 + (0.0044*T) + (0.00068*T2))) +
              (sin(b)*(-0.0064*T + (0.00004*T2))) +
              (cos(b)*(0.2397 - (0.0012*T) - (0.00008*T2))) +
              (sin(c)*(-0.0010*T)) +
              (cos(c)*(0.1245 + (0.0006*T))) +
              (sin(d)*((0.0024*T) - (0.00003*T2))) +
              (cos(d)*(0.0477 - (0.0005*T) - (0.00006*T2)));
    }
    else if (type == Type::CONJUNCTION)
    {
      delta = (0.0172 - (0.0006*T) + (0.00023*T2)) +
              (sin(M)*(-8.5885 + (0.0411*T) + (0.00020*T2))) +
              (cos(M)*(-1.1470 + (0.0352*T) - (0.00011*T2))) +
              (sin(twoM)*(0.3331 - (0.0034*T) - (0.00001*T2))) +
              (cos(twoM)*(0.1145 - (0.0045*T) + (0.00002*T2))) +
              (sin(threeM)*(-0.0169 + (0.0002*T))) +
              (cos(threeM)*(-0.0109 + (0.0004*T))) +
              (sin(a)*((-0.0337*T) + (0.00018*T2))) +
              (cos(a)*(-0.8510 + (0.0044*T) + (0.00068*T2))) +
              (sin(b)*((-0.0064*T) + (0.00004*T2))) +
              (cos(b)*(0.2397 - (0.0012*T) - (0.00008*T2))) +
              (sin(c)*(-0.0010*T)) +
              (cos(c)*(0.1245 + (0.0006*T))) +
              (sin(d)*((0.0024*T) - (0.00003*T2))) +
              (cos(d)*(0.0477 - (0.0005*T) - (0.00006*T2)));
    }
    else if (type == Type::STATION1)
    {
      delta = (-68.8840 + (0.0009*T) + (0.00023*T2)) +
              (sin(M)*(5.5452 - (0.0279*T) - (0.00020*T2))) +
              (cos(M)*(3.0727 - (0.0430*T) + (0.00007*T2))) +
              (sin(twoM)*(0.1101 - (0.0006*T) - (0.00001*T2))) +
              (cos(twoM)*(0.1654 - (0.0043*T) + (0.00001*T2))) +
              (sin(threeM)*(0.0010 + (0.0001*T))) +
              (cos(threeM)*(0.0095 - (0.0003*T))) +
              (sin(a)*(-0.0337*T + (0.00018*T2))) +
              (cos(a)*(-0.8510 + (0.0044*T) + (0.00068*T2))) +
              (sin(b)*((-0.0064*T) + (0.00004 * T2))) +
              (cos(b)*(0.2397 - (0.0012*T) - (0.00008*T2))) +
              (sin(c)*(-0.0010*T)) +
              (cos(c)*(0.1245 + (0.0006*T))) +
              (sin(d)*((0.0024*T) - (0.00003*T2))) +
              (cos(d)*(0.0477 - (0.0005*T) - (0.00006*T2)));
    }
    else
    {
      assert(type == Type::STATION2);

      delta = (68.8720 - (0.0007*T) + (0.00023*T2)) +
              (sin(M)*(5.9399 - (0.0400*T) - (0.00015*T2))) +
              (cos(M)*(-0.7998 - (0.0266*T) + (0.00014*T2))) +
              (sin(twoM)*(0.1738 - (0.0032*T))) +
              (cos(twoM)*(-0.0039 - (0.0024*T) + (0.00001*T2))) +
              (sin(threeM)*(0.0073 - (0.0002*T))) +
              (cos(threeM)*(0.0020 - (0.0002*T))) +
              (sin(a)*(-0.0337*T + (0.00018*T2))) +
              (cos(a)*(-0.8510 + (0.0044*T) + (0.00068*T2))) +
              (sin(b)*(-0.0064*T + (0.00004*T2))) +
              (cos(b)*(0.2397 - (0.0012*T) - (0.00008*T2))) +
              (sin(c)*-0.0010*T) +
              (cos(c)*(0.1245 + (0.0006*T))) +
              (sin(d)*(0.0024*T - (0.00003*T2))) +
              (cos(d)*(0.0477 - (0.0005*T) - (0.00006*T2)));
    }
  }
  else if (planet == Planet::URANUS)
  {
    if (type == Type::OPPOSITION)
    {
      delta = (0.0844 - (0.0006*T)) +
              (sin(M)*(-0.1048 + (0.0246*T))) +
              (cos(M)*(-5.1221 + (0.0104*T) + (0.00003*T2))) +
              (sin(twoM)*(-0.1428 - (0.0005*T))) +
              (cos(twoM)*(-0.0148 - (0.0013*T))) +
              (cos(threeM)*0.0055) +
              (cos(e)*0.8850) +
              (cos(f)*0.2153);
    }
    else
    {
      assert(type == Type::CONJUNCTION);

      delta = (-0.0859 + (0.0003*T)) +
              (sin(M)*(-3.8179 - (0.0148*T) + (0.00003*T2))) +
              (cos(M)*(5.1228 - (0.0105*T) - (0.00002*T2))) +
              (sin(twoM)*(-0.0803 + (0.0011*T))) +
              (cos(twoM)*(-0.1905 - (0.0006*T))) +
              (sin(threeM)*(0.0088 + (0.0001*T))) +
              (cos(e)*0.8850) +
              (cos(f)*0.2153);
    }
  }
  else
  {
    assert(planet == Planet::NEPTUNE);
    
    if (type == Type::OPPOSITION)
    {
      delta = (-0.0140 + (0.00001*T2)) +
              (sin(M)*(-1.3486 + (0.0010*T) + (0.00001*T2))) +
              (cos(M)*(0.8597 + (0.0037*T))) +
              (sin(twoM)*(-0.0082 - (0.0002*T) + (0.00001*T2))) +
              (cos(twoM)*(0.0037 - (0.0003*T))) +
              (cos(e)*-0.5964) +
              (cos(g)*0.0728);
    }
    else
    {
      assert(type == Type::CONJUNCTION);

      delta = 0.0168 +
              (sin(M)*(-2.5606 + (0.0088*T) + (0.00002*T2))) +
              (cos(M)*(-0.8611 - (0.0037*T) + (0.00002*T2))) +
              (sin(twoM)*(0.0118 - (0.0004*T) + (0.00001*T2))) +
              (cos(twoM)*(0.0307 - (0.0003*T))) +
              (cos(e)*-0.5964) +
              (cos(g)*0.0728);
    }
  }

  return JDE0 + delta;
}

double CAAPlanetaryPhenomena::ElongationValue(double k, Planet planet, bool bEastern) noexcept
{
  const double JDE0{Mean(k, planet, Type::INFERIOR_CONJUNCTION)};

  assert(planet < Planet::MARS);

  const size_t nCoefficient{static_cast<size_t>(planet)*2};
  assert(nCoefficient < g_PlanetaryPhenomenaCoefficient1.size());
#ifdef _MSC_VER
  #pragma warning(suppress : 26446 26482)
#endif //#ifdef _MSC_VER
  double M{CAACoordinateTransformation::MapTo0To360Range(g_PlanetaryPhenomenaCoefficient1[nCoefficient].M0 + (g_PlanetaryPhenomenaCoefficient1[nCoefficient].M1*k))};
  M = CAACoordinateTransformation::DegreesToRadians(M); //convert M to radians
  const double twoM{2*M};
  const double threeM{3*M};
  const double fourM{4*M};
  const double fiveM{5*M};

  const double T{(JDE0 - 2451545)/36525};
  const double T2{T*T};

  double value{0};
  if (planet == Planet::MERCURY)
  {
    if (bEastern)
    {
      value = 22.4697 +
              (sin(M)*(-4.2666 + (0.0054*T) + (0.00002*T2))) +
              (cos(M)*(-1.8537 - (0.0137*T))) +
              (sin(twoM)*(0.3598 + (0.0008*T) - (0.00001*T2))) +
              (cos(twoM)*(-0.0680 + (0.0026*T))) +
              (sin(threeM)*(-0.0524 - (0.0003*T))) +
              (cos(threeM)*(0.0052 - (0.0006*T))) +
              (sin(fourM)*(0.0107 + (0.0001*T))) +
              (cos(fourM)*(-0.0013 + (0.0001*T))) +
              (sin(fiveM)*-0.0021) +
              (cos(fiveM)*0.0003);
    }
    else
    {
      value = (22.4143 - (0.0001*T)) +
              (sin(M)*(4.3651 - (0.0048*T) - (0.00002*T2))) +
              (cos(M)*(2.3787 + (0.0121*T) - (0.00001*T2))) +
              (sin(twoM)*(0.2674 + (0.0022*T))) +
              (cos(twoM)*(-0.3873 + (0.0008*T) + (0.00001*T2))) +
              (sin(threeM)*(-0.0369 - (0.0001*T))) +
              (cos(threeM)*(0.0017 - (0.0001*T))) +
              (sin(fourM)*0.0059) +
              (cos(fourM)*(0.0061 + (0.0001*T))) +
              (sin(fiveM)*0.0007) +
              (cos(fiveM)*-0.0011);
    }
  }
  else if (planet == Planet::VENUS)
  {
    if (bEastern)
    {
      value = (46.3173 + (0.0001*T)) +
              (sin(M)*(0.6916 - (0.0024*T))) +
              (cos(M)*(0.6676 - (0.0045*T))) +
              (sin(twoM)*(0.0309 - (0.0002*T))) +
              (cos(twoM)*(0.0036 - (0.0001*T)));
    }
    else
    {
      value = 46.3245 +
              (sin(M)*(-0.5366 - (0.0003*T) + (0.00001*T2))) +
              (cos(M)*(0.3097 + (0.0016*T) - (0.00001*T2))) +
              (sin(twoM)*-0.0163) +
              (cos(twoM)*(-0.0075 + (0.0001*T)));
    }
  }

  return value;
}
