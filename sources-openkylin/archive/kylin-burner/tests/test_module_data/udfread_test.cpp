#include "lib/ky-udf-image/include/udfread.h"
#include <QtTest/QtTest>

class UdfReadTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试udf_file_type枚举
    void testUdfFileTypeEnum() {
        QVERIFY(UDF_FT_METADATA == 250);
        QVERIFY(UDF_FT_METADATA_MIRROR == 251);
    }

    // 测试udf_file_identifier结构体
    void testUdfFileIdentifierStruct() {
        struct udf_file_identifier fileIdentifier;
        
        // 验证结构体成员
        QVERIFY(sizeof(fileIdentifier.filename_utf8) > 0);
        QVERIFY(sizeof(fileIdentifier.filename) == 256);
        QVERIFY(sizeof(fileIdentifier.padLength) == sizeof(int));
        QVERIFY(sizeof(fileIdentifier.filename_length) == sizeof(int));
        QVERIFY(sizeof(fileIdentifier.icb) == sizeof(long_ad));
        QVERIFY(sizeof(fileIdentifier.characteristic) == sizeof(uint8_t));
        QVERIFY(sizeof(fileIdentifier.file_indent_desc) == sizeof(tag_file_indent_desc));
        QVERIFY(sizeof(fileIdentifier.file_entry) == sizeof(tag_file_entry));
        QVERIFY(sizeof(fileIdentifier.used_length) == sizeof(int));
    }

    // 测试udf_dir结构体
    void testUdfDirStruct() {
        struct udf_dir dir;
        
        // 验证结构体成员
        QVERIFY(sizeof(dir.file_entry) == sizeof(tag_file_entry));
        QVERIFY(sizeof(dir.num_entries) == sizeof(uint32_t));
        QVERIFY(sizeof(dir.files) > 0);
        QVERIFY(sizeof(dir.subdirs) > 0);
    }

    // 测试udfread_dir结构体
    void testUdfreadDirStruct() {
        struct udfread_dir udfDir;
        
        // 验证结构体成员
        QVERIFY(sizeof(udfDir.udf) > 0);
        QVERIFY(sizeof(udfDir.dir) > 0);
        QVERIFY(sizeof(udfDir.current_file) == sizeof(uint32_t));
    }

    // 测试udf_partitions结构体
    void testUdfPartitionsStruct() {
        struct udf_partitions partitions;
        
        // 验证结构体成员
        QVERIFY(sizeof(partitions.num_partition) == sizeof(uint32_t));
        QVERIFY(sizeof(partitions.p) == sizeof(partitions.p[0]) * 2);
        QVERIFY(sizeof(partitions.p[0].number) == sizeof(uint16_t));
        QVERIFY(sizeof(partitions.p[0].lba) == sizeof(uint32_t));
        QVERIFY(sizeof(partitions.p[0].mirror_lba) == sizeof(uint32_t));
    }

    // 测试udfread结构体
    void testUdfreadStruct() {
        struct udfread udf;
        
        // 验证结构体成员
        QVERIFY(sizeof(udf.input) > 0);
        QVERIFY(sizeof(udf.part) == sizeof(struct udf_partitions));
        QVERIFY(sizeof(udf.root_dir) > 0);
        QVERIFY(sizeof(udf.volume_identifier) > 0);
        QVERIFY(sizeof(udf.volume_set_identifier) == 128);
    }

    // 测试udfread_dirent结构体
    void testUdfreadDirentStruct() {
        struct udfread_dirent dirent;
        
        // 验证结构体成员
        QVERIFY(sizeof(dirent.d_type) == sizeof(unsigned int));
        QVERIFY(sizeof(dirent.d_name) == 512);
    }

    // 测试udfread_file结构体
    void testUdfreadFileStruct() {
        struct udfread_file file;
        
        // 验证结构体成员
        QVERIFY(sizeof(file.udf) > 0);
        QVERIFY(sizeof(file.fe) > 0);
        QVERIFY(sizeof(file.pos) == sizeof(uint64_t));
        QVERIFY(sizeof(file.block) > 0);
        QVERIFY(sizeof(file.block_valid) == sizeof(int));
        QVERIFY(sizeof(file.block_mem) > 0);
    }

    // 测试文件类型枚举
    void testFileTypeEnums() {
        QVERIFY(UDF_DT_UNKNOWN == 0);
        QVERIFY(UDF_DT_DIR == 1);
        QVERIFY(UDF_DT_REG == 2);
    }

    // 测试seek类型枚举
    void testSeekTypeEnum() {
        QVERIFY(UDF_SEEK_SET == 0);
        QVERIFY(UDF_SEEK_CUR == 1);
        QVERIFY(UDF_SEEK_END == 2);
    }

    // 测试函数声明
    void testFunctionDeclarations() {
        // 只测试函数声明，不实际调用
        int (*read_file_tree_func)(udfread*, FileNode*, struct tag_long_ad) = read_file_tree;
        int (*udfread_open_input_func)(udfread *, struct udfread_block_input *) = udfread_open_input;
        int (*udfread_open_func)(udfread *, const char *) = udfread_open;
        void (*udfread_close_func)(udfread *) = udfread_close;
        const char *(*udfread_get_volume_id_func)(udfread *) = udfread_get_volume_id;
        size_t (*udfread_get_volume_set_id_func)(udfread *, void *, size_t) = udfread_get_volume_set_id;
        UDFDIR *(*udfread_opendir_func)(udfread *, const char *) = udfread_opendir;
        UDFDIR *(*udfread_opendir_at_func)(UDFDIR *, const char *) = udfread_opendir_at;
        struct udfread_dirent *(*udfread_readdir_func)(UDFDIR *, struct udfread_dirent *) = udfread_readdir;
        void (*udfread_rewinddir_func)(UDFDIR *) = udfread_rewinddir;
        void (*udfread_closedir_func)(UDFDIR *) = udfread_closedir;
        UDFFILE *(*udfread_file_open_func)(udfread *, const char *) = udfread_file_open;
        UDFFILE *(*udfread_file_openat_func)(UDFDIR *, const char *) = udfread_file_openat;
        void (*udfread_file_close_func)(UDFFILE *) = udfread_file_close;
        int64_t (*udfread_file_size_func)(UDFFILE *) = udfread_file_size;
        uint32_t (*udfread_read_blocks_func)(UDFFILE *, void *, uint32_t, uint32_t, int) = udfread_read_blocks;
        ssize_t (*udfread_file_read_func)(UDFFILE *, void *, size_t) = udfread_file_read;
        int64_t (*udfread_file_tell_func)(UDFFILE *) = udfread_file_tell;
        int64_t (*udfread_file_seek_func)(UDFFILE *, int64_t, int) = udfread_file_seek;
        void (*test_encode_decode_func)() = test_encode_decode;
        
        QVERIFY(true); // 如果能编译通过则测试通过
    }
};

QTEST_MAIN(UdfReadTest)
#include "udfread_test.moc"