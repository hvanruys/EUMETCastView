//-----------------------------------------------------------------------------
//
//  File        : MSG_data_RadiometricQuality.h
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

#ifndef __MSG_DATA_RADIOMETRICQUALITY_H__
#define __MSG_DATA_RADIOMETRICQUALITY_H__

#include <iostream>
#include "MSG_machine.h"

#define MSG_RADIOMETRIC_QUALITY_LEN 371256

class MSG_L10RadQuality {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os, MSG_L10RadQuality &q );

    uint_2 FullImageMinimumCount;
    uint_2 FullImageMaximumCount;
    uint_2 EarthDiskMinimumCount;
    uint_2 EarthDiskMaximumCount;
    uint_2 MoonMinimumCount;
    uint_2 MoonMaximumCount;
    real_4 FullImageMeanCount;
    real_4 FullImageStandardDeviation;
    real_4 EarthDiskMeanCount;
    real_4 EarthDiskStandardDeviation;
    real_4 MoonMeanCount;
    real_4 MoonStandardDeviation;
    real_4 SpaceMeanCount;
    real_4 SpaceStandardDeviation;
    real_4 SESpaceCornerMeanCount;
    real_4 SESpaceCornerStandardDeviation;
    real_4 SWSpaceCornerMeanCount;
    real_4 SWSpaceCornerStandardDeviation;
    real_4 NESpaceCornerMeanCount;
    real_4 NESpaceCornerStandardDeviation;
    real_4 NWSpaceCornerMeanCount;
    real_4 NWSpaceCornerStandardDeviation;
    real_4 FourSpaceCornersMeanCount;
    real_4 FourSpaceCornersStandardDeviation;
    uint_4 FullImageIstogram[256];
    uint_4 EarthDiskIstogram[256];
    uint_4 ImageCentreSquareIstogram[256];
    uint_4 SESpaceCornerIstogram[128];
    uint_4 SWSpaceCornerIstogram[128];
    uint_4 NESpaceCornerIstogram[128];
    uint_4 NWSpaceCornerIstogram[128];
    real_4 FullImageEntropy[3];
    real_4 EarthDiskEntropy[3];
    real_4 ImageCentreSquareEntropy[3];
    real_4 SESpaceCornerEntropy[3];
    real_4 SWSpaceCornerEntropy[3];
    real_4 NESpaceCornerEntropy[3];
    real_4 NWSpaceCornerEntropy[3];
    real_4 FourSpaceCornersEntropy[3];
    real_4 ImageCentreSquarePSD_EW[128];
    real_4 FullImagePSD_EW[128];
    real_4 ImageCentreSquarePSD_NS[128];
    real_4 FullImagePSD_NS[128];
};

class MSG_L15RadQuality {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os, MSG_L15RadQuality &q );

    uint_2 FullImageMinimumCount;
    uint_2 FullImageMaximumCount;
    uint_2 EarthDiskMinimumCount;
    uint_2 EarthDiskMaximumCount;
    real_4 FullImageMeanCount;
    real_4 FullImageStandardDeviation;
    real_4 EarthDiskMeanCount;
    real_4 EarthDiskStandardDeviation;
    real_4 SpaceMeanCount;
    real_4 SpaceStandardDeviation;
    uint_4 FullImageIstogram[256];
    uint_4 EarthDiskIstogram[256];
    uint_4 ImageCentreSquareIstogram[256];
    real_4 FullImageEntropy[3];
    real_4 EarthDiskEntropy[3];
    real_4 ImageCentreSquareEntropy[3];
    real_4 ImageCentreSquarePSD_EW[128];
    real_4 FullImagePSD_EW[128];
    real_4 ImageCentreSquarePSD_NS[128];
    real_4 FullImagePSD_NS[128];
    real_4 SESpaceCornerL15_RMS;
    real_4 SESpaceCornerL15_Mean;
    real_4 SWSpaceCornerL15_RMS;
    real_4 SWSpaceCornerL15_Mean;
    real_4 NESpaceCornerL15_RMS;
    real_4 NESpaceCornerL15_Mean;
    real_4 NWSpaceCornerL15_RMS;
    real_4 NWSpaceCornerL15_Mean;
};

class MSG_data_RadiometricQuality {
  public:
    MSG_data_RadiometricQuality( );
    MSG_data_RadiometricQuality( unsigned const char_1 *buff );
    ~MSG_data_RadiometricQuality( );

    size_t read_from( unsigned const char_1 *buff );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_data_RadiometricQuality &r );

    MSG_L10RadQuality L10RadQuality[42];
    MSG_L15RadQuality L15RadQuality[12];
};

uint_4 maxistogram(uint_4 *istogram, int_4 len);

#endif
