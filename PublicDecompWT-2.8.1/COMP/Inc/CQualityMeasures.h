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

#ifndef CQUALITYMEASURES_H_INCLUDED
#define CQUALITYMEASURES_H_INCLUDED

/*******************************************************************************

TYPE:
Concrete class

PURPOSE:
Provide support for image quality measures (see also RS/OFO.330).

FUNCTION:
This object encapsulates methods to compute various quality indicators
on images such as RMS, maximum error....

INTERFACES:
See 'INTERFACES' in the module declaration below

RESOURCES:
Heap Memory (>2K).

REFERENCE:
None.

PROCESSING:
Upon construction, two CImage object are created by unpacking the
provided CDataFieldUncompresseImage.
The various statistics are computed on the fly from error image (which
is never explicitely computed).
The public member function GetMeanError(), GetRMSError(), GetMAError() and
GetErrorHistogram() can then be called to obtain respectively the mean error,
the RMS error, the Maximum absolute error and the error histogram between the
two images.
It should be noted that the error histogram is shifted by 1 << 15 in the 
returned vector.

DATA:
None

LOGIC:

*******************************************************************************/

#include <vector>

#include "CDataField.h"	// Util
#include "CImage.h"		// COMP


namespace COMP
{


class CQualityMeasures
{

private:

// DATA:

	CImage m_refImg;						// Reference Image
	CImage m_testImg;						// test Image
	double m_meanError;						// mean error
	double m_RMSError;						// RMS Error
	unsigned __int16 m_MAError;				// Maximum absolute error
	std::vector<unsigned long> m_HistoError;// Error histogram

public:

// INTERFACES:

	// Description:	Constructor.
	//				Performs all the computations.
	//				The statistics are being computed based on i_testImg - i_refImg
	//				Both images should have the same size.
	// Returns:		Nothing.
	CQualityMeasures
	(
		const Util::CDataFieldUncompressedImage &i_refImg,	// Reference image to which
															// the test image will be compared.
        const Util::CDataFieldUncompressedImage &i_testImg	// Image to be compared.
	);

	// Description:	Destructor
	// Returns:		Nothing.
	~CQualityMeasures();

	// Description:	Returns the pre-computed MeanError.
	// Returns:		A double representing the Mean Error.
	double GetMeanError(void) const;

	// Description:	Returns the pre-computed RMSError.
	// Returns:		A double representing the RMS error.
	double GetRMSError(void) const;

	// Description:	Returns the pre-computed Maximum Absolute error.
	// Returns:		An integer representing the maximum absolute error.
	unsigned short GetMAError(void) const;

	// Description:	Returns the error histogram, shifted by 1 << 15.
	//				For instance, HistoError[1<<15] gives the number of pixels having no error.
	// Returns:		A vector containing the histogram.
	std::vector<unsigned long> GetErrorHistogram(void) const;

};


} // end namespace


#endif
