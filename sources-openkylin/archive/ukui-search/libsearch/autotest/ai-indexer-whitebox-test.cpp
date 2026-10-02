#include <QtTest>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <cstring>

#include <kylin-ai/business/data-management/datamanagement.h>

#include "../index/ai-indexer.h"

struct _DataManagementSession {
    int id;
};

namespace {
int g_createCalls = 0;
int g_destroyCalls = 0;
int g_destroyResultCalls = 0;
bool g_featureSupported = true;
FeatureStatus g_featureStatus = AVAILABLE;
bool g_createSessionFails = false;
int g_checkFileFormatSessionErrors = 0;
int g_addTextSessionErrors = 0;
int g_addImageSessionErrors = 0;
int g_deleteSessionErrors = 0;
int g_updateSessionErrors = 0;
int g_getAllSessionErrors = 0;
QString g_lastPayload;

DataManagementStatus status(bool success = true, int errorCode = 0, const char *message = "")
{
    DataManagementStatus status{};
    status.success = success;
    status.errorCode = errorCode;
    std::strncpy(status.errorMsg, message, sizeof(status.errorMsg) - 1);
    return status;
}

char *copyResult(const QByteArray &bytes)
{
    auto *result = static_cast<char *>(std::malloc(static_cast<size_t>(bytes.size()) + 1));
    std::memcpy(result, bytes.constData(), static_cast<size_t>(bytes.size()));
    result[bytes.size()] = '\0';
    return result;
}

void resetStubState()
{
    g_createCalls = 0;
    g_destroyCalls = 0;
    g_destroyResultCalls = 0;
    g_featureSupported = true;
    g_featureStatus = AVAILABLE;
    g_createSessionFails = false;
    g_checkFileFormatSessionErrors = 0;
    g_addTextSessionErrors = 0;
    g_addImageSessionErrors = 0;
    g_deleteSessionErrors = 0;
    g_updateSessionErrors = 0;
    g_getAllSessionErrors = 0;
    g_lastPayload.clear();
}
}

extern "C" {
bool data_management_is_feature_supported()
{
    return g_featureSupported;
}

DataManagementSession *data_management_create_session()
{
    ++g_createCalls;
    if (g_createSessionFails) {
        return nullptr;
    }
    return new DataManagementSession{g_createCalls};
}

void data_management_destroy_session(DataManagementSession *session)
{
    ++g_destroyCalls;
    delete session;
}

void data_management_destroy_result(char *result)
{
    ++g_destroyResultCalls;
    std::free(result);
}

DataManagementStatus data_management_get_feature_status(DataManagementSession *, FeatureStatus *featureStatus)
{
    if (featureStatus) {
        *featureStatus = g_featureStatus;
    }
    return status(true);
}

DataManagementStatus data_management_check_file_format(DataManagementSession *, const char *, char **checkResult)
{
    if (g_checkFileFormatSessionErrors-- > 0) {
        *checkResult = copyResult("{}");
        return status(false, DM_SESSION_ERROR, "session");
    }
    *checkResult = copyResult(R"({"supported":true,"format":"txt"})");
    return status(true);
}

DataManagementStatus data_management_add_text_files(DataManagementSession *, const char *fileinfos)
{
    g_lastPayload = QString::fromUtf8(fileinfos);
    if (g_addTextSessionErrors-- > 0) {
        return status(false, DM_SESSION_ERROR, "session");
    }
    return status(true);
}

DataManagementStatus data_management_add_image_files(DataManagementSession *, const char *fileinfos)
{
    g_lastPayload = QString::fromUtf8(fileinfos);
    if (g_addImageSessionErrors-- > 0) {
        return status(false, DM_SESSION_ERROR, "session");
    }
    return status(true);
}

DataManagementStatus data_management_delete_files(DataManagementSession *, const char *fileinfos)
{
    g_lastPayload = QString::fromUtf8(fileinfos);
    if (g_deleteSessionErrors-- > 0) {
        return status(false, DM_SESSION_ERROR, "session");
    }
    return status(true);
}

DataManagementStatus data_management_update_files_content(DataManagementSession *, const char *fileinfos)
{
    g_lastPayload = QString::fromUtf8(fileinfos);
    if (g_updateSessionErrors-- > 0) {
        return status(false, DM_SESSION_ERROR, "session");
    }
    return status(true);
}

DataManagementStatus data_management_get_all_fileinfos(DataManagementSession *, char **fileinfosResult)
{
    if (g_getAllSessionErrors-- > 0) {
        *fileinfosResult = copyResult("[]");
        return status(false, DM_SESSION_ERROR, "session");
    }
    *fileinfosResult = copyResult(R"([{"filepath":"/tmp/a.txt","timestamp":42},{"filepath":"/tmp/b.png","timestamp":77}])");
    return status(true);
}
}

class AiIndexerWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void invalidSessionRejectsAllOperations();
    void createSessionAndReadyStateUseSdkStatus();
    void fileFormatAndIndexedFilesParseSdkJson();
    void mutatingOperationsRetryAfterSessionError();
    void recreateFailureReturnsFalseAfterSessionError();
};

void AiIndexerWhiteboxTest::init()
{
    resetStubState();
}

void AiIndexerWhiteboxTest::invalidSessionRejectsAllOperations()
{
    UkuiSearch::AiIndexer indexer;
    QString type;
    QJsonObject indexed;

    QVERIFY(!indexer.checkFileSupported("/tmp/a.txt", type));
    QVERIFY(!indexer.addTextFileIndex({}));
    QVERIFY(!indexer.addImageFileIndex({}));
    QVERIFY(!indexer.deleteFileIndex({"/tmp/a.txt"}));
    QVERIFY(!indexer.getAllIndexedFiles(indexed));
    QVERIFY(!indexer.update({}));
}

void AiIndexerWhiteboxTest::createSessionAndReadyStateUseSdkStatus()
{
    QVERIFY(UkuiSearch::AiIndexer::aiIsReady());
    QCOMPARE(g_createCalls, 1);

    g_featureStatus = NOT_INSTALLED;
    QVERIFY(UkuiSearch::AiIndexer::aiIsReady());

    g_featureSupported = false;
    QVERIFY(UkuiSearch::AiIndexer::aiIsReady());
}

void AiIndexerWhiteboxTest::fileFormatAndIndexedFilesParseSdkJson()
{
    UkuiSearch::AiIndexer indexer;
    QVERIFY(UkuiSearch::AiIndexer::createSession());

    QString type;
    QVERIFY(indexer.checkFileSupported("/tmp/a.txt", type));
    QCOMPARE(type, QString("txt"));

    QJsonObject indexed;
    QVERIFY(indexer.getAllIndexedFiles(indexed));
    QCOMPARE(indexed.value("/tmp/a.txt").toInt(), 42);
    QCOMPARE(indexed.value("/tmp/b.png").toInt(), 77);
    QVERIFY(g_destroyResultCalls >= 2);
}

void AiIndexerWhiteboxTest::mutatingOperationsRetryAfterSessionError()
{
    UkuiSearch::AiIndexer indexer;
    QVERIFY(indexer.reCreateSession());
    g_addTextSessionErrors = 1;
    g_addImageSessionErrors = 1;
    g_deleteSessionErrors = 1;
    g_updateSessionErrors = 1;

    const QJsonArray payload{QJsonObject{{"filepath", "/tmp/a.txt"}, {"fileformat", "txt"}}};
    QVERIFY(indexer.addTextFileIndex(payload));
    QVERIFY(g_lastPayload.contains("/tmp/a.txt"));
    QVERIFY(indexer.addImageFileIndex(payload));
    QVERIFY(indexer.deleteFileIndex({"/tmp/a.txt", "/tmp/b.txt"}));
    QVERIFY(g_lastPayload.contains("/tmp/b.txt"));
    QVERIFY(indexer.update(payload));
    QVERIFY(g_createCalls >= 4);
}

void AiIndexerWhiteboxTest::recreateFailureReturnsFalseAfterSessionError()
{
    UkuiSearch::AiIndexer indexer;
    QVERIFY(indexer.reCreateSession());
    g_addTextSessionErrors = 1;
    g_createSessionFails = true;

    QVERIFY(!indexer.addTextFileIndex({QJsonObject{{"filepath", "/tmp/a.txt"}}}));
}

QTEST_MAIN(AiIndexerWhiteboxTest)
#include "ai-indexer-whitebox-test.moc"
