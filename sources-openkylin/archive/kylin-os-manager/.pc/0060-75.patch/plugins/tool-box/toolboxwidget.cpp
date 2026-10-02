/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include <QListWidget>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QProcess>
#include <QDebug>
#include <QLocale>

#include <gsettingmonitor.h>

#include "toolboxwidget.h"
#include "kom-buriedpoint.h"

namespace BoxSize
{
static constexpr int TitleWidth = 728;
static constexpr int TitleHeight = 44;
static constexpr int DescribeWidth = 728;
static constexpr int DescribeHeight = 30;
static constexpr int ItemWidth = 360;
static constexpr int ItemHeight = 100;
} // namespace BoxSize

ToolBoxWidget::ToolBoxWidget(QWidget *parent) : QWidget(parent)
{
    m_theme = Utils::getTheme();
    m_fontSize = Utils::getFontSize();

    initWidget();
    initGsettings();
}

ToolBoxWidget::~ToolBoxWidget() = default;

void ToolBoxWidget::initWidget(void)
{
    //    setAutoFillBackground(true);
    //    setBackgroundRole(QPalette::Base);

    m_title = new QLabel(this);
    //    m_title->setFixedSize(BoxSize::TitleWidth, BoxSize::TitleHeight);
    QFont font;
    font.setPixelSize(24 * m_fontSize / 10);
    font.setBold(true);
    m_title->setFont(font);
    m_title->setText(tr("My Tool"));

    m_describe = new QLabel(this);
    //    m_describe->setFixedSize(BoxSize::DescribeWidth, BoxSize::DescribeHeight);
    m_describe->setText(tr("All kinds of system tools to help you better use the computer"));

    m_appWidget = new QListWidget(this);
    m_appWidget->resize(800, 486);

    /* 点击 listwidget 中的一项时，点击色为透明，即移除点击高亮色 */
    // QPalette palette = m_appWidget->palette();
    // palette.setColor(QPalette::Highlight, QColor(0, 0, 0, 0));
    // m_appWidget->setPalette(palette);

    m_appWidget->setFocusPolicy(Qt::NoFocus);
    m_appWidget->setViewMode(QListView::IconMode);
    m_appWidget->setFrameShape(QListView::NoFrame);
    m_appWidget->setMovement(QListView::Static);
    m_appWidget->setFlow(QListView::LeftToRight);
    //    m_appWidget->setSelectionMode(QListView::NoSelection);
    m_appWidget->setWrapping(true);
    m_appWidget->setSpacing(16);
    //    changeAppWidgetStyle();

    initAppWidget();
    connect(m_appWidget, &QListWidget::itemClicked, this, &ToolBoxWidget::startApp);

    QVBoxLayout *topLayout = new QVBoxLayout;
    topLayout->setSpacing(0);
    topLayout->setContentsMargins(40, 0, 40, 0);
    topLayout->addWidget(m_title);
    topLayout->addSpacing(8);
    topLayout->addWidget(m_describe);
    topLayout->setAlignment(m_title, Qt::AlignLeft);
    topLayout->setAlignment(m_describe, Qt::AlignLeft);

    QVBoxLayout *appWidgetLayout = new QVBoxLayout;
    appWidgetLayout->setSpacing(0);
    appWidgetLayout->setContentsMargins(24, 0, 0, 0);
    appWidgetLayout->addWidget(m_appWidget);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    vLayout->setContentsMargins(0, 24, 0, 0);
    vLayout->addLayout(topLayout);
    vLayout->addSpacing(8);
    vLayout->addLayout(appWidgetLayout);

    setLayout(vLayout);
}

void ToolBoxWidget::initGsettings(void)
{
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [=]() {
        m_theme = Utils::getTheme();
        //        changeAppWidgetStyle();
    });

    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this, [=]() {
        m_fontSize = Utils::getFontSize();
        QFont font;
        font.setPixelSize(24 * m_fontSize / 10);
        font.setBold(true);
        m_title->setFont(font);
    });

    return;
}

void ToolBoxWidget::changeAppWidgetStyle(void)
{
    if (m_theme == Theme::Dark) {
        m_appWidget->setStyleSheet(
            "QListWidget::item{background:transparent;border-radius:6;}"
            "QListWidget::item:hover{background:rgba(255,255,255,0.2);border-radius:6;}"
            //                                   "QListWidget::item:pressed{background:rgba(255,255,255,0.05);border-radius:6;}"
            "QListWidget::item:selected{background:rgba(255,255,255,0.05);border-radius:6;}");
    } else {
        m_appWidget->setStyleSheet(
            "QListWidget::item{background:transparent;border-radius:6;}"
            "QListWidget::item:hover{background:rgba(0,0,0,0.05);border-radius:6;}"
            //                                   "QListWidget::item:pressed{background:rgba(0,0,0,0.2);border-radius:6;}"
            "QListWidget::item:selected{background:rgba(0,0,0,0.2);border-radius:6;}"
            /*"QListWidget::item:selected::!active{background:transparent;border-radius:6;}"*/);
    }

    return;
}

void ToolBoxWidget::initAppWidget(void)
{
    QMap<QString, boxconfig_t> boxConfig = Utils::getBoxConfig();
    QVector<QString> loadBox = Utils::getLoadBox(boxConfig);

    for (int i = 0; i < loadBox.size(); i++) {
        createItem(boxConfig.value(loadBox.at(i)));
    }

    /* 添加敬请期待项 */
    // createStayHopeItem();

    return;
}

void ToolBoxWidget::createItem(const boxconfig_t &config)
{
    if (QFileInfo(config.exec.split(" ").at(0)).exists()) {
        QListWidgetItem *item = new QListWidgetItem(m_appWidget);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setSizeHint(QSize(BoxSize::ItemWidth, BoxSize::ItemHeight));
        item->setWhatsThis(config.exec);
        m_appWidget->addItem(item);

        QString name, comment;
        getNameComment(config, name, comment);

        QIcon icon = QIcon::fromTheme(config.icon);
        if (icon.isNull()) {
            QFileInfo fileInfo(config.defaultIcon);
            if (fileInfo.exists())
                icon.addFile(config.defaultIcon);
            else
                icon = QIcon(QPixmap(":/res/" + config.defaultIcon));
        }

        AppItem *widget = new AppItem(m_appWidget);
        widget->setIcon(icon);
        widget->setText(name);
        widget->setDescribe(comment);
        widget->setSizeIncrement(QSize(BoxSize::ItemWidth, BoxSize::ItemHeight));

        m_appWidget->setItemWidget(item, widget);
    }

    return;
}

void ToolBoxWidget::createStayHopeItem(void)
{
    QListWidgetItem *item = new QListWidgetItem(m_appWidget);
    item->setSizeHint(QSize(BoxSize::ItemWidth, BoxSize::ItemHeight));
    item->setWhatsThis("end");
    m_appWidget->addItem(item);

    AppItem *widget = new AppItem(m_appWidget);
    widget->setStayHope();
    widget->setSizeIncrement(QSize(BoxSize::ItemWidth, BoxSize::ItemHeight));

    m_appWidget->setItemWidget(item, widget);

    return;
}

void ToolBoxWidget::getNameComment(const boxconfig_t &config, QString &name, QString &comment)
{
    QString language = QLocale::system().name();
    if (language == "zh_CN") {
        name = config.name_zh_CN;
        comment = config.comment_zh_CN;
    } else if (language == "bo_CN") {
        name = config.name_bo_CN;
        comment = config.comment_bo_CN;
    } else if (language == "mn_MN") {
        name = config.name_mn_MN;
        comment = config.comment_mn_MN;
    } else {
        name = config.name_en_AU;
        comment = config.comment_en_AU;
    }

    return;
}

void ToolBoxWidget::startApp(QListWidgetItem *item)
{
    if (item->whatsThis() == "end") {
        return;
    }

    kom::BuriedPoint::uploadMessage(kom::PluginsToolBox, "StartApp", item->whatsThis());
    QProcess::startDetached(item->whatsThis());

    return;
}
