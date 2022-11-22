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

#ifndef CxRITAnnotation_included
#define CxRITAnnotation_included

/*******************************************************************************

TYPE:
Concrete Class.
					
PURPOSE:
Container for annotation text for LRIT/HRIT file header records.

FUNCTION:
This class allows to construct an annotation text for a newly assembled LRIT/HRIT
file and to modify it as processing proceeds.

INTERFACES:
See 'INTERFACES:' in the module declaration below.

RESOURCES:	
Not applicable.

REFERENCES:
LRIT/HRIT Mission Specific Implementation MSG/SPE/057.

PROCESSING:
The class CxRITAnnotation has the following member functions:
- A constructor combines takes as input the various components of which an
  annotation text can be built.
- Another constructor takes a complete annotation text string as input parameter.
- The compression and encryption flags can be set and cleared using the
  SetCompressed() and SetEncrypted() functions.
- Read access to specific components of the annotation text is provided by several
  Get...() member functions.
- GetText() returns the entire annotation text.

DATA:
See 'DATA:' in the class header below.

LOGIC:		
-

*******************************************************************************/

#include <string>

#include "ErrorHandling.h"			// Util
#include "YYYYMMDDhhmm.h"			// DISE




namespace DISE
{




class CxRITAnnotation
{

protected:

// DATA:

	enum {	e_MaxAnnotationLength	= 64,	// Max. total length of annotation text.
			e_Version				=  0,	// Annotation text layout version.
			e_LengthSpacecraftID	=  6,	// String length of Spacecraft ID.
			e_LengthProductID1		= 12,	// String length of Product ID (1).
			e_LengthProductID2		=  9,	// String length of Product ID (2).
			e_LengthProductID3		=  9,	// String length of Product ID (3).
			e_LengthProductID4		= 12 };	// String length of Product ID (4).

	bool		m_HRITFlag;			// true : HRIT file.
									// false: LRIT file.
	std::string	m_SpacecraftID;		// Disseminating spacecraft identifier.
	std::string	m_ProductID1;		// Product ID (1).
	std::string	m_ProductID2;		// Product ID (2).
	std::string	m_ProductID3;		// Product ID (3).
	std::string	m_ProductID4;		// Product ID (4).
	bool		m_CompressedFlag;	// true : File contains compressed data.
									// false: File contains non-compressed data.
	bool		m_EncryptedFlag;	// true : File contains encrypted data.
									// false: File contains non-encrypted data.

public:

// INTERFACES:

	// Description:	Constructor. Combines its input parameters to an annotation text.
	// Returns:		Nothing.
	CxRITAnnotation
	(
		const bool			i_HRITFlag     = false,		// true : HRIT file.
														// false: LRIT file.
		const std::string&	i_SpacecraftID = "_",		// Disseminating spacecraft identifier.
		const std::string&	i_ProductID1   = "_",		// Product ID (1).
		const std::string&	i_ProductID2   = "_",		// Product ID (2).
		const std::string&	i_ProductID3   = "_",		// Product ID (3).
		const std::string&	i_ProductID4   = "_",		// Product ID (4).
		const bool			i_CompressedFlag = false,	// true : File contains compressed data.
														// false: File contains non-compressed data.
		const bool			i_EncryptedFlag  = false	// true : File contains encrypted data.
														// false: File contains non-encrypted data.
	);

	// Description:	Constructor.
	// Returns:		Nothing.
	explicit CxRITAnnotation
	(
		const std::string&	i_Text		// Annotation text string.
	);

	// Description:	Destructor.
	// Returns:		Nothing.
	virtual ~CxRITAnnotation
	(
	)
	{
	}

	// Description:	Sets indicator in annotation text to 'file is compressed'.
	// Returns:		Nothing.
	void SetCompressed
	(
		const bool			i_Flag		// true : File contains compressed data.
										// false: File contains non-compressed data.
	)
	{
		m_CompressedFlag = i_Flag;
	}

	// Description:	Sets indicator in annotation text to 'file is compressed'.
	// Returns:		Nothing.
	void SetEncrypted
	(
		const bool			i_Flag		// true : File contains encrypted data.
										// false: File contains non-encrypted data.
	)
	{
		m_EncryptedFlag = i_Flag;
	}

	// Description:	Functions that provide read access to components of annotation text.
	// Returns:		Nothing.
	bool				GetHRITFlag()		const	{	return m_HRITFlag;			}
	const std::string&	GetSpacecraftID()	const	{	return m_SpacecraftID;		}
	const std::string&	GetProductID1()		const	{	return m_ProductID1;		}
	const std::string&	GetProductID2()		const	{	return m_ProductID2;		}
	const std::string&	GetProductID3()		const	{	return m_ProductID3;		}
	const std::string&	GetProductID4()		const	{	return m_ProductID4;		}
	bool				GetCompressedFlag()	const	{	return m_CompressedFlag;	}
	bool				GetEncryptedFlag()	const	{	return m_EncryptedFlag;		}

	// Description:	Returns the complete Annotation Text string.
	// Returns:		Complete Annotation Text string.
	std::string GetText
	(
	)
	const;

	// Description:	Returns an identifier string for use as key in Generic Server instances.
	// Returns:		The annotation text with compression and encryption indicators reset.
	std::string GetIdentifier
	(
	)
	const;

};



} // end namespace


#endif
