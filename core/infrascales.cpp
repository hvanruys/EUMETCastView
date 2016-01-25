#include "infrascales.h"
#include <QBoxLayout>
#include <QMouseEvent>
#include <QDebug>


InfraScales::InfraScales(QWidget *parent) : QWidget(parent)
{

    infrawidget = new InfraWidget(this);

    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;

    verticalLayoutWidget = new QWidget(this);
    verticalLayout = new QVBoxLayout(verticalLayoutWidget);
    verticalLayout->setSpacing(0);
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(0);

    labelleft = new QLabel(this);
    labelleft->setMinimumSize(QSize(10, 0));
    labelleft->setMaximumSize(QSize(10, 16777215));

    horizontalLayout->addWidget(labelleft);

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(1);
    sizePolicy.setHeightForWidth(infrawidget->sizePolicy().hasHeightForWidth());

    infrawidget->setSizePolicy(sizePolicy);
    infrawidget->setMinimumSize(QSize(0, 50));

    horizontalLayout->addWidget(infrawidget);

    labelright = new QLabel(this);
    labelright->setMinimumSize(QSize(10, 0));
    labelright->setMaximumSize(QSize(10, 16777215));

    horizontalLayout->addWidget(labelright);

    verticalLayout->addLayout(horizontalLayout);

    scalewidget = new QWidget(this);
    scalewidget->setMinimumSize(QSize(0, 30));
    scalewidget->setMaximumSize(QSize(16777215, 30));

    verticalLayout->addWidget(scalewidget);
    this->setLayout(verticalLayout);

    grablowcursor = false;
    grabhighcursor = false;
    lowlimit = 0.0;
    highlimit = 1.0;
    lowlimittemp = 0.0;
    highlimittemp = 0.0;

    inverse = false;

}

QImage InfraScales::getScalesImage(int width)
{
    QImage im( width, 80, QImage::Format_RGB32 );
    im.fill(Qt::gray);

    QPainter painter(&im);
    infrawidget->drawInfraWidget(&painter, 10, 0, width-20, 50);

    QPointF low, high;

    low.setX(10 + lowlimit * (width-20));
    low.setY(50);
    high.setX(10 + highlimit * (width-20));
    high.setY(50);

    this->drawInfraScales(&painter, low, high);
    painter.end();
    return im;

}

void InfraScales::initializeLowHigh()
{
    lowlimit = 0.0;
    highlimit = 1.0;
    infrawidget->initializeLowHigh();

    ptlowcursor.setX(this->labelleft->width() + lowlimit * this->infrawidget->width());
    ptlowcursor.setY(this->infrawidget->height());
    pthighcursor.setX(this->labelleft->width() + highlimit * this->infrawidget->width());
    pthighcursor.setY(this->infrawidget->height());

    this->update();
}

void InfraScales::setInverse(bool inverse)
{
    lowlimit = 0.0;
    highlimit = 1.0;

    this->inverse = inverse;

    infrawidget->setInverse(inverse);

}

bool InfraScales::getInverse()
{
    return infrawidget->getInverse();
}

void InfraScales::mousePressEvent( QMouseEvent *e )
{
    if(e->button() == Qt::RightButton)
        return;
    qDebug() << QString("scales mousepress x = %1 y = %2").arg(e->x()).arg(e->y());

    //if(e->x() <= this->labelleft->width())
    //    return;
    //if(e->x() > this->labelleft->width() + this->infrawidget->width())
    //    return;

    float valx = ((float)e->x()-this->labelleft->width())/(float)this->infrawidget->width();

    valx = valx < 0 ? 0 : valx;
    valx = valx > 1.0 ? 1.0 : valx;

    if(valx > lowlimit - 0.02 && valx <= lowlimit + 0.02 )
    {
        grablowcursor = true;
    }
    if(valx > highlimit - 0.02 && valx <= highlimit + 0.02 )
    {
        grabhighcursor = true;
    }
    update();
}

void InfraScales::mouseReleaseEvent( QMouseEvent *e )
{

    QRgb *row;
    float btemp;

    int height = imageptrs->ptrimageProjection->height();
    int width = imageptrs->ptrimageProjection->width();

    if(imageptrs->ptrProjectionInfra.isNull())
        return;

    int intlow = (int)((lowlimit) * 255.0);
    int inthigh = (int)((highlimit) * 255.0);
    //float delta = highlimittemp - lowlimittemp; //this->maxbtemp - this->minbtemp;
    qDebug() << QString("scales release x = %1 y = %2 lowlimit = %3 highlimit = %4").arg(e->x()).arg(e->y()).arg(lowlimit).arg(highlimit);
    qDebug() << QString("scales release x = %1 y = %2 intlow = %3 inthigh = %4").arg(e->x()).arg(e->y()).arg(intlow).arg(inthigh);

    for(int y = 0; y < height; y++)
    {
        row = (QRgb*)imageptrs->ptrimageProjection->scanLine(y);

        for(int x = 0; x < width; x++)
        {
            quint8 greyval = imageptrs->ptrProjectionInfra[y * width + x];

            if(imageptrs->ptrProjectionBrightnessTemp.isNull())
                return;
            btemp = imageptrs->ptrProjectionBrightnessTemp[y * width + x];
            if(btemp > 0)
            {
                float fval = (btemp - minprojectiontemp)/deltaprojectiontemp ;

                if(lowlimit <= fval && fval <= highlimit)
                {
                    row[x] = getColor(fval).rgb();
                }
                else
                {
                    row[x] = qRgb(greyval, greyval, greyval);
                }
            }

        }
    }

    grablowcursor = false;
    grabhighcursor = false;

    emit repaintprojectionimage();
    update();
}

void InfraScales::mouseMoveEvent( QMouseEvent *e )
{
    if(e->x() < this->labelleft->width())
        return;
    if(e->x() > this->labelleft->width() + this->infrawidget->width())
        return;

    float valx = ((float)e->x()-this->labelleft->width())/(float)this->infrawidget->width();
    if(valx > 1.0 || valx < 0)
        return;
    //QColor col = getColor(valx);

    //qDebug() << QString("scales mousemove x = %1 y = %2 lowlimit = %3 highlimit = %4").arg(e->x()).arg(e->y()).arg(lowlimit).arg(highlimit);
    if(grablowcursor)
    {
        if(valx < highlimit)
        {
            lowlimit = valx;
            ptlowcursor.setX(e->x());
            ptlowcursor.setY(this->infrawidget->height());
            infrawidget->setlowcursor(lowlimit);
        }
    }
    else if(grabhighcursor)
    {
        if(valx > lowlimit)
        {
            highlimit = valx;
            pthighcursor.setX(e->x());
            pthighcursor.setY(this->infrawidget->height());
            infrawidget->sethighcursor(highlimit);
        }
    }

    update();
}

void InfraScales::resizeEvent( QResizeEvent *e )
{

    ptlowcursor.setX(this->labelleft->width() + lowlimit * this->infrawidget->width());
    ptlowcursor.setY(this->infrawidget->height());
    pthighcursor.setX(this->labelleft->width() + highlimit * this->infrawidget->width());
    pthighcursor.setY(this->infrawidget->height());

}

void InfraScales::hideEvent(QHideEvent * event)
{
    qDebug() << QString("HideEvent ishidden = %1").arg(this->isHidden());
}

void InfraScales::setMinMaxTemp(float mintemp, float maxtemp)
{
    lowlimittemp = mintemp;
    highlimittemp = maxtemp;
    minprojectiontemp = mintemp;
    maxprojectiontemp = maxtemp;
    deltaprojectiontemp = maxprojectiontemp - minprojectiontemp;

}

QColor InfraScales::getColor(float value)
{
    return infrawidget->getColor(value);
}

QColor InfraScales::getColorLow()
{
    return infrawidget->getColor(this->lowlimit);
}

QColor InfraScales::getColorHigh()
{
    return infrawidget->getColor(this->highlimit);
}

void InfraScales::paintEvent( QPaintEvent * )
{
    QPainter painter(this);
    drawInfraScales(&painter, ptlowcursor, pthighcursor);
    painter.end();
}

void InfraScales::drawInfraScales(QPainter *paint, QPointF lowcursor, QPointF highcursor)
{
    QPen pen(Qt::black);
    pen.setWidth(1);
    paint->setPen(pen);


    //painter.drawLine(5, this->height()-10, this->width()-5, this->height()-10 );
    //painter.drawText(5, this->height()-5, QString("%1K°").arg(lowlimittemp, 4, 'f', 1));
    //painter.drawText(this->width()- 100, this->height()-5, QString("%1K°").arg(highlimittemp, 4, 'f', 1));
    //painter.drawLine(this->labelleft->width(), this->infrawidget->height(), this->labelleft->width(), this->infrawidget->height() + 5 );
    //painter.drawLine(this->labelleft->width() + this->infrawidget->width() - 1, this->infrawidget->height(), this->labelleft->width() + this->infrawidget->width() - 1, this->infrawidget->height() + 5 );

    lowcursor.setY(this->infrawidget->height());
    highcursor.setY(this->infrawidget->height());

    static const QPointF points[3] = {
        QPointF(0.0, 0.0),
        QPointF(-4.0, 9.0),
        QPointF(4.0, 9.0)
    };

    QPointF pointstranslatelow[3] = {
        points[0] + lowcursor,
        points[1] + lowcursor,
        points[2] + lowcursor
    };

    QPointF pointstranslatehigh[3] = {
        points[0] + highcursor,
        points[1] + highcursor,
        points[2] + highcursor
    };

    paint->setBrush(QBrush(Qt::SolidPattern));

    paint->drawConvexPolygon(pointstranslatelow, 3);
    paint->drawConvexPolygon(pointstranslatehigh, 3);

    drawMinMaxTemp(paint, lowcursor, highcursor);

    //painter.drawLine(0, this->height()/5.0, this->width(), this->height()/5.0 );
    //painter.drawLine(0, this->height()*2/5.0, this->width(), this->height()*2/5.0 );
    //painter.drawLine(0, this->height()*3/5.0, this->width(), this->height()*3/5.0 );
    //painter.drawLine(0, this->height()*4/5.0, this->width(), this->height()*4/5.0 );
    //painter.drawLine(0, this->height() * 4.5/5.0, this->width(), this->height() * 4.5/5.0 );
    //painter.drawLine(0, this->height()-1, this->width(), this->height()-1 );
}

void InfraScales::drawMinMaxTemp(QPainter *painter, QPointF lowcursor, QPointF highcursor)
{
    //float deltatemp = highlimittemp - lowlimittemp;
    QString mintemp = QString("%1K°").arg(lowlimittemp + lowlimit * deltaprojectiontemp, 4, 'f', 1);
    QString maxtemp = QString("%1K°").arg(highlimittemp + (highlimit-1) * deltaprojectiontemp, 4, 'f', 1);
    QFontMetrics metrics = QFontMetrics(font());
    int border = qMax(4, metrics.leading());

    QRect rectmin = metrics.boundingRect(mintemp);
    QRect rectmax = metrics.boundingRect(maxtemp);
    rectmin.setWidth(rectmin.width()+5);
    rectmax.setWidth(rectmax.width()+5);
    painter->setRenderHint(QPainter::TextAntialiasing);

//    painter->drawText(5, this->height()-rectmin.height(), rectmin.width(), rectmin.height(),
//                      Qt::AlignJustify, mintemp );
//    painter->drawText(this->width()- rectmax.width(), this->height()-rectmax.height(), rectmax.width(), rectmax.height(),
//                      Qt::AlignJustify, maxtemp );

    painter->drawText(lowcursor.x() - 10, this->height()-rectmin.height(), rectmin.width(), rectmin.height(),
                      Qt::AlignJustify, mintemp );
    painter->drawText(highcursor.x()-rectmax.width() + 10, this->height()-rectmax.height(), rectmax.width(), rectmax.height(),
                      Qt::AlignJustify, maxtemp );

}
