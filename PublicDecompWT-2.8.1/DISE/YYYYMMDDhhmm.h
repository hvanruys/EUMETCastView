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

#ifndef YYYYMMDDhhmm_included
#define YYYYMMDDhhmm_included

/*******************************************************************************

TYPE:
Free functions.
					
PURPOSE:
Conversion of a CMSGTime object from/to a 'YYYYMMDDhhmm'- or
'YYYYMMDDhhmmss'-formatted string.

FUNCTION:
Conversion of a CMSGTime object from/to a 'YYYYMMDDhhmm'- or
'YYYYMMDDhhmmss'-formatted string.

INTERFACES:
See 'INTERFACES:' in the module declaration below.

RESOURCES:	
None.

REFERENCES:
None.

PROCESSING:
See below.

DATA:
See 'DATA:' in the class header below.

LOGIC:		
-

*******************************************************************************/

#include <string>
#include "ErrorHandling.h"	// Util
#include "Types.h"


namespace DISE
{




// Description:	Converts a 'YYYYMMDDhhmm'-formatted time string into a CMSGTime object.
// Returns:		A CMSGTime object.

inline SYSTIME YYYYMMDDhhmm
(
	const std::string& i_Time
)
{
	try
	{
		PRECONDITION(i_Time.size() >= 12);
		unsigned int year	=	(i_Time[ 0] - '0') * 1000
							  + (i_Time[ 1] - '0') *  100
							  + (i_Time[ 2] - '0') *   10
							  + (i_Time[ 3] - '0') *    1;
		unsigned int month	=	(i_Time[ 4] - '0') *   10
							  + (i_Time[ 5] - '0') *    1;
		unsigned int day	=	(i_Time[ 6] - '0') *   10
							  + (i_Time[ 7] - '0') *    1;
		unsigned int hour	=	(i_Time[ 8] - '0') *   10
							  + (i_Time[ 9] - '0') *    1;
		unsigned int minute	=	(i_Time[10] - '0') *   10
							  + (i_Time[11] - '0') *    1;
		return SYSTIME(year, month, day, hour, minute);
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




// Description:	Converts a CMSGTime object into a 'YYYYMMDDhhmm'-formatted string.
// Returns:		Time value as a YYYYMMDDhhmm'-formatted string.
inline std::string YYYYMMDDhhmm
(
 	SYSTIME& i_Time
)
{
	try

	{
#ifdef WIN32
		return i_Time.FormatDate("yyyyMMdd") + i_Time.FormatTime("HHmm");
#else
		return i_Time.Format("yyyyMMddHHmm");
#endif
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




// Description:	Converts a 'YYYYMMDDhhmmss'-formatted time string into a CMSGTime object.
// Returns:		A CMSGTime object.
inline SYSTIME YYYYMMDDhhmmss
(
	const std::string& i_Time
)
{
	try
	{
		PRECONDITION(i_Time.size() >= 14);
		unsigned int year	=	(i_Time[ 0] - '0') * 1000
							  + (i_Time[ 1] - '0') *  100
							  + (i_Time[ 2] - '0') *   10
							  + (i_Time[ 3] - '0') *    1;
		unsigned int month	=	(i_Time[ 4] - '0') *   10
							  + (i_Time[ 5] - '0') *    1;
		unsigned int day	=	(i_Time[ 6] - '0') *   10
							  + (i_Time[ 7] - '0') *    1;
		unsigned int hour	=	(i_Time[ 8] - '0') *   10
							  + (i_Time[ 9] - '0') *    1;
		unsigned int minute	=	(i_Time[10] - '0') *   10
							  + (i_Time[11] - '0') *    1;
		unsigned int second	=	(i_Time[12] - '0') *   10
							  + (i_Time[13] - '0') *    1;
		return SYSTIME(year, month, day, hour, minute, second);
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




// Description:	Converts a CMSGTime object into a 'YYYYMMDDhhmmss'-formatted string.
// Returns:		Time value as a YYYYMMDDhhmmss'-formatted string.
inline std::string YYYYMMDDhhmmss
(
	SYSTIME& i_Time
)
{
	try
	{
#ifdef WIN32
		return i_Time.FormatDate("yyyyMMdd") + i_Time.FormatTime("HHmm");
#else
		return i_Time.Format("yyyyMMddHHmm");
#endif
	
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




} // end namespace


#endif

