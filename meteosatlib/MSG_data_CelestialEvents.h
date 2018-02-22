//-----------------------------------------------------------------------------
//
//  File        : MSG_data_CelestialEvents.h
//  Description : MSG HRIT-LRIT format interface
//  Project     : Meteosatlib
//  Author      : Graziano Giuliani
//  References  : MSG/SPE/057 LRIT-HRIT Mission Specific Implementation,
//                V. 4.1 9 Mar 2001
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//-----------------------------------------------------------------------------

#ifndef __MSG_DATA_CELESTIALEVENTS_H__
#define __MSG_DATA_CELESTIALEVENTS_H__

#include <iostream>
#include "MSG_machine.h"
#include "MSG_time_cds.h"

#define MSG_CELESTIAL_EVENTS_LEN          326058

typedef enum {
  MSG_ECLYPSE_NONE = 0,
  MSG_ECLYPSE_SUN  = 1,
  MSG_ECLYPSE_MOON = 2
} t_enum_MSG_Eclypse;

std::string MSG_Eclypse(t_enum_MSG_Eclypse ecl);

typedef enum {
  MSG_BODY_NONE         = 0,
  MSG_BODY_SUN          = 1,
  MSG_BODY_MOON         = 2,
  MSG_BODY_SUN_AND_MOON = 3
} t_enum_MSG_Body;

std::string MSG_Body(t_enum_MSG_Body body);

typedef enum {
  MSG_QUALITY_IMPACT_NONE        = 0,
  MSG_QUALITY_IMPACT_RADIOMETRIC = 1,
  MSG_QUALITY_IMPACT_GEOMETRIC   = 2
} t_enum_MSG_quality_impact;

std::string MSG_quality_impact(t_enum_MSG_quality_impact imp);

class MSG_Star_Coefficient {
  public:
    size_t read_from(unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_Star_Coefficient &i );
    uint_2             StarId[20];
    MSG_time_cds_short StartTime[20];
    MSG_time_cds_short EndTime[20];
    real_8             AlphaCoef[20][8];
    real_8             BetaCoef[20][8];
};

class MSG_Earth_Moon_Sun_Coefficient {
  public:
    size_t read_from(unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_Earth_Moon_Sun_Coefficient &i );
    MSG_time_cds_short StartTime;
    MSG_time_cds_short EndTime;
    real_8             AlphaCoef[8];
    real_8             BetaCoef[8];
};

class MSG_Ephemeris {
  public:
    size_t read_from(unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os, MSG_Ephemeris &e );
    MSG_time_cds_short             PeriodStartTime;
    MSG_time_cds_short             PeriodEndTime;
    MSG_time_generalized           RelatedOrbitFileTime;
    MSG_time_generalized           RelatedAttitudeFileTime;
    MSG_Earth_Moon_Sun_Coefficient EarthEphemeris[100];
    MSG_Earth_Moon_Sun_Coefficient MoonEphemeris[100];
    MSG_Earth_Moon_Sun_Coefficient SunEphemeris[100];
    MSG_Star_Coefficient           StarEphemeris[100];
};

class MSG_CelestialBodiesPosition {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_CelestialBodiesPosition &p );
    MSG_Ephemeris             Ephemeris;
};

class MSG_RelationToImage {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_RelationToImage &r );
    t_enum_MSG_Eclypse        TypeofEclypse;
    MSG_time_cds_short        EclipseStartTime;
    MSG_time_cds_short        EclipseEndTime;
    t_enum_MSG_Body           VisibleBodiesInImage;
    t_enum_MSG_Body           BodiesClosesttoFOV;
    t_enum_MSG_quality_impact ImpactOnImageQuality;
};

class MSG_data_CelestialEvents {
  public:
    MSG_data_CelestialEvents( );
    MSG_data_CelestialEvents( unsigned const char_1 *buff );
    ~MSG_data_CelestialEvents( );

    size_t read_from( unsigned const char_1 *buff );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_data_CelestialEvents &g );

    MSG_CelestialBodiesPosition CelestialBodiesPosition;
    MSG_RelationToImage         RelationToImage;
};

#endif
