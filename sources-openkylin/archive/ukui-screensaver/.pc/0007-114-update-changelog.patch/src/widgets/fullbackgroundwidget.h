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

#include <QWidget>
#include <QAbstractNativeEventFilter>
#include <QMap>
#include <QTimer>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QPair>

enum ScreenStatus
{
    UNDEFINED = 0x00,
    SCREEN_SAVER = 0x01,
    SCREEN_LOCK = 0x02,
    SCREEN_LOCK_AND_SAVER = 0x03,
    SCREEN_BLACK = 0x04,
};

class LockDialogModel;
class LockWidget;
class AgreementWindow;

typedef QPair<QString, QString> bgPath_Size;

class FullBackgroundWidget : public QWidget, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit FullBackgroundWidget(LockDialogModel *model, QWidget *parent = nullptr);
    ~FullBackgroundWidget();
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;

    QMap<QString, QPixmap *> m_allBackgroundsMap;
    QMap<bgPath_Size, QPixmap *> m_allBackgroundsDataMap;
    //    QPixmap * m_defaultPixmap ;

    QString m_currentUserName;
    QString m_oldUserName;

    QFuture<void> m_loadingAllBackgroundFuture;
    QFuture<void> m_loadingOneBackgroundFuture;

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
    void onClearScreensaver();
    void onShowSessionTools();
    void onShowAppBlockWindow(int actionType);
    void onShowMultiUsersBlockWindows(int actionType);
    void onShowSwitchUserLock();

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
Q_SIGNALS:

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

    void initCurrentBackground();
    QString getDefaultBackgroundPath();
    void loadingAllUserBackground();
    QString getUserBackgroundPath(const QString &);
    void addBackgroundData(const QString &);

    bool isOpenGradation();
    void startTransition();
    void stopTransition();
    void drawBackground(QPixmap *backgroundBack, QPixmap *backgroundFront, const QRect &rect, float alpha = 1.0);
    QPixmap *getBackground(const QString &path, const QRect &rect);

    void delayLockScreen();
    void onLockScreenTimeout();
    void stopDelayLockScreen();

private Q_SLOTS:
    void onGlobalKeyPress(const quint8 &key);
    void onGlobalKeyRelease(const quint8 &key);
    void onGlobalButtonDrag(int xPos, int yPos);
    void onGlobalButtonPressed(int xPos, int yPos);

    void RRScreenChangeEvent(bool isFirst);
    void onScreensChanged(QList<QString> listMonitors);

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

    QTimer *m_backgrondGradationTimer;
    bool m_gradualChanging = false;
    float m_backgroundAlpha = 0.0;

    ScreenStatus screenStatus = UNDEFINED;
    bool m_isBlank = false;

    QTimer *m_timerLock = nullptr;
    int m_RREventBase;
    int m_RRErrorBase;
    QList<QString> m_listMonitors;

    bool m_isSessionTools = false;
    QRect m_defaultGeometry;
};

#endif // FULLBACKGROUNDWIDGET_H
