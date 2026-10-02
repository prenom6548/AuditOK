#ifndef ISYSTEMVOLUMEWIDGET_H
#define ISYSTEMVOLUMEWIDGET_H

#include <QWidget>

#include "IDevicePortSelectItem.h"

class ISystemVolumeWidget
{
public:
    virtual void initUi() = 0;

    virtual void initSlots() = 0;

    virtual void showWidget() = 0;

    virtual QWidget* getWidget() = 0;

    virtual void setValue(int) = 0;

    virtual void addDevice(const QString&, const QVariantList&) = 0;

    virtual void removeDevice(const QString&, const QString&) = 0;

    virtual void addDevicePortItem(std::shared_ptr<IDevicePortItem>) = 0;

    virtual void removeDevicePortItem(const QString&, const QVariantList&) = 0;

    virtual void setCurrentIndex(int) = 0;

protected:
    std::shared_ptr<IDevicePortSelectItem> m_pListItem = nullptr;
};

#endif // ISYSTEMVOLUMEWIDGET_H
