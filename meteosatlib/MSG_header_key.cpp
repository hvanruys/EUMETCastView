//-----------------------------------------------------------------------------
//
//  File        : MSG_header_key.cpp
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

#include "MSG_header_key.h"

MSG_header_key::MSG_header_key( )
{ 
}

MSG_header_key::MSG_header_key( unsigned const char_1 *buff )
{
  this->read_from(buff);
}

MSG_header_key::~MSG_header_key( ) { }

void MSG_header_key::read_from( unsigned const char_1 *buff )
{
  key_number = *(buff+3);
  seed       = get_ui8(buff+4);
  return;
}

bool MSG_header_key::is_key_group_1( )
{
  return (key_number >= 64 && key_number <= 127);
}

bool MSG_header_key::is_key_group_2( )
{
  return (key_number >= 192);
}

bool MSG_header_key::is_key_reserved( )
{
  return (key_number < 64 || (key_number > 127 && key_number < 192));
}

std::ostream& operator<< ( std::ostream& os, MSG_header_key &h )
{
  os << "------------------------------------------------------" << std::endl
     << "-                    MSG KEY HEADER                  -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << "Key number          : " << (uint_2) h.key_number << std::endl
     << "Seed                : " << h.seed << std::endl;
  return os;
}
