#ifndef AVHRRSATELLITE_H
#define AVHRRSATELLITE_H

#include <QObject>
#include <QMessageBox>
//#include <QtXml>

#include "satellite.h"
#include "segmentmetop.h"
#include "segmenthrp.h"
#include "segmentviirsm.h"
#include "segmentviirsdnb.h"
#include "segmentolci.h"
#include "segmentmersi.h"
#include "segmentvii.h"

#include "segmentlistmetop.h"
#include "segmentlistvii.h"
#include "segmentlisthrp.h"
#include "segmentlistviirsm.h"
#include "segmentlistviirsdnb.h"
#include "segmentlistolci.h"
#include "segmentlistmersi.h"
#include "segmentlistvii.h"

#include "segmentlistgeostationary.h"
#include "segmentimage.h"
#include "options.h"

class SegmentList;
class SegmentListGeostationary;
class SegmentListMetop;
class SegmentListVII;
class SegmentListHRP;
class SegmentListVIIRSM;
class SegmentListVIIRSDNB;
class SegmentListOLCI;
class SegmentListMERSI;


class AVHRRSatellite  : public QObject
{
    Q_OBJECT

public:
    AVHRRSatellite(QObject *parent = 0);
    ~AVHRRSatellite();

    void ReadDirectories(QDate seldate, int hoursbefore);
    void ReadDirectoriesDatahub(QDate seldate);

    void AddSegmentsToList(QFileInfoList fileinfolist);
    SegmentListGeostationary *getActiveSegmentList();
    bool SelectedAVHRRSegments();
    bool SelectedVIIRSMSegments();
    bool SelectedVIIRSDNBSegments();
    bool SelectedOLCIefrSegments();
    bool SelectedOLCIerrSegments();
    bool SelectedMERSISegments();

    void RemoveAllSelectedAVHRR();
    void RemoveAllSelectedMETIMAGE();
    void RemoveAllSelectedVIIRSM();
    void RemoveAllSelectedVIIRSDNB();
    void RemoveAllSelectedVIIRSMNOAA20();
    void RemoveAllSelectedVIIRSDNBNOAA20();
    void RemoveAllSelectedVIIRSMNOAA21();
    void RemoveAllSelectedVIIRSDNBNOAA21();
    void RemoveAllSelectedOLCIefr();
    void RemoveAllSelectedOLCIerr();
    void RemoveAllSelectedSLSTR();
    void RemoveAllSelectedMERSI();

    void emitProgressCounter(int);

    QString GetOverviewSegments();
    QStringList GetOverviewSegmentsMetop();
    QStringList GetOverviewSegmentsMetopSGA1();
    QStringList GetOverviewSegmentsHRP();

    QStringList GetOverviewSegmentsVIIRSM();
    QStringList GetOverviewSegmentsVIIRSDNB();
    QStringList GetOverviewSegmentsVIIRSMNOAA20();
    QStringList GetOverviewSegmentsVIIRSDNBNOAA20();
    QStringList GetOverviewSegmentsVIIRSMNOAA21();
    QStringList GetOverviewSegmentsVIIRSDNBNOAA21();
    QStringList GetOverviewSegmentsOLCIefr();
    QStringList GetOverviewSegmentsOLCIerr();

    QStringList GetOverviewSegmentsMERSI();

    QStringList GetOverviewSegmentsGeo(int geoindex);
    QStringList GetDatestampsList(int geoindex);



    //void drawOverlay(char *pFileName );
    bool getShowAllSegments() { return showallsegments; }
    void setShowAllSegments(bool allseg) { showallsegments = allseg; }

    void setAbsolutePathFromMap(int geoindex, QString strdate);


    SegmentListMetop *seglmetop;
    SegmentListVII *seglmetopsga1;

    SegmentListHRP *seglhrp;
    SegmentListOLCI *seglolciefr;
    SegmentListOLCI *seglolcierr;

    SegmentListVIIRSM *seglviirsm;
    SegmentListVIIRSDNB *seglviirsdnb;
    SegmentListVIIRSM *seglviirsmnoaa20;
    SegmentListVIIRSDNB *seglviirsdnbnoaa20;
    SegmentListVIIRSM *seglviirsmnoaa21;
    SegmentListVIIRSDNB *seglviirsdnbnoaa21;

    SegmentListMERSI *seglmersi;

    QList<SegmentListGeostationary *> seglgeo;
    QList<QMap<QString, QMap<QString, QMap< int, QFileInfo > > > > segmentlistmapgeo;
    QMap<int, QMap< int, QFileInfo > > segmentlistmapgeomtgi1;
    QDate selectiondate;


private:

    void InsertToMap(QFileInfoList fileinfolist, QMap<QString, QFileInfo> *map, bool *metopTle, bool *metopsga1Tle, bool *nppTle, bool *sentinel3Tle, bool *fy3dTle,
                     QDate seldate, int hoursbefore);
    void RemoveFromList(QList<Segment*> *sl);
    void getFilenameParameters(int geosatindex, QString filename, QString *strspectrum, QString *strdate, int *filenbr);
    void getFilenameParametersMTGI1(QString filename, QString *strdate, int *filenbr, int *seqnbr);

    long nbrofpointsselected;
    long countmetop;
    long countmetopsga1;
    long counthrp;
    long countviirsm;
    long countviirsdnb;
    long countviirsmnoaa20;
    long countviirsdnbnoaa20;
    long countviirsmnoaa21;
    long countviirsdnbnoaa21;
    long countolciefr;
    long countolcierr;
    long countmetopAhrpt;
    long countmetopBhrpt;
    long countnoaa19hrpt;
    long countM01hrpt;
    long countM02hrpt;
    long countdatahubolciefr;
    long countdatahubolcierr;
    long countdatahubslstr;
    long countmersi;

    bool showallsegments;

signals:
    void signalProgress(int progress); // in formephem
    void signalResetProgressbar(int max, const QString &text);
    void signalAddedSegmentlist(void);
    void signalShowSegmentCount(void);
    //void signalMeteosatSegment(QString, QString, int);
    void progressCounter(int);
    void signalXMLProgress(QString, int, bool);


public slots:
    void AddSegmentsToListFromUdp(QByteArray thefilepath);

};


#endif // AVHRRSATELLITE_H
