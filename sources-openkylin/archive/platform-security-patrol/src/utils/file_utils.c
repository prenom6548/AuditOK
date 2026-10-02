#include "utils/file_utils.h"
#include "utils/string_utils.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#define MODULE "file_utils"

psp_error_t file_utils_read_file(const char *file_path, char **content, size_t *size)
{
    if (!file_path || !content) {
        return ERR_INVALID_PARAM;
    }
    
    FILE *fp = fopen(file_path, "r");
    if (!fp) {
        logger_error(MODULE, "Failed to open file: %s (error: %d)", file_path, errno);
        return ERR_IO;
    }
    
    /* 尝试获取文件大小（对普通文件有效） */
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char *buffer = NULL;
    size_t bytes_read = 0;
    
    if (file_size > 0) {
        /* 普通文件：预分配缓冲区 */
        buffer = (char*)malloc(file_size + 1);
        if (!buffer) {
            logger_error(MODULE, "Failed to allocate memory for file content");
            fclose(fp);
            return ERR_MEMORY;
        }
        
        bytes_read = fread(buffer, 1, file_size, fp);
        if (bytes_read == 0 && ferror(fp)) {
            logger_error(MODULE, "Error reading file: %s", file_path);
            free(buffer);
            fclose(fp);
            return ERR_IO;
        }
        buffer[bytes_read] = '\0';
    } else {
        //  虚拟文件（如/proc/*）：动态扩容读取 
        size_t capacity = 4096;  /* 初始容量 4KB */
        buffer = (char*)malloc(capacity);
        if (!buffer) {
            logger_error(MODULE, "Failed to allocate initial buffer");
            fclose(fp);
            return ERR_MEMORY;
        }
        
        while (!feof(fp)) {
            /* 确保有足够空间 */
            if (bytes_read >= capacity - 1) {
                capacity *= 2;  /* 双倍扩容 */
                char *new_buffer = (char*)realloc(buffer, capacity);
                if (!new_buffer) {
                    logger_error(MODULE, "Failed to reallocate buffer");
                    free(buffer);
                    fclose(fp);
                    return ERR_MEMORY;
                }
                buffer = new_buffer;
            }
            
            /* 分批读取 */
            size_t chunk_size = capacity - bytes_read - 1;
            size_t n = fread(buffer + bytes_read, 1, chunk_size, fp);
            if (n == 0 && ferror(fp)) {
                logger_error(MODULE, "Error reading file: %s", file_path);
                free(buffer);
                fclose(fp);
                return ERR_IO;
            }
            bytes_read += n;
        }
        
        buffer[bytes_read] = '\0';
        
        /* 收缩到实际大小 */
        char *shrink_buffer = (char*)realloc(buffer, bytes_read + 1);
        if (shrink_buffer) {
            buffer = shrink_buffer;
        }
    }
    
    fclose(fp);
    
    *content = buffer;
    if (size) {
        *size = bytes_read;
    }
    
    return ERR_OK;
}

psp_error_t file_utils_write_file(const char *file_path, const char *content, size_t size)
{
    if (!file_path || !content) {
        return ERR_INVALID_PARAM;
    }
    
    if (size == 0) {
        size = strlen(content);
    }
    
    FILE *fp = fopen(file_path, "w");
    if (!fp) {
        logger_error(MODULE, "Failed to open file for writing: %s (error: %d)", 
                    file_path, errno);
        return ERR_IO;
    }
    
    size_t bytes_written = fwrite(content, 1, size, fp);
    fclose(fp);
    
    if (bytes_written != size) {
        logger_error(MODULE, "Failed to write all bytes to file: %s", file_path);
        return ERR_IO;
    }
    
    logger_debug(MODULE, "Wrote %zu bytes to file: %s", bytes_written, file_path);
    return ERR_OK;
}

bool file_utils_file_exists(const char *file_path)
{
    if (!file_path) {
        return false;
    }
    
    struct stat st;
    return stat(file_path, &st) == 0;
}

bool file_utils_is_directory(const char *path)
{
    if (!path) {
        return false;
    }
    
    struct stat st;
    if (stat(path, &st) != 0) {
        return false;
    }
    
    return S_ISDIR(st.st_mode);
}

bool file_utils_is_regular_file(const char *path)
{
    if (!path) {
        return false;
    }
    
    struct stat st;
    if (stat(path, &st) != 0) {
        return false;
    }
    
    return S_ISREG(st.st_mode);
}

long file_utils_get_file_size(const char *file_path)
{
    if (!file_path) {
        return -1;
    }
    
    struct stat st;
    if (stat(file_path, &st) != 0) {
        logger_error(MODULE, "Failed to get file size: %s", file_path);
        return -1;
    }
    
    return (long)st.st_size;
}

psp_error_t file_utils_create_directory(const char *path, int mode)
{
    if (!path) {
        return ERR_INVALID_PARAM;
    }
    
    /* 尝试直接创建 */
    if (mkdir(path, mode) == 0) {
        logger_debug(MODULE, "Created directory: %s", path);
        return ERR_OK;
    }
    
    /* 如果已存在，不算错误 */
    if (errno == EEXIST && file_utils_is_directory(path)) {
        return ERR_OK;
    }
    
    /* 否则尝试递归创建父目录 */
    char *path_copy = string_utils_strdup(path);
    if (!path_copy) {
        return ERR_MEMORY;
    }
    
    /* 查找最后一个分隔符 */
    char *last_sep = strrchr(path_copy, '/');
    if (last_sep && last_sep != path_copy) {
        *last_sep = '\0';
        
        /* 递归创建父目录 */
        psp_error_t ret = file_utils_create_directory(path_copy, mode);
        free(path_copy);
        
        if (ret != ERR_OK) {
            return ret;
        }
        
        /* 再次尝试创建当前目录 */
        if (mkdir(path, mode) == 0) {
            logger_debug(MODULE, "Created directory: %s", path);
            return ERR_OK;
        }
    } else {
        free(path_copy);
    }
    
    logger_error(MODULE, "Failed to create directory: %s (error: %d)", path, errno);
    return ERR_IO;
}

/**
 * @brief 递归扫描目录，收集匹配扩展名的文件（返回相对路径）
 *
 * @param dir_path 绝对路径（用于 opendir）
 * @param rel_prefix 相对于扫描根目录的前缀（顶层为空字符串）
 * @param extension 文件扩展名过滤（NULL 表示不过滤）
 * @param temp_files 动态数组（输入/输出）
 * @param temp_count 当前元素数量（输入/输出）
 * @param capacity 当前数组容量（输入/输出）
 * @return psp_error_t
 */
static psp_error_t scan_directory_recursive(const char *dir_path,
                                       const char *rel_prefix,
                                       const char *extension,
                                       char ***temp_files,
                                       size_t *temp_count,
                                       size_t *capacity)
{
    DIR *dir = opendir(dir_path);
    if (!dir) {
        logger_error(MODULE, "Failed to open directory: %s (error: %d)", dir_path, errno);
        return ERR_IO;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        /* 跳过 . 和 .. */
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        /* 构造完整路径以判断是否为目录 */
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) != 0) {
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            /* 递归进入子目录 */
            char new_prefix[512];
            if (rel_prefix[0] == '\0') {
                snprintf(new_prefix, sizeof(new_prefix), "%s", entry->d_name);
            } else {
                snprintf(new_prefix, sizeof(new_prefix), "%s/%s", rel_prefix, entry->d_name);
            }
            psp_error_t ret = scan_directory_recursive(full_path, new_prefix, extension,
                                                  temp_files, temp_count, capacity);
            if (ret != ERR_OK) {
                closedir(dir);
                return ret;
            }
            continue;
        }

        /* 检查扩展名 */
        if (extension) {
            const char *dot = strrchr(entry->d_name, '.');
            if (!dot || strcmp(dot, extension) != 0) {
                continue;
            }
        }

        /* 扩容 */
        if (*temp_count >= *capacity) {
            size_t new_capacity = (*capacity == 0) ? 16 : *capacity * 2;
            char **new_files = (char**)realloc(*temp_files, new_capacity * sizeof(char*));
            if (!new_files) {
                logger_error(MODULE, "Failed to reallocate memory for files");
                closedir(dir);
                return ERR_MEMORY;
            }
            *temp_files = new_files;
            *capacity = new_capacity;
        }

        /* 构造相对路径：rel_prefix/filename 或 filename */
        char rel_path[1024];
        if (rel_prefix[0] == '\0') {
            snprintf(rel_path, sizeof(rel_path), "%s", entry->d_name);
        } else {
            snprintf(rel_path, sizeof(rel_path), "%s/%s", rel_prefix, entry->d_name);
        }

        (*temp_files)[*temp_count] = string_utils_strdup(rel_path);
        if (!(*temp_files)[*temp_count]) {
            logger_error(MODULE, "Failed to duplicate path: %s", rel_path);
            closedir(dir);
            return ERR_MEMORY;
        }

        (*temp_count)++;
    }

    closedir(dir);
    return ERR_OK;
}

psp_error_t file_utils_scan_directory(const char *dir_path, const char *extension,
                                 char ***files, size_t *count)
{
    if (!dir_path || !files || !count) {
        return ERR_INVALID_PARAM;
    }

    char **temp_files = NULL;
    size_t temp_count = 0;
    size_t capacity = 0;

    psp_error_t ret = scan_directory_recursive(dir_path, "", extension,
                                           &temp_files, &temp_count, &capacity);
    if (ret != ERR_OK) {
        for (size_t i = 0; i < temp_count; i++) {
            free(temp_files[i]);
        }
        free(temp_files);
        return ret;
    }

    *files = temp_files;
    *count = temp_count;

    return ERR_OK;
}
