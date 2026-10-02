/*
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
#ifndef PROGRAMMODEL_H
#define PROGRAMMODEL_H

#define PROGRAMMER "programmer"

#include <QWidget>

#include "../calc_programmer/processformula.h"
#include "binarykeyboary.h"
#include "programkeyboary.h"
#include "toolbar.h"
#include "programdisplay.h"

class ProgramModel : public QWidget
{
    Q_OBJECT
    friend class TestClass;
public:
    explicit ProgramModel(QWidget *parent = nullptr);

    // 浅色模式样式
    void setLightUI();

    // 深色模式样式
    void setDarkUI();

    // 处理AC键
    void handleAC();

    // 二进制显示屏是否显示
    bool isBinWinShow();

    // 返回当前显示的表达式
    QString getFormulaNow();

    // 输入复制的字符串
    void setFormulaNow(QString formula);

	void handlePasteError(void);

private:
    ProgramDisplay *m_display; // 显示屏
    ToolBar *m_toolbar; // 工具栏
    BinaryKeyboary *m_binWin; // 二进制显示
    ProgramKeyboary *m_keyboary; // 键盘

    QVBoxLayout *m_layout; // 布局

    // 接受处理结果
    QList<QString> m_result = {"0", "0", "0", "0=0", "0", "0", "", "TRUE", "TRUE"};

    // 输入数字时是否需要将前面的表达式清空
    QString m_isClearResult = QString("TRUE");

    // 是否多次重复按等于号
    QString m_isRepeat = "FALSE";

    // 二进制显示屏是否显示
    QString m_binWinShowState = QString("FALSE");

    // 处理Clear键
    void handleClear();

    // 处理等号
    void handleEqual();

    // 处理数字按键
    void handleNum(QString value);

    // 处理操作符按键
    void handleOp(QString value);

    // 处理退格键
    void handleDel();

    // 处理切换进制
    void handleBase(int base);

    // 计算正确，处理二进制显示以及字符显示
    void setBinCodeData();

signals:
public slots:
    void slotKeyBtnClicked(QString value);
    void slotBoxValueChanged(int index);
    void slotToolBtnClicked(QString value);
    void keyPressEvent(QKeyEvent *event);
};

#endif // PROGRAMMODEL_H
