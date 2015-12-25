#ifndef INFRASCALES_H
#define INFRASCALES_H

#include <QWidget>
#include <QLabel>

#include "infrawidget.h"
#include "formimage.h"
extern SegmentImage *imageptrs;

class InfraScales : public QWidget
{
    Q_OBJECT
public:
    explicit InfraScales(QWidget *parent = 0);
    QColor getColor(double value);
    QColor getColorLow();
    QColor getColorHigh();
    void setMinMaxTemp(float mintemp, float maxtemp);
    void setInverse(bool inverse);
    bool getInverse();
    QColor infraLUT[256];
    void initializeLowHigh();

protected:
    void mousePressEvent( QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent( QMouseEvent *e);
    void paintEvent( QPaintEvent * );
    void resizeEvent(QResizeEvent * e);
    void hideEvent(QHideEvent * event);
signals:
    void repaintprojectionimage();
public slots:

private:
    void drawMinMaxTemp(QPainter *painter);

    InfraWidget *infrawidget;
    QWidget *scalewidget;
    QLabel *labelleft;
    QLabel *labelright;
    QPointF ptlowcursor;
    QPointF pthighcursor;
    bool grablowcursor;
    bool grabhighcursor;
    float lowlimit; // [0, 1]
    float highlimit; // [0, 1]
    float lowlimittemp;
    float highlimittemp;

    bool down;
};

#endif // INFRASCALES_H
