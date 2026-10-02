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

#include "filefiltermodel.h"

#include "src/data/datadirectory.h"
#include "src/data/datacdrom.h"
#include "src/po/translation.h"
#include "src/frame/configuration.h"
#include "src/view/viewfilterfiles.h"

#include <QFont>
#include <QBrush>
#include <QMutex>

static FileFilterModel *g_instance = nullptr;
static QMutex g_mutex;

FileFilterModel::FileFilterModel(QObject *parent)
    : QAbstractItemModel(parent),
      m_iIndex(-1)
{

}

FileFilterModel *FileFilterModel::instance()
{
    if (g_instance == nullptr)
    {
        QMutexLocker locker(&g_mutex);
        if (g_instance == nullptr)
        {
            g_instance = new FileFilterModel();
        }
    }
    return g_instance;
}

QVariant FileFilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QFont font;
    if (orientation == Qt::Horizontal)
    {
        switch (role)
        {
        case Qt::DisplayRole:
            if (m_oHeaders.size() && section < m_oHeaders.size())
                return m_oHeaders.at(section);
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
    return QVariant();
}

bool FileFilterModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role))
    {
        switch (role)
        {
        case Qt::DisplayRole:
            if (orientation == Qt::Horizontal)
            {
                if (section > m_oHeaders.size())
                    return false;
                else if (section == m_oHeaders.size())
                    m_oHeaders << value.toString();
                else
                    m_oHeaders.replace(section, value.toString());
            }
            break;
        }
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}

QModelIndex FileFilterModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid() && row < m_oRoots.size() && row > -1)
        return createIndex(row, 0, m_oRoots.at(row));

    DataDirectory *parentDir = (DataDirectory *)(parent.internalPointer());
    if (nullptr == parentDir)
        return QModelIndex();

    return createIndex(row, column, parentDir->at(row));
}

QModelIndex FileFilterModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    DataFile *data = static_cast<DataFile *>(index.internalPointer());
    if (nullptr == data)
        return QModelIndex();

    DataDirectory *root = static_cast<DataDirectory *>(m_oRoots.at(m_iIndex));
    if (-1 == root->findIndex(data))
    {
        DataDirectory *parent = qobject_cast<DataDirectory *>(data->parent());
        if (nullptr == parent)
            return QModelIndex();

        root = qobject_cast<DataDirectory *>(parent->parent());
        if (root && root->isDirectory())
            return createIndex(root->findIndex(parent), 0, parent);
    }
    else
    {
        return createIndex(root->findIndex(data), 0, root);
    }
    return QModelIndex();
}

int FileFilterModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_oRoots.size();

    DataDirectory * dir = static_cast<DataDirectory *>(parent.internalPointer());
    if (nullptr == dir || !dir->isDirectory())
        return 0;

    return dir->counts();
}

int FileFilterModel::columnCount(const QModelIndex &parent) const
{
    return m_oHeaders.size();
}

QVariant FileFilterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    DataFile *item = static_cast<DataFile *>(index.internalPointer());
    if (nullptr == item)
        return QVariant();
#ifndef UNIT_TEST
    switch (role)
    {
    case Qt::DisplayRole:
        switch (index.column())
        {
        case 0:
            return item->display();
        case 1:
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
        break;
    case Qt::ForegroundRole:
        if (item->isDisk())
            return QBrush(QColor(179, 179, 179));
    case Qt::SizeHintRole:
        return QSize(100,36);
        break;
    }
#endif
    return QVariant();
}

int FileFilterModel::filtered()
{
    int ret = 0;
    DataDirectory *root;
    if((m_iIndex >= 0) && (m_oRoots.size() > m_iIndex)) {
        root = (DataDirectory *)(m_oRoots.at(m_iIndex));
        if (root) ret += root->count();
    }
    return ret;
#if 0
    ret = 0;
    for (int i = 0; i < m_oRoots.size(); ++i)
    {
        root = (DataDirectory *)(m_oRoots.at(i));
        if (root) ret += root->count();
    }
    return ret;
#endif
}

void FileFilterModel::addFilterNode(void *inputDevice)
{
    DataCDROM *cdrom = (DataCDROM *)(inputDevice);
    if(cdrom->m_is_using <= 0)
        return;

    QString path;
    path.append(cdrom->settingTempPath());
    path.append("/.filter/");
    path.append(cdrom->settingVolumnID());

    DataDirectory *root = new DataDirectory(path, this);
    m_oRoots << root;

    emit this->node(cdrom, cdrom->mediumIcon(), cdrom->mDisplay());
    emit this->root(createIndex(m_oRoots.size() - 1, 0, root));

}

void FileFilterModel::deleteFilterNode(int idx)
{
    if (idx >= m_oRoots.size() || idx < 0)
        return;

    DataDirectory *root = static_cast<DataDirectory *>(m_oRoots.takeAt(idx));
    if (root)
    {
        root->clearChildren();
        delete root;
        root = nullptr;
    }
}

DataDirectory *FileFilterModel::getCurrentRoot()
{
    if (m_iIndex < 0 || m_iIndex >= m_oRoots.size())
        return nullptr;
    DataDirectory *root = static_cast<DataDirectory *>(m_oRoots.at(m_iIndex));
    return root;
}

void FileFilterModel::addFilterData(int index, void *inputData)
{
#ifndef UNIT_TEST
    DataDirectory *root = (DataDirectory *)m_oRoots.at(index);
    if (nullptr == root)
        return;

    beginInsertRows(createIndex(index, 0, root), root->counts(), root->counts());
    root->append((DataFile *)inputData);
    endInsertRows();

    root->turnViewFilterFileToHidden();
#endif
}

void FileFilterModel::restoreFiles()
{
#ifndef UNIT_TEST
    QList<void *>  hiddens;
    QList<void *>  brokens;
    QList<void *>  replaces;

    if (m_iIndex < 0)
        return;

    DataDirectory *root = static_cast<DataDirectory *>(m_oRoots.at(m_iIndex));

    DataFile *child = nullptr;

    while(root->count())
    {
        child = root->at(0);
        beginRemoveRows(createIndex(m_iIndex, 0, m_oRoots.at(m_iIndex)), 0, 0);
        root->take(child);
        endRemoveRows();

        if (child->attrHidden())
        {
            hiddens << child;
        }
        else if (child->attrSymbolLink() && child->linkTarget())
        {
            replaces << child;
        }
        else
        {
            brokens << child;
        }
    }
    root->setNodeFilterHidden(false);
    root->setNodeRepleaceSymbolLinks(false);
    root->setNodeFilterBrokenSymbolLinks(false);

    emit allFilesRestored(hiddens, brokens, replaces);

    root->turnViewFilterFileToHidden();
#endif 
}

QList<void *> FileFilterModel::GetFilteredHiddenFiles(int idx, int level)
{
    QList<void *>  result;
    DataFile      *file;

    if((idx < 0) || (idx >= m_oRoots.size())){
        return result;
    }

    DataDirectory *root = static_cast<DataDirectory *>(m_oRoots.at(idx));
#ifndef UNIT_TEST
    for (int index = 0; index < root->counts(); ++index)
    {
        file = root->at(index);
        if (file->level() >= level && file->attrHidden())
        {
            if (file->isDirectory() && file->level() == level)
                continue;

            beginRemoveRows(createIndex(idx, 0, m_oRoots.at(idx)), 0, 1);
            root->take(file);
            --index;
            endRemoveRows();
            result << file;
        }
    }
    root->turnViewFilterFileToHidden();
#endif
    return result;
}

QList<void *> FileFilterModel::GetFilteredBadLinkFiles(int idx, unsigned int level)
{
    QList<void *>  result;
    if((idx < 0) || (idx >= m_oRoots.size())){
        return result;
    }

    DataDirectory *root = static_cast<DataDirectory *>(m_oRoots.at(idx));
#ifndef UNIT_TEST
    for (int index = 0; index < root->counts(); ++index)
    {
        DataFile *file = root->at(index);

        if (file->level() >= level && file->attrSymbolLink() && nullptr == file->linkTarget())
        {
            if (file->isDirectory() && file->level() == level)
                continue;

            beginRemoveRows(createIndex(idx, 0, m_oRoots.at(idx)), 0, 1);
            root->take(file);
            --index;
            endRemoveRows();
            result << file;
        }
    }
#endif
    return result;
}

QList<void *> FileFilterModel::GetFilteredReplacedLinkfiles(int idx, int level)
{
    QList<void *>  result;

    if((idx < 0) || (idx >= m_oRoots.size())){
        return result;
    }

    DataDirectory *root = static_cast<DataDirectory *>(m_oRoots.at(idx));
    int countNum = root->count();
#ifndef UNIT_TEST
    DataFile      *indexFile;
    for(int index=0; index<countNum; ++index)
    {
        indexFile = root->at(index);
        if (indexFile && indexFile->attrSymbolLink() && indexFile->linkTarget())
        {
            QString targetFilePath = indexFile->linkTarget()->source();
            QFileInfo fileInfo(targetFilePath);
            if(fileInfo.exists()){
                result << indexFile;
            }
        }
    }

    for(int index=0; index<result.size(); ++index)
    {
        DataFile* subFile = (DataFile*)result.at(index);
        beginRemoveRows(createIndex(m_iIndex, 0, m_oRoots.at(m_iIndex)), 0, 0);
        root->take(subFile);
        endRemoveRows();
    }
#endif
    return result;
}
