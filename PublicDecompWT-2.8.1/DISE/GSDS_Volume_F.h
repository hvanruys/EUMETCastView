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

#ifndef GSDS_VOLUME_F_HEADER
#define GSDS_VOLUME_F_HEADER

/*******************************************************************************
* Eumetsat Meteosat Second Generation
*
* Data Acquisition and Dissemination Facility: DADF
*
* This software is developed by VCS Nachrichtentechnik GmbH under contract
* VCS      : V97.103.1008
* Eumetsat : EUM/CO/97/499/YB
*
* Copyright VCS 1998
*
* Applicable Third-party Software Licence Information:
*
*-------------------------------------------------------------------------------
* Configuration Control Details from Visual Source Safe
*
$Archive: /Util/Inc/GSDS_Volume_F.h $
*
$History: GSDS_Volume_F.h $
 * 
*
*-------------------------------------------------------------------------------
*
********************************************************************************
*/

/**************************************************************************
****  DADF MODULE HEADER   ***

TYPE:			data types

PURPOSE:		defines the GSDS Volume F data types


FUNCTION:		In this header file the GSDS Volume F datatypes are
				defined.


INTERFACES:		See 'INTERFACES:' in the module declaration below
				The data types defined below model the data types 
				defined in 	GSDS, Volume F. 
				
				For sending these data-types across the network it 
				is necessary to define one function for each data-type which 
				returns the size of the type.
				It is also needed to define input/output operators for each 
				data-type to stream in/out the objects from a (or on to a)
				COMS::CMSGByteStream object.
				These function/operators are not provided now, they will be 
				defined in the next phase.
				Below you see an example prototype of these functions: 
	
					// Length when sent across the network.
					unsigned int NetworkLength(...) const

					// input/output operators to send objects across the network.
					CMSGByteStream& operator << (CMSGByteStream& o_Stream, GP_FAC_ENV i_FAC_ENV)
					CMSGByteStream& operator >> (CMSGByteStream& o_Stream, GP_FAC_ENV o_FAC_ENV)
				

				The output operators defined in this file are only needed to tranform
				a data-type to a human-readable string. 
				
			
RESOURCES:		negligible


REFERENCES:		GSDS Volume F


PROCESSING:		no complex processing	

****  END MODULE HEADER   ***
**************************************************************************/

#ifdef XTRA_COM_INFO
	#pragma message( "Compiling " __FILE__ ) 
#endif

#include <string>
#include "CMSGByteStream.h"




	//	Some commonly needed sizes
	static const unsigned int	StaticInt8Size		= 1;
	static const unsigned int	StaticInt16Size		= 2;
	static const unsigned int	StaticInt32Size		= 4;
	static const unsigned int	StaticInt64Size		= 8;
	static const unsigned int	StaticFloatSize		= sizeof(float);
	static const unsigned int	StaticDoubleSize	= sizeof(double);
	static const unsigned int	StaticLongSize		= sizeof(long);
	static const unsigned int	StaticCHAR80Size	= 80;
	static const unsigned int	StaticCHAR255Size	= 255;

// mvd: necessary for event build tools which generates variables of type CHAR80
struct CHAR80
{
	std::string m_value;

	CHAR80(const std::string& i_value = "")
	{
		m_value=i_value;
		m_value.resize(80);

	};

	std::string ReadableForm() const
	{	
		return m_value;
	}

	void operator = ( std::string& i_other)
		{m_value=i_other;m_value.resize(80);}

	const unsigned int IntNetworkLength() const {return StaticCHAR80Size;};
	const unsigned int ExtNetworkLength() const {return StaticCHAR80Size;};
};

inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const CHAR80& i_string);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,CHAR80& o_string);


struct CHAR255
{
	std::string m_value;

	CHAR255(const std::string& i_value = "")
	{
		m_value=i_value;
		m_value.resize(255);

	};

	std::string ReadableForm() const
	{	
		return m_value;
	}

	void operator = ( std::string& i_other)
		{m_value=i_other;m_value.resize(255);}

	const unsigned int IntNetworkLength() const {return StaticCHAR255Size;};
	const unsigned int ExtNetworkLength() const {return StaticCHAR255Size;};
};

inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const CHAR255& i_string);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,CHAR255& o_string);


// Booleans

struct BOOLEAN_BYTE
{
	_int8 m_value;
	
	BOOLEAN_BYTE(_int8 i_value = 1){m_value = i_value;};
	std::string ReadableForm()
	{
		if (m_value > 0)
			return "TRUE";
		else
			return "FALSE";
	}

	operator bool() const
	{
		return (m_value > 0);
	}

	void operator = (_int8 i_other)
		{m_value = i_other;}


	const unsigned int IntNetworkLength() const {return StaticInt8Size ;};
	const unsigned int ExtNetworkLength() const {return StaticInt8Size ;};

};

inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const BOOLEAN_BYTE& i_boolbyte);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,BOOLEAN_BYTE& o_boolbyte);


struct BOOLEAN_SHORT 
{
	_int16 m_value;
	
	BOOLEAN_SHORT(_int16 i_value = 1){m_value = i_value;};
	std::string ReadableForm()
	{
		if (m_value > 0)
			return "TRUE";
		else
			return "FALSE";
	}

	void operator = (_int16 i_other)
		{m_value = i_other;}


	operator bool() const
	{
		return (m_value > 0);
	}


	const unsigned int IntNetworkLength() const {return StaticInt16Size ;};
	const unsigned int ExtNetworkLength() const {return StaticInt16Size ;};
};


inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const BOOLEAN_SHORT& i_boolshort);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,BOOLEAN_SHORT& o_boolshort);


struct BOOLEAN_LONG
{
	_int32 m_value;
	
	BOOLEAN_LONG(_int32 i_value = 1){m_value = i_value;};
	std::string ReadableForm()
	{
		if (m_value > 0)
			return "TRUE";
		else
			return "FALSE";
	}

	void operator = (_int32 i_other)
		{m_value = i_other;}


	operator bool() const
	{
		return (m_value > 0);
	}

	const unsigned int IntNetworkLength() const {return StaticInt32Size ;};
	const unsigned int ExtNetworkLength() const {return StaticInt32Size ;};
};
inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const BOOLEAN_LONG& i_boollong);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,BOOLEAN_LONG& o_boollong);



// Enumerated
typedef unsigned __int8		ENUMERATED_BYTE;


struct UNSIGNED_BYTE
{
	UINT8 m_value;
	
	UNSIGNED_BYTE (UINT8 i_value = 0){m_value = i_value;};
	std::string ReadableForm() const
	{	
		char	buffer[20];

		itoa (m_value, buffer, 10);
		return buffer;
	}

	std::string ReadableFormHex() const
	{	
		char	buffer[20];

		itoa (m_value, buffer,16);
		return buffer;
	}

	void operator = (UINT8 i_other)
		{m_value = i_other;}

	operator UINT8 () const 
	{
		return m_value;
	}

	const unsigned int IntNetworkLength() const {return StaticInt8Size ;};
	const unsigned int ExtNetworkLength() const {return StaticInt8Size ;};
};

inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const UNSIGNED_BYTE& i_unbyte);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,UNSIGNED_BYTE& o_unbyte);

inline std::ostream& operator << (std::ostream& o_Stream, const UNSIGNED_BYTE& i_unbyte);


struct UNSIGNED_SHORT
{
	UINT16 m_value;
	
	UNSIGNED_SHORT(UINT16 i_value = 0){m_value = i_value;};

	std::string ReadableForm() const
	{	
		char	buffer[20];

		itoa (m_value, buffer, 10);
		return buffer;
	}

	void operator = (UINT16 i_other)
		{m_value = i_other;}

	void operator = (const UNSIGNED_SHORT i_other)
		{m_value = i_other.m_value;}

	operator UINT16() const { return m_value; }

	const unsigned int IntNetworkLength() const {return StaticInt16Size ;};
	const unsigned int ExtNetworkLength() const {return StaticInt16Size ;};
};

inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const UNSIGNED_SHORT& i_unshort);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,UNSIGNED_SHORT& o_unshort);

inline std::ostream& operator << (std::ostream& o_Stream, const UNSIGNED_SHORT& i_unshort);


struct UNSIGNED
{
	UINT32 m_value;
	
	UNSIGNED(UINT32 i_value = 0){m_value = i_value;};
	std::string ReadableForm() const
	{	
		char	buffer[20];

		ultoa (m_value, buffer, 10);
		return buffer;
	}

	void operator = (UINT32 i_other)
		{m_value = i_other;}


	operator UINT32() const
	{
		return m_value;
	}

	const unsigned int IntNetworkLength() const {return StaticInt32Size ;};
	const unsigned int ExtNetworkLength() const {return StaticInt32Size ;};
};

inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const UNSIGNED& i_unsigned);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,UNSIGNED& o_unsigned);


struct UNSIGNED_LONG
{
	UINT32 m_value;
	
	UNSIGNED_LONG(UINT32 i_value = 0){m_value = i_value;};
	std::string ReadableForm() const
	{	
		char	buffer[20];

		ultoa (m_value, buffer, 10);
		return buffer;
	}

	void operator = (UINT32 i_other)
		{m_value = i_other;}


	operator UINT32() const
	{
		return m_value;
	}

	const unsigned int IntNetworkLength() const {return StaticInt32Size ;};
	const unsigned int ExtNetworkLength() const {return StaticInt32Size ;};
};

inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const UNSIGNED_LONG& i_unsignedlong);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,UNSIGNED_LONG& o_unsignedlong);

inline std::ostream& operator << (std::ostream& o_Stream, const UNSIGNED& i_unsigned);


struct UNSIGNED_DOUBLE
{
	UINT64 m_value;
	
	UNSIGNED_DOUBLE(UINT64 i_value = 0){m_value = i_value;};
	std::string ReadableForm() const
	{	
		char	buffer[25];

		_ui64toa (m_value, buffer, 10);
		return buffer;
	}

	void operator = (UINT64 i_other)
		{m_value = i_other;}

	
	operator UINT64() const
	{
		return m_value;
	}

	const unsigned int IntNetworkLength() const {return StaticInt64Size ;};
	const unsigned int ExtNetworkLength() const {return StaticInt64Size ;};
};

inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const UNSIGNED_DOUBLE& i_undouble);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,UNSIGNED_DOUBLE& o_undouble);



struct INTEGER_BYTE
{
	 _int8 m_value;
	
	INTEGER_BYTE( _int8 i_value = 0){m_value = i_value;};
	std::string ReadableForm() const
	{	
		char	buffer[20];

		itoa (m_value, buffer, 10);
		return buffer;
	}

	void operator = ( _int8 i_other)
		{m_value = i_other;}

	operator _int8()
	{
		return m_value;
	}

	const unsigned int IntNetworkLength() const {return StaticInt8Size ;};
	const unsigned int ExtNetworkLength() const {return StaticInt8Size ;};
};

inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const INTEGER_BYTE& i_intbyte);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,INTEGER_BYTE& o_intbyte);



struct INTEGER_SHORT
{
	 _int16 m_value;
	
	INTEGER_SHORT( _int16 i_value = 0){m_value = i_value;};
	std::string ReadableForm() const
	{	
		char	buffer[20];

		itoa (m_value, buffer, 10);
		return buffer;
	}

	void operator = ( _int16 i_other)
		{m_value = i_other;}

	operator _int16() const
	{
		return m_value;
	}

	const unsigned int IntNetworkLength() const {return StaticInt16Size ;};
	const unsigned int ExtNetworkLength() const {return StaticInt16Size ;};
};

inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const INTEGER_SHORT& i_intshort);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,INTEGER_SHORT& o_intshort);



struct INTEGER
{
	 _int32 m_value;
	
	INTEGER( _int32 i_value = 0){m_value = i_value;};
	std::string ReadableForm() const
	{	
		char	buffer[20];

		itoa (m_value, buffer, 10);
		return buffer;
	}

	void operator = ( _int32 i_other)
		{m_value = i_other;}

	operator _int32()
	{
		return m_value;
	}

	const unsigned int IntNetworkLength() const {return StaticInt32Size ;};
	const unsigned int ExtNetworkLength() const {return StaticInt32Size ;};
};

inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const INTEGER& i_int);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,INTEGER& o_int);



struct INTEGER_DOUBLE
{
	 _int64 m_value;
	
	INTEGER_DOUBLE( _int64 i_value = 0){m_value = i_value;};
	std::string ReadableForm() const
	{	
		char	buffer[25];

		_i64toa (m_value, buffer, 10);
		return buffer;
	}

	void operator = ( _int64 i_other)
		{m_value = i_other;}

	operator _int64() const
	{
		return m_value;
	}

	const unsigned int IntNetworkLength() const {return StaticInt64Size ;};
	const unsigned int ExtNetworkLength() const {return StaticInt64Size ;};
};

inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const INTEGER_DOUBLE& i_intdouble);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,INTEGER_DOUBLE& o_intdouble);




struct REAL
{
	 float m_value;
	
	REAL( float i_value = 0){m_value = i_value;};
	std::string ReadableForm() const
	{	
		char	buffer[20];

		sprintf(buffer, "%g", m_value);
		return buffer;
	}

	void operator = ( float i_other)
		{m_value = i_other;}


	operator float() const
	{
		return m_value;
	}

	const unsigned int IntNetworkLength() const {return StaticFloatSize ;};
	const unsigned int ExtNetworkLength() const {return StaticFloatSize ;};
};

inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const REAL& i_float);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,REAL& o_float);



struct REAL_DOUBLE
{
	 double m_value;
	
	REAL_DOUBLE( double i_value = 0){m_value = i_value;};
	std::string ReadableForm() const
	{	
		char	buffer[20];

		sprintf(buffer, "%g", m_value);
		return buffer;
	}

	void operator = ( double i_other)
		{m_value = i_other;}


	operator double() const
	{
		return m_value;
	}

	const unsigned int IntNetworkLength() const {return StaticDoubleSize ;};
	const unsigned int ExtNetworkLength() const {return StaticDoubleSize ;};
};

inline COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const REAL_DOUBLE& i_double);
inline COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, REAL_DOUBLE& o_double);


// Strings
typedef std::string			CHARACTERSTRING;


struct GP_COMMAND_FAIL_REASON
{
	enum E_COMMAND_FAIL_REASON
	{
		e_NoFailReason				= 0,
		e_UnknownCommand			= 1,
		e_InvalidParameter			= 2,
		e_WrongLength				= 3,
		e_InvalidContext			= 4,
		e_CannotForward				= 5,
		e_unknownCommandFailReason	= 6
	} m_COMMAND_FAIL_REASON;

	GP_COMMAND_FAIL_REASON(){m_COMMAND_FAIL_REASON = e_NoFailReason;};
	GP_COMMAND_FAIL_REASON(E_COMMAND_FAIL_REASON i_CommandFailReason)
										{m_COMMAND_FAIL_REASON = i_CommandFailReason;};

	GP_COMMAND_FAIL_REASON(std::string i_string);
	std::string ReadableForm() const;
	
	GP_COMMAND_FAIL_REASON& operator = (const GP_COMMAND_FAIL_REASON i_fail)
	{m_COMMAND_FAIL_REASON = i_fail.m_COMMAND_FAIL_REASON;return *this;};

	GP_COMMAND_FAIL_REASON& operator = (const E_COMMAND_FAIL_REASON i_fail)
		{m_COMMAND_FAIL_REASON = i_fail;return *this;}

	bool operator == (const GP_COMMAND_FAIL_REASON& i_fail) const
		{return m_COMMAND_FAIL_REASON== i_fail.m_COMMAND_FAIL_REASON;};

	bool operator == (const GP_COMMAND_FAIL_REASON::E_COMMAND_FAIL_REASON& i_fail) const
		{return m_COMMAND_FAIL_REASON== i_fail;}


	const unsigned int IntNetworkLength() const {return StaticInt16Size;};
	const unsigned int ExtNetworkLength() const {return StaticInt16Size;}; 

};
inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_COMMAND_FAIL_REASON&  i_CommandFReason);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_COMMAND_FAIL_REASON& o_CommandFReason);

inline std::ostream& operator << (std::ostream& out, const GP_COMMAND_FAIL_REASON& gp_command_fail_reason);


struct  GP_COMMAND_ID
{
	enum EGP_COMMAND_ID
	{
		e_StartSU						= 30000,
		e_StopSU						= 30001,
		e_AbortSU						= 30002,
		e_EnableLink					= 30003,
		e_DisableLink					= 30004,
		e_SetConnectMode				= 30005,
		e_ChangeDU						= 30006,
		e_ChangeMonitoringFreq			= 30007,
		e_ChangeEventFilter				= 30008,
		e_ChangeTraceLevel				= 30009,
		e_ChangeDCPChannelAlarmsMode	= 30010,
		e_EnableCentralCommanding		= 30011,
		e_ForceToLocal					= 30012,
		e_GiveControl					= 30013,
		e_PurgeALQ						= 30014,
		e_EnableFTPLink					= 30015,
		e_DisableFTPLink				= 30016,
		e_Unknown						= 30017,
	} m_COMMAND_Id; 

	GP_COMMAND_ID ():m_COMMAND_Id(e_StartSU){};
	GP_COMMAND_ID (EGP_COMMAND_ID i_COMMAND_Id):m_COMMAND_Id(i_COMMAND_Id){};
	
	void operator = (GP_COMMAND_ID rhs){ m_COMMAND_Id = rhs.m_COMMAND_Id; };
	bool operator < (const GP_COMMAND_ID rhs) const { return m_COMMAND_Id < rhs.m_COMMAND_Id;};
	bool operator == (const GP_COMMAND_ID rhs) const 
		{return m_COMMAND_Id == rhs.m_COMMAND_Id;};
	operator UNSIGNED(){return UNSIGNED(m_COMMAND_Id);};

	const unsigned int IntNetworkLength() const {return StaticInt32Size;};
	const unsigned int ExtNetworkLength() const {return StaticInt32Size;}; 

	std::string ReadableForm() const;
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_COMMAND_ID&  i_COMMAND_Id);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_COMMAND_ID& i_COMMAND_Id);

struct  GP_COMMAND_VERI_ServiceSubTypes
{
	enum EGP_COMMAND_VERI_ServiceSubTypes
	{
		e_AcceptanceSuccess			= 0,
		e_AcceptanceFail			= 1,
		e_StartSuccess				= 2,
		e_StartFail					= 3,
		e_ProgressSuccess			= 4,
		e_ProgressFail				= 5,
		e_CompletionSuccess			= 6,
		e_CompletionFail			= 7
	} m_Value; 

	GP_COMMAND_VERI_ServiceSubTypes ():m_Value(e_AcceptanceSuccess){};
	GP_COMMAND_VERI_ServiceSubTypes (EGP_COMMAND_VERI_ServiceSubTypes i_Value)
		:	m_Value(i_Value){};
	
	void operator = (GP_COMMAND_VERI_ServiceSubTypes rhs){ m_Value = rhs.m_Value; };
	GP_COMMAND_VERI_ServiceSubTypes& operator = (const EGP_COMMAND_VERI_ServiceSubTypes i_Value)
	{
		m_Value = i_Value; 
		return *this;
	}

	bool operator < (const GP_COMMAND_VERI_ServiceSubTypes rhs) const { return m_Value < rhs.m_Value;};
	bool operator == (const GP_COMMAND_VERI_ServiceSubTypes rhs) const 
		{return m_Value == rhs.m_Value;};

	operator UNSIGNED_BYTE (){return _int8 (m_Value);};

	const unsigned int IntNetworkLength() const {return StaticInt8Size;};
	const unsigned int ExtNetworkLength() const {return StaticInt8Size;}; 

	std::string ReadableForm() const;
};

//inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_COMMAND_VERI_ServiceSubTypes&  i_Value);
//inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_COMMAND_VERI_ServiceSubTypes& i_Value);


typedef UNSIGNED			GP_EVENT_ID;

struct GP_EVENT_SEVERITY
{		 
	enum ESeverity
	{
		e_LogOnly				= 0,
		e_Warning				= 1,
		e_Error					= 2,
		e_Alarm					= 3,
		e_Maintenance			= 4,
		e_UnkownSeverity		= 5,
	} m_Severity;

	GP_EVENT_SEVERITY(){m_Severity=e_LogOnly;};
	GP_EVENT_SEVERITY(ESeverity i_Severity){m_Severity=i_Severity;};
	GP_EVENT_SEVERITY(std::string i_string);

	void operator = (GP_EVENT_SEVERITY rhs){ m_Severity = rhs.m_Severity; };
	bool operator < (GP_EVENT_SEVERITY rhs) const { return m_Severity < rhs.m_Severity;};


	GP_EVENT_SEVERITY& operator = (const ESeverity i_event)
		{m_Severity = i_event;return *this;};

	bool operator == (const GP_EVENT_SEVERITY& i_event) const
		{return m_Severity== i_event.m_Severity;};

	bool operator == (const GP_EVENT_SEVERITY::ESeverity& i_event) const
		{return m_Severity== i_event;}

	std::string ReadableForm() const;
	const unsigned int IntNetworkLength() const {return StaticInt8Size;}; 
	const unsigned int ExtNetworkLength() const {return StaticInt8Size;}; 
};
	
inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_EVENT_SEVERITY& i_EventSeverity);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_EVENT_SEVERITY& o_EventSeverity);

inline std::ostream& operator << (std::ostream& out, const GP_EVENT_SEVERITY& gp_event_severity);


struct GP_FAC_ENV
{
	enum EGP_FAC_ENV
	{
		e_NoEnvironment			= 0,
		e_Operational_On_line	= 1,
		e_Operational_Off_line	= 2,
		e_Validation_On_line	= 254,
		e_Validation_Offline	= 255,
		e_Backup_On_line		= 128,
		e_Backup_Off_line		= 129,
		e_Development			= 130,
		e_ConfigurationControl	= 131,
		e_UnknownEnvironment	= 132
	}	m_Fac_Env;

	GP_FAC_ENV():m_Fac_Env(e_NoEnvironment){};
	GP_FAC_ENV(EGP_FAC_ENV i_Fac_env):m_Fac_Env(i_Fac_env){};
	GP_FAC_ENV(std::string i_string);
	
	void operator = (GP_FAC_ENV rhs){ m_Fac_Env = rhs.m_Fac_Env; };
	
	bool operator < (const GP_FAC_ENV rhs) const 
		{return m_Fac_Env < rhs.m_Fac_Env;};
	
	bool operator == (const GP_FAC_ENV rhs) const 
		{return m_Fac_Env == rhs.m_Fac_Env;};

	
	std::string ReadableForm() const;
	const unsigned int IntNetworkLength() const 	{return StaticInt8Size;}; 
	const unsigned int ExtNetworkLength() const 	{return StaticInt8Size;}; 
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_FAC_ENV&  i_FacEnv);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_FAC_ENV& o_FacEnv);

inline std::ostream& operator << (std::ostream& out, const GP_FAC_ENV& gp_fac_env);

struct  GP_FAC_ID
{
	enum EGP_FAC_ID
	{
		e_NoFacility			= 0,
		e_BRGS					= 1,
		e_CF					= 2,
		e_DADF					= 3,
		e_IMPF					= 4,
		e_MARF					= 5,
		e_MPEF					= 6,
		e_PGS					= 7,
		e_SSF					= 8,
		e_RGS					= 32,
		e_SAF_NWC				= 64,
		e_SAF_OSI				= 65,
		e_SAF_Ozone				= 66,
		e_SAF_4					= 67,
		e_ECMWF					= 128,
		e_EGSE					= 129,
		e_GGSPS					= 130,
		e_RTH					= 131,
		e_IQGSE					= 132,
		e_RFS					= 133,
		e_EXGATE				= 134,
		e_SGS					= 254,	//	TBD
		e_UnknownFacility		= 255	//	TBD
	} m_Fac_Id; 

	GP_FAC_ID():m_Fac_Id(e_DADF){};
	GP_FAC_ID(EGP_FAC_ID i_Fac_Id):m_Fac_Id(i_Fac_Id){};
	GP_FAC_ID(std::string i_string);
	
	void operator = (GP_FAC_ID rhs){ m_Fac_Id = rhs.m_Fac_Id; };
	bool operator < (const GP_FAC_ID rhs) const { return m_Fac_Id < rhs.m_Fac_Id;};
	bool operator > (const GP_FAC_ID rhs) const { return m_Fac_Id > rhs.m_Fac_Id;};
	bool operator == (const GP_FAC_ID rhs) const 
		{return m_Fac_Id == rhs.m_Fac_Id;}

	std::string ReadableForm() const;
	const unsigned int IntNetworkLength() const {return StaticInt8Size;}; 
	const unsigned int ExtNetworkLength() const {return StaticInt8Size;}; 
};


inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_FAC_ID&  i_FacId);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_FAC_ID& o_FacId);
inline std::ostream& operator << (std::ostream& out, const GP_FAC_ID& gp_fac_id);



typedef UNSIGNED			GP_MONITORING_ID;


struct GP_SC_CHAN_ID
{
	enum E_SC_CHAN_ID
	{
		e_NO_CHANNEL				= 0,
		e_VIS_0_6					= 1,
		e_VIS_0_8					= 2,
		e_IR_1_6					= 3,
		e_IR_3_9					= 4,
		e_WV_6_2					= 5,
		e_WV_7_3					= 6,
		e_IR_8_7					= 7,
		e_IR_9_7					= 8,
		e_IR_10_8					= 9,
		e_IR_12_0					= 10,
		e_IR_13_4					= 11,
		e_HRV						= 12,
		e_UnknownChannel			= 13
	} m_SC_CHAN_ID;

	GP_SC_CHAN_ID(){m_SC_CHAN_ID = e_NO_CHANNEL;};
	GP_SC_CHAN_ID(E_SC_CHAN_ID i_SCChanId){m_SC_CHAN_ID = i_SCChanId;};
	GP_SC_CHAN_ID(std::string i_string);

	GP_SC_CHAN_ID& operator = (const GP_SC_CHAN_ID i_SCchan)
		{m_SC_CHAN_ID = i_SCchan.m_SC_CHAN_ID;return *this;};

	GP_SC_CHAN_ID& operator = (const E_SC_CHAN_ID i_SCchan)
		{m_SC_CHAN_ID = i_SCchan;return *this;}

	bool operator == (const GP_SC_CHAN_ID& i_SCchan) const
		{return m_SC_CHAN_ID== i_SCchan.m_SC_CHAN_ID;};

	bool operator == (const GP_SC_CHAN_ID::E_SC_CHAN_ID& i_SCchan) const
		{return m_SC_CHAN_ID== i_SCchan;}
	
	std::string ReadableForm() const;
	const unsigned int IntNetworkLength() const {return StaticInt8Size;}; 
	const unsigned int ExtNetworkLength() const {return StaticInt8Size;}; 
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_SC_CHAN_ID&  i_SCChanId);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_SC_CHAN_ID& o_SCChanId);
inline std::ostream& operator << (std::ostream& out, const GP_SC_CHAN_ID& gp_sc_chan_id);


struct GP_SC_DETECT_ID
{
	enum E_SC_DETECT_ID
	{	
		e_No_Detector				= 0,
		e_VIS06SouthDetector		= 1,
		e_VIS06MiddleDetector		= 2,
		e_VIS06NorthDetector		= 3,
		e_VIS08SouthDetector		= 4,
		e_VIS08MiddleDetector		= 5,
		e_VIS08NorthDetector		= 6,
		e_IR16SouthDetector			= 7,
		e_IR16MiddleDetector		= 8,
		e_IR16NorthDetector			= 9,
		e_IR39SouthDetector			= 10,
		e_IR39MiddleDetector		= 11,
		e_IR39NorthDetector			= 12,
		e_WV62SouthDetector			= 13,
		e_WV62MiddleDetector		= 14,
		e_WV62NorthDetector			= 15,	
		e_WV73SouthDetector			= 16,	
		e_WV73MiddleDetector		= 17,
		e_WV73NorthDetector			= 18,
		e_IR87SouthDetector			= 19,
		e_IR87MiddleDetector		= 20,
		e_IR87NorthDetector			= 21,
		e_IR97SouthDetector			= 22,
		e_IR97MiddleDetector		= 23,
		e_IR97NorthDetector			= 24,
		e_IR108SouthDetector		= 25,
		e_IR108MiddleDetector		= 26,
		e_IR108NorthDetector		= 27,	
		e_IR120SouthDetector		= 28,
		e_IR120MiddleDetector		= 29,
		e_IR120NorthDetector		= 30,
		e_IR134SouthDetector		= 31,
		e_IR134MiddleDetector		= 32,
		e_IR134NorthDetector		= 33,
		e_HRVSouthernDetector		= 34,
		e_HRV1AboveSouthernDetector	= 35,	
		e_HRV2AboveSouthernDetector	= 36,
		e_HRV3AboveSouthernDetector	= 37,
		e_HRV4AboveSouthernDetector	= 38,		
		e_HRV3BelowNorthernDetector	= 39,
		e_HRV2BelowNorthernDetector	= 40,
		e_HRV1BelowNorthernDetector	= 41,
		e_HRVNorthernDetector		= 42,
		e_UnknownDetector			= 43
	}	m_SC_DETECT_ID;

	GP_SC_DETECT_ID(){m_SC_DETECT_ID = e_No_Detector;};
	GP_SC_DETECT_ID(E_SC_DETECT_ID i_SCDetectId){m_SC_DETECT_ID = i_SCDetectId;};
	GP_SC_DETECT_ID(std::string i_string);

	GP_SC_DETECT_ID& operator = (const GP_SC_DETECT_ID i_SCdetect)
		{m_SC_DETECT_ID = i_SCdetect.m_SC_DETECT_ID;return *this;};

	GP_SC_DETECT_ID& operator = (const E_SC_DETECT_ID i_SCdetect)
		{m_SC_DETECT_ID = i_SCdetect;return *this;}

	bool operator == (const GP_SC_DETECT_ID& i_SCdetect) const
	{return m_SC_DETECT_ID== i_SCdetect.m_SC_DETECT_ID;};

	bool operator == (const GP_SC_DETECT_ID::E_SC_DETECT_ID& i_SCdetect) const
		{return m_SC_DETECT_ID== i_SCdetect;}

	 std::string ReadableForm() const;
	 const unsigned int IntNetworkLength() const {return StaticInt8Size;}; 
	const unsigned int ExtNetworkLength() const {return StaticInt8Size;}; 
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_SC_DETECT_ID&  i_SCDetectId);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_SC_DETECT_ID& o_SCDetectId);

inline std::ostream& operator << (std::ostream& out, const GP_SC_DETECT_ID& gp_sc_detect_id);


struct GP_SC_ID
{
	enum E_SC_ID
	{	
		e_NoSpacecraft	= 0,
		e_METEOSAT_3	= 16,
		e_METEOSAT_4	= 19,
		e_METEOSAT_5	= 20, 
		e_METEOSAT_6	= 21,
		e_MTP_1			= 150,
		e_MTP_2			= 151,
		e_MSG_1			= 321,
		e_MSG_2			= 322,
		e_MSG_3			= 323,
		e_MSG_4			= 324,
		e_METOP_1		= 11,
		e_METOP_2		= 12,
		e_METOP_3		= 13,
		e_NOAA_12		= 17012,
		e_NOAA_13		= 17013,
		e_NOAA_14		= 17014,
		e_NOAA_15		= 17015,
		e_NOAA_16		= 17016,
		e_NOAA_17		= 17017,
		e_GOES_7		= 18007,
		e_GOES_8		= 18008,
		e_GOES_9		= 18009,
		e_GOES_10		= 18010,
		e_GOES_11		= 18011,
		e_GOES_12		= 18012,
		e_GOMS_1		= 19001,
		e_GOMS_2		= 19002,
		e_GOMS_3		= 19003,
		e_GMS_4			= 20004,
		e_GMS_5			= 20005,
		e_GMS_6			= 20006,
		e_MTSAT_1		= 21001,
		e_MTSAT_2		= 21002,
		e_UnknownSpacecraft	= 21003
	} m_SC_ID;

	GP_SC_ID(E_SC_ID i_SCId = e_NoSpacecraft){m_SC_ID = i_SCId;};
	GP_SC_ID(std::string i_string);

	bool operator == (const GP_SC_ID i_SCId) const 
		{return m_SC_ID == i_SCId.m_SC_ID;};
	
	GP_SC_ID& operator = (const GP_SC_ID i_SCId)
		{m_SC_ID = i_SCId.m_SC_ID;return *this;};
		
	const unsigned int IntNetworkLength () const { return StaticInt16Size;}
	const unsigned int ExtNetworkLength () const { return StaticInt16Size;}

	std::string ReadableForm 
	(
	)
	const
	// Returns:		Corresponding GP_SC_NAME value.
	// Exceptions:	No specific ones.
	{
		switch (m_SC_ID)
		{
			case e_NoSpacecraft	: return std::string("NoSpacecraft");
			case e_METEOSAT_3	: return std::string("METEOSAT3");
			case e_METEOSAT_4	: return std::string("METEOSAT4");
			case e_METEOSAT_5	: return std::string("METEOSAT5");
			case e_METEOSAT_6	: return std::string("METEOSAT6");
			case e_MTP_1		: return std::string("MTP1");
			case e_MTP_2		: return std::string("MTP2");
			case e_MSG_1		: return std::string("MSG1");
			case e_MSG_2		: return std::string("MSG2");
			case e_MSG_3		: return std::string("MSG3");
			case e_MSG_4		: return std::string("MSG4");
			case e_METOP_1		: return std::string("METOP1");
			case e_METOP_2		: return std::string("METOP2");
			case e_METOP_3		: return std::string("METOP3");
			case e_NOAA_12		: return std::string("NOAA12");
			case e_NOAA_13		: return std::string("NOAA13");
			case e_NOAA_14		: return std::string("NOAA14");
			case e_NOAA_15		: return std::string("NOAA15");
			case e_NOAA_16		: return std::string("NOAA16");
			case e_NOAA_17		: return std::string("NOAA17");
			case e_GOES_7		: return std::string("GOES7");
			case e_GOES_8		: return std::string("GOES8");
			case e_GOES_9		: return std::string("GOES9");
			case e_GOES_10		: return std::string("GOES10");
			case e_GOES_11		: return std::string("GOES11");
			case e_GOES_12		: return std::string("GOES12");
			case e_GOMS_1		: return std::string("GOMS1");
			case e_GOMS_2		: return std::string("GOMS2");
			case e_GOMS_3		: return std::string("GOMS3");
			case e_GMS_4		: return std::string("GMS4");
			case e_GMS_5		: return std::string("GMS5");
			case e_GMS_6		: return std::string("GMS6");
			case e_MTSAT_1		: return std::string("MTSAT1");
			case e_MTSAT_2		: return std::string("MTSAT2");
		}
		return std::string("UnknownSpacecraft");
	}
};


inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_SC_ID&  i_SCId);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_SC_ID& o_SCId);

inline std::ostream& operator << (std::ostream& out, const GP_SC_ID& gp_sc_id);


struct GP_SU_ID
{
	enum E_SU_ID
	{
		e_NULL_SU	= 0,
		e_DCPE		= 30000,
		e_DDFC		= 30001,
		e_DSRX		= 30002,
		e_DSPL		= 30003,
		e_DSPH		= 30004,
		e_DSPM		= 30005,
		e_DSSP		= 30006,
		e_SYSE		= 30007,
		e_RAE		= 30008,
		e_CMSE		= 30009,
		e_MMI		= 30010
	}				m_SU_ID;

	GP_SU_ID ():m_SU_ID(e_NULL_SU) {};
	GP_SU_ID (E_SU_ID i_SU_ID): m_SU_ID (i_SU_ID)  {};
	GP_SU_ID(std::string i_string);
	GP_SU_ID (UINT32 i_SU_ID)
	{
		m_SU_ID = static_cast<GP_SU_ID::E_SU_ID> (i_SU_ID);
	}

	operator GP_SU_ID (){ return m_SU_ID; }

	// Copy a SU Id
	void operator = (GP_SU_ID rhs){ m_SU_ID = rhs.m_SU_ID; }

	bool operator < (const GP_SU_ID rhs) const 
		{return m_SU_ID < rhs.m_SU_ID;}

	bool operator == (const GP_SU_ID rhs) const 
		{return m_SU_ID == rhs.m_SU_ID;}
	
	// A string representation of the SU id that can be used in the constructor
	// to obtain the same value.
	inline std::string SUName() const;
	std::string ReadableForm() const { return SUName(); }

	const unsigned int IntNetworkLength() const {return StaticInt32Size;}; 
	const unsigned int ExtNetworkLength() const {return StaticInt32Size;}; 

	int get() {return int(m_SU_ID);}
};
inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_SU_ID& i_SUId);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,GP_SU_ID& o_SUId);


inline std::ostream& operator << (std::ostream& out, const GP_SU_ID& gp_su_id);

struct GP_SVCE_ID
{
	enum E_SVCE_ID
	{
		//	GP_SVCE_ID values as defined in GSDS, Volume F
		e_NoService						= 0,
		e_15_Data						= 1,	//	TCP/IP 
		e_CONTROL						= 5,	//	TCP/IP
		e_DCP							= 6,	//	TCP/IP 
		e_GTS_DATA_IN					= 10,	//	TCP/IP 
		e_GTS_DATA_OUT					= 11,	//	TCP/IP 
		e_HRIT_DISSEMINATION			= 12,	//	TCP/IP 
		e_LRIT_DISSEMINATION			= 13,	//	TCP/IP 
		e_METPRODUCT					= 14,	//	FTP
		e_MONITORING					= 15,	//	TCP/IP
		e_RETRIEVAL						= 17,	//	FTP
		e_SGS							= 22,	//	FTP
		e_UTC_CORR						= 23,	//	TCP/IP
		e_ECMWF							= 30,	//	

		//	DADF Internal GP_SVCE_IDs (where required).  If one of the
		//	general service IDs are also appropriate for internal DADF
		//	usage, then it is used for internal use as well.

		//	DCP Messages / Bulletins to DDFC
		e_DCPE_DDFC						= 3000,

		//	DCPE Products (raw DCP Messages and GTS bulletins) from DDFC to DSRX
		e_DDFC_DSRX_dcp					= 3001,

		//	RTH Products (GTS messages) from DDFC to DSRX
		e_DDFC_DSRX_gts					= 3002,

		e_DSRX_DSPH = 3003,						// DSRX to DSPH (requests to process HRIT files)
		e_DSRX_DSPL = 3004,						// DSRX to DSPL (requests to process LRIT files)
		e_DSRX_DSSP = 3005,						// DSRX to DSSP (input expectation information,
												//               input file status data,
												//               overview status messages)
		e_DSPH_DSSP = 3006,						// DSPH to DSSP (HRIT file status data)
		e_DSPL_DSSP = 3007,						// DSPL to DSSP (LRIT file status data)
		e_DSPM_DSSP = 3008,						// DSPM to DSSP (HRIT/LRIT reception monitoring
												//               data from US)
		e_DSPM_US_MonitoringDataHRIT = 3009,	// US   to DSPM (HRIT Monitoring Data)
		e_DSPM_US_MonitoringDataLRIT = 3010,	// US   to DSPM (LRIT Monitoring Data)
		e_DSPM_US_FileTransferHRIT   = 3011,	// US   to DSPM (HRIT File Transfer)
		e_DSPM_US_FileTransferLRIT   = 3012,	// US   to DSPM (LRIT File Transfer)

		//	SYSE to DCPE (Control Link)
		e_SYSE_DCPE_Control = 3013,

		//	SYSE to DDFC (Control Link)
		e_SYSE_DDFC_Control = 3014,

		//	SYSE to DSRX (Control Link)
		e_SYSE_DSRX_Control	= 3015,
		
		//	SYSE to DSPL (Control Link)
		e_SYSE_DSPL_Control = 3016,

		//	SYSE to DSPH (Control Link)
		e_SYSE_DSPH_Control	= 3017,

		//	SYSE to DSSP (Control Link)
		e_SYSE_DSSP_Control = 3018,

		//	SYSE to DSPM (Control Link)
		e_SYSE_DSPM_Control = 3019,

		//	SYSE to DCPE (Monitoring Link)
		e_SYSE_DCPE_Monitoring = 3020,

		//	SYSE to DDFC (Monitoring Link)
		e_SYSE_DDFC_Monitoring = 3021,

		//	SYSE to DSRX (Monitoring Link)
		e_SYSE_DSRX_Monitoring = 3022,
		
		//	SYSE to DSPL (Monitoring Link)
		e_SYSE_DSPL_Monitoring = 3023,

		//	SYSE to DSPH (Monitoring Link)
		e_SYSE_DSPH_Monitoring = 3024,

		//	SYSE to DSSP (Monitoring Link)
		e_SYSE_DSSP_Monitoring = 3025,

		//	SYSE to DSPM (Monitoring Link)
		e_SYSE_DSPM_Monitoring = 3026,


		//	***   Other FTP Services   ***

		e_DDFC_DSRX_sgs			= 3028,
		e_DDFC_DSRX_mpef		= 3029,

		//	DDFC to OFL
		e_OFL_DDFC				= 3030,
		// link for administrative messages 
		// send from an MMI to DDFC
		e_DDFC_MMI_AdminMessages,
		
		//	Absorb Status Data from other DISE SUs
		e_DSSP_StatusDataInput	= 3032,


		// MMI Service Ids.
		e_MMI_Service			= 3500,

		e_SYSE_OverviewStatus,
		e_SYSE_SU_Statuses,
		e_SYSE_LogServer,
		e_SYSE_EventServer,
		e_SYSE_MonitoringPackets,
		e_SYSE_LinkStatuses,
		e_SYSE_MMI_Commanding,
		e_DSSP_OverviewStatus,
		e_DSSP_DataStreamMonitoringHRIT,
		e_DSSP_DataStreamMonitoringLRIT,
		e_DSSP_xRITAligned,
		e_DSPH_DataStreamCommandingHRIT,
		e_DSPL_DataStreamCommandingLRIT,
		e_DSRX_QuickLook_VIS_0_6,
		e_DSRX_QuickLook_VIS_0_8,
		e_DSRX_QuickLook_IR_1_6,
		e_DSRX_QuickLook_IR_3_9,
		e_DSRX_QuickLook_WV_6_2,
		e_DSRX_QuickLook_WV_7_3,
		e_DSRX_QuickLook_IR_8_7,
		e_DSRX_QuickLook_IR_9_7,
		e_DSRX_QuickLook_IR_10_8,
		e_DSRX_QuickLook_IR_12_0,
		e_DSRX_QuickLook_IR_13_4,
		e_DSRX_QuickLook_HRV,
		e_DSRX_SEVIRIRepeatCycle,
		e_DCPE_MessageSummary,
		e_DCPE_BulletinSummary,
		e_DCPE_ChannelStatus,
		e_DDFC_TCPReception,
		e_DDFC_FTPReception,
		e_DDFC_TCPDispatch,
		e_DDFC_FTPDispatch,
		//	RAE MMI control IF
		e_RAE_Control,
		
		e_UnknownServiceID

	} m_ServiceID;

	
	GP_SVCE_ID():m_ServiceID(e_DCPE_DDFC){};
	GP_SVCE_ID(E_SVCE_ID i_ServiceID):m_ServiceID(i_ServiceID){};
	GP_SVCE_ID(std::string i_string);

	void operator = (GP_SVCE_ID rhs)
		{ m_ServiceID = rhs.m_ServiceID; };
	
	bool operator < (const GP_SVCE_ID rhs) const 
		{ return m_ServiceID < rhs.m_ServiceID;};

	bool operator > (const GP_SVCE_ID rhs) const 
	{ return m_ServiceID > rhs.m_ServiceID;};

	bool operator == (const GP_SVCE_ID rhs) const
		{return m_ServiceID == rhs.m_ServiceID;}

	std::string ReadableForm() const;
	const unsigned int IntNetworkLength() const {return StaticInt16Size;}; 
	const unsigned int ExtNetworkLength() const {return StaticInt16Size;}; 
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_SVCE_ID&  i_SVCEId);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_SVCE_ID& o_SVCEId);


inline std::ostream& operator << (std::ostream& out, const GP_SVCE_ID& gp_svce_id);

struct GP_SVCE_TYPE
{
	enum E_SVCE_TYPE
	{
		e_ServiceTypeNotDefined			= 0,

		//	Service Types as defined in GSDS_F
		e_CommandVerification			= 129,
		e_GenericCommand				= 130,
		e_MonitoringandDiagnostic		= 131,
		e_SubsystemAlive				= 142,
		e_SpacecraftTelemetry			= 150,
		e_SpacecraftTelecommand			= 151,
		e_SpacecraftTelecommandAck		= 152,
		e_10ImageDelivery				= 153,
		e_15ImageDelivery				= 154,
		e_AuxDataDelivery				= 155,
		e_CalibrationMonFeedback		= 156,
		e_CatalogueDataDelivery			= 157,
		e_DcpDataDelivery				= 158,
		e_FlightDynamicsDataDelivery	= 159,
		e_FlightDynamicsDataInput		= 160,
		e_GERBDataDelivery				= 161,
		e_GTSDataDelivery				= 162,
		e_HistoricalDCPDelivery 		= 163,
		e_HistoricalGERBDelivery		= 164,
		e_HistoricalHKDelivery			= 165,
		e_HistoricalImageDelivery		= 166,
		e_HistoricalProductDelivery		= 167,
		e_HritDataDelivery				= 168,
		e_IQGSERunResult				= 169,
		e_LritDataDelivery				= 170,
		e_MetProductsDelivery			= 171,
		e_OrbitDataDelivery				= 172,
		e_SGSDataDelivery				= 173,
		e_SpinRate						= 174,
		e_UTCCorrelation				= 175,
		e_UnknownServiceType			= 176

	} m_SVCE_TYPE;
	
	GP_SVCE_TYPE(){m_SVCE_TYPE = e_ServiceTypeNotDefined;};
	GP_SVCE_TYPE(E_SVCE_TYPE i_SVCEType){m_SVCE_TYPE = i_SVCEType;};
	GP_SVCE_TYPE(std::string i_string);

	bool operator == (const E_SVCE_TYPE i_SVCEType) const 
		{return m_SVCE_TYPE == i_SVCEType;}

	bool operator == (const GP_SVCE_TYPE i_SVCEType) const
		{return m_SVCE_TYPE == i_SVCEType.m_SVCE_TYPE;}


	GP_SVCE_TYPE& operator = (const GP_SVCE_TYPE i_SVCEType)
		{m_SVCE_TYPE = i_SVCEType.m_SVCE_TYPE;return *this;};



	const unsigned int IntNetworkLength () const { return StaticInt8Size ;}
	const unsigned int ExtNetworkLength () const { return StaticInt8Size ;}

	std::string ReadableForm() const;
};
inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& i_Stream, const GP_SVCE_TYPE& i_SType);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,GP_SVCE_TYPE& o_SType);


inline std::ostream& operator << (std::ostream& out, const GP_SVCE_TYPE& gp_svce_type);


struct GP_CPU_ADDRESS
{
	GP_CPU_ADDRESS();
	GP_CPU_ADDRESS( const UNSIGNED_BYTE   i_Qualifier_1,
					const UNSIGNED_BYTE   i_Qualifier_2,
					const UNSIGNED_BYTE   i_Qualifier_3,
					const UNSIGNED_BYTE   i_Qualifier_4);

	GP_CPU_ADDRESS(const std::string& IP_Address);

    UNSIGNED_BYTE   m_Qualifier_1;
    UNSIGNED_BYTE   m_Qualifier_2;
    UNSIGNED_BYTE   m_Qualifier_3;
    UNSIGNED_BYTE   m_Qualifier_4;

	inline std::string dot_notation() const;


	GP_CPU_ADDRESS& operator = (const GP_CPU_ADDRESS i_cpu)
		{m_Qualifier_1	= i_cpu.m_Qualifier_1;
		m_Qualifier_2	= i_cpu.m_Qualifier_2;
		m_Qualifier_3	= i_cpu.m_Qualifier_3;
		m_Qualifier_4	= i_cpu.m_Qualifier_4;
		return *this;};

	bool operator == (const GP_CPU_ADDRESS& i_Address) const;

	bool operator < (const GP_CPU_ADDRESS& i_Address) const;

	const unsigned int IntNetworkLength() const  { return  m_Qualifier_1.IntNetworkLength() * 4;}; 
	const unsigned int ExtNetworkLength() const  { return  m_Qualifier_1.ExtNetworkLength() * 4;}; 

	inline std::string ReadableForm() const { return dot_notation(); }
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& i_Stream, const GP_CPU_ADDRESS& i_Addr);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,GP_CPU_ADDRESS& i_Addr);



typedef GP_CPU_ADDRESS GP_SVCE_ADDRESS;

inline std::ostream& operator << (std::ostream& out, const GP_CPU_ADDRESS& gp_cpu_address);

struct GP_COMMAND_ACK_FLAG
{
	BOOLEAN_BYTE			m_Acceptance;
	BOOLEAN_BYTE			m_Start;
	BOOLEAN_BYTE			m_Progress;
	BOOLEAN_BYTE			m_Completion;

	GP_COMMAND_ACK_FLAG() : 
		m_Acceptance(0),
		m_Start(0),
		m_Progress(0),
		m_Completion(0)
	{}

	GP_COMMAND_ACK_FLAG(GP_COMMAND_ACK_FLAG& i_CommandAck) : 
		m_Acceptance(i_CommandAck.m_Acceptance),
		m_Start(i_CommandAck.m_Start),
		m_Progress(i_CommandAck.m_Progress),
		m_Completion(i_CommandAck.m_Completion)
	{}

	~GP_COMMAND_ACK_FLAG(){}


	bool operator ==(const GP_COMMAND_ACK_FLAG& i_AckFlag) const 
	{
		return (m_Acceptance == i_AckFlag.m_Acceptance &&
				m_Start == i_AckFlag.m_Start &&
				m_Progress == i_AckFlag.m_Progress &&
				m_Completion == i_AckFlag.m_Completion);
	}

	GP_COMMAND_ACK_FLAG& operator =(const GP_COMMAND_ACK_FLAG& i_AckFlag)
	{
		m_Acceptance = i_AckFlag.m_Acceptance;
		m_Start = i_AckFlag.m_Start;
		m_Progress = i_AckFlag.m_Progress ;
		m_Completion = i_AckFlag.m_Completion;
		return *this;
	}

	const unsigned int IntNetworkLength() const {return  m_Acceptance.IntNetworkLength() * 4;};
	const unsigned int ExtNetworkLength() const {return  m_Acceptance.ExtNetworkLength() * 4;};
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& i_Stream, const GP_COMMAND_ACK_FLAG& i_Ack);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream,GP_COMMAND_ACK_FLAG& i_Ack);



struct GP_CONFIG_ITEM_VERSION
{	
	UNSIGNED_SHORT			m_Issue;
	UNSIGNED_SHORT			m_Revision;

	GP_CONFIG_ITEM_VERSION (UNSIGNED_SHORT i_Issue = 0, UNSIGNED_SHORT i_Revision = 0) 
		:	m_Issue (i_Issue),
			m_Revision (i_Revision) {}

	GP_CONFIG_ITEM_VERSION& operator = (const GP_CONFIG_ITEM_VERSION i_Config)
		{m_Issue = i_Config.m_Issue;
		m_Revision = i_Config.m_Revision;
		return *this;};

	bool operator == (const GP_CONFIG_ITEM_VERSION& i_Config) const
		{return m_Issue== i_Config.m_Issue &&
				m_Revision == i_Config.m_Revision;};

	const unsigned int IntNetworkLength() const  
	{ 
		return m_Issue.IntNetworkLength () + m_Revision.IntNetworkLength ();
	}
	const unsigned int ExtNetworkLength() const  
	{ 
		return m_Issue.ExtNetworkLength () + m_Revision.ExtNetworkLength ();
	}

	std::string ReadableForm() const
	{
		return m_Issue.ReadableForm () + "." + m_Revision.ReadableForm ();
	}
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_CONFIG_ITEM_VERSION&  i_ConfigIVersion);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_CONFIG_ITEM_VERSION& o_ConfigIVersion);
inline std::ostream& operator << (std::ostream& out, const GP_CONFIG_ITEM_VERSION& gp_config_item_version);


struct GP_DBASE_ID
{
	GP_SC_ID				m_SpacecraftId;
	UNSIGNED				m_DBaseId;

	GP_DBASE_ID& operator = (const GP_DBASE_ID i_DBase)
		{m_SpacecraftId = i_DBase.m_SpacecraftId;
		m_DBaseId = i_DBase.m_DBaseId;
		return *this;};

	bool operator == (const GP_DBASE_ID& i_DBase) const
		{return m_SpacecraftId== i_DBase.m_SpacecraftId &&
				m_DBaseId == i_DBase.m_DBaseId;};

	const unsigned int IntNetworkLength() const 	
	{
		return		m_SpacecraftId.IntNetworkLength() + m_DBaseId.IntNetworkLength();
	}; 
	const unsigned int ExtNetworkLength() const 
	{
		return		m_SpacecraftId.ExtNetworkLength() + m_DBaseId.ExtNetworkLength();
	};

};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_DBASE_ID&  i_DBaseId);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_DBASE_ID& o_DBaseId);
inline std::ostream& operator << (std::ostream& out, const GP_DBASE_ID& gp_dbase_id);


struct GP_SW_VERSION
{
	UNSIGNED_SHORT				m_Issue;
	UNSIGNED_SHORT				m_Revision;

	GP_SW_VERSION (UNSIGNED_SHORT i_Issue = 0, UNSIGNED_SHORT i_Revision = 0) 
		:	m_Issue (i_Issue),
			m_Revision (i_Revision) {}

	GP_SW_VERSION& operator = (const GP_SW_VERSION i_SWVersion)
		{m_Issue = i_SWVersion.m_Issue;
		 m_Revision = i_SWVersion.m_Revision;	
		return *this;};

	bool operator == (const GP_SW_VERSION& i_SWVersion) const
		{return m_Issue== i_SWVersion.m_Issue &&
				m_Revision == i_SWVersion.m_Revision;};

	const unsigned int IntNetworkLength() const  
	{ 
		return m_Issue.IntNetworkLength () + m_Revision.IntNetworkLength ();
	}
	const unsigned int ExtNetworkLength() const  
	{ 
		return m_Issue.ExtNetworkLength () + m_Revision.ExtNetworkLength ();
	}
	std::string ReadableForm() const
	{
		return m_Issue.ReadableForm () + "." + m_Revision.ReadableForm ();
	}
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_SW_VERSION&  i_SWVersion);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_SW_VERSION& o_SWVersion);
inline std::ostream& operator << (std::ostream& out, const GP_SW_VERSION& gp_sw_version);


typedef CHARACTERSTRING			GP_VOLUME_DESCRIPTION;
// VARIABLE CHARACTER STRING (65536)

typedef CHARACTERSTRING			GP_VOLUME_HEADER;
// CHARACTER STRING (50)


struct GP_DU_ID
{
	enum E_DU_ID
	{
		e_NULL_DU								= 0,
		e_DCPE_ControlDB						= 31000,
		e_DCPE_Links							= 31001,
		e_DCPE_IBConfig							= 31002,
		e_DDFC_ControlDB						= 31003,
		e_DDFC_Links							= 31004,
		e_DDFC_IBConfig							= 31005,
		e_DSSP_IBConfig							= 31006,
		e_DSPH_ProcessHRIT						= 31007,
		e_DSPL_ProcessLRIT						= 31008,
		e_DSRX_ProcessingSchemeHRIT				= 31009,
		e_DSRX_ProcessingSchemeLRIT				= 31010,
		e_DSRX_Receive							= 31011,
		e_DSRX_Links							= 31012,
		e_DSPL_Links							= 31013,
		e_DSPH_Links							= 31014,
		e_DSSP_Links							= 31015,
		e_DSPM_Links							= 31016,
		e_KCE_EncryptionExportLRIT				= 31017,
		e_KCE_EncryptionExportHRIT				= 31018,
		e_SYSE_IBConfig							= 31019,
		e_SYSE_Links							= 31020,
		e_SYSE_MandC							= 31021,
		e_SYSE_ConfigurationControl				= 31022,
		e_SYSE_Macro							= 31023,
		e_RAE_ConfigDB							= 31024,
		e_RAE_IBConfig							= 31025,
		e_RAE_Links								= 31026,
		e_RAE_Events							= 31027,

	}								m_DU_ID;

	GP_DU_ID () {};
	GP_DU_ID (E_DU_ID i_DU_ID): m_DU_ID (i_DU_ID)  {};
	GP_DU_ID(std::string i_string);
	operator GP_DU_ID (){ return m_DU_ID; }

	GP_DU_ID (UINT32 i_DU_ID)
	{
		m_DU_ID = static_cast<GP_DU_ID::E_DU_ID> (i_DU_ID);
	}

	// Copy a DU Id
	void operator = (GP_DU_ID rhs){ m_DU_ID = rhs.m_DU_ID; }

	bool operator< (GP_DU_ID rhs) const { return m_DU_ID < rhs.m_DU_ID; }

	bool operator== (GP_DU_ID rhs) const { return m_DU_ID == rhs.m_DU_ID; }

	std::string ReadableForm() const;

	const unsigned int IntNetworkLength() const  { return StaticInt32Size; }
	const unsigned int ExtNetworkLength() const  { return StaticInt32Size; }

	int get() {return int(m_DU_ID);}

};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_DU_ID&  i_DUId);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_DU_ID& o_DUId);


struct GP_FTP_LINK_MODE
{
	enum E_FTP_Link_Mode 
	{
		e_FTP_Enable		= 0,
		e_FTP_LinkDisable	= 1,
	} m_FTP_Link_Mode;

	GP_FTP_LINK_MODE(E_FTP_Link_Mode i_link_mode = e_FTP_Enable){m_FTP_Link_Mode = i_link_mode;};
	GP_FTP_LINK_MODE(std::string i_string);
	
	std::string ReadableForm() const;
	

	GP_FTP_LINK_MODE& operator = (const E_FTP_Link_Mode i_FTPLinkmode)
		{m_FTP_Link_Mode = i_FTPLinkmode;return *this;};

	GP_FTP_LINK_MODE& operator = (const GP_FTP_LINK_MODE i_FTPLinkmode)
		{m_FTP_Link_Mode = i_FTPLinkmode.m_FTP_Link_Mode;return *this;};

	bool operator == (const GP_FTP_LINK_MODE::E_FTP_Link_Mode& i_FTPLinkmode) const
		{return m_FTP_Link_Mode== i_FTPLinkmode;};

	bool operator == (const GP_FTP_LINK_MODE& i_FTPLinkmode) const
		{return m_FTP_Link_Mode== i_FTPLinkmode.m_FTP_Link_Mode;};

	const unsigned int IntNetworkLength() const  { return StaticInt8Size; }
	const unsigned int ExtNetworkLength() const  { return StaticInt8Size; }

};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_FTP_LINK_MODE&  i_FTPLinkmode);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_FTP_LINK_MODE& o_FTPLinkmode);


struct GP_FTP_LINK_STATE
{
	enum E_FTP_Link_State
	{
		e_FTP_Connecting					= 0,
		e_FTP_ConnectError					= 1,
		e_FTP_ConnectionAttemptsExceeded	= 2,
		e_FTP_Transferring					= 3,
		e_FTP_TransferComplete				= 4,
		e_FTP_TransferError					= 5,
		e_FTP_TransferAttemptsExceeded		= 6,
	} m_FTP_Link_State;

	GP_FTP_LINK_STATE(E_FTP_Link_State i_link_state = e_FTP_Connecting){m_FTP_Link_State = i_link_state;};
	GP_FTP_LINK_STATE(std::string i_string);
	
	std::string ReadableForm() const;

	GP_FTP_LINK_STATE& operator = (const E_FTP_Link_State i_FTPLinkstate)
		{m_FTP_Link_State = i_FTPLinkstate;return *this;};

	GP_FTP_LINK_STATE& operator = (const GP_FTP_LINK_STATE i_FTPLinkstate)
		{m_FTP_Link_State = i_FTPLinkstate.m_FTP_Link_State;return *this;};

	bool operator == (const GP_FTP_LINK_STATE::E_FTP_Link_State& i_FTPLinkstate) const
		{return m_FTP_Link_State== i_FTPLinkstate;};

	bool operator == (const GP_FTP_LINK_STATE& i_FTPLinkstate) const
		{return m_FTP_Link_State== i_FTPLinkstate.m_FTP_Link_State;};

	const unsigned int IntNetworkLength() const  { return StaticInt8Size; }
	const unsigned int ExtNetworkLength() const  { return StaticInt8Size; }

};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_FTP_LINK_STATE&  i_FTPLinkstate);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_FTP_LINK_STATE& o_FTPLinkstate);

struct GP_PACKET_HEADER_FAILURE
{
	enum E_PACKET_HEADER_FAILURE
	{
		e_HeaderVersion				= 0,
		e_PacketType				= 1,
		e_SubHeaderType				= 2,
		e_SourceFacility			= 3,
		e_SourceEnvironment			= 4,
		e_SourceInstance			= 5,
		e_SourceSU					= 6,
		e_SourceCPU					= 7,
		e_DestinationFacility		= 8,
		e_DestinationEnvironment	= 9,
		e_SequenceCount				= 10,
		e_Length					= 11,
		e_SubHeaderVersion			= 12,
		e_ServiceType				= 13,
		e_ServiceSubType			= 14,
		e_InvalidTimestamp			= 15
	};

	GP_PACKET_HEADER_FAILURE(const E_PACKET_HEADER_FAILURE i_PACKET_HEADER_FAILURE = e_HeaderVersion) :
		m_PACKET_HEADER_FAILURE(i_PACKET_HEADER_FAILURE){}

	GP_PACKET_HEADER_FAILURE& operator = (const E_PACKET_HEADER_FAILURE i_PACKET_HEADER_FAILURE)
		{m_PACKET_HEADER_FAILURE = i_PACKET_HEADER_FAILURE;return *this;}

	GP_PACKET_HEADER_FAILURE& operator = (const GP_PACKET_HEADER_FAILURE i_PACKET_HEADER_FAILURE)
		{m_PACKET_HEADER_FAILURE = i_PACKET_HEADER_FAILURE.m_PACKET_HEADER_FAILURE;return *this;}

	bool operator == (const GP_PACKET_HEADER_FAILURE::E_PACKET_HEADER_FAILURE& i_PACKET_HEADER_FAILURE) const
		{return m_PACKET_HEADER_FAILURE== i_PACKET_HEADER_FAILURE;}

	bool operator == (const GP_PACKET_HEADER_FAILURE& i_PACKET_HEADER_FAILURE) const
		{return m_PACKET_HEADER_FAILURE== i_PACKET_HEADER_FAILURE.m_PACKET_HEADER_FAILURE;}

	E_PACKET_HEADER_FAILURE		m_PACKET_HEADER_FAILURE;

	std::string ReadableForm() const;
	const unsigned int IntNetworkLength() const  { return StaticInt8Size; }
	const unsigned int ExtNetworkLength() const  { return StaticInt8Size; }
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_PACKET_HEADER_FAILURE&  i_PACKET_HEADER_FAILURE);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_PACKET_HEADER_FAILURE& o_PACKET_HEADER_FAILURE);

struct GP_SU_STATE
{
	enum E_SU_STATE
	{
		e_OFF			= 1,
		e_ON			= 2,
		e_DEGRADED		= 3,
		e_OTHER			= 4,
	}				m_SU_STATE;

	GP_SU_STATE(const E_SU_STATE i_SU_STATE = e_OFF) : 
	m_SU_STATE(i_SU_STATE){}
	GP_SU_STATE(std::string i_string);

	GP_SU_STATE& operator = (const E_SU_STATE i_SU_STATE)
		{m_SU_STATE = i_SU_STATE; return *this;}

	GP_SU_STATE& operator = (const GP_SU_STATE i_SU_STATE)
		{m_SU_STATE = i_SU_STATE.m_SU_STATE; return *this;}

	bool operator == (const GP_SU_STATE::E_SU_STATE i_SU_STATE) const 
	{
		return m_SU_STATE == i_SU_STATE;
	}

	std::string ReadableForm() const;
	const unsigned int IntNetworkLength() const  { return StaticInt8Size; }
	const unsigned int ExtNetworkLength() const  { return StaticInt8Size; }
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_SU_STATE&  i_SUState);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_SU_STATE& o_SUState);


struct GP_SU_MODE
{
	enum E_SU_MODE
	{
		e_OFF		= 0,
		e_ON		= 1,
		e_OTHER		= 2,
	}				m_SU_MODE;

	GP_SU_MODE(const E_SU_MODE i_SU_MODE = e_OFF) : 
	m_SU_MODE(i_SU_MODE){}
	GP_SU_MODE(std::string i_string);

	GP_SU_MODE& operator = (const E_SU_MODE i_SU_MODE)
		{m_SU_MODE = i_SU_MODE; return *this;}

	GP_SU_MODE& operator = (const GP_SU_MODE i_SU_MODE)
		{m_SU_MODE = i_SU_MODE.m_SU_MODE; return *this;}

	bool operator == (const GP_SU_MODE::E_SU_MODE i_SU_MODE) const 
	{
		return m_SU_MODE == i_SU_MODE;
	}

	std::string ReadableForm() const;
	const unsigned int IntNetworkLength() const  { return StaticInt8Size; }
	const unsigned int ExtNetworkLength() const  { return StaticInt8Size; }
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_SU_MODE&  i_SUMode);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_SU_MODE& o_SUMode);


struct GP_TCP_LINK_MODE
{
	enum E_TCP_LINK_MODE
	{
		e_ENABLED		= 0,
		e_DISABLED		= 1
	};

	GP_TCP_LINK_MODE(const E_TCP_LINK_MODE i_TCPLinkMode = e_ENABLED) : 
	m_TCPLinkMode(i_TCPLinkMode){}
	GP_TCP_LINK_MODE(std::string i_string);

	GP_TCP_LINK_MODE& operator = (const E_TCP_LINK_MODE i_TCPLinkMode)
		{m_TCPLinkMode = i_TCPLinkMode; return *this;}

	GP_TCP_LINK_MODE& operator = (const GP_TCP_LINK_MODE i_TCPLinkMode)
		{m_TCPLinkMode = i_TCPLinkMode.m_TCPLinkMode; return *this;}

	bool operator == (const GP_TCP_LINK_MODE::E_TCP_LINK_MODE i_TCPLinkMode) const 
	{
		return m_TCPLinkMode == i_TCPLinkMode;
	}


	E_TCP_LINK_MODE m_TCPLinkMode;

	std::string ReadableForm() const;
	const unsigned int IntNetworkLength() const  { return StaticInt8Size; }
	const unsigned int ExtNetworkLength() const  { return StaticInt8Size; }
};


inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_TCP_LINK_MODE&  i_TCPLinkmode);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_TCP_LINK_MODE& o_TCPLinkmode);


struct GP_TCP_LINK_STATE
{
	enum E_TCP_LINK_STATE
	{
		e_OFF_STATE						= 0,
		e_CANCELLING_STATE				= 1,
		e_WAIT_STATE					= 2,
		e_RELEASING_STATE				= 3,
		e_ESTABLISHED_STATE_CONNECT		= 4,
		e_ESTABLISHED_STATE_RECONNECT	= 5
	};

	GP_TCP_LINK_STATE(const E_TCP_LINK_STATE i_TCPLinkState = e_OFF_STATE) : 
		m_TCPLinkState(i_TCPLinkState){}
		GP_TCP_LINK_STATE(std::string i_string);

	GP_TCP_LINK_STATE& operator = (const E_TCP_LINK_STATE i_TCPLinkState)
		{m_TCPLinkState = i_TCPLinkState; return *this;}

	GP_TCP_LINK_STATE& operator = (const struct GP_TCP_LINK_STATE& i_TCPLinkState)
		{m_TCPLinkState = i_TCPLinkState.m_TCPLinkState; return *this;}

	bool operator == (const GP_TCP_LINK_STATE::E_TCP_LINK_STATE i_TCPLinkState) const
		{return m_TCPLinkState == i_TCPLinkState;}

	bool operator == (const GP_TCP_LINK_STATE& i_TCPLinkState) const
		{return m_TCPLinkState == i_TCPLinkState.m_TCPLinkState;}

	E_TCP_LINK_STATE m_TCPLinkState;

	std::string ReadableForm() const;
	const unsigned int IntNetworkLength() const  { return StaticInt8Size; }
	const unsigned int ExtNetworkLength() const  { return StaticInt8Size; }
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_TCP_LINK_STATE&  i_TCPLinkstate);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_TCP_LINK_STATE& o_TCPLinkstate);


struct GP_TCP_SESSION_MODE
{
	enum E_TCP_SESSION_MODE
	{
		e_CONNECT_MODE		= 0,
		e_RECONNECT_MODE	= 1
	};
				
	GP_TCP_SESSION_MODE(const E_TCP_SESSION_MODE i_TCPSessionMode =e_CONNECT_MODE) :
		m_TCPSessionMode(i_TCPSessionMode){}
	GP_TCP_SESSION_MODE(std::string i_string);

	GP_TCP_SESSION_MODE& operator = (const E_TCP_SESSION_MODE i_TCPSessionMode)
		{m_TCPSessionMode = i_TCPSessionMode;return *this;}

	GP_TCP_SESSION_MODE& operator = (const GP_TCP_SESSION_MODE i_TCPSessionMode)
		{m_TCPSessionMode = i_TCPSessionMode.m_TCPSessionMode;return *this;}

	bool operator == (const GP_TCP_SESSION_MODE::E_TCP_SESSION_MODE& i_TCPSessionMode) const
		{return m_TCPSessionMode== i_TCPSessionMode;}

	bool operator == (const GP_TCP_SESSION_MODE& i_TCPSessionMode) const
		{return m_TCPSessionMode== i_TCPSessionMode.m_TCPSessionMode;}

	E_TCP_SESSION_MODE m_TCPSessionMode;

	std::string ReadableForm() const;
	const unsigned int IntNetworkLength() const  { return StaticInt8Size; }
	const unsigned int ExtNetworkLength() const  { return StaticInt8Size; }
};


inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_TCP_SESSION_MODE&  i_TCPSessionMode);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_TCP_SESSION_MODE& o_TCPSessionMode);



struct GP_TCP_REPLACE_MODE
{
	enum E_TCP_REPLACE_MODE
	{
		e_REPLACE_MODE		= 0,
		e_REJECT_MODE		= 1
	};

	GP_TCP_REPLACE_MODE(const E_TCP_REPLACE_MODE i_TCPReplaceMode = e_REPLACE_MODE) :
		m_TCPReplaceMode(i_TCPReplaceMode){}
	GP_TCP_REPLACE_MODE(std::string i_string);


	GP_TCP_REPLACE_MODE& operator = (const E_TCP_REPLACE_MODE i_TCPReplaceMode)
		{m_TCPReplaceMode = i_TCPReplaceMode;return *this;}

	GP_TCP_REPLACE_MODE& operator = (const GP_TCP_REPLACE_MODE i_TCPReplaceMode)
		{m_TCPReplaceMode = i_TCPReplaceMode.m_TCPReplaceMode;return *this;}

	bool operator == (const GP_TCP_REPLACE_MODE::E_TCP_REPLACE_MODE& i_TCPReplaceMode) const
		{return m_TCPReplaceMode== i_TCPReplaceMode;}

	bool operator == (const GP_TCP_REPLACE_MODE& i_TCPReplaceMode) const
		{return m_TCPReplaceMode== i_TCPReplaceMode.m_TCPReplaceMode;}

	E_TCP_REPLACE_MODE m_TCPReplaceMode;

	std::string ReadableForm() const;
	const unsigned int IntNetworkLength() const  { return StaticInt8Size; }
	const unsigned int ExtNetworkLength() const  { return StaticInt8Size; }
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_TCP_REPLACE_MODE&  i_TCPReplacemode);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_TCP_REPLACE_MODE& o_TCPReplacemode);


struct  GP_FILE_TYPE
{
	enum E_FILE_TYPE
	{
		e_Monitoring			= 0,
		e_Command				= 1,
		e_MissionData			= 2,
		e_Simulated_Monitoring	= 128,
		e_Simulated_Command		= 129,
		e_Simulated_MissionData	= 130
	}; 

	GP_FILE_TYPE(const E_FILE_TYPE i_File_Type = e_Monitoring) :
		m_File_Type(i_File_Type){}
	GP_FILE_TYPE(std::string i_string);


	GP_FILE_TYPE& operator = (const E_FILE_TYPE i_File_Type)
		{m_File_Type = i_File_Type;return *this;}

	GP_FILE_TYPE& operator = (const GP_FILE_TYPE i_File_Type)
		{m_File_Type = i_File_Type.m_File_Type;return *this;}

	bool operator == (const GP_FILE_TYPE::E_FILE_TYPE& i_File_Type) const
		{return m_File_Type== i_File_Type;}

	bool operator == (const GP_FILE_TYPE& i_File_Type) const
		{return m_File_Type== i_File_Type.m_File_Type;}

	E_FILE_TYPE m_File_Type;

	std::string ReadableForm() const;
	const unsigned int IntNetworkLength() const  { return StaticInt8Size; }
	const unsigned int ExtNetworkLength() const  { return StaticInt8Size; }
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_FILE_TYPE&  i_File_Type);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_FILE_TYPE& o_File_Type);

struct  GP_SUBHEADER_TYPE
{
	enum E_SUBHEADER_TYPE
	{
		e_NoSubheader			= 0,
		e_GP_FI_SH1				= 1
	}; 

	GP_SUBHEADER_TYPE(const E_SUBHEADER_TYPE i_Subheader_Type = e_NoSubheader) :
		m_Subheader_Type(i_Subheader_Type){}
	GP_SUBHEADER_TYPE(std::string i_string);


	GP_SUBHEADER_TYPE& operator = (const E_SUBHEADER_TYPE i_Subheader_Type)
		{m_Subheader_Type = i_Subheader_Type;return *this;}

	GP_SUBHEADER_TYPE& operator = (const GP_SUBHEADER_TYPE i_Subheader_Type)
		{m_Subheader_Type = i_Subheader_Type.m_Subheader_Type;return *this;}

	bool operator == (const GP_SUBHEADER_TYPE::E_SUBHEADER_TYPE& i_Subheader_Type) const
		{return m_Subheader_Type == i_Subheader_Type;}

	bool operator == (const GP_SUBHEADER_TYPE& i_Subheader_Type) const
		{return m_Subheader_Type == i_Subheader_Type.m_Subheader_Type;}

	E_SUBHEADER_TYPE m_Subheader_Type;

	std::string ReadableForm() const;
	const unsigned int IntNetworkLength() const  { return StaticInt8Size; }
	const unsigned int ExtNetworkLength() const  { return StaticInt8Size; }
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_SUBHEADER_TYPE&  i_Subheader_Type);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_SUBHEADER_TYPE& o_Subheader_Type);


struct GP_FI_HEADER
{
	struct CHAR409
	{
		std::string m_value;

		CHAR409(const std::string& i_value = "")
		{
			m_value=i_value;
			m_value.resize(409,' ');	
		};

		std::string ReadableForm() const
		{	
			return m_value;
		}

		void operator = ( std::string& i_other)
			{m_value=i_other;m_value.resize(409);}

		const unsigned int IntNetworkLength() const {return 409;};
		const unsigned int ExtNetworkLength() const {return 409;};
	};


	UNSIGNED_BYTE		m_HeaderVersionNo;
	GP_FILE_TYPE		m_FileType;
	GP_SUBHEADER_TYPE	m_SubHeaderType;
	GP_FAC_ID			m_SourceFacilityId;
	GP_FAC_ENV			m_SourceEnvId;
	UNSIGNED_BYTE		m_SourceInstanceId;
	GP_SU_ID			m_SourceSUId;
	GP_CPU_ADDRESS		m_SourceCPUId;
	GP_FAC_ID			m_DestFacilityId;
	GP_FAC_ENV			m_DestEnvId;
	UNSIGNED			m_DataFieldLength;
	CHAR409				m_Description;

	void SetDescriptionField();

	inline unsigned int ExtNetworkLength()const;
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_FI_HEADER&  i_FIHeader);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_FI_HEADER& o_FIHeader);


struct GP_FI_SH1
{
	struct CHAR187
	{
		std::string m_value;

		CHAR187(const std::string& i_value = "")
		{
			m_value=i_value;
			m_value.resize(187,' ');	
		};

		std::string ReadableForm() const
		{	
			return m_value;
		}

		void operator = ( std::string& i_other)
			{m_value=i_other;m_value.resize(187);}

		const unsigned int IntNetworkLength() const {return 187;};
		const unsigned int ExtNetworkLength() const {return 187;};
	};

	UNSIGNED_BYTE  m_SubheaderVersionNo;
	GP_SVCE_TYPE   m_ServiceType;
	UNSIGNED_BYTE  m_ServiceSubtype;
	Util::CMSGTime m_FileTime;
	GP_SC_ID       m_SpacecraftId;
	CHAR187		   m_Description;

	void SetDescriptionField();

	inline unsigned int ExtNetworkLength()const;
};

inline 	COMS::CMSGByteStream& operator << (COMS::CMSGByteStream& o_Stream, const GP_FI_SH1&  i_FI_SH1);
inline 	COMS::CMSGByteStream& operator >> (COMS::CMSGByteStream& i_Stream, GP_FI_SH1& o_FI_SH1);


#include "GSDS_Volume_F_Impl.h"

#endif	// GSDS_VOLUME_F_HEADER




