//-----------------------------------------------------------------------------
//
//  File        : MSG_data_image.cpp
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

#include <vector>
#include <cstring>

#include "Compress.h"
#include "CJPEGDecoder.h"
#include "CWTDecoder.h"
#include "T4Decoder.h"
#include "CompressT4.h"


#include "MSG_data_image.h"

void MSG_data_image_encoded::decode( MSG_data_image *dec )
{
  long long dlen = len * 8;

  uint_1 *ibuf = new uint_1[len];
  memcpy(ibuf, data, len);

  Util::CDataFieldCompressedImage cdata =
                    Util::CDataFieldCompressedImage(ibuf, dlen, bpp, nx, ny);

  Util::CDataFieldUncompressedImage udata;
  std::vector <short> QualityInfo;

  switch (format)
  {
    case MSG_JPEG_FORMAT:
      COMP::DecompressJPEG(cdata, bpp, udata, QualityInfo);
      break;
    case MSG_WAVELET_FORMAT:
      COMP::DecompressWT(cdata, bpp, udata, QualityInfo);
      break;
    case MSG_T4_FORMAT:
      COMP::DecompressT4(cdata, udata, QualityInfo);
      break;
    default:
      std::cerr << "Unknown compression used." << std::endl;
      throw;
  }

  COMP::CImage cimg(udata);

  int decnum = nx * ny;
  dec->data = new MSG_SAMPLE[decnum];
  memcpy(dec->data, cimg.Get( ), decnum*sizeof(MSG_SAMPLE));
  dec->len  = decnum;


  return;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_image &i )
{
  for (int pix = 0; pix < (int) i.len; pix ++)
    os << i.data[pix] << std::endl;
  return os;
}
