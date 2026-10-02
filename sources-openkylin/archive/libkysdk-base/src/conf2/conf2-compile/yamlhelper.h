#ifndef YAMLHELPER_H
#define YAMLHELPER_H
#include <QMap>
#include <QString>
#include <QVariant>

class YamlHelper
{
public:
    YamlHelper();

public:
    QMap<QString, QVariant> loadYamlDirs(const QStringList &dirs = QStringList());
    QMap<QString, QVariant> loadYamlDir(const QString &dir);
    QMap<QString, QVariant> loadYamlFile(const QString &file);

private:
    void mergeMap(QMap<QString, QVariant> &dest, const QMap<QString, QVariant> &src, int mergeType, bool passTypeChanged = "public");

    void overrideConfig(QMap<QString, QVariant> &dest, const QMap<QString, QVariant> &src, bool withoutVersion);
    void groupOverride(QMap<QString, QVariant> &dest, const QString &group, const QMap<QString, QVariant> &groupData);
    void appOverride(QMap<QString, QVariant> &dest, const QString &app, const QMap<QString, QVariant> &appData);
    void appOverrideWithoutVersion(QMap<QString, QVariant> &dest, const QString &app, const QMap<QString, QVariant> &appData);

    void mergeSelfVersion(QMap<QString, QVariant> &src);
};

#endif // YAMLHELPER_H
