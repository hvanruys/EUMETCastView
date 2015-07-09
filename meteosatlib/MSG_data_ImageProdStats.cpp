//-----------------------------------------------------------------------------
//
//  File        : MSG_data_ImageProdStats.cpp
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
#include "MSG_data_ImageProdStats.h"

size_t MSG_ActualScanningSummary::read_from( unsigned const char_1 *buff )
{
  NominalImageScanning = (uint_1) *(buff);
  ReducedScan = (uint_1) *(buff+1);
  (void) ForwardScanStart.read_from(buff+2);
  (void) ForwardScanEnd.read_from(buff+8);
  return 14;
}

std::ostream& operator<< ( std::ostream& os, MSG_ActualScanningSummary &s )
{
  os << "Nominal Image Scan. : " << (uint_2) s.NominalImageScanning << std::endl
     << "Reduced Scan        : " << (uint_2) s.ReducedScan << std::endl
     << "Forw. Scan Start    : " << s.ForwardScanStart.get_timestring( )
     << std::endl
     << "Forw. Scan End      : " << s.ForwardScanEnd.get_timestring( )
     << std::endl;
  return os;
}

size_t MSG_RadiometerBehaviour::read_from( unsigned const char_1 *buff )
{
  NominalBehaviour = (uint_1) *(buff);
  RadScanIrregularity = (uint_1) *(buff+1);
  RadStoppage = (uint_1) *(buff+2);
  RepeatCycleNotCompleted = (uint_1) *(buff+3);
  GainChangeTookPlace = (uint_1) *(buff+4);
  DecontaminationTookPlace = (uint_1) *(buff+5);
  NoBBCalibrationAchieved = (uint_1) *(buff+6);
  IncorrectTemperature = (uint_1) *(buff+7);
  InvalidBBData = (uint_1) *(buff+8);
  InvalidAuxOrHKTMData = (uint_1) *(buff+9);
  RefocusingMechanismActuated = (uint_1) *(buff+10);
  MirrorBackToReferencePos = (uint_1) *(buff+11);
  return 12;
}

std::ostream& operator<< ( std::ostream& os, MSG_RadiometerBehaviour &b )
{
  os << "Nominal Behaviour   : " << (uint_2) b.NominalBehaviour << std::endl
     << "Rad Scan Irregular. : " << (uint_2) b.RadScanIrregularity << std::endl
     << "Rad Stoppage        : " << (uint_2) b.RadStoppage << std::endl
     << "Repeat not complete : " << (uint_2) b.RepeatCycleNotCompleted
     << std::endl
     << "Gain change         : " << (uint_2) b.GainChangeTookPlace << std::endl
     << "Decontamination     : " << (uint_2) b.DecontaminationTookPlace
     << std::endl
     << "No BB Calibration   : " << (uint_2) b.NoBBCalibrationAchieved
     << std::endl
     << "Incorrect Temp.     : " << (uint_2) b.IncorrectTemperature << std::endl
     << "Invalid BB data     : " << (uint_2) b.InvalidBBData << std::endl
     << "Invalid Aux HKTM    : " << (uint_2) b.InvalidAuxOrHKTMData << std::endl
     << "Refocusing actuated : " << (uint_2) b.RefocusingMechanismActuated
     << std::endl
     << "Mirror back ref.    : " << (uint_2) b.MirrorBackToReferencePos
     << std::endl;
  return os;
}

size_t MSG_ReceptionSummaryStats::read_from( unsigned const char_1 *buff )
{
  for (int i = 0; i < 12; i ++)
    PlannedNumberOfL10Lines[i] = get_ui4(buff+i*4);
  for (int i = 0; i < 12; i ++)
    NumberOfMissingL10Lines[i] = get_ui4(buff+48+i*4);
  for (int i = 0; i < 12; i ++)
    NumberOfCorruptedL10Lines[i] = get_ui4(buff+96+i*4);
  for (int i = 0; i < 12; i ++)
    NumberOfReplacedL10Lines[i] = get_ui4(buff+144+i*4);
  return 192;
}

std::ostream& operator<< ( std::ostream& os, MSG_ReceptionSummaryStats &r )
{
  for (int i = 0; i < 12; i ++)
    os << "Channel " << std::setw(2) << std::setfill('0')
       << i+1 << std::endl
       << "Planned L10 lines   : " << r.PlannedNumberOfL10Lines[i] << std::endl
       << "Missing L10 lines   : " << r.NumberOfMissingL10Lines[i] << std::endl
       << "Corrupted L10 lines : " << r.NumberOfCorruptedL10Lines[i]
       << std::endl
       << "Replaced L10 lines  : " << r.NumberOfReplacedL10Lines[i]
       << std::endl;
  return os;
}

size_t MSG_L15ImageValidity::read_from( unsigned const char_1 *buff )
{
  NominalImage = get_ui1(buff);
  NonNominalBecauseIncomplete = get_ui1(buff+1);
  NonNominalRadiometricQuality = get_ui1(buff+2);
  NonNominalGeometricQuality = get_ui1(buff+3);
  NonNominalTimeliness = get_ui1(buff+4);
  IncompleteL15 = get_ui1(buff+5);
  return 6;
}

std::ostream& operator<< ( std::ostream& os, MSG_L15ImageValidity &v )
{
  os << "Nominal Image       : " << (uint_2) v.NominalImage << std::endl
     << "Incomplete Image    : " << (uint_2) v.NonNominalBecauseIncomplete
     << std::endl
     << "Non Radiometric OK  : " << (uint_2) v.NonNominalRadiometricQuality
     << std::endl
     << "Non Geometric OK    : " << (uint_2) v.NonNominalGeometricQuality
     << std::endl
     << "Incomplete L 1.5    : " << (uint_2) v.IncompleteL15 << std::endl;
  return os;
}

size_t MSG_ActualL15CoverageVIS_IR::read_from( unsigned const char_1 *buff)
{
  SouthernLineActual = get_i4(buff);
  NorthernLineActual = get_i4(buff+4);
  EasternColumnActual = get_i4(buff+8);
  WesternColumnActual = get_i4(buff+12);
  return 16;
}

std::ostream& operator<< ( std::ostream& os, MSG_ActualL15CoverageVIS_IR &a )
{
  os << "Southern Line IR    : " << a.SouthernLineActual << std::endl
     << "Northern Line IR    : " << a.NorthernLineActual << std::endl
     << "Eastern Column IR   : " << a.EasternColumnActual << std::endl
     << "Western Column IR   : " << a.WesternColumnActual << std::endl;
  return os;
}

size_t MSG_ActualL15CoverageHRV::read_from( unsigned const char_1 *buff )
{
  LowerSouthLineActual = get_i4(buff);
  LowerNorthLineActual = get_i4(buff+4);
  LowerEastColumnActual = get_i4(buff+8);
  LowerWestColumnActual = get_i4(buff+12);
  UpperSouthLineActual = get_i4(buff+16);
  UpperNorthLineActual = get_i4(buff+20);
  UpperEastColumnActual = get_i4(buff+24);
  UpperWestColumnActual = get_i4(buff+28);
  return 32;
}

std::ostream& operator<< ( std::ostream& os, MSG_ActualL15CoverageHRV &a )
{
  os << "Low South Line HRV  : " << a.LowerSouthLineActual << std::endl
     << "Low North Line HRV  : " << a.LowerNorthLineActual << std::endl
     << "Low East Column HRV : " << a.LowerEastColumnActual << std::endl
     << "Low West Column HRV : " << a.LowerWestColumnActual << std::endl
     << "Up South Line HRV   : " << a.UpperSouthLineActual << std::endl
     << "Up North Line HRV   : " << a.UpperNorthLineActual << std::endl
     << "Up East Column HRV  : " << a.UpperEastColumnActual << std::endl
     << "Up West Column HRV  : " << a.UpperWestColumnActual << std::endl;
  return os;
}

MSG_data_ImageProdStats::MSG_data_ImageProdStats( ) { }

MSG_data_ImageProdStats::MSG_data_ImageProdStats(unsigned const char_1 *buff)
{
  this->read_from(buff);
}

MSG_data_ImageProdStats::~MSG_data_ImageProdStats( ) { }

size_t MSG_data_ImageProdStats::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  SatelliteId = (t_enum_MSG_spacecraft) get_ui2(buff);
  position += 2;
  position += ActualScanningSummary.read_from(buff+position);
  position += RadiometerBehaviour.read_from(buff+position);
  position += ReceptionSummaryStats.read_from(buff+position);
  for (int i = 0; i < 12; i ++)
    position += L15ImageValidity[i].read_from(buff+position);
  position += ActualL15CoverageVIS_IR.read_from(buff+position);
  position += ActualL15CoverageHRV.read_from(buff+position);
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_ImageProdStats &t )
{
  os << "------------------------------------------------------" << std::endl
     << "-         MSG IMAGE PRODUCTION STATISTICS            -" << std::endl
     << "------------------------------------------------------" << std::endl
     << "Satellite Id        : " << MSG_spacecraft_name(t.SatelliteId)
     << std::endl
     << t.ActualScanningSummary
     << t.RadiometerBehaviour
     << t.ReceptionSummaryStats;
  for (int i = 0; i < 12; i ++)
    os << "Channel " << std::setw(2) << std::setfill('0')
       << i+1 << std::endl << t.L15ImageValidity[i];
  os << t.ActualL15CoverageVIS_IR
     << t.ActualL15CoverageHRV;
  return os;
}
