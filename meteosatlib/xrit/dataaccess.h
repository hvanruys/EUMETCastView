#ifndef MSAT_XRIT_DATAACCESS_H
#define MSAT_XRIT_DATAACCESS_H

/*
 * xrit/dataaccess - Higher level data access for xRIT files
 *
 * Copyright (C) 2007--2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Enrico Zini <enrico@enricozini.org>
 */

#include <string>
#include <vector>
#include <deque>
#include <MSG_data_image.h>

struct MSG_header;
struct MSG_data;

//namespace msat {
namespace xrit {

struct FileAccess;

/**
 * Higher level data access for xRIT files
 */
class DataAccess
{
protected:
        void scanSegment(const MSG_header& header);

public:
        /// Number of pixels in every segment
        size_t npixperseg;

        /// Number of lines in every segment
        size_t seglines;

        /// True if the image needs to be swapped horizontally
        bool swapX;

        /// True if the image needs to be swapped vertically
        bool swapY;

        /// True if the image is an HRV image divided in two parts
        bool hrv;

        /// Pathnames of the segment files, indexed with their index
        std::vector<std::string> segnames;

        struct scache
        {
                MSG_data* segment;
                size_t segno;
        };
        /// Segment cache
        mutable std::deque<scache> segcache;

        /// Length of a scanline
        size_t columns;

        /// Number of scanlines
        size_t lines;

        /// HRV reference grid (meaningless if not HRV)
        size_t LowerEastColumnActual;
        size_t LowerSouthLineActual;
        size_t LowerWestColumnActual;
        size_t LowerNorthLineActual;
        size_t UpperEastColumnActual;
        size_t UpperSouthLineActual;
        size_t UpperWestColumnActual;
        size_t UpperNorthLineActual;

        /// non-HRV reference grid (meaningless if HRV)
        size_t SouthLineActual;
        size_t WestColumnActual;

        DataAccess();
        ~DataAccess();

        /**
         * Scan the given segments, filling in all the various DataAccess
         * fields.
         */
        void scan(const FileAccess& fa, MSG_data& pro, MSG_data& epi, MSG_header& header);

        /**
         * Read a xRIT file (prologue, epilogue or segment)
         */
        void read_file(const std::string& file, MSG_header& head, MSG_data& data) const;

        /**
         * Read only the xRIT header of a file
         */
        void read_file(const std::string& file, MSG_header& head) const;

        /**
         * Return the X offset at which the given line starts.
         *
         * Line is intended as the scanline of the full world image starting
         * from the north. Line 0 is the northernmost scanline.
         *
         * This is always 0 for non-HRV. For HRV, it is the offset a line needs
         * to be shifted right to geographically align it in the virtual
         * fullsize image.
         */
        size_t line_start(size_t line) const;

        /**
         * Read a scanline
         *
         * The scanline will be appropriately swapped so that it goes from west
         * to east
         *
         * \a buf must be at least 'columns' elements
         */
        void line_read(size_t line, MSG_SAMPLE* buf) const;

        /**
         * Return the MSG_data corresponding to the segment with the given index.
         *
         * The pointer could be invalidated by another call to segment()
         */
        MSG_data* segment(size_t idx) const;
};

}
//}

#endif
