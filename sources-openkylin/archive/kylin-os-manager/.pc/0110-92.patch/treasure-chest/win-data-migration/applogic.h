#ifndef APPLOGIC_H
#define APPLOGIC_H

#include "constant.h"
#include <libsmbclient.h>
#include <QObject>
#include <QStandardItemModel>
#include <QStandardPaths>

// #define SMBC_SAVE_BUFFER_SIZE_MAX __off_t(1024 * 1024 * 256) //保存缓冲区最大值
// #define SMBC_READ_BUFFER_TIMEOUT 300000                      //读取超时时长
//将缓冲区的大小和超时时间缩小4倍，当前大小为64MB 时间1分15秒，处理断网连接和重连时间过长的现象
#define SMBC_SAVE_BUFFER_SIZE_MAX __off_t(1024 * 1024 * 64) //保存缓冲区最大值
#define SMBC_READ_BUFFER_TIMEOUT 75000                      //读取超时时长

#define ITEM_DATA_TYPE Qt::UserRole + 2           // URL对应的标识
#define ITEM_DATA_NAME Qt::UserRole + 3           // URL对应的名称
#define ITEM_DATA_OPENED Qt::UserRole + 4         //是否加载过子目录
#define ITEM_DATA_CAN_NOT_OPEN Qt::UserRole + 5   //是否存在子目录
#define ITEM_DATA_EXPENDED Qt::UserRole + 6       //是否展开过
#define ITEM_DATA_CHECKBOX_STATE Qt::UserRole + 7 //复选框状态


struct SmbConf
{
    QByteArray ip;
    QByteArray workgroup;
    QByteArray username;
    QByteArray password;
    int debugLevel = 0; // 0~10
};

struct SmbUrlInfo
{
    SmbUrlInfo(QByteArray u, uint t, QByteArray e = "") : url(u), type(t), result(e) {}
    QByteArray url;
    uint type;
    QByteArray result;
};
typedef QList<SmbUrlInfo> SmbUrlInfoList;
Q_DECLARE_METATYPE(SmbUrlInfoList)

class AppLogic : public QObject
{
    Q_OBJECT

signals:
    void connetcResult(int, QVariant = QVariant());
    void saveProgressValue(int);
    void saveProgressMax(int);
    void downloadingFile(const QString &str);
    void cancelFileMigrate();

public slots:
    QStandardItemModel *getStandardItemModel();
    bool *getCancelHandle();
    void connectSmbServer(const SmbConf &sc);
    void treeViewExpanded(const QModelIndex &index);
    void startMigration(const QString &path);
    void itemChanged(QStandardItem *item);
    void resave();
    void creatLog();
    void startLoad();

public:
    AppLogic();

private slots:
    static void auth_fn(const char *server, const char *share, char *workgroup, int wgmaxlen, char *username, int unmaxlen, char *password,
                        int pwmaxlen);
    void getDirsAndFiles(const QByteArray &url, QStandardItem *parentItem, int deep, int dh = 0);
    void getDirsAndFilesChild(const QByteArray &url, QStandardItem *parentItem, int deep);
    void getSavePoint(const QByteArray &url, QStandardItem *parentItem);
    void getFileUrl(const SmbUrlInfo &url);
    void saveFile(const SmbUrlInfo &url);
    bool connectSmb();
    void reconnectSmb(int &dh, int &errorNum, const QByteArray &byt, bool isDir = true);

private:
    static const QByteArray g_scheme;
    static const QString g_saveDirName;
    static QByteArray g_server;
    static QByteArray g_username;
    static QByteArray g_password;
    static QByteArray g_workgroup;
    static int g_debugLevel;
    static SMBCCTX *m_smbCtx;
    QStandardItemModel *m_itemModel = nullptr;
    QString m_savePath;
    SmbUrlInfoList m_savePointList;
    SmbUrlInfoList m_saveFileList;
    SmbUrlInfoList m_saveErrorList;
    SmbUrlInfoList m_saveSuccessList;
    QString m_logPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kylin-os-manager/win-data-migration/";
    QString m_logFileName;
    bool *m_cancel = nullptr;
    int m_readChild;
};

#endif // APPLOGIC_H
