#include <gtest/gtest.h>
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <security/pam_appl.h>
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QLightDM/Greeter>
#include <QSocketNotifier>
#include <QDebug>
#include <QSignalSpy>
#include "../../src/dbusifs/accountshelper.h"
#include "../../src/lock-backend/authpamthread.h"
#include "../../src/lock-backend/pamauthenticate.h"
#include "../../src/lock-backend/lightdmhelper.h"

class AuthPamThreadTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pAuthPamThreadDbus = new AuthPamThread();
    }

    static void TearDownTestSuite()
    {
        delete m_pAuthPamThreadDbus;
        m_pAuthPamThreadDbus = nullptr;
    }

    static AuthPamThread *m_pAuthPamThreadDbus;

    // 辅助函数，模拟文件描述符操作
    int createMockPipe()
    {
        int fds[2];
        if (pipe(fds) == -1) {
            return -1;
        }
        return fds[0];
    }

    // 辅助函数，模拟写入数据到文件描述符
    void writeToFd(int fd, const char *data)
    {
        write(fd, data, strlen(data));
    }

    // 辅助函数，从文件描述符读取数据
    std::string readFromFd(int fd)
    {
        char buffer[1024];
        ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            return buffer;
        }
        return "";
    }

    // 辅助函数，模拟 pam_start 的返回结果
    int mockPamStart(pam_handle_t **pamh, const char *service, const char *user, const struct pam_conv *conv)
    {
        *pamh = reinterpret_cast<pam_handle_t *>(1);
        return PAM_SUCCESS;
    }

    // 辅助函数，模拟 pam_authenticate 的返回结果
    int mockPamAuthenticate(pam_handle_t *pamh, int flags)
    {
        return PAM_SUCCESS;
    }

    // 辅助函数，模拟 pam_acct_mgmt 的返回结果
    int mockPamAcctMgmt(pam_handle_t *pamh, int flags)
    {
        return PAM_SUCCESS;
    }

    // 辅助函数，模拟 pam_get_item 的返回结果
    int mockPamGetItem(pam_handle_t *pamh, int item_type, const void **item)
    {
        *item = nullptr;
        return PAM_SUCCESS;
    }

    // 辅助函数，模拟 pam_strerror 的返回结果
    const char *mockPamStrerror(pam_handle_t *pamh, int pam_err)
    {
        return "Mock Error";
    }

    // 辅助函数，模拟 pam_end 的行为
    int mockPamEnd(pam_handle_t *pamh, int pam_status)
    {
        return PAM_SUCCESS;
    }
};
AuthPamThread *AuthPamThreadTest::m_pAuthPamThreadDbus = nullptr;

// 测试 _authenticate 函数
TEST_F(AuthPamThreadTest, Authenticate)
{
    m_pAuthPamThreadDbus->m_strUserName = getenv("USER");
    qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

    // 模拟 pam 函数
    auto originalPamStart = pam_start;
    auto originalPamAuthenticate = pam_authenticate;
    auto originalPamAcctMgmt = pam_acct_mgmt;
    auto originalPamGetItem = pam_get_item;
    auto originalPamStrerror = pam_strerror;
    auto originalPamEnd = pam_end;

    //    pam_start = mockPamStart;
    //    pam_authenticate = mockPamAuthenticate;
    //    pam_acct_mgmt = mockPamAcctMgmt;
    //    pam_get_item = mockPamGetItem;
    //    pam_strerror = mockPamStrerror;
    //    pam_end = mockPamEnd;

    qDebug() << "??????????????????????????????????????";
    //    m_pAuthPamThreadDbus->_authenticate(m_pAuthPamThreadDbus->m_strUserName.toLocal8Bit().data());

    // 恢复原始函数
    //    pam_start = originalPamStart;
    //    pam_authenticate = originalPamAuthenticate;
    //    pam_acct_mgmt = originalPamAcctMgmt;
    //    pam_get_item = originalPamGetItem;
    //    pam_strerror = originalPamStrerror;
    //    pam_end = originalPamEnd;
}

// 测试 startAuthPam 函数
TEST_F(AuthPamThreadTest, StartAuthPam)
{
    qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    int fdRead = createMockPipe();
    int fdWrite = createMockPipe();

    m_pAuthPamThreadDbus->startAuthPam(fdRead, fdWrite, getenv("USER"));

    EXPECT_TRUE(m_pAuthPamThreadDbus->isRunning());

    close(fdRead);
    close(fdWrite);
}

// 测试 run 函数
TEST_F(AuthPamThreadTest, Run)
{

    int fdRead = createMockPipe();
    int fdWrite = createMockPipe();

    m_pAuthPamThreadDbus->m_fdRead = fdRead;
    m_pAuthPamThreadDbus->m_fdWrite = fdWrite;
    m_pAuthPamThreadDbus->m_strUserName = getenv("USER");

    //    m_pAuthPamThreadDbus->run();

    close(fdRead);
    close(fdWrite);
}

// 测试 stopAuthPam 函数
TEST_F(AuthPamThreadTest, StopAuthPam)
{
    m_pAuthPamThreadDbus->startAuthPam(0, 0, getenv("USER"));

    m_pAuthPamThreadDbus->stopAuthPam();

    EXPECT_FALSE(m_pAuthPamThreadDbus->isRunning());
}

// 测试 PamAuthenticate 类
class PamAuthenticateTest : public ::testing::Test
{
protected:
    static AuthPamThread *m_pAuthPamThreadDbus;

    QSharedPointer<LightDMHelper> lightDMHelper;
    QSharedPointer<PamAuthenticate> pamAuthenticate;

    void SetUp() override
    {
        lightDMHelper = QSharedPointer<LightDMHelper>(
            new LightDMHelper(QSharedPointer<AccountsHelper>(new AccountsHelper(nullptr)), nullptr));
        pamAuthenticate = QSharedPointer<PamAuthenticate>(new PamAuthenticate(lightDMHelper));
    }

    void TearDown() override
    {
        pamAuthenticate.reset();
        lightDMHelper.reset();
    }

    // 辅助函数，模拟 QSocketNotifier 的 activated 信号
    void simulateSocketNotifierActivated(QSocketNotifier *socketNotifier)
    {
        //        Q_EMIT socketNotifier->activated(socketNotifier->socket());
    }

    // 辅助函数，模拟管道操作
    int createMockPipe()
    {
        int fds[2];
        if (pipe(fds) == -1) {
            return -1;
        }
        return fds[0];
    }

    // 辅助函数，模拟 AuthPamThread 的行为
    class MockAuthPamThread : public AuthPamThread
    {
    public:
    };

    // 辅助函数，模拟 LightDMHelper 的行为
    class MockLightDMHelper : public LightDMHelper
    {
    public:
        MockLightDMHelper()
            : LightDMHelper(QSharedPointer<AccountsHelper>(new AccountsHelper(nullptr)), nullptr, nullptr)
        {
        }
    };
};

// 测试 PamAuthenticate 构造函数
TEST_F(PamAuthenticateTest, Constructor)
{
    // 检查信号和槽的连接
    //    EXPECT_TRUE(QObject::connect(lightDMHelper.get(), SIGNAL(showMessage(QString,
    //    QLightDM::Greeter::MessageType)),
    //                pamAuthenticate.get(), SLOT(onLDMShowMessage(QString, QLightDM::Greeter::MessageType)));

    //    EXPECT_TRUE(QObject::connect(lightDMHelper.get(), SIGNAL(showPrompt(QString, QLightDM::Greeter::PromptType)),
    //                pamAuthenticate.get(), SLOT(onLDMShowPrompt(QString, QLightDM::Greeter::PromptType)));

    //    EXPECT_TRUE(QObject::connect(lightDMHelper.get(), SIGNAL(authenticationComplete()),
    //                pamAuthenticate.get(), SIGNAL(authenticationComplete()));
}

// 测试 inAuthentication 函数
TEST_F(PamAuthenticateTest, InAuthentication)
{
    // 测试 m_isOtherUser 为 false 时的情况
    EXPECT_FALSE(pamAuthenticate->inAuthentication());
    // 测试 m_isOtherUser 为 true 时的情况
    pamAuthenticate->m_isOtherUser = true;
    EXPECT_FALSE(pamAuthenticate->inAuthentication());
}

// 测试 isAuthenticated 函数
TEST_F(PamAuthenticateTest, IsAuthenticated)
{
    // 测试 m_isOtherUser 为 false 时的情况
    EXPECT_FALSE(pamAuthenticate->isAuthenticated());

    // 测试 m_isOtherUser 为 true 时的情况
    pamAuthenticate->m_isOtherUser = true;
    EXPECT_FALSE(pamAuthenticate->isAuthenticated());
}

// 测试 authenticationUser 函数
TEST_F(PamAuthenticateTest, AuthenticationUser)
{
    // 测试 m_isOtherUser 为 false 时的情况
    EXPECT_EQ(pamAuthenticate->authenticationUser(), "");

    // 测试 m_isOtherUser 为 true 时的情况
    pamAuthenticate->m_isOtherUser = true;
    EXPECT_EQ(pamAuthenticate->authenticationUser(), "");
}

// 测试 authenticate 函数
TEST_F(PamAuthenticateTest, Authenticate)
{
    // 测试普通用户的情况
    pamAuthenticate->authenticate("testuser");
    EXPECT_EQ(pamAuthenticate->m_strUserName, "testuser");
    EXPECT_NE(pamAuthenticate->m_threadAuthPam, nullptr);
    EXPECT_NE(pamAuthenticate->m_socketNotifier, nullptr);

    // 测试访客用户的情况
    pamAuthenticate->authenticate("*guest");

    // 测试 m_isOtherUser 为 true 的情况
    pamAuthenticate->m_isOtherUser = true;
    pamAuthenticate->authenticate("otheruser");
}

// 测试 respond 函数
TEST_F(PamAuthenticateTest, Respond)
{
    // 测试正常响应的情况
    pamAuthenticate->m_nPrompts = 1;
    pamAuthenticate->respond("response");
    EXPECT_EQ(pamAuthenticate->m_responseList.size(), 1);

    // 测试 m_nPrompts 为 0 的情况
    pamAuthenticate->m_nPrompts = 0;
    pamAuthenticate->m_isOtherUser = false;
    pamAuthenticate->respond("response");
    EXPECT_EQ(pamAuthenticate->m_responseList.size(), 1);
}

// 测试 cancelAuthentication 函数
TEST_F(PamAuthenticateTest, CancelAuthentication)
{
    // 模拟 AuthPamThread 和 QSocketNotifier
    pamAuthenticate->m_threadAuthPam = new MockAuthPamThread();
    pamAuthenticate->m_socketNotifier = new QSocketNotifier(0, QSocketNotifier::Read);

    pamAuthenticate->m_isOtherUser = false;
    pamAuthenticate->cancelAuthentication();

    EXPECT_EQ(pamAuthenticate->m_threadAuthPam, nullptr);
    EXPECT_EQ(pamAuthenticate->m_socketNotifier, nullptr);
    EXPECT_FALSE(pamAuthenticate->m_isAuthenticated);
    EXPECT_FALSE(pamAuthenticate->m_isInAuthentication);
    EXPECT_EQ(pamAuthenticate->m_messageList.size(), 0);
    EXPECT_EQ(pamAuthenticate->m_responseList.size(), 0);
    EXPECT_EQ(pamAuthenticate->m_nPrompts, 0);
}

// 测试 onSockRead 函数
TEST_F(PamAuthenticateTest, OnSockRead)
{
    // 模拟管道
    int fdToParent[2];
    if (pipe(fdToParent) == -1) {
        FAIL() << "Failed to create pipe";
    }

    pamAuthenticate->m_fdToParent[0] = fdToParent[0];

    // 测试认证完成的情况
    int authComplete = 1;
    int authRet = PAM_SUCCESS;
    write(pamAuthenticate->m_fdToParent[0], &authComplete, sizeof(authComplete));
    write(pamAuthenticate->m_fdToParent[0], &authRet, sizeof(authRet));

    QSignalSpy authenticationCompleteSpy(pamAuthenticate.get(), SIGNAL(authenticationComplete()));

    simulateSocketNotifierActivated(pamAuthenticate->m_socketNotifier);

    EXPECT_EQ(pamAuthenticate->m_isAuthenticated, true);
    EXPECT_EQ(authenticationCompleteSpy.count(), 1);

    // 测试消息接收的情况
    authComplete = 0;
    int msgLength = 1;
    write(pamAuthenticate->m_fdToParent[0], &authComplete, sizeof(authComplete));
    write(pamAuthenticate->m_fdToParent[0], &msgLength, sizeof(msgLength));

    struct pam_message message;
    message.msg_style = PAM_PROMPT_ECHO_OFF;
    message.msg = "Enter password";

    write(pamAuthenticate->m_fdToParent[0], &message.msg_style, sizeof(message.msg_style));
    write(pamAuthenticate->m_fdToParent[0], message.msg, strlen(message.msg) + 1);

    QSignalSpy showPromptSpy(pamAuthenticate.get(), SIGNAL(showPrompt(QString, PamAuth::PromptType)));

    simulateSocketNotifierActivated(pamAuthenticate->m_socketNotifier);

    EXPECT_EQ(showPromptSpy.count(), 1);

    close(fdToParent[0]);
    close(fdToParent[1]);
}
