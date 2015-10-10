//-----------------------------------------------------------------------------
//
//  File        : MSG_header.cpp
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

#include "MSG_header.h"

MSG_header::MSG_header( )
{
  image_structure = 0;
  image_navigation = 0;
  image_data_function = 0;
  annotation = 0;
  timestamp = 0;
  ancillary_text = 0;
  key = 0;
  segment_id = 0;
  segment_quality = 0;
}

MSG_header::MSG_header( std::ifstream &in )
{
  image_structure = 0;
  image_navigation = 0;
  image_data_function = 0;
  annotation = 0;
  timestamp = 0;
  ancillary_text = 0;
  key = 0;
  segment_id = 0;
  segment_quality = 0;
  this->read_from(in);
}

MSG_header::~MSG_header( )
{
  if (image_structure) delete image_structure;
  if (image_navigation) delete image_navigation;
  if (image_data_function) delete image_data_function;
  if (annotation) delete annotation;
  if (timestamp) delete timestamp;
  if (ancillary_text) delete ancillary_text;
  if (key) delete key;
  if (segment_id) delete segment_id;
  if (segment_quality) delete segment_quality;
}

void MSG_header::read_from( std::ifstream &in )
{
  unsigned char primary_header[MSG_HEADER_PRIMARY_LEN];
  unsigned char *hbuff;

  check_endianess( );
  in.read((char *) primary_header, MSG_HEADER_PRIMARY_LEN);
  if (in.fail( ))
  {
    std::cerr << "Read error from HRIT file: Primary Header." << std::endl;
    throw;
  }
  if (primary_header[0] != MSG_HEADER_PRIMARY)
  {
    std::cerr << "Error: First header type value is not primary." << std::endl;
    throw;
  }
  if (get_ui2(primary_header+1) != MSG_HEADER_PRIMARY_LEN)
  {
    std::cerr << "Error: Primary Header Length mismatch." << std::endl;
    std::cerr << "Header Length: " << get_ui2(primary_header+1) << std::endl;
    throw;
  }
  f_typecode = (t_enum_MSG_filetype) *(primary_header+3);
  total_header_length = get_ui4(primary_header+4);
  data_field_length   = get_ui8(primary_header+8);
  filesize = data_field_length/8+total_header_length;
  size_t hsize = total_header_length-MSG_HEADER_PRIMARY_LEN;
  hbuff = new unsigned char[hsize];
  if (!hbuff)
  {
    std::cerr << "Memory allocation error in MSG_header." << std::endl
              << "Requested allocation of " << hsize << " bytes." << std::endl;
    throw;
  }
  in.read((char *) hbuff, hsize);
  if (in.fail( ))
  {
    std::cerr << "Read error from HRIT file: Header body" << std::endl;
    throw;
  }
  unsigned char *pnt = hbuff;
  size_t left = hsize;
  size_t hunk_size = 0;
  size_t hqlen = 0;
  while (left)
  {
    switch(*pnt)
    {
      case MSG_HEADER_IMAGE_STRUCTURE:
        if (get_ui2(pnt+1) != MSG_IMAGE_STRUCTURE_LEN)
        {
          std::cerr << "Error: Image Structure Header mismatch." << std::endl;
          std::cerr << "Header Length: " << get_ui2(pnt+1) << std::endl;
          throw;
        }
        image_structure = new MSG_header_image_struct;
        image_structure->read_from(pnt);
        pnt = pnt + MSG_IMAGE_STRUCTURE_LEN;
        left = left - MSG_IMAGE_STRUCTURE_LEN;
        break;
      case MSG_HEADER_IMAGE_NAVIGATION:
        if (get_ui2(pnt+1) != MSG_IMAGE_NAVIGATION_LEN)
        {
          std::cerr << "Error: Image Navigation Header mismatch." << std::endl;
          std::cerr << "Header Length: " << get_ui2(pnt+1) << std::endl;
          throw;
        }
        image_navigation = new MSG_header_image_navig;
        image_navigation->read_from(pnt);
        pnt = pnt + MSG_IMAGE_NAVIGATION_LEN;
        left = left - MSG_IMAGE_NAVIGATION_LEN;
        break;
      case MSG_HEADER_IMAGE_DATA_FUNCTION:
        hunk_size = (size_t) get_ui2(pnt+1);
        image_data_function = new MSG_header_image_datafunc;
        image_data_function->read_from(pnt);
        pnt = pnt + hunk_size;
        left = left - hunk_size;
        break;
      case MSG_HEADER_ANNOTATION:
        if (get_ui2(pnt+1) != MSG_ANNOTATION_LEN)
        {
          std::cerr << "Error: Annotation Header mismatch." << std::endl;
          std::cerr << "Header Length: " << get_ui2(pnt+1) << std::endl;
          throw;
        }
        annotation = new MSG_header_annotation;
        annotation->read_from(pnt);
        pnt = pnt + MSG_ANNOTATION_LEN;
        left = left - MSG_ANNOTATION_LEN;
        break;
      case MSG_HEADER_TIMESTAMP:
        if (get_ui2(pnt+1) != MSG_TIMESTAMP_LEN)
        {
          std::cerr << "Error: Timestamp Header mismatch." << std::endl;
          std::cerr << "Header Length: " << get_ui2(pnt+1) << std::endl;
          throw;
        }
        timestamp = new MSG_header_timestamp;
        timestamp->read_from(pnt);
        pnt = pnt + MSG_TIMESTAMP_LEN;
        left = left - MSG_TIMESTAMP_LEN;
        break;
      case MSG_HEADER_ANCILLARY_TEXT:
        hunk_size = (size_t) get_ui2(pnt+1);
        ancillary_text = new MSG_header_ancillary_text;
        ancillary_text->read_from(pnt);
        pnt = pnt + hunk_size;
        left = left - hunk_size;
        break;
      case MSG_HEADER_KEY:
        if (get_ui2(pnt+1) != MSG_KEY_LEN)
        {
          std::cerr << "Error: Key Header mismatch." << std::endl;
          std::cerr << "Header Length: " << get_ui2(pnt+1) << std::endl;
          throw;
        }
        key = new MSG_header_key;
        key->read_from(pnt);
        pnt = pnt + MSG_KEY_LEN;
        left = left - MSG_KEY_LEN;
        break;
      case MSG_HEADER_SEGMENT_IDENTIFICATION:
        if (get_ui2(pnt+1) != MSG_SEGMENT_ID_LEN)
        {
          std::cerr << "Error: Segment Id Header mismatch." << std::endl;
          std::cerr << "Header Length: " << get_ui2(pnt+1) << std::endl;
          throw;
        }
        segment_id = new MSG_header_segment_id;
        segment_id->read_from(pnt);
        pnt = pnt + MSG_SEGMENT_ID_LEN;
        left = left - MSG_SEGMENT_ID_LEN;

        break;
      case MSG_HEADER_IMAGE_SEGMENT_LINE_QUALITY:
        segment_quality = new MSG_header_segment_quality;
        segment_quality->read_from(pnt, image_structure->number_of_lines);
        hqlen = get_ui2(pnt+1);
        pnt = pnt + hqlen;
        left = left - hqlen;
        break;
      default:
        std::cerr << "Unknown header type: " << (uint_2) *pnt << std::endl;
        std::cerr << "Unparsed " << left << " bytes." << std::endl;
        throw;
        break;
      }
   }
   delete [ ] hbuff;
   return;
}

std::ostream& operator<< ( std::ostream& os, MSG_header &h )
{
  os << "------------------------------------------------------" << std::endl
     << "-                  MSG PRIMARY HEADER                -" << std::endl
     << "------------------------------------------------------" << std::endl
     << "File Typecode       : " << (uint_8) h.f_typecode
     << " (" << MSG_filetype(h.f_typecode) << ")" << std::endl
     << "Total Header Length : " << (uint_8) h.total_header_length << std::endl
     << "Data Length (bits)  : " << (uint_8) h.data_field_length << std::endl;
     if (h.image_structure) os << *(h.image_structure);
     if (h.image_navigation) os << *(h.image_navigation);
     if (h.image_data_function) os << *(h.image_data_function);
     if (h.annotation) os << *(h.annotation);
     if (h.timestamp) os << *(h.timestamp);
     if (h.ancillary_text) os << *(h.ancillary_text);
     if (h.key) os << *(h.key);
     if (h.segment_id) os << *(h.segment_id);
     if (h.segment_quality) os << *(h.segment_quality);
  os << "------------------------------------------------------" << std::endl;
  return os;
}
