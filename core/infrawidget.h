#ifndef INFRAWIDGET_H
#define INFRAWIDGET_H

#include <QWidget>
#include <QPainter>

class InfraWidget : public QWidget
{
    Q_OBJECT
public:
    explicit InfraWidget(QWidget *parent = 0);
    void setlowcursor(float low);
    void sethighcursor(float high);
    QColor getColor(double value);
    void setInverse(bool inv) { inverse = inv; lowlimit = 0.0; highlimit = 1.0; }
    bool getInverse() { return inverse; }
    void initializeLowHigh() { lowlimit = 0.0; highlimit = 1.0; }
    float lowlimit;
    float highlimit;

signals:

public slots:

protected:
  void paintEvent( QPaintEvent * );
//  void mousePressEvent( QMouseEvent *);
//  void mouseReleaseEvent( QMouseEvent *);
//  void mouseMoveEvent( QMouseEvent *);
//  void resizeEvent( QResizeEvent * );
//  void wheelEvent(QWheelEvent *event);
//  void timerEvent(QTimerEvent *event);

  QColor interpolate(QColor start,QColor end,double ratio);
  bool down;
  QList<QColor> colorlist;
  bool grablowlimit;
  bool grabhighlimit;
  bool inverse;



};

#endif // INFRAWIDGET_H
