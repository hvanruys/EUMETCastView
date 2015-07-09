//-----------------------------------------------------------------------------
//
//  File        : MSG_data_IMPFConfiguration.h
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

#ifndef __MSG_DATA_IMPFCONFIGURATION_H__
#define __MSG_DATA_IMPFCONFIGURATION_H__

#include <iostream>
#include <string>
#include "MSG_machine.h"
#include "MSG_time_cds.h"

#define MSG_IMPF_CONFIGURATION_LEN 19786

typedef enum {
  MSG_SU_SW_INSTANCE_M_C           = 1,
  MSG_SU_ACCEPT_DATA_SEVIRI        = 2,
  MSG_SU_ACCEPT_DATA_HKTM          = 3,
  MSG_SU_ACCEPT_DATA_GERB          = 4,
  MSG_SU_ACCEPT_DATA_FTP_SERVER    = 5,
  MSG_SU_RCAL_GCAL                 = 6,
  MSG_SU_RQA                       = 7,
  MSG_SU_GQA                       = 8,
  MSG_SU_REQUANTIZE_RESAMPLE_VIS06 = 9,
  MSG_SU_REQUANTIZE_RESAMPLE_VIS08 = 10,
  MSG_SU_REQUANTIZE_RESAMPLE_IR16  = 11,
  MSG_SU_REQUANTIZE_RESAMPLE_IR38  = 12,
  MSG_SU_REQUANTIZE_RESAMPLE_WV62  = 13,
  MSG_SU_REQUANTIZE_RESAMPLE_WV73  = 14,
  MSG_SU_REQUANTIZE_RESAMPLE_IR87  = 15,
  MSG_SU_REQUANTIZE_RESAMPLE_IR97  = 16,
  MSG_SU_REQUANTIZE_RESAMPLE_IR108 = 17,
  MSG_SU_REQUANTIZE_RESAMPLE_IR120 = 18,
  MSG_SU_REQUANTIZE_RESAMPLE_IR134 = 19,
  MSG_SU_REQUANTIZE_RESAMPLE_HRV   = 20,
  MSG_SU_PRODUCE_QUICKLOOK_DATA    = 21,
  MSG_SU_SEND_DATA_SEVIRI_LPT0     = 22,
  MSG_SU_SEND_DATA_SEVIRI_LPT5     = 23,
  MSG_SU_SEND_DATA_HKTM            = 24,
  MSG_SU_SEND_DATA_RAW_GERB        = 25,
  MSG_SU_SEND_DATA_VALIDATED_GERB  = 26,
  MSG_SU_SEND_DATA_GENERATED_FD    = 27,
  MSG_SU_SEND_FTP_CLIENT_PUT       = 28,
  MSG_SU_OBJECT_STORE_SERVER       = 29
} t_enum_MSG_SU_instance;

std::string MSG_SU_Instance(t_enum_MSG_SU_instance inst);
std::string MSG_SU_ID(uint_4 SUId);

typedef enum {
  MSG_SU_MODE_OFF                     = 0,
  MSG_SU_MODE_ON_NON_PROCESSING       = 1,
  MSG_SU_MODE_ON_REAL_TIME_PROCESSING = 2,
  MSG_SU_MODE_ON_ANALYSIS_MODE        = 3
} t_enum_MSG_SUMode;

std::string MSG_SUMode(t_enum_MSG_SUMode mod);

typedef enum {
  MSG_SU_STATE_ERROR    = 0,
  MSG_SU_STATE_NOMINAL  = 1,
  MSG_SU_STATE_DEGRADED = 2
} t_enum_MSG_SUState;

std::string MSG_SUState(t_enum_MSG_SUState sta);

class MSG_GPConfigItemVersion {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_GPConfigItemVersion &v );
    uint_2 Issue;
    uint_2 Revision;
};

class MSG_MirrorParameters {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_MirrorParameters &p );
    real_8 MaxFeedbackVoltage;
    real_8 MinFeedbackVoltage;
    real_8 MirrorSlipEstimate;
};

class MSG_HKTMParameters {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_HKTMParameters &p );
    MSG_time_cds_short TimeS0Packet;
    MSG_time_cds_short TimeS1Packet;
    MSG_time_cds_short TimeS2Packet;
    MSG_time_cds_short TimeS3Packet;
    MSG_time_cds_short TimeS4Packet;
    MSG_time_cds_short TimeS5Packet;
    MSG_time_cds_short TimeS6Packet;
    MSG_time_cds_short TimeS7Packet;
    MSG_time_cds_short TimeS8Packet;
    MSG_time_cds_short TimeS9Packet;
    MSG_time_cds_short TimeSYPacket;
    MSG_time_cds_short TimePSPacket;
    uint_1       WSPReserved[3408]; // ?? 426
};

class MSG_SUConfiguration {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< (std::ostream& os, MSG_SUConfiguration &g);

    MSG_GPConfigItemVersion SWVersion;
    MSG_GPConfigItemVersion InfoBaseVersion[10];
};

class MSG_SUDetail {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os, MSG_SUDetail &d );

    uint_4                 SoftwareUnitId;
    t_enum_MSG_SU_instance SUIdInstance;
    t_enum_MSG_SUMode      SUMode;
    t_enum_MSG_SUState     SUState;
    MSG_SUConfiguration    SUConfiguration;
}; 

class MSG_EqualisationParms {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_EqualisationParms &e );
    real_4 ConstCoef;
    real_4 LinearCoef;
    real_4 QuadraticCoef;
};

class MSG_BlackBodyDataWarmStart {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_BlackBodyDataWarmStart &b );
    real_8                 GTotalForMethod1[12];
    real_8                 GTotalForMethod2[12];
    real_8                 GTotalForMethod3[12];
    real_8                 GBackForMethod1[12];
    real_8                 GBackForMethod2[12];
    real_8                 GBackForMethod3[12];
    real_8                 RatioGTotalToGBack[12];
    real_8                 GainInFrontOpticsCont[12];
    real_4                 CalibrationConstants[12];
    real_8                 TimeOfColdObsSeconds;
    real_8                 TimeOfColdObsNanoSecs;
    real_8                 IncidentRadiance[12];
    real_8                 TempCal;
    real_8                 TempM1;
    real_8                 TempScan;
    real_8                 TempM1Baf;
    real_8                 TempCalSurround;
    MSG_MirrorParameters   MirrorParameters;
    real_8                 LastSpinPeriod;
    MSG_HKTMParameters     HKTMParameters;
};

class MSG_WarmStartParms {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os, MSG_WarmStartParms &w );

    real_8                     ScanningLaw[1527];
    real_8                     RadFramesAlignment[3];
    real_4                     ScanningLawVariation[2];
    MSG_EqualisationParms      EqualisationParms[42];
    MSG_BlackBodyDataWarmStart BlackBodyDataWarmStart;
};

class MSG_data_IMPFConfiguration {
  public:
    MSG_data_IMPFConfiguration( );
    MSG_data_IMPFConfiguration( unsigned const char_1 *buff );
    ~MSG_data_IMPFConfiguration( );

    size_t read_from( unsigned const char_1 *buff );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_data_IMPFConfiguration &r );

    MSG_GPConfigItemVersion GPConfigItemVersion;
    MSG_SUDetail            SuDetails[50];
    MSG_WarmStartParms      WarmStartParms;
};

#endif
