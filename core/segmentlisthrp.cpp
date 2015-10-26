#include "segmentlisthrp.h"
#include "segmentimage.h"
#include "options.h"

extern Options opts;
extern SegmentImage *imageptrs;

SegmentListHRP::SegmentListHRP(QObject *parent) :
    SegmentList(parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    segtype = eSegmentType::SEG_HRP;

    qDebug() << QString("in constructor SegmentListHRP");

}

void SegmentListHRP::GetFirstLastVisibleSegmentData( QString *satnamefirst, QString *segdatefirst, QString *segtimefirst,  QString *satnamelast, QString *segdatelast, QString *segtimelast)
{

    QString first_filename;
    QString last_filename;

    //AVHR_HRP_00_M02_20130701060200Z_20130701060300Z_N_O_20130701061314Z

    if( segmentlist.count() > 0)
    {
        //segmentlist.at(indexfirstvisible)->
        first_filename = segmentlist.at(indexfirstvisible)->fileInfo.fileName();
        last_filename = segmentlist.at(indexlastvisible)->fileInfo.fileName();
        if(first_filename.mid(0,8) == "AVHR_HRP")
        {
            *satnamefirst = first_filename.mid(12, 3);
            *segdatefirst = QString("%1-%2-%3").arg(first_filename.mid(16, 4)).arg(first_filename.mid(20, 2)).arg(first_filename.mid(22, 2));
            *segtimefirst = QString("%1:%2:%3").arg(first_filename.mid(24, 2)).arg(first_filename.mid(26, 2)).arg(first_filename.mid(28, 2));
        }
        if(last_filename.mid(0,8) == "AVHR_HRP")
        {
            *satnamelast = last_filename.mid(12, 3);
            *segdatelast = QString("%1-%2-%3").arg(last_filename.mid(16, 4)).arg(last_filename.mid(20, 2)).arg(last_filename.mid(22, 2));
            *segtimelast = QString("%1:%2:%3").arg(last_filename.mid(24, 2)).arg(last_filename.mid(26, 2)).arg(last_filename.mid(28, 2));
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

