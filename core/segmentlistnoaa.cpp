#include "sgp4sdp4.h"
#include "segmentlistnoaa.h"
#include "segmentnoaa.h"
#include "segmentimage.h"
#include "options.h"

#include <QDir>
#include <QDebug>
#include <QPainter>
#include <math.h>

extern Options opts;
extern SegmentImage *imageptrs;

SegmentListNoaa::SegmentListNoaa(SatelliteList *satl, QObject *parent) :
    SegmentList(parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListNoaa");

    satlist = satl;
}

void SegmentListNoaa::GetFirstLastVisibleSegmentData( QString *satnamefirst, QString *segdatefirst, QString *segtimefirst,  QString *satnamelast, QString *segdatelast, QString *segtimelast)
{

    QString first_filename;
    QString last_filename;

    //avhrr_20131111_011500_noaa19.hrp.bz2
    if( segmentlist.count() > 0)
    {
        //segmentlist.at(indexfirstvisible)->
        first_filename = segmentlist.at(indexfirstvisible)->fileInfo.fileName();
        last_filename = segmentlist.at(indexlastvisible)->fileInfo.fileName();
        if(first_filename.mid(0,6) == "avhrr_")
        {
            *satnamefirst = first_filename.mid(22, 6);
            *segdatefirst = QString("%1-%2-%3").arg(first_filename.mid(6,4)).arg(first_filename.mid(10,2)).arg(first_filename.mid(12,2));
            *segtimefirst = QString("%1:%2:%3").arg(first_filename.mid(15, 2)).arg(first_filename.mid(17, 2)).arg(first_filename.mid(19, 2));
        }
        if(last_filename.mid(0,6) == "avhrr_")
        {
            *satnamelast = last_filename.mid(22, 6);
            *segdatelast = QString("%1-%2-%3").arg(last_filename.mid(6,4)).arg(last_filename.mid(10,2)).arg(last_filename.mid(12,2));
            *segtimelast = QString("%1:%2:%3").arg(last_filename.mid(15, 2)).arg(last_filename.mid(17, 2)).arg(last_filename.mid(19, 2));
        }
    }
    else
    {
        *satnamefirst = QString("");
        *segdatefirst = QString("");
        *segtimefirst = QString("");
        *satnamelast = QString("");
        *segdatelast = QString("");
        *segtimelast = QString("");
    }
}

