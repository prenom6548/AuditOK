/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Zero Liu    liuzenghui1@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ktabbar.h"
#include "themeController.h"
#include <QDebug>
namespace kdk
{
class KTabBarPrivate:public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KTabBar)

public:
    KTabBarPrivate(KTabBar*parent)
        :q_ptr(parent)
    {}
protected:
    void changeTheme();
private:
    KTabBar*q_ptr;
    KTabBarStyle m_kTabBarStyle;
    int m_borderRadius;
};

KTabBar::KTabBar(KTabBarStyle barStyle,QWidget* parent):
    QTabBar(parent),
    d_ptr(new KTabBarPrivate(this))
{
    Q_D(KTabBar);
    d->m_borderRadius = 6;
    d->m_kTabBarStyle = barStyle;
    d->changeTheme();
}

KTabBar::~KTabBar()
{

}

void KTabBar::readyKTabBar()
{
    Q_D(KTabBar);
    d->changeTheme();
    connect(d->m_gsetting,&QGSettings::changed,d,&KTabBarPrivate::changeTheme);
}

void KTabBar::setTabBarStyle(KTabBarStyle barStyle)
{
    Q_D(KTabBar);
    d->m_kTabBarStyle = barStyle;
}
KTabBarStyle KTabBar::barStyle()
{
    Q_D(KTabBar);
    return d->m_kTabBarStyle;
}

void KTabBar::setBorderRadius(int radius)
{
    Q_D(KTabBar);
    if(radius < 0 || radius > 20)
        return;
    d->m_borderRadius = radius;
    switch (d->m_kTabBarStyle)
    {
    case SegmentDark:
        this->setStyleSheet(this->styleSheet()+
                            QString("QTabBar::tab:first{border-top-left-radius: %1px;border-bottom-left-radius: %2px}\
                            QTabBar::tab:last{border-top-right-radius: %3px;border-bottom-right-radius: %4px}")
                                    .arg(d->m_borderRadius).arg(d->m_borderRadius).arg(d->m_borderRadius).arg(d->m_borderRadius));
        break;
    case SegmentLight:
        this->setStyleSheet(this->styleSheet()+
                            QString("QTabBar::tab{border-radius:%1px;}").arg(d->m_borderRadius));
        break;
    }
}

int KTabBar::borderRadius()
{
    Q_D(KTabBar);
    if(d->m_themeFlag == Sliding)
        return 0;
    else
        return d->m_borderRadius;
}

QSize KTabBar::sizeHint() const
{
    QSize size(36,36);
    return size;
}

QSize KTabBar::minimumTabSizeHint(int index) const
{
    Q_UNUSED(index)
    QSize size(100,36);
    return size;
}

void KTabBarPrivate::changeTheme()
{
    Q_Q(KTabBar);
    initThemeStyle();
    if (m_themeFlag == LightTheme)
    {
        switch (m_kTabBarStyle)
        {
        case SegmentDark:
            q->setStyleSheet("QTabBar{background-color:transparent;}\
                    QTabBar::tab{height:36px;color:#595959;background-color:#E6E6E6;border: 1px #E6E6E6 solid;margin:1px;}\
                    QTabBar::tab:first{border-top-left-radius: 6px;border-bottom-left-radius: 6px}\
                    QTabBar::tab:last{border-top-right-radius: 6px;border-bottom-right-radius: 6px}\
                    QTabBar::tab:hover{background:#DADADA;border:1px #3790FA solid;}\
                    QTabBar::tab:pressed{background:#B8B8B8;border:1px transparent solid;margin:1px;}\
                    QTabBar::tab:selected:!pressed{color:#FFFFFF;background:#3790FA;border:1px #3790FA solid;}");
            break;
        case Sliding:
            q->setStyleSheet("QTabBar{background-color:transparent;}\
                    QTabBar::tab{color:#595959;background-color:transparent;border:2px #E6E6E6 solid;border-bottom: 2px solid #E6E6E6;}\
                    QTabBar::tab:hover{color:#262626;background:transparent;border-bottom: 2px solid #DADADA;}\
                    QTabBar::tab:pressed{color:#595959;background:transparent;border-bottom: 2px solid #B8B8B8;}\
                    QTabBar::tab:selected:!pressed{color:#3790FA;background:transparent;border-bottom: 2px solid #3790FA;}");
            break;

        case SegmentLight:
        default:
            q->setStyleSheet("QTabBar{background-color:transparent;}\
                    QTabBar::tab{height:36px;color:#595959;background-color:transparent;border: 1px transparent solid;border-radius:6px;margin:1px;}\
                    QTabBar::tab:hover{background:rgba(0,0,0,0.05);border:1px transparent solid;margin:1px;}\
                    QTabBar::tab:pressed{background:rgba(0,0,0,0.15);border:1px transparent solid;margin:1px;}\
                    QTabBar::tab:selected:!pressed{color:#FFFFFF;background:#3790FA;border:1px transparent solid;margin:1px;}");
            break;
        }
    }
    else
    {
        switch (m_kTabBarStyle)
        {
        case SegmentDark:
            q->setStyleSheet("QTabBar{background-color:transparent;}\
                    QTabBar::tab{height:36px;color:#FFFFFF;background-color:#37373B;border: 1px #E6E6E6 solid;margin:1px;}\
                    QTabBar::tab:first{border-top-left-radius: 6px;border-bottom-left-radius: 6px}\
                    QTabBar::tab:last{border-top-right-radius: 6px;border-bottom-right-radius: 6px}\
                    QTabBar::tab:hover{background:#5F5F5F;border:1px #3790FA solid;}\
                    QTabBar::tab:pressed{background:#414141;border:1px transparent solid;margin:1px;}\
                    QTabBar::tab:selected:!pressed{color:#FFFFFF;background:#3790FA;border:1px #3790FA solid;}");
            break;
        case Sliding:
            q->setStyleSheet("QTabBar{background-color:transparent;}\
                    QTabBar::tab{color:#D9D9D9;background-color:transparent;border:2px #37373B solid;border-bottom: 2px solid #37373B;}\
                    QTabBar::tab:hover{color:#FFFFFF;background:transparent;border-bottom: 2px solid #5F5F5F;}\
                    QTabBar::tab:pressed{color:#D9D9D9;background:transparent;border-bottom: 2px solid #414141;}\
                    QTabBar::tab:selected:!pressed{color:#3790FA;background:transparent;border-bottom: 2px solid #3790FA;}");
            break;

        case SegmentLight:
        default:
            q->setStyleSheet("QTabBar{background-color:transparent;}\
                    QTabBar::tab{height:36px;color:#D9D9D9;background-color:transparent;border: 1px transparent solid;border-radius:6px;margin:1px;}\
                    QTabBar::tab:hover{background:rgba(255,255,255,0.15);;border:1px transparent solid;margin:1px;}\
                    QTabBar::tab:pressed{background:rgba(255,255,255,0.1);border:1px transparent solid;margin:1px;}\
                    QTabBar::tab:selected:!pressed{color:#FFFFFF;background:#3790FA;border:1px transparent solid;margin:1px;}");
            break;
        }
    }
}

}
#include "ktabbar.moc"
#include "moc_ktabbar.cpp"
