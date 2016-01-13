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
    QColor getColor(float value);
    QColor getColorLow();
    QColor getColorHigh();
    void setMinMaxTemp(float mintemp, float maxtemp);
    void getMinMaxTemp(float *mintemp, float *maxtemp) { *mintemp = minprojectiontemp; *maxtemp = maxprojectiontemp; }
    void setInverse(bool inverse);
    bool getInverse();
    void initializeLowHigh();
    QImage getScalesImage(int width);
    void drawInfraScales(QPainter *paint, QPointF lowcursor, QPointF highcursor);


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
    void drawMinMaxTemp(QPainter *painter, QPointF lowcursor, QPointF highcursor);

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
    float lowlimittemp; // temperature associated with lowlimit
    float highlimittemp; // temperature associated with highlimit
    float minprojectiontemp; // minimum temp. for this projection
    float maxprojectiontemp; // maximum temp. for this projection
    float deltaprojectiontemp; // = maxprojectiontemp - minprojectiontemp
    bool inverse;
};

#endif // INFRASCALES_H
