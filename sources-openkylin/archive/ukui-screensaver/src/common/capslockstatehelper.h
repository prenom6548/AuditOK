#ifndef CAPSLOCKSTATEHELPER_H
#define CAPSLOCKSTATEHELPER_H

#include <QObject>
#include <ukuikeystate.h>

class CapslockStateHelper : public QObject
{
    Q_OBJECT
public:
    static CapslockStateHelper *instance(QObject *parent = nullptr);
    explicit CapslockStateHelper(QObject *parent = nullptr);

Q_SIGNALS:
    void capslockChanged(bool state);

private:
    UkuiKeyState* m_keyState;

    static CapslockStateHelper *m_instance;
};

#endif // CAPSLOCKSTATEHELPER_H
