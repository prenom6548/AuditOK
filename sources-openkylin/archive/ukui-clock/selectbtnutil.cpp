/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "selectbtnutil.h"
#include "connection.h"
#include <QFileDialog>
#include <QFile>
#include "utils.h"
#include <QDateTime>
#include <QThread>
#include "constant_class.h"
#include "globalinstance.h"
#ifdef NOTKAIMING
#include "ukuiwindowhelper.h"
#endif

SelectBtnUtil::SelectBtnUtil(QObject *parent) : QObject(parent)
{
    m_bellQueryModel = clock_sql::getBellTable(this);
    idIndexMap = new QMap<QString,int>();
    indexIdMap = new QMap<int,QString>();
    allBellItem = new QList<QString>();
    refreshBellData();

}

SelectBtnUtil::~SelectBtnUtil()
{
    if(m_bellQueryModel != nullptr){
        delete m_bellQueryModel;
        m_bellQueryModel = nullptr;
    }
    if(idIndexMap != nullptr){
        delete idIndexMap;
        idIndexMap = nullptr;
    }
    if(indexIdMap != nullptr){
        delete indexIdMap;
        indexIdMap = nullptr;
    }
    if(allBellItem != nullptr){
        delete allBellItem;
        allBellItem = nullptr;
    }
}

int SelectBtnUtil::getBellListSize()
{
    return allBellItem->size();
}

int SelectBtnUtil::getBellIndexById(QString id)
{
    int index= idIndexMap->value(id);
    return index;
}

QString SelectBtnUtil::getBellIdByIndex(int index)
{
    QString id = indexIdMap->value(index);
    return id;
}

QString SelectBtnUtil::getBellNameById(QString id)
{
    auto sqlQuery = getBellkByPK(id);
    QString name = sqlQuery.value(1).toString();
    return name;
}

QString SelectBtnUtil::getBellPathById(QString id)
{
    auto sqlQuery = getBellkByPK(id);
    QString path = sqlQuery.value(2).toString();
    return path;
}

QString SelectBtnUtil::getDefaultBellId()
{
    if(m_defaultBellId.isEmpty()){
        auto sqlQuery = getBellkBynameEn("none");
        QString id = sqlQuery.value(0).toString();
        m_defaultBellId = id;
    }
    return m_defaultBellId;
}

QString SelectBtnUtil::getBellIdByNameEn(QString nameEn)
{
    auto sqlQuery = getBellkBynameEn(nameEn);
    QString id = sqlQuery.value(0).toString();
    return id;
}

QSqlQuery SelectBtnUtil::getBellkByPK(QString id)
{
    auto sqlQuery = clock_sql::getQSqlQuery();
    sqlQuery.prepare("select * from bell where id=:id");
    sqlQuery.bindValue(":id",id);
    sqlQuery.exec();
    sqlQuery.next();
    return sqlQuery;
}

QSqlQuery SelectBtnUtil::getBellkBynameEn(QString nameEn)
{
    auto sqlQuery = clock_sql::getQSqlQuery();
    sqlQuery.prepare("select * from bell where bell_en=:bell_en limit 1");
    sqlQuery.bindValue(":bell_en",nameEn);
    sqlQuery.exec();
    sqlQuery.next();
    return sqlQuery;

}

/**
 * @brief 检查铃声文件是否已存在
 * @param filePath 文件路径
 * @return 存在的铃声ID，不存在则返回空
 */
QString SelectBtnUtil::checkBellExists(QString filePath)
{
    auto sqlQuery = clock_sql::getQSqlQuery();
    sqlQuery.prepare("select id from bell where bell_path=:bell_path limit 1");
    sqlQuery.bindValue(":bell_path", filePath);
    sqlQuery.exec();
    if (sqlQuery.next()) {
        return sqlQuery.value(0).toString();
    }
    return QString();
}

QList<QString> *SelectBtnUtil::getAllBellItem() const
{
    return allBellItem;
}

int SelectBtnUtil::getCurrentLocale() const
{
    QString lan = QLocale().name();
    int localeNum;
    if(QString::compare(lan,"zh_CN",Qt::CaseInsensitive) == 0){
        localeNum = 1;
    }else if(QString::compare(lan,"bo_CN",Qt::CaseInsensitive) == 0){
        localeNum = 2;
    }else if(QString::compare(lan,"de_DE",Qt::CaseInsensitive) == 0){
        localeNum = 3;
    }else if(QString::compare(lan,"es_ES",Qt::CaseInsensitive) == 0){
        localeNum = 4;
    }else if(QString::compare(lan,"fr",Qt::CaseInsensitive) == 0){
        localeNum = 5;
    }else if(QString::compare(lan,"kk_KZ",Qt::CaseInsensitive) == 0){
        localeNum = 6;
    }else if(QString::compare(lan,"ky_KG",Qt::CaseInsensitive) == 0){
        localeNum = 7;
    }else if(QString::compare(lan,"mn_MN",Qt::CaseInsensitive) == 0){
        localeNum = 8;
    }else if(QString::compare(lan,"ug_CN",Qt::CaseInsensitive) == 0){
        localeNum = 9;
    }else if(QString::compare(lan,"zh_HK",Qt::CaseInsensitive) == 0){
        localeNum = 10;
    }else {
        localeNum = 11;
    }
    return localeNum;
}
void SelectBtnUtil::refreshBellData()
{
    int index=0;
    auto sqlQuery = clock_sql::getQSqlQuery();
    sqlQuery.exec("select * from bell where bell_type=0 order by create_time ASC");
    idIndexMap->clear();
    indexIdMap->clear();
    allBellItem->clear();
    while (sqlQuery.next()) {
        QString itemId = sqlQuery.value(0).toString();
        idIndexMap->insert(itemId,index);
        indexIdMap->insert(index,itemId);
        QString bellCn = sqlQuery.value(1).toString();
        allBellItem->append(bellCn);
        index++;
    }
    sqlQuery.clear();
    //最新的前5个
    sqlQuery.exec("select * from bell where bell_type=1 order by create_time DESC limit 5");
    QList<QString> * bellIdList = new QList<QString>();
    QList<QString> * bellNameList = new QList<QString>();
    while (sqlQuery.next()) {
        QString itemId = sqlQuery.value(0).toString();
        bellIdList->append(itemId);
        QString bellCn = sqlQuery.value(1).toString();
        bellNameList->append(bellCn);
    }
    //逆序
    int bellListSize = bellIdList->size();
    for (int i=0;i<bellListSize;i++) {
        QString itemId = bellIdList->at(bellListSize-1-i);
        idIndexMap->insert(itemId,index);
        indexIdMap->insert(index,itemId);
        QString bellCn = bellNameList->at(bellListSize-1-i);
        allBellItem->append(bellCn);
        index++;
    }
    delete bellIdList;
    delete bellNameList;
    //自定义铃声
    allBellItem->append(tr("diy bell"));
}

QString SelectBtnUtil::getDefaultBellStr()
{
    QString str = getDefaultBellEn();
    return tr(str.toLatin1().data());
}

QString SelectBtnUtil::getDefaultBellEn()
{
    return "relax";
}

QList<QString> *SelectBtnUtil::getDefaultBellList()
{
    QList<QString> * list = new QList<QString>();
    list->append("none");
    list->append("glass");
    list->append("bark");
    list->append("sonar");
    list->append("drip");
    return list;
}

QList<QString> *SelectBtnUtil::getDefaultBellTrList()
{
    QList<QString> * list = new QList<QString>();
    list->append(tr("none"));
    list->append(tr("glass"));
    list->append(tr("bark"));
    list->append(tr("sonar"));
    list->append(tr("drip"));
    return list;
}

bool SelectBtnUtil::mkdir(QString path){
    bool res=false;
    QDir targetDir(path);
    if(!targetDir.exists()){
        res = targetDir.mkdir(targetDir.absolutePath());
    }
    return res;
}
QString SelectBtnUtil::copyAudioFile(QString from)
{
    QString fileName =getFileNameFromPath(from);
    QString path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    path = path.append(DIY_BELL_SAVE_PATH);
    QDir dir;
    bool dirResult = dir.mkpath(path);
    qWarning()<<"dbq-dirResult"<<dirResult<<path;
    QString to = path+fileName;
    bool res = copyFile(from,to);
    qWarning()<<"dbq-复制文件结束"<<res;
    if(!res){
        to = "";
    }
    return to;
}

/**
 * @brief 获取目标路径（仅计算路径，不复制）
 * @param from 源文件路径
 * @return 目标路径
 */
QString SelectBtnUtil::getTargetPath(QString from)
{
    QString fileName =getFileNameFromPath(from);
    QString path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    path = path.append(DIY_BELL_SAVE_PATH);
    return path + fileName;
}

bool SelectBtnUtil::copyFile(QString from, QString to)
{
    if (from == to){
        return true;
    }
    if (!QFile::exists(from)){
        return false;
    }
    QFile source(from);
    bool res = source.copy(to);
    if (!res)
    {
        qDebug() << "File error" << source.error();
    }
    return res;
}

QString SelectBtnUtil::getFileNameFromPath(QString path)
{
       QStringList list = path.split(QDir::separator());
       QString name = list.at(list.size()-1);
       return name;
}

QString SelectBtnUtil::getFileNameWithoutSuffix(QString FileName)
{
    QStringList list = FileName.split(".");
    QString name = list.at(0);
    return name;
}

QString SelectBtnUtil::saveToBellTable(QString filePath)
{
    QString fileName =getFileNameFromPath(filePath);
    QString name = getFileNameWithoutSuffix(fileName);
    m_bellQueryModel->select();
    int rowNum = m_bellQueryModel->rowCount();
    int i = rowNum;
    m_bellQueryModel->insertRow(i);
    auto id = Utils::getRandomId();
    m_bellQueryModel->setData(m_bellQueryModel->index(i, 0),id);
    m_bellQueryModel->setData(m_bellQueryModel->index(i, 1), name);
    m_bellQueryModel->setData(m_bellQueryModel->index(i, 2), filePath);
    QDateTime time = QDateTime::currentDateTime();   //获取当前时间
    qint64 timeT = time.toMSecsSinceEpoch();
    m_bellQueryModel->setData(m_bellQueryModel->index(i, 3), timeT);
    m_bellQueryModel->setData(m_bellQueryModel->index(i, 4), 1);
    m_bellQueryModel->submitAll();
    //排序，暂停1ms
    QThread::msleep(1);
    refreshBellData();
    return id;
}

QString SelectBtnUtil::openAudioFileDialog(QWidget * parent)
{
    KylinQFileDialog dialog;
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("audio files(*mp3 *wav *ogg)"));
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setWindowTitle(tr("select bell"));
    QStringList fileNames;
    QString fileName="";
#ifdef NOTKAIMING
    if(GlobalInstance::getInstance()->session_desktop() == GlobalInstance::KYLIN_WLCOM){
        UkuiWindowHelper *windowHelper = new UkuiWindowHelper(&dialog);
        windowHelper->removeTitleBar();
    } else {
        kdk::UkuiStyleHelper::self()->removeHeader(&dialog);
    }
#else
    kdk::UkuiStyleHelper::self()->removeHeader(&dialog);
#endif
    if (dialog.exec()==QDialog::Accepted){
        fileNames = dialog.selectedFiles();
    }else{
    }

    if(fileNames.length()>0){
        fileName=fileNames.at(0);
    }
    return fileName;
}
