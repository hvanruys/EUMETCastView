#include "infrawidget.h"
#include <QDebug>
#include <QMouseEvent>

InfraWidget::InfraWidget(QWidget *parent) : QWidget(parent)
{
    //this->setMouseTracking(true);
    down = false;

    colorlist.append(QColor(0, 0, 200));
    colorlist.append(QColor(200, 0, 200));
    colorlist.append(QColor(0, 255, 0));
    colorlist.append(QColor(255, 255, 0));
    colorlist.append(QColor(255, 0, 0));
    colorlist.append(QColor(150, 0, 0));

    lowlimit = 0.0;
    highlimit = 1.0;
    grablowlimit = false;
    grabhighlimit = false;
    inverse = false;

}

void InfraWidget::paintEvent( QPaintEvent * )
{
    QPainter painter(this);

    QPen     pen(Qt::black);
    pen.setWidth(1);
    painter.setPen(pen);

    //painter.drawRect(this->geometry());

    QLinearGradient gradientcol;
    QPointF gradbegin(0,0);
    QPointF gradend(this->width(),this->height()/2.0);
    QPointF deltay(0, this->height()/2.0);

    gradientcol = QLinearGradient(0, 0, this->width(), 0);
    QLinearGradient gradientbw;
    gradientbw = QLinearGradient(0, 0, this->width(), 0);

    gradientcol.setColorAt(0, colorlist.at(0));
    gradientcol.setColorAt(0.2, colorlist.at(1));
    gradientcol.setColorAt(0.4, colorlist.at(2));
    gradientcol.setColorAt(0.6, colorlist.at(3));
    gradientcol.setColorAt(0.8, colorlist.at(4));
    gradientcol.setColorAt(1, colorlist.at(5));

    painter.setBrush(QBrush(gradientcol));
    painter.drawRect(QRectF(gradbegin, gradend));

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

    painter.setBrush(QBrush(gradientbw));
    painter.drawRect(QRectF(gradbegin + deltay, gradend + deltay));

    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRectF(0, 0, this->width()-1, this->height()-1));

    //painter.drawLine(0, 0, this->width(), 0 );
    //painter.drawLine(0, this->height()/5.0, this->width(), this->height()/5.0 );
    //painter.drawLine(0, this->height()*2/5.0, this->width(), this->height()*2/5.0 );
    //painter.drawLine(0, this->height()*3/5.0, this->width(), this->height()*3/5.0 );
    //painter.drawLine(0, this->height()*4/5.0, this->width(), this->height()*4/5.0 );
    //painter.drawLine(0, this->height() * 4.5/5.0, this->width(), this->height() * 4.5/5.0 );
    //painter.drawLine(0, this->height()-1, this->width(), this->height()-1 );

    painter.drawLine(lowlimit * this->width(), 0, lowlimit * this->width(), this->height());
    int valx = highlimit * this->width();
    //valx = (valx == this->width() ? this->width()-1 : valx);
    painter.drawLine(valx, 0, valx, this->height());


}


void InfraWidget::setlowcursor(float low)
{
    lowlimit = low;
}

void InfraWidget::sethighcursor(float high)
{
    highlimit = high;
}

QColor InfraWidget::getColor(double value)
{
    if(value > 1.0 || value < 0.0)
        return QColor(0,0,0);

    //Asume colorlist.count()>1 and value=[0,1]
    double stepbase = 1.0/(colorlist.count()-1);
    int interval=colorlist.count()-1;      // to fix 1 <= 0.99999999;

    for (int i=1; i<colorlist.count();i++) // remove begin and end
    {
        if(value<=i*stepbase )
        {
            interval=i;
            break;
        }
    }

    double percentage = (value-stepbase*(interval-1))/stepbase;
    QColor color(interpolate(colorlist[interval],colorlist[interval-1],percentage));
    return color;
}

QColor InfraWidget::interpolate(QColor start,QColor end,double ratio)
{
    int r = (int)(ratio*start.red() + (1-ratio)*end.red());
    int g = (int)(ratio*start.green() + (1-ratio)*end.green());
    int b = (int)(ratio*start.blue() + (1-ratio)*end.blue());
    return QColor::fromRgb(r,g,b);
}
