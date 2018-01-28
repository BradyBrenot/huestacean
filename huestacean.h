#ifndef HUESTACEAN_H
#define HUESTACEAN_H

#include <QObject>
#include "hue.h"

class Huestacean : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Hue* hue READ hue NOTIFY hueInit)

public:
    explicit Huestacean(QObject *parent = nullptr);

    Hue* hue() {
        return m_hue;
    }

signals:
    void hueInit();

private:
    Hue* m_hue;
};

#endif // HUESTACEAN_H
