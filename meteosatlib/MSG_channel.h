//-----------------------------------------------------------------------------
//
//  File        : MSG_channel.h
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

#ifndef __MSG_CHANNEL_H__
#define __MSG_CHANNEL_H__

#include <string>
#include "MSG_machine.h"
#include "MSG_spacecraft.h"

typedef enum {
  MSG_SEVIRI_NO_CHANNEL  = 0,
  MSG_SEVIRI_1_5_VIS_0_6 = 1,
  MSG_SEVIRI_1_5_VIS_0_8 = 2,
  MSG_SEVIRI_1_5_IR_1_6  = 3,
  MSG_SEVIRI_1_5_IR_3_9  = 4,
  MSG_SEVIRI_1_5_WV_6_2  = 5,
  MSG_SEVIRI_1_5_WV_7_3  = 6,
  MSG_SEVIRI_1_5_IR_8_7  = 7,
  MSG_SEVIRI_1_5_IR_9_7  = 8,
  MSG_SEVIRI_1_5_IR_10_8 = 9,
  MSG_SEVIRI_1_5_IR_12_0 = 10,
  MSG_SEVIRI_1_5_IR_13_4 = 11,
  MSG_SEVIRI_1_5_HRV     = 12,
  MSG_SEVIRI_UNKNOWN     = 13
} t_enum_MSG_seviri_channel;

typedef enum {
  MSG_GOES_NO_CHANNEL    = 0,
  MSG_GOES_VISIBLE       = 1,
  MSG_GOES_IR_3_9        = 2,
  MSG_GOES_WV_6_8        = 3,
  MSG_GOES_IR_10_7       = 4,
  MSG_GOES_IR_12_0       = 5,
  MSG_GOES_UNKNOWN       = 6
} t_enum_MSG_goes_channel;

typedef enum {
  MSG_GMS_MTSAT_NO_CHANNEL     = 0,
  MSG_GMS_MTSAT_VISIBLE        = 1,
  MSG_GMS_MTSAT_IR_3_8         = 2,
  MSG_GMS_MTSAT_WV_6_7         = 3,
  MSG_GMS_MTSAT_IR_10_8        = 4,
  MSG_GMS_MTSAT_IR_12_0        = 5,
  MSG_GMS_MTSAT_UNKNOWN        = 6
} t_enum_MSG_gms_mtsat_channel;

typedef enum {
  MSG_GOMS_NO_CHANNEL    = 0,
  MSG_GOMS_VISIBLE       = 1,
  MSG_GOMS_WATER_VAPOUR  = 2,
  MSG_GOMS_INFRARED      = 3
} t_enum_MSG_goms_channel;

typedef enum {
  MSG_HRPT_NO_CHANNEL  = 0,
  MSG_HRPT_VIS_0_6     = 1,
  MSG_HRPT_VIS_0_8     = 2,
  MSG_HRPT_NIR_1_6     = 3,
  MSG_HRPT_IR_3_8      = 4,
  MSG_HRPT_IR_10_8     = 5,
  MSG_HRPT_IR_12_0     = 6,
  MSG_HRPT_UNKNOWN     = 7
} t_enum_HRPT_channel;

std::string MSG_channel_name(t_enum_MSG_spacecraft spacecraft_id,
                             uint_1 channel_id);

#endif
