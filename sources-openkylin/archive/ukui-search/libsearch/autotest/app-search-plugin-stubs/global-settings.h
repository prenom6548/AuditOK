#ifndef GLOBAL_SETTINGS_STUB_H
#define GLOBAL_SETTINGS_STUB_H

#include <QObject>
#include <QVariant>

namespace UkuiSearch {

const static QString ICON_THEME_KEY = "iconThemeName";
const static QString STYLE_NAME_KEY = "styleName";

class GlobalSettings : public QObject
{
    Q_OBJECT
public:
    static GlobalSettings &getInstance()
    {
        static GlobalSettings settings;
        return settings;
    }

    const QVariant getValue(const QString &key)
    {
        Q_UNUSED(key)
        return {};
    }

Q_SIGNALS:
    void valueChanged(const QString &key, QVariant value);

private:
    explicit GlobalSettings(QObject *parent = nullptr) : QObject(parent) {}
};

}

#endif
