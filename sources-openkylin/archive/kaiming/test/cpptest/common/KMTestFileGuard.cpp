#include <gtest/gtest.h>
#include <fcntl.h>
#include <unistd.h>
#include "KMFileGuard.h"

class KMFileGuardTest : public ::testing::Test {
protected:
    void SetUp() override {
        fd1 = open("test_file1.txt", O_CREAT | O_RDWR, 0644);
        fd2 = open("test_file2.txt", O_CREAT | O_RDWR, 0644);
        ASSERT_GT(fd1, 2);
        ASSERT_GT(fd2, 2);
    }

    void TearDown() override {
        unlink("test_file1.txt");
        unlink("test_file2.txt");
    }

    int fd1;
    int fd2;
};

// 测试构造函数和析构函数
TEST_F(KMFileGuardTest, ConstructorAndDestructor) {
    int temp_fd = fd1;
    {
        KMFileGuard guard(fd1);
        // 通过尝试读取验证文件描述符是否有效
        char buf[10];
        ASSERT_NE(read(fd1, buf, 0), -1);
    }
    // 析构后检查文件描述符是否被关闭
    char buf[10];
    ASSERT_EQ(read(temp_fd, buf, 0), -1);
}

// 测试setFd方法
TEST_F(KMFileGuardTest, SetFd) {
    KMFileGuard guard(fd1);
    
    // 检查初始文件描述符是否有效
    char buf[10];
    ASSERT_NE(read(fd1, buf, 0), -1);
    
    // 设置新的文件描述符
    guard.setFd(fd2);
    guard.fd();
    
    // 检查旧文件描述符是否被关闭
    ASSERT_EQ(read(fd1, buf, 0), -1);
    
    // 检查新文件描述符是否有效
    ASSERT_NE(read(fd2, buf, 0), -1);
}

// 测试不关闭标准文件描述符
TEST(KMFileGuardStandardFdTest, DontCloseStandardFds) {
    int stdout_copy = dup(STDOUT_FILENO);
    {
        KMFileGuard guard(STDOUT_FILENO);
        // 检查标准输出是否仍然有效
        ASSERT_NE(write(STDOUT_FILENO, "test", 4), -1);
    }
    // 检查标准输出是否仍然有效
    ASSERT_NE(write(STDOUT_FILENO, "test", 4), -1);
    dup2(stdout_copy, STDOUT_FILENO);
    close(stdout_copy);
}
