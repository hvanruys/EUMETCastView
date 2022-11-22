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


#include "UTCTime.h"
#include "ErrorHandling.h"
#include "string.h"


#ifdef SUN
struct tm *localtime_r(const time_t *timep, struct tm *result)
{
  struct tm *tmp = localtime( timep );
  memcpy( result, tmp, sizeof( struct tm ) );
  return tmp;
}
#endif


Util::CUTCTime::CUTCTime()
{
}

Util::CUTCTime::CUTCTime(const CUTCTime& t)
{
    m_Time = t.m_Time;
}

Util::CUTCTime::CUTCTime(const timeval& t)
{
    m_Time = t.tv_sec * Util::CTimeSpan::Second()
                           + t.tv_usec * Util::CTimeSpan::Microsecond();
}

Util::CUTCTime::CUTCTime(int sec,
             int     min,
             int     hour,
             int     mday,
             int     mon,
             int     year)

  {
    try
    {
      year -= 1900;
      mon -= 1;
      tm  t={sec,min,hour,mday,mon,year};
      m_Time = mktime(&t) * Util::CTimeSpan::Second();
    }
    catch(...)
    {
     LOGCATCHANDTHROW
    }
  }


Util::CUTCTime::CUTCTime(
    int     sec,
    int     min,
    int     hour,
    int     mday,
    int     mon,
    int     year,
    int     wday,
    int     yday,
    int     isdst)
{
    try
    {
	year -= 1900;
	mon -= 1;
	tm  t={sec,min,hour,mday,mon,year,wday,yday,isdst};
	m_Time = mktime(&t) * Util::CTimeSpan::Second();
   }
    catch(...)
    {
	LOGCATCHANDTHROW
    }
} 

Util::CUTCTime::CUTCTime(const Util::CTimeSpan& t)
	:m_Time( t )
{
}


Util::CUTCTime Util::CUTCTime::Now()
{
      timeval tmp;
      gettimeofday( &tmp, NULL );
      return CUTCTime(tmp);
}

unsigned long Util::CUTCTime :: GetDayOfWeek() const
{
    try
    {
        #ifdef __GNUC__
            const long int sec =  m_Time/Util::CTimeSpan::Second();
        #else
            long long int sec = m_Time/Util::CTimeSpan::Second();
        #endif
        tm* t_Time = localtime( &sec );
        unsigned long l_Time =  t_Time->tm_wday;
        return l_Time;
    }
    catch(...)
    {
         LOGCATCHANDTHROW
    }
}

unsigned long Util::CUTCTime :: GetDayOfMonth() const
{
    try
    {
        #ifdef __GNUC__
            const long int sec =  m_Time/Util::CTimeSpan::Second();
        #else
            long long int sec =  m_Time/Util::CTimeSpan::Second();
        #endif
        tm* t_Time = localtime(&sec);
        unsigned long l_Time =  t_Time->tm_mday;
        return l_Time;

    }
    catch(...)
    {
         LOGCATCHANDTHROW
    }
}

unsigned long Util::CUTCTime :: GetDayOfYear() const
{
    try
    {
        unsigned long l_Time;
        tm *t_Time;
        #ifdef __GNUC__
            const long int sec =  m_Time/Util::CTimeSpan::Second();
        #else
            long long int sec =  m_Time/Util::CTimeSpan::Second();
        #endif
        t_Time = localtime(&sec);
        l_Time =  t_Time->tm_yday;
        return l_Time;

    }
    catch(...)
    {
         LOGCATCHANDTHROW
    }
}

unsigned long Util::CUTCTime :: GetMonth() const
{

    try
    {
        unsigned long l_Time;
        tm *t_Time;
        #ifdef __GNUC__
            const long int sec =  m_Time/Util::CTimeSpan::Second();
        #else
            long long int sec =  m_Time/Util::CTimeSpan::Second();
        #endif
         t_Time = localtime(&sec);
        l_Time =  t_Time->tm_mon+1;
        return l_Time;

    }
    catch(...)
    {
         LOGCATCHANDTHROW
    }

}

unsigned long Util::CUTCTime :: GetYear() const
{

    try
    {
        unsigned long l_Time;
        tm *t_Time;
        #ifdef __GNUC__
            const long int sec =  m_Time/Util::CTimeSpan::Second();
        #else
            long long int sec =  m_Time/Util::CTimeSpan::Second();
        #endif
        t_Time = localtime(&sec);
        l_Time =  t_Time->tm_year + 1900;
        return l_Time;
    }
    catch(...)
    {
         LOGCATCHANDTHROW
    }

}


unsigned long Util::CUTCTime ::GetHour() const
{

    try
    {
	unsigned long long hoursPlusRemainder = m_Time / Util::CTimeSpan::Hour();
	// remove     
	unsigned long hour = hoursPlusRemainder % 24 ;   
	return hour;    
    }
    catch(...)
    {
         LOGCATCHANDTHROW
    }

}

unsigned long Util::CUTCTime::GetMinuteOfHour() const
{

    try
    {
	unsigned long long minutePlusRemainder = m_Time / Util::CTimeSpan::Minute();
	// remove     
	unsigned long m = minutePlusRemainder % 60 ;   
	return m;    
    }
    catch(...)
    {
         LOGCATCHANDTHROW
    }

}

unsigned long Util::CUTCTime::GetSecondOfMinute() const
{

    try
    {
	unsigned long long sPlusRemainder = m_Time / Util::CTimeSpan::Second();
	// remove     
	unsigned long m = sPlusRemainder % 60 ;   
	return m;    
    }
    catch(...)
    {
         LOGCATCHANDTHROW
    }

}


unsigned long Util::CUTCTime::GetMilliSecondOfSecond() const
{

    try
    {
	unsigned long long sPlusRemainder = m_Time / Util::CTimeSpan::Millisecond();
	// remove     
	unsigned long m = sPlusRemainder % 1000 ;   
	return m;    
    }
    catch(...)
    {
         LOGCATCHANDTHROW
    }

}


std::string Util::CUTCTime::Format(const std::string& Spec)
{
    try
    {
    std::string time(64,'0');
	char tmp[64];
	memset( tmp, '0', 64 );
    struct tm tm;
    #ifdef __GNUC__
        const long int sec =  m_Time/Util::CTimeSpan::Second();
    #else
        long long int sec =  m_Time/Util::CTimeSpan::Second();
    #endif
    
    ::localtime_r(&sec, &tm);

    size_t ret=strftime( tmp ,64, Spec.c_str(),&tm);
    if(ret == 0) return std::string("Format time failed");

	time = tmp;
    return time.substr(0,ret);
    }
    catch(...)
    {
        LOGCATCHANDTHROW
    }
}

Util::CTimeSpan Util::CUTCTime::operator - ( const CUTCTime& other )const
{
      return (m_Time - other.m_Time);
}

bool Util::CUTCTime::operator == (const CUTCTime& t ) const
{
   return m_Time == t.m_Time;
}

bool Util::CUTCTime::operator < (const CUTCTime& t ) const
{
    return (m_Time < t.m_Time);
}

bool Util::CUTCTime::operator <= (const CUTCTime& t ) const
{
    return (m_Time <= t.m_Time);
}

Util::CUTCTime Util::CUTCTime::operator + (const Util::CTimeSpan& t ) const
{
        CUTCTime result;
        result.m_Time = m_Time + t;
        return result;
}

Util::CUTCTime Util::CUTCTime::operator - (const Util::CTimeSpan& t ) const
{
    try{
      CUTCTime result;
      result.m_Time = m_Time - t;
      return result;
    }
    catch(...)
    {
        LOGCATCHANDTHROW
    }
}

unsigned long long  Util::CUTCTime::operator/( const CTimeSpan& d ) const
{
    return m_Time/d;
}

/*Util::CUTCTime  Util::CUTCTime::operator/( unsigned int d ) const
{
    CUTCTime result;
    result.m_Time = m_Time / d;
    return result;
} */

