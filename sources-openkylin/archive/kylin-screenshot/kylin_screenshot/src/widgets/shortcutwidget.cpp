/*
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
.
 * Authors:
 *   huanhuan zhang <zhanghuanhuan@kylinos.cn>
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
#include "shortcutwidget.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QPainter>
#include <QPushButton>
#include <KWindowSystem>
#include <kwindowbuttonbar.h>
#define STYLE_ICON                "icon-theme-name"
#define STYLE_ICON_NAME           "iconThemeName"
#define ORG_UKUI_STYLE            "org.ukui.style"

ShortCutWidget::ShortCutWidget(QWidget *parent) :
    QWidget(parent)
{
    setWindowIcon(QIcon::fromTheme("kylin-screenshot"));
    setAccessibleName("kylin-screenshot_QWidget_shortCutWidget");
    const QByteArray style_id(ORG_UKUI_STYLE);
    if(QGSettings::isSchemaInstalled(style_id)){
        m_styleSettings = new QGSettings(style_id);
    }
#ifndef UNUI
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(winId(), hints);
#endif

    m_titleIcon_label = new  QLabel(this);
    m_titleIcon_label->setPixmap(QPixmap(QIcon::fromTheme("kylin-screenshot").pixmap(QSize(24, 24))));
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ"){
        m_titleIcon_label->move(356, 8);
    }else{
        m_titleIcon_label->move(8, 8);
    }
    m_titleIcon_label->setAccessibleName("kylin-screenshot_QLabel_titleIcon");
    m_titleName_label = new QLabel(this);
    m_titleName_label->setText(tr("Screenshot"));
    m_titleName_label->setAlignment(Qt::AlignLeft);
    m_titleName_label->setFixedSize(170, 25);
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        m_titleName_label->move(186, 8);
    }else{
        m_titleName_label->move(40, 8);
    }
    m_titleName_label->setAccessibleName("kylin-screenshot_QLabel_titleName");
    kdk::KWindowButtonBar *windowButtons = new kdk::KWindowButtonBar(this);
    //获取按钮指针
    minButton = windowButtons->minimumButton();
    minButton->setToolTip(tr("Min"));
    closeButton = windowButtons->closeButton();
    closeButton->setToolTip(tr("Close"));
    //隐藏最大化按钮和菜单按钮
    QPushButton *maxButton = windowButtons->maximumButton();
    kdk::KMenuButton *menuButton = windowButtons->menuButton();
    maxButton->setVisible(false);
    menuButton->setVisible(false);
    //调整按钮栏大小和位置
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        windowButtons->setLayoutDirection(Qt::RightToLeft);
        windowButtons->minimumButton()->setLayoutDirection(Qt::LeftToRight);
        windowButtons->closeButton()->setLayoutDirection(Qt::LeftToRight);
        windowButtons->move(5, 0);
    }else{
        windowButtons->move(331, 0);
    }
    connect(minButton, &QPushButton::clicked, this, &ShortCutWidget::showMinimized);
    connect(closeButton, &QPushButton::clicked, this, &ShortCutWidget::close);
    minButton->setAccessibleName("kylin-screenshot_QPushButton_min");
    closeButton->setAccessibleName("kylin-screenshot_QPushButton_exit");
    tableName = new  QLabel(this);
    tableName->setText(tr("ShortCut"));
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        tableName->move(266, 56);
    }else{
        tableName->move(25, 56);
    }
    tableName->setFixedSize(100, 30);
    tableName->setAccessibleName("kylin-screenshot_QLabel_tableName");
    setFixedSize(400, 546);
    initLabel();
    setFont();
    connect(m_styleSettings,&QGSettings::changed,this,[=](const QString &key){
        if(key==STYLE_ICON_NAME || key==STYLE_ICON){
            setWindowIcon(QIcon::fromTheme("kylin-screenshot"));
            m_titleIcon_label->setPixmap(QPixmap(QIcon::fromTheme("kylin-screenshot").pixmap(QSize(24, 24))));
        }
        if (key == "systemFontSize" || key == "systemFont") {
            setFont();
        }
        if (key == "styleName" || key == "widgetThemeName") {
            updateTitleColor();
        }
    });
    installEventFilter(this);
}

void ShortCutWidget:: initLabel()
{
    for (int i = 0; i < m_keys.size(); i++) {
        QLabel *lb = new  QLabel(this);
        lb->setFixedSize(352, 38);
        QLabel *labelchild1 = new QLabel;
        QLabel *labelchild2 = new QLabel;
        labelchild1->setText(tr(m_description.at(i)));
        labelchild1->setAlignment(Qt::AlignLeft);
        QString sDescWithBlank = m_description.at(i);
        QString sDesc = sDescWithBlank.replace(" ", "");
        lb->setAccessibleName(QString("kylin-screenshot_QLabel_f%1").arg(sDesc));
        labelchild1->setAccessibleName(QString("kylin-screenshot_QLabel_d%1").arg(sDesc));
        labelchild2->setText(tr(m_keys.at(i)));
        QString sKey = m_keys.at(i);
        labelchild2->setAccessibleName(QString("kylin-screenshot_QLabel_k%1").arg(sKey.replace(" ", "")));
        QHBoxLayout *m_layout = new  QHBoxLayout;
        labelchild1->setFixedSize(208, 38);
        m_layout->addWidget(labelchild1);
        m_layout->addWidget(labelchild2);
        lb->setLayout(m_layout);
        labels.append(lb);

        QString fontFamily = m_styleSettings->get("systemFont").toString();
        double fontSize = m_styleSettings->get("systemFontSize").toDouble();
        QFont f(fontFamily);
        f.setPointSizeF(fontSize);
        labelchild1->setFont(f);
        labelchild2->setFont(f);
        connect(m_styleSettings,&QGSettings::changed,this,[=](const QString &key){
           if (key == "systemFontSize" || key == "systemFont") {
               QString fontFamily = m_styleSettings->get("systemFont").toString();
               double fontSize = m_styleSettings->get("systemFontSize").toDouble();
               QFont f(fontFamily);
               f.setPointSizeF(fontSize);
               labelchild1->setFont(f);
               labelchild2->setFont(f);
           }
        });
    }
    for (int i = 0; i < m_keys.size(); i++) {
        labels.at(i)->move(25, 96+i*38);
    }
}

QVector<const char *> ShortCutWidget::m_keys =
{
    QT_TR_NOOP("Keypress"),
    "PrtSc",
    "Ctrl + PrtSc",
    "Shift + PrtSc",
    "←↓↑→",
    "SHIFT + ←↓↑→",
    "ESC",
    "CTRL + C",
    "CTRL + S",
    "CTRL + Z",
    QT_TR_NOOP("Mouse Wheel")
};

QVector <const char *> ShortCutWidget::m_description =
{
    QT_TR_NOOP("Description"),
    QT_TR_NOOP("Capture Full Screen"),
    QT_TR_NOOP("Capture Top Screen"),
    QT_TR_NOOP("Capture Screen selection"),
    QT_TR_NOOP("Move selection 1px"),
    QT_TR_NOOP("Resize selection 1px"),
    QT_TR_NOOP("Quit capture"),
    QT_TR_NOOP("Copy to clipboard"),
    QT_TR_NOOP("Save selection as a file"),
    QT_TR_NOOP("Undo the last modification"),
    QT_TR_NOOP("Change the tool's thickness")
};

void ShortCutWidget::paintEvent(QPaintEvent *e)
{
#ifndef UNUI
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setBrush(QBrush(QColor(95, 95, 95)));
    painter.setOpacity(0.10);
    painter.setPen(QColor(95, 95, 95));
    context.style_settings = new QGSettings("org.ukui.style");
    int borderRadius =  "classical" == context.style_settings->get("widgetThemeName").toString() ? 0 : 6;
    for (int i = 0; i < m_keys.size(); i++) {
        if (0 != i%2) {
            painter.drawRoundedRect(QRect(labels.at(i)->geometry()), borderRadius, borderRadius, Qt::AbsoluteSize);
        }
    }
#endif
}

ShortCutWidget::~ShortCutWidget()
{
}

void ShortCutWidget::setFont()
{
    QString fontFamily = m_styleSettings->get("systemFont").toString();
    double fontSize = m_styleSettings->get("systemFontSize").toDouble();
    QFont f(fontFamily);
    f.setPointSizeF(fontSize + 2);
    tableName->setFont(f);
    f.setPointSizeF(fontSize);
    m_titleName_label->setFont(f);
    // ========= 根据图标中心线和字体高度调整标题文字的纵坐标 =========
    // 先拿到当前这套字体一行文字的高度
    QFontMetrics fm(f);
    int textHeight = fm.height();
    // 图标在父窗口中的位置和高度（以实际显示的 pixmap 高度为准）
    int iconTop = m_titleIcon_label->y();
    int iconHeight = m_titleIcon_label->pixmap()->height();
    int iconCenterY = iconTop + iconHeight / 2; // 图标的竖直中心线
    // 让“截图”这行字的中心 = 图标中心
    int labelTopY = iconCenterY - textHeight / 2;
    labelTopY = qMax(0, labelTopY); // 防止出现负数

    // 只改 y，不改 x（x 仍然按语言在构造函数里控制）
    m_titleName_label->move(m_titleName_label->x(), labelTopY);
}

void ShortCutWidget::changeEvent(QEvent *event){
    if( event->type() == QEvent::ActivationChange){
        updateTitleColor();
    }
    QWidget::changeEvent(event);
}

void ShortCutWidget::updateTitleColor()
{
    QString themeStyle = m_styleSettings->get("styleName").toString();
    bool isDark = (themeStyle == "ukui-dark" || themeStyle == "ukui-black");
    bool active = this->isWindow() ? isActiveWindow() : window()->isActiveWindow();
    // 根据窗口激活状态和主题，通过调色板设置颜色
    // 同时利用 enabled/disabled 状态，让未激活窗口自动使用灰色（Disabled 颜色）
    m_titleName_label->setEnabled(active);

    QPalette pal = m_titleName_label->palette();
    if (active) {
        // 激活窗口：深色主题用白色，浅色主题用黑色
        pal.setColor(QPalette::WindowText, isDark ? Qt::white : Qt::black);
        m_titleName_label->setPalette(pal);
    }
}

