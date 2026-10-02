/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef UKUI_MENU_MENU_MAIN_WINDOW_H
#define UKUI_MENU_MENU_MAIN_WINDOW_H

#include <QObject>
#include <QRect>
#include <QScreen>
#include <QQuickView>
#include <QQuickWindow>
#include <QGSettings>
#include <window-helper.h>

namespace UkuiMenu {

class WindowModule final
{
public:
    static void defineModule(const char *uri, int versionMajor, int versionMinor);
};

class PanelGSettings : public QObject
{
    Q_OBJECT
public:
    static PanelGSettings *instance();

    int getPanelLength(QString screenName);

    ~PanelGSettings();

private:
    explicit PanelGSettings(QObject *parent = nullptr);

    bool isKeysContain(const char* key);

    QMap<QString, QVariant> getPanelLengthMap();
};

class MenuWindow : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(int panelPos READ panelPos NOTIFY panelPosChanged)
    Q_PROPERTY(double transparency READ transparency NOTIFY transparencyChanged)
    Q_PROPERTY(QRect normalRect READ normalRect NOTIFY normalRectChanged)
    Q_PROPERTY(bool effectEnabled READ effectEnabled NOTIFY effectEnabledChanged)
    Q_PROPERTY(bool windowVisible READ windowVisible NOTIFY windowVisibleChanged)
    Q_PROPERTY(bool editMode READ editMode WRITE setEditMode NOTIFY editModeChanged)
    Q_PROPERTY(bool isFullScreen READ isFullScreen WRITE setFullScreen NOTIFY fullScreenChanged)

public:
    explicit MenuWindow(QWindow *parent = nullptr);
    MenuWindow(QQmlEngine* engine, QWindow *parent);

    int panelPos() const;
    double transparency() const;
    QRect normalRect() const;
    bool effectEnabled() const;
    bool windowVisible() const;

    bool editMode() const;
    void setEditMode(bool mode);

    bool isFullScreen() const;
    virtual void setFullScreen(bool isFullScreen) {};
    void initMenuState();

    Q_INVOKABLE virtual void activeMenuWindow(bool active) {};

Q_SIGNALS:
    void panelPosChanged();
    void transparencyChanged();
    void normalRectChanged();
    void effectEnabledChanged();
    void windowVisibleChanged();
    void editModeChanged();
    void fullScreenChanged();

protected:
    virtual void init();
    virtual void initWindowProxy();
    virtual void updateGeometry() {};

    bool event(QEvent *event) override;

    void initPanelSetting();
    void updateCurrentScreenGeometry();

    int m_panelPos{4};
    int m_panelSize{48};
    int m_panelType{0};
    int m_dataIslandPosition{0};
    int m_topbarSize{0};
    QGSettings *m_setting {nullptr};

    bool m_editMode {false};
    bool m_isFullScreen{false};
    bool m_windowVisible {false};
    QRect m_normalGeometry = QRect(0,0,0,0);
    QRect m_normalMaskGeometry = QRect(0,0,0,0);
    QRect m_fullScreenGeometry = QRect(0,0,0,0);
    UkuiQuick::WindowProxy *m_windowProxy {nullptr};
    QPointer<QScreen> m_screen {nullptr};

private:
};

class MenuWindowWayland : public MenuWindow
{
    Q_OBJECT

public:
    explicit MenuWindowWayland(QWindow *parent = nullptr);
    MenuWindowWayland(QQmlEngine* engine, QWindow *parent);

    void setFullScreen(bool isFullScreen) override;

    Q_INVOKABLE void activeMenuWindow(bool active) override;

protected:
    void initWindowProxy() override;
    void updateGeometry() override;
    bool event(QEvent *event) override;

private:
    void setMenuState(bool isFullScreen);
};

class MenuWindowXcb : public MenuWindow
{
    Q_OBJECT

public:
    explicit MenuWindowXcb(QWindow *parent = nullptr);
    MenuWindowXcb(QQmlEngine* engine, QWindow *parent);

    void setFullScreen(bool isFullScreen) override;

    Q_INVOKABLE void changeWindowBlurRegion(qreal x, qreal y, qreal w, qreal h, qreal radius = 8);
    Q_INVOKABLE void activeMenuWindow(bool active) override;
    Q_INVOKABLE void finalizeExitFullScreen();

Q_SIGNALS:
    void beforeFullScreenChanged();
    void beforeFullScreenExited();

protected:
    void initWindowProxy() override;
    void updateGeometry() override;
    bool event(QEvent *event) override;

private:
    void updateGeometryOfMask();

private:
    QRect m_maskGeometry = QRect(0,0,0,0);
};

} // UkuiMenu

#endif //UKUI_MENU_MENU_MAIN_WINDOW_H
