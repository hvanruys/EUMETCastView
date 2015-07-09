//-----------------------------------------------------------------------------
//
//  File        : MSG_data_GeometricQuality.cpp
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
#include "MSG_data_GeometricQuality.h"

std::string MSG_quality_info_validity(t_enum_MSG_quality_info_validity qiv)
{
  std::string v;
  switch (qiv)
  {
    case MSG_QUALITY_INFO_VALIDITY_NOT_DERIVED:
      v = "Not Derived";
      break;
    case MSG_QUALITY_INFO_VALIDITY_DERIVED_AND_VALID:
      v = "Derived and Valid";
      break;
    case MSG_QUALITY_INFO_VALIDITY_DERIVED_AND_INVALID:
      v = "Derived and Invalid";
      break;
    case MSG_QUALITY_INFO_VALIDITY_ESTIMATED:
      v = "Estimated";
      break;
    default:
      v = "Unknown";
      break;
  }
  return v;
}

size_t MSG_Accuracy::read_from( unsigned const char_1 *buff )
{
  QualityInfoValidity = (t_enum_MSG_quality_info_validity) *(buff);
  EastWestAccuracyRMS = get_r4(buff+1);
  NorthSouthAccuracyRMS = get_r4(buff+5);
  MagnitudeRMS = get_r4(buff+9);
  EastWestUncertaintyRMS = get_r4(buff+13);
  NorthSouthUncertaintyRMS = get_r4(buff+17);
  MagnitudeUncertaintyRMS = get_r4(buff+21);
  EastWestMaxDeviation = get_r4(buff+25);
  NorthSouthMaxDeviation = get_r4(buff+29);
  MagnitudeMaxDeviation = get_r4(buff+33);
  EastWestUncertaintyMax = get_r4(buff+37);
  NorthSouthUncertaintyMax = get_r4(buff+41);
  MagnitudeUncertaintyMax = get_r4(buff+45);
  return 49;
}

std::ostream& operator<< ( std::ostream& os, MSG_Accuracy &a )
{
  os << "Quality Info Valid. : "
     << MSG_quality_info_validity(a.QualityInfoValidity) << std::endl;
  if (a.QualityInfoValidity != MSG_QUALITY_INFO_VALIDITY_NOT_DERIVED)
    os << "E/W Accuracy RMS    : " << a.EastWestAccuracyRMS << std::endl
       << "N/S Accuracy RMS    : " << a.NorthSouthAccuracyRMS << std::endl
       << "Magnitude RMS       : " << a.MagnitudeRMS << std::endl
       << "E/W Uncert. RMS     : " << a.EastWestUncertaintyRMS << std::endl
       << "N/S Uncert. RMS     : " << a.NorthSouthUncertaintyRMS << std::endl
       << "Magnitude Unc. RMS  : " << a.MagnitudeUncertaintyRMS << std::endl
       << "E/W Max Deviation   : " << a.EastWestMaxDeviation << std::endl
       << "N/S Max Deviation   : " << a.NorthSouthMaxDeviation << std::endl
       << "Magnitude Deviation : " << a.MagnitudeMaxDeviation << std::endl
       << "E/W Uncert. Max     : " << a.EastWestUncertaintyMax << std::endl
       << "N/S Uncert. Max     : " << a.NorthSouthUncertaintyMax << std::endl
       << "Magnitude Unc. Max  : " << a.MagnitudeUncertaintyMax << std::endl;
  return os;
}

size_t MSG_Residuals::read_from( unsigned const char_1 *buff )
{
  QualityInfoValidity = (t_enum_MSG_quality_info_validity) *(buff);
  EastWestResiduals = get_r4(buff+1);
  NorthSouthResiduals = get_r4(buff+5);
  EastWestUncertainty = get_r4(buff+9);
  NorthSouthUncertainty = get_r4(buff+13);
  EastWestRMS = get_r4(buff+17);
  NorthSouthRMS = get_r4(buff+21);
  EastWestMagnitude = get_r4(buff+25);
  NorthSouthMagnitude = get_r4(buff+29);
  EastWestMagnitudeUncertainty = get_r4(buff+33);
  NorthSouthMagnitudeUncertainty = get_r4(buff+37);
  return 41;
}

std::ostream& operator<< ( std::ostream& os, MSG_Residuals &a )
{
  os << "Quality Info Valid. : "
     << MSG_quality_info_validity(a.QualityInfoValidity) << std::endl;
  if (a.QualityInfoValidity != MSG_QUALITY_INFO_VALIDITY_NOT_DERIVED)
    os << "E/W Residuals       : " << a.EastWestResiduals << std::endl
       << "N/S Residuals       : " << a.NorthSouthResiduals << std::endl
       << "E/W Uncertainty     : " << a.EastWestUncertainty << std::endl
       << "N/S Uncertainty     : " << a.NorthSouthUncertainty << std::endl
       << "E/W RMS             : " << a.EastWestRMS << std::endl
       << "N/S RMS             : " << a.NorthSouthRMS << std::endl
       << "E/W Magnitude       : " << a.EastWestMagnitude << std::endl
       << "N/S Magnitude       : " << a.NorthSouthMagnitude << std::endl
       << "E/W Magnitude Unce. : " << a.EastWestMagnitudeUncertainty
       << std::endl
       << "N/S Magnitude Unce. : " << a.NorthSouthMagnitudeUncertainty
       << std::endl;
  return os;
}

size_t MSG_GeometricQualityStatus::read_from( unsigned const char_1 *buff )
{
  QualityNominal = get_ui1(buff);
  NominalAbsolute = get_ui1(buff+1);
  NominalRelativeToPreviousImage = get_ui1(buff+2);
  NominalForREL500 = get_ui1(buff+3);
  NominalForREL16 = get_ui1(buff+4);
  NominalForResMisreg = get_ui1(buff+5);
  return 6;
}

std::ostream& operator<< ( std::ostream& os, MSG_GeometricQualityStatus &s )
{
  os << "Quality Nominal     : " << (uint_2) s.QualityNominal << std::endl
     << "Nominal Absolute    : " << (uint_2) s.NominalAbsolute << std::endl
     << "Nominal Rel. Prev.  : " << (uint_2) s.NominalRelativeToPreviousImage
     << std::endl
     << "Nominal Rel. 500    : " << (uint_2) s.NominalForREL500 << std::endl
     << "Nominal Rel. 16     : " << (uint_2) s.NominalForREL16 << std::endl
     << "Nominal Rel. Misreg : " << (uint_2) s.NominalForResMisreg << std::endl;
  return os;
}

MSG_data_GeometricQuality::MSG_data_GeometricQuality( ) { }

MSG_data_GeometricQuality::MSG_data_GeometricQuality(
                                unsigned const char_1 *buff)
{
  this->read_from(buff);
}

MSG_data_GeometricQuality::~MSG_data_GeometricQuality( ) { }

size_t MSG_data_GeometricQuality::read_from( unsigned const char_1 *buff )
{
  size_t position = 0;

  for (int i = 0; i < 12; i ++)
    position += AbsoluteAccuracy[i].read_from(buff+position);
  for (int i = 0; i < 12; i ++)
    position += RelativeAccuracy[i].read_from(buff+position);
  for (int i = 0; i < 12; i ++)
    position += N500PixelsRelativeAccuracy[i].read_from(buff+position);
  for (int i = 0; i < 12; i ++)
    position += N16PixelsRelativeAccuracy[i].read_from(buff+position);
  for (int i = 0; i < 12; i ++)
    position += MisregistrationResiduals[i].read_from(buff+position);
  for (int i = 0; i < 12; i ++)
    position += GeometricQualityStatus[i].read_from(buff+position);
  return position;
}

std::ostream& operator<< ( std::ostream& os, MSG_data_GeometricQuality &t )
{
  os << "------------------------------------------------------" << std::endl
     << "-         MSG IMAGE GEOMETRIC QUALITY RESULTS        -" << std::endl
     << "------------------------------------------------------" << std::endl;
  for (int i = 0; i < 12; i ++)
    os << "Channel " << std::setw(2) << std::setfill('0') << i+1 << std::endl
       << "ABSOLUTE ACCURACY" << std::endl
       << t.AbsoluteAccuracy[i]
       << "RELATIVE ACCURACY" << std::endl
       << t.RelativeAccuracy[i]
       << "N 500 PIXELS ACCURACY" << std::endl
       << t.N500PixelsRelativeAccuracy[i]
       << "N 16 PIXELS ACCURACY" << std::endl
       << t.N16PixelsRelativeAccuracy[i]
       << "MISREGISTRATION RESIDUALS" << std::endl
       << t.MisregistrationResiduals[i]
       << "GEOMETRIC QUALITY" << std::endl
       << t.GeometricQualityStatus[i];
  return os;
}
