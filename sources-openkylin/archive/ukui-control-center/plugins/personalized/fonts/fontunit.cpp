/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <QAction>
#include <QMenu>
#include <QPainter>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#include <QGSettings>
#include <QStandardPaths>
#include <QPainterPath>

#include "fontunit.h"
#include "ukcccommon.h"
#include <kysdk/applications/accessinfohelper.h>
using namespace ukcc;

#define STYLE_FONT_SCHEMA  "org.ukui.style"
#define SYSTEM_FONT_EKY "system-font-size"
#define APPLY_FONT_KEY "system-font"
#define FONT_STYLE_KEY "systemFontStyle"
#define FONT_STYLE "system-font-style"

#define KYLIN_GTK_GSTTINGS "org.mate.interface"

FontUnit::FontUnit(const ViewData* viewData, QString installPath, QWidget *parent)
{
    m_applyFont = false;
    m_installPath = installPath;
    m_viewData = new ViewData();
    m_viewData->family = viewData->family;
    m_viewData->name = viewData->name;
    m_viewData->path = viewData->path;
    m_viewData->style = viewData->style;
    m_viewData->pathList = viewData->pathList;
    m_viewData->styleList = viewData->styleList;

    m_fontInterface = new QDBusInterface("org.ukui.ukcc.session",
                                         "/Font",
                                         "org.ukui.ukcc.session.Font",
                                         QDBusConnection::sessionBus(), this);
    if (!m_fontInterface->isValid()) {
        qCritical() << "org.ukui.ukcc.session.Font DBus error:" << m_fontInterface->lastError();
    }

    const QByteArray styleID(STYLE_FONT_SCHEMA);
    m_styleSettings = new QGSettings(styleID, QByteArray(), this);
    if (!m_styleSettings) {
        qCritical() << "org.ukui.style init failed";
    }

    const QByteArray gtkID(KYLIN_GTK_GSTTINGS);
    m_gtkSettings = new QGSettings(gtkID, QByteArray(), this);
    if (!m_gtkSettings) {
        qCritical() << "org.mate.interface init failed";
    }

    setWidgetUi();
    setWidgetLayout();
    setupConnect();
    setWidgetContent();
}

FontUnit::~FontUnit()
{
    m_applyFont = false;
    m_installPath.clear();
    if (m_viewData) {
        delete m_viewData;
        m_viewData = nullptr;
    }
}

bool FontUnit::isUsingFont()
{
    QString fontName = m_styleSettings->get(APPLY_FONT_KEY).toString();
    QString fontStyle;
    if (m_styleSettings->keys().contains(FONT_STYLE_KEY)) {
        fontStyle = m_styleSettings->get(FONT_STYLE).toString();
    } else {
        qDebug() << Q_FUNC_INFO << "org.ukui.style not contains key systemFontStyle";
    }
    if (m_viewData->family == "C059" || m_viewData->family == "Z003") {
        if (fontName.contains(m_viewData->family) && (fontStyle != m_viewData->style)) {
            if (m_viewData->styleList.contains(fontStyle)) {
                m_viewData->style = fontStyle;
                int index = m_viewData->style.indexOf(fontStyle);
                m_viewData->path = m_viewData->pathList.at(index);
                return true;
            }
        } else if (fontName.contains(m_viewData->family) && (fontStyle == m_viewData->style)) {
            return true;
        }
        return false;
    } else {
        if (fontName == m_viewData->family && (fontStyle != m_viewData->style)) {
            if (m_viewData->styleList.contains(fontStyle)) {
                m_viewData->style = fontStyle;
                int index = m_viewData->styleList.indexOf(fontStyle);
                m_viewData->path = m_viewData->pathList.at(index);
                return true;
            }
        } else if (fontName == m_viewData->family && (fontStyle == m_viewData->style)) {
            return true;
        }
        return false;
    }
}

void FontUnit::setWidgetUi()
{
    setContentsMargins(0, 0, 0, 0);
    setFixedSize(197, 156);
    setWindowFlag(Qt::Tool);
    setWindowModality(Qt::WindowModal);
    setWindowModality(Qt::ApplicationModal);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    setAttribute(Qt::WA_DeleteOnClose, true);
    if (UkccCommon::isV11()) {
        m_fontPreviewLabel = new QLabel(tr("Build a Chinese brand of world operating systems"), this);
    } else {
        m_fontPreviewLabel = new QLabel(tr("Open source collaboration, creating a better future together"), this);
    }

    m_fontPreviewLabel->setWordWrap(true);
    KDK_EXTEND_ALL_INFO_FORMAT(m_fontPreviewLabel, "Fonts", "", "font of" + m_viewData->family);
    m_fontPreviewLabel->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_fontPreviewLabel->setFixedSize(176, 68);

    m_fontNameLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_fontNameLabel, "Fonts", "", "font of" + m_viewData->family);
    // family 是会话服务从 Fontconfig 获取的首选本地化家族名。
    m_fontNameLabel->setText(m_viewData->family);
    m_fontNameLabel->setWordWrap(false);
    // fontName
    QFont nameFont = QFont();
    nameFont.setFamily("Noto Sans CJK SC");
    nameFont.setWeight(QFont::Weight::Normal);
    nameFont.setPixelSize(qApp->font().pointSize() + 4);
    m_fontNameLabel->setFont(nameFont);

    m_fontStyleLabel = new KLabel(this);
    m_fontStyleLabel->setText(QString(tr("%1 font style")).arg(m_viewData->styleList.size()));
    KDK_EXTEND_ALL_INFO_FORMAT(m_fontStyleLabel, "Fonts", "", "font of" + m_viewData->family);
    // fontStyle
    nameFont.setPixelSize(qApp->font().pointSize() + 2);
    m_fontStyleLabel->setFont(nameFont);
    m_fontStyleLabel->setFontColor(Qt::gray);

    m_fontToolButton = new QToolButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_fontToolButton, "Fonts", "", "font of" + m_viewData->family);
    m_fontToolButton->setAutoRaise(true);
    m_fontToolButton->setPopupMode(QToolButton::InstantPopup);

    m_applyFontAction = new QAction(tr("Apply Font"), m_fontToolButton);
    KDK_OBJ_INFO_FORMAT(m_applyFontAction);
    m_removeFontAction = new QAction(tr("Remove Font"), m_fontToolButton);
    KDK_OBJ_INFO_FORMAT(m_removeFontAction);
    m_checkFontAction = new QAction(tr("Check Font"), m_fontToolButton);
    KDK_OBJ_INFO_FORMAT(m_checkFontAction);

    m_fontMenu = new QMenu(m_fontToolButton);
    KDK_EXTEND_ALL_INFO_FORMAT(m_fontMenu, "Fonts", "", "font of" + m_viewData->family);
    // 覆盖 KDK 英文物件名，避免读屏读 menu 内部名
    m_fontMenu->setAccessibleName(QStringLiteral(" "));
    m_fontMenu->addAction(m_applyFontAction);
    m_fontMenu->addAction(m_checkFontAction);
    m_fontMenu->addAction(m_removeFontAction);

    m_fontToolButton->setMenu(m_fontMenu);
    m_fontToolButton->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
    m_fontToolButton->setProperty("useButtonPalette", true);
    m_fontToolButton->setFixedSize(36, 36);

    // 字体卡片工具按钮需覆盖 KDK 设置的 accessibleName（须进 zh_CN 翻译，否则读屏仍报英文）
    m_fontToolButton->setAccessibleName(
        tr("%1 %2").arg(m_viewData->family, tr("More actions")));
    m_fontToolButton->setToolTip(tr("More actions"));
}

void FontUnit::setWidgetLayout()
{
    m_leftVerLayout = new QVBoxLayout();
    m_leftVerLayout->setSpacing(0);
    m_leftVerLayout->addWidget(m_fontNameLabel);
    m_leftVerLayout->addWidget(m_fontStyleLabel);

    m_bottomHorLayout = new QHBoxLayout();
    m_bottomHorLayout->setSpacing(8);
    m_bottomHorLayout->addLayout(m_leftVerLayout);
    m_bottomHorLayout->addWidget(m_fontToolButton);

    m_baseVerLayout = new QVBoxLayout(this);
    m_baseVerLayout->addWidget(m_fontPreviewLabel);
    m_baseVerLayout->addStretch();
    m_baseVerLayout->addLayout(m_bottomHorLayout);
}

void FontUnit::setWidgetContent()
{
    m_applyFont = isUsingFont();
    if (!m_viewData->path.contains(m_installPath)) {
        m_removeFontAction->setEnabled(false);
    }

    if (m_applyFont) {
        m_applyFontAction->setEnabled(false);
        m_removeFontAction->setEnabled(false);
    } else {
        if (m_viewData->path.contains(m_installPath)) {
            m_removeFontAction->setEnabled(true);
        }
        m_applyFontAction->setEnabled(true);
    }

    // fontPreview
    QFont textFont = QFont();
    textFont.setFamily(m_viewData->family);
    textFont.setPixelSize(20);
    textFont.setStyleName(m_viewData->style);
    m_fontPreviewLabel->setFont(textFont);
    QFontMetrics fontMetrics(m_fontPreviewLabel->font());
    QString previewText = m_fontPreviewLabel->text();
    int fontWidth = fontMetrics.width(previewText);

    static const int PREVIEW_WIDTH = 170;
    static const int PREVIEW_HEIGHT = 63;
    static const int SPACING = 32;
    int fontLine = fontWidth / PREVIEW_WIDTH;
    if (fontWidth % PREVIEW_WIDTH != 0) {
        fontLine = fontLine + 1;
    }
    int fontHeight = fontMetrics.height();
    QString language = QLocale::system().name();
    if (language != "zh_CN") {
        if (fontLine >= 2 && (2 * fontHeight > PREVIEW_HEIGHT)) {
            m_fontPreviewLabel->setText(fontMetrics.elidedText(previewText, Qt::ElideRight, PREVIEW_WIDTH));
            m_fontPreviewLabel->setToolTip(previewText);
        }

        if (fontLine >= 3 && (3 * fontHeight > PREVIEW_HEIGHT)) {
            m_fontPreviewLabel->setText(fontMetrics.elidedText(previewText, Qt::ElideRight, (PREVIEW_WIDTH * 2 - SPACING)));
            m_fontPreviewLabel->setToolTip(previewText);
        }

        if (fontLine >= 4) {
            m_fontPreviewLabel->setText(fontMetrics.elidedText(previewText, Qt::ElideRight, (PREVIEW_WIDTH * 3)));
            m_fontPreviewLabel->setToolTip(previewText);
        }
    }
}

void FontUnit::setupConnect()
{
    connect(m_styleSettings , &QGSettings::changed, this, [=](const QString &key) {
        styleSettingChangeSlot();
    });

    connect(m_applyFontAction, &QAction::triggered, this, &FontUnit::applyFontSlot);
    connect(m_removeFontAction, &QAction::triggered, this, &FontUnit::removeFontSlot);
    connect(m_checkFontAction, &QAction::triggered, this, &FontUnit::checkFontSlot);
}

void FontUnit::styleSettingChangeSlot()
{
    m_applyFont = isUsingFont();
    this->update();
}

void FontUnit::applyFontSlot()
{
    UkccCommon::buriedSettings("Fonts", "apply font", QString("clicked"), m_viewData->name);
    qDebug() << Q_FUNC_INFO << "apply font" << m_viewData->family << m_viewData->path;
    m_fontInterface->call("applyFont", m_viewData->family, m_viewData->path, m_viewData->style);
    m_applyFont = true;
    this->update();
}

void FontUnit::removeFontSlot()
{
    UkccCommon::buriedSettings("Fonts", "remove font", QString("clicked"), m_viewData->name);
    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Warning);
    msg.addButton(tr("Cancle"), QMessageBox::RejectRole);
    msg.addButton(tr("Uninstall"), QMessageBox::AcceptRole);
    msg.setText(tr("Are you sure to uninstall this font"));
    msg.setInformativeText(tr("If uninstall this font, some fonts may not display as expected."));
    int ret = msg.exec();
    if (ret) {
        m_fontInterface->call("uninstallFont", m_viewData->pathList, m_viewData->name);
        emit remoteFontSig(m_viewData->pathList, m_viewData->name);
        qDebug() << Q_FUNC_INFO << m_viewData->path << "need uninstall" << m_viewData->family;
        this->close();
        this->deleteLater();
    } else {
        qDebug() << Q_FUNC_INFO << "uninstall font cancel";
    }
}

void FontUnit::fontStyleChangedSlot(QString style, QString path) {
    m_viewData->path = path;
    m_viewData->style = style;
    QFont textFont = m_fontPreviewLabel->font();
    textFont.setStyleName(style);
    m_fontPreviewLabel->setFont(textFont);

    qDebug() << Q_FUNC_INFO << "apply font" << m_viewData->family << m_viewData->path << m_viewData->style;
    m_fontInterface->call("applyFont", m_viewData->family, m_viewData->path, m_viewData->style);
    m_applyFont = true;
    this->update();
}

void FontUnit::fontStyleRemovedSlot(QString path, QString newStyle, QString newPath) {
    m_viewData->path = newPath;
    m_viewData->style = newStyle;
    QFont textFont = m_fontPreviewLabel->font();
    textFont.setStyleName(newStyle);
    m_fontPreviewLabel->setFont(textFont);
    m_fontStyleLabel->setText(QString(tr("%1 font style")).arg(m_viewData->styleList.size()));
    this->update();

    QStringList pathList = {path};
    QString fontName = m_viewData->name;
    qDebug() << Q_FUNC_INFO << "fontStyleRemovedSlot" << fontName << pathList << m_viewData->pathList << (pathList.size() == m_viewData->pathList.size());
    emit remoteFontSig(pathList, fontName);
    if (m_viewData->pathList.size() == 1 && path == m_viewData->pathList.at(0)) {
        qDebug() << Q_FUNC_INFO << "need uninstall fontUnit" << fontName;
        this->close();
        this->deleteLater();
    }
}

void FontUnit::checkFontSlot()
{
    UkccCommon::buriedSettings("Fonts", "check font info", QString("clicked"), m_viewData->name);
    m_popupFontInfo = new PopupFontInfo(m_viewData, this);
    if (m_popupFontInfo) {
        m_popupFontInfo->show();
        qDebug() << Q_FUNC_INFO << "viewData" <<  m_viewData->name <<  m_viewData->style <<  m_viewData->path;
        connect(m_popupFontInfo, &PopupFontInfo::fontStyleChanged, this, [=](QString style, QString path){
            fontStyleChangedSlot(style, path);
        });
        connect(m_popupFontInfo, &PopupFontInfo::fontStyleRemoved, this, [=](QString path, QString newStyle, QString newPath){
            fontStyleRemovedSlot(path, newStyle, newPath);
        });
    }
}

void FontUnit::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainterPath path;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int radius = qApp->property("kradius-normal").toInt();

    // 应用字体边框颜色
    if (m_applyFont) {
        QColor highlightColor = palette().color(QPalette::Highlight);
        QRectF borderRect = rect().adjusted(2, 2, -2, -2);
        path.addRoundedRect(borderRect, radius, radius);
        painter.setClipPath(path);
        painter.setClipping(false);
        QPen pen(highlightColor, 4);
        painter.setPen(pen);
        painter.drawPath(path);
        m_applyFontAction->setEnabled(false);
        m_removeFontAction->setEnabled(false);
    } else {
        if (m_viewData && m_viewData->path.contains(m_installPath)) {
            m_removeFontAction->setEnabled(true);
        }
        m_applyFontAction->setEnabled(true);
    }

    // 设置填充的画刷（可选）
    painter.setClipping(true);
    painter.setPen(Qt::transparent);
    path.addRoundedRect(this->rect(), radius, radius);
    path.setFillRule(Qt::WindingFill);

    // 鼠标悬浮
    if (m_isMouseOpt || m_viewData->path.contains(m_installPath)) {
        if (qApp->property("kcontain-hover").canConvert<QBrush>()) {
            QBrush containColor = qApp->property("kcontain-hover").value<QBrush>();
            painter.setBrush(containColor);
        }
    } else {
        // 默认底色
        if (qApp->property("kcontain-secondary-normal").canConvert<QBrush>()) {
            QBrush containColor = qApp->property("kcontain-secondary-normal").value<QBrush>();
            painter.setBrush(containColor);
        }
    }

    // 绘制圆角矩形
    painter.drawPath(path);
}

ViewData* FontUnit::getViewData()
{
    return m_viewData;
}

void FontUnit::updateViewData(ViewData* viewData) {
    m_viewData->family = viewData->family;
    m_viewData->name = viewData->name;
    m_viewData->path = viewData->path;
    m_viewData->style = viewData->style;
    m_viewData->pathList = viewData->pathList;
    m_viewData->styleList = viewData->styleList;
    QFont textFont = m_fontPreviewLabel->font();
    textFont.setStyleName(m_viewData->style);
    m_fontPreviewLabel->setFont(textFont);
    m_fontStyleLabel->setText(QString(tr("%1 font style")).arg(m_viewData->styleList.size()));
    qDebug() << Q_FUNC_INFO << "updateViewData style size" << m_viewData->styleList.size() << viewData->styleList.size() << m_viewData->style << m_viewData->path;
}


void FontUnit::setInstallFlag(bool flag)
{
    m_installFlag = flag;
    this->repaint();
}

void FontUnit::enterEvent(QEvent *event)
{
    m_isMouseOpt = true;
    this->repaint();
}

void FontUnit::leaveEvent(QEvent *event)
{
    m_isMouseOpt = false;
    this->update();
}
