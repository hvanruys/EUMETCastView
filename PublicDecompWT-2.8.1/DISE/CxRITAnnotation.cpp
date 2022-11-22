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

#include <iomanip>
#include <stdio.h>
#include <string.h>
#include "CxRITAnnotation.h"	// DISE




namespace DISE
{




// Description:	Repaces hyphens with underscores.
// Returns:		Nothing.
static void HyphenToUnderscore
(
	std::string& t_String	// Input string. May contain hyphens.
)
{
	for (std::string::size_type i = std::string::npos;
			(i = t_String.find_first_of('-')) != std::string::npos; )
		t_String[i] = '_';
}




CxRITAnnotation::CxRITAnnotation
(
		const bool			i_HRITFlag,			// true : HRIT file.
												// false: LRIT file.
		const std::string&	i_SpacecraftID,		// Disseminating spacecraft identifier.
		const std::string&	i_ProductID1,		// Product ID (1).
		const std::string&	i_ProductID2,		// Product ID (2).
		const std::string&	i_ProductID3,		// Product ID (3).
		const std::string&	i_ProductID4,		// Product ID (4).
		const bool			i_CompressedFlag,	// true : File contains compressed data.
												// false: File contains non-compressed data.
		const bool			i_EncryptedFlag		// true : File contains encrypted data.
												// false: File contains non-encrypted data.
)
: m_HRITFlag      (i_HRITFlag      )
, m_SpacecraftID  (i_SpacecraftID  )
, m_ProductID1    (i_ProductID1    )
, m_ProductID2    (i_ProductID2    )
, m_ProductID3    (i_ProductID3    )
, m_ProductID4    (i_ProductID4    )
, m_CompressedFlag(i_CompressedFlag)
, m_EncryptedFlag (i_EncryptedFlag )
{
	try
	{
		// Replace '-' with '_' to avoid ambiguities
		// ('-' is used as delimiter in annotation texts).
		HyphenToUnderscore(m_SpacecraftID);
		HyphenToUnderscore(m_ProductID1  );
		HyphenToUnderscore(m_ProductID2  );
		HyphenToUnderscore(m_ProductID3  );
		HyphenToUnderscore(m_ProductID4  );

		// Adjust length of strings.
		m_SpacecraftID.resize(e_LengthSpacecraftID, '_');
		m_ProductID1  .resize(e_LengthProductID1,   '_');
		m_ProductID2  .resize(e_LengthProductID2,   '_');
		m_ProductID3  .resize(e_LengthProductID3,   '_');
		m_ProductID4  .resize(e_LengthProductID4,   '_');
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




CxRITAnnotation::CxRITAnnotation
(
	const std::string& i_Text
)
{
	try
	{
		PRECONDITION(i_Text.size() <= e_MaxAnnotationLength);

		std::string Temp(i_Text);
		for (unsigned int i = 0; i < Temp.size(); ++i)
			if (Temp[i] == '-')
				Temp[i] = ' ';

		char LRITorHRIT;
		int  version;
		char spacecraft[e_MaxAnnotationLength + 1];
		char productID1[e_MaxAnnotationLength + 1];
		char productID2[e_MaxAnnotationLength + 1];
		char productID3[e_MaxAnnotationLength + 1];
		char productID4[e_MaxAnnotationLength + 1];
		char flags     [e_MaxAnnotationLength + 1];

		int items = sscanf(Temp.c_str(), "%c%d%s%s%s%s%s%s", &LRITorHRIT,
															 &version,
															 spacecraft,
															 productID1,
															 productID2,
															 productID3,
															 productID4,
															 flags		);
		PRECONDITION(items == 8 && strlen(flags) == 2);

		PRECONDITION(       version     == e_Version           );
		PRECONDITION(strlen(spacecraft) == e_LengthSpacecraftID);
		PRECONDITION(strlen(productID1) == e_LengthProductID1  );
		PRECONDITION(strlen(productID2) == e_LengthProductID2  );
		PRECONDITION(strlen(productID3) == e_LengthProductID3  );
		PRECONDITION(strlen(productID4) == e_LengthProductID4  );

		m_HRITFlag       = (LRITorHRIT == 'H' || LRITorHRIT == 'h') ? true : false;
		m_SpacecraftID   = std::string(spacecraft);	m_SpacecraftID.resize(e_LengthSpacecraftID, '_');
		m_ProductID1     = std::string(productID1);	m_ProductID1.resize(  e_LengthProductID1,   '_');
		m_ProductID2     = std::string(productID2);	m_ProductID2.resize(  e_LengthProductID2,   '_');
		m_ProductID3     = std::string(productID3);	m_ProductID3.resize(  e_LengthProductID3,   '_');
		m_ProductID4     = std::string(productID4);	m_ProductID4.resize(  e_LengthProductID4,   '_');
		m_CompressedFlag = (flags[0] == 'C' || flags[0] == 'c') ? true : false;
		m_EncryptedFlag  = (flags[1] == 'E' || flags[1] == 'e') ? true : false;
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




std::string CxRITAnnotation::GetText
(
)
const
{
	try
	{
		char temp[e_MaxAnnotationLength + 1];

		sprintf(temp, "%c-%03d-%.6s-%.12s-%.9s-%.9s-%.12s-%c%c",
												m_HRITFlag ? 'H' : 'L',
												e_Version,
												m_SpacecraftID.c_str(),
												m_ProductID1.c_str(),
												m_ProductID2.c_str(),
												m_ProductID3.c_str(),
												m_ProductID4.c_str(),
												m_CompressedFlag ? 'C' : '_',
												m_EncryptedFlag  ? 'E' : '_');
		return std::string(temp);
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




std::string CxRITAnnotation::GetIdentifier
(
)
const
{
	try
	{
		char temp[e_MaxAnnotationLength + 1];

		sprintf(temp, "%c-%03d-%.6s-%.12s-%.9s-%.9s-%.12s-__",
												m_HRITFlag ? 'H' : 'L',
												e_Version,
												m_SpacecraftID.c_str(),
												m_ProductID1.c_str(),
												m_ProductID2.c_str(),
												m_ProductID3.c_str(),
												m_ProductID4.c_str()	);
		return std::string(temp);
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}














} // end namespace
