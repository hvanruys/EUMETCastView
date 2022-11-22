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

#define NOMINMAX

#include <iostream>
#include <math.h>

#include "ErrorHandling.h"
#include "CQuantizationTable.h"	
#include "JPEGConst.h"

namespace COMP {
//--------------------------------------------------------------------------
CQuantizationTable::CQuantizationTable (double i_init)
{
	COMP_TRYTHIS
	m_defined = i_init == 0  ? false : true;
	m_Pq = 1; // default precision = 16 bits
	for (int i=0 ; i<64 ; i++)
	{
		m_QT.m_Values[i] = (unsigned short)i_init;
		m_Q[i]	= i_init;
		// m_Qi will be initialized by ApplyDCTCorrectionFactors()
	}
	ApplyDCTCorrectionFactors ();
	COMP_CATCHTHIS
}
//--------------------------------------------------------------------------
CQuantizationTable::CQuantizationTable (CJPEGParams &t_params)
{
	COMP_TRYTHIS
	set_QTable (t_params.m_QuantizationTable, t_params.m_QualityFactor);
	// reset the quality factor to leave the object CCompressParams in a coherent state
	t_params.m_QualityFactor = 50;
	COMP_CATCHTHIS
}
//--------------------------------------------------------------------------
void CQuantizationTable::ApplyDCTCorrectionFactors (void)
{
	COMP_TRYTHIS
	// correction to process some DCT/IDCT divisions into the quantization/dequantization 
	static const double scalefactor[8] = {
		1.0, 1.387039845, 1.306562965, 1.175875602,
		1.0, 0.785694958, 0.541196100, 0.275899379
	};

	for (int x=0, i=0 ; x<8 ; x++)
		for (int y=0 ; y<8 ; y++, i++)
			m_Qi[i] = 1.0 / (m_Q [i] *= scalefactor[x] * scalefactor[y] * 8.0);
	COMP_CATCHTHIS
}
//--------------------------------------------------------------------------
void CQuantizationTable::set_QTable (COMP::CQuantizeTable &t_V, unsigned short i_quality)
{
	COMP_TRYTHIS
	unsigned int i, j;
	unsigned __int32 coeff;
	double factor;

	// check parameters
	Assert (i_quality > 0 && i_quality < 100, Util::CParamException ());
	
	// compute the scaling factor
	if (i_quality > 50)
		// factor ranges from 1 to 0.02
		factor = (200.0 - 2.0 * i_quality) / 100.0;
	else
		// factor ranges from 50 to 1
		factor = 5000.0 / (100.0 * i_quality);
	// compute the new quantization 
	for (i=0 ; i<64 ; i++)
	{
		j = Zindex (i);
		coeff = (unsigned __int32)(factor * (double)(t_V.m_Values[i]));
		// test for underflow !!
		if (coeff < 1) coeff = 1;
		// test for overflow !!
		if (coeff > 65535) coeff = 65535;
		
		// update the quantization table for further writing of the lossy header
		t_V.m_Values[i]  = (unsigned short)coeff;
		m_QT.m_Values[j] = (unsigned short)coeff;
		m_Q [j] = coeff;
	}
	ApplyDCTCorrectionFactors ();
	m_Pq = 1; // default is 16 bits
	m_defined = true;
	COMP_CATCHTHIS
}
//--------------------------------------------------------------------------
void CQuantizationTable::write_in_header (CWBuffer & W)
{
	COMP_TRYTHIS
	short precision = m_Pq;
	unsigned int i;

	W.write_marker (cMarkerDQT);
	// DQT, lg in bytes 
	if (precision == 0) W.real_write (3 + 64, 16); // 67
	else W.real_write (3 + 128, 16); // 131
	// DQT, precision and identifier (id always 0)
	precision <<= 4;
	W.real_write (precision, 8);
	// DQT, 64 elements
	if (precision == 0)
		// write the Quantization table on 8 bits
		for (i=0 ; i<64 ; i++)
			W.real_write (m_QT.m_Values[Zindex(i)], 8);
	else
		// write the Quantization table on 16 bits
		for (i=0 ; i<64 ; i++)
			W.real_write(m_QT.m_Values[Zindex(i)], 16);	
	COMP_CATCHTHIS
}

} // end namespace COMP
