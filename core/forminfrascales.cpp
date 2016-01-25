#include "forminfrascales.h"

FormInfraScales::FormInfraScales()
{

    this->setWindowFlags(Qt::Widget|Qt::WindowStaysOnTopHint|Qt::X11BypassWindowManagerHint);
    this->setAllowedAreas(Qt::TopDockWidgetArea|Qt::BottomDockWidgetArea);
    this->setObjectName("InfraScales");
    this->setMinimumHeight(80);

    this->infrascales = new InfraScales(this);

    this->setWidget(infrascales);

}

void FormInfraScales::setFormImage(FormImage *ptr)
{
    formimage = ptr;
    connect(infrascales, SIGNAL(repaintprojectionimage()), this->formimage, SLOT(slotRepaintProjectionImage()));


}

QImage FormInfraScales::getScalesImage(int width)
{
    return this->infrascales->getScalesImage(width);
}

void FormInfraScales::initializeLowHigh()
{
    this->infrascales->initializeLowHigh();
}

void FormInfraScales::setMinMaxTemp(float mintemp, float maxtemp)
{
    this->infrascales->setMinMaxTemp(mintemp, maxtemp);
}

void FormInfraScales::getMinMaxTemp(float *mintemp, float *maxtemp)
{
    this->infrascales->getMinMaxTemp(mintemp, maxtemp);
}

void FormInfraScales::setInverse(bool inverse)
{
    this->infrascales->setInverse(inverse);
}

QColor FormInfraScales::getColor(float value)
{
    return this->infrascales->getColor(value);
}

