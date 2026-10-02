/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#ifndef AUTOHIDEWIDGET_H
#define AUTOHIDEWIDGET_H

#include <QWidget>

class QTimer;
class QPropertyAnimation;
//class SystemButton;
class QPushButton;

#include <QWidget>

class AutohideWidget : public QWidget
{
	Q_OBJECT

public:
	enum Activation { Anywhere = 1, Bottom = 2 };

	AutohideWidget(QString pkgName, QWidget * parent = 0);
	~AutohideWidget();

    void setWindowWidth(int w) {m_width = w;}
    void enableGameAction(bool b);

    void initButtons();
    void initConnections();
    void updateUi();

public slots:
	void show();
	void activate();
	void deactivate();
	void setAutoHide(bool b);
    void setAnimated(bool b) { m_useAnimation = b; };
	void setMargin(int margin) { spacing = margin; };
    void setPercWidth(int s) { m_percWidth = s;}
    void setActivationArea(Activation m) { m_activationArea = m; }
	void setHideDelay(int ms);

signals:
    void sigBack();
    void requestSettings(const QPoint &pos, bool fullscreen);
    void requestHideSettings();
    void sigExitFullscreen();
    void sigMiniSize();
    void sigClose();
    void sigForceRedraw();

public:
    bool isActive() { return m_turnedOn; };
    bool autoHide() { return m_autoHide; };
    bool isAnimated() { return m_useAnimation; };
	int margin() { return spacing; };
    int percWidth() { return m_percWidth; };
    Activation activationArea() { return m_activationArea; }
	int hideDelay();

protected:
	bool eventFilter(QObject * obj, QEvent * event);

private slots:
	void checkUnderMouse();
	void showAnimated();

private:
	void installFilter(QObject *o);
	void resizeAndMove();

private:
    QString mPkgName;
    bool m_turnedOn;
    bool m_autoHide;
    bool m_useAnimation;
	int spacing;
    int m_percWidth;
    Activation m_activationArea;
    QTimer *m_timer = nullptr;
#if QT_VERSION >= 0x040600
    QPropertyAnimation *m_animation = nullptr;
#endif
    QPushButton *m_backBtn = nullptr;
    QPushButton *m_gameKeyBtn = nullptr;
    QPushButton *m_unFullscreenBtn = nullptr;
//    QPushButton *m_minBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;
    int m_width;
};

#endif // AUTOHIDEWIDGET_H
