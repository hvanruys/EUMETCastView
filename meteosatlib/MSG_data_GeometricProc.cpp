//-----------------------------------------------------------------------------
//
//  File        : MSG_data_GeometricProc.cpp
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
#include "MSG_data_GeometricProc.h"

std::string MSG_type_of_earth_model(t_enum_MSG_type_of_earth_model mdl)
{
  std::string v;
  switch (mdl)
  {
    case MSG_TYPE_OF_EARTH_ELLIPSOID_WITH_RPN_RPS_REQ:
      v = "Ellipsoid with RPN RPS REQ";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

std::string MSG_resampling_function(t_enum_MSG_resampling_function fnc)
{
  std::string v;
  switch (fnc)
  {
    case MSG_RESAMPLING_FUNCTION_WINDOWED_SHANNON:
      v = "Windowed Shannon";
      break;
    case MSG_RESAMPLING_FUNCTION_BICUBIC_SPLINES:
      v = "Bicubic Splines";
      break;
    case MSG_RESAMPLING_FUNCTION_NEAREST_NEIGHBOUR:
      v = "Nearest neighbour";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

size_t MSG_OptAxisDistances::read_from(unsigned const char_1 *buff )
{
  for (int i = 0; i < 42; i ++)
    E_W_FocalPlane[i] = get_r4(buff+i*4);
  for (int i = 0; i < 42; i ++)
    N_S_FocalPlane[i] = get_r4(buff+168+i*4);
  return 336;
}

std::ostream& operator<< (std::ostream& os, MSG_OptAxisDistances &d)
{
  for (int i = 0; i < 42; i ++)
    os << "E/W Focal plane " << std::setw(2) << std::setfill('0') << i+1
       << "   : " << d.E_W_FocalPlane[i] << std::endl;
  for (int i = 0; i < 42; i ++)
    os << "N/S Focal plane " << std::setw(2) << std::setfill('0') << i+1
       << "   : " << d.N_S_FocalPlane[i] << std::endl;
  return os;
}

size_t MSG_EarthModel::read_from( unsigned const char_1 *buff )
{
  TypeofEarthModel = (t_enum_MSG_type_of_earth_model) *(buff);
  EquatorialRadius = get_r8(buff+1);
  NorthPolarRadius = get_r8(buff+9);
  SouthPolarRadius = get_r8(buff+17);
  return 25;
}

std::ostream& operator<< (std::ostream& os, MSG_EarthModel &e)
{
  os << "Type of Earth Model : " << MSG_type_of_earth_model(e.TypeofEarthModel)
     << std::endl
     << "Equatorial Radius   : " << e.EquatorialRadius << " Km" << std::endl
     << "North Polar Radius  : " << e.NorthPolarRadius << " Km" << std::endl
     << "South Polar Radius  : " << e.SouthPolarRadius << " Km" << std::endl;
  return os;
}

MSG_data_GeometricProc::MSG_data_GeometricProc( ) { }

MSG_data_GeometricProc::MSG_data_GeometricProc( unsigned const char_1 *buff )
{
  this->read_from(buff);
}

MSG_data_GeometricProc::~MSG_data_GeometricProc( ) { }

size_t MSG_data_GeometricProc::read_from( unsigned const char_1 *buff)
{
  size_t position = 0;
  position += OptAxisDistances.read_from(buff+position);
  position += EarthModel.read_from(buff+position);
  for (int i = 0; i < 12; i ++)
    for (int j = 0; j < 360; j ++)
      AtmosphericModel[i][j] = get_r4(buff+position+(i*360+j)*4);
  position += 12*360*4;
  for (int i = 0; i < 12; i ++)
    ResamplingFunction[i] = (t_enum_MSG_resampling_function) *(buff+position);
  position += 12;
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_GeometricProc &g )
{
  os << "------------------------------------------------------" << std::endl
     << "-           MSG GEOMETRIC PROCESSING DATA            -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << g.OptAxisDistances
     << g.EarthModel;
  for (int i = 0; i < 12; i ++)
  {
    os << "Atmospheric Model CHANNEL " << std::setw(2) << std::setfill('0')
       << i+1 << std::endl;
    for (int j = 0; j < 360; j += 4)
    {
      os << std::setw(12) << std::setfill(' ')
         << g.AtmosphericModel[i][j]   << " "
         << std::setw(12) << std::setfill(' ')
         << g.AtmosphericModel[i][j+1] << " "
         << std::setw(12) << std::setfill(' ')
         << g.AtmosphericModel[i][j+2] << " "
         << std::setw(12) << std::setfill(' ')
         << g.AtmosphericModel[i][j+3]
         << std::endl;
    }
  }
  for (int i = 0; i < 12; i ++)
  {
    os << "Resampl Function " << std::setw(2) << std::setfill('0')
       << i+1 << " : "
       << MSG_resampling_function(g.ResamplingFunction[i]) << std::endl; 
  }
  return os;
}
