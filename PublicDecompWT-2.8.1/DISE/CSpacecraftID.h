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

#ifndef CSpacecraftID_included
#define CSpacecraftID_included

/*******************************************************************************

TYPE:
Concrete class.
					
PURPOSE:
Container for a spacecraft identifier.

FUNCTION:
Not applicable.

INTERFACES:
None.

RESOURCES:	
None.

REFERENCES:
GSDS, Volume F, EUM/MSG/SPE/055.

PROCESSING:
In addition to the interface of the GP_SC_ID structure, constructors and conversion
operators are provided which allow to convert the spacecraft identifier to/from
simple integer representations.

DATA:
None.

LOGIC:
-

*******************************************************************************/



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

inline std::ostream& operator << (std::ostream& out, const GP_SC_ID& gp_sc_id);


namespace DISE
{




class CSpacecraftID : public GP_SC_ID
{

public:

// INTERFACES:

	// Description:	Constructor.
	// Returns:		Nothing.
	CSpacecraftID
	(
		unsigned short i_ = 0	// Spacecraft identifier.
	)
	{
		m_SC_ID = E_SC_ID(i_);
	}

	// Description:	Destructor.
	// Returns:		Nothing.
	virtual ~CSpacecraftID
	(
	)
	{
	}

	// Description:	retruns GP_SC_NAME representation of spacecraft identifier.
	// Returns:		Spacecraft name as defined in GSDS Volume F (GP_SC_NAME).
	std::string GP_SC_NAME
	(
	)
	const
	{
		return ReadableForm();
	}

	// Description:	Loads identifier.
	// Returns:		Reference to object.
	CSpacecraftID& operator=
	(
		unsigned short i_	// GP_SC_ID-compatible integer value.
	)
	{
		m_SC_ID = E_SC_ID(i_);
		return *this;
	}

	// Description:	Returns a GP_SC_ID-compatible integer value.
	// Returns:		A GP_SC_ID-compatible integer value..
	operator unsigned short
	(
	)
	const
	{
		return m_SC_ID;
	}

};




} // end namespace


#endif
