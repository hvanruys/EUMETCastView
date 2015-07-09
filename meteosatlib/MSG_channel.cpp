//-----------------------------------------------------------------------------
//
//  File        : MSG_channel.cpp
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

#include "MSG_channel.h"

std::string MSG_channel_name(t_enum_MSG_spacecraft spacecraft_id,
                             uint_1 spectral_channel_id)
{
  std::string schan;
  switch(spacecraft_id)
  {
    case MSG_MSG_1:
    case MSG_MSG_2:
    case MSG_MSG_3:
    case MSG_MSG_4:
      schan = "MSG Seviri 1.5 ";
      switch(spectral_channel_id)
      {
        case MSG_SEVIRI_NO_CHANNEL:
          schan = "Image data, no";
          break;
        case MSG_SEVIRI_1_5_VIS_0_6:
          schan += "Visible 0.6";
          break;
        case MSG_SEVIRI_1_5_VIS_0_8:
          schan += "Visible 0.8";
          break;
        case MSG_SEVIRI_1_5_IR_1_6:
          schan += "Infrared 1.6";
          break;
        case MSG_SEVIRI_1_5_IR_3_9:
          schan += "Infrared 3.9";
          break;
        case MSG_SEVIRI_1_5_WV_6_2:
          schan += "Water Vapour 6.2";
          break;
        case MSG_SEVIRI_1_5_WV_7_3:
          schan += "Water Vapour 7.3";
          break;
        case MSG_SEVIRI_1_5_IR_8_7:
          schan += "Infrared 8.7";
          break;
        case MSG_SEVIRI_1_5_IR_9_7:
          schan += "Infrared 9.7";
          break;
        case MSG_SEVIRI_1_5_IR_10_8:
          schan += "Infrared 10.8";
          break;
        case MSG_SEVIRI_1_5_IR_12_0:
          schan += "Infrared 12.0";
          break;
        case MSG_SEVIRI_1_5_IR_13_4:
          schan += "Infrared 13.4";
          break;
        case MSG_SEVIRI_1_5_HRV:
          schan += "HRV";
          break;
        default:
          schan += "unknown";
          break;
      }
      schan += " channel";
      break;
    case MSG_GOES_7:
    case MSG_GOES_8:
    case MSG_GOES_9:
    case MSG_GOES_10:
    case MSG_GOES_11:
    case MSG_GOES_12:
      schan = "GOES ";
      switch (spectral_channel_id)
      {
        case MSG_GOES_VISIBLE:
          schan += "Visible";
          break;
        case MSG_GOES_IR_3_9:
          schan += "Infrared 3.9";
          break;
        case MSG_GOES_WV_6_8:
          schan += "Water Vapour 6.8";
          break;
        case MSG_GOES_IR_10_7:
          schan += "Infrared 10.7";
          break;
        case MSG_GOES_IR_12_0:
          schan += "Infrared 12.0";
          break;
        default:
          schan += "unknown";
          break;
      }
      schan += " channel";
      break;
    case MSG_GMS_4:
    case MSG_GMS_5:
    case MSG_GMS_6:
    case MSG_MTSAT_1:
    case MSG_MTSAT_2:
      schan = "GMS/MTSAT ";
      switch (spectral_channel_id)
      {
        case MSG_GMS_MTSAT_VISIBLE:
          schan += "Visible";
          break;
        case MSG_GMS_MTSAT_IR_3_8:
          schan += "Infrared 3.8";
          break;
        case MSG_GMS_MTSAT_WV_6_7:
          schan += "Water Vapour 6.7";
          break;
        case MSG_GMS_MTSAT_IR_10_8:
          schan += "Infrared 10.8";
          break;
        case MSG_GMS_MTSAT_IR_12_0:
          schan += "Infrared 12.0";
          break;
        default:
          schan += "unknown";
          break;
      }
      schan += " channel";
      break;
    case MSG_GOMS_1:
    case MSG_GOMS_2:
    case MSG_GOMS_3:
      schan = "GOMS ";
      switch (spectral_channel_id)
      {
        case MSG_GOMS_VISIBLE:
          schan += "Visible";
          break;
        case MSG_GOMS_WATER_VAPOUR:
          schan += "Water Vapour";
          break;
        case MSG_GOMS_INFRARED:
          schan += "Infrared";
          break;
        default:
          schan += "unknown";
          break;
      }
      schan += " channel";
      break;
    case MSG_METOP_1:
    case MSG_METOP_2:
    case MSG_METOP_3:
    case MSG_NOAA_12:
    case MSG_NOAA_13:
    case MSG_NOAA_14:
    case MSG_NOAA_15:
      schan = "Polar orbiter single or composite ";
      switch (spectral_channel_id)
      {
        case MSG_HRPT_VIS_0_6:
          schan += "Visible 0.6";
          break;
        case MSG_HRPT_VIS_0_8:
          schan += "Visible 0.8";
          break;
        case MSG_HRPT_NIR_1_6:
          schan += "Near Infrared 1.6";
          break;
        case MSG_HRPT_IR_3_8:
          schan += "Infrared 3.8";
          break;
        case MSG_HRPT_IR_10_8:
          schan += "Infrared 10.8";
          break;
        case MSG_HRPT_IR_12_0:
          schan += "Infrared 12.0";
          break;
        default:
          schan += "unknown";
          break;
      }
      schan += " channel";
      break;
    default:
      schan = "No channel information";
      break;
  }
  return schan;
}
