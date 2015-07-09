//-----------------------------------------------------------------------------
//
//  File        : MSG_header_segment_quality.cpp
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

#include "MSG_header_segment_quality.h"

MSG_header_segment_quality::MSG_header_segment_quality( )
{
  nlines  = 0;
  lq      = 0;
}

MSG_header_segment_quality::MSG_header_segment_quality(
      unsigned const char_1 *buff,
      uint_2 nlines )
{
  lq            = 0;
  this->nlines  = 0;
  this->read_from(buff, nlines);
}

MSG_header_segment_quality::~MSG_header_segment_quality( )
{
  if (lq) delete [ ] lq;
  lq = 0;
  nlines = 0;
}

void MSG_header_segment_quality::read_from( unsigned const char_1 *buff ,
                                            uint_2 nlines)
{
  this->nlines  = (int) nlines;
  lq            = new MSG_segment_quality[this->nlines];
  unsigned char_1 *pnt = (unsigned char_1 *) buff+3;
  for (int i = 0; i < this->nlines; i ++)
  {
    lq[i].set(pnt);
    pnt = pnt+MSG_SEGMENT_QUALITY_RECORD_LEN;
  }
  return;
}

std::ostream& operator<< ( std::ostream& os, MSG_header_segment_quality &h)
{
  os << "------------------------------------------------------" << std::endl
     << "-            MSG HEADER SEGMENT QUALITY              -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << "Total number of Lines: " << h.nlines << std::endl;
  for (int i = 0; i < h.nlines; i ++)
    if (h.lq[i].line_number_in_grid > 0    && 
        (h.lq[i].line_validity > MSG_SEGMENT_VALIDITY_NOMINAL ||
         h.lq[i].line_radiometric_quality > MSG_SEGMENT_RADIOMETRIC_QUALITY_USABLE ||
         h.lq[i].line_geometric_quality > MSG_SEGMENT_GEOMETRIC_QUALITY_USABLE))
      os << "***********" << std::endl << h.lq[i];
  os << "***********" << std::endl;
  return os;
}

void MSG_segment_quality::set(unsigned const char_1 *buff)
{
  line_number_in_grid      = get_i4(buff);

  (void) line_mean_acquisition.read_from(buff+4);

  line_validity            = (t_enum_MSG_segment_validity) *(buff+10);
  line_radiometric_quality = (t_enum_MSG_segment_radiometric_quality)*(buff+11);
  line_geometric_quality   = (t_enum_MSG_segment_geometric_quality) *(buff+12);
  return;
}

std::string MSG_segment_quality::validity( )
{
  return MSG_segment_validity(line_validity);
}

std::string MSG_segment_quality::radiometric_quality( )
{
  return MSG_segment_radiometric_quality(line_radiometric_quality);
}

std::string MSG_segment_quality::geometric_quality( )
{
  return MSG_segment_geometric_quality(line_geometric_quality);
}

std::ostream& operator<< ( std::ostream& os, MSG_segment_quality &lq)
{
  os << "Line Number         : " << lq.line_number_in_grid << std::endl
     << lq.line_mean_acquisition
     << "Line validity       : " << lq.line_validity
     << " (" << lq.validity( ) << ")" << std::endl
     << "Line radiometric ql : " << lq.line_radiometric_quality
     << " (" << lq.radiometric_quality( ) << ")" << std::endl
     << "Line geometric ql   : " << lq.line_geometric_quality
     << " (" << lq.geometric_quality( ) << ")" << std::endl;
  return os;
}
