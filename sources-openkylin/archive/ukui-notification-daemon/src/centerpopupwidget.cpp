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

#include "centerpopupwidget.h"
#include "commondefinition.h"
#include <QProcess>
#include <QUrl>
#include <QStyleOption>
#include <QPainter>
#include <QToolButton>
#define TOOLBUTTON_LEFT_RIGHT_PADDING  26

CenterPopupWidget::CenterPopupWidget(QWidget *parent, NotifyReceiveInfo *entryInfo)
    : QWidget(parent)
    , m_currentNotifyInfo(entryInfo)
    , m_pIconLabel(new QLabel(this))
    , m_pSummaryLabel(new QLabel(this))
    , m_pBodyLabel(new QLabel(this))
    , m_pCloseButton(new QPushButton(this))
{
    /* 背景遮挡窗口 */
    m_centerBackground = new CenterPopupBackgroundWidget(nullptr);
    /* 获取主屏窗口属性 */
    m_pSreenInfo = new adaptScreenInfo(this);
    connect(m_pSreenInfo, &adaptScreenInfo::updataCenterWidgetSite, this, [=]() { updataWidgetLocation(); });
    /* 设置窗口属性 */
    setWidgetAttribute();
    /* 监听主题变化 */
    systemThemeChanges();
    /* 初始化图标 */
    initIconWidgetlayout();
    /* 初始化关闭按钮 */
    initCloseButtonWidget();
    /* 初始化主题、正文  */
    initSummaryBodyWidget();
    /* 初始化操作按钮 */
    initButtonWidget();
    /* 初始化整体UI */
    initMainUiLayout();
    /* 设置窗口显示位置 */
    updataWidgetLocation();

}

CenterPopupWidget::~CenterPopupWidget()
{
    if (m_pThemeGsetting) {
        delete m_pThemeGsetting;
        m_pThemeGsetting = NULL;
    }
    if (m_centerBackground) {
        delete m_centerBackground;
        m_centerBackground = NULL;
    }
}

/* 监听主题变化 */
void CenterPopupWidget::systemThemeChanges()
{
    const QByteArray styleId(ORG_UKUI_STYLE);
    if (QGSettings::isSchemaInstalled(styleId)) {
        m_pThemeGsetting = new QGSettings(styleId);
        if (m_pThemeGsetting->keys().contains(UKUI_TRANSPARENCY_SETTING_KEY)) {
            m_fTransparencyValue = m_pThemeGsetting->get(UKUI_TRANSPARENCY_SETTING_KEY).toDouble();

        }
        connect(m_pThemeGsetting, &QGSettings::changed, this, [=](const QString &key) {
            if (key == ICON_THEME_NAME) {
                convertToImage(m_currentNotifyInfo->appIcon());//更新图标
            } else if(key==STYLE_NAME || key==SYSTEM_FONT_SIZE || key==SYSTEM_FONT){
                //更新主体和正文
                if(!m_currentNotifyInfo->summary().isEmpty() && m_currentNotifyInfo->body().isEmpty()) {
                    updataSummaryTextFormat(m_currentNotifyInfo->summary(), m_pSummaryLabel);
                } else if(m_currentNotifyInfo->summary().isEmpty() && !m_currentNotifyInfo->body().isEmpty()) {
                    updataBodyTextFormat(m_currentNotifyInfo->body(), m_pBodyLabel);
                } else if(!m_currentNotifyInfo->summary().isEmpty() && !m_currentNotifyInfo->body().isEmpty()) {
                    updataSummaryBodyTextFormat(m_currentNotifyInfo->summary(), m_pSummaryLabel,
                                                m_currentNotifyInfo->body(), m_pBodyLabel);
                }
            } else if (key == UKUI_TRANSPARENCY_SETTING_KEY) {
                m_fTransparencyValue = m_pThemeGsetting->get(UKUI_TRANSPARENCY_SETTING_KEY).toDouble();
                update();
            }
        });
    }
}

/* 设置窗口属性 */
void CenterPopupWidget::setWidgetAttribute()
{
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::X11BypassWindowManagerHint); //无边框、禁止拖动、禁止改变大小、不受窗管管理
    setAttribute(Qt::WA_TranslucentBackground);                    //透明背景
    setAttribute(Qt::WA_AlwaysShowToolTips);

    return;
}

/* 初始化图标布局 */
void CenterPopupWidget::initIconWidgetlayout()
{
    m_pIconWidget = new QWidget(this);
    m_pIconWidget->setFixedSize(MASTER::centerIconWidgetSize);
    m_pIconWidget->setContentsMargins(0, 0, 0, 0);
    m_pIconWidgetLayout = new QVBoxLayout(m_pIconWidget);
    m_pIconWidgetLayout->setSpacing(0);
    /* 初始化图标label大小 */
    convertToImage(m_currentNotifyInfo->appIcon());
    m_pIconLabel->setFixedSize(MASTER::centerIconLabelSize);
    m_pIconLabel->setMargin(0);
    m_pIconLabel->setContentsMargins(0, 0, 0, 0);
    m_pIconWidgetLayout->setContentsMargins(0, 0, 0, 0);
    m_pIconWidgetLayout->addWidget(m_pIconLabel);
    m_pIconWidgetLayout->addItem(new QSpacerItem(10, 200, QSizePolicy::Expanding));
    m_pIconWidget->setLayout(m_pIconWidgetLayout);
}

/* 初始化关闭按钮 */
void CenterPopupWidget::initCloseButtonWidget()
{
    m_pCloseButtonWidget = new QWidget(this);
    m_pCloseButtonWidget->setFixedSize(MASTER::centerCloseBtnWidgetSize);
    m_pCloseButtonWidget->setContentsMargins(0, 0, 0, 0);
    m_pCloseWidgetLayout = new QHBoxLayout(m_pCloseButtonWidget);
    m_pCloseWidgetLayout->setContentsMargins(0, 5, 5, 5);
    m_pCloseWidgetLayout->setSpacing(0);

    QIcon closeButtonIcon = QIcon::fromTheme("window-close-symbolic");
    m_pCloseButton->setIcon(closeButtonIcon);
    m_pCloseButton->setIconSize(MASTER::centerCloseBtnIconSize);
    m_pCloseButton->setFixedSize(MASTER::centerCloseBtnSize);
    m_pCloseButton->setProperty("isWindowButton", 0x02);
    m_pCloseButton->setProperty("useIconHighlightEffect", 0x08);
    m_pCloseButton->setFlat(true);

    connect(m_pCloseButton, &QPushButton::clicked, this, &CenterPopupWidget::closeButtonSlots);
    m_pCloseWidgetLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
    m_pCloseWidgetLayout->addWidget(m_pCloseButton);
    m_pCloseButtonWidget->setLayout(m_pCloseWidgetLayout);
}

/* 初始化主题和正文布局 */
void CenterPopupWidget::initSummaryBodyWidget()
{
    m_pSummaryBodyWidget = new QWidget(this);
    m_pSummaryBodyWidget->setContentsMargins(0, 0, 0, 0);
    m_pSummaryBodyWidgetHLayout = new QHBoxLayout(m_pSummaryBodyWidget);
    m_pSummaryBodyWidgetHLayout->setContentsMargins(0, 0, 0, 0);
    m_pSummaryBodyWidgetHLayout->setSpacing(0);
    m_pSummaryBodyWidgetVLayout = new QVBoxLayout(m_pSummaryBodyWidget);
    m_pSummaryBodyWidgetVLayout->setContentsMargins(0, 0, 0, 0);
    m_pSummaryBodyWidgetVLayout->setSpacing(0);
    m_pSummaryLabel->setContentsMargins(0,0,0,0);
    m_pSummaryLabel->setAlignment(Qt::AlignVCenter);
    //只有主题：只显示一行。文本过长，超过部分省略显示
    //只有正文：正文不超过三行，正常显示；文本过长，超过部分省略显示
    //主题+正文：主题一行，超过省略；正文不超过两行，超过则省略
    if(!m_currentNotifyInfo->summary().isEmpty() && m_currentNotifyInfo->body().isEmpty()) {
        this->setFixedSize(MASTER::centerWidgetSize);
        m_pSummaryLabel->setVisible(true);
        m_pBodyLabel->setVisible(false);
        m_pSummaryBodyWidget->setFixedSize(MASTER::centerOnlySummarySize);
        updataSummaryTextFormat(m_currentNotifyInfo->summary(), m_pSummaryLabel);
        m_pSummaryBodyWidgetHLayout->addItem(new QSpacerItem(24, 10, QSizePolicy::Fixed));
        m_pSummaryBodyWidgetHLayout->addWidget(m_pIconWidget);
        m_pSummaryBodyWidgetHLayout->addItem(new QSpacerItem(8, 10, QSizePolicy::Fixed));
        m_pSummaryBodyWidgetHLayout->addWidget(m_pSummaryLabel);
        m_pSummaryBodyWidgetHLayout->addItem(new QSpacerItem(24, 10, QSizePolicy::Fixed));
        m_pSummaryBodyWidget->setLayout(m_pSummaryBodyWidgetHLayout);
    } else if(m_currentNotifyInfo->summary().isEmpty() && !m_currentNotifyInfo->body().isEmpty()) {
        m_pSummaryLabel->setVisible(false);
        m_pBodyLabel->setVisible(true);
        m_pBodyLabel->setContentsMargins(0,0,0,0);
        m_pBodyLabel ->setAlignment(Qt::AlignVCenter);
        updataBodyTextFormat(m_currentNotifyInfo->body(), m_pBodyLabel);
        m_pSummaryBodyWidgetHLayout->addItem(new QSpacerItem(24, 10, QSizePolicy::Fixed));
        m_pSummaryBodyWidgetHLayout->addWidget(m_pIconWidget);
        m_pSummaryBodyWidgetHLayout->addItem(new QSpacerItem(8, 10, QSizePolicy::Fixed));
        m_pSummaryBodyWidgetHLayout->addWidget(m_pBodyLabel);
        m_pSummaryBodyWidgetHLayout->addItem(new QSpacerItem(24, 10, QSizePolicy::Fixed));
        m_pSummaryBodyWidget->setLayout(m_pSummaryBodyWidgetHLayout);
    } else if(!m_currentNotifyInfo->summary().isEmpty() && !m_currentNotifyInfo->body().isEmpty()) {
        m_pSummaryLabel->setVisible(true);
        m_pBodyLabel->setVisible(true);
        m_pSummaryBodyWidget->setFixedSize(MASTER::centerSummaryBodyWidgetSize);
        updataSummaryBodyTextFormat(m_currentNotifyInfo->summary(), m_pSummaryLabel, m_currentNotifyInfo->body(), m_pBodyLabel);

        //正文
        QWidget *m_pBodyWidget = new QWidget(m_pSummaryBodyWidget);
        m_pBodyWidget->setContentsMargins(0, 0, 0, 0);
        m_pBodyWidget->setFixedWidth(376);
        QHBoxLayout *m_pBodyWidgetHLayout = new QHBoxLayout(m_pBodyWidget);
        m_pBodyWidgetHLayout->setContentsMargins(0, 0, 0, 0);
        m_pBodyWidgetHLayout->setSpacing(0);

        m_pBodyWidgetHLayout->addItem(new QSpacerItem(56, 10, QSizePolicy::Fixed));
        m_pBodyWidgetHLayout->addWidget(m_pBodyLabel);
        m_pBodyWidget->setLayout(m_pBodyWidgetHLayout);

        QWidget *m_pIconSummaryWidget = new QWidget(m_pSummaryBodyWidget);
        m_pIconSummaryWidget->setContentsMargins(0, 0, 0, 0);
        m_pIconSummaryWidget->setFixedWidth(376);
        m_pSummaryBodyWidgetHLayout->addItem(new QSpacerItem(24, 10, QSizePolicy::Fixed));
        m_pSummaryBodyWidgetHLayout->addWidget(m_pIconWidget);
        m_pSummaryBodyWidgetHLayout->addItem(new QSpacerItem(8, 10, QSizePolicy::Fixed));
        m_pSummaryBodyWidgetHLayout->addWidget(m_pSummaryLabel);
        m_pSummaryBodyWidgetHLayout->addItem(new QSpacerItem(24, 10, QSizePolicy::Fixed));
        m_pIconSummaryWidget->setLayout(m_pSummaryBodyWidgetHLayout);

        m_pSummaryBodyWidgetVLayout->addWidget(m_pIconSummaryWidget);
        m_pSummaryBodyWidgetVLayout->addItem(new QSpacerItem(10, 9, QSizePolicy::Fixed));
        m_pSummaryBodyWidgetVLayout->addWidget(m_pBodyWidget);
        m_pSummaryBodyWidget->setLayout(m_pSummaryBodyWidgetVLayout);
    }
}

/* 初始化按钮布局 */
void CenterPopupWidget::initButtonWidget()
{
    m_pButtonWidget = new QWidget(this);
    m_pButtonWidget->setFixedSize(MASTER::centerButtonWidgetSize);
    m_pButtonWidget->setContentsMargins(0, 0, 0, 0);
    m_pButtonWidgetLayout = new QHBoxLayout(m_pButtonWidget);
    m_pButtonWidgetLayout->setSpacing(10);
    m_pButtonWidgetLayout->setContentsMargins(0, 0, 0, 0);
    m_pButtonWidget->setLayout(m_pButtonWidgetLayout);
    m_pButtonWidgetLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
    //解析 actions 字段，绑定操作按钮
    processActionsBotton();
    m_pButtonWidgetLayout->addItem(new QSpacerItem(24, 10, QSizePolicy::Fixed));
}

void CenterPopupWidget::initMainUiLayout()
{
    this->setContentsMargins(0, 0, 0, 0);
    this->setMinimumSize(MASTER::centerWidgetSize);
    m_pMainWidgetLayout = new QVBoxLayout(this);
    m_pMainWidgetLayout->setContentsMargins(0, 0, 0, 0);
    m_pMainWidgetLayout->setSpacing(0);
    m_pMainWidgetLayout->addWidget(m_pCloseButtonWidget);
    m_pMainWidgetLayout->addWidget(m_pSummaryBodyWidget);
    m_pMainWidgetLayout->addItem(new QSpacerItem(10, 32, QSizePolicy::Maximum));
    m_pMainWidgetLayout->addWidget(m_pButtonWidget);
    m_pMainWidgetLayout->addItem(new QSpacerItem(10, 24, QSizePolicy::Fixed));
    this->setLayout(m_pMainWidgetLayout);
}

/* 更新主题文本格式 */
void CenterPopupWidget::updataSummaryTextFormat(QString summary, QLabel *label)
{
    label->setFixedSize(MASTER::centerOnlySummaryLabelSize);
    QString systemFont = m_pThemeGsetting->get(SYSTEM_FONT).toString();
    QString systemFontSize = m_pThemeGsetting->get(SYSTEM_FONT_SIZE).toString();

    //设置主题字体透明度
    QColor color = label->palette().text().color();
    color.setAlpha(255 * MASTER::centerSummaryFontOpacity);
    QPalette palette;
    palette.setColor(QPalette::WindowText, color);
    label->setPalette(palette);

    QFont summaryFont(systemFont, systemFontSize.toDouble());
    summaryFont.setBold(true);//主题文字加粗
    QString formatSummary;
    QFontMetrics fontMetrics(summaryFont);
    int LableWidth = label->width();
    int fontSize = fontMetrics.width(summary);
    if (fontSize > LableWidth) {
        formatSummary = summary.simplified();
        formatSummary = fontMetrics.elidedText(formatSummary, Qt::ElideRight,
                                               LableWidth - systemFontSize.toDouble()*3);
        label->setText(formatSummary);
        label->setToolTip(summary);
    } else {
        label->setText(summary);
    }
}

/* 更新正文文本格式 */
void CenterPopupWidget::updataBodyTextFormat(QString body, QLabel *label)
{
    QString systemFont = m_pThemeGsetting->get(SYSTEM_FONT).toString();
    QString systemFontSize = m_pThemeGsetting->get(SYSTEM_FONT_SIZE).toString();
    QFont bodyFont(systemFont, systemFontSize.toDouble());
    QFontMetrics fontMetrics(bodyFont);
    int fontSize = fontMetrics.width(body);
    if (fontSize < MASTER::centerOnlyBodyOneLineLabelSize.width()) { //一行正文
        this->setFixedSize(MASTER::centerWidgetOnlyBodyOneLineSize);
        m_pSummaryBodyWidget->setFixedSize(MASTER::centerOnlyBodyOneLineSize);
        label->setFixedSize(MASTER::centerOnlyBodyOneLineLabelSize);
        QString formatBody;
        int LableWidth = m_pBodyLabel->width();
        formatBody = m_currentNotifyInfo->body().simplified();
        formatBody = fontMetrics.elidedText(formatBody, Qt::ElideRight, LableWidth - systemFontSize.toDouble()*3);
        m_pBodyLabel->setText(formatBody);
    } else { //多行正文
        m_pIconWidget->setFixedHeight(MASTER::centerOnlyBodyMultilineLabelSize.height());
        this->setFixedSize(MASTER::centerWidgetOnlyBodyMultilineSize);
        m_pSummaryBodyWidget->setFixedSize(MASTER::centerOnlyBodyMultilineSize);
        label->setFixedSize(MASTER::centerOnlyBodyMultilineLabelSize);
        QString formatBody;
        int LableWidth = m_pBodyLabel->width();
        formatBody = m_currentNotifyInfo->body().simplified();
        formatBody = fontMetrics.elidedText(formatBody, Qt::ElideRight, (LableWidth - systemFontSize.toDouble()*3)*3);
        m_pBodyLabel->setText(convertMultilineText(m_pBodyLabel, formatBody));
        m_pBodyLabel->setToolTip(convertMultilineText(m_pBodyLabel, m_currentNotifyInfo->body()));
    }
    //设置正文字体透明度
    QColor color = label->palette().text().color();
    color.setAlpha(255 * MASTER::centerSummaryFontOpacity);
    QPalette palette;
    palette.setColor(QPalette::WindowText, color);
    label->setPalette(palette);
}

/* 更新主题和正文文本格式 */
void CenterPopupWidget::updataSummaryBodyTextFormat(QString summary, QLabel *summaryLabel, QString body, QLabel *bodyLabel)
{
    QString systemFont = m_pThemeGsetting->get(SYSTEM_FONT).toString();
    QString systemFontSize = m_pThemeGsetting->get(SYSTEM_FONT_SIZE).toString();
    updataSummaryTextFormat(summary, summaryLabel);

    //设置正文字体透明度
    QColor color = bodyLabel->palette().text().color();
    color.setAlpha(255 * MASTER::centerBodyFontOpacity);
    QPalette palette;
    palette.setColor(QPalette::WindowText, color);
    bodyLabel->setPalette(palette);

    bodyLabel->setFixedSize(MASTER::centerBodyLabelSize);
    bodyLabel->setContentsMargins(0,0,0,0);
    QString formatBody;
    int LableWidth = bodyLabel->width();
    formatBody = body.simplified();
    QFontMetrics fontMetricsBody(bodyLabel->font());
    formatBody = fontMetricsBody.elidedText(formatBody, Qt::ElideRight, (LableWidth - systemFontSize.toDouble()*3)*2);
    bodyLabel->setText(convertMultilineText(bodyLabel, formatBody));
    bodyLabel->setToolTip(convertMultilineText(bodyLabel, body));
}

/* 关闭按钮槽函数 */
void CenterPopupWidget::closeButtonSlots()
{
    m_closeReason = CenterPopupWidget::CLOSEDBYUSER;
    emit clickedCloseBtn(m_currentNotifyInfo->id().toInt(), m_closeReason);
    this->hide();
    this->deleteLater();

    return;
}

/* 更新窗口显示位置 */
void CenterPopupWidget::updataWidgetLocation()
{
    this->show();
    QPoint point;
    point.setX(m_pSreenInfo->m_nScreen_x + m_pSreenInfo->m_screenWidth/2 - this->width()/2);
    point.setY(m_pSreenInfo->m_nScreen_y + m_pSreenInfo->m_screenHeight/2 - this->height()/2);
    this->move(point); 
}


/* 设置通知图标 */
void CenterPopupWidget::convertToImage(QString iconPath)
{
    const qreal pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    QPixmap pixmap;
    pixmap = QIcon::fromTheme(iconPath).pixmap(MASTER::centerIconLabelSize);
    if (pixmap.isNull()) {
        QString iconUrl;
        const QUrl url(iconPath);
        iconUrl = url.isLocalFile() ? url.toLocalFile() : url.url();
        const QIcon &icon = QIcon::fromTheme(iconPath, QIcon::fromTheme("dialog-information"));
        pixmap = icon.pixmap(MASTER::centerIconLabelSize);
    }

    if (!pixmap.isNull()) {
        pixmap = pixmap.scaled(
            MASTER::centerIconLabelSize.width() * pixelRatio,
            MASTER::centerIconLabelSize.height() * pixelRatio,
            Qt::KeepAspectRatioByExpanding,
            Qt::SmoothTransformation);
        pixmap.setDevicePixelRatio(pixelRatio);
    }
    m_pIconLabel->setPixmap(pixmap);
    return;
}

/* 设置button字体长度，超出button时，设置... */
QString CenterPopupWidget::setButtonStringBody(QString text, QToolButton *button)
{
//    QString systemFontSize = m_pThemeGsetting->get(SYSTEM_FONT_SIZE).toString();
    if(button) {
        QFontMetrics fontMetrics(button->font());
        int availableWidth = button->size().width() - TOOLBUTTON_LEFT_RIGHT_PADDING;
        int fontSize = fontMetrics.width(text);
        QString formatBody = text;
        if (fontSize > availableWidth) {
            button->setToolTip(text);
            formatBody = fontMetrics.elidedText(formatBody, Qt::ElideRight, availableWidth - 10);
            return formatBody;
        } else {
            button->setToolTip(QString());
            return formatBody;
        }
    }
    return text;
}

/* 单行换成多行显示 */
QString CenterPopupWidget::convertMultilineText(QLabel *label, QString text)
{
    QString systemFontSize = m_pThemeGsetting->get(SYSTEM_FONT_SIZE).toString();
    QString strText = text;
    int AntoIndex = 1;
    QFont font = label->font();
    QFontMetrics fm(font);
    int labelWidth = label->width();

    if (!strText.isEmpty()) {
        for (int i = 1; i < strText.size() + 1; i++) {
            if (fm.width(strText.left(i)) >= (labelWidth-systemFontSize.toDouble()*2) * AntoIndex) { //当strText宽度大于labelWidth的时候添加换行符
                AntoIndex++;
                strText.insert(i - 1, "\n");
            }
        }
    }
    return strText;
}

void CenterPopupWidget::processActionsBotton()
{
    if(m_currentNotifyInfo->actions().isEmpty()) {
        m_pButtonWidget->setVisible(false);
    } else {
        QStringList list = m_currentNotifyInfo->actions();
        if (list.contains("default")) { //不绑定默认动作
            const int index = list.indexOf("default");
            list.removeAt(index);
            list.removeAt(index - 1);
        }
        //偶数元素是按钮动作，奇数元素是按钮名
        QString buttonAction;
        for (int i = 0; i != list.size(); ++i) {
            if (i % 2 == 0) {
                buttonAction = list[i];
            } else {
                QString buttonText = list[i];
                QToolButton *button = new QToolButton(this);
                button->setFixedSize(MASTER::centerButtonSize);
                connect(button, &QToolButton::clicked, this, [=](){
                    QProcess::startDetached(buttonAction);
                    m_closeReason = CenterPopupWidget::HANDLEDBYUSER;
                    m_closeActionKey = buttonAction;
                    emit clickedActionButton(m_currentNotifyInfo->id().toInt(), this->getCloseActionKey());
                    this->hide();
                    this->deleteLater();
                });

                m_pButtonWidgetLayout->addWidget(button, Qt::AlignRight);
                QString formatBody = setButtonStringBody(buttonText, button);
                button->setText(formatBody);
                button->setToolTip(buttonText);
                connect(m_pThemeGsetting, &QGSettings::changed, this, [=](QString key) {
                    if (key==STYLE_NAME || key==SYSTEM_FONT_SIZE || key==SYSTEM_FONT) {
                        button->setText(setButtonStringBody(buttonText, button));
                    }
                });
            }
        }
    }
}

uint CenterPopupWidget::getCloseReason()
{
    return m_closeReason;
}

QString CenterPopupWidget::getCloseActionKey()
{
    return m_closeActionKey;
}

/* 重新绘制背景色和窗口阴影 */
void CenterPopupWidget::paintEvent(QPaintEvent *event)
{
    //绘制背景
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setBrush(opt.palette.color(QPalette::Active, QPalette::Base));
    p.setOpacity(1);
    p.setPen(Qt::NoPen);
    QPainterPath path;
    QRect mainRect(0, 0, this->width(), this->height());
    path.addRoundedRect(mainRect, MASTER::centerWidgetRadius, MASTER::centerWidgetRadius);
    p.setRenderHint(QPainter::Antialiasing); //反锯齿
    p.drawRoundedRect(mainRect, MASTER::centerWidgetRadius, MASTER::centerWidgetRadius);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    //绘制阴影
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true); //渲染，抗锯齿
    QColor color = this->palette().shadow().color();
    QPainterPath shadowPath;
    shadowPath.setFillRule(Qt::WindingFill);
    shadowPath.addRoundedRect(0, 0,this->width(),this->height(),
                              MASTER::centerWidgetRadius, MASTER::centerWidgetRadius);
    painter.setPen(color);
    painter.drawPath(shadowPath);

    QWidget::paintEvent(event);
}

