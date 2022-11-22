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

#ifndef CJBlock_included
#define CJBlock_included

/*******************************************************************************

TYPE:
Concrete Class.

PURPOSE:	
Handle JPEG lossy specific 8*8 matrix block.

FUNCTION:	
This class provides functions to store and access a generic 8*8 elements matrix.
      
INTERFACES:	
See 'INTERFACES' in the module declaration below.
      
RESOURCES:	
      
REFERENCE:	
None

PROCESSING:	
class CJBlock:
	CJBlock () : constructs one block without initializing its elements
	CJBlock (T &) contructs one block and initializes all the elements with the
	  specified value
	CJBlock (const CBlock<T> &) copy constructor
	level_shift : performs the block level shift
	level_unshift : performs the block level inverse shift
	differential_unshift :  compute the actual DC coeff. via the difference value
	  and the previous value
	differential_shift : replace the DC coeff. by the difference between the actual
	  value and the previous one
	forward_DCT : computes the fast FDCT for a shifted pixel block
	inverse_DCT : computes the fast IDCT to produce a shifted pixel block

DATA: No specific data members
      
LOGIC:
-

*******************************************************************************/

#include "ErrorHandling.h"
#include "CBlock.h"


namespace COMP
{


template <class T> class CJBlock : public COMP::CBlock<T>
{

private:

// DATA:

	// get the k th element in the ZZ order (see norm F.1.1.5)
	static const unsigned __int8 ZZ[64];

public:

// INTERFACES:

	// Description:	Default constructor.
	// Returns:		Nothing.
	CJBlock()
	{
	}

	// Description.	Constructor. Initialises all the data elements with the given value.
	// Returns:		Nothing.
	CJBlock
	(
		const T & i_val // initialization value
	)
	: CBlock<T> (i_val)
	{
	}

	// Description:	Copy constructor.
	// Returns:		Nothing.
	CJBlock
	(
		const CJBlock<T> & i_copy // JBlock to copy
	)
	: CBlock<T> (i_copy)
	{
	}

	// Description:	Destructor.
	// Returns:		Nothing.
	~CJBlock()
	{
	}

	// Description:	Performs the block level shift.
	// Returns:		Nothing.
	inline void level_shift
	(
		CJBlock<short>& o_block,
		const unsigned short& i_sub
	);

	// Description:	Performs the block level inverse shift.
	// Returns:		Nothing.
	inline void level_unshift
	(
		CJBlock<unsigned short>&	o_block,
		const unsigned short&		i_add
	);

	// Description:	Compute the actual DC coeff. via the difference value and the previous value.
	// Returns:		Nothing.
	inline void differential_unshift
	(
		T& t_last_DC	// at calling time : previous DC value,
						// at return time : current DC value	
	);

	// Description:	Replace the DC coeff. by the difference between the actual value
	//				and the previous one
	// Returns:		Nothing.
	inline void differential_shift
	(
		T& t_last_DC	// at calling time : previous DC value,
						// at return time : current DC value	
	);

	// Description:	Computes the fast FDCT for a shifted pixel block.
	// Returns:		Nothing.
	void forward_DCT
	(
		CJBlock <double>& o_FDCT_block	// FDCT coefficients
	);

	// Description:	Computes the fast IDCT to produce a shifted pixel block.
	// Returns:		Nothing.
	void inverse_DCT
	(
		CJBlock <short>&  o_shifted_block	// shifted pixel block
	);

	// Description:	Set all the elements of the object to 0.
	// Returns:		Nothing.
	inline void Zero()
	{
		COMP_TRYTHIS
		for (unsigned int i=0 ; i<64 ; i++) CBlock<T>::m_data[i] = 0;
		COMP_CATCHTHIS
	}

	// Description:	Gets the value of the specified element (Z-scan position).
	// Returns:		The value of the specified element.
	T CZget
	(
		const unsigned int &i_k	// Z-scan index
	)
	{
		COMP_TRYTHIS_SPEED
#ifdef _DEBUG
		Assert (i_k < 64, Util::CParamException());
#endif
		return CBlock<T>::m_data[ZZ[i_k]];
		COMP_CATCHTHIS_SPEED
	}

	// Description:	Puts the given value at the specified position (Z-scan position).
	// Returns:		Nothing.
	void CZset
	(
		const unsigned int&	i_k,	// Z-scan index
		const T&			i_val	// value
	)
	{
		COMP_TRYTHIS_SPEED
		// get the k th element in the ZZ order (see norm F.1.1.5)
#ifdef _DEBUG
		Assert (i_k < 64, Util::CParamException());
#endif
		CBlock<T>::m_data[ZZ[i_k]] = i_val;
		COMP_CATCHTHIS_SPEED
	}

};




// is only supposed to operate on unsigned short:
template<>
inline void CJBlock<short>::differential_unshift (short &t_last_DC)
{
	COMP_TRYTHIS_SPEED
	// transform the DC coefficient of Qblock by adding the last_DC value
	// last_DC is then set to the new value of the DC coefficient
	t_last_DC = (CBlock<short>::m_data[0] += t_last_DC);
	COMP_CATCHTHIS_SPEED
}

template <class T>
inline void CJBlock<T>::differential_unshift (T &t_last_DC)
{
	COMP_TRYTHIS
	Assert (0, Util::CParamException());
	COMP_CATCHTHIS
}

template <>
inline void CJBlock<short>::differential_shift (short &t_last_DC)
{
	COMP_TRYTHIS_SPEED
	// transform the DC coefficient of Qblock by substracting the last_DC value
	// last_DC is then set to the new value of the DC coefficient
	short tmp = CBlock<short>::m_data[0];
	CBlock<short>::m_data[0] -= t_last_DC;
	t_last_DC = tmp;
	COMP_CATCHTHIS_SPEED
}

template <class T>
inline void CJBlock<T>::differential_shift (T &t_last_DC)
{
	COMP_TRYTHIS
	Assert (0, Util::CParamException());
	COMP_CATCHTHIS
}

template <>
inline void CJBlock<unsigned short>::level_shift (CJBlock<short> &o_block, const unsigned short &i_sub)
{
	COMP_TRYTHIS_SPEED
	for (unsigned int i=0 ; i<64 ; i++) o_block.Cset (i, CBlock<unsigned short>::m_data[i] - i_sub);
	COMP_CATCHTHIS_SPEED
}

template <class T>
inline void CJBlock<T>::level_shift (CJBlock<short> &o_block, const unsigned short &i_sub)
{
	COMP_TRYTHIS
	Assert (0, Util::CParamException());
	COMP_CATCHTHIS
}

template <>
inline void CJBlock<short>::level_unshift (CJBlock<unsigned short> &o_block, const unsigned short &i_add)
{
	COMP_TRYTHIS_SPEED
	__int32 prov;
	
	for (unsigned int i=0 ; i<64 ; i++)
		// check range
		o_block.Cset (i, (prov = CBlock<short>::m_data[i] + i_add) < 0 ? 0 : prov > 65535 ? 65535 : (unsigned short)prov);
	COMP_CATCHTHIS_SPEED
}

template <class T>
inline void CJBlock<T>::level_unshift (CJBlock<unsigned short> &o_block, const unsigned short &i_add)
{
	COMP_TRYTHIS
	Assert (0, Util::CParamException());
	COMP_CATCHTHIS
}




} // end namespace


#endif
