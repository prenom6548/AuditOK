/* greeterwrapper.h
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#ifndef GREETERWRAPPER_H
#define GREETERWRAPPER_H

#include <QLightDM/Greeter>
#include <QProcess>

enum{
    rootWinPicture,
    rootWinColor
};
class GreeterWrapper : public QLightDM::Greeter
{
    Q_OBJECT
private:
    explicit GreeterWrapper(QObject *parent = nullptr);

public:
    static GreeterWrapper *instance(QObject *parent = nullptr);
    void setLang(const QString&);
    QString lang();
    void setSession(const QString&);
    QString session();
    void startSession();
    QString userName();
    void setUserName(const QString &);
    void setrootWindow();
    bool getdrawBackgroundIsStarted();
    //type 0代表绘制背景图片，1代表绘制背景颜色
    void setrootWindowBackground(int type,unsigned int color,QString filename);
    void setRootWindowBgOptions(int nOption);
    QString getEnsureShareDir(QString username);
    void showKsplashAnimation();
    bool userLoggedIn();
    void curUserName(QString user){ m_curUserName = user;}
    QString getCurUserName(){ return m_curUserName;}

private:
    QString m_language;
    QString m_session;
    QString m_userName;
    QString m_rootWindowBackground;
    int m_nPicOptions = 0;
    int drawBackgroundType;
    unsigned int drawBackgroundColor;
    bool drawBackgroundIsStarted = false;
    void closeNM();
    QProcess *screenMonitorProcess = nullptr;
    static GreeterWrapper* m_instance;
    QString m_curUserName;
signals:
    void aboutToLogin();
    void startSessionFailed();
    void authenticationSucess(QString userName);
};

#endif // GREETERWRAPPER_H
