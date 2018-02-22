//-----------------------------------------------------------------------------
//
//  File        : MSG_data_SGS_header.h
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

#ifndef __MSG_DATA_SGS_HEADER_H__
#define __MSG_DATA_SGS_HEADER_H__

#include <iostream>
#include <string>
#include "MSG_machine.h"
#include "MSG_time_cds.h"

class MSG_SGSCommonHeader {
  public:
    MSG_SGSCommonHeader( );
    MSG_SGSCommonHeader(unsigned const char_1 *buff);
    ~MSG_SGSCommonHeader( );

    size_t read_from( unsigned const char_1 *buff );

    // Overloaded << operator
    friend std::ostream& operator<< (std::ostream& os,
                                     MSG_SGSCommonHeader &h);

    MSG_time_cds_short NominalSGSProductTime;
    uint_1             SGSProductQuality;
    uint_1             SGSProductCompleteness;
    uint_1             SGSProductTimeliness;
    uint_1             SGSProcessingInstanceId;
    char_1             BaseAlgorithmVersion[16];
    char_1             ProductAlgorithmVersion[16];
};

class MSG_SGSImageProductSpecificHeader {
  public:
    MSG_SGSImageProductSpecificHeader( );
    MSG_SGSImageProductSpecificHeader(unsigned const char_1 *buff);
    ~MSG_SGSImageProductSpecificHeader( );

    size_t read_from( unsigned const char_1 *buff );

    void dump( );

    // Overloaded << operator
    friend std::ostream& operator<< (std::ostream& os,
                                     MSG_SGSImageProductSpecificHeader &i);

    uint_4 ImageProductSpecificHeaderLenght;
    uint_1 *ImageProductSpecificHeaderData;
};

class MSG_SGSNonImageProductSpecificHeader {
  public:
    MSG_SGSNonImageProductSpecificHeader( );
};

class MSG_SGSProductSpecificHeader {
  public:
    MSG_SGSProductSpecificHeader( );
    MSG_SGSProductSpecificHeader(unsigned const char_1 *buff);
    ~MSG_SGSProductSpecificHeader( );

    size_t read_from( unsigned const char_1 *buff );

    // Overloaded << operator
    friend std::ostream& operator<< (std::ostream& os,
                                     MSG_SGSProductSpecificHeader &p);

    MSG_SGSImageProductSpecificHeader    ImageProductSpecificHeader;
    MSG_SGSNonImageProductSpecificHeader NonImageProductSpecificHeader;
};

class MSG_data_SGS_header {
  public:
    MSG_data_SGS_header( );
    MSG_data_SGS_header( unsigned const char_1 *buff );
    ~MSG_data_SGS_header( );

    size_t read_from( unsigned const char_1 *buff );

    // Overloaded << operator
    friend std::ostream& operator<< (std::ostream& os, MSG_data_SGS_header &s);

    MSG_SGSCommonHeader          SGSCommonHeader;
    MSG_SGSProductSpecificHeader SGSProductSpecificHeader;
};

#endif
