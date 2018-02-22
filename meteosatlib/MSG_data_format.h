//-----------------------------------------------------------------------------
//
//  File        : MSG_data_format.h
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

#ifndef __MSG_DATA_FORMAT_H__
#define __MSG_DATA_FORMAT_H__

#include <string>

typedef enum {
  MSG_NO_FORMAT      = 0,
  MSG_JPEG_FORMAT    = 1,
  MSG_T4_FORMAT      = 2,
  MSG_WAVELET_FORMAT = 3
} t_enum_MSG_data_format;

std::string MSG_data_format(t_enum_MSG_data_format data_field_format);

#endif
