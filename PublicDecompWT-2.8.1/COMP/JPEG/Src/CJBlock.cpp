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

#include "CJBlock.h"
#include <math.h>
#include <typeinfo>




namespace COMP
{


// get the k th element in the ZZ order (see norm F.1.1.5)
template <class T>
const unsigned __int8 CJBlock<T>::ZZ[64] =
{
	0,1,8,16,9,2,3,10,
	17,24,32,25,18,11,4,5,
	12,19,26,33,40,48,41,34,
	27,20,13,6,7,14,21,28,
	35,42,49,56,57,50,43,36,
	29,22,15,23,30,37,44,51,
	58,59,52,45,38,31,39,46,
	53,60,61,54,47,55,62,63
};


template <class T>
void CJBlock<T>::forward_DCT (CJBlock<double> & o_FDCT_block)
{
	COMP_TRYTHIS

	// is only supposed to operate on short
	Assert(typeid(T) == typeid(short), Util::CParamException());

	// Using double as FASTFLOAT
	double tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	double dtmp0, dtmp1, dtmp2, dtmp3, dtmp4, dtmp5, dtmp6, dtmp7;
	double tmp10, tmp11, tmp13, tmp14, tmp16;
	double z1, z2, z3, z4, z5, z11, z13;
	static const double c70 = 0.707106781;
	static const double c38 = 0.382683433;
	static const double c54 = 0.541196100;
	static const double c130 = 1.306562965;
	unsigned int i, j;

	/* Pass 1: process columns. */

	for (i=0 ; i<8 ; i++) 
	{
		dtmp0 = CJBlock<T>::Cget (0, i);
		dtmp1 = CJBlock<T>::Cget (1, i);
		dtmp2 = CJBlock<T>::Cget (2, i);
		dtmp3 = CJBlock<T>::Cget (3, i);
		dtmp4 = CJBlock<T>::Cget (4, i);
		dtmp5 = CJBlock<T>::Cget (5, i);
		dtmp6 = CJBlock<T>::Cget (6, i);
		dtmp7 = CJBlock<T>::Cget (7, i);

		tmp0 = dtmp0 + dtmp7;
		tmp7 = dtmp0 - dtmp7;
		tmp1 = dtmp1 + dtmp6;
		tmp6 = dtmp1 - dtmp6;
		tmp2 = dtmp2 + dtmp5;
		tmp5 = dtmp2 - dtmp5;
		tmp3 = dtmp3 + dtmp4;
		tmp4 = dtmp3 - dtmp4;

		/* Even part */

		tmp10 = tmp0 + tmp3;	/* phase 2 */
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		//tmp12 = tmp1 - tmp2;
		z1 = (tmp1 - tmp2 + tmp13) * c70; /* c4 */

		/* Odd part */

		tmp14 = tmp4 + tmp5;	/* phase 2 */
		//tmp15 = tmp5 + tmp6;
		tmp16 = tmp6 + tmp7;

		/* The rotator is modified from fig 4-8 to avoid extra negations. */
		z5 = (tmp14 - tmp16) * c38; /* c6 */
		z2 = c54 * tmp14 + z5; /* c2-c6 */
		z4 = c130 * tmp16 + z5; /* c2+c6 */
		z3 = (tmp5 + tmp6) * c70; /* c4 */

		z11 = tmp7 + z3;		/* phase 5 */
		z13 = tmp7 - z3;

		o_FDCT_block.Cset (0, i, tmp10 + tmp11); /* phase 3 */
		o_FDCT_block.Cset (1, i, z11 + z4);
		o_FDCT_block.Cset (2, i, tmp13 + z1);	/* phase 5 */
		o_FDCT_block.Cset (3, i, z13 - z2);
		o_FDCT_block.Cset (4, i,  tmp10 - tmp11);
		o_FDCT_block.Cset (5, i, z13 + z2);	/* phase 6 */
		o_FDCT_block.Cset (6, i, tmp13 - z1);
		o_FDCT_block.Cset (7, i, z11 - z4);
	}

	/* Pass 2: process rows. */

	for (i=j=0 ; i<64 ;)
	{
		dtmp0 = o_FDCT_block.Cget (i++);
		dtmp1 = o_FDCT_block.Cget (i++);
		dtmp2 = o_FDCT_block.Cget (i++);
		dtmp3 = o_FDCT_block.Cget (i++);
		dtmp4 = o_FDCT_block.Cget (i++);
		dtmp5 = o_FDCT_block.Cget (i++);
		dtmp6 = o_FDCT_block.Cget (i++);
		dtmp7 = o_FDCT_block.Cget (i++);

		tmp0 = dtmp0 + dtmp7;
		tmp7 = dtmp0 - dtmp7;
		tmp1 = dtmp1 + dtmp6;
		tmp6 = dtmp1 - dtmp6;
		tmp2 = dtmp2 + dtmp5;
		tmp5 = dtmp2 - dtmp5;
		tmp3 = dtmp3 + dtmp4;
		tmp4 = dtmp3 - dtmp4;

		/* Even part */

		tmp10 = tmp0 + tmp3;	/* phase 2 */
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		//tmp12 = tmp1 - tmp2;
		z1 = (tmp1 - tmp2 + tmp13) * c70; /* c4 */

		/* Odd part */

		tmp14 = tmp4 + tmp5;	/* phase 2 */
		//tmp15 = tmp5 + tmp6;
		tmp16 = tmp6 + tmp7;

		/* The rotator is modified from fig 4-8 to avoid extra negations. */
		z5 = (tmp14 - tmp16) * c38; /* c6 */
		z2 = c54 * tmp14 + z5; /* c2-c6 */
		z4 = c130 * tmp16 + z5; /* c2+c6 */
		z3 = (tmp5 + tmp6) * c70; /* c4 */

		z11 = tmp7 + z3;		/* phase 5 */
		z13 = tmp7 - z3;

		o_FDCT_block.Cset (j++, tmp10 + tmp11); /* phase 3 */
		o_FDCT_block.Cset (j++, z11 + z4);
		o_FDCT_block.Cset (j++, tmp13 + z1); /* phase 5 */
		o_FDCT_block.Cset (j++, z13 - z2);
		o_FDCT_block.Cset (j++, tmp10 - tmp11);
		o_FDCT_block.Cset (j++, z13 + z2); /* phase 6 */
		o_FDCT_block.Cset (j++, tmp13 - z1);
		o_FDCT_block.Cset (j++, z11 - z4);
	}
	COMP_CATCHTHIS
}

template <>
void CJBlock<double>::inverse_DCT (CJBlock<short> &o_shifted_block)
{
	COMP_TRYTHIS

	// performs the IDCT transform by inversing the flow graph
	// The initial 8 multiplications are processed in the DEQUANTIZATION function !!! (F1 by 8 and others by 16)
	double tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	double tmp10, tmp11, tmp12, tmp13;
	double z5, z10, z11, z12, z13;
	double v;
	static const double c141 = 1.414213562;
	static const double c184 = 1.847759065;
	static const double c108 = 1.082392200;
	static const double c261 = -2.613125930;
	unsigned int i, j;
	
	// Process columns
	for (i=0 ; i<8 ; i++)
	{
		tmp0 = Cget (0, i);
		tmp4 = Cget (1, i);
		tmp1 = Cget (2, i);
		tmp5 = Cget (3, i);
		tmp2 = Cget (4, i);
		tmp6 = Cget (5, i);
		tmp3 = Cget (6, i);
		tmp7 = Cget (7, i);

		// Optimization for a complete zero column
		if (! (tmp1 || tmp2 || tmp3 || tmp4 || tmp5 || tmp6 || tmp7))
		{
			Cset (0, i, tmp0);
			Cset (1, i, tmp0);
			Cset (2, i, tmp0);
			Cset (3, i, tmp0);
			Cset (4, i, tmp0);
			Cset (5, i, tmp0);
			Cset (6, i, tmp0);
			Cset (7, i, tmp0);
			continue;
		}

		// Even part

		tmp10 = tmp0 + tmp2;	/* phase 3 */
		tmp11 = tmp0 - tmp2;
	
		tmp13 = tmp1 + tmp3;	/* phases 5-3 */
		tmp12 = (tmp1 - tmp3) * c141 - tmp13; /* 2*c4 */

		tmp0 = tmp10 + tmp13;	/* phase 2 */
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;

		// Odd part
		
		z13 = tmp6 + tmp5;		/* phase 6 */
		z10 = tmp6 - tmp5;
		z11 = tmp4 + tmp7;
		z12 = tmp4 - tmp7;

		tmp7 = z11 + z13;		/* phase 5 */
		//tmp11 = (z11 - z13) * c141; /* 2*c4 */

		z5 = (z10 + z12) * c184; /* 2*c2 */
		//tmp10 = c108 * z12 - z5; /* 2*(c2-c6) */
		//tmp12 = c261 * z10 + z5; /* -2*(c2+c6) */

		tmp6 = c261 * z10 + z5 - tmp7;	/* phase 2 */
		tmp5 = (z11 - z13) * c141 - tmp6;
		tmp4 = c108 * z12 - z5 + tmp5;

		Cset (0, i, (tmp0 + tmp7));
		Cset (1, i, (tmp1 + tmp6));
		Cset (2, i, (tmp2 + tmp5));
		Cset (3, i, (tmp3 - tmp4));
		Cset (4, i, (tmp3 + tmp4));
		Cset (5, i, (tmp2 - tmp5));
		Cset (6, i, (tmp1 - tmp6));
		Cset (7, i, (tmp0 - tmp7));
	}

	// process rows
	for (i=j=0 ; i<64 ;)
	{
		tmp0 = Cget (i++);
		tmp4 = Cget (i++);
		tmp1 = Cget (i++);
		tmp5 = Cget (i++);
		tmp2 = Cget (i++);
		tmp6 = Cget (i++);
		tmp3 = Cget (i++);
		tmp7 = Cget (i++);

		// Even part
		tmp10 = tmp0 + tmp2;
		tmp11 = tmp0 - tmp2;

		tmp13 = tmp1 + tmp3;
		tmp12 = (tmp1 - tmp3) * c141 - tmp13;
		
		tmp0 = tmp10 + tmp13;
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;

		/* Odd part */
	
	    z13 = tmp6 + tmp5;
	    z10 = tmp6 - tmp5;
	    z11 = tmp4 + tmp7;
	    z12 = tmp4 - tmp7;
	
	    tmp7 = z11 + z13;
	    //tmp11 = (z11 - z13) * c141;
	
	    z5 = (z10 + z12) * c184; /* 2*c2 */
	    //tmp10 = c108 * z12 - z5; /* 2*(c2-c6) */
	    //tmp12 = c261 * z10 + z5; /* -2*(c2+c6) */

		tmp6 = c261 * z10 + z5 - tmp7;
		tmp5 = (z11 - z13) * c141 - tmp6;
		tmp4 = c108 * z12 - z5 + tmp5;

		/* Final output stage: scale down by a sampling factor and range-limit */
		o_shifted_block.Cset (j++, (short)((v = (tmp0 + tmp7) / 64.0) >= 0 ? v + 0.5 : v - 0.5));
		o_shifted_block.Cset (j++, (short)((v = (tmp1 + tmp6) / 64.0) >= 0 ? v + 0.5 : v - 0.5));
		o_shifted_block.Cset (j++, (short)((v = (tmp2 + tmp5) / 64.0) >= 0 ? v + 0.5 : v - 0.5));
		o_shifted_block.Cset (j++, (short)((v = (tmp3 - tmp4) / 64.0) >= 0 ? v + 0.5 : v - 0.5));
		o_shifted_block.Cset (j++, (short)((v = (tmp3 + tmp4) / 64.0) >= 0 ? v + 0.5 : v - 0.5));
		o_shifted_block.Cset (j++, (short)((v = (tmp2 - tmp5) / 64.0) >= 0 ? v + 0.5 : v - 0.5));
		o_shifted_block.Cset (j++, (short)((v = (tmp1 - tmp6) / 64.0) >= 0 ? v + 0.5 : v - 0.5));
		o_shifted_block.Cset (j++, (short)((v = (tmp0 - tmp7) / 64.0) >= 0 ? v + 0.5 : v - 0.5));
	}
	COMP_CATCHTHIS
}


template <class T>
void CJBlock<T>::inverse_DCT (CJBlock<short> &o_shifted_block)
{
	// is only supposed to operate on double
	Assert(0, Util::CParamException());
}


// disable warning about  "CJBlock<short>' is already instantiated" because
// it doesn't seems to be true

#pragma warning (disable: 4660)  

template class CJBlock <__int8>;
template class CJBlock <short>;
template class CJBlock <__int32>;
template class CJBlock <unsigned __int8>;
template class CJBlock <unsigned short>;
template class CJBlock <float>;
template class CJBlock <double>;

#pragma warning (default: 4660)  

} // end namespace COMP
