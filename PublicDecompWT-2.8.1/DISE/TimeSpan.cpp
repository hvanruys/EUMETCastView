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
#include "TimeSpan.h"
#include "ErrorHandling.h"

Util::CTimeSpan::CTimeSpan(const CTimeSpan& t)
{
    m_Nanoseconds = t.m_Nanoseconds;
}

Util::CTimeSpan Util::CTimeSpan::operator + (const CTimeSpan& ts ) const
{
       CTimeSpan result;
       result.m_Nanoseconds = m_Nanoseconds + ts.m_Nanoseconds;
       return result;
}

Util::CUTCTime Util::CTimeSpan::operator + (const CUTCTime& t ) const
{
    Util::CUTCTime result;
    result.m_Time = *this + t.m_Time;
    return result;
}

Util::CTimeSpan Util::CTimeSpan::operator - (const CTimeSpan& ts ) const
{
       CTimeSpan result;
       result.m_Nanoseconds = m_Nanoseconds - ts.m_Nanoseconds;
       return result;
}

Util::CTimeSpan Util::CTimeSpan::operator * ( int mul ) const
{
       CTimeSpan result;
       result.m_Nanoseconds = m_Nanoseconds * mul;
       return result;
}

Util::CTimeSpan Util::CTimeSpan::operator * ( float mul ) const
{
       CTimeSpan result;
       result.m_Nanoseconds = (unsigned long long) ( float(m_Nanoseconds) * mul );
       return result;
}


Util::CTimeSpan Util::CTimeSpan::operator / ( int div ) const
{
       CTimeSpan result;
       result.m_Nanoseconds = m_Nanoseconds / div;
       return result;
}

unsigned long long Util::CTimeSpan::operator / ( const CTimeSpan& ts  ) const
{
       return  m_Nanoseconds / ts.m_Nanoseconds;
}

Util::CTimeSpan Util::CTimeSpan::operator % ( const CTimeSpan& ts  ) const
{
       CTimeSpan result;
       result.m_Nanoseconds =  m_Nanoseconds % ts.m_Nanoseconds;
       return result;
}
