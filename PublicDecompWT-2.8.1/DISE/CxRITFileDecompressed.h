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

#ifndef CxRITFileDecompressed_included
#define CxRITFileDecompressed_included

/*******************************************************************************

TYPE:
Concrete Class.
					
PURPOSE:
Decompression of LRIT/HRIT files.

FUNCTION:
A decompressed LRIT/HRIT file can be constructed from a compressed LRIT/HRIT file.

INTERFACES:
Objects of the CxRITFileDecompressed class contain exactly one LRIT/HRIT file.
See also 'INTERFACES:' in the module declaration below.

RESOURCES:	
Heap Memory (>2K).

REFERENCES:
MSG LRIT/HRIT Global Specififation (CGMS03).
MSG LRIT/HRIT Mission Specific Implementation (EUM/MSG/SPE/057).

PROCESSING:
The only member function of the CxRITFileDecompressed class is a constructor which
takes as input a compressed LRIT/HRIT file.

DATA:
See 'DATA:' in the class headers below.

LOGIC:		
-

*******************************************************************************/

#include "Compress.h"		// COMP
#include "CompressJPEG.h"	// COMP\JPEG
#include "CompressT4.h"		// COMP\T4
#include "CompressWT.h"		// COMP\WT
#include "CxRITFile.h"		// DISE




namespace DISE
{




class CxRITFileDecompressed : public DISE::CxRITFile
{

private:

// DATA:

	std::vector<short>	m_QualityInfo;	// Quality information.

public:

// INTERFACES:

	// Description:	Constructs a decompressed version of the input file.
	// Returns:		Nothing.
	CxRITFileDecompressed
	(
		const CxRITFile& i_InputFile	// File to be compressed.
	)
	:	DISE::CxRITFile(i_InputFile)
	{
		try
		{
			// Input file is not compressed: Nothing to do.
			if (m_CompressionFlag == e_None)
				return;

			// Do not attempt to decompress an encrypted file.
			PRECONDITION(m_Annotation.GetEncryptedFlag() == false);

			// Decompress data field.
			Util::CDataFieldCompressedImage compressedImage(m_DataField,
															m_NB,
															m_NC,
															m_NL		);
			Util::CDataFieldUncompressedImage decompressedImage;
			switch (m_DataFieldRepresentation)
			{
				case e_JPEG :
					COMP::DecompressJPEG(	compressedImage,
											m_NB,
											decompressedImage,
											m_QualityInfo    );
					break;

				case e_T4   :
					COMP::DecompressT4(		compressedImage,
											decompressedImage,
											m_QualityInfo    );
					break;

				case e_WT   :
					COMP::DecompressWT(		compressedImage,
											m_NB,
											decompressedImage,
											m_QualityInfo    );
					break;

				default     :
					PRECONDITION(0);
			}

			// Store data field.
			m_DataField			= decompressedImage;

			// Adjust header fields.
			m_NB				= decompressedImage.GetNB();
			m_NC				= decompressedImage.GetNC();
			m_NL				= decompressedImage.GetNL();
			m_DataFieldLength	= decompressedImage.Size();

			m_Annotation.SetCompressed(false);
			m_CompressionFlag         = e_None;
			m_DataFieldRepresentation = e_NoSpecific;
		}
		catch (...)
		{
			LOGCATCHANDTHROW;
		}
	}

	// Description:	Provides read access to the quality information obtained
	//				during decompression.
	// Returns:		Nothing.
	const std::vector<short>& GetQualityInfo
	(
	)
	const
	{
		return m_QualityInfo;
	}

};




} // end namespace


#endif
