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

#include "semantickbmanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QThread>
#include <ctime>

extern "C" {
#include <gio/gio.h>
}

static SemanticKBManager *s_instance = nullptr;

SemanticKBManager *SemanticKBManager::globalInstance()
{
    if (!s_instance) {
        s_instance = new SemanticKBManager();
    }
    return s_instance;
}

SemanticKBManager::SemanticKBManager(QObject *parent)
    : QObject(parent)
{
    // Default credentials - should be configurable
    m_appName = "peony-idm";
    m_password = "peony-idm-password";
    m_dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
               + "/semantic-knoweldgebase/";

    // Ensure directory exists
    QDir dir(QFileInfo(m_dbPath).absolutePath());
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    init();
    loadKBMapping();
}

SemanticKBManager::~SemanticKBManager()
{
    saveKBMapping();
    destroy();
}

bool SemanticKBManager::isInitialized() const
{
#ifdef KY_SDK_ADVANCED_AI
    return m_session != nullptr;
#else
    return false;
#endif
}

bool SemanticKBManager::initSession()
{
#ifdef KY_SDK_ADVANCED_AI
    if (m_session) {
        return true;
    }

    int error = AISDK_NO_ERROR;
    m_session = kb_session_create(m_appName.toUtf8().constData(),
                                  m_password.toUtf8().constData(),
                                  m_dbPath.toUtf8().constData(),
                                  &error);
    if (!m_session) {
        m_lastError = error;
        qWarning() << "Failed to create session:" << errorMessageInternal(error);
        return false;
    }

    error = kb_session_init(m_session);
    if (error != AISDK_NO_ERROR) {
        m_lastError = error;
        qWarning() << "Failed to init session:" << errorMessageInternal(error);
        kb_session_destroy(m_session);
        m_session = nullptr;
        return false;
    }

    qDebug() << "SemanticKBManager: Session initialized successfully";
    return true;
#else
    return false;
#endif
}

bool SemanticKBManager::init()
{
    return initSession();
}

void SemanticKBManager::destroy()
{
#ifdef KY_SDK_ADVANCED_AI
    if (m_session) {
        kb_session_destroy(m_session);
        m_session = nullptr;
    }
#endif
}

QString SemanticKBManager::errorMessageInternal(int errorCode) const
{
#ifdef KY_SDK_ADVANCED_AI
    const char *msg = kb_get_error_message(errorCode);
    return msg ? QString::fromUtf8(msg) : QString("Unknown error: %1").arg(errorCode);
#else
    Q_UNUSED(errorCode);
    return QString("Knowledge base functionality is not available");
#endif
}

QString SemanticKBManager::errorMessage(int errorCode) const
{
    return errorMessageInternal(errorCode);
}

QString SemanticKBManager::generateKBName(const QString &spaceName, const QString &spaceId)
{
    // QString realName = spaceName;
    // realName.replace(" ","");
    return QString("SpaceKB_%1").arg(spaceId);
}

QString SemanticKBManager::getKBConfigPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
           + "/semantic-kb-mapping.json";
}

void SemanticKBManager::loadKBMapping()
{
    QFile file(getKBConfigPath());
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = doc.object();

    for (auto it = obj.begin(); it != obj.end(); ++it) {
        QJsonObject spaceObj = it.value().toObject();
        m_spaceToKB[it.key()] = spaceObj.value("kbId").toString();
        m_spaceConditionChanged[it.key()] = spaceObj.value("conditionChanged").toBool();
    }
}

void SemanticKBManager::saveKBMapping()
{
    QJsonObject obj;
    for (auto it = m_spaceToKB.begin(); it != m_spaceToKB.end(); ++it) {
        QJsonObject spaceObj;
        spaceObj["kbId"] = it.value();
        spaceObj["conditionChanged"] = m_spaceConditionChanged.value(it.key(), false);
        obj[it.key()] = spaceObj;
    }

    QFile file(getKBConfigPath());
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        qWarning() << "Failed to save KB mapping";
        return;
    }

    file.write(QJsonDocument(obj).toJson());
}

QString SemanticKBManager::getKBIdByName(const QString &name)
{
#ifdef KY_SDK_ADVANCED_AI
    if (!m_session) {
        return QString();
    }

    int kbCount = 0;
    int error = AISDK_NO_ERROR;
    KnowledgeBaseInfo **kbInfos = kb_knowledgebase_info_list(m_session, &kbCount, &error);

    if (error != AISDK_NO_ERROR || !kbInfos) {
        m_lastError = error;
        return QString();
    }

    QString result;
    for (int i = 0; i < kbCount; ++i) {
        std::string kbName = kb_knowledgebase_info_get_name(kbInfos[i]);
        if (QString::fromStdString(kbName) == name) {
            result = kb_knowledgebase_info_get_id(kbInfos[i]);
            break;
        }
    }

    for (int i = 0; i < kbCount; ++i) {
        kb_knowledgebase_info_free(kbInfos[i]);
    }
    free(kbInfos);

    return result;
#else
    Q_UNUSED(name);
    return QString();
#endif
}

QString SemanticKBManager::getOrCreateKBForSpace(const QString &spaceName, const QString &spaceId)
{
#ifdef KY_SDK_ADVANCED_AI
    if (!initSession()) {
        return QString();
    }

    if (m_spaceToKB.contains(spaceId)) {
        if (isSpaceConditionChanged(spaceId)) {
            auto kbid = getKBIdForSpace(spaceId);
            deleteAllDocumentsInSpace(kbid);
            setSpaceConditionChanged(spaceId, false);
        }
        return m_spaceToKB[spaceId];
    }

    QString kbName = generateKBName(spaceName, spaceId);
    QString kbDesc = QString("%1_knowledgebase").arg(spaceName);

    // Check if KB already exists
    QString existingId = getKBIdByName(kbName);
    if (!existingId.isEmpty()) {
        m_spaceToKB[spaceId] = existingId;
        saveKBMapping();
        Q_EMIT spaceKBCreated(spaceId, existingId);
        return existingId;
    }

    // Create new KB
    KnowledgeBaseInfo *kbInfo = kb_knowledgebase_info_new(kbName.toUtf8().constData(),
                                                          kbDesc.toUtf8().constData());
    if (!kbInfo) {
        m_lastError = -1;
        qWarning() << "Failed to create knowledge base info";
        return QString();
    }

    int error = AISDK_NO_ERROR;
    char *kbId = kb_knowledgebase_create(m_session, kbInfo, &error);

    if (!kbId) {
        m_lastError = error;
        QString errormsg = errorMessageInternal(error);
        qWarning() << "Failed to create knowledge base:" << errorMessageInternal(error);
        kb_knowledgebase_info_free(kbInfo);
        return QString();
    }

    m_spaceToKB[spaceId] = kbId;
    saveKBMapping();

    qDebug() << "Created KB for space:" << spaceId << "kbId:" << kbId;
    kb_knowledgebase_info_set_id(kbInfo, kbId);
    Q_EMIT spaceKBCreated(spaceId, QString(kbId));
    //kb_free_string(kbId);
    //kb_knowledgebase_info_free(kbInfo);
    return m_spaceToKB[spaceId];
#else
    Q_UNUSED(spaceName);
    Q_UNUSED(spaceId);
    return QString();
#endif
}

bool SemanticKBManager::deleteKBForSpace(const QString &spaceId)
{
#ifdef KY_SDK_ADVANCED_AI
    if (!initSession()) {
        return false;
    }

    if (!m_spaceToKB.contains(spaceId)) {
        return true;
    }

    QString kbId = m_spaceToKB[spaceId];

    int error = kb_knowledgebase_delete(m_session, kbId.toUtf8().constData());
    if (error != AISDK_NO_ERROR) {
        m_lastError = error;
        qWarning() << "Failed to delete knowledge base:" << errorMessageInternal(error);
        return false;
    }

    m_spaceToKB.remove(spaceId);
    saveKBMapping();

    Q_EMIT spaceKBDeleted(spaceId);
    return true;
#else
    Q_UNUSED(spaceId);
    return true;
#endif
}

QString SemanticKBManager::getKBIdForSpace(const QString &spaceId)
{
    return m_spaceToKB.value(spaceId);
}

bool SemanticKBManager::addDocumentToSpace(const QString &spaceId, const QString &docPath, const QString &docId)
{
#ifdef KY_SDK_ADVANCED_AI
    if (!initSession()) {
        return false;
    }

    QString kbId = getKBIdForSpace(spaceId);
    if (kbId.isEmpty()) {
        qWarning() << "No KB for space:" << spaceId;
        return false;
    }

    // Check if document already exists
    int docCount = 0;
    int listError = AISDK_NO_ERROR;
    char **docs = kb_document_list(m_session, kbId.toUtf8().constData(), &docCount, &listError);
    if (docs) {
        for (int i = 0; i < docCount; i++) {
            if (strcmp(docs[i], docId.toUtf8().constData()) == 0) {
                // Document already exists, skip adding
                qDebug() << "Document already exists, skip:" << docId;
                for (int j = 0; j < docCount; j++) {
                    kb_free_string(docs[j]);
                }
                free(docs);
                return true;
            }
            kb_free_string(docs[i]);
        }
        free(docs);
    }

    int error = kb_document_add(m_session, kbId.toUtf8().constData(),
                                 docPath.toUtf8().constData(),
                                 docId.toUtf8().constData(), nullptr);
    if (error != AISDK_NO_ERROR) {
        m_lastError = error;
        qDebug()<< "Failed to add document:" << errorMessageInternal(error);
        return false;
    }
    return true;
#else
    Q_UNUSED(spaceId);
    Q_UNUSED(docPath);
    Q_UNUSED(docId);
    return false;
#endif
}

bool SemanticKBManager::deleteDocumentFromSpace(const QString &spaceId, const QString &docId)
{
#ifdef KY_SDK_ADVANCED_AI
    if (!initSession()) {
        return false;
    }

    QString kbId = getKBIdForSpace(spaceId);
    if (kbId.isEmpty()) {
        return false;
    }

    int error = kb_document_delete(m_session, kbId.toUtf8().constData(),
                                    docId.toUtf8().constData());
    if (error != AISDK_NO_ERROR) {
        m_lastError = error;
        qWarning() << "Failed to delete document:" << errorMessageInternal(error);
        return false;
    }
    return true;
#else
    Q_UNUSED(spaceId);
    Q_UNUSED(docId);
    return false;
#endif
}

bool SemanticKBManager::addDocumentsToSpace(const QString &spaceId, const QStringList &docPaths,
                                            const QStringList &docIds, QList<int> *errorCodes)
{
#ifdef KY_SDK_ADVANCED_AI
    if (!initSession()) {
        return false;
    }

    if (docPaths.count() != docIds.count() || docPaths.isEmpty()) {
        m_lastError = -1;
        qWarning() << "Invalid document paths or ids count";
        return false;
    }

    QString kbId = getKBIdForSpace(spaceId);
    if (kbId.isEmpty()) {
        qWarning() << "No KB for space:" << spaceId;
        return false;
    }

    // First check which documents already exist
    int existingCount = 0;
    int listError = AISDK_NO_ERROR;
    char **existingDocs = kb_document_list(m_session, kbId.toUtf8().constData(), &existingCount, &listError);
    QSet<QString> existingDocIds;
    if (existingDocs) {
        for (int i = 0; i < existingCount; i++) {
            existingDocIds.insert(QString(existingDocs[i]));
            kb_free_string(existingDocs[i]);
        }
        free(existingDocs);
    }

    // Filter out already existing documents
    QStringList pathsToAdd;
    QStringList idsToAdd;
    for (int i = 0; i < docPaths.count(); i++) {
        if (existingDocIds.contains(docIds[i])) {
            qDebug() << "Document already exists, skip:" << docIds[i];
            if (errorCodes) {
                errorCodes->append(AISDK_NO_ERROR);  // Treat as success
            }
        } else {
            pathsToAdd.append(docPaths[i]);
            idsToAdd.append(docIds[i]);
        }
    }

    // If no documents to add, return early
    if (pathsToAdd.isEmpty()) {
        return true;
    }

    int count = pathsToAdd.count();
    QVector<const char*> paths(count);
    QVector<const char*> ids(count);
    QVector<char*> pathsCopy(count);
    QVector<char*> idsCopy(count);

    for (int i = 0; i < count; ++i) {
        pathsCopy[i] = strdup(pathsToAdd[i].toUtf8().constData());
        idsCopy[i] = strdup(idsToAdd[i].toUtf8().constData());
        paths[i] = pathsCopy[i];
        ids[i] = idsCopy[i];
    }

    int *errors = nullptr;
    int error = kb_document_add_multi(m_session, kbId.toUtf8().constData(),
                                       paths.data(), ids.data(), count, nullptr, &errors);

    for (int i = 0; i < count; ++i) {
        free(pathsCopy[i]);
        free(idsCopy[i]);
    }

    if (errorCodes && errors) {
        for (int i = 0; i < count; ++i) {
            errorCodes->append(errors[i]);
        }
    }

    if (errors) {
        free(errors);
    }

    if (error != AISDK_NO_ERROR) {
        m_lastError = error;
        qWarning() << "Failed to add documents:" << errorMessageInternal(error);
        return false;
    }

    return true;
#else
    Q_UNUSED(spaceId);
    Q_UNUSED(docPaths);
    Q_UNUSED(docIds);
    Q_UNUSED(errorCodes);
    return false;
#endif
}

bool SemanticKBManager::deleteDocumentsFromSpace(const QString &spaceId,
                                                   const QStringList &docIds, QList<int> *errorCodes)
{
#ifdef KY_SDK_ADVANCED_AI
    if (!initSession()) {
        return false;
    }

    if (docIds.isEmpty()) {
        return true;
    }

    QString kbId = getKBIdForSpace(spaceId);
    if (kbId.isEmpty()) {
        return false;
    }

    int count = docIds.count();
    QVector<const char*> ids(count);
    QVector<char*> idsCopy(count);

    for (int i = 0; i < count; ++i) {
        idsCopy[i] = strdup(docIds[i].toUtf8().constData());
        ids[i] = idsCopy[i];
    }

    int *errors = nullptr;
    int error = kb_document_delete_multi(m_session, kbId.toUtf8().constData(),
                                          ids.data(), count, &errors);

    for (int i = 0; i < count; ++i) {
        free(idsCopy[i]);
    }

    if (errorCodes && errors) {
        for (int i = 0; i < count; ++i) {
            errorCodes->append(errors[i]);
        }
    }

    if (errors) {
        free(errors);
    }

    if (error != AISDK_NO_ERROR) {
        m_lastError = error;
        qWarning() << "Failed to delete documents:" << errorMessageInternal(error);
        return false;
    }

    return true;
#else
    Q_UNUSED(spaceId);
    Q_UNUSED(docIds);
    Q_UNUSED(errorCodes);
    return false;
#endif
}

bool SemanticKBManager::deleteAllDocumentsInSpace(const QString &spaceId)
{
#ifdef KY_SDK_ADVANCED_AI
    if (!initSession()) {
        return false;
    }

    QString kbId = getKBIdForSpace(spaceId);
    if (kbId.isEmpty()) {
        qWarning() << "No KB for space:" << spaceId;
        return false;
    }

    int docCount = 0;
    int error = AISDK_NO_ERROR;
    char **docs = kb_document_list(m_session, kbId.toUtf8().constData(),
                                    &docCount, &error);
    if (error != AISDK_NO_ERROR || !docs) {
        m_lastError = error;
        return false;
    }

    bool allSuccess = true;
    for (int i = 0; i < docCount; ++i) {
        error = kb_document_delete(m_session, kbId.toUtf8().constData(), docs[i]);
        if (error != AISDK_NO_ERROR) {
            qWarning() << "Failed to delete document:" << docs[i] << errorMessageInternal(error);
            allSuccess = false;
        }
        kb_free_string(docs[i]);
    }
    free(docs);

    return allSuccess;
#else
    Q_UNUSED(spaceId);
    return false;
#endif
}

QStringList SemanticKBManager::listDocumentsInSpace(const QString &spaceId)
{
    QStringList result;
#ifdef KY_SDK_ADVANCED_AI
    if (!initSession()) {
        return result;
    }

    QString kbId = getKBIdForSpace(spaceId);
    if (kbId.isEmpty()) {
        return result;
    }

    int docCount = 0;
    int error = AISDK_NO_ERROR;
    char **docs = kb_document_list(m_session, kbId.toUtf8().constData(),
                                    &docCount, &error);
    if (error != AISDK_NO_ERROR || !docs) {
        m_lastError = error;
        return result;
    }

    for (int i = 0; i < docCount; ++i) {
        result.append(docs[i]);
        kb_free_string(docs[i]);
    }
    free(docs);

    return result;
#else
    Q_UNUSED(spaceId);
    return result;
#endif
}

bool SemanticKBManager::deleteFileFromAllSpaces(const QString &filePath, const QString &fileDocId)
{
#ifdef KY_SDK_ADVANCED_AI
    if (!initSession()) {
        return false;
    }

    QStringList allSpaceIds = getAllManagedSpaceIds();
    for (const QString &spaceId : allSpaceIds) {
        QString kbId = getKBIdForSpace(spaceId);
        if (kbId.isEmpty()) {
            continue;
        }

        QStringList docsInKB = listDocumentsInSpace(spaceId);
        if (docsInKB.contains(fileDocId)) {
            deleteDocumentFromSpace(spaceId, fileDocId);
        }
    }
    return true;
#else
    Q_UNUSED(filePath);
    Q_UNUSED(fileDocId);
    return false;
#endif
}

int SemanticKBManager::getDocumentStatus(const QString &spacename, const QString &spaceId, const QString &docId, int *status)
{
#ifdef KY_SDK_ADVANCED_AI
    if (!initSession()) {
        return -1;
    }

    QString kbId = getKBIdForSpace(spaceId);
    if (kbId.isEmpty()) {
        return -1;
    }

    int docStatus = 0;
    int error = AISDK_NO_ERROR;

    QString kbName = generateKBName(spacename, spaceId);
    QString kbDesc = QString("%1_knowledgebase").arg(spacename);

    // Create new KB
    KnowledgeBaseInfo *kbInfo = kb_knowledgebase_info_new("", "");
    kb_knowledgebase_info_set_id(kbInfo, kbId.toUtf8().constData());

    int progress = kb_document_get_processing_progress(m_session, kbInfo,
                                                       docId.toUtf8().constData(),
                                                       &docStatus, &error);

    kb_knowledgebase_info_free(kbInfo);

//    if (progress < 0) {
//        m_lastError = error;
//        return -1;
//    }

    if (status) {
        *status = docStatus;
    }
    if (docStatus == KB_DOC_STATUS_ERROR || error != AISDK_NO_ERROR) {
        qDebug()<<("文档处理失败：%s\n", kb_get_error_message(error));
        return 100;
    }
    return progress;
#else
    Q_UNUSED(spacename);
    Q_UNUSED(spaceId);
    Q_UNUSED(docId);
    Q_UNUSED(status);
    return -1;
#endif
}

QString SemanticKBManager::chatWithSpace(const QString &spaceId, const QString &question, const QString &model)
{
#ifdef KY_SDK_ADVANCED_AI
    if (!initSession()) {
        return QString();
    }

    QString kbId = getKBIdForSpace(spaceId);
    if (kbId.isEmpty()) {
        qWarning() << "ChatWithSpace: kbId is empty for spaceId:" << spaceId;
        return QString();
    }

    KBChatConfig *config = kb_chat_config_new();
    if (!config) {
        m_lastError = -1;
        qWarning() << "ChatWithSpace: Failed to create chat config";
        return QString();
    }

    kb_chat_config_set_new_chat(config, true);
    if (!model.isEmpty()) {
        kb_chat_config_set_model(config, model.toUtf8().constData());
    }

    QString result;
    bool finished = false;

    // Callback functor for capturing result, model info, reasoning, sources, references
    struct ChatCallbackData {
        QString *result;
        QString modelInfo;
        QString reasoningContent;
        QStringList sourceInfos;  // Store source document info
        QStringList referenceInfos;  // Store reference info
        bool *finished;
    };

    ChatCallbackData callbackData = {&result, QString(), QString(), QStringList(), QStringList(), &finished};

    int error = kb_chat_stream(m_session, nullptr, kbId.toUtf8().constData(),
                                question.toUtf8().constData(), nullptr, config,
                                [](const KBChatResponse *response, void *userData) -> int {
        if (!response) return 1;

        auto *data = static_cast<ChatCallbackData *>(userData);
        if (!data) return 1;

        int errorCode = kb_chat_response_get_error_code(response);
        if (errorCode != AISDK_NO_ERROR) {
            qWarning() << "Chat error:" << errorCode;
            *(data->finished) = true;
            return 1;
        }

        const KBChatResult *chatResult = kb_chat_response_get_result(response);
        if (chatResult) {
            // Display model info
            const char *modelStr = kb_chat_result_get_model(chatResult);
            if (modelStr && strlen(modelStr) > 0) {
                data->modelInfo = QString::fromUtf8(modelStr);
                qDebug() << "[Model]" << data->modelInfo;
            }

            // Display reasoning process
            const char *reasoning = kb_chat_result_get_reasoning_content(chatResult);
            if (reasoning && strlen(reasoning) > 0) {
                data->reasoningContent = QString::fromUtf8(reasoning);
                qDebug() << "[Reasoning]" << data->reasoningContent;
            }

            // Stream response text
            const char *text = kb_chat_result_get_response(chatResult);
            if (text && strlen(text) > 0) {
                QString chunk = QString::fromUtf8(text);
                *(data->result) += chunk;
                qDebug() << "[Response chunk]" << chunk;
            }

            // Check if finished
            if (kb_chat_result_is_finished(chatResult)) {
                qDebug() << "Chat finished";

                // Display source documents (with content and confidence)
                int sourceCount = 0;
                const KBChatSource **sources = kb_chat_result_get_source(chatResult, &sourceCount);
                if (sources && sourceCount > 0) {
                    qDebug() << "Source documents (" << sourceCount << "):";
                    for (int i = 0; i < sourceCount; i++) {
                        const char *fileId = kb_chat_source_get_file_id(sources[i]);
                        double confidence = kb_chat_source_get_confidence(sources[i]);
                        const char *content = kb_chat_source_get_content(sources[i]);

                        QString info = QString("[%1] File: %2, Confidence: %3%")
                                          .arg(i + 1)
                                          .arg(fileId ? QString::fromUtf8(fileId) : "N/A")
                                          .arg(confidence * 100);
                        data->sourceInfos.append(info);

                        if (content && strlen(content) > 0) {
                            qDebug() << "    Content:" << QString::fromUtf8(content).left(200) << "...";
                        }
                    }
                }

                // Display reference content (file number and chunk index)
                int referenceCount = 0;
                const KBChatReference **reference = kb_chat_result_get_reference(chatResult, &referenceCount);
                if (reference && referenceCount > 0) {
                    qDebug() << "References (" << referenceCount << "):";
                    for (int i = 0; i < referenceCount; i++) {
                        int fileNumber = kb_chat_reference_get_file_number(reference[i]);
                        int chunkIndex = kb_chat_reference_get_chunk_index(reference[i]);

                        QString info = QString("[%1] File#: %2, Chunk#: %3")
                                          .arg(i + 1)
                                          .arg(fileNumber)
                                          .arg(chunkIndex);
                        data->referenceInfos.append(info);
                    }
                }

                // Display reference source file IDs
                int refFileCount = 0;
                const char **refFileIds = kb_chat_result_get_reference_file_ids(chatResult, &refFileCount);
                if (refFileIds && refFileCount > 0) {
                    qDebug() << "Reference file IDs (" << refFileCount << "):";
                    for (int i = 0; i < refFileCount; i++) {
                        qDebug() << "   [" << i + 1 << "]" << (refFileIds[i] ? QString::fromUtf8(refFileIds[i]) : "N/A");
                    }
                }

                *(data->finished) = true;
                return 1;
            }
        }
        return 0;
    }, &callbackData);

    if (error != AISDK_NO_ERROR) {
        m_lastError = error;
        qWarning() << "Chat failed:" << errorMessageInternal(error);
        kb_chat_config_free(config);
        return QString();
    }

    // Run GMainLoop to process callbacks until finished or timeout (60 seconds)
    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    time_t startTime = time(nullptr);
    const int timeoutSeconds = 60;

    while (!finished && time(nullptr) - startTime < timeoutSeconds) {
        g_main_context_iteration(nullptr, FALSE);
        QThread::msleep(10);  // Sleep 10ms to avoid busy loop
    }

    g_main_loop_unref(loop);

    if (!finished) {
        qWarning() << "Chat timeout after" << timeoutSeconds << "seconds";
    }

    kb_chat_config_free(config);

    qDebug() << "############## Final result ########:" << result;
    return result;
#else
    Q_UNUSED(spaceId);
    Q_UNUSED(question);
    Q_UNUSED(model);
    return QString();
#endif
}

//QString SemanticKBManager::chatWithSpaceWithHistory(const QString &spaceId, const QString &question,
//                                                    void *history, const QString &model)
//{
//#ifdef KY_SDK_ADVANCED_AI
//    if (!initSession()) {
//        return QString();
//    }

//    QString kbId = getKBIdForSpace(spaceId);
//    if (kbId.isEmpty()) {
//        qWarning() << "ChatWithSpaceWithHistory: kbId is empty for spaceId:" << spaceId;
//        return QString();
//    }

//    if (!history) {
//        qWarning() << "ChatWithSpaceWithHistory: history is null";
//        return QString();
//    }

//    KBChatConfig *config = kb_chat_config_new();
//    if (!config) {
//        m_lastError = -1;
//        qWarning() << "ChatWithSpaceWithHistory: Failed to create chat config";
//        return QString();
//    }

//    kb_chat_config_set_new_chat(config, false);  // Use history for multi-turn conversation
//    if (!model.isEmpty()) {
//        kb_chat_config_set_model(config, model.toUtf8().constData());
//    }

//    QString result;
//    bool finished = false;

//    // Callback functor for capturing result, model info, reasoning, sources, references
//    struct ChatCallbackData {
//        QString *result;
//        QString modelInfo;
//        QString reasoningContent;
//        QStringList sourceInfos;
//        QStringList referenceInfos;
//        bool *finished;
//    };

//    ChatCallbackData callbackData = {&result, QString(), QString(), QStringList(), QStringList(), &finished};

//    int error = kb_chat_stream(m_session, nullptr, kbId.toUtf8().constData(),
//                                question.toUtf8().constData(), reinterpret_cast<KBChatHistory*>(history), config,
//                                [](const KBChatResponse *response, void *userData) -> int {
//        if (!response) return 1;

//        auto *data = static_cast<ChatCallbackData *>(userData);
//        if (!data) return 1;

//        int errorCode = kb_chat_response_get_error_code(response);
//        if (errorCode != AISDK_NO_ERROR) {
//            qWarning() << "Chat error:" << errorCode;
//            *(data->finished) = true;
//            return 1;
//        }

//        const KBChatResult *chatResult = kb_chat_response_get_result(response);
//        if (chatResult) {
//            // Display model info
//            const char *modelStr = kb_chat_result_get_model(chatResult);
//            if (modelStr && strlen(modelStr) > 0) {
//                data->modelInfo = QString::fromUtf8(modelStr);
//                qDebug() << "[Model]" << data->modelInfo;
//            }

//            // Display reasoning process
//            const char *reasoning = kb_chat_result_get_reasoning_content(chatResult);
//            if (reasoning && strlen(reasoning) > 0) {
//                data->reasoningContent = QString::fromUtf8(reasoning);
//                qDebug() << "[Reasoning]" << data->reasoningContent;
//            }

//            // Stream response text
//            const char *text = kb_chat_result_get_response(chatResult);
//            if (text && strlen(text) > 0) {
//                QString chunk = QString::fromUtf8(text);
//                *(data->result) += chunk;
//                qDebug() << "[Response chunk]" << chunk;
//            }

//            // Check if finished
//            if (kb_chat_result_is_finished(chatResult)) {
//                qDebug() << "Chat finished";

//                // Display source documents (with content and confidence)
//                int sourceCount = 0;
//                const KBChatSource **sources = kb_chat_result_get_source(chatResult, &sourceCount);
//                if (sources && sourceCount > 0) {
//                    qDebug() << "Source documents (" << sourceCount << "):";
//                    for (int i = 0; i < sourceCount; i++) {
//                        const char *fileId = kb_chat_source_get_file_id(sources[i]);
//                        double confidence = kb_chat_source_get_confidence(sources[i]);
//                        const char *content = kb_chat_source_get_content(sources[i]);

//                        QString info = QString("[%1] File: %2, Confidence: %3%")
//                                          .arg(i + 1)
//                                          .arg(fileId ? QString::fromUtf8(fileId) : "N/A")
//                                          .arg(confidence * 100);
//                        data->sourceInfos.append(info);

//                        if (content && strlen(content) > 0) {
//                            qDebug() << "    Content:" << QString::fromUtf8(content).left(200) << "...";
//                        }
//                    }
//                }

//                // Display reference content (file number and chunk index)
//                int referenceCount = 0;
//                const KBChatReference **reference = kb_chat_result_get_reference(chatResult, &referenceCount);
//                if (reference && referenceCount > 0) {
//                    qDebug() << "References (" << referenceCount << "):";
//                    for (int i = 0; i < referenceCount; i++) {
//                        int fileNumber = kb_chat_reference_get_file_number(reference[i]);
//                        int chunkIndex = kb_chat_reference_get_chunk_index(reference[i]);

//                        QString info = QString("[%1] File#: %2, Chunk#: %3")
//                                          .arg(i + 1)
//                                          .arg(fileNumber)
//                                          .arg(chunkIndex);
//                        data->referenceInfos.append(info);
//                    }
//                }

//                // Display reference source file IDs
//                int refFileCount = 0;
//                const char **refFileIds = kb_chat_result_get_reference_file_ids(chatResult, &refFileCount);
//                if (refFileIds && refFileCount > 0) {
//                    qDebug() << "Reference file IDs (" << refFileCount << "):";
//                    for (int i = 0; i < refFileCount; i++) {
//                        qDebug() << "   [" << i + 1 << "]" << (refFileIds[i] ? QString::fromUtf8(refFileIds[i]) : "N/A");
//                    }
//                }

//                *(data->finished) = true;
//                return 1;
//            }
//        }
//        return 0;
//    }, &callbackData);

//    if (error != AISDK_NO_ERROR) {
//        m_lastError = error;
//        qWarning() << "Chat failed:" << errorMessageInternal(error);
//        kb_chat_config_free(config);
//        return QString();
//    }

//    // Run GMainLoop to process callbacks until finished or timeout (60 seconds)
//    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
//    time_t startTime = time(nullptr);
//    const int timeoutSeconds = 60;

//    while (!finished && time(nullptr) - startTime < timeoutSeconds) {
//        g_main_context_iteration(nullptr, FALSE);
//        QThread::msleep(10);  // Sleep 10ms to avoid busy loop
//    }

//    g_main_loop_unref(loop);

//    if (!finished) {
//        qWarning() << "Chat timeout after" << timeoutSeconds << "seconds";
//    }

//    kb_chat_config_free(config);

//    qDebug() << "############## Final result ########:" << result;
//    return result;
//#else
//    Q_UNUSED(spaceId);
//    Q_UNUSED(question);
//    Q_UNUSED(history);
//    Q_UNUSED(model);
//    return QString();
//#endif
//}

void SemanticKBManager::removeSpace(const QString &spaceId)
{
    deleteKBForSpace(spaceId);
}

QStringList SemanticKBManager::getAllManagedSpaceIds()
{
    return m_spaceToKB.keys();
}

bool SemanticKBManager::isSpaceConditionChanged(const QString &spaceId)
{
    return m_spaceConditionChanged.value(spaceId, false);
}

void SemanticKBManager::setSpaceConditionChanged(const QString &spaceId, bool changed)
{
    m_spaceConditionChanged[spaceId] = changed;
    saveKBMapping();
}

QStringList SemanticKBManager::retrievalInSpace(const QString &spaceId, const QString &query, int topK)
{
    QStringList results;
#ifdef KY_SDK_ADVANCED_AI
    if (!initSession()) {
        return results;
    }

    QString kbId = getKBIdForSpace(spaceId);
    if (kbId.isEmpty()) {
        return results;
    }

    KBRetrievalConfig *config = kb_retrieval_config_new();
    if (!config) {
        m_lastError = -1;
        return results;
    }

    // 设置检索参数
    kb_retrieval_config_set_top_k(config, topK);
    kb_retrieval_config_set_retrieval_method(config, KB_RETRIEVAL_METHOD_HYBRID);
    kb_retrieval_config_set_rerank_method(config, KB_RERANK_METHOD_WEIGHT);

    // 检索结果上下文
    struct RetrievalContext {
        bool completed;
        QStringList results;
    };
    RetrievalContext ctx;
    ctx.completed = false;
    ctx.results = QStringList();

    int error = kb_retrieval(m_session, kbId.toUtf8().constData(), nullptr,
                              query.toUtf8().constData(), config,
                              [](const KBRetrievalResponse *response, void *userData) {
        auto *ctx = static_cast<RetrievalContext *>(userData);
        if (!response) {
            ctx->completed = true;
            return;
        }

        int errorCode = kb_retrieval_response_get_error_code(response);
        if (errorCode != AISDK_NO_ERROR) {
            ctx->completed = true;
            return;
        }

        int sourceCount = kb_retrieval_response_get_source_count(response);
        for (int i = 0; i < sourceCount; i++) {
            const KBChatSource *source = kb_retrieval_response_get_source(response, i);
            if (source) {
                const char *content = kb_chat_source_get_content(source);
                if (content && strlen(content) > 0) {
                    ctx->results.append(QString::fromUtf8(content));
                }
            }
        }

        ctx->completed = true;
    }, &ctx);

    kb_retrieval_config_free(config);

    if (error != AISDK_NO_ERROR) {
        m_lastError = error;
        qWarning() << "Retrieval failed:" << errorMessageInternal(error);
        return results;
    }

    // 等待检索完成（简单轮询）
    int waitCount = 0;
    while (!ctx.completed && waitCount < 100) {
        QThread::msleep(100);
        waitCount++;
    }

    return ctx.results;
#else
    Q_UNUSED(spaceId);
    Q_UNUSED(query);
    Q_UNUSED(topK);
    return results;
#endif
}
