#include "imagescrollarea.h"

ImageScrollArea::ImageScrollArea(QWidget *parent) :
    QScrollArea(parent)
{

    QObject *theparent = this->parent();

    fm = (FormImage *)theparent;

}

void ImageScrollArea::scrollContentsBy ( int dx, int dy )
{
    //qDebug() << QString("ImageScrollArea::scrollContentsBy ( %1, %2 )").arg(dx).arg(dy);
    QScrollArea::scrollContentsBy ( dx, dy );
}



void ImageScrollArea::paintEvent(QPaintEvent *ev)
{
    //qDebug() << QString("ImageScrollArea::paintEvent(QPaintEvent *ev)");
}

void ImageScrollArea::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        //qDebug() << QString("ImageScrollArea::mousepressevent");
        //fm->move = true;
    }
}

void ImageScrollArea::mouseMoveEvent(QMouseEvent *e)
{
    //qDebug() << QString("ImageScrollArea::mouseMoveEvent(QMouseEvent *e) e->pos() = %1 ; %2").arg(e->pos().x()).arg(e->pos().y());
}

void ImageScrollArea::mouseReleaseEvent(QMouseEvent *)
{
    //qDebug() << QString("ImageScrollArea::mousereleaseevent");
    //fm->move = false;
}

