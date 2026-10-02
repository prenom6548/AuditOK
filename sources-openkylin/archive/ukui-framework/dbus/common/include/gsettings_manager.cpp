#include <QDBusContext>
#include "gsettings_manager.h"



GsettingsManager::GsettingsManager(QObject *parent, QString schema, QHash<QString, OnKeyChanged> changedHandlers)
{
     m_pGsettings = new UKUI::QGSettings(qPrintable(schema));
    if (!UKUI::QGSettings::isSchemaInstalled(qPrintable(schema))) {
        qDebug()<<schema<<"no install.";
        return;
    }
    connect(m_pGsettings, &UKUI::QGSettings::changed, this,
            &GsettingsManager::onGsettingsChanged);

    m_settingsChangedHandlers = changedHandlers;
}

GsettingsManager::~GsettingsManager() {
    if (m_pGsettings != nullptr) {
        delete m_pGsettings;
        m_pGsettings = nullptr;
    }
}

bool GsettingsManager::peekActionIsValid(QString action, QString &errorInfo) {
    if (m_pGsettings == nullptr) {
        errorInfo = tr("schema not installed");
        return false;
    }

    if (!m_pGsettings->keys().contains(action)) {
        errorInfo = tr("Specified action %1 not supported").arg(action);
        return false;
    }

    return true;
}

void GsettingsManager::set(const QString &key, const QVariant &value) {
    m_pGsettings->set(key, value);
}

QVariant GsettingsManager::get(const QString &key) {
    return m_pGsettings->get(key);
}

void GsettingsManager::onGsettingsChanged(QString key) {
    if (m_settingsChangedHandlers.contains(key)) {
        m_settingsChangedHandlers[key]();
    }
}

template<typename ...Args>
QDBusMessage GsettingsManager::callDbus(const QString &name,
                                                     const QString &path,
                                                     const QString &interface,
                                                     const QString &method,
                                                     Args &&...args) {
    QDBusInterface inputManagerInterface(name,
                                         path,
                                         interface);
    return inputManagerInterface.call(method, std::forward<Args>(args)...);
}
