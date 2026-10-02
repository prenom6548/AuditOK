from enum import IntEnum

class ErrorCode(IntEnum):
    SUCCESS = 0
    #0~99 解析错误
    FILE_FORMAT_UNSUPPORTED = 1
    FILE_NOT_FOUND = 2
    FILE_PARSER_ERROR = 3

    #100~199 分块错误

    #200~299 解析后的处理决策错误
    FILE_CHAR_COUNT_TOO_LONG = 200

    UNKNOWN_ERROR = 999