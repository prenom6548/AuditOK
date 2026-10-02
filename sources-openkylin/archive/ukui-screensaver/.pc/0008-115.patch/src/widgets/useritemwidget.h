/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef USERITEMWIDGET_H
#define USERITEMWIDGET_H

#include <QObject>
#include <QWidget>

class userLabel;
class QLabel;
class QPixmap;
class KLabel;

class UserItemWidget : public QWidget
{
    Q_OBJECT

public:
    UserItemWidget(QWidget *parent = 0);
    ~UserItemWidget();
    /**
     * @brief 设置用户名
     * @param name   用户名
     * @return
     */
    void setUserName(QString name);
    /**
     * @brief 设置用户头像
     * @param userIcon   用户头像
     * @return
     */
    void setUserPixmap(QPixmap userIcon);
    /**
     * @brief 设置用户登录状态
     * @param status   登录状态
     * @return
     */
    void setUserStatus(bool status);
    /**
     * @brief 获取用户名
     * @return QString 用户名
     */
    QString getUserName();

    void setUserNickName(QString strNickName);
    inline QString userNickName()
    {
        return m_strNickName;
    }

    void setFontFamily(QString fontFamily);

    void setFontSize(double fontSize);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    virtual void paintEvent(QPaintEvent *event);

private:
    void init();
    void initIconLabel();

Q_SIGNALS:
    void enterWidget();
    void leaveWidget();
    /**
     * @brief 响应点击事件
     * @param userName   用户名
     * @return
     */
    void clicked(QString m_strUserName);

private:
    QLabel *m_labelHeadImg = nullptr;
    KLabel *m_labelNickName = nullptr;
    QLabel *m_labelLoggedIn = nullptr;
    QString m_strUserName;
    QString m_strNickName;
};

#endif // USERITEMWIDGET_H
