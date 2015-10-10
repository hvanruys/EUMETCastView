#include <QtGui/QMouseEvent>
#include <QDebug>
#include <QString>
#include <QColor>
#include <QtConcurrent/QtConcurrent>
#include <math.h>

#include "globe.h"
#include "options.h"
#include "segmentimage.h"
#include "gshhsdata.h"
#include "pixgeoconversion.h"

extern Options opts;
extern SegmentImage *imageptrs;
extern gshhsData *gshhsdata;

#include <QMutex>

extern QMutex g_mutex;

void Render3DColorTexture(Globe *gl, SegmentListGeostationary::eGeoSatellite sat)
{
      gl->Render3DGeoSegment( sat );
}

void Render3DColorFBO(Globe *gl, SegmentListGeostationary::eGeoSatellite sat)
{
      gl->Render3DGeoSegmentFBO( sat );
}


Globe::Globe(QWidget *parent, SatelliteList *satlist, AVHRRSatellite *seglist ):QOpenGLWidget(parent)
{

    scaleAll = 1;
    lineWidth = 1;

    bBorders = true;
    bSatellites = true;
    bSegmentNames = true;
    opts.bPhongModel = true;

    sats = satlist;
    segs = seglist;

    QImage qim(opts.backgroundimage3D);

    imageptrs->pmOriginal = new QPixmap(QPixmap::fromImage(qim));
    imageptrs->pmOut = new QPixmap(QPixmap::fromImage(qim));
    connect(&watcher, SIGNAL(finished()), this, SLOT(slotRender3DGeoFinished()));

}

void Globe::initializeGL()
{
    qDebug() << "Globe::initializeGL()";

    initializeOpenGLFunctions();

    //connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));

    distance = -3.0;
    trackBall = TrackBall(0.0f, QVector3D(0, 1, 0), TrackBall::Sphere);

    glClearColor(0.05, 0.05, 0.05, 1);

    initShaders();
    initTextures();
    //initFrameBuffer();

    geometries = new GeometryEngine(&programearthnobump);
    geometries->initSphereGeometry(1.0f, 128, 64);

    gshhsdata->initializegshhsData(&programgshhs);
    skybox = new SkyBox(&programskybox);
    //soc = new Soc(&programdraw);
    satgl = new SatGL(&programsatgl, sats, segs);
    segmentgl = new SegmentGL(&programdraw, sats, segs);
    octa = new Octahedron(&programdraw);
    sun = new GeometryEngine(&programdraw);
    sun->initSphereGeometry(1.0f, 32, 16);

    projextends = new ProjExtentsGL(&programdraw);
    texturewriter = new TextureWriter(&programtexturewriter);
}


Globe::~Globe()
{
    delete imageptrs->pmOriginal;
    qDebug() << "closing Globe";

}

QPointF Globe::pixelPosToViewPos(const QPointF& p)
{
     return QPointF(2.0 * float(p.x()) / width() - 1.0,
                         1.0 - 2.0 * float(p.y()) / height());

}


void Globe::renderStations(QMatrix4x4 projection, QMatrix4x4 modelview, QColor color)
{

    QMatrix4x4 modelviewstation;

    QVector3D pos;
    QStringList::Iterator itsname = opts.stationlistname.begin();
    QStringList::Iterator itslon = opts.stationlistlon.begin();
    QStringList::Iterator itslat = opts.stationlistlat.begin();


    while( itsname != opts.stationlistname.end() )
    {

        //qDebug() << QString("name = %1").arg(*itsname);
        LonLat2Point((*itslat).toFloat(), (*itslon).toFloat(), &pos, 1.005f);
        modelviewstation = modelview;
        modelviewstation.translate(pos);
        modelviewstation.scale(0.004f);

        octa->render(projection, modelviewstation, color);

        ++itsname;
        ++itslon;
        ++itslat;
    }

}

void Globe::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
       this->trackBall.push(pixelPosToViewPos(event->pos()), QQuaternion());
       event->accept();
    }

    if(event->buttons() & Qt::RightButton)
    {
       this->mouseDownAction(event->x(), event->y());
       event->accept();
    }


}

void Globe::mouseDownAction(int x, int y)
{
        int realy;  /*  OpenGL y coordinate position  */
        realy = this->height() - y;

        QMatrix4x4 m;
        m.rotate(this->trackBall.rotation());

        bool isselected;
        QString segname;
        if(opts.buttonMetop)
            isselected = segs->seglmetop->TestForSegmentGL( x, realy, distance, m, segs->getShowAllSegments(), segname );
        else if (opts.buttonNoaa)
            isselected = segs->seglnoaa->TestForSegmentGL( x, realy,  distance, m,  segs->getShowAllSegments(), segname );
        else if (opts.buttonHRP)
            isselected = segs->seglhrp->TestForSegmentGL( x, realy,  distance, m,  segs->getShowAllSegments(), segname );
        else if (opts.buttonGAC)
            isselected = segs->seglgac->TestForSegmentGL( x, realy,  distance, m,  segs->getShowAllSegments(), segname );
        else if (opts.buttonVIIRS)
            isselected = segs->seglviirs->TestForSegmentGL( x, realy,  distance, m,  segs->getShowAllSegments(), segname );

        emit mapClicked();

        if(isselected)
        {
            segmentnameselected = segname;
        }
        else
            sats->TestForSatGL(x, y);
}

void Globe::mouseMoveEvent(QMouseEvent *event)
{

    if(event->buttons() & Qt::LeftButton)
    {
        this->trackBall.move(pixelPosToViewPos(event->pos()), QQuaternion());
        event->accept();
    } else {
        this->trackBall.release(pixelPosToViewPos(event->pos()), QQuaternion());
    }

}

void Globe::mouseReleaseEvent(QMouseEvent *event)
{

    if(event->button() == Qt::LeftButton)
    {
        this->trackBall.release(pixelPosToViewPos(event->pos()),QQuaternion());
        event->accept();
    }

}

void Globe::wheelEvent(QWheelEvent * event)
{

    if (event->delta() > 0)
    {
        if(distance > -3)
            distance += 0.05;
        else
            distance += 0.1;
    }
    else
    {
        if(distance > -3)
            distance -= 0.05;
        else
            distance -= 0.1;
    }
    //distance += (float)(event->delta()/60 );
    if (distance < -500.0)
        distance = -500.0;
    if (distance > -1.01)
        distance = -1.01;
    event->accept();

}

void Globe::resizeGL(int w, int h)
{
    qreal aspect = qreal(w) / qreal(h ? h : 1);
    const qreal zNear = 0.01, zFar = 150.0, fov = 45.0;
    projection.setToIdentity();
    projection.perspective(fov, aspect, zNear, zFar);
}

//void Globe::printTexture()
//{

//    glBindFramebuffer(GL_READ_FRAMEBUFFER, imageptrs->fboId);
//    //set the viewport to be the size of the texture
//    glViewport(0,0, imageptrs->pmOut->width(), imageptrs->pmOut->height());
//    int row_size = ((imageptrs->pmOut->width() * 3 + 3) & ~3);
//    int data_size = row_size * imageptrs->pmOut->height();
//    unsigned char * data = new unsigned char [data_size];
//    qDebug() << QString("row_size = %1 data_size = %2").arg(row_size).arg(data_size);
//#pragma pack (push, 1)
//    struct
//    {
//        unsigned char identsize; //Size of following ID field
//        unsigned char cmaptype;//Color map type 0 = none
//        unsigned char imagetype;//Image type 2 = rgb
//        short cmapstart;//First entry in palette
//        short cmapsize;//Number of entries in palette
//        unsigned char cmapbpp;//Number of bits per palette entry
//        short xorigin;//X origin
//        short yorigin;//Y origin
//        short width;//Width in pixels
//        short height;//Height in pixels
//        unsigned char bpp;//Bits per pixel
//        unsigned char descriptor;//Descriptor bits
//    } tga_header;
//#pragma pack (pop)

//    glReadPixels(0, 0, //Origin
//                 imageptrs->pmOut->width(), imageptrs->pmOut->height(), //Size
//                 GL_BGR, GL_UNSIGNED_BYTE,//Format, type
//                 data);//Data

//    memset(&tga_header, 0, sizeof(tga_header));
//    tga_header.imagetype = 2;
//    tga_header.width = (short)imageptrs->pmOut->width();
//    tga_header.height = (short)imageptrs->pmOut->height();
//    tga_header.bpp = 24;
//    FILE * f_out = fopen("screenshot.tga", "wb");
//    fwrite(&tga_header, sizeof(tga_header), 1, f_out);
//    fwrite(data, data_size, 1, f_out);
//    fclose(f_out);
//    delete [] data;
//    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); //unbind the FBO

//}


void Globe::printTexture()
{

#ifndef OPENGL21
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    //set the viewport to be the size of the texture
    glViewport(0,0, this->parentWidget()->width(), this->parentWidget()->height());
    int row_size = ((this->parentWidget()->width() * 3 + 3) & ~3);
    int data_size = row_size * this->parentWidget()->height();
    unsigned char * data = new unsigned char [data_size];
    qDebug() << QString("row_size = %1 data_size = %2").arg(row_size).arg(data_size);
#pragma pack (push, 1)
    struct
    {
        unsigned char identsize; //Size of following ID field
        unsigned char cmaptype;//Color map type 0 = none
        unsigned char imagetype;//Image type 2 = rgb
        short cmapstart;//First entry in palette
        short cmapsize;//Number of entries in palette
        unsigned char cmapbpp;//Number of bits per palette entry
        short xorigin;//X origin
        short yorigin;//Y origin
        short width;//Width in pixels
        short height;//Height in pixels
        unsigned char bpp;//Bits per pixel
        unsigned char descriptor;//Descriptor bits
    } tga_header;
#pragma pack (pop)

    glReadPixels(0, 0, //Origin
                 this->parentWidget()->width(), this->parentWidget()->height(), //Size
                 GL_BGR, GL_UNSIGNED_BYTE,//Format, type
                 data);//Data

    memset(&tga_header, 0, sizeof(tga_header));
    tga_header.imagetype = 2;
    tga_header.width = (short)this->parentWidget()->width();
    tga_header.height = (short)this->parentWidget()->height();
    tga_header.bpp = 24;
    FILE * f_out = fopen("screenshot.tga", "wb");
    fwrite(&tga_header, sizeof(tga_header), 1, f_out);
    fwrite(data, data_size, 1, f_out);
    fclose(f_out);
    delete [] data;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); //unbind the FBO
#endif
}

void Globe::testfbo()
{

    QVector<unsigned char> rainbow;
    rainbow.append(0xff);
    rainbow.append(0xff);
    rainbow.append(0xff); /* White */

    rainbow.append(0x7f);
    rainbow.append(0x00);
    rainbow.append(0x7f); /* Violet */

    rainbow.append(0xbf);
    rainbow.append(0x00);
    rainbow.append(0xbf); /* Indigo */

    rainbow.append(0x00);
    rainbow.append(0x00);
    rainbow.append(0xff); /* Blue */

    rainbow.append(0x00);
    rainbow.append(0xff);
    rainbow.append(0x00); /* Green */

    rainbow.append(0xff);
    rainbow.append(0xff);
    rainbow.append(0x00); /* Yellow */

    rainbow.append(0xff);
    rainbow.append(0x7f);
    rainbow.append(0x00); /* Orange */

    rainbow.append(0x7f);
    rainbow.append(0x7f);
    rainbow.append(0x7f); /* Grey */

    QVector<GLfloat> positions;
    positions.append(-0.9f);
    positions.append(0.5f);  //0.0

    positions.append(-0.75f);
    positions.append(0.5f);  //0.125

    positions.append(-0.5f);
    positions.append(0.5f);  //0.250

    positions.append(-0.25f);
    positions.append(0.5f);  //0.375

    positions.append(0.0f);
    positions.append(0.5f);  //0.500

    positions.append(0.25f);
    positions.append(0.5f);  //0.625

    positions.append(0.5f);
    positions.append(0.5f);  //0.750

    positions.append(0.75f);
    positions.append(0.5f);  //0.875

    positions.append(0.9f);
    positions.append(0.5f);  //1.0


    QVector<GLfloat> texpositions;
    texpositions.append(0.0f);
    texpositions.append(0.125f);
    texpositions.append(0.250f);
    texpositions.append(0.375f);
    texpositions.append(0.500f);
    texpositions.append(0.625f);
    texpositions.append(0.750f);
    texpositions.append(0.875f);
    texpositions.append(1.0f);

    texturewriter->setupBuffers(positions, texpositions, rainbow);
    opts.fbo_changed = true;

}

void Globe::paintGL()
{

    //testfbo();

    if(opts.fbo_changed)
    {
        texturewriter->render();
        opts.fbo_changed = false;
    }

    if(opts.texture_changed)
    {
        delete textureearth;
        QPixmap pm = *(imageptrs->pmOut);
        textureearth = new QOpenGLTexture(pm.toImage().mirrored(true, false));
        textureearth->setMinificationFilter(QOpenGLTexture::Linear);
        textureearth->setMagnificationFilter(QOpenGLTexture::Linear);
        textureearth->setWrapMode(QOpenGLTexture::ClampToEdge);
        opts.texture_changed = false;
    }

    QPainter painter(this);
    painter.setPen(QColor(255,255,255));
    if(!painter.isActive())
        painter.begin(this);
    painter.beginNativePainting();

    makeCurrent();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glEnable(GL_DEPTH_TEST);

    QQuaternion quat = this->trackBall.rotation();

    // Calculate model view transformation
    QMatrix4x4 modelview;
    modelview.translate(0.0, 0.0, distance);
    modelview.rotate(quat);

    QMatrix4x4 rotmatrix;
    rotmatrix.rotate(quat);

    skybox->render(projection, rotmatrix);

    QVector3D sunPosition;

    geodetic_t sungeo;
    showSunPosition( &sungeo );
    LonLat2PointRad(sungeo.lat, sungeo.lon, &sunPosition, 100.0f);

    double first_julian, last_julian;

    if(!segs->getShowAllSegments())
    {
        if (opts.buttonMetop && segs->seglmetop->NbrOfSegments() > 0)
        {
            segs->seglmetop->GetFirstLastVisible(&first_julian, &last_julian);
            segs->seglmetop->CalculateSunPosition(first_julian, last_julian, &sunPosition);
        }
        if (opts.buttonNoaa && segs->seglnoaa->NbrOfSegments() > 0)
        {
            segs->seglnoaa->GetFirstLastVisible(&first_julian, &last_julian);
            segs->seglnoaa->CalculateSunPosition(first_julian, last_julian, &sunPosition);
        }
        if (opts.buttonHRP && segs->seglhrp->NbrOfSegments() > 0)
        {
            segs->seglhrp->GetFirstLastVisible(&first_julian, &last_julian);
            segs->seglhrp->CalculateSunPosition(first_julian, last_julian, &sunPosition);
        }
        if (opts.buttonGAC && segs->seglgac->NbrOfSegments() > 0)
        {
            segs->seglgac->GetFirstLastVisible(&first_julian, &last_julian);
            segs->seglgac->CalculateSunPosition(first_julian, last_julian, &sunPosition);
        }
        if (opts.buttonVIIRS && segs->seglviirs->NbrOfSegments() > 0)
        {
            segs->seglviirs->GetFirstLastVisible(&first_julian, &last_julian);
            segs->seglviirs->CalculateSunPosition(first_julian, last_julian, &sunPosition);
        }
    }

    glEnable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if(opts.textureOn)
    {
            textureearth->bind(0);
            geometries->render(projection, modelview, true, sunPosition.toVector4D(), opts.bPhongModel);
            textureearth->release();
    }

    if(bSatellites)
        satgl->render(projection, distance, quat);

    if(bBorders)
        gshhsdata->render(projection, modelview, bBorders);

    //soc->render(projection, modelview);

    if(opts.stationnameOn)
        renderStations(projection, modelview, QColor(255, 255, 0));

    QMatrix4x4 modelsun;
    modelsun = modelview;
    modelsun.translate(sunPosition);
    modelsun.scale(0.5f);

    octa->render(projection, modelsun, QColor(255, 255, 0));
    sun->rendersun(projection, modelsun, QColor(255, 255, 0));

    segmentgl->render(projection, distance, quat, this->width(), this->height());

    if(opts.mapextentlamberton)
    {
        projextends->renderLCC(projection, distance, quat);
    }

    if(opts.mapextentperspectiveon)
    {
        projextends->renderGVP(projection, distance, quat);
    }



    glDisable(GL_DEPTH_TEST);

    painter.endNativePainting();

    // Winvec
//    painter.save();
//    if (opts.buttonMetop && segs->seglmetop->NbrOfSegments() > 0)
//        segs->seglmetop->ShowWinvec(&painter, distance, modelview );
//    if (opts.buttonNoaa && segs->seglnoaa->NbrOfSegments() > 0)
//        segs->seglnoaa->ShowWinvec(&painter, distance, modelview );
//    painter.restore();

    QString framesPerSecond;
    if (const int elapsed = m_time.elapsed()) {
        framesPerSecond.setNum(m_frames /(elapsed / 1000.0), 'f', 2);
        painter.setPen(Qt::green);
        painter.drawText(this->width() - 200, 40, framesPerSecond + " paintGL calls / s");
    }

    QFont serifFont("Times", 10, QFont::Bold);
    painter.setPen(Qt::green);
    painter.setFont(serifFont);

    //AVHR_xxx_1B_M01_20130701051903Z_20130701052203Z_N_O_20130701054640Z
    //012345678901234567890123456789012345678901234567890123456789
    if (opts.buttonMetop && segs->seglmetop->NbrOfSegmentsSelected() > 0)
    {
        painter.drawText(10, this->height() - 40, "Last selected segment :");
        QString segdate = QString("%1-%2-%3 %4:%5:%6").arg(segmentnameselected.mid(16, 4)).arg(segmentnameselected.mid(20, 2)).arg(segmentnameselected.mid(22, 2))
                .arg(segmentnameselected.mid(24, 2)).arg(segmentnameselected.mid(26, 2)).arg(segmentnameselected.mid(28, 2));

        if(segmentnameselected.mid(0,15) == "AVHR_xxx_1B_M02")  // Metop-A
            painter.drawText(10, this->height() - 20, "Metop-A " + segdate);
        else if(segmentnameselected.mid(0,15) == "AVHR_xxx_1B_M01") // Metop-B
            painter.drawText(10, this->height() - 20, "Metop-B " + segdate);
    }
    //avhrr_20131111_011500_noaa19.hrp.bz2
    //012345678901234567890123456789012345678901234567890123456789
    else if (opts.buttonNoaa && segs->seglnoaa->NbrOfSegmentsSelected() > 0)
    {
        painter.drawText(10, this->height() - 40, "Last selected segment :");
        QString segdate = QString("%1-%2-%3 %4:%5:%6").arg(segmentnameselected.mid(6, 4)).arg(segmentnameselected.mid(10, 2)).arg(segmentnameselected.mid(12, 2))
                .arg(segmentnameselected.mid(15, 2)).arg(segmentnameselected.mid(17, 2)).arg(segmentnameselected.mid(19, 2));

        if(segmentnameselected.mid(0,5) == "avhrr" && segmentnameselected.mid(22,6) == "noaa19")
            painter.drawText(10, this->height() - 20, "NOAA 19 " + segdate);
    }
    //AVHR_HRP_00_M02_20130701060200Z_20130701060300Z_N_O_20130701061314Z
    else if (opts.buttonHRP && segs->seglhrp->NbrOfSegmentsSelected() > 0)
    {
        painter.drawText(10, this->height() - 40, "Last selected segment :");
        QString segdate = QString("%1-%2-%3 %4:%5:%6").arg(segmentnameselected.mid(16, 4)).arg(segmentnameselected.mid(20, 2)).arg(segmentnameselected.mid(22, 2))
                .arg(segmentnameselected.mid(24, 2)).arg(segmentnameselected.mid(26, 2)).arg(segmentnameselected.mid(28, 2));

        if(segmentnameselected.mid(0,15) == "AVHR_HRP_00_M02")  // Metop-A
            painter.drawText(10, this->height() - 20, "Metop-A " + segdate);
        else if(segmentnameselected.mid(0,15) == "AVHR_HRP_00_M01") // Metop-B
            painter.drawText(10, this->height() - 20, "Metop-B " + segdate);

    }
    //AVHR_GAC_1B_N19_20130701041003Z_20130701041303Z_N_O_20130701054958Z
    else if (opts.buttonGAC && segs->seglgac->NbrOfSegmentsSelected() > 0)
    {
        painter.drawText(10, this->height() - 40, "Last selected segment :");
        QString segdate = QString("%1-%2-%3 %4:%5:%6").arg(segmentnameselected.mid(16, 4)).arg(segmentnameselected.mid(20, 2)).arg(segmentnameselected.mid(22, 2))
                .arg(segmentnameselected.mid(24, 2)).arg(segmentnameselected.mid(26, 2)).arg(segmentnameselected.mid(28, 2));

        if(segmentnameselected.mid(0,15) == "AVHR_GAC_1B_N19")
            painter.drawText(10, this->height() - 20, "NOAA 19 " + segdate);

    }
    //SVMC_npp_d20141117_t0837599_e0839241_b15833_c20141117084501709131_eum_ops
    //012345678901234567890123456789012345678901234567890123456789
    else if (opts.buttonVIIRS && segs->seglviirs->NbrOfSegmentsSelected() > 0)
    {
        painter.drawText(10, this->height() - 40, "Last selected segment :");
        QString segdate = QString("%1-%2-%3 %4:%5:%6").arg(segmentnameselected.mid(10, 4)).arg(segmentnameselected.mid(14, 2)).arg(segmentnameselected.mid(16, 2))
                .arg(segmentnameselected.mid(20, 2)).arg(segmentnameselected.mid(22, 2)).arg(segmentnameselected.mid(24, 2));

        if(segmentnameselected.mid(0,8) == "SVMC_npp")
            painter.drawText(10, this->height() - 20, "SUOMI NPP " + segdate);
    }


    if (bSegmentNames && opts.buttonMetop && segs->seglmetop->NbrOfSegments() > 0)
    {
        drawSegmentNames(&painter, modelview, eSegmentType::SEG_METOP, segs->seglmetop->GetSegmentlistptr());
    }
    else if (bSegmentNames && opts.buttonNoaa && segs->seglnoaa->NbrOfSegments() > 0)
    {
        drawSegmentNames(&painter, modelview, eSegmentType::SEG_NOAA, segs->seglnoaa->GetSegmentlistptr());
    }
    else if (bSegmentNames && opts.buttonHRP && segs->seglhrp->NbrOfSegments() > 0)
    {
        drawSegmentNames(&painter, modelview, eSegmentType::SEG_HRP, segs->seglhrp->GetSegmentlistptr());
    }
    else if (bSegmentNames && opts.buttonGAC && segs->seglgac->NbrOfSegments() > 0)
    {
        drawSegmentNames(&painter, modelview, eSegmentType::SEG_GAC, segs->seglgac->GetSegmentlistptr());
    }
    else if (bSegmentNames && opts.buttonVIIRS && segs->seglviirs->NbrOfSegments() > 0)
    {
        drawSegmentNames(&painter, modelview, eSegmentType::SEG_VIIRS, segs->seglviirs->GetSegmentlistptr());
    }



    painter.setPen(Qt::yellow);
    if(bSatellites)
        drawSatelliteNames(&painter, modelview);

    painter.setPen(Qt::green);
    if(opts.stationnameOn)
        drawStationNames(&painter, modelview);


    drawInstructions(&painter);

    painter.end();

    if (!(m_frames % 100)) {
        m_time.start();
        m_frames = 0;
    }
    ++m_frames;

    update();

}

//void Globe::NewFBO()
//{
//    delete imageptrs->fbo;
//    delete imageptrs->pmOriginal;

//    QImage qim(opts.backgroundimage3D);

//    if (opts.graytextureOn)
//    {
//        imageptrs->pmOriginal = new QPixmap(qim.width(), qim.height());
//        imageptrs->pmOriginal->fill(Qt::gray);
//    }
//    else
//        imageptrs->pmOriginal = new QPixmap(QPixmap::fromImage(qim));

//    QOpenGLFramebufferObjectFormat format;
//    format.setSamples(0);
//    format.setAttachment(QOpenGLFramebufferObject::NoAttachment);

//    imageptrs->fbo = new QOpenGLFramebufferObject( qim.width(), qim.height(),format);
//    qDebug() << QString("new fbo created ---> image width = %1 height = %2").arg(qim.width()).arg(qim.height());

////    QPainter fbo_painter(imageptrs->fbo);
////    fbo_painter.drawPixmap(0, 0, *(imageptrs->pmOriginal));
////    fbo_painter.end();

//    glBindTexture(GL_TEXTURE_2D, imageptrs->fbo->texture());
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//    glEnable(GL_CULL_FACE);

//}


void Globe::initFrameBuffer()
{
#ifndef OPENGL21
    //create fboA and attach texture A to it
    glGenFramebuffers(1, &imageptrs->fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, imageptrs->fboId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, textureearth->textureId(), 0);
    if(checkFramebufferStatus())
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    else
        qDebug() << "Error creating framebuffer";
#endif
}


void Globe::drawSatelliteNames(QPainter *painter, QMatrix4x4 modelview)
{
    float mvmatrix[16], projmatrix[16];
    QVector2D win;

    QMatrix3x3 norm = modelview.normalMatrix();

    QGenericMatrix<1, 3,float> vector;
    QGenericMatrix<1, 3,float> Normal;
    QMatrix4x4 MVP;
    MVP = projection * modelview;

    float *ptr = modelview.data();
    for(int i = 0; i < 16; i++)
        mvmatrix[i] = *(ptr + i);

    ptr = projection.data();
    for(int i = 0; i < 16; i++)
        projmatrix[i] = *(ptr + i);

    QList<Satellite>::iterator sat = sats->GetSatlist()->begin();

    while ( sat != sats->GetSatlist()->end() )
    {
        if( (*sat).active  == true)
        {
            QVector3D pos = (*sat).position.normalized();

            win = glhProjectf( pos, mvmatrix, projmatrix, this->width(), this->height());

            vector(0, 0) = (*sat).position.x();
            vector(1, 0) = (*sat).position.y();
            vector(2, 0) = (*sat).position.z();

            Normal = norm * vector;
            QVector3D vecnormal;
            vecnormal.setX(Normal(0, 0));
            vecnormal.setY(Normal(1, 0));
            vecnormal.setZ(Normal(2, 0));
            vecnormal.normalize();

            float angle = 1 / MVP(3, 3);
            float result = QVector3D::dotProduct(vecnormal, QVector3D(0.0f, 0.0f, 1.0f));
            // qDebug() << "angle = " << angle;
            if(result > angle)
            {
                (*sat).winsatpos.setX(win.x());
                (*sat).winsatpos.setY(this->height() - win.y());
                painter->drawText(win.x(), this->height() - win.y(), (*sat).sat_name);
            }
            else
                (*sat).winsatpos = QVector2D(9999.0,9999.0);
        }
        ++sat;
    }
}

void Globe::drawStationNames(QPainter *painter, QMatrix4x4 modelview)
{
    float mvmatrix[16], projmatrix[16];
    QVector2D win;

    QMatrix3x3 norm = modelview.normalMatrix();

    QGenericMatrix<1, 3,float> vector;
    QGenericMatrix<1, 3,float> Normal;
    QMatrix4x4 MVP;
    MVP = projection * modelview;

    float *ptr = modelview.data();
    for(int i = 0; i < 16; i++)
        mvmatrix[i] = *(ptr + i);

    ptr = projection.data();
    for(int i = 0; i < 16; i++)
        projmatrix[i] = *(ptr + i);


    //QMatrix4x4 modelviewstation;

    QVector3D pos;
    QStringList::Iterator itsname = opts.stationlistname.begin();
    QStringList::Iterator itslon = opts.stationlistlon.begin();
    QStringList::Iterator itslat = opts.stationlistlat.begin();


    while( itsname != opts.stationlistname.end() )
    {

        //qDebug() << QString("name = %1").arg(*itsname);
        LonLat2Point((*itslat).toFloat(), (*itslon).toFloat(), &pos, 1.01f);

        win = glhProjectf( pos, mvmatrix, projmatrix, this->width(), this->height());

        vector(0, 0) = pos.x();
        vector(1, 0) = pos.y();
        vector(2, 0) = pos.z();

        Normal = norm * vector;
        QVector3D vecnormal;
        vecnormal.setX(Normal(0, 0));
        vecnormal.setY(Normal(1, 0));
        vecnormal.setZ(Normal(2, 0));
        vecnormal.normalize();

        float angle = 1 / MVP(3, 3);
        float result = QVector3D::dotProduct(vecnormal, QVector3D(0.0f, 0.0f, 1.0f));
        // qDebug() << "angle = " << angle;
        if(result > angle)
            painter->drawText(win.x(), this->height() - win.y(), *itsname);

        ++itsname;
        ++itslon;
        ++itslat;
    }

}


void Globe::drawSegmentNames(QPainter *painter, QMatrix4x4 modelview, eSegmentType seg, QList<Segment *> *segptr)
{
    //0123456789012345678901234567890123456789012345678901234567890123456
    //AVHR_GAC_1B_N19_20140127061603Z_20140127061903Z_N_O_20140127080206Z
    //avhrr_20140127_002400_noaa19
    //AVHR_xxx_1B_M02_20140126075503Z_20140126075803Z_N_O_20140126091947Z
    //SVMC_npp_d20141117_t0837599_e0839241_b15833_c20141117084501709131_eum_ops
    //012345678901234567890123456789012345678901234567890123456789
    //avhrr_20131111_011500_noaa19.hrp.bz2
    //012345678901234567890123456789012345678901234567890123456789

    float mvmatrix[16], projmatrix[16];
    QVector2D win;

    QMatrix3x3 norm = modelview.normalMatrix();

    QGenericMatrix<1, 3,float> vector;
    QGenericMatrix<1, 3,float> Normal;
    QMatrix4x4 MVP;
    MVP = projection * modelview;

    float *ptr = modelview.data();
    for(int i = 0; i < 16; i++)
        mvmatrix[i] = *(ptr + i);

    ptr = projection.data();
    for(int i = 0; i < 16; i++)
        projmatrix[i] = *(ptr + i);


    QList<Segment*>::iterator segit = segptr->begin();
    QString renderout;
    QVector3D vec;
    while ( segit != segptr->end() )
    {
        if ((*segit)->segmentshow)
        {
            vec = (*segit)->vec1 * 1.005;
            win = glhProjectf( vec, mvmatrix, projmatrix, this->width(), this->height());

            vector(0, 0) = vec.x();
            vector(1, 0) = vec.y();
            vector(2, 0) = vec.z();

            Normal = norm * vector;
            QVector3D vecnormal;
            vecnormal.setX(Normal(0, 0));
            vecnormal.setY(Normal(1, 0));
            vecnormal.setZ(Normal(2, 0));
            vecnormal.normalize();

            float angle = 1 / MVP(3, 3);
            float result = QVector3D::dotProduct(vecnormal, QVector3D(0.0f, 0.0f, 1.0f));
            // qDebug() << "angle = " << angle;
            if(result > angle)
            {

                if(seg == eSegmentType::SEG_METOP)
                {
                    renderout = QString("%1 %2:%3").arg((*segit)->fileInfo.fileName().mid(12, 3)).arg((*segit)->fileInfo.fileName().mid(24, 2)).arg((*segit)->fileInfo.fileName().mid(26, 2));
                }
                else if(seg == eSegmentType::SEG_NOAA)
                {
                    renderout = QString("%1 %2:%3").arg("N19").arg((*segit)->fileInfo.fileName().mid(15, 2)).arg((*segit)->fileInfo.fileName().mid(17, 2));
                }
                else if(seg == eSegmentType::SEG_GAC)
                {
                    renderout = QString("%1 %2:%3").arg((*segit)->fileInfo.fileName().mid(12, 3)).arg((*segit)->fileInfo.fileName().mid(24, 2)).arg((*segit)->fileInfo.fileName().mid(26, 2));
                }
                else if(seg == eSegmentType::SEG_HRP)
                {
                    renderout = QString("%1 %2:%3").arg((*segit)->fileInfo.fileName().mid(12, 3)).arg((*segit)->fileInfo.fileName().mid(24, 2)).arg((*segit)->fileInfo.fileName().mid(26, 2));
                }
                else if(seg == eSegmentType::SEG_VIIRS)
                {
                    renderout = QString("%1 %2:%3").arg((*segit)->fileInfo.fileName().mid(5, 3)).arg((*segit)->fileInfo.fileName().mid(20, 2)).arg((*segit)->fileInfo.fileName().mid(22, 2));
                }
                else
                {
                    renderout = QString("Wrong segmentlist");
                }
                painter->drawText(win.x(), this->height() - win.y(), renderout);
            }


        }
        ++segit;
    }

}

QVector2D Globe::glhProjectf(QVector3D obj, float *modelview, float *projection, int width, int height)
  {
      //Transformation vectors
      float fTempo[8];
      //Modelview transform
      fTempo[0]=modelview[0]*obj.x()+modelview[4]*obj.y()+modelview[8]*obj.z()+modelview[12];  //w is always 1
      fTempo[1]=modelview[1]*obj.x()+modelview[5]*obj.y()+modelview[9]*obj.z()+modelview[13];
      fTempo[2]=modelview[2]*obj.x()+modelview[6]*obj.y()+modelview[10]*obj.z()+modelview[14];
      fTempo[3]=modelview[3]*obj.x()+modelview[7]*obj.y()+modelview[11]*obj.z()+modelview[15];
      //Projection transform, the final row of projection matrix is always [0 0 -1 0]
      //so we optimize for that.
      fTempo[4]=projection[0]*fTempo[0]+projection[4]*fTempo[1]+projection[8]*fTempo[2]+projection[12]*fTempo[3];
      fTempo[5]=projection[1]*fTempo[0]+projection[5]*fTempo[1]+projection[9]*fTempo[2]+projection[13]*fTempo[3];
      fTempo[6]=projection[2]*fTempo[0]+projection[6]*fTempo[1]+projection[10]*fTempo[2]+projection[14]*fTempo[3];
      fTempo[7]=-fTempo[2];
      //The result normalizes between -1 and 1
//      if(fTempo[7]==0.0)	//The w value
//         return 0;
      fTempo[7]=1.0/fTempo[7];
      //Perspective division
      fTempo[4]*=fTempo[7];
      fTempo[5]*=fTempo[7];
      fTempo[6]*=fTempo[7];
      //Window coordinates
      //Map x, y to range 0-1
      QVector2D win((fTempo[4]*0.5+0.5)*width, (fTempo[5]*0.5+0.5)*height);
      //This is only correct when glDepthRange(0.0, 1.0)
      //windowCoordinate[2]=(1.0+fTempo[6])*0.5;	//Between 0 and 1
      return win;
  }


bool Globe::CreateAndLlinkShader(QOpenGLShaderProgram *program, QString vertexshader, QString fragmentshader)
{
    if(!program->create())
        return false;

    // Compile vertex shader
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexshader))
    {
        qDebug() << program->log();
        return false;
    }

    // Compile fragment shader
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentshader))
    {
        qDebug() << program->log();
        return false;
    }


    // Link shader pipeline
    if (!program->link())
    {
        qDebug() << program->log();
        return false;
    }

    return true;

}

void Globe::initShaders()
{


    if(!CreateAndLlinkShader(&programearthnobump, ":/shader/earthshader.vert", ":/shader/earthshader.frag"))
    {
        qDebug() << programearthnobump.log();
    }

    if(!CreateAndLlinkShader(&programgshhs, ":/shader/gshhs.vert", ":/shader/gshhs.frag"))
    {
        qDebug() << programgshhs.log();
    }


    if(!CreateAndLlinkShader(&programskybox, ":/shader/skybox.vert", ":/shader/skybox.frag"))
    {
        qDebug() << programskybox.log();
    }

    if(!CreateAndLlinkShader(&programdraw, ":/shader/draw.vert", ":/shader/draw.frag"))
    {
        qDebug() << programdraw.log();
    }

    if(!CreateAndLlinkShader(&programsatgl, ":/shader/sats.vert", ":/shader/sats.frag"))
    {
        qDebug() << programsatgl.log();
    }

    if(!CreateAndLlinkShader(&programtexturewriter, ":/shader/texturewriter.vert", ":/shader/texturewriter.frag"))
    {
        qDebug() << programtexturewriter.log();
    }

}

void Globe::initTextures()
{

    textureearth = new QOpenGLTexture(imageptrs->pmOriginal->toImage().mirrored(true, false));
    // Set nearest filtering mode for texture minification
    textureearth->setMinificationFilter(QOpenGLTexture::Linear);
    // Set bilinear filtering mode for texture magnification
    textureearth->setMagnificationFilter(QOpenGLTexture::Linear);
    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    textureearth->setWrapMode(QOpenGLTexture::ClampToEdge);


    static unsigned char rainbow_image[] =
    {
//         0x3f, 0x00, 0x3f, 0xff, /* Dark Violet (for 8 colors…) */

         0xff, 0xff, 0xff, 0xff,
         0x7f, 0x00, 0x7f, 0xff, /* Violet */
         0xbf, 0x00, 0xbf, 0xff, /* Indigo */
         0x00, 0x00, 0xff, 0xff, /* Blue */
         0x00, 0xff, 0x00, 0xff, /* Green */
         0xff, 0xff, 0x00, 0xff, /* Yellow */
         0xff, 0x7f, 0x00, 0xff, /* Orange */
         0xff, 0x00, 0x00, 0xff /* Red */
     };

//      glNewList(RainbowTexture = glGenLists(1), GL_COMPILE);
//        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexImage1D(GL_TEXTURE_1D, 0, 3, 8, 0, GL_RGB, GL_UNSIGNED_BYTE,
//                     roygbiv_image);
//      glEndList();
//    textureline = new QOpenGLTexture(QOpenGLTexture::Target1D);
//    textureline->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt32_RGBA8, rainbow_image);
//    textureline->setMinificationFilter(QOpenGLTexture::Linear);
//    textureline->setMagnificationFilter(QOpenGLTexture::Linear);

//    static const GLubyte toon_tex_data[] =
//    {
//    0x44, 0x00, 0x00, 0x00,
//    0x88, 0x00, 0x00, 0x00,
//    0xCC, 0x00, 0x00, 0x00,
//    0xFF, 0x00, 0x00, 0x00
//    };

}


void Globe::showSunPosition( geodetic_t *sungeo )
{
  double jul_utc;

  /* Zero vector for initializations */
  vector_t zero_vector = {0,0,0,0};

  /* Solar ECI position vector  */
  vector_t solar_vector = zero_vector;

  struct tm utc;

  UTC_Calendar_Now(&utc);
  jul_utc = Julian_Date(&utc);

  //qDebug() << QString("%1, %2, %3, %4, %5, %6, %7, %8, %9").arg(utc.tm_hour).arg(utc.tm_isdst).arg(utc.tm_mday).arg(utc.tm_min).arg(utc.tm_mon);

  /* Calculate solar position and satellite eclipse depth */
  Calculate_Solar_Position(jul_utc, &solar_vector);

  /* Calculate Sun's Lat North, Lon East and Alt. */
  Calculate_LatLonAlt(jul_utc, &solar_vector, sungeo);


}



void Globe::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

}


QSize Globe::sizeHint() const
{
    return QSize(400, 400);
}


void Globe::Render3DGeo(SegmentListGeostationary::eGeoSatellite sat)
{

    if(futureRender3DGeo.isRunning())
        return;

    if (segs->seglmeteosat->getKindofImage() != "HRV" && segs->seglmeteosat->getKindofImage() != "HRV Color")
    {
        futureRender3DGeo = QtConcurrent::run(Render3DColorTexture, this, sat);
        watcher.setFuture(futureRender3DGeo);
    }
}

void Globe::slotRender3DGeoFinished()
{
    qDebug() << "=======> futureRender3DGeo is finished";
}

void Globe::Render3DGeoSegment(SegmentListGeostationary::eGeoSatellite sat)
{

    qDebug() << "Globe::Render3DGeoSegment(SegmentListMeteosat::eGeoSatellite sat)";

    g_mutex.lock();

    for (int i = 0; i < imageptrs->ptrimageGeostationary->height(); i++)
        Render3DGeoSegmentLine( i, sat);

    g_mutex.unlock();

    QApplication::restoreOverrideCursor();
    opts.texture_changed = true;
    emit renderingglobefinished(true);
}


void Globe::Render3DGeoSegmentLine(int heightinimage, SegmentListGeostationary::eGeoSatellite sat)
{

    QRgb *scanl;
    QRgb rgbval;
    int map_x, map_y;
    double lon_rad, lat_rad;
    double lon_deg, lat_deg;
    int x, y;

    pixgeoConversion pixconv;

    //g_mutex.lock();

    QPainter fb_painter(imageptrs->pmOut);

    scanl = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(heightinimage);

    fb_painter.setPen( Qt::black );
    fb_painter.setBrush( Qt::NoBrush );

    for (int pix = 0 ; pix < imageptrs->ptrimageGeostationary->width(); pix+=1)
    {
        rgbval = scanl[pix];

        if(sat == SegmentListGeostationary::MET_10)
        {
            if(pixconv.pixcoord2geocoord(segs->seglmeteosat->geosatlon, pix, heightinimage, COFF_NONHRV, LOFF_NONHRV, CFAC_NONHRV, LFAC_NONHRV, &lat_deg, &lon_deg) == 0)
            {
                sphericalToPixel(lon_deg*PI/180.0, lat_deg*PI/180.0, x, y, imageptrs->pmOriginal->width(), imageptrs->pmOriginal->height());
                fb_painter.setPen(rgbval);
                fb_painter.drawPoint(x, y);
            }
        }
        else if(sat == SegmentListGeostationary::MET_9)
        {
            if(pixconv.pixcoord2geocoord(segs->seglmeteosatrss->geosatlon, pix, heightinimage, COFF_NONHRV, LOFF_NONHRV, CFAC_NONHRV, LFAC_NONHRV, &lat_deg, &lon_deg) == 0)
            {
                sphericalToPixel(lon_deg*PI/180.0, lat_deg*PI/180.0, x, y, imageptrs->pmOriginal->width(), imageptrs->pmOriginal->height());
                fb_painter.setPen(rgbval);
                fb_painter.drawPoint(x, y);
            }
        }
/*        else if(sat == SegmentListGeostationary::ELECTRO_N1)
        {
            if(pixconv.pixcoord2geocoord(segs->seglelectro->geosatlon, pix, heightinimage, COFF_NONHRV_ELECTRO, LOFF_NONHRV_ELECTRO, CFAC_NONHRV_ELECTRO, LFAC_NONHRV_ELECTRO, &lat_deg, &lon_deg) == 0)
            {
                sphericalToPixel(lon_deg*PI/180.0, lat_deg*PI/180.0, x, y, imageptrs->pmOriginal->width(), imageptrs->pmOriginal->height());
                fb_painter.setPen(rgbval);
                fb_painter.drawPoint(x, y);
            }
        }
*/
        else if(sat == SegmentListGeostationary::MET_7)
        {
            if(pixconv.pixcoord2geocoord(segs->seglmet7->geosatlon, pix, heightinimage, COFF_NONHRV_MET7, LOFF_NONHRV_MET7, CFAC_NONHRV_MET7, LFAC_NONHRV_MET7, &lat_deg, &lon_deg) == 0)
            {
                sphericalToPixel(lon_deg*PI/180.0, lat_deg*PI/180.0, x, y, imageptrs->pmOriginal->width(), imageptrs->pmOriginal->height());
                fb_painter.setPen(rgbval);
                fb_painter.drawPoint(x, y);
            }
        }
        else if(sat == SegmentListGeostationary::GOES_13 || sat == SegmentListGeostationary::GOES_15)
        {
            if(pixconv.pixcoord2geocoord((sat == SegmentListGeostationary::GOES_13 ? segs->seglgoes13dc3->geosatlon : segs->seglgoes15dc3->geosatlon), pix, heightinimage, COFF_NONHRV_GOES, LOFF_NONHRV_GOES, CFAC_NONHRV_GOES, LFAC_NONHRV_GOES, &lat_deg, &lon_deg) == 0)
            {
                sphericalToPixel(lon_deg*PI/180.0, lat_deg*PI/180.0, x, y, imageptrs->pmOriginal->width(), imageptrs->pmOriginal->height());
                fb_painter.setPen(rgbval);
                fb_painter.drawPoint(x, y);
            }
        }
        else if(sat == SegmentListGeostationary::MTSAT)
        {
            if(pixconv.pixcoord2geocoord(segs->seglmtsatdc3->geosatlon, pix, heightinimage, COFF_NONHRV_MTSAT, LOFF_NONHRV_MTSAT, CFAC_NONHRV_MTSAT, LFAC_NONHRV_MTSAT, &lat_deg, &lon_deg) == 0)
            {
                sphericalToPixel(lon_deg*PI/180.0, lat_deg*PI/180.0, x, y, imageptrs->pmOriginal->width(), imageptrs->pmOriginal->height());
                fb_painter.setPen(rgbval);
                fb_painter.drawPoint(x, y);
            }
        }
        else if(sat == SegmentListGeostationary::FY2E)
        {
            if(pixconv.pixcoord2geocoord(segs->seglfy2e->geosatlon, pix, heightinimage, COFF_NONHRV_FENGYUN, LOFF_NONHRV_FENGYUN, CFAC_NONHRV_FENGYUN, LFAC_NONHRV_FENGYUN, &lat_deg, &lon_deg) == 0)
            {
                sphericalToPixel(lon_deg*PI/180.0, lat_deg*PI/180.0, x, y, imageptrs->pmOriginal->width(), imageptrs->pmOriginal->height());
                fb_painter.setPen(rgbval);
                fb_painter.drawPoint(x, y);
            }
        }
        else if(sat == SegmentListGeostationary::FY2G)
        {
            if(pixconv.pixcoord2geocoord(segs->seglfy2g->geosatlon, pix, heightinimage, COFF_NONHRV_FENGYUN, LOFF_NONHRV_FENGYUN, CFAC_NONHRV_FENGYUN, LFAC_NONHRV_FENGYUN, &lat_deg, &lon_deg) == 0)
            {
                sphericalToPixel(lon_deg*PI/180.0, lat_deg*PI/180.0, x, y, imageptrs->pmOriginal->width(), imageptrs->pmOriginal->height());
                fb_painter.setPen(rgbval);
                fb_painter.drawPoint(x, y);
            }
        }

    }

    fb_painter.end();
   // g_mutex.unlock();

}

void Globe::Render3DGeoSegmentFBO(SegmentListGeostationary::eGeoSatellite sat)
{

    qDebug() << "Globe::Render3DGeoSegmentFBO(SegmentListMeteosat::eGeoSatellite sat)";

    //g_mutex.lock();

    for (int i = 0; i < imageptrs->ptrimageGeostationary->height(); i++)
        Render3DGeoSegmentLineFBO( i, sat);

    //g_mutex.unlock();

    QApplication::restoreOverrideCursor();
    emit renderingglobefinished(true);
    qDebug() << "einde Globe::Render3DGeoSegmentFBO(SegmentListMeteosat::eGeoSatellite sat)";

}

void Globe::Render3DGeoSegmentLineFBO(int heightinimage, SegmentListGeostationary::eGeoSatellite sat)
{

    QRgb *scanl;
    QRgb rgbval;
    double lon_deg, lat_deg;
    int x, y;

    pixgeoConversion pixconv;

    //qDebug() << "Globe::Render3DGeoSegmentLineFBO " << heightinimage;

    QVector<GLfloat> positions;
    QVector<GLfloat> texpositions;
    QVector<unsigned char> rainbow;


    scanl = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(heightinimage);

    for (int pix = 0 ; pix < imageptrs->ptrimageGeostationary->width(); pix+=1)
    {
        rgbval = scanl[pix];


        if(sat == SegmentListGeostationary::MET_10)
        {
            if(pixconv.pixcoord2geocoord(segs->seglmeteosat->geosatlon, pix, heightinimage, COFF_NONHRV, LOFF_NONHRV, CFAC_NONHRV, LFAC_NONHRV, &lat_deg, &lon_deg) == 0)
            {
                sphericalToPixel(lon_deg*PI/180.0, lat_deg*PI/180.0, x, y, imageptrs->pmOriginal->width(), imageptrs->pmOriginal->height());
                positions.append((float)(x / (imageptrs->pmOriginal->width()/2) - 1));
                positions.append((float)(y / (imageptrs->pmOriginal->height()/2) - 1));
                texpositions.append((float)(pix /imageptrs->pmOriginal->width()));
                rainbow.append(qRed(rgbval));
                rainbow.append(qGreen(rgbval));
                rainbow.append(qBlue(rgbval));
            }
        }
        else if(sat == SegmentListGeostationary::MET_9)
        {
            if(pixconv.pixcoord2geocoord(segs->seglmeteosatrss->geosatlon, pix, heightinimage, COFF_NONHRV, LOFF_NONHRV, CFAC_NONHRV, LFAC_NONHRV, &lat_deg, &lon_deg) == 0)
            {
                sphericalToPixel(lon_deg*PI/180.0, lat_deg*PI/180.0, x, y, imageptrs->pmOriginal->width(), imageptrs->pmOriginal->height());
                positions.append((float)(x / (imageptrs->pmOriginal->width()/2) - 1));
                positions.append((float)(y / (imageptrs->pmOriginal->height()/2) - 1));
                texpositions.append((float)(pix /imageptrs->pmOriginal->width()));
                rainbow.append(qRed(rgbval));
                rainbow.append(qGreen(rgbval));
                rainbow.append(qBlue(rgbval));
            }
        }
/*        else if(sat == SegmentListGeostationary::ELECTRO_N1)
        {
            if(pixconv.pixcoord2geocoord(segs->seglelectro->geosatlon, pix, heightinimage, COFF_NONHRV_ELECTRO, LOFF_NONHRV_ELECTRO, CFAC_NONHRV_ELECTRO, LFAC_NONHRV_ELECTRO, &lat_deg, &lon_deg) == 0)
            {
                sphericalToPixel(lon_deg*PI/180.0, lat_deg*PI/180.0, x, y, imageptrs->pmOriginal->width(), imageptrs->pmOriginal->height());
                positions.append((float)(x / (imageptrs->pmOriginal->width()/2) - 1));
                positions.append((float)(y / (imageptrs->pmOriginal->height()/2) - 1));
                texpositions.append((float)(pix /imageptrs->pmOriginal->width()));
                rainbow.append(qRed(rgbval));
                rainbow.append(qGreen(rgbval));
                rainbow.append(qBlue(rgbval));
            }
        }
*/
        else if(sat == SegmentListGeostationary::MET_7)
        {
            if(pixconv.pixcoord2geocoord(segs->seglmet7->geosatlon, pix, heightinimage, COFF_NONHRV_MET7, LOFF_NONHRV_MET7, CFAC_NONHRV_MET7, LFAC_NONHRV_MET7, &lat_deg, &lon_deg) == 0)
            {
                sphericalToPixel(lon_deg*PI/180.0, lat_deg*PI/180.0, x, y, imageptrs->pmOriginal->width(), imageptrs->pmOriginal->height());
                positions.append((float)(x / (imageptrs->pmOriginal->width()/2) - 1));
                positions.append((float)(y / (imageptrs->pmOriginal->height()/2) - 1));
                texpositions.append((float)(pix /imageptrs->pmOriginal->width()));
                rainbow.append(qRed(rgbval));
                rainbow.append(qGreen(rgbval));
                rainbow.append(qBlue(rgbval));
            }
        }
        else if(sat == SegmentListGeostationary::GOES_13 || sat == SegmentListGeostationary::GOES_15)
        {
            if(pixconv.pixcoord2geocoord((sat == SegmentListGeostationary::GOES_13 ? segs->seglgoes13dc3->geosatlon : segs->seglgoes15dc3->geosatlon), pix, heightinimage, COFF_NONHRV_GOES, LOFF_NONHRV_GOES, CFAC_NONHRV_GOES, LFAC_NONHRV_GOES, &lat_deg, &lon_deg) == 0)
            {
                sphericalToPixel(lon_deg*PI/180.0, lat_deg*PI/180.0, x, y, imageptrs->pmOriginal->width(), imageptrs->pmOriginal->height());
                positions.append((float)(x / (imageptrs->pmOriginal->width()/2) - 1));
                positions.append((float)(y / (imageptrs->pmOriginal->height()/2) - 1));
                texpositions.append((float)(pix /imageptrs->pmOriginal->width()));
                rainbow.append(qRed(rgbval));
                rainbow.append(qGreen(rgbval));
                rainbow.append(qBlue(rgbval));
            }
        }
        else if(sat == SegmentListGeostationary::MTSAT)
        {
            if(pixconv.pixcoord2geocoord(segs->seglmtsatdc3->geosatlon, pix, heightinimage, COFF_NONHRV_MTSAT, LOFF_NONHRV_MTSAT, CFAC_NONHRV_MTSAT, LFAC_NONHRV_MTSAT, &lat_deg, &lon_deg) == 0)
            {
                sphericalToPixel(lon_deg*PI/180.0, lat_deg*PI/180.0, x, y, imageptrs->pmOriginal->width(), imageptrs->pmOriginal->height());
                positions.append((float)(x / (imageptrs->pmOriginal->width()/2) - 1));
                positions.append((float)(y / (imageptrs->pmOriginal->height()/2) - 1));
                texpositions.append((float)(pix /imageptrs->pmOriginal->width()));
                rainbow.append(qRed(rgbval));
                rainbow.append(qGreen(rgbval));
                rainbow.append(qBlue(rgbval));
            }
        }

    }

    if(heightinimage == 500)
    {
        qDebug() << QString("position = %1  texpositions = %2  rainbow = %3").arg(positions.size()/2).arg(texpositions.size()).arg(rainbow.size()/3);
    }

    //texturewriter->setupBuffers(positions, texpositions, rainbow);

    //opts.fbo_changed = true;

}


void Globe::drawInstructions(QPainter *painter)
{
    QString text = " view distance = 0000000 km";
    QFontMetrics metrics = QFontMetrics(font());
    int border = qMax(4, metrics.leading());

    QRect rect = metrics.boundingRect(200, 200, width() - 2*border, int(height()*0.125),
                                      Qt::AlignLeft | Qt::TextWordWrap, text);
    //painter->setRenderHint(QPainter::TextAntialiasing);

    painter->drawText(10, border,
                      rect.width(), rect.height(),
                      Qt::AlignLeft | Qt::TextWordWrap, QString(" view distance =  %1 km").arg( - distance * (XKMPER_WGS72/2) - (XKMPER_WGS72/2) ));


}

// Key handler
void Globe::keyPressEvent(QKeyEvent *event)
{

    qDebug() << "Globe::keyPressEvent(QKeyEvent *event)";
    QMatrix mat;
    mat.reset();
    mat.rotate(180.0);

    switch (event->key())
    {
    case Qt::Key_F1:
        setWindowState(windowState() ^ Qt::WindowFullScreen); // Toggle fullscreen on F1
        break;
    case Qt::Key_G:
        if (opts.textureOn)
            opts.textureOn = false;
        else
            opts.textureOn =true;
        break;
    case Qt::Key_N:
        if (opts.stationnameOn)
            opts.stationnameOn = false;
        else
            opts.stationnameOn = true;
        break;
    case Qt::Key_B:
        toggleBorder();
        break;
    case Qt::Key_S:
        toggleSatellites();
        break;
    case Qt::Key_V:
        break;
    case Qt::Key_L:
        togglePhong();
        break;
    case Qt::Key_T:
        toggleSegmentNames();
        break;
    case Qt::Key_Escape:
        close();
        break;

    default:
        QOpenGLWidget::keyPressEvent(event); // Let base class handle the other keys
    }
}

void Globe::toggleBorder()
{
    if(bBorders)
        bBorders = false;
    else
        bBorders = true;
}


void Globe::toggleSatellites()
{
    if(bSatellites)
        bSatellites = false;
    else
        bSatellites = true;
}

void Globe::togglePhong()
{
    if(opts.bPhongModel)
        opts.bPhongModel = false;
    else
        opts.bPhongModel = true;
}

void Globe::toggleSegmentNames()
{
    if(bSegmentNames)
        bSegmentNames = false;
    else
        bSegmentNames = true;
}


bool Globe::checkFramebufferStatus()
{
#ifndef OPENGL21
    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status)
    {
    case GL_FRAMEBUFFER_COMPLETE:
        qDebug() << "Framebuffer complete.";
        return true;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        qDebug() << "[ERROR] Framebuffer incomplete: Attachment is NOT complete.";
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        qDebug() << "[ERROR] Framebuffer incomplete: No image is attached to FBO.";
        return false;

//    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
//        std::cout << "[ERROR] Framebuffer incomplete: Attached images have different dimensions." << std::endl;
//        return false;

//    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
//        std::cout << "[ERROR] Framebuffer incomplete: Color attached images have different internal formats." << std::endl;
//        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        qDebug() << "[ERROR] Framebuffer incomplete: Draw buffer.";
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        qDebug() << "[ERROR] Framebuffer incomplete: Read buffer.";
        return false;

    case GL_FRAMEBUFFER_UNSUPPORTED:
        qDebug() << "[ERROR] Framebuffer incomplete: Unsupported by FBO implementation.";
        return false;

    default:
        qDebug() << "[ERROR] Framebuffer incomplete: Unknown error.";
        return false;
    }
#endif
}

