#ifndef APPLOGIC_H
#define APPLOGIC_H

#include "constant.h"
#include <libsmbclient.h>
#include <QObject>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QTimer>
#include <QTime>
#include <QDateTime>

// #define SMBC_SAVE_BUFFER_SIZE_MAX __off_t(1024 * 1024 * 256) //保存缓冲区最大值
// #define SMBC_READ_BUFFER_TIMEOUT 300000                      //读取超时时长
//将缓冲区的大小和超时时间缩小4倍，当前大小为64MB 时间1分15秒，处理断网连接和重连时间过长的现象
#define SMBC_SAVE_BUFFER_SIZE_MAX qulonglong(1024 * 1024 * 16) //保存缓冲区最大值
#define SMBC_READ_DIR_TIMEOUT 1000                             //读取目录结构超时时长
#define SMBC_READ_BUFFER_TIMEOUT 37500                         //传输文件读取超时时长

#define ITEM_DATA_TYPE Qt::UserRole + 2           // URL对应的标识
#define ITEM_DATA_NAME Qt::UserRole + 3           // URL对应的名称
#define ITEM_DATA_OPENED Qt::UserRole + 4         //是否加载过子目录
#define ITEM_DATA_CAN_NOT_OPEN Qt::UserRole + 5   //是否存在子目录
#define ITEM_DATA_EXPENDED Qt::UserRole + 6       //是否展开过
#define ITEM_DATA_CHECKBOX_STATE Qt::UserRole + 7 //复选框状态
#define ITEM_DATA_SIZE Qt::UserRole + 8           //文件大小

// mips规避方案（处理samba返回值问题）
struct myStat
  {
    __dev_t st_dev;
    int st_pad1[3];             /* Reserved for st_dev expansion  */
#ifndef __USE_FILE_OFFSET64
    __ino_t st_ino;
#else
    __ino64_t st_ino;
#endif
    __mode_t st_mode;
    __nlink_t st_nlink;
    __uid_t st_uid;
    __gid_t st_gid;
    __dev_t st_rdev;
#if !defined __USE_FILE_OFFSET64
    unsigned int st_pad2[2];    /* Reserved for st_rdev expansion  */
    __off_t st_size;
    long int st_pad3;
#else
    unsigned int st_pad2[3];    /* Reserved for st_rdev expansion  */
    __off64_t st_size;
#endif
#ifdef __USE_XOPEN2K8
    /* Nanosecond resolution timestamps are stored in a format
       equivalent to 'struct timespec'.  This is the type used
       whenever possible but the Unix namespace rules do not allow the
       identifier 'timespec' to appear in the <sys/stat.h> header.
       Therefore we have to handle the use of this header in strictly
       standard-compliant sources special.  */
    struct timespec st_atim;            /* Time of last access.  */
    struct timespec st_mtim;            /* Time of last modification.  */
    struct timespec st_ctim;            /* Time of last status change.  */
# define st_atime st_atim.tv_sec        /* Backward compatibility.  */
# define st_mtime st_mtim.tv_sec
# define st_ctime st_ctim.tv_sec
#else
    __time_t st_atime;                  /* Time of last access.  */
    unsigned long int st_atimensec;     /* Nscecs of last access.  */
    __time_t st_mtime;                  /* Time of last modification.  */
    unsigned long int st_mtimensec;     /* Nsecs of last modification.  */
    __time_t st_ctime;                  /* Time of last status change.  */
    unsigned long int st_ctimensec;     /* Nsecs of last status change.  */
#endif
    __blksize_t st_blksize;
    unsigned int st_pad4;
#ifndef __USE_FILE_OFFSET64
    __blkcnt_t st_blocks;
#else
    __blkcnt64_t st_blocks;
#endif
    int st_pad5[14];
  };

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
    SmbUrlInfo(QByteArray u, uint t,qulonglong s = 0, QByteArray e = "") : url(u), type(t),fileSize(s), result(e) {}
    QByteArray url;
    uint type;
    QByteArray result;
    qulonglong fileSize;
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
    void interactionToUI(int type);
    void timeOutTip();

public slots:
    QStandardItemModel *getStandardItemModel();
    bool *getCancelHandle();
    void connectSmbServer(const SmbConf &sc);
    void treeViewExpanded(const QModelIndex &index);
    void startMigration(const QString &path);
    void selectiveSource(int isWin);
    void itemChanged(QStandardItem *item);
    void resave(bool b);
    void creatLog();
    void startLoad();
    void clearHistory();
    bool useBatScrip();

public:
    AppLogic();

private slots:
    static void auth_fn(const char *server, const char *share, char *workgroup, int wgmaxlen, char *username, int unmaxlen, char *password,
                        int pwmaxlen);
    void getDirsAndFiles(const QByteArray &url, QStandardItem *parentItem, int deep, int dh = 0);
    void getDirsAndFilesChild(const QByteArray &url, QStandardItem *parentItem, int deep);
    void getSavePoint(const QByteArray &url, QStandardItem *parentItem);
    void getFileUrl(const SmbUrlInfo &url);
    bool saveFile(const SmbUrlInfo &url, bool isUserFile = true);
    bool connectSmb(int timOut = SMBC_READ_DIR_TIMEOUT);
    void reconnectSmb(int &dh, int &errorNum, const QByteArray &byt, int number, bool isDir = true, int openType = O_RDONLY);
    void getAllFileUrlOfWindows();
    void getAllFileUrlOfSamba();
    void downloadFile(bool firstTime = true);
    void waitScriptResult();
    void animationPlay();
    void removeWindowsFile();
    qlonglong getFileSize(QByteArray url);
    void initBreakpointResume();
    void saveHistoryProcess(char success, int index = -1);
    int getHistoryProcess();

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
    qulonglong m_saveProgressMax;
    qulonglong m_saveProgressValue;
    QByteArray m_winBatFileName;
    QByteArray m_winTmpFileName;
    QByteArray m_winNamesFileName;
    QByteArray m_winDirsFileName;
    QByteArray m_winOutputFileName;
    QByteArray m_winLogFileName;
    QByteArray m_winFinishFileName;
    QByteArray m_workPath;
    QStringList m_smbDirList;
    QTimer *m_waitScriptResultTimer;
    QString m_configPath;
    QTimer *m_animation;
    int m_animationNumber = 0;
    int m_smbTimeOut = SMBC_READ_DIR_TIMEOUT;
    QString m_historyListFileName;
    QString m_historyProcessFileName;
    qint8 *m_historyProcess = nullptr;
    struct ListToDownloadInfo
    {
        QString dateTime;
        qint64 fileNumber;
        QString ip;
        QString username;
        QString savePath;
    } m_listToDownloadInfo;

    int m_errorNum = 0;
    QDateTime currentDateTime1;
    QDateTime currentDateTime2;
    QString m_customize = "";
};

#endif // APPLOGIC_H
