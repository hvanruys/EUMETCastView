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
//	FileName:		CHcodec.cpp
//	
//	Date Created:	21/08/1998
//
//	Author:			Van Wynsberghe Laurent 
//
//
//	Description:	contains the classes dealing with the Huffman coder / decoder
//
//
//	Last Modified:	$Dmae: 1999/01/19 13:07:53 $
//
//  RCS Id:			$Id: CHcodec.cpp,v 1.44 1999/06/01 12:25:53 youag Exp $
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

#include "ErrorHandling.h"
#include "CHcodec.h"
#include "Bitlevel.h"

namespace COMP {
void CHCoder::use_these_HT (int i_index_DC, int i_index_AC)
{
	COMP_TRYTHIS
	if (i_index_DC >= 0 && i_index_DC <= 3 && m_defined[i_index_DC])
	{
		m_index_DC = i_index_DC;
		m_pdefDC = &(m_code[i_index_DC]);
		calc_table (i_index_DC, true);
		m_pLutSizDC = &m_pdefDC->m_LutSiz[0];
		m_pLutCodDC = &m_pdefDC->m_LutCod[0];
	}
	if (i_index_AC >= 0 && i_index_AC <= 3 && m_defined[i_index_AC])
	{
		m_index_AC = i_index_AC;
		m_pdefAC = &(m_code[i_index_AC]);
		calc_table (i_index_AC, true);
		m_pLutSizAC = &m_pdefAC->m_LutSiz[0];
		m_pLutCodAC = &m_pdefAC->m_LutCod[0];
	}
	COMP_CATCHTHIS
}

void CHDecoder::use_these_HT (int i_index_DC, int i_index_AC)
{
	COMP_TRYTHIS
	if (i_index_DC >= 0 && i_index_DC <= 3 && m_defined[i_index_DC])
	{
		m_index_DC = i_index_DC;
		m_pdefDC = &(m_code[i_index_DC]);
		calc_table (i_index_DC, false);
		m_pLutSizDC = &m_pdefDC->m_LutSiz[0];
		m_pLutValDC = &m_pdefDC->m_LutVal[0];

	}
	if (i_index_AC >= 0 && i_index_AC <= 3 && m_defined[i_index_AC])
	{
		m_index_AC = i_index_AC;
		m_pdefAC = &(m_code[i_index_AC]);
		calc_table (i_index_AC, false);
		m_pLutSizAC = &m_pdefAC->m_LutSiz[0];
		m_pLutValAC = &m_pdefAC->m_LutVal[0];
	}
	COMP_CATCHTHIS
}

bool CHDecoder::decode_DIFF (short &o_diff)
{
	COMP_TRYTHIS_SPEED
// WARNING !!!
// This function need a LEFT aligned binary string !!!!!
// puts in diff the value encoded in the left aligned binary string str_bin
// and returns true if success
	
// 0° Extract the binary string from the input buffer
	unsigned __int32 str_bin = m_buf.readN (16);

// 1° Decode the symbol
	unsigned int size = m_pLutSizDC[str_bin];
	// is this a valid bit string for the Huffman table ?
	if (! size)
	{
		// No symbol found.
		return false;
	}
	unsigned int code = m_pLutValDC[str_bin];

// 2° Extract the DIFF
	if (code&0x0F) // code==16 is also coded without VLB
	{
		if ((size += code) > 16) o_diff = (short)(m_buf.read32 () >> (32 - size));
		else o_diff = (short)(str_bin >> (16 - size));

		if (! (o_diff & speed_bit16 (code)))
			// We have a negative Diff
		{
			o_diff |= speed_mask16_msb (16 - code);
			o_diff++;
		}
		else o_diff &= speed_mask16_lsb (code);
	}
	else 
	{
		if( code == 0 )
			o_diff = 0;
		else
			o_diff = -32768;
	}
	
	// Testing for marker 
	if (m_buf.in_marker (size))
	{
		// Reading a marker!
		return false;
	}
	m_buf.seek (size);
	return true;
	COMP_CATCHTHIS_SPEED
}

} // end namespace
