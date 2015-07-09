//-----------------------------------------------------------------------------
//
//  File        : MSG_data_gts.cpp
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
#include "MSG_data_gts.h"

MSG_data_gts::MSG_data_gts( )
{
  mbuff = 0; msize = 0;
}

MSG_data_gts::MSG_data_gts( unsigned const char_1 *buff, size_t datasize )
{
  mbuff = 0; msize = 0;
  this->read_from(buff, datasize);
}

MSG_data_gts::~MSG_data_gts( )
{
  if (mbuff) delete[] mbuff;
}

size_t MSG_data_gts::read_from( unsigned const char_1 *buff, size_t datasize )
{
  msize = datasize;
  if (msize == 0 || msize > (MSG_MAX_GTS_MESSAGE_SIZE*MSG_MAX_GTS_MESSAGES))
  {
    std::cerr << "Invalid GTS message(s) size." << std::endl;
    throw;
  }

  mbuff = new unsigned char_1[msize];
  memcpy(mbuff, buff, msize);
  return msize;
}

void MSG_data_gts::dump( )
{
  std::ofstream outs;
  outs.open("gts_dump.bin", std::ios_base::app);
  outs.write((char_1 *) mbuff, msize);
  return;
}

void MSG_data_gts::dump(char *filename)
{
  std::ofstream outs;
  outs.open(filename, std::ios_base::app);
  outs.write((char_1 *) mbuff, msize);
  return;
}

void MSG_data_gts::dump(std::string filename)
{
  std::ofstream outs;
  outs.open(filename.c_str( ));
  outs.write((char_1 *) mbuff, msize);
  return;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_gts &g )
{
  os << "------------------------------------------------------" << std::endl
     << "-                 MSG GTS MESSAGE                    -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << "****************" << std::endl;
  os << "****************" << std::endl;
  return os;
}
