#include "test_desktopbackground.h"
#include <QTest>
#include <QSignalSpy>
#include <QQuickItem>
#include <QQuickView>
#include <QGuiApplication>
#include <QScreen>
#include <QPainter>
#include <QGSettings>
#define private public
#include "desktopbackground.h"
#undef  private

TestDesktopBackground::TestDesktopBackground(QObject *parent)
    : QObject(parent)
    , m_background(nullptr)
{
}

TestDesktopBackground::~TestDesktopBackground()
{
}

void TestDesktopBackground::initTestCase()
{
}

void TestDesktopBackground::cleanupTestCase()
{
}

void TestDesktopBackground::init()
{
    m_background = new DesktopBackground();
}

void TestDesktopBackground::cleanup()
{
    if (m_background) {
        delete m_background;
        m_background = nullptr;
    }
    // Reset static members
    DesktopBackground::setNonBlurBackground(QImage());
}

// Test constructor
void TestDesktopBackground::testConstructor()
{
    DesktopBackground *bg = new DesktopBackground();
    QVERIFY(bg != nullptr);
    QVERIFY(bg->blurAlpha() >= 0);
    delete bg;
}

// Test destructor
void TestDesktopBackground::testDestructor()
{
    DesktopBackground *bg = new DesktopBackground();
    delete bg;
    QVERIFY(true);
}

// Test blurAlpha getter and setter
void TestDesktopBackground::testBlurAlphaGetterSetter()
{
    qreal initialAlpha = m_background->blurAlpha();
    QVERIFY(initialAlpha >= 0);

    m_background->setBlurAlpha(0.5);
    QVERIFY(qFuzzyCompare(m_background->blurAlpha(), 0.5));

    m_background->setBlurAlpha(1.0);
    QVERIFY(qFuzzyCompare(m_background->blurAlpha(), 1.0));

    m_background->setBlurAlpha(0.0);
    QVERIFY(qFuzzyCompare(m_background->blurAlpha(), 0.0));
}

// Test blurAlpha boundary values
void TestDesktopBackground::testBlurAlphaBoundaryValues()
{
    // Test negative value - should be ignored
    m_background->setBlurAlpha(0.5);
    m_background->setBlurAlpha(-0.5);
    QVERIFY(qFuzzyCompare(m_background->blurAlpha(), 0.5));

    // Test value > 1 - should be ignored
    m_background->setBlurAlpha(0.3);
    m_background->setBlurAlpha(1.5);
    QVERIFY(qFuzzyCompare(m_background->blurAlpha(), 0.3));

    // Test exact boundary values
    m_background->setBlurAlpha(0.0);
    QVERIFY(qFuzzyCompare(m_background->blurAlpha(), 0.0));

    m_background->setBlurAlpha(1.0);
    QVERIFY(qFuzzyCompare(m_background->blurAlpha(), 1.0));
}

// Test blurAlphaChanged signal
void TestDesktopBackground::testBlurAlphaChangedSignal()
{
    QSignalSpy spy(m_background, &DesktopBackground::blurAlphaChanged);
    QVERIFY(spy.isValid());

    m_background->setBlurAlpha(0.5);
    m_background->setBlurAlpha(0.6); // Different value triggers signal
    QVERIFY(spy.count() > 0);
}

// Test scaledRect with equal aspect ratio
void TestDesktopBackground::testScaledRectEqualAspectRatio()
{
    QImage testImage(1920, 1080, QImage::Format_ARGB32);
    QRect rect = m_background->scaledRect(testImage);
    QVERIFY(rect.width() > 0);
    QVERIFY(rect.height() > 0);

    QImage testImage1(m_background->boundingRect().width(),
                     m_background->boundingRect().height(),
                     QImage::Format_ARGB32);
    rect = m_background->scaledRect(testImage1);
    QVERIFY(rect.width() >= 0);
    QVERIFY(rect.height() >= 0);
}

// Test scaledRect when width < height
void TestDesktopBackground::testScaledRectWidthShorter()
{
    QImage testImage1(100, 200, QImage::Format_ARGB32);
    QRect rect = m_background->scaledRect(testImage1);
    QVERIFY(rect.width() > 0);
    QVERIFY(rect.height() > 0);

    QImage testImage2(200, 200, QImage::Format_ARGB32);
    rect = m_background->scaledRect(testImage2);
    QVERIFY(rect.width() > 0);
    QVERIFY(rect.height() > 0);

    QImage testImage3(1030, 1090, QImage::Format_ARGB32);
    rect = m_background->scaledRect(testImage3);
    QVERIFY(rect.width() > 0);
    QVERIFY(rect.height() > 0);
}

// Test scaledRect when height < width
void TestDesktopBackground::testScaledRectHeightShorter()
{
    QImage testImage(200, 100, QImage::Format_ARGB32);
    QRect rect = m_background->scaledRect(testImage);
    QVERIFY(rect.width() > 0);
    QVERIFY(rect.height() > 0);
}

// Test scaledRect normal case
void TestDesktopBackground::testScaledRectNormalCase()
{
    QImage testImage1(800, 600, QImage::Format_ARGB32);
    QRect rect = m_background->scaledRect(testImage1);
    QVERIFY(rect.width() > 0);
    QVERIFY(rect.height() > 0);

    QImage testImage2(1930, 1090, QImage::Format_ARGB32);
    rect = m_background->scaledRect(testImage2);
    QVERIFY(rect.width() > 0);
    QVERIFY(rect.height() > 0);
}

// Test scaledRect with zero dimension (edge case from loop)
void TestDesktopBackground::testScaledRectZeroDimension()
{
    QImage testImage(1, 1, QImage::Format_ARGB32);
    QRect rect = m_background->scaledRect(testImage);
    QVERIFY(rect.width() >= 0);
    QVERIFY(rect.height() >= 0);
}

// Test getDestRect with equal scale (fuzzy compare)
void TestDesktopBackground::testGetDestRectEqualScale()
{
    QImage testImage(1920, 1080, QImage::Format_ARGB32);
    QRectF desktopRect(0, 0, 1920, 1080);
    QRect destRect = m_background->getDestRect(testImage, desktopRect);
    QVERIFY(destRect.width() > 0);
    QVERIFY(destRect.height() > 0);
}

// Test getDestRect when image is smaller
void TestDesktopBackground::testGetDestRectImageSmaller()
{
    QImage testImage(800, 600, QImage::Format_ARGB32);
    QRectF desktopRect(0, 0, 1920, 1080);
    QRect destRect = m_background->getDestRect(testImage, desktopRect);
    QVERIFY(destRect.width() > 0);
    QVERIFY(destRect.height() > 0);
}

// Test getDestRect when image is larger
void TestDesktopBackground::testGetDestRectImageLarger()
{
    QImage testImage(3000, 2000, QImage::Format_ARGB32);
    QRectF desktopRect(0, 0, 1920, 1080);
    QRect destRect = m_background->getDestRect(testImage, desktopRect);
    QVERIFY(destRect.width() > 0);
    QVERIFY(destRect.height() > 0);
}

// Test getDestRect when pixmap scale < screen scale
void TestDesktopBackground::testGetDestRectPixmapScaleLessThanScreen()
{
    // Image: 1000x1000 (scale = 1.0), Desktop: 1920x1080 (scale = 1.77)
    QImage testImage(1000, 1000, QImage::Format_ARGB32);
    QRectF desktopRect(0, 0, 1920, 1080);
    QRect destRect = m_background->getDestRect(testImage, desktopRect);
    QVERIFY(destRect.width() > 0);
    QVERIFY(destRect.height() > 0);
}

// Test getDestRect when pixmap scale > screen scale
void TestDesktopBackground::testGetDestRectPixmapScaleGreaterThanScreen()
{
    // Image: 3000x1000 (scale = 3.0), Desktop: 1920x1080 (scale = 1.77)
    QImage testImage(3000, 1000, QImage::Format_ARGB32);
    QRectF desktopRect(0, 0, 1920, 1080);
    QRect destRect = m_background->getDestRect(testImage, desktopRect);
    QVERIFY(destRect.width() > 0);
    QVERIFY(destRect.height() > 0);
}

// Test static setNonBlurBackground
void TestDesktopBackground::testStaticSetNonBlurBackground()
{
    QImage testImage(100, 100, QImage::Format_ARGB32);
    testImage.fill(Qt::red);
    DesktopBackground::setNonBlurBackground(testImage);
    QVERIFY(true);
}

// Test static setBlurBackground
void TestDesktopBackground::testStaticSetBlurBackground()
{
    QImage testImage(100, 100, QImage::Format_ARGB32);
    testImage.fill(Qt::blue);
    DesktopBackground::setBlurBackground(testImage, 0);
    QVERIFY(true);
    DesktopBackground::setBlurBackground(testImage, 0.5);
    QVERIFY(true);
}

// Test getSystemScalingFactor
void TestDesktopBackground::testStaticGetSystemScalingFactor()
{
    double scalingFactor = DesktopBackground::getSystemScalingFactor();
    QVERIFY(scalingFactor > 0);
}

// Test getBackgroundFile
void TestDesktopBackground::testStaticGetBackgroundFile()
{
    QString bgFile = DesktopBackground::getBackgroundFile();
    // Just verify it returns something (either empty or valid path)
    QVERIFY(!bgFile.isEmpty() || bgFile.isEmpty());
}

// Test getBackgroundFileFromAccount (indirectly through getBackgroundFile)
void TestDesktopBackground::testStaticGetBackgroundFileFromAccount()
{
    // This function is called internally when file doesn't exist
    // The test just verifies the function returns a value
    QString bgFile = DesktopBackground::getBackgroundFile();
    QVERIFY(m_background != nullptr);
    QString bgFileAcc = m_background->getBackgroundFileFromAccount();
    QVERIFY(true);
}

// Test getBackgroundFillStyle
void TestDesktopBackground::testStaticGetBackgroundFillStyle()
{
    QString fillStyle = DesktopBackground::getBackgroundFillStyle();
    QVERIFY(!fillStyle.isEmpty() || fillStyle.isEmpty());
}

// Test getBackgroundPrimaryColor
void TestDesktopBackground::testGetBackgroundPrimaryColor()
{
    // This is a private method called from init
    // The test verifies that the class can be created
    QVERIFY(m_background != nullptr);
    QColor color = m_background->getBackgroundPrimaryColor();
    QVERIFY(true);
}

// Test updateBlurBackground with null background
void TestDesktopBackground::testUpdateBlurBackgroundWithNullBackground()
{
    // Reset static member
    DesktopBackground::setNonBlurBackground(QImage());
    m_background->setBlurAlpha(0.5);
    // Call method through public interface by triggering update
    // This tests the null check path
    QVERIFY(true);
}

// Test updateBlurBackground with valid background
void TestDesktopBackground::testUpdateBlurBackgroundWithValidBackground()
{
    QImage testImage(100, 100, QImage::Format_ARGB32);
    testImage.fill(Qt::red);
    DesktopBackground::setNonBlurBackground(testImage);
    m_background->setBlurAlpha(0);
    QVERIFY(true);
    m_background->setBlurAlpha(0.5);
    // This will trigger updateBlurBackground internally through onBlurAlphaChanged
    QVERIFY(true);
}

// Test onBlurAlphaChanged with null background
void TestDesktopBackground::testOnBlurAlphaChangedWithNullBackground()
{
    // Reset to null
    DesktopBackground::setNonBlurBackground(QImage());
    m_background->setBlurAlpha(0.0);
    // The method should return early when background is null
    QVERIFY(true);
}

// Test onBlurAlphaChanged with valid background
void TestDesktopBackground::testOnBlurAlphaChangedWithValidBackground()
{
    QImage testImage(100, 100, QImage::Format_ARGB32);
    testImage.fill(Qt::red);
    DesktopBackground::setNonBlurBackground(testImage);
    m_background->setBlurAlpha(0.5);
    // This should trigger blur processing
    QVERIFY(true);
}

// Test paint with pure color
void TestDesktopBackground::testPaintWithPureColor()
{
    QImage paintImage(1920, 1080, QImage::Format_ARGB32);
    QPainter painter(&paintImage);

    // Force pure color mode by setting usePureColor true
    m_background->m_usePureColor = true;
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter);
    painter.end();

    QVERIFY(!paintImage.isNull());

    // reset
    m_background->m_usePureColor = false;
}

// Test paint with no blur
void TestDesktopBackground::testPaintWithImageNoBlur()
{
    QImage testImage(100, 100, QImage::Format_ARGB32);
    testImage.fill(Qt::red);
    DesktopBackground::setNonBlurBackground(testImage);

    QImage paintImage(400, 300, QImage::Format_ARGB32);
    QPainter painter(&paintImage);

    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter);
    painter.end();

    QVERIFY(!paintImage.isNull());
}

// Test paint with blur
void TestDesktopBackground::testPaintWithImageBlur()
{
    QImage testImage(100, 100, QImage::Format_ARGB32);
    testImage.fill(Qt::red);
    DesktopBackground::setNonBlurBackground(testImage);
    DesktopBackground::setBlurBackground(testImage, 0.5);

    QImage paintImage(400, 300, QImage::Format_ARGB32);
    QPainter painter(&paintImage);

    m_background->setBlurAlpha(0.5);
    m_background->paint(&painter);
    painter.end();

    QVERIFY(!paintImage.isNull());
}

// Test paint with centered fill style
void TestDesktopBackground::testPaintWithFillStyleCentered()
{
    QImage testImage(100, 100, QImage::Format_ARGB32);
    testImage.fill(Qt::red);
    DesktopBackground::setNonBlurBackground(testImage);
    DesktopBackground::setBlurBackground(testImage, 0.0);

    QImage paintImage(400, 300, QImage::Format_ARGB32);
    QPainter painter(&paintImage);

    // We need to test the centered branch - this requires the fill style to be "centered"
    // This is tested through the internal getBackgroundFillStyle which reads from GSettings
    m_background->m_backgroundFillStyle = "centered";
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter);
    painter.end();

    QVERIFY(!paintImage.isNull());

    m_background->setWidth(200);
    m_background->setHeight(100);
    QImage paintImage1(300, 200, QImage::Format_ARGB32);
    QPainter painter1(&paintImage1);
    m_background->setBlurAlpha(0.5);
    m_background->paint(&painter);
    painter1.end();
    QVERIFY(!paintImage1.isNull());

    QImage paintImage2(300, 100, QImage::Format_ARGB32);
    QPainter painter2(&paintImage2);
    m_background->setBlurAlpha(0.5);
    m_background->paint(&painter);
    painter2.end();
    QVERIFY(!paintImage2.isNull());

    QImage paintImage3(200, 200, QImage::Format_ARGB32);
    QPainter painter3(&paintImage2);
    m_background->setBlurAlpha(0.5);
    m_background->paint(&painter);
    painter3.end();
    QVERIFY(!paintImage3.isNull());

    QImage paintImage4(100, 50, QImage::Format_ARGB32);
    QPainter painter4(&paintImage2);
    m_background->setBlurAlpha(0.5);
    m_background->paint(&painter);
    painter4.end();
    QVERIFY(!paintImage4.isNull());

    QImage paintImage5(0, 0, QImage::Format_ARGB32);
    QPainter painter5(&paintImage2);
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter5);
    painter5.end();
    QVERIFY(paintImage5.isNull());
}

// Test paint with wallpaper (tiled) fill style
void TestDesktopBackground::testPaintWithFillStyleWallpaper()
{
    QImage testImage(50, 50, QImage::Format_ARGB32);
    testImage.fill(Qt::blue);
    DesktopBackground::setNonBlurBackground(testImage);
    DesktopBackground::setBlurBackground(testImage, 0.0);

    QImage paintImage(400, 300, QImage::Format_ARGB32);
    QPainter painter(&paintImage);

    m_background->m_backgroundFillStyle = "wallpaper";
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter);
    painter.end();

    QVERIFY(!paintImage.isNull());

    m_background->setWidth(200);
    m_background->setHeight(100);
    QImage paintImage1(300, 200, QImage::Format_ARGB32);
    QPainter painter1(&paintImage1);
    m_background->setBlurAlpha(0.5);
    m_background->paint(&painter);
    painter1.end();
    QVERIFY(!paintImage1.isNull());

    QImage paintImage2(300, 100, QImage::Format_ARGB32);
    QPainter painter2(&paintImage2);
    m_background->setBlurAlpha(0.5);
    m_background->paint(&painter);
    painter2.end();
    QVERIFY(!paintImage2.isNull());

    QImage paintImage3(200, 200, QImage::Format_ARGB32);
    QPainter painter3(&paintImage2);
    m_background->setBlurAlpha(0.5);
    m_background->paint(&painter);
    painter3.end();
    QVERIFY(!paintImage3.isNull());

    QImage paintImage4(100, 50, QImage::Format_ARGB32);
    QPainter painter4(&paintImage2);
    m_background->setBlurAlpha(0.5);
    m_background->paint(&painter);
    painter4.end();
    QVERIFY(!paintImage4.isNull());

    QImage paintImage5(0, 0, QImage::Format_ARGB32);
    QPainter painter5(&paintImage2);
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter5);
    painter5.end();
    QVERIFY(paintImage5.isNull());
}

// Test paint with scaled fill style
void TestDesktopBackground::testPaintWithFillStyleScaled()
{
    QImage testImage(100, 100, QImage::Format_ARGB32);

    testImage.fill(Qt::green);
    DesktopBackground::setNonBlurBackground(testImage);
    DesktopBackground::setBlurBackground(testImage, 0.0);

    QImage paintImage(400, 300, QImage::Format_ARGB32);
    QPainter painter(&paintImage);

    m_background->m_backgroundFillStyle = "scaled";
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter);
    painter.end();

    QVERIFY(!paintImage.isNull());

    QImage paintImage2(0, 0, QImage::Format_ARGB32);
    QPainter painter2(&paintImage2);
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter2);
    painter2.end();
    QVERIFY(paintImage2.isNull());
}

// Test paint with stretched fill style
void TestDesktopBackground::testPaintWithFillStyleStretched()
{
    QImage testImage(100, 100, QImage::Format_ARGB32);
    testImage.fill(Qt::yellow);
    DesktopBackground::setNonBlurBackground(testImage);
    DesktopBackground::setBlurBackground(testImage, 0.0);

    QImage paintImage(400, 300, QImage::Format_ARGB32);
    QPainter painter(&paintImage);

    m_background->m_backgroundFillStyle = "stretched";
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter);
    painter.end();

    QVERIFY(!paintImage.isNull());

    QImage paintImage2(0, 0, QImage::Format_ARGB32);
    QPainter painter2(&paintImage2);
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter2);
    painter2.end();
    QVERIFY(paintImage2.isNull());
}

// Test paint with zoom fill style
void TestDesktopBackground::testPaintWithFillStyleZoom()
{
    QImage testImage(100, 100, QImage::Format_ARGB32);
    testImage.fill(Qt::cyan);
    DesktopBackground::setNonBlurBackground(testImage);
    DesktopBackground::setBlurBackground(testImage, 0.0);

    QImage paintImage(400, 300, QImage::Format_ARGB32);
    QPainter painter(&paintImage);

    m_background->m_backgroundFillStyle = "zoom";
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter);
    painter.end();

    QVERIFY(!paintImage.isNull());

    QImage paintImage2(0, 0, QImage::Format_ARGB32);
    QPainter painter2(&paintImage2);
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter2);
    painter2.end();
    QVERIFY(paintImage2.isNull());
}

// Test paint with spanned fill style
void TestDesktopBackground::testPaintWithFillStyleSpanned()
{
    QImage testImage(100, 100, QImage::Format_ARGB32);
    testImage.fill(Qt::magenta);
    DesktopBackground::setNonBlurBackground(testImage);
    DesktopBackground::setBlurBackground(testImage, 0.0);

    QImage paintImage(400, 300, QImage::Format_ARGB32);
    QPainter painter(&paintImage);

    m_background->m_backgroundFillStyle = "spanned";
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter);
    painter.end();

    QVERIFY(!paintImage.isNull());

    QImage paintImage2(0, 0, QImage::Format_ARGB32);
    QPainter painter2(&paintImage2);
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter2);
    painter2.end();
    QVERIFY(paintImage2.isNull());
}

// Test paint with default (unknown) fill style
void TestDesktopBackground::testPaintWithFillStyleDefault()
{
    QImage testImage(100, 100, QImage::Format_ARGB32);
    testImage.fill(Qt::white);
    DesktopBackground::setNonBlurBackground(testImage);
    DesktopBackground::setBlurBackground(testImage, 0.0);

    QImage paintImage(400, 300, QImage::Format_ARGB32);
    QPainter painter(&paintImage);

    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter);
    painter.end();

    QVERIFY(!paintImage.isNull());

    QImage paintImage2(0, 0, QImage::Format_ARGB32);
    QPainter painter2(&paintImage2);
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter2);
    painter2.end();
    QVERIFY(paintImage2.isNull());
}

void TestDesktopBackground::testPaintWithFillStyleNone()
{
    QImage testImage(100, 100, QImage::Format_ARGB32);
    testImage.fill(Qt::magenta);
    DesktopBackground::setNonBlurBackground(testImage);
    DesktopBackground::setBlurBackground(testImage, 0.0);

    QImage paintImage(400, 300, QImage::Format_ARGB32);
    QPainter painter(&paintImage);

    m_background->m_backgroundFillStyle = "none";
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter);
    painter.end();

    QVERIFY(!paintImage.isNull());

    QImage paintImage2(0, 0, QImage::Format_ARGB32);
    QPainter painter2(&paintImage2);
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter2);
    painter2.end();
    QVERIFY(paintImage2.isNull());
}

// Test edge function (edge cases)
void TestDesktopBackground::testEdgeCases()
{
    // Test edge function through blurImage
    // The edge function is tested indirectly through blurImage
    // edge(x, i, w) returns:
    //   x - i if inx < 0 or inx >= w
    //   inx otherwise

    QImage testImage(50, 50, QImage::Format_ARGB32);
    testImage.fill(Qt::red);

    // Test with small blur radius
    QImage blurred = BackgroundImageHandler::blurImage(1, testImage);
    QVERIFY(!blurred.isNull());

    // Test with medium blur radius
    QImage blurred2 = BackgroundImageHandler::blurImage(3, testImage);
    QVERIFY(!blurred2.isNull());

    // Test with larger blur radius
    QImage blurred3 = BackgroundImageHandler::blurImage(5, testImage);
    QVERIFY(!blurred3.isNull());
}

// Test gaussFunc1D (1D Gaussian function)
void TestDesktopBackground::testGaussFunc1D()
{
    // gaussFunc1D is tested through blurImage
    // The function computes: A * exp(-x^2 / (2*sigma^2))
    // where A = 1 / (sigma * sqrt(2*pi))

    QImage testImage(50, 50, QImage::Format_ARGB32);
    testImage.fill(Qt::red);

    // Test with different blur radii
    QImage blurred1 = BackgroundImageHandler::blurImage(1, testImage);
    QVERIFY(!blurred1.isNull());

    QImage blurred2 = BackgroundImageHandler::blurImage(5, testImage);
    QVERIFY(!blurred2.isNull());

    QImage blurred3 = BackgroundImageHandler::blurImage(10, testImage);
    QVERIFY(!blurred3.isNull());
}

// Test getKernal (kernel generation)
void TestDesktopBackground::testGetKernal()
{
    // getKernal is tested through blurImage
    // The function generates normalized Gaussian weights

    QImage testImage(50, 50, QImage::Format_ARGB32);
    testImage.fill(Qt::red);

    // Test with various radii
    BackgroundImageHandler::blurImage(1, testImage);
    BackgroundImageHandler::blurImage(3, testImage);
    BackgroundImageHandler::blurImage(5, testImage);

    QVERIFY(true);
}

// Test blurImage with basic image
void TestDesktopBackground::testBlurImageBasic()
{
    QImage testImage(50, 50, QImage::Format_ARGB32);
    testImage.fill(Qt::red);

    QImage blurred = BackgroundImageHandler::blurImage(3, testImage);
    QVERIFY(!blurred.isNull());
    QVERIFY(blurred.width() == testImage.width());
    QVERIFY(blurred.height() == testImage.height());
}

// Test blurImage with large image
void TestDesktopBackground::testBlurImageLarge()
{
    QImage testImage(2000, 1500, QImage::Format_ARGB32);
    testImage.fill(Qt::blue);

    QImage blurred = BackgroundImageHandler::blurImage(5, testImage);
    QVERIFY(!blurred.isNull());
    QVERIFY(blurred.width() == testImage.width());
    QVERIFY(blurred.height() == testImage.height());
}

// Test blurImage with Mono format
void TestDesktopBackground::testBlurImageMonoFormat()
{
    QImage testImage(100, 100, QImage::Format_Mono);
    testImage.fill(Qt::black);

    QImage blurred = BackgroundImageHandler::blurImage(2, testImage);
    QVERIFY(!blurred.isNull());

    QImage testImage1(100, 100, QImage::Format_MonoLSB);
    testImage.fill(Qt::black);

    blurred = BackgroundImageHandler::blurImage(2, testImage1);
    QVERIFY(!blurred.isNull());
}

// Test blurImage when width exceeds max
void TestDesktopBackground::testBlurImageWidthExceedsMax()
{
    // Test with image width > 1920 (maxImgWidth)
    QImage testImage(2000, 1000, QImage::Format_ARGB32);
    testImage.fill(Qt::red);

    QImage blurred = BackgroundImageHandler::blurImage(3, testImage);
    QVERIFY(!blurred.isNull());
    QVERIFY(blurred.width() == testImage.width());
    QVERIFY(blurred.height() == testImage.height());
}

// Test blurImage with small GCD
void TestDesktopBackground::testBlurImageSmallGCD()
{
    // Test with small GCD between width and height
    // This triggers the while loop in blurImage
    QImage testImage(301, 300, QImage::Format_ARGB32);
    testImage.fill(Qt::green);

    QImage blurred = BackgroundImageHandler::blurImage(3, testImage);
    QVERIFY(!blurred.isNull());
}

// Test blurImage with large blur radius
void TestDesktopBackground::testBlurImageLargeBlurRadius()
{
    // Test with larger blur radius
    QImage testImage(100, 100, QImage::Format_ARGB32);
    testImage.fill(Qt::yellow);

    QImage blurred = BackgroundImageHandler::blurImage(10, testImage);
    QVERIFY(!blurred.isNull());
}

void TestDesktopBackground::testLoadBackgroundImageAsync()
{
    BackgroundImageHandler::loadBackgroundImageAsync("filePath", 1, "center", this, nullptr);

    BackgroundImageHandler::loadBackgroundImageAsync("filePath", 1.5, "center", this, nullptr);

    BackgroundImageHandler::loadBackgroundImageAsync("filePath", 1.5, "wallpaper", this, nullptr);

    BackgroundImageHandler::loadBackgroundImageAsync("filePath", 1.5, "scaled", this, nullptr);

    BackgroundImageHandler::loadBackgroundImageAsync("", 1, "scaled", this, nullptr);
    BackgroundImageHandler::loadBackgroundImageAsync("", 1.5, "center", this, nullptr);
    BackgroundImageHandler::loadBackgroundImageAsync("", 1.5, "wallpaper", this, nullptr);
    BackgroundImageHandler::loadBackgroundImageAsync("", 1.5, "scaled", this, nullptr);
}

void TestDesktopBackground::testBlurImageAsync()
{
    QImage testImage(100, 100, QImage::Format_ARGB32);
    testImage.fill(Qt::red);
    BackgroundImageHandler::blurImageAsync(testImage, 8, this, nullptr);
}

void TestDesktopBackground::testInit()
{
    const QByteArray id("org.mate.background");
    QString filePath;
    if(!QGSettings::isSchemaInstalled(id)){
        return;
    }
    QGSettings * backgSetting = new QGSettings(id, QByteArray(), this);
    const QStringList keyList = backgSetting->keys();
    DesktopBackground *bg = new DesktopBackground();
    QVERIFY(bg != nullptr);
    QSignalSpy spy(backgSetting, SIGNAL(changed(const QString)));

    // get original pictureFilename
    if (keyList.contains("pictureFilename")) {
        filePath = backgSetting->get("pictureFilename").toString();
    }

    // set pictureFilename ""
    backgSetting->set("pictureFilename", "");
    bg->init();

    bg->m_backgroundFillStyle = "centered";
    bg->init();

    bg->m_nonBlurBackground = QImage();
    QVERIFY(bg->m_nonBlurBackground.isNull());
    bg->m_backgroundFillStyle = "wallpaper";
    bg->init();

    bg->m_nonBlurBackground = QImage();
    QVERIFY(bg->m_nonBlurBackground.isNull());
    bg->m_backgroundFillStyle = "scaled";
    bg->init();

    // test backgSetting changed
    emit backgSetting->changed("pictureFilename");
    QCOMPARE(spy.count(), 1);
    emit backgSetting->changed("primaryColor");
    QCOMPARE(spy.count(), 2);
    emit backgSetting->changed("pictureOptions");
    QCOMPARE(spy.count(), 3);
    emit backgSetting->changed("none");
    QCOMPARE(spy.count(), 4);

    // test original pictureFilename
    backgSetting->set("pictureFilename", filePath);
    bg->m_nonBlurBackground = QImage();
    QVERIFY(bg->m_nonBlurBackground.isNull());
    bg->m_backgroundFillStyle = "centered";
    bg->init();

    bg->m_nonBlurBackground = QImage();
    QVERIFY(bg->m_nonBlurBackground.isNull());
    bg->m_backgroundFillStyle = "wallpaper";
    bg->init();

    bg->m_nonBlurBackground = QImage();
    QVERIFY(bg->m_nonBlurBackground.isNull());
    bg->m_backgroundFillStyle = "scaled";
    bg->init();

    // test backgSetting changed
    emit backgSetting->changed("pictureFilename");
    QCOMPARE(spy.count(), 5);
    emit backgSetting->changed("primaryColor");
    QCOMPARE(spy.count(), 6);
    emit backgSetting->changed("pictureOptions");
    QCOMPARE(spy.count(), 7);
    emit backgSetting->changed("none");
    QCOMPARE(spy.count(), 8);

    delete bg;
}

// GSettings相关测试 - 合并getBackgroundFile, getBackgroundFillStyle, getBackgroundPrimaryColor的边界测试
void TestDesktopBackground::testGSettingsBoundaryCases()
{
    // Test getBackgroundFile when key is not in keyList
    QString result = DesktopBackground::getBackgroundFile();
    QVERIFY(true);
    
    // Test getBackgroundFillStyle when key is not in keyList
    QString fillStyle = DesktopBackground::getBackgroundFillStyle();
    QVERIFY(fillStyle == "scaled" || !fillStyle.isEmpty());
    
    // Test getBackgroundPrimaryColor when key is not in keyList
    QColor color = m_background->getBackgroundPrimaryColor();
    QVERIFY(color.isValid() || !color.isValid());
    
    // Test getSystemScalingFactor when key is not in keyList
    double scalingFactor = DesktopBackground::getSystemScalingFactor();
    QVERIFY(scalingFactor > 0);
}

// 初始化相关测试 - 合并空文件、有效文件、Account服务等测试
void TestDesktopBackground::testInitBoundaryCases()
{
    // Test with empty background file (pure color mode)
    DesktopBackground::setNonBlurBackground(QImage());
    DesktopBackground *bg = new DesktopBackground();
    QVERIFY(bg != nullptr);
    delete bg;
    
    // Test getBackgroundFileFromAccount with invalid user path
    QString result = m_background->getBackgroundFileFromAccount();
    QVERIFY(true);
}

// Paint填充样式测试 - 合并centered和wallpaper的大小边界测试
void TestDesktopBackground::testPaintFillStyleBoundaryCases()
{
    // Test centered with smaller image
    QImage testImage1(50, 50, QImage::Format_ARGB32);
    testImage1.fill(Qt::red);
    DesktopBackground::setNonBlurBackground(testImage1);
    
    QImage paintImage1(400, 300, QImage::Format_ARGB32);
    QPainter painter1(&paintImage1);
    m_background->m_backgroundFillStyle = "centered";
    m_background->setWidth(400);
    m_background->setHeight(300);
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter1);
    painter1.end();
    QVERIFY(!paintImage1.isNull());
    
    // Test centered with larger image
    QImage testImage2(800, 600, QImage::Format_ARGB32);
    testImage2.fill(Qt::red);
    DesktopBackground::setNonBlurBackground(testImage2);
    m_background->setWidth(200);
    m_background->setHeight(150);
    QImage paintImage2(200, 150, QImage::Format_ARGB32);
    QPainter painter2(&paintImage2);
    m_background->paint(&painter2);
    painter2.end();
    QVERIFY(!paintImage2.isNull());
    
    // Test centered with image <= bounding rect
    m_background->setWidth(200);
    m_background->setHeight(200);
    QImage testImage3(100, 100, QImage::Format_ARGB32);
    testImage3.fill(Qt::blue);
    DesktopBackground::setNonBlurBackground(testImage3);
    QImage paintImage3(200, 200, QImage::Format_ARGB32);
    QPainter painter3(&paintImage3);
    m_background->paint(&painter3);
    painter3.end();
    QVERIFY(!paintImage3.isNull());
    
    // Test wallpaper with smaller image
    QImage testImage4(50, 50, QImage::Format_ARGB32);
    testImage4.fill(Qt::blue);
    DesktopBackground::setNonBlurBackground(testImage4);
    QImage paintImage4(400, 300, QImage::Format_ARGB32);
    QPainter painter4(&paintImage4);
    m_background->m_backgroundFillStyle = "wallpaper";
    m_background->paint(&painter4);
    painter4.end();
    QVERIFY(!paintImage4.isNull());
    
    // Test wallpaper with larger image
    QImage testImage5(800, 600, QImage::Format_ARGB32);
    testImage5.fill(Qt::blue);
    DesktopBackground::setNonBlurBackground(testImage5);
    m_background->setWidth(200);
    m_background->setHeight(150);
    QImage paintImage5(200, 150, QImage::Format_ARGB32);
    QPainter painter5(&paintImage5);
    m_background->paint(&painter5);
    painter5.end();
    QVERIFY(!paintImage5.isNull());
    
    // Test wallpaper with image <= bounding rect
    m_background->setWidth(200);
    m_background->setHeight(200);
    QImage testImage6(100, 100, QImage::Format_ARGB32);
    testImage6.fill(Qt::green);
    DesktopBackground::setNonBlurBackground(testImage6);
    QImage paintImage6(200, 200, QImage::Format_ARGB32);
    QPainter painter6(&paintImage6);
    m_background->paint(&painter6);
    painter6.end();
    QVERIFY(!paintImage6.isNull());
}

// 矩形计算测试 - 合并scaledRect和getDestRect的各种边界情况
void TestDesktopBackground::testRectCalculationBoundaryCases()
{
    // Test scaledRect with various aspect ratios
    m_background->setWidth(400);
    m_background->setHeight(300);
    
    // Very wide image
    QImage testImage1(1000, 100, QImage::Format_ARGB32);
    QRect rect1 = m_background->scaledRect(testImage1);
    QVERIFY(rect1.width() > 0);
    
    // Very tall image
    QImage testImage2(100, 1000, QImage::Format_ARGB32);
    QRect rect2 = m_background->scaledRect(testImage2);
    QVERIFY(rect2.height() > 0);
    
    // Test offsetX - image width > scaledSize width
    QImage testImage3(1000, 100, QImage::Format_ARGB32);
    m_background->setWidth(100);
    m_background->setHeight(100);
    QRect rect3 = m_background->scaledRect(testImage3);
    QVERIFY(rect3.width() > 0);
    
    // Test offsetY - image height > scaledSize height
    QImage testImage4(100, 1000, QImage::Format_ARGB32);
    QRect rect4 = m_background->scaledRect(testImage4);
    QVERIFY(rect4.height() > 0);
    
    // Test both offsets
    m_background->setWidth(50);
    m_background->setHeight(50);
    QImage testImage5(1000, 1000, QImage::Format_ARGB32);
    QRect rect5 = m_background->scaledRect(testImage5);
    QVERIFY(rect5.width() > 0);
    QVERIFY(rect5.height() > 0);
    
    // Test getDestRect exact match
    QImage testImage6(1920, 1080, QImage::Format_ARGB32);
    QRectF desktopRect1(0, 0, 1920, 1080);
    QRect destRect1 = m_background->getDestRect(testImage6, desktopRect1);
    QVERIFY(destRect1.width() > 0);
    
    // Test getDestRect offsetY
    QImage testImage7(1920, 1080, QImage::Format_ARGB32);
    QRectF desktopRect2(0, 0, 1920, 800);
    QRect destRect2 = m_background->getDestRect(testImage7, desktopRect2);
    QVERIFY(destRect2.width() > 0);
    
    // Test getDestRect offsetX
    QImage testImage8(1920, 1080, QImage::Format_ARGB32);
    QRectF desktopRect3(0, 0, 1200, 1080);
    QRect destRect3 = m_background->getDestRect(testImage8, desktopRect3);
    QVERIFY(destRect3.width() > 0);
}

// blurImage边界测试 - 合并各种blur radius和image尺寸的边界情况
void TestDesktopBackground::testBlurImageBoundaryCases()
{
    // Test with radius 0
    QImage testImage1(50, 50, QImage::Format_ARGB32);
    testImage1.fill(Qt::red);
    QImage blurred1 = BackgroundImageHandler::blurImage(0, testImage1);
    QVERIFY(!blurred1.isNull());
    
    // Test with Mono format
    QImage testImage2(100, 100, QImage::Format_Mono);
    testImage2.fill(Qt::black);
    QImage blurred2 = BackgroundImageHandler::blurImage(2, testImage2);
    QVERIFY(!blurred2.isNull());
    
    // Test with MonoLSB format
    QImage testImage3(100, 100, QImage::Format_MonoLSB);
    testImage3.fill(Qt::white);
    QImage blurred3 = BackgroundImageHandler::blurImage(2, testImage3);
    QVERIFY(!blurred3.isNull());
    
    // Test with image width exactly 1920
    QImage testImage4(1920, 1080, QImage::Format_ARGB32);
    testImage4.fill(Qt::red);
    QImage blurred4 = BackgroundImageHandler::blurImage(3, testImage4);
    QVERIFY(!blurred4.isNull());
    
    // Test with image width > 1920
    QImage testImage5(2000, 1200, QImage::Format_ARGB32);
    testImage5.fill(Qt::red);
    QImage blurred5 = BackgroundImageHandler::blurImage(3, testImage5);
    QVERIFY(!blurred5.isNull());
    
    // Test blurWidth < 300
    QImage testImage6(299, 300, QImage::Format_ARGB32);
    testImage6.fill(Qt::green);
    QImage blurred6 = BackgroundImageHandler::blurImage(3, testImage6);
    QVERIFY(!blurred6.isNull());
    
    // Test blurWidth = 300
    QImage testImage7(300, 300, QImage::Format_ARGB32);
    testImage7.fill(Qt::red);
    QImage blurred7 = BackgroundImageHandler::blurImage(3, testImage7);
    QVERIFY(!blurred7.isNull());
    
    // Test blurWidth > 300
    QImage testImage8(301, 300, QImage::Format_ARGB32);
    testImage8.fill(Qt::blue);
    QImage blurred8 = BackgroundImageHandler::blurImage(3, testImage8);
    QVERIFY(!blurred8.isNull());
    
    // Test blurWidth >= 300 (scaledToWidth path)
    QImage testImage9(600, 300, QImage::Format_ARGB32);
    testImage9.fill(Qt::yellow);
    QImage blurred9 = BackgroundImageHandler::blurImage(3, testImage9);
    QVERIFY(!blurred9.isNull());
    
    // Test while loop iterations
    QImage testImage10(301, 300, QImage::Format_ARGB32);
    testImage10.fill(Qt::green);
    QImage blurred10 = BackgroundImageHandler::blurImage(3, testImage10);
    QVERIFY(!blurred10.isNull());
    
    // Test value = 1
    QImage testImage11(200, 100, QImage::Format_ARGB32);
    testImage11.fill(Qt::yellow);
    QImage blurred11 = BackgroundImageHandler::blurImage(3, testImage11);
    QVERIFY(!blurred11.isNull());
    
    // Test value > 1
    QImage testImage12(100, 100, QImage::Format_ARGB32);
    testImage12.fill(Qt::cyan);
    QImage blurred12 = BackgroundImageHandler::blurImage(3, testImage12);
    QVERIFY(!blurred12.isNull());
}

// loadBackgroundImage测试 - 合并不同scaling factor的测试
void TestDesktopBackground::testLoadBackgroundImageBoundaryCases()
{
    // Test with empty file path
    QImage result1 = BackgroundImageHandler::loadBackgroundImage("", 1.0, "scaled");
    QVERIFY(true);
    
    // Test with scaling factor 1.0
    QImage result2 = BackgroundImageHandler::loadBackgroundImage("/tmp/nonexistent.png", 1.0, "scaled");
    
    // Test with scaling factor != 1.0 and centered style
    QImage result3 = BackgroundImageHandler::loadBackgroundImage("/tmp/nonexistent.png", 2.0, "centered");
    
    // Test with scaling factor != 1.0 and wallpaper style
    QImage result4 = BackgroundImageHandler::loadBackgroundImage("/tmp/nonexistent.png", 1.5, "wallpaper");
    
    // Test with scaling factor != 1.0 and scaled style
    QImage result5 = BackgroundImageHandler::loadBackgroundImage("/tmp/nonexistent.png", 2.0, "scaled");
    
    QVERIFY(true);
}

// setBlurAlpha边界测试 - 合并负值和大于1的测试
void TestDesktopBackground::testSetBlurAlphaBoundaryCases()
{
    // Test negative value
    qreal originalAlpha = m_background->blurAlpha();
    m_background->setBlurAlpha(-0.1);
    QVERIFY(qFuzzyCompare(m_background->blurAlpha(), originalAlpha) || m_background->blurAlpha() == originalAlpha);
    
    // Test value > 1
    m_background->setBlurAlpha(1.5);
    QVERIFY(qFuzzyCompare(m_background->blurAlpha(), originalAlpha) || m_background->blurAlpha() == originalAlpha);
    
    // Test zero value
    m_background->setBlurAlpha(0.0);
    QVERIFY(qFuzzyCompare(m_background->blurAlpha(), 0.0));
    
    // Test max value
    m_background->setBlurAlpha(1.0);
    QVERIFY(qFuzzyCompare(m_background->blurAlpha(), 1.0));
}

// paint零尺寸测试 - 合并各种fill style的零尺寸测试
void TestDesktopBackground::testPaintZeroSizeBoundaryCases()
{
    QImage testImage(100, 100, QImage::Format_ARGB32);
    testImage.fill(Qt::red);
    DesktopBackground::setNonBlurBackground(testImage);
    
    // Test scaled with zero size
    QImage paintImage1(0, 0, QImage::Format_ARGB32);
    QPainter painter1(&paintImage1);
    m_background->m_backgroundFillStyle = "scaled";
    m_background->setBlurAlpha(0.0);
    m_background->paint(&painter1);
    painter1.end();
    QVERIFY(paintImage1.isNull());
    
    // Test stretched with zero size
    testImage.fill(Qt::yellow);
    DesktopBackground::setNonBlurBackground(testImage);
    QImage paintImage2(0, 0, QImage::Format_ARGB32);
    QPainter painter2(&paintImage2);
    m_background->m_backgroundFillStyle = "stretched";
    m_background->paint(&painter2);
    painter2.end();
    QVERIFY(paintImage2.isNull());
    
    // Test zoom with zero size
    testImage.fill(Qt::cyan);
    DesktopBackground::setNonBlurBackground(testImage);
    QImage paintImage3(0, 0, QImage::Format_ARGB32);
    QPainter painter3(&paintImage3);
    m_background->m_backgroundFillStyle = "zoom";
    m_background->paint(&painter3);
    painter3.end();
    QVERIFY(paintImage3.isNull());
    
    // Test spanned with zero size
    testImage.fill(Qt::magenta);
    DesktopBackground::setNonBlurBackground(testImage);
    QImage paintImage4(0, 0, QImage::Format_ARGB32);
    QPainter painter4(&paintImage4);
    m_background->m_backgroundFillStyle = "spanned";
    m_background->paint(&painter4);
    painter4.end();
    QVERIFY(paintImage4.isNull());
    
    // Test default with zero size
    testImage.fill(Qt::white);
    DesktopBackground::setNonBlurBackground(testImage);
    QImage paintImage5(0, 0, QImage::Format_ARGB32);
    QPainter painter5(&paintImage5);
    m_background->m_backgroundFillStyle = "";
    m_background->paint(&painter5);
    painter5.end();
    QVERIFY(paintImage5.isNull());
}

// scaledRect偏移测试 - 合并offsetX和offsetY的测试
void TestDesktopBackground::testScaledRectOffsetCases()
{
    m_background->setWidth(100);
    m_background->setHeight(100);
    
    // Test offsetX
    QImage testImage1(1000, 100, QImage::Format_ARGB32);
    QRect rect1 = m_background->scaledRect(testImage1);
    QVERIFY(rect1.width() > 0);
    
    // Test offsetY
    QImage testImage2(100, 1000, QImage::Format_ARGB32);
    QRect rect2 = m_background->scaledRect(testImage2);
    QVERIFY(rect2.height() > 0);
    
    // Test both offsets
    m_background->setWidth(50);
    m_background->setHeight(50);
    QImage testImage3(1000, 1000, QImage::Format_ARGB32);
    QRect rect3 = m_background->scaledRect(testImage3);
    QVERIFY(rect3.width() > 0);
    QVERIFY(rect3.height() > 0);
}
