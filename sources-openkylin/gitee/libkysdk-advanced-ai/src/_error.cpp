#include "_error.h"

#include <map>

#include "core.h"

namespace kyai::advancedai::knowledgebase {

// Map need to fix
std::map<std::string, KBDocumentStatus> documentStatusMap{
    {"PENDING", KB_DOC_STATUS_PENDING},
    {"PARSING", KB_DOC_STATUS_PARSING},
    {"EMBEDDING", KB_DOC_STATUS_EMBEDDING},
    {"INDEXING", KB_DOC_STATUS_INDEXING},
    {"COMPLETED", KB_DOC_STATUS_COMPLETED},
    {"FAILED", KB_DOC_STATUS_ERROR}};

std::map<int, std::string> errorMessageMap{
    {AI_COMMON_SUCCESS, "Operation successful"},

    /* General errors (1-99) */
    {AI_COMMON_INVALID_PARAMETER,
     "Invalid parameter provided. Check that all required parameters are "
     "correctly formatted and within valid ranges."},
    {AI_COMMON_PERMISSION_DENIED,
     "Permission denied. Verify that your application has the necessary "
     "permissions to perform this operation."},
    {AISDK_SERVICE_ERROR,
     "Internal server error. This is an unexpected server-side issue. Please "
     "try again later or contact support."},
    {AI_COMMON_SERVICE_TIMEOUT,
     "Operation timed out. The operation took longer than expected. Check your "
     "connection and try again."},
    {AI_COMMON_SERVER_INITIALIZATION_FAILED,
     "Initialization failed. Ensure all required components are properly "
     "configured and available."},
    {AI_COMMON_SERVER_CONNECTION_FAILED,
     "Server connection failed. Check server connectivity and ensure the "
     "server is running and accessible."},
    /* Network errors (100-199) */
    {AI_COMMON_NET_ERROR,
     "Network error occurred. Check your internet connection and firewall "
     "settings."},
    {AI_COMMON_NETWORK_TIMEOUT,
     "Network operation timed out. Check network stability and try again."},
    {AI_COMMON_NETWORK_CONNECTION_FAILED,
     "Connection failed. Verify server address and network connectivity."},
    {AI_COMMON_AUTHENTICATION_FAILURE,
     "Authentication failed. Check your credentials and ensure they are "
     "correct and active."},

    /* Session errors (200-299) */
    {AISDK_INVALID_SESSION,
     "Session is invalid. Your session may have expired. Try "
     "re-authenticating."},
    {KB_ERROR_APP_ID_INVALID,
     "Application ID is invalid. Check that your application ID is correctly "
     "formatted and registered."},
    {KB_ERROR_PASSWORD_INVALID,
     "Password is invalid. Verify your password is correct and meets security "
     "requirements."},

    /* Knowledge base errors (300-399) */
    {KB_ERROR_KB_NOT_FOUND,
     "Knowledge base not found. Verify the knowledge base ID exists and is "
     "accessible to your application."},
    {KB_ERROR_KB_ACCESS_DENIED,
     "Access to knowledge base denied. Check your permissions and ensure you "
     "have access to this knowledge base."},
    {KB_ERROR_KB_CORRUPTED,
     "Knowledge base is corrupted. Contact support to restore or recreate the "
     "knowledge base."},
    {KB_ERROR_KB_ALREADY_EXISTS,
     "Knowledge base already exists. Choose a unique name or update the "
     "existing knowledge base instead."},
    {KB_ERROR_KB_EMPTY,
     "Knowledge base is empty. Please upload relevant files."},

    /* Document errors (400-499) */
    {KB_ERROR_DOC_NOT_FOUND,
     "Document not found. Verify the document ID exists in the specified "
     "knowledge base."},
    {KB_ERROR_DOC_TOO_LARGE,
     "Document too large. Reduce document size or split into smaller documents "
     "within allowed limits."},
    {KB_ERROR_DOC_INVALID_FORMAT,
     "Unsupported document format. Convert to a supported format such as PDF, "
     "TXT, or DOCX."},
    {KB_ERROR_DOC_ACCESS_DENIED,
     "Access to document denied. Check permissions and ensure you have access "
     "to this document."},
    {KB_ERROR_DOC_CORRUPTED,
     "Document is corrupted. Verify document integrity and try uploading a "
     "valid copy."},
    {KB_ERROR_DOC_EMPTY,
     "Document is empty. Ensure the document contains content before "
     "uploading."},
    {KB_ERROR_DOC_ID_INVALID,
     "Document ID is invalid. Check that the document ID follows the required "
     "format and constraints."},
    {KB_ERROR_DOC_ID_NOT_FOUND,
     "Document ID not found. Verify the document ID exists in the knowledge "
     "base."},
    {KB_ERROR_DOC_ID_ALREADY_EXISTS,
     "Document ID already exists. Use a unique document ID or update the "
     "existing document instead."},
    {KB_ERROR_DOC_BATCH_PARTIAL_FAILED,
     "Document batch operation partially failed. Check individual document "
     "status and retry failed documents."},
    {KB_ERROR_DOC_BATCH_ALL_FAILED,
     "Document batch operation completely failed. Verify all documents are "
     "valid and try again."},
    {KB_ERROR_DOC_PARSE_FAILED,
     "Document parse failed. Check document format and content. "},

    /* Retrieval errors (500-599) */
    {KB_ERROR_RETRIEVAL_FAILED,
     "Retrieval operation failed. Check query parameters and try again."},
    {KB_ERROR_RETRIEVAL_NO_RESULTS,
     "No results found. Try rephrasing your query or check if the knowledge "
     "base contains relevant information."},
    {KB_ERROR_RETRIEVAL_INVALID_QUERY,
     "Invalid query. Ensure your query follows the required format and "
     "syntax."},
    {KB_ERROR_RETRIEVAL_TIMEOUT,
     "Retrieval timeout. The query took too long to process. Simplify your "
     "query or try again later."},
    {KB_ERROR_RETRIEVAL_FILEID_FILTER_INVALID,
     "Invalid file ID filter. Check the file ID filter syntax and ensure it "
     "matches the expected format."},

    /* Chat errors (600-699) */
    {AI_COMMON_MODEL_NOT_FOUND,
     "Chat model unavailable. The requested model may be offline or not "
     "supported. Try an alternative model."},
    {AI_COMMON_NLP_INPUT_INVALID,
     "Chat context too long. Reduce the conversation history or start a new "
     "chat session."},
    {AI_COMMON_TOO_MANY_REQUESTS,
     "Chat request limit exceeded. Wait for current requests to complete "
     "before sending new requests."},

    {AI_COMMON_SSL_CERTIFICATE_ERROR,
     "Chat request ssl certificate error. "
     "Check your system time or ssl certificate validation"},

    /* Configuration errors (700-799) */
    {KB_ERROR_CONFIG_INVALID,
     "Configuration is invalid. Check configuration values and ensure they "
     "follow the required format."},
    {KB_ERROR_CONFIG_MISSING,
     "Configuration is missing. Provide all required configuration parameters "
     "for the operation."}};

std::map<core::KBErrorCode, int> kbErrorMap{
    {core::KBErrorCode::SUCCESS, AI_COMMON_SUCCESS},

    {core::KBErrorCode::KB_ID_INVALID, AI_COMMON_INVALID_PARAMETER},
    {core::KBErrorCode::KB_CREATE_FAILED,
     AI_COMMON_SERVER_INITIALIZATION_FAILED},
    {core::KBErrorCode::KB_ALREADY_EXISTS, KB_ERROR_KB_ALREADY_EXISTS},
    {core::KBErrorCode::KB_NOT_FOUND, KB_ERROR_KB_NOT_FOUND},
    {core::KBErrorCode::KB_ACCESS_DENIED, KB_ERROR_KB_ACCESS_DENIED},
    {core::KBErrorCode::KB_CORRUPTED, KB_ERROR_KB_CORRUPTED},
    {core::KBErrorCode::KB_DATA_EMPTY, KB_ERROR_KB_EMPTY},

    {core::KBErrorCode::DOC_NOT_FOUND, KB_ERROR_DOC_NOT_FOUND},
    {core::KBErrorCode::DOC_UPLOAD_FAILED, KB_ERROR_DOC_INVALID_FORMAT},
    {core::KBErrorCode::DOC_PROCESSING_FAILED, KB_ERROR_DOC_PARSE_FAILED},
    {core::KBErrorCode::DOC_DELETE_FAILED, KB_ERROR_DOC_BATCH_ALL_FAILED},
    {core::KBErrorCode::DOC_FORMAT_UNSUPPORTED, KB_ERROR_DOC_INVALID_FORMAT},
    {core::KBErrorCode::DOC_EMPTY, KB_ERROR_DOC_EMPTY},
    {core::KBErrorCode::DOC_ALREADY_EXISTS, KB_ERROR_DOC_ID_ALREADY_EXISTS},
    {core::KBErrorCode::DOC_FILEID_INVALID, KB_ERROR_DOC_ID_INVALID},
    {core::KBErrorCode::DOC_CONTENT_TOO_LONG, KB_ERROR_DOC_TOO_LARGE},

    {core::KBErrorCode::RETRIEVAL_FAILED, KB_ERROR_RETRIEVAL_FAILED},
    {core::KBErrorCode::RETRIEVAL_NO_RESULTS, KB_ERROR_RETRIEVAL_NO_RESULTS},
    {core::KBErrorCode::RETRIEVAL_RERANK_FAILED, KB_ERROR_RETRIEVAL_FAILED},
    {core::KBErrorCode::RETRIEVAL_FILEID_FILTER_INVALID,
     KB_ERROR_RETRIEVAL_FILEID_FILTER_INVALID},

    {core::KBErrorCode::CHAT_MODEL_NOT_EXIST, AI_COMMON_MODEL_NOT_FOUND},
    {core::KBErrorCode::CHAT_MODEL_UNAVAILABLE, AI_COMMON_MODEL_NOT_FOUND},
    {core::KBErrorCode::CHAT_MODEL_NET_ERROR, AI_COMMON_NET_ERROR},
    {core::KBErrorCode::CHAT_MODEL_AUTHENTICATION_FAILURE,
     AI_COMMON_AUTHENTICATION_FAILURE},
    {core::KBErrorCode::CHAT_MODEL_RUNTIME_ERROR, AI_COMMON_RUNTIME_ERROR},
    {core::KBErrorCode::CHAT_MODEL_TOO_MANY_REQUESTS,
     AI_COMMON_TOO_MANY_REQUESTS},
    {core::KBErrorCode::CHAT_MODEL_SERVICE_ERROR, AI_COMMON_SERVICE_ERROR},
    {core::KBErrorCode::CHAT_MODEL_SERVICE_TIMEOUT, AI_COMMON_SERVICE_TIMEOUT},
    {core::KBErrorCode::CHAT_MODEL_BAD_REQUEST, AI_COMMON_INVALID_PARAMETER},
    {core::KBErrorCode::CHAT_MODEL_RUN_FAILED, AI_COMMON_MODEL_RUN_FAILED},
    {core::KBErrorCode::CHAT_MODEL_RUN_TIME_OUT, AI_COMMON_MODEL_RUN_TIME_OUT},
    {core::KBErrorCode::CHAT_MODEL_INVALID_SESSION, AI_COMMON_INVALID_SESSION},
    {core::KBErrorCode::CHAT_SSL_CERTIFICATE_ERROR,
     AI_COMMON_SSL_CERTIFICATE_ERROR},
    {core::KBErrorCode::CHAT_FREE_QUOTA_EXHAUSTED,
     AI_COMMON_FREE_ALLOCATED_QUOTA_EXCEEDED},

    {core::KBErrorCode::VECTOR_DB_NOT_FOUND, KB_ERROR_RETRIEVAL_NO_RESULTS},
    {core::KBErrorCode::VECTOR_DB_CREATE_FAILED, AI_COMMON_SERVICE_ERROR},
    {core::KBErrorCode::VECTOR_DB_DELETE_FAILED, AI_COMMON_SERVICE_ERROR},
    {core::KBErrorCode::VECTOR_DB_DATAUPLOAD_FAILED, AI_COMMON_SERVICE_ERROR},
    {core::KBErrorCode::VECTOR_DB_DATADELETE_FAILED, KB_ERROR_KB_CORRUPTED},
    {core::KBErrorCode::VECTOR_RETRIEVAL_FAILED, KB_ERROR_RETRIEVAL_FAILED},

    {core::KBErrorCode::TEXT_DB_NOT_FOUND, KB_ERROR_RETRIEVAL_NO_RESULTS},
    {core::KBErrorCode::TEXT_DB_CREATE_FAILED, AI_COMMON_SERVICE_ERROR},
    {core::KBErrorCode::TEXT_DB_DELETE_FAILED, AI_COMMON_SERVICE_ERROR},
    {core::KBErrorCode::TEXT_DB_DATAUPLOAD_FAILED, AI_COMMON_SERVICE_ERROR},
    {core::KBErrorCode::TEXT_DB_DATADELETE_FAILED, KB_ERROR_KB_CORRUPTED},
    {core::KBErrorCode::TEXT_RETRIEVAL_FAILED, KB_ERROR_RETRIEVAL_FAILED},
    {core::KBErrorCode::TEXT_DB_NO_ACCESS, KB_ERROR_KB_ACCESS_DENIED},

    {core::KBErrorCode::INVALID_PARAMETER, AI_COMMON_INVALID_PARAMETER},
    {core::KBErrorCode::OPERATION_TIMEOUT, AI_COMMON_SERVICE_TIMEOUT},
    {core::KBErrorCode::INVALID_SESSION_ID, AI_COMMON_SERVER_CONNECTION_FAILED},

    {core::KBErrorCode::UNKNOWN_ERROR, AI_COMMON_UNKNOWN_ERROR},
};

int getErrorCodeFromKnowledgebase(int code) {
    const auto errorCode = static_cast<core::KBErrorCode>(code);
    if (kbErrorMap.find(errorCode) != kbErrorMap.end()) {
        return kbErrorMap[errorCode];
    }
    return AI_COMMON_UNKNOWN_ERROR;
}

KBDocumentStatus getDocumentStatus(const std::string& status) {
    if (documentStatusMap.find(status) != documentStatusMap.end()) {
        return documentStatusMap[status];
    }
    return KB_DOC_STATUS_PENDING;
}

std::string getErrorMessageFromKnowledgebase(int code) {
    if (errorMessageMap.find(code) != errorMessageMap.end()) {
        return errorMessageMap[code];
    }
    return "Unknown error";
}

}  // namespace kyai::advancedai::knowledgebase

const char* kb_get_error_message(int error_code) {
    if (kyai::advancedai::knowledgebase::errorMessageMap.find(error_code) !=
        kyai::advancedai::knowledgebase::errorMessageMap.end()) {
        return kyai::advancedai::knowledgebase::errorMessageMap.at(error_code)
            .c_str();
    }
    return "Unknown error";
}