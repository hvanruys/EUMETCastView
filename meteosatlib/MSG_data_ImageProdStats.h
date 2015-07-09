//-----------------------------------------------------------------------------
//
//  File        : MSG_data_ImageProdStats.h
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

#ifndef __MSG_DATA_IMAGEPRODSTATS_H__
#define __MSG_DATA_IMAGEPRODSTATS_H__

#include <iostream>
#include "MSG_machine.h"
#include "MSG_time_cds.h"
#include "MSG_spacecraft.h"

#define MSG_IMAGE_PRODUCT_STATS_LEN 340

class MSG_ActualScanningSummary {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_ActualScanningSummary &s );

    uint_1             NominalImageScanning;
    uint_1             ReducedScan;
    MSG_time_cds_short ForwardScanStart;
    MSG_time_cds_short ForwardScanEnd;
};

class MSG_RadiometerBehaviour {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_RadiometerBehaviour &b );
    uint_1 NominalBehaviour;
    uint_1 RadScanIrregularity;
    uint_1 RadStoppage;
    uint_1 RepeatCycleNotCompleted;
    uint_1 GainChangeTookPlace;
    uint_1 DecontaminationTookPlace;
    uint_1 NoBBCalibrationAchieved;
    uint_1 IncorrectTemperature;
    uint_1 InvalidBBData;
    uint_1 InvalidAuxOrHKTMData;
    uint_1 RefocusingMechanismActuated;
    uint_1 MirrorBackToReferencePos;
};

class MSG_ReceptionSummaryStats {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_ReceptionSummaryStats &r );
    uint_4 PlannedNumberOfL10Lines[12];
    uint_4 NumberOfMissingL10Lines[12];
    uint_4 NumberOfCorruptedL10Lines[12];
    uint_4 NumberOfReplacedL10Lines[12];
};

class MSG_L15ImageValidity {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_L15ImageValidity &v );
    uint_1 NominalImage;
    uint_1 NonNominalBecauseIncomplete;
    uint_1 NonNominalRadiometricQuality;
    uint_1 NonNominalGeometricQuality;
    uint_1 NonNominalTimeliness;
    uint_1 IncompleteL15;
};

class MSG_ActualL15CoverageVIS_IR {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_ActualL15CoverageVIS_IR &a );
    int_4 SouthernLineActual;
    int_4 NorthernLineActual;
    int_4 EasternColumnActual;
    int_4 WesternColumnActual;
};

class MSG_ActualL15CoverageHRV {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_ActualL15CoverageHRV &a );
    int_4 LowerSouthLineActual;
    int_4 LowerNorthLineActual;
    int_4 LowerEastColumnActual;
    int_4 LowerWestColumnActual;
    int_4 UpperSouthLineActual;
    int_4 UpperNorthLineActual;
    int_4 UpperEastColumnActual;
    int_4 UpperWestColumnActual;
};

class MSG_data_ImageProdStats {
  public:
    MSG_data_ImageProdStats( );
    MSG_data_ImageProdStats( unsigned const char_1 *buff );
    ~MSG_data_ImageProdStats( );

    size_t read_from( unsigned const char_1 *buff );

    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_data_ImageProdStats &g );

    t_enum_MSG_spacecraft       SatelliteId;
    MSG_ActualScanningSummary   ActualScanningSummary;
    MSG_RadiometerBehaviour     RadiometerBehaviour;
    MSG_ReceptionSummaryStats   ReceptionSummaryStats;
    MSG_L15ImageValidity        L15ImageValidity[12];
    MSG_ActualL15CoverageVIS_IR ActualL15CoverageVIS_IR;
    MSG_ActualL15CoverageHRV    ActualL15CoverageHRV;
};

#endif
