//-----------------------------------------------------------------------------
//
//  File        : MSG_data_SatelliteStatus.h
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

#ifndef __MSG_DATA_SATELLITESTATUS_H__
#define __MSG_DATA_SATELLITESTATUS_H__

#include <iostream>
#include <string>
#include "MSG_machine.h"
#include "MSG_time_cds.h"
#include "MSG_spacecraft.h"

typedef enum {
  MSG_SATELLITE_STATUS_OPERATIONAL        = 1,
  MSG_SATELLITE_STATUS_STANDBY            = 2,
  MSG_SATELLITE_STATUS_COMMISSIONING_TEST = 3,
  MSG_SATELLITE_STATUS_MANOUVRE           = 4,
  MSG_SATELLITE_STATUS_DECONTAMINATION    = 5,
  MSG_SATELLITE_STATUS_SAFE_MODE          = 6,
  MSG_SATELLITE_STATUS_DISSEMINATION_ONLY = 7
} t_enum_MSG_SatelliteStatus;

std::string MSG_satellite_status(t_enum_MSG_SatelliteStatus status);

typedef enum {
  MSG_SATELLITE_MANOUVRE_SPIN_UP             = 0,
  MSG_SATELLITE_MANOUVRE_SPIN_DOWN           = 1,
  MSG_SATELLITE_MANOUVRE_ATTITUDE_SLEW       = 2,
  MSG_SATELLITE_MANOUVRE_N_S_STATION_KEEPING = 3,
  MSG_SATELLITE_MANOUVRE_E_W_STATION_KEEPING = 4,
  MSG_SATELLITE_MANOUVRE_STATION_RELOCATION  = 5
} t_enum_MSG_ManouvreType;

std::string MSG_satellite_manouvre(t_enum_MSG_ManouvreType manouvre);

#define MSG_SATELLITE_STATUS_LEN  60134

class MSG_orbit_coefficient {
  public:
    MSG_orbit_coefficient( );
    size_t read_from( unsigned const char_1 *buff );
    bool is_present( );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_orbit_coefficient &c );
    MSG_time_cds_short StartTime;
    MSG_time_cds_short EndTime;
    real_8             X[8];
    real_8             Y[8];
    real_8             Z[8];
    real_8             VX[8];
    real_8             VY[8];
    real_8             VZ[8];
  private:
    bool present;
};

class MSG_Orbit {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os, MSG_Orbit &o );

    MSG_time_cds_short    PeriodStartTime;
    MSG_time_cds_short    PeriodEndTime;
    MSG_orbit_coefficient OrbitPoliniomal[100];
};

class MSG_AttitudeCoeff {
  public:
    MSG_AttitudeCoeff( );
    size_t read_from( unsigned const char_1 *buff );
    bool is_present( );
    friend std::ostream& operator<< ( std::ostream& os, MSG_AttitudeCoeff &a );

    MSG_time_cds_short StartTime;
    MSG_time_cds_short EndTime;
    real_8             XOfSpinAxis[8];
    real_8             YOfSpinAxis[8];
    real_8             ZOfSpinAxis[8];
  private:
    bool present;
};

class MSG_Attitude {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os, MSG_Attitude &a );

    MSG_time_cds_short  PeriodStartTime;
    MSG_time_cds_short  PeriodEndTime;
    real_8              PrincipalAxisOffsetAngle;
    MSG_AttitudeCoeff   AttitudePolynomial[100];
};

class MSG_UTCCorrelation {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os, MSG_UTCCorrelation &u );

    MSG_time_cds_short    PeriodStartTime;
    MSG_time_cds_short    PeriodEndTime;
    MSG_time_cuc          OnBoardTimeStart;
    real_8                VarOnBoardTimeStart;
    real_8                A1;
    real_8                VarA1;
    real_8                A2;
    real_8                VarA2;
};

class MSG_SatelliteDefinition {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_SatelliteDefinition &d );
    t_enum_MSG_spacecraft      SatelliteId;
    real_4                     NominalLongitude;
    t_enum_MSG_SatelliteStatus SatelliteStatus;
};

class MSG_SatelliteOperations {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_SatelliteOperations &o );
    bool                       LastManoeuvreFlag;
    MSG_time_cds_short         LastManouvreStartTime;
    MSG_time_cds_short         LastManouvreEndTime;
    t_enum_MSG_ManouvreType    LastManouvreType;
    bool                       NextManoeuvreFlag;
    MSG_time_cds_short         NextManouvreStartTime;
    MSG_time_cds_short         NextManouvreEndTime;
    t_enum_MSG_ManouvreType    NextManouvreType;
};

class MSG_data_SatelliteStatus {
  public:
    MSG_data_SatelliteStatus( );
    MSG_data_SatelliteStatus( unsigned const char_1 *buff );
    ~MSG_data_SatelliteStatus( );

    size_t read_from( unsigned const char_1 *buff );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_data_SatelliteStatus &g );

    MSG_SatelliteDefinition    SatelliteDefinition;
    MSG_SatelliteOperations    SatelliteOperations;
    MSG_Orbit                  Orbit;
    MSG_Attitude               Attitude;
    real_8                     SpinRateatRCStart;
    MSG_UTCCorrelation         UTCCorrelation;
};

#endif
