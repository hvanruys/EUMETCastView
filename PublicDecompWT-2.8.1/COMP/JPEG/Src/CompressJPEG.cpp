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

#include "ErrorHandling.h"
#include "CompressJPEG.h"
#include "CJPEGLossLessCoder.h"
#include "CJPEGLossyCoder.h"
#include "CJPEGDecoder.h"

namespace COMP
{




CHuffmanTable::CHuffmanTable(const std::string& i_FileName)
{
	try
	{
		ReadFromFile (i_FileName);
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}


CHuffmanTable::CHuffmanTable()
{
	try
	{
		for (int b = 0; b < e_SizeofBitsTable   ; ++b)	m_bits[b]    = 0;
		for (int s = 0; s < e_SizeofSymbolsTable; ++s)	m_symbols[s] = 0;
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}


void CHuffmanTable::ReadFromFile(const std::string& i_FileName)
{
	try
	{
		std::ifstream inputStream(i_FileName.c_str());
		Assert(inputStream.good(), Util::CParamException());

		// Read marker.
		int HTId;
		inputStream >> HTId;
		Assert(HTId == e_HTIdentifier, Util::CParamException());
		Assert(inputStream.good()    , Util::CCLibException()   );
		                                     
		unsigned int sCount = 0;
		m_bits[0] = 0;
		for (int b = 1; b < e_SizeofBitsTable   ; ++b)
		{
			inputStream >> m_bits[b];
			sCount += m_bits[b];
			Assert(inputStream.good(), Util::CCLibException());
		}
		Assert(sCount <= e_SizeofSymbolsTable, Util::CParamException());
		    
		    unsigned int s;
		for (s = 0; s < sCount; ++s)
		{
			inputStream >> m_symbols[s];
			Assert(inputStream.good(), Util::CCLibException());
		}
		for (; s < e_SizeofSymbolsTable; ++s)
			m_symbols[s] = 0;
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




void CHuffmanTable::SaveToFile(const std::string& i_FileName)
{
	try
	{
		std::ofstream outputStream(i_FileName.c_str());
		Assert(outputStream.good(), Util::CCLibException());

		// Write marker.
		outputStream << (int)e_HTIdentifier << std::endl;
		Assert(outputStream.good(), Util::CCLibException());

		unsigned int sCount = 0;
		for (int b = 1; b < e_SizeofBitsTable   ; ++b)
		{
			outputStream << m_bits[b] << std::endl;
			sCount += m_bits[b];
			Assert(outputStream.good(), Util::CCLibException());
		}
		Assert(sCount <= e_SizeofSymbolsTable, Util::CParamException());

		outputStream << std::endl;
		Assert(outputStream.good(), Util::CCLibException());

		for (unsigned int s = 0; s < sCount ; ++s)
		{
			outputStream << m_symbols[s] << std::endl;
			Assert(outputStream.good(), Util::CCLibException());
		}
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




CQuantizeTable::CQuantizeTable()
{
	try
	{
		for (int q = 0; q < e_SizeofQuantizationTable; ++q)	m_Values[q] = 1;
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}


CQuantizeTable::CQuantizeTable(const std::string& i_FileName)
{
	try
	{
		ReadFromFile (i_FileName);
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}



void CQuantizeTable::ReadFromFile(const std::string& i_FileName)
{
	try
	{
		std::ifstream inputStream(i_FileName.c_str());
		Assert(inputStream.good(), Util::CParamException());

		// Read marker.
		int QTId;
		inputStream >> QTId;
		Assert(QTId == e_QTIdentifier, Util::CParamException());
		Assert(inputStream.good()    , Util::CCLibException()   );

		// Read the coefficients.
		for (int q = 0; q < e_SizeofQuantizationTable; ++q)
		{
			inputStream >> m_Values[q];
			Assert(inputStream.bad() == false, Util::CCLibException());
		}
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




void CQuantizeTable::SaveToFile(const std::string& i_FileName)
{
	try
	{
		std::ofstream outputStream(i_FileName.c_str());
		Assert(outputStream.good(), Util::CCLibException());

		// Write marker.
		outputStream << (int)e_QTIdentifier << std::endl;
		Assert(outputStream.good(), Util::CCLibException());

		// Write the coefficients.
		for (int q = 0; q < e_SizeofQuantizationTable; ++q)
		{
			outputStream << m_Values[q] << std::endl;
			Assert(outputStream.good(), Util::CCLibException());
		}
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




Util::CDataFieldCompressedImage CCompressLosslessJPEG::Compress (
			const Util::CDataFieldUncompressedImage &i_uncompressedImage)
			const
{
	COMP_TRYTHIS
	CJPEGLossLessCoder c (i_uncompressedImage, (CJPEGParams)(*this));
	c.CodeBuffer ();
	return c.GetCompressedImage ();
	COMP_CATCHTHIS
}

Util::CDataFieldCompressedImage CCompressLossyJPEG::Compress (
			const Util::CDataFieldUncompressedImage &i_uncompressedImage)
			const
{
	COMP_TRYTHIS
	CJPEGLossyCoder c (i_uncompressedImage, (CJPEGParams)(*this));
	c.CodeBuffer ();
	return c.GetCompressedImage ();
	COMP_CATCHTHIS
}

void DecompressJPEG (const Util::CDataFieldCompressedImage   &i_compressedImage, 
					    const unsigned char &i_NR,
	 				    Util::CDataFieldUncompressedImage &o_decompressedImage,
					    std::vector<short> &o_QualityInfo)
{	
	COMP_TRYTHIS
	CJPEGDecoder d (i_compressedImage);  // initialize the beast
	d.DecodeBuffer ();                  // decode & fill QualityInfo array
	o_decompressedImage = d.GetDecompressedImage (i_NR); 
	o_QualityInfo       = d.GetQualityInfo ();
	COMP_CATCHTHIS
}

} // end namespace

