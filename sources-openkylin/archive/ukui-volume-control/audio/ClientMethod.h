#ifndef CLIENTMETHOD_H
#define CLIENTMETHOD_H

#include <memory>
#include "DBusClient.h"
#include "ClientManager.h"

class ClientMethod
{
public:
    static ClientMethod& getInstance();

    void setManager(std::shared_ptr<ClientManager>);

    void setValue(int, int);

    void setMute(int, bool);

    void setEnabled(int, bool);

    void setBalanceEnabled(bool);

    void setRange(int, int, int);

    void setChecked(int, bool);

    void setCurrentIndex(int, int);

    int findData(int, const QVariant&, int = Qt::UserRole) const;

    int findText(int, const QString&) const;

    void addItem(int, const QString&, const QVariant& = QVariant());

    void removeItem(int, int);

    void clear(int);

private:
    ClientMethod() = default;
    ClientMethod(const ClientMethod&) = delete;
    ClientMethod(ClientMethod&&) = delete;
    ClientMethod operator=(const ClientMethod&) = delete;
    ClientMethod operator=(ClientMethod&&) = delete;
    virtual ~ClientMethod() = default;

private:
    std::shared_ptr<ClientManager> m_pClientManager{nullptr};
};

#endif // CLIENTMETHOD_H
