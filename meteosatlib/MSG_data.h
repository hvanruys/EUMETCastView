//-----------------------------------------------------------------------------
//
//  File        : MSG_data.h
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

#ifndef __MSG_DATA_H__
#define __MSG_DATA_H__

#include <fstream>
#include <iostream>
#include <string>


#include "MSG_hrit_specdoc.h"
#include "MSG_machine.h"
#include "MSG_filetype.h"

#include "MSG_header.h"


#include "MSG_data_image.h"
#include "MSG_data_key.h"
#include "MSG_data_gts.h"
#include "MSG_data_text.h"
#include "MSG_data_SatelliteStatus.h"
#include "MSG_data_ImageAcquisition.h"
#include "MSG_data_CelestialEvents.h"
#include "MSG_data_ImageDescription.h"
#include "MSG_data_RadiometricProc.h"
#include "MSG_data_GeometricProc.h"
#include "MSG_data_ImageProdStats.h"
#include "MSG_data_NavigExtrResult.h"
#include "MSG_data_RadiometricQuality.h"
#include "MSG_data_GeometricQuality.h"
#include "MSG_data_TimelinComple.h"
#include "MSG_data_IMPFConfiguration.h"
#include "MSG_data_SGS_header.h"


class MSG_data_level_15_header {
  public:

    MSG_data_SatelliteStatus sat_status;
    MSG_data_ImageAcquisition image_acquisition;
    MSG_data_CelestialEvents celestial_events;
    MSG_data_ImageDescription image_description;
    MSG_data_RadiometricProc radiometric_proc;
    MSG_data_GeometricProc geometric_proc;
    MSG_data_IMPFConfiguration IMPF_config;

    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_data_level_15_header &h );
};


class MSG_data_level_15_trailer {
  public:
    MSG_data_ImageProdStats product_stats;
    MSG_data_NavigExtrResult navig_result;
    MSG_data_RadiometricQuality radiometric_qlty;
    MSG_data_GeometricQuality geometric_qlty;
    MSG_data_TimelinComple timelin_comple;
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_data_level_15_trailer &t );
};

class MSG_data {
  public:
    MSG_data( );
    MSG_data( std::ifstream &in, MSG_header &header);
    ~MSG_data( );

    MSG_data_key *key_message;
    MSG_data_gts *gts_message;
    MSG_data_text *text_message;
    MSG_data_level_15_header *prologue;
    MSG_data_level_15_trailer *epilogue;
    MSG_data_SGS_header *sdsprologue;
    MSG_data_image *image;

    void read_from( std::ifstream &in, MSG_header &header );
    void read_from_himawari( std::ifstream &in, MSG_header &header );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os, MSG_data &h );

  private:
    t_enum_MSG_filetype typecode;
};

#endif
