//-----------------------------------------------------------------------------
//
//  File        : MSG_data_SGS_header.cpp
//  Description : MSG HRIT-LRIT format interface
//  Project     : Meteosatlib
//  Author      : Graziano Giuliani
//  References  : MSG/SPE/057 LRIT-HRIT Mission Specific Implementation,
//                V. 4.1 9 Mar 2001
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//-----------------------------------------------------------------------------

#include <fstream>
#include <cstring>
#include <iomanip>
#include "MSG_data_SGS_header.h"

MSG_SGSCommonHeader::MSG_SGSCommonHeader( ) { }

MSG_SGSCommonHeader::MSG_SGSCommonHeader(unsigned const char_1 *buff)
{
  this->read_from(buff);
}

MSG_SGSCommonHeader::~MSG_SGSCommonHeader( ) { }

size_t MSG_SGSCommonHeader::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;

  position += 4;
  position += NominalSGSProductTime.read_from(buff+position);
  SGSProductQuality = *(buff+position);
  position ++;
  SGSProductCompleteness = *(buff+position);
  position ++;
  SGSProductTimeliness = *(buff+position);
  position ++;
  SGSProcessingInstanceId = *(buff+position);
  position ++;
  memcpy(BaseAlgorithmVersion, buff+position, 16);
  position += 16;
  memcpy(ProductAlgorithmVersion, buff+position, 16);
  position += 16;
  
  return position;
}

std::ostream& operator<< (std::ostream& os, MSG_SGSCommonHeader &h)
{
  os << "Nom. SGS Prod. Time : " << h.NominalSGSProductTime.get_timestring( )
     << std::endl
     << "Product Quality     : " << (uint_2) h.SGSProductQuality
     << std::endl
     << "Product Completen.  : " << (uint_2) h.SGSProductCompleteness
     << std::endl
     << "Product Timeliness  : " << (uint_2) h.SGSProductTimeliness
     << std::endl
     << "Proc. Instance Id   : " << (uint_2) h.SGSProcessingInstanceId
     << std::endl
     << "Base Algorithm      : " << h.BaseAlgorithmVersion << std::endl
     << "Product Algorithm   : " << h.ProductAlgorithmVersion << std::endl;
  return os;
}

MSG_SGSImageProductSpecificHeader::MSG_SGSImageProductSpecificHeader( )
{
  ImageProductSpecificHeaderLenght = 0;
  ImageProductSpecificHeaderData = 0;
}

MSG_SGSImageProductSpecificHeader::MSG_SGSImageProductSpecificHeader(
                 unsigned const char_1 *buff)
{
  ImageProductSpecificHeaderLenght = 0;
  ImageProductSpecificHeaderData = 0;
  this->read_from(buff);
}

MSG_SGSImageProductSpecificHeader::~MSG_SGSImageProductSpecificHeader( )
{
  if (ImageProductSpecificHeaderData) delete [ ] ImageProductSpecificHeaderData;
}

size_t MSG_SGSImageProductSpecificHeader::read_from(unsigned const char_1 *buff)
{
  size_t position = 0;
  size_t datlen;

  position += 4;
  ImageProductSpecificHeaderLenght = get_ui4(buff+position);
  position += 4;
  datlen = ImageProductSpecificHeaderLenght - 8;
  ImageProductSpecificHeaderData = new uint_1[datlen];
  memcpy(ImageProductSpecificHeaderData, buff+position, datlen);
  position += datlen;
  return position;
}

void MSG_SGSImageProductSpecificHeader::dump( )
{
  std::ofstream dfile("hdump");
  dfile.write((const char *) ImageProductSpecificHeaderData+1,
              ImageProductSpecificHeaderLenght-9);
  dfile.close();
  return;
}

std::ostream& operator<<(std::ostream& os, MSG_SGSImageProductSpecificHeader &i)
{
  return os;
}

MSG_SGSNonImageProductSpecificHeader::MSG_SGSNonImageProductSpecificHeader( )
{
}

MSG_SGSProductSpecificHeader::MSG_SGSProductSpecificHeader( ) { }

MSG_SGSProductSpecificHeader::MSG_SGSProductSpecificHeader(
          unsigned const char_1 *buff)
{
  this->read_from(buff);
}

MSG_SGSProductSpecificHeader::~MSG_SGSProductSpecificHeader( ) { }

size_t MSG_SGSProductSpecificHeader::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  position += ImageProductSpecificHeader.read_from(buff);
  return position;
}

std::ostream& operator<< (std::ostream& os, MSG_SGSProductSpecificHeader &p)
{
  return os;
}

MSG_data_SGS_header::MSG_data_SGS_header( ) { }

MSG_data_SGS_header::MSG_data_SGS_header(unsigned const char_1 *buff)
{
  this->read_from(buff);
}

MSG_data_SGS_header::~MSG_data_SGS_header( ) { }

size_t MSG_data_SGS_header::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  position += SGSCommonHeader.read_from(buff+position);
  position += SGSProductSpecificHeader.read_from(buff+position);
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_SGS_header &h )
{
  os << "------------------------------------------------------" << std::endl
     << "-          MSG FOREIGN SATELLITE PROLOGUE            -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << h.SGSCommonHeader << h.SGSProductSpecificHeader;
  return os;
}
