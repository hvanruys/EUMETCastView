#include <qstringlist.h>

#include <QFile>
#include <qtextstream.h>
#include <QColor>
#include <QString>
#include <QDebug>
#include <QVector3D>

//#include "stdafx.h"
#include "satellite.h"

#include "globals.h"
#include "options.h"

extern Options opts;

Satellite::Satellite( QString satname, QString l1, QString l2, const QColor & color  )
{
    initSatellite(satname, l1, l2, color);
}

Satellite::~Satellite()
{
    delete qtle;
    delete qsgp4;
}

void Satellite::initSatellite(QString satname, QString l1, QString l2, const QColor & color)
{
    bool ok;
    QString buff;

    thecolor = color;

    qtle = new QTle(satname, l1, l2, QTle::wgs72);
    qsgp4 = new QSgp4( *qtle );

    sat_name = satname;

    catnr = l1.mid(2,5).toInt(&ok, 10);
    epoch = l1.mid(18,14).toDouble(&ok);

    line1 = l1;
    line2 = l2;

    struct tm utc;

    double
        tsince,            /* Time since epoch (in minutes) */
        jul_epoch,         /* Julian date of epoch          */
        jul_utc;           /* Julian UTC date               */

    /* Get UTC calendar and convert to Julian */
    UTC_Calendar_Now(&utc);
    jul_utc = Julian_Date(&utc);
    jul_epoch = Julian_Date_of_Epoch(epoch);
    tsince = jul_utc - jul_epoch; // in days
    //qDebug() << QString("sat_name = %1 jul_utc = %2   jul_epoch = %3  epoch = %4  tsince = %5").arg(sat_name).arg(jul_utc).arg(jul_epoch).arg(epoch).arg(tsince);

    DaysOld = (int)tsince;
    winsatpos = QVector2D(9999.0,9999.0);

}

void Satellite::RenderSatellite(QPainter *painter, bool trackon)
{
    bool bSmallscreen;

    /* Calendar date and time (UTC) */
    struct tm utc;
    struct tm utc1;
    //
    int devwidth = (painter->device())->width();
    int devheight = (painter->device())->height();
    if (devwidth < 640 && devheight < 480 )
        bSmallscreen = true;
    else
        bSmallscreen = false;
    //
    //	loc.lon = lamda0 = de2ra * opts.getObsLon();
    //	loc.lat = phi1 = de2ra * opts.getObsLat();
    //	loc.alt = de2ra * opts.getObsAlt();
    //
    int posx, posy;
    int posx1, posy1;
    int save_posx;

    thecolor = Qt::yellow;
    painter->setPen( thecolor );
    painter->setBrush( Qt::NoBrush );
    painter->setFont( QFont( "helvetica", 12) );
    //
    //
    double
        tsince,            /* Time since epoch (in minutes) */
        tsince1,
        jul_epoch,         /* Julian date of epoch          */
        jul_utc,           /* Julian UTC date               */
        jul_utc1,
        save_lon;
    //
    // // double az, el, range, rate;
    //

    /* Get UTC calendar and convert to Julian */
    UTC_Calendar_Now(&utc);
    jul_utc = Julian_Date(&utc);
    UTC_Calendar_Now(&utc1);
    utc1.tm_sec = 0;
    jul_utc1 = Julian_Date(&utc1);

    /* Convert satellite's epoch time to Julian  */
    /* and calculate time since epoch in minutes */
    jul_epoch = Julian_Date_of_Epoch(epoch);
    tsince = (jul_utc - jul_epoch) * MIN_PER_DAY; // in minuten
    tsince1 = (jul_utc1 - jul_epoch) * MIN_PER_DAY; // in minuten

    double id;
    int tdiff;

    tdiff = opts.realminutesshown;

    QEci qeci;
    qsgp4->getPosition(tsince1-tdiff, qeci);
    QGeodetic qgeo = qeci.ToGeo();

    //orbit->getPosition(tsince1-tdiff, &eci);
    //geo = eci.toGeo();

    //Eci Veci = Vorbit->FindPosition(tsince1-tdiff);
    //Vgeo = Veci.ToGeodetic();

    if (qgeo.longitude < PI)
        posx = (int)(devwidth * ( qgeo.longitude + PI ) / TWOPI);
    else
        posx = (int)(devwidth * ( qgeo.longitude - PI ) / TWOPI);
    posy = (int)( devheight * ( PIO2 - qgeo.latitude ) / PI);
    posx1=posx;
    posy1=posy;
    save_posx = posx;

    save_lon = qgeo.longitude;
    if (trackon)
    {
        thecolor = QColor(200,200,0);
        painter->setPen( thecolor );

        for( id = tsince1 - tdiff + 1; id <= tsince1; id++ )
        {
            qsgp4->getPosition(id, qeci);
            qgeo = qeci.ToGeo();

            if (qgeo.longitude < PI)
                posx = (int)(devwidth * ( qgeo.longitude + PI ) / TWOPI);
            else
                posx = (int)(devwidth * ( qgeo.longitude - PI ) / TWOPI);

            posy = (int)(devheight * ( PIO2 - qgeo.latitude ) / PI);


            if (((save_posx >= devwidth*0.9) && (posx < devwidth*0.1)) ||
                ((save_posx <= devwidth*0.1) && (posx > devwidth*0.9)) ||
                (posy <= devheight*0.02) || (posy >= devheight*0.98) )
            {
                if (bSmallscreen == false)
                    painter->drawEllipse( posx -  2 , posy - 2, 4, 4 );
                //				painter->moveTo( posx, posy );
                posx1=posx;
                posy1=posy;
            }
            else
            {
                if (bSmallscreen == false)
                    painter->drawEllipse( posx -  2 , posy - 2, 4, 4 );
                //				painter->lineTo( posx, posy );
                painter->drawLine(posx1, posy1, posx, posy);
                posx1=posx;
                posy1=posy;
            }

            save_lon = qgeo.longitude;
            save_posx = posx;

        }

        thecolor = Qt::yellow;
        painter->setPen( thecolor );

        tdiff = opts.realminutesshown;

        for( id = tsince1 + 1; id <= tsince1 + tdiff; id++ )
        {
            qsgp4->getPosition(id, qeci);
            qgeo = qeci.ToGeo();

            //orbit->getPosition(id, &eci);
            //geo = eci.toGeo();

            if (qgeo.longitude < PI)
                posx = (int)(devwidth * ( qgeo.longitude + PI ) / TWOPI);
            else
                posx = (int)(devwidth * ( qgeo.longitude - PI ) / TWOPI);
            posy = (int)(devheight * ( PIO2 - qgeo.latitude ) / PI);

            if (((save_posx >= devwidth*0.9) && (posx < devwidth*0.1)) ||
                ((save_posx <= devwidth*0.1) && (posx > devwidth*0.9)) ||
                (posy <= devheight*0.02) || (posy >= devheight*0.98) )
            {
                if (bSmallscreen == false)
                    painter->drawEllipse( posx -  2 , posy - 2, 4, 4 );
                //				painter->moveTo( posx, posy );
                posx1=posx;
                posy1=posy;
            }
            else
            {
                if (bSmallscreen == false)
                    painter->drawEllipse( posx -  2 , posy - 2, 4, 4 );
                //				painter->lineTo( posx, posy );
                painter->drawLine(posx1, posy1, posx, posy);
                posx1=posx;
                posy1=posy;
            }

            save_lon = qgeo.longitude;
            save_posx = posx;

        }
    }

    qsgp4->getPosition(tsince, qeci);
    qgeo = qeci.ToGeo();

    if (qgeo.longitude < PI)
        posx = (int)(devwidth * ( qgeo.longitude + PI ) / TWOPI);
    else
        posx = (int)(devwidth * ( qgeo.longitude - PI) / TWOPI);

    posy = (int)( devheight * ( PIO2 - qgeo.latitude ) / PI);

    equidistposition.setX(posx);
    equidistposition.setY(posy);

    painter->setPen( Qt::red );
    painter->setBrush( Qt::yellow );
    painter->drawEllipse( posx -  3 , posy - 3, 6, 6 );

    if (trackon)
        painter->setPen( Qt::yellow);
    else
        painter->setPen( Qt::red);
    painter->drawText( posx, posy, sat_name);


    showSatHorizon( qgeo.longitude, qgeo.latitude, qgeo.altitude, painter, thecolor );
    painter->setPen( Qt::yellow );
    for( id = tsince - 3; id <= tsince - 1; id++ )
    {
        qsgp4->getPosition(id, qeci);
        qgeo = qeci.ToGeo();

        //orbit->getPosition(id, &eci);
        //geo = eci.toGeo();

        if (qgeo.longitude < PI)
            posx = (int)(devwidth * ( qgeo.longitude + PI ) / TWOPI);
        else
            posx = (int)(devwidth * ( qgeo.longitude - PI ) / TWOPI);
        posy = (int)(devheight * ( PIO2 - qgeo.latitude ) / PI);
        painter->drawEllipse( posx -  2 , posy - 2, 4, 4 );
    }
}


//void Satellite::RenderSatelliteGL( QMatrix4x4 projection, QMatrix4x4 modelview, bool trackon)
//{
//    //qDebug() << "In RenderSatelliteGL()";

//    /* Calendar date and time (UTC) */

//    struct tm utc;
//    struct tm utc1;

//    double
//            tsince,            // Time since epoch (in minutes)
//            tsince_0,
//            jul_epoch,         // Julian date of epoch
//            jul_utc,           // Julian UTC date
//            jul_utc_0;

//    UTC_Calendar_Now(&utc);
//    jul_utc = Julian_Date(&utc);
//    UTC_Calendar_Now(&utc1);
//    utc1.tm_sec = 0;
//    jul_utc_0 = Julian_Date(&utc1);

//    jul_epoch = Julian_Date_of_Epoch(epoch);
//    tsince = (jul_utc - jul_epoch) * MIN_PER_DAY; // in minutes
//    tsince_0 = (jul_utc_0 - jul_epoch) * MIN_PER_DAY; // in minutes

//    double id;
//    int tdiff;


//    QEci qeci;
//    QGeodetic qgeo;
//    floatVector pos;
//    floatVector vel;



//    tdiff = opts.realminutesshown;

////    QOpenGLVertexArrayObject::Binder vaoBinder(vao);

////    if( tdiff != savetdiff)
////    {
////        savetdiff = tdiff;
////        satposBuf.bind();
////        satposBuf.allocate(tdiff * 2 * 3 * sizeof(GLfloat));
////        satposBuf.release();


////    }

////    qDebug() << "  satposBuf " << satposBuf.size();



//    ////    glColor3ub( QColor(opts.sattrackcolor).red(),  QColor(opts.sattrackcolor).green(),  QColor(opts.sattrackcolor).blue());

//    //if (trackon)
//    //{
////    satpositions.clear();

////    for( id = tsince - tdiff + 1; id <= tsince + tdiff; id++ )
////    {
////        qsgp4->getPosition(id, qeci);
////        qgeo = qeci.ToGeo();
////        LonLat2PointRad(qgeo.latitude, qgeo.longitude, &pos, 1.001f);
////        satpositions.append(pos.x);
////        satpositions.append(pos.y);
////        satpositions.append(pos.z);
////    }

////    satposBuf.write(0, satpositions.data(), satpositions.count() * sizeof(GLfloat));

////    //qDebug() << "satpositions " << satpositions.size() << "  satposBuf " << satposBuf.size();

////    program->bind();

////    program->setUniformValue("MVP", projection * modelview);
////    QMatrix3x3 norm = modelview.normalMatrix();
////    program->setUniformValue("NormalMatrix", norm);

////    QColor rendercolor(opts.globeoverlaycolor1);
////    program->setUniformValue("outcolor", QVector4D(rendercolor.redF(), rendercolor.greenF(), rendercolor.blueF(), 1.0f));
//    //for(int i = 0; i < satpositions.count(); i++)
//    //    glDrawArrays(GL_LINE_STRIP, 0, satpositions.count());



//    //}

//}



const void Satellite::GetSatelliteEphem( QGeodetic &qgeo, QTopocentric &qtopo )
{

    struct tm utc;

    double
        tsince,            /* Time since epoch (in minutes) */
        jul_epoch,         /* Julian date of epoch          */
        jul_utc;           /* Julian UTC date               */


    /* Get UTC calendar and convert to Julian */
    UTC_Calendar_Now(&utc);
    jul_utc = Julian_Date(&utc);
    /* Convert satellite's epoch time to Julian  */
    /* and calculate time since epoch in minutes */
    jul_epoch = Julian_Date_of_Epoch(epoch);
    tsince = (jul_utc - jul_epoch) * MIN_PER_DAY; // in minuten

    QEci qeci;
    qsgp4->getPosition(tsince, qeci);
    qgeo = qeci.ToGeo();


    QObserver siteEquator(opts.getObsLat(), opts.getObsLon(), opts.getObsAlt()); // 0.00 N, 100.00 W, 0 km altitude

    qtopo = siteEquator.GetLookAngle(qeci);

    //qDebug() << siteEquator.toString().c_str();

}

void Satellite::GetSatellitePosition(QVector3D &position, QVector3D &positionnorm, float &alt)
{
    struct tm utc;
    QVector3D pos;

    double
        tsince,            /* Time since epoch (in minutes) */
        jul_epoch,         /* Julian date of epoch          */
        jul_utc;           /* Julian UTC date               */


    /* Get UTC calendar and convert to Julian */
    UTC_Calendar_Now(&utc);
    jul_utc = Julian_Date(&utc);
    /* Convert satellite's epoch time to Julian  */
    /* and calculate time since epoch in minutes */
    jul_epoch = Julian_Date_of_Epoch(epoch);
    tsince = (jul_utc - jul_epoch) * MIN_PER_DAY; // in minuten

    QEci qeci;
    qsgp4->getPosition(tsince, qeci);
    QGeodetic qgeo = qeci.ToGeo();
    alt = (XKMPER + qgeo.altitude)/XKMPER;
    LonLat2PointRad(qgeo.latitude, qgeo.longitude, &position, alt);
    LonLat2PointRad(qgeo.latitude, qgeo.longitude, &positionnorm, 1.0);
    this->position = position;
    this->positionnorm = positionnorm;
}

void Satellite::showSatHorizon(double lon, double lat, double geo_alt, QPainter *painter, const QColor & col)
{
    double alpha, gamma, aa;
    int posx, posy;
    int posx1, posy1;

    // QPaintDeviceMetrics pdm( painter->device() );
    int devwidth = painter->device()->width();
    int devheight = painter->device()->height();

    painter->setPen(col);

    if (lon < PI)
        posx = (int)(devwidth * ( lon + PI) / TWOPI);
    else
        posx = (int)(devwidth * ( lon - PI) / TWOPI);

    posy = (int)( devheight * ( PIO2 - lat ) / PI);
    painter->drawEllipse( posx -  2 , posy - 2, 4, 4 );

    double wcirkel = ArcCos( XKMPER / ( XKMPER + geo_alt ));
    //double wcirkel = ((2 * XKMPER * geo_alt  +  geo_alt * geo_alt) / ( XKMPER + geo_alt))/XKMPER;
    //double wcirkel = (XKMPER * sqrt(2 * XKMPER * geo_alt  +  geo_alt * geo_alt) / ( XKMPER + geo_alt))/XKMPER;

    if (lat + wcirkel > PI/2)
    {
        posy = (int)( devheight * (lat + wcirkel - PIO2) / PI);
        posx = (int)( devwidth * (lon)/TWOPI);
    }
    else
    {
        posy = (int)( devheight * (PIO2 - (lat + wcirkel)) / PI);
        if (lon < PI)
            posx = (int)( devwidth * (lon + PI)/TWOPI);
        else
            posx = (int)( devwidth * (lon - PI)/TWOPI);
    }

    //		painter->moveTo( posx, posy );
    posx1=posx;
    posy1=posy;

    //		QPoint currposx;

    for (alpha=PIO2/20; alpha <= TWOPI+ 0.1; alpha += PIO2/20)
    {
        gamma = atan2( sin(alpha)*sin(wcirkel), ( cos(wcirkel)*sin(PIO2 - lat) - sin(wcirkel)*cos(PIO2 - lat)*cos(alpha) ));
        aa = AcTan( sin(PIO2 - lat),cos(PIO2 - lat)*cos(gamma) + (sin(gamma)/ tan(alpha)));

        if (lon + gamma < 0)
            gamma = gamma + TWOPI;

        if (lon + gamma < PI)
            posx = (int)(devwidth * ( lon + gamma + PI) / TWOPI);
        else
            posx = (int)(devwidth * ( lon + gamma - PI) / TWOPI);

        posy = (int)(devheight * ( aa ) / PI);
        //			currposx = painter->pos();

        if ( abs(posx1 - posx) > devwidth*0.2)
        {
            //                          painter->moveTo( posx, posy );
            posx1=posx;
            posy1=posy;
        }
        else
        {
            //                          painter->lineTo( posx, posy );
            painter->drawLine(posx1, posy1, posx, posy);
            posx1=posx;
            posy1=posy;
        }

        //glBegin(GL_LINE_STRIP);
        //    glColor3f(1.0f, 1.0f, 1.0f);
        //    glVertex3f(0.0f, 0.0f, 0.0f);    // V0
        //    glVertex3f(5.0f, 5.0f, 0.0f);    // V1
        //    glVertex3f(5.0f, 10.0f, 0.0f);    // V2
        //glEnd();
    }
}


///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
SatelliteList::SatelliteList()
{

}

void SatelliteList::Initialize( void )
{
    selectedsat = 0;
    selectedsat_alt = 830.0;
    qDebug() << "creator satelliteList";

    ReReadTle();
}

SatelliteList::~SatelliteList()
{
    qDebug() << "destructor SatelliteList nbr of elements = " << satlist.count();

    qDeleteAll(satlist.begin(), satlist.end());
    satlist.clear();
}

void SatelliteList::ReloadList(void)
{
    qDeleteAll(satlist.begin(), satlist.end());
    satlist.clear();

    ReReadTle();
}


void SatelliteList::SetSelectedSat(const int catnr)
{
    selectedsat = catnr;
    selectedsat_alt = GetSatAlt(catnr);
}

/*
inline int SatelliteList::GetSelectedSat(void)
{
  return(selectedsat);
}

inline double SatelliteList::GetSelectedSatAlt(void)
{
  return(selectedsat_alt);
}
*/

void SatelliteList::ReReadTle(void)
{
    QString lleft;
    QStringList tleList;


    QString line, line1, line2;
    Satellite *thesat;
    bool ok;

    satlist.clear();

    qDebug() << "ReReadTle : lengte satlist = "  << satlist.size() << " before iterator";

    for ( QStringList::Iterator it = opts.tlelist.begin(); it != opts.tlelist.end(); ++it )
    {
        QFile file( *it );
        if (file.open(QIODevice::ReadOnly))
        {
            qDebug() << "file open : " + *it;

            QTextStream stream( &file );
            while (!stream.atEnd())
            {
                line = stream.readLine(); // line of text excluding '\n'
                lleft = line.left(1);
                if ((lleft!=QString("1")) && (lleft!=QString("2")))
                {
                    line1 = stream.readLine();
                    lleft = line1.left(1);
                    if (lleft!=QString("1")) break;
                    line2 = stream.readLine();
                    lleft = line2.left(1);
                    if (lleft!=QString("2")) break;

                    thesat = new Satellite(line.trimmed(), line1, line2, Qt::yellow );
                    thesat->active = false;

                    for ( QStringList::Iterator itc = opts.catnbrlist.begin(); itc != opts.catnbrlist.end(); ++itc )
                    {
                        if ( (*itc).toInt( &ok, 10) == line1.mid(2, 5).toInt( &ok, 10 ) )
                        {
                            thesat->active = true;
                            break;
                        }
                    }

                    satlist.append( thesat );
                }
            }
            file.close();
        }
    }

    qDebug() << "ReReadTle : lengte satlist = "  << satlist.size() << " after iterator";

}

QStringList SatelliteList::GetCatnrList(void)
{
    QStringList b;

    for (int i = 0; i < satlist.size(); ++i)
    {
        if ( satlist.at(i)->active == true )
        {
            QString nr;
            nr.setNum(satlist.at(i)->catnr, 10);
            b.append(nr);
        }

    }

    //  Satellite *sat;
    //
    //  for ( sat=satlist.begin(); sat != satlist.end(); ++sat )
    //  {
    //    if (sat->active)
    //    {
    //      QString nr;
    //      nr.setNum(sat->catnr, 10);
    //      b.append(nr);
    //    }
    //  }


    //    QList<Satellite>::iterator sat = satlist.begin();
    //    while (sat != satlist.end()) {
    //        if ( sat->active)
    //        {
    //            QString nr;
    //            nr.setNum(sat->catnr, 10);
    //            b.append(nr);
    //        }
    //        ++sat;
    //    }

    return b;
}

QStringList SatelliteList::GetActiveSatList(void)
{
    QStringList b;
    QString temp;
    QString nr;
    QString daysold;
    QString strlon, strlat, stralt;
    Satellite *sat;

    if(this->satlist.count() == 0)
    {
        b << "no sats";
        return b;
    }


    QGeodetic geo;
    QTopocentric topo;


    for(int i = 0; i < satlist.count(); i++)
    {
        sat = satlist.at(i);
        if(sat->active)
        {
            QString satn = sat->sat_name;
            nr.setNum(sat->catnr, 10);
            daysold.setNum(sat->DaysOld, 10);
            sat->GetSatelliteEphem( geo, topo );

            strlon.setNum( rad2deg(geo.longitude), 'f', 1);
            strlat.setNum( rad2deg(geo.latitude), 'f', 1);
            stralt.setNum( geo.altitude, 'f', 1);

            temp = sat->sat_name + "," + nr + "," + daysold + "," + strlon + "," + strlat + "," + stralt;
            b.append(temp);
        }
    }

    return b;
}

//QStringList SatelliteList::GetActiveSatList(void)
//{
//    QStringList b;
//    QString temp;
//    QString nr;
//    QString daysold;
//    QString strlon, strlat, stralt;

//    if(satlist.count() == 0)
//    {
//        b << "no sats";
//        return b;
//    }

//    QList<Satellite>::iterator sat = satlist.begin();

//    while ( sat != satlist.end() )
//    {
//        qDebug() << (*sat).sat_name;
//        ++sat;
//    }

//    sat = satlist.begin();

//    QGeodetic geo;
//    QTopocentric topo;
//    while ( sat != satlist.end() )
//    {
//        if((*sat).active)
//        {
//            QString satn = (*sat).sat_name;
//            nr.setNum((*sat).catnr, 10);
//            daysold.setNum((*sat).DaysOld, 10);
//            (*sat).GetSatelliteEphem( geo, topo );

//            strlon.setNum( rad2deg(geo.longitude), 'f', 1);
//            strlat.setNum( rad2deg(geo.latitude), 'f', 1);
//            stralt.setNum( geo.altitude, 'f', 1);
//            qDebug() << satn << " " << nr << " " << daysold << " " << strlon << " " << strlat << " " << stralt;

//            temp = (*sat).sat_name + "," + nr + "," + daysold + "," + strlon + "," + strlat + "," + stralt;
//            b.append(temp);
//        }
//        ++sat;
//    }

//    return b;
//}

void SatelliteList::ClearActive(void)
{
    //QStringList b;

    //for (int i = 0; i < satlist.size(); ++i) {
    //  satlist.at(i).SetActive(FALSE);
    //}

    QList<Satellite*>::iterator sat = satlist.begin();

    while( sat != satlist.end() )
    {
        (*sat)->SetActive(false);

        ++sat;
    }

}


void SatelliteList::RenderAllSatellites(QPainter *painter)
{

    // qDebug() << "renderallsatellites " << satlist.size();

    QList<Satellite*>::iterator sat = satlist.begin();

    while ( sat != satlist.end() )
    {
        if((*sat)->active == true)
        {
            if( (*sat)->catnr==selectedsat )
                (*sat)->RenderSatellite(painter, true);
            else
                (*sat)->RenderSatellite(painter, false);

        }
        ++sat;

    }
}


void SatelliteList::showHorizon(double lon, double lat, double geo_alt, QPainter *painter)
{
    double alpha, gamma, aa;
    int posx, posy;
    int posx1, posy1;

    int devwidth = (painter->device())->width();
    int devheight = (painter->device())->height();

    painter->setPen(Qt::red);
    painter->setBrush(Qt::white);


    if (lon < PI)
        posx = (int)(devwidth * ( lon + PI) / TWOPI);
    else
        posx = (int)(devwidth * ( lon - PI) / TWOPI);
    posy = (int)( devheight * ( (PI/2) - lat ) / PI);

    painter->drawEllipse( posx -  2 , posy - 2, 4, 4 );

    double wcirkel = ArcCos( (EARTH_DIA/2) / ( (EARTH_DIA/2) + geo_alt ));


    if (lat + wcirkel > (PI/2))
    {
        posy = (int)( devheight * (lat + wcirkel - PI/2) / PI);
        posx = (int)( devwidth * (lon)/TWOPI);
    }
    else
    {
        posy = (int)( devheight * (PI/2 - (lat + wcirkel)) / PI);
        if (lon < PI)
            posx = (int)( devwidth * (lon + PI)/TWOPI);
        else
            posx = (int)( devwidth * (lon - PI)/TWOPI);
    }

    //  painter->moveTo( posx, posy );
    posx1=posx;
    posy1=posy;

    double sinwcirkel = sin(wcirkel);
    double coswcirkel = cos(wcirkel);
    double lat_cirkel, lon_cirkel, dlon;

    for (alpha=0; alpha <= TWOPI + 0.01; alpha += (PI/2)/20)
    {

        // lat, lon voor een koers vanuit pt 1 ( tclplus ) op een afstand wcirkel
        lat_cirkel = asin(sin(lat)*cos(wcirkel)+cos(lat)*sin(wcirkel)*cos(alpha));
        dlon=atan2(sin(alpha)*sin(wcirkel)*cos(lat), cos(wcirkel)-sin(lat)*sin(lat_cirkel));
        lon_cirkel=FMod2p(lon-dlon+PI)-PI;

        sphericalToPixel( lon_cirkel, lat_cirkel, posx, posy, devwidth, devheight );
        //painter->drawEllipse( posx-2, posy-2, 4, 4 );

        //    if ( abs(posx1 - posx ) <= devwidth*0.4)
        //       painter->drawLine( posx, posy, posx1, posy1 );
        //
        //
        if ( abs(posx1 - posx ) > devwidth*0.4)
        {
            //painter->drawEllipse( posx -  1 , posy - 1, 2, 2 );
            //      painter->moveTo( posx, posy );
            posx1=posx;
            posy1=posy;
        }
        else
        {
            //painter->drawEllipse( posx -  4 , posy - 4, 8, 8 );
            //        painter->lineTo( posx, posy );
            painter->drawLine(posx1,posy1,posx,posy);
            posx1=posx;
            posy1=posy;
        }

    }
}




void SatelliteList::TestForSat(int x, int y)
{
    for (int i = 0; i < satlist.size(); ++i)
    {
        if ( satlist.at(i)->active == true )
        {
            if ( (fabs( satlist.at(i)->equidistposition.x() - x ) < 10) && (fabs( satlist.at(i)->equidistposition.y() - y) < 10) )
            {
                if(selectedsat == satlist.at(i)->catnr)
                {
                    selectedsat = 0;
                    selectedsat_alt = 830.0;
                } else
                {
                    selectedsat = satlist.at(i)->catnr;
                    selectedsat_alt = satlist.at(i)->current_alt;
                }
                break;
            }
        }
    }
}

void SatelliteList::TestForSatGL(int x, int y)
{
    for (int i = 0; i < satlist.size(); ++i)
    {
        if ( satlist.at(i)->active == true )
        {
            if ( (fabs( satlist.at(i)->winsatpos.x() - x ) < 10) && (fabs( satlist.at(i)->winsatpos.y() - y) < 10) )
            {
                if(selectedsat == satlist.at(i)->catnr)
                {
                    selectedsat = 0;
                    selectedsat_alt = 830.0;
                } else
                {
                    selectedsat = satlist.at(i)->catnr;
                    selectedsat_alt = satlist.at(i)->current_alt;
                }
                break;
            }
        }
    }
}

void SatelliteList::GetSatelliteEphem(const int catnbr, double *deg_lon, double *deg_lat, double *alt, double *az, double *el, double *range, double *rate)
{

    QList<Satellite*>::iterator sat = satlist.begin();
    QGeodetic geo;
    QTopocentric topo;

    while ( sat != satlist.end() )
    {
        if((*sat)->catnr == catnbr)
        {
            (*sat)->GetSatelliteEphem( geo, topo );
            (*sat)->current_lon = *deg_lon = rad2deg( geo.longitude);
            (*sat)->current_lat = *deg_lat = rad2deg( geo.latitude);
            (*sat)->current_alt = *alt = geo.altitude;
            (*sat)->current_az = *az = rad2deg(topo.azimuth);
            (*sat)->current_el = *el = rad2deg(topo.elevation);
            (*sat)->current_range = *range = topo.range;
            (*sat)->current_rate = *rate = topo.range_rate;
            return;
        }
        ++sat;
    }
}

Satellite *SatelliteList::GetSatellite(const int catnbr, bool *ok)
{

    *ok = false;

    if (satlist.count() > 0)
    {
        QList<Satellite*>::iterator sat = satlist.begin();

        while ( sat != satlist.end() )
        {
            if((*sat)->catnr == catnbr)
            {
                *ok = true;
                return (*sat);
            }
            ++sat;
        }
    }
    return nullptr;
}

//bool SatelliteList::GetSatellite(const int catnbr, Satellite *sat)
//{

//    if (satlist.count() > 0)
//    {
//        QList<Satellite*>::iterator itsat = satlist.begin();

//        while ( itsat != satlist.end() )
//        {
//            if((*itsat)->catnr == catnbr)
//            {
//                *sat = **itsat;
//                return true;
//            }
//            ++itsat;
//        }
//    }

//    return false;
//}

bool SatelliteList::SatExistInList(const int catnr)
{
    QList<Satellite*>::iterator sat = satlist.begin();

    bool ret  = false;

    while ( sat != satlist.end() )
    {
        if((*sat)->catnr == catnr)
        {
            ret = true;
            break;
        }
        ++sat;
    }

    return ret;

}


void SatelliteList::SetActive(const int catnr)
{

    QList<Satellite*>::iterator sat = satlist.begin();

    while ( sat != satlist.end() )
    {
        if((*sat)->catnr == catnr)
        {
            (*sat)->active = true;
            return;
        }
        ++sat;
    }

}

double SatelliteList::GetSatAlt(const int catnr)
{
    QList<Satellite*>::iterator sat = satlist.begin();

    while( sat != satlist.end() )
    {
        if((*sat)->catnr == catnr)
            return((*sat)->current_alt);
        ++sat;
    }
    return(0);
}

