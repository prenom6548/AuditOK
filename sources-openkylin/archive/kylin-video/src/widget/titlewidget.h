/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <ukuiwindowhelper.h>

#define TITLE_BAR_FIXED_HEIGHT 40

class KMenu;
class QLabel;
class QPushButton;

class TitleWidget : public QWidget
{
    Q_OBJECT
    
    friend class unitConstruct;  // 可能存在这样的声明
public:
    explicit TitleWidget(QWidget *parent = nullptr);
    ~TitleWidget();

    int getMenuBtnX();
    void setTitle(QString title);
    void updateMaxButtonStatus(bool is_maxed);
    void setButtonState(bool is_available);

    void setMiniMode(bool mini){m_isMiniMode = mini;}
    void setHide();
    void setShow();
    void setHomePage(bool _isHomePage);

    void setUIMode(bool isTabletMode);

signals:
    void sigMiniMode();
    void sigMiniSize();
    void sigMaxSize();
    void sigShowMenu();
    void sigClose();
    void sigCanHide(bool);
    void sigDBClicked();

private:
    void initLayout();
    void initConnect();
    void resetFont();

private:
    QString m_fullTitle;
    QPushButton *btnIcon;
    QLabel  *labTitle;

    QWidget *widget;
    KMenu   *menu;

    QPushButton *m_btnMenu,
                *m_btnMiniMode,
                *m_btnMinSize,
                *m_btnMaxSize,
                *m_btnClose;

    bool m_isMiniMode = false,
         m_isHomePage = true,
         m_leaveState = false;
    QColor inactiveColor;
    bool m_isTabletMode = false;
    QTimer* m_maxBtnHoverDelayTimer;

    UkuiWindowHelper *m_windowHelper = nullptr;
protected:
    bool event(QEvent *e) override;
#ifndef WHITE_TEST
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
#endif
    bool eventFilter(QObject *obj, QEvent *event) override;
public slots:
    void onWindowActivated();
    void onWindowDeactivated();
};

#endif // TITLEWIDGET_H
