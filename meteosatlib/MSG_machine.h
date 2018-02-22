//-----------------------------------------------------------------------------
//
//  File        : MSG_machine.h
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

#ifndef __MSG_MACHINE_H__
#define __MSG_MACHINE_H__

#include <string.h>
#include <ctype.h>
#include <stddef.h>

#define char_1 char

#define uint_1 unsigned char
#define uint_2 unsigned short int
#define uint_4 unsigned int
#define uint_8 unsigned long long

#define int_1 char
#define int_2 short int
#define int_4 int
#define int_8 long long

#define real_4 float
#define real_8 double

void check_endianess();

int_1 get_i1(const unsigned char_1 *buff);
int_2 get_i2(const unsigned char_1 *buff);
int_4 get_i4(const unsigned char_1 *buff);
int_8 get_i8(const unsigned char_1 *buff);

uint_1 get_ui1(const unsigned char_1 *buff);
uint_2 get_ui2(const unsigned char_1 *buff);
uint_4 get_ui4(const unsigned char_1 *buff);
uint_8 get_ui8(const unsigned char_1 *buff);

real_4 get_r4(const unsigned char_1 *buff);
real_8 get_r8(const unsigned char_1 *buff);

void get_mem(const unsigned char_1 *source, unsigned char_1 *dest,
             size_t nbytes);

bool is_big( );

#endif
