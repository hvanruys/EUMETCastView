//-----------------------------------------------------------------------------
//
//  File        : MSG_data_CelestialEvents.cpp
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

#include <fstream>
#include <cstring>
#include <iomanip>
#include <string>
#include "MSG_data_CelestialEvents.h"

size_t MSG_CelestialBodiesPosition::read_from(unsigned const char_1 *buff)
{
  return Ephemeris.read_from(buff);
}

std::ostream& operator<< ( std::ostream& os, MSG_CelestialBodiesPosition &p )
{
  os << p.Ephemeris;
  return os;
}

size_t MSG_RelationToImage::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  TypeofEclypse = (t_enum_MSG_Eclypse) *(buff+position);
  position ++;
  EclipseStartTime.read_from(buff+position);
  position += MSG_TIME_CDS_LEN;
  EclipseEndTime.read_from(buff+position);
  position += MSG_TIME_CDS_LEN;
  VisibleBodiesInImage = (t_enum_MSG_Body) *(buff+position);
  position ++;
  BodiesClosesttoFOV = (t_enum_MSG_Body) *(buff+position);
  position ++;
  ImpactOnImageQuality = (t_enum_MSG_quality_impact) *(buff+position);
  position ++;
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_RelationToImage &r )
{
  os << "Type of Eclypse     : " << MSG_Eclypse(r.TypeofEclypse) << std::endl
     << "Eclypse Start       : " << r.EclipseStartTime.get_timestring( )
     << std::endl
     << "Eclypse End         : " << r.EclipseEndTime.get_timestring( )
     << std::endl
     << "Visible Body        : " << MSG_Body(r.VisibleBodiesInImage)
     << std::endl
     << "Body Closest to FOV : " << MSG_Body(r.BodiesClosesttoFOV)
     << std::endl
     << "Image Qualuty Impact: " << MSG_quality_impact(r.ImpactOnImageQuality)
     << std::endl;
  return os;
}

MSG_data_CelestialEvents::MSG_data_CelestialEvents( ) { }

MSG_data_CelestialEvents::MSG_data_CelestialEvents(unsigned const char_1 *buff)
{
  this->read_from(buff);
}

MSG_data_CelestialEvents::~MSG_data_CelestialEvents( ) { }

size_t MSG_data_CelestialEvents::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  position += CelestialBodiesPosition.read_from(buff+position);
  position += RelationToImage.read_from(buff+position);
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_CelestialEvents &c )
{
  os << "------------------------------------------------------" << std::endl
     << "-           MSG CELESTIAL EVENTS RECORD              -" << std::endl
     << "------------------------------------------------------" << std::endl
     << c.CelestialBodiesPosition
     << c.RelationToImage;
  return os;
}

std::string MSG_Eclypse(t_enum_MSG_Eclypse ecl)
{
  std::string v;
  switch (ecl)
  {
    case MSG_ECLYPSE_NONE:
      v = "None";
      break;
    case MSG_ECLYPSE_SUN:
      v = "Sun";
      break;
    case MSG_ECLYPSE_MOON:
      v = "Moon";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

std::string MSG_Body(t_enum_MSG_Body body)
{
  std::string v;
  switch (body)
  {
    case MSG_BODY_NONE:
      v = "None";
      break;
    case MSG_BODY_SUN:
      v = "Sun";
      break;
    case MSG_BODY_MOON:
      v = "Moon";
      break;
    case MSG_BODY_SUN_AND_MOON:
      v = "Sun and Moon";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

std::string MSG_quality_impact(t_enum_MSG_quality_impact imp)
{
  std::string v;
  switch (imp)
  {
    case MSG_QUALITY_IMPACT_NONE:
      v = "None";
      break;
    case MSG_QUALITY_IMPACT_RADIOMETRIC:
      v = "Radiometric";
      break;
    case MSG_QUALITY_IMPACT_GEOMETRIC:
      v = "Geometric";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

size_t MSG_Star_Coefficient::read_from(unsigned const char_1 *buff )
{
  for (int i = 0; i < 20; i ++)
  {
    StarId[i] = get_ui2(buff+142*i);
    StartTime[i].read_from(buff+142*i+2);
    EndTime[i].read_from(buff+142*i+8);
    for (int j = 0; j < 8; j ++)
      AlphaCoef[i][j] = get_r8(buff+142*i+14+j*8);
    for (int j = 0; j < 8; j ++)
      BetaCoef[i][j] = get_r8(buff+142*i+78+j*8);
  }
  return 2840;
}

std::ostream& operator<< ( std::ostream& os, MSG_Star_Coefficient &s )
{
  for (int i = 0; i < 20; i ++)
  {
    if (s.StartTime[i].get_day_from_epoch( ) == 0) continue;
    os << "********** " << i+1 << std::endl
       << "Star Id             : " << s.StarId[i] << std::endl
       << "Start Time          : " << s.StartTime[i].get_timestring( )
       << std::endl
       << "End Time            : " << s.EndTime[i].get_timestring( )
       << std::endl;
    for (int j = 0; j < 8; j ++)
    {
      os << "ALPHA : " << std::setw(12) << std::setfill(' ')
       << s.AlphaCoef[i][j] << " "
       << " BETA : " << std::setw(12) << std::setfill(' ')
       << s.BetaCoef[i][j] << std::endl;
    }
  } 

  return os;
}

size_t MSG_Earth_Moon_Sun_Coefficient::read_from(unsigned const char_1 *buff )
{
  StartTime.read_from(buff);
  EndTime.read_from(buff+6);
  for (int i = 0; i < 8; i ++)
    AlphaCoef[i] = get_r8(buff+12+i*8);
  for (int i = 0; i < 8; i ++)
    BetaCoef[i] = get_r8(buff+76+i*8);
  return 140;
}

std::ostream& operator<< ( std::ostream& os, MSG_Earth_Moon_Sun_Coefficient &m )
{
  if (m.StartTime.get_day_from_epoch( ) == 0) return os;;
  os << "COEFFICIENT" << std::endl
     << "Start Time          : " << m.StartTime.get_timestring( ) << std::endl
     << "End Time            : " << m.EndTime.get_timestring( ) << std::endl;
  for (int i = 0; i < 8; i ++)
  {
    os << "ALPHA : " << std::setw(12) << std::setfill(' ')
       << m.AlphaCoef[i] << " "
       << " BETA : " << std::setw(12) << std::setfill(' ')
       << m.BetaCoef[i] << std::endl;
  }

  return os;
}

size_t MSG_Ephemeris::read_from(const unsigned char_1 *buff)
{
  size_t position = 0;

  PeriodStartTime.read_from(buff);
  position += MSG_TIME_CDS_LEN;
  PeriodEndTime.read_from(buff+6);
  position += MSG_TIME_CDS_LEN;
  position += RelatedOrbitFileTime.read_from(buff+12);
  position += RelatedAttitudeFileTime.read_from(buff+27);
  for (int i = 0; i < 100; i ++)
    position += EarthEphemeris[i].read_from(buff+position);
  for (int i = 0; i < 100; i ++)
    position += MoonEphemeris[i].read_from(buff+position);
  for (int i = 0; i < 100; i ++)
    position += SunEphemeris[i].read_from(buff+position);
  for (int i = 0; i < 100; i ++)
    position += StarEphemeris[i].read_from(buff+position);
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_Ephemeris &e )
{
  os << "EPHEMERIS" << std::endl
     << "Period Start Time   : " << e.PeriodStartTime.get_timestring( )
     << std::endl
     << "Period End Time     : " << e.PeriodEndTime.get_timestring( )
     << std::endl
     << "Rel. Orbitfile Time : " << e.RelatedOrbitFileTime.get_timestring( )
     << std::endl
     << "Rel. Attitude Time  : " << e.RelatedAttitudeFileTime.get_timestring( )
     << std::endl;
  os << "EARTH EPHEMERIS" << std::endl;
  for (int i = 0; i < 100; i ++)
     os << e.EarthEphemeris[i];
  os << "MOON EPHEMERIS" << std::endl;
  for (int i = 0; i < 100; i ++)
     os << e.MoonEphemeris[i];
  os << "SUN EPHEMERIS" << std::endl;
  for (int i = 0; i < 100; i ++)
     os << e.SunEphemeris[i];
  os << "STAR EPHEMERIS" << std::endl;
  for (int i = 0; i < 100; i ++)
     os << e.StarEphemeris[i];
  os << "END EPHEMERIS" << std::endl;
  return os;
}
