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

#ifndef CQuantizationTable_included
#define CQuantizationTable_included

/*******************************************************************************

TYPE:
Concrete Class
					
PURPOSE:
Management of the quantization process
(for the lossy JPEG compression & decompression).

FUNCTION: 
This object hold one quantization table together with the needed functions to
initialize & use them.

INTERFACES:
See 'INTERFACES:' in the module declaration below.

RESOURCES:	

REFERENCES:
See Annex F, item F.1.1.4 of the ISO JPEG recommandation

PROCESSING:
A quantization table can either be constructed as flat (all quantization factors
equal to the same constant) or from a CCompressParams structure.
convert_QTable scales the quantization factors for optimization or quality purposes.
write_header() writes the QuantizationTable at the current position of the
provided buffer as specified in the JPEG norm (JPEG header).
Quantize_block() and Dequantize_block() respectively perform the quantization/
inverse quantization of the provided DCT coefficients.

DATA:
See 'DATA:' in the class header below.

LOGIC:		
-

*******************************************************************************/

#pragma warning(disable: 4275)

#include <iostream>
#include <fstream>

#include "CompressJPEG.h"

#include "CJBlock.h"
#include "CBuffer.h"
#include "Bitlevel.h"


namespace COMP
{


class CQuantizationTable
{

private:

// DATA:

	bool			m_defined;	// true if the quantization table is completely defined
	short			m_Pq;		// precision of quantization factors (0 corresponds to
								// 8 bits, 1 corresponds to 16 bits integers)
	CQuantizeTable	m_QT;		// real quantization table.
	double			m_Q[64];	// 64 quantization factors (with DCT multiplication correction)
	double			m_Qi[64];	// 64 quantization factors (    "              "              ) (inverted)

// INTERFACES:

	// Description:	Applies the DCT correction factors to the current quantization table
	// Returns:		Nothing.
	void ApplyDCTCorrectionFactors();

public:

// INTERFACES: constructors and destructors

	// Description: Default constructor. Initialises the table with the given value.
	// Returns:		Nothing.
	CQuantizationTable
	(
		double i_init = 1.0 // the initialization value
	);

	// Description:	construct the quantization table by taking the values stored
	//				in CCompressParams.
	//				When constructing the quantization table, the m_QualityFactor
	//				is taken into account.
	//				The corresponding updated parameters are written back to t_params.
	// Returns:		Nothing.
	CQuantizationTable
	(
		CJPEGParams& t_params	// compression parameters used to
								// initialise the quantization table
	);

	// Description:	Update the values in the quantization table with the given base values,
	//				after having applied the quality factor scaling.
	// Returns:		Nothing.
	void set_QTable
	(
		CQuantizeTable& t_V,			// New base values.
		unsigned short i_quality = 50	// Qualityfactor, used to scale the base values.
	);

	// Description:	Write the DQT marker and the Q table in the jpeg header
	// Returns:		Nothing.
	void write_in_header
	(
		CWBuffer& i_buf	// CWBuffer to which the header must be written
	);

	// Description:	Performs the DCT block quantization.
	// Returns:		Nothing.
	inline void Quantize_block
	(
		CJBlock<double>&	i_block,	// DCT-coefficients block
		CJBlock<short>&		o_block		// quantized block
	)
	{
		COMP_TRYTHIS_SPEED
		// Divides each block's coeff. by the associate coeff
		// of the specified quantisation table.
			double v;

#ifdef _DEBUG
				Assert (m_defined, Util::CParamException());
				// the quantization table was not initialised
#endif
			for (unsigned int i=0 ; i<64 ; i++)
				// The quantization table has been converted !
				// So we multiply by the inversed coefficient
				// (instead of divide by the original coefficient)
				// use a cast instead of floor
				o_block.Cset (i, (short)((v = i_block.Cget (i) * m_Qi[i]) >= 0 ? v + 0.5 : v - 0.5));
		COMP_CATCHTHIS_SPEED
	}

	// Description:	Performs the DCT block dequantization.
	// Returns:		Nothing.
	inline void Dequantize_block
	(
		CJBlock<short>&		i_block,	// Quantized block.
		CJBlock<double>&	o_block		// DCT-coefficients block.
	)
	{
		COMP_TRYTHIS_SPEED
		// Multiplies each block's coeff. by the associate coeff
		// of the specified quantisation table
		
#ifdef _DEBUG
			Assert (m_defined, Util::CParamException());
			// The quantization table was not initialised.
#endif
			for (unsigned int i=0 ; i<64 ; i++)
				o_block.Cset (i, i_block.Cget (i) * m_Q[i]); 
		COMP_CATCHTHIS_SPEED
	}

	// Description:	Returns the real index given a zz index.
	// Returns:		The real index.
	inline unsigned __int8 Zindex
	(
		const unsigned int& i_k	// The ZZ index
	)
	{
		COMP_TRYTHIS
			// get the k th element in the inverse ZZ order (see norm F.1.1.5)
			static const unsigned __int8 ZZ[64] = {
						0,1,8,16,9,2,3,10,
						17,24,32,25,18,11,4,5,
						12,19,26,33,40,48,41,34,
						27,20,13,6,7,14,21,28,
						35,42,49,56,57,50,43,36,
						29,22,15,23,30,37,44,51,
						58,59,52,45,38,31,39,46,
						53,60,61,54,47,55,62,63
						};
	#ifdef _DEBUG
			Assert (i_k < 64, Util::CParamException());
#endif
			return ZZ[i_k];
		COMP_CATCHTHIS
	}

};


} // end namespace


#endif
