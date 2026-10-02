#ifndef STARTUPTOOL_H
#define STARTUPTOOL_H

#include <QObject>
#include <QString>
#include <QMap>
#include "publicattributes.hpp"
class StartUpTools : public QObject
{
    Q_OBJECT
public:
    static StartUpTools *getInstance();

    void loadConfig();
    void setModel(PublicAttributes::Model model);
    QString findKey(const QString &key);
    void startUp(const QString &path);
    static QMap<QString, QString> initConfigNomalMap();
    static QMap<QString, QString> initConfigTabletMap();

private:
    StartUpTools(QObject *parent = nullptr){};

    PublicAttributes::Model m_model = PublicAttributes::Model::PcModel;

    static QMap<QString, QString> s_nomalMap;
    static QMap<QString, QString> s_tabletMap;
};

#endif  // STARTUPTOOL_H
