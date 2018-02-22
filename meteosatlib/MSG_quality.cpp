//-----------------------------------------------------------------------------
//
//  File        : MSG_quality.cpp
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

#include "MSG_quality.h"

std::string MSG_segment_validity(t_enum_MSG_segment_validity line_validity)
{
  std::string v;
  switch (line_validity)
  {
    case MSG_SEGMENT_VALIDITY_NOT_DERIVED:
      v = "Not derived";
      break;
    case MSG_SEGMENT_VALIDITY_NOMINAL:
      v = "Nominal";
      break;
    case MSG_SEGMENT_VALIDITY_BASED_ON_MISSING_DATA:
      v = "Based on missing data";
      break;
    case MSG_SEGMENT_VALIDITY_BASED_ON_CORRUPTED_DATA:
      v = "Based on corrupted data";
      break;
    case MSG_SEGMENT_VALIDITY_BASED_ON_REPLACED_OR_INTERPOLATED_DATA:
      v = "Based on missing or interpolated data";
      break;
    default:
      v = "Validity not defined";
      break;
  }
  return v;
}

std::string MSG_segment_radiometric_quality(
         t_enum_MSG_segment_radiometric_quality line_radiometric_quality)
{
  std::string v;
  switch (line_radiometric_quality)
  {
    case MSG_SEGMENT_RADIOMETRIC_QUALITY_NOT_DERIVED:
      v = "Not derived";
      break;
    case MSG_SEGMENT_RADIOMETRIC_QUALITY_NOMINAL:
      v = "Nominal";
      break;
    case MSG_SEGMENT_RADIOMETRIC_QUALITY_USABLE:
      v = "Usable";
      break;
    case MSG_SEGMENT_RADIOMETRIC_QUALITY_SUSPECT:
      v = "Suspect";
      break;
    case MSG_SEGMENT_RADIOMETRIC_QUALITY_DO_NOT_USE:
      v = "Do not use";
      break;
    default:
      v = "Radiometric quality undefined";
      break;
  }
  return v;
}

std::string MSG_segment_geometric_quality(t_enum_MSG_segment_geometric_quality
                                          line_geometric_quality)
{
  std::string v;
  switch (line_geometric_quality)
  {
    case MSG_SEGMENT_GEOMETRIC_QUALITY_NOT_DERIVED:
      v = "Not derived";
      break;
    case MSG_SEGMENT_GEOMETRIC_QUALITY_NOMINAL:
      v = "Nominal";
      break;
    case MSG_SEGMENT_GEOMETRIC_QUALITY_USABLE:
      v = "Usable";
      break;
    case MSG_SEGMENT_GEOMETRIC_QUALITY_SUSPECT:
      v = "Suspect";
      break;
    case MSG_SEGMENT_GEOMETRIC_QUALITY_DO_NOT_USE:
      v = "Do not use";
      break;
    default:
      v = "Geometric quality undefined";
      break;
  }
  return v;
}
