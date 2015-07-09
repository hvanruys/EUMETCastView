#include "segmentlistgac.h"
#include "segmentimage.h"
#include "options.h"

extern Options opts;
extern SegmentImage *imageptrs;

SegmentListGAC::SegmentListGAC(QObject *parent) :
    SegmentList(parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;

    qDebug() << QString("in constructor SegmentListGAC");

}


void SegmentListGAC::GetFirstLastVisibleSegmentData( QString *satnamefirst, QString *segdatefirst, QString *segtimefirst,  QString *satnamelast, QString *segdatelast, QString *segtimelast)
{

    QString first_filename;
    QString last_filename;

    //AVHR_GAC_1B_N19_20130701041003Z_20130701041303Z_N_O_20130701054958Z

    if( segmentlist.count() > 0)
    {
        first_filename = segmentlist.at(indexfirstvisible)->fileInfo.fileName();
        last_filename = segmentlist.at(indexlastvisible)->fileInfo.fileName();
        if(first_filename.mid(0,8) == "AVHR_GAC")
        {
            *satnamefirst = first_filename.mid(12, 3);
            *segdatefirst = first_filename.mid(16, 4) + "-" + first_filename.mid(20, 2) + "-" + first_filename.mid(22, 2);
            *segtimefirst = first_filename.mid(24, 2) + ":" + first_filename.mid(26, 2) + ":" + first_filename.mid(28, 2);
        }
        if(last_filename.mid(0,8) == "AVHR_GAC")
        {
            *satnamelast = last_filename.mid(12, 3);
            *segdatelast = last_filename.mid(16, 4) + "-" + last_filename.mid(20, 2) + "-" + last_filename.mid(22, 2);
            *segtimelast = last_filename.mid(24, 2) + ":" + last_filename.mid(26, 2) + ":" + last_filename.mid(28, 2);
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


