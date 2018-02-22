//-----------------------------------------------------------------------------
//
//  File        : MSG_data_text.cpp
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
#include "MSG_data_text.h"

MSG_data_text::MSG_data_text( )
{
  tbuff = 0; tsize = 0;
}

MSG_data_text::MSG_data_text( unsigned const char_1 *buff, size_t datasize )
{
  tbuff = 0; tsize = 0;
  this->read_from(buff, datasize);
}

MSG_data_text::~MSG_data_text( )
{
  if (tbuff) delete[] tbuff;
}

size_t MSG_data_text::read_from( unsigned const char_1 *buff, size_t datasize )
{
  tsize = datasize;
  if (tsize == 0)
  {
    std::cerr << "Invalid TEXT message size." << std::endl;
    throw;
  }

  tbuff = new unsigned char_1[tsize];
  memcpy(tbuff, buff, tsize);
  return tsize;
}

void MSG_data_text::dump( )
{
  std::ofstream outs;
  outs.open("text_dump.bin");
  outs.write((char_1 *) tbuff, tsize);
  return;
}

void MSG_data_text::dump(char *filename)
{
  std::ofstream outs;
  outs.open(filename);
  outs.write((char_1 *) tbuff, tsize);
  return;
}

void MSG_data_text::dump(std::string filename)
{
  std::ofstream outs;
  outs.open(filename.c_str( ));
  outs.write((char_1 *) tbuff, tsize);
  return;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_text &t )
{
  os << "------------------------------------------------------" << std::endl
     << "-           MSG ALPHANUMERIC TEXT FILE               -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << "****************" << std::endl;
  os << t.tbuff << std::endl;
  os << "****************" << std::endl;
  return os;
}
