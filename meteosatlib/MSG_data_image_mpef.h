//-----------------------------------------------------------------------------
//
//  File        : MSG_data_image_mpef.h
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

#ifndef __MSG_DATA_IMAGE_MPEF_H__
#define __MSG_DATA_IMAGE_MPEF_H__

typedef enum {
  MSG_MPEF_image_atmospheric_motion_vector_low_level    = 1,
  MSG_MPEF_image_atmospheric_motion_vector_medium_level = 2,
  MSG_MPEF_image_atmospheric_motion_vector_high_level   = 3,
  MSG_MPEF_image_cloud_analysis_image                   = 4,
  MSG_MPEF_image_cloud_top_height                       = 5
} t_enum_MSG_MPEF_image_product_type;

typedef enum {
  MSG_MPEF_gts_atmospheric_motion_vector = 1,
  MSG_MPEF_gts_cloud_analysis            = 2,
  MSG_MPEF_gts_clear_sky_radiance        = 3,
  MSG_MPEF_gts_tropospheric_humidity     = 4
} t_enum_MSG_MPEF_GTS_product_type;

typedef enum {
  MSG_MPEF_AMVIL = 1,
  MSG_MPEF_AMVIM = 2,
  MSG_MPEF_AMVIH = 3,
  MSG_MPEF_CLAI  = 4,
  MSG_MPEF_CTH   = 5,
  MSG_MPEF_AMV   = 6,
  MSG_MPEF_CSR   = 7,
  MSG_MPEF_TH    = 8
} t_enum_MSG_MPEF_product_type_code;

typedef int GP_CONFIG_ITEM_VERSION;

typedef struct {
  char          pad1[2];
  short int     ExpectedImageStart;
  unsigned char ImageReceivedFlag;
  char          pad2;
  short int     UsedImageStart;
  char          pad3[2];
  short int     UsedImageEnd;
} t_MSG_MPEF_image_details;

typedef struct {
  unsigned short int       MPEF_File_Id;
  unsigned char            MPEF_header_version;
  unsigned char            ManualDissAuthRequested;
  unsigned char            ManualDisseminationAuth;
  short int                NominalTime;
  unsigned char            ProductQuality[100];
  unsigned char            ProductCompleteness[100];
  unsigned char            ProductTimeliness[100];
  char                     InstanceId[2];
  t_MSG_MPEF_image_details ImagesUsed;
  GP_CONFIG_ITEM_VERSION   BaseAlgorithmVersion; 
  GP_CONFIG_ITEM_VERSION   ProductAlgorithmVersion; 
  char                     Filler[52];
} t_MSG_MPEF_product_header;

typedef struct {
  unsigned char       AMVProductHeaderVersion;
  unsigned char       ProcessingSegmentWidth;
  unsigned char       ProcessingSegmentHeight;
  unsigned short int  NoVectorsInProduct;
  unsigned short int  NoVectorsPerBand[12];
  unsigned short int  NoVectorsPassAQC;
  unsigned short int  NoVectorsPassPerBand[12];
} t_MSG_MPEF_AMV_header;

typedef struct {
  unsigned char AMVIHProductHeaderVersion;
  char          Filler[95];
} t_MSG_MPEF_AMVIH_header;

typedef struct {
  unsigned char AMVILProductHeaderVersion;
  char          Filler[95];
} t_MSG_MPEF_AMVIL_header;

typedef struct {
  unsigned char AMVIMProductHeaderVersion;
  char          Filler[95];
} t_MSG_MPEF_AMVIM_header;

typedef struct {
  unsigned char      CLAProductHeaderVersion;
  unsigned char      ProcessingSegmentWidth;
  unsigned char      ProcessingSegmentHeight;
  unsigned short int NoSegmentsInProduct;
} t_MSG_MPEF_CLA_header;

typedef struct {
  unsigned char CLAIProductHeaderVersion;
  char          Filler[95];
} t_MSG_MPEF_CLAI_header;

typedef struct {
  unsigned char      CSRProductHeaderVersion;
  unsigned char      ProcessingSegmentWidth;
  unsigned char      ProcessingSegmentHeight;
  unsigned short int NoSegmentsInProduct;
} t_MSG_MPEF_CSR_header;

typedef struct {
  unsigned char CTHProductHeaderVersion;
  char          Filler[95];
} t_MSG_MPEF_CTH_header;

typedef struct {
  unsigned char      THProductHeaderVersion;
  unsigned char      ProcessingSegmentWidth;
  unsigned char      ProcessingSegmentHeight;
  unsigned short int NoSegmentsInProduct;
} t_MSG_MPEF_TH_header;

typedef union {
  t_MSG_MPEF_AMV_header   AMV_header;
  t_MSG_MPEF_AMVIH_header AMVIH_header;
  t_MSG_MPEF_AMVIM_header AMVIM_header;
  t_MSG_MPEF_AMVIL_header AMVIL_header;
  t_MSG_MPEF_CLA_header   CLA_header;
  t_MSG_MPEF_CLAI_header  CLAI_header;
  t_MSG_MPEF_CSR_header   CSR_header;
  t_MSG_MPEF_CTH_header   CTH_header;
  t_MSG_MPEF_TH_header    TH_header;
} t_MSG_MPEF_product_specific_header;

class MSG_MPEF_product {
  public:
    MSG_MPEF_product( ) { }
    ~MSG_MPEF_product( ) { }

  private:

    const static int MSG_MPEF_image_product_pixels        = 1280;
    const static int MSG_MPEF_image_product_lines         = 1280;
    const static int MSG_MPEF_image_product_segment_files = 20;

    const static int MSG_MPEF_overlay_product_pixels = 11136;
    const static int MSG_MPEF_overlay_product_lines  = 11136;

    t_MSG_MPEF_product_header               product_header;
    t_MSG_MPEF_product_specific_header      product_specific_header;
    static char                             Data_Definition_Block[1024];

};

#endif
