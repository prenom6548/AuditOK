/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef THEMEBUTTON_H
#define THEMEBUTTON_H

#include <QObject>
#include <QWidget>
#include <QPushButton>

#include "themeiconlabel.h"

class ThemeButton : public QPushButton
{
    Q_OBJECT
public:
    ThemeButton(const QString &name, QWidget *parent = nullptr);
    ThemeButton(const QString &name, const QIcon &icon, QSize size, QWidget *parent = nullptr);
    ThemeButton(const QString &name, QPixmap pixmap, QWidget *parent = nullptr);
    ThemeButton(QString name, QStringList iStringList, QWidget *parent = nullptr);
    ThemeButton(QString name, QList<QPixmap> iconList, QWidget *parent = nullptr);

    void initUi();
    void setConnect();
    void setRadius(int radius);
    void setRect(QSize size);
    void setPixmap(QPixmap pixmap);
    void setBtnClicked(bool status);
    void setIconCenter();
    QString themeName() const { return themename; }
    // Tab 只落在当前选中项；方向键可程序化移焦到同组其它项
    void refreshGroupKeyboardNav();
    // 分区标题 + 选项名写入 accessibleName（ThemeButton 组 setBuddy 常无 LABELLED_BY）
    void setAccessibleOptionName(const QString &sectionTitle);

private:
    void btnCheckedSignalsSlot(bool status);
    void enterWidgetSlot();
    void leaveWidgetSlot();
    // 在同组 ThemeButton 间按左右键只移焦点，不触发选中
    void moveFocusInGroup(int key);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    QString themename;
    QString themeicon;
    ThemeIconLabel * mIconLabel;
    QLabel *mNameLabel;
    int mRadius = 8;
    int mWidth = 146;
    int mHeigh = 126;

Q_SIGNALS:
    void btnCheckedSignals(bool status);
};

#endif // THEMEBUTTON_H
