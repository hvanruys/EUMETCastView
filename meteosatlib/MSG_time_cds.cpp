//-----------------------------------------------------------------------------
//
//  File        : MSG_time_cds.cpp
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

#include "MSG_time_cds.h"
#include <cstring>
#include <cmath>
#include <cstdio>
#include <string>
long INT(double value)
{
  if (value >= 0)
    return static_cast<long>(value);
  else
    return static_cast<long>(value - 1);
}


double DateToJD(long Year, long Month, double Day, bool bGregorianCalendar)
{
  long Y = Year;
  long M = Month;
  if (M < 3)
  {
    Y = Y - 1;
    M = M + 12;
  }

  long B = 0;
  if (bGregorianCalendar)
  {
    long A = INT(Y / 100.0);
    B = 2 - A + INT(A / 4.0);
  }

  return INT(365.25 * (Y + 4716)) + INT(30.6001 * (M + 1)) + Day + B - 1524.5;
}

MSG_time_cds_short::MSG_time_cds_short( )
{
  day_from_epoch = 0;
  msec_in_day = 0;
  unixtime = 0;
  usecs = 0;
}

MSG_time_cds_short::MSG_time_cds_short( unsigned const char_1 *buff )
{
  day_from_epoch = 0;
  msec_in_day = 0;
  unixtime = 0;
  usecs = 0;
  this->read_from(buff);
}

MSG_time_cds_short::~MSG_time_cds_short( ) { }

size_t MSG_time_cds_short::read_from( unsigned const char_1 *buff )
{
  const static time_t aday = 86400;
  const static time_t secs_from_1958_to_1970 = -378691200;

  day_from_epoch = get_ui2(buff);
  msec_in_day    = get_ui4(buff+2);

  time_t secs_from_1_1_1958 = aday * day_from_epoch;
  uint_4 secs_in_day        = msec_in_day / 1000;
  usecs                     = msec_in_day % 1000;
  unixtime                  = secs_from_1_1_1958 +
                              secs_in_day        +
                              secs_from_1958_to_1970;
  return 6;
}

uint_2 MSG_time_cds_short::get_day_from_epoch( )
{
  return day_from_epoch;
}

uint_4 MSG_time_cds_short::get_msec_in_day( )
{
  return msec_in_day;
}

uint_2 MSG_time_cds_short::get_day_from_epoch( ) const
{
  return day_from_epoch;
}

uint_4 MSG_time_cds_short::get_msec_in_day( ) const
{
  return msec_in_day;
}

time_t MSG_time_cds_short::get_unixtime( )
{
  return unixtime;
}

struct tm *MSG_time_cds_short::get_timestruct( )
{
  return gmtime(&unixtime);
}

double MSG_time_cds_short::get_jtime()
{
     double jtime_epoch = (double) DateToJD(1958, 1, 1, true);
     return jtime_epoch + day_from_epoch + msec_in_day / 1.e3 / 60. / 60. / 24.;
}

double MSG_time_cds_short::get_jtime_1958_1_1()
{
     double jtime_epoch = (double) DateToJD(1958, 1, 1, true);
     return jtime_epoch;
}

std::string MSG_time_cds_short::get_timestring( )
{
  std::string timestring;
  char_1 tempchar[128];
  strftime(tempchar, 128, "%Y-%m-%d %H:%M:%S", gmtime(&unixtime));
  timestring = tempchar;
  *tempchar = '\0';
  sprintf(tempchar, " +%03d msecs", (unsigned int) usecs);
  timestring = timestring + tempchar;
  return timestring;
}

std::string MSG_time_cds_short::get_timestring( ) const
{
  std::string timestring;
  char_1 tempchar[128];
  strftime(tempchar, 128, "%Y-%m-%d %H:%M:%S", gmtime(&unixtime));
  timestring = tempchar;
  sprintf(tempchar, " +%03d msecs", (unsigned int) usecs);
  timestring = timestring + tempchar;
  return timestring;
}

char_1 *MSG_time_cds_short::get_timechar( )
{
  return (char_1 *) get_timestring( ).c_str( );
}

std::ostream& operator<< ( std::ostream& os, const MSG_time_cds_short &h )
{
  os << "Day from epoch      : " << h.day_from_epoch
     << " (epoch is 1958-01-01)" << std::endl
     << "Msecs in day        : " << h.msec_in_day << std::endl
     << "Actual date         : " << h.get_timestring() << std::endl;
  return os;
}

MSG_time_cuc::MSG_time_cuc( )
{
  cucvalue = 0.0;
  memset(cuctime, 0, 7);
}

MSG_time_cuc::MSG_time_cuc(unsigned const char_1 *buff )
{
  cucvalue = 0.0;
  memset(cuctime, 0, 7);
  read_from(buff);
}

MSG_time_cuc::~MSG_time_cuc( ) { }

size_t MSG_time_cuc::read_from( unsigned const char_1 *buff )
{
  memcpy(cuctime, buff, 7);
  cucvalue = pow(256,3)  * cuctime[0] + pow(256,2)  * cuctime[1] +
             256         * cuctime[2] +               cuctime[3] +
             pow(256,-1) * cuctime[4] + pow(256,-2) * cuctime[5] +
             pow(256,-3) * cuctime[6];
  return 7;
}

real_8 MSG_time_cuc::get_time_cuc_r8( )
{
  return cucvalue;
}

std::string MSG_time_cuc::get_coarse_time( ) const
{
  std::string coarse_time;
  char_1 tmp[32];
  sprintf(tmp, "%03u %03u %03u %03u", cuctime[0], cuctime[1],
                                     cuctime[2], cuctime[3]);
  coarse_time = tmp;
  return coarse_time;
}

std::string MSG_time_cuc::get_fine_time( ) const
{
  std::string fine_time;
  char_1 tmp[32];
  sprintf(tmp, "%03u %03u %03u", cuctime[4], cuctime[5], cuctime[6]);
  fine_time = tmp;
  return fine_time;
}

std::ostream& operator<< ( std::ostream& os, const MSG_time_cuc &h )
{
  os << "CUC Time            : " << h.get_coarse_time( ) << " "
     << h.get_fine_time( ) << " (" << h.cucvalue << ")" << std::endl;
  return os;
}

MSG_time_cds_expanded::MSG_time_cds_expanded( )
{
  microsecs = 0;
  nanosecs = 0;
}

MSG_time_cds_expanded::MSG_time_cds_expanded( unsigned const char_1 *buff )
{
  microsecs = 0;
  nanosecs = 0;
  read_from(buff);
}

MSG_time_cds_expanded::~MSG_time_cds_expanded( ) { }

size_t MSG_time_cds_expanded::read_from( unsigned const char_1 *buff )
{
  MSG_time_cds_short::read_from(buff);
  microsecs = get_ui2(buff+6);
  nanosecs = get_ui2(buff+8);
  return 10;
}

uint_2 MSG_time_cds_expanded::get_microsecs( )
{
  return microsecs;
}

uint_2 MSG_time_cds_expanded::get_nanosecs( )
{
  return nanosecs;
}

std::string MSG_time_cds_expanded::get_timestring( )
{
  std::string times, temp;
  char_1 tmp[32];
  sprintf(tmp, ".%03d%03d", microsecs, nanosecs);
  temp = tmp;
  times = MSG_time_cds_short::get_timestring( );
  times = times.insert(24, temp);
  return times;
}

std::string MSG_time_cds_expanded::get_timestring( ) const
{
  std::string times, temp;
  char_1 tmp[32];
  sprintf(tmp, ".%03d%03d", microsecs, nanosecs);
  temp = tmp;
  times = MSG_time_cds_short::get_timestring( );
  times = times.insert(24, temp);
  return times;
}

char_1 *MSG_time_cds_expanded::get_timechar( )
{
  return (char_1 *) get_timestring( ).c_str( );
}

std::ostream& operator<< ( std::ostream& os, const MSG_time_cds_expanded &h )
{
  os << "Day from epoch      : " << h.get_day_from_epoch( )
     << " (epoch is 1958-01-01)" << std::endl
     << "Msecs in day        : " << h.get_msec_in_day( ) << std::endl
     << "Microsecs           : " << h.microsecs << std::endl
     << "Nanosecs            : " << h.nanosecs << std::endl
     << "Actual date         : " << h.get_timestring() << std::endl;
  return os;
}

MSG_time_cds::MSG_time_cds( )
{
  microsecs = 0;
}

MSG_time_cds::MSG_time_cds( unsigned const char_1 *buff )
{
  microsecs = 0;
  read_from(buff);
}

MSG_time_cds::~MSG_time_cds( ) { }

size_t MSG_time_cds::read_from( unsigned const char_1 *buff )
{
  MSG_time_cds_short::read_from(buff);
  microsecs = get_ui2(buff+6);
  return 8;
}

uint_2 MSG_time_cds::get_microsecs( )
{
  return microsecs;
}

std::string MSG_time_cds::get_timestring( )
{
  std::string times, temp;
  char_1 tmp[32];
  sprintf(tmp, ".%03d", microsecs);
  temp = tmp;
  times = MSG_time_cds_short::get_timestring( );
  times = times.insert(24, temp);
  return times;
}

std::string MSG_time_cds::get_timestring( ) const
{
  std::string times, temp;
  char_1 tmp[32];
  sprintf(tmp, ".%03d", microsecs);
  temp = tmp;
  times = MSG_time_cds_short::get_timestring( );
  times = times.insert(24, temp);
  return times;
}

char_1 *MSG_time_cds::get_timechar( )
{
  return (char_1 *) get_timestring( ).c_str( );
}

std::ostream& operator<< ( std::ostream& os, const MSG_time_cds &h )
{
  os << "Day from epoch      : " << h.get_day_from_epoch( )
     << " (epoch is 1958-01-01)" << std::endl
     << "Msecs in day        : " << h.get_msec_in_day( ) << std::endl
     << "Microsecs           : " << h.microsecs << std::endl
     << "Actual date         : " << h.get_timestring() << std::endl;
  return os;
}

MSG_time_generalized::MSG_time_generalized( )
{
  memset(&generalized_time, 0, sizeof(struct tm));
}

MSG_time_generalized::MSG_time_generalized(unsigned const char_1 *buff )
{
  memset(&generalized_time, 0, sizeof(struct tm));
  read_from(buff);
}

MSG_time_generalized::~MSG_time_generalized( ) { }

size_t MSG_time_generalized::read_from( unsigned const char_1 *buff )
{
  sscanf((char *) buff, "%4d%2d%2d%2d%2d%2dZ", &generalized_time.tm_year,
         &generalized_time.tm_mon,  &generalized_time.tm_mday,
         &generalized_time.tm_hour, &generalized_time.tm_min,
         &generalized_time.tm_sec);
  generalized_time.tm_year -= 1900;
  generalized_time.tm_mon -= 1;
  return 15;
}

time_t MSG_time_generalized::get_unixtime( )
{
  return mktime(&generalized_time);
}

struct tm *MSG_time_generalized::get_timestruct( )
{
  return &generalized_time;
}

std::string MSG_time_generalized::get_timestring( )
{
  std::string gentime;
  char_1 tempchar[128];
  strftime(tempchar, 128, "%Y-%m-%d %H:%M:%S", &generalized_time);
  gentime = tempchar;
  return gentime;
}

std::string MSG_time_generalized::get_timestring( ) const
{
  std::string gentime;
  char_1 tempchar[128];
  strftime(tempchar, 128, "%Y-%m-%d %H:%M:%S", &generalized_time);
  gentime = tempchar;
  return gentime;
}

char_1 *MSG_time_generalized::get_timechar( )
{
  return (char_1 *) get_timestring( ).c_str( );
}

std::ostream& operator<< ( std::ostream& os, const MSG_time_generalized &h )
{
  os << "Actual date         : " << h.get_timestring() << std::endl;
  return os;
}
