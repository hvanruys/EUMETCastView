//-----------------------------------------------------------------------------
//
//  File        : MSG_header_image_navig.h
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

#ifndef __MSG_HEADER_IMAGE_NAVIG_H__
#define __MSG_HEADER_IMAGE_NAVIG_H__

#include <iostream>
#include <string>

#include "MSG_machine.h"
#include "MSG_projection.h"

class MSG_header_image_navig {
  public:
    MSG_header_image_navig( );
    MSG_header_image_navig( unsigned const char_1 *buff );
    ~MSG_header_image_navig( );

    void read_from( unsigned const char_1 *buff );

    t_enum_MSG_projection get_projection_code( );
 
    float satellite_h;

    std::string pname;
    real_4 subsatellite_longitude;
    real_4 projection_plane;
    real_4 projection_longitude;
    int_4 column_scaling_factor; 
    int_4 line_scaling_factor; 
    int_4 column_offset;
    int_4 line_offset;
    int_4 CFAC;
    int_4 LFAC;
    int_4 COFF;
    int_4 LOFF;

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_header_image_navig &h );

};

#endif
