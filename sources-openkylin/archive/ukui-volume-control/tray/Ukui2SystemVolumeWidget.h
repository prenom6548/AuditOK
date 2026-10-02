#ifndef UKUI2SYSTEMVOLUMEWIDGET_H
#define UKUI2SYSTEMVOLUMEWIDGET_H

#include <memory>
#include <QLabel>
#include <QObject>
#include <QSlider>
#include <QListView>
#include <QPushButton>

#include "ISystemVolumeWidget.h"
#include "ITrayVolumeSliderItem.h"
#include "ukui4.0/TrayListViewDelegate.h"
#include "ukui4.0/TrayListViewItemModel.h"
#include "IDevicePortSelectItem.h"

class IMainWindow;
class Ukui2SystemVolumeWidget : public QWidget, public ISystemVolumeWidget
{
    Q_OBJECT
public:
    explicit Ukui2SystemVolumeWidget(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>&,
                                     QPushButton*, QPushButton*, QPushButton*,
                                     IMainWindow*, QWidget* = nullptr);
    ~Ukui2SystemVolumeWidget();

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual void showWidget() override;

    virtual QWidget* getWidget() override;

    virtual void setValue(int) override;

    virtual void addDevice(const QString&, const QVariantList&) override;

    virtual void removeDevice(const QString&, const QString&) override;

    virtual void addDevicePortItem(std::shared_ptr<IDevicePortItem>) override;

    virtual void removeDevicePortItem(const QString&, const QVariantList&) override;

    virtual void setCurrentIndex(int) override;

Q_SIGNALS:
    void vauleChanged(int);

private Q_SLOTS:
    void vauleChangedSlots(int);

private:
    QLabel* m_pOutputLabel = nullptr;
    QLabel* m_pInputLabel = nullptr;
    QWidget* m_pBtnWidget = nullptr;
    QPushButton* m_pSwitchModuleBtn = nullptr;
    QPushButton* m_pDeviceVolumeBtn = nullptr;
    QPushButton* m_pAppVolumeBtn = nullptr;
    std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& m_sysVolumeSliderItemMap;
    IMainWindow* m_pMainWid;
};

#endif // UKUI2SYSTEMVOLUMEWIDGET_H
