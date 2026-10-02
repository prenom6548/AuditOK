#ifndef AUDIO_H
#define AUDIO_H

#include <QWidget>
#include <QObject>
#include <QEvent>
#include <QtPlugin>
#include <QProcess>
#include <ukcc/interface/interface.h>
#include <QTranslator>
#include <QtCore/qglobal.h>

#include "Ukui4SwitchButtonItem.h"
#include "Ukui4VolumeSliderItem.h"
#include "Ukui4SelectComboxItem.h"

#if defined(LIBUKCC_LIBRARY)
#  define LIBUKCC_EXPORT Q_DECL_EXPORT
#else
#  define LIBUKCC_EXPORT Q_DECL_IMPORT
#endif

namespace Ui {
class Audio;
}

/* 控制面板跳转二级弹窗支持 */
class UkccArgEvent : public QEvent
{
private:
    QString m_SecondPlugin;

public:
    const static Type TYPE = static_cast<Type>(QEvent::User+0x10);
    explicit UkccArgEvent(QString arg) : QEvent(TYPE) {
        m_SecondPlugin = arg;
    }

    QString getPluginName() {
        return m_SecondPlugin;
    }
};

class Audio : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Audio();
    ~Audio() = default;

public:
    QString plugini18nName()        Q_DECL_OVERRIDE;

    int pluginTypes()               Q_DECL_OVERRIDE;

    QWidget* pluginUi()             Q_DECL_OVERRIDE;

    bool isEnable() const           Q_DECL_OVERRIDE;

    const QString name() const      Q_DECL_OVERRIDE;

    bool isShowOnHomePage() const   Q_DECL_OVERRIDE;

    QIcon icon() const              Q_DECL_OVERRIDE;

    QString translationPath() const Q_DECL_OVERRIDE;

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    void initSearchText(); // 搜索翻译

private:
    QString m_pluginName = "";
    int m_pluginType = SYSTEM;
    QWidget* m_pWidget = nullptr;
    bool m_firstLoad = true;
//    std::unordered_map<SwitchButtonType, QString> m_switchButtonkeys {
//        {SwitchButtonType::SWITCH_BUTTON_VOLUMEBOOST, tr("Volume Increase")},
//        {SwitchButtonType::SWITCH_BUTTON_MONO, tr("Mono Audio")},
//        {SwitchButtonType::SWITCH_BUTTON_ECHOCANCEL, tr("Noise Reduction")},
//        {SwitchButtonType::SWITCH_BUTTON_LOOPBACK, tr("Voice Monitor")},
//        {SwitchButtonType::SWITCH_BUTTON_STARTUP, tr("Startup Misic")},
//        {SwitchButtonType::SWITCH_BUTTON_POWEROFF, tr("Poweroff Music")},
//        {SwitchButtonType::SWITCH_BUTTON_LOGOUT, tr("Logout Music")},
//        {SwitchButtonType::SWITCH_BUTTON_WAKEUP, tr("Wakeup Music")},
//        {SwitchButtonType::SWITCH_BUTTON_ALERT, tr("Beep Switch")}
//    };

//    std::unordered_map<VolumeSliderType, QString> m_volumeSliderKeys {
//        {VolumeSliderType::VOLUME_SLIDER_INPUT, tr("Volume")},
//        {VolumeSliderType::VOLUME_SLIDER_OUTPUT, tr("Master Volume")},
//        {VolumeSliderType::VOLUME_SLIDER_INPUTLEVEL, tr("Input Level")},
//        {VolumeSliderType::VOLUME_SLIDER_BALANCE, tr("Balance")}
//    };

//    std::unordered_map<SelectComboxType, QString> m_selectComboxKeys {
//        {SelectComboxType::SELECT_COMBOX_INPUT, tr("Input Device")},
//        {SelectComboxType::SELECT_COMBOX_OUTPUT, tr("Output Device")},
//        {SelectComboxType::SELECT_COMBOX_SOUNDEFFECT, tr("Sound Theme")},
//        {SelectComboxType::SELECT_COMBOX_VOLUMEADJUST, tr("Volume Control Sound")},
//        {SelectComboxType::SELECT_COMBOX_NOTIFICTION, tr("Notification Sound")}
//    };
};

#endif // AUDIO_H
