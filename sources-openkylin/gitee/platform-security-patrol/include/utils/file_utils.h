#ifndef UTILS_FILE_UTILS_H
#define UTILS_FILE_UTILS_H

#include "plat_sec_patrol.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 读取文件内容到字符串
 * 
 * @param file_path 文件路径
 * @param[out] content 文件内容（动态分配，需调用者释放）
 * @param[out] size 文件大小（可选）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t file_utils_read_file(const char *file_path, char **content, size_t *size);

/**
 * @brief 将字符串写入文件
 * 
 * @param file_path 文件路径
 * @param content 要写入的内容
 * @param size 内容大小（0 表示使用 strlen）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t file_utils_write_file(const char *file_path, const char *content, size_t size);

/**
 * @brief 检查文件是否存在
 * 
 * @param file_path 文件路径
 * @return 存在返回 true，否则返回 false
 */
bool file_utils_file_exists(const char *file_path);

/**
 * @brief 检查是否为目录
 * 
 * @param path 路径
 * @return 是目录返回 true，否则返回 false
 */
bool file_utils_is_directory(const char *path);

/**
 * @brief 检查是否为常规文件
 * 
 * @param path 路径
 * @return 是常规文件返回 true，否则返回 false
 */
bool file_utils_is_regular_file(const char *path);

/**
 * @brief 获取文件大小
 * 
 * @param file_path 文件路径
 * @return 文件大小（字节），失败返回 -1
 */
long file_utils_get_file_size(const char *file_path);

/**
 * @brief 创建目录（包括父目录）
 * 
 * @param path 目录路径
 * @param mode 权限模式（如 0755）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t file_utils_create_directory(const char *path, int mode);

/**
 * @brief 扫描目录中的所有文件
 * 
 * @param dir_path 目录路径
 * @param extension 文件扩展名过滤（NULL 表示所有文件）
 * @param[out] files 文件名数组（输出，需调用者释放）
 * @param[out] count 文件数量（输出）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t file_utils_scan_directory(const char *dir_path, const char *extension,
                                 char ***files, size_t *count);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_FILE_UTILS_H */
