#ifndef MODULEFACTORY_H
#define MODULEFACTORY_H

#include <QObject>
#include <QMetaEnum>
#include <QVector>
#include <QMap>
#include <QPointer>
#include <QStack>
//#include "interface.h"
#include <QDebug>

struct PluginInfo {
    int type;
    int index;
    bool mainShow;
    bool isEnable;
    QString oriNameString;
    QString nameString;
    QString namei18nString;
};

struct ModuleInfo {
    ModuleInfo(QString modulename, int moduleorder,int moduletype,
               QString themeiconname, QString localiconname) {
        clear();
        moduleName = modulename;
        moduleOrder = moduleorder;
        localIconName = localiconname;
        themeIconName = themeiconname;
        moduleType = moduletype;
    }
    ~ModuleInfo() {
        clear();
    }
    void clear() {
        moduleOrder = -1;
        moduleType = -1;
        moduleName.clear();
        moduleLocaleMap.clear();
        pluginNameList.clear();
        pluginInfoList.clear();
        localIconName.clear();
        themeIconName.clear();
    }
    void appendNameLocale(QString key, QString value) {
        moduleLocaleMap.insert(key, value);
    }
    void appendPluginNameList(int index, QString value) {
        pluginNameList.insert(index, value);
    }
    QString getModuleNameLocale(const QString locale) {
        QString moduleName = "";
        if (moduleLocaleMap.find(locale) != moduleLocaleMap.end()) {
            moduleName = moduleLocaleMap[locale];
        }
        return moduleName;
    }
    int moduleOrder;
    int moduleType;
    QString moduleName;
    QString themeIconName;
    QString localIconName;
    QMap<QString, QString> moduleLocaleMap;
    QList<QString> pluginNameList;
    QList<PluginInfo> pluginInfoList;
};

class ModulesFactory : public QObject {
    Q_OBJECT

public:
    explicit ModulesFactory();
    ~ModulesFactory();

public:
    static void loadConfig();
    static void loadPluginInfo(QObject* pluginObj);
    static ModuleInfo* getModuleInfoByIndex(int index);
    static ModuleInfo* getModuleInfoByType(int moduleType);
    static QString getModuleNamebyName(QString pluginName);
    static QString getModuleLocaleNamebyName(QString pluginName);
    static QObject* getPluginObjectByName(QString pluginil8Name);
    static QObject* getPluginObjectByType(int moduleType, QString pluginil8Name);
    static PluginInfo* getPluginInfoByName(QString pluginName);
    static bool checkModuleType(int moduleType);
    static bool checkPluginExist(QString pluginil8Name);
    static void pluginDelete();
    static int size();

private:
    static void clear();
    static bool checkModuleIndex(int index);

private:
    static int totalModule;
    static QVector<ModuleInfo *> moduleInfoVec;
    static QList<int> moduleTypeList;
    static QList<QPair<QString, QString>> pluginToModuleNameList;
    static QList<QPair<QString, QString>> pluginToModuleLocaleNameList;
    static QMap<int, QMap<QString, QPointer<QObject>>> moduleTypeToPluginMap;
};

#endif // MODULEFACTORY_H
