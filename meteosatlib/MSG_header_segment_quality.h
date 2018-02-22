//-----------------------------------------------------------------------------
//
//  File        : MSG_header_segment_quality.h
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

#ifndef __MSG_HEADER_SEGMENT_QUALITY_H__
#define __MSG_HEADER_SEGMENT_QUALITY_H__

#include <iostream>
#include <string>

#include "MSG_machine.h"
#include "MSG_time_cds.h"
#include "MSG_quality.h"

#define MSG_SEGMENT_QUALITY_RECORD_LEN 13

class MSG_segment_quality {
  public:
    int_4                                  line_number_in_grid;
    MSG_time_cds_short                     line_mean_acquisition;
    t_enum_MSG_segment_validity            line_validity; 
    t_enum_MSG_segment_radiometric_quality line_radiometric_quality; 
    t_enum_MSG_segment_geometric_quality   line_geometric_quality; 

    void set(unsigned const char_1 *buff);

    std::string validity( );
    std::string radiometric_quality( );
    std::string geometric_quality( );

    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_segment_quality &lq);
};

class MSG_header_segment_quality {
  public:
    MSG_header_segment_quality( );
    MSG_header_segment_quality( unsigned const char_1 *buff,
                                uint_2 number_of_lines );
    ~MSG_header_segment_quality( );

    void read_from( unsigned const char_1 *buff, uint_2 number_of_lines );

    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_header_segment_quality &h);

    int_4  nlines;

    MSG_segment_quality *lq;

};

#endif
