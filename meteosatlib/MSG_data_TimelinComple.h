//-----------------------------------------------------------------------------
//
//  File        : MSG_data_TimelinComple.h
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

#ifndef __MSG_DATA_TIMELINCOMPLE_H__
#define __MSG_DATA_TIMELINCOMPLE_H__

#include <iostream>
#include "MSG_machine.h"

#define MSG_TIMELINESS_COMPLETENESS_LEN 132

class MSG_Timeliness {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os, MSG_Timeliness &t );

    real_4 MaxDelay;
    real_4 MinDelay;
    real_4 MeanDelay;
};

class MSG_Completeness {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os, MSG_Completeness &c );

    uint_2 PlannedL15ImageLines;
    uint_2 GeneratedL15ImageLines;
    uint_2 ValidL15ImageLines;
    uint_2 DummyL15ImageLines;
    uint_2 CorruptedL15ImageLines;
};

class MSG_data_TimelinComple {
  public:
    MSG_data_TimelinComple( );
    MSG_data_TimelinComple( unsigned const char_1 *buff );
    ~MSG_data_TimelinComple( );

    size_t read_from( unsigned const char_1 *buff );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_data_TimelinComple &r );

    MSG_Timeliness   Timeliness;
    MSG_Completeness Completeness[12];
};

#endif
