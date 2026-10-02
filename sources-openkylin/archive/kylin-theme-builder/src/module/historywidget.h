/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include "../module/infocreatewidget.h"
#include "../fileProcess/historyinfoload.h"
#include "../fileProcess/historyinfoload.h"
#include "../fileProcess/diroperation.h"

#include <QObject>
#include <QWidget>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QToolButton>

class HistoryButton : public QPushButton
{
    Q_OBJECT
public:
    HistoryButton(QWidget* parent = nullptr);
    void setHistoryTime(const QString &text);
    void setCover(const QString &covePath);
    void setThemeCNName(const QString &themeCNName);
    void setThemeName(const QString &themeName);
    void setThemeBuilder(const QString &themeBuilder);
    void setThemeMail(const QString &themeMail);
    void setThemeType(const QString &themeType);
    void initUI();
protected:
    bool eventFilter(QObject* obj, QEvent* event);

signals:
    void updateHistoryRequested();
    void startBuild(QString &historytime);
    void hoverStateChanged(bool isHovered);
private:
    QString m_historytime = nullptr;
    QString m_showhistorytime = nullptr;
    QString m_coverpath = nullptr;
    QString m_themecnname = nullptr;
    QString m_themename  = nullptr;
    QString m_themebuilder = nullptr;
    QString m_thememail = nullptr;
    QString m_themetype  = nullptr;
    QPushButton* m_typebutton;
    QToolButton* m_menubutton;
    QMenu *m_menu;

};


class HistoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryWidget(QWidget *parent = nullptr);
    void updateHistoryDir();

public slots:
    void handleButtonClick();

signals:
    void startSecondEdit(const HistoryInfo &InfoData);
    void updateInfo();

};

#endif // HISTORYWIDGET_H
