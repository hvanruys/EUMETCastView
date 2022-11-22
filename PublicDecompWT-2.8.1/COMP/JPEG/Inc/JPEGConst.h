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

/*******************************************************************************

TYPE:	

PURPOSE:	

FUNCTION:	
      
INTERFACES:	
      
RESOURCES:	
      
REFERENCE:	
None

PROCESSING:	

DATA:
None		
      
LOGIC:
      
*******************************************************************************/

#ifndef JPEGConst_included
#define JPEGConst_included


namespace COMP
{

const unsigned __int16 cMarkerSOI  = 0xFFD8;
const unsigned __int16 cMarkerDQT  = 0xFFDB;
const unsigned __int16 cMarkerDHT  = 0xFFC4;
const unsigned __int16 cMarkerSOF1 = 0xFFC0;
const unsigned __int16 cMarkerSOF2 = 0xFFC1;
const unsigned __int16 cMarkerSOF3 = 0xFFC3;
const unsigned __int16 cMarkerDRI  = 0xFFDD;
const unsigned __int16 cMarkerAPP  = 0xFFE0;
const unsigned __int16 cMarkerRST  = 0xFFD0;
const unsigned __int16 cMarkerSOS  = 0xFFDA;
const unsigned __int16 cMarkerEOI  = 0xFFD9;


} // end namespace


#endif

