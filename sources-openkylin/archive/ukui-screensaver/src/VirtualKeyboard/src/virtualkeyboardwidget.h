/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#ifndef VIRTUALKEYBOARDWIDGET_H
#define VIRTUALKEYBOARDWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QList>
#include <QMap>
#include <QAbstractNativeEventFilter>
#include "fakekeyboard.h"
#include "plasma-shell-manager.h"

class DragWidget;
class KBTitle;
class LettersWidget;
class NumbersWidget;
class CharsWidget;
class CharsMoreWidget;
class VirtualKeyboardWidget : public QWidget, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    enum {
        VKB_PAGE_LETTERS,
        VKB_PAGE_NUMBERS,
        VKB_PAGE_CHARS,
        VKB_PAGE_CHARSMORE,
    };
    VirtualKeyboardWidget(QWidget *parent = nullptr);
    virtual ~VirtualKeyboardWidget();
    bool getFloatStatus();
    void adjustGeometry();
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;

public Q_SLOTS:
    void onNormalBtnClicked(QChar c);
    void onSpecialBtnClicked(QString keyName);

Q_SIGNALS:
    void aboutToClose();
    void aboutToFloat();
    void keyPressed(QChar c);
    void keyPressed(FuncKey::FUNCKEY key);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initUI();
    void initConnections();
    void onMouseEvents(int type, const QPoint &pos);
    void clearModifier();

private:
    double m_lfWidthScale;
    double m_lfHeightScale;
    bool m_isVertical;
    QStackedWidget *m_stackedWidget = nullptr;
    LettersWidget *m_lettersWidget = nullptr;
    NumbersWidget *m_numbersWidget = nullptr;
    CharsWidget *m_charsWidget = nullptr;
    CharsMoreWidget *m_charsMoreWidget = nullptr;
    KBTitle *m_kbTitle = nullptr;
    DragWidget *m_dragWidget = nullptr;
    int m_nCurPage;
    QList<int> m_listPageHis;
    bool m_isdragState = false;//是否为悬浮状态
    bool isMove;// 是否可移动
    QPoint lastPoint;// 拖拽控件时 记录当前控件的位置
    FakeKeyboard *vKeyboard;
};

#endif // VIRTUALKEYBOARDWIDGET_H
