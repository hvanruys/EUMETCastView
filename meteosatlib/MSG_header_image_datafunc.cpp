//-----------------------------------------------------------------------------
//
//  File        : MSG_header_image_datafunc.cpp
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

#include <cstring>
#include "MSG_header_image_datafunc.h"

MSG_header_image_datafunc::MSG_header_image_datafunc( )
{
}

MSG_header_image_datafunc::MSG_header_image_datafunc(
           unsigned const char_1 *buff)
{
  this->read_from(buff);
}

MSG_header_image_datafunc::~MSG_header_image_datafunc( ) { }

void MSG_header_image_datafunc::read_from( unsigned const char_1 *buff )
{
  size_t h_length = (size_t) get_ui2(buff+1) - 2;
  if (h_length <= 0)
  {
    std::cerr << "Error: Data Function Header length invalid." << std::endl;
    std::cerr << "Header Length : " << h_length - 1 << std::endl;
    throw;
  }
  char_1 *tmpchar = new char_1[h_length];
  if (!tmpchar)
  {
    std::cerr << "Memory error in MSG_header_image_datafunc." << std::endl;
    std::cerr << "Header Length : " << h_length - 1 << std::endl;
    throw;
  }
  memcpy(tmpchar, buff+3, h_length-1);
  tmpchar[h_length-1] = 0;
  data_definition_block = tmpchar;
  delete [ ] tmpchar;
  return;
}

std::ostream& operator<< ( std::ostream& os,
                           MSG_header_image_datafunc &h )
{
  os << "------------------------------------------------------" << std::endl
     << "-            MSG IMAGE DATA FUNCTION HEADER          -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << h.data_definition_block << std::endl;
  return os;
}
