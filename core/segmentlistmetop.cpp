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

    *x = 0;
    *y = 0;

    double lon_deg = lon_rad*180.0/PIE;
    double lat_deg = lat_rad*150.0/PIE;
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

void SegmentListMetop::GetCentralCoords(double *startcentrallon, double *startcentrallat, double *endcentrallon, double *endcentrallat)
{

    double slon, slat, elon, elat;
    double save_slon, save_slat, save_elon, save_elat;
    int startindex, endindex;

    save_slon = 65535.0;
    save_slat = 65535.0;
    save_elon = 65535.0;
    save_elat = 65535.0;

    bool first = true;

    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    while ( segsel != segsselected.end() )
    {
        SegmentMetop *segm = (SegmentMetop *)(*segsel);
        segm->GetCentralCoords(&slon, &slat, &elon, &elat, &startindex, &endindex);

        if(abs(slon) < 180.0 && abs(slat) < 90.0 && abs(elon) < 180.0 && abs(elat) < 90.0)
        {
            if(first == true)
            {
                first = false;
                save_slon = slon;
                save_slat = slat;
                save_elon = elon;
                save_elat = elat;
            }
            else
            {
                save_elon = elon;
                save_elat = elat;
            }

        }

        QApplication::processEvents();
        ++segsel;
    }

    *startcentrallon = save_slon;
    *startcentrallat = save_slat;
    *endcentrallon = save_elon;
    *endcentrallat = save_elat;

}

