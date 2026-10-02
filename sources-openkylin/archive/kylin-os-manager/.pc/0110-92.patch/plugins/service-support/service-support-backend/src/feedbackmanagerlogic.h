#ifndef FEEDBACKMANAGERLOGIC_H
#define FEEDBACKMANAGERLOGIC_H

#include <QProcess>
#include <QDBusInterface>
#include "src/informationclassitem.h"
#include "../constant.h"

class QHttpMultiPart;
class QNetworkAccessManager;
class QNetworkReply;
class FeedbackManagerLogic : public QObject
{
    Q_OBJECT

public slots:
    void startCollect();
    void cancel();

public:
    FeedbackManagerLogic(const ModuleTypeSet &set, InformationUserStruct userData, InformationClassItemList allItems,
                         const QString &savePath, bool *cancel, bool isRetry = false);
    ~FeedbackManagerLogic();
    static QString getCmdMessage(const QString &message, bool hasType = false);

private slots:
    void onProcFinish(int code);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void uploadFinish();

private:
    void creatPackage();
    void getProgress();
    void collecting();
    void saveUserData();
    void uploadData();
    void finish(FeedBackFinishType type, QString str = "");
    void collectingFile(const QString &saveDir, const QString &message, int detailed);
    void collectingCmd(const QString &saveDir, const QString &message);
    void collectingOther(const QString &saveDir, const QString &message, const QString &name);
    void appendHttpPart(QHttpMultiPart *multiPart, const QString &name, const QString &value);
    QString getPathFromJson(QString fn, QString k1, QString v1, QString keyRes, QString k2 = "", QString v2 = "");
    QStringList getFileNameFromDir(const QString &arg) const;
    void Clear();
    void ClearCache();

    QProcess *m_proc = nullptr;
    QDBusInterface *m_dbus = nullptr;
    QNetworkAccessManager *m_networkManager = nullptr;
    QNetworkReply *m_networkReply = nullptr;
    InformationUserStruct m_userData;
    ModuleTypeSet m_typeSet;
    QString m_savePath;
    QString m_tmpPath;
    QString m_timeStr;
    InformationClassItemList m_allItems;
    QTime *m_time = nullptr;
    bool *m_cancel;
    bool m_cancelSuccess = false;
    bool m_isRetry;
    QByteArray m_posData;
    QString m_beFromKey;

    QString m_cacheDir;

signals:
    void creatFinish(FeedBackFinishType, QString);
    void creatProgress(int);
    void errorMessage(QString);
};

#endif // FEEDBACKMANAGERLOGIC_H
