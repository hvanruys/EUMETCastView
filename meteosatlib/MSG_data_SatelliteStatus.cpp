//-----------------------------------------------------------------------------
//
//  File        : MSG_data_SatelliteStatus.cpp
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
#include "MSG_data_SatelliteStatus.h"

MSG_data_SatelliteStatus::MSG_data_SatelliteStatus( )
{
}

MSG_data_SatelliteStatus::MSG_data_SatelliteStatus(unsigned const char_1 *buff)
{
  this->read_from(buff);
}

MSG_data_SatelliteStatus::~MSG_data_SatelliteStatus( )
{
}

size_t MSG_data_SatelliteStatus::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  position += SatelliteDefinition.read_from(buff+position);
  position += SatelliteOperations.read_from(buff+position);
  position += Orbit.read_from(buff+position);
  position += Attitude.read_from(buff+position);
  SpinRateatRCStart = get_r8(buff+position);
  position += 8;
  position += UTCCorrelation.read_from(buff+position);
  return position;
}

MSG_orbit_coefficient::MSG_orbit_coefficient( )
{
  present = false;
}

MSG_AttitudeCoeff::MSG_AttitudeCoeff( )
{
  present = false;
}

size_t MSG_AttitudeCoeff::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  position += StartTime.read_from(buff+position);
  position += EndTime.read_from(buff+position);
  for (int i = 0; i < 8; i ++)
    XOfSpinAxis[i] = get_r8(buff+position+i*8);
  position += 64;
  for (int i = 0; i < 8; i ++)
    YOfSpinAxis[i] = get_r8(buff+position+i*8);
  position += 64;
  for (int i = 0; i < 8; i ++)
    ZOfSpinAxis[i] = get_r8(buff+position+i*8);
  position += 64;
  if (buff[0] == 0 && buff[1] == 0) present = false;
  else present = true;
  return position;
}

size_t MSG_orbit_coefficient::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  position += StartTime.read_from(buff+position);
  position += EndTime.read_from(buff+position);
  for (int i = 0; i < 8; i ++)
    X[i]  = get_r8(buff+position+i*8);
  position += 64;
  for (int i = 0; i < 8; i ++)
    Y[i]  = get_r8(buff+position+i*8);
  position += 64;
  for (int i = 0; i < 8; i ++)
    Z[i]  = get_r8(buff+position+i*8);
  position += 64;
  for (int i = 0; i < 8; i ++)
    VX[i] = get_r8(buff+position+i*8);
  position += 64;
  for (int i = 0; i < 8; i ++)
    VY[i] = get_r8(buff+position+i*8);
  position += 64;
  for (int i = 0; i < 8; i ++)
    VZ[i] = get_r8(buff+position+i*8);
  position += 64;
  if (buff[0] == 0 && buff[1] == 0) present = false;
  else present = true;
  return position;
}

bool MSG_AttitudeCoeff::is_present( )
{
  return present;
}

bool MSG_orbit_coefficient::is_present( )
{
  return present;
}

size_t MSG_Orbit::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  position += PeriodStartTime.read_from(buff+position);
  position += PeriodEndTime.read_from(buff+position);
  for (int i = 0; i < 100; i ++)
    position += OrbitPoliniomal[i].read_from(buff + position);
  return position;
}

size_t MSG_Attitude::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  position += PeriodStartTime.read_from(buff+position);
  position += PeriodEndTime.read_from(buff+position);
  PrincipalAxisOffsetAngle = get_r8(buff+position);
  position += 8;
  for (int i = 0; i < 100; i ++)
    position += AttitudePolynomial[i].read_from(buff + position);
  return position;
}

size_t MSG_UTCCorrelation::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  position += PeriodStartTime.read_from(buff+position);
  position += PeriodEndTime.read_from(buff+position);
  OnBoardTimeStart.read_from(buff+position);
  position += 7;
  VarOnBoardTimeStart = get_r8(buff+position);
  position += 8;
  A1    = get_r8(buff+position);
  position += 8;
  VarA1 = get_r8(buff+position);
  position += 8;
  A2    = get_r8(buff+position);
  position += 8;
  VarA2 = get_r8(buff+position);
  position += 8;
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_SatelliteStatus &t )
{
  os << "------------------------------------------------------" << std::endl
     << "-           MSG DATA SATELLITE STATUS                -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << t.SatelliteDefinition
     << t.SatelliteOperations
     << t.Orbit
     << t.Attitude
     << "Spin Rate at Start  : " << t.SpinRateatRCStart << std::endl
     << t.UTCCorrelation;
  return os;
}

std::ostream& operator<< ( std::ostream& os, MSG_orbit_coefficient &c )
{
  os << "COEFFICIENT:" << std::endl;
  os << "Start Time          : " << c.StartTime.get_timestring( ) << std::endl
     << "End Time            : " << c.EndTime.get_timestring( ) << std::endl;
  for (int i = 0; i < 8; i ++)
    os << "X : " << std::setw(12) << std::setfill(' ') << c.X[i] << " "
       << "Y : " << std::setw(12) << std::setfill(' ') << c.Y[i] << " "
       << "Z : " << std::setw(12) << std::setfill(' ') << c.Z[i] << std::endl;
  for (int i = 0; i < 8; i ++)
    os << "VX: " << std::setw(12) << std::setfill(' ') << c.VX[i] << " "
       << "VY: " << std::setw(12) << std::setfill(' ') << c.VY[i] << " "
       << "VZ: " << std::setw(12) << std::setfill(' ') << c.VZ[i] << std::endl;
  return os;
}

std::ostream& operator<< ( std::ostream& os, MSG_Orbit &o )
{
  os << "ORBIT RECORD" << std::endl;
  os << "Period Start Time   : " << o.PeriodStartTime.get_timestring( )
     << std::endl
     << "Period End Time     : " << o.PeriodEndTime.get_timestring( )
     << std::endl;
  os << "ORBIT Polinomial:" << std::endl;
  for (int i = 0; i < 100; i ++)
    if (o.OrbitPoliniomal[i].is_present())
      os << o.OrbitPoliniomal[i];
  os << "END ORBIT RECORD" << std::endl;
  return os;
}

std::ostream& operator<< ( std::ostream& os, MSG_AttitudeCoeff &a )
{
  os << "COEFFICIENT:" << std::endl;
  os << "Start Time          : " << a.StartTime.get_timestring( ) << std::endl
     << "End Time            : " << a.EndTime.get_timestring( ) << std::endl;
  for (int i = 0; i < 8; i ++)
    os << "XSA : " << std::setw(12) << std::setfill(' ') << a.XOfSpinAxis[i]
       << " "
       << "YSA : " << std::setw(12) << std::setfill(' ') << a.YOfSpinAxis[i]
       << " "
       << "ZSA : " << std::setw(12) << std::setfill(' ') << a.ZOfSpinAxis[i]
       << std::endl;
  return os;
}

std::ostream& operator<< ( std::ostream& os, MSG_Attitude &a )
{
  os << "ATTITUDE RECORD" << std::endl;
  os << "Period Start Time   : " << a.PeriodStartTime.get_timestring( )
     << std::endl
     << "Period End Time     : " << a.PeriodEndTime.get_timestring( )
     << std::endl
     << "Princ. Axis Off. An.: " << a.PrincipalAxisOffsetAngle
     << std::endl;
  os << "ATTITUDE Polinomial:" << std::endl;
  for (int i = 0; i < 100; i ++)
    if (a.AttitudePolynomial[i].is_present())
      os << a.AttitudePolynomial[i];
  os << "END ATTITUDE RECORD" << std::endl;
  return os;
}

std::ostream& operator<< ( std::ostream& os, MSG_UTCCorrelation &u )
{
  os << "UTC CORRELATION RECORD" << std::endl
     << "Period Start Time   : " << u.PeriodStartTime.get_timestring( )
     << std::endl
     << "Period End Time     : " << u.PeriodEndTime.get_timestring( )
     << std::endl 
     << "On board Time       : " << u.OnBoardTimeStart.get_coarse_time( )
     << " " << u.OnBoardTimeStart.get_fine_time( ) << " ("
     << u.OnBoardTimeStart.get_time_cuc_r8( ) << ")" << std::endl
     << "On board Time Var.  : " << u.VarOnBoardTimeStart << std::endl
     << "A1 time             : " << u.A1 << std::endl
     << "A1 time Variance    : " << u.VarA1 << std::endl
     << "A2 time             : " << u.A2 << std::endl
     << "A2 time Variance    : " << u.VarA2 << std::endl
     << "END UTC CORRELATION RECORD" << std::endl;
  return os;
}

std::string MSG_satellite_status(t_enum_MSG_SatelliteStatus statcode)
{
  std::string status;
  switch (statcode)
  {
    case MSG_SATELLITE_STATUS_OPERATIONAL:
      status = "Operational";
      break;
    case MSG_SATELLITE_STATUS_STANDBY:
      status = "Standby";
      break;
    case MSG_SATELLITE_STATUS_COMMISSIONING_TEST:
      status = "Commissioning or test";
      break;
    case MSG_SATELLITE_STATUS_MANOUVRE:
      status = "Manouvre";
      break;
    case MSG_SATELLITE_STATUS_DECONTAMINATION:
      status = "Decontamination";
      break;
    case MSG_SATELLITE_STATUS_SAFE_MODE:
      status = "Safe Mode";
      break;
    case MSG_SATELLITE_STATUS_DISSEMINATION_ONLY:
      status = "Dissemination Only";
      break;
    default:
      status = "Unknown";
      break;
  }
  return status;
}

std::string MSG_satellite_manouvre(t_enum_MSG_ManouvreType manouvre)
{
  std::string manv;
  switch (manouvre)
  {
    case MSG_SATELLITE_MANOUVRE_SPIN_UP:
      manv = "Spin Up";
      break;
    case MSG_SATELLITE_MANOUVRE_SPIN_DOWN:
      manv = "Spin Down";
      break;
    case MSG_SATELLITE_MANOUVRE_ATTITUDE_SLEW:
      manv = "Attitude Slew";
      break;
    case MSG_SATELLITE_MANOUVRE_N_S_STATION_KEEPING:
      manv = "N/S Station Keeping";
      break;
    case MSG_SATELLITE_MANOUVRE_E_W_STATION_KEEPING:
      manv = "E/W Station Keeping";
      break;
    case MSG_SATELLITE_MANOUVRE_STATION_RELOCATION:
      manv = "Station Relocation";
      break;
    default:
      manv = "Unknown";
      break;
  }
  return manv;
}

size_t MSG_SatelliteDefinition::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  SatelliteId = (t_enum_MSG_spacecraft) get_ui2(buff+position);
  position += 2;
  NominalLongitude = get_r4(buff+position);
  position += 4;
  SatelliteStatus = (t_enum_MSG_SatelliteStatus) *(buff+position);
  position ++;
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_SatelliteDefinition &d )
{
  os << "Satellite           : " << d.SatelliteId << " ("
     << MSG_spacecraft_name(d.SatelliteId) << ")" << std::endl
     << "Nominal Longitude   : " << d.NominalLongitude << std::endl
     << "Satellite Status    : " << MSG_satellite_status(d.SatelliteStatus)
     << std::endl;
  return os;
}

size_t MSG_SatelliteOperations::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  LastManoeuvreFlag = *(buff+position) ? true : false; 
  position ++;
  position += LastManouvreStartTime.read_from(buff+position);
  position += LastManouvreEndTime.read_from(buff+position);
  LastManouvreType = (t_enum_MSG_ManouvreType) *(buff+position);
  position ++;
  NextManoeuvreFlag = *(buff+position) ? true : false;
  position ++;
  position += NextManouvreStartTime.read_from(buff+position);
  position += NextManouvreEndTime.read_from(buff+position);
  NextManouvreType = (t_enum_MSG_ManouvreType) *(buff+position);
  position ++;
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_SatelliteOperations &o )
{
  if (o.LastManoeuvreFlag)
  {
     os << "Last Man. Flag      : " << o.LastManoeuvreFlag << std::endl
        << "Last Man. Start     : " << o.LastManouvreStartTime.get_timestring( )
        << std::endl
        << "Last Man. End       : " << o.LastManouvreEndTime.get_timestring( )
        << std::endl
        << "Last Man. Type      : "
        << MSG_satellite_manouvre(o.LastManouvreType) << std::endl;
  }
  if (o.NextManoeuvreFlag)
  {
     os << "Next Man. Flag      : " << o.NextManoeuvreFlag << std::endl
        << "Next Man. Start     : " << o.NextManouvreStartTime.get_timestring( )
        << std::endl
        << "Next Man. End       : " << o.NextManouvreEndTime.get_timestring( )
        << std::endl
        << "Next Man. Type      : " <<
            MSG_satellite_manouvre(o.NextManouvreType) << std::endl;
  }
  return os;
}
