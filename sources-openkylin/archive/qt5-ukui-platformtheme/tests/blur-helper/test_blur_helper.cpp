/*
 * Unit tests for blur-helper.cpp — offscreen, no show/exec/processEvents/qWait.
 */

#include <QtTest>
#include <QMenu>
#include <QX11Info>

#include "blur-helper.h"
#include "settings/black-list.h"

class TestableBlurHelper : public BlurHelper
{
public:
    explicit TestableBlurHelper(QObject *parent = nullptr, bool systemEnable = true)
        : BlurHelper(parent, systemEnable)
    {
    }
    void callConfirmBlurEnableDelay() { confirmBlurEnableDelay(); }
};

class TestBlurHelper : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testConstructAndBlurToggle();
    void testShouldSkipAndBlackList();
    void testEventFilterWithoutShow();
    void testRegisterNonWindowWidget();
    void testDelayUpdateAndConfirmBlur();
    void testMenuAndTipLabelPaths();
    void testBlurEnableWithRegisteredWidgets();
};

void TestBlurHelper::testConstructAndBlurToggle()
{
    BlurHelper blur(nullptr, true);
    blur.onBlurEnableChanged(true);
    blur.onBlurEnableChanged(false);

    BlurHelper blurOff(nullptr, false);
    blurOff.onBlurEnableChanged(true);
    blurOff.onBlurEnableChanged(false);
}

void TestBlurHelper::testShouldSkipAndBlackList()
{
    BlurHelper blur(nullptr, true);
    (void)blur.isApplicationInBlackList();
    QVERIFY(!blur.blackList().isEmpty());
    QVERIFY(!blackAppListWithBlurHelper().isEmpty());

    QWidget plain;
    QVERIFY(blur.shouldSkip(&plain));

    plain.setProperty("useSystemStyleBlur", true);
    QVERIFY(!blur.shouldSkip(&plain));

    QWidget skip;
    skip.setProperty("doNotBlur", true);
    blur.registerWidget(&skip);

    QMenu menu;
    menu.setProperty("useSystemStyleBlur", true);
    QVERIFY(!blur.shouldSkip(&menu));
}

void TestBlurHelper::testEventFilterWithoutShow()
{
    BlurHelper blur(nullptr, true);

    QWidget child;
    QEvent paint(QEvent::Paint);
    QVERIFY(!blur.eventFilter(&child, &paint));

    QWidget window;
    window.setWindowFlags(Qt::Window);
    QVERIFY(window.isWindow());

    blur.onBlurEnableChanged(false);
    QEvent updateReq(QEvent::UpdateRequest);
    QVERIFY(!blur.eventFilter(&window, &updateReq));

    blur.onBlurEnableChanged(true);
    QEvent layoutReq(QEvent::LayoutRequest);
    QVERIFY(!blur.eventFilter(nullptr, &layoutReq));

    blur.delayUpdate(&window, true);
    blur.delayUpdate(&window, false);
    blur.onWidgetDestroyed(&window);

    window.setProperty("useSystemStyleBlur", true);
    window.setProperty("blurRegion", QRegion(QRect(0, 0, 40, 40)));
    blur.registerWidget(&window);
    QEvent updateReq2(QEvent::UpdateRequest);
    QEvent layoutReq2(QEvent::LayoutRequest);
    QEvent hideEv(QEvent::Hide);
    QVERIFY(!blur.eventFilter(&window, &updateReq2));
    QVERIFY(!blur.eventFilter(&window, &layoutReq2));
    QVERIFY(!blur.eventFilter(&window, &hideEv));

    if (QX11Info::isPlatformX11())
        blur.unregisterWidget(&window);
}

void TestBlurHelper::testRegisterNonWindowWidget()
{
    BlurHelper blur(nullptr, true);
    QWidget panel;
    panel.setProperty("useSystemStyleBlur", true);
    blur.registerWidget(&panel);
    blur.unregisterWidget(&panel);
}

void TestBlurHelper::testDelayUpdateAndConfirmBlur()
{
    TestableBlurHelper blur(nullptr, true);
    blur.callConfirmBlurEnableDelay();

    QWidget window;
    window.setWindowFlags(Qt::Window);
    window.resize(120, 80);
    window.setProperty("useSystemStyleBlur", true);
    window.setProperty("blurRegion", QRegion(QRect(0, 0, 80, 60)));
    window.setMask(QRegion(QRect(0, 0, 80, 60)));
    blur.registerWidget(&window);
    blur.delayUpdate(&window, true);
    blur.delayUpdate(&window, false);

    if (QX11Info::isPlatformX11())
        blur.unregisterWidget(&window);
}

void TestBlurHelper::testMenuAndTipLabelPaths()
{
    BlurHelper blur(nullptr, true);

    QMenu menu;
    menu.setProperty("useSystemStyleBlur", true);
    menu.resize(80, 40);
    QVERIFY(!blur.shouldSkip(&menu));
    blur.registerWidget(&menu);
    blur.delayUpdate(&menu, false);
    blur.delayUpdate(&menu, true);

    QWidget blocked;
    blocked.setProperty("doNotBlur", true);
    blur.registerWidget(&blocked);
}

void TestBlurHelper::testBlurEnableWithRegisteredWidgets()
{
    TestableBlurHelper blur(nullptr, true);
    QWidget window;
    window.setWindowFlags(Qt::Window);
    window.setProperty("useSystemStyleBlur", true);
    window.setProperty("blurRegion", QRegion(QRect(0, 0, 60, 40)));
    blur.registerWidget(&window);

    blur.onBlurEnableChanged(true);
    blur.onBlurEnableChanged(false);
    blur.onBlurEnableChanged(true);

    if (QX11Info::isPlatformX11())
        blur.unregisterWidget(&window);
}

QTEST_MAIN(TestBlurHelper)
#include "test_blur_helper.moc"
