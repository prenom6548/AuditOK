/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#include "file-indexer-config.h"
#include "ai-indexer.h"

#include <mutex>
#include <QDebug>
#include <QDir>
#include <QMutexLocker>
#include <kylin-ai/common/common.h>
#include <dbus-connector.h>

static const QString INDEX_SETTINGS = QDir::homePath() + "/.config/org.ukui/ukui-search/ukui-search-service.conf";
static const QString CONTENT_INDEX_TARGET_TYPE = "contentIndexTarget";
static const QString OCR_CONTENT_INDEX_TARGET_TYPE = "ocrContentIndexTarget";
static const QString S_AI_FILE_TYPE = "sAiFileType";
static const QString S_AI_IMAGE_TYPE = "sAiImageType";
/**
 * changelog: 1.3 增加电源管控状态
 * changelog: 1.2 增加AI索引开关
 * changelog: 1.1 增加ocr开关
 */
static const QString GSETTINGS_VERSION = QStringLiteral("1.3");
/**
 * changelog: 1.0 init
 */
static const QString SETTINGS_VERSION = QStringLiteral("1.2");
static const QByteArray UKUI_SEARCH_SCHEMAS = QByteArrayLiteral("org.ukui.search.settings");
static const QString FILE_INDEX_ENABLE_KEY = QStringLiteral("fileIndexEnable");
static const QString CONTENT_INDEX_ENABLE_KEY = QStringLiteral("contentIndexEnable");
static const QString CONTENT_FUZZY_SEARCH_KEY = QStringLiteral("contentFuzzySearch");
static const QString CONTENT_INDEX_ENABLE_OCR_KEY = QStringLiteral("contentIndexEnableOcr");
static const QString META_DATA_INDEX_ENABLE_KEY = QStringLiteral("metaDataIndexEnable");
static const QString AI_INDEX_ENABLE_KEY = QStringLiteral("aiIndexEnable");
static const QString EXIT_POWER_CONTROL_KEY = QStringLiteral("exitPowerControl");
static const QString GSETTINGS_VERSION_KEY = QStringLiteral("version");
static const QString SETTINGS_VERSION_KEY = QStringLiteral("version");

static const QByteArray UKUI_PWMANAGER_SCHEMAS = QByteArrayLiteral("org.ukui.power-manager");
static const QString POWER_POLICY_BATTERY_KEY = QStringLiteral("powerPolicyBattery");
static const QString ON_BATTERY_PROP = QStringLiteral("OnBattery");

static std::once_flag flag;
static UkuiSearch::FileIndexerConfig *globalInstance = nullptr;
using namespace UkuiSearch;
FileIndexerConfig *FileIndexerConfig::getInstance()
{
    std::call_once(flag, [ & ] {
        globalInstance = new FileIndexerConfig();
    });
    return globalInstance;
}

FileIndexerConfig::FileIndexerConfig(QObject *parent)
    : QObject(parent),
      m_dirWatcher(DirWatcher::getDirWatcher())
{
    connect(m_dirWatcher, &DirWatcher::appendIndexItem, this, &FileIndexerConfig::appendIndexDir);
    connect(m_dirWatcher, &DirWatcher::removeIndexItem, this, &FileIndexerConfig::removeIndexDir);

    const QByteArray id(UKUI_SEARCH_SCHEMAS);
    if(QGSettings::isSchemaInstalled(id)) {
        m_gsettings = new QGSettings(id, QByteArray(), this);
        //保留旧版本配置
        if(m_gsettings->keys().contains(GSETTINGS_VERSION_KEY)) {
            QString oldVersion = m_gsettings->get(GSETTINGS_VERSION_KEY).toString();
            if (oldVersion != GSETTINGS_VERSION) {
                if (oldVersion == "0.0") {
                    bool fileIndexEnable = false;
                    if (m_gsettings->keys().contains(FILE_INDEX_ENABLE_KEY)) {
                        fileIndexEnable = m_gsettings->get(FILE_INDEX_ENABLE_KEY).toBool();
                    }
                    if (fileIndexEnable) {
                        if(m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_KEY)) {
                            m_gsettings->set(CONTENT_INDEX_ENABLE_KEY, true);
                        }
                        if(m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_OCR_KEY)) {
                            m_gsettings->set(CONTENT_INDEX_ENABLE_OCR_KEY, true);
                        }
                    }
                } else if (oldVersion == "1.0") {
                    bool contentIndex = false;
                    if(m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_KEY)) {
                        contentIndex = m_gsettings->get(CONTENT_INDEX_ENABLE_KEY).toBool();
                    }
                    if(contentIndex) {
                        if(m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_OCR_KEY)) {
                            m_gsettings->set(CONTENT_INDEX_ENABLE_OCR_KEY, true);
                        }
                    }
                }
                m_gsettings->set(GSETTINGS_VERSION_KEY, GSETTINGS_VERSION);
            }
        }
        connect(m_gsettings, &QGSettings::changed, this, [ = ](const QString &key) {
            if(key == FILE_INDEX_ENABLE_KEY) {
                Q_EMIT fileIndexEnableStatusChanged(m_gsettings->get(FILE_INDEX_ENABLE_KEY).toBool());
            } else if (key == CONTENT_INDEX_ENABLE_KEY) {
                Q_EMIT contentIndexEnableStatusChanged(m_gsettings->get(CONTENT_INDEX_ENABLE_KEY).toBool());
            } else if (key == CONTENT_INDEX_ENABLE_OCR_KEY) {
                Q_EMIT contentIndexEnableOcrStatusChanged(m_gsettings->get(CONTENT_INDEX_ENABLE_OCR_KEY).toBool());
            } else if (key == AI_INDEX_ENABLE_KEY) {
                bool aiEnable = m_gsettings->get(AI_INDEX_ENABLE_KEY).toBool();
                if (aiEnable) {
                    if (is_ai_subsystem_inited() && AiIndexer::aiIsReady()) {
                        Q_EMIT aiIndexEnableStatusChanged(aiEnable);
                        if (m_underCtrl) {
                            m_underCtrl = false;
                            qDebug() << "Exit power control.";
                            if (m_gsettings->keys().contains(EXIT_POWER_CONTROL_KEY)) {
                                m_gsettings->set(EXIT_POWER_CONTROL_KEY, true);
                            }
                        }
                    } else {
                        m_gsettings->set(AI_INDEX_ENABLE_KEY, false);
                    }
                }
            }
        });
    } else {
        qWarning() << UKUI_SEARCH_SCHEMAS << " is not found!";
    }
    initPowerSettings();
    m_settings = new QSettings(INDEX_SETTINGS, QSettings::IniFormat, this);
    bool useDefaultSettings = false;
    if(m_settings->value(SETTINGS_VERSION_KEY).toString() != SETTINGS_VERSION) {
        qDebug() << "File index Settings version changed, using default file types, current version:"
                 << SETTINGS_VERSION << "previous version:"
                 << m_settings->value(SETTINGS_VERSION_KEY).toString();
        useDefaultSettings = true;
        m_settings->setValue(SETTINGS_VERSION_KEY, SETTINGS_VERSION);
    }
    syncIndexFileTypes(useDefaultSettings);
}

FileIndexerConfig::~FileIndexerConfig()
= default;

void FileIndexerConfig::initPowerSettings()
{
    //init power manager GSettings
    const auto pwId(UKUI_PWMANAGER_SCHEMAS);
    if (QGSettings::isSchemaInstalled(pwId)) {
        m_pmGSettings = new QGSettings(pwId, QByteArray(), this);
        if (m_pmGSettings->keys().contains(POWER_POLICY_BATTERY_KEY)) {
            m_powerPolicy = m_pmGSettings->get(POWER_POLICY_BATTERY_KEY).toInt();
        }
    } else {
        qCritical() << UKUI_PWMANAGER_SCHEMAS << " not installed!\n";
    }
    if (m_pmGSettings) {
        connect(m_pmGSettings, &QGSettings::changed, this, [this](const QString &key) {
            if (key == POWER_POLICY_BATTERY_KEY) {
                m_powerPolicy = m_pmGSettings->get(key).toInt();
                updateCtrlState();
            }
        });
    }

    //init dbus interface
    auto upwConnector = new UkuiQuick::DbusConnector(QStringLiteral("org.freedesktop.UPower"),
                                              QStringLiteral("/org/freedesktop/UPower"),
                                              QStringLiteral("org.freedesktop.DBus.Properties"),
                                              QDBusConnection::systemBus());

    connect(upwConnector, &UkuiQuick::DbusConnector::ready, this, [ = ](QDBusInterface *dbusInterface) {
        if (dbusInterface->isValid()) {
            m_upwPropInterface = dbusInterface;
            qDebug() << "connect uPower signal" << connect(m_upwPropInterface, SIGNAL(PropertiesChanged(const QString&, const QVariantMap&,const QStringList&)),
                this, SLOT(onPropertiesChanged(const QString&, const QVariantMap&,const QStringList&)));
            auto *watcher = new QDBusPendingCallWatcher(m_upwPropInterface->asyncCall("Get", "org.freedesktop.UPower", ON_BATTERY_PROP));
            connect(watcher, &QDBusPendingCallWatcher::finished, this, [ this ](QDBusPendingCallWatcher *self) {
                QDBusPendingReply<QVariant> reply = *self;
                if (reply.isError()) {
                    qWarning() << "failed to get OnBattery property" << reply.error();
                } else {
                    if (reply.isValid()) {
                        if (reply.value().toBool()) {
                            m_onBattery = reply.value().toBool();
                            if (m_gsettings->keys().contains(EXIT_POWER_CONTROL_KEY)) {
                                if (!m_gsettings->get(EXIT_POWER_CONTROL_KEY).toBool()) {
                                    updateCtrlState();
                                }
                            }
                        } else {
                            qWarning() << "The battery is charging";
                        }
                    }
                }
                self->deleteLater();
            });
        }
        if (sender()) {
            sender()->deleteLater();
        }
    });
    upwConnector->start();
}

void FileIndexerConfig::updateCtrlState()
{
    if (!m_gsettings || !m_gsettings->keys().contains(AI_INDEX_ENABLE_KEY)) {
        return;
    }
    bool aiEnable = m_gsettings->get(AI_INDEX_ENABLE_KEY).toBool();

    //0-性能 1-平衡 2-节能
    if (m_powerPolicy == 2 && m_onBattery && aiEnable) {
        m_gsettings->set(AI_INDEX_ENABLE_KEY, false);
        m_underCtrl = true;
        qDebug() << "Power control enabled.";
        if (m_gsettings->keys().contains(EXIT_POWER_CONTROL_KEY)) {
            m_gsettings->set(EXIT_POWER_CONTROL_KEY, false);
        }
    } else if (!aiEnable && m_underCtrl) {
        m_underCtrl = false;
        m_gsettings->set(AI_INDEX_ENABLE_KEY, true);
    }
}

void FileIndexerConfig::onPropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    if (interface != QStringLiteral("org.freedesktop.UPower")) {
        return;
    }

    if (!changedProperties.contains(ON_BATTERY_PROP)) {
        return;
    }

    m_onBattery = changedProperties.value(ON_BATTERY_PROP).toBool();
    updateCtrlState();
}

QStringList FileIndexerConfig::currentIndexableDir()
{
    return DirWatcher::getDirWatcher()->currentIndexableDir();
}

QStringList FileIndexerConfig::currentBlackListOfIndex()
{
    return DirWatcher::getDirWatcher()->currentBlackListOfIndex();
}

bool FileIndexerConfig::isFileIndexEnable()
{
    QMutexLocker locker(&m_mutex);
    if(m_gsettings) {
        if(m_gsettings->keys().contains(FILE_INDEX_ENABLE_KEY)) {
            return m_gsettings->get(FILE_INDEX_ENABLE_KEY).toBool();
        } else {
            qWarning() << "FileIndexerConfig: Can not find key:" << FILE_INDEX_ENABLE_KEY << "in" << UKUI_SEARCH_SCHEMAS;
            return false;
        }
    } else {
        qWarning() << "FileIndexerConfig:" << UKUI_SEARCH_SCHEMAS << " is not found!";
        return false;
    }
}

bool FileIndexerConfig::isContentIndexEnable()
{
    QMutexLocker locker(&m_mutex);
    if(m_gsettings) {
        if(m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_KEY)) {
            return m_gsettings->get(CONTENT_INDEX_ENABLE_KEY).toBool();
        } else {
            qWarning() << "FileIndexerConfig: Can not find key:" << CONTENT_INDEX_ENABLE_KEY << "in" << UKUI_SEARCH_SCHEMAS;
            return false;
        }
    } else {
        qWarning() << "FileIndexerConfig:" << UKUI_SEARCH_SCHEMAS << " is not found!";
        return false;
    }
}

bool FileIndexerConfig::isFuzzySearchEnable()
{
    QMutexLocker locker(&m_mutex);
    if(m_gsettings) {
        if(m_gsettings->keys().contains(CONTENT_FUZZY_SEARCH_KEY)) {
            return m_gsettings->get(CONTENT_FUZZY_SEARCH_KEY).toBool();
        } else {
            qWarning() << "FileIndexerConfig: Can not find key:" << CONTENT_FUZZY_SEARCH_KEY << "in" << UKUI_SEARCH_SCHEMAS;
            return false;
        }
    } else {
        qWarning() << "FileIndexerConfig:" << UKUI_SEARCH_SCHEMAS << " is not found!";
        return false;
    }
}

bool FileIndexerConfig::isOCREnable()
{
    QMutexLocker locker(&m_mutex);
    if(m_gsettings) {
        if(m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_OCR_KEY)) {
            return m_gsettings->get(CONTENT_INDEX_ENABLE_OCR_KEY).toBool();
        } else {
            qWarning() << "FileIndexerConfig: Can not find key:" << CONTENT_INDEX_ENABLE_OCR_KEY << "in" << UKUI_SEARCH_SCHEMAS;
            return false;
        }
    } else {
        qWarning() << "FileIndexerConfig:" << UKUI_SEARCH_SCHEMAS << " is not found!";
        return false;
    }
}

bool FileIndexerConfig::isMetaDataIndexEnable()
{
    return m_settings->value(META_DATA_INDEX_ENABLE_KEY, true).toBool();
}

bool FileIndexerConfig::isAiIndexEnable()
{
    QMutexLocker locker(&m_mutex);
    if(m_gsettings) {
        if(m_gsettings->keys().contains(AI_INDEX_ENABLE_KEY)) {
            return m_gsettings->get(AI_INDEX_ENABLE_KEY).toBool();
        } else {
            qWarning() << "FileIndexerConfig: Can not find key:" << AI_INDEX_ENABLE_KEY << "in" << UKUI_SEARCH_SCHEMAS;
            return false;
        }
    } else {
        qWarning() << "FileIndexerConfig:" << UKUI_SEARCH_SCHEMAS << " is not found!";
        return false;
    }
}

QMap<QString, bool> FileIndexerConfig::contentIndexTarget()
{
    QMutexLocker locker(&m_mutex);
    return m_targetFileTypeMap;
}

QMap<QString, bool> FileIndexerConfig::ocrContentIndexTarget()
{
    QMutexLocker locker(&m_mutex);
    return m_targetPhotographTypeMap;
}

QMap<QString, bool> FileIndexerConfig::aiIndexFileTarget()
{
    QMutexLocker locker(&m_mutex);
    return m_sAiFileTypeMap;
}

QMap<QString, bool> FileIndexerConfig::aiIndexImageTarget()
{
    QMutexLocker locker(&m_mutex);
    return m_sAiImageTypeMap;
}

void FileIndexerConfig::syncIndexFileTypes(bool useDefaultSettings)
{
    QMutexLocker locker(&m_mutex);
    m_settings->sync();
    //文本内容
    m_settings->beginGroup(CONTENT_INDEX_TARGET_TYPE);
    if(m_settings->allKeys().isEmpty() || useDefaultSettings) {
        m_settings->remove("");
        for(const QString &type : m_targetFileTypeMap.keys()) {
            m_settings->setValue(type, m_targetFileTypeMap[type]);
        }
    } else {
        m_targetFileTypeMap.clear();
        for(const QString &type : m_settings->allKeys()) {
            m_targetFileTypeMap.insert(type, m_settings->value(type).toBool());
        }
    }
    m_settings->endGroup();
    //图片ocr
    m_settings->beginGroup(OCR_CONTENT_INDEX_TARGET_TYPE);
    if(m_settings->allKeys().isEmpty() || useDefaultSettings) {
        m_settings->remove("");
        for(const QString &type : m_targetPhotographTypeMap.keys()) {
            m_settings->setValue(type, m_targetPhotographTypeMap[type]);
        }
    } else {
        m_targetPhotographTypeMap.clear();
        for(const QString &type : m_settings->allKeys()) {
            m_targetPhotographTypeMap.insert(type, m_settings->value(type).toBool());
        }
    }
    m_settings->endGroup();
    //“AI文本内容”
    m_settings->beginGroup(S_AI_FILE_TYPE);
    if(m_settings->allKeys().isEmpty() || useDefaultSettings) {
        m_settings->remove("");
        for(const QString &type : m_sAiFileTypeMap.keys()) {
            m_settings->setValue(type, m_sAiFileTypeMap[type]);
        }
    } else {
        m_sAiFileTypeMap.clear();
        for(const QString &type : m_settings->allKeys()) {
            m_sAiFileTypeMap.insert(type, m_settings->value(type).toBool());
        }
    }
    m_settings->endGroup();
    //“AI图片”
    m_settings->beginGroup(S_AI_IMAGE_TYPE);
    if(m_settings->allKeys().isEmpty() || useDefaultSettings) {
        m_settings->remove("");
        for(const QString &type : m_sAiImageTypeMap.keys()) {
            m_settings->setValue(type, m_sAiImageTypeMap[type]);
        }
    } else {
        m_sAiImageTypeMap.clear();
        for(const QString &type : m_settings->allKeys()) {
            m_sAiImageTypeMap.insert(type, m_settings->value(type).toBool());
        }
    }
    m_settings->endGroup();
}