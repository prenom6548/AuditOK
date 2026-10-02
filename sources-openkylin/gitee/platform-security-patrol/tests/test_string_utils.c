/**
 * @file test_string_utils.c
 * @brief 字符串工具函数单元测试
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "utils/string_utils.h"
#include <stdlib.h>
#include <string.h>

// ==================== trim 函数测试 ====================

static void test_trim_basic(void)
{
    char str[] = "  hello world  ";
    char *result = string_utils_trim(str);
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_STRING_EQUAL(result, "hello world");
}

static void test_trim_no_whitespace(void)
{
    char str[] = "hello";
    char *result = string_utils_trim(str);
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_STRING_EQUAL(result, "hello");
}

static void test_trim_only_whitespace(void)
{
    char str[] = "   ";
    char *result = string_utils_trim(str);
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_STRING_EQUAL(result, "");
}

static void test_trim_empty_string(void)
{
    char str[] = "";
    char *result = string_utils_trim(str);
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_STRING_EQUAL(result, "");
}

static void test_trim_null_input(void)
{
    char *result = string_utils_trim(NULL);
    CU_ASSERT_PTR_NULL(result);
}

static void test_trim_tabs_and_newlines(void)
{
    char str[] = "\t\n  hello  \n\t";
    char *result = string_utils_trim(str);
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_STRING_EQUAL(result, "hello");
}

// ==================== concat 函数测试 ====================

static void test_concat_basic(void)
{
    char *result = string_utils_concat("hello", " world");
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_STRING_EQUAL(result, "hello world");
    free(result);
}

static void test_concat_null_params(void)
{
    char *result = string_utils_concat(NULL, "world");
    CU_ASSERT_PTR_NULL(result);
    
    result = string_utils_concat("hello", NULL);
    CU_ASSERT_PTR_NULL(result);
}

static void test_concat_empty_strings(void)
{
    char *result = string_utils_concat("", "");
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_STRING_EQUAL(result, "");
    free(result);
}

// ==================== starts_with 函数测试 ====================

static void test_starts_with_true(void)
{
    CU_ASSERT_TRUE(string_utils_starts_with("hello world", "hello"));
}

static void test_starts_with_false(void)
{
    CU_ASSERT_FALSE(string_utils_starts_with("hello world", "world"));
}

static void test_starts_with_empty_prefix(void)
{
    CU_ASSERT_TRUE(string_utils_starts_with("hello", ""));
}

static void test_starts_with_null_string(void)
{
    CU_ASSERT_FALSE(string_utils_starts_with(NULL, "test"));
}

static void test_starts_with_null_prefix(void)
{
    CU_ASSERT_FALSE(string_utils_starts_with("test", NULL));
}

// ==================== contains 函数测试 ====================

static void test_contains_true(void)
{
    CU_ASSERT_TRUE(string_utils_contains("hello world", "lo wo"));
}

static void test_contains_false(void)
{
    CU_ASSERT_FALSE(string_utils_contains("hello world", "xyz"));
}

static void test_contains_empty_substring(void)
{
    CU_ASSERT_TRUE(string_utils_contains("hello", ""));
}

static void test_contains_null_string(void)
{
    CU_ASSERT_FALSE(string_utils_contains(NULL, "test"));
}

static void test_contains_null_substring(void)
{
    CU_ASSERT_FALSE(string_utils_contains("test", NULL));
}

// ==================== split 函数测试 ====================

static void test_split_basic(void)
{
    char **tokens = NULL;
    size_t count = 0;
    psp_error_t ret = string_utils_split("hello,world,test", ",", &tokens, &count);
    
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_STRING_EQUAL(tokens[0], "hello");
    CU_ASSERT_STRING_EQUAL(tokens[1], "world");
    CU_ASSERT_STRING_EQUAL(tokens[2], "test");
    
    string_utils_free_array(tokens, count);
}

static void test_split_single_item(void)
{
    char **tokens = NULL;
    size_t count = 0;
    psp_error_t ret = string_utils_split("hello", ",", &tokens, &count);
    
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_STRING_EQUAL(tokens[0], "hello");
    
    string_utils_free_array(tokens, count);
}

static void test_split_empty(void)
{
    char **tokens = NULL;
    size_t count = 0;
    psp_error_t ret = string_utils_split("", ",", &tokens, &count);
    
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_EQUAL(count, 0);
    
    if (tokens) {
        string_utils_free_array(tokens, count);
    }
}

static void test_split_null_input(void)
{
    char **tokens = NULL;
    size_t count = 0;
    psp_error_t ret = string_utils_split(NULL, ",", &tokens, &count);
    CU_ASSERT_NOT_EQUAL(ret, ERR_OK);
}

// ==================== strdup 函数测试 ====================

static void test_strdup_basic(void)
{
    char *result = string_utils_strdup("hello world");
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_STRING_EQUAL(result, "hello world");
    free(result);
}

static void test_strdup_null(void)
{
    char *result = string_utils_strdup(NULL);
    CU_ASSERT_PTR_NULL(result);
}

static void test_strdup_empty(void)
{
    char *result = string_utils_strdup("");
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_STRING_EQUAL(result, "");
    free(result);
}

// ==================== atoi 函数测试 ====================

static void test_atoi_positive(void)
{
    int value = 0;
    psp_error_t ret = string_utils_atoi("123", &value);
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_EQUAL(value, 123);
}

static void test_atoi_negative(void)
{
    int value = 0;
    psp_error_t ret = string_utils_atoi("-456", &value);
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_EQUAL(value, -456);
}

static void test_atoi_zero(void)
{
    int value = 0;
    psp_error_t ret = string_utils_atoi("0", &value);
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_EQUAL(value, 0);
}

static void test_atoi_invalid(void)
{
    int value = 0;
    psp_error_t ret = string_utils_atoi("abc", &value);
    CU_ASSERT_NOT_EQUAL(ret, ERR_OK);
}

static void test_atoi_null_param(void)
{
    psp_error_t ret = string_utils_atoi("123", NULL);
    CU_ASSERT_NOT_EQUAL(ret, ERR_OK);
}

// ==================== snprintf 函数测试 ====================

static void test_snprintf_basic(void)
{
    char buffer[64];
    int ret = string_utils_snprintf(buffer, sizeof(buffer), "Hello %s", "World");
    CU_ASSERT(ret > 0);
    CU_ASSERT_STRING_EQUAL(buffer, "Hello World");
}

static void test_snprintf_overflow(void)
{
    char buffer[10];
    int ret = string_utils_snprintf(buffer, sizeof(buffer), "This is a very long string");
    CU_ASSERT(ret >= 0);
    // 应该被截断但仍然以 '\0' 结尾
    CU_ASSERT(strlen(buffer) < sizeof(buffer));
}

// ==================== strncpy 函数测试 ====================

static void test_strncpy_basic(void)
{
    char dest[20];
    psp_error_t ret = string_utils_strncpy(dest, "hello", sizeof(dest));
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_STRING_EQUAL(dest, "hello");
}

static void test_strncpy_overflow(void)
{
    char dest[5];
    psp_error_t ret = string_utils_strncpy(dest, "hello world", sizeof(dest));
    CU_ASSERT_NOT_EQUAL(ret, ERR_OK);
}

static void test_strncpy_null_params(void)
{
    char dest[20];
    psp_error_t ret = string_utils_strncpy(NULL, "hello", sizeof(dest));
    CU_ASSERT_NOT_EQUAL(ret, ERR_OK);
    
    ret = string_utils_strncpy(dest, NULL, sizeof(dest));
    CU_ASSERT_NOT_EQUAL(ret, ERR_OK);
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
    pSuite = CU_add_suite("StringUtils_Test", init_suite, clean_suite);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    /* 添加测试用例 */
    // trim 测试
    CU_add_test(pSuite, "test_trim_basic", test_trim_basic);
    CU_add_test(pSuite, "test_trim_no_whitespace", test_trim_no_whitespace);
    CU_add_test(pSuite, "test_trim_only_whitespace", test_trim_only_whitespace);
    CU_add_test(pSuite, "test_trim_empty_string", test_trim_empty_string);
    CU_add_test(pSuite, "test_trim_null_input", test_trim_null_input);
    CU_add_test(pSuite, "test_trim_tabs_and_newlines", test_trim_tabs_and_newlines);
    
    // concat 测试
    CU_add_test(pSuite, "test_concat_basic", test_concat_basic);
    CU_add_test(pSuite, "test_concat_null_params", test_concat_null_params);
    CU_add_test(pSuite, "test_concat_empty_strings", test_concat_empty_strings);
    
    // starts_with 测试
    CU_add_test(pSuite, "test_starts_with_true", test_starts_with_true);
    CU_add_test(pSuite, "test_starts_with_false", test_starts_with_false);
    CU_add_test(pSuite, "test_starts_with_empty_prefix", test_starts_with_empty_prefix);
    CU_add_test(pSuite, "test_starts_with_null_string", test_starts_with_null_string);
    CU_add_test(pSuite, "test_starts_with_null_prefix", test_starts_with_null_prefix);
    
    // contains 测试
    CU_add_test(pSuite, "test_contains_true", test_contains_true);
    CU_add_test(pSuite, "test_contains_false", test_contains_false);
    CU_add_test(pSuite, "test_contains_empty_substring", test_contains_empty_substring);
    CU_add_test(pSuite, "test_contains_null_string", test_contains_null_string);
    CU_add_test(pSuite, "test_contains_null_substring", test_contains_null_substring);
    
    // split 测试
    CU_add_test(pSuite, "test_split_basic", test_split_basic);
    CU_add_test(pSuite, "test_split_single_item", test_split_single_item);
    CU_add_test(pSuite, "test_split_empty", test_split_empty);
    CU_add_test(pSuite, "test_split_null_input", test_split_null_input);
    
    // strdup 测试
    CU_add_test(pSuite, "test_strdup_basic", test_strdup_basic);
    CU_add_test(pSuite, "test_strdup_null", test_strdup_null);
    CU_add_test(pSuite, "test_strdup_empty", test_strdup_empty);
    
    // atoi 测试
    CU_add_test(pSuite, "test_atoi_positive", test_atoi_positive);
    CU_add_test(pSuite, "test_atoi_negative", test_atoi_negative);
    CU_add_test(pSuite, "test_atoi_zero", test_atoi_zero);
    CU_add_test(pSuite, "test_atoi_invalid", test_atoi_invalid);
    CU_add_test(pSuite, "test_atoi_null_param", test_atoi_null_param);
    
    // snprintf 测试
    CU_add_test(pSuite, "test_snprintf_basic", test_snprintf_basic);
    CU_add_test(pSuite, "test_snprintf_overflow", test_snprintf_overflow);
    
    // strncpy 测试
    CU_add_test(pSuite, "test_strncpy_basic", test_strncpy_basic);
    CU_add_test(pSuite, "test_strncpy_overflow", test_strncpy_overflow);
    CU_add_test(pSuite, "test_strncpy_null_params", test_strncpy_null_params);
    
    /* 运行测试 */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
    /* 清理 */
    CU_cleanup_registry();
    
    return CU_get_error();
}
