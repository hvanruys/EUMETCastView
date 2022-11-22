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

#ifndef CJPEGLosslessCoder_included
#define CJPEGLosslessCoder_included

/*******************************************************************************

TYPE:
Free Functions.
					
PURPOSE: 
Encapsulate the coding of an image using JPEG LossLess.

FUNCTION: 
Holds JPEG LossLess-specific data & functions together
and performs the needed operations to compress an uncompressed
buffer.

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
WriteHeader : Writes a lossless JPEG header file into the buffer
make_optimal_DIFF_table : computes an optimal DC Huffman table, such that the 
                          compressed image will be the smallest possible
DPCM_coder : performs the compression of the original image and makes the entropy-coded segment

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
#include "CHOptim.h"


namespace COMP
{


class CJPEGLossLessCoder : public COMP::CJPEGCoder
{

private:

// DATA:

	unsigned short m_default_value; // JPEG default initialisation value

// INTERFACES:

	// Description:	Writes a lossless JPEG header into the buffer
	// Returns:		Nothing.
	void WriteHeader();
	
	// Description:	Performs the compression of the given line of the image.
	// Returns:		Nothing.
	void CodeNextLine
	(
		unsigned short i_cur_line,		// Image line # to compress.
		unsigned short i_predictor0,	// Predictor to use to compute the prediction
										// of the first pixels.
		unsigned short i_predictor		// Predictor to use to compute the prediction
										// for the remaining pixels.
	);
	
	// Description:	Performs the compression of the original image and creates
	//				the entropy-coded segment
	// Returns:		Nothing.
	void DPCM_coder();

public:

	// Description:	Constructor. Initialise the coder with its input data
	//				and allocate the output buffer.
	// Returns:		Nothing.
	CJPEGLossLessCoder
	(
		const Util::CDataFieldUncompressedImage&	i_cdfui,
													// Uncompressed input image datafield.
		const CJPEGParams&							i_param
													// Compression parameters.
	);

	// Description:	Destructor.
	// Returns:		Nothing.
	~CJPEGLossLessCoder();

	// Description:	Performs the coding of the input buffer in the output buffer
	// Returns:		Nothing.
	void CodeBuffer();

};


} // end namespace


#endif
