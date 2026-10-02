/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "decorationbridge.h"
#include "decoratedclient.h"
#include "decorations_logging.h"
#include "settings.h"
// KWin core
#include "abstract_client.h"
#include "wayland/server_decoration_interface.h"
#include "wayland_server.h"
#include "workspace.h"
#include <config-kwin.h>

// KDecoration
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/Decoration>
#include <KDecoration2/DecorationSettings>

// Frameworks
#include <KPluginFactory>
#include <KPluginMetaData>

// Qt
#include <QFontDatabase>
#include <QGSettings>
#include <QMetaProperty>
#include <QPainter>
#include <QScreen>
#include <QtDBus>

namespace KWin
{
namespace Decoration
{

static const QString s_aurorae = QStringLiteral("org.kde.kwin.aurorae");
static const QString s_pluginName = QStringLiteral("org.kde.kdecoration2");
#if HAVE_BREEZE_DECO
static const QString s_defaultPlugin = QStringLiteral(BREEZE_KDECORATION_PLUGIN_ID);
#else
static const QString s_defaultPlugin = s_aurorae;
#endif

KWIN_SINGLETON_FACTORY(DecorationBridge)

DecorationBridge::DecorationBridge(QObject *parent)
    : KDecoration2::DecorationBridge(parent)
    , m_factory(nullptr)
    , m_blur(false)
    , m_showToolTips(false)
    , m_settings()
    , m_noPlugin(false)
{
    readDecorationOptions();
}

DecorationBridge::~DecorationBridge()
{
    s_self = nullptr;
}

QString DecorationBridge::readPlugin()
{
    return kwinApp()->config()->group(s_pluginName).readEntry("library", s_defaultPlugin);
}

static bool readNoPlugin()
{
    return kwinApp()->config()->group(s_pluginName).readEntry("NoPlugin", false);
}

QString DecorationBridge::readTheme() const
{
    return kwinApp()->config()->group(s_pluginName).readEntry("theme", m_defaultTheme);
}

void DecorationBridge::readDecorationOptions()
{
    m_showToolTips = kwinApp()->config()->group(s_pluginName).readEntry("ShowToolTips", true);
}

bool DecorationBridge::hasPlugin()
{
    const DecorationBridge *bridge = DecorationBridge::self();
    if (!bridge) {
        return false;
    }
    return !bridge->m_noPlugin && bridge->m_factory;
}

void DecorationBridge::init()
{
    using namespace KWaylandServer;
    m_noPlugin = readNoPlugin();
    if (m_noPlugin) {
        if (waylandServer()) {
            waylandServer()->decorationManager()->setDefaultMode(ServerSideDecorationManagerInterface::Mode::None);
        }
        return;
    }
    m_plugin = readPlugin();
    m_settings = QSharedPointer<KDecoration2::DecorationSettings>::create(this);
    initPlugin();
    if (!m_factory) {
        if (m_plugin != s_defaultPlugin) {
            // try loading default plugin
            m_plugin = s_defaultPlugin;
            initPlugin();
        }
        // default plugin failed to load, try fallback
        if (!m_factory) {
            m_plugin = s_aurorae;
            initPlugin();
        }
    }
    if (waylandServer()) {
        waylandServer()->decorationManager()->setDefaultMode(m_factory ? ServerSideDecorationManagerInterface::Mode::Server : ServerSideDecorationManagerInterface::Mode::None);
    }

    // 读取dpi值
    m_dpi = 96;
    QScreen *primary = QGuiApplication::primaryScreen();
    if (primary) {
        m_dpi = primary->logicalDotsPerInchX();
        if (m_dpi < 30) {
            m_dpi = 30; // 设置一个限度，不允许dpi小于30
        }
    }

    QString strTheme, strIconTheme;
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings *pThemeSettings = new QGSettings("org.ukui.style", "/org/ukui/style/", this);
        if (true == pThemeSettings->keys().contains("styleName")) {
            strTheme = pThemeSettings->get("style-name").toString();
            qDebug() << "DecorationBridge::init theme:" << strTheme;
        }
        if (true == pThemeSettings->keys().contains("iconThemeName")) {
            strIconTheme = pThemeSettings->get("icon-theme-name").toString();
            qDebug() << "DecorationBridge::init icon theme:" << strIconTheme;
        }
        if (pThemeSettings)
            delete pThemeSettings;
        pThemeSettings = nullptr;
    }

    if ("ukui-light" == strTheme) {
        m_themeId = 0;
    } else if ("ukui-dark" == strTheme) {
        m_themeId = 1;
    } else {
        // 默认ukui-default
        m_themeId = 0;
    }

    if (strIconTheme == "ukui-icon-theme-fashion") {
        m_iconTheme = 1;
    } else if (strIconTheme == "ukui-icon-theme-classical") {
        m_iconTheme = 2;
    } else {
        m_iconTheme = 0;
    }

    const QFont font = QFontDatabase::systemFont(QFontDatabase::TitleFont);
    m_nFont = font.pointSize();
    m_strFontFamily = font.family();

    QDBusConnection::sessionBus().connect(QString(),
                                          QStringLiteral("/KGlobalSettings"),
                                          QStringLiteral("org.kde.KGlobalSettings"),
                                          QStringLiteral("slotThemeChange"),
                                          this, SLOT(slotThemeUpdate(int)));

    QDBusConnection::sessionBus().connect(QString(),
                                          QStringLiteral("/KGlobalSettings"),
                                          QStringLiteral("org.kde.KGlobalSettings"),
                                          QStringLiteral("slotIconThemeChange"),
                                          this, SLOT(slotIconThemeUpdate(int)));

    QDBusConnection::sessionBus().connect(QString(),
                                          QStringLiteral("/KGlobalSettings"),
                                          QStringLiteral("org.kde.KGlobalSettings"),
                                          QStringLiteral("slotFontChange"),
                                          this, SLOT(fontUpdate(int, QString)));

    m_bTabletMode = false;
    // kwin启动时获取运行模式: 0:PC模式, 1:平板模式
    QDBusMessage message = QDBusMessage::createMethodCall("com.kylin.statusmanager.interface",
                                                          "/",
                                                          "com.kylin.statusmanager.interface",
                                                          "get_current_tabletmode");

    QDBusMessage response = QDBusConnection::sessionBus().call(message);

    if (response.type() == QDBusMessage::ReplyMessage) {
        m_bTabletMode = response.arguments().takeFirst().toBool();
        // printf("TabletMode = %d\n", m_bTabletMode);
    } else {
        printf("TabletMode qdus invalid\n");
    }
    // 通过dbus监听PC-平板模式切换
    QDBusConnection::sessionBus().connect("com.kylin.statusmanager.interface",
                                          "/",
                                          "com.kylin.statusmanager.interface",
                                          "mode_change_signal",
                                          this,
                                          SLOT(slotSetSurface(bool)));
}

void DecorationBridge::slotThemeUpdate(int themeId)
{
    m_themeId = themeId;
}

void DecorationBridge::fontUpdate(int nfont, QString strFamily)
{
    m_nFont = nfont;
    m_strFontFamily = strFamily;
    QFont font;
    font.setPointSize(nfont);
    font.setFamily(strFamily);

    Q_EMIT sig_updateFont(font);
}

void DecorationBridge::slotIconThemeUpdate(int themeId)
{
    m_iconTheme = themeId;
}

void DecorationBridge::slotSetSurface(bool bSurface)
{
    if (m_bTabletMode == bSurface) {
        return;
    }
    m_bTabletMode = bSurface;
}

void DecorationBridge::initPlugin()
{
    const KPluginMetaData metaData = KPluginMetaData::findPluginById(s_pluginName, m_plugin);
    if (!metaData.isValid()) {
        qCWarning(KWIN_DECORATIONS) << "Could not locate decoration plugin" << m_plugin;
        return;
    }
    qCDebug(KWIN_DECORATIONS) << "Trying to load decoration plugin: " << metaData.fileName();
    auto factoryResult = KPluginFactory::loadFactory(metaData);
    if (!factoryResult) {
        qCWarning(KWIN_DECORATIONS) << "Error loading plugin:" << factoryResult.errorText;
    } else {
        m_factory = factoryResult.plugin;
        loadMetaData(metaData.rawData());
    }
}

static void recreateDecorations()
{
    Workspace::self()->forEachAbstractClient([](AbstractClient *c) {
        c->invalidateDecoration();
    });
}

void DecorationBridge::reconfigure()
{
    readDecorationOptions();

    if (m_noPlugin != readNoPlugin()) {
        m_noPlugin = !m_noPlugin;
        // no plugin setting changed
        if (m_noPlugin) {
            // decorations disabled now
            m_plugin = QString();
            delete m_factory;
            m_factory = nullptr;
            m_settings.clear();
        } else {
            // decorations enabled now
            init();
        }
        recreateDecorations();
        return;
    }

    const QString newPlugin = readPlugin();
    if (newPlugin != m_plugin) {
        // plugin changed, recreate everything
        auto oldFactory = m_factory;
        const auto oldPluginName = m_plugin;
        m_plugin = newPlugin;
        initPlugin();
        if (m_factory == oldFactory) {
            // loading new plugin failed
            m_factory = oldFactory;
            m_plugin = oldPluginName;
        } else {
            recreateDecorations();
            // TODO: unload and destroy old plugin
        }
    } else {
        // same plugin, but theme might have changed
        const QString oldTheme = m_theme;
        m_theme = readTheme();
        if (m_theme != oldTheme) {
            recreateDecorations();
        }
    }
}

void DecorationBridge::loadMetaData(const QJsonObject &object)
{
    // reset all settings
    m_blur = false;
    m_recommendedBorderSize = QString();
    m_theme = QString();
    m_defaultTheme = QString();

    // load the settings
    const QJsonValue decoSettings = object.value(s_pluginName);
    if (decoSettings.isUndefined()) {
        // no settings
        return;
    }
    const QVariantMap decoSettingsMap = decoSettings.toObject().toVariantMap();
    auto blurIt = decoSettingsMap.find(QStringLiteral("blur"));
    if (blurIt != decoSettingsMap.end()) {
        m_blur = blurIt.value().toBool();
    }
    auto recBorderSizeIt = decoSettingsMap.find(QStringLiteral("recommendedBorderSize"));
    if (recBorderSizeIt != decoSettingsMap.end()) {
        m_recommendedBorderSize = recBorderSizeIt.value().toString();
    }
    findTheme(decoSettingsMap);

    Q_EMIT metaDataLoaded();
}

void DecorationBridge::findTheme(const QVariantMap &map)
{
    auto it = map.find(QStringLiteral("themes"));
    if (it == map.end()) {
        return;
    }
    if (!it.value().toBool()) {
        return;
    }
    it = map.find(QStringLiteral("defaultTheme"));
    m_defaultTheme = it != map.end() ? it.value().toString() : QString();
    m_theme = readTheme();
}

std::unique_ptr<KDecoration2::DecoratedClientPrivate> DecorationBridge::createClient(KDecoration2::DecoratedClient *client, KDecoration2::Decoration *decoration)
{
    return std::unique_ptr<DecoratedClientImpl>(new DecoratedClientImpl(static_cast<AbstractClient *>(decoration->parent()), client, decoration));
}

std::unique_ptr<KDecoration2::DecorationSettingsPrivate> DecorationBridge::settings(KDecoration2::DecorationSettings *parent)
{
    return std::unique_ptr<SettingsImpl>(new SettingsImpl(parent));
}

KDecoration2::Decoration *DecorationBridge::createDecoration(AbstractClient *client)
{
    if (m_noPlugin) {
        return nullptr;
    }
    if (!m_factory) {
        return nullptr;
    }
    QVariantMap args({{QStringLiteral("bridge"), QVariant::fromValue(this)}});

    // use right-to-left layout for some languages in decorations
    const QString systemLang = QLocale::system().name();
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        QGuiApplication::setLayoutDirection(Qt::RightToLeft);
    } else {
        QGuiApplication::setLayoutDirection(Qt::LeftToRight);
    }

    if (!m_theme.isEmpty()) {
        args.insert(QStringLiteral("theme"), m_theme);
    }
    args.insert(QStringLiteral("dpi"), m_dpi); // 每创建一个渲染端，就把dpi值带过去，后面每新建一个客户就不需要反复获取获取dpi值
    args.insert(QStringLiteral("themeId"), m_themeId); // 针对UKUI定制的主题id
    args.insert(QStringLiteral("systemFontSize"), m_nFont); // 标题栏字体大小
    args.insert(QStringLiteral("systemFont"), m_strFontFamily); // 标题栏字体类型
    args.insert(QStringLiteral("tabletMode"), m_bTabletMode);
    args.insert(QStringLiteral("iconTheme"), m_iconTheme);
    auto deco = m_factory->create<KDecoration2::Decoration>(client, QVariantList({args}));
    bool useUKUIDeco = client->property("ukui-decoration").toBool();
    deco->setProperty("ukui-decoration", useUKUIDeco);
    deco->setProperty("hasDecoration", true);
    QVector4D ubr = qvariant_cast<QVector4D>(client->property("ubr"));
    deco->setProperty("ubr", ubr);
    deco->setProperty("isWaylandDeco", client->surface() ? true : false);
    deco->setSettings(m_settings);
    deco->init();
    return deco;
}

static QString settingsProperty(const QVariant &variant)
{
    if (QLatin1String(variant.typeName()) == QLatin1String("KDecoration2::BorderSize")) {
        return QString::number(variant.toInt());
    } else if (QLatin1String(variant.typeName()) == QLatin1String("QVector<KDecoration2::DecorationButtonType>")) {
        const auto &b = variant.value<QVector<KDecoration2::DecorationButtonType>>();
        QString buffer;
        for (auto it = b.begin(); it != b.end(); ++it) {
            if (it != b.begin()) {
                buffer.append(QStringLiteral(", "));
            }
            buffer.append(QString::number(int(*it)));
        }
        return buffer;
    }
    return variant.toString();
}

QString DecorationBridge::supportInformation() const
{
    QString b;
    if (m_noPlugin) {
        b.append(QStringLiteral("Decorations are disabled"));
    } else {
        b.append(QStringLiteral("Plugin: %1\n").arg(m_plugin));
        b.append(QStringLiteral("Theme: %1\n").arg(m_theme));
        b.append(QStringLiteral("Plugin recommends border size: %1\n").arg(m_recommendedBorderSize.isNull() ? "No" : m_recommendedBorderSize));
        b.append(QStringLiteral("Blur: %1\n").arg(m_blur));
        const QMetaObject *metaOptions = m_settings->metaObject();
        for (int i = 0; i < metaOptions->propertyCount(); ++i) {
            const QMetaProperty property = metaOptions->property(i);
            if (QLatin1String(property.name()) == QLatin1String("objectName")) {
                continue;
            }
            b.append(QStringLiteral("%1: %2\n").arg(property.name(), settingsProperty(m_settings->property(property.name()))));
        }
    }
    return b;
}

} // Decoration
} // KWin
