#include "segmentlistmetop.h"
#include "options.h"
#include "segmentimage.h"
#include "globals.h"

extern Options opts;
extern SegmentImage *imageptrs;


SegmentListMetop::SegmentListMetop(QObject *parent) :
    SegmentList(parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    seglisttype = eSegmentType::SEG_METOP;

    qDebug() << QString("in constructor SegmentListMetop");


}


bool  SegmentListMetop::GetGeoLocation(double lon_rad, double lat_rad, int *x, int *y)
{
    //qDebug() << QString("SegmentListMetop::GetGeoLocation segsselected count = %1").arg(segsselected.count());

    float flon;
    float flat;

    *x = 0;
    *y = 0;

    double lon_deg = lon_rad*180.0/PI;
    double lat_deg = lat_rad*150.0/PI;
    if( segsselected.count() > 0)
    {
       for(int i = 0; i < segsselected.count(); i++)
        {
            for(int heightinsegment = 0; heightinsegment < segsselected.at(i)->NbrOfLines; heightinsegment++)
            {
                for(int col = 0; col < 102; col=col+101)
                {
                    //qDebug() << QString(" lat 1 = %1 lat 2 = %2").arg(segsselected.at(i)->earthloc_lat[heightinsegment*103 + col]).arg(segsselected.at(i)->earthloc_lat[heightinsegment*103 + col + 1]);

                    if(segsselected.at(i)->earthloc_lat[heightinsegment*103 + col] > lat_deg && segsselected.at(i)->earthloc_lat[heightinsegment*103 + col + 1] <= lat_deg)
                    {
                        *x = col;
                        *y = heightinsegment;
                        return true;
                    }

                }

            }
        }

    }
    return false;
}


