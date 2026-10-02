#include <QObject>
#include <QtPlugin>

class UnsupportedPlugin : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukui.search.fixture.UnsupportedPlugin" FILE "unsupported-plugin.json")
};

#include "unsupported-plugin.moc"
