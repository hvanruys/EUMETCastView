#ifndef SATELLITE_H
#define SATELLITE_H

#include "sgp4sdp4.h"
//#include "stdafx.h"

#include "qtle.h"
#include "qsgp4.h"
#include "qeci.h"
#include "qgeocentric.h"
#include "qgeodetic.h"
#include "qobserver.h"


#include <QPainter>
#include <QString>
#include <QColor>
#include <QVector2D>
#include <QVector3D>

class Satellite
{

    friend class SatelliteList;

public:

    Satellite() {}
    ~Satellite();
    Satellite(QString satname, QString l1, QString l2, const QColor & color );
    void initSatellite(QString satname, QString l1, QString l2, const QColor & color);
    void SetActive(bool act) {  active = act; }
    void RenderSatellite( QPainter *painter, bool trackon);
    void showSatHorizon(double lon, double lat, double geo_alt, QPainter *painter, const QColor & color);

    void const GetSatelliteEphem(QGeodetic &qgeo, QTopocentric &qtopo );
    void GetSatellitePosition(QVector3D &position, QVector3D &positionnorm, float &alt);
    int GetCatalogueNbr() { return catnr; }
    double GetEpoch() { return epoch; }
    QString line1;
    QString line2;
    bool active; // sat is selected
    QString sat_name; /* Satellite name string    */
    QString idesg;    /* International Designator */
    QVector2D winsatpos;
    QTle *qtle;
    QSgp4 *qsgp4;
    QVector3D position;
    QVector3D positionnorm;
    float altitude;
    QVector2D equidistposition;


private:

    //void initializeGlRendering();
    int minutesshown;

    double epoch;

    double
        xndt2o, xndd6o, bstar,
        xincl, xnodeo, eo, omegao, xmo, xno;
    int
        catnr,  /* Catalogue Number  */
        elset,  /* Element Set       */
        revnum; /* Revolution Number */

    //double geo_alt;
    QColor thecolor;
    int DaysOld;
    double current_lon, current_lat, current_alt;
    double current_az, current_el, current_rate, current_range;
};

class SatelliteList
{
public:
    SatelliteList(void);
    ~SatelliteList();
    void Initialize();
    QStringList GetCatnrList(void);
    QStringList GetActiveSatList(void);
    //  bool IsActive(const int catnr);
    void RenderAllSatellites(QPainter *painter);
    void showHorizon(double lon, double lat, double geo_alt, QPainter *painter);
    void ReloadList(void);
    void SetActive(const int catnr);
    void ClearActive(void);
    //  int NbrActiveSats(void);
    void SetSelectedSat(const int catnr);
    int GetSelectedSat(void) { return(selectedsat); }
    double GetSelectedSatAlt(void) { return(selectedsat_alt); }
    void GetSatelliteEphem(const int catnbr, double *deg_lon, double *deg_lat, double *alt, double *az, double *el, double *range, double *rate);
    void TestForSat(int x, int y);
    void TestForSatGL(int x, int y);

    Satellite* GetSatellite(const int catnr, bool *ok);
    //bool GetSatellite(const int catnr, Satellite *sat);
    bool SatExistInList(const int catnr);
    QList<Satellite *>  *GetSatlist(void) { return(& satlist); }

private:

    QList<Satellite *> satlist;
    void ReReadTle(void);
    int selectedsat;
    double selectedsat_alt;
    double GetSatAlt(const int catnr);
    int savetdiff;
    QVector<float> satpositions;


};

#endif
