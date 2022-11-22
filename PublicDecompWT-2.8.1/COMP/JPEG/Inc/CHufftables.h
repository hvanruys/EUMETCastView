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

#ifndef Huffhead_included
#define Huffhead_included

/*******************************************************************************

TYPE:
Concrete classes.
					
PURPOSE:
Management of the Huffman coding tables for both JPEG lossy and lossless modes 


FUNCTION:
These classes allows the application to work with multiple Huffman tables,
and having in a single object the specification of the Huffman AC and DC tables,
but also the complete definition of these tables for both coding and decoding process.
The coding and decoding tables are separated to improve the performance of these two processes.
This partial duplication introduce a small a very small overhead in terms of memory
but this really increase the speed of computation.

INTERFACES:
See 'INTERFACES:' in the module declaration below.

RESOURCES:	
Heap Memory (>2K).

REFERENCES:
In the ISO/IEC 10918-1 JPEG recommandation, see the following items
Annex B, item B.2.4.2 for the Huffman tables specification syntax
Annex C, for the Huffman specification

PROCESSING:
the class CHT_def contains the DEFINITION of one Huffman table (components full description) 
the class CHT_head contains the HEADER of one Huffman table (specification of the table)
the class CHT_all contains the informations needed to manage all the Huffman tables
for the application (max.4)

DATA:
See 'DATA:' in the class header below.

LOGIC:		
-

*******************************************************************************/

#pragma warning(disable: 4275)

#include <iostream>
#include <fstream>
#include <vector>

#include "CompressJPEG.h"

#include "Bitlevel.h"
#include "CBlock.h"
#include "CBuffer.h"


namespace COMP
{


class CHT_def
{

protected:

// DATA:

	unsigned int m_nbtot;					// total number of Huffman codes
	unsigned __int8 m_Hsi[256];				// HUFFSI array
	unsigned __int16 m_Hco[256];			// HUFFCO array
	std::vector<unsigned __int8>  m_LutSiz;	// LUT for fast coding/decoding (symbol size)
	std::vector<unsigned __int8>  m_LutVal;	// LUT for fast decoding (symbol value)
	std::vector<unsigned __int32> m_LutCod;	// LUT for fast coding (symbol code)

public:

// INTERFACES:

	// Description:	Constructor.
	// Returns:		Nothing.
	CHT_def();

	// friend classes declarations
	friend class CHT_all;
	friend class CHCoder;
	friend class CHDecoder;
};


class CHT_head
{

protected:

// DATA:

	unsigned int	m_nbtot;	// total number of Huffman codes
	CHuffmanTable	m_HT;		// Huffman table definition

public:

// INTERFACE:

	// Description:	Constructor.
	// Returns:		Nothing.
	CHT_head();

	// friend classes declarations
	friend class CHT_all;
	friend class CHCoder;
	friend class CHDecoder;
};


class CHT_all
{

protected:

// DATA:

	bool		m_defined[4];	// DC0, DC1, AC0, AC1
	CHT_head	m_head[4];		// tables headers
	CHT_def		m_code[4];		// coding tables definitions

private:

// INTERFACES:

	// Description:	Compute the LUT for fast huffman coding/decoding.
	// Return:		Nothing.
	void compute_lut
	(
		unsigned int i_index,	// index of the Huffman table [0,4] 
		bool i_for_compression	// true if we must build a LUT for compression purposes
								// false otherwise
	);

public:

// INTERFACE:

	// Description:	Default constructor.
	// Returns:		Nothing.
	CHT_all();

	// Description:	Constructs the complete Huffman table object
	//				with the specified definitions.
	// Returns:		Nothing.
	CHT_all
	(
		const CJPEGParams&	i_params	// compression parameters
	);

	// Description:	Reads all the huffman tables of the segment,
	//				and puts them into the CHT_all object.
	// Returns:		true in case of success, false otherwise.
	bool read_HT_from_header
	(
		CRBuffer&			R			// input buffer
	);

	// Description:	Write the DC and AC huffman tables of the CHT_all object.
	// Returns:		Nothing.
	void write_HT_to_header 
	(
		CWBuffer&						o_buf,	// output buffer
		CJPEGParams::ECompressionMode	i_Mode  // true if Lossy JPEG, otherwise Lossless
	);

	// Description: Initialises the internal structure using the provided parameters.
	// Returns:		Nothing.
	void setParam
	(
		const CJPEGParams& i_param	//	compression parameters frow which to initialise the 
									//	Huffman decoder
	);

	// Description:	Takes the header arrays from the CHT_all object
	//				and makes an Huffman table (code arrays)
	// Returns:		Nothing.
	void calc_table
	(
		unsigned int	i_index,			// index of the table
		bool			i_for_compression	// true  if we must build a Huffman table
											//       for compression purposes
											// false otherwise
	);

};


} // end namespace


#endif
