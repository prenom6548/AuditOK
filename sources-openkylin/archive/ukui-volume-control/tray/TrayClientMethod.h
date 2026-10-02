#ifndef TRAYCLIENTMETHOD_H
#define TRAYCLIENTMETHOD_H

#include <memory>
#include "DBusClient.h"
#include "TrayClientManager.h"

class TrayClientMethod
{
public:
    static TrayClientMethod& getInstance();

public:
    void setManager(std::shared_ptr<TrayClientManager>);

    void setValue(int, int, int) ;

    void setIcon(int, const QIcon&);

    void setChecked(int, bool);

    void setCurrentIndex(int);

    void addDeviceItem(int, const ClientDeviceInfo&);

    void removeDeviceItem(const QString&, const QString&);

    void addAppItem(int, const QString&, const QString&);

    void removeAppItem(int);

    void updateDescription(const QString&, const QString&);

private:
    TrayClientMethod() = default;
    TrayClientMethod(const TrayClientMethod&) = delete;
    TrayClientMethod(TrayClientMethod&&) = delete;
    TrayClientMethod operator=(const TrayClientMethod&) = delete;
    TrayClientMethod operator=(TrayClientMethod&&) = delete;
    virtual ~TrayClientMethod() = default;

private:
    std::shared_ptr<TrayClientManager> m_pClientManager{nullptr};
};

#endif // TRAYCLIENTMETHOD_H
