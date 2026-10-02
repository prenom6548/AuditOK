#ifndef NETFRAME_H
#define NETFRAME_H
#include <QLabel>
#include <QGridLayout>
#include <QCheckBox>
#include <QTextBrowser>
#include <QEvent>
#include <QScrollBar>
#include <QThread>
#include "../PluginService/ui_unit/middleframemanager.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QPluginLoader>
#include <QTabWidget> // 添加QTabWidget的头文件
#include <kylin-nm/kylinnetworkmanager.h>

using namespace KInstaller;

class NetFrame : public MiddleFrameManager
{
    Q_OBJECT

public:
    explicit NetFrame(QWidget *parent = nullptr);
    void initUI();//画出本插件全部界面
    void initAllConnect();//初始化信号与槽函数
    void translateStr();//切换国际化语言显示

public slots:
    // void enableNextButton(bool value);
    void clickNextButton();//重新实现下一步按钮槽函数
    void nextBtnAvailable();//是否使能下一步按钮
    void onDeviceStatusChanged(QString deviceName, KyDeviceState newstate, KyDeviceState oldstate,
                                                   NetworkManager::Device::StateChangeReason reason);

private:
    void changeEvent(QEvent *event) override;//主要用于响应语言切换信号

public://定义相关类型的成员变量，以便实现插件主界面以及业务逻辑
    QLabel* m_mainTitle;
    QTextBrowser *m_ShowTextBrowser;
    QString m_strShowText;
    QLabel *m_subtitleLabel;
    QLabel *m_titleLabel;

private slots:
    QWidget* loadWiredPlugin();      // 添加有线网络插件加载
    QWidget* loadWirelessPlugin();   // 添加无线网络插件加载
    void currentRowChangeSlot(int row);

Q_SIGNALS:
    void currentChanged(int);
    void switchToPlugin(int index);

private:
    QWidget *loadPlugin(const QString &pluginPath); 
    QWidget *widget;
    QWidget *wiredWidget;
    QWidget *wirelessWidget;
    KyNetworkManager* manager;
    QThread*        thread;
    QStackedWidget *m_pluginStackedWidget;
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_netTabBar;
    QPushButton *wiredBtn;
    QPushButton *wifiBtn;
};

#endif // NETFRAME_H
