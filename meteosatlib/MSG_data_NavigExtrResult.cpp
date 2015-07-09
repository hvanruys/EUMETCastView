//-----------------------------------------------------------------------------
//
//  File        : MSG_data_NavigExtrResult.cpp
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
#include "MSG_data_NavigExtrResult.h"

std::string MSG_horizon_id(t_enum_MSG_horizon_id id)
{
  std::string v;
  switch (id)
  {
    case MSG_HORIZON_ID_SOUTH:
      v = "South";
      break;
    case MSG_HORIZON_ID_NORTH:
      v = "North";
      break;
    case MSG_HORIZON_ID_EAST:
      v = "East";
      break;
    case MSG_HORIZON_ID_WEST:
      v = "West";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

size_t MSG_HorizonObservation::read_from( unsigned const char_1 *buff )
{
  HorizonId = (t_enum_MSG_horizon_id) *buff;
  Alpha = get_r8(buff+1);
  AlphaConfidence = get_r8(buff+9);
  Beta = get_r8(buff+17);
  BetaConfidence = get_r8(buff+25);
  (void) ObservationTime.read_from(buff+33);
  SpinRate = get_r8(buff+41);
  AlphaDeviation = get_r8(buff+49);
  BetaDeviation = get_r8(buff+57);
  return 65;
}

std::ostream& operator<< ( std::ostream& os, MSG_HorizonObservation &o )
{
  os << "Horizon Id          : " << MSG_horizon_id(o.HorizonId) << std::endl
     << "Alpha               : " << o.Alpha << std::endl
     << "Alpha Confidence    : " << o.AlphaConfidence << std::endl
     << "Beta                : " << o.Beta << std::endl
     << "Beta Confidence     : " << o.BetaConfidence << std::endl
     << "Observation Time    : " << o.ObservationTime.get_timestring( )
     << std::endl
     << "Spin Rate           : " << o.SpinRate << std::endl
     << "Alpha Deviation     : " << o.AlphaDeviation << std::endl
     << "Beta Deviation      : " << o.BetaDeviation << std::endl;
  return os;
}

size_t MSG_StarObservation::read_from( unsigned const char_1 *buff )
{
  StarId = get_ui2(buff);
  Alpha = get_r8(buff+2);
  AlphaConfidence = get_r8(buff+10);
  Beta = get_r8(buff+18);
  BetaConfidence = get_r8(buff+26);
  (void) ObservationTime.read_from(buff+34);
  SpinRate = get_r8(buff+42);
  AlphaDeviation = get_r8(buff+50);
  BetaDeviation = get_r8(buff+58);
  return 66;
}

std::ostream& operator<< ( std::ostream& os, MSG_StarObservation &o )
{
  if (o.StarId == 0) return os;
  os << "Star Id             : " << o.StarId << std::endl
     << "Alpha               : " << o.Alpha << std::endl
     << "Alpha Confidence    : " << o.AlphaConfidence << std::endl
     << "Beta                : " << o.Beta << std::endl
     << "Beta Confidence     : " << o.BetaConfidence << std::endl
     << "Observation Time    : " << o.ObservationTime.get_timestring( )
     << std::endl
     << "Spin Rate           : " << o.SpinRate << std::endl
     << "Alpha Deviation     : " << o.AlphaDeviation << std::endl
     << "Beta Deviation      : " << o.BetaDeviation << std::endl;
  return os;
}

size_t MSG_LandmarkObservation::read_from( unsigned const char_1 *buff )
{
  LandmarkId = get_ui2(buff);
  LandmarkLongitude = get_r8(buff+2);
  LandmarkLatitude = get_r8(buff+10);
  Alpha = get_r8(buff+18);
  AlphaConfidence = get_r8(buff+26);
  Beta = get_r8(buff+34);
  BetaConfidence = get_r8(buff+42);
  (void) ObservationTime.read_from(buff+50);
  SpinRate = get_r8(buff+60);
  AlphaDeviation = get_r8(buff+66);
  BetaDeviation = get_r8(buff+74);
  return 82;
}

std::ostream& operator<< ( std::ostream& os, MSG_LandmarkObservation &o )
{
  if (o.LandmarkId == 0) return os;
  os << "Landmark Id         : " << o.LandmarkId << std::endl
     << "Longitude           : " << o.LandmarkLongitude << std::endl
     << "Latitude            : " << o.LandmarkLatitude << std::endl
     << "Alpha               : " << o.Alpha << std::endl
     << "Alpha Confidence    : " << o.AlphaConfidence << std::endl
     << "Beta                : " << o.Beta << std::endl
     << "Beta Confidence     : " << o.BetaConfidence << std::endl
     << "Observation Time    : " << o.ObservationTime.get_timestring( )
     << std::endl
     << "Spin Rate           : " << o.SpinRate << std::endl
     << "Alpha Deviation     : " << o.AlphaDeviation << std::endl
     << "Beta Deviation      : " << o.BetaDeviation << std::endl;
  return os;
}

MSG_data_NavigExtrResult::MSG_data_NavigExtrResult( ) { }

MSG_data_NavigExtrResult::MSG_data_NavigExtrResult(unsigned const char_1 *buff)
{
  this->read_from(buff);
}

MSG_data_NavigExtrResult::~MSG_data_NavigExtrResult( ) { }

size_t MSG_data_NavigExtrResult::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;

  for (int i = 0; i < 4; i ++)
    position += ExtractedHorizons[i].read_from(buff+position);
  for (int i = 0; i < 20; i ++)
    position += ExtractedStars[i].read_from(buff+position);
  for (int i = 0; i < 50; i ++)
    position += ExtractedLandmarks[i].read_from(buff+position);
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_NavigExtrResult &r )
{
  os << "------------------------------------------------------" << std::endl
     << "-         MSG NAVIGATION EXTRACTION RESULT           -" << std::endl
     << "------------------------------------------------------" << std::endl;
  for (int i = 0; i < 4; i ++)
    os << r.ExtractedHorizons[i];
  for (int i = 0; i < 20; i ++)
    os << r.ExtractedStars[i];
  for (int i = 0; i < 50; i ++)
    os << r.ExtractedLandmarks[i];
  return os;
}
