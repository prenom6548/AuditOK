#ifndef IDEVICEMANAGERITEM_H
#define IDEVICEMANAGERITEM_H

#include <QWidget>

#include "ISwitchButtonItem.h"

class IDeviceManagerItem
{
public:
    IDeviceManagerItem() = default;
    virtual ~IDeviceManagerItem() = default;

public:
    virtual bool isChecked() const = 0;

    virtual void setChecked(bool) = 0;

    virtual QWidget* getWidget() = 0;

    virtual std::shared_ptr<ISwitchButton> getSwitchBtn() const = 0;

    virtual QVariant getData() const = 0;
};

#endif // IDEVICEMANAGERITEM_H
