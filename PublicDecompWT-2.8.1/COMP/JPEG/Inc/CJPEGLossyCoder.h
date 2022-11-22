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

#ifndef CJPEGLossyCoder_included
#define CJPEGLossyCoder_included

/*******************************************************************************

TYPE:
Free Functions.
					
PURPOSE: 
Encapsulate the coding of an image using JPEG Lossy

FUNCTION: 
Holds JPEG Lossy-specific data & functions together and
performs the needed operations to compress an uncompressed buffer.

INTERFACES:
See 'INTERFACES:' in the module declaration below.

RESOURCES:	
Heap Memory (>2K).

REFERENCES:
Digital compression and coding of continuous-tone still images.
Part 1, Requirements and guidelines, ISO/IEC 10918-1

PROCESSING:
The constructor initialise the object using the passed parameters
(CDataFieldUncompressedImage & CCompressParam).
The member function CodeBuffer() performs the actual JPEG encoding
of the data. 
GetCompressedImage() must then be used to obtain the compressed data 
field (from the inherited CJPEGCoder object).
Private member functions:
WriteHeader : writes the lossy JPEG header to the output buffer
EncodeBlock() is used to encode the given block of quantized DCT
coefficients in the buffer.

DATA:
None.

LOGIC:		
-

*******************************************************************************/

#include <memory>

#include "CompressJPEG.h"
#include "CDataField.h"
#include "CJPEGCoder.h"
#include "CBuffer.h"
#include "CHcodec.h"
#include "CJBlock.h"
#include "CQuantizationTable.h"
#include "CHOptim.h"


namespace COMP
{


class CJPEGLossyCoder : public COMP::CJPEGCoder
{

private:

// DATA:

	CQuantizationTable m_cqtbl;  // Quantization tables used
	
// INTERFACE:

	// Description: Writes the lossy JPEG header to the output buffer.
	// Returns:		Nothing.
	void WriteHeader();
	
	// Description:	Encodes a 8*8 block of quantized DCT coef. and writes 
	//				it into output stream.
	// Returns:		Nothing.
	void EncodeBlock
	(
		CJBlock<short>& i_blk	// Quantized DCT coefficients to code.
	);
	
public:

	// Description:	Constructor. Initialise the coder with its input data and
	//				allocate the output buffer.
	// Returns:		Nothing.
	CJPEGLossyCoder
	(
		const Util::CDataFieldUncompressedImage&	i_cdfui,
													// Uncompressed input image datafield.
		const CJPEGParams&							i_param
													// Compression parameters.
	);

	// Description:	Destructor
	// Returns:		Nothing.
	~CJPEGLossyCoder();
	
	// Description:	Performs the coding of the input buffer in the output buffer.
	// Returns:		Nothing.
	void CodeBuffer();   

};


} // end namespace


#endif
