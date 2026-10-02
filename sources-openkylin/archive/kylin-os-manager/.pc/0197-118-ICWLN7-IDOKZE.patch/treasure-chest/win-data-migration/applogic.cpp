#include "applogic.h"
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <QTime>
#include <QFontMetrics>
#include <QEventLoop>
#include <QSettings>
#include "kom-configure.h"
#include "batscript.h"

#define RETRY_NUMBER 3
#define PROGRESS_MAX 100

const QByteArray AppLogic::g_scheme = "smb://";
const QString AppLogic::g_saveDirName = "win-migration";
QByteArray AppLogic::g_server = "";
QByteArray AppLogic::g_username = "";
QByteArray AppLogic::g_password = "";
QByteArray AppLogic::g_workgroup = "";
int AppLogic::g_debugLevel = 0;
SMBCCTX *AppLogic::m_smbCtx = nullptr;

void AppLogic::auth_fn(const char *server, const char *share, char *workgroup, int wgmaxlen, char *username, int unmaxlen, char *password,
                       int pwmaxlen)
{
    Q_UNUSED(server);
    Q_UNUSED(share);
    strncpy(workgroup, g_workgroup.data(), wgmaxlen);
    strncpy(username, g_username.data(), unmaxlen);
    strncpy(password, g_password.data(), pwmaxlen);
}

AppLogic::AppLogic()
{
    kom::Configure configure;
    m_readChild = configure.value(CONFIG_CHEST_WIN_DATA_MIGRATION, CONFIG_CHEST_WIN_DATA_MIGRATION_READ_DIR_CHILD, false).toInt();
    m_customize = configure.value(CONFIG_PLUGIN_GLOBAL, CONFIG_PLUGIN_GLOBAL_CUSTOMIZE).toString();

    if (m_readChild <= 0) {
        m_readChild = 1;
    }
    qDebug() << "子目录深度：" << m_readChild;

    m_winBatFileName = "_KomGetDirTree.bat";
    m_winOutputFileName = "_KomDirTree.txt";
    m_winNamesFileName = "_KomSmbNameList.txt";
    m_winDirsFileName = "_KomSmbDirList.txt";
    m_winLogFileName = "_KomSmbLog.txt";
    m_winFinishFileName = "_KomSmbFinish.txt";
    m_winTmpFileName = "_KomTmpFile.txt";
    m_configPath =
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kylin-os-manager/win-data-migration/";
    m_historyListFileName = m_configPath + "_historyList";
    m_historyProcessFileName = m_configPath + "_historyProcessFileName";
    m_waitScriptResultTimer = new QTimer(this);
    m_animation = new QTimer(this);
    m_animation->setInterval(1000);
    connect(m_animation, &QTimer::timeout, this, &AppLogic::animationPlay);
    connect(m_waitScriptResultTimer, &QTimer::timeout, this, &AppLogic::waitScriptResult);
}

bool AppLogic::connectSmb(int timOut)
{
    qDebug() << "建立连接 " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    //建立smb连接
    m_smbCtx = smbc_new_context();
    if (!m_smbCtx) {
        return false;
    }
    smbc_setDebug(m_smbCtx, g_debugLevel);
    smbc_setFunctionAuthData(m_smbCtx, auth_fn);
    smbc_setTimeout(m_smbCtx, timOut);
    if (!smbc_init_context(m_smbCtx)) {
        smbc_free_context(m_smbCtx, false);
        return false;
    }
    //设置context到smb
    auto *old = smbc_set_context(m_smbCtx);
    if (old) {
        smbc_free_context(old, 1);
    }
    //绑定回调函数
    //    smbc_setFunctionCheckServer(m_smbCtx, serverCallBack);
    return true;
}

void AppLogic::connectSmbServer(const SmbConf &sc)
{
    g_server = sc.ip;
    g_workgroup = sc.workgroup;
    g_username = sc.username;
    g_password = sc.password;
    g_debugLevel = sc.debugLevel;
    m_itemModel->clear();
    if (!connectSmb()) {
        qDebug() << "建立连接失败 " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        emit connetcResult(SMB_ERROR_2);
        return;
    }
    //确认URL是否可达
    QByteArray url = g_scheme + g_server;
    int dh = smbc_opendir(url.data());
    smbc_closedir(dh);
    if (dh < 0) {
        emit connetcResult(SMB_ERROR_2);
        return;
    }
    QStandardItem *item = new QStandardItem;
    item->setText(g_server);
    item->setIcon(QIcon::fromTheme("system"));
    m_itemModel->setHorizontalHeaderItem(0, item);
    emit connetcResult(SMB_CONNECT_SUCCESS);
    if (m_customize == "icbc") {
        qDebug()<<"it is icbc env";
        m_listToDownloadInfo.username = sc.username;
        m_listToDownloadInfo.ip = sc.ip;
        QFile history(m_historyListFileName);
        bool hasHistory = false;
        // if (history.exists() && QFile(m_historyProcessFileName).exists()) {
        if (history.exists()) {
            if (!history.open(QIODevice::ReadOnly | QIODevice::Text)) {
                return;
            }
            QByteArray splitType = " - : - ";
            QString header = history.readLine();
            QStringList headerList = header.split(splitType);
            if (headerList.length() < 5) {
                return;
            }
            if (headerList.at(2) == m_listToDownloadInfo.ip && headerList.at(3) == m_listToDownloadInfo.username) {
                qDebug()<<"检测到异常中断的迁移操作";
                m_listToDownloadInfo.dateTime = headerList.at(0);
                m_listToDownloadInfo.fileNumber = headerList.at(1).toInt();
                m_listToDownloadInfo.savePath = headerList.at(4);
                m_saveFileList.clear();
                emit connetcResult(SMB_START_BACKPOINT_RESUME);

                animationPlay();
                m_animation->start();
                while (!history.atEnd()) {
                    QString data = history.readLine();
                    if (data.endsWith('\n')) {
                        data.chop(1);
                    }
                    QStringList dataList = data.split(splitType);
                    if (dataList.length() < 2) {
                        continue;
                    }
                    SmbUrlInfo info(dataList.first().toLocal8Bit(), SMBC_FILE, dataList.last().toULongLong());
                    m_saveFileList.append(info);
                    hasHistory = true;
                    QEventLoop().processEvents();
                }
                m_animation->stop();
            }
            history.close();
            if (hasHistory) {
                m_savePath = m_listToDownloadInfo.savePath;
                qDebug()<<"开始断点续传";
                downloadFile(false);
            }
        }
    }
}

void AppLogic::startLoad()
{
    getDirsAndFiles(g_scheme + g_server, m_itemModel->invisibleRootItem(), m_readChild);
}


void AppLogic::getDirsAndFiles(const QByteArray &url, QStandardItem *parentItem, int deep, int dh)
{
    //读取过的目录不再重复读取,仅遍历子项加载
    if (parentItem->data(ITEM_DATA_OPENED).toBool()) {
        if (dh > 0) {
            smbc_closedir(dh);
        }
        getDirsAndFilesChild(url, parentItem, deep);
        return;
    }
    parentItem->setData(true, ITEM_DATA_OPENED);
    if (dh == 0) {
        dh = smbc_opendir(url.data());
    }
    struct smbc_dirent *dirp;
    while ((dirp = smbc_readdir(dh)) != NULL) {
        if ((dirp->smbc_type != SMBC_IPC_SHARE) && (strcmp(dirp->name, ".") != 0) && (strcmp(dirp->name, "..") != 0)) {
            QStandardItem *item = new QStandardItem;
            item->setData(dirp->name, ITEM_DATA_NAME);
            item->setData(dirp->smbc_type, ITEM_DATA_TYPE);
            item->setEditable(false);
            item->setSelectable(false);
            item->setCheckable(true);
            item->setCheckState(parentItem->checkState());
            item->setData(item->checkState(), ITEM_DATA_CHECKBOX_STATE);
            item->setAutoTristate(true);
            if (dirp->smbc_type == SMBC_FILE) {
                item->setIcon(QIcon::fromTheme("application-octet-stream"));
                item->setText(dirp->name);
                qulonglong fileSize = getFileSize(url + "/" + dirp->name);
                item->setData(fileSize, ITEM_DATA_SIZE);
            } else {
                item->setIcon(QIcon::fromTheme("folder-open"));
                item->setText(Constant::g_loadingeText + dirp->name);
            }
            parentItem->appendRow(item);
        }
    }
    smbc_closedir(dh);
    getDirsAndFilesChild(url, parentItem, deep);
    parentItem->setText(parentItem->data(ITEM_DATA_NAME).toString());
    m_itemModel->invisibleRootItem()->appendRow(new QStandardItem);
    m_itemModel->invisibleRootItem()->removeRow(m_itemModel->invisibleRootItem()->rowCount() - 1);
}

void AppLogic::getDirsAndFilesChild(const QByteArray &url, QStandardItem *parentItem, int deep)
{
    //仅读取特定深度目录
    if (deep < 1) {
        return;
    }
    for (int i = 0; i < parentItem->rowCount(); i++) {
        QStandardItem *item = parentItem->child(i);
        if (item->data(ITEM_DATA_CAN_NOT_OPEN).toBool()) {
            continue;
        }
        QByteArray childUrl = url + "/" + item->data(ITEM_DATA_NAME).toByteArray();
        if (item->data(ITEM_DATA_TYPE).toUInt() == SMBC_FILE) {
#ifdef DEBUG_MODE
            qulonglong fileSize = item->data(ITEM_DATA_SIZE).toULongLong();
            QString sizeStr = QString::number(fileSize, 'f') + " Bit";
            fileSize = fileSize / 1024;
            if (fileSize > 1) {
                sizeStr = QString::number(fileSize, 'f', 2) + " Kb";
            }
            fileSize = fileSize / 1024;
            if (fileSize > 1) {
                sizeStr = QString::number(fileSize, 'f', 2) + " Mb";
            }
            fileSize = fileSize / 1024;
            if (fileSize > 1) {
                sizeStr = QString::number(fileSize, 'f', 2) + " Gb";
            }
            item->setToolTip(sizeStr);
#endif
            continue;
        }
        int dh = smbc_opendir(childUrl.data());
        int errorNum = errno;
        //从smb中读取的URL几乎不存在错误的情况，报EINVAL错误一般是连接问题,如果异常断开则尝试断开重连一次
        reconnectSmb(dh, errorNum, childUrl, RETRY_NUMBER);
        if (dh > 0) {
            getDirsAndFiles(childUrl, item, deep - 1, dh);
        } else {
            item->setData(true, ITEM_DATA_CAN_NOT_OPEN);
            item->setToolTip(strerror(errorNum));
            item->setCheckState(Qt::Unchecked);
            item->setCheckable(false);
            item->setAutoTristate(false);
            item->setText(item->data(ITEM_DATA_NAME).toString());
            item->setEnabled(false);
            if (errorNum == EINVAL) {
                //如果重连一次仍然失败则结束并报错
                *m_cancel = true;
                emit connetcResult(SMB_ERROR_7);
                //emit cancelFileMigrate();
                return;
            }
        }
    }
}

void AppLogic::reconnectSmb(int &dh, int &errorNum, const QByteArray &byt, int number, bool isDir, int openType)
{
    if (errorNum == 0 && dh <= 0) {
        if ((isDir && m_smbTimeOut == SMBC_READ_DIR_TIMEOUT) || (!isDir && m_smbTimeOut == SMBC_READ_BUFFER_TIMEOUT)) {
            if (isDir) {
                dh = smbc_opendir(byt.data());
            } else {
                dh = smbc_open(byt.data(), openType, 0);
            }
            if (dh > 0) {
                return;
            }
            errorNum = errno;
        } else {
            if (isDir) {
                m_smbTimeOut = SMBC_READ_DIR_TIMEOUT;
            } else {
                m_smbTimeOut = SMBC_READ_BUFFER_TIMEOUT;
            }
            errorNum = EINVAL;
        }
    }
    if (number < 1) {
        return;
    }
    number--;
    if (dh < 0 && errorNum == EINVAL) {
        if (connectSmb(m_smbTimeOut)) {
            if (isDir) {
                dh = smbc_opendir(byt.data());
            } else {
                dh = smbc_open(byt.data(), openType, 0);
            }
            errorNum = errno;
        }
        qDebug() << "触发重新连接:" << QString(byt) << isDir << dh << errorNum;
        reconnectSmb(dh, errorNum, byt, number, isDir);
    }
#ifdef DEBUG_MODE
    else if (dh < 0 && errorNum != EINVAL && !byt.contains(m_winFinishFileName)) {
        qDebug() << "连接失败:" << byt << errorNum << strerror(errorNum);
    }
#endif
}

QStandardItemModel *AppLogic::getStandardItemModel()
{
    if (m_itemModel == nullptr) {
        m_itemModel = new QStandardItemModel(this);
        connect(m_itemModel, &QStandardItemModel::itemChanged, this, &AppLogic::itemChanged);
    }
    return m_itemModel;
}

bool *AppLogic::getCancelHandle()
{
    if (m_cancel == nullptr) {
        m_cancel = new bool;
        *m_cancel = false;
    }
    return m_cancel;
}

void AppLogic::itemChanged(QStandardItem *item)
{
    //状态不匹配，说明是checkState有变化,对其他itemChanged事件不处理
    if (item->checkState() == item->data(ITEM_DATA_CHECKBOX_STATE).toInt()) {
        return;
    }
    if (item->data(ITEM_DATA_CAN_NOT_OPEN).toBool()) {
        item->setData(item->checkState(), ITEM_DATA_CHECKBOX_STATE);
        return;
    }
    //操作子节点
    if (item->checkState() == Qt::Unchecked || item->checkState() == Qt::Checked) {
        for (int i = 0; i < item->rowCount(); i++) {
            if (!item->child(i)->data(ITEM_DATA_CAN_NOT_OPEN).toBool()) {
                item->child(i)->setCheckState(item->checkState());
            }
        }
    }
    //操作父节点
    if (item->parent() && item->parent() != m_itemModel->invisibleRootItem()
        && item->parent()->checkState() == item->parent()->data(ITEM_DATA_CHECKBOX_STATE).toInt()) {
        //存在父节点 且 父节点不是根节点 且 父节点不是此次动作的源头
        int allCheckState = 0;
        int trueCount = item->parent()->rowCount();
        for (int i = 0; i < item->parent()->rowCount(); i++) {
            if (item->parent()->child(i)->data(ITEM_DATA_CAN_NOT_OPEN).toBool()) {
                trueCount -= 1;
            }
            allCheckState += item->parent()->child(i)->checkState();
        }
        if (allCheckState == 0) {
            item->parent()->setCheckState(Qt::Unchecked);
        } else if (allCheckState == Qt::Checked * trueCount) {
            item->parent()->setCheckState(Qt::Checked);
        } else {
            item->parent()->setCheckState(Qt::PartiallyChecked);
        }
    }
    //重置状态使其保持一致
    item->setData(item->checkState(), ITEM_DATA_CHECKBOX_STATE);
}

void AppLogic::treeViewExpanded(const QModelIndex &index)
{
    if (*m_cancel) {
        return;
    }
    emit connetcResult(SMB_LOAD_URLS_START);
    QStandardItem *itemExpand = m_itemModel->itemFromIndex(index);
    if (itemExpand->data(ITEM_DATA_EXPENDED).toBool()) {
        return;
    }
    itemExpand->setData(true, ITEM_DATA_EXPENDED);
    QByteArray url = itemExpand->text().toLocal8Bit();
    QStandardItem *parentItem = itemExpand->parent();
    while (parentItem != nullptr) {
        url = parentItem->text().toLocal8Bit() + "/" + url;
        parentItem = parentItem->parent();
    }
    url = g_scheme + g_server + "/" + url;
    getDirsAndFiles(url, itemExpand, m_readChild);
    emit connetcResult(SMB_LOAD_URLS_FINISH);
}

void AppLogic::startMigration(const QString &path)
{
    if (m_itemModel->invisibleRootItem()->rowCount() == 0) {
        emit cancelFileMigrate();
        emit connetcResult(SMB_ERROR_3);
        return;
    }
    //emit connetcResult(SMB_START_SAVE);
    m_logFileName = "";
    *m_cancel = false;
    m_savePath = path;
    if (!m_savePath.endsWith("/")) {
        m_savePath += "/";
    }
    m_savePath += g_saveDirName;
    m_savePointList.clear();
    m_saveFileList.clear();
    m_saveErrorList.clear();
    m_saveSuccessList.clear();
    m_smbDirList.clear();
    emit saveProgressValue(0);
    //获取开头节点
    getSavePoint(g_scheme + g_server, m_itemModel->invisibleRootItem());
    if (m_savePointList.isEmpty()) {
        emit cancelFileMigrate();
        emit connetcResult(SMB_ERROR_3);
        return;
    }
    if (m_customize == "icbc") {
        qDebug()<<"in funciton startMigration, it's icbc";
        if (useBatScrip()) {
            return;
        }
    }
    getAllFileUrlOfSamba();
}

// 判断是否含有未获取的节点，含有未获取的节点，使用bat脚本
bool AppLogic::useBatScrip()
{
    //是否可写以及是否存在未获取的节点
    bool hasDir = false;
    for (const SmbUrlInfo &savePoint : m_savePointList) {
        if (savePoint.type != SMBC_FILE) {
            hasDir = true;
            int tmpInsex = savePoint.url.indexOf("/", 7) + 1;
            QString tmpStr = savePoint.url.mid(tmpInsex, savePoint.url.indexOf("/", tmpInsex));
            if (!m_smbDirList.contains(tmpStr)) {
                m_smbDirList.append(tmpStr);
            }
        } else {
            m_saveFileList.append(savePoint);
        }
    }
    int writeBat = -1;
    if (hasDir) {
        QByteArray firstUrl = m_savePointList.first().url;
        m_workPath = firstUrl.mid(0, firstUrl.indexOf("/", firstUrl.indexOf("/", 7) + 1));
        int errorNum = 0;
        reconnectSmb(writeBat, errorNum, m_workPath + "/" + m_winBatFileName, RETRY_NUMBER, false,
                     O_WRONLY | O_CREAT | O_TRUNC);
        smbc_close(writeBat);
        smbc_unlink(QByteArray(m_workPath + "/" + m_winBatFileName));
    }
    //存在目录且有写权限
    if (writeBat > 0) {
        qDebug() << "可基于bat构建迁移队列";
        emit connetcResult(SMB_NEED_SELECTIVE_SOURCE);
        return true;
    }
    return false;
}

void AppLogic::selectiveSource(int isWin)
{
    if (isWin) {
        getAllFileUrlOfWindows();
        return;
    }
    getAllFileUrlOfSamba();
}

void AppLogic::getAllFileUrlOfWindows()
{
    int writeBat = -1;
    int writeName = -1;
    int writeDir = -1;
    QByteArray batScript = BatScript::creatBatScript(m_winDirsFileName, m_winNamesFileName, m_winOutputFileName,
                                                     m_winLogFileName, m_winTmpFileName, m_winFinishFileName);
    QByteArray smbDirData;
    QByteArray smbNameData;
    QStringList smbNameList;
    for (const QString &name : m_smbDirList) {
        QString tmpName = name;
        tmpName.replace("/", "\\");
        tmpName.replace(" ", ":");
        QStringList tmpList = tmpName.split("\\");
        if (tmpList.length() < 2) {
            continue;
        }
        smbDirData.append(tmpName + " ");
        QString tmpStr = tmpList.first();
        if (!smbNameList.contains(tmpStr)) {
            smbNameList.append(tmpStr);
        }
    }
    for (const QString &name : smbNameList) {
        smbNameData.append(name + " ");
    }
    int errorNum = 0;
    reconnectSmb(writeBat, errorNum, m_workPath + "/" + m_winBatFileName, RETRY_NUMBER, false,
                 O_WRONLY | O_CREAT | O_TRUNC);
    smbc_write(writeBat, batScript.data(), batScript.size());
    smbc_close(writeBat);
    errorNum = 0;
    reconnectSmb(writeName, errorNum, m_workPath + "/" + m_winNamesFileName, RETRY_NUMBER, false,
                 O_WRONLY | O_CREAT | O_TRUNC);
    smbc_write(writeName, smbNameData.data(), smbNameData.size());
    smbc_close(writeName);
    errorNum = 0;
    reconnectSmb(writeDir, errorNum, m_workPath + "/" + m_winDirsFileName, RETRY_NUMBER, false,
                 O_WRONLY | O_CREAT | O_TRUNC);
    smbc_write(writeDir, smbDirData.data(), smbDirData.size());
    smbc_close(writeDir);
    emit connetcResult(SMB_NEED_EXEC_BAT_SCRIPT, QString(m_workPath + "/" + m_winBatFileName));
    m_waitScriptResultTimer->start(1000);
}

void AppLogic::waitScriptResult()
{
    if (*m_cancel) {
        emit cancelFileMigrate(); // 重置界面状态
        m_waitScriptResultTimer->stop();
        removeWindowsFile();
        return;
    }
    QFile file(m_configPath + m_winOutputFileName);
    bool checkWindowsFinishTypeFile = true;
#ifdef DEBUG_MODE
//    checkWindowsFinishTypeFile = !file.exists();
#endif
    if (checkWindowsFinishTypeFile) {
        int dh = -1;
        int errorNum = 0;
        reconnectSmb(dh, errorNum, m_workPath + "/" + m_winFinishFileName, 2, false);
        if (dh < 1) {
            return;
        }
        qDebug() << "脚本执行完毕";
        smbc_close(dh);
        m_waitScriptResultTimer->stop();
        SmbUrlInfo outputFile(m_workPath + "/" + m_winOutputFileName, SMBC_FILE);
        outputFile.fileSize = getFileSize(outputFile.url);
        saveFile(outputFile, false);
        SmbUrlInfo logFile(m_workPath + "/" + m_winLogFileName, SMBC_FILE);
        logFile.fileSize = getFileSize(logFile.url);
        saveFile(logFile, false);
        SmbUrlInfo batFile(m_workPath + "/" + m_winBatFileName, SMBC_FILE);
        batFile.fileSize = getFileSize(batFile.url);
        saveFile(batFile, false);
    }
    m_waitScriptResultTimer->stop();
    removeWindowsFile();
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "打开文件失败：" << file.fileName();
    }
    bool isSmbMap = false;
    bool isFileList = false;
    QMap<QByteArray, QByteArray> smbMap;
    bool hasInfo = false;
    QByteArray splitType = " - : - ";
    QByteArrayList tmpDeduplicationList;
    emit connetcResult(SMB_START_SAVE);
    animationPlay();
    m_animation->start();
    while (!file.atEnd()) {
        QByteArray tmpArr = file.readLine();
        if (tmpArr.endsWith('\n')) {
            tmpArr.chop(1);
        }
        if (tmpArr.endsWith(' ')) {
            tmpArr.chop(1);
        }
        if (tmpArr == "[SmbMap]") {
            isSmbMap = true;
            continue;
        }
        if (tmpArr == "[FileList]") {
            isSmbMap = false;
            isFileList = true;
            continue;
        }
        if (!tmpArr.contains(splitType)) {
            qDebug() << "error info :" << tmpArr;
            continue;
        }
        QStringList tmpList = QString(tmpArr).split(splitType);
        if (isSmbMap) {
            smbMap.insert(tmpList.last().toLocal8Bit(), tmpList.first().toLocal8Bit());
            continue;
        }
        tmpArr = tmpList.first().toLocal8Bit();
        for (const QByteArray &dir : smbMap.keys()) {
            if (tmpArr.startsWith(dir)) {
                tmpArr.replace(dir, smbMap.value(dir));
                tmpArr.replace('\\', '/');
                QByteArray url = g_scheme + g_server + "/" + tmpArr;
                tmpDeduplicationList.append(url);
                bool valueToLongBool;
                qulonglong value = tmpList.last().toULongLong(&valueToLongBool);
                if (!valueToLongBool) {
                    qDebug() << "文件大小转换bool = false （toULongLong） 转换数值为：" << value;
                }
                SmbUrlInfo info(url, SMBC_FILE, value);
                m_saveFileList.append(info);
                hasInfo = true;
                break;
            }
        }

        QEventLoop().processEvents();
        if (*m_cancel) {
            m_animation->stop();
            return;
        }
    }
    kom::Configure conf;
    if (hasInfo && conf.value(CONFIG_PLUGIN_GLOBAL, CONFIG_PLUGIN_GLOBAL_DEBUG, false).toBool() == false) {
        QFile::remove(m_configPath + m_winOutputFileName);
        QFile::remove(m_configPath + m_winLogFileName);
        QFile::remove(m_configPath + m_winBatFileName);
        QFile::remove(m_configPath + m_winTmpFileName);
    }
    m_animation->stop();
    downloadFile();
}

void AppLogic::animationPlay()
{
    QString animation;
    if (m_animationNumber == 0) {
        animation = ".";
        m_animationNumber += 1;
    } else if (m_animationNumber == 1) {
        animation = "..";
        m_animationNumber += 1;
    } else {
        animation = "...";
        m_animationNumber = 0;
    }
    emit downloadingFile(Constant::g_creatListText + animation);
}

void AppLogic::removeWindowsFile()
{
    smbc_unlink(QByteArray(m_workPath + "/" + m_winFinishFileName));
    smbc_unlink(QByteArray(m_workPath + "/" + m_winDirsFileName));
    smbc_unlink(QByteArray(m_workPath + "/" + m_winNamesFileName));
    smbc_unlink(QByteArray(m_workPath + "/" + m_winBatFileName));
    smbc_unlink(QByteArray(m_workPath + "/" + m_winLogFileName));
    smbc_unlink(QByteArray(m_workPath + "/" + m_winOutputFileName));
    smbc_unlink(QByteArray(m_workPath + "/" + m_winTmpFileName));
}

qlonglong AppLogic::getFileSize(QByteArray url)
{
    qlonglong fileSize = 0;
    struct stat fileStat;
    memset(&fileStat, 0, sizeof(struct stat));
    smbc_stat(url, &fileStat);
    fileSize = fileStat.st_size;
    return fileSize;
}

void AppLogic::getAllFileUrlOfSamba()
{
    qDebug() << " === === == void AppLogic::getAllFileUrlOfSamba() == === === ";
    m_saveFileList.clear();
    emit connetcResult(SMB_START_SAVE);
    QTime time;
    time.start();
    m_animation->start();
    //获取下载文件列表
    for (const SmbUrlInfo &savePoint : m_savePointList) {
        qDebug() << " === === == 获取下载文件列表 == === === " << savePoint.url;
        if (*m_cancel) {
            emit cancelFileMigrate(); // 添加信号取消文件迁移，文件迁移完成后界面关闭
            return;
        }
        getFileUrl(savePoint);
    }
    m_animation->stop();
    qDebug() << "获取所有下载节点时长（毫秒）：" << time.elapsed();
    downloadFile();
}

void AppLogic::saveHistoryProcess(char success , int index)
{
    QFile file(m_historyProcessFileName);
    if (index > 0) {
        file.remove();
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qDebug() << "打开断点续传缓存文件失败" << m_historyProcessFileName;
        return;
    }
    else{
        qDebug() << "打开断点续传缓存文件成功" << m_historyProcessFileName;
    }
    if (index > 0) {
        for (int i = 0; i < m_saveFileList.length(); i++) {
            if ( i ==  index) {
                file.write(&success, 1);
            } else {
                char tmp = (char)m_historyProcess[i];
                file.write(&tmp, 1);
            }
        }
    }else {
        file.write(&success, 1);
    }

    file.close();
}

int AppLogic::getHistoryProcess()
{
    if (m_historyProcess) {
        delete m_historyProcess;
    }
    qint64 fileNumber = m_listToDownloadInfo.fileNumber;
    qDebug()<<"断点续传总列表大小："<<fileNumber ;
    m_historyProcess = new qint8[fileNumber]{0};
    QFile file(m_historyProcessFileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "打开文件失败：getHistoryProcess";
        return 0;
    }
    qint64 index = 0;
    while (!file.atEnd()) {
        auto i = file.read((char *)m_historyProcess + index, 4096);
        index += i;
    }
    file.remove();
    return index;
}

void AppLogic::clearHistory()
{
    QFile(m_historyListFileName).remove();
    QFile(m_historyProcessFileName).remove();
    qDebug()<<"清除断点续传缓存";
}

void AppLogic::initBreakpointResume()
{
    //首次（非续传）将迁移队列保存本地
    QByteArray splitType = " - : - ";
    QFile file(m_historyListFileName);
    clearHistory();
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qDebug() << "首次打开 断点续传缓存失败" << m_historyListFileName;
        return;
    }
    //写入头数据
    m_listToDownloadInfo.dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd.hh:mm:ss");
    m_listToDownloadInfo.fileNumber = m_saveFileList.length();
    m_listToDownloadInfo.savePath = m_savePath;
    QString header;
    header += m_listToDownloadInfo.dateTime + splitType;
    header += QString::number(m_listToDownloadInfo.fileNumber) + splitType;
    header += m_listToDownloadInfo.ip + splitType;
    header += m_listToDownloadInfo.username + splitType;
    header += m_listToDownloadInfo.savePath + splitType;
    header += "\n";
    file.write(header.toLocal8Bit());
    for (const SmbUrlInfo &info : m_saveFileList) {
        QByteArray tmp = info.url + splitType + QByteArray::number(info.fileSize) + '\n';
        file.write(tmp);
    }
    file.close();
}

void AppLogic::downloadFile(bool firstTime)
{
    m_saveSuccessList.clear();
    m_saveErrorList.clear();
    QTime time;
    time.start();
    if (m_saveFileList.isEmpty()) {
        emit connetcResult(SMB_ERROR_3);
        emit cancelFileMigrate();
        return;
    }
    if (firstTime) {
        clearHistory();
        initBreakpointResume();
    }
    int historyNumber = getHistoryProcess();
    qulonglong progressMax = 0;
    qulonglong progressValue = 0;
    for (int i = 0; i < m_saveFileList.length(); i++) {
        SmbUrlInfo url = m_saveFileList.at(i);
        progressMax += url.fileSize;
        if (i < historyNumber) {
            if (historyNumber != m_saveFileList.length() || m_historyProcess[i] == 1 ) {
                progressValue += url.fileSize;
            }
        }
    }
    if (progressValue > 0) {
        qDebug()<<"中断前已下载大小："<<progressValue;
        qDebug()<<"中断前已下载数量："<<historyNumber;
    }
    emit saveProgressMax(PROGRESS_MAX);
    m_saveProgressMax = progressMax;
    m_saveProgressValue = progressValue;
    if (m_saveProgressMax == 0) {
        emit saveProgressValue(PROGRESS_MAX);
    } else {
        emit saveProgressValue(m_saveProgressValue * PROGRESS_MAX / m_saveProgressMax);
    }
    //创建目录
    if (!QDir().mkpath(m_savePath)) {
        qDebug()<<"创建目录失败"<< m_savePath;
        emit connetcResult(SMB_ERROR_4);
        emit cancelFileMigrate();
        return;
    }
    qDebug() << " === === 下载数量 === === " << m_saveFileList.size();
    //执行下载
    for (int i = 0; i < m_saveFileList.length(); i++) {
        qDebug() <<" === === 下载 === === " << m_saveFileList[i].url;
        if (*m_cancel) {
            break;
        }
        // 工行版本需要有断点续传功能，其他版本无此功能，此处做判断区分
        if (m_customize != "icbc") {
            saveFile(m_saveFileList.at(i));
            continue;
        }
        SmbUrlInfo url = m_saveFileList.at(i);
        if (historyNumber < m_saveFileList.length() && i < historyNumber) {
            if (m_historyProcess[i] == 1) {
                m_saveSuccessList.append(
                    SmbUrlInfo(url.url, url.type, url.fileSize, Constant::g_migrationSuccessText.toLocal8Bit()));
            } else {
                m_saveErrorList.append(
                    SmbUrlInfo(url.url, url.type, url.fileSize, Constant::g_migrationSuccessText.toLocal8Bit()));
            }
            saveHistoryProcess(m_historyProcess[i]);
            continue;
        }
        if (m_historyProcess[i] == 1) {
            m_saveSuccessList.append(
                SmbUrlInfo(url.url, url.type, url.fileSize, Constant::g_migrationSuccessText.toLocal8Bit()));
            continue;
        }
        bool success = false;
        QString path = "";
        do {
            if (path == QString(m_saveFileList.at(i).url)) {
                currentDateTime2 = QDateTime::currentDateTime(); 
                int secondsDiff = currentDateTime1.secsTo(currentDateTime2);
                if (secondsDiff > 120) {
                    currentDateTime1 = currentDateTime2;
                    emit timeOutTip();
                }
            }
            qDebug() << " ----- 下载文件：" << m_saveFileList.at(i).url;
            success = saveFile(m_saveFileList.at(i));
            qDebug() << " errorNum: " << m_errorNum << "strerror:"<< strerror(errno);

            if (m_errorNum != ETIMEDOUT) {
                break;
            }
            path = QString(m_saveFileList.at(i).url);

        } while (success == false);
        
        if (success) {
            if (historyNumber == m_saveFileList.length()) {
                saveHistoryProcess(1 , i);
            } else {
                saveHistoryProcess(1);
            }
        } else {
            if (historyNumber == m_saveFileList.length()) {
                saveHistoryProcess(0, i);
            } else {
                saveHistoryProcess(0);
            }
        }
    }
    if (*m_cancel) {
        emit cancelFileMigrate(); // 添加信号取消文件迁移，文件迁移完成后界面关闭
        clearHistory();
        return;
    }
    m_logFileName = m_listToDownloadInfo.dateTime + ".log";
    creatLog();
    emit connetcResult(SMB_ERROR_5, QVariant::fromValue(m_saveErrorList));
    emit cancelFileMigrate();
    qDebug() << "传输文件用时（毫秒）：" << time.elapsed();
    int alltime = 0;
    alltime += time.elapsed();
    qDebug() << "迁移文件总用时（毫秒）：" << alltime;
}

void AppLogic::resave(bool b)
{
    if (b) {
        downloadFile(false);
    } else {
        clearHistory();
    }
}

void AppLogic::getSavePoint(const QByteArray &url, QStandardItem *parentItem)
{
    if (*m_cancel) {
        return;
    }
    for (int i = 0; i < parentItem->rowCount(); i++) {
        QStandardItem *item = parentItem->child(i);
        QByteArray savePoint = url + "/" + item->data(ITEM_DATA_NAME).toByteArray();
        if (item->checkState() == Qt::Checked) {
            // 判断被选中的item是否含有子分支
            if (item->hasChildren()) {
                getSavePoint(savePoint, item);
            } else {
                m_savePointList.append(SmbUrlInfo(savePoint, item->data(ITEM_DATA_TYPE).toUInt(), item->data(ITEM_DATA_SIZE).toULongLong()));
            }
        } else if (item->checkState() == Qt::PartiallyChecked) {
            getSavePoint(savePoint, item);
        }
    }
}

void AppLogic::getFileUrl(const SmbUrlInfo &url)
{
    if (url.type == SMBC_FILE) {
        m_saveFileList.append(url);
        return;
    }
    if (*m_cancel) {
        return;
    }
    QString creatListHeader = Constant::g_creatListText + ":";
    int length = 330;//显示文字的label长度
    QString data = url.url;
    data = data.mid(data.indexOf("/",data.indexOf("/",7) + 1));
    creatListHeader += data.mid(0,20);
    data = data.mid(20);
    QString creatList = creatListHeader + data;
    QFont font;
    QFontMetrics fm(font);
    // while(fm.width(creatList) > length) {
    //     data = data.mid(2);
    //     creatList = creatListHeader + "..." + data;
    // }
    creatList = fm.elidedText(creatList, Qt::ElideMiddle, length);
    emit downloadingFile(creatList);
    int dh = smbc_opendir(url.url.data());
    int errorNum = errno;
    reconnectSmb(dh, errorNum, url.url, RETRY_NUMBER);
    SmbUrlInfoList tmpList;
    if (dh > 0) {
        struct smbc_dirent *dirp;
        while ((dirp = smbc_readdir(dh)) != NULL) {
            if ((dirp->smbc_type != SMBC_IPC_SHARE) && (strcmp(dirp->name, ".") != 0) && (strcmp(dirp->name, "..") != 0)) {
                QByteArray childUrl = url.url + "/" + QByteArray(dirp->name);
                qulonglong fileSize = 0;
                if (dirp->smbc_type == SMBC_FILE) {
                    fileSize = getFileSize(childUrl);
                }
                tmpList.append(SmbUrlInfo(childUrl, dirp->smbc_type, fileSize));
            }
        }
    } else {
        if (errorNum == EINVAL) {
            //如果重连N次仍然失败则结束并报错
            *m_cancel = true;
            emit connetcResult(SMB_ERROR_7);
            emit cancelFileMigrate();
            return;
        }
        m_saveFileList.append(url);
    }
    smbc_closedir(dh);
    for (const SmbUrlInfo &info : tmpList) {
        getFileUrl(info);
    }
}

bool AppLogic::saveFile(const SmbUrlInfo &url, bool isUserFile)
{
    m_errorNum = 0;
    QString savePath = url.url;
    qDebug() << "url is " <<savePath;
    if (isUserFile) {
        savePath.replace(g_scheme + g_server, m_savePath);
    } else {
        savePath = m_configPath + savePath.split("/").last();
    }
    if (url.type == SMBC_DIR || url.type == SMBC_FILE_SHARE) {
        QDir().mkpath(savePath);
        return true;
    }
    QDir().mkpath(QFileInfo(savePath).absolutePath());
    qulonglong tmpSaveProgressValue = m_saveProgressValue;
    qulonglong tmpProgressValue = m_saveProgressValue;

    qDebug() << "当前迁移成功文件数据大小： " << m_saveProgressValue;
    emit saveProgressValue(m_saveProgressValue * PROGRESS_MAX / m_saveProgressMax);
    //打开并读取文件
    int ret = 0;
    int fd = -1;
    // int errorNum = EINVAL;
    //reconnectSmb(fd, errorNum, url.url,RETRY_NUMBER, false,SMBC_READ_BUFFER_TIMEOUT);
    int errorNum = 0;
    reconnectSmb(fd, errorNum, url.url, RETRY_NUMBER, false);
    if (fd < 0) {
        m_errorNum = errno;
        qDebug()<<"errno is "<< errno;
        m_saveErrorList.append(SmbUrlInfo(url.url, url.type, url.fileSize, strerror(errno)));
        qDebug()<< "远程文件打开失败:"<<strerror(errno);
        return false;
    }

    QString saveDir = savePath.mid(0, savePath.lastIndexOf('/'));
    QDir().mkpath(saveDir);
    QFile localFile(savePath);
    if (!localFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "本地文件打开失败:" << localFile.errorString();
        m_saveErrorList.append(SmbUrlInfo(url.url, url.type, url.fileSize, Constant::g_creatFileFailedText.toLocal8Bit()));
       // emit saveProgressValue(m_saveProgressValue * PROGRESS_MAX / m_saveProgressMax);
        return false;
    }

//    //获取文件基础信息
//#ifdef IS_MIPS_TYPE
//    struct myStat fileStat;
//    memset(&fileStat, 0, sizeof(struct myStat));
//    smbc_fstat(fd, ( struct stat*)&fileStat);
//    if (fileStat.st_size == 0 && fileStat.st_pad3 > 0) {
//        fileStat.st_size = fileStat.st_pad3;
//    }
//#else
//    struct stat fileStat;
//    memset(&fileStat, 0, sizeof(struct stat));
//    smbc_fstat(fd, &fileStat);
//#endif

    auto bufferSize = qMin(url.fileSize, (qulonglong)SMBC_SAVE_BUFFER_SIZE_MAX);
    char *buffer = new char[bufferSize];
    QList<int> tmpList;
    auto waittime = 0;
    QTime time;
    time.start();
    do {
        ret = smbc_read(fd, buffer, bufferSize);
        tmpList.append(ret);
        if (ret < 0) {
            waittime = time.elapsed();
            if (errno == ETIMEDOUT) {
                qDebug() << "迁移过程中，受到网络波动影响，文件重新迁移，进度回退" << tmpProgressValue;
                currentDateTime1 = QDateTime::currentDateTime(); 
               // m_saveProgressValue = tmpProgressValue;
                m_saveErrorList.append(SmbUrlInfo(url.url, url.type, url.fileSize, Constant::g_creatFileFailedText.toLocal8Bit()));
                //emit saveProgressValue(tmpProgressValue * PROGRESS_MAX / m_saveProgressMax);
            }
        } else if (ret > 0) {
#ifdef DEBUG_MODE
            qDebug() << url.url << " | " << QString::number(double(ret) / 1024 / 1024, 'f') + "MB";
#endif
            localFile.write(buffer, ret);
            //tmpSaveProgressValue += ret;
        }
        if (*m_cancel) {
            break;
        }
    } while (ret > 0);

    m_saveProgressValue += url.fileSize;
    int proc = m_saveProgressValue * PROGRESS_MAX / m_saveProgressMax;
    qDebug()<<"success, ProgressValue is "<< proc;
    qDebug()<<"current max is "<<m_saveProgressMax <<" m_saveProgressValue is "<< m_saveProgressValue;
    emit downloadingFile(savePath.split("/").last());
    emit saveProgressValue(m_saveProgressValue * PROGRESS_MAX / m_saveProgressMax);
    smbc_close(fd);
    localFile.close();
    delete buffer;

    m_errorNum = errno;
    if (*m_cancel) {
        localFile.remove();
        return false;
    }
    if (url.fileSize != localFile.size()) {
        qDebug() << "完整性校验不通过：" << url.url << errno << strerror(errno) << "win文件大小" << url.fileSize
                 << "本地文件大小" << localFile.size();
        qDebug() << "迁移失败!错误码：" << errno << " 等待时长：" << waittime << " 超时时长：" << smbc_getTimeout(m_smbCtx) << " 迁移队列：" << tmpList;
        if (m_customize == "icbc") {
            if (m_errorNum == ETIMEDOUT) {
                localFile.remove();
                return false;
            }
        }
        if (isUserFile) {
            bool exists = false;
            SmbUrlInfo target(url.url, url.type, url.fileSize, Constant::g_fileErrorText.toLocal8Bit());
            for (const auto& item : m_saveErrorList) {
                if (item.url == target.url &&
                    item.type == target.type &&
                    item.fileSize == target.fileSize) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                m_saveErrorList.append(target);
            }
        }
        localFile.remove();
        return false;
    }
    if (isUserFile) {
        m_saveSuccessList.append(SmbUrlInfo(url.url, url.type, url.fileSize, Constant::g_migrationSuccessText.toLocal8Bit()));
    }
    return true;
}

void AppLogic::creatLog()
{
    qDebug()<<"输出日志："<< m_logPath + m_logFileName;
    if (m_saveSuccessList.isEmpty() && m_saveErrorList.isEmpty()) {
        return;
    }
    if (m_saveErrorList.isEmpty()) {
        clearHistory();
    }
    QFile file(m_logPath + m_logFileName);
    if (!file.open(QIODevice::Text | QIODevice::Truncate | QIODevice::WriteOnly)) {
        emit connetcResult(SMB_ERROR_6);
        //emit cancelFileMigrate();
        return;
    }
    for (const SmbUrlInfo &si : m_saveSuccessList) {
        file.write(si.url + " | " + si.result + "\n");
    }
    for (const SmbUrlInfo &si : m_saveErrorList) {
        file.write(si.url + " | " + si.result + "\n");
    }
    emit connetcResult(SMB_CREAT_LOG_FINISH, m_logFileName);
    //emit cancelFileMigrate();
}
