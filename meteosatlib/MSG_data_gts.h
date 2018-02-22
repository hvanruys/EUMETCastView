//-----------------------------------------------------------------------------
//
//  File        : MSG_data_gts.h
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

#ifndef __MSG_DATA_GTS_H__
#define __MSG_DATA_GTS_H__

#include <iostream>
#include "MSG_machine.h"

#define MSG_MAX_GTS_MESSAGE_SIZE 15000
#define MSG_MAX_GTS_MESSAGES     65535

class MSG_data_gts {
  public:
    MSG_data_gts( );
    MSG_data_gts( unsigned const char_1 *buff, size_t datasize );
    ~MSG_data_gts( );

    size_t read_from( unsigned const char_1 *buff, size_t datasize );

    void dump( );
    void dump(char_1 *filename);
    void dump(std::string filename);

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os, MSG_data_gts &g );

  private:
    size_t msize;
    unsigned char_1 *mbuff;
};

#endif
