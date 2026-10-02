#include "theme-style.h"
#include "global-settings.h"

#include <QBrush>
#include <QVariant>
#include <QApplication>

static ThemeStyle *global_instance = nullptr;

ThemeStyle *ThemeStyle::instance()
{
    if (!global_instance) {
        global_instance = new ThemeStyle;
    }
    return global_instance;
}

ThemeStyle::ThemeStyle(QObject *parent) : QObject(parent)
{
    auto globalSettings = Peony::GlobalSettings::getInstance();
    QString widgetThemeName = globalSettings->getValue("widgetThemeName").toString();
    if (widgetThemeName.contains("classical")) {
        setRadius(0);
    } else {
        setRadius(6);
    }
    connect(globalSettings, &Peony::GlobalSettings::valueChanged, this, [=](const QString &key){
        if (key == "widgetThemeName") {
            QString widgetThemeName = globalSettings->getValue("widgetThemeName").toString();
            if (widgetThemeName.contains("classical")) {
                setRadius(0);
            } else {
                setRadius(6);
            }
        }
    });
}

QColor ThemeStyle::highlight()
{
    QVariant light = qApp->property("highlight-active");
    QBrush brush = light.value<QBrush>();
    QColor color = brush.color();
    color.setAlpha(255*0.8);
    return color;
}

QColor ThemeStyle::midlight()
{
    QVariant light = qApp->property("highlight-active");
    QBrush brush = light.value<QBrush>();
    QColor color = brush.color();
    color.setAlpha(255*0.5);
    return color;
}


