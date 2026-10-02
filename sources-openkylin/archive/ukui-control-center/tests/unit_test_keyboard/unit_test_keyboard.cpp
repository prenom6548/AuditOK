#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#include "../../registeredSession/plugins/keyboard/keyboardinterface.h"

class KeyboardInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pKeyboardDbus = new KeyBoardInterface();
    }

    static void TearDownTestSuite()
    {
        delete m_pKeyboardDbus;
        m_pKeyboardDbus = nullptr;
    }

    static KeyBoardInterface *m_pKeyboardDbus;
};

KeyBoardInterface *KeyboardInterfaceTest::m_pKeyboardDbus = nullptr;

TEST_F(KeyboardInterfaceTest, keyRepeat)
{
    // get current value
    bool repeatKeyValue = m_pKeyboardDbus->getKeyRepeat();

    // call setKeyRepeat function to set different value
    m_pKeyboardDbus->setKeyRepeat(!repeatKeyValue);

    // test the current value and the value you have set
    ASSERT_EQ(!repeatKeyValue, m_pKeyboardDbus->getKeyRepeat());

    // restore the value before test
    m_pKeyboardDbus->setKeyRepeat(repeatKeyValue);
}

TEST_F(KeyboardInterfaceTest, nokeyRepeat)
{
    // get current value
    QString originKey = m_pKeyboardDbus->KEYREPEAT_KEY;
    m_pKeyboardDbus->KEYREPEAT_KEY = "";

    bool repeatKeyValue = m_pKeyboardDbus->getKeyRepeat();

    // call setKeyRepeat function to set different value
    m_pKeyboardDbus->setKeyRepeat(!repeatKeyValue);

    // test the current value and the value you have set
    ASSERT_EQ(!repeatKeyValue, m_pKeyboardDbus->getKeyRepeat());

    // restore the value before test
    m_pKeyboardDbus->KEYREPEAT_KEY = originKey;
}

TEST_F(KeyboardInterfaceTest, keyDelay)
{
    int delayKeyValue = m_pKeyboardDbus->getDelay();

    m_pKeyboardDbus->setDelay(++delayKeyValue);

    ASSERT_EQ(delayKeyValue, m_pKeyboardDbus->getDelay());

    m_pKeyboardDbus->setDelay(--delayKeyValue);
}

TEST_F(KeyboardInterfaceTest, nokeyDelay)
{
    // get current value
    QString originKey = m_pKeyboardDbus->DELAY_KEY;
    m_pKeyboardDbus->DELAY_KEY = "";

    int delayKeyValue = m_pKeyboardDbus->getDelay();

    m_pKeyboardDbus->setDelay(++delayKeyValue);

    ASSERT_EQ(delayKeyValue, m_pKeyboardDbus->getDelay());

    // restore the value before test
    m_pKeyboardDbus->DELAY_KEY = originKey;
}

TEST_F(KeyboardInterfaceTest, keySpeed)
{
    int speedKeyValue = m_pKeyboardDbus->getRate();

    m_pKeyboardDbus->setSpeed(++speedKeyValue);

    ASSERT_EQ(speedKeyValue, m_pKeyboardDbus->getRate());

    m_pKeyboardDbus->setSpeed(--speedKeyValue);
}

TEST_F(KeyboardInterfaceTest, nokeySpeed)
{
    // get current value
    QString originKey = m_pKeyboardDbus->SPEED_KEY;
    m_pKeyboardDbus->SPEED_KEY = "";

    int speedKeyValue = m_pKeyboardDbus->getRate();

    m_pKeyboardDbus->setSpeed(++speedKeyValue);

    ASSERT_EQ(speedKeyValue, m_pKeyboardDbus->getRate());

    // restore the value before test
    m_pKeyboardDbus->SPEED_KEY = originKey;
}


TEST_F(KeyboardInterfaceTest, keyTips)
{
    bool tipsKeyValue = m_pKeyboardDbus->getShowLockTip();

    m_pKeyboardDbus->setKeyTips(!tipsKeyValue);

    ASSERT_EQ(!tipsKeyValue, m_pKeyboardDbus->getShowLockTip());

    m_pKeyboardDbus->setKeyTips(tipsKeyValue);
}

TEST_F(KeyboardInterfaceTest, nokeyTips)
{
    // get current value
    QString originKey = m_pKeyboardDbus->KEYTIPS_KEY;
    m_pKeyboardDbus->KEYTIPS_KEY = "";

    bool tipsKeyValue = m_pKeyboardDbus->getShowLockTip();

    m_pKeyboardDbus->setKeyTips(!tipsKeyValue);

    ASSERT_EQ(!tipsKeyValue, m_pKeyboardDbus->getShowLockTip());

    // restore the value before test
    m_pKeyboardDbus->KEYTIPS_KEY = originKey;
}

TEST_F(KeyboardInterfaceTest, showIconOnDesktop)
{
    bool currentValue = m_pKeyboardDbus->getShowOnDesktop();

    m_pKeyboardDbus->setIconOnDesktop(!currentValue);

    ASSERT_EQ(!currentValue, m_pKeyboardDbus->getShowOnDesktop());

    m_pKeyboardDbus->setIconOnDesktop(currentValue);
}

TEST_F(KeyboardInterfaceTest, noshowIconOnDesktop)
{
    // get current value
    QString originKey = m_pKeyboardDbus->FLOAT_KEY;
    m_pKeyboardDbus->FLOAT_KEY = "";

    bool currentValue = m_pKeyboardDbus->getShowOnDesktop();

    m_pKeyboardDbus->setIconOnDesktop(!currentValue);

    ASSERT_EQ(!currentValue, m_pKeyboardDbus->getShowOnDesktop());

    // restore the value before test
    m_pKeyboardDbus->FLOAT_KEY = originKey;
}

TEST_F(KeyboardInterfaceTest, showOnTray)
{
    bool currentValue = m_pKeyboardDbus->getShowOnTray();

    m_pKeyboardDbus->setIconOnTray(!currentValue);

    ASSERT_EQ(!currentValue, m_pKeyboardDbus->getShowOnTray());

    m_pKeyboardDbus->setIconOnTray(currentValue);
}

TEST_F(KeyboardInterfaceTest, keyboardSize)
{
    int currentValue = m_pKeyboardDbus->getKeyboardSize();

    m_pKeyboardDbus->setKeyboardSize(++currentValue);

    ASSERT_EQ(currentValue, m_pKeyboardDbus->getKeyboardSize());

    m_pKeyboardDbus->setKeyboardSize(--currentValue);
}

TEST_F(KeyboardInterfaceTest, nokeyboardSize)
{
    // get current value
    QString originKey = m_pKeyboardDbus->SCALE_KEY;
    m_pKeyboardDbus->SCALE_KEY = "";

    int currentValue = m_pKeyboardDbus->getKeyboardSize();

    m_pKeyboardDbus->setKeyboardSize(++currentValue);

    ASSERT_EQ(currentValue, m_pKeyboardDbus->getKeyboardSize());

    // restore the value before test
    m_pKeyboardDbus->SCALE_KEY = originKey;
}

TEST_F(KeyboardInterfaceTest, textSize)
{
    int currentValue = m_pKeyboardDbus->getTextSize();

    m_pKeyboardDbus->setTextSize(++currentValue);

    ASSERT_EQ(currentValue, m_pKeyboardDbus->getTextSize());

    m_pKeyboardDbus->setTextSize(--currentValue);
}

TEST_F(KeyboardInterfaceTest, animation)
{
    bool currentValue = m_pKeyboardDbus->getAnimation();

    m_pKeyboardDbus->setAnimation(!currentValue);

    ASSERT_EQ(!currentValue, m_pKeyboardDbus->getAnimation());

    m_pKeyboardDbus->setAnimation(currentValue);
}

TEST_F(KeyboardInterfaceTest, noanimation)
{
    // get current value
    QString originKey = m_pKeyboardDbus->ANIMATION_KEY;
    m_pKeyboardDbus->ANIMATION_KEY = "";
    bool currentValue = m_pKeyboardDbus->getAnimation();

    m_pKeyboardDbus->setAnimation(!currentValue);

    ASSERT_EQ(!currentValue, m_pKeyboardDbus->getAnimation());

    // restore the value before test
    m_pKeyboardDbus->ANIMATION_KEY = originKey;
}

TEST_F(KeyboardInterfaceTest, soundEffect)
{
    bool currentValue = m_pKeyboardDbus->getSoundEffect();

    m_pKeyboardDbus->setSoundEffect(!currentValue);

    ASSERT_EQ(!currentValue, m_pKeyboardDbus->getSoundEffect());

    m_pKeyboardDbus->setSoundEffect(currentValue);
}



