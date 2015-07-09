#include "satgl.h"
#include "options.h"

extern Options opts;

SatGL::SatGL(QOpenGLShaderProgram *prog, SatelliteList *satlist, AVHRRSatellite *seglist)
{
    sats = satlist;
    segs = seglist;

    tdiff = 0;
    program = prog;

    initializeOpenGLFunctions();

    program->bind();

    vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    positionsBuf.create();
    positionsBuf.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    positionsBuf.bind();

//    QVector<GLfloat> positions;

//    QList<Satellite>::iterator sat = sats->GetSatlist()->begin();

    nbrActiveSats = 0;

//    while ( sat != sats->GetSatlist()->end() )
//    {
//        if( (*sat).active  == true)
//        {
//            QVector3D pos, posssp;
//            (*sat).GetSatellitePosition(pos, posssp, alt);
//            positions.append(0.0f);
//            positions.append(0.0f);
//            positions.append(0.0f);
//            positions.append(pos.x());
//            positions.append(pos.y());
//            positions.append(pos.z());
//            qDebug() << (*sat).sat_name;
//            nbrActiveSats++;
//        }
//        ++sat;
//    }

//    positionsBuf.allocate(positions.data(), positions.size() * sizeof(GLfloat));
//    qDebug() << "SatGL : satposBufsize " << positionsBuf.size() << "  nbr of vertices = " << positionsBuf.size() / (3 * sizeof(GLfloat));
//    qDebug() << "SatGL : nbrActiveSats " << nbrActiveSats;
//    qDebug() << "SatGL : nbrActiveSats = " << nbrActiveSats << "  positions.size = " << positions.size() << " positionsBuf.size() = " << positionsBuf.size();

    vertexPosition = program->attributeLocation("VertexPosition");
    program->enableAttributeArray(vertexPosition);
    program->setAttributeBuffer(vertexPosition, GL_FLOAT, 0, 3);

    vaotrail.create();
    QOpenGLVertexArrayObject::Binder vaoBinder1(&vaotrail);

    positionsTrail.create();
    positionsTrail.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    positionsTrail.bind();
    positionsTrail.allocate(tdiff * 2 * 3 *sizeof(GLfloat));
    program->enableAttributeArray(vertexPosition);
    program->setAttributeBuffer(vertexPosition, GL_FLOAT, 0, 3);

    octa = new Octahedron(program);
    horizon = new SatHorizon(program);

}


void SatGL::render(QMatrix4x4 projection, float distance, QQuaternion quat )
{
    QMatrix4x4 modelview;
    modelview.translate(0.0, 0.0, distance);
    modelview.rotate(quat);


    QVector<GLfloat> positions;

    QList<Satellite>::iterator sat = sats->GetSatlist()->begin();

    int nbrSats = 0;
    int nbrVertices;

    QMatrix4x4 modelocta;
    QColor col(255, 0, 0);
    float alt;
    while ( sat != sats->GetSatlist()->end() )
    {
        if( (*sat).active  == true)
        {
            QVector3D pos, posnorm;
            (*sat).GetSatellitePosition(pos, posnorm, alt);
            positions.append(0.0f);
            positions.append(0.0f);
            positions.append(0.0f);
            positions.append(pos.x());
            positions.append(pos.y());
            positions.append(pos.z());
            modelocta = modelview;
            modelocta.translate(posnorm.x(), posnorm.y(), posnorm.z());
            modelocta.scale(0.005f);

            octa->render(projection, modelocta, col);
            QColor horizoncolour(opts.sathorizoncolor);
            horizon->render(projection, distance, quat, posnorm, alt, horizoncolour);

            nbrSats++;
        }
        ++sat;
    }

    positionsBuf.bind();
    if(nbrSats != nbrActiveSats)
    {
        nbrActiveSats = nbrSats;

        positionsBuf.allocate(positions.data(), positions.size() * sizeof(GLfloat));

        qDebug() << "positionsBuf.size() != nbrActiveSats * 2 * 3 * sizeof(GLfloat)";
        qDebug() << "nbrActiveSats = " << nbrActiveSats << " positionsBuf.size() = " << positionsBuf.size();
    }
    else
    {
        positionsBuf.write(0, positions.data(), positions.size() * sizeof(GLfloat));
    }

    nbrVertices = positionsBuf.size() / (3 * sizeof(GLfloat));

    positionsBuf.release();

    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    program->bind();

    program->setUniformValue("MVP", projection * modelview);
    QMatrix3x3 norm = modelview.normalMatrix();
    program->setUniformValue("NormalMatrix", norm);

    QColor rendercolor(255, 0, 0);
    program->setUniformValue("outcolor", QVector4D(rendercolor.redF(), rendercolor.greenF(), rendercolor.blueF(), 1.0f));

    glDrawArrays(GL_LINES, 0, nbrVertices);

    sat = sats->GetSatlist()->begin();
    while ( sat != sats->GetSatlist()->end() )
    {
        if( (*sat).active  == true)
        {
            if( (*sat).GetCatalogueNbr() == sats->GetSelectedSat() )
                RenderTrail(&(*sat), projection, distance, quat, true);
            else
                RenderTrail(&(*sat), projection, distance, quat, false);
        }
        ++sat;
    }

    //    QMatrix4x4 mod;
    //    mod.setToIdentity();
    //    mod.translate(0.0, 0.0, distance);
    //    mod.rotate(quat);

    //    modelocta = mod;
    //    modelocta.translate(0.0, 1.0, 0.0);
    //    modelocta.scale(0.009f);
    //    octa->render(projection, modelocta, col);

    //    modelocta = mod;
    //    modelocta.translate(0.0, 0.0, 1.0);
    //    modelocta.scale(0.009f);
    //    octa->render(projection, modelocta, col);

    //    modelocta = mod;
    //    modelocta.translate(1.0, 0.0, 0.0);
    //    modelocta.scale(0.009f);
    //    octa->render(projection, modelocta, col);


}

void SatGL::RenderTrail(Satellite *sat, QMatrix4x4 projection, float distance, QQuaternion quat, bool trackon) // QMatrix4x4 modelview, bool trackon)
{
    QVector<GLfloat> postrail;

    double
            tsince,            // Time since epoch (in minutes)
            jul_epoch,         // Julian date of epoch
            jul_utc;           // Julian UTC date


    QSgp4Date nowutc = QSgp4Date::NowUTC();
    jul_utc = nowutc.Julian();
    jul_epoch = Julian_Date_of_Epoch(sat->GetEpoch());

    QMatrix4x4 model;
    model.translate(0.0, 0.0, distance);
    model.rotate(quat);

    QMatrix4x4 modelview;
    modelview = model;
    QMatrix4x4 modelocta;
    QColor col(0,255,255);

    QEci qeci;
    QVector3D pos;
    double id;
    int nbrVertices = 0;

    tsince = (jul_utc - jul_epoch) * MIN_PER_DAY; // in minutes

    for( id = tsince - 5; id < tsince; id++ )
    {
        (*sat).qsgp4->getPosition(id, qeci);
        QGeodetic qgeo = qeci.ToGeo();
        LonLat2PointRad(qgeo.latitude, qgeo.longitude, &pos, 1.001f);
        if(id < tsince && id >= tsince - 5 )
        {
            modelocta = model;
            modelocta.translate(pos.x(), pos.y(), pos.z());
            modelocta.scale(0.004f);
            octa->render(projection, modelocta, col);
        }
    }


    if(trackon)
    {
        for( id = tsince - opts.realminutesshown + 1; id <= tsince + opts.realminutesshown; id++ )  // nbr of id's = 2 * opts.realminutesshown
        {
            (*sat).qsgp4->getPosition(id, qeci);
            QGeodetic qgeo = qeci.ToGeo();
            LonLat2PointRad(qgeo.latitude, qgeo.longitude, &pos, 1.001f);
            postrail.append(pos.x());
            postrail.append(pos.y());
            postrail.append(pos.z());
        }

        positionsTrail.bind();

        if(tdiff != opts.realminutesshown)
        {
            tdiff = opts.realminutesshown;
            positionsTrail.allocate(postrail.data(), postrail.size() * sizeof(GLfloat));

        }
        else
        {
            positionsTrail.write(0, postrail.data(), postrail.size() * sizeof(GLfloat));
        }

        nbrVertices = positionsTrail.size() / (3 * sizeof(GLfloat));

        positionsTrail.release();

        QOpenGLVertexArrayObject::Binder vaoBinder(&vaotrail);

        program->bind();

        program->setUniformValue("MVP", projection * modelview);
        QMatrix3x3 norm = modelview.normalMatrix();
        program->setUniformValue("NormalMatrix", norm);

        QColor rendercolor(opts.sattrackcolor);
        program->setUniformValue("outcolor", QVector4D(rendercolor.redF(), rendercolor.greenF(), rendercolor.blueF(), 1.0f));

        glDrawArrays(GL_LINE_STRIP, 0, nbrVertices);

    }
}




SatGL::~SatGL()
{
    vao.destroy();
    positionsBuf.destroy();
    vaotrail.destroy();
    positionsTrail.destroy();
    delete octa;
}

