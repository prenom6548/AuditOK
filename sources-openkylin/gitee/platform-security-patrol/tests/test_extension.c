/**
 * @file test_extension.c
 * @brief 插件扩展模块单元测试
 *
 * 覆盖范围（仅测试公共接口）：
 *   - extension_create_manager() / extension_free_manager(): 管理器生命周期
 *   - extension_scan_plugins(): 目录扫描
 *   - extension_load_plugins(): 插件加载（含平台匹配）
 *   - extension_unload_plugins(): 插件卸载
 *   - extension_invoke_collectors(): 环境采集调用
 *   - extension_load_capabilities(): 能力库加载
 *   - extension_get_plugin_type(): 类型查询
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "core/extension.h"
#include "plat_sec_patrol.h"
#include <stdlib.h>
#include <string.h>

// ==================== extension_create_manager 测试 ====================

static void test_create_manager_valid_dir(void)
{
    plugin_manager_t *mgr = extension_create_manager("/usr/lib/plugins");
    CU_ASSERT_PTR_NOT_NULL(mgr);
    CU_ASSERT_PTR_NOT_NULL(mgr->plugin_dir);
    CU_ASSERT_STRING_EQUAL(mgr->plugin_dir, "/usr/lib/plugins");
    CU_ASSERT_EQUAL(mgr->count, 0);
    CU_ASSERT_EQUAL(mgr->capacity, 0);
    CU_ASSERT_PTR_NULL(mgr->plugins);
    extension_free_manager(mgr);
}

static void test_create_manager_null_dir(void)
{
    plugin_manager_t *mgr = extension_create_manager(NULL);
    CU_ASSERT_PTR_NULL(mgr);
}

static void test_create_manager_empty_dir(void)
{
    plugin_manager_t *mgr = extension_create_manager("");
    CU_ASSERT_PTR_NOT_NULL(mgr);
    CU_ASSERT_STRING_EQUAL(mgr->plugin_dir, "");
    extension_free_manager(mgr);
}

// ==================== extension_free_manager 测试 ====================

static void test_free_manager_null(void)
{
    /* 不应崩溃 */
    extension_free_manager(NULL);
}

static void test_free_manager_valid(void)
{
    plugin_manager_t *mgr = extension_create_manager("/tmp/test");
    CU_ASSERT_PTR_NOT_NULL(mgr);
    extension_free_manager(mgr);
}

// ==================== extension_scan_plugins 测试 ====================

static void test_scan_plugins_null_dir(void)
{
    char **files = NULL;
    size_t count = 0;
    psp_error_t ret = extension_scan_plugins(NULL, &files, &count);
    CU_ASSERT_NOT_EQUAL(ret, ERR_OK);
}

static void test_scan_plugins_nonexistent_dir(void)
{
    char **files = NULL;
    size_t count = 0;
    psp_error_t ret = extension_scan_plugins("/nonexistent/plugin/dir", &files, &count);
    CU_ASSERT_NOT_EQUAL(ret, ERR_OK);
    CU_ASSERT_EQUAL(count, 0);
}

// ==================== extension_load_plugins 测试 ====================

static void test_load_plugins_null_manager(void)
{
    environment_t env;
    memset(&env, 0, sizeof(env));
    psp_error_t ret = extension_load_plugins(NULL, &env);
    CU_ASSERT_EQUAL(ret, ERR_INVALID_PARAM);
}

static void test_load_plugins_nonexistent_dir(void)
{
    plugin_manager_t *mgr = extension_create_manager("/nonexistent/dir");
    CU_ASSERT_PTR_NOT_NULL(mgr);

    environment_t env;
    memset(&env, 0, sizeof(env));
    env.cpu.vendor = "HygonGenuine";

    psp_error_t ret = extension_load_plugins(mgr, &env);
    CU_ASSERT_EQUAL(ret, ERR_NOT_FOUND);

    extension_free_manager(mgr);
}

// ==================== extension_unload_plugins 测试 ====================

static void test_unload_plugins_null_manager(void)
{
    psp_error_t ret = extension_unload_plugins(NULL);
    CU_ASSERT_EQUAL(ret, ERR_INVALID_PARAM);
}

static void test_unload_plugins_empty_manager(void)
{
    plugin_manager_t *mgr = extension_create_manager("/tmp");
    CU_ASSERT_PTR_NOT_NULL(mgr);

    psp_error_t ret = extension_unload_plugins(mgr);
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_EQUAL(mgr->count, 0);

    extension_free_manager(mgr);
}

// ==================== extension_invoke_collectors 测试 ====================

static void test_invoke_collectors_null_manager(void)
{
    plugin_env_list_t *env = NULL;
    psp_error_t ret = extension_invoke_collectors(NULL, &env);
    CU_ASSERT_EQUAL(ret, ERR_INVALID_PARAM);
}

static void test_invoke_collectors_null_output(void)
{
    plugin_manager_t *mgr = extension_create_manager("/tmp");
    psp_error_t ret = extension_invoke_collectors(mgr, NULL);
    CU_ASSERT_EQUAL(ret, ERR_INVALID_PARAM);
    extension_free_manager(mgr);
}

static void test_invoke_collectors_empty_manager(void)
{
    plugin_manager_t *mgr = extension_create_manager("/tmp");
    plugin_env_list_t *env = NULL;

    psp_error_t ret = extension_invoke_collectors(mgr, &env);
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_PTR_NOT_NULL(env);
    CU_ASSERT_EQUAL(env->count, 0);

    plugin_env_list_free(env);
    extension_free_manager(mgr);
}

// ==================== extension_load_capabilities 测试 ====================

static void test_load_capabilities_null_manager(void)
{
    capability_list_t *caps = extension_load_capabilities(NULL, NULL);
    CU_ASSERT_PTR_NULL(caps);
}

static void test_load_capabilities_empty_manager(void)
{
    plugin_manager_t *mgr = extension_create_manager("/tmp");
    environment_t env;
    memset(&env, 0, sizeof(env));

    capability_list_t *caps = extension_load_capabilities(mgr, &env);
    CU_ASSERT_PTR_NOT_NULL(caps);
    CU_ASSERT_EQUAL(caps->count, 0);

    capability_list_free(caps);
    extension_free_manager(mgr);
}

// ==================== extension_get_plugin_type 测试 ====================

static void test_get_plugin_type_null(void)
{
    plugin_type_t type = extension_get_plugin_type(NULL);
    CU_ASSERT_EQUAL(type, PLUGIN_TYPE_COLLECTOR);
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

    if (CU_initialize_registry() != CUE_SUCCESS) {
        return CU_get_error();
    }

    pSuite = CU_add_suite("Extension_Test", init_suite, clean_suite);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* create_manager */
    CU_add_test(pSuite, "create_manager_valid", test_create_manager_valid_dir);
    CU_add_test(pSuite, "create_manager_null", test_create_manager_null_dir);
    CU_add_test(pSuite, "create_manager_empty", test_create_manager_empty_dir);

    /* free_manager */
    CU_add_test(pSuite, "free_manager_null", test_free_manager_null);
    CU_add_test(pSuite, "free_manager_valid", test_free_manager_valid);

    /* scan_plugins */
    CU_add_test(pSuite, "scan_plugins_null_dir", test_scan_plugins_null_dir);
    CU_add_test(pSuite, "scan_plugins_nonexistent", test_scan_plugins_nonexistent_dir);

    /* load_plugins */
    CU_add_test(pSuite, "load_plugins_null_manager", test_load_plugins_null_manager);
    CU_add_test(pSuite, "load_plugins_nonexistent_dir", test_load_plugins_nonexistent_dir);

    /* unload_plugins */
    CU_add_test(pSuite, "unload_plugins_null", test_unload_plugins_null_manager);
    CU_add_test(pSuite, "unload_plugins_empty", test_unload_plugins_empty_manager);

    /* invoke_collectors */
    CU_add_test(pSuite, "invoke_collectors_null_manager", test_invoke_collectors_null_manager);
    CU_add_test(pSuite, "invoke_collectors_null_output", test_invoke_collectors_null_output);
    CU_add_test(pSuite, "invoke_collectors_empty", test_invoke_collectors_empty_manager);

    /* load_capabilities */
    CU_add_test(pSuite, "load_capabilities_null", test_load_capabilities_null_manager);
    CU_add_test(pSuite, "load_capabilities_empty", test_load_capabilities_empty_manager);

    /* get_plugin_type */
    CU_add_test(pSuite, "get_plugin_type_null", test_get_plugin_type_null);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}
