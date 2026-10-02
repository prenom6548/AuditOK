/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef LOCALINFO_H
#define LOCALINFO_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QDesktopServices>
#include <QEvent>

#include "view/common/globalsizedata.h"
#include "global/utils/inisetting.h"
#include "global/utils/addrset.h"
#include "global/utils/globalutils.h"
#include "localupdatename.h"
#include "ukuiwindowhelper.h"
// sdk 搜索框
#include "ksearchlineedit.h"
using namespace kdk;

class LocalInfo : public QWidget
{
    Q_OBJECT
    friend class TestLocalInfo;
public:
    explicit LocalInfo(QWidget *parent = nullptr);
    ~LocalInfo();

    // 单例，初始化返回指针
    static LocalInfo *getInstance(QWidget *parent = nullptr);

    // 初始化组件
    void setWidgetUi();

    // 设置组件样式
    void setWidgetStyle();

    void showList();

    // 跟随主题深浅模式
    void changeTheme();

    int w;
    int h;

    QLabel *m_userAvatarLab; // 用户头像
    QLabel *m_searchIconLab; // 搜索图标
    QLabel *m_nicknameLab;   // 用户昵称
    QLabel *m_userIpAddrLab; // 用户IP

    QPushButton *m_updateNameBtn;   // 修改本机昵称按钮
    QPushButton *m_openFilePathBtn; // 打开文件保存目录按钮

    // QLineEdit *m_searchLineEdit; // 搜索输入框
    
    // sdk 搜索输入框
    KSearchLineEdit *m_searchLineEdit;

    // 显示用户昵称和IP信息
    void displayInfo(QString nickname, QString ip);

    // 获取配置文件信息
    void getIniText();

    // 初始化界面后检查本机昵称
    void checkLocalName();

    void paintEvent(QPaintEvent *event);

    bool event(QEvent *event);

    bool eventFilter(QObject *watch, QEvent *e);

    // 获取本机头像
    QString getLocalAvatar();

    void changeFontSize();

private:
    QVBoxLayout *m_localInfoLayout; // 本机信息总体布局
    QHBoxLayout *m_nameLayout;      // 本机昵称布局
    QVBoxLayout *m_ipLayout;        // 本机IP布局
    QHBoxLayout *m_avatarLayout;    // 本机头像布局
    QHBoxLayout *m_searchLayout;    // 搜索图像布局文本框

    LocalUpdateName *m_updateNameWid;

    UkuiWindowHelper *m_windowHelper;

    QColor m_backColor;

    IniSettings *m_settings;

    // 修改备注
    void slotUpdateName();

    // 打开文件保存目录
    void slotOpenFilePath();
    void slotChangeFont();  //控件的字体进行更换；
    void setBackgroundColor(); // 高亮色的切换

// xc数据
public:
    void slotChangeFlatHSize(); // 改变横平板模式下的尺寸
    void slotChangeFlatVSize(); // 改变竖平板模式下的尺寸
    void slotChangePCSize();   // 改变PC模式下的尺寸
    
private:
    int m_nameWidth = 150;

signals:
    void changeLocalName(bool isOnline);
};




#endif // LOCALINFO_H
