#ifndef SEGMENTGAC_H
#define SEGMENTGAC_H

#include <QObject>
#include <QFile>
#include <QImage>

#include "segment.h"

class SegmentGAC : public Segment
{
    Q_OBJECT

public:
    explicit SegmentGAC(QFile *filesegment = 0, SatelliteList *satl = 0, QObject *parent = 0);
    ~SegmentGAC();

    bool inspectMPHRrecord(QByteArray mphr_record);
    quint32 get_next_header( QByteArray ba );
    void RenderSegmentlineInTexture( int channel, int nbrLine, int nbrTotalLine );

    void ComposeProjection(int inputchannel, eProjections proj);
    void RenderSegmentlineInProjection( int channel, int nbrLine, int heightintotalimage, eProjections proj );
    void ComposeSegmentLCCProjection(int inputchannel);
    void ComposeSegmentGVProjection(int inputchannel);
    void ComposeSegmentSGProjection(int inputchannel);


    Segment *ReadSegmentInMemory();
    int ReadNbrOfLines();

    double earth_loc_lon_first[360], earth_loc_lat_first[360];
    double earth_loc_lon_last[360], earth_loc_lat_last[360];
    double earth_loc_altitude[360];

signals:
    
public slots:
    
};

#endif // SEGMENTGAC_H
