/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "parsebackuplist.h"
#include <QFile>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QDebug>
#include <algorithm>

/*
<?xml version='1.0'?>
<backupList>
     <BackupPoint>
          <Comment>21-07-21 14:14:01</Comment>
          <Time>21-07-21 14:14:03</Time>
          <Uuid>{beecb746-561f-4fa1-99ba-19fb849a1ba7}</Uuid>
          <Size>24.26KB</Size>
          <State>backup finished</State>
          <Type>2</Type>
          <Position>0</Position>
          <UserId>1000</UserId>
          <PrefixDestPath>/</PrefixDestPath>
     </BackupPoint>
</backupList>
*/

#define BACKUPLIST "backupList"
#define BACKUPPOINT "BackupPoint"
#define COMMENT "Comment"
#define TIME "Time"
#define UUID "Uuid"
#define SIZE "Size"
#define STATE "State"
#define TYPE "Type"
#define POSITION "Position"
#define USERID "UserId"
#define OS "OS"
#define ARCH "Arch"
#define ARCHDETECT "ArchDetect"
#define PREFIXDESTPATH "PrefixDestPath"
#define NOTE "note"
#define SYSTEMHASH "SystemHash"
#define OVLCOMMITHASH "OvlCommitHash"
#define SYSTEMCOMMITHASHMAP "SystemCommitHashMap"
#define DATACOMMITHASHMAP "DataCommitHashMap"
#define PATH "Path"
#define USERNAME "User"
#define BRANCHNAME "BranchName"
#define KAREPATH "KarePath"

#define STATUE_BACKUP_FINESHED "backup finished"

ParseBackupList::ParseBackupList(const QString& xmlPath)
    : m_xmlPath(xmlPath)
{
    QFile xmlFile(m_xmlPath);
    if (!xmlFile.exists() || 0 == xmlFile.size()) {
        InitXml();
    }
}

/**
 * @brief 初始化xml文件
 * @return
 */
bool ParseBackupList::InitXml()
{
    QFile xmlFile(m_xmlPath);
    qDebug() << "初始化xml文件" <<  m_xmlPath;
    if (!xmlFile.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        qDebug() << "无法打开文件：" << xmlFile.errorString();
        return false;
    }


    QDomDocument doc;

    QDomProcessingInstruction ins = doc.createProcessingInstruction("xml", "version=\'1.0\'");
    doc.appendChild(ins);

    QDomElement root = doc.createElement(BACKUPLIST);
    doc.appendChild(root);

    QTextStream out(&xmlFile);
    doc.save(out, QDomNode::NodeType::CDATASectionNode);

    xmlFile.close();
    return true;
}

/**
 * @brief 校验xml格式是否正确
 * @return
 */
bool ParseBackupList::isXmlCorrect()
{
    QFile xmlFile(m_xmlPath);
    if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Open file " << m_xmlPath << " failure";
        return false;
    }
    QXmlStreamReader reader(&xmlFile);
    while (!reader.atEnd()) {
        if (reader.isStartElement()) {
        }
        reader.readNext();
    }
    if (reader.hasError()) {
        qDebug() << "xml parse error!";
        xmlFile.close();
        return false;
    }
    xmlFile.close();
    return true;
}

/**
 * @brief 读取xml文件
 * @param doc
 * @return
 */
bool ParseBackupList::Doc_setContent(QDomDocument& doc)
{
    QFile xmlFile(m_xmlPath);
    if (!xmlFile.open(QIODevice::ReadOnly)) {
        qDebug() << "open backuplist.xml failed!" << m_xmlPath;
        return false;
    }
    QString errStr;
    int errLine;
    int errCol;

    if (!doc.setContent(&xmlFile, false, &errStr, &errLine, &errCol)) {
        qDebug() << QString("parse backuplist.xml error at line %1, column %2:%3").arg(errLine).arg(errCol).arg(errStr);
        xmlFile.close();
        return false;
    }
    xmlFile.close();
    return true;
}

/**
 * @brief ParseBackupList::BackupPoint::operator <
 * @param other
 * @return
 */
bool ParseBackupList::BackupPoint::operator < (const BackupPoint &other)
{
    return other.m_time > this->m_time;
}

/**
 * @brief 出厂备份修改backuplist.xml，仅保留出厂备份信息
 * @param factoryBackupUuid，出厂备份的uuid
 * @return ParseResult枚举类型
 * @author zhaominyong
 * @since 2021/06/27
 */
ParseBackupList::ParseResult ParseBackupList::updateForFactoryRestore(const QString& factoryBackupUuid)
{
    QDomDocument doc;
    if (!Doc_setContent(doc))
        return XML_PARSE_ERR;

    QDomElement root = doc.documentElement();
    QDomNodeList list = root.childNodes();

    for (int i = 0; i < list.count(); i++) {
        QDomNode node = list.at(i);
        if (!node.isElement())
            continue ;
        QDomElement element = node.toElement();
        QDomNodeList nodes = element.elementsByTagName(UUID);
        if (0 < nodes.count()) {
            QDomElement uuidElement = nodes.at(0).toElement();
            QString tag = uuidElement.tagName();
            QString text = uuidElement.text();
            if (uuidElement.text() != factoryBackupUuid) {
                root.removeChild(node);
                --i;
            }
        }
    }

    QFile xmlFile(m_xmlPath);
    if (!xmlFile.open(QIODevice::WriteOnly)) {
        qDebug() << "update state failed";
        return FAIL;
    }
    QTextStream out(&xmlFile);
    doc.save(out, QDomNode::NodeType::EntityReferenceNode);
    out.flush();
    xmlFile.close();
    return SUCCESS;
}

/**
 * @brief 根据comment查找uuid
 * @param comment
 * @return uuid
 */
QString ParseBackupList::findUuidByComment(const QString& comment)
{
    QDomDocument doc;
    if (!Doc_setContent(doc))
        return "";

    QDomElement root = doc.documentElement();
    QDomNodeList list = root.childNodes();

    for (int i = 0; i < list.count(); i++) {
        QDomNode node = list.at(i);
        if (!node.isElement())
            continue;

        QDomElement eleComment = node.firstChildElement(COMMENT);
        if (eleComment.isNull())
            continue;

        if (comment != eleComment.text())
            continue;

        QDomElement eleUuid = node.firstChildElement(UUID);
        if (eleUuid.isNull())
            return "";

        return eleUuid.text();
    }

    return "";
}

/**
 * @brief 根据Uuid查找备份点信息
 * @param Uuid
 * @return 备份点信息
 */
ParseBackupList::BackupPoint ParseBackupList::findBackupPointByUuid(const QString& Uuid)
{
    BackupPoint backupPoint;
    QDomDocument doc;
    if (!Doc_setContent(doc))
        return backupPoint;

    QDomElement root = doc.documentElement();
    QDomNodeList list = root.childNodes();

    for (int i = 0; i < list.count(); i++) {
        QDomNode node = list.at(i);
        if (!node.isElement())
            continue;

        QDomElement eleUuid = node.firstChildElement(UUID);
        qDebug() << eleUuid.text() ;
        qDebug() << Uuid ;
        if (eleUuid.isNull() || Uuid != eleUuid.text())
            continue;

        elementNodeToBackupPoint(node.toElement(), backupPoint);
        return backupPoint;
    }

    return backupPoint;
}

/**
 * @brief 获取xml文件中备份点uuid和backupname的映射
 * @param uuid_name，xml文件中备份点uuid和backupname的映射
 * @return
 */
void ParseBackupList::getXmlUuidNameMap(QMap<QString, QString> &uuid_name)
{
    BackupPoint backupPoint;
    QDomDocument doc;
    if (!Doc_setContent(doc))
        return ;

    QDomElement root = doc.documentElement();
    QDomNodeList list = root.childNodes();

    for (int i = 0; i < list.count(); i++) {
        QDomNode node = list.at(i);
        if (!node.isElement())
            continue;

        QDomElement eleUuid = node.firstChildElement(UUID);
        if (eleUuid.isNull() || eleUuid.text().isEmpty())
            continue;

        QDomElement eleBackupName = node.firstChildElement(COMMENT);
        if (eleBackupName.isNull() || eleBackupName.text().isEmpty())
            continue;

        uuid_name.insert(eleUuid.text(), eleBackupName.text());
    }

    return ;
}

/**
 * @brief 获取xml文件中备份点对应目录的commit哈希
 * @param uuid_name，xml文件中备份点uuid和backupname的映射
 * @return
 */
void ParseBackupList::getXmlCommitHashMap(QString &systemHash,QMap<QString, QString> &systemCommitHashMap,QString uuid)
{
    QDomDocument doc;
    if (!Doc_setContent(doc))
        return ;

    QDomElement root = doc.documentElement();
    QDomNodeList list = root.childNodes();

    for (int i = 0; i < list.count(); i++)
    {
        QDomNode node = list.at(i);
        if (!node.isElement())
            continue;

        QDomElement eleUuid = node.firstChildElement(UUID);
        if (eleUuid.text() == uuid)
        {
            QDomElement commitHash = node.firstChildElement("SystemCommitHashMap");
            if (commitHash.isNull() || commitHash.text().isEmpty())
                continue;

            QDomNodeList commits = commitHash.childNodes();
            for (int j = 0; j < commits.count(); j++)
            {
                QDomNode commitNode = commits.at(j);
                if (commitNode.isElement()) {
                    QString key = commitNode.nodeName();
                    QString value = commitNode.firstChild().nodeValue();
                    systemCommitHashMap.insert(key, value);
                }
            }

            QDomElement nSystemHash = node.firstChildElement("SystemHash");
            if (nSystemHash.isNull() || nSystemHash.text().isEmpty())
                continue;

            if (nSystemHash.isElement()) {
                QString key = nSystemHash.nodeName();
                QString value = nSystemHash.firstChild().nodeValue();
                systemHash = value;
            }
        }
    }

    return ;
}

/**
 * @brief 获取自定义备份路径列表
 * @param customizePaths
 */
void ParseBackupList::getCustomizePaths(QStringList &customizePaths)
{
    QDomDocument doc;
    if (!Doc_setContent(doc))
        return ;

    QDomElement root = doc.documentElement();
    QDomNodeList list = root.childNodes();

    for (int i = 0; i < list.count(); i++) {
        QDomNode node = list.at(i);
        if (!node.isElement())
            continue;

        QDomElement elePrefixPath = node.firstChildElement(PREFIXDESTPATH);
        if (!elePrefixPath.isNull()) {
            QString prefixPath = elePrefixPath.text() + BACKUP_SNAPSHOTS_PATH;
            prefixPath.replace("//", "/");
            if (!customizePaths.contains(prefixPath))
                customizePaths << prefixPath;
        }
    }
}

/**
 * @brief 获取最后一次系统备份，排除自动备份点、自定义备份点
 * @return
 * @note 经过测试发现出厂备份点也不能作为基准点进行增量，因为出厂备份的安全标记和其它不同，咨询安全同事说解决不了。所以也排除出厂备份。
 */
ParseBackupList::BackupPoint ParseBackupList::getLastSysBackupPoint()
{
    BackupPoint backupPoint;
    QDomDocument doc;
    if (!Doc_setContent(doc))
        return backupPoint;

    QDomElement root = doc.documentElement();
    QDomNodeList list = root.childNodes();

    for (int i = 0; i < list.count(); i++) {
        QDomNode node = list.at(i);
        if (!node.isElement())
            continue;

        QDomElement eleType = node.firstChildElement(TYPE);
        if (eleType.isNull() || (BackupType::BACKUP_SYSTEM != eleType.text().toInt() && BackupType::INC_BACKUP_SYSTEM != eleType.text().toInt()))
            continue;

        QDomElement eleUuid = node.firstChildElement(UUID);
        if (eleUuid.isNull() || eleUuid.text() == AUTO_BACKUP_UUID || eleUuid.text() == FACTORY_BACKUP_UUID)
            continue;

        QDomElement eleState = node.firstChildElement(STATE);
        if (eleState.isNull() || eleState.text() != QString(STATUE_BACKUP_FINESHED))
            continue;

        QDomElement elePosition = node.firstChildElement(POSITION);
        if (!elePosition.isNull() && elePosition.text().toInt() == BackupPosition::CUSTOMIZE)
            continue;

        elementNodeToBackupPoint(node.toElement(), backupPoint);
    }

    return backupPoint;
}

/**
 * @brief elementNode --> BackupPoint
 * @param node, QDomElement
 * @param backupPoint, BackupPoint
 */
void ParseBackupList::elementNodeToBackupPoint(const QDomElement& node, BackupPoint& backupPoint)
{
    QDomElement eleUuid = node.firstChildElement(UUID);
    if (!eleUuid.isNull())
        backupPoint.m_uuid = eleUuid.text();

    QDomElement eleComment = node.firstChildElement(COMMENT);
    if (!eleComment.isNull())
        backupPoint.m_backupName = eleComment.text();
    if (backupPoint.m_uuid == FACTORY_BACKUP_UUID)
        backupPoint.m_backupName = QObject::tr("factory backup");

    QDomElement eleTime = node.firstChildElement(TIME);
    if (!eleTime.isNull())
        backupPoint.m_time = eleTime.text();

    QDomElement eleSize = node.firstChildElement(SIZE);
    if (!eleSize.isNull())
        backupPoint.m_size = eleSize.text();

    QDomElement eleState = node.firstChildElement(STATE);
    if (!eleState.isNull())
        backupPoint.m_state = eleState.text();

    QDomElement eleType = node.firstChildElement(TYPE);
    if (!eleType.isNull())
        backupPoint.m_type = eleType.text().toInt();

    QDomElement elePosition = node.firstChildElement(POSITION);
    if (!elePosition.isNull() && !elePosition.text().isEmpty())
        backupPoint.m_iPosition = elePosition.text().toInt();

    QDomElement eleUserId = node.firstChildElement(USERID);
    if (!eleUserId.isNull())
        backupPoint.m_userId = eleUserId.text();

    QDomElement eleUserName = node.firstChildElement(USERNAME);
        if (!eleUserName.isNull())
            backupPoint.m_user = eleUserName.text();

    QDomElement eleOS = node.firstChildElement(OS);
    if (!eleOS.isNull())
        backupPoint.m_os = eleOS.text();

    QDomElement eleArch = node.firstChildElement(ARCH);
    if (!eleArch.isNull())
        backupPoint.m_arch = eleArch.text();

    QDomElement eleArchDetect = node.firstChildElement(ARCHDETECT);
    if (!eleArchDetect.isNull())
        backupPoint.m_archdetect = eleArchDetect.text();

    QDomElement elePrefixPath = node.firstChildElement(PREFIXDESTPATH);
    if (!elePrefixPath.isNull()) {
        backupPoint.m_path = elePrefixPath.text();
    } else {
        backupPoint.m_path = m_xmlPath;
        backupPoint.m_path.replace(SYS_UPDATE_BACKUP_XML_PATH, "");
        backupPoint.m_path.replace(BACKUP_XML_PATH, "");
    }
    QDomElement eleNote = node.firstChildElement(NOTE);
    if (!eleNote.isNull())
        backupPoint.m_note = eleNote.text();

    QDomElement branchName = node.firstChildElement(BRANCHNAME);
    if (!branchName.isNull())
        backupPoint.m_branchName = branchName.text();

    QDomElement commitHash = node.firstChildElement(SYSTEMCOMMITHASHMAP);
    if (!(commitHash.isNull() || commitHash.text().isEmpty()))
    {
        QDomNodeList commits = commitHash.childNodes();
        for (int j = 0; j < commits.count(); j++)
        {
            QDomNode commitNode = commits.at(j);
            if (commitNode.isElement()) {
                QString key = commitNode.nodeName();
                QString value = commitNode.firstChild().nodeValue();
                backupPoint.m_systemCommitHashMap.insert(key, value);
            }
        }
    }

    QDomElement nSystemHash = node.firstChildElement(SYSTEMHASH);
    if (!(nSystemHash.isNull() || nSystemHash.text().isEmpty()))
    {

        if (nSystemHash.isElement()) {
            QString key = nSystemHash.nodeName();
            QString value = nSystemHash.firstChild().nodeValue();
            backupPoint.m_systemHash = value;
        }
    }

    QDomElement karePath = node.firstChildElement(KAREPATH);
    if (!karePath.isNull())
    {
        backupPoint.m_karePaths = karePath.text();
    }
}

/**
 * @brief backupPoint --> ElementNode
 * @param backupPoint, BackupPoint
 * @param node, QDomElement
 */
void ParseBackupList::backupPointToElementNode(const BackupPoint& backupPoint, QDomDocument& doc, QDomNode& node)
{
    node.appendChild(createTextElement(doc, COMMENT, backupPoint.m_backupName));
    node.appendChild(createTextElement(doc, TIME, backupPoint.m_time));
    node.appendChild(createTextElement(doc, UUID, backupPoint.m_uuid));
    node.appendChild(createTextElement(doc, SIZE, backupPoint.m_size));
    node.appendChild(createTextElement(doc, STATE, backupPoint.m_state));
    node.appendChild(createTextElement(doc, TYPE, QString::number(backupPoint.m_type)));
    node.appendChild(createTextElement(doc, NOTE, backupPoint.m_note));
    node.appendChild(createTextElement(doc, POSITION, QString::number(backupPoint.m_iPosition)));
    node.appendChild(createTextElement(doc, PATH, backupPoint.m_path));
    node.appendChild(createTextElement(doc, USERNAME, backupPoint.m_user));
    if (!backupPoint.m_userId.isEmpty()) {
        node.appendChild(createTextElement(doc, USERID, backupPoint.m_userId));
    }
    if (!backupPoint.m_os.isEmpty()) {
        node.appendChild(createTextElement(doc, OS, backupPoint.m_os));
    }
    if (!backupPoint.m_arch.isEmpty()) {
        node.appendChild(createTextElement(doc, ARCH, backupPoint.m_arch));
    }
    if (!backupPoint.m_archdetect.isEmpty()) {
        node.appendChild(createTextElement(doc, ARCHDETECT, backupPoint.m_archdetect));
    }
    if (!backupPoint.m_path.isEmpty()) {
        node.appendChild(createTextElement(doc, PREFIXDESTPATH, backupPoint.m_path));
    }
    if (!backupPoint.m_systemHash.isEmpty()) {
        node.appendChild(createTextElement(doc, SYSTEMHASH, backupPoint.m_systemHash));
    }
    if (!backupPoint.m_systemCommitHashMap.isEmpty()) {
        // 创建一个新的元素作为根节点
        QDomElement backupElement = doc.createElement("SystemCommitHashMap");
        // 将 QMap 中的键值对作为子元素添加
        QMap<QString, QString>::const_iterator it = backupPoint.m_systemCommitHashMap.constBegin();
        while (it != backupPoint.m_systemCommitHashMap.constEnd()) {
            // 使用 createTextElement 将键值对转换为子元素
            QString key = it.key();
            key.replace("/", "");
            backupElement.appendChild(createTextElement(doc, key, it.value()));
            ++it;
        }
        node.appendChild(backupElement);
    }
    if (!backupPoint.m_branchName.isEmpty()) {
        node.appendChild(createTextElement(doc, BRANCHNAME, backupPoint.m_branchName));
    }
    // 新增 DataCommitHashMap 保存逻辑
    if (!backupPoint.m_dataCommitHashMap.isEmpty()) {
        QDomElement dataElem = doc.createElement("DataCommitHashMap");
        for (auto it = backupPoint.m_dataCommitHashMap.constBegin();
             it != backupPoint.m_dataCommitHashMap.constEnd(); ++it) {
            QDomElement entry = doc.createElement("entry");
            entry.setAttribute("path", it.key());
            entry.setAttribute("type", it.value().isDirectory ? "dir" : "file");
            entry.setAttribute("hash", it.value().hash);
            dataElem.appendChild(entry);
        }
        node.appendChild(dataElem);
    }

    if (!backupPoint.m_karePaths.isEmpty()) {
        node.appendChild(createTextElement(doc, KAREPATH, backupPoint.m_karePaths));
    }
}

/**
 * @brief createTextElement
 * @param tagName
 * @param text
 * @return <tagName>text</tagName>
 */
QDomElement ParseBackupList::createTextElement(QDomDocument& doc, const QString& tagName, const QString& text)
{
    QDomElement node = doc.createElement(tagName);
    QDomText textNode = doc.createTextNode(text);
    node.appendChild(textNode);

    return node;
}

/**
 * @brief 新增备份节点
 * @param backupPoint, 备份点信息
 * @return SUCCESS成功； XML_PARSE_ERR失败
 */
ParseBackupList::ParseResult ParseBackupList::addItem(const BackupPoint& backupPoint)
{
    QDomDocument doc;
    if (!Doc_setContent(doc))
        return XML_PARSE_ERR;

    QDomElement backupPointNode = doc.createElement(BACKUPPOINT);
    backupPointToElementNode(backupPoint, doc, backupPointNode);

    QDomElement root = doc.documentElement();
    root.appendChild(backupPointNode);

    QFile xmlFile(m_xmlPath);
    if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return FAIL;

    QTextStream out(&xmlFile);
    doc.save(out, QDomNode::NodeType::CDATASectionNode);
    xmlFile.close();
    return SUCCESS;
}

/**
 * @brief 更新备份点信息
 * @param backupPoint，新的备份点信息
 * @return SUCCESS成功； XML_PARSE_ERR失败
 */
ParseBackupList::ParseResult ParseBackupList::updateItem(const BackupPoint & backupPoint)
{
    QDomDocument doc;
    if (!Doc_setContent(doc))
        return XML_PARSE_ERR;

    QDomElement root = doc.documentElement();
    QDomNodeList list = root.childNodes();

    int i = 0;
    for (; i < list.count(); i++) {
        QDomNode node = list.at(i);
        if (!node.isElement())
            continue;

        QDomElement eleUuid = node.firstChildElement(UUID);
        if (eleUuid.isNull() || backupPoint.m_uuid != eleUuid.text())
            continue;

        break ;
    }

    // 找到了旧节点
    if (i < list.count()) {
        // 移除旧节点，更新后的节点放到最后
        root.removeChild(list.at(i));
    }
    QDomElement newNode = doc.createElement(BACKUPPOINT);
    backupPointToElementNode(backupPoint, doc, newNode);
    root.appendChild(newNode);

    QFile xmlFile(m_xmlPath);
    if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return FAIL;
    }

    QTextStream out(&xmlFile);
    doc.save(out, QDomNode::NodeType::EntityReferenceNode);
    xmlFile.close();
    return SUCCESS;
}

/**
 * @brief 删除备份点记录
 * @param uuid
 * @return SUCCESS成功； XML_PARSE_ERR失败
 */
ParseBackupList::ParseResult ParseBackupList::deleteItem(const QString& uuid)
{
    QDomDocument doc;
    if (!Doc_setContent(doc))
        return XML_PARSE_ERR;

    QDomElement root = doc.documentElement();
    QDomNodeList list = root.childNodes();

    for (int i = 0; i < list.count(); i++) {
        QDomNode node = list.at(i);
        if (!node.isElement())
            continue;

        QDomElement eleUuid = node.firstChildElement(UUID);
        if (eleUuid.isNull() || uuid != eleUuid.text())
            continue;

        root.removeChild(node);

        break ;
    }

    QFile xmlFile(m_xmlPath);
    if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return FAIL;
    }

    QTextStream out(&xmlFile);
    doc.save(out, QDomNode::NodeType::EntityReferenceNode);
    xmlFile.close();
    return SUCCESS;
}

/**
 * @brief 获取备份列表
 * @return
 */
QList<ParseBackupList::BackupPoint> ParseBackupList::getBackupPointList()
{
    QList<ParseBackupList::BackupPoint> backupPointList;
    QDomDocument doc;
    if (!Doc_setContent(doc))
        return backupPointList;

    QDomElement root = doc.documentElement();
    QDomNodeList list = root.childNodes();

    for (int i = 0; i < list.count(); i++) {
        QDomNode node = list.at(i);
        if (!node.isElement())
            continue;

        QDomElement eleUuid = node.firstChildElement(UUID);
        if (eleUuid.isNull())
            continue;

        BackupPoint backupPoint;
        elementNodeToBackupPoint(node.toElement(), backupPoint);
        backupPointList << backupPoint;
    }

    // std::sort(backupPointList.begin(), backupPointList.end());

    return backupPointList;
}

/**
 * @brief 获取xml文件中数据备份点对应目录的commit哈希
 * @param uuid_name，xml文件中备份点uuid和backupname的映射
 * @return
 */
void ParseBackupList::getDataCommitHashMap(QString &systemHash, QMap<QString, FileEntry> &m_dataCommitHashMap, QString uuid)
{
    QDomDocument doc;
    if (!Doc_setContent(doc))
        return ;

    QDomElement root = doc.documentElement();
    QDomNodeList list = root.childNodes();

    for (int i = 0; i < list.count(); i++)
    {
        QDomNode node = list.at(i);
        if (!node.isElement())
            continue;

        QDomElement eleUuid = node.firstChildElement(UUID);
        if (eleUuid.text() == uuid)
        {
            QDomElement commitHash = node.firstChildElement("DataCommitHashMap");
            if (commitHash.isNull())
                continue;

            QDomNodeList entries = commitHash.elementsByTagName("entry");
            for (int j = 0; j < entries.count(); j++)
            {
                QDomElement entryElement = entries.at(j).toElement();
                if (!entryElement.isNull()) {
                    QString path = entryElement.attribute("path");
                    QString hash = entryElement.attribute("hash");
                    QString type = entryElement.attribute("type");

                    FileEntry entry;
                    entry.hash = hash;
                    entry.isDirectory = (type == "dir");

                    m_dataCommitHashMap.insert(path, entry);
                }
            }

            QDomElement nSystemHash = node.firstChildElement("SystemHash");
            if (nSystemHash.isNull() || nSystemHash.text().isEmpty())
                continue;

            if (nSystemHash.isElement()) {
                QString key = nSystemHash.nodeName();
                QString value = nSystemHash.firstChild().nodeValue();
                systemHash = value;
            }
        }
    }
}



int ParseBackupList::countBackupPoint()
{
    QDomDocument doc;
    if (!Doc_setContent(doc))
        return 0;

    QDomElement root = doc.documentElement();
    QDomNodeList list = root.childNodes();

    return list.count();
}

void ParseBackupList::getOldestBackupPoint(BackupPoint& backupPoint)
{
    QDomDocument doc;
    if (!Doc_setContent(doc))
    {
        return;
    }

    QDomElement root = doc.documentElement();
    QDomNodeList list = root.childNodes();

    if(list.count() > 0)
    {
        QDomNode node = list.at(0);

        elementNodeToBackupPoint(node.toElement(), backupPoint);
    }
}
