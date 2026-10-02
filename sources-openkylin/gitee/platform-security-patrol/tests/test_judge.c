/**
 * @file test_judge.c
 * @brief 能力判定模块单元测试
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "core/judge.h"
#include "models/capability.h"
#include "models/environment.h"
#include <stdlib.h>
#include <string.h>

// ==================== 辅助函数 ====================

/**
 * @brief 创建测试用的环境数据列表
 */
static plugin_env_list_t* create_test_env_list(void)
{
    plugin_env_list_t *env = plugin_env_list_create();
    
    // 添加 CPU 信息
    plugin_env_list_add(env, "vendor", "HygonGenuine", "cpu_collector");
    plugin_env_list_add(env, "model", "Hygon C86-3G", "cpu_collector");
    plugin_env_list_add(env, "flags", "svm sse sse2", "cpu_collector");
    
    // 添加固件信息
    plugin_env_list_add(env, "firmware_version", "1.2.2", "firmware_collector");
    plugin_env_list_add(env, "bios_vendor", "OEM", "firmware_collector");
    
    // 添加 OS 信息
    plugin_env_list_add(env, "CONFIG_TEST_FEATURE", "y", "os_collector");
    plugin_env_list_add(env, "kernel_version", "5.10.0", "os_collector");
    
    return env;
}

/**
 * @brief 创建测试用的能力定义
 */
static capability_t* create_test_capability(void)
{
    capability_t *cap = capability_create();
    
    capability_set_id(cap, "test_cap");
    capability_set_name(cap, "Test Capability");
    
    // CPU 层规则 - requirements
    layer_rule_t *cpu_rule = &cap->cpu_rule;
    condition_group_t *cpu_group = condition_group_create(true); // AND group
    
    condition_t *cpu_cond = condition_create();
    const char *cpu_values[] = {"Hygon C86-3G"};
    condition_set(cpu_cond, "model", cpu_values, 1, "==");
    cpu_cond->type = COND_TYPE_CPU_MODEL;
    condition_group_add(cpu_group, cpu_cond);
    layer_rule_add_and_group(cpu_rule, cpu_group);
    
    // Firmware 层规则 - alternatives (OR groups)
    layer_rule_t *fw_rule = &cap->firmware_rule;
    condition_group_t *fw_group1 = condition_group_create(true);
    
    condition_t *fw_cond1 = condition_create();
    const char *fw_values[] = {"1.2.0"};
    condition_set(fw_cond1, "firmware_version", fw_values, 1, ">=");
    fw_cond1->type = COND_TYPE_FIRMWARE;
    condition_group_add(fw_group1, fw_cond1);
    layer_rule_add_or_group(fw_rule, fw_group1);
    
    // OS 层规则 - requirements
    layer_rule_t *os_rule = &cap->os_rule;
    condition_group_t *os_group = condition_group_create(true);
    
    condition_t *os_cond = condition_create();
    const char *os_values[] = {"y"};
    condition_set(os_cond, "CONFIG_TEST_FEATURE", os_values, 1, "==");
    os_cond->type = COND_TYPE_KERNEL_CONFIG;
    condition_group_add(os_group, os_cond);
    layer_rule_add_and_group(os_rule, os_group);
    
    return cap;
}

// ==================== judgement_result_create 测试 ====================

static void test_judgement_result_create(void)
{
    judgement_result_t *result = judgement_result_create();
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_EQUAL(result->overall_status, CAP_STATUS_UNKNOWN);
    CU_ASSERT_EQUAL(result->cpu_status, LAYER_STATUS_UNKNOWN);
    CU_ASSERT_EQUAL(result->firmware_status, LAYER_STATUS_UNKNOWN);
    CU_ASSERT_EQUAL(result->os_status, LAYER_STATUS_UNKNOWN);
    judgement_result_free(result);
}

// ==================== judge_single_capability 测试 ====================

/**
 * @brief 辅助函数：判定单个能力的所有层
 */
static judgement_result_t* judge_single_capability(capability_t *cap, plugin_env_list_t *env)
{
    judgement_result_t *result = judgement_result_create();
    
    judge_cpu_layer(env, cap, result);
    judge_firmware_layer(env, cap, result);
    judge_os_layer(env, cap, result);
    
    return result;
}

static void test_judge_capability_all_supported(void)
{
    capability_t *cap = create_test_capability();
    plugin_env_list_t *env = create_test_env_list();
    
    // 创建判定结果对象
    judgement_result_t *result = judgement_result_create();
    
    // 分别判定各层状态
    judge_cpu_layer(env, cap, result);
    judge_firmware_layer(env, cap, result);
    judge_os_layer(env, cap, result);
    
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_EQUAL(result->overall_status, CAP_STATUS_AVAILABLE);
    CU_ASSERT_EQUAL(result->cpu_status, LAYER_STATUS_SUPPORTED);
    CU_ASSERT_EQUAL(result->firmware_status, LAYER_STATUS_SUPPORTED);
    CU_ASSERT_EQUAL(result->os_status, LAYER_STATUS_SUPPORTED);
    
    judgement_result_free(result);
    capability_free(cap);
    plugin_env_list_free(env);
}

static void test_judge_capability_cpu_unsupported(void)
{
    capability_t *cap = create_test_capability();
    plugin_env_list_t *env = create_test_env_list();
    
    // 修改 CPU model 为不支持的值
    for (size_t i = 0; i < env->count; i++) {
        if (strcmp(env->items[i]->key, "model") == 0) {
            free((void*)env->items[i]->value);
            env->items[i]->value = strdup("Unsupported CPU");
            break;
        }
    }
    
    judgement_result_t *result = judge_single_capability(cap, env);
    
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_EQUAL(result->cpu_status, LAYER_STATUS_UNSUPPORTED);
    CU_ASSERT_EQUAL(result->overall_status, CAP_STATUS_UNSUPPORTED);
    
    judgement_result_free(result);
    capability_free(cap);
    plugin_env_list_free(env);
}

static void test_judge_capability_firmware_disabled(void)
{
    capability_t *cap = create_test_capability();
    plugin_env_list_t *env = create_test_env_list();
    
    // 修改 firmware_version 为禁用的值
    for (size_t i = 0; i < env->count; i++) {
        if (strcmp(env->items[i]->key, "firmware_version") == 0) {
            free((void*)env->items[i]->value);
            env->items[i]->value = strdup("disabled");
            break;
        }
    }
    
    judgement_result_t *result = judge_single_capability(cap, env);
    
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_EQUAL(result->firmware_status, LAYER_STATUS_DISABLED);
    CU_ASSERT_EQUAL(result->overall_status, CAP_STATUS_DISABLED);
    
    judgement_result_free(result);
    capability_free(cap);
    plugin_env_list_free(env);
}

static void test_judge_capability_os_missing_config(void)
{
    capability_t *cap = create_test_capability();
    plugin_env_list_t *env = create_test_env_list();
    
    // 移除 CONFIG_TEST_FEATURE
    for (size_t i = 0; i < env->count; i++) {
        if (strcmp(env->items[i]->key, "CONFIG_TEST_FEATURE") == 0) {
            // 手动释放 item 的内存
            free(env->items[i]->key);
            free(env->items[i]->value);
            free(env->items[i]->source);
            free(env->items[i]);
            
            // 移动后续元素
            for (size_t j = i; j < env->count - 1; j++) {
                env->items[j] = env->items[j + 1];
            }
            env->count--;
            break;
        }
    }
    
    judgement_result_t *result = judge_single_capability(cap, env);
    
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_EQUAL(result->os_status, LAYER_STATUS_UNSUPPORTED);
    CU_ASSERT_EQUAL(result->overall_status, CAP_STATUS_UNAVAILABLE);
    
    judgement_result_free(result);
    capability_free(cap);
    plugin_env_list_free(env);
}

// ==================== judge_specific_capability 测试 ====================

static void test_judge_specific_capability_found(void)
{
    capability_list_t *caps = capability_list_create();
    capability_t *cap = create_test_capability();
    capability_list_add(caps, cap);
    
    plugin_env_list_t *env = create_test_env_list();
    
    // 查找特定能力
    capability_t *found_cap = capability_list_find_by_id(caps, "test_cap");
    CU_ASSERT_PTR_NOT_NULL(found_cap);
    
    if (found_cap) {
        // 判定该能力
        judgement_result_t *result = judge_single_capability(found_cap, env);
        
        CU_ASSERT_PTR_NOT_NULL(result);
        CU_ASSERT_STRING_EQUAL(found_cap->id, "test_cap");
        
        judgement_result_free(result);
    }
    
    capability_list_free(caps);
    plugin_env_list_free(env);
}

static void test_judge_specific_capability_not_found(void)
{
    capability_list_t *caps = capability_list_create();
    plugin_env_list_t *env = create_test_env_list();
    
    // 查找不存在的能力
    capability_t *found_cap = capability_list_find_by_id(caps, "nonexistent_cap");
    CU_ASSERT_PTR_NULL(found_cap);
    
    capability_list_free(caps);
    plugin_env_list_free(env);
}

// ==================== 版本比较测试（通过 judge 公共接口间接覆盖） ====================

/**
 * @brief 固件版本不满足 >= 条件时，firmware_layer 应为 UNSUPPORTED
 *
 * 间接测试 judge.c 中 static compare_versions() 的 < 分支：
 *   系统固件 "1.1.0" < 要求 "1.2.0" → 条件不满足
 */
static void test_judge_firmware_version_insufficient(void)
{
    capability_t *cap = create_test_capability();
    plugin_env_list_t *env = create_test_env_list();

    /* 将固件版本改为低于要求的值（要求 >= 1.2.0，实际 1.1.0） */
    for (size_t i = 0; i < env->count; i++) {
        if (strcmp(env->items[i]->key, "firmware_version") == 0) {
            free((void*)env->items[i]->value);
            env->items[i]->value = strdup("1.1.0");
            break;
        }
    }

    judgement_result_t *result = judgement_result_create();
    judge_firmware_layer(env, cap, result);

    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_EQUAL(result->firmware_status, LAYER_STATUS_UNSUPPORTED);

    judgement_result_free(result);
    capability_free(cap);
    plugin_env_list_free(env);
}

/**
 * @brief 固件版本恰好等于要求值时，>= 应通过
 *
 * 间接测试 compare_versions() 的 == 分支：
 *   系统固件 "1.2.0" == 要求 "1.2.0" → 条件满足
 */
static void test_judge_firmware_version_exact_match(void)
{
    capability_t *cap = create_test_capability();
    plugin_env_list_t *env = create_test_env_list();

    for (size_t i = 0; i < env->count; i++) {
        if (strcmp(env->items[i]->key, "firmware_version") == 0) {
            free((void*)env->items[i]->value);
            env->items[i]->value = strdup("1.2.0");
            break;
        }
    }

    judgement_result_t *result = judgement_result_create();
    judge_firmware_layer(env, cap, result);

    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_EQUAL(result->firmware_status, LAYER_STATUS_SUPPORTED);

    judgement_result_free(result);
    capability_free(cap);
    plugin_env_list_free(env);
}

/**
 * @brief OS 层版本比较：内核版本 >= 要求时通过
 *
 * 构造一个带 >= 操作符的 OS 条件，间接测试 compare_versions() 的 > 分支
 */
static void test_judge_os_version_comparison_pass(void)
{
    /* 构建带版本比较条件的 capability */
    capability_t *cap = capability_create();
    capability_set_id(cap, "version_test_cap");
    capability_set_name(cap, "Version Test");

    layer_rule_t *os_rule = &cap->os_rule;
    condition_group_t *os_group = condition_group_create(true);

    condition_t *ver_cond = condition_create();
    const char *ver_values[] = {"5.4.0"};
    condition_set(ver_cond, "kernel_version", ver_values, 1, ">=");
    ver_cond->type = COND_TYPE_FIRMWARE;
    condition_group_add(os_group, ver_cond);
    layer_rule_add_and_group(os_rule, os_group);

    /* 环境：内核版本 5.10.0 > 要求 5.4.0 */
    plugin_env_list_t *env = plugin_env_list_create();
    plugin_env_list_add(env, "kernel_version", "5.10.0", "os_collector");

    judgement_result_t *result = judgement_result_create();
    judge_os_layer(env, cap, result);

    CU_ASSERT_EQUAL(result->os_status, LAYER_STATUS_SUPPORTED);

    judgement_result_free(result);
    capability_free(cap);
    plugin_env_list_free(env);
}

/**
 * @brief OS 层版本比较：内核版本 < 要求时不通过
 */
static void test_judge_os_version_comparison_fail(void)
{
    capability_t *cap = capability_create();
    capability_set_id(cap, "version_test_cap");
    capability_set_name(cap, "Version Test");

    layer_rule_t *os_rule = &cap->os_rule;
    condition_group_t *os_group = condition_group_create(true);

    condition_t *ver_cond = condition_create();
    const char *ver_values[] = {"5.15.0"};
    condition_set(ver_cond, "kernel_version", ver_values, 1, ">=");
    ver_cond->type = COND_TYPE_FIRMWARE;
    condition_group_add(os_group, ver_cond);
    layer_rule_add_and_group(os_rule, os_group);

    /* 环境：内核版本 5.10.0 < 要求 5.15.0 */
    plugin_env_list_t *env = plugin_env_list_create();
    plugin_env_list_add(env, "kernel_version", "5.10.0", "os_collector");

    judgement_result_t *result = judgement_result_create();
    judge_os_layer(env, cap, result);

    CU_ASSERT_EQUAL(result->os_status, LAYER_STATUS_UNSUPPORTED);

    judgement_result_free(result);
    capability_free(cap);
    plugin_env_list_free(env);
}

// ==================== 状态转换函数测试 ====================

static void test_cap_status_to_string(void)
{
    CU_ASSERT_STRING_EQUAL(cap_status_to_string(CAP_STATUS_AVAILABLE), "available");
    CU_ASSERT_STRING_EQUAL(cap_status_to_string(CAP_STATUS_UNSUPPORTED), "unsupported");
    CU_ASSERT_STRING_EQUAL(cap_status_to_string(CAP_STATUS_DISABLED), "disabled");
    CU_ASSERT_STRING_EQUAL(cap_status_to_string(CAP_STATUS_UNAVAILABLE), "unavailable");
    CU_ASSERT_STRING_EQUAL(cap_status_to_string(CAP_STATUS_UNKNOWN), "unknown");
}

static void test_layer_status_to_string(void)
{
    CU_ASSERT_STRING_EQUAL(layer_status_to_string(LAYER_STATUS_SUPPORTED), "supported");
    CU_ASSERT_STRING_EQUAL(layer_status_to_string(LAYER_STATUS_UNSUPPORTED), "unsupported");
    CU_ASSERT_STRING_EQUAL(layer_status_to_string(LAYER_STATUS_DISABLED), "disabled");
    CU_ASSERT_STRING_EQUAL(layer_status_to_string(LAYER_STATUS_UNKNOWN), "unknown");
}

// ==================== 测试套件初始化 ====================

static int init_suite(void)
{
    return 0;
}

static int clean_suite(void)
{
    return 0;
}

// ==================== 主函数 ====================

int main(void)
{
    CU_pSuite pSuite = NULL;
    
    /* 初始化 CUnit 测试注册表 */
    if (CU_initialize_registry() != CUE_SUCCESS) {
        return CU_get_error();
    }
    
    /* 添加测试套件 */
    pSuite = CU_add_suite("Judge_Test", init_suite, clean_suite);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    /* 添加测试用例 */
    // 基础结构测试
    CU_add_test(pSuite, "test_judgement_result_create", test_judgement_result_create);
    
    // 判定逻辑测试
    CU_add_test(pSuite, "test_judge_capability_all_supported", test_judge_capability_all_supported);
    CU_add_test(pSuite, "test_judge_capability_cpu_unsupported", test_judge_capability_cpu_unsupported);
    CU_add_test(pSuite, "test_judge_capability_firmware_disabled", test_judge_capability_firmware_disabled);
    CU_add_test(pSuite, "test_judge_capability_os_missing_config", test_judge_capability_os_missing_config);
    
    // 特定能力判定测试
    CU_add_test(pSuite, "test_judge_specific_capability_found", test_judge_specific_capability_found);
    CU_add_test(pSuite, "test_judge_specific_capability_not_found", test_judge_specific_capability_not_found);

    // 版本比较测试（通过 judge 公共接口间接覆盖）
    CU_add_test(pSuite, "test_judge_firmware_version_insufficient", test_judge_firmware_version_insufficient);
    CU_add_test(pSuite, "test_judge_firmware_version_exact_match", test_judge_firmware_version_exact_match);
    CU_add_test(pSuite, "test_judge_os_version_comparison_pass", test_judge_os_version_comparison_pass);
    CU_add_test(pSuite, "test_judge_os_version_comparison_fail", test_judge_os_version_comparison_fail);

    // 状态转换测试
    CU_add_test(pSuite, "test_cap_status_to_string", test_cap_status_to_string);
    CU_add_test(pSuite, "test_layer_status_to_string", test_layer_status_to_string);
    
    /* 运行测试 */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
    /* 清理 */
    CU_cleanup_registry();
    
    return CU_get_error();
}
