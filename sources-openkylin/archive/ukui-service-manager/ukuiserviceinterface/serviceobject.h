#ifndef SERVICEOBJECT_H
#define SERVICEOBJECT_H

#include <QObject>

namespace UKUI {

class ServiceObject : public QObject
{
    Q_OBJECT
public:
    explicit ServiceObject(QObject *parent = nullptr);

protected:
    void emitUkuiServiceObjectStarted();
    void emitUkuiServiceObjectFinished();
    void emitUkuiServiceObjectTriggered();
};

}

#endif // SERVICEOBJECT_H
