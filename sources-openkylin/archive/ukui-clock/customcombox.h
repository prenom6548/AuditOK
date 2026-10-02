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
#ifndef CUSTOMCOMBOX_H
#define CUSTOMCOMBOX_H

#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <customlistwidget.h>
#include <QTimer>

namespace Ui {
class CustomComBox;
}

class item_button;

class CustomComBox : public QComboBox
{
    Q_OBJECT
public:
    explicit CustomComBox(int rowNum, QWidget *parent = nullptr);
    ~CustomComBox();
    friend class TestClass;  // 核心修改：声明测试类为友元

    //下拉框
    CustomListWidget* list_widget_ {nullptr};
    //文本框，搜索框
    QLineEdit* line_edit_ {nullptr};
    item_button *btn[20];
    QListWidgetItem *aItem[20];
    int rowNum_all = 0;
    //下拉框显示标志
    bool show_flag_;

    bool comboxClicked = false;


    //添加一条选项
    void set_aItem(int rowNum);

    int getCurrentHoverNum() const;
    void setCurrentHoverNum(int value);

    void showPopup() override;
    void hidePopup() override;

public slots:
    void setTextHoverStyle(int num);

protected:

    //事件过滤器
    virtual bool eventFilter(QObject *watched,QEvent *event) override;

signals:
    void comboxClose();
    void comboxPopup();
    void ishoverd();
    void left();

private:
    void settingsStyle();                                                               //监听主题
    void blackStyle();                                                                  //黑色主题
    void whiteStyle();                                                                  //白色主题
    void updateLabelFront(QLabel *label, int size);
    void updateSelectItemLabelFont(int size);
    int hoverNum = -1;
    QTimer m_timer;
    bool m_ignoreMouseRelease = false;
private slots:
    void onTimerTimeout();
};


class item_button : public QPushButton
{
    Q_OBJECT

public:
    explicit item_button(QWidget *parent = nullptr);
    ~item_button();
    QLabel *iconLabel;
    QLabel *textLabel;
};

#endif // CUSTOMCOMBOX_H
