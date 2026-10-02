/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
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

#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QCheckBox>
#include <QFrame>
#include <QVBoxLayout>
#include <QSettings>

#include "gamelistview.h"

class AddGameWidget;
class RemoveGameWidget;
class QLabel;
class QPushButton;
class QStackedWidget;
class QButtonGroup;

class GameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GameWidget(QWidget *parent = nullptr);
    ~GameWidget();

signals:
    void requestClearGame(const QString &pkgName);
public slots:
    void onClickeditem(const QString &pkgname,const QString &appname);
protected:
//    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void loadAtscValue();

private:
//    QStackedWidget *m_stackedWidget = nullptr;
    QWidget *m_titleWidget = nullptr;
//    QButtonGroup *m_navButtonsGroup = nullptr;
    QFrame *m_contentFrame = nullptr;
    AddGameWidget *m_addGameWidget = nullptr;
    GameListView *m_view = nullptr;
    RemoveGameWidget *m_removeGameWidget = nullptr;
    QCheckBox *m_astcCheckBox;
    QPushButton *m_refreshBtn = nullptr;
    QSettings *m_qsettings = nullptr;
    QString m_confName;
    QString m_pkgName = nullptr;
    QString m_appName = nullptr;
};

#endif // GAMEWIDGET_H
