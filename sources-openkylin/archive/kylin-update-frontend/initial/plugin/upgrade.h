#ifndef UPGRADE_H
#define UPGRADE_H

#include <QObject>

#include <ukcc/interface/interface.h>
#include "src/upgrademain.h"
#include <kysdk/kysdk-system/libkysysinfo.h>

class Upgrade : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)
public:
    Upgrade();
    ~Upgrade();

//    QString get_plugin_name() Q_DECL_OVERRIDE;
//    int get_plugin_type()     Q_DECL_OVERRIDE;
//    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;
//    void plugin_delay_control() Q_DECL_OVERRIDE;
//    const QString name() const  Q_DECL_OVERRIDE;

    QString plugini18nName()        Q_DECL_OVERRIDE;
    int pluginTypes()               Q_DECL_OVERRIDE;
    QWidget *pluginUi()             Q_DECL_OVERRIDE;
    bool isEnable() const           Q_DECL_OVERRIDE;
    const QString name() const      Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const   Q_DECL_OVERRIDE;
    QIcon icon() const              Q_DECL_OVERRIDE;
    QString translationPath() const Q_DECL_OVERRIDE;


private:
    QString pluginName;
    int pluginType;
    UpgradeMain *pluginWidget;
    bool mFirstLoad;

private:
    void initSearchText();      // 搜索翻译

};

#endif // UPGRADE_H
