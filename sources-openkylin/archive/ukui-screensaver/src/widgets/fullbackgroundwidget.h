/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef FULLBACKGROUNDWIDGET_H
#define FULLBACKGROUNDWIDGET_H
#include "../common/windowdata.h"
#include "shutdownanimation.h"
#include <QWidget>
#include <QAbstractNativeEventFilter>
#include <QHash>
#include <QMap>
#include <QPixmap>
#include <QSize>
#include <QElapsedTimer>
#include <QTimer>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QPair>
#include <QMutex>
#include <QQueue>
#include <QVariantAnimation>
#include <ukuiwindowhelper.h>

enum ScreenStatus
{
    UNDEFINED = 0x00,
    SCREEN_SAVER = 0x01,
    SCREEN_LOCK = 0x02,
    SCREEN_LOCK_AND_SAVER = 0x03,
    SCREEN_BLACK = 0x04,
};

enum BackgroundType
{
    BACKGROUND_IMAGE,
    BACKGROUND_COLOR
};

struct Background
{
    BackgroundType type;
    QColor color;
    QString image;
};

struct Transition
{
    QSharedPointer<Background> from;
    QSharedPointer<Background> to;
    float stage;
    bool started;
};

enum EventCode {
    EC_ShowScreenSaver,
    EC_ClearScreenSaver
};

// 事件信息结构体
struct EventInfo {
    quint32 eventCode;
    QString strParam; // 事件参数
    qint64 timeStamp;

    EventInfo(quint32 code, QString param, qint64 time)
            : eventCode(code), strParam(param), timeStamp(time) {}
};

class LockDialogModel;
class LockWidget;
class AgreementWindow;
class LockScreenTransitionController;

typedef QPair<QString, QString> bgPath_Size;

class FullBackgroundWidget : public QWidget, public QAbstractNativeEventFilter
{
    Q_OBJECT
    enum class VisualMode {
        LockStable,
        SessionToolsStable,
        EnterFromScreensaverStart
    };
public:
    explicit FullBackgroundWidget(LockDialogModel *model, bool isStartup = false, QWidget *parent = nullptr);
    ~FullBackgroundWidget();
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
    void init();

public:
    int RegisteSubWnd(quint64 uWndId);
    int UnRegisteSubWnd(quint64 uWndId);
    QList<quint64> GetSubWndIds();
    inline bool IsStartupMode()
    {
        return m_isStartupMode;
    }

public Q_SLOTS:
    void onShowBlankScreensaver(int nDelay = 0, bool isHasLock = true);
    void onShowLock(bool isStartup);
    void onShowSessionIdle();
    void onShowLockScreensaver();
    void onShowScreensaver();
    void onCloseScreensaver();
    void onDesktopResized();
    void onPrepareForSleep(bool sleep);
    void laterActivate();
    void setLockState();
    void clearLockState();
    void onClearScreensaver();
    void onShowSessionTools();
    void onShowShutdownAnimation(int power);
    void onShowAppBlockWindow(int actionType);
    void onShowMultiUsersBlockWindows(int actionType);
    void onShowSwitchUserLock();
    void onShowSwitchToUser(QString strUserName);

    void onCurUserChanged(const QString &strUserName);
    void onUpdateUserBackground(const QString &strUserName);
    void onTransition();
    void onAuthSucceed(QString strUserName);

    void onRemoveUserBackground(const QString &);
    void onAddUserBackground(const QString &);
    void onUserBackgroundChanged(const QString &);
    void onCurrentUserBackground(const QString &);
    void onSecondRunParam(const QString &);

    void onStartFcitx();
    void enqueueEvent(quint32 eventCode, QString param = "");

    void onAnimationTimeout();

    /** 认证区动效期间冻结壁纸绘制（壁纸不变时由 LockWidget 退场触发） */
    void beginAuthUiWallpaperSnapshot();
    void endAuthUiWallpaperSnapshot();
    bool isWallpaperAnimActive() const { return m_wallpaperAnimActive; }
    bool isAuthUiWallpaperSnapshotActive() const { return m_authUiSnapshotActive; }
    void scheduleWallpaperRepaint();

    void startUnlockTransition();

Q_SIGNALS:
    /** 壁纸 blur/scale 过渡结束（无动画时也会立即发出） */
    void wallpaperTransitionFinished();
    void StartupModeChanged(bool isStartup);

protected:
    void mousePressEvent(QMouseEvent *e);
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event);
    void keyReleaseEvent(QKeyEvent *e);

private:
    void initUI();
    void initConnections();
    bool eventFilter(QObject *obj, QEvent *event);
    void fakeFocusIn(WId window);
    QString getWindowNameFromWid(WId window);
    QString getFocusWindowName();
    void tryGrabKeyboard();
    void moveToPrimaryScreen();
    void moveToCurrentScreen();
    void setCurrentBackground(QString cmd);
    void applyVisualMode(VisualMode mode);
    QString getDesktopBackground();

    void initCurrentBackground();
    QString getDefaultBackgroundPath();
    void loadingAllUserBackground();
    QString getUserBackgroundPath(const QString &);
    void addBackgroundData(const QString &);

    bool isOpenGradation();
    void startTransition(QSharedPointer<Background> &from, QSharedPointer<Background> &to);
    void stopTransition();
    void drawBackground(QPainter &painter, QSharedPointer<Background> &backgroundBack,
                        QSharedPointer<Background> &backgroundFront, const QRect &rect, float alpha = 1.0f);
    void drawWallpaperLayers(QPainter &painter, const QPixmap &sharpPixmap, const QPixmap &blurPixmap,
                             const QRect &rect, qreal blurProgress, qreal scaleProgress, float layerAlpha);
    void invalidateStableWallpaperCache();
    bool drawStableCompositedWallpaper(QPainter &painter, const QPixmap &sharpPixmap, const QPixmap &blurPixmap,
                                       const QRect &rect, qreal blurProgress, qreal scaleProgress, float layerAlpha);
    QPixmap *getBackground(const QString &path, const QRect &rect);
    QPixmap *getSharpBackground(const QString &path, const QRect &rect);
    void setBackground(QSharedPointer<Background> &background);
    void initBlurAnimation();
    void startBlurTransition();
    void setWallpaperAnimActive(bool active);
    bool shouldPaintLockSubtreeInBackgroundPass() const;
    void onWallpaperAnimationFinished();
    void invalidateWallpaperAnimCaches();
    /** 解锁动效视觉终态：blur/scale 归零并刷新（不关窗、不写 root 壁纸） */
    void applyUnlockWallpaperVisualFinishedState();
    /** 动效墙钟结束后执行原认证成功后的关窗与收尾（同步/异步方式与原先一致） */
    void completeUnlockAfterAnimation(const QString &strUserName);

    void delayLockScreen();
    void onLockScreenTimeout();
    void stopDelayLockScreen();

    void setIsStartupMode(bool isStartup);
    void sendStartedFinished();
    /** 50ms 防抖：合并屏幕变化后再调用 onDesktopResized */
    void scheduleDesktopResized();

private Q_SLOTS:
    void onGlobalKeyPress(const quint8 &key);
    void onGlobalKeyRelease(const quint8 &key);
    void onGlobalButtonDrag(int xPos, int yPos);
    void onGlobalButtonPressed(int xPos, int yPos);

    void RRScreenChangeEvent(bool isFirst);
    void onScreensChanged(QList<QString> listMonitors);
    /**
     * @brief processEventQueue 处理事件队列
     */
    void processEventQueue();
    void persistAuthSuccessLoginInfo(const QString &strUserName);

private:
    LockDialogModel *m_modelLockDialog = nullptr;
    bool m_isStartupMode = false;
    // 上层窗口管理
    QList<quint64> m_listWndIds;
    // 锁屏
    LockWidget *m_lockWidget = nullptr;
    AgreementWindow *m_agreementWindow = nullptr;

    int m_tryGrabTimes = 0;
    bool m_bIsLockState = false;
    QList<QWidget *> widgetXScreensaverList;

    QTimer *m_backgrondGradationTimer = nullptr;
    QSharedPointer<Background> m_background;
    Transition m_transition;

    ScreenStatus screenStatus = UNDEFINED;
    bool m_isBlank = false;
    /** 从黑色空白屏保（带锁）回到锁屏时置位，用于播放进场动效 */
    bool m_lockEnterFromBlankPending = false;

    QTimer *m_timerLock = nullptr;
    int m_RREventBase;
    int m_RRErrorBase;
    QList<QString> m_listMonitors;

    QRect m_defaultGeometry;
    UkuiWindowHelper *m_windowHelper = nullptr;
    QTimer *m_desktopResizeDebounceTimer = nullptr;

    // 按键事件队列相关
    QQueue<EventInfo> m_keyEventQueue;
    QMutex m_queueMutex;
    QTimer *m_queueProcessTimer = nullptr;

    QMap<QString, QPixmap *> m_allBackgroundsMap;
    QMap<bgPath_Size, QPixmap *> m_allBackgroundsDataMap;
    QMap<bgPath_Size, QPixmap *> m_allBackgroundsSharpDataMap;
    QString m_currentUserName;
    QString m_oldUserName;
    QVariantAnimation *m_blurAnimation = nullptr;
    QVariantAnimation *m_scaleAnimation = nullptr;
    qreal m_blurProgress = 1.0;
    qreal m_scaleProgress = 1.0;
    bool m_wallpaperAnimActive = false;
    bool m_wallpaperRepaintScheduled = false;
    QElapsedTimer m_wallpaperRepaintClock;
    bool m_authUiSnapshotActive = false;
    QHash<QString, QPixmap> m_wallpaperSnapshots;
    QPixmap m_layerCompositeCache;
    QSize m_layerCompositeCacheSize;
    QHash<QString, QPixmap> m_stableWallpaperCache;
    bool m_isUnlockAnimating = false;
    bool m_isEnteringFromScreensaver = false;
    LockScreenTransitionController *m_unlockTransition = nullptr;

    QFuture<void> m_loadingOneBackgroundFuture;
    bool m_isInited = false;

    ShutdownAnimation *m_animation = nullptr;
    QTimer *m_animationTimer = nullptr;
    bool m_closeWindow = false;
    QString m_command = "";
    bool m_stopAnimationTimer = false;
};

#endif // FULLBACKGROUNDWIDGET_H
