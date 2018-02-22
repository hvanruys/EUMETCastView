//-----------------------------------------------------------------------------
//
//  File        : MSG_data_TimelinComple.cpp
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
#include <iomanip>
#include "MSG_data_TimelinComple.h"

size_t MSG_Timeliness::read_from( unsigned const char_1 *buff )
{
  MaxDelay = get_r4(buff);
  MinDelay = get_r4(buff+4);
  MeanDelay = get_r4(buff+8);
  return 12;
}

std::ostream& operator<< ( std::ostream& os, MSG_Timeliness &t )
{
  os << "Max Delay           : " << t.MaxDelay << std::endl
     << "Min Delay           : " << t.MinDelay << std::endl
     << "Mean Delay          : " << t.MeanDelay << std::endl;
  return os;
}

size_t MSG_Completeness::read_from( unsigned const char_1 *buff )
{
  PlannedL15ImageLines = get_ui2(buff);
  GeneratedL15ImageLines = get_ui2(buff+2);
  ValidL15ImageLines = get_ui2(buff+4);
  DummyL15ImageLines = get_ui2(buff+6);
  CorruptedL15ImageLines = get_ui2(buff+8);
  return 10;
}

std::ostream& operator<< ( std::ostream& os, MSG_Completeness &c )
{
  os << "Planned Image Lines : " << c.PlannedL15ImageLines << std::endl
     << "Generated Lines     : " << c.GeneratedL15ImageLines << std::endl
     << "Valid Lines         : " << c.ValidL15ImageLines << std::endl
     << "Dummy Lines         : " << c.DummyL15ImageLines << std::endl
     << "Corrupted Lines     : " << c.CorruptedL15ImageLines << std::endl;
  return os;
}

MSG_data_TimelinComple::MSG_data_TimelinComple( ) { }

MSG_data_TimelinComple::MSG_data_TimelinComple(unsigned const char_1 *buff)
{
  this->read_from(buff);
}

MSG_data_TimelinComple::~MSG_data_TimelinComple( ) { }

size_t MSG_data_TimelinComple::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  position += Timeliness.read_from(buff+position);
  for (int i = 0; i < 12; i ++)
    position += Completeness[i].read_from(buff+position);
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_TimelinComple &t )
{
  os << "------------------------------------------------------" << std::endl
     << "-         MSG IMAGE PRODUCTION STATISTICS            -" << std::endl
     << "------------------------------------------------------" << std::endl;
  os << t.Timeliness;
  for (int i = 0; i < 12; i ++)
    os << "Channel " << std::setw(2) << std::setfill('0') << i+1 << std::endl
       << t.Completeness[i];
  return os;
}
