//-----------------------------------------------------------------------------
//
//  File        : MSG_data_IMPFConfiguration.cpp
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
#include "MSG_data_IMPFConfiguration.h"

std::string MSG_SU_Instance(t_enum_MSG_SU_instance inst)
{
  std::string v;
  switch (inst)
  {
    case MSG_SU_SW_INSTANCE_M_C:
      v = "SW Instance M&C";
      break;
    case MSG_SU_ACCEPT_DATA_SEVIRI:
      v = "Accept Data Seviri";
      break;
    case MSG_SU_ACCEPT_DATA_HKTM:
      v = "Accept Data HKTM";
      break;
    case MSG_SU_ACCEPT_DATA_GERB:
      v = "Accept Data GERB";
      break;
    case MSG_SU_ACCEPT_DATA_FTP_SERVER:
      v = "Accept Data FTP Server";
      break;
    case MSG_SU_RCAL_GCAL:
      v = "Rcal/Gcal";
      break;
    case MSG_SU_RQA:
      v = "RQA";
      break;
    case MSG_SU_GQA:
      v = "GQA";
      break;
    case MSG_SU_REQUANTIZE_RESAMPLE_VIS06:
      v = "Requantize/Resample Visible Channel 0.6";
      break;
    case MSG_SU_REQUANTIZE_RESAMPLE_VIS08:
      v = "Requantize/Resample Visible Channel 0.8";
      break;
    case MSG_SU_REQUANTIZE_RESAMPLE_IR16:
      v = "Requantize/Resample Infrared Channel 1.6";
      break;
    case MSG_SU_REQUANTIZE_RESAMPLE_IR38:
      v = "Requantize/Resample Infrared Channel 3.8";
      break;
    case MSG_SU_REQUANTIZE_RESAMPLE_WV62:
      v = "Requantize/Resample Water Vapour Channel 6.2";
      break;
    case MSG_SU_REQUANTIZE_RESAMPLE_WV73:
      v = "Requantize/Resample Water Vapour Channel 7.3";
      break;
    case MSG_SU_REQUANTIZE_RESAMPLE_IR87:
      v = "Requantize/Resample Infrared Channel 8.7";
      break;
    case MSG_SU_REQUANTIZE_RESAMPLE_IR97:
      v = "Requantize/Resample Infrared Channel 9.7";
      break;
    case MSG_SU_REQUANTIZE_RESAMPLE_IR108:
      v = "Requantize/Resample Infrared Channel 10.8";
      break;
    case MSG_SU_REQUANTIZE_RESAMPLE_IR120:
      v = "Requantize/Resample Infrared Channel 12.0";
      break;
    case MSG_SU_REQUANTIZE_RESAMPLE_IR134:
      v = "Requantize/Resample Infrared Channel 13.4";
      break;
    case MSG_SU_REQUANTIZE_RESAMPLE_HRV:
      v = "Requantize/Resample HRV Channel";
      break;
    case MSG_SU_PRODUCE_QUICKLOOK_DATA:
      v = "Produce Quicklook Data";
      break;
    case MSG_SU_SEND_DATA_SEVIRI_LPT0:
      v = "Send Seviri Data LPT0";
      break;
    case MSG_SU_SEND_DATA_SEVIRI_LPT5:
      v = "Send Seviri Data LPT5";
      break;
    case MSG_SU_SEND_DATA_HKTM:
      v = "Send Data HKTM";
      break;
    case MSG_SU_SEND_DATA_RAW_GERB:
      v = "Send Data Raw GERB";
      break;
    case MSG_SU_SEND_DATA_VALIDATED_GERB:
      v = "Send Data Validated GERB";
      break;
    case MSG_SU_SEND_DATA_GENERATED_FD:
      v = "Send Data Generated FD";
      break;
    case MSG_SU_SEND_FTP_CLIENT_PUT:
      v = "Send Data FTP Client PUT";
      break;
    case MSG_SU_OBJECT_STORE_SERVER:
      v = "Object Store Server";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

std::string MSG_SU_ID(uint_4 SUId)
{
  std::string v;

  if (SUId == 0) v = "No particular SU";
  else if (SUId >= 10000 && SUId < 20000) v = "BRGS";
  else if (SUId >= 20000 && SUId < 30000) v = "CF";
  else if (SUId >= 30000 && SUId < 40000) v = "DADF";
  else if (SUId >= 40000 && SUId < 50000) v = "IMPF";
  else if (SUId >= 50000 && SUId < 60000) v = "MARF";
  else if (SUId >= 60000 && SUId < 70000) v = "MPEF";
  else if (SUId >= 70000 && SUId < 80000) v = "PGS";
  else if (SUId >= 80000 && SUId < 90000) v = "SSF";
  else if (SUId >= 90000 && SUId < 100000) v = "SIU";
  else if (SUId >= 100000 && SUId < 101000) v = "LEOP_CC";
  else v = "Unknown";
  return v;
}

std::string MSG_SUMode(t_enum_MSG_SUMode mod)
{
  std::string v;
  switch (mod)
  {
    case MSG_SU_MODE_OFF:
      v = "Off";
      break;
    case MSG_SU_MODE_ON_NON_PROCESSING:
      v = "Non Processing";
      break;
    case MSG_SU_MODE_ON_REAL_TIME_PROCESSING:
      v = "On - Real Time Processing";
      break;
    case MSG_SU_MODE_ON_ANALYSIS_MODE:
      v = "On - Analysis Mode";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

std::string MSG_SUState(t_enum_MSG_SUState sta)
{
  std::string v;
  switch (sta)
  {
    case MSG_SU_STATE_ERROR:
      v = "Error";
      break;
    case MSG_SU_STATE_NOMINAL:
      v = "Nominal";
      break;
    case MSG_SU_STATE_DEGRADED:
      v = "Degraded";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

size_t MSG_GPConfigItemVersion::read_from( unsigned const char_1 *buff )
{
  Issue = get_ui2(buff);
  Revision = get_ui2(buff+2);
  return 4;
}

std::ostream& operator<< ( std::ostream& os, MSG_GPConfigItemVersion &v )
{
  os << "Issue               : " << v.Issue << std::endl
     << "Revision            : " << v.Revision << std::endl;
  return os;
}

size_t MSG_MirrorParameters::read_from( unsigned const char_1 *buff )
{
  MaxFeedbackVoltage = get_r8(buff);
  MinFeedbackVoltage = get_r8(buff+8);
  MirrorSlipEstimate = get_r8(buff+16);
  return 24;
}

std::ostream& operator<< ( std::ostream& os, MSG_MirrorParameters &p )
{
  os << "Max Feedback Voltage: " << p.MaxFeedbackVoltage << std::endl
     << "Min Feedback Voltage: " << p.MinFeedbackVoltage << std::endl
     << "Mirror Slip Estimate: " << p.MirrorSlipEstimate << std::endl;
  return os;
}

size_t MSG_HKTMParameters::read_from( unsigned const char_1 *buff)
{
  (void) TimeS0Packet.read_from(buff);
  (void) TimeS1Packet.read_from(buff+6);
  (void) TimeS2Packet.read_from(buff+12);
  (void) TimeS3Packet.read_from(buff+18);
  (void) TimeS4Packet.read_from(buff+24);
  (void) TimeS5Packet.read_from(buff+30);
  (void) TimeS6Packet.read_from(buff+36);
  (void) TimeS7Packet.read_from(buff+42);
  (void) TimeS8Packet.read_from(buff+48);
  (void) TimeS9Packet.read_from(buff+54);
  (void) TimeSYPacket.read_from(buff+60);
  (void) TimePSPacket.read_from(buff+66);
  return 3480; // ?? 498
}

std::ostream& operator<< ( std::ostream& os, MSG_HKTMParameters &p )
{
  os << "Time S0 Packet      : " << p.TimeS0Packet.get_timestring( )<< std::endl
     << "Time S1 Packet      : " << p.TimeS1Packet.get_timestring( )<< std::endl
     << "Time S2 Packet      : " << p.TimeS2Packet.get_timestring( )<< std::endl
     << "Time S3 Packet      : " << p.TimeS3Packet.get_timestring( )<< std::endl
     << "Time S4 Packet      : " << p.TimeS4Packet.get_timestring( )<< std::endl
     << "Time S5 Packet      : " << p.TimeS5Packet.get_timestring( )<< std::endl
     << "Time S6 Packet      : " << p.TimeS6Packet.get_timestring( )<< std::endl
     << "Time S7 Packet      : " << p.TimeS7Packet.get_timestring( )<< std::endl
     << "Time S8 Packet      : " << p.TimeS8Packet.get_timestring( )<< std::endl
     << "Time S9 Packet      : " << p.TimeS9Packet.get_timestring( )<< std::endl
     << "Time SY Packet      : " << p.TimeSYPacket.get_timestring( )<< std::endl
     << "Time PS Packet      : " << p.TimePSPacket.get_timestring( )
     << std::endl;
  return os;
}

size_t MSG_SUConfiguration::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  position += SWVersion.read_from(buff+position);
  for (int i = 0; i < 10; i ++)
    position += InfoBaseVersion[i].read_from(buff+position);
  return position;
}

std::ostream& operator<< (std::ostream& os, MSG_SUConfiguration &g)
{
  os << "SW VERSION" << std::endl;
  os << g.SWVersion;
  os << "INFO BASE VERSION" << std::endl;
  for (int i = 0; i < 10; i ++)
    os << g.InfoBaseVersion[i];
  return os;
}

size_t MSG_SUDetail::read_from( unsigned const char_1 *buff )
{
  SoftwareUnitId = get_ui4(buff);
  SUIdInstance = (t_enum_MSG_SU_instance) *(buff+4);
  SUMode = (t_enum_MSG_SUMode) *(buff+5);
  SUState = (t_enum_MSG_SUState) *(buff+6);
  return (7 + SUConfiguration.read_from(buff+7));
}

std::ostream& operator<< ( std::ostream& os, MSG_SUDetail &d )
{
  os << "Software Unit ID    : " << d.SoftwareUnitId << " ("
     << MSG_SU_ID(d.SoftwareUnitId) << ")" << std::endl
     << "Software Unit Inst. : " << MSG_SU_Instance(d.SUIdInstance)
     << std::endl
     << "Software Unit Mode  : " << MSG_SUMode(d.SUMode) << std::endl
     << "Software Unit State : " << MSG_SUState(d.SUState) << std::endl
     << d.SUConfiguration;
  return os;
}

size_t MSG_EqualisationParms::read_from( unsigned const char_1 *buff )
{
  ConstCoef = get_r4(buff);
  LinearCoef = get_r4(buff+4);
  QuadraticCoef = get_r4(buff+8);
  return 12;
}

std::ostream& operator<< ( std::ostream& os, MSG_EqualisationParms &e )
{
  os << "Constant Coefficient: " << e.ConstCoef << std::endl
     << "Linear Coefficient  : " << e.LinearCoef << std::endl
     << "Quadratic Coeff.    : " << e.QuadraticCoef << std::endl;
  return os;
}

size_t MSG_BlackBodyDataWarmStart::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  for (int i = 0; i < 12; i ++)
    GTotalForMethod1[i] = get_r8(buff+position+i*8);
  position += 96;
  for (int i = 0; i < 12; i ++)
    GTotalForMethod2[i] = get_r8(buff+position+i*8);
  position += 96;
  for (int i = 0; i < 12; i ++)
    GTotalForMethod3[i] = get_r8(buff+position+i*8);
  position += 96;
  for (int i = 0; i < 12; i ++)
    GBackForMethod1[i] = get_r8(buff+position+i*8);
  position += 96;
  for (int i = 0; i < 12; i ++)
    GBackForMethod2[i] = get_r8(buff+position+i*8);
  position += 96;
  for (int i = 0; i < 12; i ++)
    GBackForMethod3[i] = get_r8(buff+position+i*8);
  position += 96;
  for (int i = 0; i < 12; i ++)
    RatioGTotalToGBack[i] = get_r8(buff+position+i*8);
  position += 96;
  for (int i = 0; i < 12; i ++)
    GainInFrontOpticsCont[i] = get_r8(buff+position+i*8);
  position += 96;
  for (int i = 0; i < 12; i ++)
    CalibrationConstants[i] = get_r4(buff+position+i*8);
  position += 48;
  TimeOfColdObsSeconds = get_r8(buff+position);
  position += 8;
  TimeOfColdObsNanoSecs = get_r8(buff+position);
  position += 8;
  for (int i = 0; i < 12; i ++)
    IncidentRadiance[i] = get_r8(buff+position+i*8);
  position += 96;
  TempCal = get_r8(buff+position);
  position += 8;
  TempM1 = get_r8(buff+position);
  position += 8;
  TempScan = get_r8(buff+position);
  position += 8;
  TempM1Baf = get_r8(buff+position);
  position += 8;
  TempCalSurround = get_r8(buff+position);
  position += 8;
  position += MirrorParameters.read_from(buff+position);
  LastSpinPeriod = get_r8(buff+position);
  position += 8;
  position += HKTMParameters.read_from(buff+position);
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_BlackBodyDataWarmStart &b )
{
  os << "Gain for Method 1   : " << std::endl;
  for (int i = 0; i < 12; i += 4)
    os << std::setw(12) << std::setfill(' ') << b.GTotalForMethod1[i] << " "
       << std::setw(12) << std::setfill(' ') << b.GTotalForMethod1[i+1] << " "
       << std::setw(12) << std::setfill(' ') << b.GTotalForMethod1[i+2] << " "
       << std::setw(12) << std::setfill(' ') << b.GTotalForMethod1[i+3]
       << std::endl;
  os << "Gain for Method 2   : " << std::endl;
  for (int i = 0; i < 12; i += 4)
    os << std::setw(12) << std::setfill(' ') << b.GTotalForMethod2[i] << " "
       << std::setw(12) << std::setfill(' ') << b.GTotalForMethod2[i+1] << " "
       << std::setw(12) << std::setfill(' ') << b.GTotalForMethod2[i+2] << " "
       << std::setw(12) << std::setfill(' ') << b.GTotalForMethod2[i+3]
       << std::endl;
  os << "Gain for Method 3   : " << std::endl;
  for (int i = 0; i < 12; i += 4)
    os << std::setw(12) << std::setfill(' ') << b.GTotalForMethod3[i] << " "
       << std::setw(12) << std::setfill(' ') << b.GTotalForMethod3[i+1] << " "
       << std::setw(12) << std::setfill(' ') << b.GTotalForMethod3[i+2] << " "
       << std::setw(12) << std::setfill(' ') << b.GTotalForMethod3[i+3]
       << std::endl;
  os << "Gain BB for Method 1: " << std::endl;
  for (int i = 0; i < 12; i += 4)
    os << std::setw(12) << std::setfill(' ') << b.GBackForMethod1[i] << " "
       << std::setw(12) << std::setfill(' ') << b.GBackForMethod1[i+1] << " "
       << std::setw(12) << std::setfill(' ') << b.GBackForMethod1[i+2] << " "
       << std::setw(12) << std::setfill(' ') << b.GBackForMethod1[i+3]
       << std::endl;
  os << "Gain BB for Method 2: " << std::endl;
  for (int i = 0; i < 12; i += 4)
    os << std::setw(12) << std::setfill(' ') << b.GBackForMethod2[i] << " "
       << std::setw(12) << std::setfill(' ') << b.GBackForMethod2[i+1] << " "
       << std::setw(12) << std::setfill(' ') << b.GBackForMethod2[i+2] << " "
       << std::setw(12) << std::setfill(' ') << b.GBackForMethod2[i+3]
       << std::endl;
  os << "Gain BB for Method 3: " << std::endl;
  for (int i = 0; i < 12; i += 4)
    os << std::setw(12) << std::setfill(' ') << b.GBackForMethod3[i] << " "
       << std::setw(12) << std::setfill(' ') << b.GBackForMethod3[i+1] << " "
       << std::setw(12) << std::setfill(' ') << b.GBackForMethod3[i+2] << " "
       << std::setw(12) << std::setfill(' ') << b.GBackForMethod3[i+3]
       << std::endl;
  os << "Gain Ratio          : " << std::endl;
  for (int i = 0; i < 12; i += 4)
    os << std::setw(12) << std::setfill(' ') << b.RatioGTotalToGBack[i] << " "
       << std::setw(12) << std::setfill(' ') << b.RatioGTotalToGBack[i+1] << " "
       << std::setw(12) << std::setfill(' ') << b.RatioGTotalToGBack[i+2] << " "
       << std::setw(12) << std::setfill(' ') << b.RatioGTotalToGBack[i+3]
       << std::endl;
  os << "Gain Front Optics   : " << std::endl;
  for (int i = 0; i < 12; i += 4)
    os << std::setw(12) << std::setfill(' ') << b.GainInFrontOpticsCont[i]
       << " "
       << std::setw(12) << std::setfill(' ') << b.GainInFrontOpticsCont[i+1]
       << " "
       << std::setw(12) << std::setfill(' ') << b.GainInFrontOpticsCont[i+2]
       << " "
       << std::setw(12) << std::setfill(' ') << b.GainInFrontOpticsCont[i+3]
       << std::endl;
  os << "Calibration Const.  : " << std::endl;
  for (int i = 0; i < 12; i += 4)
    os << std::setw(12) << std::setfill(' ') << b.CalibrationConstants[i]
       << " "
       << std::setw(12) << std::setfill(' ') << b.CalibrationConstants[i+1]
       << " "
       << std::setw(12) << std::setfill(' ') << b.CalibrationConstants[i+2]
       << " "
       << std::setw(12) << std::setfill(' ') << b.CalibrationConstants[i+3]
       << std::endl;
  os << "Time Cold Obs Secs  : " << b.TimeOfColdObsSeconds << std::endl
     << "Time Cold Obs NanoS.: " << b.TimeOfColdObsNanoSecs << std::endl;
  os << "Incident Radiance   : " << std::endl;
  for (int i = 0; i < 12; i += 4)
    os << std::setw(12) << std::setfill(' ') << b.IncidentRadiance[i] << " "
       << std::setw(12) << std::setfill(' ') << b.IncidentRadiance[i+1] << " "
       << std::setw(12) << std::setfill(' ') << b.IncidentRadiance[i+2] << " "
       << std::setw(12) << std::setfill(' ') << b.IncidentRadiance[i+3]
       << std::endl;
  os << "Temp. BB cold read. : " << b.TempCal << std::endl
     << "Temp. M1 cold read. : " << b.TempM1 << std::endl
     << "Temp. SC cold read. : " << b.TempScan << std::endl
     << "Temp. BF cold read. : " << b.TempM1Baf << std::endl
     << "Temp. Surround      : " << b.TempCalSurround << std::endl
     << b.MirrorParameters
     << "Last Spin Period    : " << b.LastSpinPeriod << std::endl
     << b.HKTMParameters;
  return os;
}

size_t MSG_WarmStartParms::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  for (int i = 0; i < 1527; i ++)
    ScanningLaw[i] = get_r8(buff+position+i*8);
  position += 1527*8;
  for (int i = 0; i < 3; i ++)
    RadFramesAlignment[i] = get_r8(buff+position+i*8);
  position += 24;
  ScanningLawVariation[0] = get_r4(buff+position);
  ScanningLawVariation[1] = get_r4(buff+position+4);
  position += 8;
  for (int i = 0; i < 42; i ++)
    position += EqualisationParms[i].read_from(buff+position);
  position += BlackBodyDataWarmStart.read_from(buff+position);
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_WarmStartParms &w )
{
  os << "Scanning Law        : " << std::endl;
  for (int i = 0; i < 1527; i += 3)
    os << std::setw(12) << std::setfill(' ') << w.ScanningLaw[i] << " "
       << std::setw(12) << std::setfill(' ') << w.ScanningLaw[i+1] << " "
       << std::setw(12) << std::setfill(' ') << w.ScanningLaw[i+2]
       << std::endl;
  os << "Rad. Frames Align.  : " << std::endl;
  os << std::setw(12) << std::setfill(' ') << w.RadFramesAlignment[0] << " "
     << std::setw(12) << std::setfill(' ') << w.RadFramesAlignment[1] << " "
     << std::setw(12) << std::setfill(' ') << w.RadFramesAlignment[2]
     << std::endl;
  os << "Scanning Law Variat.: " << std::endl;
  os << std::setw(12) << std::setfill(' ') << w.ScanningLawVariation[0] << " "
     << std::setw(12) << std::setfill(' ') << w.ScanningLawVariation[1]
     << std::endl;
  os << "Equalisation Params.: " << std::endl;
  for (int i = 0; i < 42; i ++)
    os << w.EqualisationParms[i];
  os << w.BlackBodyDataWarmStart;
  return os;
}

MSG_data_IMPFConfiguration::MSG_data_IMPFConfiguration( ) { }

MSG_data_IMPFConfiguration::MSG_data_IMPFConfiguration(
                    unsigned const char_1 *buff)
{
  this->read_from(buff);
}

MSG_data_IMPFConfiguration::~MSG_data_IMPFConfiguration( ) { }

size_t MSG_data_IMPFConfiguration::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  position += GPConfigItemVersion.read_from(buff+position);
  for (int i = 0; i < 50; i ++)
    position += SuDetails[i].read_from(buff+position);
  position += WarmStartParms.read_from(buff+position);
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_IMPFConfiguration &t )
{
  os << "------------------------------------------------------" << std::endl
     << "-           MSG IMPF CONFIGURATION RECORD            -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << t.GPConfigItemVersion;
  for (int i = 0; i < 50; i ++)
    os << t.SuDetails[i];
  os << t.WarmStartParms;
  return os;
}
