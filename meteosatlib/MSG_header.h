//-----------------------------------------------------------------------------
//
//  File        : MSG_header.h
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

#ifndef __MSG_HEADER_H__
#define __MSG_HEADER_H__

#include <iostream>
#include <fstream>
#include <string>

#include "MSG_machine.h"
#include "MSG_filetype.h"

#include "MSG_header_image_struct.h"
#include "MSG_header_image_navig.h"
#include "MSG_header_image_datafunc.h"
#include "MSG_header_annotation.h"
#include "MSG_header_timestamp.h"
#include "MSG_header_ancillary_text.h"
#include "MSG_header_key.h"
#include "MSG_header_segment_id.h"
#include "MSG_header_segment_quality.h"


typedef enum {
  MSG_HEADER_PRIMARY                     = 0,
  MSG_HEADER_IMAGE_STRUCTURE             = 1,
  MSG_HEADER_IMAGE_NAVIGATION            = 2,
  MSG_HEADER_IMAGE_DATA_FUNCTION         = 3,
  MSG_HEADER_ANNOTATION                  = 4,
  MSG_HEADER_TIMESTAMP                   = 5,
  MSG_HEADER_ANCILLARY_TEXT              = 6,
  MSG_HEADER_KEY                         = 7,
  MSG_HEADER_RESERVED                    = 8,
  MSG_HEADER_SEGMENT_IDENTIFICATION      = 128,
  MSG_HEADER_IMAGE_SEGMENT_LINE_QUALITY  = 129,
  MSG_HEADER_MISSION_RESERVED_FUTURE_USE = 130
} t_enum_MSG_headertype;

#define MSG_HEADER_PRIMARY_LEN     16
#define MSG_IMAGE_STRUCTURE_LEN     9
#define MSG_IMAGE_NAVIGATION_LEN   51
#define MSG_ANNOTATION_LEN         64
#define MSG_TIMESTAMP_LEN          10    
#define MSG_KEY_LEN                12
#define MSG_SEGMENT_ID_LEN         13

class MSG_header {
  public:
    MSG_header( );
    MSG_header( std::ifstream &in );
    ~MSG_header( );

    void read_from( std::ifstream &in );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os, MSG_header &h );

    uint_4 total_header_length;
    uint_8 data_field_length;
    size_t filesize;
    t_enum_MSG_filetype f_typecode;

    MSG_header_image_struct *image_structure;
    MSG_header_image_navig *image_navigation;
    MSG_header_image_datafunc *image_data_function;
    MSG_header_annotation *annotation;
    MSG_header_timestamp *timestamp;
    MSG_header_ancillary_text *ancillary_text;
    MSG_header_key *key;
    MSG_header_segment_id *segment_id;
    MSG_header_segment_quality *segment_quality;

};

#endif
