#include <glib.h>
#include <gio/gio.h>
#include <mutex>
#include <QDebug>
#include <QVariant>

#include "panel-gsettings.h"

namespace UkuiPanel {

static std::once_flag onceFlag;
static PanelGSettings* g_instance = nullptr;
static GSettings * m_settings = nullptr;
static GSettingsSchema * m_schema = nullptr;
static const char *m_panelLengthKey = "panellength";
UkuiPanel::PanelGSettings *UkuiPanel::PanelGSettings::instance()
{
    std::call_once(onceFlag, [ & ] {
        g_instance = new PanelGSettings();
    });
    return g_instance;
}

int PanelGSettings::getPanelLength(QString screenName)
{
    if (!m_settings || !m_schema) return -1;
    if (!isKeysContain(m_panelLengthKey)) return -1;

    QMap<QString, QVariant> map = getPanelLengthMap();
    if (!map.contains(screenName)) {
        return -1;
    }
    return map.value(screenName).toInt();
}

void PanelGSettings::setPanelLength(QString screenName, int length)
{
    if (!m_settings || !m_schema) return;
    if (!isKeysContain(m_panelLengthKey)) return;

    QMap<QString, QVariant> map = getPanelLengthMap();
    map.insert(screenName, QVariant(length));

    GVariantBuilder builder;
    QMapIterator<QString, QVariant> it(map);

    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);

    while (it.hasNext()) {
        it.next();
        GVariant *gvar;
        QByteArray key = it.key().toUtf8();
        if (it.value().canConvert(QVariant::UInt)) {
            gvar = g_variant_new_uint32(it.value().toUInt());
        }

        g_variant_builder_add (&builder, "{sv}", key.constData(), gvar);

    }
    GVariant* result = g_variant_builder_end (&builder);

    g_settings_set_value(m_settings, m_panelLengthKey, result);
}

void PanelGSettings::setPanelType(int type)
{
    if (!m_qgSettings || !m_qgSettings->keys().contains(QStringLiteral("paneltype"))) return;

    m_qgSettings->set(QStringLiteral("paneltype"), QVariant(type));
}

void PanelGSettings::setIslandPosition(QString islandId, int islandPosition)
{
    if (!m_qgSettings) return;
    if ((islandId == "org.ukui.dataIsland")
            && (m_qgSettings->keys().contains(QStringLiteral("dataislandposition")))) {
        m_qgSettings->set(QStringLiteral("dataislandposition"), QVariant(islandPosition));
    }
    if ((islandId == "org.ukui.settingsIsland")
            && (m_qgSettings->keys().contains(QStringLiteral("settingsislandposition")))) {
        m_qgSettings->set(QStringLiteral("settingsislandposition"), QVariant(islandPosition));
    }
}

void PanelGSettings::setTopBarSize(int size)
{
    if (!m_qgSettings || !m_qgSettings->keys().contains(QStringLiteral("topbarsize"))) return;

    m_qgSettings->set(QStringLiteral("topbarsize"), QVariant(size));
}

UkuiPanel::PanelGSettings::~PanelGSettings()
{
    if (m_settings) {
        g_object_unref(m_settings);
    }
    if (m_schema) {
        g_settings_schema_unref(m_schema);
    }

    g_instance = nullptr;
}

UkuiPanel::PanelGSettings::PanelGSettings(QObject *parent) : QObject(parent)
{
    GSettingsSchemaSource *source;

    source = g_settings_schema_source_get_default();
    m_schema = g_settings_schema_source_lookup(source, "org.ukui.panel.settings", true);

    if (!m_schema) {
        m_settings = nullptr, m_qgSettings = nullptr;
        return;
    }

    m_settings = g_settings_new_with_path("org.ukui.panel.settings", "/org/ukui/panel/settings/");

    if (isKeysContain(m_panelLengthKey)) {
        GVariant *empty_a_sv = g_variant_new_array(G_VARIANT_TYPE("{sv}"), NULL, 0);
        g_settings_set_value(m_settings, m_panelLengthKey, empty_a_sv);
    }

    m_qgSettings = new QGSettings("org.ukui.panel.settings", "/org/ukui/panel/settings/", this);
}

QMap<QString, QVariant> PanelGSettings::getPanelLengthMap()
{
    GVariant *gvalue = g_settings_get_value(m_settings, m_panelLengthKey);

    GVariantIter iter;
    QMap<QString, QVariant> map;
    const gchar *key;
    size_t str_len;
    GVariant *val = NULL;
    g_variant_iter_init (&iter, gvalue);
    QVariant qvar;

    while (g_variant_iter_next (&iter, "{&sv}", &key, &val)) {
        if (g_variant_is_of_type(val, G_VARIANT_TYPE_UINT32)) {
            qvar = QVariant::fromValue(static_cast<quint32>(g_variant_get_uint32(val)));
            map.insert(key, qvar);
        }
    }

    g_variant_unref(gvalue);

    return map;
}

bool PanelGSettings::isKeysContain(const char *key)
{
    if (!m_settings || !m_schema) return false;

    gchar **keys = g_settings_schema_list_keys(m_schema);
    if (g_strv_contains(keys, key)) {
        g_strfreev(keys);
        return true;
    } else {
        g_strfreev(keys);
        return false;
    }
}

} // UkuiPanel
