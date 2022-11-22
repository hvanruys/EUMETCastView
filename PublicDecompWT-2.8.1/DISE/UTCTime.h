/*
 * Copyright 2011-2019, European Organisation for the Exploitation of Meteorological Satellites (EUMETSAT)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef UTC_TIME_H
#define UTC_TIME_H  


#include <sys/time.h>

#ifdef _MINGW32__
struct tm *localtime_r(const time_t *timep, struct tm *result);
#endif

#include <string>

namespace Util
{
    class CTimeSpan;
}

#include "TimeSpan.h"

namespace Util
{
/**
 * timespan class
 *
**/
class CUTCTime
{
 public:
/**
 * default constructor (sets the time to the epoch)
 **/
    CUTCTime();

/**
 * copy  constructor
 **/
    CUTCTime(const CUTCTime& t);

/**
 * constructor takes timeval object
 **/
    CUTCTime(const timeval& t);

 
/**
 * constructor takes timespan object
 **/
    CUTCTime(const CTimeSpan& t);
 
 
/**
 * constructor. To use if you want to set a specific time
 *  @param
 *   sec seconds
 *  , min minutes
 *  , hour hour
 *  , mday day of the month
 *  , mon month MM 1-12
 *  , year YYYY, e.g 2002
 **/
    CUTCTime(int     sec,
             int     min = 0,
             int     hour = 0,
             int     mday = 0,
             int     mon = 0,
             int     year= 0);

    CUTCTime(int     sec,
             int     min,
             int     hour,
             int     mday,
             int     mon,
             int     year,
	     int     wday,
	     int     yday,
	     int     isdst);


/**
 * destructor
 **/
    ~CUTCTime(){};

/**
 * static functions retruns a UTCTime object that holds the current time
 * @return
 * UTCTime object
 **/
    static CUTCTime Now();

/**
 * retruns the day of the UTCTime object
 * @return
 * day of week for the time you have specified. e.g. create an object with
 * CUTCTime(0,0,0,3,7,2002), returns the week days(3) of July 1.
 **/
    unsigned long GetDayOfWeek() const;
/**
 * retruns the day of the UTCTime object
 * @return
 * day of month for the time you have specified.
 **/
    unsigned long GetDayOfMonth() const;

/**
 * retruns the day of the UTCTime object
 * @return
 * day of year for the time you have specified.
 **/
    unsigned long GetDayOfYear() const;
/**
 * retruns the month of the UTCTime object
 * @return
 * month for the time you have specified.
 **/
    unsigned long GetMonth() const;
/**
 * retruns the year the UTCTime object
 * @return
 * year for the time you have specified.
 **/
    unsigned long GetYear() const;

    CTimeSpan operator - ( const CUTCTime& other ) const;


/**
get the Hour (0..23)
*/
	unsigned long GetHour() const;
	
	unsigned long GetMinuteOfHour() const;
	unsigned long GetSecondOfMinute() const;
	unsigned long GetMilliSecondOfSecond() const;
	

/**
 * compare operator
 * @param
 * t UTC time to compare with
 * @returns true if equal
 * @returns false if not
 **/
    bool operator == (const CUTCTime& t ) const;

/**
 * shifts the time by adding the timepan
 *
 * @param
 * t the timespan
 * @returns
 * the shifted time object
**/
    CUTCTime operator + (const CTimeSpan& t ) const ;

/**
 * shifts the time by substracting the timepan
 *
 * @param
 * t the timespan
 * @returns
 * the shifted time object
**/
    CUTCTime operator - (const CTimeSpan& t ) const;


/**
 * compare operator
 * @param 
 * t UTC time to compare with
 * @returns true if less than
 * @returns false if greater than
 **/

    unsigned long long operator/( const CTimeSpan& d ) const;


    bool operator < (const CUTCTime& t ) const;

    bool operator <= (const CUTCTime& t ) const;

/**
 * The format function formats time according to the format specification format 
 * Ordinary characters placed in the format string are copied to  s  without  conversion.   Conversion
 * specifiers are introduced by a `%' character, and are replaced in the returned string as follows:
 *     - \%a     The abbreviated weekday name according to the current locale.
 *     - \%A     The full weekday name according to the current locale.
 *     - \%b     The abbreviated month name according to the current locale.
 *     - \%B     The full month name according to the current locale.
 *     - \%c     The preferred date and time representation for the current locale.
 *     - \%C     The century number (year/100) as a 2-digit integer. (SU)
 *     - \%d     The day of the month as a decimal number (range 01 to 31).
 *     - \%D     Equivalent  to  %m/%d/%y.  (Yecch - for Americans only.  Americans should note that in other
 *             countries %d/%m/%y is rather common. This means that in international context this format is
 *             ambiguous and should not be used.) (SU)
 *     - \%e     Like  %d,  the  day  of  the  month as a decimal number, but a leading zero is replaced by a
 *            space. (SU)
 *     - \%E     Modifier: use alternative format, see below. (SU)
 *     - \%F     Equivalent to %Y-%m-%d (the ISO 8601 date format). (C99)
 *     - \%G     The ISO 8601 year with century as a decimal number.  The 4-digit year corresponding  to  the
 *             ISO  week number (see %V).  This has the same format and value as %y, except that if the ISO
 *             week number belongs to the previous or next year, that year is used instead. (TZ)
 *     - \%g     Like %G, but without century, i.e., with a 2-digit year (00-99). (TZ)
 *     - \%h     Equivalent to %b. (SU)
 *     - %H     The hour as a decimal number using a 24-hour clock (range 00 to 23).
 *     - \%I     The hour as a decimal number using a 12-hour clock (range 01 to 12).
 *     - \%j     The day of the year as a decimal number (range 001 to 366).
 *     - \%k     The hour (24-hour clock) as a decimal number (range 0 to 23); single digits are preceded  by
 *            a blank. (See also %H.) (TZ)
 *     - \%l     The  hour (12-hour clock) as a decimal number (range 1 to 12); single digits are preceded by
 *            a blank. (See also %I.) (TZ)
 *     - \%m     The month as a decimal number (range 01 to 12).
 *     - \%M     The minute as a decimal number (range 00 to 59).
 *     - \%n     A newline character. (SU)
 *     - \%O     Modifier: use alternative format, see below. (SU)
 *     - \%p     Either `AM' or `PM' according to the given time value, or the corresponding strings for  the
 *     - \%P     Like %p but in lowercase: `am' or `pm' or a corresponding string for the current locale. (GNU)
 *     - \%r     The time in a.m. or p.m. notation.  In the POSIX locale this is equivalent to `%I:%M:%S %p'. (SU)
 *     - \%R     The time in 24-hour notation (%H:%M). (SU) For a version including the seconds, see %T below.
 *     - \%s     The number of seconds since the Epoch, i.e., since 1970-01-01 00:00:00 UTC. (TZ)
 *     - \%S     The second as a decimal number (range 00 to 61).
 *     - \%t     A tab character. (SU)
 *     - \%T     The time in 24-hour notation (%H:%M:%S). (SU)
 *     - \%u     The day of the week as a decimal, range 1 to 7, Monday being 1.  See also %w. (SU)
 *     - \%U     The  week number of the current year as a decimal number, range 00 to 53, starting with the 
 *             first Sunday as the first day of week 01. See also %V and %W.
 *     - \%V     The ISO 8601:1988 week number of the current year as a decimal number, range 01 to 53, where
 *             week 1 is the first week that has at least 4 days in the current year, and  with  Monday  as
 *             the first day of the week. See also %U and %W. (SU)
 *     - \%w     The day of the week as a decimal, range 0 to 6, Sunday being 0.  See also %u.
 *     - \%W     The  week  number of the current year as a decimal number, range 00 to 53, starting with the
 *             first Monday as the first day of week 01.
 *     - \%x     The preferred date representation for the current locale without the time.
 *     - \%X     The preferred time representation for the current locale without the date.
 *     - \%y     The year as a decimal number without a century (range 00 to 99).
 *     - \%Y     The year as a decimal number including the century.
 *     - \%z     The time-zone as hour offset from GMT.  Required to emit RFC822-conformant dates (using "%a,
 *             %d %b %Y %H:%M:%S %z"). (GNU)
 *     - \%Z     The time zone or name or abbreviation.
 *     - \%+     The date and time in date(1) format. (TZ)
 *     - \%%     A literal `%' character.
 *
 * @param 
 * Spec time format specification
 * @return
 * time-string with corresponding time, months: 1-12
 **/
    std::string Format(const std::string& Spec="%d %m %Y - %H:%M:%S");

 private:
    friend class CTimeSpan;
    CTimeSpan m_Time;
};

} 

#endif
