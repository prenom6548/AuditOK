/*
 * Peony Intelligent Spaces Service
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: He JinQin <hejinqin@kylinos.cn>
 *
 */

#ifndef SEMANTICKBMANAGER_H
#define SEMANTICKBMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>

#ifdef KY_SDK_ADVANCED_AI
#include "kylin-ai/advai/knowledgebase/session.h"
#include "kylin-ai/advai/knowledgebase/knowledgebaseinfo.h"
#include "kylin-ai/advai/knowledgebase/chatconfig.h"
#include "kylin-ai/advai/knowledgebase/chatresult.h"
#include "kylin-ai/advai/knowledgebase/chatresponse.h"
#include "kylin-ai/advai/knowledgebase/chathistory.h"
#include "kylin-ai/advai/knowledgebase/types.h"
#include "kylin-ai/advai/knowledgebase/error.h"
#include "kylin-ai/advai/knowledgebase/retrievalconfig.h"
#include "kylin-ai/advai/knowledgebase/retrievalresponse.h"
#endif

/**
 * @brief SemanticKBManager - Manages semantic knowledge bases for spaces
 *
 * This class provides management of semantic knowledge bases associated
 * with intelligent spaces. It uses a single shared KBSession for all
 * operations, initialized at startup.
 */
class SemanticKBManager : public QObject
{
    Q_OBJECT
public:
    static SemanticKBManager *globalInstance();

    // Session management
    bool init();
    void destroy();
    bool isInitialized() const;

    // Knowledge base management
    QString getOrCreateKBForSpace(const QString &spaceName, const QString &spaceId);
    bool deleteKBForSpace(const QString &spaceId);
    QString getKBIdForSpace(const QString &spaceId);

    // Document operations
    bool addDocumentToSpace(const QString &spaceId, const QString &docPath, const QString &docId);
    bool addDocumentsToSpace(const QString &spaceId, const QStringList &docPaths, const QStringList &docIds, QList<int> *errorCodes = nullptr);
    bool deleteDocumentFromSpace(const QString &spaceId, const QString &docId);
    bool deleteDocumentsFromSpace(const QString &spaceId, const QStringList &docIds, QList<int> *errorCodes = nullptr);
    bool deleteAllDocumentsInSpace(const QString &spaceId);
    QStringList listDocumentsInSpace(const QString &spaceId);
    int getDocumentStatus(const QString &spacename,const QString &spaceId, const QString &docId, int *status = nullptr);
    bool deleteFileFromAllSpaces(const QString &filePath, const QString &fileDocId);

    // Chat operations
    QString chatWithSpace(const QString &spaceId, const QString &question,
                          const QString &model = QString());
//    QString chatWithSpaceWithHistory(const QString &spaceId, const QString &question,
//#ifdef KY_SDK_ADVANCED_AI
//                                     KBChatHistory *history, const QString &model = QString());
//#else
//                                     void *history, const QString &model = QString());
//#endif

    // Retrieval operations
    QStringList retrievalInSpace(const QString &spaceId, const QString &query, int topK = 5);

    // Utility
    void removeSpace(const QString &spaceId);
    QStringList getAllManagedSpaceIds();

    // Condition change tracking
    bool isSpaceConditionChanged(const QString &spaceId);
    void setSpaceConditionChanged(const QString &spaceId, bool changed);

    // Error handling
    int lastError() const {
        #ifdef KY_SDK_ADVANCED_AI
            return m_lastError;
        #else
            return -1;
        #endif
    }
    QString errorMessage(int errorCode) const;

Q_SIGNALS:
    void documentProcessingFinished(const QString &spaceId, const QString &docId, bool success);
    void spaceKBCreated(const QString &spaceId, const QString &kbId);
    void spaceKBDeleted(const QString &spaceId);

private:
    explicit SemanticKBManager(QObject *parent = nullptr);
    ~SemanticKBManager();
    Q_DISABLE_COPY(SemanticKBManager)

    bool initSession();
    QString generateKBName(const QString &spaceName, const QString &spaceId);
    QString getKBIdByName(const QString &name);
    QString errorMessageInternal(int errorCode) const;

    // Config path for KB mapping
    QString getKBConfigPath() const;
    void loadKBMapping();
    void saveKBMapping();

    QMap<QString, QString> m_spaceToKB;  // spaceId -> kbId
    QMap<QString, bool> m_spaceConditionChanged;  // spaceName -> condition changed flag
    QString m_appName;
    QString m_password;
    QString m_dbPath;

    // Single shared session for all KB operations
#ifdef KY_SDK_ADVANCED_AI
    KBSession *m_session = nullptr;
    int m_lastError = AISDK_NO_ERROR;
#endif
};

#endif // SEMANTICKBMANAGER_H
