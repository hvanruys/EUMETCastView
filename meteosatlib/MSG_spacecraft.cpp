//-----------------------------------------------------------------------------
//
//  File        : MSG_spacecraft.cpp
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

#include "MSG_spacecraft.h"

std::string MSG_spacecraft_name( t_enum_MSG_spacecraft spacecraft_id )
{
  std::string spcname;
  switch(spacecraft_id)
  {
    case MSG_NO_SPACECRAFT:
      spcname = "Non Spacecraft";
      break;
    case MSG_METOP_1:
      spcname = "METOP1";
      break;
    case MSG_METOP_2:
      spcname = "METOP2";
      break;
    case MSG_METOP_3:
      spcname = "METOP3";
      break;
    case MSG_METEOSAT_3:
      spcname = "METEOSAT3";
      break;
    case MSG_METEOSAT_4:
      spcname = "METEOSAT4";
      break;
    case MSG_METEOSAT_5:
      spcname = "METEOSAT5";
      break;
    case MSG_METEOSAT_6:
      spcname = "METEOSAT6";
      break;
    case MSG_MTP_1:
      spcname = "MTP1";
      break;
    case MSG_MTP_2:
      spcname = "MTP2";
      break;
    case MSG_MSG_1:
      spcname = "MSG1";
      break;
    case MSG_MSG_2:
      spcname = "MSG2";
      break;
    case MSG_MSG_3:
      spcname = "MSG3";
      break;
    case MSG_MSG_4:
      spcname = "MSG4";
      break;
    case MSG_NOAA_12:
      spcname = "NOAA12";
      break;
    case MSG_NOAA_13:
      spcname = "NOAA13";
      break;
    case MSG_NOAA_14:
      spcname = "NOAA14";
      break;
    case MSG_NOAA_15:
      spcname = "NOAA15";
      break;
    case MSG_GOES_7:
      spcname = "GOES7";
      break;
    case MSG_GOES_8:
      spcname = "GOES8";
      break;
    case MSG_GOES_9:
      spcname = "GOES9";
      break;
    case MSG_GOES_10:
      spcname = "GOES10";
      break;
    case MSG_GOES_11:
      spcname = "GOES11";
      break;
    case MSG_GOES_12:
      spcname = "GOES12";
      break;
    case MSG_GOMS_1:
      spcname = "GOMS1";
      break;
    case MSG_GOMS_2:
      spcname = "GOMS2";
      break;
    case MSG_GOMS_3:
      spcname = "GOMS3";
      break;
    case MSG_GMS_4:
      spcname = "GMS4";
      break;
    case MSG_GMS_5:
      spcname = "GMS5";
      break;
    case MSG_GMS_6:
      spcname = "GMS6";
      break;
    case MSG_MTSAT_1:
      spcname = "MTSAT1";
      break;
    case MSG_MTSAT_2:
      spcname = "MTSAT2";
      break;
    case MSG_HIMAWARI_8:
      spcname = "HIMAWARI-8";
      break;

  default:
      spcname = "Unknown";
      break;
  }
  return spcname;
}
