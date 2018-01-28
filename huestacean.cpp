#include "huestacean.h"

Huestacean::Huestacean(QObject *parent) : QObject(parent)
{
    m_hue = new Hue;
    emit hueInit();
}
