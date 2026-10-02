#ifndef MATCHPPDSTHREAD_H
#define MATCHPPDSTHREAD_H

#include <QObject>
#include<QMutexLocker>
#include "findppdsthread.h"

enum class matchRes:int
{
    EXACT_MATCH = 0,
    MATCHED_NORMAL,
    MATCHED_GENERIC,
    INPUT_ERR,
};

class MatchPPDsThread : public QObject
{
    Q_OBJECT
public:
    static MatchPPDsThread* getInstance();

protected:

signals:
    void matchFailed();

    void start();
public slots:
    int initPPDMatch(QString bandName,
                      QString printerName,
                      QString suggestPpdName,
                      ppdPrinterMap data,
                      ppdPrinterMap extra,
                      QString &exactMatchPPD,
                      QStringList &matchResultList);
    void detectStart();
private:
    explicit MatchPPDsThread(QObject *parent = nullptr);
    ~MatchPPDsThread();
    MatchPPDsThread(MatchPPDsThread const&);
    MatchPPDsThread& operator =(MatchPPDsThread const&);

    static QMutex m_mutex;
    QMap<QString, QMap<QString, PPDsAndAttr> > m_originData;

    QString m_printerBandName = nullptr;
    QString m_printerModelName = nullptr;

    int exactMatch(QString printerModel
                   , QMap<QString, PPDsAndAttr> map
                   , QString &exactMatchPPD
                   , QStringList &matchResultList);
    int genericMatch(QString printerModel
                     , QMap<QString, PPDsAndAttr> map
                     , QString &exactMatchPPD
                     , QStringList &matchResultList);

    bool m_exactMatchFlag = false;

    QString originStringHandle(QString originString);

    void modelUniform(QString handledStr, QString& outputStr);
    bool hasThisBandName(ppdPrinterMap extra, QString inputName);
    QString getMfgName(ppdPrinterMap extra, QString inputName);
};

#endif // MATCHPPDSTHREAD_H
