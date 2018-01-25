//-----------------------------------------------------------------------------
//
//  File        : MSG_data_ImageDescription.h
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

#ifndef __MSG_DATA_IMAGEDESCRIPTION_H__
#define __MSG_DATA_IMAGEDESCRIPTION_H__

#include <iostream>
#include <string>
#include "MSG_machine.h"

#define MSG_IMAGE_DESCRIPTION_LEN    101

typedef enum {
  MSG_PROJECTION_TYPE_UNDEFINED     = 0,
  MSG_PROJECTION_TYPE_GEOSTATIONARY = 1
} t_enum_MSG_projection_type;

std::string MSG_projection_type(t_enum_MSG_projection_type type);

typedef enum {
  MSG_PLANNED_CHAN_PROCESSING_UNDEFINED     = 0,
  MSG_PLANNED_CHAN_PROCESSING_SPECTRAL      = 1,
  MSG_PLANNED_CHAN_PROCESSING_EFFECTIVE     = 2,
} t_enum_MSG_planned_chan_processing;

std::string MSG_planned_chan_processing(t_enum_MSG_planned_chan_processing type);

typedef enum {
  MSG_ORIGIN_CORNER_NW = 0,
  MSG_ORIGIN_CORNER_SW = 1,
  MSG_ORIGIN_CORNER_SE = 2,
  MSG_ORIGIN_CORNER_NE = 3
} t_enum_MSG_grid_origin;

std::string MSG_grid_origin(t_enum_MSG_grid_origin org);

typedef enum {
  MSG_DIRECTION_LINE_NS = 0,
  MSG_DIRECTION_LINE_SN = 1
} t_enum_MSG_direction_line;

std::string MSG_direction_line(t_enum_MSG_direction_line dir);

typedef enum {
  MSG_DIRECTION_COLUMN_EW = 0,
  MSG_DIRECTION_COLUMN_WE = 1
} t_enum_MSG_direction_column;

std::string MSG_direction_column(t_enum_MSG_direction_column dir);

class MSG_grid {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os, MSG_grid &g );
//  private:
    int_4                      NumberofLines;
    int_4                      NumberofColumns;
    real_4                     LineDirGridStep;
    real_4                     ColumnDirGridStep;
    t_enum_MSG_grid_origin     GridOrigin;
};

class MSG_coverage_IR {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os, MSG_coverage_IR &c );
  private:
    int_4 SouthernLinePlanned;
    int_4 NorthernLinePlanned;
    int_4 EasternColumnPlanned;
    int_4 WesternColumnPlanned;
};

class MSG_coverage_HRV {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os, MSG_coverage_HRV &c );
  private:
    int_4 LowerSouthLinePlanned;
    int_4 LowerNorthLinePlanned;
    int_4 LowerEastColumnPlanned;
    int_4 LowerWestColumnPlanned;
    int_4 UpperSouthLinePlanned;
    int_4 UpperNorthLinePlanned;
    int_4 UpperEastColumnPlanned;
    int_4 UpperWestColumnPlanned;
};

class MSG_ProjectionDescription {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_ProjectionDescription &d );
    t_enum_MSG_projection_type  TypeOfProjection;
    real_4                      LongitudeOfSSP;
};

class MSG_Level1_5ImageProduction {
  public:
    size_t read_from( unsigned const char_1 *buff );
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_Level1_5ImageProduction &p );
    t_enum_MSG_direction_line   ImageProcDirection;
    t_enum_MSG_direction_column PixelGenDirection;
    t_enum_MSG_planned_chan_processing   PlannedChanProcessing[12];
};

class MSG_data_ImageDescription {
  public:
    MSG_data_ImageDescription( );
    MSG_data_ImageDescription( unsigned const char_1 *buff );
    ~MSG_data_ImageDescription( );

    size_t read_from( unsigned const char_1 *buff );

    // Overloaded << operator
    friend std::ostream& operator<< ( std::ostream& os,
                                      MSG_data_ImageDescription &g );

    MSG_ProjectionDescription   ProjectionDescription;
    MSG_grid                    ReferenceGridVIS_IR;
    MSG_grid                    ReferenceGridHRV;
    MSG_coverage_IR             PlannedCoverageVIS_IR;
    MSG_coverage_HRV            PlannedCoverageHRV;
    MSG_Level1_5ImageProduction Level1_5ImageProduction;
};

#endif
