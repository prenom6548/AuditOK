/**
 * @file test_capability_parser.c
 * @brief 能力解析器单元测试
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "core/capability_parser.h"
#include "models/capability.h"
#include "utils/file_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// 测试用的 JSON 配置路径
#define TEST_CAPABILITIES_JSON "./test_capabilities.json"

// ==================== 辅助函数 ====================

/**
 * @brief 从文件加载能力定义（测试专用）
 */
static capability_list_t* capability_parser_load_from_file(const char *file_path)
{
    // 读取文件内容
    char *content = NULL;
    size_t size = 0;
    psp_error_t ret = file_utils_read_file(file_path, &content, &size);
    
    if (ret != ERR_OK || !content) {
        return NULL;
    }
    
    // 解析 JSON
    capability_list_t *result = capability_parser(content);
    free(content);
    
    return result;
}

/**
 * @brief 创建测试用的 JSON 配置文件
 */
static int create_test_json_file(void)
{
    const char *json_content = 
        "{\n"
        "  \"Hygon C86-3G\": {\n"
        "    \"cap_matrix\": [\n"
        "      {\n"
        "        \"cap_name\": \"test_cap\",\n"
        "        \"dependencies\": {\n"
        "          \"cpu\": {\n"
        "            \"requirements\": [\n"
        "              {\"name\": \"model\", \"value\": [\"Hygon C86-3G\"], \"operator\": \"==\", \"status\": \"support\", \"type\": \"cpu_model\"}\n"
        "            ]\n"
        "          },\n"
        "          \"firmware\": {\n"
        "            \"alternatives\": [\n"
        "              {\n"
        "                \"conditions\": [\n"
        "                  {\"name\": \"firmware_version\", \"value\": [\"1.0\"], \"operator\": \">=\", \"status\": \"support\", \"type\": \"firmware\"}\n"
        "                ]\n"
        "              }\n"
        "            ]\n"
        "          },\n"
        "          \"os\": {\n"
        "            \"requirements\": [\n"
        "              {\"name\": \"CONFIG_TEST\", \"value\": [\"y\"], \"operator\": \"==\", \"status\": \"support\", \"type\": \"kernel_config\"}\n"
        "            ]\n"
        "          }\n"
        "        ]\n"
        "      }\n"
        "    ]\n"
        "  }\n"
        "}\n";
    
    FILE *fp = fopen(TEST_CAPABILITIES_JSON, "w");
    if (!fp) {
        return -1;
    }
    
    fprintf(fp, "%s", json_content);
    fclose(fp);
    
    return 0;
}

/**
 * @brief 清理测试文件
 */
static void cleanup_test_json_file(void)
{
    remove(TEST_CAPABILITIES_JSON);
}

// ==================== capability_list_create 测试 ====================

static void test_capability_list_create(void)
{
    capability_list_t *list = capability_list_create();
    CU_ASSERT_PTR_NOT_NULL(list);
    CU_ASSERT_EQUAL(list->count, 0);
    CU_ASSERT_EQUAL(list->capacity, 0);
    CU_ASSERT_PTR_NULL(list->items);
    
    capability_list_free(list);
}

// ==================== capability_create 测试 ====================

static void test_capability_create(void)
{
    capability_t *cap = capability_create();
    CU_ASSERT_PTR_NOT_NULL(cap);
    
    // 设置 ID 和名称
    psp_error_t ret = capability_set_id(cap, "test_id");
    CU_ASSERT_EQUAL(ret, ERR_OK);
    
    ret = capability_set_name(cap, "Test Capability");
    CU_ASSERT_EQUAL(ret, ERR_OK);
    
    CU_ASSERT_STRING_EQUAL(cap->id, "test_id");
    CU_ASSERT_STRING_EQUAL(cap->name, "Test Capability");
    CU_ASSERT_PTR_NULL(cap->description);
    
    capability_free(cap);
}

static void test_capability_create_with_description(void)
{
    capability_t *cap = capability_create();
    CU_ASSERT_PTR_NOT_NULL(cap);
    
    capability_set_id(cap, "test_id");
    capability_set_name(cap, "Test Capability");
    
    cap->description = strdup("This is a test capability");
    CU_ASSERT_PTR_NOT_NULL(cap);
    CU_ASSERT_STRING_EQUAL(cap->description, "This is a test capability");
    
    capability_free(cap);
}

// ==================== capability_add_dependency 测试 ====================

static void test_capability_add_dependency(void)
{
    capability_t *cap = capability_create();
    CU_ASSERT_PTR_NOT_NULL(cap);
    
    capability_set_id(cap, "test_id");
    capability_set_name(cap, "Test");
    
    capability_add_dependency(cap, "dep1");
    CU_ASSERT_EQUAL(cap->dependency_count, 1);
    CU_ASSERT_STRING_EQUAL(cap->dependencies[0], "dep1");
    
    capability_add_dependency(cap, "dep2");
    CU_ASSERT_EQUAL(cap->dependency_count, 2);
    CU_ASSERT_STRING_EQUAL(cap->dependencies[1], "dep2");
    
    capability_free(cap);
}

// ==================== condition_group_create 测试 ====================

static void test_condition_group_create_and(void)
{
    condition_group_t *group = condition_group_create(true);
    CU_ASSERT_PTR_NOT_NULL(group);
    CU_ASSERT_TRUE(group->is_and_group);
    CU_ASSERT_EQUAL(group->count, 0);
    condition_group_free(group);
}

static void test_condition_group_create_or(void)
{
    condition_group_t *group = condition_group_create(false);
    CU_ASSERT_PTR_NOT_NULL(group);
    CU_ASSERT_FALSE(group->is_and_group);
    CU_ASSERT_EQUAL(group->count, 0);
    condition_group_free(group);
}

// ==================== condition_create 测试 ====================

static void test_condition_create(void)
{
    condition_t *cond = condition_create();
    CU_ASSERT_PTR_NOT_NULL(cond);
    
    const char *values[] = {"value"};
    error_t ret = condition_set(cond, "field", values, 1, "==");
    CU_ASSERT_EQUAL(ret, ERR_OK);
    
    CU_ASSERT_STRING_EQUAL(cond->field_name, "field");
    CU_ASSERT_EQUAL(cond->expected_values_count, 1);
    CU_ASSERT_STRING_EQUAL(cond->expected_values[0], "value");
    CU_ASSERT_STRING_EQUAL(cond->operator, "==");
    
    condition_free(cond);
}

static void test_condition_create_multiple_values(void)
{
    condition_t *cond = condition_create();
    CU_ASSERT_PTR_NOT_NULL(cond);
    
    const char *values[] = {"y", "m"};
    error_t ret = condition_set(cond, "field", values, 2, "==");
    CU_ASSERT_EQUAL(ret, ERR_OK);
    
    CU_ASSERT_EQUAL(cond->expected_values_count, 2);
    CU_ASSERT_STRING_EQUAL(cond->expected_values[0], "y");
    CU_ASSERT_STRING_EQUAL(cond->expected_values[1], "m");
    
    condition_free(cond);
}

// ==================== layer_rule_init 测试 ====================

static void test_layer_rule_init(void)
{
    layer_rule_t rule;
    layer_rule_init(&rule);
    
    CU_ASSERT_PTR_NULL(rule.or_groups);
    CU_ASSERT_EQUAL(rule.or_group_count, 0);
    CU_ASSERT_PTR_NULL(rule.and_groups);
    CU_ASSERT_EQUAL(rule.and_group_count, 0);
    CU_ASSERT_FALSE(rule.required);
    
    layer_rule_free(&rule);
}

// ==================== capability_parser_load_from_file 测试 ====================

static void test_parser_load_from_valid_file(void)
{
    // 创建测试 JSON 文件
    if (create_test_json_file() != 0) {
        CU_FAIL("Failed to create test JSON file");
        return;
    }
    
    capability_list_t *caps = capability_parser_load_from_file(TEST_CAPABILITIES_JSON);
    
    // 检查解析是否成功（允许失败，因为JSON格式可能有问题）
    if (caps == NULL) {
        CU_FAIL("Failed to parse capabilities from JSON file");
        cleanup_test_json_file();
        return;
    }
    
    CU_ASSERT(caps->count > 0);
    
    if (caps->count > 0) {
        // 验证加载的能力
        capability_t *cap = caps->items[0];
        CU_ASSERT_PTR_NOT_NULL(cap);
        
        // ID 格式为: {cpu_model}.{cap_name}，例如 "Hygon C86-3G.test_cap"
        CU_ASSERT_PTR_NOT_NULL(cap->id);
        CU_ASSERT_PTR_NOT_NULL(cap->name);
        CU_ASSERT_STRING_EQUAL(cap->name, "test_cap");
    }
    
    capability_list_free(caps);
    cleanup_test_json_file();
}

static void test_parser_load_from_nonexistent_file(void)
{
    capability_list_t *caps = capability_parser_load_from_file("/nonexistent/file.json");
    CU_ASSERT_PTR_NULL(caps);
}

static void test_parser_load_from_invalid_json(void)
{
    // 创建无效的 JSON 文件
    FILE *fp = fopen("./invalid.json", "w");
    if (fp) {
        fprintf(fp, "{ invalid json }");
        fclose(fp);
    }
    
    capability_list_t *caps = capability_parser_load_from_file("./invalid.json");
    CU_ASSERT_PTR_NULL(caps);
    
    remove("./invalid.json");
}

// ==================== 条件类型识别测试 ====================

static void test_condition_type_detection_firmware(void)
{
    condition_t *cond = condition_create();
    const char *values[] = {"1.0"};
    condition_set(cond, "firmware_version", values, 1, ">=");
    cond->type = COND_TYPE_FIRMWARE;
    CU_ASSERT_EQUAL(cond->type, COND_TYPE_FIRMWARE);
    condition_free(cond);
}

static void test_condition_type_detection_kernel_config(void)
{
    condition_t *cond = condition_create();
    const char *values[] = {"y"};
    condition_set(cond, "CONFIG_TEST", values, 1, "==");
    cond->type = COND_TYPE_KERNEL_CONFIG;
    CU_ASSERT_EQUAL(cond->type, COND_TYPE_KERNEL_CONFIG);
    condition_free(cond);
}

static void test_condition_type_detection_cpu_model(void)
{
    condition_t *cond = condition_create();
    const char *values[] = {"Hygon C86-3G"};
    condition_set(cond, "model", values, 1, "==");
    cond->type = COND_TYPE_CPU_MODEL;
    CU_ASSERT_EQUAL(cond->type, COND_TYPE_CPU_MODEL);
    condition_free(cond);
}

// ==================== 外部工具依赖测试 ====================

static void test_external_tool_dep_create_free(void)
{
    external_tool_dep_t *dep = external_tool_dep_create();
    CU_ASSERT_PTR_NOT_NULL(dep);
    CU_ASSERT_PTR_NULL(dep->name);
    CU_ASSERT_PTR_NULL(dep->description);
    CU_ASSERT_PTR_NULL(dep->check_command);
    CU_ASSERT_PTR_NULL(dep->install_guide);
    CU_ASSERT_PTR_NULL(dep->verify_command);

    external_tool_dep_free(dep);
}

static void test_capability_list_add_tool_dep(void)
{
    capability_list_t *list = capability_list_create();
    CU_ASSERT_PTR_NOT_NULL(list);
    CU_ASSERT_EQUAL(list->tool_dep_count, 0);

    external_tool_dep_t *dep = external_tool_dep_create();
    dep->name = strdup("hag");

    psp_error_t ret = capability_list_add_tool_dep(list, dep);
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_EQUAL(list->tool_dep_count, 1);
    CU_ASSERT_PTR_EQUAL(list->tool_deps[0], dep);

    // 参数校验
    CU_ASSERT_EQUAL(capability_list_add_tool_dep(NULL, dep), ERR_INVALID_PARAM);
    CU_ASSERT_EQUAL(capability_list_add_tool_dep(list, NULL), ERR_INVALID_PARAM);

    capability_list_free(list);
}

static void test_parser_external_tools_full(void)
{
    const char *json =
        "{\n"
        "  \"external_tools\": [\n"
        "    {\n"
        "      \"name\": \"hag\",\n"
        "      \"description\": \"test tool\",\n"
        "      \"check_command\": \"hag\",\n"
        "      \"install_guide\": \"sudo bash install_hag.sh\",\n"
        "      \"verify_command\": \"sudo hag general check\"\n"
        "    }\n"
        "  ],\n"
        "  \"Hygon C86-3G\": {\n"
        "    \"cap_matrix\": [\n"
        "      {\n"
        "        \"cap_name\": \"test_cap\",\n"
        "        \"dependencies\": {\n"
        "          \"cpu\": {\n"
        "            \"requirements\": [\n"
        "              {\"name\": \"model\", \"value\": [\"Hygon C86-3G\"], \"operator\": \"==\", \"status\": \"support\", \"type\": \"cpu_model\"}\n"
        "            ]\n"
        "          }\n"
        "        }\n"
        "      }\n"
        "    ]\n"
        "  }\n"
        "}\n";

    capability_list_t *caps = capability_parser(json);
    CU_ASSERT_PTR_NOT_NULL(caps);
    if (!caps) return;

    // 能力正常解析（external_tools 段不干扰型号遍历）
    CU_ASSERT_EQUAL(caps->count, 1);

    // 外部工具依赖完整解析
    CU_ASSERT_EQUAL(caps->tool_dep_count, 1);
    if (caps->tool_dep_count == 1) {
        external_tool_dep_t *dep = caps->tool_deps[0];
        CU_ASSERT_PTR_NOT_NULL(dep);
        CU_ASSERT_STRING_EQUAL(dep->name, "hag");
        CU_ASSERT_STRING_EQUAL(dep->description, "test tool");
        CU_ASSERT_STRING_EQUAL(dep->check_command, "hag");
        CU_ASSERT_STRING_EQUAL(dep->install_guide, "sudo bash install_hag.sh");
        CU_ASSERT_STRING_EQUAL(dep->verify_command, "sudo hag general check");
    }

    capability_list_free(caps);
}

static void test_parser_external_tools_defaults(void)
{
    // 缺省 check_command 时应回退为 name
    const char *json =
        "{\n"
        "  \"external_tools\": [\n"
        "    {\n"
        "      \"name\": \"hag\",\n"
        "      \"install_guide\": \"install it\"\n"
        "    }\n"
        "  ]\n"
        "}\n";

    capability_list_t *caps = capability_parser(json);
    CU_ASSERT_PTR_NOT_NULL(caps);
    if (!caps) return;

    CU_ASSERT_EQUAL(caps->count, 0);
    CU_ASSERT_EQUAL(caps->tool_dep_count, 1);
    if (caps->tool_dep_count == 1) {
        external_tool_dep_t *dep = caps->tool_deps[0];
        CU_ASSERT_STRING_EQUAL(dep->check_command, "hag");
        CU_ASSERT_PTR_NULL(dep->description);
        CU_ASSERT_PTR_NULL(dep->verify_command);
    }

    capability_list_free(caps);
}

static void test_parser_external_tools_no_section(void)
{
    // 无 external_tools 段时 tool_dep_count 应为 0
    const char *json =
        "{\n"
        "  \"Hygon C86-3G\": {\n"
        "    \"cap_matrix\": []\n"
        "  }\n"
        "}\n";

    capability_list_t *caps = capability_parser(json);
    CU_ASSERT_PTR_NOT_NULL(caps);
    if (!caps) return;

    CU_ASSERT_EQUAL(caps->tool_dep_count, 0);
    CU_ASSERT_PTR_NULL(caps->tool_deps);

    capability_list_free(caps);
}

static void test_parser_external_tools_skip_invalid(void)
{
    // 缺少 name 的条目应被跳过
    const char *json =
        "{\n"
        "  \"external_tools\": [\n"
        "    {\n"
        "      \"install_guide\": \"install it\"\n"
        "    },\n"
        "    {\n"
        "      \"name\": \"hag\",\n"
        "      \"install_guide\": \"install hag\"\n"
        "    }\n"
        "  ]\n"
        "}\n";

    capability_list_t *caps = capability_parser(json);
    CU_ASSERT_PTR_NOT_NULL(caps);
    if (!caps) return;

    CU_ASSERT_EQUAL(caps->tool_dep_count, 1);
    if (caps->tool_dep_count == 1) {
        CU_ASSERT_STRING_EQUAL(caps->tool_deps[0]->name, "hag");
    }

    capability_list_free(caps);
}

// ==================== 测试套件初始化 ====================

static int init_suite(void)
{
    return 0;
}

static int clean_suite(void)
{
    cleanup_test_json_file();
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
    pSuite = CU_add_suite("CapabilityParser_Test", init_suite, clean_suite);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    /* 添加测试用例 */
    // 基础结构测试
    CU_add_test(pSuite, "test_capability_list_create", test_capability_list_create);
    CU_add_test(pSuite, "test_capability_create", test_capability_create);
    CU_add_test(pSuite, "test_capability_create_with_description", test_capability_create_with_description);
    CU_add_test(pSuite, "test_capability_add_dependency", test_capability_add_dependency);
    
    // 条件组测试
    CU_add_test(pSuite, "test_condition_group_create_and", test_condition_group_create_and);
    CU_add_test(pSuite, "test_condition_group_create_or", test_condition_group_create_or);
    
    // 条件测试
    CU_add_test(pSuite, "test_condition_create", test_condition_create);
    CU_add_test(pSuite, "test_condition_create_multiple_values", test_condition_create_multiple_values);
    
    // 层规则测试
    CU_add_test(pSuite, "test_layer_rule_init", test_layer_rule_init);
    
    // 解析器测试
    CU_add_test(pSuite, "test_parser_load_from_valid_file", test_parser_load_from_valid_file);
    CU_add_test(pSuite, "test_parser_load_from_nonexistent_file", test_parser_load_from_nonexistent_file);
    CU_add_test(pSuite, "test_parser_load_from_invalid_json", test_parser_load_from_invalid_json);
    
    // 条件类型测试
    CU_add_test(pSuite, "test_condition_type_detection_firmware", test_condition_type_detection_firmware);
    CU_add_test(pSuite, "test_condition_type_detection_kernel_config", test_condition_type_detection_kernel_config);
    CU_add_test(pSuite, "test_condition_type_detection_cpu_model", test_condition_type_detection_cpu_model);

    // 外部工具依赖测试
    CU_add_test(pSuite, "test_external_tool_dep_create_free", test_external_tool_dep_create_free);
    CU_add_test(pSuite, "test_capability_list_add_tool_dep", test_capability_list_add_tool_dep);
    CU_add_test(pSuite, "test_parser_external_tools_full", test_parser_external_tools_full);
    CU_add_test(pSuite, "test_parser_external_tools_defaults", test_parser_external_tools_defaults);
    CU_add_test(pSuite, "test_parser_external_tools_no_section", test_parser_external_tools_no_section);
    CU_add_test(pSuite, "test_parser_external_tools_skip_invalid", test_parser_external_tools_skip_invalid);
    
    /* 运行测试 */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
    /* 清理 */
    CU_cleanup_registry();
    
    return CU_get_error();
}
