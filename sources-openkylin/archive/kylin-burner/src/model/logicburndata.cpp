/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "logicburndata.h"
#include "logicmain.h"
#include "src/common/utils.h"
#include "src/common/dbuscaller.h"
#include "src/frame/configuration.h"
#include "src/frame/logger.h"
#include "src/frame/auditlog.h"
#include "src/po/translation.h"
#include "src/data/datacdrom.h"
#include "src/data/datadirectory.h"
#include "src/data/datafile.h"
#include "src/view/modaldialog.h"
#include "src/view/execdialog.h"
#include "src/view/filefilterrules.h"
#include "src/view/tip_burnresult.h"
#include "src/view/mainwindow.h"
#include "src/view/progress.h"
#include "src/process/processlsof.h"
#include "src/model/menufactory.h"
#include "src/model/diroptutils.h"
#include "src/view/progress.h"

#include "burnjob.h"
#include "burnjobmanager.h"

#include "ky-disc-info/include/disc_interface.h"
#include "ky-udf-image/include/interface.h"

#include <QDebug>
#include <QFont>
#include <QDir>
#include <QUrl>
#include <QCoreApplication>
#include <QStorageInfo>
#include <QMimeData>
#include <algorithm>
#include <QGSettings/QGSettings>
#include <QStandardPaths>
#include <QMessageBox>

#define APPLICATIONTYPE QString("application/x-kylinburnerdata")

using namespace ky_plugin_burn;
using namespace dfmburn;

LogicBurnData::LogicBurnData(QObject *parent)
    : QAbstractItemModel(parent),
      m_iIndex(-1),
      m_addAction_canceled(false),
      m_bAppRunning(true),
      m_file_device_index(-1)
{
    LOGGER->registerName(this, _("BURNDATA"));
    m_current_item = nullptr;

    connect(this, SIGNAL(setting(void*)), ModalDialog::instance()->showPage(ModalDialog::BusnessSetting), SLOT(load(void *)));

    connect(this, SIGNAL(title(QString)), ModalDialog::instance()->getPageByIndex(ModalDialog::Progress), SIGNAL(title(QString)));
    connect(this, SIGNAL(setRange(int,int)), ModalDialog::instance()->getPageByIndex(ModalDialog::Progress), SIGNAL(setRange(int, int)));
    connect(this, SIGNAL(setValue(int,QIcon,QString)), ModalDialog::instance()->getPageByIndex(ModalDialog::Progress), SIGNAL(setValue(int,QIcon,QString)));
    connect(this, SIGNAL(addValue(int,QIcon,QString)), ModalDialog::instance()->getPageByIndex(ModalDialog::Progress), SIGNAL(addValue(int,QIcon,QString)));
    connect(LogicMain::instance(), &LogicMain::exit, [=](){m_bAppRunning = false;});
}

QVariant LogicBurnData::headerData(int section, Qt::Orientation orientation, int role) const
{
    QFont font;
    if (orientation == Qt::Horizontal)
    {
        switch (role)
        {
        case Qt::DisplayRole:
            if (m_oHHeaders.size() && section < m_oHHeaders.size())
                return m_oHHeaders.at(section);
            else
                return QString();
            break;
        case Qt::FontRole:
            font.setPixelSize(14);
            font.setWeight(60);
            return font;
        default:
            break;
        }
    }
    else
    {
        switch (role)
        {
        case Qt::DisplayRole:
            if (m_oVHeaders.size() && section < m_oHHeaders.size())
                return m_oVHeaders.at(section);
            break;
        default:
            break;
        }
    }

    return QVariant();
}

bool LogicBurnData::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role)) {
        switch (role)
        {
        case Qt::DisplayRole:
            if (orientation == Qt::Horizontal)
            {
                if (section > m_oHHeaders.size())
                    return false;
                else if (section == m_oHHeaders.size())
                    m_oHHeaders << value.toString();
                else
                    m_oHHeaders.replace(section, value.toString());
            }
            else
            {
                if (section > m_oVHeaders.size())
                    return false;
                else if (section == m_oVHeaders.size())
                    m_oVHeaders << value.toString();
                else
                    m_oVHeaders.replace(section, value.toString());
            }
            break;
        }
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}

QModelIndex LogicBurnData::index(int row, int column, const QModelIndex &parent) const
{

    if (!parent.isValid() && row < m_oRoots.size() && row > -1)
        return createIndex(row, column, m_oRoots.at(row));

    DataDirectory *dir = static_cast<DataDirectory *>(parent.internalPointer());
    if (nullptr == dir)
        return QModelIndex();
    return createIndex(row, column, dir->at(row));
    return QModelIndex();
}

QModelIndex LogicBurnData::parent(const QModelIndex &index) const
{
    if (!index.isValid()) return QModelIndex();

    DataFile *item = static_cast<DataFile *>(index.internalPointer());
    if ((nullptr == item))
    {
        return QModelIndex();
    }

    if (item->isDirectory()) {
        DataDirectory *itemDir = static_cast<DataDirectory *>(item);
        if (itemDir->isRoot()) {
            return QModelIndex();
        }
    }

    DataDirectory *parent = static_cast<DataDirectory *>(item->parent());
    if (nullptr == parent)
    {
        return QModelIndex();
    }

    if (parent->isRoot())
        return createIndex(m_oRoots.indexOf(parent), 0, parent);

    DataDirectory *grandParent = static_cast<DataDirectory *>(parent->parent());
    if (!grandParent->isDirectory())
        return QModelIndex();

    return createIndex(grandParent->findIndex(parent), 0, parent);
}

int LogicBurnData::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_oRoots.size();
    if (m_oVHeaders.size())
        return m_oVHeaders.size();

    DataDirectory *dir = static_cast<DataDirectory *>(parent.internalPointer());
    if (nullptr == dir || !dir->isDirectory())
        return 0;
    return dir->counts();
}

int LogicBurnData::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_oHHeaders.size();
    return m_oHHeaders.size();
}

QVariant LogicBurnData::data(const QModelIndex &index, int role) const
{
    DataFile *item;
    QFont f;

    if (!index.isValid())
        return QVariant();

    item = static_cast<DataFile *>(index.internalPointer());
    if (nullptr == item) return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
        switch (index.column())
        {
        case 0:
            return item->display();
        case 1:
            if (".." == item->display())
                return _("Parent");
            return item->type();
        case 2:
            if (".." == item->display())
                return QString("-");
            return item->stringSize();
        case 3:
            return item->source();
        }
        break;
    case Qt::EditRole:
        if (0 == index.column())
            return item->display();
    case Qt::DecorationRole:
        if (0 == index.column())
        {
            if (item->isDisk())
            {
                return QIcon(item->icon().pixmap(QSize(24, 24), QIcon::Disabled));
            }
            else
            {
                return item->icon();
            }
        }
        break;
    case Qt::FontRole:
        if (item->attrSymbolLink() && 0 == index.column())
        {
            f.setItalic(true);
            return f;
        }
        break;
    case Qt::ForegroundRole:
        if (item->isDisk())
            return QBrush(QColor(179, 179, 179));
        break;
    case Qt::ToolTipRole:
        switch (index.column())
        {
        case 0:
            return item->display();
        case 1:
            return item->type();
        case 2:
            return item->stringSize();
        case 3:
            return item->source();
        }
        break;
    case Qt::SizeHintRole:
        return QSize(100,36);
        break;
    }

    return QVariant();
}

bool LogicBurnData::setData(const QModelIndex &index, const QVariant &value, int role)
{
    DataFile *item;
    QString newName;

    if (data(index, role) != value) {
        if (Qt::EditRole == role)
        {
            item = static_cast<DataFile *>(index.internalPointer());
            if (nullptr == item)
                goto exit;
            newName = value.toString();
            if (newName.trimmed().isEmpty())
            {
                EXEC->error(_("Rename") + " - " + _("Empty Name"),
                            _("Cannot rename to empty."));
                goto exit;
            }
            if (newName.toUtf8().size() > 103)
            {
                EXEC->error(_("Rename") + " - " + _("Name Too Long"),
                            _("New name length must be less than 103 unicode charset."));
                goto exit;
            }
            if (duplicatedName(newName, index.parent()))
            {
                EXEC->error(_("Rename") + " - " + _("Duplicate Name"),
                            _("There is a item named by %1.").arg(newName));
                goto exit;
            }
            item->rename(value.toString());
            BurnData::m_bDirReNameing = false;
            // fix169480 修改当前文件、目录及目录下所有子文件将被缓存的目的路径
            fixFileDstPathByIndex(newName, index);

            emit dataChanged(index, index, QVector<int>() << role);
            return true;
        }
        return false;
    }
exit:
    BurnData::m_bDirReNameing = false;
    return false;
}

Qt::ItemFlags LogicBurnData::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    DataFile *file = static_cast<DataFile *>(index.internalPointer());
    if (nullptr == file || file->isDisk())
        return QAbstractItemModel::flags(index);

    DataCDROM *cdrom = static_cast<DataCDROM *>(m_oBurners.at(m_iIndex));
    if (!cdrom->isUdf() && cdrom->mediumStatus() == DataMedium::MEDIUM_FINALIZED)
        return QAbstractItemModel::flags(index);

    if(cdrom->fileSystemTypeWithExtendName() == UDF201){
        return QAbstractItemModel::flags(index);
    }

    if (".." == index.data().toString())
        return QAbstractItemModel::flags(index);

    if (0 == index.column())
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

Qt::DropActions LogicBurnData::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList LogicBurnData::mimeTypes() const
{
    QStringList types = QAbstractItemModel::mimeTypes();
    types.append(APPLICATIONTYPE);
    types << "text/uri-list" << "text/plain" << "text/html";
    return types;
}

QMimeData *LogicBurnData::mimeData(const QModelIndexList &indexes) const
{
    QMimeData  *ret = new QMimeData();
    unsigned long long ptr;
    QByteArray  data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    foreach (QModelIndex idx, indexes)
    {
        if (0 == idx.column())
        {
            ptr = (unsigned long long)(idx.internalPointer());
            stream << ptr;
        }
    }

    ret->setData(APPLICATIONTYPE, data);

    return ret;
}

bool LogicBurnData::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    unsigned long long ptr;
    QModelIndex oldParentIndex;

    DataFile *file = static_cast<DataFile *>(parent.internalPointer());
    if (!file->isDirectory())
        return false;

    if (data->hasUrls())
    {
        QList<QUrl> urls = data->urls();
        QStringList willAdd;
        while (urls.size()){
            QUrl indexUrl = convertTrashUrlToFileUrl(urls.takeFirst());
            willAdd << indexUrl.path();
        }

        addDatasToIndex(willAdd, parent);
        if(!m_addAction_canceled){
            emit rootIndexChanged(parent);
        }

        return true;
    }

    if (data->hasFormat(APPLICATIONTYPE))
    {
        QByteArray bytes = data->data(APPLICATIONTYPE);
        QDataStream stream(bytes);
        file = static_cast<DataFile *>(parent.internalPointer());
        if (nullptr == file || !file->isDirectory())
            return false;

        DataDirectory *newParent = static_cast<DataDirectory *>(file);

        file = nullptr;
        QModelIndex newParentIndex = parent;
        if (".." == newParent->display()){
            newParent = qobject_cast<DataDirectory *>(newParent->parent()->parent());
            newParentIndex = parent.parent().parent();
        }

        if (nullptr == newParent){
            return false;
        }

        while (!stream.atEnd()){
            ptr = 0;
            stream >> ptr;
            file = (DataFile *)(ptr);
            DataDirectory *oldParent = qobject_cast<DataDirectory *>(file->parent());
            if (!oldParent || oldParent == newParent){
                continue;
            }

            if (file->isDirectory()){
                DataDirectory *walk = newParent;
                while (walk) {
                    if (walk == file) {
                        file = nullptr;
                        break;
                    }
                    walk = qobject_cast<DataDirectory *>(walk->parent());
                }
                if (!file) {
                    continue;
                }
            }

            const int oldRow = oldParent->findIndex(file);
            if (oldRow < 0) {
                continue;
            }

            oldParentIndex = modelIndexOfDirectory(oldParent);
            newParentIndex = modelIndexOfDirectory(newParent);
            if (!oldParentIndex.isValid() || !newParentIndex.isValid()) {
                continue;
            }

            beginInsertRows(newParentIndex, newParent->counts(), newParent->counts());
            beginRemoveRows(oldParentIndex, oldRow, oldRow);
            file->changeParent(newParent);
            endRemoveRows();
            endInsertRows();
        }

        return true;
    }

    return false;
}

bool LogicBurnData::isUp(QModelIndex idx)
{
    if (".." == idx.siblingAtColumn(0).data().toString()){
        return true;
    }

    return false;
}

bool LogicBurnData::isFolder(QModelIndex idx)
{
    DataDirectory *dir;
    dir = static_cast<DataDirectory *>(idx.internalPointer());
    if (dir && dir->isDirectory() /* && dir->count() > 0 */) return true;
    return false;
}

bool LogicBurnData::checkAdd(QModelIndex idx)
{
    DataDirectory *dir = static_cast<DataDirectory *>(idx.internalPointer());

    DataCDROM *cdrom = nullptr;
    if (-1 != m_iIndex && m_iIndex < m_oBurners.size())
        cdrom = static_cast<DataCDROM *>(m_oBurners.at(m_iIndex));

       bool mediumCheck = false;
       bool directoryCheck = false;
       bool diskCheck = false;

      if (cdrom && !cdrom->isUdf() && cdrom->mediumStatus() == DataMedium::MEDIUM_FINALIZED)
          mediumCheck = true;

      if(cdrom){
          MEDIUM_SYSTEM_TYPE sysType = cdrom->fileSystemTypeWithExtendName();
          if(sysType == UDF201){
              mediumCheck = true;
          }
      }

      if (nullptr == dir || !dir->isDirectory())
          directoryCheck = true;
      if (dir && dir->isDisk())
          diskCheck = true;

      if(mediumCheck || directoryCheck || diskCheck)
      {
          emit addEnabled(false);
          emit folderEnabled(false);
          return false;
      }
      else
      {
          emit addEnabled(true);
          emit folderEnabled(true);
          return true;
      }
}

bool LogicBurnData::checkAdd(QModelIndexList idxes)
{
    if (idxes.size() > 1) return false;
    return checkAdd(idxes.first());
}

bool LogicBurnData::checkDel(QModelIndexList idxes)
{
    if (0 == idxes.size())
    {
        emit delEnabled(false);
        return false;
    }

    while (idxes.size())
    {
        DataDirectory *dir = static_cast<DataDirectory *>(idxes.takeFirst().internalPointer());
        DataFile *file = static_cast<DataFile *>(dir);
        if (!file)
        {
            emit delEnabled(false);
            return false;
        }
        if (dir && dir->isDirectory() && (dir->isDisk() || dir->isRoot()))
        {
            emit delEnabled(false);
            return false;
        }

        if (file && file->isDisk())
        {
            emit delEnabled(false);
            return false;
        }
        if (file && ".." == file->display())
        {
            emit delEnabled(false);
            return false;
        }
    }
    emit delEnabled(true);
    return true;
}

bool LogicBurnData::checkCls(QModelIndex idx)
{
    DataDirectory *dir = static_cast<DataDirectory *>(idx.internalPointer());
    if (nullptr == dir || !dir->isDirectory())
    {
        emit clsEnabled(false);
        return false;
    }
    if (dir->isDisk())
    {
        emit clsEnabled(false);
        return false;
    }
     bool ret = dir->canClean();
     emit clsEnabled(ret);
     return ret;
}

bool LogicBurnData::checkDefaultAddpage(QModelIndex idx)
{
    DataDirectory *dir = static_cast<DataDirectory *>(idx.internalPointer());
    if (nullptr == dir)
    {
        emit defaultAddPageEnabled(true);
        return true;
    }

    if (dir->isRoot() && !m_oRoots.contains((void *)dir)) {
        LOGGER->warn(_("checkDefaultAddpage: stale root DataDirectory* detected, dir=%1 not in m_oRoots(size=%2), idx.row=%3 - skip to avoid UAF crash")
                     .arg((qulonglong)dir, 0, 16).arg(m_oRoots.size()).arg(idx.row()));
        emit defaultAddPageEnabled(true);
        return true;
    }

    if (dir->isRoot() && dir->count() == 0 && !MainWindow::Instance()->currentDataRomIsUDF())
    {
        emit defaultAddPageEnabled(true);
        return true;
    }
    emit defaultAddPageEnabled(false);
    return false;
}

bool LogicBurnData::checkOpen(QModelIndexList idxes)
{
    if (idxes.size() != 1)
        return false;
    DataFile *file = static_cast<DataFile *>(idxes.first().internalPointer());
    if (nullptr == file)
        return false;
    if (!file->isDirectory())
        return false;
    DataDirectory *dir = static_cast<DataDirectory *>(file);
    Q_UNUSED(dir);
    return true;
}

bool LogicBurnData::checkOpenLocal(QModelIndexList idxes)
{
    if (idxes.size() != 1)
        return false;
    DataFile *file = static_cast<DataFile *>(idxes.first().internalPointer());

    if (nullptr == file)
        return false;
    if (_("NewFolder") == file->type())
        return false;
    return true;
}

bool LogicBurnData::checkProperties(QModelIndexList idxes)
{
    if (idxes.size() != 1)
        return false;

    DataFile *file = static_cast<DataFile *>(idxes.first().internalPointer());
    if (nullptr == file)
        return false;
    if (_("NewFolder") == file->type())
        return false;
    if (".." == file->display())
        return false;
    return true;
}

/**
 * @brief 检测是否有Item被选中
 * @param idxes 一个Item索引列表QList<QModelIndex>
 * @return
 */
bool LogicBurnData::checkSelectModelItem(QModelIndexList idxes)
{
    if (0 == idxes.size()) return false;
    else
        return true;

}

bool LogicBurnData::checkSize()
{
    DataCDROM *cdrom;
//    QStorageInfo storage;

    if (0 == m_oRoots.size()) return false;
    if (m_iIndex < 0 || m_iIndex >= m_oRoots.size())
        return false;

    DataDirectory *dir = static_cast<DataDirectory *>(m_oRoots.at(m_iIndex));
#if 1
    //既要添加的0B的文件能刻录，也要避免没有添加文件btnBusnes按钮置灰
    //使用文件数量代替文件大小来做判断条件
    if(dir->CountOneLevelNotDiskFile()<=0)  return false;
#else
    //使容量进行判断是否添加了新文件
    if (0 == dir->nSize()) return false;
#endif
    if (m_iIndex >= m_oBurners.size()) return false;
    cdrom = static_cast<DataCDROM *>(m_oBurners.at(m_iIndex));


    //bug120965,155809,解决由于要刻录的文件容量大于光盘剩余容量造成的刻录错误，
    if (cdrom->isCDROM() )
    {
        if (dir->nSize() > cdrom->mediumFreeSize())
        return false;
    }
    return true;
}

bool LogicBurnData::checkTempPathAvailabeSize()
{
    DataDirectory *dir = static_cast<DataDirectory *>(m_oRoots.at(m_iIndex));
    DataCDROM *cdrom = static_cast<DataCDROM *>(m_oBurners.at(m_iIndex));

    QStorageInfo storage;
    storage.setPath(cdrom->settingTempPath());

    if (dir->size() > storage.bytesAvailable()) {
        return false;
    }
    return true;
}

QUrl LogicBurnData::convertTrashUrlToFileUrl(const QUrl &trashUrl)
{
    if(trashUrl.scheme() != "trash"){
        return trashUrl;
    }

    QString path = trashUrl.path();
    QString fullPath = QDir::homePath().append("/.local/share/Trash/files") + path;

    QUrl fileUrl;
    fileUrl.setScheme("file");
    fileUrl.setPath(fullPath);

    return fileUrl;
}

int LogicBurnData::createIsoByCommand(QString volumeName, QString destPath)
{
    //burn option
    BurnOptions opts;
    BurnJobManager::Config config;
    opts |= BurnOption::kJolietSupport;
    opts |= BurnOption::kKeepAppendable;

    config.opts = opts;
    config.volName = volumeName;


    QString output_path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    output_path.append("/").append(volumeName).append(".iso");

    QFileInfo file_info(output_path);
    file_info.setFile(output_path);
    if (file_info.exists())
    {
        bool result = MyMessageBox::showIsoReplaceMessageBox(file_info.fileName());
        if (!result) {
            return 0;
        }

        if (file_info.isFile()) QFile::remove(file_info.absoluteFilePath());
        else { QDir dir(file_info.absoluteFilePath()); dir.removeRecursively(); }
    }

    BurnJobManager::instance()->disconnect();
    connect(BurnJobManager::instance(), &BurnJobManager::logMsgGenerated, this, [&](int level, const QStringList msgList){
        for(QString msg : msgList){
            if(level == 0){
                LOGGER->error(msg);
            }else if(level == 1){
                LOGGER->info(msg);
            }
        }
    });

    QEventLoop loop;
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, &loop, &QEventLoop::quit);
    //start burn job
    BurnJobManager::instance()->startCreateISOImage(output_path, QUrl::fromLocalFile(destPath), config);
    loop.exec();
    return 0;
}

void LogicBurnData::progressWidgetDelayResponeTimer(int ms)
{
    QEventLoop eventloop;
    QTimer *dataDelayCopyTimer = new QTimer(this);
    connect(dataDelayCopyTimer, &QTimer::timeout, this, [&](){
        dataDelayCopyTimer->stop();
        eventloop.quit();
    });
    dataDelayCopyTimer->start(ms);
    eventloop.exec();
    dataDelayCopyTimer->deleteLater();
}

QString LogicBurnData::stringSize(unsigned long size)
{
    int unit;
    qreal rsize;
    QString ret;

    unit = -1;
    rsize = size * 1.0;

    while (rsize > 1024.00)
    {
        ++unit; rsize /= 1024.00;
    }

    ret = QString("%1 ").arg(rsize, 0, 'f', 1);

    switch (unit)
    {
    case 0:
        ret.append(_("KB"));
        break;
    case 1:
        ret.append(_("MB"));
        break;
    case 2:
        ret.append(_("GB"));
        break;
    default:
        ret.append(_("B"));
        break;
    }
    return ret;
}

unsigned int LogicBurnData::level(QModelIndex idx)
{
    DataFile *data = static_cast<DataFile *>(idx.internalPointer());
    if (nullptr == data) return 0;
    return data->level();
}

int LogicBurnData::filter(QModelIndex idx)
{
    int ret = 0;
    DataDirectory *node = nullptr;
    if (!idx.isValid())
    {
        node = (DataDirectory *)(m_oRoots.at(m_iIndex));
    }
    else
    {
        node = static_cast<DataDirectory *>(idx.internalPointer());
    }

    if (node)
    {
        FileFilterRules::instance()->init(node);
    }
    return ret;
}

bool LogicBurnData::resetFileterConfigToModel()
{
    DataDirectory *node = (DataDirectory *)(m_oRoots.at(m_iIndex));
    if (node)
    {
        FileFilterRules::instance()->init(node);
    }
    return true;
}


int LogicBurnData::doFilterHidden(QModelIndex idx)
{
    int ret = 0;
    DataDirectory *node = nullptr;

    if (!idx.isValid())
    {
        node = (DataDirectory *)(m_oRoots.at(m_iIndex));
    }
    else
    {
        node = static_cast<DataDirectory *>(idx.internalPointer());
    }

    if (!node)
    {
        return ret;
    }

    node->execHidden();

    if (idx.isValid())
    {
        emit rootIndex(idx);
    }
    else
    {
        emit rootIndex(createIndex(m_iIndex, 0, node));
    }

    return ret;
}

int LogicBurnData::doRecoveryHidden(QList<void *> data, QModelIndex idx)
{
    DataFile *file;
    DataDirectory *parent;
    DataDirectory *root;

    if (data.size() == 0)
    {
        if (!idx.isValid())
        {
            if((m_iIndex < 0) || (m_iIndex >= m_oRoots.size())){
                 return -1;
            }
            parent = (DataDirectory *)(m_oRoots.at(m_iIndex));
        }
        else
        {
            parent = static_cast<DataDirectory *>(idx.internalPointer());
        }

        parent->execHidden(false);
    }

    while (data.size())
    {
        QCoreApplication::processEvents();
        file = static_cast<DataFile *>(data.takeFirst());
        parent = qobject_cast<DataDirectory *>(file->parent());
        root = qobject_cast<DataDirectory *>(parent->parent());
        if (root)
        {
            beginInsertRows(createIndex(root->findIndex(parent), 0, parent), parent->counts(), 1);
        }
        else
        {
            beginInsertRows(createIndex(m_iIndex, 0, m_oRoots.at(m_iIndex)), parent->counts(), 1);
        }
        parent->execHidden(false);
        parent->append(file);
        emit file->sizeChanged(file->size(), true);
        connect(parent, &DataDirectory::folderHidden, file, &DataFile::slotsDoHidde,Qt::UniqueConnection);
        endInsertRows();
    }
    return 0;
}

int LogicBurnData::doFilterBroken(QModelIndex idx)
{
    int ret = 0;
    DataDirectory *node = nullptr;

    if (!idx.isValid())
        node = (DataDirectory *)(m_oRoots.at(m_iIndex));
    else
        node = static_cast<DataDirectory *>(idx.internalPointer());

    if (!node)
        return ret;

    node->execBroken();

    if (idx.isValid())
        emit rootIndex(idx);
    else
        emit rootIndex(createIndex(m_iIndex, 0, node));

    return ret;
}

int LogicBurnData::doRecoveryBroken(QList<void *> data, QModelIndex idx)
{
    DataFile *file;
    DataDirectory *parent;
    DataDirectory *root;

    if (data.size() == 0)
    {
        if (!idx.isValid()){

            if((m_iIndex < 0) || (m_iIndex >= m_oRoots.size())){
                 return -1;
            }

            parent = (DataDirectory *)(m_oRoots.at(m_iIndex));
        }else{
            parent = static_cast<DataDirectory *>(idx.internalPointer());
        }
        parent->execBroken(false);
    }

    while (data.size())
    {
        QCoreApplication::processEvents();
        file = static_cast<DataFile *>(data.takeFirst());
        parent = qobject_cast<DataDirectory *>(file->parent());
        root = qobject_cast<DataDirectory *>(parent->parent());
        if (root)
            beginInsertRows(createIndex(root->findIndex(parent), 0, parent), parent->counts(), 1);
        else
            beginInsertRows(createIndex(m_iIndex, 0, m_oRoots.at(m_iIndex)), parent->counts(), 1);
        parent->execBroken(false);
        parent->append(file);
        emit file->sizeChanged(file->size(), true);
        connect(parent, &DataDirectory::folderBroken, file, &DataFile::slotsDoBroken);
        endInsertRows();
    }
    return 0;
}

int LogicBurnData::doRecoveryRepleace(QList<void *> data,QModelIndex idx)
{
    DataFile *file;
    DataDirectory *parent;
    DataDirectory *root;

    if (0 == data.size())
    {
        if((m_iIndex < 0) || (m_iIndex >= m_oRoots.size())){
            return -1;
        }

        parent = static_cast<DataDirectory *>(m_oRoots.at(m_iIndex));
        parent->execRepleace(false);
    }

    while (data.size())
    {
        QCoreApplication::processEvents();
        file = static_cast<DataFile *>(data.takeFirst());
        parent = qobject_cast<DataDirectory *>(file->parent());
        root = qobject_cast<DataDirectory *>(parent->parent());

        if (root)
            beginInsertRows(createIndex(root->findIndex(parent), 0, parent), parent->counts(), parent->counts());
        else
            beginInsertRows(createIndex(m_iIndex, 0, m_oRoots.at(m_iIndex)), parent->counts(), parent->counts());

        parent->execRepleace(false);

        //解决122706的闪退问题，功能还有问题。
        //badsoftlink or normal link
        if(parent->take(file->linkTarget()))
        {
            parent->append(file);
        }
        else
        {
            file->disconnect();
            connect(file,&DataFile::destroyed,this,[&]{file = nullptr;},Qt::UniqueConnection);
            file->deleteLater();
        }
        if(file)
        {
            connect(parent, &DataDirectory::folderReplaced, file, &DataFile::slotsDoRepleace);
        }
        endInsertRows();
    }
    return 0;
}

void LogicBurnData::fixFileDstPathByIndex(QString newPath, QModelIndex idx)
{
    DataDirectory *dir = static_cast<DataDirectory *>(idx.internalPointer());
    if (dir->isDirectory())
    {
        dir->setFileDestination(newPath);
    }
    else
    {
        DataFile *file = static_cast<DataFile *>(idx.internalPointer());
        QString s = file->display();
        int ret = s.lastIndexOf('/');
        if(ret == -1)
            return;
        s.remove(ret+1, s.length()-ret+1);
        s.append(newPath);
        s = s + '/' + file->display();
        file->setDestination(s);
    }
}

int LogicBurnData::findTheRootIndexofFile(DataFile *inputFile)
{
    DataDirectory* parentDir = static_cast<DataDirectory *>(inputFile->parent());
    int limitTimes = 100;
    while(!m_oRoots.contains(parentDir))
    {
        parentDir = static_cast<DataDirectory *>(parentDir->parent());
        if(--limitTimes < 0)
            break;
    }
    return m_oRoots.indexOf(parentDir);
}

int LogicBurnData::recoverHiddenFilesByDeviceIndex(QList<void *> dataList, int deviceIndex)
{
    DataDirectory *parent;
    DataDirectory *root;

    if((deviceIndex >= m_oRoots.size()) || (dataList.size() == 0))
    {
        return -1;
    }

    while (dataList.size())
    {
        QCoreApplication::processEvents();
        DataFile *file = static_cast<DataFile *>(dataList.takeFirst());
        parent = qobject_cast<DataDirectory *>(file->parent());
        root = qobject_cast<DataDirectory *>(parent->parent());
        if (root)
        {
            beginInsertRows(createIndex(root->findIndex(parent), 0, parent), parent->counts(), parent->counts());
        }
        else
        {
            beginInsertRows(createIndex(deviceIndex, 0, m_oRoots.at(deviceIndex)), parent->counts(), parent->counts());
        }
        parent->execHidden(false);
        parent->append(file);
        emit file->sizeChanged(file->size(), true);
        connect(parent, &DataDirectory::folderHidden, file, &DataFile::slotsDoHidde,Qt::UniqueConnection);
        endInsertRows();
    }

    return 0;
}

int LogicBurnData::recoverBrokenFilesByDeviceIndex(QList<void *> dataList, int deviceIndex)
{
    DataDirectory *parent = nullptr;
    if((deviceIndex >= m_oRoots.size()) || (dataList.size() == 0))
    {
        return -1;
    }
    while (dataList.size())
    {
        QCoreApplication::processEvents();
        DataFile *file = static_cast<DataFile *>(dataList.takeFirst());
        parent = qobject_cast<DataDirectory *>(file->parent());

        DataDirectory *root = qobject_cast<DataDirectory *>(parent->parent());
        if (root)
            beginInsertRows(createIndex(root->findIndex(parent), 0, parent), parent->counts(), parent->counts());
        else
            beginInsertRows(createIndex(deviceIndex, 0, m_oRoots.at(deviceIndex)), parent->counts(), parent->counts());

        parent->execBroken(false);
        parent->append(file);
        emit file->sizeChanged(file->size(), true);
        connect(parent, &DataDirectory::folderBroken, file, &DataFile::slotsDoBroken);
        endInsertRows();
    }
    return 0;
}

int LogicBurnData::recoverReplaceLinkFilesByDeviceIndex(QList<void *> dataList, int deviceIndex)
{
    DataDirectory *parent;
    if((deviceIndex >= m_oRoots.size()) || (0 == dataList.size()))
    {
        return -1;
    }

    while (dataList.size())
    {
        QCoreApplication::processEvents();
        DataFile *file = static_cast<DataFile *>(dataList.takeFirst());
        parent = qobject_cast<DataDirectory *>(file->parent());
        DataDirectory *root = qobject_cast<DataDirectory *>(parent->parent());

        if (root)
            beginInsertRows(createIndex(root->findIndex(parent), 0, parent), parent->counts(), parent->counts());
        else
            beginInsertRows(createIndex(deviceIndex, 0, m_oRoots.at(deviceIndex)), parent->counts(), parent->counts());

        parent->execRepleace(false);

        //解决122706的闪退问题，功能还有问题。
        //badsoftlink or normal link
        if(parent->take(file->linkTarget()))
        {
            parent->append(file);
        }
        else
        {
            file->disconnect();
            connect(file,&DataFile::destroyed,this,[&]{file = nullptr;},Qt::UniqueConnection);
            file->deleteLater();
        }
        if(file)
        {
            connect(parent, &DataDirectory::folderReplaced, file, &DataFile::slotsDoRepleace);
        }
        endInsertRows();
    }
    return 0;
}

int LogicBurnData::getDeviceCount()
{
    return m_oRoots.count();
}

bool LogicBurnData::updateListOrder()
{
    QList<void *> m_oTempRoots;
    QList<void *> m_oTempBurners;

    int i = 0;
    for (i = 0; i < MainWindow::Instance()->getCountDevice(); i++) {
        QString dev = MainWindow::Instance()->getDeviceByIndex(i);
        int j = 0;
        for(j = 0; j < m_oBurners.size(); j++) {
            DataCDROM *cdrom = static_cast<DataCDROM*>(m_oBurners[j]);
            if (cdrom->device() == dev) {
                m_oTempBurners <<  m_oBurners[j];
                m_oTempRoots << m_oRoots[j];
                break;
            }
        }
        if (j > m_oBurners.size())
            return false;
    }
    if (i > m_oBurners.size())
        return false;

    if (m_oBurners.size() != m_oTempBurners.size())
        return false;

    m_oBurners.clear();
    m_oRoots.clear();

    for (int z = 0; z < m_oTempBurners.size(); z++) {
        m_oBurners << m_oTempBurners[z];
        m_oRoots << m_oTempRoots[z];
    }
    return true;
}

int LogicBurnData::doFilterRepleace(QModelIndex idx)
{
    int ret = 0;
    DataDirectory *node = nullptr;
    if (!idx.isValid())
    {
        node = (DataDirectory *)(m_oRoots.at(m_iIndex));
    }
    else
    {
        node = static_cast<DataDirectory *>(idx.internalPointer());
    }
    if (!node)
        return ret;

    node->execRepleace();

    if (node->linkTarget())
    {
        DataDirectory *link = static_cast<DataDirectory *>(node->linkTarget());
        if (link)
        {
            link->execRepleace();
        }
    }

    if (idx.isValid())
    {
        emit rootIndex(idx);
    }
    else
    {
        emit rootIndex(createIndex(m_iIndex, 0, node));
    }

    return ret;
}

void LogicBurnData::changeIndex(int idx)
{
    if (idx < 0 || idx >= m_oRoots.size())
        return;

    //更新列表排序，与mainWindow保持一致
    updateListOrder();

    DataDirectory *root = static_cast<DataDirectory *>(m_oRoots.at(idx));
    if (root)
    {
        m_iIndex = idx;
        emit rootIndex(createIndex(idx, 0, root));
        emit sizeChanged(root->size());
    }
}

int LogicBurnData::addDatasToIndex(QStringList files, QModelIndex idx)
{
    DataDirectory *root;
    DataCDROM     *cdrom;

    m_addAction_canceled = false;
    m_file_device_index = m_iIndex;

    DataDirectory *parent = static_cast<DataDirectory *>(idx.internalPointer());
    if (nullptr == parent || !parent->isDirectory())
        return 0;

    int ret = 0;
    QList<DataDirectory *>parents;
    parents << parent;
    QString str = parent->display();

    //add file to all device
    if (Configuration::instance()->get("DataSetting", "Add Data", true).toBool())
    {
        for (int i = 0; i < m_oRoots.size(); ++i)
        {
            if (i == m_iIndex)
                continue; // current idx has saved.
            root = (DataDirectory *)(m_oRoots.at(i));

            if (parent->isRoot())
                parent = (DataDirectory *)(m_oRoots.at(i));
            else
                parent = (DataDirectory *)(root->findChildByDisplay(str));

            cdrom = static_cast<DataCDROM *>(m_oBurners.at(i));

            if (cdrom && cdrom->mediumStatus() == DataMedium::MEDIUM_FINALIZED)
                continue;

            if (parent)
                parents << parent;
        }
    }

    ModalDialog::instance()->showPage(ModalDialog::Progress);
    ModalDialog::instance()->show();

    emit title(_("Add File(s)"));
    emit setRange(0, parents.size() * calcFileCount(files));
    emit setValue(0);

    parent = nullptr;

    QFileInfo fileInfo;
    foreach (parent, parents)
    {
        root = qobject_cast<DataDirectory *>(parent->parent());
        if (nullptr == root)
            idx = createIndex(m_oRoots.indexOf(parent), 0, parent);
        else
            idx = createIndex(root->findIndex(parent), 0, parent);

        parent->setModeIndex(idx);

        beginInsertRows(idx, parent->counts(), parent->counts() + files.size() - 1);
        foreach (str, files)
        {
            QCoreApplication::processEvents();
            fileInfo.setFile(str);
            if (!fileInfo.exists())
            {
                emit addValue(1);
                continue;
            }
            if (fileInfo.isDir())
            {
                if (2 == Configuration::instance()->get("DataSetting", "Duplicate Data", 0).toInt())
                {
                    /* 重名文件选择‘替换’*/
                    //replace same name dir
                    DataFile *dataFile = (DataFile*)(parent->findChildByDisplay(fileInfo.fileName(),true));
                    bool isExistDuplicateDir = false;
                    if(dataFile && dataFile->isDirectory())
                    {
                        isExistDuplicateDir = true;
                    }
                    if (isExistDuplicateDir)
                    {
                        /* 有重名目录，删除目录所连的ModelIndex */
                        BurnData::instance()->delModelIndex(dataFile->getModelIndex());
                    }
                }
                ret += addDirectoy(fileInfo.absoluteFilePath(), parent);
            }
            else
            {
                ret += addFile(fileInfo.absoluteFilePath(), parent);
            }

            if (m_addAction_canceled)
                break;
        }
        endInsertRows();
    }
    disconnect(ModalDialog::instance());
    return ret;
}


int LogicBurnData::deleteDatasFromIndex(QModelIndexList idxes, QModelIndex inputParent)
{
    //input check
    if (0 == idxes.size())
        return 0;

    DataDirectory *parent = static_cast<DataDirectory *>(inputParent.internalPointer());
    if (nullptr == parent || !parent->isDirectory())
        return 0;

    DataFile *child = static_cast<DataFile *>(idxes.at(0).internalPointer());
    if (nullptr == child)
        return 0;

    QList<DataDirectory *>parents;
    parents << parent;
    QString str = parent->display();

    //search all root parent
    if (Configuration::instance()->get("DataSetting", "Delete Data", false).toBool())
    {
        DataDirectory *root = nullptr;
        for (int index = 0; index < m_oRoots.size(); ++index)
        {
            // current idx has saved.
            if (index == m_iIndex)
                continue;

            root = (DataDirectory *)(m_oRoots.at(index));

            if (parent->isRoot())
            {
                parent = (DataDirectory *)(m_oRoots.at(index));
            }
            else
            {
                parent = (DataDirectory *)(root->findChildByDisplay(str));

            }
            if (parent)
            {
                parents << parent;
            }
        }
    }

    QStringList childNames;
    foreach (QModelIndex idx, idxes)
    {
        QCoreApplication::processEvents();
        child = static_cast<DataFile *>(idx.internalPointer());
        if (".." == child->display())
            continue;
        childNames << child->display();
    }

    DataDirectory *rootIndexDir = nullptr;
    int size = 0;
    foreach (parent, parents)
    {
        QCoreApplication::processEvents();
        rootIndexDir = qobject_cast<DataDirectory *>(parent->parent());
        if (nullptr == rootIndexDir)
            inputParent = createIndex(m_oRoots.indexOf(parent), 0, parent);
        else
            inputParent = createIndex(rootIndexDir->findIndex(parent), 0, parent);

        size = 0;

        child = nullptr;
        foreach (str, childNames)
        {
            if (parent->findChildByDisplay(str, true))
            {
                ++size;
            }

            if (nullptr == child)
            {
                child = (DataFile *)(parent->findChildByDisplay(str, true));
            }
        }
        //@1 parent @2 firstIndex  @3 last
        beginRemoveRows(inputParent, parent->findIndex(child), parent->findIndex(child) + size - 1);
        foreach (str, childNames)
        {
            QCoreApplication::processEvents();
            child = (DataFile *)(parent->findChildByDisplay(str, true));
            if (child && ".." != child->display() && !child->isDisk())
            {
                parent->take(child);
                emit child->sizeChanged(child->size(),false);
                //delete child;
                //child = nullptr;
            }
        }
        endRemoveRows();
    }
    if (parents.first()->count() == 0)
    {
        rootIndexDir = qobject_cast<DataDirectory *>(parents.first()->parent());
        if (rootIndexDir)
        {
            if (-1 != m_oRoots.indexOf(rootIndexDir))
            {
                emit rootIndex(createIndex(m_oRoots.indexOf(rootIndexDir), 0, rootIndexDir));
            }
            else
            {
                parent = qobject_cast<DataDirectory *>(rootIndexDir->parent());
                if (parent)
                {
                    emit rootIndex(createIndex(parent->findIndex(rootIndexDir), 0, rootIndexDir));
                }
            }
        }
        else
        {
            emit rootIndex(createIndex(m_oRoots.indexOf(parents.first()), 0, parents.first()));
        }

    }

    return 0;
}

int LogicBurnData::clearDatasUnderIndexd(QModelIndex parent)
{
    int ret;
    DataDirectory *p;

    p = static_cast<DataDirectory *>(parent.internalPointer());
    if (nullptr == p || !p->isDirectory()) return 0;
    ret = 0;
    beginRemoveRows(parent, 0, p->count());
    p->clear();
    endRemoveRows();
    if (p->isRoot())
        rootIndex(createIndex(m_oRoots.indexOf(p), 0, p));
    else
        rootIndex(parent.parent().parent());
    return ret;
}

int LogicBurnData::clearDatasUnderIndex(QModelIndex p)
{
    if(!p.isValid()){
        return -1;
    }
    DataDirectory *root = nullptr;
    QList<DataDirectory *>parents;

    int ret = 0;
    DataDirectory *parent = static_cast<DataDirectory *>(p.internalPointer());
    if (nullptr == parent || !parent->isDirectory())
        return 0;

    QString str = parent->display();
    parents.clear();
    parents << parent;
    if (CFG->get("DataSetting", "Clean Data", false).toBool())
    {
        for (int i = 0; i < m_oRoots.size(); ++i)
        {
            if (i == m_iIndex) continue; // current idx has saved.
            root = (DataDirectory *)(m_oRoots.at(i));
            if (parent->isRoot())
                parent = (DataDirectory *)(m_oRoots.at(i));
            else
                parent = (DataDirectory *)(root->findChildByDisplay(str));
            if (parent) parents << parent;
        }
    }

    foreach (parent, parents)
    {
        root = qobject_cast<DataDirectory *>(parent->parent());
        if (nullptr == root)
            p = createIndex(m_oRoots.indexOf(parent), 0, parent);
        else
            p = createIndex(root->findIndex(parent), 0, parent);
        beginRemoveRows(p, 0, parent->count());
        ret += parent->count();
        parent->clear();
        endRemoveRows();
    }

    if (parents.first()->isRoot())
        rootIndex(createIndex(m_oRoots.indexOf(parents.first()), 0, parents.first()));
    else if (p.parent().isValid())
        rootIndex(p.parent());
    else
        rootIndex(createIndex(m_oRoots.indexOf(parents.first()), 0, parents.first()));

    return ret;
}

bool LogicBurnData::duplicatedName(QString name, QModelIndex p)
{
    DataDirectory *parent = static_cast<DataDirectory *>(p.internalPointer());
    if (nullptr == parent)
        return true;

    return parent->duplicate(name);
}

void LogicBurnData::addEmptyFolder(QString name, QModelIndex inputParent)
{
    DataDirectory *parentDir = static_cast<DataDirectory *>(inputParent.internalPointer());
    if (nullptr == parentDir || !parentDir->isDirectory())
        return;

    QString path = parentDir->destination() + "/" + name;
    beginInsertRows(inputParent, parentDir->count() - 1, 1);
    DataDirectory *item = new DataDirectory(path, parentDir);
    parentDir->appendOrDeleteSubfile(item,true);
    endInsertRows();
}

void LogicBurnData::deleteRoot(int idx)
{
    if (idx >= m_oRoots.size() || idx < 0)
        return;

    if(m_current_item != nullptr){
        bool is_rename_dlg_show = m_current_item->isRenameDlgShown();
        if(is_rename_dlg_show && (m_file_device_index == idx)){
            m_addAction_canceled = true;
            qDebug() << "add_action_is canceled";
            m_current_item->closeInputChangeDlg();
        }
    }

    if(ExecDialog::instance()->isVisible()){
       ExecDialog::instance()->accept();
    }

    QMetaObject::invokeMethod(this, "slotsAsynDeleteRoot", Qt::QueuedConnection, Q_ARG(int, idx));
}

void LogicBurnData::slotRestoreAllFiles(QList<void *> hiddens, QList<void *> brokens, QList<void *> replaces)
{
    QModelIndex idx;
    DataDirectory *root = nullptr;
    DataDirectory *parent = nullptr;
    DataFile *child = nullptr;

    while (hiddens.size()) {
        child = static_cast<DataFile *>(hiddens.takeFirst());
        if (nullptr == child)
            continue;
        parent = static_cast<DataDirectory *>(child->parent());
        root = static_cast<DataDirectory *>(parent->parent());

        if (parent->isRoot()) {
            idx = createIndex(m_oRoots.indexOf(parent), 0, parent);
        }
        else {
            idx = createIndex(root->findIndex(parent), 0, parent);
        }

        beginInsertRows(idx, parent->counts(), 1);
        parent->execHidden(false);
        parent->append(child);
        endInsertRows();
        connect(parent, &DataDirectory::folderHidden, child, &DataFile::slotsDoHidde, Qt::UniqueConnection);
    }


    while (brokens.size()) {
        child = static_cast<DataFile *>(brokens.takeFirst());
        if (nullptr == child) continue;
        parent = dynamic_cast<DataDirectory *>(child->parent());
        root = dynamic_cast<DataDirectory *>(parent->parent());

        if(parent->isRoot()) {
            idx = createIndex(m_oRoots.indexOf(parent), 0, parent);
        }
        else {
            idx = createIndex(root->findIndex(parent), 0, parent);
        }

        beginInsertRows(idx, parent->counts(), 1);
        parent->execBroken(false);
        parent->append(child);
        endInsertRows();
        connect(parent, &DataDirectory::folderBroken, child,&DataFile::slotsDoBroken,Qt::UniqueConnection);
    }

    if (replaces.size()) {
        doRecoveryRepleace(replaces,QModelIndex());
    }

    root = (DataDirectory *)(m_oRoots.at(m_iIndex));
    if(root && root->isRoot()){
        root->setNodeFilterHidden(false);
        root->setNodeRepleaceSymbolLinks(false);
        root->setNodeFilterBrokenSymbolLinks(false);
    }

    emit rootIndex(QModelIndex());
}

void LogicBurnData::stopLoadingData()
{
    m_bAppRunning = false;
}

void LogicBurnData::slotsSetting()
{
    ModalDialog::instance()->showPage(ModalDialog::BusnessSetting);
    ModalDialog::instance()->show();

    if((m_iIndex < 0) || (m_iIndex >= m_oBurners.size())){
        return;
    }

    setting(m_oBurners.at(m_iIndex));
}

void LogicBurnData::slotsCreateISO(void *inputCDrom)
{
    //inputdevice is not used or exist exit
    DataCDROM *cdromDevice  = (DataCDROM *)inputCDrom;
    if(cdromDevice->m_is_using <= 0) return;
    if (-1 != m_oBurners.indexOf(cdromDevice)) return;

    LOGGER->debug(_("Add new create iso data file. %1").arg(cdromDevice->display()));

    //get device map path
    QString filePath = cdromDevice->settingTempPath();
    filePath.append("/");
    filePath.append(cdromDevice->settingVolumnID());

    //make deivce path
    QDir dir;
    dir.setPath(filePath);
    if (dir.exists()) {
        dir.removeRecursively();
    }
    dir.mkpath(filePath);

    //record root dirpath
    DataDirectory * root_dir = new DataDirectory(filePath, this);
    m_oRoots << root_dir;
    m_oBurners << cdromDevice;

    m_mutex_list.push_back(new QMutex());

    //notify the ui
    emit addEnabled(true);
    emit folderEnabled(true);

    connect(root_dir, SIGNAL(updateSize(ulong)), this, SLOT(slotsUpdateSize(ulong)));
    connect(cdromDevice, SIGNAL(volumnID(QString)), this, SLOT(slotsUpdateCDROM()));
    connect(cdromDevice, SIGNAL(tempPath(QString)), this, SLOT(slotsUpdateCDROM()));
    connect(LogicMain::instance(), &LogicMain::storagePath, cdromDevice, &DataCDROM::changeISOPath);

    QStringList filepathStrlist;
    if (!(cdromDevice->mediumMountPoint().isEmpty())) {
        dir.setPath(cdromDevice->mediumMountPoint());
        foreach (QFileInfo fileinfo, dir.entryInfoList(QDir::Files | QDir::AllEntries | QDir::Hidden)) {
            if ("." == fileinfo.fileName() || ".." == fileinfo.fileName()) continue;
            filepathStrlist << fileinfo.absoluteFilePath();
        }

        //add data
        if (filepathStrlist.size()) {
            addDiskData(filepathStrlist, root_dir);
        }

        if (cdromDevice->mediumStatus() == DataMedium::MEDIUM_FINALIZED) {
            cdromDevice->updateMediumUsedSize(root_dir->size());
        }
    }
    else {
        ModalDialog::instance()->accept();
    }

    emit g_user_signal->sigInitCreateISO(cdromDevice);
}

void LogicBurnData::slotsCreateISO()
{
    if((m_iIndex < 0) || (m_iIndex >= m_oBurners.size())){
        return;
    }

    DataCDROM *cdrom = (DataCDROM*)m_oBurners.at(m_iIndex);
    if (!cdrom->settingIsShowBeforBurn()) {
        m_isContinue = true;
    }
    else {
        m_isContinue = false;
        connect(ModalDialog::instance(), &QDialog::rejected, [&](){
            this->m_isContinue = true;
        });

        ModalDialog::instance()->showPage(ModalDialog::BusnessSetting);
        emit setting(m_oBurners.at(m_iIndex));
        ModalDialog::instance()->wexec();
    }

    if(false == checkTempPathAvailabeSize()) {
        QString errInfo = _("The available space of the current temporary path is insufficient. Please re-select the temporary path!");
        Tip_BurnResult::instance()->slotsWarn(errInfo);
        BurnData::instance()->refresh();
        m_isContinue = false;
    }

    if (m_isContinue == false) {
        return;
    }

    //burn option
    BurnOptions opts;
    BurnJobManager::Config config;
    opts |= BurnOption::kJolietSupport;
    opts |= BurnOption::kKeepAppendable;

    config.opts = opts;
    config.speeds = cdrom->settingSpeed();
    config.volName = cdrom->settingVolumnID();

    QString destPath = cdrom->settingTempPath();
    if (destPath.endsWith("/")) {
        destPath = destPath + cdrom->settingVolumnID();
    } else{
        destPath = destPath + '/' + cdrom->settingVolumnID();
    }

    QString volumeId = cdrom->getSettings()->getVolumnID();
    QString output_path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    output_path.append("/").append(volumeId).append(".iso");

    QFileInfo file_info;
    file_info.setFile(cdrom->mDisplay());
    if(output_path != cdrom->mDisplay()){
        output_path = cdrom->mDisplay();
    }

    if (file_info.exists())
    {
        bool result = MyMessageBox::showIsoReplaceMessageBox(file_info.fileName());
        if (!result) {
            return;
        }

        if (file_info.isFile()) QFile::remove(file_info.absoluteFilePath());
        else { QDir dir(file_info.absoluteFilePath()); dir.removeRecursively(); }
    }

    QIcon progressIcon(QIcon::fromTheme("ukui-loading-0-symbolic"));
    Progress* progressDlg = static_cast<Progress*>(ModalDialog::instance()->showPage(ModalDialog::Progress));
    progressDlg->title( _("Creating ISO"));
    progressDlg->setsSetRange(0,100);
    progressDlg->slotsSetValue(0, progressIcon,_("Creating ISO"));
    ModalDialog::instance()->cancelEnable(false);
    ModalDialog::instance()->show();

    // auditlog
    QGSettings auditLogGSettings("org.ukui.kylinburner");
    QString auditLogPath="/var/log/kylin-burner/os_audit.log";
    bool isOpenAuditLog = auditLogGSettings.get("burneraudit-isrunning").toBool();
    AuditLog burnDataLog;

    //copy files to dest
    DataDirectory *filesDirectory = (DataDirectory*)(m_oRoots.at(m_iIndex));
    if (filesDirectory->nSize() > (4UL * 1024UL * 1024UL * 1024UL)) {
        int ret = QMessageBox::warning(MainWindow::Instance(), "",  _("Burn data more than 4GB, the burn operation may fail. Do you want to continue with the burn operation?"), _("Cancel"), _("OK"), "", 1);
        if (ret <= 0) {
            ModalDialog::instance()->hide();
            return;
        }
    }
    connect(filesDirectory, &DataDirectory::copied, this, [&](QString src, QString des){
        if (isOpenAuditLog) {
            auditLogGSettings.set("burner-os-auditlog-name",auditLogPath);
            burnDataLog.appendFile(src);
        }
    },Qt::UniqueConnection);

    progressWidgetDelayResponeTimer();
    filesDirectory->copy();
    if(!DataFile::isCopySuccess) {
        DataFile::isCopySuccess = true;
        LOGGER->info("(LogicBurnData::slotsCreateISO) 拷贝文件失败");
        ModalDialog::instance()->hide();
        Tip_BurnResult::instance()->slotsFailed( _("Creating ISO"));
        return;
    }

    //umount device
    bool umountSucceed = DataCDROM::umountMedium(cdrom->device());
    if (!umountSucceed) {
        LOGGER->error(QString("(ProcessBurnData::working) cdrom run umount:%1").arg(umountSucceed));
    }

    BurnJobManager::instance()->disconnect();
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, this, [&](int jobType, bool result){

        if (isOpenAuditLog) {
            auditLogGSettings.set("burner-os-auditlog-name",auditLogPath);
            burnDataLog.ouputCreateResult(cdrom->display(), cdrom->mDisplay(), true== result);;
        }

        ModalDialog::instance()->cancelEnable(false);
        ModalDialog::instance()->accept();

        if(result){
            Tip_BurnResult::instance()->slotsSuccess( _("Creating ISO"));
        }else{
            Tip_BurnResult::instance()->slotsFailed( _("Creating ISO"));
        }

    });

    connect(BurnJobManager::instance(), &BurnJobManager::currentJobProgressChanged, this, [&](int value){
        if(value >= 100){
            progressDlg->slotsSetValueWithoutClosing(value, _("Creating ISO"));
        }else{
            progressDlg->slotsSetValueWithoutClosing(value,  _("Creating ISO"));
        }
    },Qt::UniqueConnection);

    connect(BurnJobManager::instance(), &BurnJobManager::logMsgGenerated, this, [&](int level, const QStringList msgList){
        for(QString msg : msgList){
            if(level == 0){
                LOGGER->error(msg);
            }else if(level == 1){
                LOGGER->info(msg);
            }
        }
    });

    QEventLoop loop;
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, &loop, &QEventLoop::quit);
    //start burn job
    BurnJobManager::instance()->startCreateISOImage(output_path, QUrl::fromLocalFile(destPath), config);
    loop.exec();
}

// 将按钮点击与刻录数据功能分离
// 点击按钮后 设置页面自适应弹出
// 根据设置项确定iso9660还是udf格式

// 设置弹窗取消时保持不变状态

void LogicBurnData::slotsBurnData()
{
    if((m_iIndex < 0) || (m_iIndex >= m_oBurners.size())){
        return;
    }

    DataCDROM *cdrom = (DataCDROM*)m_oBurners.at(m_iIndex);
    if (cdrom->isOpen()) {
        cdrom->close();
    }

    if (!cdrom->settingIsShowBeforBurn()) {
        m_isContinue = true;
    } else {
        m_isContinue = false;
        connect(ModalDialog::instance(), &QDialog::rejected, [&](){
            this->m_isContinue = true;
        });
        ModalDialog::instance()->showPage(ModalDialog::BusnessSetting);
        emit setting(m_oBurners.at(m_iIndex));
        ModalDialog::instance()->wexec();
    }

    if(false == checkTempPathAvailabeSize()) {
        QString errInfo = _("The available space of the current temporary path is insufficient. Please re-select the temporary path!");
        Tip_BurnResult::instance()->slotsWarn(errInfo);
        BurnData::instance()->refresh();
        m_isContinue = false;
    }

    if (m_isContinue == false) {
        return;
    }

    QIcon progressIcon(QIcon::fromTheme("ukui-loading-0-symbolic"));
    m_progressDlg = static_cast<Progress*>(ModalDialog::instance()->showPage(ModalDialog::Progress));
    m_progressDlg->setsSetRange(0,100);
    m_progressDlg->slotsSetValue(0, progressIcon,  _("Prepare to start burn data."));
    ModalDialog::instance()->cancelEnable(false);

    if(cdrom->isEnableUDF() && cdrom->getBurnFormat() == "UDF") {
        QString progress_title;
        if (cdrom->mediumStatus() == DataMedium::MEDIUM_EMPTY) {
            progress_title = _("Burning UDF Append Data");
        } else {
            progress_title = _("Burning UDF Format Data");
        }
        m_progressDlg->title(progress_title);
        ModalDialog::instance()->show();

    } else {
        m_progressDlg->title(_("Burning Data"));
        ModalDialog::instance()->show();
    }


    progressWidgetDelayResponeTimer(100);
    if(cdrom->isEnableUDF() && cdrom->getBurnFormat() == "UDF") {
        slotsBurnUDFData();
    } else {
        slotsBurnFileByEngine();
    }
}

//刻录UDF数据的槽函数
void LogicBurnData::slotsBurnUDFData()
{
    BurnOptions opts;
    opts |= BurnOption::kJolietSupport;
    opts |= BurnOption::kKeepAppendable;

    DataCDROM *cdrom = (DataCDROM*)m_oBurners.at(m_iIndex);
    BurnJobManager::Config config;
    config.opts = opts;
    config.speeds = cdrom->settingSpeed();
    config.volName = cdrom->settingVolumnID();

    if (cdrom->checkUsingOfOtherApp(true)) {
        return;
    }

    QString destPath = cdrom->settingTempPath();
    if (destPath.endsWith("/")) {
        destPath = destPath + cdrom->settingVolumnID();
    } else{
        destPath = destPath + '/' + cdrom->settingVolumnID();
    }

    //umount device
    bool umountSucceed = DataCDROM::umountMedium(cdrom->device());
    if (!umountSucceed) {
        LOGGER->info(QString("(ProcessBurnData::working) cdrom run umount:%1").arg(umountSucceed));
    }

    QString inform_msg_title;
    if (cdrom->mediumStatus() == DataMedium::MEDIUM_EMPTY) {
        inform_msg_title = _("Burn  UDF Format Data");
    }
    else {
        inform_msg_title = _("UDF Append Burn");
    }

    // auditlog
    QGSettings auditLogGSettings("org.ukui.kylinburner");
    QString auditLogPath="/var/log/kylin-burner/os_audit.log";
    bool isOpenAuditLog = auditLogGSettings.get("burneraudit-isrunning").toBool();
    AuditLog burnDataLog;
    QString burnStatus ;
    if (cdrom->mediumStatus() == DataMedium::MEDIUM_EMPTY) {
        burnStatus = _("Burn Data");
    } else {
        burnStatus = _("Append Burn");
    }

    //copy files to dest
    DataDirectory *filesDirectory = (DataDirectory*)(m_oRoots.at(m_iIndex));
    if (filesDirectory->nSize() > (4UL * 1024UL * 1024UL * 1024UL)) {
        int ret = QMessageBox::warning(MainWindow::Instance(), "",  _("Burn data more than 4GB, the burn operation may fail. Do you want to continue with the burn operation?"), _("Cancel"), _("OK"), "", 1);
        if (ret <= 0) {
            ModalDialog::instance()->hide();
            return;
        }
    }
    connect(filesDirectory, &DataDirectory::copied, this, [&](QString src, QString des){
        if (isOpenAuditLog) {
            auditLogGSettings.set("burner-os-auditlog-name",auditLogPath);
            burnDataLog.appendFile(src);
        }
    },Qt::UniqueConnection);

    progressWidgetDelayResponeTimer();
    filesDirectory->copy();
    if(!DataFile::isCopySuccess) {
        DataFile::isCopySuccess = true;
        LOGGER->info("(LogicBurnData::slotsBurnUDFData) 拷贝文件失败");
        ModalDialog::instance()->hide();
        Tip_BurnResult::instance()->slotsFailed(inform_msg_title);
        return;
    }

    int ret = -1;
    if (cdrom->mediumStatus() == DataMedium::MEDIUM_EMPTY) {
        ret = formatUDF102();
    } else {
        ret = appendBurnDataUDF102();
    }

    if(ret != 0){
        LOGGER->error(QString("build udf data error cdrom status is :%1,return:%2").arg(cdrom->mediumStatus()).arg(ret));
        Tip_BurnResult::instance()->slotsFailed(inform_msg_title);
        return;
    }


    BurnJobManager::instance()->disconnect();
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobProgressChanged, this, [&](int value){
        qDebug() << "ProgressValue is:" << value;
        if(value >= 100){
            m_progressDlg->slotsSetValueWithoutClosing(value, _("Fixating(Will wait minutes)"));
        }else{
            m_progressDlg->slotsSetValueWithoutClosing(value, _("medium is burning..."));
        }
    },Qt::UniqueConnection);

    //Qt::UniqueConnection doesn't work
    disconnect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, 0, 0);
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, this, [&](int jobType, bool result){


        if (isOpenAuditLog) {
            auditLogGSettings.set("burner-os-auditlog-name",auditLogPath);
            burnDataLog.outputResult(cdrom->display(), cdrom->mediumType(), burnStatus, true==result);
        }

        emit g_user_signal->noMedium(cdrom);
        cdrom->eject();

        ModalDialog::instance()->cancelEnable(false);
        ModalDialog::instance()->accept();
        if(result){
            Tip_BurnResult::instance()->slotsSuccess(inform_msg_title);
        }else{
            Tip_BurnResult::instance()->slotsFailed(inform_msg_title);
        }

    });

    connect(BurnJobManager::instance(), &BurnJobManager::logMsgGenerated, this, [&](int level, const QStringList msgList){
        for(QString msg : msgList){
            if(level == 0){
                LOGGER->error(msg);
            }else if(level == 1){
                LOGGER->info(msg);
            }
        }
    });

    QEventLoop loop;
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, &loop, &QEventLoop::quit);
    //start burn job
    BurnJobManager::instance()->startBurnUDFFile(cdrom->device(), QUrl::fromLocalFile(QString("/tmp/ky_udf_image.iso")), config);
    loop.exec();

    for (int i = 0; i < m_oBurners.size(); ++i){
        if(m_oBurners.at(i) == cdrom){
            cdrom->m_bStart = false;
            break;
        }
    }
    return;
}

void LogicBurnData::slotsEmptyMedium(void *)
{
    LOGGER->debug(_("Add new create iso data file."));
}

void LogicBurnData::slotsUpdateCDROM()
{
    int idx = m_oBurners.indexOf(sender());
    if (-1 == idx)
        return;

    DataCDROM *cdrom = (DataCDROM *)(sender());
    DataDirectory *root = (DataDirectory *)(m_oRoots.at(idx));

    QString rootPath = cdrom->settingTempPath() + "/" + cdrom->settingVolumnID();
    root->rename(rootPath);

    emit sizeChanged(root->size());
    emit updateDisk(idx, sender());
}

void LogicBurnData::slotsDoFilterHidden(void *data)
{
    if (Configuration::instance()->get("DataSetting", "Add Data", true).toBool())
    {
        int index = findTheRootIndexofFile((DataFile*)data);
        if(index != -1)
        {
            emit hiddenFileFiltered(index, data);
        }
        else
        {
            emit hiddenFileFiltered(m_iIndex, data);
        }
    }
    else
    {
        emit hiddenFileFiltered(m_iIndex, data);
    }
}

void LogicBurnData::slotsDoFilterBroken(void *data)
{
    if (Configuration::instance()->get("DataSetting", "Add Data", true).toBool())
    {
        int index = findTheRootIndexofFile((DataFile*)data);
        if(index != -1)
        {
            emit brokenLinkfileFiltered(index, data);
        }
        else
        {
            emit brokenLinkfileFiltered(m_iIndex, data);
        }
    }
    else
    {
        emit brokenLinkfileFiltered(m_iIndex, data);
    }

}

void LogicBurnData::slotsDoRepleace(void *data)
{
    if (Configuration::instance()->get("DataSetting", "Add Data", true).toBool())
    {
        int index = findTheRootIndexofFile((DataFile*)data);
        if(index != -1)
        {
            emit replaceLinkFileFiltered(index, data);
        }
        else
        {
           emit replaceLinkFileFiltered(m_iIndex, data);
        }
    }
    else
    {
        emit replaceLinkFileFiltered(m_iIndex, data);
    }
}

void LogicBurnData::slotsUpdateSize(unsigned long fileSize)
{
    DataDirectory *root = static_cast<DataDirectory *>(sender());
    if (root && -1 != m_iIndex && m_oRoots.indexOf(root) == m_iIndex)
        emit sizeChanged(fileSize);
}

bool LogicBurnData::slotsBurnFileByEngine()
{
    BurnOptions opts;
    opts |= BurnOption::kJolietAndRockRidge;
    opts |= BurnOption::kKeepAppendable;

    //concat the dest filepath
    DataCDROM *cdrom = (DataCDROM*)m_oBurners.at(m_iIndex);
    if (cdrom->checkUsingOfOtherApp(true)) {
        return false;
    }

    BurnJobManager::Config config;
    config.opts = opts;
    config.speeds = cdrom->settingSpeed();
    config.volName = cdrom->settingVolumnID();

    QString destPath = cdrom->settingTempPath();
    if (destPath.endsWith("/")) {
        destPath = destPath + cdrom->settingVolumnID();
    } else{
        destPath = destPath + '/' + cdrom->settingVolumnID();
    }

    //umount device
    bool umountSucceed = DataCDROM::umountMedium(cdrom->device());
    if (!umountSucceed) {
        LOGGER->info(QString("(ProcessBurnData::working) cdrom run umount:%1").arg(umountSucceed));
    }

    // auditlog
    QGSettings auditLogGSettings("org.ukui.kylinburner");
    QString auditLogPath="/var/log/kylin-burner/os_audit.log";
    bool isOpenAuditLog = auditLogGSettings.get("burneraudit-isrunning").toBool();
    AuditLog burnDataLog;
    QString burnStatus ;
    if (cdrom->mediumStatus() == DataMedium::MEDIUM_EMPTY) {
        burnStatus = _("Burn Data");
    } else {
        burnStatus = _("Append Burn");
    }

    //copy files to dest
    DataDirectory *filesDirectory = (DataDirectory*)(m_oRoots.at(m_iIndex));
    if (filesDirectory->nSize() > (4UL * 1024UL * 1024UL * 1024UL)) {
        int ret = QMessageBox::warning(MainWindow::Instance(), "",  _("Burn data more than 4GB, the burn operation may fail. Do you want to continue with the burn operation?"), _("Cancel"), _("OK"), "", 1);
        if (ret <= 0) {
            ModalDialog::instance()->hide();
            return false;
        }
    }
    QStringList sourceFilePath;
    connect(filesDirectory, &DataDirectory::copied, this, [&](QString src, QString des){
        sourceFilePath << src;
        if (isOpenAuditLog) {
            auditLogGSettings.set("burner-os-auditlog-name",auditLogPath);
            burnDataLog.appendFile(src);
        }
    },Qt::UniqueConnection);

    progressWidgetDelayResponeTimer();
    filesDirectory->copy();
    if(!DataFile::isCopySuccess) {
        DataFile::isCopySuccess = true;
        LOGGER->info("(LogicBurnData::slotsBurnFileByEngine) 拷贝文件失败");
        ModalDialog::instance()->hide();
        Tip_BurnResult::instance()->slotsFailed(_("Burn Data"));
        return false;
    }

    /* 检测源文件是否包含敏感信息*/
    DBusCaller caller;
    int result = caller.callSensitiveInfoCheck("", sourceFilePath);
    if (!caller.handleSensitiveInfoCheckResult(result)) {
        ModalDialog::instance()->hide();
        Tip_BurnResult::instance()->slotsFailed(_("Sensitive Info Check "));
        return false;
    }

    bool createIsoFlag = cdrom->getSettings()->getCreateISO();
    if(createIsoFlag){
        QString volumeId = cdrom->getSettings()->getVolumnID();
        QString copyRight = cdrom->getSettings()->getCopyright();
        QString publisher = cdrom->getSettings()->getPublisher();
        QString prepare = cdrom->getSettings()->getPreparer();
        QString destPath = filesDirectory->destination();
        uint64_t fileSize = filesDirectory->nSize();
        int flag = createIsoByCommand(volumeId, destPath);
        LOGGER->debug(_("create iso image return %1").arg(flag));
    }

    // data verfication function : generate md5 file to destPath
    if (cdrom->settingVerficationData()) {
        DirOptUtils diropt;
        diropt.getFileListUnderDir1(destPath);
        diropt.testPrintMap();
        diropt.generateCheckFile();
    }

    BurnJobManager::instance()->disconnect();
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobProgressChanged, this, [&](int value){
        qDebug() << "ProgressValue is:" << value;
        if(value >= 100){
            m_progressDlg->slotsSetValueWithoutClosing(value, _("Fixating(Will wait minutes)"));
        }else{
            m_progressDlg->slotsSetValueWithoutClosing(value, _("medium is burning..."));
        }
    },Qt::UniqueConnection);

    //Qt::UniqueConnection doesn't work
    disconnect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, 0, 0);
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, this, [&](int jobType, bool result){
        if (isOpenAuditLog) {
            auditLogGSettings.set("burner-os-auditlog-name",auditLogPath);
            burnDataLog.outputResult(cdrom->display(), cdrom->mediumType(), burnStatus, true==result);
        }

        emit g_user_signal->noMedium(cdrom);
        cdrom->eject();

        ModalDialog::instance()->cancelEnable(false);
        ModalDialog::instance()->accept();
        if(result){
            Tip_BurnResult::instance()->slotsSuccess(_("Burn Data"));
        }else{
            Tip_BurnResult::instance()->slotsFailed(_("Burn Data"));
        }

    });

    connect(BurnJobManager::instance(), &BurnJobManager::logMsgGenerated, this, [&](int level, const QStringList msgList){
        for(QString msg : msgList){
            if(level == 0){
                LOGGER->error(msg);
            }else if(level == 1){
                LOGGER->info(msg);
            }
        }
    });

    QEventLoop loop;
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, &loop, &QEventLoop::quit);
    //start burn job
    BurnJobManager::instance()->startBurnISOFiles(cdrom->device(), QUrl::fromLocalFile(destPath), config);
    loop.exec();

    for (int i = 0; i < m_oBurners.size(); ++i){
        if(m_oBurners.at(i) == cdrom){
            cdrom->m_bStart = false;
            break;
        }
    }

    return true;
}
void LogicBurnData::slotsAsynDeleteRoot(int idx)
{
    QThread::msleep(300);

    if (idx < 0 || idx >= m_oRoots.size()) {
        LOGGER->warn(_("slotsAsynDeleteRoot: invalid idx=%1, m_oRoots.size()=%2")
                     .arg(idx).arg(m_oRoots.size()));
        return;
    }

    DataDirectory *root = static_cast<DataDirectory *>(m_oRoots.at(idx));
    beginRemoveRows(createIndex(idx, 0, root), 0, root->count());
    root->clean();
    endRemoveRows();
    disconnect(root, SIGNAL(updateSize(ulong)), this, SLOT(slotsUpdateSize(ulong)));

    beginRemoveRows(QModelIndex(), idx, idx);
    m_oBurners.takeAt(idx);
    m_oRoots.takeAt(idx);
    endRemoveRows();

    root->deleteLater();
}

int LogicBurnData::addDiskData(QStringList files, void *root)
{  
    int index = m_oRoots.indexOf(root);

    if (-1 == index)
        return 0;

    DataCDROM *cdrom = static_cast<DataCDROM *>(m_oBurners.at(index));
    if (!cdrom)
        return 0;

    //show load progress
    int ret = 0;

    ModalDialog::instance()->showPage(ModalDialog::Progress);
    emit title(_("Loading Disk Data."));
    emit setRange(0, calcFileCount(files));
    ModalDialog::instance()->cancelEnable(false);
    ModalDialog::instance()->show();

    //update file list
    QFileInfo fileInfo;
    QModelIndex parent = createIndex(index, 0, root);

    beginInsertRows(parent, 0, files.size());
    foreach (QString fileIndex, files)
    {
        QCoreApplication::processEvents();
        if (!m_bAppRunning)
            break;

        //mutex is locked when deleting data
        if((index < m_mutex_list.size()) && (!m_mutex_list.at(index)->tryLock(2000))){
           break;
        }

        fileInfo.setFile(fileIndex);

        if (fileInfo.isSymLink() && fileInfo.absoluteFilePath().contains(fileInfo.symLinkTarget()))
        {
            ret += addFile(fileInfo.absoluteFilePath(), root, true);
        }
        else if (fileInfo.isDir())
        {
            ret += addDirectoy(fileInfo.absoluteFilePath(), root, true);
        }
        else
        {
            ret += addFile(fileInfo.absoluteFilePath(), root, true);
        }

        //release lock
        if((index < m_mutex_list.size())){
           m_mutex_list.at(index)->unlock();
        }
    }
    endInsertRows();

    ModalDialog::instance()->accept();
    return ret;
}

int LogicBurnData::calcFileCount(QStringList intputList)
{
    int ret = 0;
    QFileInfo fileInfo;
    QDir dir;
    foreach (QString indexFile, intputList)
    {
        fileInfo.setFile(indexFile);
        if (fileInfo.isDir() && !fileInfo.isSymLink())
        {
            dir.setPath(indexFile);
            ret += calcFileCountInDir(dir.absolutePath());
        }
        ++ret;
    }
    return ret;
}

int LogicBurnData::calcFileCountInDir(QString inputDirPath)
{
    int ret = 0;
    QFileInfo file(inputDirPath);
    QDir dir(inputDirPath);

    foreach (file, dir.entryInfoList())
    {
        if ("." == file.fileName() || ".." == file.fileName())
            continue;

        if (file.isDir())
        {
            ret += calcFileCountInDir(file.absoluteFilePath());
        }
        ++ret;
    }
    return ret;
}

int LogicBurnData::addDirectoy(QString path, void *root, bool disk)
{
    DataDirectory *parent = static_cast<DataDirectory *>(root);
    if (nullptr == parent || !parent->isDirectory()) return 0;

    DataDirectory *item = new DataDirectory(path, parent, disk);
    if(parent){
        m_current_item = parent;
        parent->appendOrDeleteSubfile(item,true);
        m_current_item = nullptr;
    }

    if (!item->valid())
    {
        ModalDialog::instance()->accept();
        return 0;
    }
    if (!disk)
    {
        connect(item, &DataDirectory::doHidden, this, &LogicBurnData::slotsDoFilterHidden);
        connect(item, &DataDirectory::doBroken, this, &LogicBurnData::slotsDoFilterBroken);
        connect(item, &DataDirectory::doRepleace, this, &LogicBurnData::slotsDoRepleace);
    }

    int ret = 1;
    QDir dir;
    dir.setPath(path);

    QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::AllEntries | QDir::Hidden);
    QModelIndex idx = createIndex(parent->findIndex(item), 0, item);
    item->setModeIndex(idx);

    beginInsertRows(idx, 0, files.size() - 1);
    foreach (QFileInfo fileInfo, files)
    {
        QCoreApplication::processEvents();

        if ("." == fileInfo.fileName() || ".." == fileInfo.fileName())
            continue;

        if (fileInfo.isSymLink() && (fileInfo.symLinkTarget().endsWith("./") || fileInfo.symLinkTarget().endsWith("../")))
            continue;

        if (fileInfo.isDir())
        {
             ret += addDirectoy(fileInfo.absoluteFilePath(), item, disk);
        }
        else
        {
            ret += addFile(fileInfo.absoluteFilePath(), item, disk);
        }

        if (m_addAction_canceled || !m_bAppRunning)
            break;
    }
    endInsertRows();

    if (!disk)
    {
        item->filter();
    }

    emit addValue(1, item->icon(), _("Added %1 to %2").arg(item->display()).arg(parent->display()));

    return ret;
}

int LogicBurnData::addFile(QString path, void *input_parent, bool isDisk)
{
    DataDirectory *parent = static_cast<DataDirectory *>(input_parent);
    if (nullptr == parent || !parent->isDirectory())
        return 0;

    m_current_item = parent;
    DataFile *item = new DataFile(path, parent, isDisk);
    m_current_item = nullptr;
    if (!item->valid())
    {
        ModalDialog::instance()->accept();
        return 0;
    }

    // not file from disk monitor its state
    if (!isDisk)
    {
        connect(item, &DataFile::doHidden, this, &LogicBurnData::slotsDoFilterHidden);
        connect(item, &DataFile::doBroken, this, &LogicBurnData::slotsDoFilterBroken);
        connect(item, &DataFile::doRepleace, this, &LogicBurnData::slotsDoRepleace);

        item->filter();
        if(!item->valid())
        {
            connect(item,&DataFile::destroyed,this,[&](){item = nullptr;},Qt::UniqueConnection);
            item->deleteLater();
            ModalDialog::instance()->accept();
            return 0;
        }
    }

    //update progress
    emit addValue(1, item->icon(), _("Added %1 to %2").arg(item->display()).arg(parent->display()));
    return 1;
}

int LogicBurnData::appendBurnDataUDF102()
{

    DataDirectory *filesDirectory = (DataDirectory*)(m_oRoots.at(m_iIndex));

    //geneate UDF iso
    DataCDROM *cdrom = (DataCDROM*)m_oBurners.at(m_iIndex);
    QString file_save_path = filesDirectory->destination();

    QString volume_id = cdrom->settingVolumnID();
    QString device_id = cdrom->device();
    QString tmp_image_path = QString("/tmp/ky_udf_image.iso");
    int64_t prev_session_pos, next_session_pos;
    int get_offset_result = getMultiSeeionParameters(device_id.toStdString(),&prev_session_pos, &next_session_pos);
    if(get_offset_result != 0){
        LOGGER->error(QString("get device:%1offset eror return:%2").arg(device_id).arg(get_offset_result));
        return -1;
    }

    int handle_result = build_append_udf102_image(device_id.toStdString(), next_session_pos, file_save_path.toStdString(),
                                                  volume_id.toStdString(),tmp_image_path.toStdString());
    if(handle_result != 0){
        LOGGER->error(QString("append UDF 1.02 error filepath:%1, volume_id:%2 result:%3").arg(file_save_path).arg(volume_id).arg(handle_result));
        return -2;
    }
    return 0;
}

int LogicBurnData::formatUDF102()
{
    DataDirectory *filesDirectory = (DataDirectory*)(m_oRoots.at(m_iIndex));
    //geneate UDF iso
    DataCDROM *cdrom = (DataCDROM*)m_oBurners.at(m_iIndex);
    QString file_save_path = filesDirectory->destination();
    QString volume_id = cdrom->settingVolumnID();
    QString tmp_image_path = QString("/tmp/ky_udf_image.iso");

    int handle_result = init_udf102_image(file_save_path.toStdString(), volume_id.toStdString(),tmp_image_path.toStdString());
    if(handle_result != 0){
        LOGGER->error(QString("format UDF 1.02 error filepath:%1, volume_id:%2 result:%3").arg(file_save_path).arg(volume_id).arg(handle_result));
        return -1;
    }
    return 0;
}

QModelIndex LogicBurnData::modelIndexOfDirectory(DataDirectory *dir) const
{
    if (!dir)
        return QModelIndex();
    if (dir->isRoot()) {
        const int ri = m_oRoots.indexOf(dir);
        if (ri < 0)
            return QModelIndex();
        return createIndex(ri, 0, dir);
    }
    DataDirectory *parentDir = qobject_cast<DataDirectory *>(dir->parent());
    if (!parentDir)
        return QModelIndex();
    return createIndex(parentDir->findIndex(dir), 0, dir);
}

QModelIndex LogicBurnData::modelIndexForDataFile(DataFile *file) const
{
    if (!file)
        return QModelIndex();
    if (file->isDirectory())
        return modelIndexOfDirectory(static_cast<DataDirectory *>(file));

    DataDirectory *parent = qobject_cast<DataDirectory *>(file->parent());
    if (!parent)
        return QModelIndex();
    const int row = parent->findIndex(file);
    if (row < 0)
        return QModelIndex();
    const QModelIndex parentIdx = modelIndexOfDirectory(parent);
    if (!parentIdx.isValid())
        return QModelIndex();
    return index(row, 0, parentIdx);
}
