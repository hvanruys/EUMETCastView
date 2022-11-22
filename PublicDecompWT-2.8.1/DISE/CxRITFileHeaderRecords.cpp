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

#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <assert.h>

#include "ErrorHandling.h"			// Util
#include "CxRITFileHeaderRecords.h"	// DISE




namespace DISE
{




std::istream& operator>>
(
	std::istream&            i_Stream,
	DISE::SLineQualityEntry& i_This
)
{
	try
	{
		i_Stream	>> i_This.m_LineNumberInGrid
					>> i_This.m_LineMeanAcquisitionTime
					>> i_This.m_LineValidity
					>> i_This.m_LineRadiometricQuality
					>> i_This.m_LineGeometricQuality;
		AssertCLib(i_Stream.good());
		return i_Stream;
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




std::ostream& operator<<
(
	std::ostream&                  i_Stream,
	const DISE::SLineQualityEntry& i_This
)
{
	try
	{
		i_Stream	<< i_This.m_LineNumberInGrid
					<< i_This.m_LineMeanAcquisitionTime
					<< i_This.m_LineValidity
					<< i_This.m_LineRadiometricQuality
					<< i_This.m_LineGeometricQuality;
		AssertCLib(i_Stream.good());
		return i_Stream;
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




void CxRITFileHeaderRecords::GetHeaderRecordSequence
(
	std::vector<SHeaderRecordSequenceItem>& o_HeaderRecordSequence
)
const
{
	try
	{
		o_HeaderRecordSequence.clear();

		unsigned long length;

		// Primary Header record.
		{
			length =	sizeof(NBO::UNSIGNED_BYTE  ) +	// Header_Type
						sizeof(NBO::UNSIGNED_SHORT ) +	// Header_Record_Length
						sizeof(NBO::ENUMERATED_BYTE) +	// File_Type_Code
						sizeof(NBO::UNSIGNED       ) +	// Total_Header_Length
						sizeof(NBO::UNSIGNED_DOUBLE)  ;	// Data_Field_Length
			o_HeaderRecordSequence.push_back(
				SHeaderRecordSequenceItem(e_PrimaryHeader, length));
		}

		// Image Structure Header record.
		if (m_FileTypeCode == e_ImageDataFile)
		{
			length =	sizeof(NBO::UNSIGNED_BYTE  ) +	// Header_Type
						sizeof(NBO::UNSIGNED_SHORT ) +	// Header_Record_Length
						sizeof(NBO::UNSIGNED_BYTE  ) +	// NB
						sizeof(NBO::UNSIGNED_SHORT ) +	// NC
						sizeof(NBO::UNSIGNED_SHORT ) +	// NL
						sizeof(NBO::ENUMERATED_BYTE)  ;	// Compression_Flag
			o_HeaderRecordSequence.push_back(
				SHeaderRecordSequenceItem(e_ImageStructure, length));
		}

		// Image Navigation Header record.
		if (m_FileTypeCode == e_ImageDataFile)
		{
			length =	sizeof(NBO::UNSIGNED_BYTE  ) +	// Header_Type
						sizeof(NBO::UNSIGNED_SHORT ) +	// Header_Record_Length
						e_SizeofProjectionName       +
						4 * sizeof(NBO::INTEGER)      ;
			o_HeaderRecordSequence.push_back(
				SHeaderRecordSequenceItem(e_ImageNavigation, length));
		}

		// Image Data Function Header record.
		if (m_DataDefinitionBlock.size() > 0)
		{
			length =	sizeof(NBO::UNSIGNED_BYTE )  +	// Header_Type
						sizeof(NBO::UNSIGNED_SHORT)  +	// Header_Record_Length
						m_DataDefinitionBlock.size()  ;
			o_HeaderRecordSequence.push_back(
				SHeaderRecordSequenceItem(e_ImageDataFunction, length));
		}

		// Annotation Header record.
		{
			length =	sizeof(NBO::UNSIGNED_BYTE )   +	// Header_Type
						sizeof(NBO::UNSIGNED_SHORT)   +	// Header_Record_Length
						m_Annotation.GetText().size()  ;
			o_HeaderRecordSequence.push_back(
				SHeaderRecordSequenceItem(e_Annotation, length));
		}

		// Time Stamp Header record.
		if (!(m_TimeStamp == SYSTIME()))
		{
			length =	sizeof(NBO::UNSIGNED_BYTE ) +	// Header_Type
						sizeof(NBO::UNSIGNED_SHORT) +	// Header_Record_Length
						sizeof(NBO::UNSIGNED_BYTE ) +	// CDS_P_Field
						sizeof(NBO::TIME_CDS_SHORT)  ;	// CDS_T_Field
			o_HeaderRecordSequence.push_back(
				SHeaderRecordSequenceItem(e_TimeStamp, length));
		}

		// Ancillary Text Header record.
		if (m_AncillaryText.size() > 0)
		{
			length =	sizeof(NBO::UNSIGNED_BYTE ) +	// Header_Type
						sizeof(NBO::UNSIGNED_SHORT) +	// Header_Record_Length
						m_AncillaryText.size()       ;
			o_HeaderRecordSequence.push_back(
				SHeaderRecordSequenceItem(e_AncillaryText, length));
		}

		// Encryption Key Header record.
		if (m_Annotation.GetEncryptedFlag())
		{
			length =	sizeof(NBO::UNSIGNED_BYTE  ) +	// Header_Type
						sizeof(NBO::UNSIGNED_SHORT ) +	// Header_Record_Length
						sizeof(NBO::UNSIGNED_BYTE  ) +	// Key_Number
						sizeof(NBO::UNSIGNED_DOUBLE)  ;	// Seed
			o_HeaderRecordSequence.push_back(
				SHeaderRecordSequenceItem(e_KeyHeader, length));
		}

		
		// Segment Identification Header record.
		if ((m_FileTypeCode == e_ImageDataFile) ||
		    ((m_FileTypeCode == e_GTSMessage) && (m_Annotation.GetProductID1().substr(0,4) == "MPEF")))
		{
			length =	sizeof(NBO::UNSIGNED_BYTE   ) +	// Header_Type
						sizeof(NBO::UNSIGNED_SHORT  ) +	// Header_Record_Length
						sizeof(NBO::ENUMERATED_SHORT) +	// Spacecraft_ID
						sizeof(NBO::ENUMERATED_BYTE ) +	// Spectral_Channel_ID
						sizeof(NBO::UNSIGNED_SHORT  ) +	// Segm_Seq_No
						sizeof(NBO::UNSIGNED_SHORT  ) +	// Planned_Start_Segment_No
						sizeof(NBO::UNSIGNED_SHORT  ) +	// Planned_End_Segment_No
						sizeof(NBO::ENUMERATED_BYTE )  ;// Data_Field_Representation
			o_HeaderRecordSequence.push_back(
				SHeaderRecordSequenceItem(e_SegmentIdentification, length));
		}

		// Image Segment Line Quality Header record.
		if (m_LineQualityEntries.size() > 0)
		{
			length =	sizeof(NBO::UNSIGNED_BYTE )   +	// Header_Type
						sizeof(NBO::UNSIGNED_SHORT)   +	// Header_Record_Length
						m_LineQualityEntries.size() *
						sizeof(SLineQualityEntry)      ;
			o_HeaderRecordSequence.push_back(
				SHeaderRecordSequenceItem(e_ImageSegmentLineQuality, length));
		}
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




std::istream& CxRITFileHeaderRecords::Read
(
	std::istream& i_Stream
)
{
	try
	{
		// Initialise.
		*this = DISE::CxRITFileHeaderRecords();

		DISE::CxRITFileHeaderRecords::EHeaderType	type;
		unsigned short								length;
		unsigned __int32			accLength			= 0;        //Lorenzo 31/05/2007. change "__int64" for "__int32"
		unsigned __int32			totalHeaderLength	= 0;                //Lorenzo 31/05/2007. change "__int64" for "__int32"

		assert(sizeof(int)==4);		//Lorenzo 31/05/2007. The size of the integer should be 32 bits


		// Read header record by header record.
		do
		{
			// Read record type and length.
			NBO::UNSIGNED_BYTE  type_BE;
			NBO::UNSIGNED_SHORT length_BE;
			i_Stream >> type_BE
					 >> length_BE;
			PRECONDITION(i_Stream.good());
			type   = EHeaderType((unsigned char)type_BE);
			length = length_BE; 

			// Read record specific part.
			switch (type)
			{
				case e_PrimaryHeader           : 
					{
						NBO::ENUMERATED_BYTE fileTypeCode_BE;
						NBO::UNSIGNED        totalHeaderLength_BE;
						NBO::UNSIGNED_DOUBLE dataFieldLength_BE;
						i_Stream >> fileTypeCode_BE
								 >> totalHeaderLength_BE
								 >> dataFieldLength_BE;
						m_FileTypeCode = EFileTypeCode((int)fileTypeCode_BE);
						totalHeaderLength = totalHeaderLength_BE;
						m_DataFieldLength = dataFieldLength_BE;
					}
					break;

				case e_ImageStructure          : 
					{
						NBO::UNSIGNED_BYTE   NB_BE;
						NBO::UNSIGNED_SHORT  NC_BE;
						NBO::UNSIGNED_SHORT  NL_BE;
						NBO::ENUMERATED_BYTE compressionFlag_BE;
						i_Stream >> NB_BE
								 >> NC_BE
								 >> NL_BE
								 >> compressionFlag_BE;
						m_NB = NB_BE;
						m_NC = NC_BE;
						m_NL = NL_BE;
						m_CompressionFlag = DISE::CxRITFileHeaderRecords::ECompressionFlag(
														(unsigned char)compressionFlag_BE);
					}
					break;

				case e_ImageNavigation         : 
					{
						m_ProjectionName.erase();
						for (int i = 0; i < e_SizeofProjectionName; ++i)
						{
							char c;
							i_Stream.get(c);
							m_ProjectionName += c;
						}
						NBO::INTEGER temp;
						i_Stream >> temp;	m_CFAC = temp;
						i_Stream >> temp;	m_LFAC = temp;
						i_Stream >> temp;	m_COFF = temp;
						i_Stream >> temp;	m_LOFF = temp;
					}
					break;

				case e_ImageDataFunction       : 
					{
						for (int i = sizeof(NBO::UNSIGNED_BYTE )
								   + sizeof(NBO::UNSIGNED_SHORT); i < length; ++i)
						{
							char c;
							i_Stream.get(c);
							m_DataDefinitionBlock += c;
						}
					}
					break;

				case e_Annotation              : 
					{
						std::string temp;
						for (int i = sizeof(NBO::UNSIGNED_BYTE )
								   + sizeof(NBO::UNSIGNED_SHORT); i < length; ++i)
						{
							char c;
							i_Stream.get(c);
							temp += c;
						}
						m_Annotation = DISE::CxRITAnnotation(temp);
					}
					break;

				case e_TimeStamp               : 
					{
						NBO::UNSIGNED_BYTE  CDS_T_Field;
						NBO::TIME_CDS_SHORT CDS_P_Field;
						i_Stream >> CDS_T_Field
								 >> CDS_P_Field;
						PRECONDITION(CDS_T_Field == 0x40);
						m_TimeStamp = CDS_P_Field.CUTCTime();
					}
					break;

				case e_AncillaryText           : 
					{
						for (int i = sizeof(NBO::UNSIGNED_BYTE )
								   + sizeof(NBO::UNSIGNED_SHORT); i < length; ++i)
						{
							char c;
							i_Stream.get(c);
							m_AncillaryText += c;
						}
					}
					break;

				case e_KeyHeader               : 
					{
						NBO::UNSIGNED_BYTE   keyNumber_BE;
						NBO::UNSIGNED_DOUBLE seed_BE;
						i_Stream >> keyNumber_BE
								 >> seed_BE;
						m_Annotation.SetEncrypted(true);
						m_KeyNumber = keyNumber_BE;
						m_Seed      = seed_BE;
					}
					break;

				case e_SegmentIdentification   : 
					{
						NBO::ENUMERATED_SHORT spacecraftID_BE;
						NBO::ENUMERATED_BYTE  spectralChannelID_BE;
						NBO::UNSIGNED_SHORT   segmentSeqNo_BE;
						NBO::UNSIGNED_SHORT   plannedStartSegmentNo_BE;
						NBO::UNSIGNED_SHORT   plannedEndSegmentNo_BE;
						NBO::ENUMERATED_BYTE  dataFieldRepresentation_BE;
						i_Stream >> spacecraftID_BE
								 >> spectralChannelID_BE
								 >> segmentSeqNo_BE
								 >> plannedStartSegmentNo_BE
								 >> plannedEndSegmentNo_BE
								 >> dataFieldRepresentation_BE;
						m_SpacecraftID            = spacecraftID_BE;
						m_SpectralChannelID       = spectralChannelID_BE;
						m_SegmentSeqNo            = segmentSeqNo_BE;
						m_PlannedStartSegmentNo   = plannedStartSegmentNo_BE;
						m_PlannedEndSegmentNo     = plannedEndSegmentNo_BE;
						m_DataFieldRepresentation =
								EDataFieldRepresentation((int)dataFieldRepresentation_BE);
					}
					break;

				case e_ImageSegmentLineQuality : 
					{
						for (int i = sizeof(NBO::UNSIGNED_BYTE )
								   + sizeof(NBO::UNSIGNED_SHORT); i < length;
												i += sizeof(DISE::SLineQualityEntry))
						{
							DISE::SLineQualityEntry LineQualityEntry;
							i_Stream >> LineQualityEntry;
							m_LineQualityEntries.push_back(LineQualityEntry);
						}
					}
					break;

				default                                             : 
					PRECONDITION(0);
			}
		}
		while ((accLength += length )< totalHeaderLength);
		PRECONDITION(totalHeaderLength != 0        );
		PRECONDITION(totalHeaderLength == accLength);

		return i_Stream;
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




std::ostream& CxRITFileHeaderRecords::Write
(
	std::ostream& i_Stream
)
const
{
	try
	{
		// Determine sequence of header records.
		std::vector<SHeaderRecordSequenceItem> headerRecordSequence;
		GetHeaderRecordSequence(headerRecordSequence);

		// Write header records:
		std::vector<SHeaderRecordSequenceItem>::const_iterator iter;
		for (iter =  headerRecordSequence.begin();
			 iter != headerRecordSequence.end()  ; ++iter)
		{
			i_Stream	<< NBO::UNSIGNED_BYTE (iter->m_HeaderType  )
						<< NBO::UNSIGNED_SHORT((unsigned short)iter->m_HeaderLength);
			AssertCLib(i_Stream.good());

			switch (iter->m_HeaderType)
			{
				case e_PrimaryHeader           :
					{
						unsigned long totalHeaderLength = 0;
						for (unsigned int i = 0; i < headerRecordSequence.size(); ++i)
							totalHeaderLength += headerRecordSequence[i].m_HeaderLength;

						i_Stream	<< NBO::ENUMERATED_BYTE(m_FileTypeCode)
									<< NBO::UNSIGNED       (totalHeaderLength)
									<< NBO::UNSIGNED_DOUBLE(m_DataFieldLength);
						AssertCLib(i_Stream.good());
					}
					break;

				case e_ImageStructure          :
					{
						i_Stream	<< NBO::UNSIGNED_BYTE  (m_NB                )
									<< NBO::UNSIGNED_SHORT (m_NC                )
									<< NBO::UNSIGNED_SHORT (m_NL                )
									<< NBO::ENUMERATED_BYTE(m_CompressionFlag   );
						AssertCLib(i_Stream.good());
					}
					break;

				case e_ImageNavigation         :
					{
						for (unsigned int i = 0; i < m_ProjectionName.size(); ++i)
							i_Stream << m_ProjectionName[i];
						i_Stream	<< NBO::INTEGER(m_CFAC)
									<< NBO::INTEGER(m_LFAC)
									<< NBO::INTEGER(m_COFF)
									<< NBO::INTEGER(m_LOFF);
						AssertCLib(i_Stream.good());
					}
					break;

				case e_ImageDataFunction       :
					{
#ifdef WIN32
						for (unsigned int i = 0; i < m_DataDefinitionBlock.size(); ++i)
							i_Stream << m_DataDefinitionBlock[i];

						/*i_Stream.write(m_DataDefinitionBlock.begin(),m_DataDefinitionBlock.size());*/
#else
						i_Stream.write(m_DataDefinitionBlock.begin().base(),m_DataDefinitionBlock.size());
#endif
						AssertCLib(i_Stream.good());
					}
					break;

				case e_Annotation              :
					{
						std::string temp = m_Annotation.GetText();
						for (unsigned int i = 0; i < temp.size(); ++i)
							i_Stream << temp[i];
						AssertCLib(i_Stream.good());
					}
					break;

				case e_TimeStamp               :
					{
						i_Stream	<< NBO::UNSIGNED_BYTE (0x40           )
									<< NBO::TIME_CDS_SHORT(m_TimeStamp    );
						AssertCLib(i_Stream.good());
					}
					break;

				case e_AncillaryText           :
					{
						for (unsigned int i = 0; i < m_AncillaryText.size(); ++i)
							i_Stream << m_AncillaryText[i];
						AssertCLib(i_Stream.good());
					}
					break;

				case e_KeyHeader               :
					{
						i_Stream	<< NBO::UNSIGNED_BYTE  (m_KeyNumber    )
									<< NBO::UNSIGNED_DOUBLE(m_Seed         );
						AssertCLib(i_Stream.good());
					}
					break;

				case e_SegmentIdentification   :
					{
						i_Stream	<< NBO::ENUMERATED_SHORT(m_SpacecraftID             )
									<< NBO::ENUMERATED_BYTE (m_SpectralChannelID        )
									<< NBO::UNSIGNED_SHORT  (m_SegmentSeqNo             )
									<< NBO::UNSIGNED_SHORT  (m_PlannedStartSegmentNo    )
									<< NBO::UNSIGNED_SHORT  (m_PlannedEndSegmentNo      )
									<< NBO::ENUMERATED_BYTE (m_DataFieldRepresentation  );
						AssertCLib(i_Stream.good());
					}
					break;

				case e_ImageSegmentLineQuality :
					{
						for (unsigned int i = 0; i < m_LineQualityEntries.size(); ++i)
							i_Stream << m_LineQualityEntries[i];
						AssertCLib(i_Stream.good());
					}
					break;
			}
		}

		return i_Stream;
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




std::string CxRITFileHeaderRecords::GetDisplayString
(
	const bool	i_CRFlag
)
const
{
	try
	{
		// Determine sequence of header records.
		std::vector<SHeaderRecordSequenceItem> headerRecordSequence;
		GetHeaderRecordSequence(headerRecordSequence);

		// Write header records.
		const std::string endLine(i_CRFlag ? "\r\n" : "\n");

		std::ostringstream temp;

		std::vector<SHeaderRecordSequenceItem>::const_iterator iter;
		for (iter =  headerRecordSequence.begin();
			 iter != headerRecordSequence.end()  ; ++iter)
		{
			switch (iter->m_HeaderType)
			{
				case e_PrimaryHeader           :
					try
					{
						std::string fileTypeCode;
						switch (m_FileTypeCode)
						{
							case e_ImageDataFile		:
								fileTypeCode = "Image data file";				break;
							case e_GTSMessage			:
								fileTypeCode = "GTS messages file";				break;
							case e_AlphanumericTextFile	:
								fileTypeCode = "Alphanumeric text file";		break;
							case e_EncryptionKeyMessage	:
								fileTypeCode = "Encryption key message file";	break;
							case e_RepeatCyclePrologue	:
								fileTypeCode = "Repeat cycle prologue file";	break;
							case e_RepeatCycleEpilogue	:
								fileTypeCode = "Repeat cycle epilogue file";	break;
							case e_DCPMessage			:
								fileTypeCode = "DCP messages file";				break;
						}

						temp
						<< "Primary Header Record"
						<< " (type " << (int)iter->m_HeaderType
						<< ", length " << (int)iter->m_HeaderLength << ")"			<< endLine
						<< " File Type Code     : " << m_FileTypeCode
													<< " ("  << fileTypeCode.c_str() << ")"
																					<< endLine
						<< " Total Header Length: " << (int)GetTotalHeaderLength()	<< endLine
						<< " Data Field Length  : " << (int)m_DataFieldLength		<< endLine
																					<< endLine;
					}
					catch (...)
					{
						LOGCATCHANDTHROW;
					}
					break;

				case e_ImageStructure          :
					try
					{
						std::string compressionFlag;
						switch (m_CompressionFlag)
						{
							case e_None		:
								compressionFlag = "No compression";			break;
							case e_Lossless	:
								compressionFlag = "Lossless compression";	break;
							case e_Lossy	:
								compressionFlag = "Lossy compression";		break;
						}

						temp
						<< "Image Structure Header Record"					
						<< " (type " << (int)iter->m_HeaderType
						<< ", length " << (int)iter->m_HeaderLength << ")"		<< endLine
						<< " NB                 : " << (int)m_NB				<< endLine
						<< " NC                 : " << (int)m_NC				<< endLine
						<< " NL                 : " << (int)m_NL				<< endLine
						<< " Compression Flag   : " << (int)m_CompressionFlag
						                               << " ("  << compressionFlag.c_str() << ")"
																				<< endLine
																				<< endLine;
					}
					catch (...)
					{
						LOGCATCHANDTHROW;
					}
					break;

				case e_ImageNavigation         :
					try
					{
						temp
						<< "Image Navigation Header Record"					
						<< " (type " << (int)iter->m_HeaderType
						<< ", length " << (int)iter->m_HeaderLength << ")"		<< endLine
						<< " Projection Name    : " << m_ProjectionName.c_str()	<< endLine
						<< " CFAC               : " << (int)m_CFAC				<< endLine
						<< " LFAC               : " << (int)m_LFAC				<< endLine
						<< " COFF               : " << (int)m_COFF				<< endLine
						<< " LOFF               : " << (int)m_LOFF				<< endLine
																				<< endLine;
					}
					catch (...)
					{
						LOGCATCHANDTHROW;
					}
					break;

				case e_ImageDataFunction       :
					try
					{
						temp
						<< "Image Data Function Header Record"					
						<< " (type " << (int)iter->m_HeaderType
						<< ", length " << (int)iter->m_HeaderLength << ")"		<< endLine
						<< " Data Defin. Block  : "	<< endLine << ">>>>"		<< endLine
													<< m_DataDefinitionBlock.c_str()
													<< endLine << "<<<<"		<< endLine
																				<< endLine;
					}
					catch (...)
					{
						LOGCATCHANDTHROW;
					}
					break;

				case e_Annotation              :
					try
					{
						temp
						<< "Annotation Header Record"					
						<< " (type " << (int)iter->m_HeaderType
						<< ", length " << (int)iter->m_HeaderLength << ")"		<< endLine
						<< " Annotation Text    : "	<< m_Annotation.GetText().c_str()
																				<< endLine
																				<< endLine;
					}
					catch (...)
					{
						LOGCATCHANDTHROW;
					}
					break;

				case e_TimeStamp               :
					try
					{
						temp
						<< "Time Stamp Header Record"					
						<< " (type " << (int)iter->m_HeaderType
						<< ", length " << (int)iter->m_HeaderLength << ")"		<< endLine
						    //<< " Time Stamp         : "	<< m_TimeStamp.Format().c_str() << endLine
						<< endLine;
					}
					catch (...)
					{
						LOGCATCHANDTHROW;
					}
					break;

				case e_AncillaryText           :
					try
					{
						temp
						<< "Ancillary Text Header Record"					
						<< " (type " << (int)iter->m_HeaderType
						<< ", length " << (int)iter->m_HeaderLength << ")"		<< endLine
						<< " Ancillary Text     : "	<< endLine << ">>>>"		<< endLine
													<< m_AncillaryText.c_str()
													<< endLine << "<<<<"		<< endLine
																				<< endLine;
					}
					catch (...)
					{
						LOGCATCHANDTHROW;
					}
					break;

				case e_KeyHeader               :
					try
					{
						char seed[17];
						sprintf(seed, "%016llx", m_Seed);

						temp
						<< "Key Header Record"					
						<< " (type " << (int)iter->m_HeaderType
						<< ", length " << (int)iter->m_HeaderLength << ")"		<< endLine
						<< " Key Number         : " << (int)m_KeyNumber			<< endLine
						<< " Seed               : " << seed << " (hex)"			<< endLine
																				<< endLine;
					}
					catch (...)
					{
						LOGCATCHANDTHROW;
					}
					break;

				case e_SegmentIdentification   :
					try
					{
						std::string spacecraftID = m_SpacecraftID.GP_SC_NAME();
						std::string dataFieldRepresentation;
						switch (m_DataFieldRepresentation)
						{
							case e_NoSpecific	:
								dataFieldRepresentation = "No specific formatting";		break;
							case e_JPEG			:
								dataFieldRepresentation = "JPEG interchange format";	break;
							case e_T4			:
								dataFieldRepresentation = "T.4 coded file format";		break;
							case e_WT			:
								dataFieldRepresentation = "Wavelet-Transform format";	break;
						}

						temp
						<< "Segment Identification Record"					
						<< " (type " << (int)iter->m_HeaderType
						<< ", length " << (int)iter->m_HeaderLength << ")"					<< endLine
						<< " Satellite                : " << (int)m_SpacecraftID
						                                  << " ("  << spacecraftID.c_str() << ")"
																							<< endLine
						<< " Spectral Channel ID      : " << (int)m_SpectralChannelID		<< endLine
						<< " Segm Seq No              : " << (int)m_SegmentSeqNo			<< endLine
						<< " Planned Start Segm Seq No: " << (int)m_PlannedStartSegmentNo	<< endLine
						<< " Planned End Segm Seq No  : " << (int)m_PlannedEndSegmentNo		<< endLine
						<< " Data Field Representation: " << (int)m_DataFieldRepresentation
					                                  << " ("  << dataFieldRepresentation.c_str() << ")"
																							<< endLine
																							<< endLine;
					}
					catch (...)
					{
						LOGCATCHANDTHROW;
					}
					break;

				case e_ImageSegmentLineQuality :
					try
					{
						temp
						<< "Image Segment Line Quality Record"					
						<< " (type " << (int)iter->m_HeaderType
						<< ", length " << (int)iter->m_HeaderLength << ")"		<< endLine
						<< " Line Quality Entries: "							<< endLine;

						std::deque<DISE::SLineQualityEntry>::const_iterator iter;
						for (iter =  m_LineQualityEntries.begin();
						     iter != m_LineQualityEntries.end()  ; ++iter)
						{
							temp << " " << (int)iter->m_LineNumberInGrid
							    //<< "," << iter->m_LineMeanAcquisitionTime.CMSGTime().ReadableForm().c_str()
								 << "," << (int)iter->m_LineValidity
								 << "," << (int)iter->m_LineRadiometricQuality
								 << "," << (int)iter->m_LineGeometricQuality
								 << endLine;
						}
					}
					catch (...)
					{
						LOGCATCHANDTHROW;
					}
					break;
			}
		}

		return temp.str();
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}














} // end namespace
