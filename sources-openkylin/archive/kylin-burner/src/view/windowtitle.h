/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef WINDOWTITLE_H
#define WINDOWTITLE_H

#include <QWidget>
#include <QMenu>
#include <QRect>

namespace Ui {
class WindowTitle;
}

class WindowTitle : public QWidget
{
    Q_OBJECT
Q_SIGNALS:
    void close();
    void max();
    void min();
public:
    explicit WindowTitle(QWidget *parent = nullptr);
    ~WindowTitle();
public:
    void onlyShowButton();
    void onlyShowLogo();
    void onlyShowInDialog();
    void onlyVisibleClose(bool);
    void setIconAndTitle(QIcon, QString);

public slots:
    void onWindowIconChange(QIcon);
    void onWindowTitleChange(QString);
    void onWindowShowMaxed(bool);
    void setIsTabletMode(bool);
    void slotsSetCloseEnabel(bool);
    void slotChangeUIFontSize(int fontSize);
    void configTitleLabel();

protected:
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
private:
    bool             m_bShowTitle;
    bool             m_bAppRunning;
    QMenu           *m_oMenu;
    QAction         *m_oActionSelf;
    QAction         *m_oActionMax;
    QAction         *m_oActionMin;
    QAction         *m_oActionClose;
private:
    Ui::WindowTitle *ui;
};

#endif // WINDOWTITLE_H
