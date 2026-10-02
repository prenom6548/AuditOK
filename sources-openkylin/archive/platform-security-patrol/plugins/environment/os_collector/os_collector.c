/**
 * @file os_collector.c
 * @brief 操作系统信息采集插件
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include "plugin_env_utils.h"
#include "logger.h"

#define MODULE "os_collector"

const char* plugin_type = "collector";

/**
 * @brief 采集 OS 发行版信息
 */
static psp_error_t collect_os_distribution(plugin_env_list_t *env_list, const char *plugin_name)
{
    if (!env_list || !plugin_name) {
        return ERR_INVALID_PARAM;
    }
    
    FILE *fp = fopen("/etc/os-release", "r");
    if (!fp) {
        logger_warn(MODULE, "Failed to open /etc/os-release");
        return ERR_IO;
    }
    
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        char *equals = strchr(line, '=');
        if (!equals) continue;
        
        *equals = '\0';
        char *key = line;
        char *value = equals + 1;
        
        /* 去除引号 */
        while (*value == '"' || *value == '\'') value++;
        char *newline = strchr(value, '\n');
        if (newline) *newline = '\0';
        newline = strchr(value, '"');
        if (newline) *newline = '\0';
        
        if (strcmp(key, "NAME") == 0) {
            plugin_env_list_add(env_list, "distribution", value, plugin_name);
        } else if (strcmp(key, "VERSION") == 0) {
            plugin_env_list_add(env_list, "version", value, plugin_name);
        } else if (strcmp(key, "ID") == 0) {
            plugin_env_list_add(env_list, "id", value, plugin_name);
        }
    }
    
    fclose(fp);
    return ERR_OK;
}

/**
 * @brief 采集内核版本信息
 */
static psp_error_t collect_kernel_version(plugin_env_list_t *env_list, const char *plugin_name)
{
    if (!env_list || !plugin_name) {
        return ERR_INVALID_PARAM;
    }
    
    struct utsname uts;
    if (uname(&uts) == 0) {
        char kernel_info[512];
        snprintf(kernel_info, sizeof(kernel_info), "%s %s %s",
                uts.sysname, uts.release, uts.version);
        plugin_env_list_add(env_list, "kernel", kernel_info, plugin_name);
        logger_debug(MODULE, "Kernel version: %s", kernel_info);
    }
    
    return ERR_OK;
}

/**
 * @brief 从内核配置文件读取指定配置项的值
 * 
 * @param config_path 配置文件路径
 * @param config_name 配置项名称（如 CONFIG_AMD_MEM_ENCRYPT）
 * @param value_buffer 输出缓冲区，存储配置值（y/m/n 或具体值）
 * @param buffer_size 缓冲区大小
 * @return 成功返回 ERR_OK，未找到返回 ERR_NOT_FOUND，失败返回 ERR_IO
 */
static psp_error_t read_kernel_config(const char *config_path, const char *config_name, 
                                  char *value_buffer, size_t buffer_size)
{
    if (!config_path || !config_name || !value_buffer || buffer_size == 0) {
        return ERR_INVALID_PARAM;
    }
    
    FILE *fp = fopen(config_path, "r");
    if (!fp) {
        return ERR_IO;
    }
    
    char line[1024];
    size_t name_len = strlen(config_name);
    psp_error_t result = ERR_NOT_FOUND;
    
    while (fgets(line, sizeof(line), fp)) {
        /* 跳过注释和空行 */
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        
        /* 查找配置项 */
        if (strncmp(line, config_name, name_len) == 0 && line[name_len] == '=') {
            /* 提取等号后面的值 */
            char *value_start = line + name_len + 1;
            
            /* 去除行尾换行符 */
            char *newline = strchr(value_start, '\n');
            if (newline) {
                *newline = '\0';
            }
            newline = strchr(value_start, '\r');
            if (newline) {
                *newline = '\0';
            }
            
            /* 复制值到缓冲区 */
            strncpy(value_buffer, value_start, buffer_size - 1);
            value_buffer[buffer_size - 1] = '\0';
            
            result = ERR_OK;
            break;
        }
    }
    
    fclose(fp);
    return result;
}

/**
 * @brief 采集内核配置项
 * 
 * 从 /boot/config-$(uname -r) 或 /proc/config.gz 读取内核编译配置
 * 键名格式：os.CONFIG_XXX（如 os.CONFIG_AMD_MEM_ENCRYPT）
 */
static psp_error_t collect_kernel_config(plugin_env_list_t *env_list, const char *plugin_name)
{
    if (!env_list || !plugin_name) {
        return ERR_INVALID_PARAM;
    }
    
    /* 需要采集的内核配置项列表 */
    const char *config_items[] = {
        "CONFIG_AMD_MEM_ENCRYPT",
        "CONFIG_KVM_AMD_SEV",
        "CONFIG_CRYPTO_DEV_CCP",
        "CONFIG_CRYPTO_DEV_CCP_DD",
        "CONFIG_CRYPTO_DEV_SP_CCP",
        "CONFIG_CRYPTO_DEV_SP_PSP",
        "CONFIG_HYGON_CSV",
        "CONFIG_CSV_GUEST"
    };
    
    const size_t config_count = sizeof(config_items) / sizeof(config_items[0]);
    
    /* 构建配置文件路径 */
    struct utsname uts;
    char config_path[512];
    
    if (uname(&uts) != 0) {
        logger_warn(MODULE, "Failed to get kernel release via uname");
        return ERR_IO;
    }
    
    /* 优先尝试 /boot/config-$(uname -r) */
    snprintf(config_path, sizeof(config_path), "/boot/config-%s", uts.release);
    
    /* 如果 /boot/config 不存在，尝试 /proc/config.gz（需要解压） */
    FILE *test_fp = fopen(config_path, "r");
    if (!test_fp) {
        /* 尝试 /proc/config.gz 需要先解压，这里暂时不支持 */
        logger_warn(MODULE, "Kernel config file not found: %s", config_path);
        logger_warn(MODULE, "Try: zcat /proc/config.gz > /tmp/kernel_config and set env var");
        return ERR_IO;
    }
    fclose(test_fp);
    
    logger_info(MODULE, "Reading kernel config from: %s", config_path);
    
    /* 逐个读取配置项 */
    int found_count = 0;
    for (size_t i = 0; i < config_count; i++) {
        char value[64] = {0};
        psp_error_t ret = read_kernel_config(config_path, config_items[i], value, sizeof(value));
        
        if (ret == ERR_OK) {
            /* 构建键名：直接使用CONFIG_XXX */
            
            /* 添加环境变量 */
            plugin_env_list_add(env_list, config_items[i], value, plugin_name);
            // logger_debug(MODULE, "Kernel config: %s = %s", config_items[i], value);
            found_count++;
        } else if (ret == ERR_NOT_FOUND) {
            /* 配置项不存在，设置为 n */
            plugin_env_list_add(env_list, config_items[i], "n", plugin_name);
            // logger_debug(MODULE, "Kernel config: %s = n (not found)", config_items[i]);
        } else {
            logger_warn(MODULE, "Failed to read kernel config: %s", config_items[i]);
        }
    }
    
    logger_info(MODULE, "Collected %d/%zu kernel config items", found_count, config_count);
    return ERR_OK;
}

/**
 * @brief 检查安全设备节点是否存在
 * 
 * @param device_paths 设备路径数组
 * @param device_count 设备路径数量
 * @param device_type 设备类型标识(用于日志,如 "TPM", "TCM")
 * @return true if device found
 */
static bool check_security_device_exists(const char **device_paths,
                                         size_t device_count,
                                         const char *device_type)
{
    if (!device_paths || !device_type) {
        return false;
    }
    
    for (size_t i = 0; i < device_count; i++) {
        struct stat st;
        if (stat(device_paths[i], &st) == 0) {
            logger_info(MODULE, "%s device node found: %s", device_type, device_paths[i]);
            return true;
        }
    }
    
    logger_debug(MODULE, "No %s device node found", device_type);
    return false;
}

/**
 * @brief 采集并记录TPM设备节点存在性
 * 
 * @param env_list 环境变量列表
 * @param plugin_name 插件名称
 * @return ERR_OK on success
 */
static psp_error_t collect_tpm_device_node(plugin_env_list_t *env_list, const char *plugin_name)
{
    if (!env_list || !plugin_name) {
        return ERR_INVALID_PARAM;
    }
    
    const char *tpm_devices[] = {
        "/dev/tpm0",      /* 传统TPM设备节点 */
        "/dev/tpmrm0"     /* TPM Resource Manager (推荐) */
    };
    
    bool tpm_found = check_security_device_exists(
        tpm_devices, sizeof(tpm_devices) / sizeof(tpm_devices[0]), "TPM"
    );
    
    plugin_env_list_add(env_list, "TPM Driver", 
                       tpm_found ? "true" : "false", plugin_name);
    
    return ERR_OK;
}

/**
 * @brief 采集并记录TCM设备节点存在性
 * 
 * @param env_list 环境变量列表
 * @param plugin_name 插件名称
 * @return ERR_OK on success
 */
static psp_error_t collect_tcm_device_node(plugin_env_list_t *env_list, const char *plugin_name)
{
    if (!env_list || !plugin_name) {
        return ERR_INVALID_PARAM;
    }
    
    const char *tcm_devices[] = {
        "/dev/tcm0"       /* TCM设备节点 */
    };
    
    bool tcm_found = check_security_device_exists(
        tcm_devices, sizeof(tcm_devices) / sizeof(tcm_devices[0]), "TCM"
    );
    
    plugin_env_list_add(env_list, "TCM Driver", 
                       tcm_found ? "true" : "false", plugin_name);
    
    return ERR_OK;
}

/**
 * @brief 采集TPM/TCM设备节点存在性
 */
static psp_error_t collect_security_device_nodes(plugin_env_list_t *env_list, const char *plugin_name)
{
    if (!env_list || !plugin_name) {
        return ERR_INVALID_PARAM;
    }
    
    /* 采集TPM设备节点 */
    psp_error_t ret = collect_tpm_device_node(env_list, plugin_name);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect TPM device node info");
    }
    
    /* 采集TCM设备节点 */
    ret = collect_tcm_device_node(env_list, plugin_name);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect TCM device node info");
    }
    
    return ERR_OK;
}

/**
 * @brief 检查系统使用的包管理器类型
 * 
 * @return "rpm" for CentOS/RHEL/Fedora, "dpkg" for Ubuntu/Debian, NULL if unknown
 */
static const char* get_package_manager_type(void)
{
    /* 检查是否存在 rpm 命令 */
    if (access("/usr/bin/rpm", X_OK) == 0 || access("/bin/rpm", X_OK) == 0) {
        logger_debug(MODULE, "Detected RPM-based package manager");
        return "rpm";
    }
    
    /* 检查是否存在 dpkg 命令 */
    if (access("/usr/bin/dpkg", X_OK) == 0 || access("/bin/dpkg", X_OK) == 0) {
        logger_debug(MODULE, "Detected DPKG-based package manager");
        return "dpkg";
    }
    
    logger_warn(MODULE, "Unknown package manager type");
    return NULL;
}

/**
 * @brief 验证包名是否安全（只允许字母、数字、短横线、点号、加号）
 */
static bool is_safe_package_name(const char *name)
{
    if (!name || *name == '\0') return false;
    for (const char *p = name; *p; p++) {
        if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') || *p == '-' || *p == '.' || 
              *p == '+' || *p == '_')) {
            return false;
        }
    }
    return true;
}

/**
 * @brief 通过rpm查询软件包版本
 * 
 * @param package_name 软件包名称
 * @param version_buffer 输出版本号缓冲区
 * @param buffer_size 缓冲区大小
 * @return 成功返回ERR_OK，失败返回ERR_IO或ERR_NOT_FOUND
 */
static psp_error_t get_rpm_package_version(const char *package_name, 
                                       char *version_buffer, size_t buffer_size)
{
    if (!package_name || !version_buffer || buffer_size == 0) {
        return ERR_INVALID_PARAM;
    }
    
    if (!is_safe_package_name(package_name)) {
        logger_error(MODULE, "Unsafe package name rejected: %s", package_name);
        return ERR_INVALID_PARAM;
    }

    /* 构建rpm查询命令：rpm -q --queryformat '%{VERSION}' package_name */
    char cmd[512];
    int cmd_len = snprintf(cmd, sizeof(cmd), "rpm -q --queryformat '%%{VERSION}' %s 2>/dev/null", package_name);
    if (cmd_len < 0 || (size_t)cmd_len >= sizeof(cmd)) {
        logger_warn(MODULE, "Package name too long: %s", package_name);
        return ERR_INVALID_PARAM;
    }
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        logger_warn(MODULE, "Failed to execute rpm command for %s", package_name);
        return ERR_IO;
    }
    
    if (fgets(version_buffer, buffer_size, fp) == NULL) {
        pclose(fp);
        logger_debug(MODULE, "Package %s not found via rpm", package_name);
        return ERR_NOT_FOUND;
    }
    
    /* 去除行尾换行符 */
    size_t len = strlen(version_buffer);
    if (len > 0 && version_buffer[len - 1] == '\n') {
        version_buffer[len - 1] = '\0';
    }
    
    int ret = pclose(fp);
    if (ret != 0) {
        logger_debug(MODULE, "Package %s query failed (exit code: %d)", package_name, ret);
        return ERR_NOT_FOUND;
    }
    
    // logger_debug(MODULE, "RPM package %s version: %s", package_name, version_buffer);
    return ERR_OK;
}

/**
 * @brief 通过dpkg查询软件包版本
 * 
 * @param package_name 软件包名称
 * @param version_buffer 输出版本号缓冲区
 * @param buffer_size 缓冲区大小
 * @return 成功返回ERR_OK，失败返回ERR_IO或ERR_NOT_FOUND
 */
static psp_error_t get_dpkg_package_version(const char *package_name,
                                        char *version_buffer, size_t buffer_size)
{
    if (!package_name || !version_buffer || buffer_size == 0) {
        return ERR_INVALID_PARAM;
    }

    if (!is_safe_package_name(package_name)) {
        logger_error(MODULE, "Unsafe package name rejected: %s", package_name);
        return ERR_INVALID_PARAM;
    }
    
    /* 构建dpkg查询命令：dpkg-query -W -f='${Version}' package_name */
    char cmd[512];
    int cmd_len = snprintf(cmd, sizeof(cmd), "dpkg-query -W -f='${Version}' %s 2>/dev/null", package_name);
    if (cmd_len < 0 || (size_t)cmd_len >= sizeof(cmd)) {
        logger_warn(MODULE, "Package name too long: %s", package_name);
        return ERR_INVALID_PARAM;
    }
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        logger_warn(MODULE, "Failed to execute dpkg command for %s", package_name);
        return ERR_IO;
    }
    
    if (fgets(version_buffer, buffer_size, fp) == NULL) {
        pclose(fp);
        logger_debug(MODULE, "Package %s not found via dpkg", package_name);
        return ERR_NOT_FOUND;
    }
    
    /* 去除行尾换行符 */
    size_t len = strlen(version_buffer);
    if (len > 0 && version_buffer[len - 1] == '\n') {
        version_buffer[len - 1] = '\0';
    }
    
    int ret = pclose(fp);
    if (ret != 0) {
        logger_debug(MODULE, "Package %s query failed (exit code: %d)", package_name, ret);
        return ERR_NOT_FOUND;
    }
    
    logger_debug(MODULE, "DPKG package %s version: %s", package_name, version_buffer);
    return ERR_OK;
}

/**
 * @brief 获取软件包版本（自动选择包管理器）
 * 
 * @param package_name 软件包名称
 * @param version_buffer 输出版本号缓冲区
 * @param buffer_size 缓冲区大小
 * @return 成功返回ERR_OK，失败返回ERR_IO或ERR_NOT_FOUND
 */
static psp_error_t get_package_version(const char *package_name,
                                   char *version_buffer, size_t buffer_size)
{
    if (!package_name || !version_buffer || buffer_size == 0) {
        return ERR_INVALID_PARAM;
    }
    
    const char *pkg_manager = get_package_manager_type();
    if (!pkg_manager) {
        logger_warn(MODULE, "No supported package manager found");
        return ERR_NOT_FOUND;
    }
    
    if (strcmp(pkg_manager, "rpm") == 0) {
        return get_rpm_package_version(package_name, version_buffer, buffer_size);
    } else if (strcmp(pkg_manager, "dpkg") == 0) {
        return get_dpkg_package_version(package_name, version_buffer, buffer_size);
    }
    
    return ERR_NOT_FOUND;
}

/**
 * @brief 采集TPM/TCM相关软件包版本信息
 * 
 * 采集以下软件包版本：
 * - tpm2-tools
 * - tpm2-abrmd
 * - tpm2-tss
 * 
 * @param env_list 环境变量列表
 * @param plugin_name 插件名称
 * @return ERR_OK on success
 */
static psp_error_t collect_tpm_software_versions(plugin_env_list_t *env_list, const char *plugin_name)
{
    if (!env_list || !plugin_name) {
        return ERR_INVALID_PARAM;
    }
    
    /* 需要采集的软件包列表 */
    const char *package_name[] = {
        "tpm2-tools",
        "tpm2-abrmd",
        "tpm2-tss"
    };
    
    const size_t pkg_count = sizeof(package_name) / sizeof(package_name[0]);
    int collected_count = 0;
    
    for (size_t i = 0; i < pkg_count; i++) {
        char version[128] = {0};
        psp_error_t ret = get_package_version(package_name[i], version, sizeof(version));
        
        if (ret == ERR_OK) {
            plugin_env_list_add(env_list, package_name[i], version, plugin_name);
            logger_info(MODULE, "Package %s version: %s", package_name[i], version);
            collected_count++;
        } else {
            /* 软件包未安装，设置为空字符串或"not_installed" */
            plugin_env_list_add(env_list, package_name[i], "not_installed", plugin_name);
            logger_debug(MODULE, "Package %s not installed", package_name[i]);
        }
    }
    
    logger_info(MODULE, "Collected %d/%zu TPM software package versions", 
               collected_count, pkg_count);
    return ERR_OK;
}

/**
 * @brief 通过dmesg检查TDM驱动是否加载成功
 * 
 * TDM驱动成功加载的标志是在dmesg中出现：
 * "tdm: TDM driver loaded successfully!"
 * 
 * @param env_list 环境变量列表
 * @param plugin_name 插件名称
 * @return ERR_OK on success
 */
static psp_error_t collect_tdm_driver_status(plugin_env_list_t *env_list, const char *plugin_name)
{
    if (!env_list || !plugin_name) {
        return ERR_INVALID_PARAM;
    }
    
    /* 执行 dmesg 命令并过滤 TDM 驱动加载信息 */
    const char *cmd = "dmesg 2>/dev/null | grep -i 'tdm: TDM driver loaded successfully!'";
    FILE *fp = popen(cmd, "r");
    
    bool tdm_driver_loaded = false;
    if (fp) {
        char line[1024];
        if (fgets(line, sizeof(line), fp)) {
            /* 如果读取到内容，说明找到了匹配的行 */
            tdm_driver_loaded = true;
            logger_debug(MODULE, "Found TDM driver loaded message in dmesg");
        }
        pclose(fp);
    } else {
        logger_warn(MODULE, "Failed to execute dmesg command (may need root privileges)");
    }
    
    /* 将TDM驱动状态添加到环境列表 */
    plugin_env_list_add(env_list, "TDM Driver", 
                       tdm_driver_loaded ? "true" : "false", 
                       plugin_name);
    
    logger_info(MODULE, "TDM Driver status: %s", 
               tdm_driver_loaded ? "loaded" : "not loaded");
    
    return ERR_OK;
}

int plugin_init(void)
{
    logger_debug(MODULE, "OS collector initialized");
    return 0;
}

/**
 * @brief 采集函数（必须实现）
 * 
 * 返回插件环境信息列表，由 core 层统一管理
 */
plugin_env_list_t* collect(const char *plugin_name)
{
    if (!plugin_name) {
        logger_error(MODULE, "Plugin name is NULL");
        return NULL;
    }
    
    logger_info(MODULE, "Collecting OS information (plugin: %s)", plugin_name);
    
    /* 创建插件环境列表 */
    plugin_env_list_t *env_list = plugin_env_list_create();
    if (!env_list) {
        logger_error(MODULE, "Failed to create environment list");
        return NULL;
    }
    
    /* 采集 OS 发行版信息 */
    psp_error_t ret = collect_os_distribution(env_list, plugin_name);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect OS distribution info");
    }
    
    /* 采集内核版本信息 */
    ret = collect_kernel_version(env_list, plugin_name);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect kernel version");
    }
    
    /* 采集内核配置项 */
    ret = collect_kernel_config(env_list, plugin_name);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect kernel config items");
    }
    
    /* 采集TPM/TCM设备节点 */
    ret = collect_security_device_nodes(env_list, plugin_name);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect security device nodes");
    }
    
    /* 采集TPM/TCM相关软件包版本 */
    ret = collect_tpm_software_versions(env_list, plugin_name);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect TPM software versions");
    }
    
    /* 采集TDM驱动状态 */
    ret = collect_tdm_driver_status(env_list, plugin_name);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect TDM driver status");
    }
    
    /* TODO: 检查安全模块加载状态 */
    /* - SELinux: /sys/fs/selinux/enforce */
    /* - AppArmor: /sys/kernel/security/apparmor/profiles */
    
    logger_info(MODULE, "OS collection completed with %zu items", env_list->count);
    return env_list;
}

void plugin_fini(void)
{
    logger_debug(MODULE, "OS collector finalized");
}
