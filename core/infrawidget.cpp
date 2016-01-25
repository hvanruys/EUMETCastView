#include "infrawidget.h"
#include "colormaps.h"
#include "options.h"

#include <QDebug>
#include <QMouseEvent>

extern Options opts;

InfraWidget::InfraWidget(QWidget *parent) : QWidget(parent)
{
    //this->setMouseTracking(true);

    lowlimit = 0.0;
    highlimit = 1.0;
    grablowlimit = false;
    grabhighlimit = false;
    inverse = false;

}

void InfraWidget::paintEvent( QPaintEvent * )
{
    QPainter painter(this);
    drawInfraWidget(&painter, 0, 0, this->width(), this->height());
    painter.end();
}

void InfraWidget::drawInfraWidget(QPainter *paint, int x, int y, int width, int height)
{
    QPen     pen(Qt::black);
    pen.setWidth(1);
    paint->setPen(pen);

//    qDebug() << QString("InfraWidget::drawInfraWidget(QPainter *paint, int x, int y, int width, int height)");
//    qDebug() << QString("magma = %1").arg(opts.colormapMagma);
//    qDebug() << QString("inferno = %1").arg(opts.colormapInferno);
//    qDebug() << QString("plasma = %1").arg(opts.colormapPlasma);
//    qDebug() << QString("viridis = %1").arg(opts.colormapViridis);

    //painter.drawRect(this->geometry());

    QLinearGradient gradientcol;

    gradientcol = QLinearGradient(0, 0, width, 0);

    QColor mycol;

    for(int i = 0; i < 256; i++ )
    {
        if(opts.colormapMagma)
            mycol.setRgbF(magma_data[i][0], magma_data[i][1], magma_data[i][2]);
        else if(opts.colormapInferno)
            mycol.setRgbF(inferno_data[i][0], inferno_data[i][1], inferno_data[i][2]);
        else if(opts.colormapPlasma)
            mycol.setRgbF(plasma_data[i][0], plasma_data[i][1], plasma_data[i][2]);
        else if(opts.colormapViridis)
            mycol.setRgbF(viridis_data[i][0], viridis_data[i][1], viridis_data[i][2]);
        gradientcol.setColorAt((qreal)i/255.0, mycol);
    }


    QLinearGradient gradientbw;
    gradientbw = QLinearGradient(0, 0, width, 0);

    QPointF gradbegin(x, y);
    QPointF gradend(width + x, y + (height/2.0));
    QPointF deltay(0, height/2.0);

    paint->setBrush(QBrush(gradientcol));
    paint->drawRect(QRectF(gradbegin, gradend));

    if(inverse)
    {
        gradientbw.setColorAt(0, QColor(255,255,255)); //white
        gradientbw.setColorAt(1, QColor(0,0,0));       //black
    }
    else
    {
        gradientbw.setColorAt(0, QColor(0,0,0));       //black
        gradientbw.setColorAt(1, QColor(255,255,255)); //white
    }

    paint->setBrush(QBrush(gradientbw));
    paint->drawRect(QRectF(gradbegin + deltay, gradend + deltay));

    paint->setBrush(Qt::NoBrush);
    paint->drawRect(QRectF(x, y, width-1, height-1));

    //painter.drawLine(0, 0, this->width(), 0 );
    //painter.drawLine(0, this->height()/5.0, this->width(), this->height()/5.0 );
    //painter.drawLine(0, this->height()*2/5.0, this->width(), this->height()*2/5.0 );
    //painter.drawLine(0, this->height()*3/5.0, this->width(), this->height()*3/5.0 );
    //painter.drawLine(0, this->height()*4/5.0, this->width(), this->height()*4/5.0 );
    //painter.drawLine(0, this->height() * 4.5/5.0, this->width(), this->height() * 4.5/5.0 );
    //painter.drawLine(0, this->height()-1, this->width(), this->height()-1 );

    paint->drawLine(x + lowlimit * width, 0, x + lowlimit * width, height);
    paint->drawLine(x + highlimit * width, 0, x + highlimit * width, height);

}


void InfraWidget::setlowcursor(float low)
{
    lowlimit = low;
}

void InfraWidget::sethighcursor(float high)
{
    highlimit = high;
}

QColor InfraWidget::getColor(float value)
{
    QColor col;
    int index = qRound(value * 255.0);
    index = index > 255 ? 255 : index;
    if(opts.colormapMagma)
        col.setRgbF(magma_data[index][0], magma_data[index][1], magma_data[index][2] );
    else if(opts.colormapInferno)
        col.setRgbF(inferno_data[index][0], inferno_data[index][1], inferno_data[index][2] );
    else if(opts.colormapPlasma)
        col.setRgbF(plasma_data[index][0], plasma_data[index][1], plasma_data[index][2] );
    else if(opts.colormapViridis)
        col.setRgbF(viridis_data[index][0], viridis_data[index][1], viridis_data[index][2] );
    return(col);
}

//QColor InfraWidget::getColor(double value)
//{
//    if(value > 1.0 || value < 0.0)
//        return QColor(0,0,0);

//    //Asume colorlist.count()>1 and value=[0,1]
//    int colormapcount = 256;
//    double stepbase = 1.0/(colormapcount-1);
//    int interval=colormapcount-1;      // to fix 1 <= 0.99999999;

//    for (int i=1; i<colormapcount;i++) // remove begin and end
//    {
//        if(value<=i*stepbase )
//        {
//            interval=i;
//            break;
//        }
//    }

//    double percentage = (value-stepbase*(interval-1))/stepbase;
//    QColor start;
//    start.setRgbF(plasma_data[interval][0], plasma_data[interval][1], plasma_data[interval][2]);
//    QColor end;
//    end.setRgbF(plasma_data[interval-1][0], plasma_data[interval-1][1], plasma_data[interval-1][2]);
//    QColor color(interpolate(start,end,percentage));
//    return color;
//}

//QColor InfraWidget::interpolate(QColor start,QColor end,double ratio)
//{
//    int r = (int)(ratio*start.red() + (1-ratio)*end.red());
//    int g = (int)(ratio*start.green() + (1-ratio)*end.green());
//    int b = (int)(ratio*start.blue() + (1-ratio)*end.blue());
//    return QColor::fromRgb(r,g,b);
//}
