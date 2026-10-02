/**
 * @file test_file_utils.c
 * @brief 文件工具函数单元测试
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "utils/file_utils.h"
#include "utils/string_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#define TEST_FILE "./test_file.txt"
#define TEST_DIR "./test_dir"

// ==================== file_exists 测试 ====================

static void test_file_exists_true(void)
{
    // 创建测试文件
    FILE *fp = fopen(TEST_FILE, "w");
    CU_ASSERT_PTR_NOT_NULL(fp);
    if (fp) fclose(fp);
    
    CU_ASSERT_TRUE(file_utils_file_exists(TEST_FILE));
    
    remove(TEST_FILE);
}

static void test_file_exists_false(void)
{
    CU_ASSERT_FALSE(file_utils_file_exists("/nonexistent/file.txt"));
}

static void test_file_exists_null_path(void)
{
    CU_ASSERT_FALSE(file_utils_file_exists(NULL));
}

// ==================== is_directory 测试 ====================

static void test_is_directory_true(void)
{
#ifdef _WIN32
    _mkdir(TEST_DIR);
#else
    mkdir(TEST_DIR, 0755);
#endif
    
    CU_ASSERT_TRUE(file_utils_is_directory(TEST_DIR));
    
    rmdir(TEST_DIR);
}

static void test_is_directory_false(void)
{
    // 创建普通文件
    FILE *fp = fopen(TEST_FILE, "w");
    if (fp) {
        fprintf(fp, "test");
        fclose(fp);
    }
    
    CU_ASSERT_FALSE(file_utils_is_directory(TEST_FILE));
    
    remove(TEST_FILE);
}

static void test_is_directory_nonexistent(void)
{
    CU_ASSERT_FALSE(file_utils_is_directory("/nonexistent/dir"));
}

static void test_is_directory_null_path(void)
{
    CU_ASSERT_FALSE(file_utils_is_directory(NULL));
}

// ==================== is_regular_file 测试 ====================

static void test_is_regular_file_true(void)
{
    FILE *fp = fopen(TEST_FILE, "w");
    if (fp) {
        fprintf(fp, "test");
        fclose(fp);
    }
    
    CU_ASSERT_TRUE(file_utils_is_regular_file(TEST_FILE));
    
    remove(TEST_FILE);
}

static void test_is_regular_file_false_for_directory(void)
{
#ifdef _WIN32
    _mkdir(TEST_DIR);
#else
    mkdir(TEST_DIR, 0755);
#endif
    
    CU_ASSERT_FALSE(file_utils_is_regular_file(TEST_DIR));
    
    rmdir(TEST_DIR);
}

static void test_is_regular_file_nonexistent(void)
{
    CU_ASSERT_FALSE(file_utils_is_regular_file("/nonexistent/file.txt"));
}

// ==================== read_file 测试 ====================

static void test_read_file_success(void)
{
    const char *content = "Hello, World!\nTest content.";
    
    FILE *fp = fopen(TEST_FILE, "w");
    CU_ASSERT_PTR_NOT_NULL(fp);
    if (fp) {
        fprintf(fp, "%s", content);
        fclose(fp);
    }
    
    char *read_content = NULL;
    size_t size = 0;
    psp_error_t ret = file_utils_read_file(TEST_FILE, &read_content, &size);
    
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_PTR_NOT_NULL(read_content);
    CU_ASSERT_STRING_EQUAL(read_content, content);
    CU_ASSERT(size > 0);
    
    free(read_content);
    remove(TEST_FILE);
}

static void test_read_file_nonexistent(void)
{
    char *read_content = NULL;
    size_t size = 0;
    psp_error_t ret = file_utils_read_file("/nonexistent/file.txt", &read_content, &size);
    
    CU_ASSERT_NOT_EQUAL(ret, ERR_OK);
    CU_ASSERT_PTR_NULL(read_content);
}

static void test_read_file_null_params(void)
{
    psp_error_t ret = file_utils_read_file(TEST_FILE, NULL, NULL);
    CU_ASSERT_NOT_EQUAL(ret, ERR_OK);
}

// ==================== write_file 测试 ====================

static void test_write_file_success(void)
{
    const char *content = "Test write content\nLine 2";
    
    psp_error_t ret = file_utils_write_file(TEST_FILE, content, 0);
    
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_TRUE(file_utils_file_exists(TEST_FILE));
    
    // 验证写入内容
    char *read_content = NULL;
    size_t size = 0;
    ret = file_utils_read_file(TEST_FILE, &read_content, &size);
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_STRING_EQUAL(read_content, content);
    
    free(read_content);
    remove(TEST_FILE);
}

static void test_write_file_with_size(void)
{
    const char *content = "Test write content";
    
    psp_error_t ret = file_utils_write_file(TEST_FILE, content, 4); // 只写 "Test"
    
    CU_ASSERT_EQUAL(ret, ERR_OK);
    
    char *read_content = NULL;
    size_t size = 0;
    ret = file_utils_read_file(TEST_FILE, &read_content, &size);
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_STRING_EQUAL(read_content, "Test");
    
    free(read_content);
    remove(TEST_FILE);
}

static void test_write_file_null_content(void)
{
    psp_error_t ret = file_utils_write_file(TEST_FILE, NULL, 0);
    CU_ASSERT_NOT_EQUAL(ret, ERR_OK);
}

// ==================== get_file_size 测试 ====================

static void test_get_file_size_success(void)
{
    const char *content = "1234567890"; // 10 bytes
    
    FILE *fp = fopen(TEST_FILE, "w");
    CU_ASSERT_PTR_NOT_NULL(fp);
    if (fp) {
        fprintf(fp, "%s", content);
        fclose(fp);
    }
    
    long size = file_utils_get_file_size(TEST_FILE);
    
    CU_ASSERT_EQUAL(size, 10);
    
    remove(TEST_FILE);
}

static void test_get_file_size_nonexistent(void)
{
    long size = file_utils_get_file_size("/nonexistent/file.txt");
    CU_ASSERT_EQUAL(size, -1);
}

static void test_get_file_size_null_param(void)
{
    long size = file_utils_get_file_size(NULL);
    CU_ASSERT_EQUAL(size, -1);
}

// ==================== create_directory 测试 ====================

static void test_create_directory_success(void)
{
    psp_error_t ret = file_utils_create_directory(TEST_DIR, 0755);
    
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_TRUE(file_utils_is_directory(TEST_DIR));
    
    rmdir(TEST_DIR);
}

static void test_create_directory_nested(void)
{
    psp_error_t ret = file_utils_create_directory("./test_parent/test_child", 0755);
    
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_TRUE(file_utils_is_directory("./test_parent/test_child"));
    
    rmdir("./test_parent/test_child");
    rmdir("./test_parent");
}

static void test_create_directory_null_path(void)
{
    psp_error_t ret = file_utils_create_directory(NULL, 0755);
    CU_ASSERT_NOT_EQUAL(ret, ERR_OK);
}

// ==================== scan_directory 测试 ====================

static void test_scan_directory_txt_files(void)
{
#ifdef _WIN32
    _mkdir(TEST_DIR);
#else
    mkdir(TEST_DIR, 0755);
#endif
    
    // 创建测试文件
    FILE *fp1 = fopen(TEST_DIR "/file1.txt", "w");
    FILE *fp2 = fopen(TEST_DIR "/file2.txt", "w");
    FILE *fp3 = fopen(TEST_DIR "/file3.log", "w");
    if (fp1) fclose(fp1);
    if (fp2) fclose(fp2);
    if (fp3) fclose(fp3);
    
    char **files = NULL;
    size_t count = 0;
    psp_error_t ret = file_utils_scan_directory(TEST_DIR, ".txt", &files, &count);
    
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_EQUAL(count, 2); // 只有 .txt 文件
    
    if (files) {
        string_utils_free_array(files, count);
    }
    
    remove(TEST_DIR "/file1.txt");
    remove(TEST_DIR "/file2.txt");
    remove(TEST_DIR "/file3.log");
    rmdir(TEST_DIR);
}

static void test_scan_directory_all_files(void)
{
#ifdef _WIN32
    _mkdir(TEST_DIR);
#else
    mkdir(TEST_DIR, 0755);
#endif
    
    FILE *fp1 = fopen(TEST_DIR "/file1.txt", "w");
    FILE *fp2 = fopen(TEST_DIR "/file2.log", "w");
    if (fp1) fclose(fp1);
    if (fp2) fclose(fp2);
    
    char **files = NULL;
    size_t count = 0;
    psp_error_t ret = file_utils_scan_directory(TEST_DIR, NULL, &files, &count);
    
    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT(count >= 2); // 所有文件
    
    if (files) {
        string_utils_free_array(files, count);
    }
    
    remove(TEST_DIR "/file1.txt");
    remove(TEST_DIR "/file2.log");
    rmdir(TEST_DIR);
}

static void test_scan_directory_nonexistent(void)
{
    char **files = NULL;
    size_t count = 0;
    psp_error_t ret = file_utils_scan_directory("/nonexistent/dir", ".txt", &files, &count);
    
    CU_ASSERT_NOT_EQUAL(ret, ERR_OK);
    CU_ASSERT_EQUAL(count, 0);
}

static void test_scan_directory_null_params(void)
{
    psp_error_t ret = file_utils_scan_directory(NULL, ".txt", NULL, NULL);
    CU_ASSERT_NOT_EQUAL(ret, ERR_OK);
}

// ==================== scan_directory 递归扫描测试 ====================

/**
 * 递归扫描子目录，验证返回相对路径
 * 目录结构:
 *   test_dir/
 *     top.txt
 *     sub/
 *       nested.txt
 */
static void test_scan_directory_recursive_basic(void)
{
    mkdir(TEST_DIR, 0755);
    mkdir(TEST_DIR "/sub", 0755);

    FILE *fp1 = fopen(TEST_DIR "/top.txt", "w");
    FILE *fp2 = fopen(TEST_DIR "/sub/nested.txt", "w");
    if (fp1) fclose(fp1);
    if (fp2) fclose(fp2);

    char **files = NULL;
    size_t count = 0;
     ret = file_utils_scan_directory(TEST_DIR, ".txt", &files, &count);

    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_EQUAL(count, 2);

    if (count == 2 && files) {
        /* 验证一个无路径前缀（顶层），一个带子目录前缀 */
        int found_top = 0, found_nested = 0;
        for (size_t i = 0; i < count; i++) {
            if (strcmp(files[i], "top.txt") == 0) found_top = 1;
            if (strcmp(files[i], "sub/nested.txt") == 0) found_nested = 1;
        }
        CU_ASSERT_TRUE(found_top);
        CU_ASSERT_TRUE(found_nested);
    }

    if (files) string_utils_free_array(files, count);

    remove(TEST_DIR "/top.txt");
    remove(TEST_DIR "/sub/nested.txt");
    rmdir(TEST_DIR "/sub");
    rmdir(TEST_DIR);
}

/**
 * 子目录中按扩展名过滤
 * 目录结构:
 *   test_dir/
 *     sub/
 *       keep.so
 *       skip.log
 */
static void test_scan_directory_recursive_filter(void)
{
    mkdir(TEST_DIR, 0755);
    mkdir(TEST_DIR "/sub", 0755);

    FILE *fp1 = fopen(TEST_DIR "/sub/keep.so", "w");
    FILE *fp2 = fopen(TEST_DIR "/sub/skip.log", "w");
    if (fp1) fclose(fp1);
    if (fp2) fclose(fp2);

    char **files = NULL;
    size_t count = 0;
    psp_error_t ret = file_utils_scan_directory(TEST_DIR, ".so", &files, &count);

    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_EQUAL(count, 1);

    if (count == 1 && files) {
        CU_ASSERT_STRING_EQUAL(files[0], "sub/keep.so");
    }

    if (files) string_utils_free_array(files, count);

    remove(TEST_DIR "/sub/keep.so");
    remove(TEST_DIR "/sub/skip.log");
    rmdir(TEST_DIR "/sub");
    rmdir(TEST_DIR);
}

/**
 * 多层嵌套目录递归
 * 目录结构:
 *   test_dir/
 *     a.so
 *     level1/
 *       b.so
 *       level2/
 *         c.so
 */
static void test_scan_directory_recursive_deep(void)
{
    mkdir(TEST_DIR, 0755);
    mkdir(TEST_DIR "/level1", 0755);
    mkdir(TEST_DIR "/level1/level2", 0755);

    FILE *fp1 = fopen(TEST_DIR "/a.so", "w");
    FILE *fp2 = fopen(TEST_DIR "/level1/b.so", "w");
    FILE *fp3 = fopen(TEST_DIR "/level1/level2/c.so", "w");
    if (fp1) fclose(fp1);
    if (fp2) fclose(fp2);
    if (fp3) fclose(fp3);

    char **files = NULL;
    size_t count = 0;
    psp_error_t ret = file_utils_scan_directory(TEST_DIR, ".so", &files, &count);

    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_EQUAL(count, 3);

    if (count == 3 && files) {
        int found_a = 0, found_b = 0, found_c = 0;
        for (size_t i = 0; i < count; i++) {
            if (strcmp(files[i], "a.so") == 0) found_a = 1;
            if (strcmp(files[i], "level1/b.so") == 0) found_b = 1;
            if (strcmp(files[i], "level1/level2/c.so") == 0) found_c = 1;
        }
        CU_ASSERT_TRUE(found_a);
        CU_ASSERT_TRUE(found_b);
        CU_ASSERT_TRUE(found_c);
    }

    if (files) string_utils_free_array(files, count);

    remove(TEST_DIR "/a.so");
    remove(TEST_DIR "/level1/b.so");
    remove(TEST_DIR "/level1/level2/c.so");
    rmdir(TEST_DIR "/level1/level2");
    rmdir(TEST_DIR "/level1");
    rmdir(TEST_DIR);
}

/**
 * 空子目录不产生错误结果
 * 目录结构:
 *   test_dir/
 *     file.so
 *     empty_sub/   (空目录)
 */
static void test_scan_directory_recursive_empty_subdir(void)
{
    mkdir(TEST_DIR, 0755);
    mkdir(TEST_DIR "/empty_sub", 0755);

    FILE *fp = fopen(TEST_DIR "/file.so", "w");
    if (fp) fclose(fp);

    char **files = NULL;
    size_t count = 0;
    psp_error_t ret = file_utils_scan_directory(TEST_DIR, ".so", &files, &count);

    CU_ASSERT_EQUAL(ret, ERR_OK);
    CU_ASSERT_EQUAL(count, 1);

    if (count == 1 && files) {
        CU_ASSERT_STRING_EQUAL(files[0], "file.so");
    }

    if (files) string_utils_free_array(files, count);

    remove(TEST_DIR "/file.so");
    rmdir(TEST_DIR "/empty_sub");
    rmdir(TEST_DIR);
}

// ==================== 测试套件初始化 ====================

static int init_suite(void)
{
    return 0;
}

static int clean_suite(void)
{
    // 清理可能残留的测试文件
    remove(TEST_FILE);
    remove(TEST_DIR "/top.txt");
    remove(TEST_DIR "/file.so");
    remove(TEST_DIR "/a.so");
    remove(TEST_DIR "/sub/nested.txt");
    remove(TEST_DIR "/sub/keep.so");
    remove(TEST_DIR "/sub/skip.log");
    remove(TEST_DIR "/level1/b.so");
    remove(TEST_DIR "/level1/level2/c.so");
    rmdir(TEST_DIR "/sub");
    rmdir(TEST_DIR "/level1/level2");
    rmdir(TEST_DIR "/level1");
    rmdir(TEST_DIR "/empty_sub");
    rmdir(TEST_DIR);
    rmdir("./test_parent/test_child");
    rmdir("./test_parent");
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
    pSuite = CU_add_suite("FileUtils_Test", init_suite, clean_suite);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    /* 添加测试用例 */
    // file_exists 测试
    CU_add_test(pSuite, "test_file_exists_true", test_file_exists_true);
    CU_add_test(pSuite, "test_file_exists_false", test_file_exists_false);
    CU_add_test(pSuite, "test_file_exists_null_path", test_file_exists_null_path);
    
    // is_directory 测试
    CU_add_test(pSuite, "test_is_directory_true", test_is_directory_true);
    CU_add_test(pSuite, "test_is_directory_false", test_is_directory_false);
    CU_add_test(pSuite, "test_is_directory_nonexistent", test_is_directory_nonexistent);
    CU_add_test(pSuite, "test_is_directory_null_path", test_is_directory_null_path);
    
    // is_regular_file 测试
    CU_add_test(pSuite, "test_is_regular_file_true", test_is_regular_file_true);
    CU_add_test(pSuite, "test_is_regular_file_false_for_directory", test_is_regular_file_false_for_directory);
    CU_add_test(pSuite, "test_is_regular_file_nonexistent", test_is_regular_file_nonexistent);
    
    // read_file 测试
    CU_add_test(pSuite, "test_read_file_success", test_read_file_success);
    CU_add_test(pSuite, "test_read_file_nonexistent", test_read_file_nonexistent);
    CU_add_test(pSuite, "test_read_file_null_params", test_read_file_null_params);
    
    // write_file 测试
    CU_add_test(pSuite, "test_write_file_success", test_write_file_success);
    CU_add_test(pSuite, "test_write_file_with_size", test_write_file_with_size);
    CU_add_test(pSuite, "test_write_file_null_content", test_write_file_null_content);
    
    // get_file_size 测试
    CU_add_test(pSuite, "test_get_file_size_success", test_get_file_size_success);
    CU_add_test(pSuite, "test_get_file_size_nonexistent", test_get_file_size_nonexistent);
    CU_add_test(pSuite, "test_get_file_size_null_param", test_get_file_size_null_param);
    
    // create_directory 测试
    CU_add_test(pSuite, "test_create_directory_success", test_create_directory_success);
    CU_add_test(pSuite, "test_create_directory_nested", test_create_directory_nested);
    CU_add_test(pSuite, "test_create_directory_null_path", test_create_directory_null_path);
    
    // scan_directory 测试
    CU_add_test(pSuite, "test_scan_directory_txt_files", test_scan_directory_txt_files);
    CU_add_test(pSuite, "test_scan_directory_all_files", test_scan_directory_all_files);
    CU_add_test(pSuite, "test_scan_directory_nonexistent", test_scan_directory_nonexistent);
    CU_add_test(pSuite, "test_scan_directory_null_params", test_scan_directory_null_params);

    // scan_directory 递归扫描测试
    CU_add_test(pSuite, "test_scan_directory_recursive_basic", test_scan_directory_recursive_basic);
    CU_add_test(pSuite, "test_scan_directory_recursive_filter", test_scan_directory_recursive_filter);
    CU_add_test(pSuite, "test_scan_directory_recursive_deep", test_scan_directory_recursive_deep);
    CU_add_test(pSuite, "test_scan_directory_recursive_empty_subdir", test_scan_directory_recursive_empty_subdir);
    
    /* 运行测试 */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
    /* 清理 */
    CU_cleanup_registry();
    
    return CU_get_error();
}
