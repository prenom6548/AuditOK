#ifndef PANELGSETTINGS_H
#define PANELGSETTINGS_H

#include <QObject>
#include <QGSettings>

namespace UkuiPanel {

class PanelGSettings : public QObject
{
    Q_OBJECT
public:
    static PanelGSettings *instance();

    int getPanelLength(QString screenName);
    void setPanelLength(QString screenName, int length);

    void setPanelType(int type = 0);
    void setIslandPosition(QString islandId, int islandPosition);
    void setTopBarSize(int size);

    ~PanelGSettings();

private:
    PanelGSettings(QObject *parent = nullptr);

    QMap<QString, QVariant> getPanelLengthMap();

    bool isKeysContain(const char * key);

    QGSettings *m_qgSettings = nullptr;
};

} // UkuiPanel

#endif // PANELGSETTINGS_H
