//-----------------------------------------------------------------------------
//
//  File        : MSG_data_ImageAcquisition.cpp
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
#include "MSG_data_ImageAcquisition.h"

size_t MSG_PlannedAquisitionTime::read_from( unsigned const char_1 *buff )
{
  (void) TrueRepeatCycleStart.read_from(buff);
  (void) PlannedForwardScanEnd.read_from(buff+10);
  (void) PlannedRepeatCycleEnd.read_from(buff+20);
  return 30;
}

std::ostream& operator<< ( std::ostream& os, MSG_PlannedAquisitionTime &t )
{
  os << "True Rep. Cycle St. : " << t.TrueRepeatCycleStart.get_timestring( )
     << std::endl
     << "Pl. Fw. Scan End    : " << t.PlannedForwardScanEnd.get_timestring( )
     << std::endl
     << "Pl. Rp. Cycle End   : " << t.PlannedRepeatCycleEnd.get_timestring( )
     << std::endl;
  return os;
}

size_t MSG_RadiometerStatus::read_from( unsigned const char_1 *buff )
{
  for (int i = 0; i < 12; i ++)
    ChannelStatus[i] = (t_enum_MSG_boolean_status) *(buff+i);
  for (int i = 0; i < 42; i ++)
    DetectorStatus[i] = (t_enum_MSG_boolean_status) *(buff+12+i);
  return 54;
}

std::ostream& operator<< ( std::ostream& os, MSG_RadiometerStatus &s )
{
  for (int i = 0; i < 12; i ++)
  {
    os << "Channel " << std::setw(2) << std::setfill('0') << i+1
       << " Status   : " << MSG_boolean_status(s.ChannelStatus[i]) << std::endl;
  }
  for (int i = 0; i < 42; i ++)
  {
    os << "Detector " << std::setw(2) << std::setfill('0') << i+1
       << " Status  : " << MSG_boolean_status(s.DetectorStatus[i]) << std::endl;
  }
  return os;
}

size_t MSG_RadiometerSettings::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  for (int i = 0; i < 42; i ++)
    MDUSamplingDelays[i] = get_ui2(buff+position+i*2);
  position += 84;
  MDUNomHRVDelay1 = get_ui2(buff+position);
  position += 2;
  MDUNomHRVDelay2 = get_ui2(buff+position);
  position += 2;
  position += 2; // skip 16 bit
  MDUNomHRVBreakline = get_ui2(buff+position);
  position += 2;
  DHSSyncSelection = (t_enum_MSG_DHS_Sync) *(buff+position);
  position ++;
  for (int i = 0; i < 42; i ++)
    MDUOutGain[i] = get_ui2(buff+position+i*2);
  position += 84;
  memcpy(MDUCoarseGain, buff+position, 42);
  position += 42;
  for (int i = 0; i < 42; i ++)
    MDUFineGain[i] = get_ui2(buff+position+i*2);
  position += 84;
  for (int i = 0; i < 42; i ++)
    MDUNumericalOffset[i] = get_ui2(buff+position+i*2);
  position += 84;
  for (int i = 0; i < 42; i ++)
    PUGain[i] = get_ui2(buff+position+i*2);
  position += 84;
  for (int i = 0; i < 27; i ++)
    PUOffset[i] = get_ui2(buff+position+i*2);
  position += 54;
  for (int i = 0; i < 15; i ++)
    PUBias[i] = get_ui2(buff+position+i*2);
  position += 30;
  L0_LineCounter = get_ui2(buff+position);
  position += 2;
  K1_RetraceLines = get_ui2(buff+position);
  position += 2;
  K2_PauseDeciseconds = get_ui2(buff+position);
  position += 2;
  K3_RetraceLines = get_ui2(buff+position);
  position += 2;
  K4_PauseDeciseconds = get_ui2(buff+position);
  position += 2;
  K5_RetraceLines = get_ui2(buff+position);
  position += 2;
  X_DeepSpaceWindowPosition =
                  (t_enum_MSG_DeepSpace_Window_Position)*(buff+position);
  position ++;
  RefocusingLines = get_ui2(buff+position);
  position += 2;
  RefocusingDirection = (t_enum_MSG_Refocusing_Direction) *(buff+position);
  position ++;
  RefocusingPosition = get_ui2(buff+position);
  position += 2;
  ScanRefPosFlag = *(buff+position) ? true : false;
  position ++;
  ScanRefPosNumber = get_ui2(buff+position);
  position += 2;
  ScanRefPotVal = get_r4(buff+position);
  position += 4;
  ScanFirstLine = get_ui2(buff+position);
  position += 2;
  ScanLastLine = get_ui2(buff+position);
  position += 2;
  RetraceStartLine = get_ui2(buff+position);
  position += 2;
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_RadiometerSettings &s )
{
  for (int i = 0; i < 42; i ++)
  {
    os << "MDU Sampling delay" << std::setw(2) << std::setfill('0') << i+1
       << ": " << s.MDUSamplingDelays[i] << std::endl;
  }
  os << "MDU Nom. HRV Dly 1  : " << s.MDUNomHRVDelay1 << std::endl
     << "MDU Nom. HRV Dly 2  : " << s.MDUNomHRVDelay2 << std::endl
     << "MDU Nom. HRV Brkl   : " << s.MDUNomHRVBreakline << std::endl
     << "DHS Sync Selection  : " << MSG_DHS_Sync(s.DHSSyncSelection)
     << std::endl;
  for (int i = 0; i < 42; i ++)
  {
     os << "MDU Out Gain " << std::setw(2) << std::setfill('0') << i+1
        << "     : " << s.MDUOutGain[i] << std::endl
        << "MDU Coarse Gain " << std::setw(2) << std::setfill('0') << i+1
        << "  : " << (uint_2) s.MDUCoarseGain[i] << std::endl
        << "MDU Fine Gain " << std::setw(2) << std::setfill('0') << i+1
        << "    : " << s.MDUFineGain[i] << std::endl
        << "MDU Num. Offset " << std::setw(2) << std::setfill('0') << i+1
        << "  : " << s.MDUNumericalOffset[i] << std::endl
        << "PU Gain " << std::setw(2) << std::setfill('0') << i+1
        << "          : " << s.PUGain[i] << std::endl;
  }
  for (int i = 0; i < 27; i ++)
  {
     os << "PU Offset " << std::setw(2) << std::setfill('0') << i+1
        << "        : " << s.PUOffset[i] << std::endl;
  }
  for (int i = 0; i < 15; i ++)
  {
     os << "PU BIAS " << std::setw(2) << std::setfill('0') << i+1
        << "          : " << s.PUBias[i] << std::endl;
  }
  os << "L0 Line Counter     : " << s.L0_LineCounter << std::endl
     << "K1 Retrace Lines    : " << s.K1_RetraceLines << std::endl
     << "K2 Pause Deciseconds: " << s.K2_PauseDeciseconds << std::endl
     << "K3 Retrace Lines    : " << s.K3_RetraceLines << std::endl
     << "K4 Pause Deciseconds: " << s.K4_PauseDeciseconds << std::endl
     << "K5 Retrace Lines    : " << s.K5_RetraceLines << std::endl
     << "Deep Spc. Wnd. Pos. : " 
     << MSG_DeepSpace_Window_Position(s.X_DeepSpaceWindowPosition) << std::endl
     << "Refocusing Lines    : " << s.RefocusingLines << std::endl
     << "Refocusing Dir.     : "
     << MSG_Refocusing_Direction(s.RefocusingDirection) << std::endl
     << "Refocusing Position : " << s.RefocusingPosition << std::endl
     << "Scan Ref Pos Flag   : " << s.ScanRefPosFlag << std::endl
     << "Scan Ref Pos Num    : " << s.ScanRefPosNumber << std::endl
     << "Scan Ref. Pot. Val  : " << s.ScanRefPotVal << std::endl
     << "Scan First Line     : " << s.ScanFirstLine << std::endl
     << "Scan Last Line      : " << s.ScanLastLine << std::endl
     << "Retrace Start Line  : " << s.RetraceStartLine << std::endl;
  return os;
}

size_t MSG_RadiometerOperations::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  LastGainChangeFlag = *(buff+position) ? true : false;
  position ++;
  position += LastGainChangeTime.read_from(buff+position);
  DecontaminationNow = *(buff+position) ? true : false;
  position ++;
  position += DecontaminationStart.read_from(buff+position);
  position += DecontaminationEnd.read_from(buff+position);
  BBCalScheduled = *(buff+position) ? true : false;
  position ++;
  BBCalibrationType = (t_enum_MSG_BB_Calibration) *(buff+position);
  position ++;
  BBFirstLine = get_ui2(buff+position);
  position += 2;
  BBLastLine = get_ui2(buff+position);
  position += 2;
  ColdFocalPlaneOpTemp = get_ui2(buff+position);
  position += 2;
  WarmFocalPlaneOpTemp = get_ui2(buff+position);  
  position += 2;
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_RadiometerOperations &s )
{
  os << "Last Gain Ch. Flag  : " << s.LastGainChangeFlag << std::endl
     << "Last Gain Change    : " << s.LastGainChangeTime.get_timestring( )
     << std::endl
     << "Decontamination Now : " << s.DecontaminationNow << std::endl
     << "Decont. Start       : " << s.DecontaminationStart.get_timestring( )
     << std::endl
     << "Decont. End         : " << s.DecontaminationEnd.get_timestring( )
     << std::endl
     << "BB Cal. Scheduled   : " << s.BBCalScheduled << std::endl
     << "BB Calib. Type      : " << MSG_BB_Calibration(s.BBCalibrationType)
     << std::endl
     << "BB First Line       : " << s.BBFirstLine << std::endl
     << "BB Last Line        : " << s.BBLastLine << std::endl
     << "Cold Plane Temp.    : " << s.ColdFocalPlaneOpTemp << std::endl
     << "Warm Plane Temp.    : " << s.WarmFocalPlaneOpTemp << std::endl;
  return os;
}

MSG_data_ImageAcquisition::MSG_data_ImageAcquisition( ) { }

MSG_data_ImageAcquisition::MSG_data_ImageAcquisition(
                                     unsigned const char_1 *buff )
{
  this->read_from(buff);
}

MSG_data_ImageAcquisition::~MSG_data_ImageAcquisition( ) { }

size_t MSG_data_ImageAcquisition::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;

  position += PlannedAquisitionTime.read_from(buff+position);
  position += RadiometerStatus.read_from(buff+position);
  position += RadiometerSettings.read_from(buff+position);
  position += RadiometerOperations.read_from(buff+position);

  return position;
}

std::string MSG_boolean_status(t_enum_MSG_boolean_status stat)
{
  std::string v;
  switch (stat)
  {
    case MSG_BOOLEAN_STATUS_OFF:
      v = "Off";
      break;
    case MSG_BOOLEAN_STATUS_ON:
      v = "On";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

std::string MSG_DHS_Sync( t_enum_MSG_DHS_Sync sync )
{
  std::string v;
  switch (sync)
  {
    case MSG_DHS_SYNC_SUN:
      v = "Sun";
      break;
    case MSG_DHS_SYNC_EARTH_NORTH:
      v = "Earth/North";
      break;
    case MSG_DHS_SYNC_EARTH_SOUTH:
      v = "Earth/South";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

std::string MSG_DeepSpace_Window_Position(
     t_enum_MSG_DeepSpace_Window_Position pos)
{
  std::string v;
  switch (pos)
  {
    case MSG_DEEP_SPACE_WINDOW_NO_DELAY:
      v = "No Delay";
      break;
    case MSG_DEEP_SPACE_WINDOW_DELAY_1:
      v = "Predefined Delay 1";
      break;
    case MSG_DEEP_SPACE_WINDOW_DELAY_2:
      v = "Predefined Delay 2";
      break;
    case MSG_DEEP_SPACE_WINDOW_DELAY_3:
      v = "Predefined delay 3";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

std::string MSG_Refocusing_Direction(t_enum_MSG_Refocusing_Direction dir)
{
  std::string v;
  switch (dir)
  {
    case MSG_REFOCUSING_DIRECTION_UP:
      v = "Up";
      break;
    case MSG_REFOCUSING_DIRECTION_DOWN:
      v = "Down";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

std::string MSG_BB_Calibration(t_enum_MSG_BB_Calibration cal)
{
  std::string v;
  switch (cal)
  {
    case MSG_BB_CALIBRATION_HOT:
      v = "Hot";
      break;
    case MSG_BB_CALIBRATION_AMBIENT:
      v = "Ambient";
      break;
    case MSG_BB_CALIBRATION_INDETERMINATE:
      v = "Indeterminate";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_ImageAcquisition &t )
{
  os << "------------------------------------------------------" << std::endl
     << "-           MSG IMAGE ACQUISITION RECORD             -" << std::endl
     << "------------------------------------------------------" << std::endl
     << t.PlannedAquisitionTime
     << t.RadiometerStatus
     << t.RadiometerSettings
     << t.RadiometerOperations;

  return os;
}
