#ifndef AVHRRSATELLITE_H
#define AVHRRSATELLITE_H

#include <QObject>
#include <QMessageBox>
//#include <QtXml>

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
#include "segmentdatahub.h"
#include "segmentmersi.h"

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
#include "segmentlistdatahub.h"
#include "segmentlistmersi.h"
#include "datahubaccessmanager.h"

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
class SegmentListDatahub;
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
    bool SelectedSLSTRSegments();
    bool SelectedMERSISegments();

    void RemoveAllSelectedAVHRR();
    void RemoveAllSelectedVIIRSM();
    void RemoveAllSelectedVIIRSDNB();
    void RemoveAllSelectedVIIRSMNOAA20();
    void RemoveAllSelectedVIIRSDNBNOAA20();
    void RemoveAllSelectedOLCIefr();
    void RemoveAllSelectedOLCIerr();
    void RemoveAllSelectedSLSTR();
    void RemoveAllSelectedDatahubOLCIefr();
    void RemoveAllSelectedDatahubOLCIerr();
    void RemoveAllSelectedDatahubSLSTR();
    void RemoveAllSelectedMERSI();

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
    QStringList GetOverviewSegmentsVIIRSMNOAA20();
    QStringList GetOverviewSegmentsVIIRSDNBNOAA20();
    QStringList GetOverviewSegmentsOLCIefr();
    QStringList GetOverviewSegmentsOLCIerr();
    QStringList GetOverviewSegmentsSLSTR();

    QStringList GetOverviewSegmentsDatahubOLCIefr();
    QStringList GetOverviewSegmentsDatahubOLCIerr();
    QStringList GetOverviewSegmentsDatahubSLSTR();

    QStringList GetOverviewSegmentsMERSI();

    QStringList GetOverviewSegmentsGeo(int geoindex);



    //void drawOverlay(char *pFileName );
    bool getShowAllSegments() { return showallsegments; }
    void setShowAllSegments(bool allseg) { showallsegments = allseg; }

    void LoadXMLfromDatahub(QDate selecteddate, QString type);
    void ReadXMLfiles();
    void setXMLDate(QDate date) { xmlselectdate = date; }
    void setAbsolutePathFromMap(int geoindex, QString strdate);


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
    SegmentListDatahub *segldatahubolciefr;
    SegmentListDatahub *segldatahubolcierr;
    SegmentListDatahub *segldatahubslstr;
    SegmentListVIIRSM *seglviirsmnoaa20;
    SegmentListVIIRSDNB *seglviirsdnbnoaa20;
    SegmentListMERSI *seglmersi;

    QList<SegmentListGeostationary *> seglgeo;
    QList<QMap<QString, QMap<QString, QMap< int, QFileInfo > > > > segmentlistmapgeo;
    QMap<int, QMap< int, QFileInfo > > segmentlistmapgeomtgi1;
    QDate selectiondate;


private:

    void InsertToMap(QFileInfoList fileinfolist, QMap<QString, QFileInfo> *map, bool *noaaTle, bool *metopTle, bool *nppTle, bool *sentinel3Tle, bool *fy3dTle,
                     QDate seldate, int hoursbefore);
    void RemoveFromList(QList<Segment*> *sl);
    void CreateListfromXML(QDomDocument document);
    void getFilenameParameters(int geosatindex, QString filename, QString *strspectrum, QString *strdate, int *filenbr);
    void getFilenameParametersMTGI1(QString filename, QString *strdate, int *filenbr, int *seqnbr);

    DatahubAccessManager hubmanager;
    QDate xmlselectdate;
    long nbrofpointsselected;
    long countmetop;
    long countnoaa;
    long counthrp;
    long countgac;
    long countviirsm;
    long countviirsdnb;
    long countviirsmnoaa20;
    long countviirsdnbnoaa20;
    long countolciefr;
    long countolcierr;
    long countslstr;
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

private slots:
    void XMLFileDownloaded();
    void XMLPagesDownloaded(int pages);

public slots:
    void AddSegmentsToListFromUdp(QByteArray thefilepath);

};


#endif // AVHRRSATELLITE_H
