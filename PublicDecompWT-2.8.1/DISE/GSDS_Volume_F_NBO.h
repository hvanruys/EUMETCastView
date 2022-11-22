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

#ifndef GSDS_Volume_F_NBO_included
#define GSDS_Volume_F_NBO_included

/*******************************************************************************

TYPE:
Concrete classes, overloaded operators.
					
PURPOSE:
C++ data types modelling the GSDS Volume F data types
(byte packed, in big-endian byte order).

FUNCTION:
The data types defined below model the data types defined in GSDS, Volume F.
Size (in bytes memory usage) and internal representation (big-endian byte order)
are identical to the representation in data transferred across DADF external
interfaces. 
Data structures describing external messages or data can be built based on
these data types.
Note: The data types defined below should only be used where data to/from external
interfaces are internally stored in the same representation. Anywhere else,
the data types from GSDS_Volume_F.h should be used.
Note also: This is not a complete collection of everything defined in GSDS, but
only the subset of types required for use in DISE and the US.

INTERFACES:
See under processing.

RESOURCES:	
Not applicable.

REFERENCES:
GSDS, Volume F.

PROCESSING:
For all 'elementary' data types is provided:
- An assignment operator which takes a little-endian variable of the corresponding
  size as input.
- A constructor which takes a litle-endian variable of the corresponding size as input.
- A conversion operator to obtain a little-endian value.
To data structures with several 'elementary' members, these operators can be
applied field-by-field.
Stream output and input operators are foreseen for most types.

DATA:
None.

LOGIC:		
-

*******************************************************************************/

#include <iostream>

#include "ErrorHandling.h"	// Util
#include "Types.h"

inline const bool isBigEndian() {
	char tmp[] = {0, 1};
	return (*(short *)tmp != 1);
}

namespace NBO
{




#pragma pack (push,1)




// Structures temporarily used for byte-swapping.
union U2 { unsigned __int16	i;	unsigned char c[2]; };
union U4 { unsigned __int32	i;	unsigned char c[4]; };
union U8 { unsigned __int64	i;	unsigned char c[8]; };
union S2 { short			i;	unsigned char c[2]; };
union S4 { long				i;	unsigned char c[4]; };
union R4 { float			i;	unsigned char c[4]; };
union R8 { double			i;	unsigned char c[8]; };




// General-purpose data types.


class UNSIGNED_BYTE
{

private:

	unsigned char m_;

public:

	UNSIGNED_BYTE             (unsigned char i_ = 0)       { m_ = i_;               }
	UNSIGNED_BYTE& operator = (unsigned char i_    )       { m_ = i_; return *this; }
	               operator    unsigned char      () const {          return m_;    }

	UNSIGNED_BYTE(const UNSIGNED_BYTE& i_)
	{
		m_=i_.m_;
	}

	friend std::ostream& operator<<
	(
		std::ostream&        i_Stream,
		const UNSIGNED_BYTE& i_Value
	)
	{
		i_Stream.write((const char*)&i_Value.m_, 1);
		return i_Stream;
	}

	friend std::istream& operator>>
	(
		std::istream&  i_Stream,
		UNSIGNED_BYTE& o_Value
	)
	{
		i_Stream.read((char*)&o_Value.m_, 1);
		return i_Stream;
	}

};




class UNSIGNED_SHORT
{

private:

	unsigned char m_[2];

public:

	UNSIGNED_SHORT(unsigned short i_ = 0)
	{
		U2 temp;
		temp.i = i_;
		if (isBigEndian()) {
			m_[0] = temp.c[1];
			m_[1] = temp.c[0];
		} else {
			m_[1] = temp.c[1];
			m_[0] = temp.c[0];
		}
	}

	UNSIGNED_SHORT(const UNSIGNED_SHORT& i_)
	{
		m_[0] = i_.m_[0];
		m_[1] = i_.m_[1];
	}

	UNSIGNED_SHORT& operator=(unsigned short i_)
	{
		U2 temp;
		temp.i = i_;
		if (isBigEndian()) {
			m_[0] = temp.c[1];
			m_[1] = temp.c[0];
		} else {
			m_[0] = temp.c[0];
			m_[1] = temp.c[1];
		}
		return *this;
	}

	operator unsigned short() const
	{
		U2 temp;
		if (isBigEndian()) {
			temp.c[1] = m_[0];
			temp.c[0] = m_[1];
		} else {
			temp.c[0] = m_[0];
			temp.c[1] = m_[1];
		}
		return temp.i;
	}

	friend std::ostream& operator<<(std::ostream& i_Stream, const UNSIGNED_SHORT& i_Value)
	{
		i_Stream.write((const char*)i_Value.m_, 2);
		return i_Stream;
	}

	friend std::istream& operator>>(std::istream& i_Stream,       UNSIGNED_SHORT& o_Value)
	{
		i_Stream.read((char*)o_Value.m_, 2);
		return i_Stream;
	}

};




class UNSIGNED
{

private:

	unsigned char m_[4];

public:

	UNSIGNED(unsigned long i_ = 0)
	{
		U4 temp;
		temp.i = i_;
		if (isBigEndian()) {
			m_[0] = temp.c[3];
			m_[1] = temp.c[2];
			m_[2] = temp.c[1];
			m_[3] = temp.c[0];
		} else {
			m_[0] = temp.c[0];
			m_[1] = temp.c[1];
			m_[2] = temp.c[2];
			m_[3] = temp.c[3];
		}
	}

	UNSIGNED(const UNSIGNED& i_)
	{
		m_[0] = i_.m_[0];
		m_[1] = i_.m_[1];
		m_[2] = i_.m_[2];
		m_[3] = i_.m_[3];
	}

	UNSIGNED& operator=(unsigned long i_)
	{
		U4 temp;
		temp.i = i_;
		if (isBigEndian()) {
			m_[0] = temp.c[3];
			m_[1] = temp.c[2];
			m_[2] = temp.c[1];
			m_[3] = temp.c[0];
		} else {
			m_[0] = temp.c[0];
			m_[1] = temp.c[1];
			m_[2] = temp.c[2];
			m_[3] = temp.c[3];
		}
		return *this;
	}

	operator unsigned long() const
	{
		U4 temp;
		if (isBigEndian()) {
			temp.c[0] = m_[3];
			temp.c[1] = m_[2];
			temp.c[2] = m_[1];
			temp.c[3] = m_[0];
		} else {
			temp.c[0] = m_[0];
			temp.c[1] = m_[1];
			temp.c[2] = m_[2];
			temp.c[3] = m_[3];
		}
		return temp.i;
	}

	friend std::ostream& operator<<(std::ostream& i_Stream, const UNSIGNED& i_Value)
	{
		i_Stream.write((const char*)i_Value.m_, 4);
		return i_Stream;
	}

	friend std::istream& operator>>(std::istream& i_Stream,       UNSIGNED& o_Value)
	{
		i_Stream.read((char*)o_Value.m_, 4);
		return i_Stream;
	}

};





class UNSIGNED_DOUBLE
{

private:

	unsigned char m_[8];

public:

	UNSIGNED_DOUBLE(unsigned __int64 i_ = 0)
	{
		U8 temp;
		temp.i = i_;
		if (isBigEndian()) {
			m_[0] = temp.c[7];
			m_[1] = temp.c[6];
			m_[2] = temp.c[5];
			m_[3] = temp.c[4];
			m_[4] = temp.c[3];
			m_[5] = temp.c[2];
			m_[6] = temp.c[1];
			m_[7] = temp.c[0];
		} else {
			m_[0] = temp.c[0];
			m_[1] = temp.c[1];
			m_[2] = temp.c[2];
			m_[3] = temp.c[3];
			m_[4] = temp.c[4];
			m_[5] = temp.c[5];
			m_[6] = temp.c[6];
			m_[7] = temp.c[7];
		}
	}

	UNSIGNED_DOUBLE(const UNSIGNED_DOUBLE& i_)
	{
		m_[0] = i_.m_[0];
		m_[1] = i_.m_[1];
		m_[2] = i_.m_[2];
		m_[3] = i_.m_[3];
		m_[4] = i_.m_[4];
		m_[5] = i_.m_[5];
		m_[6] = i_.m_[6];
		m_[7] = i_.m_[7];
	}

	UNSIGNED_DOUBLE& operator=(unsigned __int64 i_)
	{
		U8 temp;
		temp.i = i_;
		if (isBigEndian()) {
			m_[0] = temp.c[7];
			m_[1] = temp.c[6];
			m_[2] = temp.c[5];
			m_[3] = temp.c[4];
			m_[4] = temp.c[3];
			m_[5] = temp.c[2];
			m_[6] = temp.c[1];
			m_[7] = temp.c[0];
		} else {
			m_[0] = temp.c[0];
			m_[1] = temp.c[1];
			m_[2] = temp.c[2];
			m_[3] = temp.c[3];
			m_[4] = temp.c[4];
			m_[5] = temp.c[5];
			m_[6] = temp.c[6];
			m_[7] = temp.c[7];
		}
		return *this;
	}

	operator unsigned __int64() const
	{
		U8 temp;
		if (isBigEndian()) {
			temp.c[0] = m_[7];
			temp.c[1] = m_[6];
			temp.c[2] = m_[5];
			temp.c[3] = m_[4];
			temp.c[4] = m_[3];
			temp.c[5] = m_[2];
			temp.c[6] = m_[1];
			temp.c[7] = m_[0];
		} else {
			temp.c[0] = m_[0];
			temp.c[1] = m_[1];
			temp.c[2] = m_[2];
			temp.c[3] = m_[3];
			temp.c[4] = m_[4];
			temp.c[5] = m_[5];
			temp.c[6] = m_[6];
			temp.c[7] = m_[7];
		}
		return temp.i;
	}

	friend std::ostream& operator<<(std::ostream& i_Stream, const UNSIGNED_DOUBLE& i_Value)
	{
		i_Stream.write((const char*)i_Value.m_, 8);
		return i_Stream;
	}

	friend std::istream& operator>>(std::istream& i_Stream,       UNSIGNED_DOUBLE& o_Value)
	{
		i_Stream.read((char*)o_Value.m_, 8);
		return i_Stream;
	}

};




typedef UNSIGNED_BYTE  BOOLEAN_BYTE;

typedef UNSIGNED_BYTE  ENUMERATED_BYTE;
typedef UNSIGNED_SHORT ENUMERATED_SHORT;
typedef UNSIGNED       ENUMERATED_LONG;




class INTEGER_BYTE
{

private:

	char m_;

public:

	INTEGER_BYTE             (char i_ = 0)       { m_ = i_;               }
	INTEGER_BYTE& operator = (char i_    )       { m_ = i_; return *this; }
	              operator    char      () const {          return m_;    }

	INTEGER_BYTE(const INTEGER_BYTE& i_)
	{
		m_=i_.m_;
	}

	friend std::ostream& operator<<
	(
		std::ostream&       i_Stream,
		const INTEGER_BYTE& i_Value
	)
	{
		i_Stream.write(&i_Value.m_, 1);
		return i_Stream;
	}

	friend std::istream& operator>>
	(
		std::istream& i_Stream,
		INTEGER_BYTE& o_Value
	)
	{
		i_Stream.read(&o_Value.m_, 1);
		return i_Stream;
	}

};




class INTEGER_SHORT
{

private:

	unsigned char m_[2];

public:

	INTEGER_SHORT(short i_ = 0)
	{
		S2 temp;
		temp.i = i_;
		if (isBigEndian()) {
			m_[0] = temp.c[1];
			m_[1] = temp.c[0];
		} else {
			m_[0] = temp.c[0];
			m_[1] = temp.c[1];
		}
	}

	INTEGER_SHORT(const INTEGER_SHORT& i_)
	{
		m_[0]=i_.m_[0];
		m_[1]=i_.m_[1];
	}

	INTEGER_SHORT& operator=(short i_)
	{
		S2 temp;
		temp.i = i_;
		if (isBigEndian()) {
			m_[0] = temp.c[1];
			m_[1] = temp.c[0];
		} else {
			m_[0] = temp.c[0];
			m_[1] = temp.c[1];
		}
		return *this;
	}

	operator short() const
	{
		S2 temp;
		if (isBigEndian()) {
			temp.c[1] = m_[0];
			temp.c[0] = m_[1];
		} else {
			temp.c[1] = m_[1];
			temp.c[0] = m_[0];
		}
		return temp.i;
	}

	friend std::ostream& operator<<(std::ostream& i_Stream, const INTEGER_SHORT& i_Value)
	{
		i_Stream.write((const char*)i_Value.m_, 2);
		return i_Stream;
	}

	friend std::istream& operator>>(std::istream& i_Stream,       INTEGER_SHORT& o_Value)
	{
		i_Stream.read((char*)o_Value.m_, 2);
		return i_Stream;
	}

};




class INTEGER
{

private:

	unsigned char m_[4];

public:

	INTEGER(long i_ = 0)
	{
		S4 temp;
		temp.i = i_;
		if (isBigEndian()) {
			m_[0] = temp.c[3];
			m_[1] = temp.c[2];
			m_[2] = temp.c[1];
			m_[3] = temp.c[0];
		} else {
			m_[0] = temp.c[0];
			m_[1] = temp.c[1];
			m_[2] = temp.c[2];
			m_[3] = temp.c[3];
		}
	}

	INTEGER(const INTEGER& i_)
	{
		m_[0] = i_.m_[0];
		m_[1] = i_.m_[1];
		m_[2] = i_.m_[2];
		m_[3] = i_.m_[3];
	}

	INTEGER& operator=(long i_)
	{
		S4 temp;
		temp.i = i_;
		if (isBigEndian()) {
			m_[0] = temp.c[3];
			m_[1] = temp.c[2];
			m_[2] = temp.c[1];
			m_[3] = temp.c[0];
		} else {
			m_[0] = temp.c[0];
			m_[1] = temp.c[1];
			m_[2] = temp.c[2];
			m_[3] = temp.c[3];
		}
		return *this;
	}

	operator long() const
	{
		S4 temp;
		if (isBigEndian()) {
			temp.c[0] = m_[3];
			temp.c[1] = m_[2];
			temp.c[2] = m_[1];
			temp.c[3] = m_[0];
		} else {
			temp.c[0] = m_[0];
			temp.c[1] = m_[1];
			temp.c[2] = m_[2];
			temp.c[3] = m_[3];
		}
		return temp.i;
	}

	friend std::ostream& operator<<(std::ostream& i_Stream, const INTEGER& i_Value)
	{
		i_Stream.write((const char*)i_Value.m_, 4);
		return i_Stream;
	}

	friend std::istream& operator>>(std::istream& i_Stream,       INTEGER& o_Value)
	{
		i_Stream.read((char*)o_Value.m_, 4);
		return i_Stream;
	}

};




class REAL	// Complies with IEEE 754-1985.
{

private:

	unsigned char m_[4];

public:

	REAL(float i_ = 0)
	{
		R4 temp;
		temp.i = i_;
		if (isBigEndian()) {
			m_[0] = temp.c[3];
			m_[1] = temp.c[2];
			m_[2] = temp.c[1];
			m_[3] = temp.c[0];
		} else {
			m_[0] = temp.c[0];
			m_[1] = temp.c[1];
			m_[2] = temp.c[2];
			m_[3] = temp.c[3];
		}
	}

	REAL(const REAL& i_)
	{
		m_[0] = i_.m_[0];
		m_[1] = i_.m_[1];
		m_[2] = i_.m_[2];
		m_[3] = i_.m_[3];
	}

	REAL& operator=(float i_)
	{
		R4 temp;
		temp.i = i_;
		if (isBigEndian()) {
			m_[0] = temp.c[3];
			m_[1] = temp.c[2];
			m_[2] = temp.c[1];
			m_[3] = temp.c[0];
		} else {
			m_[0] = temp.c[0];
			m_[1] = temp.c[1];
			m_[2] = temp.c[2];
			m_[3] = temp.c[3];
		}
		return *this;
	}

	operator float() const
	{
		R4 temp;
		if (isBigEndian()) {
			temp.c[0] = m_[3];
			temp.c[1] = m_[2];
			temp.c[2] = m_[1];
			temp.c[3] = m_[0];
		} else {
			temp.c[0] = m_[0];
			temp.c[1] = m_[1];
			temp.c[2] = m_[2];
			temp.c[3] = m_[3];
		}
		return temp.i;
	}

	friend std::ostream& operator<<(std::ostream& i_Stream, const REAL& i_Value)
	{
		i_Stream.write((const char*)i_Value.m_, 4);
		return i_Stream;
	}

	friend std::istream& operator>>(std::istream& i_Stream,       REAL& o_Value)
	{
		i_Stream.read((char*)o_Value.m_, 4);
		return i_Stream;
	}

};




class REAL_DOUBLE	// Complies with IEEE 754-1985.
{

private:

	unsigned char m_[8];

public:

	REAL_DOUBLE(double i_ = 0)
	{
		R8 temp;
		temp.i = i_;
		if (isBigEndian()) {
			m_[0] = temp.c[7];
			m_[1] = temp.c[6];
			m_[2] = temp.c[5];
			m_[3] = temp.c[4];
			m_[4] = temp.c[3];
			m_[5] = temp.c[2];
			m_[6] = temp.c[1];
			m_[7] = temp.c[0];
		} else {
			m_[0] = temp.c[0];
			m_[1] = temp.c[1];
			m_[2] = temp.c[2];
			m_[3] = temp.c[3];
			m_[4] = temp.c[4];
			m_[5] = temp.c[5];
			m_[6] = temp.c[6];
			m_[7] = temp.c[7];
		}
	}

	REAL_DOUBLE(const REAL_DOUBLE& i_)
	{
		m_[0] = i_.m_[0];
		m_[1] = i_.m_[1];
		m_[2] = i_.m_[2];
		m_[3] = i_.m_[3];
		m_[4] = i_.m_[4];
		m_[5] = i_.m_[5];
		m_[6] = i_.m_[6];
		m_[7] = i_.m_[7];
	}

	REAL_DOUBLE& operator=(double i_)
	{
		R8 temp;
		temp.i = i_;
		if (isBigEndian()) {
			m_[0] = temp.c[7];
			m_[1] = temp.c[6];
			m_[2] = temp.c[5];
			m_[3] = temp.c[4];
			m_[4] = temp.c[3];
			m_[5] = temp.c[2];
			m_[6] = temp.c[1];
			m_[7] = temp.c[0];
		} else {
			m_[0] = temp.c[0];
			m_[1] = temp.c[1];
			m_[2] = temp.c[2];
			m_[3] = temp.c[3];
			m_[4] = temp.c[4];
			m_[5] = temp.c[5];
			m_[6] = temp.c[6];
			m_[7] = temp.c[7];
		}
		return *this;
	}

	operator double() const
	{
		R8 temp;
		if (isBigEndian()) {
			temp.c[0] = m_[7];
			temp.c[1] = m_[6];
			temp.c[2] = m_[5];
			temp.c[3] = m_[4];
			temp.c[4] = m_[3];
			temp.c[5] = m_[2];
			temp.c[6] = m_[1];
			temp.c[7] = m_[0];
		} else {
			temp.c[0] = m_[0];
			temp.c[1] = m_[1];
			temp.c[2] = m_[2];
			temp.c[3] = m_[3];
			temp.c[4] = m_[4];
			temp.c[5] = m_[5];
			temp.c[6] = m_[6];
			temp.c[7] = m_[7];
		}
		return temp.i;
	}

	friend std::ostream& operator<<(std::ostream& i_Stream, const REAL_DOUBLE& i_Value)
	{
		i_Stream.write((const char*)i_Value.m_, 8);
		return i_Stream;
	}

	friend std::istream& operator>>(std::istream& i_Stream,       REAL_DOUBLE& o_Value)
	{
		i_Stream.read((char*)o_Value.m_, 8);
		return i_Stream;
	}

};




class TIME_CDS_SHORT
{

public:

	UNSIGNED_SHORT m_Day;
	UNSIGNED       m_MillisecondsOfDay;

	TIME_CDS_SHORT()
	: m_Day              (0)
	, m_MillisecondsOfDay(0)
	{
	}

	TIME_CDS_SHORT(const TIME_CDS_SHORT& i_)
	{
		m_Day=i_.m_Day;
		m_MillisecondsOfDay=i_.m_MillisecondsOfDay;
	}

	TIME_CDS_SHORT(SYSTIME i_)
	{

#ifdef WIN32
		SYSTIME refTime(1958, 1, 1, 0, 0, 0, 0);
#else
		SYSTIME	refTime(0, 0, 0, 1, 1, 1958);
#endif
		SYSTIMESPAN tempSpan = i_ - refTime;
#ifdef WIN32
		m_Day                		= static_cast<unsigned short>(tempSpan.GetSpanInMilliseconds() / (1000*60*60*24));
		m_MillisecondsOfDay  		= static_cast<unsigned long >(tempSpan.GetSpanInMilliseconds() % (1000*60*60*24));
#else
		unsigned long long seconds = tempSpan.getNanoseconds() / (1000*1000);
		m_Day                		= static_cast<unsigned short>(seconds / (1000*60*60*24));
		m_MillisecondsOfDay  		= static_cast<unsigned long >(seconds % (1000*60*60*24));
#endif
	}

	TIME_CDS_SHORT& operator=(SYSTIME i_)
	{
#ifdef WIN32
		SYSTIME refTime(1958, 1, 1, 0, 0, 0, 0);
#else
		SYSTIME	refTime(0, 0, 0, 1, 1, 1958);
#endif
		SYSTIMESPAN tempSpan = i_ - refTime;
#ifdef WIN32
		m_Day               = static_cast<unsigned short>(tempSpan.GetSpanInMilliseconds() / (1000*60*60*24));
		m_MillisecondsOfDay = static_cast<unsigned long >(tempSpan.GetSpanInMilliseconds() % (1000*60*60*24));
#else
		unsigned long long seconds = tempSpan.getNanoseconds() / (1000*1000);
		m_Day                		= static_cast<unsigned short>(seconds / (1000*60*60*24));
		m_MillisecondsOfDay  		= static_cast<unsigned long >(seconds % (1000*60*60*24));
#endif
		return *this;
	}

	SYSTIME CUTCTime() const
	{
#ifdef WIN32
		SYSTIME 	tempTime(1958, 1, 1, 0, 0, 0, 0);
#else
		SYSTIME		tempTime(0, 0, 0, 1, 1, 1958);
#endif
		unsigned short		tempDay          = m_Day;
		unsigned long		tempMilliseconds = m_MillisecondsOfDay;
		SYSTIMESPAN	tempSpan(tempDay*SYSTIMESPAN::Day() + tempMilliseconds*SYSTIMESPAN::Millisecond());
		tempTime = tempTime+tempSpan;
		return tempTime;
	}

	friend std::ostream& operator<<(std::ostream& i_Stream, const TIME_CDS_SHORT& i_Value)
	{
		return i_Stream << i_Value.m_Day
						<< i_Value.m_MillisecondsOfDay;
	}


	friend std::istream& operator>>(std::istream& i_Stream,       TIME_CDS_SHORT& o_Value)
	{
		return i_Stream >> o_Value.m_Day
						>> o_Value.m_MillisecondsOfDay;
	}

};




class TIME_CDS
{

public:

	TIME_CDS_SHORT m_TIME_CDS_SHORT;
	UNSIGNED_SHORT m_MicrosecondsOfMilliseconds;

	TIME_CDS(){};

	TIME_CDS(const TIME_CDS& i_)
	{
		m_TIME_CDS_SHORT=i_.m_TIME_CDS_SHORT;
		m_MicrosecondsOfMilliseconds=i_.m_MicrosecondsOfMilliseconds;
	}

};




class TIME_CDS_EXPANDED
{

public:

	TIME_CDS_SHORT m_TIME_CDS_SHORT;
	UNSIGNED_SHORT m_MicrosecondsOfMilliseconds;
	UNSIGNED_SHORT m_NanosecondsOfMicroseconds;

	TIME_CDS_EXPANDED()
	: m_TIME_CDS_SHORT				(SYSTIME()	)
	, m_MicrosecondsOfMilliseconds	(0					)
	, m_NanosecondsOfMicroseconds	(0					)
	{
	}

	TIME_CDS_EXPANDED(const TIME_CDS_EXPANDED& i_)
	{
		m_TIME_CDS_SHORT=i_.m_TIME_CDS_SHORT;
		m_MicrosecondsOfMilliseconds=i_.m_MicrosecondsOfMilliseconds;
		m_NanosecondsOfMicroseconds=i_.m_NanosecondsOfMicroseconds;
	}

	TIME_CDS_EXPANDED(SYSTIME i_)
	: m_TIME_CDS_SHORT				(i_	)
	, m_MicrosecondsOfMilliseconds	(0	)
	, m_NanosecondsOfMicroseconds	(0	)
	{
	}

	TIME_CDS_EXPANDED& operator=(SYSTIME i_)
	{
		m_TIME_CDS_SHORT = i_;
		m_MicrosecondsOfMilliseconds = 0;
		m_NanosecondsOfMicroseconds  = 0;
		return *this;
	}

	SYSTIME CUTCTime() const
	{
		return m_TIME_CDS_SHORT.CUTCTime();
	}

	friend std::ostream& operator<<(std::ostream&, const TIME_CDS_EXPANDED&);
	friend std::istream& operator>>(std::istream&,       TIME_CDS_EXPANDED&);

};




class TIME_GENERALIZED
{

public:
	TIME_GENERALIZED(){}

	TIME_GENERALIZED(const TIME_GENERALIZED& i_)
	{
		m_[0]=i_.m_[0];
		m_[1]=i_.m_[1];
		m_[2]=i_.m_[2];
		m_[3]=i_.m_[3];
		m_[4]=i_.m_[4];
		m_[5]=i_.m_[5];
		m_[6]=i_.m_[6];
		m_[7]=i_.m_[7];
		m_[8]=i_.m_[8];
		m_[9]=i_.m_[9];
		m_[10]=i_.m_[10];
		m_[11]=i_.m_[11];
		m_[12]=i_.m_[12];
		m_[13]=i_.m_[13];
		m_[14]=i_.m_[14];
	
	}

	unsigned char m_[15];

};




class TIME_GENERALIZED_EXPANDED
{

public:

	unsigned char m_[25];

	TIME_GENERALIZED_EXPANDED(){}

	TIME_GENERALIZED_EXPANDED(const TIME_GENERALIZED_EXPANDED& i_)
	{
		m_[0]=i_.m_[0];
		m_[1]=i_.m_[1];
		m_[2]=i_.m_[2];
		m_[3]=i_.m_[3];
		m_[4]=i_.m_[4];
		m_[5]=i_.m_[5];
		m_[6]=i_.m_[6];
		m_[7]=i_.m_[7];
		m_[8]=i_.m_[8];
		m_[9]=i_.m_[9];
		m_[10]=i_.m_[10];
		m_[11]=i_.m_[11];
		m_[12]=i_.m_[12];
		m_[13]=i_.m_[13];
		m_[14]=i_.m_[14];
		m_[15]=i_.m_[15];
		m_[16]=i_.m_[16];
		m_[17]=i_.m_[17];
		m_[18]=i_.m_[18];
		m_[19]=i_.m_[19];
		m_[20]=i_.m_[20];
		m_[21]=i_.m_[21];
		m_[22]=i_.m_[22];
		m_[23]=i_.m_[23];
		m_[24]=i_.m_[24];
	}

};




// Special-purpose simple data types.

typedef ENUMERATED_BYTE  GP_FAC_ENV;
typedef ENUMERATED_BYTE  GP_FAC_ID;
typedef ENUMERATED_BYTE  GP_SC_CHAN_ID;
typedef ENUMERATED_SHORT GP_SC_ID;
typedef UNSIGNED         GP_SU_ID;
typedef ENUMERATED_BYTE  GP_SVCE_TYPE;




// Special-purpose data structures.

class GP_CONFIG_ITEM_VERSION
{

public:

	UNSIGNED_SHORT m_Issue;
	UNSIGNED_SHORT m_Revision;

	GP_CONFIG_ITEM_VERSION(){}

	GP_CONFIG_ITEM_VERSION(const GP_CONFIG_ITEM_VERSION& i_)
	{
		m_Issue=i_.m_Issue;
		m_Revision=i_.m_Revision;
	}
};




class GP_CPU_ADDRESS
{

public:

	UNSIGNED_BYTE m_Qualifier_1;
	UNSIGNED_BYTE m_Qualifier_2;
	UNSIGNED_BYTE m_Qualifier_3;
	UNSIGNED_BYTE m_Qualifier_4;

	GP_CPU_ADDRESS(){};

	GP_CPU_ADDRESS(const GP_CPU_ADDRESS& i_)
	{
		m_Qualifier_1=i_.m_Qualifier_1;
		m_Qualifier_2=i_.m_Qualifier_2;
		m_Qualifier_3=i_.m_Qualifier_3;
		m_Qualifier_4=i_.m_Qualifier_4;
	}

	friend std::ostream& operator<<(std::ostream& i_Stream, const GP_CPU_ADDRESS& i_Value)
	{
		return i_Stream << i_Value.m_Qualifier_1
						<< i_Value.m_Qualifier_2
						<< i_Value.m_Qualifier_3
						<< i_Value.m_Qualifier_4;
	}

	friend std::istream& operator>>(std::istream& i_Stream,       GP_CPU_ADDRESS& o_Value)
	{
		return i_Stream >> o_Value.m_Qualifier_1
						>> o_Value.m_Qualifier_2
						>> o_Value.m_Qualifier_3
						>> o_Value.m_Qualifier_4;
	}

};




class GP_FI_HEADER
{

public:

	UNSIGNED_BYTE   m_HeaderVersionNo;
	ENUMERATED_BYTE m_FileType;
	ENUMERATED_BYTE m_SubHeaderType;
	GP_FAC_ID       m_SourceFacilityId;
	GP_FAC_ENV      m_SourceEnvId;
	UNSIGNED_BYTE   m_SourceInstanceId;
	GP_SU_ID        m_SourceSUId;
	GP_CPU_ADDRESS  m_SourceCPUId;
	GP_FAC_ID       m_DestFacilityId;
	GP_FAC_ENV      m_DestEnvId;
	UNSIGNED        m_DataFieldLength;
	char            m_Description[409];

	GP_FI_HEADER(){};

	GP_FI_HEADER(const GP_FI_HEADER& i_)
	{
		m_HeaderVersionNo=i_.m_HeaderVersionNo;
		m_FileType=i_.m_FileType;
		m_SubHeaderType=i_.m_SubHeaderType;
		m_SourceFacilityId=i_.m_SourceFacilityId;
		m_SourceEnvId=i_.m_SourceEnvId;
		m_SourceInstanceId=i_.m_SourceInstanceId;
		m_SourceSUId=i_.m_SourceSUId;
		m_SourceCPUId=i_.m_SourceCPUId;
		m_DestFacilityId=i_.m_DestFacilityId;
		m_DestEnvId=i_.m_DestEnvId;
		m_DataFieldLength=i_.m_DataFieldLength;
		strcpy(m_Description,i_.m_Description);
	}
	
	friend std::ostream& operator<<(std::ostream& i_Stream, const GP_FI_HEADER& i_Value)
	{
		i_Stream << i_Value.m_HeaderVersionNo
				 << i_Value.m_FileType
				 << i_Value.m_SubHeaderType
				 << i_Value.m_SourceFacilityId
				 << i_Value.m_SourceEnvId
				 << i_Value.m_SourceInstanceId
				 << i_Value.m_SourceSUId
				 << i_Value.m_SourceCPUId
				 << i_Value.m_DestFacilityId
				 << i_Value.m_DestEnvId
				 << i_Value.m_DataFieldLength;
		i_Stream.write(i_Value.m_Description, sizeof(i_Value.m_Description));
		return i_Stream;
	}

	friend std::istream& operator>>(std::istream& i_Stream,       GP_FI_HEADER& o_Value)
	{
		i_Stream >> o_Value.m_HeaderVersionNo
				 >> o_Value.m_FileType
				 >> o_Value.m_SubHeaderType
				 >> o_Value.m_SourceFacilityId
				 >> o_Value.m_SourceEnvId
				 >> o_Value.m_SourceInstanceId
				 >> o_Value.m_SourceSUId
				 >> o_Value.m_SourceCPUId
				 >> o_Value.m_DestFacilityId
				 >> o_Value.m_DestEnvId
				 >> o_Value.m_DataFieldLength;
		i_Stream.read(o_Value.m_Description, sizeof(o_Value.m_Description));
		return i_Stream;
	}

};




class GP_FI_SH1
{

public:

	UNSIGNED_BYTE  m_SubheaderVersionNo;
	GP_SVCE_TYPE   m_ServiceType;
	UNSIGNED_BYTE  m_ServiceSubtype;
	TIME_CDS_SHORT m_FileTime;
	GP_SC_ID       m_SpacecraftId;
	char           m_Description[187];

	GP_FI_SH1(){};

	GP_FI_SH1(const GP_FI_SH1& i_)
	{
		m_SubheaderVersionNo=i_.m_SubheaderVersionNo;
		m_ServiceType=i_.m_ServiceType;
		m_ServiceSubtype=i_.m_ServiceSubtype;
		m_FileTime=i_.m_FileTime;
		m_SpacecraftId=i_.m_SpacecraftId;
		strcpy(m_Description,i_.m_Description);
	}

	friend std::ostream& operator<<(std::ostream& i_Stream, const GP_FI_SH1& i_Value)
	{
		i_Stream << i_Value.m_SubheaderVersionNo
				 << i_Value.m_ServiceType
				 << i_Value.m_ServiceSubtype
				 << i_Value.m_FileTime
				 << i_Value.m_SpacecraftId;
		i_Stream.write(i_Value.m_Description, sizeof(i_Value.m_Description));
		return i_Stream;
	}

	friend std::istream& operator>>(std::istream& i_Stream,       GP_FI_SH1& o_Value)
	{
		i_Stream >> o_Value.m_SubheaderVersionNo
				 >> o_Value.m_ServiceType
				 >> o_Value.m_ServiceSubtype
				 >> o_Value.m_FileTime
				 >> o_Value.m_SpacecraftId;
		i_Stream.read(o_Value.m_Description, sizeof(o_Value.m_Description));
		return i_Stream;
	}

};




class GP_PK_HEADER
{

public:

	UNSIGNED_BYTE   m_HeaderVersionNo;
	ENUMERATED_BYTE m_PacketType;
	ENUMERATED_BYTE m_SubHeaderType;
	GP_FAC_ID       m_SourceFacilityId;
	GP_FAC_ENV      m_SourceEnvId;
	UNSIGNED_BYTE   m_SourceInstanceId;
	GP_SU_ID        m_SourceSUId;
	GP_CPU_ADDRESS  m_SourceCPUId;
	GP_FAC_ID       m_DestFacilityId;
	GP_FAC_ENV      m_DestEnvId;
	UNSIGNED_SHORT  m_SequenceCount;
	UNSIGNED        m_PacketLength;

	GP_PK_HEADER(){};

	GP_PK_HEADER(const GP_PK_HEADER& i_)
	{
		m_HeaderVersionNo=i_.m_HeaderVersionNo;
		m_PacketType=i_.m_PacketType;
		m_SubHeaderType=i_.m_SubHeaderType;
		m_SourceFacilityId=i_.m_SourceFacilityId;
		m_SourceEnvId=i_.m_SourceEnvId;
		m_SourceInstanceId=i_.m_SourceInstanceId;
		m_SourceSUId=i_.m_SourceInstanceId;
		m_SourceCPUId=i_.m_SourceCPUId;
		m_DestFacilityId=i_.m_DestFacilityId;
		m_DestEnvId=i_.m_DestEnvId;
		m_SequenceCount=i_.m_SequenceCount;
		m_PacketLength=i_.m_PacketLength;
	}

};




class GP_PK_SH1
{

public:

	UNSIGNED_BYTE  m_SubheaderVersionNo;
	BOOLEAN_BYTE   m_CheckSumFlag;
	unsigned char  m_Acknowledgement[4];
	GP_SVCE_TYPE   m_ServiceType;
	UNSIGNED_BYTE  m_ServiceSubtype;
	TIME_CDS_SHORT m_PacketTime;
	GP_SC_ID       m_SpacecraftId;

	GP_PK_SH1(){};

	GP_PK_SH1(const GP_PK_SH1& i_)
	{
		m_SubheaderVersionNo=i_.m_SubheaderVersionNo;
		m_CheckSumFlag=i_.m_CheckSumFlag;

		m_Acknowledgement[0]=i_.m_Acknowledgement[0];
		m_Acknowledgement[1]=i_.m_Acknowledgement[1];
		m_Acknowledgement[2]=i_.m_Acknowledgement[2];
		m_Acknowledgement[3]=i_.m_Acknowledgement[3];

		m_ServiceType=i_.m_ServiceType;
		m_ServiceSubtype=i_.m_ServiceSubtype;
		m_PacketTime=i_.m_PacketTime;
		m_SpacecraftId=i_.m_SpacecraftId;
	}

};




#pragma pack(pop)




} // end namespace


#endif
