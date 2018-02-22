//-----------------------------------------------------------------------------
//
//  File        : MSG_data_GeometricProc.h
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

#ifndef __MSG_DATA_GEOMETRICPROC_H__
#define __MSG_DATA_GEOMETRICPROC_H__

#include <iostream>
#include <string>
#include "MSG_machine.h"

#define MSG_GEOMETRIC_PROCESSING_LEN 17653

typedef enum {
  MSG_TYPE_OF_EARTH_NONE                       = 0,
  MSG_TYPE_OF_EARTH_ELLIPSOID_WITH_RPN_RPS_REQ = 1
} t_enum_MSG_type_of_earth_model;

std::string MSG_type_of_earth_model(t_enum_MSG_type_of_earth_model mdl);

typedef enum {
  MSG_RESAMPLING_FUNCTION_WINDOWED_SHANNON  = 1,
  MSG_RESAMPLING_FUNCTION_BICUBIC_SPLINES   = 2,
  MSG_RESAMPLING_FUNCTION_NEAREST_NEIGHBOUR = 3
} t_enum_MSG_resampling_function;

std::string MSG_resampling_function(t_enum_MSG_resampling_function fnc);

class MSG_OptAxisDistances {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< (std::ostream& os, MSG_OptAxisDistances &d);

    real_4 E_W_FocalPlane[42];
    real_4 N_S_FocalPlane[42];
};

class MSG_EarthModel {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< (std::ostream& os, MSG_EarthModel &e);

    t_enum_MSG_type_of_earth_model TypeofEarthModel;
    real_8                         EquatorialRadius;
    real_8                         NorthPolarRadius;
    real_8                         SouthPolarRadius;
};

class MSG_data_GeometricProc {
  public:
    MSG_data_GeometricProc( );
    MSG_data_GeometricProc( unsigned const char_1 *buff );
    ~MSG_data_GeometricProc( );

    size_t read_from( unsigned const char_1 *buff );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_data_GeometricProc &g );
    MSG_OptAxisDistances           OptAxisDistances;
    MSG_EarthModel                 EarthModel;
    real_4                         AtmosphericModel[12][360];
    t_enum_MSG_resampling_function ResamplingFunction[12];
};

#endif
