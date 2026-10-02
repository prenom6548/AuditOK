#include "applogic.h"
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <QTime>
#include "kom-configure.h"

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
    if (m_readChild <= 0) {
        m_readChild = 1;
    }
    qDebug() << "子目录深度：" << m_readChild;
}

bool AppLogic::connectSmb()
{
    //建立smb连接
    m_smbCtx = smbc_new_context();
    if (!m_smbCtx) {
        return false;
    }
    smbc_setDebug(m_smbCtx, g_debugLevel);
    smbc_setFunctionAuthData(m_smbCtx, auth_fn);
    smbc_setTimeout(m_smbCtx, SMBC_READ_BUFFER_TIMEOUT);
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
            struct stat fileStat;
            smbc_stat(childUrl.data(), &fileStat);
            double fileSize = fileStat.st_size;
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
        reconnectSmb(dh, errorNum, childUrl);
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
                return;
            }
        }
    }
}

void AppLogic::reconnectSmb(int &dh, int &errorNum, const QByteArray &byt, bool isDir)
{
    if (dh < 0 && errorNum == EINVAL) {
        if (connectSmb()) {
            if (isDir) {
                dh = smbc_opendir(byt.data());
            } else {
                dh = smbc_open(byt.data(), O_RDONLY, 0);
            }
            errorNum = errno;
        }
        qDebug() << "触发重新连接:" << QString(byt) << isDir << dh << errorNum;
    }
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
        emit connetcResult(SMB_ERROR_3);
        return;
    }
    QTime time;
    int alltime = 0;
    time.start();
    emit connetcResult(SMB_START_SAVE);
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
    emit saveProgressValue(0);
    //获取开头节点
    getSavePoint(g_scheme + g_server, m_itemModel->invisibleRootItem());
    //获取下载文件列表
    for (const SmbUrlInfo &savePoint : m_savePointList) {
        qDebug() <<" === === == 获取下载文件列表 == === === " << savePoint.url;
        if (*m_cancel) {
            emit cancelFileMigrate(); // 添加信号取消文件迁移，文件迁移完成后界面关闭
            return;
        }
        getFileUrl(savePoint);
    }
    qDebug() << "获取所有下载节点时长（毫秒）：" << time.elapsed();
    alltime += time.elapsed();
    time.restart();
    if (m_saveFileList.isEmpty()) {
        emit connetcResult(SMB_ERROR_3);
        return;
    }
    emit saveProgressMax(m_saveFileList.length());
    //创建目录
    if (!QDir().mkpath(m_savePath)) {
        emit connetcResult(SMB_ERROR_4);
        return;
    }
        qDebug() <<" === === 下载数量 === === " << m_saveFileList.size();
    //执行下载
    for (int i = 0; i < m_saveFileList.length(); i++) {
        qDebug() <<" === === 下载 === === " << m_saveFileList[i].url;
        if (*m_cancel) {
            emit cancelFileMigrate(); // 添加信号取消文件迁移，文件迁移完成后界面关闭
            break;
        }
        saveFile(m_saveFileList.at(i));
        emit saveProgressValue(i + 1);
    }
    if (*m_cancel) {
        emit cancelFileMigrate(); // 添加信号取消文件迁移，文件迁移完成后界面关闭
    }
    m_logFileName = QDateTime::currentDateTime().toString("yyyy.MM.dd HH:mm:ss") + ".log";
    creatLog();
    emit connetcResult(SMB_ERROR_5, QVariant::fromValue(m_saveErrorList));
    qDebug() << "传输文件用时（毫秒）：" << time.elapsed();
    alltime += time.elapsed();
    qDebug() << "迁移文件总用时（毫秒）：" << alltime;
}

void AppLogic::resave()
{
    if (m_saveErrorList.isEmpty()) {
        return;
    }
    QTime time;
    time.start();
    SmbUrlInfoList errorList = m_saveErrorList;
    emit saveProgressMax(errorList.length());
    m_saveErrorList.clear();
    //执行下载
    for (int i = 0; i < errorList.length(); i++) {
        saveFile(errorList.at(i));
        emit saveProgressValue(i + 1);
    }
    creatLog();
    qDebug() << "重试用时（毫秒）：" << time.elapsed();
    emit connetcResult(SMB_ERROR_5, QVariant::fromValue(m_saveErrorList));
}

void AppLogic::getSavePoint(const QByteArray &url, QStandardItem *parentItem)
{
    for (int i = 0; i < parentItem->rowCount(); i++) {
        QStandardItem *item = parentItem->child(i);
        QByteArray savePoint = url + "/" + item->data(ITEM_DATA_NAME).toByteArray();
        if (item->checkState() == Qt::Checked) {
            // 判断被选中的item是否含有子分支
            if (item->hasChildren()) {
                getSavePoint(savePoint, item);
            } else {
                m_savePointList.append(SmbUrlInfo(savePoint, item->data(ITEM_DATA_TYPE).toUInt()));
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
    int dh = smbc_opendir(url.url.data());
    int errorNum = errno;
    reconnectSmb(dh, errorNum, url.url);
    SmbUrlInfoList tmpList;
    if (dh > 0) {
        struct smbc_dirent *dirp;
        while ((dirp = smbc_readdir(dh)) != NULL) {
            if ((dirp->smbc_type != SMBC_IPC_SHARE) && (strcmp(dirp->name, ".") != 0) && (strcmp(dirp->name, "..") != 0)) {
                QByteArray childUrl = url.url + "/" + QByteArray(dirp->name);
                tmpList.append(SmbUrlInfo(childUrl, dirp->smbc_type));
            }
        }
    } else {
        if (errorNum == EINVAL) {
            //如果重连一次仍然失败则结束并报错
            *m_cancel = true;
            emit connetcResult(SMB_ERROR_7);
            return;
        }
        m_saveFileList.append(url);
    }
    smbc_closedir(dh);
    for (const SmbUrlInfo &info : tmpList) {
        getFileUrl(info);
    }
}

void AppLogic::saveFile(const SmbUrlInfo &url)
{
    QString savePath = url.url;
    savePath.replace(g_scheme + g_server, m_savePath);
    if (url.type == SMBC_DIR || url.type == SMBC_FILE_SHARE) {
        QDir().mkpath(savePath);
        return;
    }
    emit downloadingFile(savePath.split("/").last());
    //打开并读取文件
    int ret = 0;
    int fd = smbc_open(url.url.data(), O_RDONLY, 0);
    int errorNum = errno;
    reconnectSmb(fd, errorNum, url.url, false);
    if (fd < 0) {
        m_saveErrorList.append(SmbUrlInfo(url.url, url.type, strerror(errno)));
        return;
    }
    QString saveDir = savePath.mid(0, savePath.lastIndexOf('/'));
    QDir().mkpath(saveDir);
    QFile localFile(savePath);
    if (!localFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        m_saveErrorList.append(SmbUrlInfo(url.url, url.type, Constant::g_creatFileFailedText.toLocal8Bit()));
        return;
    }
    //获取文件基础信息
    struct stat fileStat;
    memset(&fileStat, 0, sizeof(struct stat));
    smbc_fstat(fd, &fileStat);
    auto bufferSize = qMin(fileStat.st_size, SMBC_SAVE_BUFFER_SIZE_MAX);
    char *buffer = new char[bufferSize];
    QList<int> tmpList;
    do {
        QTime time;
        time.start();
        ret = smbc_read(fd, buffer, bufferSize);
        tmpList.append(ret);
        if (ret < 0) {
            qDebug() << "迁移失败!错误码：" << errno << " 等待时长：" << time.elapsed() << " 超时时长：" << smbc_getTimeout(m_smbCtx) << " 迁移队列："
                     << tmpList;
        }
        if (ret > 0) {
            localFile.write(buffer, ret);
        }
        if (*m_cancel) {
            emit cancelFileMigrate(); // 添加信号取消文件迁移，文件迁移完成后界面关闭
            break;
        }
    } while (ret > 0);
    smbc_close(fd);
    localFile.close();
    delete buffer;

    if (*m_cancel) {
        localFile.remove();
        return;
    }
    if (fileStat.st_size != localFile.size()) {
        qDebug() << "完整性校验不通过：" << url.url << errno << strerror(errno) << "win文件大小" << fileStat.st_size << "本地文件大小" << localFile.size();
        m_saveErrorList.append(SmbUrlInfo(url.url, url.type, Constant::g_fileErrorText.toLocal8Bit()));
        localFile.remove();
        return;
    }
    m_saveSuccessList.append(SmbUrlInfo(url.url, url.type, Constant::g_migrationSuccessText.toLocal8Bit()));
}

void AppLogic::creatLog()
{
    if (m_saveSuccessList.isEmpty() && m_saveErrorList.isEmpty()) {
        return;
    }
    QFile file(m_logPath + m_logFileName);
    if (!file.open(QIODevice::Text | QIODevice::Truncate | QIODevice::WriteOnly)) {
        emit connetcResult(SMB_ERROR_6);
        return;
    }
    for (const SmbUrlInfo &si : m_saveSuccessList) {
        file.write(si.url + " | " + si.result + "\n");
    }
    for (const SmbUrlInfo &si : m_saveErrorList) {
        file.write(si.url + " | " + si.result + "\n");
    }
    emit connetcResult(SMB_CREAT_LOG_FINISH, m_logFileName);
}
