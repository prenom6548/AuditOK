#include "global-settings-adapter.h"
#include "global-settings.h"
#include "common.h"

#include <QDebug>
#include <QGSettings>

using namespace Peony;

static GlobalSettingsAdapter *global_instance = nullptr;

GlobalSettingsAdapter *GlobalSettingsAdapter::getInstance()
{
    if (!global_instance) {
        global_instance = new GlobalSettingsAdapter;
    }
    return global_instance;
}

GlobalSettingsAdapter::GlobalSettingsAdapter(QObject *parent)
    : QObject(parent)
{
    // 连接单例的信号到适配器信号
    GlobalSettings* settings = GlobalSettings::getInstance();
    connect(settings, &GlobalSettings::valueChanged,
            this, &GlobalSettingsAdapter::onGlobalSettingsChanged);

    if (QGSettings::isSchemaInstalled(PANEL_SETTINGS))
    {
        //panel monitor
        if (!m_panelSetting)
            m_panelSetting = new QGSettings(PANEL_SETTINGS, QByteArray(), this);
        connect(m_panelSetting, &QGSettings::changed, this, [=](const QString &key){
            if (key == "panelposition" || key == "panelsize" || key == "settingsislandposition" || key == "paneltype") {
                panelSettingsChanged();
            }
        });
        panelSettingsChanged();
    }
}

QVariant GlobalSettingsAdapter::getValue(const QString &key) const
{
    if (key == "panelposition" || key == "panelsize" || key == "settingsislandposition" || key == "paneltype") {
        return m_panelSetting->get(key);
    }
    return GlobalSettings::getInstance()->getValue(key);
}

void GlobalSettingsAdapter::setValue(const QString &key, const QVariant &value)
{
    if (key == "panelposition" || key == "panelsize" || key == "settingsislandposition" || key == "paneltype") {
        return;
    }
    GlobalSettings::getInstance()->setValue(key, value);
}

bool GlobalSettingsAdapter::isKeyAvailable(const QString &key) const
{
    return GlobalSettings::getInstance()->isExist(key);
}

bool GlobalSettingsAdapter::useAutoLayout()
{
    return GlobalSettings::getInstance()->getValue("desktopUseAutoLayout").toBool();
}

void GlobalSettingsAdapter::panelSettingsChanged()
{
    int settingsislandposition = m_panelSetting->keys().contains("settingsislandposition") ? m_panelSetting->get("settingsislandposition").toInt() : -1;
    int paneltype = m_panelSetting->keys().contains("paneltype") ? m_panelSetting->get("paneltype").toInt() : -1;
    int position = m_panelSetting->get("panelposition").toInt();
    int margins = m_panelSetting->get("panelsize").toInt();

    if (settingsislandposition == 1 && paneltype == 1) {
        setPanelMargins(QMargins(0, 32, 0, margins));
        return;
    }
    switch (position) {
    case 1: {
        setPanelMargins(QMargins(0, margins, 0, 0));
        break;
    }
    case 2: {
        setPanelMargins(QMargins(margins, 0, 0, 0));
        break;
    }
    case 3: {
        setPanelMargins(QMargins(0, 0, margins, 0));
        break;
    }
    default: {
        setPanelMargins(QMargins(0, 0, 0, margins));
        break;
    }
    }
    return;
}

void GlobalSettingsAdapter::onGlobalSettingsChanged(const QString &key)
{
    QVariant value = getValue(key);
    Q_EMIT valueChanged(key, value);
}
