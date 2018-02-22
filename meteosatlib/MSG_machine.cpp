//-----------------------------------------------------------------------------
//
//  File        : MSG_machine.cpp
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
#include "MSG_machine.h"

static bool isbig = false;

void check_endianess()
{
  union {
    unsigned char_1 byte[4];
    int val;
  } word;
  word.val = 0;
  word.byte[3] = 0x1;
  if (word.val != 1)
    isbig = false;
  else
    isbig = true;
}

bool is_big( )
{
  return isbig;
}

int_1 get_i1(const unsigned char_1 *buff)
{
  return *((int_1 *) buff);
}

int_2 get_i2(const unsigned char_1 *buff)
{
  if (isbig) return *((int_2 *) buff);
  char_1 i2[2];
  i2[0] = buff[1];
  i2[1] = buff[0];
  return *((int_2 *) i2);
}

int_4 get_i4(const unsigned char_1 *buff)
{
  if (isbig) return *((int_4 *) buff);
  char_1 i4[4];
  i4[0] = buff[3];
  i4[1] = buff[2];
  i4[2] = buff[1];
  i4[3] = buff[0];
  return *((int_4 *) i4);
}

int_8 get_i8(const unsigned char_1 *buff)
{
  if (isbig) return *((int_8 *) buff);
  char_1 i8[8];
  i8[0] = buff[7];
  i8[1] = buff[6];
  i8[2] = buff[5];
  i8[3] = buff[4];
  i8[4] = buff[3];
  i8[5] = buff[2];
  i8[6] = buff[1];
  i8[7] = buff[0];
  return *((int_8 *) i8);
}

uint_1 get_ui1(const unsigned char_1 *buff)
{
  return *((uint_1 *) buff);
}

uint_2 get_ui2(const unsigned char_1 *buff)
{
  if (isbig) return *((uint_2 *) buff);
  char_1 ui2[2];
  ui2[0] = buff[1];
  ui2[1] = buff[0];
  return *((uint_2 *) ui2);
}

uint_4 get_ui4(const unsigned char_1 *buff)
{
  if (isbig) return *((uint_4 *) buff);
  char_1 ui4[4];
  ui4[0] = buff[3];
  ui4[1] = buff[2];
  ui4[2] = buff[1];
  ui4[3] = buff[0];
  return *((uint_4 *) ui4);
}

uint_8 get_ui8(const unsigned char_1 *buff)
{
  if (isbig) return *((uint_8 *) buff);
  char_1 ui8[8];
  ui8[0] = buff[7];
  ui8[1] = buff[6];
  ui8[2] = buff[5];
  ui8[3] = buff[4];
  ui8[4] = buff[3];
  ui8[5] = buff[2];
  ui8[6] = buff[1];
  ui8[7] = buff[0];
  return *((uint_8 *) ui8);
}

real_4 get_r4( const unsigned char_1 *buff )
{
  if (isbig) return *((real_4 *) buff);
  unsigned char_1 sw[4];
  sw[0] = buff[3];
  sw[1] = buff[2];
  sw[2] = buff[1];
  sw[3] = buff[0];
  return *((real_4 *) sw);
}

real_8 get_r8( const unsigned char_1 *buff )
{
  if (isbig) return *((real_8 *) buff);
  unsigned char_1 sw[8];
  sw[0] = buff[7];
  sw[1] = buff[6];
  sw[2] = buff[5];
  sw[3] = buff[4];
  sw[4] = buff[3];
  sw[5] = buff[2];
  sw[6] = buff[1];
  sw[7] = buff[0];
  return *((real_8 *) sw);
}

void get_mem(const unsigned char_1 *source, unsigned char_1 *dest,
             size_t nbytes)
{
  memcpy(dest, source, nbytes);
  return;
}
