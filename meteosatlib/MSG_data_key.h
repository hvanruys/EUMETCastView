//-----------------------------------------------------------------------------
//
//  File        : MSG_data_key.h
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

#ifndef __MSG_DATA_KEY_H__
#define __MSG_DATA_KEY_H__

#include <iostream>
#include "MSG_machine.h"

#define MSG_MAX_NUMBER_KEY_MESSAGES 65536
#define MSG_KEY_MESSAGE_LEN           197

class MSG_encryption_key {
  public:
    uint_2 User_Station_number;
    uint_1 Key_Number;
    char_1 Public_key[192];
    uint_2 Public_Key_CRC;

    friend std::ostream& operator<< ( std::ostream& os, MSG_encryption_key &k );
};

class MSG_data_key {
  public:
    MSG_data_key( );
    MSG_data_key( unsigned const char_1 *buff, size_t datasize );
    ~MSG_data_key( );

    size_t read_from( unsigned const char_1 *buff, size_t datasize );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os, MSG_data_key &k );

  private:
    int_4 nkeys;
    MSG_encryption_key *keys;
};

#endif
