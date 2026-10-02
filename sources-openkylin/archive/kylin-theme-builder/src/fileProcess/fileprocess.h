#ifndef FILEPROCESS_H
#define FILEPROCESS_H

#include <QObject>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QStandardPaths>
#include <QMap>

class FileProcess
{
public:
    FileProcess();

    QMap<QString, QVariant> FileSearch(const QString &m_date);
    void FileCreate(const QString &themes);

    void FileClean(const QString &m_date);

    static QString g_date;
    static QString g_themeCNName;
    static QString g_themeENName;
    static QString g_builderName;
    static QString g_builderMailName;
    static QString g_createThemeType;
    static QString g_debPath;
    static bool g_isSetup;
    static bool g_isBatchImport;

private:
    QDir m_builder;

};

#endif // FILEPROCESS_H
