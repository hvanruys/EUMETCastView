#ifndef IMAGESCROLLAREA_H
#define IMAGESCROLLAREA_H

#include <QScrollArea>
#include <QDebug>
#include "formimage.h"

class ImageScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit ImageScrollArea(QWidget *parent = 0);

protected:
    void scrollContentsBy ( int dx, int dy );
    void paintEvent(QPaintEvent *ev);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:
    FormImage *fm;



signals:

public slots:

};

#endif // IMAGESCROLLAREA_H
