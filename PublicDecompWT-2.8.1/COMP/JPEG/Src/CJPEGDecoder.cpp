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
//	FileName:		CJPEGDecoder.h
//	
//	Date Created:	10/8/1998
//
//	Author:			Van Wynsberghe Laurent 
//
//
//	Description:	CJPEGCoder class declaration.
//
//
//	Last Modified:	$Dmae: 1999/05/29 11:43:53 $
//
//  RCS Id:			$Id: CJPEGDecoder.cpp,v 1.59 1999/05/29 11:43:53 xne Exp $
//
//////////////////////////////////////////////////////////////////////////
//:End Ignore

#include "CJPEGDecoder.h"
#include "Bitlevel.h"
#include "JPEGConst.h"

namespace COMP {
//------------------------------------------------------------------------
CJPEGParams::ECompressionMode CJPEGDecoder::Init (void)
{
	COMP_TRYTHIS
	// read the compressed file header into the buffer, and set the decompression parameters
	CJPEGParams::ECompressionMode compression_mode = CJPEGParams::e_Unknown;
	unsigned int lg;
	unsigned short marker_code;
	
	// reset the reading function
	m_buf.real_rewind ();
	// scan the file for a SOF marker. No state-machinery here
	// since eventual error/bad file would be caught when _really_
	// trying to interpret the header.
	for (;;)
	{
		// Reading next marker
		if (! m_buf.read_marker (marker_code))
		{
			// Could not read marker.
			m_buf.real_rewind ();
			return CJPEGParams::e_Unknown;
		}
		m_buf.real_seek (16);
		switch (marker_code)
		{
		case cMarkerSOF1 : // SOF baseline DCT
		case cMarkerSOF2 : // SOF extended sequential DCT
			m_buf.real_rewind ();
			return CJPEGParams::e_LossyJPEG;
			break;

		case cMarkerSOF3 : // SOF 
			m_buf.real_rewind ();
			return CJPEGParams::e_LosslessJPEG;
			break;
 
		case cMarkerSOI : break;   // SOI

		case cMarkerDQT : // DQT
		case cMarkerDRI : // DRI
		case cMarkerDHT : // DHT
		case cMarkerAPP : // Application segment, just skip it;
			// lg in bytes
			lg = m_buf.readN (16);
			m_buf.real_seek (8 * lg);
			break;

		default : 
			// unknown/unsupported marker -> buffer corrupted?
			m_buf.real_rewind ();
			return CJPEGParams::e_Unknown;
		}
	}
	COMP_CATCHTHIS
}
//------------------------------------------------------------------------
//-------JPEG-Lossy Decoder methods
void CJPEGDecoder::ReadJPEGLossyFooter (void)
{
	COMP_TRYTHIS
	// read the compressed file footer from the buffer
	unsigned short marker_code;

	// Go to the current position and read the marker code
	if (! m_buf.read_marker (marker_code) || marker_code != cMarkerEOI)
	{
		// Find the RI the last lines are in
		unsigned long n_MCU_per_line = (m_Image.GetW() + 7) / 8;
		unsigned long max_MCU = n_MCU_per_line * ((m_Image.GetH() + 7) / 8);
		unsigned long lastgood_MCU = 0;
		if (!m_param.m_RestartInterval)
		{
			lastgood_MCU = 0;
		}
		else
		{
			unsigned long doubtfull_MCU = max_MCU % m_param.m_RestartInterval;
			if (doubtfull_MCU == 0) 
				doubtfull_MCU += m_param.m_RestartInterval;
			lastgood_MCU = max_MCU - doubtfull_MCU;
		}
		unsigned short corner_line = (unsigned short)((lastgood_MCU / n_MCU_per_line) * 8);
		// Negate the lines between the current line and the end
		m_qinfo.Negate (corner_line, m_Image.GetH() - 1);
	}
	COMP_CATCHTHIS
}
//------------------------------------------------------------------------
void CJPEGDecoder::ZeroMCU (unsigned short i_from_line, unsigned short i_from_column,
							unsigned short i_to_line, unsigned short i_to_column)
{
	COMP_TRYTHIS
	unsigned short corner_column = i_from_column;
	unsigned short corner_line = i_from_line;
	CJBlock <unsigned short>	pixels_block;

	pixels_block.Zero ();
	for(; corner_column<=i_to_column || corner_line<i_to_line ;)
	{
		m_Image.put_block (pixels_block, corner_column, corner_line);
		corner_column += 8;
		if (corner_column >= m_Image.GetW())
		{
			corner_column = 0;
			corner_line += 8;
		}
		if (corner_line >= i_to_line) // go out of this 
			break;
	}
	COMP_CATCHTHIS
}
//------------------------------------------------------------------------
bool CJPEGDecoder::PerformLossyResync (unsigned long &t_cur_MCU, 
									   unsigned long &t_RSTm,
									   unsigned short &t_corner_line, 
									   unsigned short &t_corner_column,
									   bool t_good_line)
{
	COMP_TRYTHIS
	// Find the next marker
	unsigned short end_corner_line;
	unsigned short end_corner_column;
	unsigned long end_MCU;
	short new_RSTm;
	unsigned long n_MCU_per_line = (m_Image.GetW() + 7) / 8;
	unsigned long max_MCU = n_MCU_per_line * ((m_Image.GetH() + 7) / 8);
	unsigned long start_MCU = t_RSTm * m_param.m_RestartInterval; // MCU index of the start of the current interval
	unsigned short start_corner_line = (unsigned short)((start_MCU / n_MCU_per_line) * 8);

	new_RSTm = FindNextMarker (); // return the RSTinterval 
	if (new_RSTm < 0)          // no usable marker found
	{
		end_corner_line = ((m_Image.GetH() - 1) / 8) * 8;
		end_corner_column = ((m_Image.GetW() - 1) / 8) * 8;
		end_MCU = max_MCU - 1;
	}
	else
	{
		m_buf.seek (16); // eat the marker....
		new_RSTm = (short)(new_RSTm - (t_RSTm % 8));
		if (new_RSTm < 0) new_RSTm += 8;
		t_RSTm += new_RSTm;  // this is the new RST marker we found
		end_MCU = (t_RSTm + 1) * m_param.m_RestartInterval - 1;
		end_corner_line   = (unsigned short)((end_MCU / n_MCU_per_line) * 8);
		end_corner_column = (unsigned short)((end_MCU % n_MCU_per_line) * 8);
	}

	m_qinfo.Negate (start_corner_line, cmin (t_corner_line + 8, m_Image.GetH()) - 1);
	ZeroMCU (t_corner_line, t_corner_column, end_corner_line, end_corner_column); 
	
	if (end_corner_line > t_corner_line)  // to guard against the effect of the min if at the top of the image
		m_qinfo.Zero (cmin (t_corner_line + 8, m_Image.GetH()), 
				      cmin (end_corner_line + 8, m_Image.GetH()) - 1);

	// update the MCU position
	t_cur_MCU = end_MCU + 1;

	// update t_corner_line and t_corner_column:
	t_corner_line   = (unsigned short)((t_cur_MCU / n_MCU_per_line) * 8);
	t_corner_column = (unsigned short)((t_cur_MCU % n_MCU_per_line) * 8);
	
	// good_line is always false unless the new start MCU is at the
	// begining of a line.
	if ((t_cur_MCU % n_MCU_per_line) == 0) t_good_line = true;

	if (t_cur_MCU == max_MCU) return true;        // decoding must stop
	else return false;		// decoding may continue
	COMP_CATCHTHIS
}
//------------------------------------------------------------------------
void CJPEGDecoder::DecodeLossyBuffer (void)
{
	COMP_TRYTHIS
	if (read_LOSSY_header ())
	{
		unsigned short shift;

		switch (m_Image.GetNB())
		{
		case 8 : 
			shift = 128;
			break;
		case 12 : 
			shift = 2048;
			break;
		default: 
			Assert (0, Util::CParamException());
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
		bool			good_line		= true;
		unsigned short marker_code;

		while (! stop)
		{
			if (DecodeBlock (Qblock))
			{
 				// perform block differential unshift
				Qblock.differential_unshift (last_DC);
				// Perform block quantization
				m_cqtbl.Dequantize_block (Qblock, FDCT_block);
				// Compute the FDCT of the block
				FDCT_block.inverse_DCT (shifted_block);
				// Perform the level shift
				shifted_block.level_unshift (pixels_block, shift);
				// Put block in place
				m_Image.put_block (pixels_block, corner_column, corner_line);
				cur_MCU++;
				// Compute new Upper Left corner
				corner_column += 8;
				if (corner_column >= m_Image.GetW())
				{
					if (good_line)
						m_qinfo.Set (corner_line, cmin (corner_line + 8, m_Image.GetH()) - 1, m_Image.GetW());
					good_line	  = true;
					corner_column =  0;
					corner_line   += 8;
					if (corner_line >= m_Image.GetH()) stop = true;
				}
				// is a Restart marker expected?
				if (m_param.m_RestartInterval && 
					  (cur_MCU % m_param.m_RestartInterval) == 0 && 
					     ! stop)
				{
					// is there a restart marker?
					if (m_buf.read_marker (marker_code) &&
						marker_code == (cMarkerRST + (cur_interval % 8))) 
						m_buf.seek (16);
					else
					{
						// Couldn't find expected restart marker.
						if (good_line)
						{
							//XXX shouldnt i_to == cur_line ???
							m_qinfo.Set (corner_line,  cmin(corner_line + 8, m_Image.GetH()) - 1, 
										-corner_column);
						}
						good_line = false;
						stop = PerformLossyResync (cur_MCU, cur_interval, 
							                       corner_line, corner_column,
												   good_line);
					}
					cur_interval++;
					// Reset the prediction for the DC coefficient.
					last_DC = 0;
				}
			}
			else
			{
				if (good_line)
				{
					m_qinfo.Set (corner_line, cmin (corner_line + 8, m_Image.GetH()) - 1, 
						        -(corner_column - 1));
				}
				good_line = false;
				stop = PerformLossyResync (cur_MCU, cur_interval, 
										   corner_line, corner_column,
										   good_line);
				cur_interval++;
				// Reset the prediction for the DC coefficient.
				last_DC = 0;
			}
		}
		ReadJPEGLossyFooter ();
	}
	else  // could not read the JPEG header
	{
		m_qinfo.Zero (0, m_Image.GetH() - 1); 
		m_Image.Zero (0, m_Image.GetH() - 1);
	}
	COMP_CATCHTHIS
}
//------------------------------------------------------------------------
bool CJPEGDecoder::DecodeBlock (CJBlock<short> &o_blk)
{
	COMP_TRYTHIS
// decodes a 8*8 bloc of quantized parameters readed onto disk,
// the decoding process is function of the specified AC & DC Huffman tables.
	unsigned int		nb_coeff;
	unsigned int		r;
	short				coeff;

	// decoding DC parameter
	if (! m_hdecoder.decode_DIFF (coeff))
	{
		o_blk.Zero ();
		return false;
	}
	o_blk.Cset (0, coeff);
	nb_coeff = 1;
	// decoding the 63 AC parameters
	while (nb_coeff < 64)
	{
		if (! m_hdecoder.decode_R_AC (r, coeff))
		{
			o_blk.Zero ();
			return false;
		}
		if (r == 0)
			if (coeff == 0)
				// set all the remaining AC parameters to zero
				while (nb_coeff < 64) o_blk.CZset (nb_coeff++, 0);
			else
				o_blk.CZset (nb_coeff++, coeff);
		else
			//check if we have a valid run-lenght
			if ((nb_coeff + r) > 63)
			{
				o_blk.Zero ();
				return false;
			}
			else
			{
				// set the next r AC parameters to zero
				while (r--) o_blk.CZset (nb_coeff++, 0);
				// set the current AC parameters to the decoded value
				o_blk.CZset (nb_coeff++, coeff);
			}
	}
	return true;
	COMP_CATCHTHIS
}
//------------------------------------------------------------------------
bool CJPEGDecoder::read_LOSSY_header (void)
{
	COMP_TRYTHIS
// read the compressed file header into the buffer, and set the decompression parameters
	unsigned int lg, i;
	unsigned short NLines, NColumns;
	unsigned short marker_code;
	unsigned short real_RI = 0;
	unsigned __int32 str_bin;
	int selected_DC_table = -1;
	int selected_AC_table = -1;
	unsigned short precision = 0;
	bool DQT_ok = false;
	bool DRI_ok	= false;
	bool SOF_ok	= false;
	bool DHT_ok	= false;
	bool SOS_ok	= false;

	// 0° reset the reading function
	m_buf.real_rewind ();
	// 1° SOI
	// read SOI marker
	if (! m_buf.read_marker (marker_code) || marker_code != cMarkerSOI)
	{
		return false;
	}
	m_buf.real_seek (16);
	
	while (! (DRI_ok && SOF_ok && DHT_ok && SOS_ok && DQT_ok))
	{
		// Reading next marker
		if (! m_buf.read_marker (marker_code))
		{
			if (! (SOF_ok && DHT_ok && SOS_ok && DQT_ok))
			{
				return false; // missing marker code in header
			}
			else if (! DRI_ok)
			{
				// DRI is optional
				m_param.m_RestartInterval = 0;
				DRI_ok = true;
			}
			break;
		}
		m_buf.real_seek (16);
		switch (marker_code)
		{
		case cMarkerDQT : // DQT
			// DQT, lg in bytes
			str_bin = m_buf.readN (16);
			m_buf.real_seek (16);
			lg = str_bin;
			// DQT, PqTq
			str_bin = m_buf.readN (8);
			m_buf.real_seek (8);
			if (str_bin & 0x0F) 
			{
				return false; // bad PqTq values in DQT segment"));
			}
			precision = (unsigned short)(str_bin >> 4);
			// DQT, 64 coefficients
			if (precision == 0)
			{
				if (lg != 67)
				{
					return false; // Bad length for DQT segment
				}
				for (i=0 ; i<64 ; i++)
				{
					str_bin = m_buf.readN (8);
					m_buf.real_seek (8);
					m_param.m_QuantizationTable.m_Values[i] = (unsigned short)str_bin;
				}
			}
			else
			{
				if (precision == 1)
				{
					if (lg != 131)
					{
						return false; // Bad length for DQT segment
					}
					// cout<<"reading Q on 16 bits"<<endl;
					for (i=0 ; i<64 ; i++)
					{
						str_bin = m_buf.readN (16);
						m_buf.real_seek (16);
						m_param.m_QuantizationTable.m_Values[i] = (unsigned short)str_bin;
					}
				}
				else
				{
					return false; // Bad precision fot DQT table
				}
			}
			// The quantization tables had been modified at compression time, and saved
			// so we don't need to remodify them at decompression time
			m_param.m_QualityFactor = 50;
			m_cqtbl.set_QTable (m_param.m_QuantizationTable);
			DQT_ok = true;
			break;
			
		case cMarkerDRI : // DRI
			// DRI, lg in bytes
			str_bin = m_buf.readN (16);
			m_buf.real_seek (16);
			lg = str_bin;
			if (lg != 4) 
			{
				return false; // Bad length for DRI segment
			}
			// DRI, lg restart interval in MCU
			str_bin = m_buf.readN (16);
			m_buf.real_seek (16);
			real_RI = (unsigned short)str_bin;
			DRI_ok = true;
			m_param.m_RestartInterval = real_RI;
			break;

		case cMarkerSOF1 : // SOF baseline DCT
		case cMarkerSOF2 : // SOF extended sequential DCT
			m_param.m_Mode = CJPEGParams::e_LossyJPEG;
			// SOF, lg in bytes
			str_bin = m_buf.readN (16);
			m_buf.real_seek (16);
			lg = str_bin;
			if (lg != 11) 
			{
				return false; // bad length for SOF segment.
			}
			// SOF, sample precision in bits
			str_bin = m_buf.readN (8);
			m_buf.real_seek (8);
			m_param.m_BitsPerPixel = str_bin;
			if (m_Image.GetNB() != m_param.m_BitsPerPixel)
			{
				return false; // mismatch between image depth.
			}
			// SOF, number of lines
			str_bin = m_buf.readN (16);
			m_buf.real_seek (16);
			NLines = (unsigned short)str_bin;
			if (NLines != m_Image.GetH())
			{
				return false; // the number of lines from the CDataField != the number of lines coded in the JPEG stream
			}
			// SOF, number of MCU per line
			str_bin = m_buf.readN (16);
			m_buf.real_seek (16);
			NColumns = (unsigned short)str_bin;
			if (NColumns != m_Image.GetW())
			{
				return false; // the number of cols from the CDataField != the number of cols coded in the JPEG stream
			}
			// SOF, number of components per frame + C1 + Td1 + Ta1
			str_bin = m_buf.read32 ();
			m_buf.real_seek (32);
// XXX 0x01001100 ??
			if (str_bin != 0x01011100) 
			{
				return false; // bad number of components per frame in SOF segment
			}
			SOF_ok = true;
			break;
	
		case cMarkerDHT : // DHT
			DHT_ok = m_hdecoder.read_HT_from_header (m_buf);
			break;			
			
		case cMarkerSOS : // SOS
			{
			const unsigned short c_Se = 0x3F;	// Se application value
			const unsigned short c_Se_compatible = 0x00;  // other Se value, only for compatibility with the "buggy" prototype 
			// SOS, lg in bytes
			str_bin = m_buf.readN (16);
			m_buf.real_seek (16);
			lg = str_bin;
			if (lg != 8) 
			{
				return false; // bad length for SOS segment
			}

			// SOS, number of components in scan + CS1 + Td1 + Ta1 + Start spectral
			str_bin = m_buf.read32 ();
			m_buf.real_seek (32);
			// Test Ns and CSk
			if ((str_bin >> 16) != 0x0101) 
			{
				return false; // bad number of components per scan, or bad component selector in SOS segment
			}
			// SOS, Huffman table selection TDk and TAk
			str_bin &= 0xFFFF;
			switch (str_bin >> 8)
			{
			case 0x00 : selected_DC_table = 0; selected_AC_table = 2; break;
			case 0x01 : selected_DC_table = 0; selected_AC_table = 3; break;
			case 0x10 : selected_DC_table = 1; selected_AC_table = 2; break;
			case 0x11 : selected_DC_table = 1; selected_AC_table = 3; break;
			default : 
				return false; // bad TcTh number in DHT segment 
			}
			// Test for SS
			if (str_bin & 0xFF) 
			{
				return false; // bad value for SS [00] in SOS segment
			}
			// SOS, Stop spectral + Ah + Al point transform
			str_bin = m_buf.readN (16);
			m_buf.real_seek (16);
			if ((str_bin >> 8) != c_Se && (str_bin >> 8) != c_Se_compatible) 
			{
				return false; // bad value for Se in SOS segment  
							  // testing for Se=0 and Ah=0
			}
			str_bin;
			m_param.m_Predictor = 0;  // In lossy mode, this parameter is meaningless
			m_param.m_PointTransform = str_bin & 0xF;
			SOS_ok = true;
			break;
			}
		case cMarkerAPP : // Application segment, just skip it;
			// App, lg in bytes
			str_bin = m_buf.readN (16);
			lg = str_bin;
			m_buf.real_seek (8 * lg);
			break;
		
		default : 
				return false; // unknown marker code in header
		}
	}
	// Selected Huffman tables (AC & DC)
	m_hdecoder.use_these_HT (selected_DC_table, selected_AC_table);
	
	m_buf.resync ();
	return true;
	COMP_CATCHTHIS
}
//------------------------------------------------------------------------
//-------JPEG-LossLess Decoder methods
void CJPEGDecoder::DecodeLossLessBuffer (void)
{
	COMP_TRYTHIS
	if (read_LOSSLESS_header())
	{
		DPCM_decoder ();
		ReadJPEGLossLessFooter ();
	}
	else  // could not read the JPEG header
	{
		m_qinfo.Zero (0, m_Image.GetH()-1);
		m_Image.Zero (0, m_Image.GetH()-1);
	}
	COMP_CATCHTHIS
}
//------------------------------------------------------------------------
void CJPEGDecoder::ReadJPEGLossLessFooter (void)
{
	COMP_TRYTHIS
	// read the compressed file footer from the buffer
	unsigned short marker_code;

	// Go to the current position and read the marker code
	if ( ! m_buf.read_marker (marker_code) || marker_code != cMarkerEOI)
	{
		// Find the RI the last lines are in
		unsigned short lastgoodline = 0;
		if (m_param.m_RestartInterval)
		{
			unsigned short doubtfull_lines = m_Image.GetH() % m_param.m_RestartInterval;
			if (doubtfull_lines == 0) 
				doubtfull_lines += m_param.m_RestartInterval;
			lastgoodline = m_Image.GetH() - doubtfull_lines;
		}
		// Negate the lines between the current line and the end
		m_qinfo.Negate (lastgoodline, m_Image.GetH() - 1);
	}
	COMP_CATCHTHIS
}
//------------------------------------------------------------------------
bool CJPEGDecoder::read_LOSSLESS_header (void)
{
	COMP_TRYTHIS
	// read the LOSSLESS compressed file header into the buffer, and set the decompression parameters
	unsigned int lg;
	int selected_table = -1;
	unsigned short NLines;
	unsigned short NColumns;
	unsigned short marker_code;
	unsigned short real_RI = 0;
	unsigned __int32 str_bin;
	bool DRI_ok	= false;
	bool SOF_ok	= false;
	bool DHT_ok	= false;
	bool SOS_ok	= false;

	// 0° reset the reading function
	m_buf.real_rewind ();
	// read SOI marker
	if (! m_buf.read_marker (marker_code) || marker_code != cMarkerSOI)
	{
		return false;
	}
	m_buf.real_seek (16);
	
	while (! (DRI_ok && SOF_ok && DHT_ok && SOS_ok))
	{
		// Reading next marker
		if (! m_buf.read_marker (marker_code)) // unable to find marker in coded stream
		{
			if (! (SOF_ok && DHT_ok && SOS_ok))
			{
				return false; // missing marker code in header
			}
			else if (! DRI_ok) 
			{	// DRI is optional
				m_param.m_RestartInterval = 0;
				DRI_ok = true;
			}
			break;
		}
		m_buf.real_seek (16);
		switch (marker_code)
		{
		case cMarkerDRI : // DRI
			// DRI, lg in bytes
			str_bin = m_buf.readN (16);
			m_buf.real_seek (16);
			lg = str_bin;
			if (lg != 4) 
			{
				return false; // Bad length for DRI segment
			}
			// DRI, lg restart interval in MCU
			str_bin = m_buf.readN (16);
			m_buf.real_seek (16);
			real_RI = (unsigned short)str_bin;
			if (real_RI % m_Image.GetW())
			{
				return false; // RI must be a multiple of the image width
			}
			DRI_ok = true;
			break;
		
		case cMarkerSOF3 : // SOF 
			m_param.m_Mode = CJPEGParams::e_LosslessJPEG;
			// SOF, lg in bytes
			str_bin = m_buf.readN (16);
			m_buf.real_seek (16);
			lg = str_bin;
			if (lg != 11) 
			{
				return false; // bad length for SOF segment"
			}
			// SOF, sample precision in bits
			str_bin = m_buf.readN (8);
			m_buf.real_seek (8);
			m_param.m_BitsPerPixel = str_bin;
			if (m_Image.GetNB() != m_param.m_BitsPerPixel)
			{
				return false; // mismatch between image depth.
			}
			// SOF, number of lines
			str_bin = m_buf.readN (16);
			m_buf.real_seek (16);
			NLines = (unsigned short)str_bin;
			if (NLines != m_Image.GetH())
			{
				return false; // the number of lines from the CDataField != the number of lines coded in the JPEG stream
			}
			// SOF, number of MCU per line
			str_bin = m_buf.readN (16);
			m_buf.real_seek (16);
			NColumns = (unsigned short)str_bin;
			if (NColumns != m_Image.GetW())
			{
				return false; // the number of cols from the CDataField != the number of cols coded in the JPEG stream
			}
			// SOF, number of components per frame + C1 + Td1 + Ta1
			str_bin = m_buf.read32 ();
			m_buf.real_seek (32);
// XXX 0x01001100 ??
			if (str_bin != 0x01011100) 
			{
				return false; // bad number of components per frame in SOF segment
			}
			SOF_ok = true;
			break;
		
		case cMarkerDHT : // DHT
			{
			DHT_ok = m_hdecoder.read_HT_from_header (m_buf);
			if (! DHT_ok)
			{
				return false;
			}
			break;
			}
	
		case cMarkerSOS : // SOS
			// SOS, lg in bytes
			str_bin = m_buf.readN (16);
			m_buf.real_seek (16);
			lg = str_bin;
			if (lg != 8) 
			{
				return false; // bad length for SOS segment
			}
			// SOS, number of components in scan + CS1 + Td1 + Ta1 + Ss predictor
			str_bin = m_buf.read32 ();
			m_buf.real_seek (32);
			// SOS, Ns and CSk
			if ((str_bin >> 16) != 0x0101) 
			{
				return false; // bad number of components per scan, or bad component selector in SOS segment
			}
			// SOS, Huffman table selection TDk and TAk
			str_bin &= 0xFFFF;
			switch (str_bin >> 8) 
			{
			case 0x00 :	selected_table = 0; break;
			case 0x01 :	selected_table = 1; break;
			case 0x10 : selected_table = 2; break;
			case 0x11 :	selected_table = 3; break;
			default : 
				return false; // bad TcTh number in DHT segment 
			}
			if (selected_table > 1) // AC table selection
			{
				return false; // AC Huffman table selected, not allowed in LOSSLESS mode
			}
			// SOS, Ss, predictor selection
			m_param.m_Predictor = str_bin & 0xFF;
			if ((m_param.m_Predictor <= 0) || (m_param.m_Predictor > 7))
			{
				return false; // Bad Ss (predictor selection) in SOS segment
			}
			// SOS, Se + Ah + Al point transform
			str_bin = m_buf.read32 ();
			m_buf.real_seek (16);
			if (str_bin >> 20)
			{
				return false; // bad Se or Ah in SOS segment // testing for Se=0 and Ah=0
			}
			m_param.m_PointTransform = (unsigned short)(str_bin >> 16);
			if (m_param.m_PointTransform >= m_Image.GetNB())
			{
				return false; // nonsense value for the PointTransform
			}
			m_default_value = 1 << (m_Image.GetNB() - m_param.m_PointTransform - 1);
			SOS_ok = true;
			break;

		case cMarkerAPP : // Application segment, just skip it;
			// App, lg in bytes
			str_bin = m_buf.readN (16);
			lg = str_bin;
			m_buf.real_seek (16 + 8 * lg);
			break;

		default:
			return false; // unknown marker code in header
		}
	}
	
	// Selected Huffman tables (DC)
	m_hdecoder.use_these_HT (selected_table, -1);
	// Update the restart interval field 
	if (real_RI > 0) m_param.m_RestartInterval = real_RI / NColumns;
	else // No restart interval 
//XXX		m_param.m_RestartInterval = NLines;
		m_param.m_RestartInterval = 0;

	m_buf.resync ();
	return true;
	COMP_CATCHTHIS
}
//-----------------------------------------------------------------------
short CJPEGDecoder::FindNextMarker (void)
{
	COMP_TRYTHIS
	// resynchronization with the next marker (or at least with the EOF)
	unsigned short code;

	m_buf.byteAlign ();
	for (;;)
		if (m_buf.read_marker (code) && code >= cMarkerRST && code <= (cMarkerRST + 7))
			return (code & 0x000F);
		else
			if (m_buf.reached_end ()) return -1;
			else m_buf.seek (8);
	COMP_CATCHTHIS
}
//-----------------------------------------------------------------------
bool CJPEGDecoder::DecodeNextLine (unsigned short i_cur_line,
								   unsigned short i_predictor0,
								   unsigned short i_predictor)
{
	COMP_TRYTHIS
	short diff;
	unsigned long base_MCU = i_cur_line * m_Image.GetW();
	unsigned short *p0 = m_Image.Get() + base_MCU;
	unsigned short *pB = m_Image.Get() + base_MCU - m_Image.GetW();
	unsigned short v0;
	__int32 vB, vC;
	unsigned short cur_MCU_in_line = 0;
	unsigned short imgW = m_Image.GetW();
	bool failed = false;
	
	Assert (i_cur_line < m_Image.GetH(), Util::CParamException ());
	vB = i_cur_line ? *pB : 0;
	switch (i_predictor0)
	{
	case 0 :
		if (m_hdecoder.decode_DIFF (diff)) 
		{
			*p0++ = v0 = m_default_value + diff;
		}
		else failed = true;
		break;
	case 1 :
	case 3 :
		if (m_hdecoder.decode_DIFF (diff)) *p0++ = v0 = diff;
		else failed = true;
		break;
	case 2 :
	case 4 :
	case 6 :
		if (m_hdecoder.decode_DIFF (diff)) *p0++ = v0 = vB + diff;
		else failed = true;
		break;
	case 5 :
	case 7 :
		if (m_hdecoder.decode_DIFF (diff)) *p0++ = v0 = (vB >> 1) + diff;
		else failed = true;
		break;
	default :
		Assert (0, Util::CNamedException ("ERROR CodeNextLine : bad predictor value"));
	}

	// now, loop on all other MCUs of the line...
	if (! failed)
		switch (i_predictor)
		{
		case 0 : 
			Assert(0,Util::CParamException());
			for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
				if (m_hdecoder.decode_DIFF (diff)) *p0++ = m_default_value + diff;
				else 
				{
					failed = true;
					break;
				}
			break;
		case 1 :
			for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
				if (m_hdecoder.decode_DIFF (diff)) *p0++ = v0 += diff;
				else 
				{
					failed = true;
					break;
				}
			break;
		case 2 :
			for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
				if (m_hdecoder.decode_DIFF (diff)) *p0++ = *++pB + diff;
				else 
				{
					failed = true;
					break;
				}
			break;
		case 3 :
			for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
				if (m_hdecoder.decode_DIFF (diff)) *p0++ = *pB++ + diff; 
				else 
				{
					failed = true;
					break;
				}
			break;
		case 4 :
			for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
			{
				vC = vB;
				vB = *++pB;
				if (m_hdecoder.decode_DIFF (diff)) *p0++ = v0 += vB - vC + diff;
				else 
				{
					failed = true;
					break;
				}
			}
			break;
		case 5 :
			for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
			{
				vC = vB;
				vB = *++pB;
				if (m_hdecoder.decode_DIFF (diff)) *p0++ = v0 += ((vB - vC) >> 1) + diff;
				else 
				{
					failed = true;
					break;
				}
			}
			break;
		case 6 :
			for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
			{
				vC = vB;
				vB = *++pB;
				if (m_hdecoder.decode_DIFF (diff)) *p0++ = v0 = vB + ((v0 - vC) >> 1) + diff;
				else 
				{
					failed = true;
					break;
				}
			}
			break;
		case 7 :
			for (cur_MCU_in_line=1 ; cur_MCU_in_line<imgW ; cur_MCU_in_line++)
				if (m_hdecoder.decode_DIFF (diff)) *p0++ = v0 = ((v0 + *++pB) >> 1) + diff;
				else
				{
					failed = true;
					break;
				}
			break;
		default : 
			Assert (0, Util::CParamException ());
		}

	if (failed)
	{
		m_qinfo.Set (i_cur_line, cur_MCU_in_line);
		m_Image.ZeroPixels ((unsigned long)(p0 - m_Image.Get()), base_MCU + m_Image.GetW() - 1);
		return false;
	}
	m_qinfo.Set (i_cur_line, m_Image.GetW());
	return true;
	COMP_CATCHTHIS
}
//------------------------------------------------------------------------
void CJPEGDecoder::DPCM_decoder (void)
{
	COMP_TRYTHIS
	short RSTm = 0;

	// loop over all the lines in the image
	for (unsigned long cur_line=0 ; cur_line<m_Image.GetH() ; RSTm++)
	{
		// loop on the lines inside one restart interval
		unsigned short predictor0 = 0; // use default value for first MCU of first line of RI
		unsigned short predictor = 1; // use pixel right for first line of RI
		unsigned short start_line_in_RI = (unsigned short)cur_line;
		unsigned short end_line_in_RI = 0;
		for (unsigned long cur_line_in_RI=0 ; 
		        ( (m_param.m_RestartInterval ==0) ||
				  (cur_line_in_RI<m_param.m_RestartInterval)) &&
				cur_line < m_Image.GetH() ;
				cur_line_in_RI++, cur_line++)
		{
			if (! DecodeNextLine ((unsigned short)cur_line, predictor0, predictor))
			{
				short new_RSTm;

				cur_line_in_RI = m_param.m_RestartInterval; // so we get out of this loop
				new_RSTm = FindNextMarker (); // return the RSTinterval 
				if (new_RSTm < 0 || new_RSTm > 7) end_line_in_RI = m_Image.GetH() - 1;
				else
				{
					new_RSTm -= RSTm % 8;
					if (new_RSTm < 0) new_RSTm += 8;
					RSTm += new_RSTm;  // this is the new RST marker we expect to find
					end_line_in_RI = (RSTm + 1) * m_param.m_RestartInterval - 1;
				}
				m_qinfo.Negate (start_line_in_RI, (unsigned short)cur_line);
				m_qinfo.Zero ((unsigned short)(cur_line + 1), end_line_in_RI); 
				m_Image.Zero ((unsigned short)(cur_line + 1), end_line_in_RI);
				cur_line = end_line_in_RI;
			}
			predictor0 = 2; // use pixel up for first MCU of subsequent lines
			predictor  = m_param.m_Predictor;
		}
		if (cur_line < m_Image.GetH())
		{
			// Look for the Restart marker
			unsigned short marker_code = 0;
			if (! m_buf.read_marker (marker_code) || marker_code != (cMarkerRST + (RSTm % 8)))
			{ 
				// we didn't find a marker
				// or the marker we found is not the expected one.
				unsigned short end_line_in_RI;
				short new_RSTm;

				new_RSTm = FindNextMarker (); // return the RSTinterval 
				if (new_RSTm < 0)          // no usable marker found
				{
					end_line_in_RI = m_Image.GetH() - 1;
				}
				else
				{
					new_RSTm -= RSTm % 8;
					if (new_RSTm < 0) new_RSTm += 8;
					RSTm += new_RSTm;  // this is the new RST marker we expect to find
					end_line_in_RI = (RSTm + 1) * m_param.m_RestartInterval - 1;
				}
				m_qinfo.Negate (start_line_in_RI, (unsigned short)cur_line);
				m_qinfo.Zero ((unsigned short)(cur_line + 1), end_line_in_RI); 
				m_Image.Zero ((unsigned short)(cur_line + 1), end_line_in_RI);
				cur_line = end_line_in_RI;
				if (++cur_line == m_Image.GetH())
					return; // quit the loop if we just decoded the last line of the image
			}
			// We were so lucky to find the expected marker, now skip it.
			m_buf.seek (16);
		}
	}
	COMP_CATCHTHIS
}

CJPEGDecoder::CJPEGDecoder (const Util::CDataFieldCompressedImage &i_cdfci):
	m_Image (i_cdfci.GetNC(), i_cdfci.GetNL(), i_cdfci.GetNB()),
	m_buf (i_cdfci),
	m_hdecoder (m_buf),
	m_qinfo (i_cdfci)
{
}

CJPEGDecoder::~CJPEGDecoder ()
{
}

void CJPEGDecoder::DecodeBuffer (void)
{
	COMP_TRYTHIS
	CJPEGParams::ECompressionMode mode = Init();
	switch (mode)
	{
	case CJPEGParams::e_LosslessJPEG:
		DecodeLossLessBuffer ();
		m_Image.Inverse_point_transform (m_param.m_PointTransform);
		break;
	case CJPEGParams::e_LossyJPEG:
		DecodeLossyBuffer ();
		break;
	default:
		// sth bad happened (buffer corrupted) -> clear the image
		m_qinfo.Zero (0, m_Image.GetH() - 1); 
		m_Image.Zero (0, m_Image.GetH() - 1);
		return;
	}
	COMP_CATCHTHIS
}

Util::CDataFieldUncompressedImage CJPEGDecoder::GetDecompressedImage (const unsigned short i_NR)
{
	COMP_TRYTHIS
	return m_Image.pack (i_NR);
	COMP_CATCHTHIS
}

std::vector<short> CJPEGDecoder::GetQualityInfo ()
{
	return m_qinfo;
}

} // end namespace
