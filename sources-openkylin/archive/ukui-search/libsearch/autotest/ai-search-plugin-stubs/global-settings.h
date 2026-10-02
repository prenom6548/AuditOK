#ifndef AI_SEARCH_PLUGIN_GLOBAL_SETTINGS_STUB_H
#define AI_SEARCH_PLUGIN_GLOBAL_SETTINGS_STUB_H

#include <QHash>
#include <QObject>
#include <QVariant>

namespace UkuiSearch {

const static QString ICON_THEME_KEY = "iconThemeName";
const static QString STYLE_NAME_KEY = "styleName";

extern QHash<QString, QVariant> g_globalSettingsValues;

class GlobalSettings : public QObject
{
    Q_OBJECT
public:
    static GlobalSettings &getInstance()
    {
        static GlobalSettings settings;
        return settings;
    }

    QVariant getValue(const QString &key) const
    {
        return g_globalSettingsValues.value(key);
    }

    void setValue(const QString &key, const QVariant &value)
    {
        g_globalSettingsValues.insert(key, value);
    }

Q_SIGNALS:
    void valueChanged(const QString &key, QVariant value = QVariant());

private:
    explicit GlobalSettings(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
};

}  // namespace UkuiSearch

#endif
