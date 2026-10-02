#ifndef KBCOMMON_H
#define KBCOMMON_H

#include <string>
#include <vector>

namespace kyai::advancedai::knowledgebase::core {

// 错误码枚举
enum class KBErrorCode {
    SUCCESS = 0,  // 成功

    // 知识库
    KB_ID_INVALID = 1,  // 无效的知识库ID
    KB_CREATE_FAILED,   // 知识库创建失败
    KB_ALREADY_EXISTS,  // 知识库已存在
    KB_NOT_FOUND,       // 知识库不存在
    KB_ACCESS_DENIED,   // 知识库访问被拒
    KB_CORRUPTED,       // 知识库损坏
    KB_DATA_EMPTY,      // 知识库内无数据

    // 文档
    DOC_NOT_FOUND = 101,     // 文档未找到
    DOC_UPLOAD_FAILED,       // 文档上传失败
    DOC_PROCESSING_FAILED,   // 文档处理失败
    DOC_DELETE_FAILED,       // 文档删除失败
    DOC_FORMAT_UNSUPPORTED,  // 文档格式不支持
    DOC_EMPTY,               // 文件内容为空
    DOC_ALREADY_EXISTS,      // 文件已存在
    DOC_FILEID_INVALID,      // 文件ID无效
    DOC_CONTENT_TOO_LONG,    // 文件字符数超长

    // 检索
    RETRIEVAL_FAILED = 201,           // 检索失败
    RETRIEVAL_NO_RESULTS,             // 未检索到结果
    RETRIEVAL_RERANK_FAILED,          // 检索结果重排失败
    RETRIEVAL_FILEID_FILTER_INVALID,  // 检索指定的文件ID无效

    // 聊天
    CHAT_MODEL_NOT_EXIST = 301,         // 聊天模型未安装
    CHAT_MODEL_UNAVAILABLE,             // 聊天模型不可用
    CHAT_MODEL_NET_ERROR,               // 云端聊天模型无法连接
    CHAT_MODEL_AUTHENTICATION_FAILURE,  // 聊天模型鉴权失败
    CHAT_MODEL_RUNTIME_ERROR,           // 聊天模型服务异常
    CHAT_MODEL_TOO_MANY_REQUESTS,       // 聊天模型请求过多
    CHAT_MODEL_SERVICE_ERROR,           // 聊天模型服务错误
    CHAT_MODEL_SERVICE_TIMEOUT,         // 聊天模型服务超时
    CHAT_MODEL_BAD_REQUEST,             // 聊天模型请求错误
    CHAT_MODEL_RUN_FAILED,              // 聊天模型运行失败
    CHAT_MODEL_RUN_TIME_OUT,            // 聊天模型运行超时
    CHAT_MODEL_INVALID_SESSION,         // 聊天模型无效会话
    CHAT_SSL_CERTIFICATE_ERROR,  // 系统时间异常或数字证书过期
    CHAT_FREE_QUOTA_EXHAUSTED,   // 免费账号 Token 额度已耗尽

    // 数据库
    VECTOR_DB_NOT_FOUND = 401,    // 向量数据库未找到
    VECTOR_DB_CREATE_FAILED,      // 向量数据库创建失败
    VECTOR_DB_DELETE_FAILED,      // 向量数据库删除失败
    VECTOR_DB_DATAUPLOAD_FAILED,  // 向量数据库内容上传失败
    VECTOR_DB_DATADELETE_FAILED,  // 向量数据库内容删除失败
    VECTOR_RETRIEVAL_FAILED,      // 向量数据库检索失败

    TEXT_DB_NOT_FOUND = 501,    // 文本检索数据库未找到
    TEXT_DB_CREATE_FAILED,      // 文本检索数据库创建失败
    TEXT_DB_DELETE_FAILED,      // 文本检索数据库删除失败
    TEXT_DB_DATAUPLOAD_FAILED,  // 文本检索数据库内容上传失败
    TEXT_DB_DATADELETE_FAILED,  // 文本检索数据库内容删除失败
    TEXT_RETRIEVAL_FAILED,      // 文本检索数据库检索失败
    TEXT_DB_NO_ACCESS,          // 文本检索数据库无访问权限

    // 通用错误
    INVALID_PARAMETER = 901,  // 无效参数
    OPERATION_TIMEOUT,        // 操作超时
    INVALID_SESSION_ID,       // 无效会话ID

    UNKNOWN_ERROR = 1000  // 未知错误
};
// 知识库类型
enum class KBType {
    RAG = 0,  // 检索增强生成 (Retrieval-Augmented Generation)
    FAQ,      // 问答知识库 (Frequently Asked Questions)
};
// 索引类型
enum class IndexType {
    FLAT = 0,  // 平面索引（基础索引类型）
    HNSW,  // 分层可导航小世界图（高效近似最近邻搜索）- 当前不支持
    IVFFLAT  // 倒排文件索引（大规模数据集优化）- 当前不支持
};
// 文档处理状态
enum class DocumentUploadStatus {
    PENDING = 0,  // 等待处理
    PARSING,      // 文档解析中
    EMBEDDING,    // 向量嵌入中
    INDEXING,     // 索引建立中
    COMPLETED,    // 处理完成
    FAILED        // 处理失败
};
// 检索方法
enum class RetrievalMethod {
    SEMANTIC = 0,  // 语义检索
    TEXT,          // 文本检索
    KEYWORD,       // 关键词检索
    HYBRID         // 混合检索
};
// 重排方法
enum class RerankMethod {
    RRF = 0,   // 倒数排名融合 (Reciprocal Rank Fusion)
    WEIGHT = 1  // 权重融合 (Weighted)
};
// 请求处理状态
enum class RequestProcessingStatus {
    PENDING = 0,          // 请求等待中
    QUERY_EXPANDING,      // 查询扩展中
    RETRIEVING,           // 内容检索中
    RESPONSE_GENERATING,  // 响应生成中
    COMPLETED,            // 处理完成
    FAILED                // 处理失败
};

}  // namespace kyai::advancedai::knowledgebase::kbcommon
#endif