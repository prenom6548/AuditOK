#ifndef SCREENSAVERPLUGINSO_H
#define SCREENSAVERPLUGINSO_H

#include <QWidget>

class ScreensaverPlugin
{
public:
    virtual ~ScreensaverPlugin() {}
    //插件实例的名称
    virtual QString name() const = 0;

    //创建UI的实例
    virtual QWidget* createWidget(bool isScreensaver, QWidget* parent) = 0;

    //获得插件的展示名称
    virtual QString displayName() const = 0;
};

//定义了在QT系统中该接口的全局唯一的ID
//实现该SDK的插件也要定义相同的ID
//接口的ID中包含了版本信息,通过该ID我们可以区别不同版本的SDK和插件
//Q_DECLARE_INTERFACE宏将类型和ID关联起来,这样QT就可以验证加载的插件是否可以转换成MyPluginInterface类型
#define interface_iid "org.ukui.screensaver.screensaver-default1.0.0"
Q_DECLARE_INTERFACE(ScreensaverPlugin, interface_iid)

#endif // SCREENSAVERPLUGINSO_H
