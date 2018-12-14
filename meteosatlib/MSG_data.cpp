//-----------------------------------------------------------------------------
//
//  File        : MSG_data.cpp
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

#include <string>
#include <fstream>
#include "MSG_data.h"

#define BYTE_SWAP2(x) \
    (((x & 0xFF00) >> 8) | \
     ((x & 0x00FF) << 8))

std::ostream& operator<< ( std::ostream& os, MSG_data_level_15_header &h )
{
  os << h.sat_status
     << h.image_acquisition
     << h.celestial_events
     << h.image_description
     << h.radiometric_proc
     << h.geometric_proc
     << h.IMPF_config;
  return os;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_level_15_trailer &t )
{
  os << t.product_stats
     << t.navig_result
     << t.radiometric_qlty
     << t.geometric_qlty
     << t.timelin_comple;
  return os;
}

MSG_data::MSG_data( )
{
  key_message  = 0;
  gts_message  = 0;
  text_message = 0;
  prologue     = 0;
  epilogue     = 0;
  image        = 0;
  sdsprologue  = 0;
}

MSG_data::MSG_data( std::ifstream &in, MSG_header &header)
{
  read_from(in, header);
}

MSG_data::~MSG_data( )
{
  if (key_message) delete key_message;
  if (gts_message) delete gts_message;
  if (text_message) delete text_message;
  if (prologue) delete prologue;
  if (epilogue) delete epilogue;
  if (image) delete image;
  if (sdsprologue) delete sdsprologue;
}

void MSG_data::read_from( std::ifstream &in, MSG_header &header )
{
  size_t dsize;
  size_t dpos;
  unsigned char_1 *dbuff = 0;
  unsigned char_1 *dpnt = 0;

  typecode = header.f_typecode;
  switch (typecode)
  {
    case MSG_FILE_IMAGE_DATA:
      image = new MSG_data_image;

      dsize = header.data_field_length / 8;
      dbuff = new unsigned char_1[dsize];
      in.read((char *) dbuff, dsize);

      if (in.fail( ))
      {
        std::cerr << "Read error from HRIT file: Data field." << std::endl;
        throw;
      }


      if (header.image_structure->compression_flag == MSG_NO_COMPRESSION)
      {
        image->len = dsize;
        image->data = (MSG_SAMPLE *) ( new char[dsize] );
        memcpy(image->data, dbuff, dsize);
      }
      else
      {
        MSG_data_image_encoded encoded;
        encoded.data   = dbuff;
        encoded.len    = dsize;
        encoded.bpp    = header.image_structure->number_of_bits_per_pixel;
        encoded.nx     = header.image_structure->number_of_columns;
        encoded.ny     = header.image_structure->number_of_lines;
        encoded.format = header.segment_id->data_field_format;
        encoded.decode( image );
      }
      break;

    case MSG_FILE_GTS_MESSAGE:

      dsize = header.data_field_length / 8;
      dbuff = new unsigned char_1[dsize];
      in.read((char *) dbuff, dsize);
      if (in.fail( ))
      {
        std::cerr << "Read error from HRIT file: Data field." << std::endl;
        throw;
      }
      gts_message = new MSG_data_gts;
      dpos = gts_message->read_from(dbuff, dsize);
      {
        std::string outname;
        outname = header.annotation->annotation;
        outname += ".bin";
        gts_message->dump(outname);
      }

      break;

    case MSG_FILE_ALPHANUMERIC_TEXT:

      dsize = header.data_field_length / 8;
      dbuff = new unsigned char_1[dsize];
      in.read((char *) dbuff, dsize);
      if (in.fail( ))
      {
        std::cerr << "Read error from HRIT file: Data field." << std::endl;
        throw;
      }
      text_message = new MSG_data_text;
      dpos = text_message->read_from(dbuff, dsize);


      break;

    case MSG_FILE_ENCRYPTION_KEY_MESSAGE:

      dsize = header.data_field_length / 8;
      dbuff = new unsigned char_1[dsize];
      in.read((char *) dbuff, dsize);
      if (in.fail( ))
      {
        std::cerr << "Read error from HRIT file: Data field." << std::endl;
        throw;
      }
      key_message = new MSG_data_key;
      dpos = key_message->read_from(dbuff, dsize);

      break;

    case MSG_FILE_REPEAT_CYCLE_PROLOGUE:
      dsize = header.data_field_length / 8;

      dbuff = new unsigned char_1[dsize];
      in.read((char *) dbuff, dsize);
      if (in.fail( ))
      {
        std::cerr << "Read error from HRIT file: Data field." << std::endl;
        throw;
      }

      if (header.annotation->product_id_1.find("MSG")
              != std::string::npos ||
          header.annotation->product_id_1.find("GOMS1_4")
              != std::string::npos)
      {
        prologue = new MSG_data_level_15_header;

        dpnt = dbuff;
        dpos = prologue->sat_status.read_from(dpnt);
        dpnt += dpos;
        dpos = prologue->image_acquisition.read_from(dpnt);
        dpnt += dpos;
        dpos = prologue->celestial_events.read_from(dpnt);
        dpnt += dpos;
        dpos = prologue->image_description.read_from(dpnt);
        dpnt += dpos;
        dpos = prologue->radiometric_proc.read_from(dpnt);
        dpnt += dpos;
        dpos = prologue->geometric_proc.read_from(dpnt);
        dpnt += dpos;
      }
      else if (header.annotation->product_id_1.find("GOES")
                      != std::string::npos ||
               header.annotation->product_id_1.find("GMS")
                      != std::string::npos ||
               header.annotation->product_id_1.find("MTSAT")
                      != std::string::npos ||
               header.annotation->product_id_1.find("NOAA")
                      != std::string::npos ||
               header.annotation->product_id_1.find("METOP")
                      != std::string::npos)

      {
        dpnt = dbuff;
        sdsprologue = new MSG_data_SGS_header;
        dpos = sdsprologue->read_from(dpnt);
        dpnt += dpos;
      }
      else if (header.annotation->product_id_1.find("MET5")
                      != std::string::npos)
      {
        dpnt = dbuff;
        std::cout << "I have no info on MET5 file format !!!!" << std::endl;
      }
      else
        std::cout << "Unknown Image prologue file content annotation->product_id : " << header.annotation->product_id_1 << std::endl;
      break;

    case MSG_FILE_REPEAT_CYCLE_EPILOGUE:
      dsize = header.data_field_length / 8;
      dbuff = new unsigned char_1[dsize];
      in.read((char *) dbuff, dsize);
      if (in.fail( ))
      {
        std::cerr << "Read error from HRIT file: Data field." << std::endl;
        throw;
      }

      if (header.annotation->product_id_1.find("MSG")
              != std::string::npos ||
          header.annotation->product_id_1.find("GOMS1_4")
              != std::string::npos)
      {

        epilogue = new MSG_data_level_15_trailer;

        dpnt = dbuff+1;
        dpos = 1;

        dpos = epilogue->product_stats.read_from(dpnt);
        dpnt += dpos;
        dpos = epilogue->navig_result.read_from(dpnt);
        dpnt += dpos;
        dpos = epilogue->radiometric_qlty.read_from(dpnt);
        dpnt += dpos;
        dpos = epilogue->geometric_qlty.read_from(dpnt);
        dpnt += dpos;
        dpos = epilogue->timelin_comple.read_from(dpnt);
        dpnt += dpos;
      }
      else
        std::cout << "Unknown Image epilogue file content" << std::endl;
      break;

    case MSG_FILE_DCP_MESSAGE:
      break;

    case MSG_FILE_BINARY_MESSAGE:

      dsize = header.data_field_length / 8;
      dbuff = new unsigned char_1[dsize];
      in.read((char *) dbuff, dsize);
      if (in.fail( ))
      {
        std::cerr << "Read error from HRIT file: Data field." << std::endl;
        throw;
      }
      {
        std::ofstream os;
        std::string outname;
        outname = header.annotation->annotation;
        outname += ".bin";
        os.open(outname.c_str( ));
        if (!os.good())
          throw;
        os.write((char *) dbuff, (size_t) dsize);
        os.close( );
      }
      break;

    default:
      std::cerr << "Unknown MSG file type " << header.f_typecode << std::endl;
      throw;
      break;
  }

  if (dbuff) delete [ ] dbuff;

  return;
}

void MSG_data::read_from_himawari( std::ifstream &in, MSG_header &header )
{
  size_t dsize;
  size_t dpos;
  unsigned char_1 *dbuff = 0;
  unsigned char_1 *dpnt = 0;

  typecode = header.f_typecode;
  switch (typecode)
  {
    case MSG_FILE_IMAGE_DATA:
      image = new MSG_data_image;

      dsize = header.data_field_length / 8;
      dbuff = new unsigned char_1[dsize];
      in.read((char *) dbuff, dsize);
      if (in.fail( ))
      {
        std::cerr << "Read error from HRIT file: Data field." << std::endl;
        throw;
      }

      if (header.image_structure->compression_flag == MSG_NO_COMPRESSION)
      {
        image->len = dsize;
        image->data = (MSG_SAMPLE *) ( new char[dsize] );
        memcpy(image->data, dbuff, dsize);
      }
      break;


    default:
      std::cerr << "Unknown MSG file type " << header.f_typecode << std::endl;
      throw;
      break;
  }

  if (dbuff) delete [ ] dbuff;

  return;
}

std::ostream& operator<< ( std::ostream& os, MSG_data &h )
{
  os << "------------------------------------------------------" << std::endl
     << "-                  MSG MESSAGE DATA                  -" << std::endl
     << "------------------------------------------------------" << std::endl;
  switch (h.typecode)
  {
    case MSG_FILE_IMAGE_DATA:
      os << *(h.image);
      break;
    case MSG_FILE_GTS_MESSAGE:
      os << *(h.gts_message);
      break;
    case MSG_FILE_ALPHANUMERIC_TEXT:
      os << *(h.text_message);
      break;
    case MSG_FILE_ENCRYPTION_KEY_MESSAGE:
      os << *(h.key_message);
      break;
    case MSG_FILE_REPEAT_CYCLE_PROLOGUE:
      os << *(h.prologue);
      break;
    case MSG_FILE_REPEAT_CYCLE_EPILOGUE:
      os << *(h.epilogue);
      break;
    case MSG_FILE_DCP_MESSAGE:
      os << "DCP message." << std::endl;
      break;
    case MSG_FILE_BINARY_MESSAGE:
      os << "Binary encoded message." << std::endl;
      break;
    default:
      os << "Unknown MSG file type " << h.typecode << std::endl;
      break;
  }
  return os;
}
