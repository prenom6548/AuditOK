#ifndef IDEVICEPORTITEM_H
#define IDEVICEPORTITEM_H

#include <memory>
#include <QWidget>
#include <QVariant>

class IDevicePortItem
{
public:
    IDevicePortItem(const QString& cardName, const QString& portName) : m_devName(cardName), m_portName(portName) {}
    virtual ~IDevicePortItem() {}

public:
    virtual QWidget* getWidget() = 0;

    virtual bool isChecked() = 0;

    virtual void setChecked(bool) = 0;

    virtual void setIcon(const QIcon&) = 0;

    QString getCardName() const {return m_devName;}

    QString getPortName() const {return m_portName;}

protected:
    QString m_devName;
    QString m_portName;
};

class IDevicePortSelectItem {
public:
    virtual void initSlots() = 0;

    virtual void setCurrentIndex(int) = 0;

    virtual void insertItem(std::shared_ptr<IDevicePortItem>) = 0;

    virtual void removeItem(const QString&, const QString&) = 0;

    virtual QVariant currentData(int = Qt::UserRole) const = 0;

    virtual int count() const = 0;

    virtual QWidget* getWidget() = 0;
};

#endif // IDEVICEPORTITEM_H
