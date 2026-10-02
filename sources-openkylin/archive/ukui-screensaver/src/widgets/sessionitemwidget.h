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
#ifndef SESSIONITEMWIDGET_H
#define SESSIONITEMWIDGET_H

#include <QObject>
#include <QWidget>

class userLabel;
class QLabel;
class QPixmap;

class SessionItemWidget : public QWidget
{
    Q_OBJECT

public:
    SessionItemWidget(QWidget *parent = 0);
    ~SessionItemWidget();
    /**
     * @brief 设置会话名
     * @param name   会话名
     * @return
     */
    void setSessionName(QString name);
    /**
     * @brief 设置会话类型
     * @param nType 会话类型
     * @return
     */
    void setSessionType(int nType);

    inline QString sessionName()
    {
        return m_strSessionName;
    }

    inline QString sessionImgPath()
    {
        return m_strImgPath;
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    virtual void paintEvent(QPaintEvent *event);

private:
    void init();
    void initIconLabel();

Q_SIGNALS:
    void enterWidget();
    void leaveWidget();

private:
    QLabel *m_labelImg = nullptr;
    QLabel *m_labelName = nullptr;
    QString m_strSessionName;
    int m_nSessionType;
    QString m_strImgPath;
};

#endif // SESSIONITEMWIDGET_H
