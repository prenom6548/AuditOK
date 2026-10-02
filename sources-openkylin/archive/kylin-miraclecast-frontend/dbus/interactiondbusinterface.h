#ifndef INTERACTIONDBUSINTERFACE_H
#define INTERACTIONDBUSINTERFACE_H

#include <QDBusAbstractInterface>

class InteractionDBusInterface : public QDBusAbstractInterface
{
public:
    inline const char * getInterfacePath() const{
        return "/org/freedesktop/miracleagent";
    }
    inline const char *getInterfaceName() const{
        return "org.freedesktop.miracleagent.op";
    }

    /*
     * brief InteractionDBusInterface
     * param service
     * param connection
     * param parent
    */
    InteractionDBusInterface(const QString &service, // const QString &path, const char *interface,
                             const QDBusConnection &connection, QObject *parent = nullptr);

    QString getPinCode(int right);
};

#endif // INTERACTIONDBUSINTERFACE_H
