//-----------------------------------------------------------------------------
//
//  File        : MSG_time_cds.h
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

#ifndef __MSG_TIME_CDS_H__
#define __MSG_TIME_CDS_H__

#include <iostream>
#include <ctime>

#include "MSG_machine.h"

#define MSG_TIME_CDS_LEN   6
#define MSG_TIME_CDS_LEN_2 12

class MSG_time_cuc {
  public:
    MSG_time_cuc( );
    MSG_time_cuc(unsigned const char_1 *buff );
    ~MSG_time_cuc( );

    size_t read_from( unsigned const char_1 *buff );

    real_8 get_time_cuc_r8( );
    std::string get_coarse_time( ) const ;
    std::string get_fine_time( ) const ;

    //Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os, const MSG_time_cuc &h );

  private:
    uint_1 cuctime[7];
    real_8 cucvalue;
};

class MSG_time_generalized {
  public:
    MSG_time_generalized( );
    MSG_time_generalized(unsigned const char_1 *buff );
    ~MSG_time_generalized( );

    size_t read_from( unsigned const char_1 *buff );

    time_t get_unixtime( );
    struct tm *get_timestruct( );
    std::string get_timestring( );
    std::string get_timestring( ) const;
    char_1 *get_timechar( );

    //Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      const MSG_time_generalized &h );

  private:
    struct tm generalized_time;
};

class MSG_time_cds_short {
  public:
    MSG_time_cds_short( );
    MSG_time_cds_short(unsigned const char_1 *buff );
    ~MSG_time_cds_short( );

    size_t read_from( unsigned const char_1 *buff );

    uint_2 get_day_from_epoch( );
    uint_2 get_day_from_epoch( ) const;
    uint_4 get_msec_in_day( );
    uint_4 get_msec_in_day( ) const;
    time_t get_unixtime( );
    struct tm *get_timestruct( );
    std::string get_timestring( );
    std::string get_timestring( ) const;
    char_1 *get_timechar( );

    //Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      const MSG_time_cds_short &h );

  private:
    uint_2 day_from_epoch;
    uint_4 msec_in_day;
    time_t unixtime;
    time_t usecs;
};

class MSG_time_cds: public MSG_time_cds_short {
  public:
    MSG_time_cds( );
    MSG_time_cds( unsigned const char_1 *buff );
    ~MSG_time_cds( );

    size_t read_from( unsigned const char_1 *buff );

    uint_2 get_microsecs( );
    std::string get_timestring( );
    std::string get_timestring( ) const;
    char_1 *get_timechar( );

    //Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os, const MSG_time_cds &h );

  private:
    uint_2 microsecs;
};

class MSG_time_cds_expanded: public MSG_time_cds_short {
  public:
    MSG_time_cds_expanded( );
    MSG_time_cds_expanded( unsigned const char_1 *buff );
    ~MSG_time_cds_expanded( );

    size_t read_from( unsigned const char_1 *buff );

    uint_2 get_microsecs( );
    uint_2 get_nanosecs( );
    std::string get_timestring( );
    std::string get_timestring( ) const;
    char_1 *get_timechar( );

    //Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      const MSG_time_cds_expanded &h );

  private:
    uint_2 microsecs;
    uint_2 nanosecs;
};

#endif
