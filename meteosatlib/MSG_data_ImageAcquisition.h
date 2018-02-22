//-----------------------------------------------------------------------------
//
//  File        : MSG_data_ImageAcquisition.h
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

#ifndef __MSG_DATA_IMAGE_AQUISITION_H__
#define __MSG_DATA_IMAGE_AQUISITION_H__

#include <iostream>
#include <string>
#include "MSG_machine.h"
#include "MSG_time_cds.h"

#define MSG_IMAGE_ACQUISITION_LEN 700

typedef enum {
  MSG_BOOLEAN_STATUS_OFF = 0,
  MSG_BOOLEAN_STATUS_ON  = 1
} t_enum_MSG_boolean_status;

std::string MSG_boolean_status(t_enum_MSG_boolean_status stat);

typedef enum {
  MSG_DHS_SYNC_SUN         = 0,
  MSG_DHS_SYNC_EARTH_NORTH = 1,
  MSG_DHS_SYNC_EARTH_SOUTH = 2
} t_enum_MSG_DHS_Sync;

std::string MSG_DHS_Sync( t_enum_MSG_DHS_Sync sync );

typedef enum {
  MSG_DEEP_SPACE_WINDOW_NO_DELAY = 0,
  MSG_DEEP_SPACE_WINDOW_DELAY_1  = 1,
  MSG_DEEP_SPACE_WINDOW_DELAY_2  = 2,
  MSG_DEEP_SPACE_WINDOW_DELAY_3  = 3
} t_enum_MSG_DeepSpace_Window_Position;

std::string MSG_DeepSpace_Window_Position(
        t_enum_MSG_DeepSpace_Window_Position pos);

typedef enum {
  MSG_REFOCUSING_DIRECTION_UP   = 0,
  MSG_REFOCUSING_DIRECTION_DOWN = 1
} t_enum_MSG_Refocusing_Direction;

std::string MSG_Refocusing_Direction(t_enum_MSG_Refocusing_Direction dir);

typedef enum {
  MSG_BB_CALIBRATION_HOT           = 0,
  MSG_BB_CALIBRATION_AMBIENT       = 1,
  MSG_BB_CALIBRATION_INDETERMINATE = 2
} t_enum_MSG_BB_Calibration;

std::string MSG_BB_Calibration(t_enum_MSG_BB_Calibration cal);

class MSG_PlannedAquisitionTime {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_PlannedAquisitionTime &t );

    MSG_time_cds_expanded TrueRepeatCycleStart;
    MSG_time_cds_expanded PlannedForwardScanEnd;
    MSG_time_cds_expanded PlannedRepeatCycleEnd;
};

class MSG_RadiometerStatus {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_RadiometerStatus &s );
    t_enum_MSG_boolean_status ChannelStatus[12];
    t_enum_MSG_boolean_status DetectorStatus[42];
};

class MSG_RadiometerSettings {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_RadiometerSettings &s );
    uint_2                               MDUSamplingDelays[42];
    uint_2                               MDUNomHRVDelay1;
    uint_2                               MDUNomHRVDelay2;
    uint_2                               MDUNomHRVBreakline;
    t_enum_MSG_DHS_Sync                  DHSSyncSelection;
    uint_2                               MDUOutGain[42];
    uint_1                               MDUCoarseGain[42];
    uint_2                               MDUFineGain[42];
    uint_2                               MDUNumericalOffset[42];
    uint_2                               PUGain[42];
    uint_2                               PUOffset[27];
    uint_2                               PUBias[15];
    uint_2                               L0_LineCounter;
    uint_2                               K1_RetraceLines;
    uint_2                               K2_PauseDeciseconds;
    uint_2                               K3_RetraceLines;
    uint_2                               K4_PauseDeciseconds;
    uint_2                               K5_RetraceLines;
    t_enum_MSG_DeepSpace_Window_Position X_DeepSpaceWindowPosition;
    uint_2                               RefocusingLines;
    t_enum_MSG_Refocusing_Direction      RefocusingDirection;
    uint_2                               RefocusingPosition;
    bool                                 ScanRefPosFlag;
    uint_2                               ScanRefPosNumber;
    real_4                               ScanRefPotVal;
    uint_2                               ScanFirstLine;
    uint_2                               ScanLastLine;
    uint_2                               RetraceStartLine;
};

class MSG_RadiometerOperations {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_RadiometerOperations &s );
    bool                      LastGainChangeFlag;
    MSG_time_cds_short        LastGainChangeTime;
    bool                      DecontaminationNow;
    MSG_time_cds_short        DecontaminationStart;
    MSG_time_cds_short        DecontaminationEnd;
    bool                      BBCalScheduled;
    t_enum_MSG_BB_Calibration BBCalibrationType;
    uint_2                    BBFirstLine;
    uint_2                    BBLastLine;
    uint_2                    ColdFocalPlaneOpTemp;
    uint_2                    WarmFocalPlaneOpTemp;
};

class MSG_data_ImageAcquisition {
  public:
    MSG_data_ImageAcquisition( );
    MSG_data_ImageAcquisition( unsigned const char_1 *buff );
    ~MSG_data_ImageAcquisition( );

    size_t read_from( unsigned const char_1 *buff );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_data_ImageAcquisition &g );

    MSG_PlannedAquisitionTime PlannedAquisitionTime;
    MSG_RadiometerStatus      RadiometerStatus;
    MSG_RadiometerSettings    RadiometerSettings;
    MSG_RadiometerOperations  RadiometerOperations;
};

#endif
