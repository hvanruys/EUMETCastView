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
//	FileName:		CJPEGLossLessCoder.cpp
//	
//	Date Created:	10/8/1998
//
//	Author:			Van Wynsberghe Laurent 
//
//
//	Description:	The lossless JPEG coder / decoder functions
//
//
//	Last Modified:	$Dmae: 1999/05/27 08:45:53 $
//
//  RCS Id:			$Id: CJPEGLossLessCoder.cpp,v 1.29 1999/05/27 08:45:53 xne Exp $
//////////////////////////////////////////////////////////////////////// 
//:End Ignore

#include "ErrorHandling.h"
#include "CJPEGLossLessCoder.h"
#include "JPEGConst.h"

namespace COMP
{


void CJPEGLossLessCoder::WriteHeader (void)
{
	COMP_TRYTHIS
	// write the compressed file header into the buffer

	// 1° SOI 
	m_buf.write_marker (cMarkerSOI);
	
	// 2° DRI
	unsigned __int32 real_RI = m_param.m_RestartInterval * m_Image.GetW();
	// The RI is coded on 16 bits, hence max RI = 1<<16 -1 MCU...
	Assert( real_RI < (1<<16), Util::CParamException());
	m_buf.write_marker (cMarkerDRI);
	// DRI, lg in bytes
	m_buf.real_write (4, 16);
	// DRI, lg restart interval in MCU
	m_buf.real_write (real_RI, 16);
	
	// 3° SOF for LOSSLESS 
	m_buf.write_marker (cMarkerSOF3);
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
	
	// 4° DHT
	m_hcoder.write_HT_to_header (m_buf, CJPEGParams::e_LosslessJPEG);
	
	// 5° SOS
	m_buf.write_marker (cMarkerSOS);
	// SOS, lg in bytes
	m_buf.real_write (8, 16);
	// SOS, number of components in scan
	m_buf.real_write (1, 8);
	// SOS, Cs1
	m_buf.real_write (1, 8);
	// SOS, Td1Ta1 (We will always use DC Table #0, because that is the only defined ! )
	m_buf.real_write (0, 8);
	// SOS, Ss predictor
	m_buf.real_write (m_param.m_Predictor, 8);
	// SOS, Se
	m_buf.real_write (0, 8);
	// SOS, AhAl point transform
	m_buf.real_write (m_param.m_PointTransform, 8);
	COMP_CATCHTHIS
}


void CJPEGLossLessCoder::CodeNextLine (unsigned short i_cur_line,
									   unsigned short i_predictor0,
									   unsigned short i_predictor)
{
	COMP_TRYTHIS
	unsigned long base_MCU = i_cur_line * m_Image.GetW();
	unsigned short *p0 = m_Image.Get() + base_MCU;
	unsigned short *pB = m_Image.Get() + base_MCU - m_Image.GetW();
	__int32 v0, vA, vB, vC;
	unsigned short cur_MCU_in_line;
	unsigned short imgW = m_Image.GetW();
	
#ifdef DEBUG
	Assert (i_cur_line < m_Image.GetH(), Util::CParamException ());
#endif
	vB = i_cur_line ? *pB : 0;
	switch (i_predictor0)
	{
	case 0 :
		m_hcoder.encode_DIFF ((v0 = *p0++) - m_default_value);
		break;
	case 1 :
	case 3 :
		m_hcoder.encode_DIFF ((short)(v0 = *p0++));
		break;
	case 2 :
	case 4 :
	case 6 :
		m_hcoder.encode_DIFF ((short)((v0 = *p0++) - vB));
		break;
	case 5 :
	case 7 :
		m_hcoder.encode_DIFF ((short)((v0 = *p0++) - (vB >> 1)));
		break;
	default :
		Assert (0, Util::CParamException());
	}

	// now, loop on all other MCUs of the line...
	switch (i_predictor)
	{
	case 0 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
			m_hcoder.encode_DIFF (*p0++ - m_default_value);
		break;
	case 1 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
		{
			vA = v0;
			m_hcoder.encode_DIFF ((short)((v0 = *p0++) - vA));
		}
		break;
	case 2 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
			m_hcoder.encode_DIFF (*p0++ - *++pB);
		break;
	case 3 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
			m_hcoder.encode_DIFF (*p0++ - *pB++);
		break;
	case 4 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
		{
			vA = v0;
			vC = vB;
			m_hcoder.encode_DIFF ((short)((v0 = *p0++) - vA - ((vB = *++pB) - vC)));
		}
		break;
	case 5 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
		{
			vA = v0;
			vC = vB;
			m_hcoder.encode_DIFF ((short)((v0 = *p0++) - vA - (((vB = *++pB) - vC) >> 1)));
		}
		break;
	case 6 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
		{
			vA = v0;
			vC = vB;
			m_hcoder.encode_DIFF ((short)((v0 = *p0++) - (vB = *++pB) - ((vA - vC) >> 1)));
		}
		break;
	case 7 :
		for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
		{
			vA = v0;
			m_hcoder.encode_DIFF ((v0 = *p0++) - ((vA + *++pB) >> 1));
		}
		break;
	default : 
		Assert (0, Util::CParamException());
	}
	COMP_CATCHTHIS
}


void CJPEGLossLessCoder::DPCM_coder (void)
{
	COMP_TRYTHIS
	unsigned short RSTm = 0;
	// loop over all the lines in the image
	for (unsigned short cur_line=0 ; cur_line<m_Image.GetH() ; RSTm++)
	{
		// loop on the lines inside one restart interval
		CodeNextLine (cur_line++, 0, 1);
		for (unsigned short cur_line_in_RI = 1; 
			    ( (m_param.m_RestartInterval ==0) || 
				  (cur_line_in_RI<m_param.m_RestartInterval)) &&
				cur_line<m_Image.GetH() ; cur_line_in_RI++)
			CodeNextLine (cur_line++, 2, m_param.m_Predictor);
		if (cur_line < m_Image.GetH())
			// Code the Restart marker
			m_buf.write_marker (cMarkerRST + (RSTm % 8));
	}
	COMP_CATCHTHIS
}


CJPEGLossLessCoder::CJPEGLossLessCoder (const Util::CDataFieldUncompressedImage &i_cdfui,
									    const CJPEGParams &i_param):
	CJPEGCoder( i_cdfui, i_param)
{
	COMP_TRYTHIS

	//---
	Assert (m_param.m_BitsPerPixel >= 2 && m_param.m_BitsPerPixel <= 16, Util::CParamException());
	//---
	Assert (m_param.m_Predictor > 0 && m_param.m_Predictor <= 7, Util::CParamException());

	Assert (m_param.m_PointTransform < m_Image.GetNB(),
			Util::CNamedException("Impossible point transform requested."));

	Assert (m_Image.GetNB() == m_param.m_BitsPerPixel,
			Util::CNamedException(
				"Number of bits per pixel in compression parameter set and in image do not match."));

	m_default_value = 1 << (m_Image.GetNB() - m_param.m_PointTransform - 1);

	COMP_CATCHTHIS
}


CJPEGLossLessCoder::~CJPEGLossLessCoder ()
{
}


void CJPEGLossLessCoder::CodeBuffer (void)
{
	COMP_TRYTHIS
	m_Image.Forward_point_transform (m_param.m_PointTransform);
	// If HT optimization needed
	if (m_param.m_OptimizedCoding)
	{
		CHOptim hopt (m_param);
		hopt.accumulateFrequencies (m_Image);
		if (hopt.computeOptimalHuffmanTables (false))
			m_hcoder.setParam (m_param);
	}
	m_hcoder.use_these_HT (0, -1);
	WriteHeader ();
	DPCM_coder ();
	WriteJPEGFooter ();
	COMP_CATCHTHIS
}


} // end namespace

