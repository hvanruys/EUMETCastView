//-----------------------------------------------------------------------------
//
//  File        : MSG_data_RadiometricProc.cpp
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
#include <cmath>

#if WIN32
#define _USE_MATH_DEFINES
#include <math.h>
#endif
#include "MSG_data_RadiometricProc.h"

std::string MSG_image_quality_flag(t_enum_MSG_image_quality_flag iqf)
{
  std::string v;
  switch (iqf)
  {
    case MSG_IMPF_CAL_OK:
      v = "IMPF Calibration OK";
      break;
    case MSG_IMPF_CAL_DUBIOUS:
      v = "IMPF Calibration Dubious";
      break;
    case MSH_IMPF_CAL_USE_MPEF_CAL:
      v = "Use MPEF calibration";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

std::string MSG_reference_data_flag(t_enum_MSG_reference_data_flag rd)
{
  std::string v;
  switch (rd)
  {
    case MSG_REFERENCE_DATA_NOT_CALCULATED:
      v = "Not calculated";
      break;
    case MSG_REFERENCE_DATA_MET_DATA:
      v = "Meteorological data";
      break;
    case MSG_REFERENCE_DATA_FSD:
      v = "Foreign satellite data";
      break;
    case MSG_REFERENCE_DATA_MIXED_MET_FSD:
      v = "Both meteo and foreign satellite";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

std::string MSG_absolute_calibration_method(
                          t_enum_MSG_absolute_calibration_method acm)
{
  std::string v;
  switch (acm)
  {
    case MSG_ABSOLUTE_CALIBRATION_NOT_CHANGED:
      v = "Not changed";
      break;
    case MSG_ABSOLUTE_CALIBRATION_EXTERNAL:
      v = "External";
      break;
    case MSG_ABSOLUTE_CALIBRATION_VICARIOUS:
      v = "Vicarious";
      break;
    case MSG_ABSOLUTE_CALIBRATION_CROSS_SATELLITE:
      v = "Cross satellite";
      break;
    case MSG_ABSOLUTE_CALIBRATION_MIXED_VIC_XSAT:
      v = "Mixed vicarious and cross satellite";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

size_t MSG_Image_Calibration::read_from( unsigned const char_1 *buff )
{
  Cal_Slope  = get_r8(buff);
  Cal_Offset = get_r8(buff+8);
  return 16;
}

std::ostream& operator<< ( std::ostream& os, MSG_Image_Calibration &c )
{
  os << "Calibration Slope   : " << c.Cal_Slope << std::endl
     << "Calibration Offset  : " << c.Cal_Offset << std::endl;
  return os;
}

size_t MSG_ExtractedBBData::read_from( unsigned const char_1 *buff )
{
  NumberOfPixelsUsed   = get_ui4(buff);
  MeanCount            = get_r4(buff+4);
  RMS                  = get_r4(buff+8);
  MaxCount             = get_ui2(buff+12);
  MinCount             = get_ui2(buff+14);
  BB_Processing_Slope  = get_r8(buff+16);
  BB_Processing_Offset = get_r8(buff+24);
  return 32;
}

std::ostream& operator<< ( std::ostream& os, MSG_ExtractedBBData &b )
{
  os << "Number of Pix. Used : " << b.NumberOfPixelsUsed << std::endl
     << "Mean Count          : " << b.MeanCount << std::endl
     << "RMS                 : " << b.RMS << std::endl
     << "Max Count           : " << b.MaxCount << std::endl
     << "Min Count           : " << b.MinCount << std::endl
     << "Processing Slope    : " << b.BB_Processing_Slope << std::endl
     << "Processing Offset   : " << b.BB_Processing_Offset << std::endl;
  return os;
}

size_t MSG_IMPF_CAL_Data::read_from( unsigned const char_1 *buff )
{
  ImageQualityFlag = (t_enum_MSG_image_quality_flag) *(buff);
  ReferenceDataFlag = (t_enum_MSG_reference_data_flag) *(buff+1);
  AbsCalMethod = (t_enum_MSG_absolute_calibration_method) *(buff+2);
  // skip 1 byte;
  AbsCalWeightVic = get_r4(buff+4);
  AbsCalWeightXsat = get_r4(buff+8);
  AbsCalCoeff = get_r4(buff+12);
  AbsCalError = get_r4(buff+16);
  CalMonBias = get_r4(buff+20);
  CalMonRms = get_r4(buff+24);
  OffsetCount = get_r4(buff+28);
  return 32;
}

size_t MSG_RPSummary::read_from( unsigned const char_1 *buff )
{
  for (int i = 0; i < 12; i ++)
    RadianceLinearization[i] = *(buff+i) ? true : false;
  for (int i = 0; i < 12; i ++)
    DetectorEqualization[i] = *(buff+12+i) ? true : false;
  for (int i = 0; i < 12; i ++)
    OnboardCalibrationResult[i] = *(buff+24+i) ? true : false;
  for (int i = 0; i < 12; i ++)
    MPEFCalFeedBack[i] = *(buff+36+i) ? true : false;
  for (int i = 0; i < 12; i ++)
    MTFAdaptation[i] = *(buff+48+i) ? true : false;
  for (int i = 0; i < 12; i ++)
    StraylightCorrectionFlag[i] = *(buff+60+i) ? true : false;
  return 72;
}

std::ostream& operator<< ( std::ostream& os, MSG_RPSummary &s )
{
  os << "Radiance Lin.       : ";
  for (int i = 0; i < 12; i ++)
    os << s.RadianceLinearization[i] << " ";
  os << std::endl
     << "Detector Eq.        : ";
  for (int i = 0; i < 12; i ++)
    os << s.DetectorEqualization[i] << " ";
  os << std::endl
     << "OnBoard Cal. Result : ";
  for (int i = 0; i < 12; i ++)
    os << s.OnboardCalibrationResult[i] << " ";
  os << std::endl
     << "MPEF Cal. Feedback  : ";
  for (int i = 0; i < 12; i ++)
    os << s.MPEFCalFeedBack[i] << " ";
  os << std::endl
     << "MTF Adaptation      : ";
  for (int i = 0; i < 12; i ++)
    os << s.MTFAdaptation[i] << " ";
  os << std::endl
     << "Straylight Correct. : ";
  for (int i = 0; i < 12; i ++)
    os << s.StraylightCorrectionFlag[i] << " ";
  return os;
}

size_t MSG_RadProcMTFAdaptation::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  for (int i = 0; i < 33; i ++)
    for (int j = 0; j < 16; j ++)
      VIS_IRMTFCorrectionE_W[i][j] = get_r4(buff+position+i*64+j*4);
  position += 33*16*4;
  for (int i = 0; i < 33; i ++)
    for (int j = 0; j < 16; j ++)
      VIS_IRMTFCorrectionN_S[i][j] = get_r4(buff+position+i*64+j*4);
  position += 33*16*4;
  for (int i = 0; i < 9; i ++)
    for (int j = 0; j < 16; j ++)
      HRVIRMTFCorrectionE_W[i][j] = get_r4(buff+position+i*64+j*4);
  position += 9*16*4;
  for (int i = 0; i < 9; i ++)
    for (int j = 0; j < 16; j ++)
      HRVIRMTFCorrectionN_S[i][j] = get_r4(buff+position+i*64+j*4);
  position += 9*16*4;
  return position;
}

std::ostream& operator<< (std::ostream& os, MSG_RadProcMTFAdaptation &f)
{
  for (int i = 0; i < 33; i ++)
  {
    os << "VIS IR MTF E/W " << std::setw(2) << std::setfill('0') << i+1
       << "   : " << std::endl;
    for(int j = 0; j < 16; j += 4)
    {
      os << std::setw(12) << std::setfill(' ')
         << f.VIS_IRMTFCorrectionE_W[i][j]   << " "
         << std::setw(12) << std::setfill(' ')
         << f.VIS_IRMTFCorrectionE_W[i][j+1] << " "
         << std::setw(12) << std::setfill(' ')
         << f.VIS_IRMTFCorrectionE_W[i][j+2] << " "
         << std::setw(12) << std::setfill(' ')
         << f.VIS_IRMTFCorrectionE_W[i][j+3]
         << std::endl;
    } 
  }
  for (int i = 0; i < 33; i ++)
  {
    os << "VIS IR MTF N/S " << std::setw(2) << std::setfill('0') << i+1
       << "   : " << std::endl;
    for(int j = 0; j < 16; j += 4)
    {
      os << std::setw(12) << std::setfill(' ')
         << f.VIS_IRMTFCorrectionN_S[i][j]   << " "
         << std::setw(12) << std::setfill(' ')
         << f.VIS_IRMTFCorrectionN_S[i][j+1] << " "
         << std::setw(12) << std::setfill(' ')
         << f.VIS_IRMTFCorrectionN_S[i][j+2] << " "
         << std::setw(12) << std::setfill(' ')
         << f.VIS_IRMTFCorrectionN_S[i][j+3]
         << std::endl;
    } 
  }
  for (int i = 0; i < 9; i ++)
  {
    os << "HRV MTF E/W " << std::setw(2) << std::setfill('0') << i+1
       << "      : " << std::endl;
    for(int j = 0; j < 16; j += 4)
    {
      os << std::setw(12) << std::setfill(' ')
         << f.HRVIRMTFCorrectionE_W[i][j]   << " "
         << std::setw(12) << std::setfill(' ')
         << f.HRVIRMTFCorrectionE_W[i][j+1] << " "
         << std::setw(12) << std::setfill(' ')
         << f.HRVIRMTFCorrectionE_W[i][j+2] << " "
         << std::setw(12) << std::setfill(' ')
         << f.HRVIRMTFCorrectionE_W[i][j+3]
         << std::endl;
    } 
  }
  for (int i = 0; i < 9; i ++)
  {
    os << "HRV MTF N/S " << std::setw(2) << std::setfill('0') << i+1
       << "      : " << std::endl;
    for(int j = 0; j < 16; j += 4)
    {
      os << std::setw(12) << std::setfill(' ')
         << f.HRVIRMTFCorrectionN_S[i][j]   << " "
         << std::setw(12) << std::setfill(' ')
         << f.HRVIRMTFCorrectionN_S[i][j+1] << " "
         << std::setw(12) << std::setfill(' ')
         << f.HRVIRMTFCorrectionN_S[i][j+2] << " "
         << std::setw(12) << std::setfill(' ')
         << f.HRVIRMTFCorrectionN_S[i][j+3]
         << std::endl;
    } 
  }
  return os;
}

size_t MSG_MPEFCalFeedback::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  for (int i = 0; i < 12; i ++)
    position += IMPF_CAL_Data[i].read_from(buff+position);
  return position;
}

std::ostream& operator<< (std::ostream& os, MSG_MPEFCalFeedback &f)
{
  for (int i = 0; i < 12; i ++)
    os << "CHANNEL " << std::setw(2) << std::setfill('0') << i+1 << std::endl
       << f.IMPF_CAL_Data[i];
  return os;
}

std::ostream& operator<< ( std::ostream& os, MSG_IMPF_CAL_Data &b )
{
  os << "Image Quality       : " << MSG_image_quality_flag(b.ImageQualityFlag)
     << std::endl
     << "Reference data      : " << MSG_reference_data_flag(b.ReferenceDataFlag)
     << std::endl
     << "Absolute Cal. Meth. : "
     << MSG_absolute_calibration_method(b.AbsCalMethod) << std::endl
     << "Abs Cal Weight Vic. : " << b.AbsCalWeightVic << std::endl
     << "Abs Cal Weight Xsat.: " << b.AbsCalWeightXsat << std::endl
     << "Abs Cal Coefficient : " << b.AbsCalCoeff << std::endl
     << "Abs Cal Error       : " << b.AbsCalError << std::endl
     << "Cal Mon Bias        : " << b.CalMonBias << std::endl
     << "Cal Mon RMS         : " << b.CalMonRms << std::endl
     << "Offset Count        : " << b.OffsetCount << std::endl;
  return os;
}

size_t MSG_BlackBodyDataUsed::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  position += BBObservationUTC.read_from(buff+position);
  position += OnBoardBBTime.read_from(buff+position);
  for (int i = 0; i < 42; i ++)
    MDUOutGain[i] = get_ui2(buff+position+i*2);
  position += 42*2;
  for (int i = 0; i < 42; i ++)
    MDUCoarseGain[i] = *(buff+position+i);
  position += 42;
  for (int i = 0; i < 42; i ++)
    MDUFineGain[i] = get_ui2(buff+position+i*2);
  position += 42*2;
  for (int i = 0; i < 42; i ++)
    MDUNumerical_Offset[i] = get_ui2(buff+position+i*2);
  position += 42*2;
  for (int i = 0; i < 42; i ++)
    PUGain[i] = get_ui2(buff+position+i*2);
  position += 42*2;
  for (int i = 0; i < 27; i ++)
    PUOffset[i] = get_ui2(buff+position+i*2);
  position += 27*2;
  for (int i = 0; i < 15; i ++)
    PUBias[i] = get_ui2(buff+position+i*2);
  position += 15*2;
  memcpy(DCRValues, buff, 63);
  position += 63;
  XDeepSpaceWindowPosition = (t_enum_MSG_DeepSpace_Window_Position)
                             *(buff+position);
  position += 1;
  FCUNominalColdFocalPlaneTemp = get_ui2(buff+position);
  position += 2;
  FCURedundantColdFocalPlaneTemp = get_ui2(buff+position);
  position += 2;
  FCUNominalWarmFocalPlaneVHROTemp = get_ui2(buff+position);
  position += 2;
  FCURedundantWarmFocalPlaneVHROTemp = get_ui2(buff+position);
  position += 2;
  FCUNominalScanMirrorSensor1Temp = get_ui2(buff+position);
  position += 2;
  FCURedundantScanMirrorSensor1Temp = get_ui2(buff+position);
  position += 2;
  FCUNominalScanMirrorSensor2Temp = get_ui2(buff+position);
  position += 2;
  FCURedundantScanMirrorSensor2Temp = get_ui2(buff+position);
  position += 2;
  FCUNominalM1MirrorSensor1Temp = get_ui2(buff+position);
  position += 2;
  FCURedundantM1MirrorSensor1Temp = get_ui2(buff+position);
  position += 2;
  FCUNominalM1MirrorSensor2Temp = get_ui2(buff+position);
  position += 2;
  FCURedundantM1MirrorSensor2Temp = get_ui2(buff+position);
  position += 2;
  FCUNominalM23AssemblySensor1Temp = *(buff+position);
  position += 1;
  FCURedundantM23AssemblySensor1Temp = *(buff+position);
  position += 1;
  FCUNominalM23AssemblySensor2Temp = *(buff+position);
  position += 1;
  FCURedundantM23AssemblySensor2Temp = *(buff+position);
  position += 1;
  FCUNominalM1BaffleTemp = get_ui2(buff+position);
  position += 2;
  FCURedundantM1BaffleTemp = get_ui2(buff+position);
  position += 2;
  FCUNominalBlackBodySensorTemp = get_ui2(buff+position);
  position += 2;
  FCURedundantBlackBodySensorTemp = get_ui2(buff+position);
  position += 2;
  memcpy((void *) &FCUNominalSMMStatus, buff+position, 2);
  position += 2;
  memcpy((void *) &FCURedundantSMMStatus, buff+position, 2);
  position += 2;
  for (int i = 0; i < 12; i ++)
    position += ExtractedBBData[i].read_from(buff+position);
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_BlackBodyDataUsed &b )
{
  os << "BB Observation UTC  : " << b.BBObservationUTC.get_timestring( )
     << std::endl
     << "On Board Time       : " << b.OnBoardBBTime.get_time_cuc_r8( )
     << std::endl;
  for (int i = 0; i < 42; i ++)
    os << "MDU Out Gain " << std::setw(2) << std::setfill('0') << i+1
       << "     : " << b.MDUOutGain[i] << std::endl;
  for (int i = 0; i < 42; i ++)
    os << "MDU Coarse Gain " << std::setw(2) << std::setfill('0') << i+1
       << "  : " << (uint_2) b.MDUCoarseGain[i] << std::endl;
  for (int i = 0; i < 42; i ++)
    os << "MDU Fine Gain " << std::setw(2) << std::setfill('0') << i+1
       << "    : " << b.MDUFineGain[i] << std::endl;
  for (int i = 0; i < 42; i ++)
    os << "MDU Num. Offset " << std::setw(2) << std::setfill('0') << i+1
       << "  : " << b.MDUNumerical_Offset[i] << std::endl;
  for (int i = 0; i < 42; i ++)
    os << "PU Gain " << std::setw(2) << std::setfill('0') << i+1
       << "          : " << b.PUGain[i] << std::endl;
  for (int i = 0; i < 27; i ++)
    os << "PU Offset " << std::setw(2) << std::setfill('0') << i+1
       << "       : " << b.PUOffset[i] << std::endl;
  for (int i = 0; i < 15; i ++)
    os << "PU Bias " << std::setw(2) << std::setfill('0') << i+1
       << "         : " << b.PUBias[i] << std::endl;
  for (int i = 0; i < 42; i ++)
    os << "DCR Values " << std::setw(2) << std::setfill('0') << i+1
       << "       : "
       << std::setw(3) << std::setfill('0') << std::hex << b.DCRValues[i].Value
       << std::dec << std::endl;
  os << "Deep Space Win      : "
     << MSG_DeepSpace_Window_Position(b.XDeepSpaceWindowPosition) << std::endl
     << "FCU Nom. CF Temp    : " << b.FCUNominalColdFocalPlaneTemp << std::endl
     << "FCU Red. CF Temp    : " << b.FCURedundantColdFocalPlaneTemp
     << std::endl
     << "FCU Nom. WF Temp    : " << b.FCUNominalWarmFocalPlaneVHROTemp
     << std::endl
     << "FCU Red. WF Temp    : " << b.FCURedundantWarmFocalPlaneVHROTemp
     << std::endl
     << "FCU Nom. SM S1 Temp : " << b.FCUNominalScanMirrorSensor1Temp
     << std::endl
     << "FCU Red. SM S1 Temp : " << b.FCURedundantScanMirrorSensor1Temp
     << std::endl
     << "FCU Nom. SM S2 Temp : " << b.FCUNominalScanMirrorSensor2Temp
     << std::endl
     << "FCU Red. SM S2 Temp : " << b.FCURedundantScanMirrorSensor2Temp
     << std::endl
     << "FCU Nom. M1 S1 Temp : " << b.FCUNominalM1MirrorSensor1Temp
     << std::endl
     << "FCU Red. M1 S1 Temp : " << b.FCURedundantM1MirrorSensor1Temp
     << std::endl
     << "FCU Nom. M1 S2 Temp : " << b.FCUNominalM1MirrorSensor2Temp
     << std::endl
     << "FCU Red. M1 S2 Temp : " << b.FCURedundantM1MirrorSensor2Temp
     << std::endl
     << "FCU Nom. M23 S1 T   : " << (uint_2) b.FCUNominalM23AssemblySensor1Temp
     << std::endl
     << "FCU Red. M23 S1 T   : "
     << (uint_2) b.FCURedundantM23AssemblySensor1Temp << std::endl
     << "FCU Nom. M23 S2 T   : " << (uint_2) b.FCUNominalM23AssemblySensor2Temp
     << std::endl
     << "FCU Red. M23 S2 T   : "
     << (uint_2) b.FCURedundantM23AssemblySensor2Temp << std::endl
     << "FCU Nom. M1 Baffle T: " << b.FCUNominalM1BaffleTemp << std::endl
     << "FCU Red. M1 Baffle T: " << b.FCURedundantM1BaffleTemp << std::endl
     << "FCU Nom. BB Sensor T: " << b.FCUNominalBlackBodySensorTemp
     << std::endl
     << "FCU Red. BB Sensor T: " << b.FCURedundantBlackBodySensorTemp
     << std::endl
     << "FCU Nom. SMM Status : " << b.FCUNominalSMMStatus.status << std::endl
     << "FCU Red. SMM Status : " << b.FCURedundantSMMStatus.status << std::endl;
  for (int i = 0; i < 12; i ++)
    os << "CHANNEL " << std::setw(2) << std::setfill('0') << i+1 << std::endl
       << b.ExtractedBBData[i];
  return os;
}

MSG_data_RadiometricProc::MSG_data_RadiometricProc( ) { }

MSG_data_RadiometricProc::MSG_data_RadiometricProc(unsigned const char_1 *buff)
{
  this->read_from(buff);
}

MSG_data_RadiometricProc::~MSG_data_RadiometricProc( ) { }

size_t MSG_data_RadiometricProc::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  position += RPSummary.read_from(buff+position);
  for (int i = 0; i < 12; i ++)
    position += ImageCalibration[i].read_from(buff+position);
  position += BlackBodyDataUsed.read_from(buff+position);
  position += MPEFCalFeedback.read_from(buff+position);
  for (int i = 0; i < 42; i ++)
    for (int j = 0; j < 64; j ++)
      RadTransform[i][j] = get_r4(buff+position+i*256+j*4);
  position += 42*64*4;
  position += RadProcMTFAdaptation.read_from(buff+position);
  for (int i = 0; i < 12; i ++)
    for (int j = 0; j < 8; j ++)
      for (int k = 0; k < 8; k ++)
        StraylightCorrection[i][j][k] = get_r4(buff+position+i*256+j*32+k*4);
  position += 12*8*8*4;
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_RadiometricProc &t )
{
  os << "------------------------------------------------------" << std::endl
     << "-          MSG DATA RADIOMETRIC PROCESSING           -" << std::endl
     << "------------------------------------------------------" << std::endl
     << t.RPSummary;
  os << std::endl << "IMAGE CALIBRATION" << std::endl;
  for (int i = 0; i < 12; i ++)
    os << "CHANNEL " << std::setw(2) << std::setfill('0') << i+1 << std::endl
       << t.ImageCalibration[i];
  os << "END CALIBRATION" << std::endl
     << t.BlackBodyDataUsed << t.MPEFCalFeedback;
  for (int i = 0; i < 42; i ++)
  {
    os << "Rad Transform " << std::setw(2) << std::setfill('0') << i+1
       << "    : " << std::endl;
    for(int j = 0; j < 64; j += 4)
    {
      os << std::setw(12) << std::setfill(' ') << t.RadTransform[i][j]   << " "
         << std::setw(12) << std::setfill(' ') << t.RadTransform[i][j+1] << " "
         << std::setw(12) << std::setfill(' ') << t.RadTransform[i][j+2] << " "
         << std::setw(12) << std::setfill(' ') << t.RadTransform[i][j+3]
         << std::endl;
    } 
  }
  os << t.RadProcMTFAdaptation;
  for (int i = 0; i < 12; i ++)
  {
    os << "Straylight Corr. " << std::setw(2) << std::setfill('0') << i+1
       << " : " << std::endl;
    for(int j = 0; j < 8; j ++)
    {
      for(int k = 0; k < 8; k += 8)
      {
        os << std::setw(12) << std::setfill(' ')
           << t.StraylightCorrection[i][j][k]   << " "
           << std::setw(12) << std::setfill(' ')
           << t.StraylightCorrection[i][j][k+1] << " "
           << std::setw(12) << std::setfill(' ')
           << t.StraylightCorrection[i][j][k+2] << " "
           << std::setw(12) << std::setfill(' ')
           << t.StraylightCorrection[i][j][k+3]
           << std::endl;
      } 
    } 
  }
  return os;
}

float *MSG_data_RadiometricProc::get_calibration(int channel, int bpp)
{
  float *calib;
  int ncal = (int) pow(2.0, bpp);

  calib = new float[ncal];
  memset(calib, 0, ncal*sizeof(float));

  if (channel > 3 && channel < 12)
  {
    const double cold_ccw[8] = { 2569.094, 1598.566, 1362.142, 1149.083,
                              //    4         5         6         7
                                 1034.345,  930.659,  839.661,  752.381 };
                              //    8         9        10        11
    const double cold_A[8]   = { 0.9959,  0.9963,  0.9991,  0.9996,
                              //    4        5        6        7
                                 0.9999,  0.9983,  0.9988,  0.9981 };
                              //    8        9       10       11
    const double cold_B[8] = { 3.471,   2.2219,  0.485,   0.181, 
                              //    4        5        6        7
                               0.060,   0.627,   0.397,   0.576  };
                              //    8        9       10       11
    const double c1 = 0.0000119104;
    const double c2 = 1.43877;
    double radiance = 0.0;
    double cenwave = cold_ccw[channel - 4];
    double c2nuc = cenwave * c2;
    double c1nuc3 = pow(cenwave, 3.0) * c1;
    double A = cold_A[channel - 4];
    double B = cold_B[channel - 4];

    for (int i = 0; i < ncal; i ++)
    {
      radiance = ((double) i * ImageCalibration[channel-1].Cal_Slope) + 
                  ImageCalibration[channel-1].Cal_Offset;
      calib[i] = (float) ((c2nuc / log(c1nuc3/radiance + 1.0) - B) / A);
    }
  }
  else
  {
    for (int i = 0; i < ncal; i ++)
    {
      calib[i] = ((double) i * ImageCalibration[channel-1].Cal_Slope) +
                 ImageCalibration[channel-1].Cal_Offset;
    }
  }
  
  return calib;
}

void MSG_data_RadiometricProc::get_slope_offset(int channel, double& slope, double& offset, bool& scalesToInt)
{
  if (channel > 3 && channel < 12)
  {
    const double cold_A[8]   = { 0.9959,  0.9963,  0.9991,  0.9996,
                              //    4        5        6        7
                                 0.9999,  0.9983,  0.9988,  0.9981 };
                              //    8        9       10       11
    const double cold_B[8] = { 3.471,   2.2219,  0.485,   0.181, 
                              //    4        5        6        7
                               0.060,   0.627,   0.397,   0.576  };
                              //    8        9       10       11
    slope = 1/cold_A[channel - 4];
    offset = -cold_B[channel - 4];
    scalesToInt = false;
  }
  else
  {
    slope = ImageCalibration[channel-1].Cal_Slope;
    offset = ImageCalibration[channel-1].Cal_Offset;
    scalesToInt = true;
  }
}

double jday(int yr, int month, int day)
{
  bool leap;
  double j = 0.0;

  if (((yr%4) == 0 && (yr%100) != 0) || (yr%400) == 0)
    leap = true;
  else
    leap = false;
  
  if (month == 1) j = 0.0;
  if (month == 2) j = 31.0;
  if (month == 3) j = 59.0;
  if (month == 4) j = 90.0;
  if (month == 5) j = 120.0;
  if (month == 6) j = 151.0;
  if (month == 7) j = 181.0;
  if (month == 8) j = 212.0;
  if (month == 9) j = 243.0;
  if (month == 10) j = 273.0;
  if (month == 11) j = 304.0;
  if (month == 12) j = 334.0;
  if (month > 2 && leap) j = j + 1.0;

  j = j + day;
  return j;
}

double cozena(double day, double hour, double dlat, double dlon)
{
  double coz;

  const double sinob = 0.3978;
  const double dpy = 365.242;
  const double dph = 15.0;
  
  double rpd = M_PI/180.0;

  double dpr = 1.0/rpd;
  double dang = 2.0*M_PI*(day-1.0)/dpy;
  double homp = 12.0 + 0.123570*sin(dang) - 0.004289*cos(dang) +
                0.153809*sin(2.0*dang) + 0.060783*cos(2.0*dang);
  double hang = dph* (hour-homp) - dlon;
  double ang = 279.9348*rpd + dang;
  double sigma = (ang*dpr+0.4087*sin(ang)+1.8724*cos(ang)-
                 0.0182*sin(2.0*ang)+0.0083*cos(2.0*ang))*rpd;
  double sindlt = sinob*sin(sigma);
  double cosdlt = sqrt(1.0-sindlt*sindlt);

  coz = sindlt*sin(rpd*dlat) + cosdlt*cos(rpd*dlat)*cos(rpd*hang);

  return coz;
}

double scan2zen(double scan, double satheight)
{
  const double rearth = 6357.0;
  double temp = (rearth + satheight)*(sin(scan)/rearth);

  if (temp > 1.0) temp = 1.0;
  return asin(temp);
}

double sza(int yr, int month, int day, int hour, int minute,
           float lat, float lon)
{
  double hourz = (float) hour + ((float) minute) / 60.0;
  double jd = jday(yr, month, day);
  double zenith;

  zenith = cozena(jd, hourz,(double) lat, (double) lon);
  return zenith;
}

float radiance_to_reflectance(int chnum, float radiance,
                              int year, int month, int day,
                              int hour, int minute,
                              float lat, float lon)
{
  if (chnum != 1 || chnum != 2 || chnum != 3 || chnum != 12)
  {
    std::cerr << "Wrong channel number : " << chnum << std::endl;
    throw;
  }

  double jd = jday(year, month, day);
  double esd = 1.0 - 0.0167 * cos( 2.0 * M_PI * (jd - 3) / 365.0);
  double oneoveresdsquare = 1.0 / (esd*esd);
  double torad[4] = { 20.76 * oneoveresdsquare, 23.24 * oneoveresdsquare,
                      19.85 * oneoveresdsquare, 25.11 * oneoveresdsquare };

  double tr = (chnum < 4) ? torad[chnum-1] : torad[3];
  return 100.0 * radiance / tr / sza(year, month, day, hour, minute, lat, lon);
}
