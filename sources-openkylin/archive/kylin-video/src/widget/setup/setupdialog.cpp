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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "setupdialog.h"
#include "ui_setupdialog.h"
#include <KF5/KWindowSystem/kwindoweffects.h>

#include <QDebug>
#include <QPainter>
#include <QGSettings>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QListWidgetItem>

#include "global/globalsignal.h"
#include "global/global.h"

#include "setupcodec.h"
#include "setupplay.h"
#include "setupscreenshot.h"
#include "setupshortcut.h"
#include "setupsubtitle.h"
#include "setupsystem.h"
#include "setupvolume.h"

using namespace Global;

SetUpDialog::SetUpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetUpDialog)
{
    ui->setupUi(this);
    initStyle();
    initListWidget();

    if (FOLLOW_SYS_THEME) {
        // 根据主题设置样式
        if(g_gsettings->get("style-name").toString() == STYLE_UKUI_LIGHT)
            slotChangeTheme(false);
        else
            slotChangeTheme(true);
    }
    else {
        slotChangeTheme(true);
    }
    initConnect();
}

SetUpDialog::~SetUpDialog()
{
    delete ui;
}

void SetUpDialog::setIndex(int index)
{
    ui->listWidget->setCurrentRow(index);
}

/** **********************************************
 * 主题颜色修改
 * 说明：所有需要根据主题修改颜色的控件都在此函数设置样式
 * @param: is_black_theme 是否为黑色主题
 *************************************************/
void SetUpDialog::slotChangeTheme(bool is_black_theme)
{
    m_isBlackTheme = is_black_theme;
    QString listHoverColor = "";
    if(is_black_theme)
    {
        m_leftColor.setRgb(19, 19, 20);
//        m_systemPage->setBlackTheme();
        m_playPage->setBlackTheme();
        m_screenshotPage->setBlackTheme();
        m_subtitlePage->setBlackTheme();
        m_volumePage->setBlackTheme();
        m_codecPage->setBlackTheme();
        m_shortcutPage->setBlackTheme();

        ui->right->setStyleSheet("#right{background-color:rgb(38,38,38);}");
        ui->lab_name->setStyleSheet("color:rgb(255,255,255);");
        // 设置左侧list字体颜色
        for(int i=0; i<ui->listWidget->count(); i++)
        {
            QListWidgetItem *it = ui->listWidget->item(i);
            it->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            it->setSizeHint(QSize(116, 32));
            it->setTextColor(QColor(249, 249, 249));
        }
        listHoverColor = "222,222,222";
    }
    else
    {
        m_leftColor.setRgb(249, 249, 249);
//        m_systemPage->setDefaultTheme();
        m_playPage->setDefaultTheme();
        m_screenshotPage->setDefaultTheme();
        m_subtitlePage->setDefaultTheme();
        m_volumePage->setDefaultTheme();
        m_codecPage->setDefaultTheme();
        m_shortcutPage->setDefaultTheme();

        ui->right->setStyleSheet("#right{background-color:#f9f9f9;}");
        ui->lab_name->setStyleSheet("color:rgb(48,49,51);");

        // 设置左侧list字体颜色
        for(int i=0; i<ui->listWidget->count(); i++)
        {
            QListWidgetItem *it = ui->listWidget->item(i);
            it->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            it->setSizeHint(QSize(116, 32));
            it->setTextColor(QColor(Qt::black));
        }
        listHoverColor = "26,26,26";
    }
    updateHightLightColor();
    ui->btn_cancel->setProperty("useButtonPalette", true);
    update();
}

bool SetUpDialog::event(QEvent *e)
{
    // 主要处理 palette 事件
    if (e->type() == QEvent::ApplicationPaletteChange) {
        m_highlightColor = QApplication::palette(this).highlight().color();
        updateHightLightColor();
    }
    return QDialog::event(e);
}

void SetUpDialog::showEvent(QShowEvent *e)
{
    m_systemPage->initData();
    m_playPage->initData();
    m_screenshotPage->initData();
    m_subtitlePage->initData();
    m_volumePage->initData();
    // 每次显示前，确保快捷键页从后端状态刷新，避免显示未保存的冲突值
    m_shortcutPage->reloadFromShortcuts();

    resize(620, 400);
    g_shortcut->makeAllInvalid();
    resetFont();
}

void SetUpDialog::hideEvent(QHideEvent *e)
{
    g_shortcut->makeAllValid();
}

void SetUpDialog::done(int r)
{
    // 对话框关闭前，先让当前焦点控件失去焦点，触发focusOutEvent,确保ShortEdit中的KDE全局快捷键被正确恢复
    if (QWidget *focusWidget = QApplication::focusWidget()) {
        focusWidget->clearFocus();
    }
    QDialog::done(r);
}

void SetUpDialog::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    m_leftColor.setAlphaF(m_windowOpacity);
    QBrush brush = QBrush(m_leftColor);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect, 0, 0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SetUpDialog::updateHightLightColor()
{
    QString listHoverColor = m_isBlackTheme ? "222,222,222" : "26,26,26";
    ui->listWidget->setStyleSheet(
                QString("QListWidget{background-color:rgba(19,19,20,0);padding:0px 5px;}"
                        "QListWidget::item{width:116px;border-radius:6px;}"
                        "QListWidget::item:selected{color:rgb(255,255,255);background-color:%1;}"
                        "QListWidget::item:hover:!selected{background-color:rgba(%2,30);}")
                        .arg(m_highlightColor.name())
                        .arg(listHoverColor));

    {
//        QPalette tmp_palette = palette();
//        m_systemPage->setPalette(tmp_palette);
    }
}

void SetUpDialog::initStyle()
{
    if (g_gsettings_control_center)
        m_windowOpacity = g_gsettings_control_center->get("transparency").toDouble();
    else
        m_windowOpacity = 1.0;
    setFixedSize(620, 400);
    setAttribute(Qt::WA_TranslucentBackground);

    m_highlightColor = QApplication::palette(this).highlight().color();
    // 标题
    setWindowTitle(tr("Video Player Set up"));

    {
        m_systemPage = new SetupSystem;
        m_systemPage->setAccessibleName("kylin-video_SetupSystem_m_systemPage");
        ui->stackedWidget->addWidget(m_systemPage);

        m_playPage = new SetupPlay;
        m_playPage->setAccessibleName("kylin-video_SetupPlay_m_playPage");
        ui->stackedWidget->addWidget(m_playPage);

        m_screenshotPage = new SetupScreenshot;
        m_screenshotPage->setAccessibleName("kylin-video_SetupScreenshot_m_screenshotPage");
        ui->stackedWidget->addWidget(m_screenshotPage);

        m_subtitlePage = new SetupSubtitle;
        m_subtitlePage->setAccessibleName("kylin-video_SetupSubtitle_m_subtitlePage");
        ui->stackedWidget->addWidget(m_subtitlePage);

        m_volumePage = new SetupVolume;
        m_volumePage->setAccessibleName("kylin-video_SetupVolume_m_volumePage");
        ui->stackedWidget->addWidget(m_volumePage);

        m_codecPage = new SetupCodec;
        m_codecPage->setAccessibleName("kylin-video_SetupCodec_m_codecPage");
        ui->stackedWidget->addWidget(m_codecPage);

        m_shortcutPage = new SetupShortcut;
        m_shortcutPage->setAccessibleName("kylin-video_SetupShortcut_m_shortcutPage");
        ui->stackedWidget->addWidget(m_shortcutPage);
    }

    // 左侧列表
    ui->listWidget->setFixedWidth(140);
    ui->listWidget->setSpacing(2);
    // 设置默认选中左侧列表中第一个
    ui->listWidget->setCurrentRow(0);
    ui->stackedWidget->setCurrentIndex(0);

    ui->widget->setAttribute(Qt::WA_TranslucentBackground);
    ui->lab_name->setAttribute(Qt::WA_TranslucentBackground);

    ui->btn_icon->setFixedSize(QSize(24,24));
    ui->btn_icon->setIconSize(QSize(24,24));
    ui->btn_icon->setIcon(QIcon::fromTheme("kylin-video"));
    ui->btn_icon->setStyleSheet("QPushButton{border:0px;background:transparent;}"
                                "QPushButton::hover{border:0px;background:transparent;}"
                                "QPushButton::pressed{border:0px;background:transparent;}");
    ui->btn_icon->setAccessibleName("kylin-video_QPushButton_btn_icon");
    QFont f("Noto Sans CJK SC Regular");
    f.setPixelSize(14);
    ui->lab_name->setFont(f);
    ui->lab_name->setText(tr("Setup"));

    ui->btn_close->setFixedSize(30, 30);
    ui->btn_close->setProperty("isWindowButton", 0x2);
    ui->btn_close->setProperty("useIconHighlightEffect", 0x8);
    ui->btn_close->setFlat(true);
    ui->btn_close->setIcon(QIcon::fromTheme("window-close-symbolic"));
    ui->btn_close->setToolTip(tr("close"));
    ui->btn_close->setAccessibleName("kylin-video_QPushButton_btn_close");

    connect(ui->btn_close, &QPushButton::clicked, [&](){accept();});

    resetFont();
}

/** **********************************************
 * 初始化左侧列表
 *************************************************/
void SetUpDialog::initListWidget()
{
    ui->listWidget->addItem(" " + tr("System"));
    ui->listWidget->addItem(" " + tr("Play"));
    ui->listWidget->addItem(" " + tr("ScreenShot"));
    ui->listWidget->addItem(" " + tr("Subtitle"));
    ui->listWidget->addItem(" " + tr("Audio"));
    ui->listWidget->addItem(" " + tr("Codec"));
    ui->listWidget->addItem(" " + tr("Shortcut"));
    for(int i=0; i<ui->listWidget->count(); ++i)
    {
        QListWidgetItem *it = ui->listWidget->item(i);
        resetText(it);
    }
}

void SetUpDialog::initConnect()
{
    if (FOLLOW_SYS_THEME) {
        // 主题改变
        connect(g_user_signal, &GlobalUserSignal::sigTheme, this, [&](int theme){
            switch (theme) {
            case 0:
                if(g_gsettings->get("style-name").toString() == STYLE_UKUI_LIGHT)
                    slotChangeTheme(false);
                else
                    slotChangeTheme(true);
                break;
            case 1:
                slotChangeTheme(false);
                break;
            case 2:
                slotChangeTheme(true);
                break;
            default:
                break;
            }
        });
    }
    connect(g_gsettings, &QGSettings::changed, this, [&](QString key){
        if (FOLLOW_SYS_THEME) {
            // 如果不是跟随主题的话直接返回,0是跟随主题
            if (key == "styleName") {
                if(g_gsettings->get("style-name").toString() == STYLE_UKUI_LIGHT)
                    slotChangeTheme(false);
                else
                    slotChangeTheme(true);
            }
        }

        // 字体大小改变
        if (key == "systemFontSize" || key == "systemFont") {
            resetFont();
        }
    });

    if (g_gsettings_control_center)
        connect(g_gsettings_control_center, &QGSettings::changed, this, [&](QString key){
            // 如果不是跟随主题的话直接返回
            if (key == "transparency") {
                m_windowOpacity = g_gsettings_control_center->get("transparency").toDouble();
                update();
            }
        });

    connect(ui->listWidget, &QListWidget::currentRowChanged, [&](int index){ui->stackedWidget->setCurrentIndex(index);});
    connect(ui->btn_cancel, &QPushButton::clicked, [&](){m_shortcutPage->clearChange();g_config->clearChange();accept();});
    connect(ui->btn_ok, &QPushButton::clicked, [&](){m_shortcutPage->flushChange();g_config->flushChange();accept();});
}

void SetUpDialog::resetFont()
{
    QString fontFamily = g_gsettings->get("system-font").toString();
    int fontSize = g_gsettings->get("systemFontSize").toDouble();
    double fsize = g_gsettings->get("systemFontSize").toDouble();

    m_systemPage->setWidgetFont(fontFamily, fontSize);
    m_codecPage->setWidgetFont(fontFamily, fontSize);
    m_playPage->setWidgetFont(fontFamily, fontSize);
    m_subtitlePage->setWidgetFont(fontFamily, fontSize);
    m_shortcutPage->setWidgetFont(fontFamily, fontSize);
    m_volumePage->setWidgetFont(fontFamily, fontSize);
    m_screenshotPage->setWidgetFont(fontFamily, fontSize);

    QFont f(fontFamily);
    f.setPointSize(fontSize);
    ui->listWidget->setFont(f);
    ui->btn_cancel->setFont(f);
    ui->btn_ok->setFont(f);

    f.setPointSize(fontSize);
    ui->lab_name->setFont(f);

    for(int i=0; i<ui->listWidget->count(); ++i)
    {
        QListWidgetItem *it = ui->listWidget->item(i);
        resetText(it);
    }
}

void SetUpDialog::resetText(QListWidgetItem* item)
{
    QFontMetrics fontWidth(font());//得到每个字符的宽度
    QString item_name = item->text();
    if (item->toolTip() != "")
    {
        item_name = item->toolTip();
    }
    QString show_name = fontWidth.elidedText(item_name, Qt::ElideRight, ui->listWidget->width() - 30);
    item->setText(show_name);
    if (show_name.endsWith("…"))
        item->setToolTip(item_name);
    else
        item->setToolTip("");
}
