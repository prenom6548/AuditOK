#ifndef AUDIOPULGINUKUI2_H
#define AUDIOPULGINUKUI2_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include <QProcess>
#include <QTranslator>
#include <QtCore/qglobal.h>

#include "include/interface.h"
#include "Ukui2SwitchButtonItem.h"
#include "Ukui2VolumeSliderItem.h"
#include "Ukui2SelectComboxItem.h"

#if defined(LIBUKCC_LIBRARY)
#  define LIBUKCC_EXPORT Q_DECL_EXPORT
#else
#  define LIBUKCC_EXPORT Q_DECL_IMPORT
#endif

namespace Ui {
class Audio;
}

class AudioPulginUkui2 : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    AudioPulginUkui2();
    ~AudioPulginUkui2();

public:
    QString get_plugin_name()       Q_DECL_OVERRIDE;

    int get_plugin_type()           Q_DECL_OVERRIDE;

    QWidget * get_plugin_ui()       Q_DECL_OVERRIDE;

    void plugin_delay_control()     Q_DECL_OVERRIDE;

    const QString name() const      Q_DECL_OVERRIDE;

private:
    bool m_firstLoad = false;
    int m_pluginType = DEVICES;
    QString m_pluginName = "";
    Ui::Audio* m_pUi = nullptr;
    QWidget* m_pPluginWidget = nullptr;

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
//        {VolumeSliderType::VOLUME_SLIDER_INPUT, tr("Input")},
//        {VolumeSliderType::VOLUME_SLIDER_OUTPUT, tr("Output")},
//        {VolumeSliderType::VOLUME_SLIDER_INPUTLEVEL, tr("Input Level")},
//        {VolumeSliderType::VOLUME_SLIDER_BALANCE, tr("Balance")}
//    };

//    std::unordered_map<SelectComboxType, QString> m_selectComboxKeys {
//        {SelectComboxType::SELECT_COMBOX_INPUT, tr("Input")},
//        {SelectComboxType::SELECT_COMBOX_OUTPUT, tr("Output")},
//    };
};

#endif // AUDIOPULGINUKUI2_H
