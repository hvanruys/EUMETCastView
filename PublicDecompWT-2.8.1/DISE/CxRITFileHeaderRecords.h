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

#ifndef CxRITFileHeaderRecords_included
#define CxRITFileHeaderRecords_included

/*******************************************************************************

TYPE:
Concrete Classes.
					
PURPOSE:
Class definition for LRIT/HRIT files.

FUNCTION:
The classes provided below allow to access contents of the LRIT/HRIT file header
records.

INTERFACES:
Objects of the CxRITFileHeaderRecords class contain the header record parameters of
exactly one LRIT/HRIT file.
See also 'INTERFACES:' in the module declaration below.

RESOURCES:	
Heap Memory (>2K).

REFERENCES:
MSG LRIT/HRIT Global Specififation (CGMS03).
MSG LRIT/HRIT Mission Specific Implementation (EUM/MSG/SPE/057).

PROCESSING:
Member functions of the CxRITFileHeaderRecords class:
- CxRITFileHeaderRecords(): Constructors are provided for
  - assembling a file from given LRIT/HRIT header record members, and
  - for reading a file from disk,
- Write() writes the LRIT/HRIT file header records to a specified output stream.

DATA:
See 'DATA:' in the class headers below.

LOGIC:		
-

*******************************************************************************/

#pragma warning(disable: 4786)


#include <iostream>
#include <fstream>
#include <memory>
#include <string.h>
#include <deque>
#include <vector>

#include "GSDS_Volume_F_NBO.h"		// Util




#include "CxRITAnnotation.h"		// DISE
#include "CSpacecraftID.h"			// DISE
#include "CSpectralChannelID.h"		// DISE




namespace DISE
{


// LRIT/HRIT Primary Header Record.
#pragma pack(push, 1)
struct SPrimaryHeaderRecord
{
	NBO::UNSIGNED_BYTE		m_HeaderType;
	NBO::UNSIGNED_SHORT		m_HeaderRecordLength;
	NBO::ENUMERATED_BYTE	m_FileTypeCode;
	NBO::UNSIGNED			m_TotalHeaderLength;
	NBO::UNSIGNED_DOUBLE	m_DataFieldLength;
};
#pragma pack(pop)




// Encryption Key Message as contained in LRIT/HRIT Encryption Key Message Files.
#pragma pack(push, 1)
struct SEncryptionKeyMessage
{
	NBO::UNSIGNED_SHORT  m_UserStationNumber;
	NBO::UNSIGNED_BYTE   m_KeyNumber;
	NBO::UNSIGNED_DOUBLE m_PublicKey[3];
	NBO::UNSIGNED_SHORT  m_PublicKeyCRC;
};
#pragma pack(pop)




// Line Quality Entry structure.
#pragma pack(push, 1)
struct SLineQualityEntry
{

	enum ELineQuality		// Line Validity values and
							// Line Quality values.
	{
		e_NotDerived = 0,	// Not derived.
		e_Nominal    = 1,	// Nominal.
		e_Usable     = 2,	// Usable.
		e_Suspect    = 3,	// Suspect.
		e_DoNotUse   = 4	// Do not use.
	};

// DATA:

	NBO::INTEGER         m_LineNumberInGrid;		// Line number in grid.
	NBO::TIME_CDS_SHORT  m_LineMeanAcquisitionTime;	// Line mean acquisition time.
	NBO::ENUMERATED_BYTE m_LineValidity;			// Line validity.
	NBO::ENUMERATED_BYTE m_LineRadiometricQuality;	// Line radiometric quality.
	NBO::ENUMERATED_BYTE m_LineGeometricQuality;	// Line geometric quality.

// INTERFACES:

	// Description:	Constructor.
	// Returns:		Nothing.
	SLineQualityEntry
	(
		const long				i_LineNumberInGrid			= 0,
#ifdef WIN32
		const SYSTIME 	i_LineMeanAcquisitionTime = SYSTIME(1958, 1, 1, 0, 0, 0, 0),
#else
		const SYSTIME	i_LineMeanAcquisitionTime = SYSTIME(0, 0, 0, 1, 1, 1958),
#endif		
		const unsigned char		i_LineValidity				= e_NotDerived,
		const unsigned char		i_LineRadiometricQuality	= e_NotDerived,
		const unsigned char		i_LineGeometricQuality		= e_NotDerived
	)
	: m_LineNumberInGrid		(i_LineNumberInGrid							)
	, m_LineMeanAcquisitionTime	(i_LineMeanAcquisitionTime					)
	, m_LineValidity			((	i_LineValidity <= e_DoNotUse)
								 ?	i_LineValidity :  e_NotDerived			)
	, m_LineRadiometricQuality	((	i_LineRadiometricQuality <= e_DoNotUse)
								 ?	i_LineRadiometricQuality :  e_NotDerived)
	, m_LineGeometricQuality	((	i_LineGeometricQuality <= e_DoNotUse)
								 ?	i_LineGeometricQuality :  e_NotDerived	)
	{
	}

	// Description:	Copy constructor.
	//				Guarantees that member values of new object are within allowed range.
	// Returns:		Nothing.
	SLineQualityEntry
	(
		const SLineQualityEntry&	i_Reference
	)
	: m_LineNumberInGrid		(i_Reference.m_LineNumberInGrid							)
	, m_LineMeanAcquisitionTime	(i_Reference.m_LineMeanAcquisitionTime					)
	, m_LineValidity			((	i_Reference.m_LineValidity <= e_DoNotUse)
								 ?	i_Reference.m_LineValidity :  e_NotDerived			)
	, m_LineRadiometricQuality	((	i_Reference.m_LineRadiometricQuality <= e_DoNotUse)
								 ?	i_Reference.m_LineRadiometricQuality :  e_NotDerived)
	, m_LineGeometricQuality	((	i_Reference.m_LineGeometricQuality <= e_DoNotUse)
								 ?	i_Reference.m_LineGeometricQuality :  e_NotDerived	)
	{
	}

};
#pragma pack(pop)


// Description:	Overloaded stream input operator for Line Quality Entry structures.
// Returns:		Input stream.
std::istream& operator>>
(
	std::istream&					i_Stream,	// Input stream.
	DISE::SLineQualityEntry&		i_This		// Contents of Line Quality Entry structure.
);


// Description:	Overloaded stream output operator for Line Quality Entry structures.
// Returns:		Output stream.
std::ostream& operator<<
(
	std::ostream&					i_Stream,	// Output stream.
	const DISE::SLineQualityEntry&	i_This		// Contents of Line Quality Entry structure.
);




// Used as default parameter value.
static const std::vector<DISE::SLineQualityEntry> gs_NoLineQualityEntries;




// File type codes as used in Primary Header Record.
enum EFileTypeCode
{
	e_NotSpecified			=  -1,	// File type not known.
	e_ImageDataFile			=   0,	// Image data file.
	e_GTSMessage			=   1,	// GTS messages file.
	e_AlphanumericTextFile	=   2,	// Alphanumeric text file.
	e_EncryptionKeyMessage	=   3,	// Encryption key message File.
	e_RepeatCyclePrologue	= 128,	// Repeat cycle prologue file.
	e_RepeatCycleEpilogue	= 129,	// Repeat cycle epilogue file.
	e_DCPMessage			= 130	// DCP messages file.
};




class CxRITFileHeaderRecords
{

public:

	enum ECompressionFlag					// Compression flag values:
	{
		e_None     = 0,						// No compression.
		e_Lossless = 1,						// Lossless compression.
		e_Lossy    = 2						// Lossy compression.
	};

	enum EDataFieldRepresentation			// Data field representation types:
	{
		e_NoSpecific = 0,					// No specific formatting.
		e_JPEG       = 1,					// JPEG interchange format (image-type data only).
		e_T4         = 2,					// T.4 coded file format (image-type data only).
		e_WT         = 3//TBC				// Wavelet-based format (image-type data only).
	};

protected:

	enum EHeaderType			// Values of header type identifiers.
	{
		e_PrimaryHeader           =   0,
		e_ImageStructure          =   1,
		e_ImageNavigation         =   2,
		e_ImageDataFunction       =   3,
		e_Annotation              =   4,
		e_TimeStamp               =   5,
		e_AncillaryText           =   6,
		e_KeyHeader               =   7,
		e_SegmentIdentification   = 128,
		e_ImageSegmentLineQuality = 129
	};

// DATA:

	// Parameters for Primary Header Record.
	EFileTypeCode		m_FileTypeCode;			// File type code.
	unsigned __int64	m_DataFieldLength;		// Data field length [bits].

	// Parameters for Image Structure Record.
	unsigned char		m_NB;					// Number of bits per pixel.
	unsigned short		m_NC;					// Number of image columns.
	unsigned short		m_NL;					// Number of image lines.
	ECompressionFlag	m_CompressionFlag;		// Compression flag.

	// Parameters for Image Navigation Record.
	enum { e_SizeofProjectionName = 32 };		// Projection name string length.
	std::string			m_ProjectionName;		// Projection name.
	long				m_CFAC;					// Column scaling factor.
	long				m_LFAC;					// Line scaling factor.
	long				m_COFF;					// Column offset.
	long				m_LOFF;					// Line offset.

	// Parameters for Image Data Function Record.
	std::string			m_DataDefinitionBlock;	// Data definition block.

	// Parameters for Annotation Record.
	DISE::CxRITAnnotation	m_Annotation;		// Annotation text.

	// Parameters for Time Stamp Record.
	SYSTIME		m_TimeStamp;			// Time stamp.

	// Parameters for Ancillary Text Record.
	std::string			m_AncillaryText;		// Ancillary text.

	// Parameters for Key Header Record.
	// (Refer to m_Annotation for an encryption flag.)
	unsigned char		m_KeyNumber;			// Encryption key number.
	unsigned __int64	m_Seed;					// Seed.

	// Parameters for Segment Identification Record.
	DISE::CSpacecraftID      m_SpacecraftID;			// Spacecraft identifier.
	DISE::CSpectralChannelID m_SpectralChannelID;		// Spectral channel identifier.
	unsigned short			 m_SegmentSeqNo;			// Segment sequence number.
	unsigned short			 m_PlannedStartSegmentNo;	// Number of first segment
														// planned to be disseminated.
	unsigned short			 m_PlannedEndSegmentNo;		// Number of last segment
														// planned to be disseminated.
	EDataFieldRepresentation m_DataFieldRepresentation;	// Data field representation type.

	// Parameters for Image Segment Line Quality Record.
	std::deque<DISE::SLineQualityEntry>
	                         m_LineQualityEntries;		// Line quality entries.

// INTERFACES:

	// One item in the sequence of header records.
	struct SHeaderRecordSequenceItem
	{

	// DATA:

		EHeaderType		m_HeaderType;	// Type of Header Record.
		unsigned long	m_HeaderLength;	// Length of Header Record [bytes].

	// INTERFACES:

		// Description:	Constructor.
		// Returns:		Nothing.
		SHeaderRecordSequenceItem
		(
			const EHeaderType	i_HeaderType   = e_PrimaryHeader,
			const unsigned long	i_HeaderLength = 0
		)
		: m_HeaderType	(i_HeaderType	)
		, m_HeaderLength(i_HeaderLength	)
		{
		}

	};

	// Description:	Determines the sequence of Header Records in the HRIT/LRIT file.
	// Returns:		The sequence of header records.
	void GetHeaderRecordSequence
	(
		std::vector<SHeaderRecordSequenceItem>& o_HeaderRecordSequence
	)
	const;

	// Description:	Reads LRIT/HRIT Header Records from an input stream.
	// Returns:		Input stream.
	virtual std::istream& Read
	(
		std::istream&					// Input stream.
	);

public:

// INTERFACES:

	// Description:	Writes LRIT/HRIT Header Records to an output stream.
	// Returns:		Output stream.
	virtual std::ostream& Write
	(
		std::ostream&					// Output stream.
	)
	const;

	// Description:	Constructor.
	//				Used for LRIT/HRIT files containing non-image type data.
	// Returns:		Nothing.
	CxRITFileHeaderRecords
	(
		const EFileTypeCode				i_FileTypeCode,
		unsigned __int64				i_DataFieldLength,
		const std::string&				i_DataDefinitionBlock,
		const DISE::CxRITAnnotation&	i_Annotation,
		const SYSTIME&			i_TimeStamp,
		const std::string&				i_AncillaryText			= "",
		const DISE::CSpacecraftID		i_SpacecraftID			= DISE::CSpacecraftID(0),
		const DISE::CSpectralChannelID	i_SpectralChannelID		= DISE::CSpectralChannelID(0),
		const unsigned short			i_SegmentSeqNo			= 1,
		const unsigned short			i_PlannedStartSegmentNo	= 1,
		const unsigned short			i_PlannedEndSegmentNo	= 1,
		const unsigned char*			i_pKeyNumber			= NULL,
		const unsigned __int64*					i_pSeed					= NULL
	)
	: m_FileTypeCode			(i_FileTypeCode									)
	, m_DataFieldLength			(i_DataFieldLength								)
	, m_NB						(0												)
	, m_NC						(0												)
	, m_NL						(0												)
	, m_CompressionFlag			(e_None											)
	, m_ProjectionName			(std::string("")								)
	, m_CFAC					(0												)
	, m_LFAC					(0												)
	, m_COFF					(0												)
	, m_LOFF					(0												)
	, m_DataDefinitionBlock		(i_DataDefinitionBlock							)
	, m_Annotation				(i_Annotation									)
	, m_TimeStamp				(i_TimeStamp									)
	, m_AncillaryText			(i_AncillaryText								)
	, m_KeyNumber				((i_pKeyNumber && i_pSeed) ? *i_pKeyNumber : 0	)
	, m_Seed					((i_pKeyNumber && i_pSeed) ? *i_pSeed      : 0	)
	, m_SpacecraftID			(i_SpacecraftID									)
	, m_SpectralChannelID		(i_SpectralChannelID							)
	, m_SegmentSeqNo			(i_SegmentSeqNo									)
	, m_PlannedStartSegmentNo	(i_PlannedStartSegmentNo						)
	, m_PlannedEndSegmentNo		(i_PlannedEndSegmentNo							)
	, m_DataFieldRepresentation	(e_NoSpecific									)
	{
		// Some adjustments.
		m_ProjectionName.resize(e_SizeofProjectionName, ' ');
		m_Annotation.SetEncrypted(i_pKeyNumber && i_pSeed);
	}

	// Description:	Constructor.
	//				Used for LRIT/HRIT files containing image type data.
	// Returns:		Nothing.
	CxRITFileHeaderRecords
	(
		const unsigned __int64			i_DataFieldLength,
		unsigned char					i_NB,
		unsigned short					i_NC,
		unsigned short					i_NL,
		const ECompressionFlag&			i_CompressionFlag,
		const std::string&				i_ProjectionName,
		const long						i_ColumnScalingFactor,
		const long						i_LineScalingFactor,
		const long						i_ColumnOffset,
		const long						i_LineOffset,
		const std::string&				i_DataDefinitionBlock,
		const DISE::CxRITAnnotation&	i_Annotation,
		const SYSTIME&			i_TimeStamp,// = Util::CMSGTime::GetTheCurrentTime(),
		const std::string&				i_AncillaryText				= "",
		const unsigned char*			i_pKeyNumber				= NULL,
		const unsigned __int64*					i_pSeed						= NULL,
		const DISE::CSpacecraftID&		i_SpacecraftID				= DISE::CSpacecraftID(0),
		const DISE::CSpectralChannelID&	i_SpectralChannelID			= DISE::CSpectralChannelID(0),
		const unsigned short			i_SegmentSeqNo				= 1,
		const unsigned short			i_PlannedStartSegmentNo		= 1,
		const unsigned short			i_PlannedEndSegmentNo		= 1,
		const EDataFieldRepresentation& i_DataFieldRepresentation	= e_NoSpecific,
		const std::deque<SLineQualityEntry>& i_LineQualityEntries	= std::deque<SLineQualityEntry>()
	)
	: m_FileTypeCode			(e_ImageDataFile								)
	, m_DataFieldLength			(i_DataFieldLength								)
	, m_NB						(i_NB											)
	, m_NC						(i_NC											)
	, m_NL						(i_NL											)
	, m_CompressionFlag			(i_CompressionFlag								)
	, m_ProjectionName			(i_ProjectionName								)
	, m_CFAC					(i_ColumnScalingFactor							)
	, m_LFAC					(i_LineScalingFactor							)
	, m_COFF					(i_ColumnOffset									)
	, m_LOFF					(i_LineOffset									)
	, m_DataDefinitionBlock		(i_DataDefinitionBlock							)
	, m_Annotation				(i_Annotation									)
	, m_TimeStamp				(i_TimeStamp									)
	, m_AncillaryText			(i_AncillaryText								)
	, m_KeyNumber				((i_pKeyNumber && i_pSeed) ? *i_pKeyNumber : 0	)
	, m_Seed					((i_pKeyNumber && i_pSeed) ? *i_pSeed      : 0	)
	, m_SpacecraftID			(i_SpacecraftID									)
	, m_SpectralChannelID		(i_SpectralChannelID							)
	, m_SegmentSeqNo			(i_SegmentSeqNo									)
	, m_PlannedStartSegmentNo	(i_PlannedStartSegmentNo						)
	, m_PlannedEndSegmentNo		(i_PlannedEndSegmentNo							)
	, m_DataFieldRepresentation	(i_DataFieldRepresentation						)
	, m_LineQualityEntries		(i_LineQualityEntries							)
	{
		// Some adjustments.
		m_ProjectionName.resize(e_SizeofProjectionName, ' ');
		m_Annotation.SetCompressed(i_CompressionFlag != e_None);
		m_Annotation.SetEncrypted(i_pKeyNumber && i_pSeed);
	}

	// Description:	Default constructor.
	//				Only to be used when objects are received from a network stream.
	// Returns:		Nothing.
	CxRITFileHeaderRecords
	(
		const DISE::CxRITFileHeaderRecords&	i_Reference	// Reference object.
	)
	: m_FileTypeCode			(i_Reference.GetFileTypeCode()				)
	, m_DataFieldLength			(i_Reference.GetDataFieldLength()			)
	, m_NB						(i_Reference.GetNB()						)
	, m_NC						(i_Reference.GetNC()						)
	, m_NL						(i_Reference.GetNL()						)
	, m_CompressionFlag			(i_Reference.GetCompressionFlag()			)
	, m_ProjectionName			(i_Reference.GetProjectionName()			)
	, m_CFAC					(i_Reference.GetCFAC()						)
	, m_LFAC					(i_Reference.GetLFAC()						)
	, m_COFF					(i_Reference.GetCOFF()						)
	, m_LOFF					(i_Reference.GetLOFF()						)
	, m_DataDefinitionBlock		(i_Reference.GetDataDefinitionBlock()		)
	, m_Annotation				(i_Reference.GetAnnotation()				)
	, m_TimeStamp				(i_Reference.GetTimeStamp()					)
	, m_AncillaryText			(i_Reference.GetAncillaryText()				)
	, m_KeyNumber				(i_Reference.GetKeyNumber()					)
	, m_Seed					(i_Reference.GetSeed()						)
	, m_SpacecraftID			(i_Reference.GetSpacecraftID()				)
	, m_SpectralChannelID		(i_Reference.GetSpectralChannelID()			)
	, m_SegmentSeqNo			(i_Reference.GetSegmentSeqNo()				)
	, m_PlannedStartSegmentNo	(i_Reference.GetPlannedStartSegmentNo()		)
	, m_PlannedEndSegmentNo		(i_Reference.GetPlannedEndSegmentNo()		)
	, m_DataFieldRepresentation	(i_Reference.GetDataFieldRepresentation()	)
	, m_LineQualityEntries		(i_Reference.GetLineQualityEntries()		)
	{
	}

	// Description:	Default constructor.
	//				Only to be used when objects are received from a network stream.
	// Returns:		Nothing.
	CxRITFileHeaderRecords
	(
	)
	: m_FileTypeCode			(e_ImageDataFile)
	, m_DataFieldLength			(0				)
	, m_NB						(0				)
	, m_NC						(0				)
	, m_NL						(0				)
	, m_CompressionFlag			(e_None			)
	, m_CFAC					(0				)
	, m_LFAC					(0				)
	, m_COFF					(0				)
	, m_LOFF					(0				)
	, m_KeyNumber				(0				)
	, m_Seed					(0				)
	, m_SegmentSeqNo			(1				)
	, m_PlannedStartSegmentNo	(1				)
	, m_PlannedEndSegmentNo		(1				)
	, m_DataFieldRepresentation	(e_NoSpecific	)
	{
		// Some adjustments.
		m_ProjectionName.resize(e_SizeofProjectionName, ' ');
	}

	// Description:	Constructor. Loads LRIT/HRIT file from a stream.
	// Returns:		Nothing.
	explicit CxRITFileHeaderRecords
	(
		std::istream&	i_Stream	// Input stream.
	)
	{
		try
		{
			Read(i_Stream);
			PRECONDITION(i_Stream.fail() == false);
		}
		catch(...)
		{
			LOGCATCHANDTHROW;
		}
	}

	// Description:	Constructor. Loads LRIT/HRIT file.
	// Returns:		Nothing.
	explicit CxRITFileHeaderRecords
	(
		const std::string& i_FileName	// File name.
	)
	{
		try
		{
			// Open input file.
			std::ifstream inputStream(	i_FileName.c_str(),std::ios_base::binary | std::ios_base::in);
			Assert(inputStream.good(), Util::CCLibException());

			// Read from input file.
			*this = CxRITFileHeaderRecords(inputStream);
		}
		catch (...)
		{
		    LOGCATCHANDTHROW
		}
	}

	// Description:	Destructor.
	// Returns:		Nothing.
	virtual ~CxRITFileHeaderRecords
	(
	)
	{
	}

	// Description:	Provide read-only access to member variables.
	// Returns:		Const. reference to member variable.
	EFileTypeCode			GetFileTypeCode()				const { return m_FileTypeCode;			}
	unsigned __int64		GetDataFieldLength()			const { return m_DataFieldLength;		}
	unsigned char			GetNB()							const { return m_NB;					}
	unsigned short			GetNC()							const { return m_NC;					}
	unsigned short			GetNL()							const { return m_NL;					}
	const ECompressionFlag&	GetCompressionFlag()			const { return m_CompressionFlag;		}
	const std::string&		GetProjectionName()				const { return m_ProjectionName;		}
	long					GetCFAC()						const { return m_CFAC;					}
	long					GetLFAC()						const { return m_LFAC;					}
	long					GetCOFF()						const { return m_COFF;					}
	long					GetLOFF()						const { return m_LOFF;					}
	const std::string&		GetDataDefinitionBlock()		const { return m_DataDefinitionBlock;	}
	const DISE::CxRITAnnotation&	GetAnnotation()			const { return m_Annotation;			}
	const SYSTIME&	GetTimeStamp()					const { return m_TimeStamp;				}
	const std::string&		GetAncillaryText()				const { return m_AncillaryText;			}
	unsigned char			GetKeyNumber()					const { return m_KeyNumber;				}
	unsigned __int64		GetSeed()						const { return m_Seed;					}
	const DISE::CSpacecraftID&		GetSpacecraftID()		const { return m_SpacecraftID;			}
	const DISE::CSpectralChannelID&	GetSpectralChannelID()	const { return m_SpectralChannelID;		}
	unsigned short			GetSegmentSeqNo()				const { return m_SegmentSeqNo;			}
	unsigned short			GetPlannedStartSegmentNo()		const { return m_PlannedStartSegmentNo;	}
	unsigned short			GetPlannedEndSegmentNo()		const { return m_PlannedEndSegmentNo;	}
	EDataFieldRepresentation GetDataFieldRepresentation()	const { return m_DataFieldRepresentation; }
	const std::deque<DISE::SLineQualityEntry>&GetLineQualityEntries()
															const { return m_LineQualityEntries;	}
	      std::deque<DISE::SLineQualityEntry>&GetLineQualityEntries()
															      { return m_LineQualityEntries;	}

	// Description:	Returns the accumulated length [bits] of all header records.
	// Returns:		The accumulated length [bits] of all header records.
	unsigned long GetTotalHeaderLength
	(
	)
	const
	{
		try
		{
			std::vector<SHeaderRecordSequenceItem> headerRecordSequence;
			GetHeaderRecordSequence(headerRecordSequence);

			unsigned long length = 0;
			for (unsigned int i = 0; i < headerRecordSequence.size(); ++i)
				length += headerRecordSequence[i].m_HeaderLength;

			return length * 8;
		}
		catch (...)
		{
			LOGCATCHANDTHROW;
		}
	}

	// Description:	Returns the total length [bits] of the LRIT/HRIT file.
	// Returns:		The total length [bits] of the file.
	unsigned __int64 GetTotalLength
	(
	)
	const
	{
		return GetTotalHeaderLength() + m_DataFieldLength;
	}

	// Description:	Writes the header record contents into a text string
	//				for display and trace purposes.
	// Returns:		Header record contents as a text string.
	std::string GetDisplayString
	(
		const bool i_CRFlag = false	// true : End of text line indicated by CR+LF.
									// false: End of text line indicated by LF.
	)
	const;
};




} // end namespace


#endif
