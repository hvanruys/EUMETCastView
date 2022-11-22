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

#ifndef _CMSG_TIME_HEADER_
#define _CMSG_TIME_HEADER_

/**************************************************************************
****  DADF MODULE HEADER   ***


TYPE:			Concrete class
					

PURPOSE:		The Time and Time span classes


FUNCTION:		To allow easy access to timespans and times in many formats 


INTERFACES:		See Below:


RESOURCES:		None:


PROCESSING:		


DATA:			See Below:


LOGIC:			


****  END MODULE HEADER   ***
**************************************************************************/
/*	MFC provides the CTime and CTimespan classes, which provide general 
	time and time difference functionality. The classes however do not provide all 
	the functionality needed particularly as they do not support time down to the 
	granularity of milliseconds. We therefore need to derive a specific set of classes 
	to deal with functionality required.

    The internal representation for our specific classes CMsgTime and CMsgTimeSpan 
	could be done in several ways. 
	1) Our own internal representation.
	2) Win32 FILETIME
	3) Win32 SYSTEMTIME

	FILETIME provides accuracy down to the nanosecond although this is not required 
	from the specification given in Appendix F it does give us the ability to maintain this level
	of granularity :-
	typedef struct _FILETIME {
	WORD dwLowDateTime;   
    WORD dwHighDateTime;  
	} FILETIME, *PFILETIME, *LPFILETIME;

	It is felt that the FILETIME structure allows a more efficent method of storing dates and 
	although this structure is not particularly human compliant the addition of days etc is 
	slightly easier and comes down to the addition of a WORD onto the date.

    To be able to do this it is only going to be possible - without extensive checking 
	to be able to add timespans of weeks being the largest unit of granularity. 
	If this is going to be a problem then it may need modification at a later date. 
	This does ease up the development of the timespan class as, when is a Month after the 
	30th Jan, is it the 1/2 March depending upon the Leap year factor.

	Nb1 : although it appears that comms packet structures need to communicate to the nearest 
	n-sec the spec only requires to millisec.

	Nb2 : The epoc for the Win32 is Jan 1st 1601, the epoc for MSG, 
		  given in appendix F is Jan 1st 1958. Any initialisation of the 
		  CMSGTime class without parameters will set the date to MSG 
		  epoch.
*/

#ifdef XTRA_COM_INFO
	#pragma message( "Compiling " __FILE__ ) 
#endif


//
// System includes
//
#include <windows.h>

//
// STL Includes
//
#include <string>


//
// Needed for the div operator.
//
#include <stdlib.h>
#include <winnls.h>

#include <objbase.h> // needed for CoFileTimeNow.
#include <memory> 

//
// Local includes.
//
#include "ErrorHandling.h"


//
// Pre class definitions.
//
namespace Util
{
	class CMSGTime;
	class CMSGTimeSpan;
}


/////////////////////////////////////////////////////////////
//
//	ClassName:		CMSGTime
//	
//	Date Created:	11/12/1997
//
//	Author:			Paul Baker
//
//
//	Purpose:		To hold and manipulate time information compliant with both the 
//					MSG wide requirements and any further DADF specific requirements.	
//
//	Last Updated:	
//
//
//////////////////////////////////////////////////////////////
enum E_CDS_DEFINES 
  { 	
	E_MIN_NANOSECONDS = 0,
	E_MAX_NANOSECONDS = 1000,
	E_MIN_MILLISECONDS  = 0,
	E_MAX_MILLISECONDS  = 1000,
	E_MIN_SECOND  = 0,
	E_MAX_SECOND  = 60,
	E_MIN_MINUTE  = 0,
	E_MAX_MINUTE  = 60,
	E_MIN_HOUR  = 0,
	E_MAX_HOUR  = 24,
	E_MIN_DAY  = 1,
	E_MAX_DAY  = 31,
	E_MIN_MONTH  = 1,
	E_MAX_MONTH  = 12,
	E_MAX_DATE_LEN = 30,
	E_MAX_TIME_LEN = E_MAX_DATE_LEN,
	E_TIME_PARSE_ELEMENTS = 5,
	E_MAX_DATE_SEGMENT_SIZE = 5
  };

struct TIME_CDS_SHORT
{
	unsigned short	day;
	unsigned int  milliseconds;

	

	TIME_CDS_SHORT& operator = (const TIME_CDS_SHORT i_Time)
		{day	= i_Time.day;
		 milliseconds = i_Time.milliseconds;
		return *this;};

	bool operator == (const TIME_CDS_SHORT& i_Time) const
		{return day	 == i_Time.day &&
		milliseconds == i_Time.milliseconds; };


};

namespace Util
{
	class CMSGTime
	{
			// member variables.
		public:
			// DATA:
			FILETIME m_sTimeHolder;

		enum E_MSG_DAY_TYPE { e_MSG_DAY_TYPE_START = 0,
						  e_Sun = 1, 
						  e_Mon, 
						  e_Tue, 
						  e_Wed, 
						  e_Thur, 
						  e_Fri, 
						  e_Sat, 
						  e_MSG_DAY_TYPE_LEN };


		enum E_MSG_MONTH_TYPE { e_MSG_MONTH_TYPE_START = 0,
						  e_Jan = 1, 
						  e_Feb, 
						  e_Mar, 
						  e_Apr, 
						  e_May, 
						  e_Jun, 
						  e_Jul, 
						  e_Aug, 
						  e_Sep, 
						  e_Oct, 
						  e_Nov, 
						  e_Dec,  
						  e_MSG_MONTH_TYPE_LEN };


		enum e_DAYS_IN_YEAR
		{
			e_DaysInNormalYear = 365,
			e_DaysInLeapYear = 366
		};


    		// Member functions.
		public:

			// Description
			//		Constructor
			//		This is aso the default constructor.
			// Arguments
			//		i_wYear		The year
			//		i_wMonth	The month
			//		i_wHour		The hour
			//		i_wMinute	The minute
			//		i_wSecond	The second
			//		i_wMilliseconds	The Millisecs
			explicit CMSGTime(
					WORD i_wYear = 1958, 
					WORD i_wMonth  =  1,
					WORD i_wDay = 1,
					WORD i_wHour =  0,
					WORD i_wMinute =  0,
					WORD i_wSecond =  0,
					WORD i_wMilliseconds =  0);


			// Description
			//		Constructor
			// Arguments
			//		i_sTime		Appendix F time constructor
			explicit CMSGTime(const TIME_CDS_SHORT& i_sTime);

			// Description
			//		Constructor
			// Arguments
			//		i_sTime		Appendix F time constructor
			explicit CMSGTime(const SYSTEMTIME& i_sSystemTime);

			 
			// Description
			//		Constructor
			// Arguments
			//		i_cMSGTime		Copy constructor
			CMSGTime(const CMSGTime& i_cMSGTime);


			// Description
			//		Constructor
			// Arguments
			//		i_sFileTime		The file time
			explicit CMSGTime(const FILETIME& i_sFileTime);


			 // Description
			 //		Destructor
			 ~CMSGTime();


			// Descripton
			//		Output format member variables.
			//		needed for communications....
			//		The TIME_CDS_SHORT is a packed structure.
			//		The other CDS time functions are not needed at this time.
			
			
			 TIME_CDS_SHORT GetCDSTimeShort() const;


			// Descripton
			//		Floor/Ceiling functions.
			//		These return the floor/ceiling for the specified portion of the date. 
			//		All detail below that level is set to its base level i.e. seconds to 
			//		0, day to 1 etc...
			// Returns
			//		CMSGTime	the Floor/Celing data
			 CMSGTime  GetSecondFloor() const;
			 CMSGTime  GetSecondCeiling() const;
			
			 CMSGTime  GetMinuteFloor() const;
			 CMSGTime  GetMinuteCeiling() const;
			
			 CMSGTime  GetHourFloor() const;
			 CMSGTime  GetHourCeiling() const;
			
			 CMSGTime  GetDayFloor() const;
			 CMSGTime  GetDayCeiling() const;
			
			 CMSGTime  GetMonthFloor() const;
			 CMSGTime  GetMonthCeiling() const;
			
			 CMSGTime  GetYearFloor() const;
			 CMSGTime  GetYearCeiling() const;
			
			// Descripton
			//		General Access methods to the elements of a time stamp
			// Returns
			//		WORD	The value requested	
			// 
			 WORD GetMilliseconds() const;
			 WORD GetSecond() const;
			 WORD GetMinute() const;
			 WORD GetHour() const;
			 WORD GetDay() const;
			 WORD GetDayOfWeek() const;
			 WORD GetDayOfYear() const;
			 WORD GetMonth() const;
			 WORD GetYear() const;

			 
			 // Description
			//		Return the NT File time struct.
			// Returns
			//		FILETIME	the information requested
			FILETIME GetNTFileTime() const;

			// Description
			//		Return the NT system time struct.
			// Returns
			//		SYSTEMTIME	the information requested
			SYSTEMTIME GetNTSystemTime() const;


			//Description 
			//	Checks CMSGTime object and returns tru if it is equal to  default
			//	time and date.
			//Returns
			//	Boolean

			bool IsEpoch() const; 
			
			

			// Description
			//		Get the current time (UTC) in a CMSGTime class.
			//		Note this returns a new object.
			// Returns
			//		CMSGTime	The current time (UTC) 
			static CMSGTime GetTheCurrentTime();

			// Description
			//		Supply a time in a string and return a CMSGTime class 
			//		The format will be in the form.
			//		YYYY.DDD.HH.MM.SS
			//		YYYY =	Year
			//		DDD	=	Day of Year
			//		HH	=	Hour of Day
			//		MM	=	Minute of Hour
			//		SS	=	Second of Minute.
			//		If any other string is supplied it will be ignored.
			//		Note: This does not alter the current object. 
			// Arguments
			//		i_csTimeString	The time format string
			// Returns
			//		CMSGTime	the time class
			static CMSGTime Parse(std :: string i_csTimeString);
			
			
			// Description
			//		Supply a format string and then return the time held in the current 
			//		object using the supplied formatting information. 
			//		The format identifiers will be in the form given in the MMI specification
			//		h	Hours with no leading zero for single-digit hours; 12-hour clock
			//		hh	Hours with leading zero for single-digit hours; 12-hour clock
			//		H	Hours with no leading zero for single-digit hours; 24-hour clock
			//		HH	Hours with leading zero for single-digit hours; 24-hour clock
			//		m	Minutes with no leading zero for single-digit minutes
			//		mm	Minutes with leading zero for single-digit minutes
			//		s	Seconds with no leading zero for single-digit seconds
			//		ss	Seconds with leading zero for single-digit seconds
			//		t	One character time marker string, such as A or P
			//		tt	Multicharacter time marker string, such as AM or PM
			//		For example, to get the time string
			//				"11:29:40 PM"
			//		use the following picture string:
			//				"hh':'mm':'ss tt
			// Argument
			//		i_csFormat	The format string to specify the format of the string
			// Returns
			//		std::string	The time in the specified format

			std::string FormatTime(std::string i_csFormat = (std::string) 
					"HH':'mm':'ss") const;


			//Description
			//Supply a format string and then return the date held in the current
			//object using the supplied formatting infomration
			// The format identifiers will be the the form given in the MMI specification
			//		d	Day of month as digits with no leading zero for single-digit days.
			//		dd	Day of month as digits with leading zero for single-digit days.
			//		ddd	Day of week as a three-letter abbreviation. 
			//			The function uses the LOCALE_SABBREVDAYNAME value associated with the 
			//			specified locale.
			//		dddd Day of week as its full name. 
			//			The function uses the LOCALE_SDAYNAME value associated with the 
			//			specified locale.
			//		M	Month as digits with no leading zero for single-digit months.
			//		MM	Month as digits with leading zero for single-digit months.
			//		MMM	Month as a three-letter abbreviation. 
			//			The function uses the LOCALE_SABBREVMONTHNAME value associated 
			//			with the specified locale.
			//		MMMM	Month as its full name. 
			//			The function uses the LOCALE_SMONTHNAME value associated with the 
			//			specified locale.
			//		y	Year as last two digits, but with no leading zero for years less than 10.
			//		yy	Year as last two digits, but with leading zero for years less than 10.
			//		yyyy	Year represented by full four digits.
			//		gg	Period/era string. 
			//			The function uses the CAL_SERASTRING value associated with the specified 
			//			locale. 
			//			This element is ignored if the date to be formatted does not have an 
			//			associated era or period string.
			//		For example, to get the date string
			//			"Wed, Aug 31 94"
			//		Use the following picture string:
			//			"ddd',' MMM dd yy" 
			//
			// Argument
			//		i_csFormat	The format string to specify the format of the string
			// Returns
			//		std::string	The time in the specified format
			std::string FormatDate(std::string i_csFormat = (std::string) 
					"ddd',' MMM dd yy") const;


			// Description:	Provides the time in a string format compatible with
			//				Oracle's DATE type ('YYYY MM DD HH24 MI SS').
			// Returns:		The time in a string format compatible with Oracle's DATE type.
			std::string GetOracleDATE
			(
			)
			const
			{
				return	  FormatDate("yyyy MM dd"         )
						+ FormatTime(          " HH mm ss");
			}

			// Description:	Provides the time in a string format ('DDD.HH24:MI:SS').
			// Returns:		The time in a string format compatible with Oracle's DATE type.
			std::string GetDayOfTheYearTime() const;
			

			// Description:	Creates an MSGTime object from a string formatted as
			//				Oracle's DATE type ('YYYY MM DD HH24 MI SS').
			// Returns:		New MSGTime object.
			static Util::CMSGTime ParseOracleDATE
			(
				const std::string& i_OracleDate	// Time stamp in string format.
			)
			{
				int year, month, day, hours, minutes, seconds;
				sscanf(i_OracleDate.c_str(), "%d %d %d %d %d %d",
									&year, &month, &day, &hours, &minutes, &seconds);
				return Util::CMSGTime(year, month, day, hours, minutes, seconds);
			}

			std::string ReadableForm() const
			{
				return GetOracleDATE();
			}


			// Description
			//		set the individual element of a CMSGTime 
			//		to be to a particular value.
			// Arguments
			//		i_wMilliseconds		The Millisecond values to set
			 void SetMilliseconds(WORD i_wMilliseconds);

			// Description
			//		set the individual element of a CMSGTime 
			//		to be to a particular value.
			// Arguments
			//		i_wSecond		The Second values to set
			 void SetSecond(WORD i_wSecond);

			// Description
			//		set the individual element of a CMSGTime 
			//		to be to a particular value.
			// Arguments
			//		i_wMinute		The Minute values to set
  		    void SetMinute(WORD i_wMinute);

			// Description
			//		set the individual element of a CMSGTime 
			//		to be to a particular value.
			// Arguments
			//		i_wHour		The Hour values to set
			 void SetHour(WORD i_wHour);

			// Description
			//		set the individual element of a CMSGTime 
			//		to be to a particular value.
			// Arguments
			//		i_wDay		The Day values to set
			 void SetDay(WORD i_wDay);

			// Description
			//		set the individual element of a CMSGTime 
			//		to be to a particular value.
			// Arguments
			//		i_wDay		The Day values to set
			 void SetDayOfYear(WORD i_wDay); 
				// note this will also affect the day and Month settings.

			// Description
			//		set the individual element of a CMSGTime 
			//		to be to a particular value.
			// Arguments
			//		i_wMonth		The Month values to set
			 void SetMonth(WORD i_wMonth);

			// Description
			//		set the individual element of a CMSGTime 
			//		to be to a particular value.
			// Arguments
			//		i_wYear		The Year values to set
			 void SetYear(WORD i_wYear);
			
			 // Description
			 //		Overloaded = operator. 
			 // Arguments
			 //		i_Time	The time to set this one to
			 //	Returns
			 //		CMSGtime	The New time now the = has been applied.
			 CMSGTime& operator = (const CMSGTime& i_Time);
			 
			 // Description
			 //		Overloaded = operator. 
			 // Arguments
			 //		i_Time	The time to set this one to
			 //	Returns
			 //		CMSGtime	The New time now the = has been applied.
			 CMSGTime& operator = (const FILETIME& i_sTime);

			 // Description
			 //		Overloaded = operator. 
			 // Arguments
			 //		i_Time	The time to set this one to
			 //	Returns
			 //		CMSGtime	The New time now the = has been applied.
			 CMSGTime& operator = (const SYSTEMTIME& i_sTime);

			 // Description
			 //		Overloaded = operator. 
			 // Arguments
			 //		i_Time	The time to set this one to
			 //	Returns
			 //		CMSGtime	The New time now the = has been applied.
			 CMSGTime& operator = (const TIME_CDS_SHORT& i_sTime);

			 // Description
			 //		Arithmetic + operator.
			 // Arguments
			 //		i_CTimeSpan		The Timespan used in the calculation
			 //	Returns
			 //		CMSGTime		The New Time
			 CMSGTime	 operator + (const CMSGTimeSpan& i_cTimeSpan) const ;

			 // Description
			 //		Arithmetic - operator.
			 // Arguments
			 //		i_CTimeSpan		The Timespan used in the calculation
			 //	Returns
			 //		CMSGTime		The New Time
			 CMSGTime     operator - (const CMSGTimeSpan& i_cTimeSpan) const ;

			 // Description
			 //		Arithmetic - operator.
			 // Arguments
			 //		i_cTime		The Time used in the calculation
			 //	Returns
			 //		CMSGTimeSpan		The New Time
			 CMSGTimeSpan operator - (const CMSGTime& i_cTime) const ;


			 // Description
			 //		Arithmetic += operator.
			 // Arguments
			 //		i_CTimeSpan		The Timespan used in the calculation
			 //	Returns
			 //		CMSGTime		The New Time
			 CMSGTime& operator += (const CMSGTimeSpan& i_TimeSpan);

			 // Description
			 //		Arithmetic -= operator.
			 // Arguments
			 //		i_CTimeSpan		The Timespan used in the calculation
			 //	Returns
			 //		CMSGTime		The New Time
			 CMSGTime& operator -= (const CMSGTimeSpan& i_TimeSpan);
 			
			 
			 // Description
			 //		boolean  == operator.
			 // Arguments
			 //		i_Time		The Time used in the calculation
			 //	Returns
			 //		bool		Outcome of the comparison
			 bool operator == (const CMSGTime& i_Time) const;

			 // Description
			 //		boolean  < operator.
			 // Arguments
			 //		i_Time		The Time used in the calculation
			 //	Returns
			 //		bool		Outcome of the comparison
			 bool operator <  (const CMSGTime& i_Time) const;

			 // Description
			 //		boolean  > operator.
			 // Arguments
			 //		i_Time		The Time used in the calculation
			 //	Returns
			 //		bool		Outcome of the comparison
			 bool operator >  (const CMSGTime& i_Time) const;

			 // Description
			 //		boolean  <= operator.
			 // Arguments
			 //		i_Time		The Time used in the calculation
			 //	Returns
			 //		bool		Outcome of the comparison
			 bool operator <= (const CMSGTime& i_Time) const;

			 // Description
			 //		boolean  >= operator.
			 // Arguments
			 //		i_Time		The Time used in the calculation
			 //	Returns
			 //		bool		Outcome of the comparison
			 bool operator >= (const CMSGTime& i_Time) const;

			
		
		
	};

} // end of the namespace



/////////////////////////////////////////////////////////////
//
//	ClassName:		CMSGTimeSpan
//	
//	Date Created:	12/12/1997
//
//	Author:			Paul Baker
//
//
//	Purpose:		To hold and manipulate time span information compliant with both the 
//					MSG wide requirements and any further DADF specific requirements.	
//
//	Last Updated:	
//
//
//////////////////////////////////////////////////////////////
namespace Util
{
	class CMSGTimeSpan
	{
		// Member variables.
	private:
		__int64  m_iTimeSpan;
	public:

		// Description
		//		Construtor
		//	Arguments
		//		__int64		time span in hundred nano second intervals
		explicit CMSGTimeSpan(__int64 i_wHundredNanosecs = 0);

		
		// Description
		//		Construtor
		//	Arguments
		//		i_sTimeSpan		Timespan to set this object to 
		CMSGTimeSpan(const CMSGTimeSpan& i_sTimeSpan);

		// Description
		//		Destructor
		~CMSGTimeSpan();

		// Description
		//		Getter function
		// Returns
		//		__int62		The span in Days
		 __int64 GetSpanInDays()		 const;

		// Description
		//		Getter function
		// Returns
		//		__int62		The span in Hours
		 __int64 GetSpanInHours()		 const;

		// Description
		//		Getter function
		// Returns
		//		__int62		The span in Minutes
		 __int64 GetSpanInMinutes()		 const;
		 
		// Description
		//		Getter function
		// Returns
		//		__int62		The span in Seconds
		 __int64 GetSpanInSeconds()		 const;

		// Description
		//		Getter function
		// Returns
		//		__int62		The span in Milliseonds
		 __int64 GetSpanInMilliseconds() const;
		

		// Description
		//		Getter function
		// Returns
		//		WORD	The Day part of the span
		 WORD GetDayPart()			 const;

		// Description
		//		Getter function
		// Returns
		//		WORD	The Hour part of the span
		 WORD GetHourPart()			 const;

		// Description
		//		Getter function
		// Returns
		//		WORD	The Minute part of the span
		 WORD GetMinutePart()		 const;

		// Description
		//		Getter function
		// Returns
		//		WORD	The Second part of the span
		 WORD GetSecondPart()		 const;

		// Description
		//		Getter function
		// Returns
		//		WORD	The Millisecond part of the span
		 WORD GetMillisecondPart()	 const;

		
		// Description
		//		Getter function
		// Returns
		//		__int64		The timespan in Millisecond
		 __int64 GetTimeSpan()		 const; 
		

 		// Description
		//		Setter function
		// Argument
		//		i_wDay		The span in Days
		 void SetSpanInDays(__int64 i_wDay);

		// Description
		//		Setter function
		// Argument
		//		i_wHour		The span in Hour
		 void SetSpanInHours(__int64 i_wHour);

		// Description
		//		Setter function
		// Argument
		//		i_wMinute		The span in Minutes
		 void SetSpanInMinutes(__int64 i_wMinute);

		// Description
		//		Setter function
		// Argument
		//		i_wDay		The span in Seconds
		 void SetSpanInSeconds(__int64 i_wSecond);

		// Description
		//		Setter function
		// Argument
		//		i_wMillisecond		The span in Millisecond
		 void SetSpanInMilliseconds(__int64 i_wMillisecond);

		// Description
		//		Setter function
		// Argument
		//		i_wTimeSpan		The span
		 void SetTimeSpan(__int64 i_wTimeSpan); 

		// Description
		//		Setter function
		// Argument
		//		i_wTimeSpan		The span
		 void SetTimeSpan(const CMSGTimeSpan& i_wTimeSpan); 

		//
		// Increment functions to remove the need from the user to do any bounds
		// Checking. i.e. makes sure that if you increment the seconds and in the process set 
		// seconds > 60 then it will set seconds to 0 and then increment the minutes.
		//

		// Description
		//		Incrementer function
		// Argument
		//		i_wMilliseconds		The Increment value
		 void IncMilliseconds(__int64 i_wMilliseconds = 1);

		// Description
		//		Incrementer function
		// Argument
		//		i_wSecond		The Increment value
		 void IncSecond(__int64 i_wSecond =1);

		// Description
		//		Incrementer function
		// Argument
		//		i_wMinute		The Increment value
		 void IncMinute(__int64 i_wMinute = 1);

		// Description
		//		Incrementer function
		// Argument
		//		i_wHour		The Increment value
		void IncHour(__int64 i_wHour =1);

		// Description
		//		Incrementer function
		// Argument
		//		i_wDay		The Increment value
		void IncDay(__int64 i_wDay =1);

		//
		// Decrement functions to remove the need from the user to do any bounds
		// Checking. i.e. makes sure that if you Decrement the seconds and in the process set 
		// seconds < 0  then it will set seconds to 59 and then decrement the minutes.
		//

		// Description
		//		Decrementer function
		// Argument
		//		i_wMilliseconds		The Decrement value
		 void DecMilliseconds(__int64 i_wMilliseconds = 1);

		// Description
		//		Decrementer function
		// Argument
		//		i_wSecond		The Decrement value
		 void DecSecond(__int64 i_wSecond =1);

		// Description
		//		Decrementer function
		// Argument
		//		i_wMinute		The Decrement value
		void DecMinute(__int64 i_wMinute = 1);

		// Description
		//		Decrementer function
		// Argument
		//		i_wHour		The Decrement value
		void DecHour(__int64 i_wHour =1);

		// Description
		//		Decrementer function
		// Argument
		//		i_wDay		The Decrement value
		void DecDay(__int64 i_wDay =1);

		// Description
		//		= operator 
		// Argument
		//		i_CTimeSpan		The timspan to operate on
		 CMSGTimeSpan& operator =  (const CMSGTimeSpan& i_cTimeSpan);

		// Description
		//		+= operator 
		// Argument
		//		i_CTimeSpan		The timspan to operate on
		 CMSGTimeSpan& operator += (const CMSGTimeSpan& i_cTimeSpan);

		// Description
		//		-= operator 
		// Argument
		//		i_CTimeSpan		The timspan to operate on
		 CMSGTimeSpan& operator -= (const CMSGTimeSpan& i_cTimeSpan);
		


		// Description
		//		numerical operator 
		// Argument
		//		i_CTimeSpan		The timspan to apply
		 CMSGTimeSpan operator + (const CMSGTimeSpan& i_cTimeSpan) const;

		// Description
		//		numerical operator 
		// Argument
		//		i_CTimeSpan		The timspan to apply
		 CMSGTimeSpan operator - (const CMSGTimeSpan& i_cTimeSpan) const;

		// Description
		//		numerical operator 
		// Argument
		//		multiplier		The multiplyer to apply
		 CMSGTimeSpan operator * (int multiplier )				  const; 

		// Description
		//		numerical operator 
		// Argument
		//		multiplier		The multiplier to apply
		 CMSGTimeSpan operator / (int multiplier )				  const;
		

		// Description
		//		Boolean comparators.
		// Argument
		//		i_cTimeSpan		The Timespan to compare with
		 bool operator == (const CMSGTimeSpan& i_cTimeSpan) const;

		// Description
		//	Boolean comparators.
		// Arguent
		//		i_cTimeSpan		The Timespan to compare with
		 bool operator <  (const CMSGTimeSpan& i_cTimeSpan) const;

		// Description
		//		Boolean comparators.
		// Argument
		//		i_cTimeSpan		The Timespan to compare with
		 bool operator >  (const CMSGTimeSpan& i_cTimeSpan) const;

		// Description
		//		Boolean comparators.
		// Argument
		//		i_cTimeSpan		The Timespan to compare with
		 bool operator <= (const CMSGTimeSpan& i_cTimeSpan) const;

		// Description
		//		Boolean comparators.
		// Argument
		//		i_cTimeSpan		The Timespan to compare with
		 bool operator >= (const CMSGTimeSpan& i_cTimeSpan) const;

		//
		// Range of nanosecond default values for Week, Day, Hour etc....
		//
		static CMSGTimeSpan& Zero();
		static CMSGTimeSpan& Day();
		static CMSGTimeSpan& Hour();
		static CMSGTimeSpan& Minute();
		static CMSGTimeSpan& Second();
		static CMSGTimeSpan& Millisecond();

	
	 };
} // end of the namspace 


namespace Util
{
	// Description
	//		* operator
	// Arguments
	//		i_iMultiplier	The multiplier
	//		i_Span			The span to use
	// Returns
	//		__int64			The timespan in milliseconds
	 __int64 operator * ( const __int64 i_iMultiplier,const Util::CMSGTimeSpan& i_Span  );

	// Description
	//		+ operator
	// Arguments
	//		i_iAdd			The Additive
	//		i_Span			The span to use
	// Returns
	//		__int64			The timespan in milliseconds
     __int64 operator + ( const __int64  i_iAdd,const Util::CMSGTimeSpan& i_Span  ) ;

	// Description
	//		/ operator
	// Arguments
	//		i_iDiv			The divisor
	//		i_Span			The span to use
	// Returns
	//		__int64			The timespan in milliseconds
     __int64 operator / ( const __int64 i_iDiv,const Util::CMSGTimeSpan& i_Span  ) ;

	// Description
	//		- operator
	// Arguments
	//		i_iSub			The subtractor
	//		i_Span			The span to use
	// Returns
	//		__int64			The timespan in milliseconds
     __int64 operator - ( const __int64 i_iSub,const Util::CMSGTimeSpan& i_Span  );

	// Description
	//		Output operator.
	// Arguments
	//		o_Stream		The stream
	//		time			The time class to use
	// Returns
	//		std::ostream	The stream with the timespan on it.
    std::ostream& operator << (std::ostream& o_Stream, const Util::CMSGTime& time );

	// Description
	//		Insertion operator.
	// Arguments
	//		i_Stream		The stream
	//		time			The time class to use
	// Returns
	//		std::ostream	The stream with the timespan on it.
    std::istream& operator >> (std::istream& i_Stream, Util::CMSGTime& time );

	// Description
	//		Extraction operator.
	// Arguments
	//		o_Stream		The stream
	//		span			The timespan class to use
	// Returns
	//		std::ostream	The stream with the timespan on it.
    std::ostream& operator << (std::ostream& o_Stream, const Util::CMSGTimeSpan& span);	

	// Description
	//		Insertion operator.
	// Arguments
	//		i_Stream		The stream
	//		span			The timespan class to use
	// Returns
	//		std::ostream	The stream with the timespan on it.
    std::istream& operator >> (std::istream& i_Stream, Util::CMSGTimeSpan& span);	

	// Description
	//		Get the number of days in the month for the month supplied.
	// Arguments
	//		i_wYear			The Year
	//		i_wMonth		The Month
     WORD GetDaysInMonth (WORD i_wYear, WORD i_wMonth);


	// Description
	//		Is the year supplied a leap year.
	// Arguments
	//		i_wYear			The Year
    bool IsLeapYear(WORD w_Year);

 
	// Description
	//		Get the number of days in the year..
	// Arguments
	//		i_wYear			The Year
     WORD GetDaysInYear(WORD w_Year);

	 // Description
	// Convert between the Day of the year and the Month day formats.
	// Arguments
	//		i_wYear			Year Value
	//		i_wDayOfYear	Day of the Year Value
	//		o_wMonth		Month answer
	//		o_wDay			Day answer
	void DayOfYearToMonthDay(WORD i_wYear, WORD i_wDayOfYear, WORD& o_wMonth, WORD& o_wDay);

	// Description
	// Convert between the Day of the year and the Month day formats.
	// Arguments
	//		i_wYear			Year Value
	//		o_wMonth		Month Value
	//		o_wDay			Day Value
	//		i_wDayOfYear	Day of the Year Answer
	void MonthDayToDayOfYear(WORD i_wYear, WORD o_wMonth, WORD o_wDay,WORD& i_wDayOfYear);



} // end of the namespace

#include "MSGTime_impl.h"

#endif // MSGTime_included
