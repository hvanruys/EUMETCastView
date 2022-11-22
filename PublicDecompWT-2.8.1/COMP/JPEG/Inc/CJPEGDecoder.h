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

#ifndef CJPEGDecoder_included
#define CJPEGDecoder_included

/******************************************************************************

TYPE:
Concrete class.

PURPOSE: 
Base JPEG Deoder object featuring common functionalities between both the 
lossy & the lossless decoder.

FUNCTION:	
This object is merely a container for common JPEG-decoder sub-objects
and is used to have a centralised initialisation point.

INTERFACES:	
See 'INTERFACES' in the module declaration below 
      
RESOURCES:	
Heap Memory (>2K).
      
REFERENCE:	
None.

PROCESSING:	
The constructor takes a CDataFieldCompressedImage and uses is to 
initialise the sub-objects.

DATA:
None			
      
LOGIC:
-

*******************************************************************************/

#include "CDataField.h"
#include "CompressJPEG.h"
#include "CImage.h"
#include "CBuffer.h"
#include "CHcodec.h"
#include "CJBlock.h"
#include "CQuantizationTable.h"
#include "CQualityInfo.h"


namespace COMP
{


class CJPEGDecoder
{

protected:

//DATA:

	CImage				m_Image;	//	image data buffer
	CJPEGParams			m_param;	//	compression parameters
	CRBuffer			m_buf;		//	compressed data buffer
	CHDecoder			m_hdecoder;	//	huffmann coder 
	CQuantizationTable	m_cqtbl;	//	quantization tables
	CQualityInfo		m_qinfo;	//	quality information vector

	//-------JPEG Lossless specific data members
	unsigned short m_default_value; // JPEG default initialization value


//INTERFACES:

	// Description:	Reads the JPEG header in order to determine the compression
	//				method used (Lossy/LossLess).
	//				If the header could not be read, the m_qinfo & m_image are 
	//				initialised to zero.
	// Returns:		A DISE::ECompressionMode specifying the used compression method.
	//				e_maxECompressionMode is returned if the header could not be decoded
	//				(due to an error for instance).
	CJPEGParams::ECompressionMode Init();

	//-------JPEG-Lossy Decoder methods

	// Description:	Decodes the compressed data buffer considering it contains a
	//				lossy-coded image.
	// Returns:		Nothing.
	void DecodeLossyBuffer();
	
	// Description:	Reads a lossy JPEG header from the input buffer
	//				and performs the suitable initialisations in m_param, m_chdecoder.
	// Returns:		true if successful, false if an error was discovered when trying
	//				to read the header.
	bool read_LOSSY_header();
	
	// Description:	Reads the JPEG footer.
	// Returns:		Nothing.
	void ReadJPEGLossyFooter();
	
	// Description:	Seeks the JPEG stream for the next restart marker. When found,
	//				update^s the QualityInformation and CImage accordingly.
	// Returns:		true  if EOI was reached,
	//				false if a valid restart marker was found.
	bool PerformLossyResync
	(
		unsigned long&	t_cur_MCU,			// MCU being processed when the error occurred.
		unsigned long&	t_cur_interval,		// Current RI when the error occurred.
		unsigned short&	t_corner_line,		// Co-ordinates of the next block to decode.
		unsigned short&	t_corner_column,	// "
		bool			t_good_line			// true if pixels in current line are to be
											// considered as good.
	);
	
	// Description:	Put zeroes in specified MCU blocks (8x8)in the image.
	// Returns:		Nothing.
	void ZeroMCU
	(
		unsigned short i_from_line,		// Co-ordinate of the upper left corner
										//	of the first MCU to clear.
		unsigned short i_from_column,	// "
		unsigned short i_to_line,		// Co-ordinate of the upper left corner
										//	of the last MCU to clear.
		unsigned short i_to_column		// "
	);

	// Description:	Reads the JPEG stream and dencodes a 8*8 block of quantized coefficients.
	// Returns:		true if decoding was successful.
	bool DecodeBlock
	(
		CJBlock<short>&	o_blk	// Decoded quantized DCT coefficients.
	);

	//-------JPEG-LossLess Decoder methods

	// Description:	Decodes the compressed data buffer considering it contains
	//				a lossless-coded image.
	// Returns:		Nothing.
	void DecodeLossLessBuffer();
	
	// Description:	Reads a lossless JPEG header from the input buffer
	//				and performs the suitable initialisations in m_param, m_chdecoder.
	// Returns:		true if successful,
	//				false if an error was discovered when trying to read the header.
	bool read_LOSSLESS_header();
	
	// Description:	Reads the JPEG footer.
	// Returns:		Nothing.
	void ReadJPEGLossLessFooter();

	// Description:	Performs the decompression of the JPEG lossless coded segment,
	//				including error detection & recovery in the data segment.
	// Returns:		Nothing.
	void DPCM_decoder();

	// Description:	Performs the decoding of the given line of the image.
	//				The quality information corrsponding to the decoded line
	//				is updated.
	//				If an error is detected, the remaining of the line is set
	//				to zero and false is returned. The Quality information is
	//				updated accordingly.
	// Returns: 	true if decoding was successful
	//				false if an error was detected
	bool DecodeNextLine
	(
		unsigned short i_cur_line,		// image line # to decode.
		unsigned short i_predictor0,	// predictor to use to compute the prediction
										// of the first pixels.
		unsigned short i_predictor		// predictor to use to compute the prediction
										// for the remaining pixels.
	);

	// Description:	Scans the JPEG stream for the next marker.
	//				If a restart marker is found (FFD0--FFD7), its ID(0-7) is returned.
	//				Else, a negative number is returned.
	//				In any case, the buffer position is set BEFOR the marker,
	//				if a marker was found.
	// Returns:		
	short FindNextMarker();
	
public:

// INTERFACES:

	// Description:	Creates a CJPEGDecoder.
	// Returns:		Nothing.
	CJPEGDecoder
	(
		const Util::CDataFieldCompressedImage& i_cdfci	// Buffer containing
														// the image data to decompress.
	);
	
	// Description:	Desctructor.
	// Returns:		Nothing.
	~CJPEGDecoder();
	
	// Description:	Performs the JPEG decoding by first determining the type
	//				of the coded stream
	// Returns:		Nothing.
	void DecodeBuffer();           

	// Description:	Return the decompressed image. The packing is handled by CImage,
	//				that could throw a CParamException if the requested conversion
	//				is unsupported.
	// Returns:		The decompressed image, with the specified representation.
	Util::CDataFieldUncompressedImage GetDecompressedImage
	(
		const unsigned short i_NR
	);
	
	// Description:	Returns the quality information array.
	// Returns:		A vector of one element per image line, each element containing
	//				the number of valid pixels in the corresponding line.
	//				A negative value indicates that this value is doubtful.
	std::vector<short> GetQualityInfo();

};


} // end namespace


#endif


