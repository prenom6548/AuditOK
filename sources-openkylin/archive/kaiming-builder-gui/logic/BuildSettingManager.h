#ifndef BUILDSETTINGMANAGER_H
#define BUILDSETTINGMANAGER_H
#include <QObject>
#include <QString>

class BuildSettingManager : public QObject
{
    Q_OBJECT
public:
    explicit BuildSettingManager(QObject *parent = nullptr);


    void loadConfig();
    void saveConfig() const;

    QStringList getCommandString() const;

    bool m_buildOnly = false;
    bool m_finishOnly = false;
    bool m_exportOnly = false;
    bool m_forceClean = false;
    bool m_keepSource = false;

private:
    QString getConfigPath() const;

};

#endif // BUILDSETTINGMANAGER_H
