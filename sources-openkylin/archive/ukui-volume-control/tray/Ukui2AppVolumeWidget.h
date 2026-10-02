#ifndef UKUI2APPVOLUMEWIDGET_H
#define UKUI2APPVOLUMEWIDGET_H

#include <QHBoxLayout>
#include <QScrollArea>

#include "IAppVolumeWidget.h"
#include "ITrayVolumeSliderItem.h"

class IMainWindow;
class Ukui2AppVolumeWidget : public QWidget, public IAppVolumeWidget
{
    Q_OBJECT
public:
    explicit Ukui2AppVolumeWidget(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>&,
                                  std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>&,
                                  QPushButton*, QPushButton*, QPushButton*,
                                  IMainWindow*, QWidget* = nullptr);
    ~Ukui2AppVolumeWidget();

public:
    virtual void initUi() override;

    virtual void showWidget() override;

    virtual QWidget* getWidget() override;

    virtual void addAppWidget(int) override;

    virtual void removeAppWidget(int) override;

    virtual void addAppToWidget(const QString&, const QString&) override;

    virtual void removeAppfromWidget(const QString&);

private:
    QLabel* m_pSysVolumeLabel = nullptr;           // 系统音量标签
    QLabel* m_pAppLabel = nullptr;                 // 应用音量标签
    QScrollArea* m_pAppArea = nullptr;             // 应用音量滚动区域
    QWidget* m_pDisplayAppVolumeWidget = nullptr;  // 应用音量显示区域
    QVBoxLayout* m_pVlayout = nullptr;
    QPushButton* m_pSwitchModuleBtn = nullptr;
    QPushButton* m_pDeviceVolumeBtn = nullptr;
    QPushButton* m_pAppVolumeBtn = nullptr;
    QWidget* m_pBtnWidget = nullptr;
    std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& m_sysVolumeSliderItemMap;
    std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& m_appVolumeSliderItemMap;
    IMainWindow* m_pMainWid;
};



#endif // UKUI2APPVOLUMEWIDGET_H
