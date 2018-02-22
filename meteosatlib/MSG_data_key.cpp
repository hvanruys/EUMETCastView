//-----------------------------------------------------------------------------
//
//  File        : MSG_data_key.cpp
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

#include <cstring>
#include "MSG_data_key.h"

MSG_data_key::MSG_data_key( )
{
  keys = 0;
}

MSG_data_key::MSG_data_key( unsigned const char_1 *buff, size_t datasize )
{
  keys = 0;
  this->read_from(buff, datasize);
}

MSG_data_key::~MSG_data_key( )
{
  if (keys) delete [ ] keys;
}

size_t MSG_data_key::read_from( unsigned const char_1 *buff, size_t datasize )
{
  nkeys = datasize / MSG_KEY_MESSAGE_LEN;
  if (nkeys > MSG_MAX_NUMBER_KEY_MESSAGES || nkeys <= 0)
  {
    std::cerr << "Key message number or size invalid." << std::endl;
    throw;
  }

  keys = new MSG_encryption_key[nkeys];
  unsigned char_1 *pnt = (unsigned char_1 *) buff;
  for (int i = 0; i < nkeys; i ++)
  {
    keys[i].User_Station_number = get_ui2(pnt);
    keys[i].Key_Number = *(pnt+1);
    memcpy(keys[i].Public_key, pnt+2, 192);
    keys[i].Public_Key_CRC = get_ui2(pnt+195);
    pnt = pnt + MSG_KEY_MESSAGE_LEN;
  }
  return datasize;
}

std::ostream& operator<< ( std::ostream& os, MSG_encryption_key &k )
{
  std::string keystr = k.Public_key;
  os << "User Station Number : " << (uint_2) k.User_Station_number
     << std::endl
     << "Key Number          : " << (uint_2) k.Key_Number << std::endl
     << "Public Key          : " << &keystr << std::endl
     << "Public Key CRC      : " << k.Public_Key_CRC << std::endl;
  return os;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_key &k )
{
  os << "------------------------------------------------------" << std::endl
     << "-            MSG ENCRYPTION KEY MESSAGE              -" << std::endl
     << "------------------------------------------------------" << std::endl;
  std::string keystr;
  for (int i = 0; i < k.nkeys; i ++)
  {
    os << "****************" << std::endl;
    os << k.keys[i];
  }
  os << "****************" << std::endl;
  return os;
}
