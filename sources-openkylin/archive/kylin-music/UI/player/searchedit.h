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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SEARCHEDIT_H
#define SEARCHEDIT_H

#include <QLineEdit>
#include <QFocusEvent>
#include <QDebug>
#include "UI/player/searchresult.h"
#include <QMenu>

#include "ksearchlineedit.h"

using namespace  kdk;

class SearchResult;

class SearchEdit : public KSearchLineEdit
{
    Q_OBJECT

public:
    SearchEdit(QWidget *parent = nullptr);

    void setWidget(QWidget *mainWidget);
    void moveSearchResult();
    SearchResult *m_result = nullptr;

protected:
    void keyPressEvent(QKeyEvent *event);
#ifndef WHITE_TEST
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
#endif
Q_SIGNALS:
    void sigFoucusIn();

    //发送展示搜索结果界面
    void signalReturnPressed(QString searchEdit);

    //搜索的关键字
    void signalReturnText(QString text);

public Q_SLOTS:
    void slotTextChanged();
    void slotReturnPressed();


private:
    QWidget *m_mainWidget = nullptr;
};

#endif // SEARCHEDIT_H
