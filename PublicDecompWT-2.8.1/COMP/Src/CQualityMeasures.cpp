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
//      FileName:       CQualityMeasures.cpp
//      
//      Date Created:   28/9/1998
//
//      Author:         Xavier Neyt
//
//
//      Description:    CQualityMeasures class declaration.
//
//
//      Last Modified:  $Dmae: 1999/03/01 20:07:41 $
//
//      RCS Id:  $Id: CQualityMeasures.cpp,v 1.7 1999/03/01 20:07:41 xne Exp $
//
//		$Rma: CQualityMeasures.cpp,v $
//		Revision 1.7  1999/03/01 20:07:41  xne
//		(Re)introducing CVS/RCS commit messages
//
//////////////////////////////////////////////////////////////////////////
//:End Ignore

#include <math.h>

#include "CQualityMeasures.h"

namespace COMP {
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CQualityMeasures::CQualityMeasures (const Util::CDataFieldUncompressedImage &i_refImg, 
								    const Util::CDataFieldUncompressedImage &i_testImg):
	m_refImg (i_refImg),
	m_testImg (i_testImg)
{
	COMP_TRYTHIS
	unsigned __int16 minError = 0; // positive, but to be interpreted as negative
	unsigned __int16 maxError = 0; // always positive

	// Do both images have the same size?
	Assert (m_refImg.GetH() == m_testImg.GetH(), Util::CParamException());
	Assert (m_refImg.GetW() == m_testImg.GetW(), Util::CParamException());

	m_meanError	= 0;
	m_RMSError	= 0;
	m_MAError	= 0;
	signed __int32 error = 0; // the error is the difference of two 16bpp images

	m_HistoError.resize (1 << 16);
	unsigned short zeroposition = 1 << 15;

	for (unsigned long i=0; i<m_refImg.GetSize() ; i++)
	{
		error = m_testImg.Cget(i) - m_refImg.Cget(i);
		m_meanError += error;
		m_RMSError += error * error;
		m_HistoError [error + zeroposition] ++;
		if (error < 0) error = -error;    // equivalent of abs()
		m_MAError = (m_MAError > (unsigned __int16)error) ? m_MAError : error;
	}
	m_meanError /= m_refImg.GetSize();
	m_RMSError = sqrt (m_RMSError / m_refImg.GetSize());
	COMP_CATCHTHIS
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CQualityMeasures::~CQualityMeasures ()
{
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
double CQualityMeasures::GetMeanError(void) const
{
	return m_meanError;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
double CQualityMeasures::GetRMSError(void) const
{
	return m_RMSError;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
unsigned short CQualityMeasures::GetMAError(void) const
{
	return m_MAError;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
std::vector<unsigned long> CQualityMeasures::GetErrorHistogram(void) const
{
	return m_HistoError;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


} // end namespace COMP

