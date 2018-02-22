//-----------------------------------------------------------------------------
//
//  File        : MSG_header_image_struct.h
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

#ifndef __MSG_HEADER_IMAGE_STRUCT_H__
#define __MSG_HEADER_IMAGE_STRUCT_H__

#include <iostream>
#include <string>

#include "MSG_machine.h"
#include "MSG_compression.h"

class MSG_header_image_struct {
  public:
    MSG_header_image_struct( );
    MSG_header_image_struct( unsigned const char_1 *buff );
    ~MSG_header_image_struct( );

    void read_from( unsigned const char_1 *buff );

    uint_1 number_of_bits_per_pixel;
    uint_2 number_of_columns;
    uint_2 number_of_lines;
    t_enum_MSG_image_compression compression_flag;
    uint_8 image_pixels;
    uint_8 image_size;

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_header_image_struct &h );

};

#endif
