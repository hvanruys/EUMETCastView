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

#ifndef CxRITFile_included
#define CxRITFile_included

/*******************************************************************************

TYPE:
Concrete Classes.
					
PURPOSE:
Class definition for LRIT/HRIT files.

FUNCTION:
The classes provided below allow to access and manipulate the contents of
LRIT/HRIT files.

INTERFACES:
Objects of the CxRITFile class contain exactly one LRIT/HRIT file.
See also 'INTERFACES:' in the module declaration below.

RESOURCES:	
Heap Memory (>2K).

REFERENCES:
MSG LRIT/HRIT Global Specififation (CGMS03).
MSG LRIT/HRIT Mission Specific Implementation (EUM/MSG/SPE/057).

PROCESSING:
Member functions of the CxRITFile class:
- CxRITFile(): Constructors are provided for
  - assembling a file from given LRIT/HRIT header record members and a data field,
    and for
  - reading a file from disk,
- Write() stores the LRIT/HRIT file in a specified disk directory.

DATA:
See 'DATA:' in the class headers below.

LOGIC:		
-

*******************************************************************************/

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <deque>

#include "CDataField.h"				// Util
#include "ErrorHandling.h"			// Util
#include "CxRITFileHeaderRecords.h"	// DISE




namespace DISE
{




class CxRITFile	: public DISE::CxRITFileHeaderRecords
{

protected:

// DATA:

	// Data field for all kinds of data.
	Util::CDataField					m_DataField;

	// Used to serialise read and write operations of HRIT/LRIT files.
	//static Util::CSmartCriticalSection	ms_LockFileIO;

public:

// INTERFACES:

	// Description:	Copy constructor.
	// Returns:		Nothing.
	CxRITFile
	(
		const CxRITFile& i_Reference
	)
	: CxRITFileHeaderRecords(i_Reference			)
	, m_DataField			(i_Reference.m_DataField)
	{
	}

	// Description:	Constructor.
	//				Used for LRIT/HRIT files containing non-image type data.
	// Returns:		Nothing.
	CxRITFile
	(
		Util::CDataField&				i_DataField,
		const EFileTypeCode				i_FileTypeCode,
		const DISE::CxRITAnnotation&	i_Annotation,
		const std::string&				i_DataDefinitionBlock	= "",
		const std::string&				i_AncillaryText			= "",
		const DISE::CSpacecraftID		i_SpacecraftID			= DISE::CSpacecraftID(0),
		const DISE::CSpectralChannelID	i_SpectralChannelID		= DISE::CSpectralChannelID(0),
		const unsigned short			i_SegmentSeqNo			= 1,
		const unsigned short			i_PlannedStartSegmentNo	= 1,
		const unsigned short			i_PlannedEndSegmentNo	= 1,
		const unsigned char*			i_pKeyNumber			= NULL,
		const unsigned __int64*			i_pSeed					= NULL
	);

	// Description:	Constructor.
	//				Used for LRIT/HRIT files containing compressed or non-compressed
	//				image type data.
	// Returns:		Nothing.
	CxRITFile
	(
		Util::CDataFieldCompressedImage&	i_DataField,
		const ECompressionFlag				i_CompressionFlag,
		const EDataFieldRepresentation		i_DataFieldRepresentation,
		const std::deque<SLineQualityEntry>& i_LineQualityEntries,
		const DISE::CxRITAnnotation&		i_Annotation,
		const std::string&					i_ProjectionName,
		const long							i_ColumnScalingFactor,
		const long							i_LineScalingFactor,
		const long							i_ColumnOffset,
		const long							i_LineOffset,
		const std::string&					i_DataDefinitionBlock	= "",
		const std::string&					i_AncillaryText			= "",
		const DISE::CSpacecraftID&			i_SpacecraftID			= DISE::CSpacecraftID(0),
		const DISE::CSpectralChannelID&		i_SpectralChannelID		= DISE::CSpectralChannelID(0),
		const unsigned short				i_SegmentSeqNo			= 1,
		const unsigned short				i_PlannedStartSegmentNo	= 1,
		const unsigned short				i_PlannedEndSegmentNo	= 1,
		const unsigned char*				i_pKeyNumber			= NULL,
		const unsigned __int64*						i_pSeed					= NULL
	);

	// Description:	Constructor. Loads LRIT/HRIT file from a stream.
	// Returns:		Nothing.
	CxRITFile
	(
		std::istream&		i_Stream	// Input stream.
	);

	// Description:	Constructor. Loads LRIT/HRIT file.
	// Returns:		Nothing.
	CxRITFile
	(
		const std::string&	i_FileName	// File name.
	);

	// Description:	Default constructor.
	//				Only to be used when objects are received from a network stream.
	// Returns:		Nothing.
	CxRITFile
	(
	);

	// Description:	Destructor.
	// Returns:		Nothing.
	virtual ~CxRITFile
	(
	)
	{
	}

	// Description:	Writes header records of LRIT/HRIT file to an output stream.
	// Returns:		Output stream
	std::ostream& WriteHeaderRecords
	(
		std::ostream&		i_Stream		// Output stream.
	)
	const;

	// Description:	Writes LRIT/HRIT file to an output stream.
	// Returns:		Output stream
	std::ostream& Write
	(
		std::ostream&		i_Stream		// Output stream.
	)
	const;

	// Description:	Writes LRIT/HRIT file to the specified file.
	//				To reduce disk fragmentation, the write operation is thread safe,
	//				i.e., only one file can be written at a time.
	// Returns:		Nothing.
	void Write
	(
		const std::string&	i_FileName	// Output file path and name.
	)
	const;

	// Description:	Provide access to data field.
	// Returns:		Reference to member variable.
	Util::CDataField& GetDataField()
	{
		return m_DataField;
	}

	const Util::CDataField& GetDataField() const
	{
		return m_DataField;
	}

	// Description:	Produces a bit pattern in which
	//				0 bits indicate equivalence of the corresponding bits,
	//				1 bits indicate a difference of the corresponding bits
	//				in current and reference file.
	// Returns:		false if files are equal, i.e., the difference pattern has only 0 bits.
	//				true  if files are different.
	bool Subtract
	(
		Util::CDataField&		o_Difference,	// Difference bit pattern.
		const DISE::CxRITFile&	i_Reference		// Reference file.
	)
	const;

};




} // end namespace


#endif
