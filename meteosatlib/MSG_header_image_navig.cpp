//-----------------------------------------------------------------------------
//
//  File        : MSG_header_image_navig.cpp
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

#include <cstring>
#include <cstdio>

#include "MSG_header_image_navig.h"

MSG_header_image_navig::MSG_header_image_navig( )
{
  satellite_h = 42164.0;
}

MSG_header_image_navig::MSG_header_image_navig( unsigned const char_1 *buff )
{
  read_from(buff);
}

MSG_header_image_navig::~MSG_header_image_navig( ) { }

void MSG_header_image_navig::read_from( unsigned const char_1 *buff )
{
  char_1 projection_name[32];
  memcpy(projection_name, buff+3, 32);
  column_scaling_factor = get_i4(buff+35);
  CFAC                  = column_scaling_factor;
  line_scaling_factor   = get_i4(buff+39);
  LFAC                  = line_scaling_factor;
  column_offset         = get_i4(buff+43);
  COFF                  = column_offset;
  line_offset           = get_i4(buff+47);
  LOFF                  = line_offset;
  if (projection_name[0] == 'G')
  {
    projection_name[4] = 0;
    pname = projection_name;
    char_1 *pnt = strchr(projection_name+5, ')');
    if (pnt != NULL) *pnt = 0;
    sscanf(projection_name+5, "%f", &subsatellite_longitude);
  }
  else if (projection_name[0] == 'P' && projection_name[1] == 'O')
  {
    char_1 *tmp;
    projection_name[5] = 0;
    pname = projection_name;
    tmp = strchr(projection_name+6, ',');
    *(tmp) = 0;
    sscanf(projection_name+6, "%f", &projection_plane);
    *(strchr(tmp, ')')) = 0;
    sscanf(tmp, "%f", &projection_longitude);
  }
  else if (projection_name[0] == 'P' && projection_name[1] == 'S')
    pname = "PSD";
  else if (projection_name[0] == 'M')
    pname = "MERCATOR";
  else
    std::cerr << "Unknown Projection in Navigation Header." << std::endl;
  return;
}

t_enum_MSG_projection MSG_header_image_navig::get_projection_code( )
{
  if (pname == "GEOS")
    return MSG_PROJECTION_GEOS;
  else if (pname == "POLAR")
    return MSG_PROJECTION_POLAR;
  else if (pname == "MERCATOR")
    return MSG_PROJECTION_MERCATOR;
  else
    return MSG_UNDEFINED;
}

std::ostream& operator<< ( std::ostream& os, MSG_header_image_navig &h )
{
  os << "------------------------------------------------------" << std::endl
     << "-            MSG IMAGE NAVIGATION HEADER             -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << "Projection Name     : " << h.pname << std::endl;
  switch (h.get_projection_code())
  {
    case MSG_PROJECTION_GEOS:
      os << "Subsatellite Lat.   : " << h.subsatellite_longitude << std::endl;
      break;
    case MSG_PROJECTION_POLAR:
      os << "Projection plane    : " << h.projection_plane << std::endl
         << "Projection long.    : " << h.projection_longitude << std::endl;
      break;
    default:
      break;
  }
  os << "Column scale factor : " << (int_8) h.column_scaling_factor << std::endl
     << "Line scale factor   : " << (int_8) h.line_scaling_factor << std::endl
     << "Column offset       : " << (int_8) h.column_offset << std::endl
     << "Line Offset         : " << (int_8) h.line_offset << std::endl;
  return os;
}
