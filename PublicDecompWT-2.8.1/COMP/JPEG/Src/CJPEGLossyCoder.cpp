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
//	FileName:		CJPEGLossyCoder.cpp
//	
//	Date Created:	10/8/1998
//
//	Author:			Van Wynsberghe Laurent 
//
//
//	Description:	The lossy JPEG coder  --- implementation
//
//
//	Last Modified:	$Dmae: 1999/05/22 15:59:47 $
//
//  RCS Id:			$Id: CJPEGLossyCoder.cpp,v 1.35 1999/05/22 15:59:47 xne Exp $
//////////////////////////////////////////////////////////////////////////// 
//:End Ignore

#include "ErrorHandling.h"
#include "CJPEGLossyCoder.h"
#include "JPEGConst.h"

namespace COMP
{


void CJPEGLossyCoder::WriteHeader (void)
{
	COMP_TRYTHIS
	const unsigned short c_Se = 0x3F;	// Se application value
	// 1° SOI 
	m_buf.write_marker (cMarkerSOI);
	// 2. DQT
	m_cqtbl.write_in_header (m_buf);

	// 3° DRI
	m_buf.write_marker (cMarkerDRI);
	// DRI, lg in bytes
	m_buf.real_write (4, 16);
	// DRI, lg restart interval in MCU
	m_buf.real_write (m_param.m_RestartInterval, 16);
	
	// 4° SOF for LOSSY 
	m_buf.write_marker (cMarkerSOF2);
	// SOF, lg in bytes
	m_buf.real_write (11, 16);
	// SOF, sample precision in bits
	m_buf.real_write (m_Image.GetNB(), 8);
	// SOF, number of lines
	m_buf.real_write (m_Image.GetH(), 16);
	// SOF, number of MCU per line
	m_buf.real_write (m_Image.GetW(), 16);
	// SOF, number of components by frame
	m_buf.real_write (1, 8);
	// SOF, C1
	m_buf.real_write (1, 8);
	// SOF, H1V1
	m_buf.real_write (0x11, 8);
	// SOF, Tq1
	m_buf.real_write (0, 8);
	
	// 5° DHT 
	m_hcoder.write_HT_to_header (m_buf, CJPEGParams::e_LossyJPEG);
	
	// 6° SOS
	m_buf.write_marker (cMarkerSOS);
	// SOS, lg in bytes
	m_buf.real_write (8, 16);
	// SOS, number of components in scan
	m_buf.real_write (1, 8);
	// SOS, Cs1
	m_buf.real_write (1, 8);
	// SOS, DC selector AC selector
	m_buf.real_write (0, 8);
	// SOS, Spectral start 
	m_buf.real_write (0, 8);
	// SOS, Spectral stop
	m_buf.real_write (c_Se, 8); // value 00 or 63
	// SOS, AhAl point transform
	m_buf.real_write (m_param.m_PointTransform, 8);
	COMP_CATCHTHIS
}


CJPEGLossyCoder::CJPEGLossyCoder (const Util::CDataFieldUncompressedImage &i_cdfui,
								  const CJPEGParams &i_param):
	CJPEGCoder (i_cdfui, i_param),
	m_cqtbl (m_param)
{
	COMP_TRYTHIS

	Assert(	m_Image.GetNB() ==  8 ||
			m_Image.GetNB() == 10 ||
			m_Image.GetNB() == 12,
			Util::CNamedException("Cannot compress image with this number of bits per pixel."));

	Assert(	m_Image.GetNB() == m_param.m_BitsPerPixel,
			Util::CNamedException(
				"Number of bits per pixel in compression parameter set and in image do not match."));

	COMP_CATCHTHIS
}


CJPEGLossyCoder::~CJPEGLossyCoder ()
{
}


void CJPEGLossyCoder::EncodeBlock (CJBlock<short> &i_blk)
{
	COMP_TRYTHIS
	// Processing the DC coefficient
	m_hcoder.encode_DIFF (i_blk.Cget (0));
	// Coding the AX coefficients
	unsigned int r = 0;
	for  (unsigned int k=1 ; k<64 ; k++)
	{
		short coeff = i_blk.CZget (k);
		if (coeff == 0) r++;
		else
		{
			if (r == 0) m_hcoder.encode_AC (coeff);
			else
			{
				// encode the run-length of 16 zero coefficients
				while (r > 15)
				{
					// put "F0" byte
					m_hcoder.encode_R_AC (15, 0);
					r -= 16;
				}
				// encode R AC
				m_hcoder.encode_R_AC (r, coeff);
				r = 0;
			}
		}
	}
	// if the last coefficients are 0, encode and EOB byte.
	if (r) m_hcoder.encode_AC (0);
	COMP_CATCHTHIS
}


void CJPEGLossyCoder::CodeBuffer (void)
{
	COMP_TRYTHIS
	unsigned int shift;

	// Requantize the image
	m_Image.Requantize (m_param.m_RequantizationMode);

	// If HT optimization needed
	if (m_param.m_OptimizedCoding)
	{
		CHOptim hopt (m_param);
		hopt.accumulateFrequencies (m_Image);
		if (hopt.computeOptimalHuffmanTables (false))
			m_hcoder.setParam (m_param);
	}
	m_hcoder.use_these_HT (0, 2);
	// Write the JPEG header
	WriteHeader ();
	// Compute the "shift" value
	switch (m_Image.GetNB())
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
	CJBlock <unsigned short>	pixels_block;
	CJBlock <short>			shifted_block;
	CJBlock <double>			FDCT_block;
	CJBlock <short>			Qblock;

	unsigned long	cur_MCU			= 0;
	unsigned long	cur_interval	= 0;
	unsigned short  corner_line		= 0;
	unsigned short  corner_column	= 0;
	short			last_DC			= 0;
	bool			stop			= false;

	while (! stop)
	{
		// Get current block
		m_Image.get_block (pixels_block, corner_column, corner_line);
		// Perform the level shift
		pixels_block.level_shift (shifted_block, shift);
		// Compute the FDCT of the block
		shifted_block.forward_DCT (FDCT_block);
		// Perform block quantization
		m_cqtbl.Quantize_block (FDCT_block, Qblock);
		// Perform block differential shift
		Qblock.differential_shift (last_DC);
		// Perform Huffman coding
		EncodeBlock (Qblock);
		
		// Compute new Upper Left corner
		corner_column += 8;
		if (corner_column >= m_Image.GetW())
		{
			corner_column =  0;
			corner_line   += 8;
			if (corner_line >= m_Image.GetH()) stop = true;
		}
		cur_MCU ++;
		// Write a restart marker if needed...
		if (m_param.m_RestartInterval && 
			   ((cur_MCU % m_param.m_RestartInterval) == 0) && 
			       ! stop)
		{
			m_buf.write_marker ((unsigned short)(cMarkerRST + (cur_interval % 8)));
			cur_interval++;
			// Reset the prediction for the DC coefficient.
			last_DC = 0;
		}
	}
	WriteJPEGFooter ();
	COMP_CATCHTHIS
}


} // end namespace

