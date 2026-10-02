/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "displaymanager.h"
#include "kmrewindow.h"
#include "android_display/normaldisplaywidget.h"
#include "android_display/egldisplaywidget.h"
#include "gamekey/gamekeymanager.h"
#include "widgets/loadingwidget.h"
#include "widgets/autohidewidget.h"
#include "widgets/titlebar.h"
#include "widgets/titlebarjoystick.h"
#include "widgets/menu.h"
#include "widgets/warningnotice.h"
#include "widgets/popuptip.h"
#include "widgets/recordscreenwidget.h"
#include "windowstretchworker.h"
#include "app_control_manager.h"
#include "displaybackend/displaybackend.h"
#include "gles/gles_display_helper.h"
#include "gles/gles_display_work_manager.h"
#include "emugl/emugl_display_work_manager.h"
#include "gamekey/JoystickCommon.h"
#include "gamekey/toast.h"
#include "config/appsettings.h"
#include "config/preferences.h"
#include "windowmanager.h"
#include "kmreenv.h"
#include "sessionsettings.h"

#include <syslog.h>
#include <QDesktopWidget>
#include <QApplication>
#include <QGSettings>

#define MIN_WINDOW_WIDTH 320
#define MIN_WINDOW_HEIGHT 540

#define MIN_GLES_MAJOR_VERSION 2

DisplayManager::DisplayManager(KmreWindow *window, int width, int height, QString pkgName) 
    : QObject(window)
    , mMainWindow(window)
    , mAppSizeConfig({false, 0, 0})
    , mInitialWidth(width)
    , mInitialHeight(height)
    , mPkgName(pkgName)
    , mAccessoryDisplayPkgName(pkgName + ":secondary")
    , mImageRotation(0)
    , mCurrentRotation(0)
    , mDisplayMode(DISPLAY_MODE_SINGLE_MAIN)
    , mLastDisplayMode(DISPLAY_MODE_SINGLE_MAIN)
    , mMainDisplayId(-1)
    , mAccessoryDisplayId(-1)
    , mCurrentFocusedDisplayId(-1)
    , mIsWindowMaxSized(false)
    , mWindowUnfocused(false)
    , mIsMultiDisplayEnabled(false)
    , mIsDynamicDisplaySizeSupported(false)
{
    mInitialOrientation = mInitialWidth <= mInitialHeight ? Qt::PortraitOrientation/*纵向*/ : Qt::LandscapeOrientation/*横向*/;
    mIsMultiDisplayEnabled = AppSettings::getInstance().isAppMultiperEnabled(mPkgName);
    mIsDynamicDisplaySizeSupported = AppSettings::getInstance().isAppSupportDDS(mPkgName);

    updateAppSizeConfig();
}

DisplayManager::~DisplayManager()
{
    if (mPopupTip) {
        mPopupTip->close();
    }
}

void DisplayManager::saveWindowGeometry()
{
    if ((!mMainWindow->isFullScreen()) && (!mIsWindowMaxSized)) {
        mPoint = mMainWindow->pos();
        mSize = mMainWindow->size();
    }
}

QRect DisplayManager::getScreenSize()
{
    QDesktopWidget* desktop = QApplication::desktop();
    return desktop->screenGeometry(mMainWindow);
}

QRect DisplayManager::getAvailableScreenSize()
{
    QDesktopWidget* desktop = QApplication::desktop();
    return desktop->availableGeometry(mMainWindow);
}

void DisplayManager::getScreenInfo(QRect &allScreenSize, QRect &availableSize)
{
    QDesktopWidget* desktop = QApplication::desktop();
    allScreenSize = QRect(0, 0, desktop->width(), desktop->height());
    availableSize = desktop->availableGeometry();
}

QSize DisplayManager::getMainWidgetSize() 
{
    return mMainWindow->isFullScreen() ? getScreenSize().size() : mMainWindow->size();
}

void DisplayManager::initUI()
{
    mHLayout = new QHBoxLayout();
    mTitleLayout = new QHBoxLayout();
    mMainLayout = new QVBoxLayout();
    QString titleName = mMainWindow->getTitleName();

    mTitleMenu = mMainWindow->createMenu();// must created before titlebar

    mTitleBar = mMainWindow->createTitlebar();
    mTitleBar->setTitleName(titleName);

    mTitleBarJoystick = mMainWindow->createJoystickTitlebar();
    mTitleBarJoystick->setVisible(false);
    mTitleBarJoystick->setTitle(titleName);

    mGameKeyStackWidget = mMainWindow->creatGameKeyStackWidget();
    mGameKeyStackWidget->setFixedWidth(DEFAULT_GAME_KEY_SET_WIDGET_WIDTH);

    mAutohideWidget = mMainWindow->createAutoHideWidget();
    mAutohideWidget->setAutoHide(true);
    mAutohideWidget->setVisible(false);
    //mLoadingDisplay = mMainWindow->createLoadingWidget();

    if (!createMainDisplay()) {
        syslog(LOG_CRIT, "[DisplayManager] Create main display(%d) failed!", mMainDisplayId);
        return;
    }
    
    mHLayout->setSpacing(0);
    if (mLoadingDisplay) {
        mHLayout->addWidget(mLoadingDisplay);
        mMainDisplay->setVisible(false);
    }
    mHLayout->addWidget(mMainDisplay);
    mGameKeyStackWidget->setVisible(false);
    mHLayout->addWidget(mGameKeyStackWidget);
    mHLayout->setAlignment(Qt::AlignLeft);

    mTitleLayout->setSpacing(0);
    mTitleLayout->setMargin(0);
    mTitleLayout->addWidget(mTitleBar);
    mTitleLayout->addWidget(mTitleBarJoystick);

    mMainLayout->setSpacing(0);
    //显示区域留边框，防止界面拉伸时安卓画面响应鼠标长按事件
    mMainLayout->setContentsMargins(MOUSE_MARGINS, 0, MOUSE_MARGINS, MOUSE_MARGINS);
    mMainLayout->addLayout(mTitleLayout);
    mMainLayout->addLayout(mHLayout);
    mMainLayout->addStretch();

    mMainWindow->setLayout(mMainLayout);

    mStretchWorker = std::make_shared<WindowStretchWorker>(mMainWindow);
}

void DisplayManager::initialize()
{
    if (mAppSizeConfig.fullScreen) {
        mMainWindow->show();// 避免在某些系统下出现全屏启动时显示异常
        enterFullscreen();
    }
    else {
        getDisplaySizeForNormalScreen(mInitialWidth, mInitialHeight, mDisplayWidth, mDisplayHeight, true);

        setDisplaySize();
        setMainWidgetSize();
        
        mMainWindow->move(getWindowInitPos());
    }

    mStretchWorker->init();
}

void DisplayManager::updateAppSizeConfig() 
{
    AppSettings::AppConfig appConfig = AppSettings::getInstance().getAppConfig(mPkgName);
    if (appConfig.bootFullScreen || appConfig.bootMaxSize) {
        mAppSizeConfig.fullScreen = true;
    }
    else {
        QRect rect = getScreenSize();
        if ((appConfig.bootWidth > 0) && (appConfig.bootWidth <= rect.width()) &&
            (appConfig.bootHeight > 0) && (appConfig.bootHeight <= rect.height())) {
            mAppSizeConfig.width = appConfig.bootWidth;
            mAppSizeConfig.height = appConfig.bootHeight;
        }
        else {
            mAppSizeConfig.width = -1;
            mAppSizeConfig.height = -1;
        }
    }

    syslog(LOG_DEBUG, "[%s] fullScreen = %d, width = %d, height = %d", 
            __func__, mAppSizeConfig.fullScreen, mAppSizeConfig.width, mAppSizeConfig.height);
}

void DisplayManager::onExitFullscreen()
{
    if (mMainWindow->isFullScreen()) {
        setDefaultWindowSize();
    }
}

void DisplayManager::onSwitchFullscreen()
{
    if (mMainWindow->isFullScreen()) {
        setDefaultWindowSize();
    }
    else {
        enterFullscreen();
    }
    mMainWindow->setCursor(Qt::ArrowCursor);//qApp->setOverrideCursor(this->cursor());
}

void DisplayManager::enterFullscreen()
{
    syslog(LOG_DEBUG, "[DisplayManager] enterFullscreen...");
    mTitleBar->setVisible(false);
    GameKeyManager *gameKeyManager = mMainWindow->getGameKeyManager();
    if (gameKeyManager && gameKeyManager->isSettingsPanelVisible()) {//如果设置菜单已展开，切换到全屏时，收回设置菜单并保存当前键盘映射的属性值
        mMainWindow->handleGameKey();
    }

    saveWindowGeometry();

    QDesktopWidget* desktop = QApplication::desktop();
    if (desktop) {
        QRect screenRect = desktop->screenGeometry(mMainWindow);
        if (screenRect.width() > 0 && screenRect.height() > 0) {
            mMainWindow->setMaximumSize(screenRect.width(), screenRect.height());
        } else {
            mMainWindow->setMaximumSize(DEFAULT_MAX_WIDTH, DEFAULT_MAX_HEIGHT);
        }
    } else {
        mMainWindow->setMaximumSize(DEFAULT_MAX_WIDTH, DEFAULT_MAX_HEIGHT);
    }

    mStretchWorker->setEnabled(false);
    mMainWindow->showFullScreen();

    mHLayout->setAlignment(Qt::AlignHCenter);
    mMainLayout->setContentsMargins(0, 0, 0, 0);

    resizeWindowWhileFullScreen(mImageRotation, (mDisplayMode != DISPLAY_MODE_SINGLE_ACCESSORY), false);

    mAutohideWidget->deactivate();
    mAutohideWidget->enableGameAction(mMainWindow->isGameKeySettingsEnabled());
    //全屏时显示浮动工具栏
    QTimer::singleShot(10, mAutohideWidget, SLOT(activate()));
    
    updateWidgetSizeAndPos();
}

// for pad mode
void DisplayManager::updateWindowMaxSize(RotationState rs)
{
    syslog(LOG_DEBUG, "[%s] Maxsize window...", __func__);
    QRect geometry = getMaxWindowGeometry(rs);
    if ((geometry.x() >= 0) && (geometry.y() >= 0) && (geometry.width() > 0) && (geometry.height() > 0)) {
        if (mMainWindow->isFullScreen()) {
            syslog(LOG_DEBUG, "[%s] exit full screen first.", __func__);
            if (mMainWindow->getGameKeyManager()->isSettingsPanelVisible()) {//如果设置菜单已展开，退出全屏时，收回设置菜单并保存当前键盘映射的属性值
                mMainWindow->handleGameKey();
            }

            mMainWindow->showNormal();
        }
        else {
            saveWindowGeometry();
        }

        mTitleBar->setVisible(false);
        mHLayout->setAlignment(Qt::AlignLeft);
        mMainLayout->setContentsMargins(0, 0, 0, 0);
        mAutohideWidget->setVisible(false);//解决全屏模式按F11或ESC退出时，浮动窗口不消失的问题
        mAutohideWidget->deactivate();//隐藏浮动工具栏

        resetWindowSize(geometry.size(), true, true);
        mIsWindowMaxSized = true;
        mMainWindow->move(geometry.topLeft());
        mStretchWorker->setEnabled(false);
    }
}

void DisplayManager::updateMainDisplay(int displayId, int width, int height)
{
    syslog(LOG_DEBUG, "[%s] Update main display: id = %d, width = %d, height = %d", __func__, displayId, width, height);
    mMainDisplayId = displayId;
    if (!mWindowUnfocused) {
        mCurrentFocusedDisplayId = mMainDisplayId;
        focusWindow();
    }
    else {
        unFocusWindow();
    }

    mMainDisplay->setAutoUpdate(false);// disable auto updating of QT engine
    mMainDisplay->connectDisplay(mMainDisplayId, width, height);
    
    if ((width != mInitialWidth) || (height != mInitialHeight)) {
        syslog(LOG_DEBUG, "[%s] Update main display size...", __func__);
        // TODO
    }

    mMainDisplay->setDisplaySize(mDisplayWidth, mDisplayHeight);

    showWarningMessage();
}

static bool checkVersion(QOpenGLContext &context, sp<QSurfaceFormat> format)
{
    QSurfaceFormat currSurface = context.format();
    QPair<int,int> currVersion = currSurface.version();
    QPair<int,int> reqVersion = format->version();
    if (currVersion.first > reqVersion.first) {
        return true;
    }

    return (currVersion.first == reqVersion.first && currVersion.second >= reqVersion.second);
}

static sp<QSurfaceFormat> getFirstSupported(std::vector<sp<QSurfaceFormat>> &formats)
{
    QOpenGLContext context;
    for (sp<QSurfaceFormat> format : formats) {
        context.setFormat(*(format.get()));
        if (context.create()) {
            if (checkVersion(context, format)) return format;
        }
    }

    return nullptr;
}

static sp<QSurfaceFormat> getSupportedSurfaceFormat()
{
    std::vector<sp<QSurfaceFormat>> formats;
    sp<QSurfaceFormat> glesFormat = std::make_shared<QSurfaceFormat>();
    glesFormat->setRenderableType(QSurfaceFormat::OpenGLES);
    glesFormat->setVersion(MIN_GLES_MAJOR_VERSION, 0);
    glesFormat->setProfile(QSurfaceFormat::CoreProfile);
    formats.push_back(glesFormat);

    sp<QSurfaceFormat> format = getFirstSupported(formats);
    if (format == nullptr) {
        syslog(LOG_ERR, "[%s]: Failed to get supported format.", __func__);
        return nullptr;
    }

    return format;
}

DisplayWidget* DisplayManager::createDisplay(int displayId, int width, int height, int orientation, QString pkgName, QString displayName)
{
    syslog(LOG_DEBUG, "[%s] displayId = %d, pkgName = %s", __func__, displayId, pkgName.toStdString().c_str());
    DisplayWidget* displayWidget = nullptr;

    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        displayWidget = new EglDisplayWidget(mMainWindow, displayId, width, height, orientation, displayName);
        QSurfaceFormat f = displayWidget->format();
        if ((f.renderableType() != QSurfaceFormat::OpenGLES) || (f.majorVersion() < MIN_GLES_MAJOR_VERSION)) {
            sp<QSurfaceFormat> format = getSupportedSurfaceFormat();
            if (format == nullptr) {
                delete displayWidget;
                displayWidget = nullptr;
                syslog(LOG_ERR, "Failed to find supported surface format for display widget.");
                return nullptr;
            }
            format->setDepthBufferSize(0);
            displayWidget->setFormat(*(format.get()));
        }
    } else {
        displayWidget = new NormalDisplayWidget(mMainWindow, displayId, width, height, orientation, displayName);
    }

    displayWidget->initConnections();
    if (displayWidget->registerDisplay(this)) {
        connect(displayWidget, SIGNAL(sigBack()), mMainWindow, SLOT(onResponseBack()));
        connect(displayWidget, SIGNAL(shareFile(QString, QString, int, bool)), mMainWindow, SLOT(shareFileToAndroid(QString, QString, int, bool)));
    }
    else {
        syslog(LOG_ERR, "[%s] Register display:%d failed!", __func__, displayId);
        delete displayWidget;
        displayWidget = nullptr;
    }

    return displayWidget;
}

bool DisplayManager::createMainDisplay()
{
    mMainDisplay = createDisplay(mMainDisplayId, mInitialWidth, mInitialHeight, mInitialOrientation, mPkgName, mPkgName);
    if (!mMainDisplay) {
        syslog(LOG_ERR, "[DisplayManager] Create display(%d) failed!", mMainDisplayId);
        return false;
    }
    mMainDisplay->updateDDSSupportState(mIsDynamicDisplaySizeSupported);
    connect(mMainDisplay, SIGNAL(requestHideSubWindow()), this, SLOT(onHidePopupSubWindows()));
    return true;
}

bool DisplayManager::createAccessoryDisplay(int displayId)
{
    syslog(LOG_DEBUG, "[DisplayManager] create accessory display(%d)", displayId);
    mAccessoryDisplayId = displayId;
    
    mAccessoryDisplay = createDisplay(mAccessoryDisplayId, mInitialWidth, mInitialHeight, mInitialOrientation, mPkgName, mAccessoryDisplayPkgName);
    if (!mAccessoryDisplay) {
        syslog(LOG_ERR, "DisplayManager: Create accessory display(%d) failed!", mAccessoryDisplayId);
        return false;
    }
    mAccessoryDisplay->updateDDSSupportState(mIsDynamicDisplaySizeSupported);
    mAccessoryDisplay->setVisible(false);
    mHLayout->insertWidget(3, mAccessoryDisplay);
    mAccessoryDisplay->setDisplaySize(mDisplayWidth, mDisplayHeight);
    mAccessoryDisplay->setAutoUpdate(false);

    return true;
}

bool DisplayManager::isWindowStretchReady() 
{
    if (mStretchWorker) {
        return mStretchWorker->isWindowStretchReady();
    }
    return false;
}

void DisplayManager::onDisplayForceRedraw()
{
    if ((mDisplayMode != DISPLAY_MODE_SINGLE_ACCESSORY) && mMainDisplay) {
        mMainDisplay->forceRedraw();
    }
    if ((mDisplayMode != DISPLAY_MODE_SINGLE_MAIN) && mAccessoryDisplay) {
        mAccessoryDisplay->forceRedraw();
    }
}

void DisplayManager::rotationChanged(int rotation, bool isMainDisplay)
{
    syslog(LOG_DEBUG, "[DisplayManager] rotationChanged, rotation = %d, isMainDisplay = %d, mDisplayMode = %d, isFullScreen = %d", 
            rotation, isMainDisplay, mDisplayMode, mMainWindow->isFullScreen());
    hideGameKeysWhileRotation();

    mImageRotation = rotation;
    if (mMainWindow->isFullScreen()) {
        resizeWindowWhileFullScreen(rotation, isMainDisplay, (mCurrentRotation != rotation));
    }
    else {
        resizeWindowWhileNormalScreen(rotation, isMainDisplay, (mCurrentRotation != rotation));
    }
    mCurrentRotation = rotation;

    if (isMainDisplay && mMainDisplay) {
        mMainDisplay->setImageRotation(mImageRotation);
    }
    if ((!isMainDisplay) && mAccessoryDisplay) {
        mAccessoryDisplay->setImageRotation(mImageRotation);
    }

    JoystickManager* joystickManager = mMainWindow->getJoystickManager();
    if (joystickManager) {
        if (joystickManager->isJoystickSetWidgetInitialed() && 
            joystickManager->getGameKeyStatus() != JoystickManager::OnlyInit) {
            Toast::getToast()->setParentAttr(mMainWindow);
            Toast::getToast()->setMessage(tr("Screen rotation, please reset button！"));
        }
    }
}

void DisplayManager::onDisplayRotationChanged(int displayId, int rotation)
{
    if (displayId == mMainDisplayId) {
        rotationChanged(rotation, true);
    }
    else if (displayId == mAccessoryDisplayId) {
        rotationChanged(rotation, false);
    }
}

void DisplayManager::onMultiplierSwitch(QString pkgName, int displayId, bool enable)
{
    syslog(LOG_DEBUG, "[DisplayManager] received MultiplierSwitch signal. pkgName: %s, displayId = %d, enable = %d", 
            pkgName.toStdString().c_str(), displayId, enable);
    if (pkgName == mAccessoryDisplayPkgName) {
        if ((displayId > 0) && (!mAccessoryDisplay)) {
            if (!createAccessoryDisplay(displayId)) {
                return;
            }
        }
        switchMultipler(enable);
    }
}

void DisplayManager::unFocusWindow()
{
    if (mMainDisplay && (!mWindowUnfocused)) {
        syslog(LOG_DEBUG, "[DisplayManager] unFocusWindow: '%d'", mMainDisplayId);
        KmreWindowManager::getInstance()->focusWindowDisplay(-mMainDisplayId);
        mWindowUnfocused = true;
        mMainDisplay->resetControlData();
        if (mAccessoryDisplay) {
            mAccessoryDisplay->resetControlData();
        }
        
        mMainDisplay->changeWidgetFocus();   //防止长按控件时鼠标焦点出去后控件动作不释放
    }
}

void DisplayManager::focusWindow()
{
    if (!mMainWindow->isReadyClose()) {
        if ((mCurrentFocusedDisplayId != mMainDisplayId) && (mCurrentFocusedDisplayId != mAccessoryDisplayId)) {
            mCurrentFocusedDisplayId = (mDisplayMode == DISPLAY_MODE_SINGLE_ACCESSORY) ? mAccessoryDisplayId : mMainDisplayId;
        }
        focusDisplayByDisplayId(mCurrentFocusedDisplayId, true);
        mWindowUnfocused = false;
    }
}

bool DisplayManager::focusDisplayByDisplayId(int displayId, bool force)
{
    if (displayId == mMainDisplayId) {
        if (mMainDisplay && (force || (mCurrentFocusedDisplayId != mMainDisplayId))) {
            KmreWindowManager::getInstance()->focusWindowDisplay(mMainDisplayId);
            mMainDisplay->onRequestActiveWindow();
            mCurrentFocusedDisplayId = mMainDisplayId;
            if (mAccessoryDisplay) {
                mAccessoryDisplay->resetControlData();
            }
            syslog(LOG_DEBUG, "[DisplayManager] focusDisplayByDisplayId, displayId = %d, force = %d", displayId, force);
            return true;
        }
    }
    else if (displayId == mAccessoryDisplayId) {
        if (mAccessoryDisplay && (force || (mCurrentFocusedDisplayId != mAccessoryDisplayId))) {
            KmreWindowManager::getInstance()->focusWindowDisplay(mAccessoryDisplayId);
            mAccessoryDisplay->onRequestActiveWindow();
            mCurrentFocusedDisplayId = mAccessoryDisplayId;
            if (mMainDisplay) {
                mMainDisplay->resetControlData();
            }
            syslog(LOG_DEBUG, "[DisplayManager] focusDisplayByDisplayId, displayId = %d, force = %d", displayId, force);
            return true;
        }
    }
    return false;
}

void DisplayManager::switchMultipler(bool enable)
{
    syslog(LOG_DEBUG, "[DisplayManager] switchMultipler, enable = %d, currentDisplayOrientation = %d, isFullScreen = %d",
            enable, utils::currentDisplayOrientation(mInitialOrientation, mImageRotation), mMainWindow->isFullScreen());
    if (mAccessoryDisplay) {
        bool forceChangeFocus = false;
        DisplayMode newMode = enable ? DISPLAY_MODE_MULTI : DISPLAY_MODE_SINGLE_MAIN;
        if (mDisplayMode != newMode) {
            setDisplayMode(newMode, true);
            autoAdjustMultipler();
            setDisplaySize();
            setMainWidgetSize();
            disableInputMethod();
            forceChangeFocus = true;
        }
        mMainWindow->setCursor(Qt::ArrowCursor);//qApp->setOverrideCursor(this->cursor());
        focusDisplayByDisplayId(enable ? mAccessoryDisplayId : mMainDisplayId, forceChangeFocus);
    }
}

void DisplayManager::autoAdjustMultipler()
{
    if ((mMainWindow->isFullScreen() || mIsWindowMaxSized) && (mDisplayMode == DISPLAY_MODE_MULTI)) {
        QRect screenSize = getScreenSize();
        int displayOrientation = utils::currentDisplayOrientation(mInitialOrientation, mImageRotation);
        if ((screenSize.width() < screenSize.height()) && (displayOrientation == Qt::PortraitOrientation)) {
            syslog(LOG_DEBUG, "[DisplayManager] autoAdjustMultipler...");
            setDisplayMode(DISPLAY_MODE_SINGLE_ACCESSORY);
        }
    }
}

QRect DisplayManager::getMaxWindowGeometry(RotationState rs)
{
    QSize screenSize = getAvailableScreenSize().size();
    if ((screenSize.width() <= 0) || (screenSize.height() <= 0)) {
        syslog(LOG_ERR, "[%s] Get screen size failed!", __func__);
        return QRect(0, 0, 0, 0);
    }

    int screenWidth = screenSize.width();
    int screenHeight = screenSize.height();
    int maxWidth = 0;
    int maxHeight = 0;
    int displayOrientation = utils::currentDisplayOrientation(mInitialOrientation, mImageRotation);
    syslog(LOG_DEBUG, "[%s] screenWidth = %d, screenHeight = %d, mInitialWidth = %d, mInitialHeight = %d, displayOrientation = %d, rs = %d", 
        __func__, screenWidth, screenHeight, mInitialWidth, mInitialHeight, displayOrientation, rs);

    if (mIsDynamicDisplaySizeSupported) {
        maxWidth = screenWidth;
        maxHeight = screenHeight;
    }
    else {
        float screenAspect = screenWidth / static_cast<float>(screenHeight);
        float windowAspect = 0;

        windowAspect = (mInitialWidth + (MOUSE_MARGINS * 2)) / static_cast<float>(mInitialHeight + DEFAULT_TITLEBAR_HEIGHT);

        if (displayOrientation == Qt::PortraitOrientation) {//窗口竖屏
            windowAspect = (windowAspect > 1.f) ? (1.f / windowAspect) : windowAspect;
        }
        else {
            windowAspect = (windowAspect > 1.f) ?  windowAspect : (1.f / windowAspect);
        }
        
        if (screenAspect > windowAspect) {
            maxHeight = screenHeight;
            maxWidth = maxHeight * windowAspect;
        }
        else {
            maxWidth = screenWidth;
            maxHeight = maxWidth / windowAspect;
        }
    }

    if (screenWidth > screenHeight) {//显示器横屏
        if (displayOrientation == Qt::PortraitOrientation) {//窗口竖屏
            if (rs != ROTATION_NONE) {
                switchPortraitOrientation(rs == ROTATION_MAIN_DISPLAY);
            }
            if (mDisplayMode == DISPLAY_MODE_SINGLE_ACCESSORY) {
                setDisplayMode(DISPLAY_MODE_MULTI);
            }
            if ((!mIsDynamicDisplaySizeSupported) && (mDisplayMode == DISPLAY_MODE_MULTI)) {
                maxWidth = (maxWidth - MOUSE_MARGINS) * 2;
            }
        }
        else {//窗口横屏
            if (rs != ROTATION_NONE) {
                switchLandScapeOrientation(rs == ROTATION_MAIN_DISPLAY);
            }
        }
    }
    else {//显示器竖屏
        if (displayOrientation == Qt::PortraitOrientation) {//窗口竖屏
            if (rs != ROTATION_NONE) {
                switchPortraitOrientation(rs == ROTATION_MAIN_DISPLAY);
            }
            if (mDisplayMode == DISPLAY_MODE_MULTI) {
                setDisplayMode(DISPLAY_MODE_SINGLE_ACCESSORY);
            }
        }
        else {//窗口横屏
            if (rs != ROTATION_NONE) {
                switchLandScapeOrientation(rs == ROTATION_MAIN_DISPLAY);
            }
        }
    }


    int xPos = (screenWidth - maxWidth) / 2;
    int yPos = (screenHeight - maxHeight) / 2;

    syslog(LOG_DEBUG, "[%s] xPos = %d, yPos = %d, maxWidth = %d, maxHeight = %d, mDisplayMode = %d", 
        __func__, xPos, yPos, maxWidth, maxHeight, mDisplayMode);
    return QRect(xPos, yPos, maxWidth, maxHeight);
}

//全屏的时候才调用该函数
void DisplayManager::resizeWindowWhileFullScreen(int rotation, bool isMainDisplay, bool rotationChanged)
{
    int screenWidth = 0;
    int screenHeight = 0;
    int displayOrientation = utils::currentDisplayOrientation(mInitialOrientation, rotation);
    syslog(LOG_DEBUG, "[DisplayManager][%s] rotation = %d, isMainDisplay = %d, rotationChanged = %d, displayOrientation = %d", 
            __func__, rotation, isMainDisplay, rotationChanged, displayOrientation);

    QRect screenSize = getScreenSize();
    if (screenSize.width() > 0 && screenSize.height() > 0) {
        screenWidth = screenSize.width();
        screenHeight = screenSize.height();
    }

    if (screenWidth == 0 || screenHeight == 0) {
        mStretchWorker->setEnabled(true);
        mMainWindow->showNormal();
        resizeWindowWhileNormalScreen(mImageRotation, (mDisplayMode != DISPLAY_MODE_SINGLE_ACCESSORY), rotationChanged);
        return;
    }

    syslog(LOG_DEBUG, "[DisplayManager] screenWidth = %d, screenHeight = %d", screenWidth, screenHeight);
    if (screenWidth > screenHeight) {//显示器横屏
        if (displayOrientation == Qt::PortraitOrientation) {//窗口竖屏
            int origWidth = (mInitialOrientation == Qt::PortraitOrientation) ? mInitialWidth : mInitialHeight;
            int origHeight = (mInitialOrientation == Qt::PortraitOrientation) ? mInitialHeight : mInitialWidth;

            mDisplayWidth = mIsDynamicDisplaySizeSupported ? screenWidth : (screenHeight * (origWidth / static_cast<double>(origHeight)));
            mDisplayHeight = screenHeight;
            if (rotationChanged) {
                switchPortraitOrientation(isMainDisplay);
            }
            if (mDisplayMode == DISPLAY_MODE_SINGLE_ACCESSORY) {
                setDisplayMode(DISPLAY_MODE_MULTI);
            }
            setDisplaySize();
        }
        else {//窗口横屏
            mDisplayWidth = screenWidth;
            mDisplayHeight = screenHeight;

            if (rotationChanged) {
                switchLandScapeOrientation(isMainDisplay);
            }
            setDisplaySize();
        }
    }
    else {//显示器竖屏
        if (displayOrientation == Qt::PortraitOrientation) {//窗口竖屏
            mDisplayWidth = screenWidth;
            mDisplayHeight = screenHeight;
            if (rotationChanged) {
                switchPortraitOrientation(isMainDisplay);
            }
            autoAdjustMultipler();
            setDisplaySize();
        }
        else {//窗口横屏
            int origWidth = (mInitialOrientation == Qt::PortraitOrientation) ? mInitialHeight : mInitialWidth;
            int origHeight = (mInitialOrientation == Qt::PortraitOrientation) ? mInitialWidth : mInitialHeight;

            mDisplayWidth = screenWidth;
            mDisplayHeight = mIsDynamicDisplaySizeSupported ? screenHeight : (screenWidth * (origHeight / static_cast<double>(origWidth)));

            if (rotationChanged) {
                switchLandScapeOrientation(isMainDisplay);
            }
            setDisplaySize();
        }
    }

    GameKeyManager *gameKeyManager = mMainWindow->getGameKeyManager();
    if (gameKeyManager && gameKeyManager->isSettingsPanelVisible()) {
        gameKeyManager->saveGameKeys();
    }

    updateWidgetSizeAndPos();

    mMainDisplay->changeInputBoxPos();
    if (mAccessoryDisplay) {
        mAccessoryDisplay->changeInputBoxPos();
    }
}

void DisplayManager::resizeWindowWhileNormalScreen(int rotation, bool isMainDisplay, bool rotationChanged, bool adjustLastSize)
{
    //QPoint p = mPoint;//先取出原来端坐标，再继续后续屏幕旋转后尺寸等的计算
    syslog(LOG_DEBUG, "[DisplayManager] resizeWindowWhileNormalScreen, rotation = %d, isMainDisplay = %d, rotationChanged = %d", 
            rotation, isMainDisplay, rotationChanged);

    //解决初始界面为倒立的横屏（270度），且会在最终显示切换为垂直屏幕的应用的显示bug，如蓝信+App
    if (mInitialOrientation == Qt::PortraitOrientation) {//原本为纵向（竖立）
        if (rotation == 1 || rotation == 3) {//横屏显示
            getDisplaySizeForNormalScreen(mInitialHeight, mInitialWidth, mDisplayWidth, mDisplayHeight, (!rotationChanged) && adjustLastSize);
            if (rotationChanged) {
                switchLandScapeOrientation(isMainDisplay);
            }
        }
        else {//竖屏显示
            getDisplaySizeForNormalScreen(mInitialWidth, mInitialHeight, mDisplayWidth, mDisplayHeight, (!rotationChanged) && adjustLastSize);
            if (rotationChanged) {
                switchPortraitOrientation(isMainDisplay);
            }
        }
    }
    else {//原本为横向（水平）
        if (rotation == 1 || rotation == 3) {//竖屏显示(example: 蓝信+App)
            //mInitialHeight:540, mInitialWidth:960, mDisplayWidth:960, mDisplayHeight:540
            getDisplaySizeForNormalScreen(mInitialHeight, mInitialWidth, mDisplayWidth, mDisplayHeight, (!rotationChanged) && adjustLastSize);
            if (rotationChanged) {
                switchPortraitOrientation(isMainDisplay);
            }
        }
        else {//横屏显示
            getDisplaySizeForNormalScreen(mInitialWidth, mInitialHeight, mDisplayWidth, mDisplayHeight, (!rotationChanged) && adjustLastSize);
            if (rotationChanged) {
                switchLandScapeOrientation(isMainDisplay);
            }
        }
    }

    setDisplaySize();
    setMainWidgetSize();

    mMainWindow->move(mPoint);//最后使用原来端坐标设置界面的位置
    mMainWindow->setCursor(Qt::ArrowCursor);//qApp->setOverrideCursor(this->cursor());

    GameKeyManager *gameKeyManager = mMainWindow->getGameKeyManager();
    if (gameKeyManager && gameKeyManager->isSettingsPanelVisible()) {
        gameKeyManager->saveGameKeys();
    }

    updateWidgetSizeAndPos();

    static bool start_completed = false;
    if (start_completed) {
        disableInputMethod();
    }
    start_completed = true;
}

void DisplayManager::switchLandScapeOrientation(bool isMainDisplay)
{
    syslog(LOG_DEBUG, "[DisplayManager] switchLandScapeOrientation, isMainDisplay = %d, mDisplayMode = %d", 
            isMainDisplay, mDisplayMode);
    if (mDisplayMode == DISPLAY_MODE_MULTI) {
        mMainDisplay->setVisible(isMainDisplay);
        if (mAccessoryDisplay) {
            mAccessoryDisplay->setVisible(!isMainDisplay);
        }
        setDisplayMode(isMainDisplay ? DISPLAY_MODE_SINGLE_MAIN : DISPLAY_MODE_SINGLE_ACCESSORY);
    }
}

void DisplayManager::switchPortraitOrientation(bool isMainDisplay)
{
    syslog(LOG_DEBUG, "[DisplayManager] switchPortraitOrientation, isMainDisplay = %d, mLastDisplayMode = %d", 
            isMainDisplay, mLastDisplayMode);
    if (mLastDisplayMode == DISPLAY_MODE_MULTI) {
        mMainDisplay->setVisible(true);
        if (mAccessoryDisplay) {
            mAccessoryDisplay->setVisible(true);
        }
        setDisplayMode(DISPLAY_MODE_MULTI);
    }
}

//窗口界面拉伸后，更新各控件的固定尺寸
void DisplayManager::resetWindowSize(QSize size, bool updateDisplay, bool hideEdge)
{
    // syslog(LOG_DEBUG, "[DisplayManager][%s] sz.width = %d, sz.height = %d, updateDisplay = %d", 
    //     __func__, size.width(), size.height(), updateDisplay);
    mMainWindow->setFixedSize(size);

    if (updateDisplay) {
        if (hideEdge) {
            mDisplayWidth = size.width();
            if (mDisplayMode == DISPLAY_MODE_MULTI) {
                mDisplayWidth /= 2;
            }
            mDisplayHeight = size.height();
        }
        else {
            if (mDisplayMode == DISPLAY_MODE_MULTI) {
                mDisplayWidth = (size.width() - DEFAULT_SLIDER_WIDTH_OR_HEIGHT - (MOUSE_MARGINS * 2)) / 2;
            }
            else {
                mDisplayWidth = size.width() - DEFAULT_SLIDER_WIDTH_OR_HEIGHT - (MOUSE_MARGINS * 2);
            }
            mDisplayHeight = size.height() - DEFAULT_TITLEBAR_HEIGHT - MOUSE_MARGINS;
        }

        if (mIsDynamicDisplaySizeSupported && mMainDisplay) {
            QSize minSize = getMinWindowSize();
            int minWidth = minSize.width() - MOUSE_MARGINS * 2;
            int minHeight = minSize.height() - DEFAULT_TITLEBAR_HEIGHT - MOUSE_MARGINS;

            mMainDisplay->blurUpdate(minWidth, minHeight);// 动态分辨率模式下，防止拉伸窗口出现黑屏
        }

        setDisplaySize();
        updateWidgetSizeAndPos();
    }

    mAppSizeConfig.width = mDisplayWidth;
    mAppSizeConfig.height = mDisplayHeight;
}

void DisplayManager::setMainWidgetSize()
{
    syslog(LOG_DEBUG, "[DisplayManager] setMainWidgetSize, mDisplayMode = %d, mDisplayWidth = %d, mDisplayHeight = %d, isFullScreen = %d", 
            mDisplayMode, mDisplayWidth, mDisplayHeight, mMainWindow->isFullScreen());
    if (!mMainWindow->isFullScreen()) {
        int mainWidgetWidth = (mDisplayMode == DISPLAY_MODE_MULTI) ? (mDisplayWidth * 2) : mDisplayWidth;
        mainWidgetWidth += MOUSE_MARGINS * 2;
        int mainWidgetHeight = mDisplayHeight + DEFAULT_TITLEBAR_HEIGHT + MOUSE_MARGINS;

        mMainWindow->setFixedSize(mainWidgetWidth, mainWidgetHeight);
        mMainWindow->setMinimumSize(mainWidgetWidth, mainWidgetHeight);

        mStretchWorker->updateGeoAndRatio(getMinWindowSize());
    }
}

void DisplayManager::setDisplaySize()
{
    // syslog(LOG_DEBUG, "[DisplayManager] setDisplaySize, mDisplayMode = %d, mDisplayWidth = %d, mDisplayHeight = %d", 
    //         mDisplayMode, mDisplayWidth, mDisplayHeight);
    if (mDisplayMode == DISPLAY_MODE_SINGLE_ACCESSORY) {
        if (mMainDisplay) {
            mMainDisplay->setVisible(false);
        }
    }
    else {
        if (mMainDisplay) {
            mMainDisplay->setDisplaySize(mDisplayWidth, mDisplayHeight);
            mMainDisplay->setVisible(true);
        }
    }

    if (mDisplayMode == DISPLAY_MODE_SINGLE_MAIN) {
        if (mAccessoryDisplay) {
            mAccessoryDisplay->setVisible(false);
        }
    }
    else {
        if (mAccessoryDisplay) {
            mAccessoryDisplay->setDisplaySize(mDisplayWidth, mDisplayHeight);
            mAccessoryDisplay->setVisible(true);
        }
    }
    
    int mainWidgetWidth =  (mDisplayMode == DISPLAY_MODE_MULTI) ? (mDisplayWidth * 2) : mDisplayWidth;
    mAutohideWidget->setWindowWidth(mainWidgetWidth + MOUSE_MARGINS * 2);
    if (mLoadingDisplay && mLoadingDisplay->isVisible()) {
        if (mMainDisplay) {
            mMainDisplay->setVisible(false);
        }
        if (mAccessoryDisplay) {
            mAccessoryDisplay->setVisible(false);
        }
        mLoadingDisplay->setFixedSize(mainWidgetWidth, mDisplayHeight);
    }

    mTitleBar->updateTitleName();
}

void DisplayManager::appResumed()
{
    if (mLoadingDisplay) {
        mLoadingDisplay->setVisible(false);
    }

    if ((mDisplayMode != DISPLAY_MODE_SINGLE_ACCESSORY) && mMainDisplay) {
        mMainDisplay->setVisible(true);
    }
    if ((mDisplayMode != DISPLAY_MODE_SINGLE_MAIN) && mAccessoryDisplay) {
        mAccessoryDisplay->setVisible(true);
    }
    syslog(LOG_DEBUG, "[%s] Resume app, force redraw.", __func__);
    //防止界面显示时是黑屏画面
    onDisplayForceRedraw();
}

RecordScreenWidget* DisplayManager::getRecordScreenWidget(bool create) 
{
    if (create && !mRecordScreenWidget) {
        mRecordScreenWidget = mMainWindow->createRecordScreenWidget();
        mRecordScreenWidget->setVisible(false);
    }

    return mRecordScreenWidget;
}

void DisplayManager::resizeRecordScreenWidget()
{
    if (mRecordScreenWidget) {
        if (mRecordScreenWidget->isRecordStarted()) {
            mRecordScreenWidget->slotHide(false);
        }
        else {
            mRecordScreenWidget->slotHide(true);
        }
        mRecordScreenWidget->move(mMainWindow->width() - MOUSE_MARGINS - RECORD_WIDGET_MIN_WIDTH, DEFAULT_TITLEBAR_HEIGHT);
        mRecordScreenWidget->setFixedSize(RECORD_WIDGET_MIN_WIDTH, mMainWindow->height() - MOUSE_MARGINS - DEFAULT_TITLEBAR_HEIGHT);
        mRecordScreenWidget->updateHideIcon();
    }
}

void DisplayManager::getDisplaySizeForNormalScreen(int innerWidth, int innerHeight, int &outerWidth, int &outerHeight, bool lastSize)
{
    QSize screenSize = QSize(0, 0);
    int screenOrientation = Qt::LandscapeOrientation; //默认显示器横屏

    QDesktopWidget* desktop = QApplication::desktop();
    if (desktop) {
        //screenSize = desktop->screenGeometry(mMainWindow).size();//
        screenSize = desktop->availableGeometry(mMainWindow).size();
        if (screenSize.width() < screenSize.height()) {
            screenOrientation = Qt::PortraitOrientation;//显示器竖屏
        }
    }

    if (screenSize.width() == 0 || screenSize.height() == 0) {
        outerWidth = DEFAULT_VERTICAL_WIDTH;
        outerHeight = DEFAULT_VERTICAL_HEIGHT;
        return;
    }

    QSize defaultSize(0, 0);
    QSize innerSize(innerWidth, innerHeight);
    if (screenOrientation == Qt::LandscapeOrientation) {//显示器横屏
        if (innerWidth < innerHeight) {//窗口竖屏
            if ((innerSize == QSize(1280, 1920)) || 
                (innerSize == QSize(960, 1440)) ||
                (innerSize == QSize(720, 1080))) {
                defaultSize = QSize(540, 810);
            }
            else if ((innerSize == QSize(1080, 1920)) || 
                    (innerSize == QSize(720, 1280))) {
                defaultSize = QSize(450, 800);
            }
            else if (innerSize == QSize(720, 900)) {
                defaultSize = QSize(480, 600);
            }
            else if (innerSize == QSize(540, 960)) {
                defaultSize = QSize(450, 800);
            }
            else if (innerSize == QSize(540, 720)) {
                defaultSize = QSize(450, 600);
            }
            else {
                defaultSize = QSize(450, 800);
            }
        }
        else if (innerWidth > innerHeight) {//窗口横屏
            defaultSize = innerSize;
        }
        else if (innerWidth >= 1280){
            defaultSize = QSize(800, 800);
        }
        else {
            defaultSize = QSize(600, 600);
        }
    }
    else {//显示器竖屏
        if (innerWidth < innerHeight) {//窗口竖屏
            defaultSize = innerSize;
        }
        else if (innerWidth > innerHeight) {//窗口横屏
            if ((innerSize == QSize(1920, 1280)) || 
                (innerSize == QSize(1440, 960)) ||
                (innerSize == QSize(1080, 720))) {
                defaultSize = QSize(540, 810);
            }
            else if ((innerSize == QSize(1920, 1080)) ||
                (innerSize == QSize(1280, 720))) {
                defaultSize = QSize(800, 450);
            }
            else if (innerSize == QSize(900, 720)) {
                defaultSize = QSize(600, 480);
            }
            else if (innerSize == QSize(960, 540)) {
                defaultSize = QSize(800, 450);
            }
            else if (innerSize == QSize(720, 540)) {
                defaultSize = QSize(600, 450);
            }
            else {
                defaultSize = QSize(800, 450);
            }
        }
        else if (innerWidth >= 1280){
            defaultSize = QSize(800, 800);
        }
        else {
            defaultSize = QSize(600, 600);
        }
    }

    int panelHeight = 80;
    int titlebarHeight = DEFAULT_TITLEBAR_HEIGHT;
    int paddingPixels = 50;
    // Attention: 在wayland下，availableGeometry.height()为整个屏幕的高度，而不是可用高度
    KmreWindowManager::getInstance()->getSystemPanelHeight(panelHeight);

    int maxWidth = screenSize.width() - (paddingPixels * 2)  - (MOUSE_MARGINS * 2);
    int maxHeight = screenSize.height() - paddingPixels - titlebarHeight - MOUSE_MARGINS;
    int perfectMaxHeight = screenSize.height() * 0.75;
    int perfectMaxWidth = defaultSize.width() * (perfectMaxHeight / static_cast<double>(defaultSize.height()));

    syslog(LOG_DEBUG, "[DisplayManager][%s] innerWidth = %d, innerHeight = %d, default width = %d, default height = %d, "
            "maxWidth = %d, maxHeight = %d, screenWidth = %d, screenHeight = %d, mIsMultiDisplayEnabled = %d, lastSize = %d", 
        __func__, innerWidth, innerHeight, defaultSize.width(), defaultSize.height(), 
        maxWidth, maxHeight, screenSize.width(), screenSize.height(), mIsMultiDisplayEnabled, lastSize);

    if ((defaultSize.width() <= maxWidth) && (defaultSize.height() <= maxHeight)) {
        if (defaultSize.height() > perfectMaxHeight) {
            outerWidth = perfectMaxWidth;
            outerHeight = perfectMaxHeight;
        }
        else {
            outerWidth = defaultSize.width();
            outerHeight = defaultSize.height();
        }

        if ((!mIsMultiDisplayEnabled) && lastSize) {
            int tmpWidth = screenSize.width() - (MOUSE_MARGINS * 2);
            int tmpHeight = screenSize.height() - titlebarHeight - MOUSE_MARGINS;
            //syslog(LOG_DEBUG, "[DisplayManager][%s] tmpWidth = %d, tmpHeight = %d, outerWidth = %d, outerHeight = %d", 
            //    __func__, tmpWidth, tmpHeight, outerWidth, outerHeight);

            if ((mAppSizeConfig.width >= outerWidth) && (mAppSizeConfig.width <= tmpWidth) &&
                (mAppSizeConfig.height >= outerHeight) && (mAppSizeConfig.height <= tmpHeight)) {
                
                bool expectedRotation = outerWidth >= outerHeight;
                bool lastRotation = mAppSizeConfig.width >= mAppSizeConfig.height;

                //syslog(LOG_DEBUG, "[DisplayManager][%s] mIsDynamicDisplaySizeSupported = %d, expectedRotation = %d, lastRotation = %d", 
                //    __func__, mIsDynamicDisplaySizeSupported, expectedRotation, lastRotation);
                if (mIsDynamicDisplaySizeSupported || (expectedRotation == lastRotation)) {
                    outerWidth = mAppSizeConfig.width;
                    outerHeight = mAppSizeConfig.height;
                    syslog(LOG_DEBUG, "[DisplayManager][%s] set config size: outerWidth = %d, outerHeight = %d", 
                        __func__, outerWidth, outerHeight);
                }
            }
        }
    } 
    else if ((defaultSize.width() > maxWidth) && (defaultSize.height() <= maxHeight)) {
        outerWidth = maxWidth;
        outerHeight = defaultSize.height() * (maxWidth / static_cast<double>(defaultSize.width()));
    } 
    else if ((defaultSize.width() <= maxWidth) && (defaultSize.height() > maxHeight)) {
        outerWidth = perfectMaxWidth;
        outerHeight = perfectMaxHeight;
    } 
    else {
        int innerOrientation = (innerWidth > innerHeight ? Qt::LandscapeOrientation : Qt::PortraitOrientation);

        if (screenOrientation == Qt::LandscapeOrientation && innerOrientation == Qt::PortraitOrientation) {
            outerWidth = defaultSize.width() * (maxHeight / static_cast<double>(defaultSize.height()));
            outerHeight = maxHeight;
        } 
        else if (screenOrientation == Qt::PortraitOrientation && innerOrientation == Qt::LandscapeOrientation) {
            outerWidth = maxWidth;
            outerHeight = defaultSize.height() * (maxWidth / static_cast<double>(defaultSize.width()));
        } 
        else {
            int tmpWidth = maxHeight * (defaultSize.width() / static_cast<double>(defaultSize.height()));
            if (tmpWidth <= maxWidth) {
                outerWidth = tmpWidth;
                outerHeight = maxHeight;
            } 
            else {
                outerWidth = maxWidth;
                outerHeight = maxWidth * (defaultSize.height() / static_cast<double>(defaultSize.width()));
            }
        }
    }

    outerWidth = (outerWidth < MIN_WINDOW_WIDTH) ? MIN_WINDOW_WIDTH : outerWidth;
    outerHeight = (outerHeight < MIN_WINDOW_HEIGHT) ? MIN_WINDOW_HEIGHT : outerHeight;
    mTitleBar->updateTitle(outerWidth);

    syslog(LOG_DEBUG, "[DisplayManager][%s] outerWidth = %d, outerHeight = %d", __func__, outerWidth, outerHeight);
}

QSize DisplayManager::getMinWindowSize()
{
    int displayWidth, displayHeight;
    if (mInitialOrientation == Qt::PortraitOrientation) {//原本为纵向（竖立）
        if (mImageRotation == 1 || mImageRotation == 3) {//横屏显示
            getDisplaySizeForNormalScreen(mInitialHeight, mInitialWidth, displayWidth, displayHeight, false);
        }
        else {//竖屏显示
            getDisplaySizeForNormalScreen(mInitialWidth, mInitialHeight, displayWidth, displayHeight, false);
        }
    }
    else {//原本为横向（水平）
        if (mImageRotation == 1 || mImageRotation == 3) {//竖屏显示(example: 蓝信+App)
            getDisplaySizeForNormalScreen(mInitialHeight, mInitialWidth, displayWidth, displayHeight, false);
        }
        else {//横屏显示
            getDisplaySizeForNormalScreen(mInitialWidth, mInitialHeight, displayWidth, displayHeight, false);
        }
    }

    int minWidth = ((mDisplayMode == DISPLAY_MODE_MULTI) ? (displayWidth * 2) : displayWidth) + (MOUSE_MARGINS * 2);
    int minHeight = displayHeight + DEFAULT_TITLEBAR_HEIGHT + MOUSE_MARGINS;

    syslog(LOG_DEBUG, "[DisplayManager] getMinWindowSize, minWidth = %d, minHeight = %d", minWidth, minHeight);
    return QSize(minWidth, minHeight);
}

QSize DisplayManager::getMaxScreenSize()
{
    QDesktopWidget* desktop = ((QApplication*)QApplication::instance())->desktop();
    int screenNum = desktop->screenNumber(mMainWindow);//Screen holding most of this
    if (screenNum < 0) {
        syslog(LOG_CRIT, "[DisplayManager] getMaxScreenSize, Can't get screen number!");
        return QSize(0, 0);
    }
    QRect screenGeo = desktop->screenGeometry(screenNum);

    //处理panel的高度，在wayland下，availableGeometry.height()为整个屏幕的高度，而不是可用高度
    int panelposition = 0;
    int panelsize = 0;
    if (QGSettings::isSchemaInstalled("org.ukui.panel.settings")) {
        QGSettings settings("org.ukui.panel.settings", "/org/ukui/panel/settings/", this);
        if (settings.keys().contains("panelposition")) {
            panelposition = settings.get("panelposition").toInt();
        }
        if (settings.keys().contains("panelsize")) {
            panelsize = settings.get("panelsize").toInt();
        }
    }
    if (panelposition == 0 || panelposition == 1) {
        screenGeo.setHeight(screenGeo.height() - panelsize);
    }
    else {
        screenGeo.setWidth(screenGeo.width() - panelsize);
    }

    syslog(LOG_DEBUG, "[DisplayManager] getMaxScreenSize, width = %d, height = %d", 
            screenGeo.width(), screenGeo.height());
    return QSize(screenGeo.width(), screenGeo.height());
}

void DisplayManager::enableInputMethod(int id, bool enable, int x, int y) 
{
    if (mMainDisplay && (mDisplayMode != DISPLAY_MODE_SINGLE_ACCESSORY)) {
        mMainDisplay->enableInputMethod(id, enable, x, y);
    }
    if (mAccessoryDisplay && (mDisplayMode != DISPLAY_MODE_SINGLE_MAIN)) {
        mAccessoryDisplay->enableInputMethod(id, enable, x, y);
    }
}

void DisplayManager::disableInputMethod()
{
    if (mMainDisplay && (mDisplayMode != DISPLAY_MODE_SINGLE_ACCESSORY)) {
        enableInputMethod(mMainDisplayId, false, 0, 0);
    }
    if (mAccessoryDisplay && (mDisplayMode != DISPLAY_MODE_SINGLE_MAIN)) {
        enableInputMethod(mAccessoryDisplayId, false, 0, 0);
    }
}

void DisplayManager::enableDisplay(int displayId, bool enable) 
{
    if ((displayId == mMainDisplayId) && mMainDisplay) {
        mMainDisplay->enableDisplayUpdate(enable);
    }
    else if ((displayId == mAccessoryDisplayId) && mAccessoryDisplay) {
        mAccessoryDisplay->enableDisplayUpdate(enable);
    }
}

void DisplayManager::hide() 
{
    //应用切换到后台，disableUpdateBuffer,不要再进行界面实时更新
    enableDisplay(mMainDisplayId, false);
    enableDisplay(mAccessoryDisplayId, false);
}

void DisplayManager::show() 
{
    //应用切换到前台，enableUpdateBuffer，及时更新界面
    showDisplays();
}

void DisplayManager::showDisplays()
{
    switch (mDisplayMode) {
        case DISPLAY_MODE_SINGLE_MAIN:
            enableDisplay(mMainDisplayId, true);
            enableDisplay(mAccessoryDisplayId, false);
        break;
        case DISPLAY_MODE_SINGLE_ACCESSORY:
            enableDisplay(mMainDisplayId, false);
            enableDisplay(mAccessoryDisplayId, true);
        break;
        case DISPLAY_MODE_MULTI:
            enableDisplay(mMainDisplayId, true);
            enableDisplay(mAccessoryDisplayId, true);
        break;
        default: break;
    }
    onDisplayForceRedraw();
}

void DisplayManager::setDisplayMode(DisplayMode mode, bool switchMultipler)
{
    if (mDisplayMode != mode) {
        mLastDisplayMode = switchMultipler ? mode : mDisplayMode;
        mDisplayMode = mode;
        syslog(LOG_DEBUG, "[DisplayManager] setDisplayMode, mLastDisplayMode = %d, mDisplayMode = %d", 
                mLastDisplayMode, mDisplayMode);
        showDisplays();
    }
}

void DisplayManager::showTip(QString msg, int showTime)
{
    if (!mPopupTip) {
        mPopupTip = std::make_shared<PopupTip>();
    }

    mPopupTip->setTipMessage(msg);
    mPopupTip->popup(mMainWindow->geometry().center(), showTime);
}

bool DisplayManager::isFullScreen()
{
    return mMainWindow->isFullScreen();
}

// 隐藏弹出的子界面，包括menu和tips
void DisplayManager::onHidePopupSubWindows()
{
    if (mPopupTip && mPopupTip->isVisible()) {
        mPopupTip->hide();
    }
}

//进入游戏摇杆设置模式的方法
void DisplayManager::setJoystickEditMode(bool isGoEdit){
    mStretchWorker->setEnabled(!isGoEdit);
    setMainWidgetSize(isGoEdit);
    if(isGoEdit){
        mGameKeyStackWidget->setDefaultStackIndex();
    }
    mGameKeyStackWidget->setVisible(isGoEdit);
    mTitleBar->setVisible(!isGoEdit);
    mTitleBarJoystick->setVisible(isGoEdit);

}

void DisplayManager::requestRationWorker() 
{
    mStretchWorker->setEnabled(true);
}

void DisplayManager::setMainWidgetSize(bool isGoEdit){
    if(isGoEdit){
        if (!mMainWindow->isFullScreen()) {
            int mainWidgetWidth = (mDisplayMode == DISPLAY_MODE_MULTI) ? (mDisplayWidth * 2) : mDisplayWidth;
            mMainWindow->setFixedSize(mainWidgetWidth + MOUSE_MARGINS*2+DEFAULT_GAME_KEY_SET_WIDGET_WIDTH, mDisplayHeight + DEFAULT_TITLEBAR_HEIGHT + MOUSE_MARGINS);
            mMainWindow->setMinimumSize(mainWidgetWidth + MOUSE_MARGINS*2+DEFAULT_GAME_KEY_SET_WIDGET_WIDTH, mDisplayHeight + DEFAULT_TITLEBAR_HEIGHT + MOUSE_MARGINS);
            qDebug() <<"DisplayManager::setMainWidgetSize 设置主窗口的大小为:" <<mainWidgetWidth;
        }
    }else{
        setMainWidgetSize();
    }
}

void DisplayManager::setDefaultWindowSize(bool adjustLastSize)
{
    GameKeyManager *gameKeyManager = mMainWindow->getGameKeyManager();
    if (mMainWindow->isFullScreen()) {
        syslog(LOG_DEBUG, "[DisplayManager] exit full screen...");
        if (gameKeyManager && gameKeyManager->isSettingsPanelVisible()) {//如果设置菜单已展开，退出全屏时，收回设置菜单并保存当前键盘映射的属性值
            mMainWindow->handleGameKey();
        }

        mMainWindow->showNormal();
    }

    mTitleBar->setVisible(true);
    mHLayout->setAlignment(Qt::AlignLeft);
    //显示区域留边框，防止界面拉伸时安卓画面响应鼠标长按事件
    mMainLayout->setContentsMargins(MOUSE_MARGINS, 0, MOUSE_MARGINS, MOUSE_MARGINS);
    mAutohideWidget->setVisible(false);//解决全屏模式按F11或ESC退出时，浮动窗口不消失的问题
    mAutohideWidget->deactivate();//隐藏浮动工具栏

    syslog(LOG_DEBUG, "[DisplayManager] setDefaultWindowSize...");
    resizeWindowWhileNormalScreen(mImageRotation, (mDisplayMode != DISPLAY_MODE_SINGLE_ACCESSORY), false, adjustLastSize);
    updateWidgetSizeAndPos();

    mIsWindowMaxSized = false;
    mStretchWorker->setEnabled(true);
}

void DisplayManager::updateWidgetSizeAndPos()
{
    GameKeyManager *gameKeyManager = mMainWindow->getGameKeyManager();
    if (gameKeyManager) {
        gameKeyManager->updateGameKeySizeAndPos();
    }

    JoystickManager* joystickManager = mMainWindow->getJoystickManager();
    if (joystickManager) {
        joystickManager->updateGameKeySizeAndPos();
    }

    if (mRecordScreenWidget) {
        mRecordScreenWidget->updateRecordPanelPos();
    }
}

void DisplayManager::getMainWidgetInfo(int &pos_x, int &pos_y, int &margin_width, int &titlebar_height)
{
    if (mMainWindow->isFullScreen()) {
        QRect screenSize = getScreenSize();
        int displayWidth, displayHeight;
        QSize size = getMainWidgetDisplaySize();
        pos_x = (screenSize.width() - size.width()) / 2;
        pos_y = (screenSize.height() - size.height()) / 2;
        margin_width = 0;
        titlebar_height = 0;
    }
    else {
        QRect mainWidgetRect = mMainWindow->geometry();
        pos_x = mainWidgetRect.x();
        pos_y = mainWidgetRect.y();
        margin_width = MOUSE_MARGINS;
        titlebar_height = DEFAULT_TITLEBAR_HEIGHT;
    }
}

QSize DisplayManager::getMainWidgetDisplaySize()
{
    return QSize(mDisplayWidth, mDisplayHeight);
}

QSize DisplayManager::getMainWidgetInitialSize()
{
    if (isCurrentLandscapeOrientation()) {
        return QSize(getInitialHeight(), getInitialWidth());
    }
    else {
        return QSize(getInitialWidth(), getInitialHeight());
    }
}

void DisplayManager::getMainWidgetSize(int &displayWidth, int &displayHeight, int &initialWidth, int &initialHeight)
{
    QSize size = getMainWidgetDisplaySize();
    displayWidth = size.width();
    displayHeight = size.height();
    size = getMainWidgetInitialSize();
    initialWidth = size.width();
    initialHeight = size.height();
}

void DisplayManager::showWarningMessage()
{
    auto [isWarned, message] = AppSettings::getInstance().isAppWarned(mPkgName);
    if (isWarned) {
        mWarning = new WarningNotice(message, mMainWindow);
        mWarning->show();// using 'Non Modality' and nonblock, can't be set to modality on V10-SP1-2403, why ?
    }
}

QPoint DisplayManager::getWindowInitPos()
{
    int order = KmreWindowManager::getInstance()->getRunningAppNum();
    order = (order < 0) ? 0 : order;

    QRect rect = getAvailableScreenSize();
    QPoint pos((rect.width() - mDisplayWidth) / 2, (rect.height() - mDisplayHeight) / 2);
    if ((pos.x() > 0) && (pos.y() > 0)) {
        pos.setX(pos.x() + (order * 20));
        pos.setY(pos.y() + (order * 20));
    }
    else {
        pos.setX(100);
        pos.setY(60);
    }

    return pos;
}

DisplayWidget* DisplayManager::getFocusedDisplay()
{
    if (mCurrentFocusedDisplayId == mMainDisplayId) {
        return mMainDisplay;
    }
    else if (mCurrentFocusedDisplayId == mAccessoryDisplayId) {
        return mAccessoryDisplay;
    }
    return nullptr;
}

void DisplayManager::hideGameKeysWhileMoving()
{
    GameKeyManager *gameKeyManager = mMainWindow->getGameKeyManager();
    if (gameKeyManager) {
        gameKeyManager->hideGameKeysWhileMove();
    }

    JoystickManager* joystickManager = mMainWindow->getJoystickManager();
    if (joystickManager) {
        joystickManager->hideGameKeysWhileMove();
    }
}

void DisplayManager::showGameKeysAfterMoving()
{
    GameKeyManager *gameKeyManager = mMainWindow->getGameKeyManager();
    if (gameKeyManager) {
        gameKeyManager->showGameKeysAfterMove();
    }

    JoystickManager* joystickManager = mMainWindow->getJoystickManager();
    if (joystickManager) {
        joystickManager->showGameKeysAfterMove();
    }
}

void DisplayManager::hideGameKeysWhileRotation()
{
    GameKeyManager *gameKeyManager = mMainWindow->getGameKeyManager();
    if (gameKeyManager) {
        gameKeyManager->hideGameKeysWhileRotation();
    }

    JoystickManager* joystickManager = mMainWindow->getJoystickManager();
    if (joystickManager) {
        joystickManager->hideGameKeysWhileRotation();
    }
}

void DisplayManager::showGameKeysAfterShownFromTray()
{
    GameKeyManager *gameKeyManager = mMainWindow->getGameKeyManager();
    if (gameKeyManager) {
        gameKeyManager->showGameKeysAfterShownFromTray();
    }
}

void DisplayManager::hideGameKeysWhileHidenInTray()
{
    GameKeyManager *gameKeyManager = mMainWindow->getGameKeyManager();
    if (gameKeyManager) {
        gameKeyManager->hideGameKeysWhileHidenInTray();
    }
}
