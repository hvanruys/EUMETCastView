//-----------------------------------------------------------------------------
//
//  File        : MSG_header_ancillary_text.cpp
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

#include "MSG_header_ancillary_text.h"

MSG_header_ancillary_text::MSG_header_ancillary_text( )
{
  ancillary_text = 0;
}

MSG_header_ancillary_text::MSG_header_ancillary_text(
            unsigned const char_1 *buff)
{
  ancillary_text = 0;
  this->read_from(buff);
}

MSG_header_ancillary_text::~MSG_header_ancillary_text( )
{
  if (ancillary_text)
    delete [ ] ancillary_text;
}

void MSG_header_ancillary_text::read_from( unsigned const char_1 *buff )
{
  size_t h_length = (size_t) get_ui2(buff+1) - 3;
  if (h_length <= 0)
  {
    std::cerr << "Error: Data Function Header length invalid." << std::endl;
    std::cerr << "Header Length : " << h_length << std::endl;
    throw;
  }
  ancillary_text = new char_1[h_length];
  if (!ancillary_text)
  {
    std::cerr << "Memory error in MSG_header_ancillary_text." << std::endl;
    std::cerr << "Header Length : " << h_length << std::endl;
    throw;
  }
  memcpy(ancillary_text, buff+3, h_length);
  return;
}

std::ostream& operator<< ( std::ostream& os, MSG_header_ancillary_text &h )
{
  os << "------------------------------------------------------" << std::endl
     << "-              MSG ANCILLARY TEXT HEADER             -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << h.ancillary_text << std::endl;
  return os;
}
