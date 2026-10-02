#ifndef UKCC5DEVICEPORTITEM_H
#define UKCC5DEVICEPORTITEM_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <klabel.h>

#include "IDevicePortSelectItem.h"
using namespace kdk;

class Ukcc5DevicePortItem : public QWidget, public IDevicePortItem
{
public:
    Ukcc5DevicePortItem(KLabel*, QLabel*, const QString&, const QString&, QWidget* = nullptr);
    ~Ukcc5DevicePortItem() = default;

public:
    void initUi();

    virtual QWidget* getWidget() override;

    virtual bool isChecked() override;

    virtual void setChecked(bool) override;

    virtual void setIcon(const QIcon&) override;

private:
    KLabel* m_pDisplayLabel = nullptr;
    QLabel* m_pIconLabel = nullptr;
    bool m_bIsChecked = false;
};
#endif // UKCC5DEVICEPORTITEM_H
