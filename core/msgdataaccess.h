#ifndef MSGDATAACCESS_H
#define MSGDATAACCESS_H

#include "msgdataaccess.h"
#include <string>
#include <vector>
#include <deque>
#include <MSG_data_image.h>
#include <QStringList>
#include <QFile>

struct MSG_header;
struct MSG_data;
class MsgFileAccess;

/**
 * Higher level data access for xRIT files
 */
class MsgDataAccess
{
protected:
        void scanSegment(const MSG_header& header);

public:
        /// Number of pixels in every segment
        int npixperseg;

        /// Number of lines in every segment
        int seglines;

        /// True if the image needs to be swapped horizontally
        bool swapX;

        /// True if the image needs to be swapped vertically
        bool swapY;

        /// True if the image is an HRV image divided in two parts
        bool hrv;

        /// Pathnames of the segment files, indexed with their index
        QStringList segnames;

        struct scache
        {
                MSG_data* segment;
                int segno;
        };
        /// Segment cache
        mutable std::deque<scache> segcache;

        /// Length of a scanline
        int columns;

        /// Number of scanlines
        int lines;

        /// HRV reference grid (meaningless if not HRV)
        int LowerEastColumnActual;
        int LowerSouthLineActual;
        int LowerWestColumnActual;
        int LowerNorthLineActual;
        int UpperEastColumnActual;
        int UpperSouthLineActual;
        int UpperWestColumnActual;
        int UpperNorthLineActual;

        /// non-HRV reference grid (meaningless if HRV)
        int SouthLineActual;
        int WestColumnActual;

        MsgDataAccess();
        ~MsgDataAccess();

        /**
         * Scan the given segments, filling in all the various DataAccess
         * fields.
         */
        // ********
        //void scan(MsgFileAccess fa, MSG_data& pro, MSG_data& epi, MSG_header& header);

        /**
         * Read a xRIT file (prologue, epilogue or segment)
         */
        void read_file(const QString file, MSG_header& head, MSG_data& data) const;

        /**
         * Read only the xRIT header of a file
         */
        void read_file(const QString file, MSG_header& head) const;

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
        //*******************
        //int line_start(int line) const;

        /**
         * Read a scanline
         *
         * The scanline will be appropriately swapped so that it goes from west
         * to east
         *
         * \a buf must be at least 'columns' elements
         */
        //***********************
        //void line_read(int line, MSG_SAMPLE* buf) const;

        /**
         * Return the MSG_data corresponding to the segment with the given index.
         *
         * The pointer could be invalidated by another call to segment()
         */
        //*******************************
        //MSG_data* segment(int idx) const;
};



#endif

