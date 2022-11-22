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

#ifndef CxRITFileCompressed_included
#define CxRITFileCompressed_included

/*******************************************************************************

TYPE:
Concrete Class.
					
PURPOSE:
Compression of LRIT/HRIT files.

FUNCTION:
A compressed LRIT/HRIT file can be constructed from a non-compressed LRIT/HRIT file
and a set of compression parameters.

INTERFACES:
Objects of the CxRITFileCompressed class contain exactly one LRIT/HRIT file.
See also 'INTERFACES:' in the module declaration below.

RESOURCES:	
Heap Memory (>2K).

REFERENCES:
MSG LRIT/HRIT Global Specififation (CGMS03).
MSG LRIT/HRIT Mission Specific Implementation (EUM/MSG/SPE/057).

PROCESSING:
The only member function of the CxRITFileCompressed class is a constructor which
takes as input a non-compressed LRIT/HRIT file and a reference to a CCompress
object (providing the compression parameters and algorithm).

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




class CxRITFileCompressed : public DISE::CxRITFile
{

public:

// INTERFACES:

	// Description:	Constructs a compressed version of the input file.
	// Returns:		Nothing.
	CxRITFileCompressed
	(
		const DISE::CxRITFile&	i_InputFile,	// File to be compressed.
		const COMP::CCompress&	i_Params		// Compression parameters and algorithm.
	)
	: DISE::CxRITFile(i_InputFile)
	{
		try
		{
			// If file is already compressed, or does not contain image data:
			// there is nothing to do.
			if (m_CompressionFlag != e_None ||
				m_FileTypeCode != e_ImageDataFile)
				return;

			// Compress data field and store data field.
			Util::CDataFieldUncompressedImage uncompressedImage(m_DataField,
																m_NB,
																m_NC,
																m_NL,
																m_NB		);
			Util::CDataFieldCompressedImage compressedImage = i_Params.Compress(uncompressedImage);
			m_DataField = compressedImage;

			// Adjust header fields.
			m_NB				= compressedImage.GetNB();
			m_NC				= compressedImage.GetNC();
			m_NL				= compressedImage.GetNL();
			m_DataFieldLength	= compressedImage.Size();

			m_Annotation.SetCompressed(true);

			if		(typeid(i_Params) == typeid(COMP::CCompressLosslessJPEG	))
			{
				m_CompressionFlag         = e_Lossless;
				m_DataFieldRepresentation = e_JPEG;
			}
			else if (typeid(i_Params) == typeid(COMP::CCompressLossyJPEG	))
			{
				m_CompressionFlag         = e_Lossy;
				m_DataFieldRepresentation = e_JPEG;
			}
			else if (typeid(i_Params) == typeid(COMP::CCompressT4			))
			{
				m_CompressionFlag         = e_Lossless;
				m_DataFieldRepresentation = e_T4;
			}
			else if (typeid(i_Params) == typeid(COMP::CCompressWT			))
			{
				m_CompressionFlag         = i_Params.IsLossless()	?	e_Lossless
																	:	e_Lossy;
				m_DataFieldRepresentation = e_WT;
			}
			else
			{
				m_CompressionFlag         = e_None;
				m_DataFieldRepresentation = e_NoSpecific;
			}
		}
		catch (...)
		{
			LOGCATCHANDTHROW;
		}
	}

};




} // end namespace


#endif
