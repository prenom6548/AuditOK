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
#ifndef TINYCOUNTDOWN_H
#define TINYCOUNTDOWN_H

#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include "constant_class.h"
#include <xcb/xcb.h>
#include <QTime>
#include <QTimer>
#include <QStyleOption>
#include <QGSettings>
#include <QPainterPath>
#include "gsettingsubject.h"
#include "xatom-helper.h"
#include "commontooltip.h"
namespace Ui {
class tinyCountdown;
}

class tinyCountdown : public QWidget
{
    Q_OBJECT
public:
    explicit tinyCountdown(QWidget *parent = nullptr);
    ~tinyCountdown();
    friend class TestClass;  // 核心修改：声明测试类为友元

    void updateTimeInfo(QString str);
    void updateTimeLabelElided();  // 倒计时迷你窗口时间标签缩略显示

 /*
    void focusOutEvent ( QFocusEvent * event ) override;
    void focusInEvent ( QFocusEvent * event ) override;
*/

    void closeEvent(QCloseEvent *event) override;
    void paintEvent(QPaintEvent * event) override;
    void updateWidgetRadius();
    void showThisWindow();
    void clearData();
protected:
     //绘制背景
     // Draw background


signals:
    void mainWindowClick();
    void finishClick();
    void suspendClick(int m_onRun);

public slots:
        void set_dialog_close();
        void showTop();
        void updateOnRunState(int onRun);
private slots:
    void showMainWindow();


private:
    Ui::tinyCountdown *ui;
    bool mousePressed;
    QPoint dragPosition;                                            //拖动坐标
    void closeStyle();
    void settingsStyle();
    void switchStyle();
    void suspendRunBtnStyle();
    void finishBtnStyle();
    void suspendRunBtnIcon();
    bool m_onRun = false;
};

#endif // TINYCOUNTDOWN_H
