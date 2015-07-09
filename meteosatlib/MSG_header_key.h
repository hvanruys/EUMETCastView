//-----------------------------------------------------------------------------
//
//  File        : MSG_header_key.h
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

#ifndef __MSG_HEADER_KEY_H__
#define __MSG_HEADER_KEY_H__

#include <iostream>
#include "MSG_machine.h"

class MSG_header_key {
  public:
    MSG_header_key( );
    MSG_header_key( unsigned const char_1 *buff );
    ~MSG_header_key( );

    void read_from( unsigned const char_1 *buff );

    bool is_key_group_1( );
    bool is_key_group_2( );
    bool is_key_reserved( );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os, MSG_header_key &h );

    uint_1 key_number;
    uint_8 seed;

};

#endif
