#ifndef UKUI4APPVOLUMEWIDGET_H
#define UKUI4APPVOLUMEWIDGET_H

#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include "IAppVolumeWidget.h"
#include "ITrayVolumeSliderItem.h"

class IMainWindow;
class Ukui4AppVolumeWidget : public QWidget, public IAppVolumeWidget
{
    Q_OBJECT
public:
    explicit Ukui4AppVolumeWidget(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>&,
                                  std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>&,
                                  IMainWindow*, QWidget* = nullptr);
    ~Ukui4AppVolumeWidget();

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
    QLabel* m_pAppVolumeLabel = nullptr;           // 应用音量标签
    QScrollArea* m_pAppArea = nullptr;             // 应用音量滚动区域
    QWidget* m_pDisplayAppVolumeWidget = nullptr;  // 应用音量显示区域
    QVBoxLayout* m_pVlayout = nullptr;
    int sinkInputCount = 0;

    std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& m_sysVolumeSliderItemMap;
    std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& m_appVolumeSliderItemMap;
    IMainWindow* m_pMainWid;
};

#endif // UKUI4APPVOLUMEWIDGET_H
