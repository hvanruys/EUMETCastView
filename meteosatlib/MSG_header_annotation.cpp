//-----------------------------------------------------------------------------
//
//  File        : MSG_header_annotation.cpp
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

#include "MSG_header_annotation.h"

#include <cstring>

MSG_header_annotation::MSG_header_annotation( )
{
}

MSG_header_annotation::MSG_header_annotation( unsigned const char_1 *buff )
{ 
  this->read_from(buff);
}

MSG_header_annotation::~MSG_header_annotation( ) { }

void MSG_header_annotation::read_from( unsigned const char_1 *buff )
{
  char_1 tmp[62];
  memcpy(tmp, buff+3, 61);
  tmp[61] = 0;
  annotation = tmp;
  xrit_channel_id = annotation.substr(0,1);
  annotation_version = annotation.substr(2,3);
  disseminating_s_c = annotation.substr(6,6);
  product_id_1 = annotation.substr(13,12);
  product_id_2 = annotation.substr(26,9);
  product_id_3 = annotation.substr(36,9);
  product_id_4 = annotation.substr(46,12);
  flags = annotation.substr(59,2);
  return;
}

std::ostream& operator<< ( std::ostream& os, MSG_header_annotation &h )
{
  os << "------------------------------------------------------" << std::endl
     << "-            MSG IMAGE ANNOTATION HEADER             -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << "XRIT channel ID     : " << h.xrit_channel_id << std::endl
     << "Annotation Version  : " << h.annotation_version << std::endl
     << "Disseminating S/C   : " << h.disseminating_s_c << std::endl
     << "Product ID 1        : " << h.product_id_1 << std::endl
     << "Product ID 2        : " << h.product_id_2 << std::endl
     << "Product ID 3        : " << h.product_id_3 << std::endl
     << "Product ID 4        : " << h.product_id_4 << std::endl
     << "Flags               : " << h.flags << std::endl;
  return os;
}
