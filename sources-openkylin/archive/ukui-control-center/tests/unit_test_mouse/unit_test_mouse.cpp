#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#include "../../registeredSession/plugins/mouse/mouseinterface.h"

class MouseInterfaceTest:public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pMouseDbus = new MouseInterface();
    }

    static void TearDownTestSuite()
    {
        delete m_pMouseDbus;
        m_pMouseDbus = nullptr;
    }

    static MouseInterface *m_pMouseDbus;
};

MouseInterface *MouseInterfaceTest::m_pMouseDbus =  nullptr;

TEST_F(MouseInterfaceTest, dominantHand)
{
    // get current value
    bool currentValue = m_pMouseDbus->getDominantHand();

    // call set function to set different value
    m_pMouseDbus->setDominantHand(!currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(!currentValue, m_pMouseDbus->getDominantHand());

    // restore the value before test
    m_pMouseDbus->setDominantHand(currentValue);
}

TEST_F(MouseInterfaceTest, nondominantHand)
{
    // get current value
    QString originKey = m_pMouseDbus->DOMINANT_HAND_KEY;
    m_pMouseDbus->DOMINANT_HAND_KEY = "";
    bool currentValue = m_pMouseDbus->getDominantHand();

    // call set function to set different value
    m_pMouseDbus->setDominantHand(!currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(!currentValue, m_pMouseDbus->getDominantHand());

    // restore the value before test
    m_pMouseDbus->DOMINANT_HAND_KEY = originKey;
}

TEST_F(MouseInterfaceTest, scrollDirection)
{
    // get current value
    bool currentValue = m_pMouseDbus->getScrollDirection();

    // call set function to set different value
    m_pMouseDbus->setScrollDirection(!currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(!currentValue, m_pMouseDbus->getScrollDirection());

    // restore the value before test
    m_pMouseDbus->setScrollDirection(currentValue);
}

TEST_F(MouseInterfaceTest, nonscrollDirection)
{
    // get current value
    QString originKey = m_pMouseDbus->SCROLL_DIRECTION_KEY;
    m_pMouseDbus->SCROLL_DIRECTION_KEY = "";
    bool currentValue = m_pMouseDbus->getScrollDirection();

    // call set function to set different value
    m_pMouseDbus->setScrollDirection(!currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(!currentValue, m_pMouseDbus->getScrollDirection());

    // restore the value before test
    m_pMouseDbus->SCROLL_DIRECTION_KEY = originKey;
}


TEST_F(MouseInterfaceTest, wheelSpeed)
{
    // get current value
    int currentValue = m_pMouseDbus->getWheelSpeed();

    // call set function to set different value
    m_pMouseDbus->setWheelSpeed(++currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(currentValue, m_pMouseDbus->getWheelSpeed());

    m_pMouseDbus->resetKey("wheelSpeed");
    int wheelSpeed = m_pMouseDbus->getWheelSpeed();
    ASSERT_EQ(wheelSpeed, 1);

    // restore the value before test
    m_pMouseDbus->setWheelSpeed(--currentValue);
}

TEST_F(MouseInterfaceTest, nonwheelSpeed)
{
    // get current value
    QString originKey = m_pMouseDbus->WHEEL_SPEED_KEY;
    m_pMouseDbus->WHEEL_SPEED_KEY = "";
    int currentValue = m_pMouseDbus->getWheelSpeed();

    // call set function to set different value
    m_pMouseDbus->setWheelSpeed(++currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(currentValue, m_pMouseDbus->getWheelSpeed());

    // restore the value before test
    m_pMouseDbus->SCROLL_DIRECTION_KEY = originKey;
}

TEST_F(MouseInterfaceTest, doubleClickInterval)
{
    // get current value
    int currentValue = m_pMouseDbus->getDoubleClick();

    // call set function to set different value
    m_pMouseDbus->setDoubleClick(++currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(currentValue, m_pMouseDbus->getDoubleClick());

    // restore the value before test
    m_pMouseDbus->setDoubleClick(--currentValue);
}

TEST_F(MouseInterfaceTest, nondoubleClickInterval)
{
    // get current value
    QString originKey = m_pMouseDbus->DOUBLECLICK_KEY;
    m_pMouseDbus->DOUBLECLICK_KEY = "";
    int currentValue = m_pMouseDbus->getDoubleClick();

    // call set function to set different value
    m_pMouseDbus->setDoubleClick(++currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(currentValue, m_pMouseDbus->getDoubleClick());

    // restore the value before test
    m_pMouseDbus->DOUBLECLICK_KEY = originKey;
}

TEST_F(MouseInterfaceTest, pointerSpeed)
{
    // get current value
    int currentValue = m_pMouseDbus->getPointerSpeed();

    // call set function to set different value
    m_pMouseDbus->setPointerSpeed(++currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(currentValue, m_pMouseDbus->getPointerSpeed());

    // restore the value before test
    m_pMouseDbus->setPointerPosition(--currentValue);
}

TEST_F(MouseInterfaceTest, nonpointerSpeed)
{
    // get current value
    QString originKey = m_pMouseDbus->POINTER_SPEED_KEY;
    m_pMouseDbus->POINTER_SPEED_KEY = "";
    int currentValue = m_pMouseDbus->getPointerSpeed();

    // call set function to set different value
    m_pMouseDbus->setPointerSpeed(++currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(currentValue, m_pMouseDbus->getPointerSpeed());

    // restore the value before test
    m_pMouseDbus->POINTER_SPEED_KEY = originKey;
}

TEST_F(MouseInterfaceTest, mouseAcceleration)
{
    // get current value
    bool currentValue = m_pMouseDbus->getMouseAcceleration();

    // call set function to set different value
    m_pMouseDbus->setMouseAcceleration(!currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(!currentValue, m_pMouseDbus->getMouseAcceleration());

    // restore the value before test
    m_pMouseDbus->setMouseAcceleration(currentValue);
}

TEST_F(MouseInterfaceTest, nonmouseAcceleration)
{
    // get current value
    QString originKey = m_pMouseDbus->MOUSE_ACCELERATION_KEY;
    m_pMouseDbus->MOUSE_ACCELERATION_KEY = "";
    bool currentValue = m_pMouseDbus->getMouseAcceleration();

    // call set function to set different value
    m_pMouseDbus->setMouseAcceleration(!currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(!currentValue, m_pMouseDbus->getMouseAcceleration());

    // restore the value before test
    m_pMouseDbus->MOUSE_ACCELERATION_KEY = originKey;
}

TEST_F(MouseInterfaceTest, mouseTailLight)
{
    // get current value
    bool currentValue = m_pMouseDbus->getTailLightEffect();

    // call set function to set different value
    m_pMouseDbus->setTailLightEffect(!currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(!currentValue, m_pMouseDbus->getTailLightEffect());
}


TEST_F(MouseInterfaceTest, mouseShakeCursor)
{
    // get current value
    bool currentValue = m_pMouseDbus->getShakeCursor();

    // call set function to set different value
    m_pMouseDbus->setShakeCursor(!currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(!currentValue, m_pMouseDbus->getShakeCursor());
}

TEST_F(MouseInterfaceTest, nonShakeCursor)
{
    // get current value
    QString originKey = m_pMouseDbus->SHAKE_CURSOR;
    m_pMouseDbus->SHAKE_CURSOR = "";
    bool currentValue = m_pMouseDbus->getShakeCursor();

    // call set function to set different value
    m_pMouseDbus->setShakeCursor(!currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(!currentValue, m_pMouseDbus->getShakeCursor());

    // restore the value before test
    m_pMouseDbus->SHAKE_CURSOR = originKey;
}

TEST_F(MouseInterfaceTest, pointerPosition)
{
    // get current value
    bool currentValue = m_pMouseDbus->getPointerPosition();

    // call set function to set different value
    m_pMouseDbus->setPointerPosition(!currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(!currentValue, m_pMouseDbus->getPointerPosition());

    // restore the value before test
    m_pMouseDbus->setPointerPosition(currentValue);
}

TEST_F(MouseInterfaceTest, nonpointerPosition)
{
    // get current value
    QString originKey = m_pMouseDbus->POINTER_POSITION_KEY;
    m_pMouseDbus->POINTER_POSITION_KEY = "";
    bool currentValue = m_pMouseDbus->getPointerPosition();

    // call set function to set different value
    m_pMouseDbus->setPointerPosition(!currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(!currentValue, m_pMouseDbus->getPointerPosition());

    // restore the value before test
    m_pMouseDbus->POINTER_POSITION_KEY = originKey;
}

TEST_F(MouseInterfaceTest, pointerSize)
{
    // get current value
    int currentValue = m_pMouseDbus->getPointerSize();

    // call set function to set different value
    m_pMouseDbus->setPointerSize(++currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(currentValue, m_pMouseDbus->getPointerSize());

    m_pMouseDbus->resetKey("cursorSize");
    int pointerSize = m_pMouseDbus->getPointerSize();
    ASSERT_EQ(pointerSize, 24);

    // restore the value before test
    m_pMouseDbus->setPointerSize(--currentValue);
}

TEST_F(MouseInterfaceTest, nonpointerSize)
{
    // get current value
    QString originKey = m_pMouseDbus->POINTER_SIZE_KEY;
    m_pMouseDbus->POINTER_SIZE_KEY = "";
    int currentValue = m_pMouseDbus->getPointerSize();

    // call set function to set different value
    m_pMouseDbus->setPointerSize(++currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(currentValue, m_pMouseDbus->getPointerSize());

    // restore the value before test
    m_pMouseDbus->POINTER_SIZE_KEY = originKey;
}

TEST_F(MouseInterfaceTest, blinkCursorOnText)
{
    // get current value
    bool currentValue = m_pMouseDbus->getCursorBlink();

    // call set function to set different value
    m_pMouseDbus->setCursorBlink(!currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(!currentValue, m_pMouseDbus->getCursorBlink());

    // restore the value before test
    m_pMouseDbus->setCursorBlink(currentValue);
}

TEST_F(MouseInterfaceTest, nonblinkCursorOnText)
{
    // get current value
    QString originKey = m_pMouseDbus->CURSOR_BLINK_KEY;
    m_pMouseDbus->CURSOR_BLINK_KEY = "";
    bool currentValue = m_pMouseDbus->getCursorBlink();

    // call set function to set different value
    m_pMouseDbus->setCursorBlink(!currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(!currentValue, m_pMouseDbus->getCursorBlink());

    // restore the value before test
    m_pMouseDbus->CURSOR_BLINK_KEY = originKey;
}


TEST_F(MouseInterfaceTest, cursorSpeed)
{
    // get current value
    int currentValue = m_pMouseDbus->getCursorSpeed();

    // call set function to set different value
    m_pMouseDbus->setCursorSpeed(++currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(currentValue, m_pMouseDbus->getCursorSpeed());

    // restore the value before test
    m_pMouseDbus->setCursorSpeed(--currentValue);
}

TEST_F(MouseInterfaceTest, noncursorSpeed)
{
    // get current value
    QString originKey = m_pMouseDbus->CURSOR_SPEED_KEY;
    m_pMouseDbus->CURSOR_SPEED_KEY = "";
    int currentValue = m_pMouseDbus->getCursorSpeed();

    // call set function to set different value
    m_pMouseDbus->setCursorSpeed(++currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(currentValue, m_pMouseDbus->getCursorSpeed());

    // restore the value before test
    m_pMouseDbus->CURSOR_SPEED_KEY = originKey;
}

TEST_F(MouseInterfaceTest, styleCursorSpeed)
{
    // get current value
    int currentValue = m_pMouseDbus->getCursorSpeed();

    // call set function to set different value
    m_pMouseDbus->setStyleCursorSpeed(++currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(currentValue, m_pMouseDbus->getCursorSpeed());

    // restore the value before test
    m_pMouseDbus->setStyleCursorSpeed(--currentValue);
}

TEST_F(MouseInterfaceTest, nonStyleCursorSpeed)
{
    // get current value
    QString originKey = m_pMouseDbus->CURSOR_BLINK_KEY;
    m_pMouseDbus->CURSOR_BLINK_KEY = "";
    int currentValue = m_pMouseDbus->getCursorSpeed();

    // call set function to set different value
    m_pMouseDbus->setStyleCursorSpeed(++currentValue);

    // test the current value and the value you have set
    ASSERT_EQ(currentValue, m_pMouseDbus->getCursorSpeed());

    // restore the value before test
    m_pMouseDbus->CURSOR_BLINK_KEY = originKey;
}
