#include "projextentsgl.h"

#include "segmentimage.h"
#include "options.h"
#include "pixgeoconversion.h"

extern Options opts;
extern SegmentImage *imageptrs;

ProjExtentsGL::ProjExtentsGL(QOpenGLShaderProgram *prog)
{

    program = prog;

    initializeOpenGLFunctions();

    program->bind();

    vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    positionsBuf.create();
    positionsBuf.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    positionsBuf.bind();

    nbrOfVertices = 40;
    positionsBuf.allocate( nbrOfVertices * 3 * sizeof(GLfloat));

    vertexPosition = program->attributeLocation("VertexPosition");
    program->enableAttributeArray(vertexPosition);
    program->setAttributeBuffer(vertexPosition, GL_FLOAT, 0, 3);

    horizon = new SatHorizon(program);
    octa = new Octahedron(program);

}

void ProjExtentsGL::renderLCCtest(QMatrix4x4 projection, float dist, QQuaternion quat)
{
    QMatrix4x4 modelview;
    modelview.translate(0.0, 0.0, dist);
    modelview.rotate(quat);



    double lon_rad;
    double lat_rad;
    int x,y;

    QVector3D pos;
    QMatrix4x4 modelocta;

    QColor yel(Qt::yellow);
    QColor green(Qt::green);


//    for (int j = 0; j < cylequi->height(); j++)
//    {
//        for (int i = 0; i < cylequi->width(); i++)
//        {
//            if (imageptrs->lcc->map_inverse(i, j, lon_rad, lat_rad))
//            {
//                cylequi->sphericalToPixel(lon_rad, lat_rad, x, y );

//            }
//        }
//    }

    modelocta = modelview;
    if (imageptrs->lcc->map_inverse(0, 0, lon_rad, lat_rad))
    {
        LonLat2PointRad((float)lat_rad, (float)lon_rad, &pos, 1.001f);
        modelocta.translate(pos);
        modelocta.scale(0.01f);
        octa->render(projection, modelocta, yel);
    }
    modelocta = modelview;
    if (imageptrs->lcc->map_inverse(imageptrs->pmOut->width(), 0, lon_rad, lat_rad))
    {
        LonLat2PointRad((float)lat_rad, (float)lon_rad, &pos, 1.001f);
        modelocta.translate(pos);
        modelocta.scale(0.01f);
        octa->render(projection, modelocta, green);
    }
    modelocta = modelview;
    if (imageptrs->lcc->map_inverse(0, imageptrs->pmOut->height(), lon_rad, lat_rad))
    {
        LonLat2PointRad((float)lat_rad, (float)lon_rad, &pos, 1.001f);
        modelocta.translate(pos);
        modelocta.scale(0.01f);
        octa->render(projection, modelocta, yel);
    }
    modelocta = modelview;
    if (imageptrs->lcc->map_inverse(imageptrs->pmOut->width(), imageptrs->pmOriginal->height(), lon_rad, lat_rad))
    {
        LonLat2PointRad((float)lat_rad, (float)lon_rad, &pos, 1.001f);
        modelocta.translate(pos);
        modelocta.scale(0.01f);
        octa->render(projection, modelocta, yel);
    }


    //this->CalculateGreatCircleArc(&positions, (float)opts.mapextentsouth*PI/180, (float)opts.mapextentwest*PI/180, (float)opts.mapextentnorth*PI/180, (float)opts.mapextentwest*PI/180 );

//    modelocta = modelview;
//    LonLat2PointRad((float)opts.mapextentsouth*PI/180, (float)opts.mapextentwest*PI/180, &pos, 1.0f);
//    modelocta.translate(pos);
//    modelocta.scale(0.01f);
//    octa->render(projection, modelocta, yel);



}

void ProjExtentsGL::renderLCC(QMatrix4x4 projection, float dist, QQuaternion quat)
{
    QMatrix4x4 modelview;
    modelview.translate(0.0, 0.0, dist);
    modelview.rotate(quat);

    RenderContour(projection, modelview);
}

void ProjExtentsGL::renderGVP(QMatrix4x4 projection, float dist, QQuaternion quat)
{
    QVector3D pos;

    LonLat2PointRad(opts.mapgvplat * PI/180.0, opts.mapgvplon * PI/180.0, &pos, 1.001f);

    QColor col(opts.mapgvpextentcolor);

    horizon->render(projection, dist, quat, pos, 1 + opts.mapgvpheight/XKMPER_WGS72, col );
    horizon->render(projection, dist, quat, pos, 1 + 0.00005, col );
}

void ProjExtentsGL::RenderContour(QMatrix4x4 projection, QMatrix4x4 modelview)
{
    QVector3D vec;
    QVector3D pos;
    QVector<GLfloat> positions;

    this->CalculateGreatCircleArc(&positions, (float)opts.mapextentsouth*PI/180, (float)opts.mapextentwest*PI/180, (float)opts.mapextentnorth*PI/180, (float)opts.mapextentwest*PI/180 );
    this->CalculateSmallCircleArc(&positions, (float)opts.mapextentwest*PI/180, (float)opts.mapextenteast*PI/180, (float)opts.mapextentnorth*PI/180);
    this->CalculateGreatCircleArc(&positions, (float)opts.mapextentnorth*PI/180, (float)opts.mapextenteast*PI/180, (float)opts.mapextentsouth*PI/180, (float)opts.mapextenteast*PI/180 );
    this->CalculateSmallCircleArc(&positions, (float)opts.mapextenteast*PI/180, (float)opts.mapextentwest*PI/180, (float)opts.mapextentsouth*PI/180);

    positionsBuf.bind();
    positionsBuf.write(0, positions.data(), positions.size() * sizeof(GLfloat));
    positionsBuf.release();

    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    program->bind();
    program->setUniformValue("MVP", projection * modelview);
    QColor rendercolor(opts.maplccextentcolor);

    program->setUniformValue("outcolor", QVector4D(rendercolor.redF(), rendercolor.greenF(), rendercolor.blueF(), 1.0f));

    QMatrix3x3 norm = modelview.normalMatrix();
    program->setUniformValue("NormalMatrix", norm);

    glDrawArrays(GL_LINE_LOOP, 0, 40);
}

void ProjExtentsGL::CalculateGreatCircleArc(QVector<GLfloat> *positions, float lat_first, float lon_first, float lat_last, float lon_last)
{
    QVector3D pos;

    double sinlatdiff = sin((lat_first-lat_last)/2);
    double sinlondiff = sin((lon_first-lon_last)/2);

    double sinpsi = sqrt(sinlatdiff * sinlatdiff + cos(lat_first)*cos(lat_last)*sinlondiff * sinlondiff);
    double delta = 2*asin(sinpsi);

    int nDelta = 10;
    double deltax = delta / (nDelta - 1);
    double lonpos, latpos, dlon, tc;

    tc = fmod(atan2(sin(lon_first-lon_last)*cos(lat_last), cos(lat_first)*sin(lat_last)-sin(lat_first)*cos(lat_last)*cos(lon_first-lon_last)) , 2 * PI);
    for (int pix = 0 ; pix < nDelta; pix++)
    {
        latpos = asin(sin(lat_first)*cos(deltax * pix)+cos(lat_first)*sin(deltax * pix)*cos(tc));
        dlon=atan2(sin(tc)*sin(deltax * pix)*cos(lat_first),cos(deltax * pix)-sin(lat_first)*sin(latpos));
        lonpos=fmod( lon_first-dlon + PI,2*PI )-PI;

        LonLat2PointRad(latpos, lonpos, &pos, 1.001f);

        positions->append(pos.x());
        positions->append(pos.y());
        positions->append(pos.z());
    }
}

void ProjExtentsGL::CalculateSmallCircleArc(QVector<GLfloat> *positions, float lon_first, float lon_last, float lat)
{
    double lon1, lon2;
    double dlon,lonpos;
    QVector3D pos;

    int nDelta = 10;

    if (lon_first <= lon_last)
    {
        lon1 = lon_first;
        lon2 = lon_last;
    }
    else
    {
        lon2 = lon_first;
        lon1 = lon_last;
    }

    if (lon2-lon1 > PI)
        dlon = (2*PI - (lon2-lon1))/(nDelta-1);
    else
        dlon = (lon2 - lon1)/(nDelta-1);

    for (int pix = 0 ; pix < nDelta; pix++)
    {
        if (lon2-lon1 > PI)
            lonpos = lon2 + dlon*pix;
        else
            lonpos = lon1 + dlon*pix;
        if(lonpos > PI)
            lonpos = - (2*PI - lonpos);

        LonLat2PointRad(lat, lonpos, &pos, 1.001f);
        positions->append(pos.x());
        positions->append(pos.y());
        positions->append(pos.z());
    }

}

ProjExtentsGL::~ProjExtentsGL()
{

}

