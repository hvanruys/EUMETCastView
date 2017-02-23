#ifndef AVHRRSATELLITE_H
#define AVHRRSATELLITE_H

#include <QObject>
#include <QMessageBox>

#include "satellite.h"
#include "segmentmetop.h"
#include "segmentnoaa.h"
#include "segmenthrp.h"
#include "segmentgac.h"
#include "segmentviirsm.h"
#include "segmentviirsdnb.h"
#include "segmentolci.h"
#include "segmenthrpt.h"
#include "segmentslstr.h"

#include "segmentlistmetop.h"
#include "segmentlistnoaa.h"
#include "segmentlisthrp.h"
#include "segmentlistgac.h"
#include "segmentlistgeostationary.h"
#include "segmentlistviirsm.h"
#include "segmentlistviirsdnb.h"
#include "segmentlistolci.h"
#include "segmentlisthrpt.h"
#include "segmentlistslstr.h"

#include "segmentimage.h"
#include "options.h"

class SegmentList;
class SegmentListGeostationary;
class SegmentListNoaa;
class SegmentListMetop;
class SegmentListHRP;
class SegmentListGAC;
class SegmentListVIIRSM;
class SegmentListVIIRSDNB;
class SegmentListOLCI;
class SegmentListSLSTR;
class SegmentListHRPT;


class AVHRRSatellite  : public QObject
{
    Q_OBJECT

public:
    AVHRRSatellite(QObject *parent = 0, SatelliteList *lst = 0);
    void ReadDirectories(QDate seldate, int hoursbefore);
    void AddSegmentsToList(QFileInfoList fileinfolist);
    SegmentListGeostationary *getActiveSegmentList();
    bool SelectedAVHRRSegments();
    bool SelectedVIIRSMSegments();
    bool SelectedVIIRSDNBSegments();
    bool SelectedOLCIefrSegments();
    bool SelectedOLCIerrSegments();
    bool SelectedSLSTRSegments();

    void RemoveAllSelectedAVHRR();
    void RemoveAllSelectedVIIRSM();
    void RemoveAllSelectedVIIRSDNB();
    void RemoveAllSelectedOLCIefr();
    void RemoveAllSelectedOLCIerr();
    void RemoveAllSelectedSLSTR();

    void emitProgressCounter(int);

    QString GetOverviewSegments();
    QStringList GetOverviewSegmentsMetop();
    QStringList GetOverviewSegmentsNoaa();
    QStringList GetOverviewSegmentsGAC();
    QStringList GetOverviewSegmentsHRP();

    QStringList GetOverviewSegmentsMetopAhrpt();
    QStringList GetOverviewSegmentsMetopBhrpt();
    QStringList GetOverviewSegmentsNoaa19hrpt();
    QStringList GetOverviewSegmentsM01hrpt();
    QStringList GetOverviewSegmentsM02hrpt();

    QStringList GetOverviewSegmentsVIIRSM();
    QStringList GetOverviewSegmentsVIIRSDNB();
    QStringList GetOverviewSegmentsOLCIefr();
    QStringList GetOverviewSegmentsOLCIerr();
    QStringList GetOverviewSegmentsSLSTR();

    QStringList GetOverviewSegmentsMeteosat();
    QStringList GetOverviewSegmentsMeteosatRss();
    QStringList GetOverviewSegmentsMeteosat7();
    QStringList GetOverviewSegmentsMeteosat8();
    QStringList GetOverviewSegmentsGOES13();
    QStringList GetOverviewSegmentsGOES15();
    QStringList GetOverviewSegmentsGOES16();
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
    SegmentListVIIRSM *seglviirsm;
    SegmentListVIIRSDNB *seglviirsdnb;
    SegmentListOLCI *seglolciefr;
    SegmentListOLCI *seglolcierr;
    SegmentListSLSTR *seglslstr;
    SegmentListHRPT *seglmetopAhrpt;
    SegmentListHRPT *seglmetopBhrpt;
    SegmentListHRPT *seglnoaa19hrpt;
    SegmentListHRPT *seglM01hrpt;
    SegmentListHRPT *seglM02hrpt;


    SegmentListGeostationary *seglmeteosat;
    SegmentListGeostationary *seglmeteosatrss;
    SegmentListGeostationary *seglmet7;
    SegmentListGeostationary *seglmet8;
    SegmentListGeostationary *seglgoes13dc3;
    SegmentListGeostationary *seglgoes15dc3;
    SegmentListGeostationary *seglgoes13dc4;
    SegmentListGeostationary *seglgoes15dc4;
    SegmentListGeostationary *seglgoes16;
    SegmentListGeostationary *seglfy2e;
    SegmentListGeostationary *seglfy2g;
    SegmentListGeostationary *seglh8;

    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapmeteosat;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapmeteosatrss;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapmet7;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapmet8;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapgoes13dc3;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapgoes15dc3;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapgoes13dc4;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapgoes15dc4;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapgoes16;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapfy2e;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmapfy2g;
    QMap<QString, QMap<QString, QMap< int, QFileInfo > > > segmentlistmaph8;




private:

    void InsertToMap(QFileInfoList fileinfolist, QMap<QString, QFileInfo> *map, bool *noaaTle, bool *metopTle, bool *nppTle, bool *sentinel3Tle, QDate seldate, int hoursbefore);
    void RemoveFromList(QList<Segment*> *sl);

    SatelliteList *satlist;
    long nbrofpointsselected;
    long countmetop;
    long countnoaa;
    long counthrp;
    long countgac;
    long countviirsm;
    long countviirsdnb;
    long countviirsmdnb;
    long countolciefr;
    long countolcierr;
    long countslstr;
    long countmetopAhrpt;
    long countmetopBhrpt;
    long countnoaa19hrpt;
    long countM01hrpt;
    long countM02hrpt;

    bool showallsegments;

signals:
    void signalProgress(int progress); // in formephem
    void signalResetProgressbar(int max, const QString &text);
    void signalAddedSegmentlist(void);
    void signalNothingSelected(void);
    //void signalMeteosatSegment(QString, QString, int);
    void progressCounter(int);

public slots:
    void AddSegmentsToListFromUdp(QByteArray thefilepath);

};


#endif // AVHRRSATELLITE_H
