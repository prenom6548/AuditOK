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

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "common.h"
#include "utils.h"
#include "gamekey/gamekeystackwidget.h"
#include "gamekey/keyboardgamekeymanager.h"

#include <memory>
#include <tuple>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>

//竖屏：显示区域窗口的默认宽度和高度
#define DEFAULT_VERTICAL_WIDTH  450
#define DEFAULT_VERTICAL_HEIGHT 800
const int defaultVerticalHeight= 800;

//横屏：显示区域窗口的默认宽度和高度
#define DEFAULT_HORIZONTAL_WIDTH  1280
#define DEFAULT_HORIZONTAL_HEIGHT 720

using namespace kmre;

class KmreWindow;
class LoadingWidget;
class AutohideWidget;
class TitleBar;
class TitleBarJoystick;
class TitleMenu;
class WarningNotice;
class PopupTip;
class RecordScreenWidget;
class DisplayWidget;
class WindowStretchWorker;

class DisplayManager : public QObject
{
    Q_OBJECT

public:
    explicit DisplayManager(KmreWindow *window, int width, int height, QString pkgName);
    ~DisplayManager();

    typedef enum{
        ROTATION_NONE = 0,          // 无旋转
        ROTATION_MAIN_DISPLAY,      // 主窗口旋转
        ROTATION_ACCESSORY_DISPLAY, // 副窗口旋转
    }RotationState;

    typedef enum{
        DISPLAY_MODE_SINGLE_MAIN,       // 普通模式（单主窗口）
        DISPLAY_MODE_SINGLE_ACCESSORY,  // 平行视界模式（单附属窗口,主窗口隐藏）
        DISPLAY_MODE_MULTI,             // 平行视界模式
    }DisplayMode;

public slots:
    void onSwitchFullscreen();
    void onExitFullscreen();
    void onDisplayRotationChanged(int displayId, int rotation);
    void onDisplayForceRedraw();
    void onMultiplierSwitch(QString pkgName, int displayId, bool enable);
    void onHidePopupSubWindows();

public:
    void initUI();
    void initialize();
    void enterFullscreen();
    void updateWindowMaxSize(RotationState rs = ROTATION_NONE);
    bool isWindowMaxSized(){return mIsWindowMaxSized;}
    int  getImageRotation(){return mImageRotation;}
    DisplayMode getCurrentDisplayMode() {return mDisplayMode;}
    void resizeWindowWhileFullScreen(int rotation, bool isMainDisplay, bool rotationChanged = true);
    void resizeWindowWhileNormalScreen(int rotation, bool isMainDisplay, bool rotationChanged = true, bool adjustLastSize = true);
    void resetWindowSize(QSize size, bool updateDisplay = true, bool hideEdge = false);
    void setDefaultWindowSize(bool adjustLastSize = true);
    void showTip(QString msg, int showTime = 0);
    void saveWindowGeometry();
    void updateWidgetSizeAndPos();
    void updateMainDisplay(int displayId, int width, int height);
    
    bool isCurrentLandscapeOrientation() { 
        return utils::currentDisplayOrientation(mInitialOrientation, mImageRotation) == Qt::LandscapeOrientation;
    }
    bool isFullScreen();
    QSize getMinWindowSize();
    QSize getMaxScreenSize();
    int getOriginalInitialWidth(){return mInitialWidth;}
    int getOriginalInitialHeight(){return mInitialHeight;}
    int getInitialWidth() { return isCurrentLandscapeOrientation() ? mInitialWidth : DEFAULT_VERTICAL_WIDTH;}
    int getInitialHeight() { return isCurrentLandscapeOrientation() ? mInitialHeight : DEFAULT_VERTICAL_HEIGHT;}
    int getDisplayWidth(){return mDisplayWidth;}
    int getDisplayHeight(){return mDisplayHeight;}
    int getCurrentFocusedDisplayId() {return mCurrentFocusedDisplayId;}
    void setCurrentFocusedDisplayId(int displayId) {mCurrentFocusedDisplayId = displayId;}
    QString getCurrentFocusedDisplayPkgName() {
        return (mCurrentFocusedDisplayId == mAccessoryDisplayId) ? mAccessoryDisplayPkgName : mPkgName;
    }
    QSize getMainWidgetSize();
    QSize getMainWidgetDisplaySize();
    QSize getMainWidgetInitialSize();
    void getMainWidgetSize(int &displayWidth, int &displayHeight, int &initialWidth, int &initialHeight);
    void getMainWidgetInfo(int &pos_x, int &pos_y, int &margin_width, int &titlebar_height);
    int getMainDisplayId() {return mMainDisplayId;}
    DisplayWidget* getFocusedDisplay();
    DisplayWidget *getMainDisplayWidget() {return mMainDisplay;}
    DisplayWidget *getAccessoryDisplayWidget() {return mAccessoryDisplay;}
    int getAccessoryDisplayId() {return mAccessoryDisplayId;}
    QString getAccessoryDisplayPkgName() {return mAccessoryDisplayPkgName;}
    QRect getScreenSize();
    QRect getAvailableScreenSize();
    void getScreenInfo(QRect &allScreenSize, QRect &availableSize);
    AutohideWidget* getAutohideWidget() {return mAutohideWidget;}
    TitleBarJoystick* getTitleBarJoystick() {return mTitleBarJoystick;}
    RecordScreenWidget* getRecordScreenWidget(bool create = false);
    TitleMenu* getTitleMenu() {return mTitleMenu;}
    TitleBar* getTitleBar() {return mTitleBar;}
    GameKeyStackWidget* getGameKeyStackWidget(){return mGameKeyStackWidget;}
    bool isWindowStretchReady();
    int getInitialOrientation() {return mInitialOrientation;}
    int getCurrentDisplayOrientation() {
        return utils::currentDisplayOrientation(mInitialOrientation, mImageRotation);
    }
    void enableInputMethod(int id, bool enable, int x, int y);
    void disableInputMethod();
    void focusWindow();
    void unFocusWindow();
    bool focusDisplayByDisplayId(int displayId, bool force = false);
    bool isWindowFocused(){return !mWindowUnfocused;}
    void requestRationWorker();
    bool isDDSSupported(){ return mIsDynamicDisplaySizeSupported;}
    bool isMultiDisplayEnabled() {return mIsMultiDisplayEnabled;}
    void appResumed();
    void resizeRecordScreenWidget();
    void hide();
    void show();
    void setJoystickEditMode(bool isGoEdit);
    void hideGameKeysWhileMoving();
    void showGameKeysAfterMoving();
    void hideGameKeysWhileRotation();
    void showGameKeysAfterShownFromTray();
    void hideGameKeysWhileHidenInTray();

private:
    KmreWindow *mMainWindow = nullptr;

    DisplayMode mDisplayMode, mLastDisplayMode;
    int mCurrentFocusedDisplayId;
    bool mWindowUnfocused;
    int mMainDisplayId;
    int mAccessoryDisplayId;
    QHBoxLayout *mHLayout = nullptr;
    QHBoxLayout *mTitleLayout = nullptr;
    QVBoxLayout *mMainLayout = nullptr;
    DisplayWidget* mMainDisplay = nullptr;// 主屏
    DisplayWidget* mAccessoryDisplay = nullptr;// 副屏
    LoadingWidget* mLoadingDisplay = nullptr;// 载入窗口
    AutohideWidget* mAutohideWidget = nullptr;// 全屏时的标题栏
    TitleBar* mTitleBar = nullptr;// 非全屏时的标题栏
    TitleBarJoystick* mTitleBarJoystick = nullptr;  //设置手柄键鼠时的标题栏
    TitleMenu* mTitleMenu = nullptr;// 标题栏菜单
    WarningNotice* mWarning = nullptr;
    GameKeyStackWidget* mGameKeyStackWidget = nullptr; //外设设置菜单
    RecordScreenWidget* mRecordScreenWidget = nullptr;// 录屏窗口
    std::shared_ptr<PopupTip> mPopupTip;//
    std::shared_ptr<WindowStretchWorker> mStretchWorker;// 控制窗口拉伸

    struct AppSizeConfig {
        bool fullScreen;
        int width;
        int height;
    }mAppSizeConfig;

    const int mInitialWidth, mInitialHeight;
    int mDisplayWidth, mDisplayHeight;
    int mInitialOrientation;//窗口方向 landscape portrait
    int mImageRotation;//图像的旋转角度0(0)    1(90)     2(180)   3(270)
    int mCurrentRotation;
    bool mIsWindowMaxSized;
    QSize mVirtualDisplaySize, mPhysicalDisplaySize;

    QPoint mPoint;
    QSize mSize;
    QString mPkgName;
    QString mAccessoryDisplayPkgName;
    bool mIsMultiDisplayEnabled;
    bool mIsDynamicDisplaySizeSupported;
    
private:
    void updateAppSizeConfig();
    void setDisplayMode(DisplayMode mode, bool switchMultipler = false);
    void setMainWidgetSize();
    void setDisplaySize();
    void getDisplaySizeForNormalScreen(int _width, int _height, int &calculatedWidth, int &calculatedHeight, bool lastSize);
    DisplayWidget* createDisplay(int displayId, int width, int height, int orientation, QString pkgName, QString displayName);
    bool createMainDisplay();
    bool createAccessoryDisplay(int displayId);
    void switchMultipler(bool enable);
    void autoAdjustMultipler();
    void rotationChanged(int rotation, bool isMainDisplay);
    void switchLandScapeOrientation(bool isMainDisplay);
    void switchPortraitOrientation(bool isMainDisplay);
    void enableDisplay(int displayId, bool enable);
    void showDisplays();
    void setMainWidgetSize(bool isGoEdit);
    QPoint getWindowInitPos();
    QRect getMaxWindowGeometry(RotationState rs);
    void showWarningMessage();
};
#endif // DISPLAY_MANAGER_H
