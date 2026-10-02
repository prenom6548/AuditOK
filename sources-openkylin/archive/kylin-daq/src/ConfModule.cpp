/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
#include "ConfModule.h"

QScopedPointer<ConfModule> ConfModule::ms_instance;
ConfModule::ConfModule(QObject *parent)
    :BaseModule(s_moduleName,parent)
    ,m_timer(new QTimer(this))
    ,m_defaultFilePath(KylinDaqCommon::gs_defaultConfFilePath)
    ,m_cacheFolderPath(KylinDaqCommon::gs_cacheFolderPath)
    ,m_confFileName(KylinDaqCommon::gs_confFileName)
    ,m_fileSystemWatcher(new QFileSystemWatcher(this))
    ,m_confReadTimer(new QTimer(this))
{
    checkDir(m_cacheFolderPath);
    m_fileSystemWatcher->addPath(m_cacheFolderPath);
    m_fileSystemWatcher->addPath(m_cacheFolderPath+m_confFileName);
    m_fileSystemWatcher->addPath(m_defaultFilePath);

    // 目录变化：仍然调用原来的 checkDir，同时触发一次延迟读取
    connect(m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged,this, &ConfModule::checkDir);

    // 文件变化：不直接 readConf，而是做防抖延迟
    connect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged,
            this, [this](const QString &path){
        Q_UNUSED(path);
        // 1：避免读到删/建/写的中间态
        m_confReadTimer->start(1000);
    });

    // 配置读取防抖定时器
    m_confReadTimer->setSingleShot(true);
    connect(m_confReadTimer, &QTimer::timeout,
            this, &ConfModule::doReadConfDelayed);

    // 其余原有连接逻辑（若有）请保持不变，例如：
    connect(m_timer, &QTimer::timeout, this, &ConfModule::getConf);
    connect(this , &ConfModule::pollingIntervalChanged ,this, &ConfModule::setTimePollingInterval);
}

ConfModule::~ConfModule()
{

}

ConfModule* ConfModule::instance(void)
{
    if (ms_instance.isNull()) {
        ms_instance.reset(new ConfModule());
    }
    return ms_instance.data();
}

void ConfModule::setTimePollingInterval(qlonglong sec)
{
    m_timer->stop();
    m_timer->setInterval(KylinDaqCommon::secToMsec(sec));
    m_timer->start();
}

QJsonObject ConfModule::readJson(const QString &filePath)
{
    QJsonObject jsonObj;
    QFile file(filePath);
    if(file.exists())
    {
        if(file.open(QFile::ReadOnly))
        {
            QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
            if(jsonDoc.isObject())
            {
                jsonObj = jsonDoc.object();
            }
            else
            {
                qWarning()<< "ConfModule::readJson: not json object, path ="<< filePath;
                qWarning()<< "c =" << jsonDoc;
            }
            file.close();
        }
        else
        {
            qWarning() << "ConfModule::readJson: open failed" << filePath << file.errorString();
        }
    }
    else
    {
        qWarning()<< "ConfModule::readJson: file not exist"<< filePath;
    }
    return jsonObj;
}

void ConfModule::writeJson(const QString &filePath, QJsonObject &jsonObj)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        qWarning() << "ConfModule::writeJson: open failed" << filePath << file.errorString();
        return;
    }
    QJsonDocument jsonDoc(jsonObj);
    QByteArray data = jsonDoc.toJson(QJsonDocument::Indented);
    qint64 writeByteNum = file.write(data);
    if(writeByteNum <= 0)
    {
        qWarning()<<"ConfModule::writeJson: writeByteNum ="<< writeByteNum<<"path ="<<filePath;
    }
    file.close();
}

/**
 * 延迟真正读取配置：
 * - 4：先确保 watcher 还在监控两个配置文件，如果丢了就补 addPath；
 * - 2：如果此刻文件仍不存在，说明刚删未重建，本轮直接跳过；
 * - 1：真正存在时，再调用 readConf，避免读到中间态。
 */
void ConfModule::doReadConfDelayed()
{
    const QString cacheConfPath = m_cacheFolderPath + m_confFileName;

    // 4）防止删除/重命名后 watcher 被移除，每次读前都补一次
    if (!m_fileSystemWatcher->files().contains(cacheConfPath)) {
        m_fileSystemWatcher->addPath(cacheConfPath);
    }
    if (!m_fileSystemWatcher->files().contains(m_defaultFilePath)) {
        m_fileSystemWatcher->addPath(m_defaultFilePath);
    }

    // 2）重建+覆盖：此时文件可能还没真正写好，不存在就跳过
    if (!QFile::exists(m_defaultFilePath)) {
        qWarning() << "ConfModule::doReadConfDelayed: default conf not exist" << m_defaultFilePath;
        return;
    }
    if (!QFile::exists(cacheConfPath)) {
        qWarning() << "ConfModule::doReadConfDelayed: cache conf not exist" << cacheConfPath;
        return;
    }

    // 1）时序窗口：延迟一段时间后再真正读取
    readConf(cacheConfPath);
}

void ConfModule::readConf(const QString &filePath)
{
    QJsonObject defaultConfJsonObj = readJson(m_defaultFilePath);
    QJsonObject confJsonObj = readJson(filePath);
    QJsonObject sendConfJsonObj;
    for(auto iter = defaultConfJsonObj.begin(); iter != defaultConfJsonObj.end(); ++iter)
    {
        if(iter.value().isObject())
        {
            QString moduleName = iter.key();
            if(!confJsonObj.isEmpty()
                && confJsonObj.contains(moduleName)
                && confJsonObj[moduleName].isObject())
            {
                sendConfJsonObj.insert(moduleName, confJsonObj[moduleName].toObject());
            }
        }
    }
    if(!sendConfJsonObj.isEmpty() && !defaultConfJsonObj.isEmpty())
    {
        qDebug()<< "ConfModule::readConf: file="<< m_fileSystemWatcher->files();
        emit sendConf(sendConfJsonObj, defaultConfJsonObj);
    }
}

void ConfModule::updateProperty(const QJsonObject &conf, const QJsonObject &defaultConf)
{
    qlonglong value = 0;
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_pollingIntervalKey, value);
    setPollingInterval(value);
}

void ConfModule::writeConf(const QString &moduleName, const QString &key, const QVariant &value)
{
    QString filePath = m_cacheFolderPath + m_confFileName;
    QJsonObject confJsonObj = readJson(filePath);
    QJsonObject moduleJsonObj;
    if(confJsonObj.contains(moduleName) && confJsonObj[moduleName].isObject())
    {
        moduleJsonObj = confJsonObj[moduleName].toObject();
        moduleJsonObj[key] = value.toJsonValue();
        confJsonObj[moduleName] = moduleJsonObj;
    }
    else
    {
        moduleJsonObj[key] = value.toJsonValue();
        confJsonObj.insert(moduleName, moduleJsonObj);
    }
    writeJson(filePath, confJsonObj);

    // 写完后确保 watch 还在（对应 4，但影响极小）
    if (!m_fileSystemWatcher->files().contains(filePath)) {
        m_fileSystemWatcher->addPath(filePath);
    }
}

void ConfModule::writeConf2(const QString &conf)
{
    qDebug() << "ConfModule::writeConf2:" << conf;
    bool updateConf = false;
    QString filePath = m_cacheFolderPath + m_confFileName;
    QJsonObject defaultConfJsonObj = readJson(m_defaultFilePath);
    QJsonObject confJsonObj = readJson(filePath);

    QJsonDocument jsonDoc = QJsonDocument::fromJson(conf.toUtf8());
    if(!jsonDoc.isObject())
    {
        return;
    }
    QJsonObject jsonObj = jsonDoc.object();
    for(auto moduleIterT = jsonObj.begin(); moduleIterT != jsonObj.end(); ++moduleIterT)
    {
        QString moduleName = moduleIterT.key();
        if(!defaultConfJsonObj.contains(moduleName))
        {
            continue;
        }
        if(!moduleIterT.value().isObject())
        {
            continue;
        }
        //判断moduleconfig合不合法
        QJsonObject defaultModuleConfJsonObj = defaultConfJsonObj[moduleName].toObject();
        QJsonObject moduleConfJsonObj = confJsonObj[moduleName].toObject();
        QJsonObject moduleJsonObj = moduleIterT.value().toObject();
        for(auto moduleConfIterT = moduleJsonObj.begin(); moduleConfIterT!=moduleJsonObj.end(); ++moduleConfIterT)
        {
            QString moduleConfKey = moduleConfIterT.key();
            if(!defaultModuleConfJsonObj.contains(moduleConfKey))
            {
                continue;
            }
            if(moduleName != s_transmitModuleKey || moduleConfKey != s_transmitModuleUploadStateKey)
            {
                moduleConfJsonObj[moduleConfKey] = moduleConfIterT.value();
            }
            updateConf = true;
        }
        confJsonObj[moduleName] = moduleConfJsonObj;
    }
    if(updateConf)
    {
        qInfo()<< "ConfModule::writeConf2 change config";
        writeJson(filePath, confJsonObj);

        if (!m_fileSystemWatcher->files().contains(filePath)) {
            m_fileSystemWatcher->addPath(filePath);
        }
    }
}

void ConfModule::checkDir(const QString &folderPath)
{
    QDir tempDir(folderPath);
    if(!tempDir.exists())
    {
        tempDir.mkpath(folderPath);
    }
}