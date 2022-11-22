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
//	FileName:		CHOptim.cpp
//	
//	Date Created:	1998/10/23
//
//	Author:			Ouaghli Youssef
//
//
//	Description:	The Huffman table optimization
//
//
//	Last Modified:	$Dmae: 1999/03/02 11:13:44 $
//
//  RCS Id:			$Id: CHOptim.cpp,v 1.13 1999/05/23 17:43:41 xne Exp $
//
//	$Rma: CHOptim.cpp,v $
//	Revision 1.8  1999/03/02 11:13:44  xne
//	Resolved Point transform/Requantization problem in optiwrapper.cpp
//
//////////////////////////////////////////////////////////////////////// 
//:End Ignore

#include "ErrorHandling.h"
#include "CHOptim.h"

namespace COMP {

CHOptim::CHOptim (CJPEGParams &i_param) :
m_param (i_param), 
m_cqtbl (i_param),
m_default_value( 0 )
{
	COMP_TRYTHIS
	init ();
	COMP_CATCHTHIS
}

void CHOptim::init (void)
{
	COMP_TRYTHIS
	int i;
	for (i=0 ; i<17 ; i++)
		m_freqDC[i] = 0;
	for (i=0 ; i<256 ; i++)
		m_freqAC[i] = 0;
	COMP_CATCHTHIS
}

void CHOptim::accumulateFrequenciesNextLine (CImage &i_image,
											 unsigned short i_cur_line,
										     unsigned short i_predictor0,
										     unsigned short i_predictor)
{
	COMP_TRYTHIS
	unsigned long base_MCU = i_cur_line * i_image.GetW();
	unsigned short *p0 = i_image.Get() + base_MCU;
	unsigned short *pB = i_image.Get() + base_MCU - i_image.GetW();
	__int32 v0, vA, vB, vC;
	unsigned short cur_MCU_in_line;
	unsigned short imgW = i_image.GetW();
	
#ifdef DEBUG
	Assert (i_cur_line < i_image.GetH(), Util::CParamException ());
#endif
	vB = i_cur_line ? *pB : 0;
	switch (i_predictor0)
	{
	case 0 :
		m_freqDC[speed_csize ((v0 = *p0++) - m_default_value)] ++;
		break;
	case 1 :
	case 3 :
		m_freqDC[speed_csize ((v0 = *p0++))] ++;
		break;
	case 2 :
	case 4 :
	case 6 :
		m_freqDC[speed_csize ((v0 = *p0++) - vB)] ++;
		break;
	case 5 :
	case 7 :
		m_freqDC[speed_csize ((v0 = *p0++) - (vB >> 1))] ++;
		break;
	default :
		Assert (0, Util::CParamException());
	}

	// now, loop on all other MCUs of the line...
	switch (i_predictor)
	{
	case 0 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
			m_freqDC[speed_csize (*p0++ - m_default_value)] ++;
		break;
	case 1 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
		{
			vA = v0;
			m_freqDC[speed_csize ((v0 = *p0++) - vA)] ++;
		}
		break;
	case 2 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
			m_freqDC[speed_csize (*p0++ - *++pB)] ++;
		break;
	case 3 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
			m_freqDC[speed_csize (*p0++ - *pB++)] ++;
		break;
	case 4 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
		{
			vA = v0;
			vC = vB;
			m_freqDC[speed_csize ((v0 = *p0++) - vA - ((vB = *++pB) - vC))] ++;
		}
		break;
	case 5 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
		{
			vA = v0;
			vC = vB;
			m_freqDC[speed_csize ((v0 = *p0++) - vA - (((vB = *++pB) - vC) >> 1))] ++;
		}
		break;
	case 6 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
		{
			vA = v0;
			vC = vB;
			m_freqDC[speed_csize ((v0 = *p0++) - (vB = *++pB) - ((vA - vC) >> 1))] ++;
		}
		break;
	case 7 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
		{
			vA = v0;
			m_freqDC[speed_csize ((v0 = *p0++) - ((vA + *++pB) >> 1))] ++;
		}
		break;
	default : 
		Assert (0, Util::CParamException());
	}
	COMP_CATCHTHIS
}

void CHOptim::accumulateFrequenciesLossless (CImage &i_image)
{
	COMP_TRYTHIS
	// Warning: Point transform of the image must be already done (if needed)
	// 
	// loop over all the lines in the image
	for (unsigned short cur_line=0 ; cur_line<i_image.GetH() ;)
	{
		// loop on the lines inside one restart interval
		accumulateFrequenciesNextLine (i_image, cur_line++, 0, 1);
		for (unsigned short cur_line_in_RI = 1;
		       ((m_param.m_RestartInterval ==0)||
				(cur_line_in_RI<m_param.m_RestartInterval)) &&
				cur_line<i_image.GetH() ; cur_line_in_RI++)
			accumulateFrequenciesNextLine (i_image, cur_line++, 2, m_param.m_Predictor);
	}
	COMP_CATCHTHIS
}

void CHOptim::accumulateFrequenciesBlock (CJBlock<short> &i_blk)
{
	COMP_TRYTHIS
	// Processing the DC coefficient
	m_freqDC[speed_csize (i_blk.Cget (0))] ++;
	// Processing the AC coefficients
	unsigned int r = 0;
	for  (unsigned int k=1 ; k<64 ; k++)
	{
		short coeff = i_blk.CZget (k);
		if (coeff == 0) r++;
		else
		{
			if (r == 0)
				m_freqAC[speed_csize (coeff)] ++;
			else
			{
				// encode the run-length of 16 zero coefficients
				while (r > 15)
				{
					// put "F0" byte
					m_freqAC[0xF0] ++;
					r -= 16;
				}
				// encode R AC
				m_freqAC[(r << 4) | speed_csize (coeff)] ++;
				r = 0;
			}
		}
	}
	// if the last coefficients are 0, encode and EOB byte.
	if (r)
		m_freqAC[0x00] ++;
	COMP_CATCHTHIS
}

void CHOptim::accumulateFrequenciesLossy (CImage &i_image)
{
	COMP_TRYTHIS
	// XXX: Requantization of the image must be already done (if needed)
	unsigned int shift;

	// Compute the "shift" value
	switch (i_image.GetNB())
	{
	case 8 : 
		shift = 128;
		break;
	case 12 : 
		shift = 2048;
		break;
	default: 
		Assert (0, Util::CParamException ());
	}
	// Perform the actual coding
	CJBlock <unsigned short> pixels_block;
	CJBlock <short> shifted_block;
	CJBlock <double> FDCT_block;
	CJBlock <short> Qblock;

	unsigned long	cur_MCU = 0;
	unsigned short	corner_line	= 0;
	unsigned short	corner_column = 0;
	short			last_DC = 0;
	bool			stop = false;

	while (! stop)
	{
		// Get current block
		i_image.get_block (pixels_block, corner_column, corner_line);
		// Perform the level shift
		pixels_block.level_shift (shifted_block, shift);
		// Compute the FDCT of the block
		shifted_block.forward_DCT (FDCT_block);
		// Perform block quantization
		m_cqtbl.Quantize_block (FDCT_block, Qblock);
		// Perform block differential shift
		Qblock.differential_shift (last_DC);
		// Perform Huffman coding
		accumulateFrequenciesBlock (Qblock);
		
		// Compute new Upper Left corner
		corner_column += 8;
		if (corner_column >= i_image.GetW())
		{
			corner_column =  0;
			corner_line   += 8;
			if (corner_line >= i_image.GetH()) stop = true;
		}
		cur_MCU ++;
		// Write a restart marker if needed...
		if (m_param.m_RestartInterval && (cur_MCU % m_param.m_RestartInterval) == 0)
			// Reset the prediction for the DC coefficient.
			last_DC = 0;
	}
	COMP_CATCHTHIS
}

void CHOptim::accumulateFrequencies (CImage &i_image)
{
	COMP_TRYTHIS
	switch (m_param.m_Mode)
	{
	case CJPEGParams::e_LosslessJPEG:
		m_default_value = 1 << (m_param.m_BitsPerPixel - m_param.m_PointTransform - 1);
		accumulateFrequenciesLossless (i_image);
		break;
	case CJPEGParams::e_LossyJPEG:
		accumulateFrequenciesLossy (i_image);
		break;
	default:
		Assert (0, Util::CParamException ());
	}
	COMP_CATCHTHIS
}

bool CHOptim::recurseTree (unsigned int i_start, unsigned int i_stop, unsigned int i_level,
						   unsigned __int32 *i_freq, CHuffmanTable &o_ht)
{
	COMP_TRYTHIS
	const unsigned int c_levelMax = 16;
	
	if (i_start == i_stop) // we have a leaf of the tree
		o_ht.m_bits[i_level]++;
	else // recurse until leaf
		if (i_level < c_levelMax)
		{
			unsigned int k;

			unsigned __int32 f = i_freq[i_start];
			for (k=i_start+1 ; k<=i_stop ; k++)
				if (f != i_freq[k]) break;
			if (k == i_stop + 1)
				// All remaining symbols have same frequency, so use a shortcut
				if ((k - i_start) <= (unsigned int)(1 << (c_levelMax - i_level)))
				{
					o_ht.m_bits[i_level + speed_csize (i_stop - i_start)] += k - i_start;
					return true;
				}
			// recurse until leaf
			unsigned __int32 sumLeft = i_freq[i_start];
			unsigned __int32 sumRight = 0;
			for (k=i_start+1 ; k<=i_stop ; k++) sumRight += i_freq[k];
			// Search best splitting 
			for (k=i_start+1 ; sumLeft<sumRight ; k++)
			{
				sumLeft += i_freq[k];
				sumRight -= i_freq[k];
			}
			// Adapt the splitting to avoid problems
			unsigned __int32 nbNodes = 1 << (c_levelMax - i_level - 1);
			while ((i_stop - k + 1) > nbNodes) k++;
			if (k > i_stop || (k - i_start) > nbNodes) return false;
			// Two recursive calls, one for each branch of the subtree
			if (! recurseTree (i_start, k - 1, i_level + 1, i_freq, o_ht))
				return false;
			if (! recurseTree (k, i_stop, i_level + 1, i_freq, o_ht))
				return false;
		}
		else  // No more bits are available
			return false;
	return true;
	COMP_CATCHTHIS
}

bool CHOptim::computeOptimalTable (bool i_full, bool i_DC, CHuffmanTable &o_ht, unsigned int &o_nbSymb)
{
	COMP_TRYTHIS
	unsigned __int32 *freq = i_DC ? m_freqDC : m_freqAC;
	// Sorting the frequencies and associated symbols
	bool bSwap = true;
	while (bSwap)
	{
		bSwap = false;
		for (unsigned int i=0; i < (o_nbSymb-1); i++)
			if (freq[i+1] > freq[i])
			{
				unsigned __int32 sF = freq[i];
				freq[i] = freq[i+1];
				freq[i+1] = sF;
				unsigned short sS = o_ht.m_symbols[i];
				o_ht.m_symbols[i] = o_ht.m_symbols[i+1];
				o_ht.m_symbols[i+1] = sS;
				bSwap = true;
			}
	}

	// Compute the optimal BITS vector
	for (int i=0 ; i<=16 ; i++) o_ht.m_bits[i] = 0;
	if (! i_full)
		while (o_nbSymb > 1 && freq[o_nbSymb-1] == 0) o_nbSymb--;
	if (o_nbSymb <= 1) // Only one symbol -> uniform image
		o_ht.m_bits[1] = 1;
	else
		if (! recurseTree (0, o_nbSymb-1, 0, freq, o_ht)) return false;
	return true;
	COMP_CATCHTHIS
}

bool CHOptim::computeOptimalHuffmanTables (bool i_full)
{
	COMP_TRYTHIS
	switch (m_param.m_Mode)
	{
	case CJPEGParams::e_LosslessJPEG:
		{
			for (int i=0 ; i<17 ; i++)
				m_param.m_dcTable.m_symbols[i] = i;
			unsigned int nbSymbDC = 17;
			if (! computeOptimalTable (i_full, true, m_param.m_dcTable, nbSymbDC)) return false;
			break;
		}
	case CJPEGParams::e_LossyJPEG:
		{
			int i;
			for (i=0 ; i<16 ; i++)
				m_param.m_dcTable.m_symbols[i] = i;
 			unsigned int nbSymbDC = 16;
			if (! computeOptimalTable (i_full, true, m_param.m_dcTable, nbSymbDC)) return false;
			for (i=0 ; i<256 ; i++)
				m_param.m_acTable.m_symbols[i] = i;
			unsigned int nbSymbAC = 256;
			if (! computeOptimalTable (i_full, false, m_param.m_acTable, nbSymbAC)) return false;
			break;
		}
	default:
		Assert (0, Util::CParamException ());
	}
	return true;
	COMP_CATCHTHIS
}

} // end namespace COMP
