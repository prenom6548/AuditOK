/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "ai-indexer.h"
#include <unistd.h>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QMutex>
#include <QDebug>
#include <kylin-ai/business/data-management/datamanagement.h>

namespace UkuiSearch {
class AiIndexerPrivate
{
public:
    static QMutex m_mutex;
    static bool m_sessionValid;
    static bool m_modelReady;
    static bool m_AISupported;
    static DataManagementSession* m_session;
};
DataManagementSession* AiIndexerPrivate::m_session = nullptr;
bool AiIndexerPrivate::m_sessionValid = false;
bool AiIndexerPrivate::m_modelReady = false;
bool AiIndexerPrivate::m_AISupported = false;
QMutex AiIndexerPrivate::m_mutex;
AiIndexer::AiIndexer(QObject *parent) : QObject(parent), d(new AiIndexerPrivate)
{
    AiIndexerPrivate::m_mutex.lock();
}

AiIndexer::~AiIndexer()
{
    AiIndexerPrivate::m_mutex.unlock();
    if(d) {
        delete d;
        d = nullptr;
    }
}

bool AiIndexer::createSession()
{
    if(!AiIndexerPrivate::m_sessionValid) {
        AiIndexerPrivate::m_session = data_management_create_session();
        qDebug() << "===creatSession===" << AiIndexerPrivate::m_session;
        if (AiIndexerPrivate::m_session) {
            AiIndexerPrivate::m_sessionValid = true;
            return true;
        }
        return false;
    } else {
        return true;
    }
}

bool AiIndexer::reCreateSession()
{
    destroySession();
    return createSession();
}

void AiIndexer::destroySession()
{
    AiIndexerPrivate::m_sessionValid = false;
    data_management_destroy_session(AiIndexerPrivate::m_session);
}

bool AiIndexer::checkFileSupported(const QString &filePath, QString &result)
{
    if(!AiIndexerPrivate::m_sessionValid) {
        return false;
    }
    char *originalResult;
    DataManagementStatus resStatus = data_management_check_file_format(AiIndexerPrivate::m_session, filePath.toLocal8Bit().data(), &originalResult);
    if (resStatus.errorCode == 101) {
        qWarning() << "AiIndexer:DATA_MANAGEMENT_SESSION_ERROR occured, re Creating session";
        if(!reCreateSession()) {
            qWarning() << "AiIndexer:Failed to create session";
            data_management_destroy_result(originalResult);
            return false;
        }
        qDebug() << "Creating session success.";
        resStatus = data_management_check_file_format(AiIndexerPrivate::m_session, filePath.toLocal8Bit().data(), &originalResult);
    }
    if (resStatus.success) {
        QJsonObject jo = QJsonDocument::fromJson(QByteArray(originalResult)).object();
        data_management_destroy_result(originalResult);
        if(jo.value(QStringLiteral("supported")).toBool()) {
            result = jo.value(QStringLiteral("format")).toString();
            return true;
        }
    };
    return false;
}

bool AiIndexer::addTextFileIndex(const QJsonArray &object)
{
    if(!AiIndexerPrivate::m_sessionValid) {
        return false;
    }
    DataManagementStatus resStatus = data_management_add_text_files(AiIndexerPrivate::m_session, QJsonDocument(object).toJson().data());
    if(resStatus.errorCode == 101) {
        qWarning() << "AiIndexer:DATA_MANAGEMENT_SESSION_ERROR occured, re Creating session";
        if(!reCreateSession()) {
            qWarning() << "AiIndexer:Failed to create session";
            return false;
        }
        qDebug() << "Creating session success.";
        resStatus = data_management_add_text_files(AiIndexerPrivate::m_session, QJsonDocument(object).toJson().data());
    }
    if(!resStatus.success) {
        qWarning() << "Fail to call data_management_add_text_files, input: " << QJsonDocument(object).toJson().data()
                   << "result:" << resStatus.errorMsg;
        return false;
    }
    return true;
}

bool AiIndexer::addImageFileIndex(const QJsonArray &object)
{
    if(!AiIndexerPrivate::m_sessionValid) {
        return false;
    }
    DataManagementStatus resStatus = data_management_add_image_files(AiIndexerPrivate::m_session, QJsonDocument(object).toJson().data());
    if(resStatus.errorCode == 101) {
        qWarning() << "AiIndexer:DATA_MANAGEMENT_SESSION_ERROR occured, re Creating session";
        if(!reCreateSession()) {
            qWarning() << "AiIndexer:Failed to create session";
            return false;
        }
        qDebug() << "Creating session success.";
        resStatus = data_management_add_image_files(AiIndexerPrivate::m_session, QJsonDocument(object).toJson().data());
    }
    if (!resStatus.success) {
        qWarning() << "Fail to call data_management_add_image_files, input: " << QJsonDocument(object).toJson().data()
                   << "result:" << resStatus.errorMsg;
        return false;
    }
    return true;
}

bool AiIndexer::deleteFileIndex(const QStringList &files)
{
    if(!AiIndexerPrivate::m_sessionValid) {
        qWarning() << "deleteFileIndex: Session is not valid";
        return false;
    }
    QJsonArray tmpArray;

    for(const QString &file : files) {
        QJsonObject oneFile;
        oneFile.insert(QStringLiteral("filepath"), file);
        tmpArray.append(oneFile);
    }
    DataManagementStatus resStatus = data_management_delete_files(AiIndexerPrivate::m_session, QJsonDocument(tmpArray).toJson().data());
    if (resStatus.errorCode == 101) {
        qWarning() << "AiIndexer:DATA_MANAGEMENT_SESSION_ERROR occured, re Creating session";
        if(!reCreateSession()) {
            qWarning() << "AiIndexer:Failed to create session";
            return false;
        }
        qDebug() << "Creating session success.";
        resStatus = data_management_delete_files(AiIndexerPrivate::m_session, QJsonDocument(tmpArray).toJson().data());
    }
    if (!resStatus.success) {
        qWarning() << "Fail to call data_management_delete_files, input: " << QJsonDocument(tmpArray).toJson().data()
                   << "result:" << resStatus.errorMsg;
        return false;
    }
    return true;
}

bool AiIndexer::getAllIndexedFiles(QJsonObject &object)
{
    if(!AiIndexerPrivate::m_sessionValid) {
        return false;
    }
    char *originalResult;
    DataManagementStatus resStatus = data_management_get_all_fileinfos(AiIndexerPrivate::m_session, &originalResult);
    if (resStatus.errorCode == 101) {
        qWarning() << "AiIndexer:DATA_MANAGEMENT_SESSION_ERROR occured, re Creating session";
        if(!reCreateSession()) {
            qWarning() << "AiIndexer:Failed to create session";
            data_management_destroy_result(originalResult);
            return false;
        }
        qDebug() << "Creating session success.";
        resStatus = data_management_get_all_fileinfos(AiIndexerPrivate::m_session, &originalResult);
    }
    if (resStatus.success) {
        QJsonArray array = QJsonDocument::fromJson(QByteArray(originalResult)).array();
        data_management_destroy_result(originalResult);
        for(const QJsonValueRef value : array) {
            object.insert(value.toObject().value(QStringLiteral("filepath")).toString(), value.toObject().value(QStringLiteral("timestamp")).toInt());
        }
        return true;
    }
    return false;
}

bool AiIndexer::update(const QJsonArray &object)
{
    if(!AiIndexerPrivate::m_sessionValid) {
        return false;
    }
    DataManagementStatus resStatus = data_management_update_files_content(AiIndexerPrivate::m_session, QJsonDocument(object).toJson().data());
    if (resStatus.errorCode == 101) {
        qWarning() << "AiIndexer:DATA_MANAGEMENT_SESSION_ERROR occured, re creating session";
        if(!reCreateSession()) {
            qWarning() << "AiIndexer:Failed to create session";
            return false;
        }
        qDebug() << "Creating session success.";
        resStatus = data_management_update_files_content(AiIndexerPrivate::m_session, QJsonDocument(object).toJson().data());
    }
    if (!resStatus.success) {
        qWarning() << "Fail to call data_management_update_files_content, input: " << QJsonDocument(object).toJson().data()
                   << "result:" << resStatus.errorMsg;
        return false;
    }
    return true;
}

bool AiIndexer::aiIsReady() {
    if (data_management_is_feature_supported()) {
        AiIndexerPrivate::m_AISupported = true;
    } else {
        qWarning() << "AI Search is not supported";
    }

    if (createSession()) {
        FeatureStatus status = FeatureStatus::UNKNOWN;
        DataManagementStatus result = data_management_get_feature_status(AiIndexerPrivate::m_session, &status);
        if (result.success) {
            if (status == FeatureStatus::AVAILABLE) {
                AiIndexerPrivate::m_modelReady = true;
            } else {
                qWarning() << "Model is not available";
            }
        } else {
            qWarning() << "Get feature status failed, result is" << result.errorCode << result.errorMsg;
        }
    } else {
        qWarning() << "Fail to create session";
    }

    return (AiIndexerPrivate::m_AISupported && AiIndexerPrivate::m_modelReady);
}

} // UkuiSearch