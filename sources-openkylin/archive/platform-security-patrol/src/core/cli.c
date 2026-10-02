#include "core/cli.h"
#include "core/base_collector.h"
#include "core/capability_parser.h"
#include "core/judge.h"
#include "core/executor.h"
#include "core/extension.h"
#include "logger.h"
#include "plugin_env_utils.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"
#include "utils/cmd_runner.h"
#include "utils/system_paths.h"
#include "models/environment.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include "cJSON.h"

#define MODULE "cli"

static void print_usage(void)
{
    printf("Usage: plat-sec-patrol [OPTIONS] COMMAND\n");
    printf("\n");
    printf("Platform Security Patrol Tool - Detect and enable hardware security capabilities\n");
    printf("\n");
    printf("Commands:\n");
    printf("  check [CAP_ID]     Check capabilities and report status (table format)\n");
    printf("  enable <CAP_ID>    Enable a specific capability\n");
    printf("  disable <CAP_ID>   Disable a specific capability\n");
    printf("  list               List all available capabilities\n");
    printf("\n");
    printf("Options:\n");
    printf("  -f, --format FORMAT    Output format: table (default) or json\n");
    printf("  -v, --verbose          Enable verbose output (debug level)\n");
    printf("  -p, --plugin-dir DIR   Specify plugin directory\n");
    printf("  -h, --help             Show this help message\n");
    printf("  -V, --version          Show version information\n");
    printf("\n");
    printf("Examples:\n");
    printf("  plat-sec-patrol check                     # Table format (default)\n");
    printf("  plat-sec-patrol check --format table      # Table format\n");
    printf("  plat-sec-patrol check --format json       # JSON format\n");
    printf("  plat-sec-patrol enable tcm\n");
    printf("  plat-sec-patrol list --verbose\n");
}

static void print_version(void)
{
    printf("plat-sec-patrol version 1.0.0\n");
    printf("Platform Security Patrol Tool for Hygon/Chinese CPU platforms\n");
}

/**
 * @brief 输出执行结果（支持JSON和文本格式）
 * 
 * @param result 执行结果对象
 * @param format 输出格式
 */
static void print_execution_result(const execution_result_t *result, output_format_t format)
{
    if (!result) {
        return;
    }
    
    if (format == FORMAT_JSON) {
        cJSON *json_result = cJSON_CreateObject();
        cJSON_AddStringToObject(json_result, "capability_id", result->capability_id);
        cJSON_AddStringToObject(json_result, "action", result->action);
        cJSON_AddBoolToObject(json_result, "operation_completed", result->operation_completed);
        
        if (result->message) {
            cJSON_AddStringToObject(json_result, "message", result->message);
        }
        if (result->guidance) {
            cJSON_AddStringToObject(json_result, "guidance", result->guidance);
        }
        
        char *json_str = cJSON_PrintUnformatted(json_result);
        if (json_str) {
            printf("%s\n", json_str);
            free(json_str);
        }
        
        cJSON_Delete(json_result);
    } else {
        printf("Capability: %s\n", result->capability_id ? result->capability_id : "");
        printf("Action: %s\n", result->action ? result->action : "");
        printf("Operation Completed: %s\n", result->operation_completed ? "yes" : "no");
        
        if (result->message) {
            printf("Message: %s\n", result->message);
        }
        if (result->guidance) {
            printf("\nTo enable this capability, please address the following issues:\n%s\n", 
                   result->guidance);
        }
    }
}

psp_error_t cli_parse_args(int argc, char **argv, cli_config_t *config)
{
    if (!config) {
        return ERR_INVALID_PARAM;
    }
    
    /* 初始化配置 */
    memset(config, 0, sizeof(cli_config_t));
    config->command = CMD_CHECK;
    config->format = FORMAT_TEXT;
    config->verbose = false;
    config->plugin_dir = NULL;
    config->capability_id = NULL;
    
    static struct option long_options[] = {
        {"format",     required_argument, 0, 'f'},
        {"verbose",    no_argument,       0, 'v'},
        {"plugin-dir", required_argument, 0, 'p'},
        {"help",       no_argument,       0, 'h'},
        {"version",    no_argument,       0, 'V'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "f:p:vhV", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'f':
                if (strcmp(optarg, "table") == 0) {
                    config->format = FORMAT_TEXT;
                } else if (strcmp(optarg, "json") == 0) {
                    config->format = FORMAT_JSON;
                } else {
                    fprintf(stderr, "Invalid format: %s (use 'table' or 'json')\n", optarg);
                    return ERR_INVALID_PARAM;
                }
                break;
                
            case 'v':
                config->verbose = true;
                break;
                
            case 'p':
                config->plugin_dir = optarg;
                break;
                
            case 'h':
                print_usage();
                exit(0);
                
            case 'V':
                print_version();
                exit(0);
                
            default:
                return ERR_INVALID_PARAM;
        }
    }
    
    /* 解析命令 */
    if (optind >= argc) {
        fprintf(stderr, "Error: No command specified\n");
        print_usage();
        return ERR_INVALID_PARAM;
    }
    
    const char *cmd_str = argv[optind];
    if (strcmp(cmd_str, "check") == 0) {
        config->command = CMD_CHECK;
        if (optind + 1 < argc) {
            config->capability_id = argv[optind + 1];
        }
    } else if (strcmp(cmd_str, "enable") == 0) {
        config->command = CMD_ENABLE;
        if (optind + 1 >= argc) {
            fprintf(stderr, "Error: 'enable' requires capability ID\n");
            return ERR_INVALID_PARAM;
        }
        config->capability_id = argv[optind + 1];
    } else if (strcmp(cmd_str, "disable") == 0) {
        config->command = CMD_DISABLE;
        if (optind + 1 >= argc) {
            fprintf(stderr, "Error: 'disable' requires capability ID\n");
            return ERR_INVALID_PARAM;
        }
        config->capability_id = argv[optind + 1];
    } else if (strcmp(cmd_str, "list") == 0) {
        config->command = CMD_LIST;
    } else {
        fprintf(stderr, "Error: Unknown command: %s\n", cmd_str);
        print_usage();
        return ERR_INVALID_PARAM;
    }
    
    logger_debug(MODULE, "Parsed command line: command=%d, format=%d, verbose=%d",
                config->command, config->format, config->verbose);
    
    return ERR_OK;
}

/**
 * @brief 格式化输出判定结果（表格格式）
 */
static void print_results_table(const judgement_result_list_t *results)
{
    if (!results || results->count == 0) {
        printf("No capabilities found.\n");
        return;
    }
    
    /* 定义列宽 */
    const int COL_NAME = 25;      /* Name 列宽度 */
    const int COL_OVERALL = 18;   /* Overall Status 列宽度 */
    const int COL_CPU = 12;       /* CPU Layer 列宽度 */
    const int COL_FIRMWARE = 16;  /* Firmware Layer 列宽度 */
    const int COL_OS = 12;        /* OS Layer 列宽度 */
    const int COL_DESC = 35;      /* Description 列宽度 */
    
    /* 打印表头 */
    printf("\n");
    printf("%-*s %-*s %-*s %-*s %-*s %-*s\n",
           COL_NAME, "Name",
           COL_OVERALL, "Overall Status",
           COL_CPU, "CPU Layer",
           COL_FIRMWARE, "Firmware Layer",
           COL_OS, "OS Layer",
           COL_DESC, "Description");
    
    /* 打印分隔线 */
    int total_width = COL_NAME + COL_OVERALL + COL_CPU + COL_FIRMWARE + COL_OS + COL_DESC + 5; /* 5 个空格间隔 */
    for (int i = 0; i < total_width; i++) {
        printf("-");
    }
    printf("\n");
    
    /* 打印每一行数据 */
    for (size_t i = 0; i < results->count; i++) {
        judgement_result_t *result = results->items[i];
        if (!result) continue;
        
        /* 获取能力名称（优先使用 name，否则使用 id） */
        const char *name = result->capability_name ? result->capability_name : 
                          (result->capability_id ? result->capability_id : "unknown");
        
        /* 状态字符串 */
        const char *overall_str = cap_status_to_string(result->overall_status);
        const char *cpu_str = layer_status_to_string(result->cpu_status);
        const char *firmware_str = layer_status_to_string(result->firmware_status);
        const char *os_str = layer_status_to_string(result->os_status);
        
        /* 描述信息 */
        const char *desc = result->description ? result->description : "N/A";
        
        /* 打印行 */
        printf("%-*s %-*s %-*s %-*s %-*s %-*s\n",
               COL_NAME, name,
               COL_OVERALL, overall_str,
               COL_CPU, cpu_str,
               COL_FIRMWARE, firmware_str,
               COL_OS, os_str,
               COL_DESC, desc);
    }
    
    /* 打印总计 */
    printf("\nTotal: %zu capabilities\n", results->count);
    
    /* 统计各状态数量 */
    size_t available_count = 0;
    size_t unsupported_count = 0;
    size_t unavailable_count = 0;
    size_t unknown_count = 0;
    size_t disabled_count = 0;
    
    for (size_t i = 0; i < results->count; i++) {
        judgement_result_t *result = results->items[i];
        if (!result) continue;
        
        switch (result->overall_status) {
            case CAP_STATUS_AVAILABLE:
            case CAP_STATUS_SUPPORTED:
                available_count++;
                break;
            case CAP_STATUS_UNSUPPORTED:
                unsupported_count++;
                break;
            case CAP_STATUS_DISABLED:
                disabled_count++;
                break;
            case CAP_STATUS_UNAVAILABLE:
                unavailable_count++;
                break;
            default:
                unknown_count++;
        }
    }
    
    printf("  Available: %zu, Disabled: %zu, Unsupported: %zu, Unavailable: %zu, Unknown: %zu\n",
           available_count, disabled_count, unsupported_count, unavailable_count, unknown_count);
    printf("\n");
}

/*
=== Capability:tpm2.0 ===
Overall status:DISABLED 
Layer Analysis:
CPU Layer:  SUPPORTED
            -- Hygon C86-4G 7258 supports tpm2.0
Firmware:   UNSUPPORTED
            -- tpm enabled in bios
os Layer:   UNSUPPORTED
            -- CONFIG_TCG_TPM=n
            -- CONFIG_TCG_TIS_CORE=n
            -- Module tpm_tis:not loaded

Failure Reasons:
1.Kernel config CONFIG TCG TIS CORE is not enabled
2.TPM TIS driver module is not loaded
3.Device node /dev/tpmg does not exist
*/

static void print_cap_detail_result(char *capability_id, const judgement_result_list_t *results)
{
    if (!capability_id || !results || results->count == 0) {
        printf("No capabilities found.\n");
        return;
    }
    
    for (size_t i = 0; i < results->count; i++) {
        judgement_result_t *result = results->items[i];
        if (!result || !result->capability_id) continue;

        if (strcmp(result->capability_id, capability_id) == 0){
            printf("\n=== Capability %s Status Report ===\n", capability_id);
            printf("Capability: %s\n", result->capability_id);
            printf("Status:     %s\n", cap_status_to_string(result->overall_status));
            printf("  -- CPU Layer:      %s\n", layer_status_to_string(result->cpu_status));
            printf("  -- Firmware Layer: %s\n", layer_status_to_string(result->firmware_status));
            printf("  -- OS Layer:       %s\n", layer_status_to_string(result->os_status));
            return;
        }
    }
    printf("No capabilities %s found.\n", capability_id);
}

/**
 * @brief 格式化输出判定结果（JSON 格式）
 */
static void print_results_json(const judgement_result_list_t *results)
{
    cJSON *json_root = cJSON_CreateArray();
    if (!json_root) {
        fprintf(stderr, "Failed to create JSON array\n");
        return;
    }
    
    for (size_t i = 0; i < results->count; i++) {
        judgement_result_t *result = results->items[i];
        if (!result) continue;
        
        cJSON *json_item = cJSON_CreateObject();
        if (!json_item) continue;
        
        cJSON_AddStringToObject(json_item, "capability_id", 
                               result->capability_id ? result->capability_id : "");
        cJSON_AddStringToObject(json_item, "overall_status", 
                               cap_status_to_string(result->overall_status));
        cJSON_AddStringToObject(json_item, "cpu_status", 
                               layer_status_to_string(result->cpu_status));
        cJSON_AddStringToObject(json_item, "firmware_status", 
                               layer_status_to_string(result->firmware_status));
        cJSON_AddStringToObject(json_item, "os_status", 
                               layer_status_to_string(result->os_status));
        
        if (result->evidence) {
            cJSON_AddStringToObject(json_item, "evidence", result->evidence);
        }
        
        cJSON_AddItemToArray(json_root, json_item);
    }
    
    //char *json_str = cJSON_PrintUnformatted(json_root);
    char *json_str = cJSON_Print(json_root);
    if (json_str) {
        printf("%s\n", json_str);
        free(json_str);
    }
    
    cJSON_Delete(json_root);
}

/**
 * @brief 检查能力库声明的外部工具是否缺失，并输出告警与安装指导
 *
 * 文本格式下警告块输出到 stdout（跟随检测结果之后）；
 * JSON 格式下输出到 stderr，保证 stdout 为纯净 JSON，便于自动化消费。
 *
 * @param caps 能力列表（含外部工具依赖声明）
 * @param format 输出格式
 */
static void report_missing_external_tools(const capability_list_t *caps, output_format_t format)
{
    if (!caps || caps->tool_dep_count == 0) {
        return;
    }

    FILE *out = (format == FORMAT_JSON) ? stderr : stdout;
    bool any_missing = false;

    for (size_t i = 0; i < caps->tool_dep_count; i++) {
        const external_tool_dep_t *dep = caps->tool_deps[i];
        if (!dep || cmd_runner_command_exists(dep->check_command)) {
            continue;
        }

        if (!any_missing) {
            fprintf(out, "\nWARNING: 平台能力库声明的外部工具缺失，相关能力检测结果可能不准确:\n");
            any_missing = true;
        }

        fprintf(out, "  [%s]\n", dep->name ? dep->name : "unknown");
        if (dep->install_guide) {
            fprintf(out, "    安装方法: %s\n", dep->install_guide);
        }
        if (dep->verify_command) {
            fprintf(out, "    安装验证: %s\n", dep->verify_command);
        }

        logger_warn(MODULE, "External tool '%s' not found: %s (install: %s)",
                   dep->name ? dep->name : "unknown",
                   dep->description ? dep->description : "no description",
                   dep->install_guide ? dep->install_guide : "N/A");
    }
}

/**
 * @brief 初始化插件系统并加载能力库
 * 
 * @param config CLI配置
 * @param base_env 基础环境信息（用于插件匹配）
 * @param[out] manager 输出的插件管理器指针
 * @param[out] env_data 输出的环境数据列表指针
 * @param[out] caps 输出的能力列表指针
 * @return error_t 错误码
 */
static psp_error_t get_information_from_plugins(const cli_config_t *config, 
                                        plugin_env_list_t **env_data,
                                        capability_list_t **caps)
{
    /* 采集基础环境信息（仅用于插件匹配） */
    environment_t *base_env = base_collector_collect();
    if (!base_env) {
        logger_error(MODULE, "Failed to collect base environment information");
        return ERR_GENERIC;
    }
    logger_info(MODULE, "Base environment collected: CPU=%s, Firmware=%s, OS=%s",
            base_env->cpu.vendor ? base_env->cpu.vendor : "unknown",
            base_env->firmware.type ? base_env->firmware.type : "unknown",
            base_env->os.distribution ? base_env->os.distribution : "unknown");

    /* 创建并初始化插件管理器 */
    const char *plugin_dir = config->plugin_dir ? config->plugin_dir : DEFAULT_PLUGIN_DIR;
    plugin_manager_t *manager = extension_create_manager(plugin_dir);
    if (!manager) {
        logger_error(MODULE, "Failed to create plugin manager");
        environment_free(base_env);
        return ERR_MEMORY;
    }

    /* 基于基础环境信息加载匹配的插件 */
    psp_error_t ret = extension_load_plugins(manager, base_env);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "No plugins loaded");
    }

    /* 调用环境感知插件采集扩展信息 */
    *env_data = NULL;
    ret = extension_invoke_collectors(manager, env_data);
    if (ret != ERR_OK || !*env_data) {
        logger_warn(MODULE, "Failed to collect environment data");
    } else {
        logger_info(MODULE, "Collected %zu environment items", (*env_data)->count);
    }
    
    /* 调用能力库插件，并解析内容 */
    *caps = extension_load_capabilities(manager, base_env);
    if (!*caps || (*caps)->count == 0) {
        logger_warn(MODULE, "No capabilities loaded");
    }
    
    environment_free(base_env);
    extension_free_manager(manager);
    return ERR_OK;
}

psp_error_t cmd_check(const cli_config_t *config)
{
    logger_info(MODULE, "Executing check command");
    
    /* 初始化插件系统并加载能力库 */
    plugin_env_list_t *env_data = NULL;
    capability_list_t *caps = NULL;

    psp_error_t ret = get_information_from_plugins(config, &env_data, &caps);
    if (ret != ERR_OK) {
        logger_error(MODULE, "Failed to get information from plugin");
        return ret;
    }
   
    /* 判定能力状态（使用 base_env 和 env_data） */
    judgement_result_list_t *results = judge_capabilities(env_data, caps);
    if (!results) {
        logger_error(MODULE, "Failed to judge capabilities");
        capability_list_free(caps);
        plugin_env_list_free(env_data);
        return ERR_GENERIC;
    }

    // 如果有指定某个能力，则输出单能力信息
    if (config->capability_id)
    {
        print_cap_detail_result(config->capability_id, results);
    }
    else {
        /* 输出所有安全能力最终结果 */
        if (config->format == FORMAT_JSON) {
            print_results_json(results);
        } else {
            print_results_table(results);  /* 使用表格格式输出 */
        }
    }

    /* 检查能力库声明的外部工具是否缺失，输出告警与安装指导 */
    report_missing_external_tools(caps, config->format);
    
    /* 清理资源 */
    judgement_result_list_free(results);
    capability_list_free(caps);
    plugin_env_list_free(env_data);

    logger_info(MODULE, "Check command completed");
    return ERR_OK;
}

psp_error_t cmd_enable(const cli_config_t *config)
{
    logger_info(MODULE, "Executing enable command for capability: %s", 
               config->capability_id ? config->capability_id : "unknown");
    
    if (!config->capability_id) {
        logger_error(MODULE, "No capability ID specified");
        return ERR_INVALID_PARAM;
    }

    /* 初始化插件系统并加载能力库 */
    plugin_env_list_t *env_data = NULL;
    capability_list_t *caps = NULL;
    psp_error_t ret = get_information_from_plugins(config, &env_data, &caps);
    if (ret != ERR_OK) {
        logger_error(MODULE, "Failed to get information from plugin");
        return ret;
    }
   
    /* 执行使能操作（内部会加载插件并判定状态） */
    command_t cmd_type = CMD_ENABLE;
    execution_result_t *result = executor_control(cmd_type, config->capability_id, env_data, caps);
    if (!result) {
        logger_error(MODULE, "Failed to execute enable operation");
        return ERR_GENERIC;
    }
    
    /* 输出结果 */
    print_execution_result(result, config->format);
    
    bool completed = result->operation_completed;

    /* 清理资源 */
    execution_result_free(result);
    capability_list_free(caps);
    plugin_env_list_free(env_data);

    logger_info(MODULE, "Enable command completed");
    return completed ? ERR_OK : ERR_EXECUTION;
}

psp_error_t cmd_disable(const cli_config_t *config)
{
    logger_info(MODULE, "Executing disable command for capability: %s", 
               config->capability_id ? config->capability_id : "unknown");
    
    if (!config->capability_id) {
        logger_error(MODULE, "No capability ID specified");
        printf("Error: No capability ID specified.\n");
        printf("Usage: plat-sec-patrol disable <CAPABILITY_ID>\n");
        return ERR_INVALID_PARAM;
    }
    
    /* 初始化插件系统并加载能力库 */
    plugin_env_list_t *env_data = NULL;
    capability_list_t *caps = NULL;
    psp_error_t ret = get_information_from_plugins(config, &env_data, &caps);
    if (ret != ERR_OK) {
        logger_error(MODULE, "Failed to get information from plugin");
        return ret;
    }
    
    command_t cmd_type = CMD_DISABLE;
    execution_result_t *result = executor_control(cmd_type, config->capability_id, env_data, caps);
    if (!result) {
        logger_error(MODULE, "Failed to execute disable operation");
        capability_list_free(caps);
        plugin_env_list_free(env_data);
        return ERR_GENERIC;
    }

    /* 输出结果 */
    print_execution_result(result, config->format);

    /* 在释放前保存结果 */
    bool completed = result->operation_completed;

    /* 清理资源 */
    capability_list_free(caps);
    execution_result_free(result);
    plugin_env_list_free(env_data);

    logger_info(MODULE, "Disable command completed for %s", config->capability_id);
    return completed ? ERR_OK : ERR_EXECUTION;
}

psp_error_t cmd_list(const cli_config_t *config)
{
    logger_info(MODULE, "Executing list command");
    
    /* 采集环境信息 */
    environment_t *env = base_collector_collect();
    if (!env) {
        logger_error(MODULE, "Failed to collect environment information");
        return ERR_GENERIC;
    }
    
    /* 创建插件管理器 */
    const char *plugin_dir = config->plugin_dir ? config->plugin_dir : DEFAULT_PLUGIN_DIR;
    plugin_manager_t *manager = extension_create_manager(plugin_dir);
    if (!manager) {
        environment_free(env);
        return ERR_MEMORY;
    }
    
    /* 加载插件 */
    psp_error_t ret = extension_load_plugins(manager, env);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "No plugins loaded");
    }
    
    /* 从插件加载能力库 */
    capability_list_t *caps = extension_load_capabilities(manager, env);
    if (!caps || caps->count == 0) {
        logger_warn(MODULE, "No capabilities loaded");
        printf("No capabilities available.\n");
        
        extension_free_manager(manager);
        environment_free(env);
        return ERR_NOT_FOUND;
    }
    
    /* 输出能力列表 */
    printf("\n=== Available Capabilities ===\n\n");
    printf("Total: %zu capabilities\n\n", caps->count);
    
    for (size_t i = 0; i < caps->count; i++) {
        capability_t *cap = caps->items[i];
        if (!cap) continue;
        
        printf("[%zu] %s\n", i + 1, cap->name ? cap->name : cap->id);
        printf("    ID: %s\n", cap->id ? cap->id : "unknown");
        
        if (cap->description) {
            printf("    Description: %s\n", cap->description);
        }
        
        if (cap->dependency_count > 0) {
            printf("    Dependencies: ");
            for (size_t j = 0; j < cap->dependency_count; j++) {
                printf("%s", cap->dependencies[j]);
                if (j < cap->dependency_count - 1) {
                    printf(", ");
                }
            }
            printf("\n");
        }
        
        printf("\n");
    }
    
    /* 清理资源 */
    capability_list_free(caps);
    extension_free_manager(manager);
    environment_free(env);
    
    logger_info(MODULE, "List command completed");
    return ERR_OK;
}

int cli_run(int argc, char **argv)
{
    /* 解析命令行参数 */
    cli_config_t config;
    psp_error_t ret = cli_parse_args(argc, argv, &config);
    if (ret != ERR_OK) {
        return EXIT_FAILURE;
    }
    
    /* 检查权限（需要 root） */
    if (config.command == CMD_CHECK ||
        config.command == CMD_ENABLE ||
        config.command == CMD_DISABLE) {
        if (geteuid() != 0) {
            logger_error(MODULE, "Operation requires root privileges");
            return ERR_PERMISSION;
        }
    }

    /* 初始化日志 */
    log_level_t log_level = config.verbose ? LOG_LEVEL_DEBUG : LOG_LEVEL_INFO;
    ret = logger_init(log_level, DEFAULT_LOG_DIR);
    if (ret != ERR_OK) {
        /* 如果无法写入日志目录，降级到 stderr */
        logger_set_level(log_level);
    }
    
    logger_info(MODULE, "CLI started, command=%d, verbose=%d", config.command, config.verbose);
    
    /* 执行对应命令 */
    switch (config.command) {
        case CMD_CHECK:
            ret = cmd_check(&config);
            break;
            
        case CMD_ENABLE:
            ret = cmd_enable(&config);
            break;
            
        case CMD_DISABLE:
            ret = cmd_disable(&config);
            break;
            
        case CMD_LIST:
            ret = cmd_list(&config);
            break;
            
        default:
            logger_error(MODULE, "Unknown command code: %d", config.command);
            ret = ERR_INVALID_PARAM;
    }
    
    /* 关闭日志 */
    logger_close();
    
    return (ret == ERR_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}
