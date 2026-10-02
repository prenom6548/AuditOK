/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "shredqthread.h"
#include <QByteArray>
#include <QDebug>
#include <QTime>
#include <QFileInfo>
#include <QDir>

ShredQThread::ShredQThread(QStringList strFilePath)
{
    this->m_strFileNames = strFilePath;
}

ShredQThread::~ShredQThread() {}

// void ShredQThread::run()
// {
//     QDateTime time1 = QDateTime::currentDateTime();
//     bool isSuccess = true;
//     QStringList failFileName = {};
//     QStringList successFileName = {};
//     if (m_strFileNames.length() > 0) {
//         for (int i = 0; i < m_strFileNames.length(); i++) {
//             QByteArray ba = m_strFileNames.at(i).toUtf8();
//             // 将文件名和文件大小传入后续函数
//             // int result = do_file(ba.data());
//             int result = do_file(ba.data(), ba.length());
//             if (result != 0) {
//                 isSuccess = false;
//                 // failed
//                 qDebug() << Q_FUNC_INFO << result << m_strFileNames.at(i);
//                 failFileName.append(m_strFileNames.at(i));
//             } else {
//                 successFileName.append(m_strFileNames.at(i));
//             }
//         }
//     } else {
//         isSuccess = false;
//     }
    
//     QDateTime time2 = QDateTime::currentDateTime();
//     // int secs = time1.secsTo(time2);
//     // qDebug() << "time1" << time1 << "time2:" << time2 << " ======= " << secs;
//     qDebug() << "time1" << time1 << "time2:" << time2 ;
//     if (isSuccess) {
//         Q_EMIT success();
//     } else {
//         Q_EMIT failed(failFileName, successFileName);
//     }
// }

void ShredQThread::run()
{
    m_failFileName.clear();
    m_successFileName.clear();
    m_fileList.clear();
    m_allFailFiles.clear();
    m_allRmDirs.clear();
    m_allRemainFailDirs.clear();

    if (m_strFileNames.length() > 0) {
        for (int n = 0; n < m_strFileNames.length(); n++) {
            //找到所有的要粉碎的文件
            listOfShredFiles(m_strFileNames.at(n));
        }
    }
    if (m_strFileNames.length() > 0) {
        //判断可粉碎文件列表不为空
        if (m_fileList.length() > 0) {
            //遍历粉碎列表去粉碎文件
            for (int m = 0; m < m_fileList.length(); m++) {
                QByteArray ba = m_fileList.at(m).toUtf8();
                //文件粉碎功能
                // int result = do_file(ba.data());
                int result = do_file(ba.data(), ba.length());
                if (result != 0) {
                    // failed
                    qDebug() << Q_FUNC_INFO << result << m_fileList.at(m);
                    if (m_strFileNames.contains(m_fileList.at(m))) {
                        if (!m_failFileName.contains(m_fileList.at(m))) {
                            m_failFileName.append(m_fileList.at(m));
                        }
                    }
                    if (!m_allFailFiles.contains(m_fileList.at(m))) {
                        m_allFailFiles.append(m_fileList.at(m));
                    }
                } else {
                    m_successFileName.append(m_fileList.at(m));
                }
            }
        }
    }
    //查找需要删除的文件夹，如有删不掉的，存起来
    for (int k = 0; k < m_strFileNames.length(); k++) {
        QFileInfo file(m_strFileNames.at(k));
        if (file.isDir()) {
            //            findRmDir(m_strFileNames.at(i));
            m_allRmDirs.append(m_strFileNames.at(k));
        }
    }
    m_allRemainFailDirs = m_allRmDirs;
    if (m_allRmDirs.length() > 0) {
        for (int p = 0; p < m_allRmDirs.length(); p++) {
            findRmDir(m_allRmDirs.at(p));
        }
    }

    //统一对还存在的主文件夹进行删除-这时候已经处理完所有子文件夹和没有文件的文件夹
    if (m_allRemainFailDirs.length() > 0) {
        for (int q = 0; q < m_allRemainFailDirs.length(); q++) {
            findRmDir(m_allRemainFailDirs.at(q));
        }
    }
    qDebug() << m_allFailFiles;
    if (m_allFailFiles.length() < 1) {
        Q_EMIT success();
    } else {
        // qDebug() << "失败的文件夹: " << m_failFileName << "成功文件: " << m_successFileName << "全部失败的文件夹和文件名: " << m_allFailFiles;
        qDebug() << "失败的文件夹: " << m_failFileName << "全部失败的文件夹和文件名: " << m_allFailFiles;
        Q_EMIT failed(m_failFileName, m_successFileName, m_allFailFiles);
    }
    return;
}

QStringList ShredQThread::listOfShredFiles(QString targetFile)
{

    QFileInfo fileInfo(targetFile);
    if (fileInfo.isDir()) {
        QDir dir(targetFile);
        QFileInfoList listInfo = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
        qDebug() << " 文件夹中包含内:" << listInfo;
        if (listInfo.length() < 1) {
            return m_fileList;
        }
        //存所有的文件夹和对应的文件数目-备用
        m_dirMap.insert(targetFile, listInfo.length());
        for (int j = 0; j < listInfo.length(); j++) {
            //遍历文件夹
            QFileInfo subFileInfo(listInfo.at(j).absoluteFilePath());
            if (subFileInfo.isDir()) {
                listOfShredFiles(listInfo.at(j).absoluteFilePath());
            } else {
                if (!m_fileList.contains(listInfo.at(j).absoluteFilePath())) {
                    m_fileList.append(listInfo.at(j).absoluteFilePath());
                }
            }
        }

    } else {
        if (fileInfo.exists()) {
            if (!m_fileList.contains(targetFile)) {
                m_fileList.append(targetFile);
            }
        } else {
            if (!m_failFileName.contains(targetFile)) {
                m_failFileName.append(targetFile);
            }
            if (!m_allFailFiles.contains(targetFile)) {
                m_allFailFiles.append(targetFile);
            }
        }
    }
    return m_fileList;
}

void ShredQThread::findRmDir(QString dirPath)
{
    QDir dir(dirPath);
 
    // 确保路径是一个目录
    if (!dir.exists()) {
        return;
    }
    // 遍历并删除所有文件和子目录
    foreach (QFileInfo fileInfo, dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
        if (fileInfo.isDir()) {
            // 如果是目录，递归删除
            findRmDir(fileInfo.absoluteFilePath());
        } else {
            // 如果是文件，直接删除
            removeFile(fileInfo.absoluteFilePath());
        }
    }
 
    // 删除现在空的目录
    if (!dir.rmdir(QDir::currentPath())) {
    qDebug() << " 删除目录路径 " << dirPath;
        // 如果目录不为空，则不能删除，可以在这里处理错误
        if (removeDir(dirPath)) {
            if (m_strFileNames.contains(dirPath) && !m_successFileName.contains(dirPath)) {
                m_successFileName.append(dirPath);
                m_allRemainFailDirs.removeOne(dirPath);
            }
        } else {
            if (m_strFileNames.contains(dirPath) && !m_failFileName.contains(dirPath)) {
                m_failFileName.append(dirPath);
            }
            if (!m_allFailFiles.contains(dirPath)) {
                m_allFailFiles.append(dirPath);
            }
        }
    }
}

bool ShredQThread::removeDir(QString dir)
{
    QFileInfo dirInfo(dir);
    if (dirInfo.exists()) {
        QByteArray ba = dir.toUtf8();
        //文件夹删除
        int result = rm_dir(ba.data());
        if (result != 0) {
            qDebug() << "rm - Rm dir fail";
            return false;
        } else {
            return true;
        }
    } else {
        qDebug() << "rm - Dir is not exists";
        return false;
    }
    return false;
}

void ShredQThread::removeFile(QString file)
{
    QByteArray ba = file.toUtf8();
    //文件粉碎功能
    // int result = do_file(ba.data());
    int result = do_file(ba.data(), ba.length());
    if (result != 0) {
        // failed
        qDebug() << Q_FUNC_INFO << result << file;
        if (m_strFileNames.contains(file)) {
            if (!m_failFileName.contains(file)) {
                m_failFileName.append(file);
            }
        }
        if (!m_allFailFiles.contains(file)) {
            m_allFailFiles.append(file);
        }
    } else {
        m_successFileName.append(file);
    }
}
