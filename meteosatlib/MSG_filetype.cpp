//-----------------------------------------------------------------------------
//
//  File        : MSG_filetype.cpp
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

#include "MSG_filetype.h"
#include "MSG_hrit_specdoc.h"

std::string MSG_filetype(t_enum_MSG_filetype filetype)
{
  std::string s_ftypecode;
  switch (filetype)
  {
    case MSG_FILE_IMAGE_DATA:
      s_ftypecode = "MSG Image Data File";
      break;
    case MSG_FILE_GTS_MESSAGE:
      s_ftypecode = "MSG GTS Message File";
      break;
    case MSG_FILE_ALPHANUMERIC_TEXT:
      s_ftypecode = "MSG Alphanumeric Text File";
      break;
    case MSG_FILE_ENCRYPTION_KEY_MESSAGE:
      s_ftypecode = "MSG Encryption Key Message File";
      break;
    case MSG_FILE_REPEAT_CYCLE_PROLOGUE:
      s_ftypecode = "MSG Repeat Cycle Prologue";
      break;
    case MSG_FILE_REPEAT_CYCLE_EPILOGUE:
      s_ftypecode = "MSG Repeat Cycle Epilogue";
      break;
    case MSG_FILE_DCP_MESSAGE:
      s_ftypecode = "MSG DCP Message (unprocessed)";
      break;
    case MSG_FILE_BINARY_MESSAGE:
      s_ftypecode = "MSG Binary File Message";
      break;
    default:
      s_ftypecode = "MSG Mission Unknown file typecode as of ";
      s_ftypecode += msg_global_hrit_specdoc;
      break;
  }
  return s_ftypecode;
}
