/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#ifndef PROMPTWIDGET_H
#define PROMPTWIDGET_H
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QFrame>
#include <QObject>
#include <QDialog>
#include <QPainter>
#include <QStyleOption>
#include "controller/Interaction.h"
#include "global/viewvar.h"
#include <QMovie>
class PromptWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PromptWidget(QWidget *parent = nullptr);
    void initLayout(QString netError, int m_judgeFullScreen);
    int m_judgeFullScreen = 0;
    void isFullScreen(int m_isFullScreen);

private:
    QHBoxLayout *m_networkErrorLayout = nullptr;
    QPushButton *m_networkWarnIcon = nullptr;
    QString m_iconPath;
    QPixmap m_networkErrorIcon;
    QLabel *m_networkErrorCode = nullptr;
    void paintEvent(QPaintEvent *event);
    void addItems();
    void initConnect();
    QString netErrorWords = " "; // 网络异常提示信息字段
    void lable2SetFontSizeSlot(int size);
    QGSettings *m_fontSize = nullptr;
    QFont m_font;

signals:
};

#endif // PROMPTWIDGET_H
