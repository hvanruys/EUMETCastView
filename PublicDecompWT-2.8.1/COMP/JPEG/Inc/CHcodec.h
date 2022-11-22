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

#ifndef CHcodec_included
#define CHcodec_included

/*******************************************************************************

TYPE:
Concrete classes
					
PURPOSE:
Management of the Huffman coding and decoding process 
for both JPEG lossy and lossless modes 

FUNCTION:
The objects contain the Huffman tables and the input or output buffer.
So it is possible to perform in the same object ;
- for CHCoder : the coding of the coefficients and the writing into the output buffer
- for CHDecoder : the reading of the binary string into the input buffer and its decoding

NB : construction of the CHDecoder object 
The constructor only builds the CRBuffer sub-object, leaving the CHT_all sub-object
empty (use the default constructor).
As soon as the decompression parameters are known (after the JPEG header was read), 
the set_Huffman_tables method must be called to put the real values in the CHT_all 
sub-object.  


INTERFACES:
See 'INTERFACES:' in the module declaration below.

RESOURCES:	
Heap Memory (>2K).

REFERENCES:
In the ISO/IEC 10918-1 JPEG recommandation, see the following items
Annex F, item F.1.1.5 for the AC/DC coefficients encoding (lossy mode)
Annex H, item H.1.1.2 for the differences encoding (lossless mode)

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
#include <memory>

#include "CompressJPEG.h"
#include "CImage.h"
#include "CBlock.h"
#include "CBuffer.h"
#include "CHufftables.h"


namespace COMP
{


class CHCoder : public COMP::CHT_all
{

private:

// DATA:

	unsigned int		m_index_AC;		// index of the AC coding table 
	unsigned int		m_index_DC;		// index of the DC coding table
	CHT_def*			m_pdefAC;		//  Pointers used only to increase performance
	CHT_def*			m_pdefDC;		//	by skipping indirections
	unsigned __int8*	m_pLutSizAC;	//  Pointers used only to increase performance
	unsigned __int32*	m_pLutCodAC;	//  by skipping indirections
	unsigned __int8*	m_pLutSizDC;	//  Pointers used only to increase performance
	unsigned __int32*	m_pLutCodDC;	//  by skipping indirections
	CWBuffer&			m_buf;			// reference to compressed data buffer 

public:

// INTERFACES:

	// Description:	Constructor.
	// Returns:		Nothing.
	CHCoder
	(
		const CJPEGParams&	i_params,	// gets the Huffman tables header to build CHT_all
		CWBuffer&			i_buf		// pre-allocated output buffer
	)
	: CHT_all(i_params)
	, m_buf(i_buf)
	{
	}

	// Description:	Destructor.
	// Returns:		Nothing.
	~CHCoder()
	{
	}

	// Description:	Set the correct values for the indexes of the Huffman tables
	//				to be used in the coding process.
	// Returns:		Nothing.
	void use_these_HT
	(
		int i_index_DC,	// DC Huffman table index
		int i_index_AC	// AC Huffman table index
	);

	// Description:	Encodes a difference or a DC coefficient a specified number of times.
	// Returns:		Nothing.
	inline void encode_DIFF
	(
		const short& i_diff	// difference to be coded
	)
	{
		COMP_TRYTHIS_SPEED
		// writes into the output buffer the binary string which encodes the (diff) symbol
		unsigned long coef = i_diff + 32768L;
		unsigned int size = m_pLutSizDC[coef];
		// is the symbol defined in the Huffman table ?
		Assert (size, Util::CParamException());
		m_buf.write (m_pLutCodDC[coef], size);
		COMP_CATCHTHIS_SPEED
	}

	// Description:	Encodes a difference or a DC coefficient.
	// Returns:		Nothing.
	inline void encode_R_AC
	(
		const unsigned int&	i_r,	// 0 runlength to be coded
		const short&		i_ac	// AC coefficient to be coded
	)
	{
		COMP_TRYTHIS_SPEED
		// writes into the output buffer the binary string which encodes the (r ac) symbol
		unsigned int ssss = speed_csize (i_ac);
		unsigned int rs = (i_r << 4) | ssss;
		unsigned int size = m_pdefAC->m_Hsi[rs];
		// is the symbol defined in the Huffman table ?
		Assert (size, Util::CParamException());
		unsigned __int32 code = m_pdefAC->m_Hco[rs];
		if (ssss)
		{
			// coding AC as described in the norm...
			code <<= ssss;
			code |= (i_ac < 0 ? i_ac - 1 : i_ac) & speed_mask16_lsb (ssss); // ssss low order bits of ac
			// prefixed by the Huffman code
			size += ssss;
		}
		m_buf.write (code, size); 
		COMP_CATCHTHIS_SPEED
	}

	// Description:	Encodes an AC coefficient.
	// Returns:		Nothing.
	inline void encode_AC
	(
		const short &i_ac		// AC coefficient to be coded
	)
	{
		COMP_TRYTHIS_SPEED
		// writes into the output buffer the binary string which encodes the (0 ac) symbol
		unsigned long coef = i_ac + 32768L;
		unsigned int size = m_pLutSizAC[coef];
		// is the symbol defined in the Huffman table ?
		Assert (size, Util::CParamException());
		m_buf.write (m_pLutCodAC[coef], size);
		COMP_CATCHTHIS_SPEED
	}

};


class CHDecoder : public COMP::CHT_all
{

private:

// DATA:

	unsigned int		m_index_AC;		// index of the AC decoding table 
	unsigned int		m_index_DC;		// index of the DC decoding table
	CHT_def*			m_pdefAC;		//  only to increase performances
	CHT_def*			m_pdefDC;		//	by skipping indirections
	unsigned __int8*	m_pLutSizAC;	//  Pointers used only to increase performance
	unsigned __int8*	m_pLutValAC;	//  by skipping indirections
	unsigned __int8*	m_pLutSizDC;	//  Pointers used only to increase performance
	unsigned __int8*	m_pLutValDC;	//  by skipping indirections
	CRBuffer&			m_buf;			// reference to compressed data buffer

public:

// INTERFACES :

	// Description:	Constructor.
	// Returns:		Nothing.
	CHDecoder
	(
		CRBuffer& i_buf		// JPEG compressed image buffer
	)
	: m_buf (i_buf)
	{
	}

	// Description:	Destructor.
	// Returns:		Nothing.
	~CHDecoder()
	{
	}
	
	// Description:	Set the correct values for the indexes of the Huffman tables
	//				to be used in the decoding process
	// Returns:		Nothing.
	void use_these_HT
	(
		int i_index_DC,		// DC Huffman table index
		int i_index_AC		// AC Huffman table index
	);
	
	// Description:
	// Returns:		true if the binary string was successfully read and decoded.
	 bool decode_DIFF
	(
		short& o_diff		// decoded difference
	);

	// Description:
	// Returns:		true if the binary string was successfully read and decoded.
	inline bool decode_R_AC
	(
		unsigned int& o_r,	// decoded 0 runlength
		short& o_ac			// decoded AC parameter
	)
	{
		COMP_TRYTHIS_SPEED
	// WARNING !!!
	// This function need a LEFT aligned binary string !!!!!
	// puts in r and ac the values encoded in the left aligned binary string str_bin

	// 0° Extract the binary string from the input buffer
		unsigned __int32 str_bin = m_buf.readN (16);

	// 1° Decode the symbol
		unsigned int size = m_pLutSizAC[str_bin];
		// is this a valid bit string for the Huffman table ?
		if (! size)
		{
#ifdef _DEBUG
			std::cerr << "No symbol found !" << std::endl;
#endif
			return false;
		}
		unsigned int code = m_pLutValAC[str_bin];

	// 2° Extract R and AC
		if (code)
		{
			o_r = code >> 4;
			code &= 0x0F;
			if ((size += code) > 16) o_ac = (short)(m_buf.read32 () >> (32 - size));
			else o_ac = (short)(str_bin >> (16 - size));
			if (! (o_ac & speed_bit16 (code)))
				// We have a negative AC
			{
				o_ac |= speed_mask16_msb (16 - code);
				o_ac++;
			}
			else o_ac &= speed_mask16_lsb (code);
		}
		else
		{
			o_r = 0;
			o_ac = 0;
		}
		
		// Testing for marker 
		if (m_buf.in_marker (size))
		{
#ifdef _DEBUG
			std::cerr << "Oops: reading a marker !" << std::endl;
#endif
			return false;
		}
		m_buf.seek (size);
		return true;
		COMP_CATCHTHIS_SPEED
	}

};


} // end namespace


#endif
