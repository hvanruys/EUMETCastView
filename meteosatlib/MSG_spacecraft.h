//-----------------------------------------------------------------------------
//
//  File        : MSG_spacecraft.h
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

#ifndef __MSG_SPACECRAFT_H__
#define __MSG_SPACECRAFT_H__

#include <string>

typedef enum {
  MSG_NO_SPACECRAFT = 0,
  MSG_METOP_1       = 11,
  MSG_METOP_2       = 12,
  MSG_METOP_3       = 13,
  MSG_METEOSAT_3    = 16,
  MSG_METEOSAT_4    = 19,
  MSG_METEOSAT_5    = 20,
  MSG_METEOSAT_6    = 21,
  MSG_MTP_1         = 150,
  MSG_MTP_2         = 151,
  MSG_MSG_1         = 321,
  MSG_MSG_2         = 322,
  MSG_MSG_3         = 323,
  MSG_MSG_4         = 324,
  MSG_NOAA_12       = 17012,
  MSG_NOAA_13       = 17013,
  MSG_NOAA_14       = 17014,
  MSG_NOAA_15       = 17015,
  MSG_GOES_7        = 18007,
  MSG_GOES_8        = 18008,
  MSG_GOES_9        = 18009,
  MSG_GOES_10       = 18010,
  MSG_GOES_11       = 18011,
  MSG_GOES_12       = 18012,
  MSG_GOMS_1        = 19001,
  MSG_GOMS_2        = 19002,
  MSG_GOMS_3        = 19003,
  MSG_GMS_4         = 20004,
  MSG_GMS_5         = 20005,
  MSG_GMS_6         = 20006,
  MSG_MTSAT_1       = 21001,
  MSG_MTSAT_2       = 21002,
  MSG_HIMAWARI_8    = 21003,

  MSG_UNDEFINED_SPACECRAFT = 21004
} t_enum_MSG_spacecraft;

std::string MSG_spacecraft_name( t_enum_MSG_spacecraft id );

#endif
