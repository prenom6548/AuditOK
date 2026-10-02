#ifndef FEEDBACKMANAGER_H
#define FEEDBACKMANAGER_H

#include <QObject>
#include "src/informationclassitem.h"
#include "../constant.h"
class QNetworkAccessManager;
class QNetworkReply;
class QTimer;
class FeedbackManager : public QObject
{
    Q_OBJECT
signals:
    //创建压缩包完成
    void creatFinish(FeedBackFinishType, QString);
    //创建压缩包进度
    void creatProgress(int);
    //错误信息
    void errorMessage(QString);
    //上传失败
    void uploadError();
    //开始采集
    void startCollect();
    //反馈历史结果信息
    void historyInfo(const QList<HistoryInfo> &);
    //查询项目结果信息
    void prijectInfo(FeedBackInternalVerifyRes, const QList<ProjectInfo> &);
    //取消
    void cancelSignal();
    //切换模式
    void internalModeChange(InternalMode);

public:
    static FeedbackManager *getInstance();
    //获取日志模块清单
    InformationClassItemList getLogFileItems();
    //获取分类模块清单
    ChooseClassList getChooseClassList();
    //获取分类模块
    ChooseClassItem getChooseClassItem(int code);
    //设置采集类型
    void setTypes(ModuleTypeSet set);
    //设置用户填写区
    void setUserData(const InformationUserStruct &u);
    void setSavePath(const QString &path);
    //开始采集
    void startCollecting();
    //获取历史记录
    void getHistoryData(int page = 1);
    //取消
    void cancel();
    //重新上传
    void retryUpload(QString code);
    //确认问题已解决
    void closeBug(QString id);
    //获取组信息
    QString getGroup();
private slots:
    void getNetWorkDataPri(QNetworkReply *reply);


private:
    FeedbackManager();
    void getHistoryDataPri(const QByteArray &byte);
    void getCloseBugDataPri(const QByteArray &byte);
    void startGetPri(const QByteArray &byte, const QString &address, bool encipher);
    static FeedbackManager *m_feedbackManager;
    QThread *m_thread = nullptr;
    enum netWorkType { BugInfo = 0, CloseBug } m_netWorkType;
    QNetworkAccessManager *m_networkManager = nullptr;
    QNetworkReply *m_reply = nullptr;
    QTimer *m_networkManagerTimer = nullptr;
    InformationUserStruct m_userData;
    ModuleTypeSet m_typeSet;
    QString m_savePath;
    QString m_packageFile;
    InformationClassItemList m_allItems;
    ChooseClassList m_chooseItemList;
    bool *m_cancel = nullptr;
    int m_PageIndex = 1;
    InternalMode m_internalMode = OpenKylin;
};

#endif // FEEDBACKMANAGER_H
