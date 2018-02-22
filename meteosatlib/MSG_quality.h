//-----------------------------------------------------------------------------
//
//  File        : MSG_quality.h
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

#ifndef __MSG_QUALITY_H__
#define __MSG_QUALITY_H__

#include <string>

typedef enum {
  MSG_SEGMENT_VALIDITY_NOT_DERIVED                            = 0,
  MSG_SEGMENT_VALIDITY_NOMINAL                                = 1,
  MSG_SEGMENT_VALIDITY_BASED_ON_MISSING_DATA                  = 2,
  MSG_SEGMENT_VALIDITY_BASED_ON_CORRUPTED_DATA                = 3,
  MSG_SEGMENT_VALIDITY_BASED_ON_REPLACED_OR_INTERPOLATED_DATA = 4
} t_enum_MSG_segment_validity;

typedef enum {
  MSG_SEGMENT_RADIOMETRIC_QUALITY_NOT_DERIVED = 0,
  MSG_SEGMENT_RADIOMETRIC_QUALITY_NOMINAL     = 1,
  MSG_SEGMENT_RADIOMETRIC_QUALITY_USABLE      = 2,
  MSG_SEGMENT_RADIOMETRIC_QUALITY_SUSPECT     = 3,
  MSG_SEGMENT_RADIOMETRIC_QUALITY_DO_NOT_USE  = 4
} t_enum_MSG_segment_radiometric_quality;

typedef enum {
  MSG_SEGMENT_GEOMETRIC_QUALITY_NOT_DERIVED = 0,
  MSG_SEGMENT_GEOMETRIC_QUALITY_NOMINAL     = 1,
  MSG_SEGMENT_GEOMETRIC_QUALITY_USABLE      = 2,
  MSG_SEGMENT_GEOMETRIC_QUALITY_SUSPECT     = 3,
  MSG_SEGMENT_GEOMETRIC_QUALITY_DO_NOT_USE  = 4
} t_enum_MSG_segment_geometric_quality;

std::string MSG_segment_validity(t_enum_MSG_segment_validity line_validity);
std::string MSG_segment_radiometric_quality(
         t_enum_MSG_segment_radiometric_quality line_radiometric_quality);
std::string MSG_segment_geometric_quality(t_enum_MSG_segment_geometric_quality
                                          line_geometric_quality);

#endif
