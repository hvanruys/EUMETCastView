//-----------------------------------------------------------------------------
//
//  File        : MSG_data_NavigExtrResult.h
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

#ifndef __MSG_DATA_NAVIGEXTRRESULT_H__
#define __MSG_DATA_NAVIGEXTRRESULT_H__

#include <iostream>
#include <string>
#include "MSG_machine.h"
#include "MSG_time_cds.h"

#define MSG_NAVIGATION_EXTR_RESULT_LEN 5680

typedef enum {
  MSG_HORIZON_ID_SOUTH = 0,
  MSG_HORIZON_ID_NORTH = 1,
  MSG_HORIZON_ID_EAST  = 2,
  MSG_HORIZON_ID_WEST  = 3
} t_enum_MSG_horizon_id;

std::string MSG_horizon_id(t_enum_MSG_horizon_id id);

class MSG_HorizonObservation {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_HorizonObservation &o );
    t_enum_MSG_horizon_id HorizonId;
    real_8       Alpha;
    real_8       AlphaConfidence;
    real_8       Beta;
    real_8       BetaConfidence;
    MSG_time_cds ObservationTime;
    real_8       SpinRate;
    real_8       AlphaDeviation;
    real_8       BetaDeviation;
};

class MSG_StarObservation {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_StarObservation &o );
    uint_2       StarId;
    real_8       Alpha;
    real_8       AlphaConfidence;
    real_8       Beta;
    real_8       BetaConfidence;
    MSG_time_cds ObservationTime;
    real_8       SpinRate;
    real_8       AlphaDeviation;
    real_8       BetaDeviation;
};

class MSG_LandmarkObservation {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_LandmarkObservation &o );
    uint_2       LandmarkId;
    real_8       LandmarkLongitude;
    real_8       LandmarkLatitude;
    real_8       Alpha;
    real_8       AlphaConfidence;
    real_8       Beta;
    real_8       BetaConfidence;
    MSG_time_cds ObservationTime;
    real_8       SpinRate;
    real_8       AlphaDeviation;
    real_8       BetaDeviation;
};

class MSG_data_NavigExtrResult {
  public:
    MSG_data_NavigExtrResult( );
    MSG_data_NavigExtrResult( unsigned const char_1 *buff );
    ~MSG_data_NavigExtrResult( );

    size_t read_from( unsigned const char_1 *buff );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_data_NavigExtrResult &r );

    MSG_HorizonObservation  ExtractedHorizons[4];
    MSG_StarObservation     ExtractedStars[20];   
    MSG_LandmarkObservation ExtractedLandmarks[50];
};

#endif
