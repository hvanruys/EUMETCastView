//-----------------------------------------------------------------------------
//
//  File        : MSG_header_segment_id.h
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

#ifndef __MSG_HEADER_SEGMENT_ID_H__
#define __MSG_HEADER_SEGMENT_ID_H__

#include <iostream>
#include <string>

#include "MSG_machine.h"
#include "MSG_data_format.h"
#include "MSG_spacecraft.h"
#include "MSG_channel.h"

class MSG_header_segment_id {
  public:
    MSG_header_segment_id( );
    MSG_header_segment_id( unsigned const char_1 *buff );
    ~MSG_header_segment_id( );

    void read_from( unsigned const char_1 *buff );
    void read_from_himawari( unsigned const char_1 *buff );

    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_header_segment_id &h);

    t_enum_MSG_spacecraft spacecraft_id;
    uint_1 spectral_channel_id;
    uint_2 sequence_number;
    uint_2 planned_start_segment_sequence_number;
    uint_2 planned_end_segment_sequence_number;
    t_enum_MSG_data_format data_field_format;

    // Himawari
    uint_1 image_segm_seq_no;
    uint_1 total_no_image_segm;
    uint_2 line_no_image_segm;

};

#endif
