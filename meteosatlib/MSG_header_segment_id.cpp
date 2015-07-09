//-----------------------------------------------------------------------------
//
//  File        : MSG_header_segment_id.cpp
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

#include <string>

#include <cstring>
#include "MSG_header_segment_id.h"

MSG_header_segment_id::MSG_header_segment_id( )
{
}

MSG_header_segment_id::MSG_header_segment_id( unsigned const char_1 *buff )
{
  this->read_from(buff);
}

MSG_header_segment_id::~MSG_header_segment_id( ) { }

void MSG_header_segment_id::read_from( unsigned const char_1 *buff )
{
  spacecraft_id = (t_enum_MSG_spacecraft) get_ui2(buff+3);
  spectral_channel_id = *(buff+5);
  sequence_number = get_ui2(buff+6);
  planned_start_segment_sequence_number = get_ui2(buff+8);
  planned_end_segment_sequence_number = get_ui2(buff+10);
  data_field_format = (t_enum_MSG_data_format) *(buff+12);
  return;
}

std::ostream& operator<< ( std::ostream& os, MSG_header_segment_id &h)
{
  os << "------------------------------------------------------" << std::endl
     << "-              MSG HEADER SEGMENT ID                 -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << "Spacecraft ID       : " << h.spacecraft_id
     << " (" << MSG_spacecraft_name(h.spacecraft_id) << ")" << std::endl
     << "Data channel ID     : " << (int) h.spectral_channel_id
     << " (" << MSG_channel_name(h.spacecraft_id, h.spectral_channel_id)
     << ")" << std::endl
     << "Segment seq No      : " << h.sequence_number << std::endl
     << "Planned seq start   : " << h.planned_start_segment_sequence_number
     << std::endl
     << "Planned seq end     : " << h.planned_end_segment_sequence_number
     << std::endl
     << "Data format         : " << h.data_field_format
     << " (" << MSG_data_format(h.data_field_format) << ")" << std::endl;
  return os;
}
