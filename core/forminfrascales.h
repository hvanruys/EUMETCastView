#ifndef FORMINFRASCALES_H
#define FORMINFRASCALES_H

#include <QWidget>
#include <QDockWidget>
#include "infrascales.h"
#include "formimage.h"

class InfraScales;
class FormImage;

class FormInfraScales : public QDockWidget
{
public:
    FormInfraScales();
    QImage getScalesImage(int width);
    void initializeLowHigh();
    void setMinMaxTemp(float mintemp, float maxtemp);
    void getMinMaxTemp(float *mintemp, float *maxtemp);
    void setInverse(bool inverse);
    QColor getColor(float value);
    void setFormImage(FormImage *ptr);

private:
    InfraScales *infrascales;
    FormImage *formimage;


};

#endif // FORMINFRASCALES_H
