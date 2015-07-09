//-----------------------------------------------------------------------------
//
//  File        : MSG_data_RadiometricQuality.cpp
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
#include <cstring>
#include <iomanip>
#include "MSG_data_RadiometricQuality.h"

size_t MSG_L10RadQuality::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  FullImageMinimumCount = get_ui2(buff+position);
  position += 2;
  FullImageMaximumCount = get_ui2(buff+position);
  position += 2;
  EarthDiskMinimumCount = get_ui2(buff+position);
  position += 2;
  EarthDiskMaximumCount = get_ui2(buff+position);
  position += 2;
  MoonMinimumCount = get_ui2(buff+position);
  position += 2;
  MoonMaximumCount = get_ui2(buff+position);
  position += 2;
  FullImageMeanCount = get_r4(buff+position);
  position += 4;
  FullImageStandardDeviation = get_r4(buff+position);
  position += 4;
  EarthDiskMeanCount = get_r4(buff+position);
  position += 4;
  EarthDiskStandardDeviation = get_r4(buff+position);
  position += 4;
  MoonMeanCount = get_r4(buff+position);
  position += 4;
  MoonStandardDeviation = get_r4(buff+position);
  position += 4;
  SpaceMeanCount = get_r4(buff+position);
  position += 4;
  SpaceStandardDeviation = get_r4(buff+position);
  position += 4;
  SESpaceCornerMeanCount = get_r4(buff+position);
  position += 4;
  SESpaceCornerStandardDeviation = get_r4(buff+position);
  position += 4;
  SWSpaceCornerMeanCount = get_r4(buff+position);
  position += 4;
  SWSpaceCornerStandardDeviation = get_r4(buff+position);
  position += 4;
  NESpaceCornerMeanCount = get_r4(buff+position);
  position += 4;
  NESpaceCornerStandardDeviation = get_r4(buff+position);
  position += 4;
  NWSpaceCornerMeanCount = get_r4(buff+position);
  position += 4;
  NWSpaceCornerStandardDeviation = get_r4(buff+position);
  position += 4;
  FourSpaceCornersMeanCount = get_r4(buff+position);
  position += 4;
  FourSpaceCornersStandardDeviation = get_r4(buff+position);
  position += 4;
  for (int i = 0; i < 256; i ++)
    FullImageIstogram[i] = get_ui4(buff+position+i*4);
  position += 256*4;
  for (int i = 0; i < 256; i ++)
    EarthDiskIstogram[i] = get_ui4(buff+position+i*4);
  position += 256*4;
  for (int i = 0; i < 256; i ++)
    ImageCentreSquareIstogram[i] = get_ui4(buff+position+i*4);
  position += 256*4;
  for (int i = 0; i < 128; i ++)
    SESpaceCornerIstogram[i] = get_ui4(buff+position+i*4);
  position += 128*4;
  for (int i = 0; i < 128; i ++)
    SWSpaceCornerIstogram[i] = get_ui4(buff+position+i*4);
  position += 128*4;
  for (int i = 0; i < 128; i ++)
    NESpaceCornerIstogram[i] = get_ui4(buff+position+i*4);
  position += 128*4;
  for (int i = 0; i < 128; i ++)
    NWSpaceCornerIstogram[i] = get_ui4(buff+position+i*4);
  position += 128*4;
  for (int i = 0; i < 3; i ++)
    FullImageEntropy[i] = get_r4(buff+position+i*4);
  position += 12;
  for (int i = 0; i < 3; i ++)
    EarthDiskEntropy[i] = get_r4(buff+position+i*4);
  position += 12;
  for (int i = 0; i < 3; i ++)
    ImageCentreSquareEntropy[i] = get_r4(buff+position+i*4);
  position += 12;
  for (int i = 0; i < 3; i ++)
    SESpaceCornerEntropy[i] = get_r4(buff+position+i*4);
  position += 12;
  for (int i = 0; i < 3; i ++)
    SWSpaceCornerEntropy[i] = get_r4(buff+position+i*4);
  position += 12;
  for (int i = 0; i < 3; i ++)
    NESpaceCornerEntropy[i] = get_r4(buff+position+i*4);
  position += 12;
  for (int i = 0; i < 3; i ++)
    NWSpaceCornerEntropy[i] = get_r4(buff+position+i*4);
  position += 12;
  for (int i = 0; i < 3; i ++)
    FourSpaceCornersEntropy[i] = get_r4(buff+position+i*4);
  position += 12;
  for (int i = 0; i < 128; i ++)
    ImageCentreSquarePSD_EW[i] = get_r4(buff+position+i*4);
  position += 128*4;
  for (int i = 0; i < 128; i ++)
    FullImagePSD_EW[i] = get_r4(buff+position+i*4);
  position += 128*4;
  for (int i = 0; i < 128; i ++)
    ImageCentreSquarePSD_NS[i] = get_r4(buff+position+i*4);
  position += 128*4;
  for (int i = 0; i < 128; i ++)
    FullImagePSD_NS[i] = get_r4(buff+position+i*4);
  position += 128*4;
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_L10RadQuality &q )
{
  os << "Full Image Min Count: " << q.FullImageMinimumCount << std::endl
     << "Full Image Max Count: " << q.FullImageMaximumCount << std::endl
     << "Earth Disk Min Count: " << q.EarthDiskMinimumCount << std::endl
     << "Earth Disk Max Count: " << q.EarthDiskMaximumCount << std::endl
     << "Moon Min Count      : " << q.MoonMinimumCount << std::endl
     << "Moon Max Count      : " << q.MoonMaximumCount << std::endl
     << "Full Mean Count     : " << q.FullImageMeanCount << std::endl
     << "Full Std. Deviation : " << q.FullImageStandardDeviation << std::endl
     << "Earth Mean Count    : " << q.EarthDiskMeanCount << std::endl
     << "Earth Std. Deviation: " << q.EarthDiskStandardDeviation << std::endl
     << "Moon Mean Count     : " << q.MoonMeanCount << std::endl
     << "Moon Std. Deviation : " << q.MoonStandardDeviation << std::endl
     << "Space Mean Count    : " << q.SpaceMeanCount << std::endl
     << "Space Std. Deviation: " << q.SpaceStandardDeviation << std::endl
     << "SE Corner Mean      : " << q.SESpaceCornerMeanCount << std::endl
     << "SE Corner Std. Dev. : " << q.SESpaceCornerStandardDeviation<< std::endl
     << "SW Corner Mean      : " << q.SWSpaceCornerMeanCount << std::endl
     << "SW Corner Std. Dev. : " << q.SWSpaceCornerStandardDeviation<< std::endl
     << "NE Corner Mean      : " << q.NESpaceCornerMeanCount << std::endl
     << "NE Corner Std. Dev. : " << q.NESpaceCornerStandardDeviation<< std::endl
     << "NW Corner Mean      : " << q.NWSpaceCornerMeanCount << std::endl
     << "NW Corner Std. Dev. : " << q.NWSpaceCornerStandardDeviation<< std::endl
     << "4 Space Corner Mean : " << q.FourSpaceCornersMeanCount << std::endl
     << "4 Space Corner Std. : " << q.FourSpaceCornersStandardDeviation
     << std::endl;
  os << "Full Image Istogram : " << std::endl;
  for (int i = 0; i < 256; i ++)
  {
    int count = q.FullImageIstogram[i] / maxistogram(q.FullImageIstogram, 256);
    os << std::setw(3) << std::setfill(' ') << i+1 << " ";
    for (int i = 0; i < count; i ++)
      os << "*";
    os << std::endl;
  }
  os << "Earth Disk Istogram : " << std::endl;
  for (int i = 0; i < 256; i ++)
  {
    int count = q.EarthDiskIstogram[i] / maxistogram(q.EarthDiskIstogram, 256);
    os << std::setw(3) << std::setfill(' ') << i+1 << " ";
    for (int i = 0; i < count; i ++)
      os << "*";
    os << std::endl;
  }
  os << "Image Centre Ist.   : " << std::endl;
  for (int i = 0; i < 256; i ++)
  {
    int count = q.ImageCentreSquareIstogram[i] /
                maxistogram(q.ImageCentreSquareIstogram, 256);
    os << std::setw(3) << std::setfill(' ') << i+1 << " ";
    for (int i = 0; i < count; i ++)
      os << "*";
    os << std::endl;
  }
  os << "SE Corner Istogram  : " << std::endl;
  for (int i = 0; i < 128; i ++)
  {
    int count = q.SESpaceCornerIstogram[i] /
                maxistogram(q.SESpaceCornerIstogram, 128);
    os << std::setw(3) << std::setfill(' ') << i+1 << " ";
    for (int i = 0; i < count; i ++)
      os << "*";
    os << std::endl;
  }
  os << "SW Corner Istogram  : " << std::endl;
  for (int i = 0; i < 128; i ++)
  {
    int count = q.SWSpaceCornerIstogram[i] /
                maxistogram(q.SWSpaceCornerIstogram, 128);
    os << std::setw(3) << std::setfill(' ') << i+1 << " ";
    for (int i = 0; i < count; i ++)
      os << "*";
    os << std::endl;
  }
  os << "NE Corner Istogram  : " << std::endl;
  for (int i = 0; i < 128; i ++)
  {
    int count = q.NESpaceCornerIstogram[i] / 
                maxistogram(q.NESpaceCornerIstogram, 128);
    os << std::setw(3) << std::setfill(' ') << i+1 << " ";
    for (int i = 0; i < count; i ++)
      os << "*";
    os << std::endl;
  }
  os << "NW Corner Istogram  : " << std::endl;
  for (int i = 0; i < 128; i ++)
  {
    int count = q.NWSpaceCornerIstogram[i] /
                maxistogram(q.NWSpaceCornerIstogram, 128);
    os << std::setw(3) << std::setfill(' ') << i+1 << " ";
    for (int i = 0; i < count; i ++)
      os << "*";
    os << std::endl;
  }
  os << "Full Image Entropy  : " << std::setw(12) << std::setfill(' ')
     << std::setw(12) << std::setfill(' ') << q.FullImageEntropy[0] << " "
     << std::setw(12) << std::setfill(' ') << q.FullImageEntropy[1] << " "
     << std::setw(12) << std::setfill(' ') << q.FullImageEntropy[2] <<
     std::endl;
  os << "Earth Disk Entropy  : " << std::setw(12) << std::setfill(' ')
     << std::setw(12) << std::setfill(' ') << q.EarthDiskEntropy[0] << " "
     << std::setw(12) << std::setfill(' ') << q.EarthDiskEntropy[1] << " "
     << std::setw(12) << std::setfill(' ') << q.EarthDiskEntropy[2]
     << std::endl;
  os << "Image Square Entropy: " << std::setw(12) << std::setfill(' ')
     << std::setw(12)<<std::setfill(' ') << q.ImageCentreSquareEntropy[0] << " "
     << std::setw(12)<<std::setfill(' ') << q.ImageCentreSquareEntropy[1] << " "
     << std::setw(12)<<std::setfill(' ') << q.ImageCentreSquareEntropy[2]
     << std::endl;
  os << "SE Spc Corner Entr. : " << std::setw(12) << std::setfill(' ')
     << std::setw(12)<<std::setfill(' ') << q.SESpaceCornerEntropy[0] << " "
     << std::setw(12)<<std::setfill(' ') << q.SESpaceCornerEntropy[1] << " "
     << std::setw(12)<<std::setfill(' ') << q.SESpaceCornerEntropy[2]
     << std::endl;
  os << "SW Spc Corner Entr. : " << std::setw(12) << std::setfill(' ')
     << std::setw(12)<<std::setfill(' ') << q.SWSpaceCornerEntropy[0] << " "
     << std::setw(12)<<std::setfill(' ') << q.SWSpaceCornerEntropy[1] << " "
     << std::setw(12)<<std::setfill(' ') << q.SWSpaceCornerEntropy[2]
     << std::endl;
  os << "NE Spc Corner Entr. : " << std::setw(12) << std::setfill(' ')
     << std::setw(12)<<std::setfill(' ') << q.NESpaceCornerEntropy[0] << " "
     << std::setw(12)<<std::setfill(' ') << q.NESpaceCornerEntropy[1] << " "
     << std::setw(12)<<std::setfill(' ') << q.NESpaceCornerEntropy[2]
     << std::endl;
  os << "NW Spc Corner Entr. : " << std::setw(12) << std::setfill(' ')
     << std::setw(12)<<std::setfill(' ') << q.NWSpaceCornerEntropy[0] << " "
     << std::setw(12)<<std::setfill(' ') << q.NWSpaceCornerEntropy[1] << " "
     << std::setw(12)<<std::setfill(' ') << q.NWSpaceCornerEntropy[2]
     << std::endl;
  os << "4 Spc Corners Entr. : " << std::setw(12) << std::setfill(' ')
     << std::setw(12)<<std::setfill(' ') << q.FourSpaceCornersEntropy[0] << " "
     << std::setw(12)<<std::setfill(' ') << q.FourSpaceCornersEntropy[1] << " "
     << std::setw(12)<<std::setfill(' ') << q.FourSpaceCornersEntropy[2]
     << std::endl;
  os << "Image Squ. PSD E/W  :" << std::endl;
  for (int i = 0; i < 128; i += 4)
    os << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_EW[i]<< " "
       << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_EW[i+1]<< " "
       << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_EW[i+2]<< " "
       << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_EW[i+3]
       << std::endl;
  os << "Full Image PSD E/W  :" << std::endl;
  for (int i = 0; i < 128; i += 4)
    os << std::setw(12) << std::setfill(' ') << q.FullImagePSD_EW[i] << " "
       << std::setw(12) << std::setfill(' ') << q.FullImagePSD_EW[i+1] << " "
       << std::setw(12) << std::setfill(' ') << q.FullImagePSD_EW[i+2] << " "
       << std::setw(12) << std::setfill(' ') << q.FullImagePSD_EW[i+3]
       << std::endl;
  os << "Image Squ. PSD N/S  :" << std::endl;
  for (int i = 0; i < 128; i += 4)
    os << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_NS[i]<< " "
       << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_NS[i+1]<< " "
       << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_NS[i+2]<< " "
       << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_NS[i+3]
       << std::endl;
  os << "Full Image PSD N/S  :" << std::endl;
  for (int i = 0; i < 128; i += 4)
    os << std::setw(12) << std::setfill(' ') << q.FullImagePSD_NS[i] << " "
       << std::setw(12) << std::setfill(' ') << q.FullImagePSD_NS[i+1] << " "
       << std::setw(12) << std::setfill(' ') << q.FullImagePSD_NS[i+2] << " "
       << std::setw(12) << std::setfill(' ') << q.FullImagePSD_NS[i+3]
       << std::endl;
  return os;
}

size_t MSG_L15RadQuality::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  FullImageMinimumCount = get_ui2(buff+position);
  position += 2;
  FullImageMaximumCount = get_ui2(buff+position);
  position += 2;
  EarthDiskMinimumCount = get_ui2(buff+position);
  position += 2;
  EarthDiskMaximumCount = get_ui2(buff+position);
  position += 2;
  FullImageMeanCount = get_r4(buff+position);
  position += 4;
  FullImageStandardDeviation = get_r4(buff+position);
  position += 4;
  EarthDiskMeanCount = get_r4(buff+position);
  position += 4;
  EarthDiskStandardDeviation = get_r4(buff+position);
  position += 4;
  SpaceMeanCount = get_r4(buff+position);
  position += 4;
  SpaceStandardDeviation = get_r4(buff+position);
  position += 4;
  for (int i = 0; i < 256; i ++)
    FullImageIstogram[i] = get_ui4(buff+position+i*4);
  position += 256*4;
  for (int i = 0; i < 256; i ++)
    EarthDiskIstogram[i] = get_ui4(buff+position+i*4);
  position += 256*4;
  for (int i = 0; i < 256; i ++)
    ImageCentreSquareIstogram[i] = get_ui4(buff+position+i*4);
  position += 256*4;
  for (int i = 0; i < 3; i ++)
    FullImageEntropy[i] = get_r4(buff+position+i*4);
  position += 12;
  for (int i = 0; i < 3; i ++)
    EarthDiskEntropy[i] = get_r4(buff+position+i*4);
  position += 12;
  for (int i = 0; i < 3; i ++)
    ImageCentreSquareEntropy[i] = get_r4(buff+position+i*4);
  position += 12;
  for (int i = 0; i < 128; i ++)
    ImageCentreSquarePSD_EW[i] = get_r4(buff+position+i*4);
  position += 128*4;
  for (int i = 0; i < 128; i ++)
    FullImagePSD_EW[i] = get_r4(buff+position+i*4);
  position += 128*4;
  for (int i = 0; i < 128; i ++)
    ImageCentreSquarePSD_NS[i] = get_r4(buff+position+i*4);
  position += 128*4;
  for (int i = 0; i < 128; i ++)
    FullImagePSD_NS[i] = get_r4(buff+position+i*4);
  position += 128*4;
  SESpaceCornerL15_RMS = get_r4(buff+position);
  position += 4;
  SESpaceCornerL15_Mean = get_r4(buff+position);
  position += 4;
  SWSpaceCornerL15_RMS = get_r4(buff+position);
  position += 4;
  SWSpaceCornerL15_Mean = get_r4(buff+position);
  position += 4;
  NESpaceCornerL15_RMS = get_r4(buff+position);
  position += 4;
  NESpaceCornerL15_Mean = get_r4(buff+position);
  position += 4;
  NWSpaceCornerL15_RMS = get_r4(buff+position);
  position += 4;
  NWSpaceCornerL15_Mean = get_r4(buff+position);
  position += 4;
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_L15RadQuality &q )
{
  os << "Full Image Min Count: " << q.FullImageMinimumCount << std::endl
     << "Full Image Max Count: " << q.FullImageMaximumCount << std::endl
     << "Earth Disk Min Count: " << q.EarthDiskMinimumCount << std::endl
     << "Earth Disk Max Count: " << q.EarthDiskMaximumCount << std::endl
     << "Full Mean Count     : " << q.FullImageMeanCount << std::endl
     << "Full Std. Deviation : " << q.FullImageStandardDeviation << std::endl
     << "Earth Mean Count    : " << q.EarthDiskMeanCount << std::endl
     << "Earth Std. Deviation: " << q.EarthDiskStandardDeviation << std::endl
     << "Space Mean Count    : " << q.SpaceMeanCount << std::endl
     << "Space Std. Deviation: " << q.SpaceStandardDeviation << std::endl;
  os << "Full Image Istogram : " << std::endl;
  for (int i = 0; i < 256; i ++)
  {
    int count = q.FullImageIstogram[i] / maxistogram(q.FullImageIstogram, 256);
    os << std::setw(3) << std::setfill(' ') << i+1 << " ";
    for (int i = 0; i < count; i ++)
      os << "*";
    os << std::endl;
  }
  os << "Earth Disk Istogram : " << std::endl;
  for (int i = 0; i < 256; i ++)
  {
    int count = q.EarthDiskIstogram[i] / maxistogram(q.EarthDiskIstogram, 256);
    os << std::setw(3) << std::setfill(' ') << i+1 << " ";
    for (int i = 0; i < count; i ++)
      os << "*";
    os << std::endl;
  }
  os << "Image Centre Ist.   : " << std::endl;
  for (int i = 0; i < 256; i ++)
  {
    int count = q.ImageCentreSquareIstogram[i] /
                maxistogram(q.ImageCentreSquareIstogram, 256);
    os << std::setw(3) << std::setfill(' ') << i+1 << " ";
    for (int i = 0; i < count; i ++)
      os << "*";
    os << std::endl;
  }
  os << "Full Image Entropy  : " << std::setw(12) << std::setfill(' ')
     << std::setw(12) << std::setfill(' ') << q.FullImageEntropy[0] << " "
     << std::setw(12) << std::setfill(' ') << q.FullImageEntropy[1] << " "
     << std::setw(12) << std::setfill(' ') << q.FullImageEntropy[2] <<
     std::endl;
  os << "Earth Disk Entropy  : " << std::setw(12) << std::setfill(' ')
     << std::setw(12) << std::setfill(' ') << q.EarthDiskEntropy[0] << " "
     << std::setw(12) << std::setfill(' ') << q.EarthDiskEntropy[1] << " "
     << std::setw(12) << std::setfill(' ') << q.EarthDiskEntropy[2]
     << std::endl;
  os << "Image Square Entropy: " << std::setw(12) << std::setfill(' ')
     << std::setw(12)<<std::setfill(' ') << q.ImageCentreSquareEntropy[0] << " "
     << std::setw(12)<<std::setfill(' ') << q.ImageCentreSquareEntropy[1] << " "
     << std::setw(12)<<std::setfill(' ') << q.ImageCentreSquareEntropy[2]
     << std::endl;
  os << "Image Squ. PSD E/W  :" << std::endl;
  for (int i = 0; i < 128; i += 4)
    os << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_EW[i]<< " "
       << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_EW[i+1]<< " "
       << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_EW[i+2]<< " "
       << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_EW[i+3]
       << std::endl;
  os << "Full Image PSD E/W  :" << std::endl;
  for (int i = 0; i < 128; i += 4)
    os << std::setw(12) << std::setfill(' ') << q.FullImagePSD_EW[i] << " "
       << std::setw(12) << std::setfill(' ') << q.FullImagePSD_EW[i+1] << " "
       << std::setw(12) << std::setfill(' ') << q.FullImagePSD_EW[i+2] << " "
       << std::setw(12) << std::setfill(' ') << q.FullImagePSD_EW[i+3]
       << std::endl;
  os << "Image Squ. PSD N/S  :" << std::endl;
  for (int i = 0; i < 128; i += 4)
    os << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_NS[i]<< " "
       << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_NS[i+1]<< " "
       << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_NS[i+2]<< " "
       << std::setw(12)<<std::setfill(' ')<<q.ImageCentreSquarePSD_NS[i+3]
       << std::endl;
  os << "Full Image PSD N/S  :" << std::endl;
  for (int i = 0; i < 128; i += 4)
    os << std::setw(12) << std::setfill(' ') << q.FullImagePSD_NS[i] << " "
       << std::setw(12) << std::setfill(' ') << q.FullImagePSD_NS[i+1] << " "
       << std::setw(12) << std::setfill(' ') << q.FullImagePSD_NS[i+2] << " "
       << std::setw(12) << std::setfill(' ') << q.FullImagePSD_NS[i+3]
       << std::endl;
  os << "SE Space Corner RMS : " << q.SESpaceCornerL15_RMS << std::endl
     << "SE Space Corner Mean: " << q.SESpaceCornerL15_Mean << std::endl
     << "SW Space Corner RMS : " << q.SWSpaceCornerL15_RMS << std::endl
     << "SW Space Corner Mean: " << q.SWSpaceCornerL15_Mean << std::endl
     << "NE Space Corner RMS : " << q.NESpaceCornerL15_RMS << std::endl
     << "NE Space Corner Mean: " << q.NESpaceCornerL15_Mean << std::endl
     << "NW Space Corner RMS : " << q.NWSpaceCornerL15_RMS << std::endl
     << "NW Space Corner Mean: " << q.NWSpaceCornerL15_Mean << std::endl;
  return os;
}

MSG_data_RadiometricQuality::MSG_data_RadiometricQuality( ) { }

MSG_data_RadiometricQuality::MSG_data_RadiometricQuality(
                    unsigned const char_1 *buff)
{
  this->read_from(buff);
}

MSG_data_RadiometricQuality::~MSG_data_RadiometricQuality( ) { }

size_t MSG_data_RadiometricQuality::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;
  for (int i = 0; i < 42; i ++)
    position += L10RadQuality[i].read_from(buff+position);
  for (int i = 0; i < 12; i ++)
    position += L15RadQuality[i].read_from(buff+position);
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_RadiometricQuality &t )
{
  os << "------------------------------------------------------" << std::endl
     << "-         MSG IMAGE RADIOMETRIC QUALITY RECORD       -" << std::endl
     << "------------------------------------------------------" << std::endl;
  for (int i = 0; i < 42; i ++)
    os << t.L10RadQuality[i];
  for (int i = 0; i < 12; i ++)
    os << t.L15RadQuality[i];
  return os;
}

uint_4 maxistogram(uint_4 *istogram, int_4 len)
{
  uint_4 maxval = 0;
  for (int i = 0; i < len; i ++)
    if (maxval < istogram[i]) maxval = istogram[i];
  if (maxval == 0) return 1;
  return maxval/76;
}
