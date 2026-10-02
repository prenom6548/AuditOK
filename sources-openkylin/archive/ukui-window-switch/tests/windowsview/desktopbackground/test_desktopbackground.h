#ifndef TEST_DESKTOPBACKGROUND_H
#define TEST_DESKTOPBACKGROUND_H

#include <QObject>
#include <QString>
#include <QImage>
#include <QRect>
#include <QPainter>
#include <QQuickItem>
#include <QQuickPaintedItem>
#include <QRectF>

class DesktopBackground;
class BackgroundImageHandler;

class TestDesktopBackground : public QObject
{
    Q_OBJECT
public:
    explicit TestDesktopBackground(QObject *parent = nullptr);
    ~TestDesktopBackground();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // DesktopBackground constructor/destructor tests
    void testConstructor();
    void testDestructor();

    // blurAlpha tests
    void testBlurAlphaGetterSetter();
    void testBlurAlphaBoundaryValues();
    void testBlurAlphaChangedSignal();

    // scaledRect tests
    void testScaledRectEqualAspectRatio();
    void testScaledRectWidthShorter();
    void testScaledRectHeightShorter();
    void testScaledRectNormalCase();
    void testScaledRectZeroDimension();

    // getDestRect tests
    void testGetDestRectEqualScale();
    void testGetDestRectImageSmaller();
    void testGetDestRectImageLarger();
    void testGetDestRectPixmapScaleLessThanScreen();
    void testGetDestRectPixmapScaleGreaterThanScreen();

    // Static methods tests
    void testStaticSetNonBlurBackground();
    void testStaticSetBlurBackground();
    void testStaticGetSystemScalingFactor();
    void testStaticGetBackgroundFile();
    void testStaticGetBackgroundFileFromAccount();
    void testStaticGetBackgroundFillStyle();
    void testGetBackgroundPrimaryColor();

    // updateBlurBackground and onBlurAlphaChanged
    void testUpdateBlurBackgroundWithNullBackground();
    void testUpdateBlurBackgroundWithValidBackground();
    void testOnBlurAlphaChangedWithNullBackground();
    void testOnBlurAlphaChangedWithValidBackground();

    // Paint tests
    void testPaintWithPureColor();
    void testPaintWithImageNoBlur();
    void testPaintWithImageBlur();
    void testPaintWithFillStyleCentered();
    void testPaintWithFillStyleWallpaper();
    void testPaintWithFillStyleScaled();
    void testPaintWithFillStyleStretched();
    void testPaintWithFillStyleZoom();
    void testPaintWithFillStyleSpanned();
    void testPaintWithFillStyleDefault();
    void testPaintWithFillStyleNone();

    // BackgroundImageHandler tests
    void testEdgeCases();
    void testGaussFunc1D();
    void testGetKernal();
    void testBlurImageBasic();
    void testBlurImageLarge();
    void testBlurImageMonoFormat();
    void testBlurImageWidthExceedsMax();
    void testBlurImageSmallGCD();
    void testBlurImageLargeBlurRadius();
    void testLoadBackgroundImageAsync();
    void testBlurImageAsync();

    void testInit();
    
    void testGSettingsBoundaryCases();
    void testInitBoundaryCases();
    void testPaintFillStyleBoundaryCases();
    void testRectCalculationBoundaryCases();
    void testBlurImageBoundaryCases();
    void testLoadBackgroundImageBoundaryCases();
    void testSetBlurAlphaBoundaryCases();
    void testPaintZeroSizeBoundaryCases();
    void testScaledRectOffsetCases();
    
private:
    DesktopBackground *m_background;
};

#endif // TEST_DESKTOPBACKGROUND_H
