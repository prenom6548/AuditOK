#include "datafile.h"
#include "datadirectory.h"
#include "src/frame/logger.h"
#include "src/po/translation.h"

#include <QFileInfo>
#include <QDateTime>
#include <QMimeType>
#include <QMimeDatabase>
#include <QDebug>

bool DataFile::isCopySuccess = true;

DataFile::DataFile(QString path, QObject *parent, bool fromDisk)
    : QObject(parent)
{
    QFileInfo fileInfo;

    m_oLinkTarget = nullptr;
    m_bIsDisk = fromDisk;
    m_u32Level = 0;

    DataDirectory *parentDir = qobject_cast<DataDirectory *>(parent);
    fileInfo.setFile(path);

    m_bIsDirectory = fileInfo.isDir();
    m_oDisplayName = fileInfo.fileName();

    // check displayname length
    if (m_oDisplayName.length() > NAME_LENGTH)
    {
        if (fileInfo.suffix().length())
        {
            m_oDisplayName = fileInfo.fileName().mid(0, NAME_LENGTH - fileInfo.suffix().length() - 1);
            m_oDisplayName.append(".").append(fileInfo.suffix());
        }
        else
        {
            m_oDisplayName = fileInfo.fileName().mid(0, NAME_LENGTH);
        }
    }


    if (!m_bIsDirectory)
        LOGGER->registerName(this, _("FILE %1").arg(m_oDisplayName));
    else
        LOGGER->registerName(this, _("DIR %1").arg(m_oDisplayName));


    m_u64Size = fileInfo.size();
    m_oCreateTime = fileInfo.birthTime().toString("yyyy-MMMM-dd dddd HH:mm:ss");
    m_oAddTime = QDateTime::currentDateTime().toString("yyyy-MMMM-dd dddd HH:mm:ss");

    LOGGER->debug(_("Create time : %1").arg(m_oCreateTime));
    LOGGER->debug(_("Size : %1").arg(m_u64Size));
    LOGGER->debug(_("Add time : %1").arg(m_oAddTime));

    QMimeType type = QMimeDatabase().mimeTypeForFile(fileInfo.absoluteFilePath());
    QString suffixName = fileInfo.suffix();

    QList<QString> unknownSuffixList;
    unknownSuffixList << "dat" << "swf" << "rmvb";

    QList<QString> audioSuffixList;
    audioSuffixList << "ape" << "mmf";

    if(unknownSuffixList.contains(suffixName)){
        m_oIcon = QIcon::fromTheme("video-mp4", QIcon::fromTheme("unknown"));
    }else if(audioSuffixList.contains(suffixName)){
        m_oIcon = QIcon::fromTheme("audio-flac", QIcon::fromTheme("unknown"));
    }else{
        m_oIcon = QIcon::fromTheme(type.iconName(), QIcon::fromTheme("unknown"));
    }

    m_oType = type.comment();
    m_oSource = fileInfo.absoluteFilePath();

    LOGGER->debug(_("Type : %1").arg(m_oType));
    LOGGER->debug(_("Source path : %1").arg(m_oSource));

    m_oDestination = m_oSource;
    m_bAttrHidden = fileInfo.isHidden();
    m_bAttrLink = fileInfo.isSymLink();

    LOGGER->debug(_("is %1HIDDEN.").arg(m_bAttrHidden ? "" : "not "));
    LOGGER->debug(_("is %1SymbolLink.").arg(m_bAttrLink ? "" : "not "));

    DataFile *tmp_file = nullptr;
#ifndef UNIT_TEST
    if (m_bAttrLink)
    {
        if (m_bIsDirectory)
        {
            m_oLinkTarget = new DataDirectory(fileInfo.symLinkTarget(), nullptr, fromDisk);
            //找到最终的链接文件
            while (true)
            {
                if (nullptr ==  m_oLinkTarget->linkTarget())
                    break;
                tmp_file = new DataDirectory(m_oLinkTarget->linkTarget()->source(), nullptr, fromDisk);
                delete m_oLinkTarget;
                m_oLinkTarget = tmp_file;
            }
        }
        else
        {
            m_oLinkTarget = new DataFile(fileInfo.symLinkTarget(), nullptr, fromDisk);
            //找到最终链接的文件
            while (true)
            {
                if (nullptr ==  m_oLinkTarget->linkTarget())
                    break;
                tmp_file = new DataFile(m_oLinkTarget->linkTarget()->source(), nullptr, fromDisk);
                delete m_oLinkTarget;
                m_oLinkTarget = tmp_file;
            }
        }

        m_oLinkTarget->setParent(parent);

        if (parentDir)
            m_oLinkTarget->setDestination(parentDir->destination() + "/" + m_oLinkTarget->display());
        connect(m_oLinkTarget, &DataFile::subFileChanged, parentDir, &DataDirectory::appendOrDeleteSubfile);
        LOGGER->debug(_("Symbol link target : %1").arg(m_oLinkTarget->source()));
    }
#endif
    if (nullptr == parentDir)
        return;

    m_oDestination = parentDir->destination() + "/" + m_oDisplayName;

    if (m_oDisplayName == "..")
    {
        m_u64Size = 0;
        m_bIsDirectory = false;
    }

    //file property change signal
    connect(this, &DataFile::sizeChanged, parentDir, &DataDirectory::doSize);
    connect(this, &DataFile::directoryChanged, parentDir, &DataDirectory::doDirectory);
    connect(this, &DataFile::fileChanged, parentDir, &DataDirectory::doFile);
    connect(this, &DataFile::subFileChanged, parentDir, &DataDirectory::appendOrDeleteSubfile);
    connect(this, &DataFile::copied, parentDir, &DataDirectory::copied);

    //filter files
    connect(parentDir, &DataDirectory::update, this, &DataFile::onUpdate);
    connect(parentDir, &DataDirectory::folderHidden, this, &DataFile::slotsDoHidde);
    connect(parentDir, &DataDirectory::folderBroken, this, &DataFile::slotsDoBroken);
    connect(parentDir, &DataDirectory::folderReplaced, this, &DataFile::slotsDoRepleace);

#ifndef UNIT_TEST
    if (m_oLinkTarget)
    {
        connect(parentDir, &DataDirectory::update, m_oLinkTarget, &DataFile::onUpdate);
        connect(parentDir, &DataDirectory::folderHidden, m_oLinkTarget, &DataFile::slotsDoHidde,Qt::UniqueConnection);
        connect(parentDir, &DataDirectory::folderBroken, m_oLinkTarget, &DataFile::slotsDoBroken);
        connect(parentDir, &DataDirectory::folderReplaced, m_oLinkTarget,&DataFile::slotsDoRepleace);
    }

    if (parentDir->linkTarget())
    {
        connect(parentDir->linkTarget(), &DataFile::update, this, &DataFile::onUpdate);
        connect(parentDir->linkTarget(), &DataFile::hidden, this, &DataFile::slotsDoHidde);
        connect(parentDir->linkTarget(), &DataFile::broken, this, &DataFile::slotsDoBroken);
        connect(parentDir->linkTarget(), &DataFile::repleace, this, &DataFile::slotsDoRepleace);

        connect(this, &DataFile::sizeChanged, (DataDirectory*)parentDir->linkTarget(), &DataDirectory::doSize);
        connect(this, &DataFile::directoryChanged, (DataDirectory*)parentDir->linkTarget(), &DataDirectory::doDirectory);
        connect(this, &DataFile::fileChanged, (DataDirectory*)parentDir->linkTarget(), &DataDirectory::doFile);
    }
#endif
    m_u32Level = parentDir->level();
    if (m_bIsDirectory)
        ++m_u32Level;

    emit sizeChanged(m_u64Size, true);

    if (".." != m_oDisplayName)
    {
        if (m_bIsDirectory)
        {
             emit directoryChanged(true, this);
        }
        else
        {
             emit fileChanged(true, this);
        }
    }

    m_bIsValid = true;
    if (!m_bIsDirectory)
    {
        emit subFileChanged(this, true);
    }
}

QString DataFile::stringSize()
{
    int unit = -1;
    qreal rsize = m_u64Size * 1.0;

    while (rsize > 1024.00)
    {
        ++unit; rsize /= 1024.00;
    }

    QString ret = QString("%1 ").arg(rsize, 0, 'f', 1);

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

void DataFile::rename(QString name, bool add)
{
#ifndef UNIT_TEST
    DataDirectory *parent = static_cast<DataDirectory *>(this->parent());
    if (nullptr == parent)
        return;

    //更新父节点关于子节点的hash表
    if (!add)
        parent->updateChild(m_oDisplayName, name);

    m_oDestination = parent->destination() + "/" + name;
    m_oDisplayName = name;

    if (isDirectory())
    {
        parent = static_cast<DataDirectory *>(this);
        parent->rename(name);
    }
#endif
}

void DataFile::copy()
{
#ifndef UNIT_TEST
    DataDirectory *dir;

    if (".." == m_oDisplayName)
        return;

    if (m_bIsDisk)
        return;

    if (m_bIsDirectory)
    {
        dir = static_cast<DataDirectory *>(this);
        dir->copy();
        return;
    }

    if (QFile::copy(m_oSource, m_oDestination))
    {
        emit copied(m_oSource, m_oDestination);
        LOGGER->debug(_("Copied file %1 to %2").arg(m_oSource).arg(m_oDestination));
    }
    else
    {
        LOGGER->warn(QString("QFile::copy( %1 to %2 ) 失败").arg(m_oSource).arg(m_oDestination));
        DataFile::isCopySuccess = false;
    }
#endif
}

void DataFile::filter()
{
#ifndef UNIT_TEST
    DataDirectory *parent = qobject_cast<DataDirectory *>(this->parent());
    if (nullptr == parent) return;

    if (parent->filterHidden() && m_bAttrHidden)
    {
        slotsDoHidde(true);
    }

    if (parent->filterBrokenSymbolLinks() && m_bAttrLink && nullptr == m_oLinkTarget)
    {
        slotsDoBroken(true);
    }

    if (parent->repleaceSymbolLinks() && m_oLinkTarget)
    {
        slotsDoRepleace(true);
    }
#endif
}

void DataFile::changeParent(DataFile *inputParent)
{
    if (!inputParent->isDirectory())
        return;

    DataDirectory *newParent = static_cast<DataDirectory *>(inputParent);
    DataDirectory *oldParent = qobject_cast<DataDirectory *>(parent());

    emit sizeChanged(m_u64Size, false);

    if (m_bIsDirectory)
        emit directoryChanged(false, this);
    else
        emit fileChanged(false, this);

    emit subFileChanged(this, false);

    disconnect(this, &DataFile::sizeChanged, oldParent, &DataDirectory::doSize);
    disconnect(this, &DataFile::directoryChanged, oldParent, &DataDirectory::doDirectory);
    disconnect(this, &DataFile::fileChanged, oldParent, &DataDirectory::doFile);
    disconnect(this, &DataFile::subFileChanged, oldParent, &DataDirectory::appendOrDeleteSubfile);
    disconnect(this, &DataFile::copied, oldParent, &DataDirectory::copied);

    disconnect(oldParent, &DataFile::update, this, &DataFile::onUpdate);
    disconnect(oldParent, &DataDirectory::folderHidden, this, &DataFile::slotsDoHidde);
    disconnect(oldParent, &DataDirectory::folderBroken, this, &DataFile::slotsDoBroken);
    disconnect(oldParent, &DataDirectory::folderReplaced, this,  &DataFile::slotsDoRepleace);

    connect(this, &DataFile::sizeChanged, newParent, &DataDirectory::doSize);
    connect(this, &DataFile::directoryChanged, newParent, &DataDirectory::doDirectory);
    connect(this, &DataFile::fileChanged, newParent, &DataDirectory::doFile);
    connect(this, &DataFile::subFileChanged, newParent, &DataDirectory::appendOrDeleteSubfile);
    connect(this, &DataFile::copied, newParent, &DataDirectory::copied);

    connect(newParent, &DataDirectory::update, this, &DataFile::onUpdate);
    connect(newParent, &DataDirectory::folderHidden, this, &DataFile::slotsDoHidde,Qt::UniqueConnection);
    connect(newParent, &DataDirectory::folderBroken, this, &DataFile::slotsDoBroken);
    connect(newParent, &DataDirectory::folderReplaced, this, &DataFile::slotsDoRepleace);

    //when change parent level change
    ++m_u32Level;

    setParent(newParent);

    emit sizeChanged(m_u64Size, true);
    if (m_bIsDirectory){
        emit directoryChanged(true, this);
    } else {
        emit fileChanged(true, this);
    }

    emit subFileChanged(this, true);

    newParent->update(newParent->destination());
}

void DataFile::onUpdate(QString path)
{
    m_oDestination = path + "/" + m_oDisplayName;
    LOGGER->debug(_("Update destination to %1").arg(m_oDestination));
    if (m_bIsDirectory){
         emit update(m_oDestination);
    }

}

void DataFile::slotsDoHidde(bool isEnable)
{
    if ((".." == m_oDisplayName) || m_bIsDisk)
        return;

    DataDirectory *parent = qobject_cast<DataDirectory *>(this->parent());

    if (m_bAttrHidden)
    {
        if (isEnable)
        {
            parent->take(this);
            emit sizeChanged(this->m_u64Size, false);
            emit doHidden(this);
            disconnect(parent, &DataDirectory::folderHidden, this, &DataFile::slotsDoHidde);
            LOGGER->debug(_("Filter hidden file. %1").arg(m_oDisplayName));
        }
    }
    else
    {
        if (m_bIsDirectory)
        {
            parent = qobject_cast<DataDirectory *>(this);
            parent->execHidden(isEnable);
        }
    }
}

void DataFile::slotsDoBroken(bool enableBrokenLinkCheck)
{
#ifndef UNIT_TEST
    if ((".." == m_oDisplayName) || m_bIsDisk)
        return;

    DataDirectory *parent = qobject_cast<DataDirectory *>(this->parent());
    //已经替换的链接文件不再检查坏链接问题
    int thisIndex = parent->findIndex(this);
    int linkTargetIndex = parent->findIndex(m_oLinkTarget);
    if((-1 == thisIndex) && (-1 != linkTargetIndex))
    {
        return;
    }

    if (m_bAttrLink)
    {
        if (enableBrokenLinkCheck)
        {
            QFileInfo fileInfo;
            fileInfo.setFile(m_oSource);
            if (!fileInfo.symLinkTarget().isEmpty())
            {
                fileInfo.setFile(fileInfo.symLinkTarget());
                if (fileInfo.exists())
                {
                    LOGGER->debug(_("Symbo link file %1 has target file %2, do not filter.")
                          .arg(m_oDisplayName).arg(fileInfo.absoluteFilePath()));
                    return;
                }
            }

            //is badlink destory badlink
            if (m_oLinkTarget)
            {
                connect(m_oLinkTarget,&DataFile::destroyed,this,[&]{m_oLinkTarget = nullptr;},Qt::UniqueConnection);
                m_oLinkTarget->deleteLater();
            }

            parent->take(this);
            emit sizeChanged(m_u64Size, false);
            emit doBroken(this);

            disconnect(parent, &DataDirectory::folderBroken, this, &DataFile::slotsDoBroken);
            LOGGER->debug(_("Filter broken symbol link file. %1").arg(m_oDisplayName));
        }
    }
    else
    {
        if (m_bIsDirectory)
        {
            parent = qobject_cast<DataDirectory *>(this);
            parent->execBroken(enableBrokenLinkCheck);
        }
    }
#endif
}


void DataFile::slotsDoRepleace(bool inputFlag)
{
#ifndef UNIT_TEST
    if ((".." == m_oDisplayName) || m_bIsDisk)
        return;

    DataDirectory *parent = qobject_cast<DataDirectory *>(this->parent());

    QFileInfo fileInfo;
    if (m_bAttrLink && m_oLinkTarget && inputFlag)
    {
        fileInfo.setFile(m_oLinkTarget->source());
        if (!fileInfo.exists())
        {
            connect(m_oLinkTarget,&DataFile::destroyed,this,[&]{m_oLinkTarget = nullptr;},Qt::UniqueConnection);
            m_oLinkTarget->deleteLater();
            return;
        }

        parent->take(this);
        parent->append(m_oLinkTarget);
        emit doRepleace(this);

        disconnect(parent, &DataDirectory::folderReplaced, this, &DataFile::slotsDoRepleace);
        LOGGER->debug(_("Repleace symbol link file. %1").arg(m_oDisplayName));
        return;
    }

    if (m_bIsDirectory)
    {
        parent = qobject_cast<DataDirectory *>(this);
        parent->execRepleace(inputFlag);
        if (parent->linkTarget())
        {
            parent = static_cast<DataDirectory *>(parent->linkTarget());
            parent->execRepleace(inputFlag);
        }
    }
#endif
}

DataFile::~DataFile()
{
    emit sizeChanged(m_u64Size, false);
    if (m_bIsDirectory)
    {
        emit directoryChanged(false, this);
    }
    else
    {
        emit fileChanged(false, this);
    }
    emit subFileChanged(this, false);
}
