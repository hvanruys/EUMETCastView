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

#ifndef CHOptim_included
#define CHOptim_included

/*******************************************************************************

TYPE:
class
					
PURPOSE: 
Compute optimum huffman tables for the JPEG coder, given images.

FUNCTION: 

INTERFACES:
See 'INTERFACES:' in the module declaration below.

RESOURCES:	
Heap Memory (>2K).

REFERENCES:

PROCESSING:
The constructors initializes the object with a CCompressParams 
structure identical to what would be used to compress an image.
accumulateFrequencies() can then be used to accumulate the frequencies
of the symbols deduced from the given image.
When all required images are processed, computeOptimalHuffmanTables()
must be called to actually compute the optimal huffman table from the
accumulated frequencies.

DATA:

LOGIC:		
-

*******************************************************************************/

#include <memory>
#include "CompressJPEG.h"
#include "CDataField.h"

#include "CBuffer.h"
#include "CImage.h"
#include "CJBlock.h"
#include "CQuantizationTable.h"


namespace COMP
{


class CHOptim
{

private:

//DATA:

	CJPEGParams&		m_param;			// compression parameters
	unsigned short		m_default_value;	// JPEG default initialization value (Lossless)
	CQuantizationTable	m_cqtbl;			// Quantization tables use (Lossy)
	unsigned __int32	m_freqDC[17];		// array of DC symbols frequencies
	unsigned __int32	m_freqAC[256];		// array of AC symbols frequencies

// INTERFACES:
	
	// Description:	Compute and update the symbol histogram from the given image, 
	//				in the JPEG lossless case.
	// Returns:		Nothing.
	void accumulateFrequenciesLossless
	(
		CImage &i_image	// Image from which to accumulate Lossless Huffman symbols frequencies
	);

	// Description:	Compute and update the symbol histogram from the given image, 
	//				in the JPEG lossy case.
	// Returns:		Nothing.
	void accumulateFrequenciesLossy
	(
		CImage &i_image	// Image from which to accumulate Lossy Huffman symbols frequencies
	);
	
	// Description:	Subfunction of the Lossless accumulation routine, that accumulates
	//				the symbol frequencies in one line.
	// Returns:		Nothing.
	void accumulateFrequenciesNextLine
	(
		CImage &i_image,			// Image from which to accumulate Lossless Huffman
									// symbols frequencies
		unsigned short i_cur_line,	// current line to consider
		unsigned short i_predictor0,// predictor for first pixel of the line
		unsigned short i_predictor	// predictor for other pixels
	);

	// Description:	Subfunction of the lossy accumulation routing, that accumulates
	//				the symbol frequencies in on 8x8 block
	// Returns:		Nothing.
	void accumulateFrequenciesBlock 
	(
		CJBlock<short> &i_blk	// 8x8 block from which to accumulate Lossy Huffman
								// symbols frequencies
	);

	// Description:	Compute one optimal table (either DC or AC)
	// Returns:		Nothing.
	bool computeOptimalTable 
	(
		bool			i_full,		// If true builds a complete Huffman table (all symbols
									// are defined) otherwise only symbols having non null
									// frequencies are considered
		bool			i_DC,		// Consider DC Huffman table if true, otherwise AC table
		CHuffmanTable&	o_ht,		// Output SHuffmanTable structure
		unsigned int&	o_nbSymb	// Initial and final number of Huffman symbols
	);

	// Description:	Recursive routine, assigning a huffman code based on the frequency
	//				of the coded symbol.
	// Returns:		Nothing.
	bool recurseTree 
	(
		unsigned int	i_start,	// index of the start element
		unsigned int	i_stop,		// index of the last element
		unsigned int	i_level,	// current level in the Huffman tree (0 is root)
		unsigned __int32* i_freq,	// array of sorted frequencies
		CHuffmanTable&	o_ht		// Output SHuffmanTable structure where the Bits array
									// is modified
	);

public:

// INTERFACES:

	// Description:	Constructor.
	// Returns:		Nothing.
	CHOptim
	(
		CJPEGParams &i_param	// CJPEGParams class
								// Allow to know if Lossless or Lossy
								// Allow to initialize quantification tables
								// Allow to get various needed parameter
	);

	// Description:	Resets the frequency table
	// Returns:		Nothing.
	void init();

	// Description:	Computes & accumulates the symbol frequencies of the given image.
	// Returns:		Nothing.
	void accumulateFrequencies 
	(
		CImage &i_image	// Image from which to accumulate Huffman symbols frequencies
	);

	// Description:	Compute and writes back in i_params the optimal huffman table for
	//				the symbol histogram accumulated so far.
	// Returns:		Nothing.
	bool computeOptimalHuffmanTables
	(
		bool i_full	// If true builds a complete Huffman table (all symbols are defined)
					// otherwise only symbols having non null frequencies are considered.
	);

};


} // end namespace


#endif
