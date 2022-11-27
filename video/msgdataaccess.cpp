#include "msgdataaccess.h"
#include "msgfileaccess.h"
#include <MSG_HRIT.h>
#include <stdexcept>
#include <QDebug>

using namespace std;

MsgDataAccess::MsgDataAccess() : npixperseg(0)
{
}

MsgDataAccess::~MsgDataAccess()
{
    //qDebug() << "MsgDataAccess::~MsgDataAccess() segcach count = " << segcache.size();
    for (std::deque<scache>::iterator i = segcache.begin();
                    i != segcache.end(); ++i)
            delete i->segment;
}

void MsgDataAccess::read_file(QString file, MSG_header& head) const
{

        std::ifstream hrit(file.toStdString().c_str(), (std::ios::binary | std::ios::in));
        if (hrit.fail()) throw std::runtime_error(file.toStdString() + ": cannot open");
        head.read_from(hrit);
        hrit.close();
}


void MsgDataAccess::read_file(const QString file, MSG_header& head, MSG_data& data) const
{
        std::ifstream hrit(file.toStdString().c_str(), (std::ios::binary | std::ios::in));
        if (hrit.fail())
                throw std::runtime_error(file.toStdString() + ": cannot open");
        head.read_from(hrit);
        if (head.segment_id && head.segment_id->data_field_format == MSG_NO_FORMAT)
                throw std::runtime_error(file.toStdString() + ": product dumped in binary format");
        data.read_from(hrit, head);
        hrit.close();
}

void MsgDataAccess::scanSegment(const MSG_header& header)
{
        // Decoding information
        int totalsegs = header.segment_id->planned_end_segment_sequence_number;
        seglines = header.image_structure->number_of_lines;
#if 0
        cerr << "NCOL " << header.image_structure->number_of_columns << endl;
        cerr << "NLIN " << header.image_structure->number_of_lines << endl;
#endif
        columns = header.image_structure->number_of_columns;
        lines = seglines * totalsegs;
        npixperseg = columns * seglines;
        hrv = header.segment_id->spectral_channel_id == MSG_SEVIRI_1_5_HRV;

        // See if the image needs to be rotated
        swapX = header.image_navigation->column_scaling_factor < 0;
        swapY = header.image_navigation->line_scaling_factor < 0;
}


bool MsgDataAccess::scan(const MsgFileAccess fa, MSG_data& pro, MSG_data& epi, MSG_header& header)
{
        // Read prologue
        MSG_header PRO_head;
        qDebug() << "Reading prologue";
        read_file(fa.prologueFile(), PRO_head, pro);

        // Read epilogue
        MSG_header EPI_head;
        qDebug() << "Reading epilogue";
        read_file(fa.epilogueFile(), EPI_head, epi);

        // Sort the segment names by their index
        QStringList segfiles = fa.segmentFiles();
        for (QStringList::const_iterator i = segfiles.begin(); i != segfiles.end(); ++i)
        {
                qDebug() << "Scanning segment " + *i;
                read_file(QString(*i), header);
                if (header.segment_id->data_field_format == MSG_NO_FORMAT)
                {
                        qDebug() << *i + ": product dumped in binary format";
                        return false;
                }

                int idx = header.segment_id->sequence_number-1;
                if (idx < 0) continue;
                if ((size_t)idx >= segnames.size())
                        segnames.resize(idx + 1);
                segnames[idx] = *i;
        }

        if (segnames.empty()) throw std::runtime_error("no segments found");

        // Read common info just once from a random segment
        scanSegment(header);

        if (hrv)
        {
                MSG_ActualL15CoverageHRV& cov = epi.epilogue->product_stats.ActualL15CoverageHRV;
                LowerEastColumnActual = cov.LowerEastColumnActual;
                LowerNorthLineActual = cov.LowerNorthLineActual;
                LowerWestColumnActual = cov.LowerWestColumnActual;
                LowerSouthLineActual = cov.LowerSouthLineActual;
                UpperEastColumnActual = cov.UpperEastColumnActual;
                UpperSouthLineActual = cov.UpperSouthLineActual;
                UpperWestColumnActual = cov.UpperWestColumnActual;
                UpperNorthLineActual = cov.UpperNorthLineActual;
        } else {

                //WestColumnActual = 1856 - header.image_navigation->column_offset + 1;
                //SouthLineActual = 1856 - header.image_navigation->line_offset + 1;

                WestColumnActual = 1;
                SouthLineActual = 1;
        }

        return true;
}

bool MsgDataAccess::scan(const MsgFileAccess fa, MSG_data& pro, MSG_data& epi)
{
        // Read prologue
        MSG_header PRO_head;
        qDebug() << "Reading prologue";
        read_file(fa.prologueFile(), PRO_head, pro);

        // Read epilogue
        MSG_header EPI_head;
        qDebug() << "Reading epilogue";
        read_file(fa.epilogueFile(), EPI_head, epi);

        return true;
}

bool MsgDataAccess::scan(const MsgFileAccess fa, MSG_header& header)
{
        // Sort the segment names by their index
        QStringList segfiles = fa.segmentFiles();
        for (QStringList::const_iterator i = segfiles.begin(); i != segfiles.end(); ++i)
        {
                qDebug() << "Scanning segment " + *i;
                read_file(QString(*i), header);
                if (header.segment_id->data_field_format == MSG_NO_FORMAT)
                {
                        qDebug() << *i + ": product dumped in binary format";
                        return false;
                }

                int idx = header.segment_id->sequence_number-1;
                if (idx < 0) continue;
                if ((size_t)idx >= segnames.size())
                        segnames.resize(idx + 1);
                segnames[idx] = *i;
        }

        if (segnames.empty()) throw std::runtime_error("no segments found");

        // Read common info just once from a random segment
        scanSegment(header);

        return true;
}

MSG_data* MsgDataAccess::segment(size_t idx) const
{
        // Check to see if the segment we need is the current one
        if (segcache.empty() || segcache.begin()->segno != idx)
        {
                // If not, check to see if we can find the segment in the cache
                std::deque<scache>::iterator i = segcache.begin();
                for ( ; i != segcache.end(); ++i)
                        if (i->segno == idx)
                                break;
                if (i == segcache.end())
                {
                        // Not in cache: we need to load it

                        // Do not load missing segments
                        if (idx >= segnames.size()) return 0;
                        if (segnames[idx].isEmpty()) return 0;

                        // Remove the last recently used if the cache is full
                        if (segcache.size() == 2)
                        {
                                delete segcache.rbegin()->segment;
                                segcache.pop_back();
                        }

                        // Load the segment
                        // ProgressTask p("Reading segment " + segnames[idx]);
                        MSG_header header;
                        scache new_scache;
                        new_scache.segment = new MSG_data;
                        new_scache.segno = idx;
                        read_file(segnames[idx], header, *new_scache.segment);

                        // Put it in the front
                        segcache.push_front(new_scache);
                } else {
                        // The segment is in the cache: bring it to the front
                        scache tmp = *i;
                        segcache.erase(i);
                        segcache.push_front(tmp);
                }
        }
        return segcache.begin()->segment;
}

size_t MsgDataAccess::line_start(size_t line) const
{
        if (!hrv) return WestColumnActual - 1;
        if (line >= UpperNorthLineActual) return 0;

        // Bring line in the domain of the HRV reference grid
        line = UpperNorthLineActual - line;

        if (line < LowerSouthLineActual) return 0;
        if (line <= LowerNorthLineActual) return 11136 - LowerWestColumnActual;
        if (line < UpperSouthLineActual) return 0;
        if (line <= UpperNorthLineActual) return 11136 - UpperWestColumnActual;
        return 0;
}

void MsgDataAccess::line_read(size_t line, MSG_SAMPLE* buf) const
{
        size_t segnum;
        size_t segline;

        if (hrv)
        {
                line = 11136 - line;
                segnum = (line - LowerSouthLineActual) / seglines;
                segline = (line - LowerSouthLineActual) % seglines;
        }
        else
        {
                line = 3712 - line;
                segnum = (line - SouthLineActual) / seglines;
                segline = (line - SouthLineActual) % seglines;
        }

        MSG_data* d = segment(segnum);
        if (d == 0)
        {
                //bzero(buf, columns * sizeof(MSG_SAMPLE));
                memset(buf, '0', columns * sizeof(MSG_SAMPLE));
                return;
        }

        if (swapX)
        {
                for (size_t i = 0; i < columns; ++i)
                        buf[columns - i - 1] = d->image->data[segline * columns + i];
        } else
                memcpy(buf, d->image->data + segline * columns, columns * sizeof(MSG_SAMPLE));
}

