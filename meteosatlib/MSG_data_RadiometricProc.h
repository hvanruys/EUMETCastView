//-----------------------------------------------------------------------------
//
//  File        : MSG_data_RadiometricProc.h
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

#ifndef __MSG_DATA_RADIOMETRICPROC_H__
#define __MSG_DATA_RADIOMETRICPROC_H__

#include <iostream>
#include "MSG_machine.h"
#include "MSG_time_cds.h"
#include "MSG_data_ImageAcquisition.h"

#define MSG_DATA_RADIOMETRIC_PROC_LEN 20815

class MSG_Image_Calibration {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_Image_Calibration &c );
    real_8 Cal_Slope;
    real_8 Cal_Offset;
};

class MSG_ExtractedBBData {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_ExtractedBBData &b );
  private:
    uint_4 NumberOfPixelsUsed;
    real_4 MeanCount;
    real_4 RMS;
    uint_2 MaxCount;
    uint_2 MinCount;
    real_8 BB_Processing_Slope;
    real_8 BB_Processing_Offset;
};

typedef enum {
  MSG_IMPF_CAL_OK           = 0,
  MSG_IMPF_CAL_DUBIOUS      = 1,
  MSH_IMPF_CAL_USE_MPEF_CAL = 2
} t_enum_MSG_image_quality_flag;

std::string MSG_image_quality_flag(t_enum_MSG_image_quality_flag iqf);

typedef enum {
  MSG_REFERENCE_DATA_NOT_CALCULATED = 0,
  MSG_REFERENCE_DATA_MET_DATA       = 1,
  MSG_REFERENCE_DATA_FSD            = 2,
  MSG_REFERENCE_DATA_MIXED_MET_FSD  = 3
} t_enum_MSG_reference_data_flag;

std::string MSG_reference_data_flag(t_enum_MSG_reference_data_flag rd);

typedef enum {
  MSG_ABSOLUTE_CALIBRATION_NOT_CHANGED     = 0,
  MSG_ABSOLUTE_CALIBRATION_EXTERNAL        = 1,
  MSG_ABSOLUTE_CALIBRATION_VICARIOUS       = 2,
  MSG_ABSOLUTE_CALIBRATION_CROSS_SATELLITE = 3,
  MSG_ABSOLUTE_CALIBRATION_MIXED_VIC_XSAT  = 4
} t_enum_MSG_absolute_calibration_method;

std::string MSG_absolute_calibration_method(
                          t_enum_MSG_absolute_calibration_method acm);

class MSG_IMPF_CAL_Data {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_IMPF_CAL_Data &b );
  private:
    t_enum_MSG_image_quality_flag          ImageQualityFlag;
    t_enum_MSG_reference_data_flag         ReferenceDataFlag;
    t_enum_MSG_absolute_calibration_method AbsCalMethod;
    // Skip 1 byte
    real_4                                 AbsCalWeightVic;
    real_4                                 AbsCalWeightXsat;
    real_4                                 AbsCalCoeff;
    real_4                                 AbsCalError;
    real_4                                 CalMonBias;
    real_4                                 CalMonRms;
    real_4                                 OffsetCount;
};

#if WIN32
typedef struct
{
  uint_2 Value:12;
} t_MSG_DCR_Value;

typedef struct
{
  uint_2 status:16;
} t_MSG_SMMStatus;
#else
typedef struct {
  uint_2 Value:12;
} __attribute__((packed)) t_MSG_DCR_Value;

typedef struct {
  uint_2 status:16;
} __attribute__((packed)) t_MSG_SMMStatus;
#endif

class MSG_BlackBodyDataUsed {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_BlackBodyDataUsed &b );
  private:
    MSG_time_cds_expanded                BBObservationUTC;
    MSG_time_cuc                         OnBoardBBTime;
    uint_2                               MDUOutGain[42];
    uint_1                               MDUCoarseGain[42];
    uint_2                               MDUFineGain[42];
    uint_2                               MDUNumerical_Offset[42];
    uint_2                               PUGain[42];
    uint_2                               PUOffset[27];
    uint_2                               PUBias[15];
    t_MSG_DCR_Value                      DCRValues[12];
    t_enum_MSG_DeepSpace_Window_Position XDeepSpaceWindowPosition;
    uint_2                               FCUNominalColdFocalPlaneTemp;
    uint_2                               FCURedundantColdFocalPlaneTemp;
    uint_2                               FCUNominalWarmFocalPlaneVHROTemp;
    uint_2                               FCURedundantWarmFocalPlaneVHROTemp;
    uint_2                               FCUNominalScanMirrorSensor1Temp;
    uint_2                               FCURedundantScanMirrorSensor1Temp;
    uint_2                               FCUNominalScanMirrorSensor2Temp;
    uint_2                               FCURedundantScanMirrorSensor2Temp;
    uint_2                               FCUNominalM1MirrorSensor1Temp;
    uint_2                               FCURedundantM1MirrorSensor1Temp;
    uint_2                               FCUNominalM1MirrorSensor2Temp;
    uint_2                               FCURedundantM1MirrorSensor2Temp;
    uint_1                               FCUNominalM23AssemblySensor1Temp;
    uint_1                               FCURedundantM23AssemblySensor1Temp;
    uint_1                               FCUNominalM23AssemblySensor2Temp;
    uint_1                               FCURedundantM23AssemblySensor2Temp;
    uint_2                               FCUNominalM1BaffleTemp;
    uint_2                               FCURedundantM1BaffleTemp;
    uint_2                               FCUNominalBlackBodySensorTemp;
    uint_2                               FCURedundantBlackBodySensorTemp;
    t_MSG_SMMStatus                      FCUNominalSMMStatus;
    t_MSG_SMMStatus                      FCURedundantSMMStatus;
    MSG_ExtractedBBData                  ExtractedBBData[12];
};

class MSG_RadProcMTFAdaptation {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< (std::ostream& os,
                                     MSG_RadProcMTFAdaptation &f);

    real_4 VIS_IRMTFCorrectionE_W[33][16];
    real_4 VIS_IRMTFCorrectionN_S[33][16];
    real_4 HRVIRMTFCorrectionE_W[9][16];
    real_4 HRVIRMTFCorrectionN_S[9][16];
};

class MSG_MPEFCalFeedback {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< (std::ostream& os, MSG_MPEFCalFeedback &f);

    MSG_IMPF_CAL_Data IMPF_CAL_Data[12];
};

class MSG_RPSummary {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os, MSG_RPSummary &s );

    bool RadianceLinearization[12];
    bool DetectorEqualization[12];
    bool OnboardCalibrationResult[12];
    bool MPEFCalFeedBack[12];
    bool MTFAdaptation[12];
    bool StraylightCorrectionFlag[12];
};

class MSG_data_RadiometricProc {
  public:
    MSG_data_RadiometricProc( );
    MSG_data_RadiometricProc( unsigned const char_1 *buff );
    ~MSG_data_RadiometricProc( );

    size_t read_from( unsigned const char_1 *buff );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_data_RadiometricProc &g );

    float *get_calibration(int channel, int bpp);
    void get_slope_offset(int channel, double& slope, double& offset, bool& scalesToInt);

    MSG_RPSummary            RPSummary;
    MSG_Image_Calibration    ImageCalibration[12];
    MSG_BlackBodyDataUsed    BlackBodyDataUsed;
    MSG_MPEFCalFeedback      MPEFCalFeedback;
    real_4                   RadTransform[42][64];
    MSG_RadProcMTFAdaptation RadProcMTFAdaptation;
    real_4                   StraylightCorrection[12][8][8];
};

double cozena(double day, double hour, double lat, double lon);
double jday(int yr, int month, int day);
double scan2zen(double scan, double satheight);

float radiance_to_reflectance(int chnum, float radiance,
                              int year, int month, int day,
                              int hour, int minute,
                              float lat, float lon);

#endif
