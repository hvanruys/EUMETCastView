//-----------------------------------------------------------------------------
//
//  File        : MSG_compression.h
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

#ifndef __MSG_COMPRESSION_H__
#define __MSG_COMPRESSION_H__

#include <string>

typedef enum {
  MSG_NO_COMPRESSION       = 0,
  MSG_LOSSLESS_COMPRESSION = 1,
  MSG_LOSSY_COMPRESSION    = 2
} t_enum_MSG_image_compression;

std::string MSG_image_compression(t_enum_MSG_image_compression
                                  compression_flag);
#endif
