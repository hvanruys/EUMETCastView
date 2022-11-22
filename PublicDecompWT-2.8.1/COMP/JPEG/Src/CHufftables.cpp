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

//:Ignore
//////////////////////////////////////////////////////////////////////////
//
//	FileName:		CHufftables.cpp
//	
//	Date Created:	21/08/1998
//
//	Author:			Van Wynsberghe Laurent 
//
//
//	Description:	contains the classes for Huffman Tables definition and specification
//				
//
//	Last Modified:	$Dmae: 1999/05/28 14:37:44 $
//
//  RCS Id:			$Id: CHufftables.cpp,v 1.46 1999/05/28 14:37:44 youag Exp $
//
//
////////////////////////////////////////////////////////////////////////////  
//:End Ignore

#define NOMINMAX

#pragma warning(disable: 4275)
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>

#include "ErrorHandling.h"
#include "CHufftables.h"
#include "JPEGConst.h"

namespace COMP {

// CLASS CHT_def
CHT_def::CHT_def (void)
{
	COMP_TRYTHIS
	m_nbtot = 0;
	COMP_CATCHTHIS
}

// CLASS CHT_head
CHT_head::CHT_head (void)
{
	COMP_TRYTHIS
	m_nbtot = 0;
	COMP_CATCHTHIS
}

// CLASS CHT_all
CHT_all::CHT_all (void)
{
	COMP_TRYTHIS
	for (int i=0 ; i<4 ; i++) m_defined[i] = false;
	COMP_CATCHTHIS
}

CHT_all::CHT_all (const CJPEGParams &i_params)
{
	COMP_TRYTHIS
	for (int i=0 ; i<4 ; i++) m_defined[i] = false;
	setParam (i_params);
	COMP_CATCHTHIS
}

void CHT_all::calc_table (unsigned int i_index, bool i_for_compression)
{
	COMP_TRYTHIS
	// build the Huffman table with the given parameters 

	// Index out of range
	Assert (i_index < 4, Util::CParamException());
	CHT_head &phead = m_head[i_index];
	CHT_def &pcode = m_code[i_index];
	pcode.m_nbtot = phead.m_nbtot;

	unsigned int nbBits = 1;
	unsigned int nbSymb = 0;
	unsigned int k = 0;
	do
		if (nbSymb < phead.m_HT.m_bits[nbBits])
		{
			pcode.m_Hsi[k++] = nbBits;
			nbSymb++;
		}
		else
		{
			nbBits++;
			nbSymb = 0;
		}	
	while (nbBits <= 16);
	Assert (k == pcode.m_nbtot, Util::CParamException ());
	for (; k<256 ; k++)
		pcode.m_Hsi[k] = 0;
	
	if (pcode.m_nbtot > 0)
	{
		k = 0;
		unsigned __int16 code = 0;
		unsigned int size = pcode.m_Hsi[0];
		do
		{
			do
				pcode.m_Hco[k++] = code++;
			while (code < (1 << size) && k < pcode.m_nbtot && pcode.m_Hsi[k] == size);
			// The number of codes of the specified size is too big
			Assert (code <= (1 << size), Util::CParamException());
			Assert (pcode.m_Hsi[k] != size, Util::CParamException());
			if (k < pcode.m_nbtot && pcode.m_Hsi[k] != 0)
				do
					code <<= 1;
				while (pcode.m_Hsi[k] != ++size);
		}
		while (k < pcode.m_nbtot && pcode.m_Hsi[k] != 0);	
	}

	// Codes swapping for coding_only tables
	if (i_for_compression)
	{
		unsigned __int8  Hsi[256];
		unsigned __int16 Hco[256];
		for (k=0 ; k<256 ; k++)
		{
			Hco[k] = 0;
			Hsi[k] = 0;
		}
		for (k=0 ; k<pcode.m_nbtot ; k++)
		{
			unsigned int i = phead.m_HT.m_symbols[k];
			Hco[i] = pcode.m_Hco[k];
			Hsi[i] = pcode.m_Hsi[k];
		}
		// copy the arrays
		for (k=0 ; k<256 ; k++)
		{
			pcode.m_Hco[k] = Hco[k];
			pcode.m_Hsi[k] = Hsi[k];
		}
	}
	// compute LUT for fast Huffman coding / decoding
	compute_lut (i_index, i_for_compression);
	COMP_CATCHTHIS
}

void CHT_all::setParam (const CJPEGParams &i_params)
{
	COMP_TRYTHIS
	// takes the headers vector and converts into usable Huffman AC and DC tables
	// the CHT_all structure must be allocated outside of the function (before call)
	unsigned int i;
	const unsigned int index_DC = 0;
	const unsigned int index_AC = 2;
	CHT_head &phead_DC = m_head[index_DC];
	CHT_head &phead_AC = m_head[index_AC];
	
	phead_DC.m_nbtot = 0;
	phead_AC.m_nbtot = 0;

	// update BITS arrays
	for (i=1 ; i<=16 ; i++)
	{
		phead_DC.m_nbtot += (phead_DC.m_HT.m_bits[i] = i_params.m_dcTable.m_bits[i]);
		phead_AC.m_nbtot += (phead_AC.m_HT.m_bits[i] = i_params.m_acTable.m_bits[i]);
	}

	Assert (phead_DC.m_nbtot <= 256 && phead_AC.m_nbtot <= 256, Util::CParamException ());
	// update HUFFVAL arrays
	for (i=0 ; i<phead_DC.m_nbtot ; i++)
		phead_DC.m_HT.m_symbols[i] = i_params.m_dcTable.m_symbols[i];
	for (i=0 ; i<phead_AC.m_nbtot ; i++)
		phead_AC.m_HT.m_symbols[i] = i_params.m_acTable.m_symbols[i];
	
	m_defined[index_DC] = true;
	m_defined[index_AC] = true;
	COMP_CATCHTHIS
}

bool CHT_all::read_HT_from_header (CRBuffer &i_buf)
{
	COMP_TRYTHIS
	// the DHT marker has been read just before calling

	// DHT, lg in bytes
	unsigned int lg = i_buf.readN (16);
	i_buf.real_seek (16);
	unsigned int nbBytes = 2;
	// loop over the Huffman Tables
	while (true) 
	{
		unsigned int i, current_HT;

		// DHT, TcTh 
		unsigned __int32 str_bin = i_buf.readN (16);
		if (str_bin > 0xFF00)
			// Finished the DHT segment, find marker for new segment
			break;
		i_buf.real_seek (8);
		nbBytes++;
		switch (str_bin >> 8)
		{
			case 0x00 : 
				current_HT = 0;
				break;
			case 0x01 :
				current_HT = 1;
				break;
			case 0x10 :
				current_HT = 2;
				break;
			case 0x11 :
				current_HT = 3;
				break;
			default   :	
				{
				return false; // bad values for TcTh(DC) in DHT segment
				}
		}
		CHT_head &phead = m_head[current_HT];
		// DHT, bits vector
		phead.m_nbtot = 0;
		for (i=1 ; i<=16 ; i++)
		{
			phead.m_nbtot += (phead.m_HT.m_bits[i] = (unsigned short)i_buf.readN(8));
			i_buf.real_seek (8);
			nbBytes++;
		}
		Assert (phead.m_nbtot <= 256, Util::CParamException ());
		// DHT, symbols vector
		for (i=0 ; i<phead.m_nbtot ; i++)
		{
			phead.m_HT.m_symbols[i] = (unsigned short)i_buf.readN(8);
			i_buf.real_seek (8);
			nbBytes++;
		}
		m_defined[current_HT] = true;
	}
	// Testing the length of the Huffman segment
	if (lg != nbBytes)
	{
		return false; // Bad length for DHT segment
	}
	return true;
	COMP_CATCHTHIS
}

void CHT_all::write_HT_to_header (CWBuffer &o_buf, CJPEGParams::ECompressionMode i_mode)
{
	COMP_TRYTHIS
	unsigned int i;
	const unsigned int index_DC = 0;
	const unsigned int index_AC = 2;

	o_buf.write_marker (cMarkerDHT);
	// DHT, length in bytes
	if (i_mode == CJPEGParams::e_LosslessJPEG)
	{
		Assert (m_defined[index_DC], Util::CParamException ());
		unsigned int nbBytes = 2 + 1 + 16 + m_head[index_DC].m_nbtot;
		o_buf.real_write (nbBytes, 16);
	}
	else
	{
		Assert (m_defined[index_DC] && m_defined[index_AC], Util::CParamException ());
		unsigned int nbBytes = 2 + 1 + 16 + m_head[index_DC].m_nbtot + 1 + 16 + m_head[index_AC].m_nbtot;
		o_buf.real_write (nbBytes, 16);
	}
	
	if (i_mode == CJPEGParams::e_LosslessJPEG || i_mode == CJPEGParams::e_LossyJPEG)
	{
		// DHT, TcTh
		o_buf.real_write (0, 8);
		// DHT, bits vector
		for (i=1 ; i<=16 ; i++)
			o_buf.real_write (m_head[index_DC].m_HT.m_bits[i], 8);
		// DHT, symbols vector
		for (i=0 ; i<m_head[index_DC].m_nbtot ; i++)
			o_buf.real_write (m_head[index_DC].m_HT.m_symbols[i], 8);
	}
	if (i_mode == CJPEGParams::e_LossyJPEG)
	{
		// DHT, TcTh AC table
		o_buf.real_write (0x10, 8);
		// DHT, AC bits vector
		for (i=1 ; i<=16 ; i++)
			o_buf.real_write (m_head[index_AC].m_HT.m_bits[i], 8);
		// DHT, AC symbols vector
		for (i=0 ; i<m_head[index_AC].m_nbtot ; i++)
			o_buf.real_write (m_head[index_AC].m_HT.m_symbols[i], 8);
	}
	COMP_CATCHTHIS
}

void CHT_all::compute_lut (unsigned int i_index, bool i_for_compression)
{
	COMP_TRYTHIS
	Assert (i_index < 4, Util::CParamException());
	CHT_head &phead = m_head[i_index];
	CHT_def &pcode = m_code[i_index];
	if (i_for_compression)  
	// LUT for coding
	{
		pcode.m_LutSiz = std::vector<unsigned __int8 >(65536, 0);
		pcode.m_LutCod = std::vector<unsigned __int32>(65536, 0);
		//XXX notice: coef should range from -32767 to 32768, but this
		//    is quite unhandy given the 16 bit representation. 
		//    but luckily, 32768 == -32768
		//    The conclusion is that the coefs will range from -32768 to 32767.
		// coef == 0 -> ssss == 0
		unsigned int size = pcode.m_Hsi[0];
		Assert (size >= 0 && size <= 16, Util::CParamException ());
		pcode.m_LutSiz[32768L] = size;
		if (size) pcode.m_LutCod[32768L] = pcode.m_Hco[0];
		for (unsigned int ssss=1 ; ssss<=15 ; ssss++)
		{
			unsigned int size = pcode.m_Hsi[ssss];
			Assert (size >= 0 && size <= 16, Util::CParamException ());
			int nbCoef = 1 << (ssss - 1);
			if (size)
			{
				size += ssss;
				unsigned __int32 code = pcode.m_Hco[ssss];
				code <<= ssss;
				unsigned int mask = (1 << ssss) - 1;
				for (int coef=nbCoef-1 ; coef>=0 ; coef--)
				{
					// Positive coef
					pcode.m_LutSiz[(nbCoef + coef) + 32768L] = size;
					pcode.m_LutCod[(nbCoef + coef) + 32768L] = code | (nbCoef + coef);
					// Negative coef
					pcode.m_LutSiz[32768L - (nbCoef + coef)] = size;
					pcode.m_LutCod[32768L - (nbCoef + coef)] = code | ((-(nbCoef + coef) - 1) & mask);
				}
			}
			else
				for (int coef=nbCoef-1 ; coef>=0 ; coef--)
				{
					pcode.m_LutSiz[(nbCoef + coef) + 32768L] = 0;		// Positive coef
					pcode.m_LutSiz[32768L - (nbCoef + coef)] = 0;	// Negative coef
				}
		}
		// coef == +-32768 -> ssss == 16
		size = pcode.m_Hsi[16];
		Assert (size >= 0 && size <= 16, Util::CParamException ());
		pcode.m_LutSiz[0] = size;
		if (size) pcode.m_LutCod[0] = pcode.m_Hco[16];
	}
	else 
	// LUT for decoding
	{
		pcode.m_LutSiz = std::vector<unsigned __int8>(65536, 0);
		pcode.m_LutVal = std::vector<unsigned __int8>(65536, 0);
		if (pcode.m_nbtot > 0)
		{
			for (unsigned int j = 0; j < pcode.m_nbtot; j++)
			{
				unsigned __int16 size = pcode.m_Hsi[j];
				Assert (size >= 0 && size <= 16, Util::CParamException ());
				if (size)
				{
					unsigned __int16 code = pcode.m_Hco[j];
					unsigned __int8 val = (unsigned __int8)phead.m_HT.m_symbols[j];
					unsigned __int16 base = code << (16 - size);
					unsigned __int16 offs = 1 << (16 - size);
					for (int k=offs-1 ; k>=0 ; k--)
					{
						Assert (pcode.m_LutSiz[base + k] == 0, Util::CParamException ());
						pcode.m_LutSiz[base + k] = (unsigned __int8)size;
						pcode.m_LutVal[base + k] = val;
					}
				}
			}
		}
	}
	COMP_CATCHTHIS
}

}  // end namespace
