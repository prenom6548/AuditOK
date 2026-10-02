#ifndef LANGUAGEINFO_H
#define LANGUAGEINFO_H
#include <QString>
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>
//sudo apt install libqapt-dev
#include <QApt/DownloadProgress>
#include <QApt/Globals>
#include <QApt/Backend>
#include <QtConcurrentRun>

class LanguageInfo : public QObject{
    Q_OBJECT
public:
    LanguageInfo(QObject *parent = nullptr);
    ~LanguageInfo();
    void init();

private:
    bool isAvailable(QString languageCode);
    bool isInstalled(QString languageCode);
    void readLangInfo(QString localFile = "/usr/share/xml/iso-codes/iso_639_3.xml");
    void getAvailableLanguage();
    void getInstalledLanguage();

private:
    QApt::Backend *m_backend = nullptr;
    QFuture<void> threadRun;
    QMap<QString, QString> localLangMap;
    QMap<QString, QString> availableLangMap;
    QMap<QString, QString> installedLangMap;

Q_SIGNALS:
    void initFinished(QMap<QString, QString> availableLangMap,QMap<QString, QString> installedLangMap);
};

#endif // LANGUAGEINFO_H
