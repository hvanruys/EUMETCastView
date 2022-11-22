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

#ifndef CompressJPEG_included
#define CompressJPEG_included

/*******************************************************************************

TYPE:
Concrete classes and free functions.
					
PURPOSE:
Data structure for JPEG compression parameters.
Classes for the JPEG compression algorithm.
Free functions for JPEG decompression.

FUNCTION:
Performs compression and decompression of image-type data according to the JPEG
interchange format.

INTERFACES:
See 'INTERFACES:' in the module declaration below.

RESOURCES:	
Heap Memory (>2K).

REFERENCES:
None.

PROCESSING:
Derived from the abstract base class CCompress, the concrete classes
- CCompressLosslessJPEG,
- CCompressLossyJPEG,
provide the algorithm specific parameters and Compress() methods.

- DecompressJPEG() decompresses JPEG-coded input image data.

For passing image data to and from the routines, Util::CDataField... objects are
used.

DATA:
See 'DATA:' in the class header below.

LOGIC:		
-

*******************************************************************************/

#include <sstream>
#include <string>
#include <vector>


#include "CDataField.h"		// Util
#include "SmartPtr.h"		// Util
#include "ErrorHandling.h"	// Util
#include "RMAErrorHandling.h"	// COMP
#include "Compress.h"		// COMP




namespace COMP
{




// Huffman coding table.
class CHuffmanTable
{

public:

	enum {	e_SizeofBitsTable		= 16 + 1,
			e_SizeofSymbolsTable	= 256,
			e_HTIdentifier			= 0xFFC4 };

// DATA:

	// Bits[i] = number of symbols with codes of i bits length.
	unsigned short m_bits[e_SizeofBitsTable];

	// Symbols in order of increasing code length.
	unsigned short m_symbols[e_SizeofSymbolsTable];

// INTERFACES:

	// Description:	Constructor. Loads table from a text configuration file.
	// Returns:		Nothing.
	CHuffmanTable
	(
		const std::string& i_FileName	// Path and name of configuration file.
	);

	// Description:	Destructor.
	// Returns:		Nothing.
	virtual ~CHuffmanTable
	(
	)
	{
	}

	// Description:	Default constructor.
	// Returns:		Nothing.
	CHuffmanTable
	(
	);

	// Description:	Loads table from a text configuration file
	// Returns:		Nothing.
	void ReadFromFile
	(
		const std::string& i_FileName	// Path and name of configuration file.
	);

	// Description:	Saves table to a text configuration file.
	// Returns:		Nothing.
	void SaveToFile
	(
		const std::string& i_FileName	// Path and name of configuration file.
	);

};


// Quantization table.
class CQuantizeTable
{

public:

	enum {	e_SizeofQuantizationTable	= 64,
			e_QTIdentifier				= 0xFFDB };

// DATA:

	// Values of quantization table.
	unsigned short m_Values[e_SizeofQuantizationTable];

// INTERFACES:

	// Description:	Default constructor.
	// Returns:		Nothing.
	CQuantizeTable
	(
	);

	// Description:	Constructor. Loads table from a text configuration file.
	// Returns:		Nothing.
	CQuantizeTable
	(
		const std::string& i_FileName	// Path and name of configuration file.
	);

	// Description:	Destructor.
	// Returns:		Nothing.
	virtual ~CQuantizeTable
	(
	)
	{
	}

	// Description:	Loads table from a text configuration file
	// Returns:		Nothing.
	void ReadFromFile
	(
		const std::string& i_FileName	// Path and name of configuration file.
	);

	// Description:	Saves table to a text configuration file.
	// Returns:		Nothing.
	void SaveToFile
	(
		const std::string& i_FileName	// Path and name of configuration file.
	);

	// Description:	Provide read-only or read/write access to table.
	// Returns:		Nothing.
		  unsigned short& operator[](int i_Index)       { return m_Values[i_Index]; }
	const unsigned short& operator[](int i_Index) const { return m_Values[i_Index]; }

};


// Container class for JPEG compression/decompression parameters.
class CJPEGParams
{

public:

	// Enumeration of compression modes.
	enum ECompressionMode
	{
		e_Unknown		= 0,	// Not yet known.
		e_LossyJPEG		= 1,	// Lossy JPEG compression
		e_LosslessJPEG	= 2		// Lossless JPEG compression
	};

// DATA:

	ECompressionMode	m_Mode;					// JPEG compression mode.

	unsigned int		m_BitsPerPixel;			// This parameter set is intended to be
												// applied to images with the specified
												// number of bits per pixel.

	unsigned int		m_RestartInterval;		// Number of image lines between
												// subsequent restart markers.

	unsigned int		m_Predictor;			// Predictor (0..7), lossless mode only.
	unsigned int		m_PointTransform;		// Point transform, lossless mode only.

	ERequantizationMode	m_RequantizationMode;	// Requantization mode,
												// for lossy 10bpp images only.

	CQuantizeTable		m_QuantizationTable;	// Coefficient quantization table. Used
												// to quantize the 8x8 DCT coefficients.
												// Values are given in zigzag order.

	unsigned int		m_QualityFactor;		// Quality factor, used to scale the
												// quantization table (0..100).

	CHuffmanTable		m_dcTable;				// DC Huffman coding tables.
	CHuffmanTable		m_acTable;				// AC Huffman coding tables.

	bool				m_OptimizedCoding;		// true : Use optimized tables.
												// false: Use default tables.

public:

// INTERFACES:

	// Description:	Constructor.
	//				Parameter set shall be used for compression in JPEG lossless mode.
	// Returns:		Nothing.
	CJPEGParams
	(
		unsigned int	i_BitsPerPixel,
		unsigned int	i_RestartInterval,
		unsigned int	i_Predictor,
		unsigned int	i_PointTransform,
		CHuffmanTable&	i_dcTable,
		bool			i_OptimizedCoding
	)
	: m_Mode				(e_LosslessJPEG			)
	, m_BitsPerPixel		(i_BitsPerPixel			)
	, m_RestartInterval		(i_RestartInterval		)
	, m_Predictor			(i_Predictor			)
	, m_PointTransform		(i_PointTransform		)
	, m_RequantizationMode	(e_NoRequantization		)
	, m_dcTable				(i_dcTable				)
	, m_OptimizedCoding		(i_OptimizedCoding		)
	{
		PRECONDITION(m_BitsPerPixel >=  2 && m_BitsPerPixel <= 16	);
		PRECONDITION(	m_Predictor >  0	 &&
						m_Predictor <= 7		);
	}

	// Description:	Constructor.
	//				Parameter set shall be used for compression in JPEG lossy mode.
	// Returns:		Nothing.
	CJPEGParams
	(
		unsigned int		i_BitsPerPixel,
		unsigned int		i_RestartInterval,
		ERequantizationMode	i_RequantizationMode,
		CQuantizeTable&		i_QuantizationTable,
		unsigned int		i_QualityFactor,
		CHuffmanTable&		i_dcTable,
		CHuffmanTable&		i_acTable,
		bool				i_OptimizedCoding
	)
	: m_Mode				(e_LossyJPEG			)
	, m_BitsPerPixel		(i_BitsPerPixel			)
	, m_RestartInterval		(i_RestartInterval		)
	, m_RequantizationMode	(i_RequantizationMode	)
	, m_QuantizationTable	(i_QuantizationTable	)
	, m_QualityFactor		(i_QualityFactor		)
	, m_dcTable				(i_dcTable				)
	, m_acTable				(i_acTable				)
	, m_OptimizedCoding		(i_OptimizedCoding		)
	{
		switch (m_RequantizationMode)
		{
			case e_10to12		:
			case e_10to8_floor	:
			case e_10to8_floor1	:
			case e_10to8_round	:
			case e_10to8_ceil	:	PRECONDITION(m_BitsPerPixel == 10);	break;
		}
	}

	// Description:	Default constructor. Used during decompression.
	// Returns:		Nothing.
	CJPEGParams
	(
	)
	: m_Mode				(e_Unknown				)
	, m_BitsPerPixel		(0						)
	, m_RestartInterval		(0						)
	, m_Predictor			(0						)
	, m_PointTransform		(0						)
	, m_RequantizationMode	(e_NoRequantization		)
	, m_QuantizationTable	(						)
	, m_QualityFactor		(0						)
	, m_dcTable				(						)
	, m_acTable				(						)
	, m_OptimizedCoding		(false					)
	{
	}

	// Description:	Denstructor.
	// Returns:		Nothing.
	virtual ~CJPEGParams
	(
	)
	{
	}

	// Description:	Retruns the set of compression parameters in string form.
	// Returns:		Compression parameters in readable form.
	std::string GetTraceString
	(
	)
	const
	{
		std::ostringstream temp;

		temp <<	  "Compression Mode        : " << (int)m_Mode
			 <<	"\nBits Per Pixel          : " << m_BitsPerPixel
			 <<	"\nRestart Interval        : " << m_RestartInterval
			 <<	"\nPredictor               : " << m_Predictor
			 <<	"\nPoint Transform         : " << m_PointTransform
			 <<	"\nRequantization Mode     : " << (int)m_RequantizationMode
				// m_QuantizationTable omitted.
			 <<	"\nQuality Factor          : " << m_QualityFactor
				// m_dcTable omitted.
				// m_acTable omitted.
			 <<	"\nOptimized Coding        : " << (int)m_OptimizedCoding;
					
		return temp.str();
	}

};




// Parameters and Compress() function for lossless JPEG compression.
class CCompressLosslessJPEG : public COMP::CCompress
							, public COMP::CJPEGParams
{

public:

// INTERFACES:

	// Description:	Compresses the input image data according to
	//				the specified parameters.
	// Returns:		Compressed data field.
	Util::CDataFieldCompressedImage Compress
	(
		const Util::CDataFieldUncompressedImage& i_Image	// Image to compress.
	)
	const;

	// Description:	Constructor.
	// Returns:		Nothing.
	CCompressLosslessJPEG
	(
		const CJPEGParams& i_Params
	)
	: CCompress		(			)
	, CJPEGParams	(i_Params	)
	{
		Assert(i_Params.m_Mode == e_LosslessJPEG, Util::CParamException());
	}

	// Description:	Default constructor.
	//				Only to be used when objects are received from a network stream.
	// Returns:		Compressed data field.
	CCompressLosslessJPEG
	(
	)
	: CCompress		()
	, CJPEGParams	()
	{
	}

	// Description:	Tells whether compression is lossless or lossy.
	// Returns:		true if compression is lossless, i.e. if the 
	//				point transform is zero.
	bool IsLossless
	(
	)
	const
	{
		return (m_PointTransform == 0);
	}

	// Description:	Retruns the set of compresion parameters in string form.
	// Returns:		Compression parameters in readable form.
	std::string GetTraceString
	(
	)
	const
	{
		return CJPEGParams::GetTraceString();
	}

};




// Parameters and Compress() function for lossy JPEG compression.
class CCompressLossyJPEG :	  public COMP::CCompress
							, public COMP::CJPEGParams
{

public:

// INTERFACES:

	// Description:	Compresses the input image data according to
	//				the specified parameters.
	// Returns:		Compressed data field.
	Util::CDataFieldCompressedImage Compress
	(
		const Util::CDataFieldUncompressedImage& i_Image	// Image to compress.
	)
	const;

	// Description:	Constructor.
	// Returns:		Nothing.
	CCompressLossyJPEG
	(
		const CJPEGParams& i_Params
	)
	: CCompress		(			)
	, CJPEGParams	(i_Params	)
	{
		Assert(i_Params.m_Mode == e_LossyJPEG, Util::CParamException());
	}

	// Description:	Default constructor.
	//				Only to be used when objects are received from a network stream.
	// Returns:		Compressed data field.
	CCompressLossyJPEG
	(
	)
	: CCompress		()
	, CJPEGParams	()
	{
	}

	// Description:	Tells whether compression is lossless or lossy.
	// Returns:		false: Compression is lossy.
	bool IsLossless
	(
	)
	const
	{
		return false;
	}

	// Description:	Returns the set of compresion parameters in string form.
	// Returns:		Compression parameters in readable form.
	std::string GetTraceString
	(
	)
	const
	{
		return CJPEGParams::GetTraceString();
	}

};




// Description:	Decompresses input image data which are coded according
//				to the JPEG standard.
// Returns:		Nothing.
void DecompressJPEG
(
	const Util::CDataFieldCompressedImage&
							i_Image,			// Compressed image data.
	const unsigned char&	i_NR,				// Desired image representation [bits/pixel].
	Util::CDataFieldUncompressedImage&
							o_ImageDecompressed,// Decompressed image data.
	std::vector<short>&		o_QualityInfo		// Vector with one field per image line.
												// The function will store there the number
												// of 'good' pixels, counted from the first
												// column.
												// If there are no errors, the values will be
												// equal to the total number of image columns.
);




} // end namespace


#endif
