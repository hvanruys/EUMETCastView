//-----------------------------------------------------------------------------
//
//  File        : MSG_header_image_struct.cpp
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

#include "MSG_header_image_struct.h"

MSG_header_image_struct::MSG_header_image_struct( )
{
}

MSG_header_image_struct::MSG_header_image_struct( unsigned const char_1 *buff )
{ 
  this->read_from(buff);
}

MSG_header_image_struct::~MSG_header_image_struct( ) { }

void MSG_header_image_struct::read_from( unsigned const char_1 *buff )
{
  number_of_bits_per_pixel  = buff[3];
  number_of_columns         = get_ui2(buff+4);
  number_of_lines           = get_ui2(buff+6);
  compression_flag          = (t_enum_MSG_image_compression) buff[8];
  image_pixels              = number_of_columns * number_of_lines;
  image_size                = (image_pixels*number_of_bits_per_pixel) / 8;
  if ((image_pixels*number_of_bits_per_pixel) % 8) image_size ++;
  return;
}

std::ostream& operator<< ( std::ostream& os, MSG_header_image_struct &h )
{
  os << "------------------------------------------------------" << std::endl
     << "-             MSG IMAGE STRUCTURE HEADER             -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << "Bits per pixel      : " << (uint_8) h.number_of_bits_per_pixel
     << std::endl
     << "Number of Columns   : " << (uint_8) h.number_of_columns << std::endl
     << "Number of Lines     : " << (uint_8) h.number_of_lines << std::endl
     << "Compression Flag    : " << (uint_8) h.compression_flag
     << " (" << MSG_image_compression(h.compression_flag) << ")" << std::endl;
  return os;
}
