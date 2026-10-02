#ifndef FEEDBACKMANAGERLOGIC_H
#define FEEDBACKMANAGERLOGIC_H

#include <QProcess>
#include <QDBusInterface>
#include "src/informationclassitem.h"
#include "../constant.h"

class QHttpMultiPart;
class QNetworkAccessManager;
class QNetworkReply;

class FeedbackManagerLogic : public QObject {
    Q_OBJECT
public:
    FeedbackManagerLogic(const ModuleTypeSet &set, InformationUserStruct userData, InformationClassItemList allItems,
                         const QString &savePath, bool *cancel, bool isRetry = false);
    ~FeedbackManagerLogic();
    void setBeFrom(BeFrom type, const QString &key);
    void setOnlyGenerateLogs(bool ok);

Q_SIGNALS:
    void creatFinish(FeedBackFinishType, QString);
    void creatProgress(int);
    void errorMessage(QString);

public Q_SLOTS:
    // 开始创建工单
    void startCollect();
    void cancel();

private Q_SLOTS:
    void onProcFinish(int code, QProcess::ExitStatus status);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void uploadFinish();

private:
    void creatPackage();
    void getProgress();
    void collecting();
    void saveUserData();
    void saveDebugData();
    void uploadData();
    void finish(FeedBackFinishType type, QString str = "");
    void appendHttpPart(QHttpMultiPart *multiPart, const QString &name, const QString &value);
    void appendVerifyHttpPart(QHttpMultiPart *multiPart);
    void Clear();
    void ClearCache();

    QProcess *               m_proc = nullptr;
    QDBusInterface *         m_dbus = nullptr;
    QNetworkAccessManager *  m_networkManager = nullptr;
    QNetworkReply *          m_networkReply = nullptr;
    InformationUserStruct    m_userData;
    ModuleTypeSet            m_typeSet;
    QString                  m_savePath;
    QString                  m_tmpPath;
    InformationClassItemList m_allItems;
    QTime *                  m_time = nullptr;
    bool *                   m_cancel;
    bool                     m_cancelSuccess = false;
    bool                     m_isRetry;
    QByteArray               m_posData;
    BeFrom                   m_beFrom = FromKOM;
    QString                  m_beFromKey;
    QString                  m_cacheDir;
    bool                     m_onlyGenerateLogs = false;    // 仅生成日志
};

#endif
