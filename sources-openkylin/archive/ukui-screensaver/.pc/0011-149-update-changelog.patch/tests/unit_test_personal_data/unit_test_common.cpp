#include "../../src/common/definetypes.h"
#include "../../src/common/biodefines.h"
#include "../../src/common/commonfunc.h"
#include "../../src/common/configuration.h"
#include "../../src/common/global_utils.h"
#include "../../src/common/rsac.h"
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include <QIcon>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <iostream>
#include "../../src/common/rsac.h"
#include "../../src/common/utils.h"

#include "stubext.h"
using namespace stub_ext;

// 辅助函数，用于初始化OpenSSL库

void InitOpenSSL() {

    OpenSSL_add_all_algorithms();

    ERR_load_crypto_strings();

}



// 辅助函数，用于清理OpenSSL库

void CleanupOpenSSL() {

    EVP_cleanup();

    ERR_free_strings();

}

class CommonTest : public testing::Test
{
protected:

    void SetUp() override {

        InitOpenSSL();

    }



    void TearDown() override {

        CleanupOpenSSL();

    }

    RSAC rsac;
};

TEST_F(CommonTest, scaledPixmap)
{
    setCursorCenter();
    QPixmap pixmap = QIcon::fromTheme("view-refresh-symbolic").pixmap(48, 48);
    QPixmap newPixmap = scaledPixmap(pixmap);
    ASSERT_EQ(newPixmap.isNull(), false);
}

TEST_F(CommonTest, PixmapToRound)
{
    QPixmap pixmap = QIcon::fromTheme("view-refresh-symbolic").pixmap(48, 48);
    QPixmap newPixmap = PixmapToRound(pixmap, 4);
    ASSERT_EQ(newPixmap.isNull(), false);
}

TEST_F(CommonTest, PixmapToRound2)
{
    QPixmap pixmap = QIcon::fromTheme("view-refresh-symbolic").pixmap(48, 48);
    QPixmap newPixmap = PixmapToRound(pixmap, 0, 0, 0, 0);
    ASSERT_EQ(newPixmap.isNull(), false);
}

TEST_F(CommonTest, loadSvg)
{
    QPixmap iconPixmap = loadSvg(":/image/assets/show-password.png", "white", 16);
    ASSERT_EQ(iconPixmap.isNull(), false);
}

TEST_F(CommonTest, drawSymbolicColoredPixmap)
{
    QPixmap retryIcon = QIcon::fromTheme("view-refresh-symbolic").pixmap(48, 48);
    ASSERT_EQ(drawSymbolicColoredPixmap(retryIcon, "white").isNull(), false);
    ASSERT_EQ(getLoadingIcon(16).width(), 16);
}

TEST_F(CommonTest, scaleBlurPixmap)
{
    QPixmap *pixmap = new QPixmap(scaleBlurPixmap(16, 16, "/usr/share/background/house.png"));
    ASSERT_EQ(pixmap->width(), 16);
}

TEST_F(CommonTest, blurPixmap)
{
    QPixmap pixmap = QIcon::fromTheme("view-refresh-symbolic").pixmap(48, 48);
    QPixmap newpixmap = blurPixmap(pixmap);
    ASSERT_EQ(newpixmap.width(), 16);
}

TEST_F(CommonTest, getValue)
{
    if (Configuration::instance()) {
        QString color = Configuration::instance()->getValue("background-color").toString();
        ASSERT_EQ(color.isEmpty(), false);
    }
}

TEST_F(CommonTest, getCurrentUser)
{
    if (Configuration::instance()) {
        Configuration::instance()->getCurrentUser(getenv("USER"));
    }
}

TEST_F(CommonTest, setValue)
{
    if (Configuration::instance()) {
        QString color = Configuration::instance()->getValue("backgroundPath").toString();
        QString oldvalue = Configuration::instance()->getUserConfig("backgroundPath").toString();
        Configuration::instance()->setValue("backgroundPath", oldvalue);
        ASSERT_EQ(oldvalue.isEmpty(), false);
    }
}

TEST_F(CommonTest, hasValue)
{
    if (Configuration::instance()) {
        bool value = Configuration::instance()->hasValue("backgroundPath");
        ASSERT_EQ(value, false);
    }
}


TEST_F(CommonTest, getIs990)
{
    if (Configuration::instance()) {
        bool value = Configuration::instance()->getIs990();
        ASSERT_EQ(value, false);
    }
}

TEST_F(CommonTest, getLastNumLock)
{
    if (Configuration::instance()) {
        bool value = Configuration::instance()->getLastNumLock();
        Configuration::instance()->saveLastNumLock(value);
        ASSERT_EQ(value, false);
    }
}

TEST_F(CommonTest, getRootBackgroundOption)
{
    if (Configuration::instance()) {
        int value = Configuration::instance()->getRootBackgroundOption(getenv("USER"));
        ASSERT_EQ(value, 1);
    }
}

TEST_F(CommonTest, getDefaultBackgroundName)
{
    if (Configuration::instance()) {
        QString background = Configuration::instance()->getDefaultBackgroundName();
        ASSERT_EQ(background.isEmpty(), false);
    }
}

TEST_F(CommonTest, checkCapsLockState)
{
    checkIslivecd();
    KillFocusOfKydroid();
    ASSERT_EQ(checkCapsLockState(), false);
}

TEST_F(CommonTest, getDefaultFontSize)
{
    ASSERT_EQ(getDefaultFontSize(), 14.0);
}

TEST_F(CommonTest, getUserFontSize)
{
    ASSERT_EQ(getUserFontSize(getenv("USER")), 14.0);
}

TEST_F(CommonTest, getUserThemeColor)
{
    ASSERT_EQ(getUserThemeColor(getenv("USER")), "test");
}

// 测试密钥生成并保存到文件

TEST_F(CommonTest, GenerateKeyPairToFile) {

    QString priKeyFile = "test_private.pem";

    QString pubKeyFile = "test_public.pem";

    rsac.generateKeyPair(priKeyFile, pubKeyFile, 1024);



    // 这里可以添加额外的检查来验证文件是否成功生成且内容有效

    // 例如，打开文件并读取内容，检查是否为有效的PEM格式密钥

    // 但由于这是一个简单的测试示例，我们省略了这些步骤



    // 清理生成的文件

    QFile::remove(priKeyFile);

    QFile::remove(pubKeyFile);

}



// 测试密钥生成并保存到QByteArray

TEST_F(CommonTest, GenerateKeyPairToByteArray) {

    QByteArray privateKey;

    QByteArray publicKey;

    rsac.generateKeyPair(privateKey, publicKey, 1024);



    // 检查生成的密钥是否非空

    EXPECT_FALSE(privateKey.isEmpty());

    EXPECT_FALSE(publicKey.isEmpty());



    // 这里可以添加额外的检查来验证密钥的有效性

    // 例如，使用OpenSSL函数解析生成的密钥并检查其属性

    // 但由于这是一个简单的测试示例，我们省略了这些步骤

}



// 测试加密和解密

TEST_F(CommonTest, EncryptDecrypt) {

    QByteArray privateKey;

    QByteArray publicKey;

    rsac.generateKeyPair(privateKey, publicKey, 1024);



    QByteArray plaintext = "Hello, RSA!";

    QByteArray ciphertext;

    QByteArray decryptedtext;



    bool encryptResult = rsac.encrypt(plaintext, ciphertext, publicKey);

    EXPECT_TRUE(encryptResult);



    bool decryptResult = rsac.decrypt(ciphertext, decryptedtext, privateKey);

    EXPECT_TRUE(decryptResult);



    EXPECT_EQ(plaintext, decryptedtext);

}



// 测试签名和验签

TEST_F(CommonTest, SignVerify) {

    QByteArray privateKey;

    QByteArray publicKey;

    rsac.generateKeyPair(privateKey, publicKey, 1024);



    QByteArray message = "This is a test message.";

    QByteArray signature;

    bool signResult = rsac.sign(message, signature, privateKey);

    EXPECT_TRUE(signResult);

    bool verifyResult = rsac.verify(message, signature, publicKey);

    EXPECT_TRUE(verifyResult);

    // 尝试使用错误的消息进行验签，应该失败

    bool verifyWithWrongMessageResult = rsac.verify("Wrong message.", signature, publicKey);

    EXPECT_FALSE(verifyWithWrongMessageResult);

}
