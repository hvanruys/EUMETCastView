//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
//
//  File        : MSG_header_timestamp.cpp
//  Description : MSG HRIT-LRIT format interface
//  Project     : Meteosatlib
//  Authors     : Graziano Giuliani (Lamma Regione Toscana)
//              : Enrico Zini <enrico@enricozini.com>
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

#include "MSG_HRIT.h"

bool MSG_HRIT::open(char *name)
{
  hrit_ifstream.open(name, (std::ios_base::binary | std::ios_base::in));
  if (hrit_ifstream.fail( ))
  {
    std::cerr << "Cannot open input hrit file " << name << std::endl;
    return false;
  }
  return true;
}

void MSG_HRIT::read( )
{
  l15_head.read_from(hrit_ifstream);
  l15_data.read_from(hrit_ifstream, l15_head);
  return;
}

void MSG_HRIT::close( )
{
  hrit_ifstream.close( );
}

std::ostream& operator<< ( std::ostream& os, const MSG_HRIT &hrit )
{
  os << (MSG_header&) hrit.l15_head << (MSG_data&) hrit.l15_data;
  return os;
}

#ifdef TESTME

int main(int argc, char **argv)
{
  MSG_HRIT hrit;

  if (! hrit.open(argv[1]))
    return 1;

  hrit.read( );

  std::cout << hrit;

  hrit.close( );
  return 0;
}

#endif
