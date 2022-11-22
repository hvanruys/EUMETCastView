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

#ifndef CJPEGCoder_included
#define CJPEGCoder_included

/*******************************************************************************

TYPE:
Free functions.

PURPOSE: 
Base JPEG Coder object featuring common functionalities between both the 
lossy & the lossless coder.

FUNCTION:	
This object is merely a container for common JPEG-coder sub-objects
and is used to have a centralised initialisation point.

INTERFACES:	
See 'INTERFACES' in the module declaration below.
      
RESOURCES:	
Heap Memory (>2K).
      
REFERENCE:	
None.

PROCESSING:	
The constructor takes a CDataFieldUncompressed image and a CCompressParam
and uses these to initialise the subobjects.
The private member function WriteJPEGFooter() writes the JPEG EOI marker
to the compressed data buffer.

DATA:
None			
      
LOGIC:
-

*******************************************************************************/

#include "CDataField.h"		// Util
#include "CompressJPEG.h"	// COMP
#include "CImage.h"			// COMP
#include "CBuffer.h"		// COMP
#include "CHcodec.h"		// COMP\JPEG
#include "JPEGConst.h"		// COMP\JPEG


namespace COMP
{


class CJPEGCoder
{

protected:

//DATA:

	CJPEGParams	m_param;	//	compression parameters
	CImage		m_Image;	//	image data buffer
	CWBuffer	m_buf;		//	compressed data buffer
	CHCoder		m_hcoder;	//	huffmann coder 

	enum E_constants
	{
		e_ExpectedCompressionFactor = 1, 	// Expected compression factor, used
											// to allocate the first CWBuffer.
		e_MaxConstants = -1
	};

//INTERFACE:

	// Description:	Writes the EOI marker to the data buffer
	// Returns:		Nothing.
	void WriteJPEGFooter()
	{
		COMP_TRYTHIS
			m_buf.write_marker(cMarkerEOI);
			m_buf.close();
		COMP_CATCHTHIS
	}

public:

//INTERFACE

	// Description:	Creates a CJPEGCoder.
	// Returns:		Nothing.
	CJPEGCoder
	(
		const Util::CDataFieldUncompressedImage &i_cdfui,	// Buffer containing the image
															// data to compress
		const CJPEGParams &i_param							// Parameters of the compression
	)
	: m_Image (i_cdfui)
	, m_param (i_param)
	, m_buf ((unsigned __int32)(i_cdfui.GetLength () * e_ExpectedCompressionFactor / 8))
	, m_hcoder (m_param, m_buf)
	{
	}
	
	// Description:	Destructor.
	// Returns:		Nothing.
	~CJPEGCoder()
	{
	}

	// Description:	Returns a CDFCI containing the compressed data buffer.
	// Returns:		A CDataFieldCompressedImage containing the compressed data buffer.	
	Util::CDataFieldCompressedImage GetCompressedImage()
	{
		COMP_TRYTHIS
			Util::CDataFieldCompressedImage cdfci (m_buf,
												   (unsigned char)m_Image.GetNB(),
												   m_Image.GetW(),
												   m_Image.GetH());
			return cdfci;
		COMP_CATCHTHIS
	}

};


} // end namespace


#endif
