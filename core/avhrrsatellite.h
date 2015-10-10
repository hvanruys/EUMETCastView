#ifndef AVHRRSATELLITE_H
#define AVHRRSATELLITE_H

#include <QObject>
#include <QMessageBox>

#include "satellite.h"
#include "segmentmetop.h"
#include "segmentnoaa.h"
#include "segmenthrp.h"
#include "segmentgac.h"
#include "segmentviirs.h"

#include "segmentlistmetop.h"
#include "segmentlistnoaa.h"
#include "segmentlisthrp.h"
#include "segmentlistgac.h"
#include "segmentlistgeostationary.h"
#include "segmentlistviirs.h"

#include "segmentimage.h"
#include "options.h"

class SegmentList;
class SegmentListNoaa;
class SegmentListMetop;
class SegmentListHRP;
class SegmentListGAC;
class SegmentListGeostationary;
class SegmentListVIIRS;

class AVHRRSatellite  : public QObject
{
    Q_OBJECT

public:
    AVHRRSatellite(QObject *parent = 0, SatelliteList *lst = 0);
    void ReadDirectories(QDate seldate, int hoursbefore);
    void AddSegmentsToList(QFileInfoList fileinfolist);
    SegmentListGeostationary *getActiveSegmentList();
    bool SelectedAVHRRSegments();
    bool SelectedVIIRSSegments();

    void RemoveAllSelectedAVHRR();
    void RemoveAllSelectedVIIRS();
    QString GetOverviewSegments();
    QStringList GetOverviewSegmentsMetop();
    QStringList GetOverviewSegmentsNoaa();
    QStringList GetOverviewSegmentsGAC();
    QStringList GetOverviewSegmentsHRP();
    QStringList GetOverviewSegmentsVIIRS();

    QStringList GetOverviewSegmentsMeteosat();
    QStringList GetOverviewSegmentsMeteosatRss();
    QStringList GetOverviewSegmentsMeteosat7();
    // QStringList GetOverviewSegmentsElectro();
    QStringList GetOverviewSegmentsGOES13();
    QStringList GetOverviewSegmentsGOES15();
    QStringList GetOverviewSegmentsMTSAT();
    QStringList GetOverviewSegmentsFY2E();
    QStringList GetOverviewSegmentsFY2G();
    QStringList GetOverviewSegmentsH8();


    void drawOverlay(char *pFileName );
    bool getShowAllSegments() { return showallsegments; }
    void setShowAllSegments(bool allseg) { showallsegments = allseg; }

    SegmentListMetop *seglmetop;
    SegmentListNoaa *seglnoaa;
    SegmentListHRP *seglhrp;
    SegmentListGAC *seglgac;
    SegmentListVIIRS *seglviirs;

    SegmentList *segmentlistnoaa;
    SegmentList *segmentlisthrp;
    SegmentList *segmentlistgac;
    SegmentList *segmentlistmetop;
    SegmentListVIIRS *segmentlistviirs;

    SegmentListGeostationary *seglmeteosat;
    SegmentListGeostationary *seglmeteosatrss;
    SegmentListGeostationary *seglmet7;
    //SegmentListGeostationary *seglelectro;
    SegmentListGeostationary *seglgoes13dc3;
    SegmentListGeostationary *seglgoes15dc3;
    SegmentListGeostationary *seglmtsatdc3;
    SegmentListGeostationary *seglgoes13dc4;
    SegmentListGeostationary *seglgoes15dc4;
    SegmentListGeostationary *seglmtsatdc4;
    SegmentListGeostationary *seglfy2e;
    SegmentListGeostationary *seglfy2g;
    SegmentListGeostationary *seglh8;

    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapmeteosat;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapmeteosatrss;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapmet7;
    //QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapelectro;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapgoes13dc3;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapgoes15dc3;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapmtsatdc3;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapgoes13dc4;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapgoes15dc4;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapmtsatdc4;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapfy2e;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapfy2g;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmaph8;


private:

    SatelliteList *satlist;
    long nbrofpointsselected;
    long countmetop;
    long countnoaa;
    long counthrp;
    long countgac;
    long countviirs;
    bool showallsegments;



signals:
    void signalProgress(int progress); // in formephem
    void signalResetProgressbar(int max, const QString &text);
    void signalAddedSegmentlist(void);
    void signalNothingSelected(void);
    //void signalMeteosatSegment(QString, QString, int);

public slots:
    void AddSegmentsToListFromUdp(QByteArray thefilepath);

};


#endif // AVHRRSATELLITE_H
