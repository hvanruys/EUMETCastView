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

#ifndef CSpectralChannelID_included
#define CSpectralChannelID_included

/*******************************************************************************

TYPE:
Concrete class..
					
PURPOSE:
Container for a spectral channel identifier.

FUNCTION:
Not applicable.

INTERFACES:
None.

RESOURCES:	
None.

REFERENCES:
GSDS, Volume F, EUM/MSG/SPE/055.
LRIT/HRIT Mission Specific Implementation, EUM/MSG/SPE/057.

PROCESSING:
Constructors and conversion operators are provided which allow to convert the
spectral channel identifier to/from simple integer representations.
In addition to GP_SC_CHAN_ID, the following methods are provided:
- Conversion to/from integer types.
- Conversion of SEVIRI spectral channel identifiers into a name string as defined
  in LRIT/HRIT MSI.

DATA:
None.

LOGIC:
-

*******************************************************************************/



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

};

inline std::ostream& operator << (std::ostream& out, const GP_SC_CHAN_ID& gp_sc_chan_id);

namespace DISE
{




class CSpectralChannelID : public GP_SC_CHAN_ID
{

public:

	// Value range of SEVIRI spectral channel identifiers.
	enum
	{
		e_FirstSEVIRIChannel =  1,
		e_LastSEVIRIChannel  = 12
	};

// INTERFACES:

	// Description:	Constructor.
	// Returns:		Nothing.
	CSpectralChannelID
	(
		const unsigned char i_ID = 0	// Spectral channel identifier.
	)
	{
		m_SC_CHAN_ID = E_SC_CHAN_ID(i_ID);
	}

	// Description:	Destructor.
	// Returns:		Nothing.
	virtual ~CSpectralChannelID
	(
	)
	{
	}

	// Description:	Converts identifier into a name string of a SEVIRI channel
	//				as defined in LRIT/HRIT MSI.
	//				DO NOT USE for other imagetaking spacecrafts than MSG satellites!
	// Returns:		SEVIRI Spectral channel name.
	std::string GetNameSEVIRI
	(
	)
	const
	{
		switch (m_SC_CHAN_ID)
		{
			case GP_SC_CHAN_ID::e_NO_CHANNEL : return std::string("");
			case GP_SC_CHAN_ID::e_VIS_0_6    : return std::string("VIS006");
			case GP_SC_CHAN_ID::e_VIS_0_8    : return std::string("VIS008");
			case GP_SC_CHAN_ID::e_IR_1_6     : return std::string("IR_016");
			case GP_SC_CHAN_ID::e_IR_3_9     : return std::string("IR_039");
			case GP_SC_CHAN_ID::e_WV_6_2     : return std::string("WV_062");
			case GP_SC_CHAN_ID::e_WV_7_3     : return std::string("WV_073");
			case GP_SC_CHAN_ID::e_IR_8_7     : return std::string("IR_087");
			case GP_SC_CHAN_ID::e_IR_9_7     : return std::string("IR_097");
			case GP_SC_CHAN_ID::e_IR_10_8    : return std::string("IR_108");
			case GP_SC_CHAN_ID::e_IR_12_0    : return std::string("IR_120");
			case GP_SC_CHAN_ID::e_IR_13_4    : return std::string("IR_134");
			case GP_SC_CHAN_ID::e_HRV        : return std::string("HRV___");
		}
		return std::string("");
	}

	// Description:	Loads identifier.
	// Returns:		Reference to object.
	CSpectralChannelID& operator=
	(
		unsigned char i_ID	// GP_SC_CHAN_ID-compatible integer value.
	)
	{
		m_SC_CHAN_ID = E_SC_CHAN_ID(i_ID);
		return *this;
	}

	// Description:	Returns a GP_SC_CHAN_ID-compatible integer value.
	// Returns:		A GP_SC_CHAN_ID-compatible integer value..
	operator unsigned char
	(
	)
	const
	{
		return m_SC_CHAN_ID;
	}

};




} // end namespace


#endif
