//-----------------------------------------------------------------------------
//
//  File        : MSG_data_ImageDescription.cpp
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
#include "MSG_data_ImageDescription.h"

std::string MSG_projection_type(t_enum_MSG_projection_type type)
{
  std::string v;
  switch (type)
  {
    case MSG_PROJECTION_TYPE_GEOSTATIONARY:
      v = "Geostationary, Earth Centered in grid";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

std::string MSG_grid_origin(t_enum_MSG_grid_origin org)
{
  std::string v;
  switch (org)
  {
    case MSG_ORIGIN_CORNER_NW:
      v = "Grid corner North-West";
      break;
    case MSG_ORIGIN_CORNER_SW:
      v = "Grid corner South-West";
      break;
    case MSG_ORIGIN_CORNER_SE:
      v = "Grid corner South-East";
      break;
    case MSG_ORIGIN_CORNER_NE:
      v = "Grid corner North-East";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

std::string MSG_direction_line(t_enum_MSG_direction_line dir)
{
  std::string v;
  switch (dir)
  {
    case MSG_DIRECTION_LINE_NS:
      v = "Direction North-South";
      break;
    case MSG_DIRECTION_LINE_SN:
      v = "Direction South-North";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

std::string MSG_direction_column(t_enum_MSG_direction_column dir)
{
  std::string v;
  switch (dir)
  {
    case MSG_DIRECTION_COLUMN_EW:
      v = "Direction East-West";
      break;
    case MSG_DIRECTION_COLUMN_WE:
      v = "Direction West-East";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

size_t MSG_grid::read_from( unsigned const char_1 *buff)
{
  NumberofLines = get_i4(buff);
  NumberofColumns = get_i4(buff+4);
  LineDirGridStep = get_r4(buff+8);
  ColumnDirGridStep = get_r4(buff+12);
  GridOrigin = (t_enum_MSG_grid_origin) *(buff+16);
  return 17;
}

std::ostream& operator<< ( std::ostream& os, MSG_grid &g )
{
  os << "Number Of Lines     : " << g.NumberofLines << std::endl
     << "Number of Columns   : " << g.NumberofColumns << std::endl
     << "Grid Step SSP line  : " << g.LineDirGridStep << std::endl
     << "Grid Step SSP col.  : " << g.ColumnDirGridStep << std::endl
     << "Grid Origin         : " << MSG_grid_origin(g.GridOrigin) << std::endl;
  return os;
}

size_t MSG_coverage_IR::read_from( unsigned const char_1 *buff )
{
  SouthernLinePlanned = get_i4(buff);
  NorthernLinePlanned = get_i4(buff+4);
  EasternColumnPlanned = get_i4(buff+8);
  WesternColumnPlanned = get_i4(buff+12);
  return 16;
}

std::ostream& operator<< ( std::ostream& os, MSG_coverage_IR &c )
{
  os << "Southern line Plan. : " << c.SouthernLinePlanned << std::endl
     << "Northern line Plan. : " << c.NorthernLinePlanned << std::endl
     << "Eastern col. Plan.  : " << c.EasternColumnPlanned << std::endl
     << "Western col. Plan.  : " << c.WesternColumnPlanned << std::endl;
  return os;
}

size_t MSG_coverage_HRV::read_from( unsigned const char_1 *buff )
{
  LowerSouthLinePlanned = get_i4(buff);
  LowerNorthLinePlanned = get_i4(buff+4);
  LowerEastColumnPlanned = get_i4(buff+8);
  LowerWestColumnPlanned = get_i4(buff+12);
  UpperSouthLinePlanned = get_i4(buff+16);
  UpperNorthLinePlanned = get_i4(buff+20);
  UpperEastColumnPlanned = get_i4(buff+24);
  UpperWestColumnPlanned = get_i4(buff+28);
  return 32;
}

std::ostream& operator<< ( std::ostream& os, MSG_coverage_HRV &c )
{
  os << "Low South line Plan.: " << c.LowerSouthLinePlanned << std::endl
     << "Low North line Plan.: " << c.LowerNorthLinePlanned << std::endl
     << "Low East col. Plan. : " << c.LowerEastColumnPlanned << std::endl
     << "Low West col. Plan. : " << c.LowerWestColumnPlanned << std::endl
     << "Up South line Plan. : " << c.UpperSouthLinePlanned << std::endl
     << "Up North line Plan. : " << c.UpperNorthLinePlanned << std::endl
     << "Up East col. Plan.  : " << c.UpperEastColumnPlanned << std::endl
     << "Up West col. Plan.  : " << c.UpperWestColumnPlanned << std::endl;
  return os;
}

size_t MSG_ProjectionDescription::read_from( unsigned const char_1 *buff )
{
  TypeOfProjection = (t_enum_MSG_projection_type) *(buff);
  LongitudeOfSSP = get_r4(buff+1);
  return 5;
}

std::ostream& operator<< ( std::ostream& os, MSG_ProjectionDescription &d )
{
  os << "Type of Projection  : " << MSG_projection_type(d.TypeOfProjection)
     << std::endl
     << "SSP Longitude       : " << d.LongitudeOfSSP << std::endl;
  return os;
}

size_t MSG_Level1_5ImageProduction::read_from( unsigned const char_1 *buff )
{
  ImageProcDirection = (t_enum_MSG_direction_line) *(buff);
  PixelGenDirection = (t_enum_MSG_direction_column) *(buff+1);
  for (int i = 0; i < 12; i ++)
    PlannedChanProcessing[i] = *(buff+2+i) ? true : false;
  return 14;
}

std::ostream& operator<< ( std::ostream& os, MSG_Level1_5ImageProduction &p )
{
  os << "Image Processing Dir: " << MSG_direction_line(p.ImageProcDirection)
     << std::endl
     << "Pixel Processing Dir: " << MSG_direction_column(p.PixelGenDirection)
     << std::endl;
  for (int i = 0; i < 12; i ++)
  {
    os << "Channel " << std::setw(2) << std::setfill('0') << i+1
       << " is plan. : " << p.PlannedChanProcessing[i] << std::endl;
  }
  return os;
}

MSG_data_ImageDescription::MSG_data_ImageDescription( ) { }

MSG_data_ImageDescription::MSG_data_ImageDescription(
                                 unsigned const char_1 *buff )
{
  this->read_from(buff);
}

MSG_data_ImageDescription::~MSG_data_ImageDescription( ) { }

size_t MSG_data_ImageDescription::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  position += ProjectionDescription.read_from(buff+position);
  position += ReferenceGridVIS_IR.read_from(buff+position);
  position += ReferenceGridHRV.read_from(buff+position);
  position += PlannedCoverageVIS_IR.read_from(buff+position);
  position += PlannedCoverageHRV.read_from(buff+position);
  position += Level1_5ImageProduction.read_from(buff+position);
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_ImageDescription &d )
{
  os << "------------------------------------------------------" << std::endl
     << "-           MSG IMAGE DESCRIPTION RECORD             -" << std::endl
     << "------------------------------------------------------" << std::endl
     << d.ProjectionDescription
     << "VIS-IR REFERENCE GRID" << std::endl << d.ReferenceGridVIS_IR
     << "HRV REFERENCE GRID" << std::endl << d.ReferenceGridHRV
     << "VIS-IR PLANNED COVERAGE" << std::endl << d.PlannedCoverageVIS_IR
     << "HRV PLANNED COVERAGE" << std::endl << d.PlannedCoverageHRV
     << d.Level1_5ImageProduction;
  return os;
}
