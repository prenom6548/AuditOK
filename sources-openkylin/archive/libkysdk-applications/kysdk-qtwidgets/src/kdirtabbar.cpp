/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yaxian Liu <liuyaxian@kylinos.cn>
 *
 */

#include "kdirtabbar.h"
#include "themeController.h"
#include <QBoxLayout>
#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QFontMetrics>
#include <QList>
#include <QMenu>
#include <QPainter>
#include <QSpacerItem>
#include <QString>
#include <QStringList>
#include <QStyle>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QToolButton>

namespace kdk
{

class CustomButton : public QToolButton
{
    Q_OBJECT
public:
    CustomButton(QWidget *parent = nullptr);
    QIcon setIconColor(QIcon icon, QColor color)
    {
        QPixmap pixmap = icon.pixmap(QSize(16, 16));
        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(pixmap.rect(), color);
        QIcon colorIcon = QIcon(pixmap);
        return colorIcon;
    }

protected:
    void paintEvent(QPaintEvent *event);
};

class KDirTabBarPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KDirTabBar)

public:
    KDirTabBarPrivate(KDirTabBar *parent);

    void clearButtons();
    void addButton(const QString &uri, QString buttonName, bool setMenu = true);

    void doLayout();

    void changeTheme();

private:
    KDirTabBar *q_ptr;
    int m_visibleDepth;
    int m_textLength;
    QString m_currentPath;
    QString m_rootPath;
    QHBoxLayout *buttonLayout;
    QHBoxLayout *m_layout;
    //    QSpacerItem *spacer;

    QList<QToolButton *> m_buttonList;
    QList<QFileInfo *> m_button_info;
};

KDirTabBar::KDirTabBar(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KDirTabBarPrivate(this))
{
    Q_D(KDirTabBar);
    d->m_visibleDepth = 5;
    d->m_textLength = 5;
    //    d->spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    d->buttonLayout = new QHBoxLayout;
    d->buttonLayout->setContentsMargins(0, 0, 0, 0);
    d->buttonLayout->setSpacing(0);
    d->buttonLayout->setMargin(0);
    d->m_layout = new QHBoxLayout(this);
    d->m_layout->setContentsMargins(0, 0, 0, 0);
    d->m_layout->setSpacing(0);
    d->m_layout->setMargin(0);
    d->m_layout->addLayout(d->buttonLayout);
    //    d->m_layout->addItem(d->spacer);
    d->m_layout->addStretch();
    setLayout(d->m_layout);
    //    setFocusPolicy(Qt::ClickFocus);
    setObjectName("KDirTabBar");

    connect(d->m_gsetting,&QGSettings::changed,this,[=](QString key){
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName")
        {
            d->changeTheme();
        }
    });
}

KDirTabBar::~KDirTabBar()
{
}
void KDirTabBar::setCurrentPath(const QString Dirpath)
{
    Q_D(KDirTabBar);
    QList<QFileInfo *> lastInfo;
    for (auto info : d->m_button_info) {
        QFileInfo *fileinfo = new QFileInfo();
        fileinfo = info;
        lastInfo.append(fileinfo);
    }

    d->m_currentPath = Dirpath;
    d->m_button_info.clear();
    auto tmpUri = Dirpath;
    QFileInfo *file = new QFileInfo(tmpUri);
    while (tmpUri != "/" && tmpUri != "") {
        d->m_button_info.prepend(file);
        tmpUri = file->absolutePath();
        file = new QFileInfo(tmpUri);
    }
    if (lastInfo.isEmpty()) {
        d->clearButtons();
        for (auto info : d->m_button_info) {
            d->addButton(info->absoluteFilePath(), info->fileName(), true);
        }
    } else {
        while (d->m_button_info.size() < d->m_buttonList.size()) // 新的比旧的列表短时，去掉多余的Qlist项
        {
            d->m_buttonList.last()->deleteLater();
            d->m_buttonList.removeLast();
        }
        for (int i = d->m_buttonList.size() - 1; i >= 0; i--) {
            if (lastInfo[i]->absoluteFilePath() != d->m_button_info[i]->absoluteFilePath()) // 对比新旧列表中的每一项，如果不相同则remove按钮列表中的元素
            {
                d->m_buttonList[i]->hide();
                d->m_buttonList[i]->deleteLater();
                d->m_buttonList.removeAt(i);
            }
        }
        for (int i = d->m_buttonList.size(); i < d->m_button_info.size(); i++) {
            d->addButton(d->m_button_info[i]->absoluteFilePath(), d->m_button_info[i]->fileName(), true);
        }
    }

    for (auto info : lastInfo) {

        delete info;
    }
    lastInfo.clear();

    d->doLayout(); // 布局，最大显示个数
}

QString KDirTabBar::currentPath() const
{
    Q_D(const KDirTabBar);
    return d->m_currentPath;
}

void KDirTabBar::setVisibleDepth(const int depth)
{
    Q_D(KDirTabBar);
    d->m_visibleDepth = depth;
    // dolayout
}

int KDirTabBar::visibleDepth()
{
    Q_D(KDirTabBar);
    return d->m_visibleDepth;
}

void KDirTabBar::setIcon(int index, const QIcon &icon)
{
    Q_D(KDirTabBar);
    if (index < d->m_buttonList.size()) {
        d->m_buttonList[index - 1]->setIcon(icon);
        d->doLayout();
    }
}

QIcon KDirTabBar::icon(int index)
{
    Q_D(KDirTabBar);
    return d->m_buttonList[index - 1]->icon();
}

void KDirTabBar::setTextLength(int length)
{
    Q_D(KDirTabBar);
    d->m_textLength = length;
}

int KDirTabBar::textLength()
{
    Q_D(KDirTabBar);
    return d->m_textLength;
}

QList<QToolButton *> KDirTabBar::buttons()
{
    Q_D(KDirTabBar);
    return d->m_buttonList;
}

// void KDirTabBar::mousePressEvent(QMouseEvent *event)
//{
//     QWidget::mousePressEvent(event);

//}

void KDirTabBarPrivate::addButton(const QString &uri, QString buttonName, bool setMenu)
{
    Q_Q(KDirTabBar);
    CustomButton *button = new CustomButton(q);
    QString displayName = buttonName;
    button->setFocusPolicy(Qt::FocusPolicy(button->focusPolicy() & ~Qt::TabFocus));
    button->setAutoRaise(true);
    button->setProperty("uri", uri);
    button->setIconSize(QSize(16, 16));
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setPopupMode(CustomButton::MenuButtonPopup);

    m_buttonList.append(static_cast<CustomButton *>(button));

    buttonLayout->addWidget(button);
    button->setIcon(QIcon::fromTheme("folder"));
    QFontMetrics ftm(q->font());

    if (displayName.length() > m_textLength) {
        int charWidth = ftm.averageCharWidth();
        displayName = ftm.elidedText(displayName, Qt::ElideRight, m_textLength * charWidth);
    }

    button->setText(displayName);

    if (buttonName != displayName) {
        button->setToolTip(buttonName);
    }

    connect(button, &CustomButton::clicked, [=]() {
        q->setCurrentPath(uri);
        emit q->tabBarClicked(uri);
    });

    if (setMenu) {
        QList<QFileInfo> uris;
        QDirIterator it(uri, QDir::Dirs | QDir::NoDotAndDotDot);
        while (it.hasNext()) {
            it.next();
            QFileInfo fileInfo = it.fileInfo();
            if (fileInfo.isDir()) {
                uris << fileInfo;
            }
        }

        QMenu *menu = new QMenu;
        connect(button, &CustomButton::destroyed, menu, &QMenu::deleteLater);
        const int WIDTH_EXTEND = 5;

        connect(menu, &QMenu::aboutToShow, this, [=]() {
            menu->setMinimumWidth(button->width() + WIDTH_EXTEND);
        });
        connect(menu, &QMenu::aboutToHide, this, [=]() {
        });

        if (uris.isEmpty())
            button->setPopupMode(CustomButton::InstantPopup);
        else {
            QList<QAction *> actions;
            for (auto uri : uris) {
                QIcon icon;

                if (uri.baseName() == "图片")
                    icon = QIcon::fromTheme("folder-pictures");
                else if (uri.baseName() == "下载")
                    icon = QIcon::fromTheme("folder-download");
                else if (uri.baseName() == "公共的")
                    icon = QIcon::fromTheme("folder-publicshare");
                else if (uri.baseName() == "文档")
                    icon = QIcon::fromTheme("folder-documents");
                else if (uri.baseName() == "桌面")
                    icon = QIcon::fromTheme("user-desktop");
                else if (uri.baseName() == "模板")
                    icon = QIcon::fromTheme("folder-templates");
                else if (uri.baseName() == "视频")
                    icon = QIcon::fromTheme("folder-videos");
                else if (uri.baseName() == "音乐")
                    icon = QIcon::fromTheme("folder-music");
                else
                    icon = QIcon::fromTheme("folder");

                QAction *action = new QAction(icon, uri.fileName(), this);
                actions << action;
                connect(action, &QAction::triggered, [=]() {
                    q->setCurrentPath(uri.absoluteFilePath());
                    emit q->tabBarClicked(uri.absoluteFilePath());
                });
            }
            menu->addActions(actions);
            if (menu)
                button->setMenu(menu);
        }
    }
}

void KDirTabBarPrivate::doLayout()
{
    Q_Q(KDirTabBar);
    int counter = 0;
    if (m_buttonList.size() > m_visibleDepth) {
        counter = m_buttonList.size() - m_visibleDepth;
    }
    for (int i = 0; i < m_buttonList.size(); i++) {
        m_buttonList[i]->setVisible(false);
    }
    for (counter; counter < m_buttonList.size(); counter++) {
        m_buttonList[counter]->setVisible(true);
        m_buttonList[counter]->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        m_buttonList[counter]->adjustSize();
        //        buttonLayout->addWidget(m_buttonList[counter]);
    }
}

void KDirTabBarPrivate::changeTheme()
{
    Q_Q(KDirTabBar);
    initThemeStyle();
}

void KDirTabBarPrivate::clearButtons()
{
    Q_Q(KDirTabBar);
    for (auto button : m_buttonList) {
        button->hide();
        button->deleteLater();
    }
    m_buttonList.clear();
}

KDirTabBarPrivate::KDirTabBarPrivate(KDirTabBar *parent)
    : q_ptr(parent)
{
    setParent(parent);
}

CustomButton::CustomButton(QWidget *parent)
{
}

void CustomButton::paintEvent(QPaintEvent *event)
{
    QStyleOptionToolButton option;
    initStyleOption(&option);
    // QToolButton::paintEvent(event);

    int ra = ThemeController::getRadiusFromDT("kradius-normal");
    if (ra == -1)
        ra = 6;
    QColor textColor = ThemeController::getPaletteColorFromDT("text-active");
    QColor backgroundColor;
    QPen pen;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (ThemeController::themeMode() == LightTheme) { // 浅色模式
        if (!option.state.testFlag(QStyle::State_Enabled)) {
            textColor = ThemeController::getPaletteColorFromDT("text-disable");
        } else {
            // 三态
            if (option.state.testFlag(QStyle::State_Sunken)) {
                backgroundColor = ThemeController::getCustomColorFromDT("kgray-10");
                // 绘制背景颜色
                painter.setBrush(backgroundColor);
                pen = QPen(backgroundColor);
                painter.setPen(pen);
                painter.drawRoundedRect(rect(), ra, ra);
            } else if (option.state.testFlag(QStyle::State_MouseOver)) {
                backgroundColor = ThemeController::getCustomColorFromDT("kgray-7");
                // 绘制背景颜色
                painter.setBrush(backgroundColor);
                pen = QPen(backgroundColor);
                painter.setPen(pen);
                painter.drawRoundedRect(rect(), ra, ra);
            }
        }
    } else {
        if (!option.state.testFlag(QStyle::State_Enabled)) {
            textColor = ThemeController::getPaletteColorFromDT("text-disable");
        } else {
            // 三态
            if (option.state.testFlag(QStyle::State_Sunken)) {
                backgroundColor = ThemeController::getCustomColorFromDT("kgray-13");
                // 绘制背景颜色
                painter.setBrush(backgroundColor);
                pen = QPen(backgroundColor);
                painter.setPen(pen);
                painter.drawRoundedRect(rect(), ra, ra);
            } else if (option.state.testFlag(QStyle::State_MouseOver)) {
                backgroundColor = QColor("#545454");
                // 绘制背景颜色
                painter.setBrush(backgroundColor);
                pen = QPen(backgroundColor);
                painter.setPen(pen);
                painter.drawRoundedRect(rect(), ra, ra);
            }
        }
    }

    painter.end();

    // 绘制图标、文本、箭头
    QPainter p(this);
    QRect textRect;
    uint tf = Qt::AlignVCenter;

    textRect = QRect(32, 0, option.rect.width() - 64, option.rect.height());

    if (menu() && menu()->isVisible()) {
        p.drawPixmap(this->width() - 24, (this->height() - 16) / 2, 16, 16,
                     ThemeController::drawColoredPixmap(QIcon::fromTheme("ukui-down-symbolic").pixmap(16, 16), textColor));
    } else if (menu() && !menu()->isVisible()) {
        p.drawPixmap(this->width() - 24, (this->height() - 16) / 2, 16, 16,
                     setIconColor(QIcon::fromTheme("ukui-end-symbolic"), textColor).pixmap(16.16));
    } else
        textRect = QRect(32, 0, option.rect.width() - 32, option.rect.height());

    if (!icon().isNull())
        p.drawPixmap(8, (option.rect.height() - 16) / 2, 16, 16, icon().pixmap(16, 16));
    else
        textRect = QRect(8, 0, option.rect.width() - 32, option.rect.height());

    pen = QPen(textColor);
    p.setPen(pen);
    p.drawText(textRect, tf, this->text());

    p.end();
}

}

#include "kdirtabbar.moc"
#include "moc_kdirtabbar.cpp"
