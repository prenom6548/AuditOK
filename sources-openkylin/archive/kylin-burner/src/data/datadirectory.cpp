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

#include "datadirectory.h"
#include "src/po/translation.h"
#include "src/frame/configuration.h"
#include "src/view/execdialog.h"
#include "src/view/modaldialog.h"
#include "src/view/mainwindow.h"
#include "src/frame/logger.h"

#include <QObject>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QScopedPointer>

bool DataDirectory::g_bFilterHidden = false;
bool DataDirectory::g_bFilterBroken = false;//是否执行损坏连接文件过滤
bool DataDirectory::g_bRepleace = false;    //是否执行连接文件替换

void DataDirectory::setFilterHidden(bool filter)
{
    DataDirectory::g_bFilterHidden = filter;
}

/**
 * @brief 文件、目录重命名后需修改被缓存的目的路径
 * @param name 重命名文件或目录的名字
 */
void DataDirectory::setFileDestination(QString name)
{
    /* 修改文件或目录自身的被缓存的目的路径 */
    QString newPath = destination();
    int ret = newPath.lastIndexOf('/');
    if(ret == -1) 
    {
        return;
    }
#ifndef UNIT_TEST
    newPath.remove(ret+1, newPath.length()-ret+1);
    newPath.append(name);
    m_oDestination = newPath;

    /* 目录：修改其子目录或子文件的被缓存的目的路径 */
    for (int i = 0; i < m_oChildren.size(); ++i)
    {
        if (m_oChildren.at(i)->isDirectory()) {
            DataDirectory *dir = (DataDirectory*)m_oChildren.at(i);
            dir->setAllChildFileDestinationByPath(m_oDestination);
        } else {
            DataFile *file = m_oChildren.at(i);
            QString fileDestination = m_oDestination + '/' + file->display();
            file->setDestination(fileDestination);
        }
    }
#endif
}

/**
 * @brief DataDirectory::修改父目录下所有子目录或文件的被缓存的目的路径
 * @param fatherPath 父目录的被缓存的目的路径
 */
void DataDirectory::setAllChildFileDestinationByPath(QString fatherPath)
{
    /* 通过父结点的m_oDestination修改当前目录的m_oDestination */

    /* 修改当前文件、目录结点的m_oDestination */
    m_oDestination = fatherPath + '/' + display();
#ifndef UNIT_TEST
    /* 修改子目录或子文件的m_oDestination */
    for (int i = 0; i < m_oChildren.size(); ++i)
    {
        if (m_oChildren.at(i)->isDirectory())
        {
            DataDirectory *dir = (DataDirectory*)m_oChildren.at(i);
            dir->setAllChildFileDestinationByPath(m_oDestination);
        }
        else
        {
            DataFile *file = m_oChildren.at(i);
            QString fileDestination = m_oDestination + '/' + file->display();
            file->setDestination(fileDestination);
        }
    }
#endif
}

void DataDirectory::setFilterBrokenSymbolLinks(bool filter)
{
    DataDirectory::g_bFilterBroken = filter;
}

void DataDirectory::setRepleaceSymbolLinks(bool filter)
{
    DataDirectory::g_bRepleace = filter;
}

DataDirectory::DataDirectory(QString path, QObject *parent, bool fromDisk) :
    DataFile(path, parent, fromDisk)
{
    DataDirectory *dir;
    m_oChildren.clear();
    m_bIsDirectory = true;
    m_oHashChildren.clear();
    m_u64Files = 0;
    m_u64Dirs = 0;
    m_u64NewDataSize = 0;
    m_inputchange_dlg = new InputChange;

    dir = qobject_cast<DataDirectory *>(parent);
    if (dir) m_bIsRoot = false;
    else m_bIsRoot = true;


    if (m_bIsRoot){
        m_u64Size = 0;
    }
    if (".." == m_oDisplayName)
    {
        m_oIcon = QIcon::fromTheme("go-up-symbolic");
        m_oSource = _("Return to parent level.");
    }

    if (!m_bIsRoot && m_oSource == m_oDestination)
    {
        m_oIcon = dir->icon();
        m_oType = _("NewFolder");
        m_u32Level = dir->level() + 1;
        m_oSource.clear();
    }

    // 加载节点特定的过滤配置
    if (m_bIsRoot) {
        loadNodeFilterConfig();
    }
}

DataFile *DataDirectory::at(int row)
{
    if (row < 0) return nullptr;
    if (row < m_oChildren.size())
        return m_oChildren.at(row);
    return nullptr;
}

int DataDirectory::findIndex(DataFile *item)
{
    int index = m_oChildren.indexOf(item);
    if (index < 0) {
        return -1;
    }
    return index;
}

int DataDirectory::findIndex(DataDirectory *dir)
{
    DataFile *item = static_cast<DataFile *>(dir);
    return findIndex(item);
}


//移除文件
bool DataDirectory::take(DataFile *file)
{
    int index = m_oChildren.indexOf(file);
    if (-1 == index)
    {
        return false;
    }
    m_oChildren.takeAt(index);
    m_oHashChildren.remove(file->display());

    return true;
}

void DataDirectory::append(DataFile *inputFile)
{
#ifndef UNIT_TEST
    bool renamed;

    //依据策略对重复的文件进行重命名
    DataFile *sourceFile = m_oHashChildren.value(inputFile->display());
    if (sourceFile)
    {
        LOGGER->debug(_("Duplicate child %1").arg(inputFile->display()));
        int suffix = 0;
        QFileInfo fileInfo;
        fileInfo.setFile(sourceFile->destination());
        QString newName;
        int configValue = Configuration::instance()->get("DataSetting", "Duplicate Data", 0).toInt();
        switch (configValue)
        {

        //rename
        case 0:
            do
            {

                renamed = m_inputchange_dlg->input(newName, _("Rename"), _("File name duplicate, need to rename:"), inputFile->display());
                if (!renamed) {
                    break;
                }

                if (newName.trimmed().isEmpty())
                {
                    ExecDialog::instance()->error(_("Add File(s)") + " - " + _("Empty Name"),
                                _("Cannot named with empty."));
                    continue;
                }

                //hidden filename is begin with.
                if(inputFile->attrHidden() && !newName.startsWith("."))
                {
                    ExecDialog::instance()->error(_("Add File(s)") + " - " + _("Not Hidden Name"),
                                _("Cannot named with None hidden name"));
                    continue;
                }

                if (m_oHashChildren.value(newName))
                {
                    ExecDialog::instance()->error(_("Add File(s)") + " - " + _("Duplicate Name"),
                                _("There is data named %1").arg(newName));
                    continue;
                }
                if (newName.toUtf8().size() > 103)
                {
                    ExecDialog::instance()->error(_("Add File(s)") + " - " + _("Name Too Long"),
                                _("Cannot add named length more than 103 unicode charsets."));
                    continue;
                }
                if (renamed)
                    break;
            } while (true);

            if (renamed)
            {
                inputFile->rename(newName,true);
                break;
            }
            else
            {
                inputFile->setValid(false);
                return;
            }
        //append suffix
        case 1:
            do
            {
                if (fileInfo.suffix().isEmpty())
                {
                    newName = QString("%1_%2").arg(fileInfo.baseName()).arg(suffix++);
                    if (newName.length() > NAME_LENGTH)
                    {
                        newName = fileInfo.fileName().mid(0, NAME_LENGTH - QString("_%1").arg(suffix).length());
                        newName.append(QString("_%2").arg(suffix));
                    }
                }
                else
                {
                    newName = QString("%1_%2.%3").arg(fileInfo.baseName()).arg(suffix++).arg(fileInfo.suffix());
                    if (newName.length() > NAME_LENGTH)
                    {
                        newName = fileInfo.fileName().mid(0, NAME_LENGTH - QString("_%1.%2").arg(suffix).arg(fileInfo.suffix()).length());
                        newName.append(QString("_%1.%2").arg(suffix).arg(fileInfo.suffix()).length());
                    }
                }
            } while (m_oHashChildren.value(newName));
            inputFile->rename(newName);
            break;
        default:
            if (!sourceFile->isDisk())
            {
                connect(sourceFile, &DataFile::destroyed,this,[&]{sourceFile = nullptr;},Qt::UniqueConnection);
                sourceFile->deleteLater();
            }
            break;
        }
    }
    if(inputFile)
    {
        m_oHashChildren.insert(inputFile->display(), inputFile);
        m_oChildren << inputFile;
        LOGGER->debug(_("Add child %1").arg(inputFile->display()));
    }
    return;
#endif
}

int DataDirectory::count() const
{
    if (m_bIsRoot)
        return m_oChildren.count();

    if (m_oChildren.count() == 1 && ".." == m_oChildren.first()->display())
        return 0;

    return m_oChildren.count() - 1;
}

bool DataDirectory::canClean() const
{
    int i;
    if (0 == m_oChildren.size())
        return false;

    for (i = m_bIsRoot ? 0 : 1; i < m_oChildren.size(); ++i)
    {
        if (!m_oChildren.at(i)->isDisk())
            break;
    }

    if (i < m_oChildren.size())
        return true;

    return false;
}

bool DataDirectory::duplicate(QString name) const
{
    if (m_oHashChildren.value(name))
        return true;

    return false;
}

void DataDirectory::rename(QString name, bool add)
{
    DataDirectory *parent;
    QDir dir;

    if (m_bIsRoot)
    {
        dir.setPath(destination());
        dir.removeRecursively();
        m_oSource = name;
        m_oDestination = name;
        update(m_oDestination);
    }
    else
    {
        parent = static_cast<DataDirectory *>(this->parent());
        if (nullptr == parent) return;
        if (!add)
            parent->updateChild(m_oDisplayName, name);
        m_oDisplayName = name;
        m_oDestination = parent->destination() + "/" + m_oDisplayName;
    }
}

void DataDirectory::updateChild(QString oldName, QString newName)
{
    if ((oldName == newName) || (!m_oHashChildren.value(oldName)))
        return;
    m_oHashChildren.insert(newName, m_oHashChildren.value(oldName));
    m_oHashChildren.remove(oldName);
}

void DataDirectory::clear()
{
    for (int i = m_oChildren.size() - 1; i > -1 && m_oChildren.size(); --i)
    {
        if (m_oChildren.at(i)->isDisk() ||
                ".." == (m_oChildren.at(i))->display())
            continue;
        // 注意：主界面列表会删除一次目录结构树，其它页面不用再删除，只需要调用clearChildren函数即可。否则删除DataFile指针会造成闪退
        delete m_oChildren.at(i);
    }
}

void DataDirectory::clean()
{
    for (int i = m_oChildren.size() - 1; i > -1 && m_oChildren.size(); --i)
    {
        // 注意：主界面列表会删除一次目录结构树，其它页面不用再删除，只需要调用clearChildren函数即可。否则删除DataFile指针会造成闪退
        delete m_oChildren.at(i);
    }
}

void DataDirectory::clearChildren()
{
    m_oChildren.clear();
    m_oHashChildren.clear();
}

void DataDirectory::copy()
{

    if ((".." == m_oDisplayName) || m_bIsDisk){
        return;
    }

    QDir dir;
    dir.setPath(m_oDestination);
    if (dir.exists())
    {
        bool success = dir.removeRecursively();
        if (!success)
        {
            LOGGER->warn(QString("(DataDirectory::copy) 删除目录m_oDestination：%1 失败").arg(m_oDestination));
        }
    }

    bool success = dir.mkpath(m_oDestination);
    if (!success)
    {
        LOGGER->warn(QString("(DataDirectory::copy) 创建目录m_oDestination：%1 失败").arg(m_oDestination));
    }

    emit copied(m_oSource, m_oDestination);

    for (int i = 0; i < m_oChildren.size(); ++i)
    {
        QCoreApplication::processEvents();
        m_oChildren.at(i)->copy();
    }
}

void DataDirectory::execHidden(bool isEnable)
{
    emit folderHidden(isEnable);
}

void DataDirectory::execBroken(bool flag)
{
    emit folderBroken(flag);
}

void DataDirectory::execRepleace(bool b)
{
    emit folderReplaced(b);
}

void DataDirectory::turnViewFilterFileToHidden()
{
    execHidden(false);
}

void *DataDirectory::findChildByDisplay(QString name, bool child)
{
    void *ret;
    DataDirectory *dir;
    DataFile *file;

    ret = m_oHashChildren.value(name);
    if (ret) return ret;
    if (child) return ret; // if child is true, means only find in self chidren
    foreach( file, m_oChildren )
    {
        if (file->isDirectory())
        {
            dir = qobject_cast<DataDirectory *>(file);
            ret = dir->findChildByDisplay(name);
            if (ret) return ret;
        }
    }
    return nullptr;
}

/***************************************************
 * 计算最外层（根目录）下有多少非光盘文件，即新添加文件
 * ************************************************/
int DataDirectory::CountOneLevelNotDiskFile()
{
    DataFile *file;
    int index = 0;
    foreach( file, m_oChildren )
    {
        if (!(file->isDisk()))
        {
              index++;
        }
    }
    return index;
}

bool DataDirectory::isGreateThan2GByNewAddSingleFile()
{
#ifndef UNIT_TEST
    DataFile *file;
    foreach( file, m_oChildren )
    {
        if (!file->isDisk()) {
            if (!file->isDirectory())
            { //文件
                unsigned long capacity2G = 2147483648; //2*1024*1024*1024
                if (file->size() >= capacity2G) {

                    return true;
                } else {

                }
            }
            // 目录
            else
            {
                DataDirectory *dir = static_cast<DataDirectory*>(file);
                if (dir->isGreateThan2GByNewAddSingleFile()) {
                    return true;
                }
            }
        }
    }
#endif
    return false;
}

void DataDirectory::doSize(unsigned long size, bool isAppend)
{

    DataFile *senderFile = static_cast<DataFile *>(sender());

    if (isAppend)
    {
        m_u64Size += size;
        if (senderFile && !senderFile->isDisk())
        {
            m_u64NewDataSize += size; 
        }
        LOGGER->debug(_("Add size %1, now size is %2").arg(size).arg(m_u64Size));
    }
    else
    {
        m_u64Size -= size;
        if (senderFile && !senderFile->isDisk())
        {
            m_u64NewDataSize -= size;
        }
        LOGGER->debug(_("Sub size %1, now size is %2").arg(size).arg(m_u64Size));
    }
    emit this->sizeChanged(size, isAppend);
    emit updateSize(m_u64Size);
}

void DataDirectory::doDirectory(bool d, DataFile *item)
{
    if (d)
        ++m_u64Dirs;
    else
        --m_u64Dirs;
    emit directoryChanged(d, item);
    LOGGER->debug(_("Includes %1 dir(s), after %2").arg(m_u64Dirs).arg(d ? _("add") : _("delete")));
}

void DataDirectory::doFile(bool flag, DataFile *item)
{
    if (flag)
        ++m_u64Files;
    else
        --m_u64Files;
    emit fileChanged(flag, item);
    LOGGER->debug(_("Includes %1 file(s), after %2").arg(m_u64Files).arg(flag ? _("add") : _("delete")));
}

void DataDirectory::appendOrDeleteSubfile(DataFile *child, bool isAppend)
{
#ifndef UNIT_TEST
    int suffix;
    bool renamed;
    DataDirectory *linked;
    QString newName;
    QFileInfo fileInfo;

    int idx = m_oChildren.indexOf(child);
    if (isAppend && -1 == idx)
    {
        DataFile *oldFile = m_oHashChildren.value(child->display());
        if (oldFile)
        {
            // todo if has same name.
            LOGGER->debug(_("Duplicate child %1").arg(child->display()));
            switch (Configuration::instance()->get("DataSetting", "Duplicate Data", 0).toInt())
            {
            case 0:
                do
                {
                    renamed = m_inputchange_dlg->input(newName, _("Rename"), _("File name duplicate, need to rename:"),child->display());
                    if (!renamed)
                        break;

                    if (newName.trimmed().isEmpty())
                    {
                        ExecDialog::instance()->error(_("Add File(s)") + " - " + _("Empty Name"),
                                    _("Cannot named with empty."));
                        continue;
                    }
                    if (m_oHashChildren.value(newName))
                    {
                        ExecDialog::instance()->error(_("Add File(s)") + " - " + _("Duplicate Name"),
                                    _("There is data named %1").arg(newName));
                        continue;
                    }
                    if (newName.toUtf8().size() > 103)
                    {
                        ExecDialog::instance()->error(_("Add File(s)") + " - " + _("Name Too Long"),
                                    _("Cannot add named length more than 103 unicode charsets."));
                        continue;
                    }
                    if (renamed)
                        break;
                } while (true);

                if (renamed)
                {
                    child->rename(newName, true);
                }
                else
                {
                    child->setValid(false);
                    return;
                }
                break;
                // add suffix
            case 1:
                suffix = 0;
                fileInfo.setFile(child->destination());
                do
                {
                    if (fileInfo.suffix().isEmpty())
                    {
                        newName = QString("%1_%2").arg(fileInfo.baseName()).arg(suffix++);
                        if (newName.length() > NAME_LENGTH)
                        {
                            newName = fileInfo.fileName().mid(0, NAME_LENGTH - QString("_%1").arg(suffix).length());
                            newName.append(QString("_%2").arg(suffix));
                        }
                    }
                    else
                    {
                        newName = QString("%1_%2.%3").arg(fileInfo.baseName()).arg(suffix++).arg(fileInfo.suffix());
                        if (newName.length() > NAME_LENGTH)
                        {
                            newName = fileInfo.fileName().mid(0, NAME_LENGTH - QString("_%1.%2").arg(suffix).arg(fileInfo.suffix()).length());
                            newName.append(QString("_%1.%2").arg(suffix).arg(fileInfo.suffix()).length());
                        }
                    }
                } while (m_oHashChildren.value(newName));
                child->rename(newName, true);
                break;
            default:
                if (!oldFile->isDisk())
                {
                    delete oldFile;
                    oldFile = nullptr;
                }
                else
                {
                    child->setValid(false);
                    return;
                }
                break;
            }
        }
        m_oHashChildren.insert(child->display(), child);
        m_oChildren << child;
        if (m_oLinkTarget)
        {
            linked = static_cast<DataDirectory *>(m_oLinkTarget);
            linked->append(child);
        }
        LOGGER->debug(_("Add child %1").arg(child->display()));
        return;
    }

    if (!isAppend && -1 != idx)
    {

        m_oHashChildren.remove(child->display());
        m_oChildren.takeAt(idx);
        if (m_oLinkTarget)
        {
            linked = static_cast<DataDirectory *>(m_oLinkTarget);
            linked->take(child);

        }
        LOGGER->debug(_("Move child %1 out").arg(child->display()));
        return;
    }
#endif
}

void DataDirectory::closeInputChangeDlg()
{
    m_inputchange_dlg->slotClose();
}

bool DataDirectory::isRenameDlgShown()
{
    return m_inputchange_dlg->isVisible();
}

QString DataDirectory::getNodeConfigKey() const
{
    // 使用路径作为节点的唯一标识
    QString key = m_oSource;
    if (key.isEmpty()) {
        key = m_oDestination;
    }
    // 移除特殊字符，确保配置键的有效性
    key.replace("/", "_");
    key.replace("\\", "_");
    key.replace(":", "_");
    return QString("NodeFilter_%1").arg(key);
}


void DataDirectory::loadNodeFilterConfig()
{
    if (!m_bIsRoot) return;
    // 加载节点特定的过滤规则
    m_bNodeFilterHidden = false;
    m_bNodeFilterBroken = false;
    m_bNodeRepleace = false;
}

DataDirectory::~DataDirectory()
{
    if(m_inputchange_dlg){
        delete m_inputchange_dlg;
    }
}
