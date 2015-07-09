//-----------------------------------------------------------------------------
//
//  File        : MSG_data_GeometricQuality.h
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

#ifndef __MSG_DATA_GEOMETRICQUALITY_H__
#define __MSG_DATA_GEOMETRICQUALITY_H__

#include <iostream>
#include <string>
#include "MSG_machine.h"

#define MSG_GEOMETRIC_QUALITY_LEN 2916

typedef enum {
  MSG_QUALITY_INFO_VALIDITY_NOT_DERIVED         = 0,
  MSG_QUALITY_INFO_VALIDITY_DERIVED_AND_VALID   = 1,
  MSG_QUALITY_INFO_VALIDITY_DERIVED_AND_INVALID = 2,
  MSG_QUALITY_INFO_VALIDITY_ESTIMATED           = 3
} t_enum_MSG_quality_info_validity;

std::string MSG_quality_info_validity(t_enum_MSG_quality_info_validity qiv);

class MSG_Accuracy {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_Accuracy &a );
    t_enum_MSG_quality_info_validity QualityInfoValidity;
    real_4                           EastWestAccuracyRMS;
    real_4                           NorthSouthAccuracyRMS;
    real_4                           MagnitudeRMS;
    real_4                           EastWestUncertaintyRMS;
    real_4                           NorthSouthUncertaintyRMS;
    real_4                           MagnitudeUncertaintyRMS;
    real_4                           EastWestMaxDeviation;
    real_4                           NorthSouthMaxDeviation;
    real_4                           MagnitudeMaxDeviation;
    real_4                           EastWestUncertaintyMax;
    real_4                           NorthSouthUncertaintyMax;
    real_4                           MagnitudeUncertaintyMax;
};

class MSG_Residuals {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_Residuals &a );
    t_enum_MSG_quality_info_validity QualityInfoValidity;
    real_4                           EastWestResiduals;
    real_4                           NorthSouthResiduals;
    real_4                           EastWestUncertainty;
    real_4                           NorthSouthUncertainty;
    real_4                           EastWestRMS;
    real_4                           NorthSouthRMS;
    real_4                           EastWestMagnitude;
    real_4                           NorthSouthMagnitude;
    real_4                           EastWestMagnitudeUncertainty;
    real_4                           NorthSouthMagnitudeUncertainty;
};

class MSG_GeometricQualityStatus {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_GeometricQualityStatus &s );
    uint_1 QualityNominal;
    uint_1 NominalAbsolute;
    uint_1 NominalRelativeToPreviousImage;
    uint_1 NominalForREL500;
    uint_1 NominalForREL16;
    uint_1 NominalForResMisreg;
};

class MSG_data_GeometricQuality {
  public:
    MSG_data_GeometricQuality( );
    MSG_data_GeometricQuality( unsigned const char_1 *buff );
    ~MSG_data_GeometricQuality( );

    size_t read_from( unsigned const char_1 *buff );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_data_GeometricQuality &r );

    MSG_Accuracy               AbsoluteAccuracy[12];
    MSG_Accuracy               RelativeAccuracy[12];
    MSG_Accuracy               N500PixelsRelativeAccuracy[12];
    MSG_Accuracy               N16PixelsRelativeAccuracy[12];
    MSG_Residuals              MisregistrationResiduals[12];
    MSG_GeometricQualityStatus GeometricQualityStatus[12];
};

#endif
