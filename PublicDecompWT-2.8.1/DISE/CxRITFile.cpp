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

#pragma warning(disable: 4275 4786)

#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "CxRITFile.h"						// DISE


namespace DISE
{




//Util::CSmartCriticalSection CxRITFile::ms_LockFileIO;




CxRITFile::CxRITFile
(
	Util::CDataField&				i_DataField,
	const EFileTypeCode				i_FileTypeCode,
	const DISE::CxRITAnnotation&	i_Annotation,
	const std::string&				i_DataDefinitionBlock,
	const std::string&				i_AncillaryText,
	const DISE::CSpacecraftID		i_SpacecraftID,
	const DISE::CSpectralChannelID	i_SpectralChannelID,
	const unsigned short			i_SegmentSeqNo,
	const unsigned short			i_PlannedStartSegmentNo,
	const unsigned short			i_PlannedEndSegmentNo,
	const unsigned char*			i_pKeyNumber,
	const unsigned __int64*			i_pSeed
)
: CxRITFileHeaderRecords(i_FileTypeCode,
						 i_DataField.Size(),
						 i_DataDefinitionBlock,
						 i_Annotation,
#ifdef WIN32
						 Util::CMSGTime::GetTheCurrentTime(),
#else
						 Util::CUTCTime::Now(),
#endif
						 i_AncillaryText,
						 i_SpacecraftID,
						 i_SpectralChannelID,
						 i_SegmentSeqNo,
						 i_PlannedStartSegmentNo,
						 i_PlannedEndSegmentNo,
						 i_pKeyNumber,
						 i_pSeed							)
, m_DataField			(i_DataField						)
{
}




CxRITFile::CxRITFile
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
	const std::string&					i_DataDefinitionBlock,
	const std::string&					i_AncillaryText,
	const DISE::CSpacecraftID&			i_SpacecraftID,
	const DISE::CSpectralChannelID&		i_SpectralChannelID,
	const unsigned short				i_SegmentSeqNo,
	const unsigned short				i_PlannedStartSegmentNo,
	const unsigned short				i_PlannedEndSegmentNo,
	const unsigned char*				i_pKeyNumber,
	const unsigned __int64*				i_pSeed
)
: CxRITFileHeaderRecords(i_DataField.Size(),
						 i_DataField.GetNB(),
						 i_DataField.GetNC(),
						 i_DataField.GetNL(),
						 i_CompressionFlag,
						 i_ProjectionName,
						 i_ColumnScalingFactor,
						 i_LineScalingFactor,
						 i_ColumnOffset,
						 i_LineOffset,
						 i_DataDefinitionBlock,
						 i_Annotation,
#ifdef WIN32
						 Util::CMSGTime::GetTheCurrentTime(),
#else
						 Util::CUTCTime::Now(),
#endif
						 i_AncillaryText,
						 i_pKeyNumber,
						 i_pSeed,
						 i_SpacecraftID,
						 i_SpectralChannelID,
						 i_SegmentSeqNo,
						 i_PlannedStartSegmentNo,
						 i_PlannedEndSegmentNo,
						 i_DataFieldRepresentation,
						 i_LineQualityEntries				)
, m_DataField			(i_DataField						)
{
}




CxRITFile::CxRITFile
(
	std::istream& i_Stream
)
{
	try
	{
		*this = DISE::CxRITFile();
		CxRITFileHeaderRecords::Read(i_Stream);
		m_DataField.Resize(m_DataFieldLength);
		i_Stream >> m_DataField;
		PRECONDITION(i_Stream.fail() == false);
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




CxRITFile::CxRITFile
(
	const std::string&	i_FileName
)
{
	try
	{
		//Util::CLockCriticalSection lock(ms_LockFileIO);

		std::ifstream input(i_FileName.c_str(),	std::ios_base::binary |
												std::ios_base::in     );
		Assert(input.good(), Util::CCLibException());

		*this = CxRITFile(input);
		Assert(input.fail() == false,Util::CCLibException());
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




CxRITFile::CxRITFile
(
)
: CxRITFileHeaderRecords()
{
}




std::ostream& CxRITFile::WriteHeaderRecords
(
	std::ostream&	i_Stream
)
const
{
	try
	{
		CxRITFileHeaderRecords::Write(i_Stream);
		AssertCLib(i_Stream.good());
		return i_Stream;
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




std::ostream& CxRITFile::Write
(
	std::ostream&	i_Stream
)
const
{
	try
	{
		CxRITFileHeaderRecords::Write(i_Stream);
		i_Stream << m_DataField;
		AssertCLib(i_Stream.good());
		return i_Stream;
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




void CxRITFile::Write
(
	const std::string&	i_FileName
)
const
{
	try
	{
		//Util::CLockCriticalSection lock(ms_LockFileIO);

		std::ofstream output(i_FileName.c_str(), std::ios_base::binary |
												 std::ios_base::out     );
		Assert(output.good(),Util::CCLibException());

		this->Write(output);
		Assert(output.good(),Util::CCLibException());
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




bool CxRITFile::Subtract
(
	Util::CDataField&		o_Difference,
	const DISE::CxRITFile&	i_Reference
)
const
{
	try
	{
		bool different = false;

		// Create binary version of header records from current file.
		std::string thisHeaders;
		{
			std::ostringstream temp(std::ios_base::binary |
									std::ios_base::out     );
			WriteHeaderRecords(temp);
			thisHeaders = temp.str();
		}

		// Create binary version of header records from reference file.
		std::string referenceHeaders;
		{
			std::ostringstream temp(std::ios_base::binary |
									std::ios_base::out     );
			i_Reference.WriteHeaderRecords(temp);
			referenceHeaders = temp.str();
		}

		// Check for a difference in the header records.
		if (thisHeaders.size() != referenceHeaders.size() ||
			memcmp(thisHeaders.c_str(), referenceHeaders.c_str(), thisHeaders.size()))
			different = true;

		// Check for a difference in the data field.
		if (this->GetDataField().Size() != i_Reference.GetDataField().Size() ||
			memcmp(	this->GetDataField().Data(),
					i_Reference.GetDataField().Data(),
					(unsigned int)(this->GetDataField().Size() / 8)))
			different = true;

		// Difference found: Perform binary file subtraction.
		if (different)
		{
			int thisLength		= thisHeaders.size()
								+ (int)((this->GetDataField().Size() + 7) / 8);
			int referenceLength = referenceHeaders.size()
								+ (int)((i_Reference.GetDataField().Size() + 7) / 8);

			o_Difference = Util::CDataField(thisLength * 8);
			if (referenceLength > thisLength)
				o_Difference.Resize(referenceLength * 8);

			o_Difference.Write(	0,
								(const unsigned char*)thisHeaders.c_str(),
								thisHeaders.size()									);

			o_Difference.Write(	thisHeaders.size(),
								GetDataField().Data(),
								(int)((GetDataField().Size() + 7) / 8)	);

			const unsigned char* from	= (const unsigned char*)referenceHeaders.c_str();
			const unsigned char* to		= (const unsigned char*)referenceHeaders.c_str()
										+ referenceHeaders.size();

			int i;
			for (i = 0; from < to; ++i)
				o_Difference.Data()[i] ^= *from++;

			from	= i_Reference.GetDataField().Data();
			to		= i_Reference.GetDataField().Data()
					+ (i_Reference.GetDataField().Size() + 7) / 8;
			for (         ; from < to; ++i)
				o_Difference.Data()[i] ^= *from++;
		}

		// Report comparison result.
		return different;
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}






} // end namespace
