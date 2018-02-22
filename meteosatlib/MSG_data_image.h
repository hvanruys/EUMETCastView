//-----------------------------------------------------------------------------
//
//  File        : MSG_data_image.h
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

#ifndef __MSG_DATA_IMAGE_H__
#define __MSG_DATA_IMAGE_H__

#include "MSG_data_format.h"
#include "MSG_machine.h"

typedef enum {
  MSG_SEVIRI_IMAGE                   = 0,
  MSG_SEVIRI_HRV_IMAGE               = 1,
  MSG_FOREIGN_SATELLITE_IMAGE        = 2,
  MSG_MPEF_IMAGE_CLAI                = 3,
  MSG_MPEF_IMAGE_CTH                 = 4,
  MSG_SAF_IMAGE                      = 5,
  MSG_MPEF_OVERLAY_AMVIL             = 6,
  MSG_MPEF_OVERLAY_AMVIM             = 7,
  MSG_MPEF_OVERLAY_AMVIH             = 8,
  MSG_SERVICE_OVERLAY                = 9,
  MSG_TEST_COMPRESSION_SERVICE_IMAGE = 10,
  MSG_TEST_ENCRYPTION_SERVICE_IMAGE  = 11
} t_MSG_image_data_type;


typedef uint_2 MSG_SAMPLE;

class MSG_data_image {
  public:

    MSG_data_image( )
    {
      data = 0;
      len = 0;
    }

    ~MSG_data_image( )
    {
      if (data) delete [ ] data;
    }

    t_MSG_image_data_type type;
    size_t len;
    MSG_SAMPLE *data;
    friend std::ostream& operator<< ( std::ostream& os, MSG_data_image &i );

};

class MSG_data_image_encoded {
  public:
    size_t len;
    uint_1 *data;
    int nx;
    int ny;
    int bpp;
    t_enum_MSG_data_format format;

    void decode( MSG_data_image *dec );

};


#endif
