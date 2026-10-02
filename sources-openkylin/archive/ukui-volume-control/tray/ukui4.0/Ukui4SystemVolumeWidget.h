#ifndef UKUI4SYSTEMVOLUMEWIDGET_H
#define UKUI4SYSTEMVOLUMEWIDGET_H

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
class Ukui4SystemVolumeWidget : public QWidget, public ISystemVolumeWidget
{
    Q_OBJECT
public:
    explicit Ukui4SystemVolumeWidget(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>&,
                                 std::unordered_map<int, std::shared_ptr<IDevicePortItem>>&,
                                 IMainWindow*, QWidget* = nullptr);
    ~Ukui4SystemVolumeWidget();

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
    QSlider* m_pSysVolumeSlider = nullptr;         // 系统音量滑动条
    QPushButton* m_pSysVolumeMuteBtn = nullptr;    // 系统音量静按钮
    QLabel* m_pSystemVolumeLabel = nullptr;        // 系统音量标签
    QLabel* m_pOutputLabel = nullptr;
    QLabel* m_pSystemVolumePercentLabel = nullptr; // 系统音量百分百标签
    TrayListViewItemModel* m_pModel = nullptr;
    TrayListViewDelegate* m_pDelegate = nullptr;
    std::unordered_map<int, std::shared_ptr<IDevicePortItem>>& m_devicePortItemMap;
    std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& m_sysVolumeSliderItemMap;
    IMainWindow* m_pMainWid;
};

#endif // UKUI4SYSTEMVOLUMEWIDGET_H
