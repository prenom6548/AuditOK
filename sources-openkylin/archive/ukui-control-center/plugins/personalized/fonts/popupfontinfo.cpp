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
#include <QLocale>
#include <QDebug>
#include <gsettingmonitor.h>
#include <usermanual.h>
#include <QPainterPath>
#include <QSizePolicy>
#include "popupfontinfo.h"
#include "../interface/ukcccommon.h"
#include <kysdk/applications/klineframe.h>
#include <QStandardPaths>
#include <QFontDatabase>

#define STYLE_FONT_SCHEMA  "org.ukui.style"
#define SYSTEM_FONT_EKY "system-font-size"
#define APPLY_FONT_KEY "system-font"
#define FONT_STYLE         "system-font-style"
#define FONT_STYLE_KEY     "systemFontStyle"

using namespace ukcc;

PopupFontInfo::PopupFontInfo(ViewData* viewData, QWidget *parent) : KDialog(parent)
{
    qRegisterMetaType<FontInformation>("FontInformation");
    qDBusRegisterMetaType<FontInformation>();
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

    m_pathList = viewData->pathList;
    m_styleList = viewData->styleList;
    m_viewData = viewData;
    QString fontPath = viewData->path;
    qDebug() << Q_FUNC_INFO << "PopupFontInfo" << viewData->style << viewData->path << viewData->name;
    QDBusReply<FontInformation> ret =  m_fontInterface->call("getFontDetails", fontPath);
    if (ret.isValid()) {
        m_font = ret.value();
    }
    setWidegtUi();
    setWidLayout();
    initConnect();
}

PopupFontInfo::~PopupFontInfo()
{
    m_pathList.clear();
    m_styleList.clear();
}

void PopupFontInfo::setWidegtUi()
{
    const int valueWidth = 288;
    const int labelwidth = 80;
    this->setFixedSize(420, 523);
    setWindowFlag(Qt::Tool);
    setWindowModality(Qt::WindowModal);
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_DeleteOnClose,true);
    setWindowIcon("ukui-control-center");
    setWindowTitle(m_font.name);

    m_fontName = new KLabel(this);
    m_nameValue = new KLabel(this);
    m_fontStyle = new KLabel(this);
    m_styleValue = new KLabel(this);
    m_fontType = new KLabel(this);
    m_typeValue = new KLabel(this);
    m_fontVersion = new KLabel(this);
    m_versionValue = new KLabel(this);
    m_fontPath = new KLabel(this);
    m_pathValue = new KLabel(this);
    m_fontCopyright = new KLabel(this);
    m_copyrightValue = new KLabel(this);
    m_fontTrademark = new KLabel(this);
    m_trademarkValue = new KLabel(this);

    m_stylelabel = new KLabel(this);
    m_stylelabel->setFixedWidth(labelwidth);
    m_stylelabel->setText(tr("FontStyle"));
    m_stylelabel->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    m_styleCombBox = new QComboBox(this);
    m_styleCombBox->setFixedSize(valueWidth, 36);
    m_styleCombBox->clear();
    for (const auto& style : m_styleList) {
        m_styleCombBox->addItem(style);
        if (style == m_viewData->style) {
            m_styleCombBox->setCurrentText(style);
        }
    }
    m_stylelabel->setBuddy(m_styleCombBox);

    m_fontName->setText(tr("FullName"));
    m_fontName->setFixedSize(labelwidth, 36);

    m_nameValue->setFixedSize(valueWidth, 36);
    m_nameValue->setFontColor(Qt::gray);
    m_nameValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    // 与卡片标题一致，使用本地化家族名和当前样式组成全称。
    m_nameValue->setText(QString("%1 %2").arg(m_viewData->family, m_viewData->style));
    m_nameValue->setCursor(Qt::IBeamCursor);

    m_removeFontLabel = new KLabel(this);
    m_removeFontLabel->setFixedWidth(labelwidth);
    m_removeFontButton = new KPushButton(this);
    m_removeFontButton->setText(tr("Remove Font"));
    m_removeFontButton->setFixedSize(88, 36);

    m_fontStyle->setText(tr("FontStyle"));
    m_fontStyle->setFixedSize(labelwidth, 36);

    m_styleValue->setFontColor(Qt::gray);
    m_styleValue->setFixedSize(valueWidth, 36);
    m_styleValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_styleValue->setText(m_viewData->style);
    m_styleValue->setCursor(Qt::IBeamCursor);

    m_fontType->setText(tr("FontType"));
    m_fontType->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_fontType->setFixedSize(labelwidth, 36);

    m_typeValue->setFontColor(Qt::gray);
    m_typeValue->setFixedSize(valueWidth, 36);
    m_typeValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_typeValue->setText(m_font.type);
    m_typeValue->setCursor(Qt::IBeamCursor);

    m_fontVersion->setText(tr("FontVersion"));
    m_fontVersion->setFixedSize(labelwidth, 36);

    m_versionValue->setFontColor(Qt::gray);
    m_versionValue->setText(m_font.version);
    m_versionValue->setFixedSize(valueWidth, 36);
    m_versionValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_versionValue->setCursor(Qt::IBeamCursor);

    m_fontPath->setText(tr("FontPath"));
    m_fontPath->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_fontPath->setFixedWidth(labelwidth);

    m_pathValue->setFontColor(Qt::gray);
    m_pathValue->setText(m_viewData->path);
    m_pathValue->setFixedWidth(valueWidth);
    m_pathValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_pathValue->adjustSize();
    m_pathValue->setWordWrap(true);
    m_pathValue->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_pathValue->setCursor(Qt::IBeamCursor);

    m_fontCopyright->setText(tr("FontCopyright"));
    m_fontCopyright->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_fontCopyright->setFixedWidth(labelwidth);

    m_copyrightValue->setFontColor(Qt::gray);
    m_copyrightValue->setText(m_font.copyright);
    m_copyrightValue->setFixedWidth(valueWidth);
    m_copyrightValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_copyrightValue->adjustSize();
    m_copyrightValue->setWordWrap(true);
    m_copyrightValue->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_copyrightValue->setCursor(Qt::IBeamCursor);

    m_fontTrademark->setText(tr("FontTrademark"));
    m_fontTrademark->setFixedSize(labelwidth, 36);

    m_trademarkValue->setFontColor(Qt::gray);
    m_trademarkValue->setText(m_font.manufacturer);
    m_trademarkValue->setFixedSize(valueWidth, 36);
    m_trademarkValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_trademarkValue->setCursor(Qt::IBeamCursor);

    m_continueBtn = new KPushButton();
    m_continueBtn->setFixedHeight(36);
    m_continueBtn->setText(tr("Confirm"));
    m_continueBtn->setProperty("isImportant", true);

    m_cancelBtn = new KPushButton();
    m_cancelBtn->setText(tr("Cancel"));
    m_cancelBtn->setFixedHeight(36);
}

void PopupFontInfo::clickRemoveFontButtonClick() {
    UkccCommon::buriedSettings("Fonts", "check fontInfo remove", QString("clicked"), m_viewData->name);
    QString style = m_styleCombBox->currentText();
    int index = m_styleCombBox->currentIndex();
    qDebug() << Q_FUNC_INFO << "style index" << index << "style" << style << m_viewData->pathList.size() << m_viewData->path << m_viewData->pathList;
    if (index >= 0 && index < m_pathList.size()) {
        QMessageBox msg(this);
        msg.setIcon(QMessageBox::Warning);
        msg.addButton(tr("Cancle"), QMessageBox::RejectRole);
        msg.addButton(tr("Uninstall"), QMessageBox::AcceptRole);
        msg.setText(tr("Are you sure to uninstall this font"));
        msg.setInformativeText(tr("If uninstall this font, some fonts may not display as expected."));
        int ret = msg.exec();
        if (ret) {
            QString path = m_pathList.at(index);
            QStringList pathList = {path};
            qDebug() << Q_FUNC_INFO << "uninstallFont path" << path;
            m_fontInterface->call("uninstallFont", pathList, m_viewData->name);
            if (m_styleCombBox->count() == 1) {
                emit fontStyleRemoved(path, "", "");
                this->close();
            } else {
                m_styleCombBox->removeItem(index);
                m_styleCombBox->setCurrentIndex(0);
                m_viewData->pathList.removeAll(path);
                m_viewData->styleList.removeAll(style);
                m_pathList.removeAll(path);
                m_styleList.removeAll(style);
                if (m_viewData->styleList.size() <= 1) {
                    m_previewGroup->setVisible(false);
                }
                QString newStyle = m_styleCombBox->currentText();
                int curIndex = m_styleCombBox->currentIndex();
                qDebug() << Q_FUNC_INFO << "remove path" << curIndex << newStyle << m_viewData->pathList.size() << path;
                m_viewData->path = m_viewData->pathList.at(curIndex);
                m_viewData->style = newStyle;
                qDebug() << Q_FUNC_INFO << "clickRemoveFontButtonClick curIndex" << curIndex << "path" << m_viewData->pathList.at(curIndex) << "style" << newStyle;
                emit fontStyleRemoved(path, newStyle, m_viewData->path);
            }
        } else {
            qDebug() << Q_FUNC_INFO << "uninstall font cancel";
        }
    }
}

void PopupFontInfo::slotCancelClick() {
    UkccCommon::buriedSettings("Fonts", "check fontInfo cancel", QString("clicked"), m_viewData->name);
    this->close();
}

void PopupFontInfo::styleCombBoxChangedSlot(int index) {
    UkccCommon::buriedSettings("Fonts", "check fontInfo combox changed", QString("clicked"), m_viewData->name);
    qDebug() << Q_FUNC_INFO << "style index" << index;
    QString style = m_styleCombBox->currentText();
    QFont textFont = m_letterLabel->font();
    textFont.setStyleName(style);
    m_letterLabel->setFont(textFont);
    m_numberLabel->setFont(textFont);
    m_wordLabel->setFont(textFont);

    if (index >= 0 && index < m_pathList.size()) {
        m_pathValue->setText(m_pathList.at(index));
    }
    m_styleValue->setText(style);
    m_nameValue->setText(QString("%1 %2").arg(m_viewData->family, style));
    qDebug() << Q_FUNC_INFO << "styleCombBoxChangedSlot index" << index << "path" << m_pathList.at(index) << "style" << style;
    QString installPath = QStandardPaths::writableLocation(QStandardPaths::FontsLocation) + "/";
    if (!m_viewData->path.contains(installPath) || isUsingFont(m_viewData->family, style)) {
        m_removeFontButton->setEnabled(false);
    } else {
        m_removeFontButton->setEnabled(true);
    }
}

void PopupFontInfo::initConnect() {
    connect(m_continueBtn, &QPushButton::clicked, this, &PopupFontInfo::slotContinueClick);

    connect(m_cancelBtn, &QPushButton::clicked, this, &PopupFontInfo::slotCancelClick);

    connect(m_removeFontButton, &KPushButton::clicked, this, &PopupFontInfo::clickRemoveFontButtonClick);

    connect(m_styleCombBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        styleCombBoxChangedSlot(index);
    });
}

void PopupFontInfo::setWidLayout()
{

    UkccFrame* fontInfoFrame = new UkccFrame(this);
    fontInfoFrame->setFixedWidth(376);
    QVBoxLayout* infoVLayout = new QVBoxLayout(fontInfoFrame);
    infoVLayout->setContentsMargins(0, 0, 0, 8);
    fontInfoFrame->setLayout(infoVLayout);

    // 样式
    QHBoxLayout* styleHLayout = new QHBoxLayout(fontInfoFrame);
    styleHLayout->addWidget(m_stylelabel);
    styleHLayout->addWidget(m_styleCombBox);

    // 预览图
    QWidget* previewWidget = new QWidget(this);
    previewWidget->setFixedSize(376, 160);

    QFont textFont = QFont();
    textFont.setFamily(m_viewData->family);
    textFont.setPixelSize(qApp->font().pointSize() + 10);
    QString style = m_styleCombBox->currentText();
    textFont.setStyleName(style);

    // fontStyle
    m_letterLabel = new KLabel(previewWidget);
    m_letterLabel->setFont(textFont);
    m_letterLabel->setAlignment(Qt::AlignCenter);
    m_letterLabel->setText("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    m_numberLabel = new KLabel(previewWidget);
    m_numberLabel->setFont(textFont);
    m_numberLabel->setText("123456789");
    m_numberLabel->setAlignment(Qt::AlignCenter);

    m_wordLabel = new KLabel(previewWidget);
    m_wordLabel->setFont(textFont);
    m_wordLabel->setText(tr("Build a Chinese brand of world operating systems"));
    m_wordLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout* previewVLayout = new QVBoxLayout(previewWidget);
    previewVLayout->setContentsMargins(16, 24, 16, 24);
    previewVLayout->addWidget(m_letterLabel);
    previewVLayout->addWidget(m_numberLabel);
    previewVLayout->addWidget(m_wordLabel);

    m_previewGroup = new KBackgroundGroup(fontInfoFrame);
    m_previewGroup->setBorderVisible(true);
    m_previewGroup->addWidget(previewWidget);

    // 全称
    QHBoxLayout* nameHLayout = new QHBoxLayout();
    nameHLayout->addWidget(m_fontName, 0, Qt::AlignVCenter);
    nameHLayout->addWidget(m_nameValue, 1, Qt::AlignVCenter);

    // 版本
    QHBoxLayout* versionHLayout = new QHBoxLayout();
    versionHLayout->addWidget(m_fontVersion, 0, Qt::AlignVCenter);
    versionHLayout->addWidget(m_versionValue, 1, Qt::AlignVCenter);

    // 位置
    QHBoxLayout* pathHLayout = new QHBoxLayout(fontInfoFrame);
    pathHLayout->addWidget(m_fontPath);
    pathHLayout->addWidget(m_pathValue);

    // 卸载
    QHBoxLayout* removeHLayout = new QHBoxLayout(fontInfoFrame);
    removeHLayout->addWidget(m_removeFontLabel);
    removeHLayout->addWidget(m_removeFontButton);
    removeHLayout->addStretch(1);

    // 样式
    QHBoxLayout* fontStyleHLayout = new QHBoxLayout(fontInfoFrame);
    fontStyleHLayout->addWidget(m_fontStyle, 0, Qt::AlignVCenter);
    fontStyleHLayout->addWidget(m_styleValue, 1, Qt::AlignVCenter);

    // 种类
    QHBoxLayout* typeHLayout = new QHBoxLayout(fontInfoFrame);
    typeHLayout->addWidget(m_fontType, 0, Qt::AlignVCenter);
    typeHLayout->addWidget(m_typeValue, 1, Qt::AlignVCenter);

    // 版权
    QHBoxLayout* copyrightHLayout = new QHBoxLayout(fontInfoFrame);
    copyrightHLayout->addWidget(m_fontCopyright);
    copyrightHLayout->addWidget(m_copyrightValue);

    // 商标
    QHBoxLayout* trademarkHLayout = new QHBoxLayout(fontInfoFrame);
    trademarkHLayout->addWidget(m_fontTrademark, 0, Qt::AlignVCenter);
    trademarkHLayout->addWidget(m_trademarkValue, 1, Qt::AlignVCenter);

    infoVLayout->addLayout(styleHLayout);
    infoVLayout->addWidget(m_previewGroup);
    infoVLayout->addLayout(nameHLayout);
    infoVLayout->addLayout(versionHLayout);
    infoVLayout->addLayout(pathHLayout);
    infoVLayout->addLayout(removeHLayout);
    infoVLayout->addLayout(fontStyleHLayout);
    infoVLayout->addLayout(typeHLayout);
    infoVLayout->addLayout(copyrightHLayout);
    infoVLayout->addLayout(trademarkHLayout);

    if (m_styleList.size() <= 1) {
        m_previewGroup->setVisible(false);
    }

    if (m_font.type.isEmpty()) {
        m_fontType->hide();
        m_typeValue->hide();
    }

    if (m_font.version.isEmpty()) {
        m_fontVersion->hide();
        m_versionValue->hide();
    }

    if (m_font.copyright.isEmpty()) {
        m_fontCopyright->hide();
        m_copyrightValue->hide();
    }

    if (m_font.manufacturer.isEmpty()) {
        m_fontTrademark->hide();
        m_trademarkValue->hide();
    }

    QString installPath = QStandardPaths::writableLocation(QStandardPaths::FontsLocation) + "/";
    qDebug() << Q_FUNC_INFO << "intallPath" << installPath << m_viewData->path;
    if (!m_viewData->path.contains(installPath) || isUsingFont(m_viewData->family, m_viewData->style)) {
        m_removeFontButton->setEnabled(false);
    }

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidget(fontInfoFrame);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setFocusPolicy(Qt::NoFocus);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_scrollArea->verticalScrollBar()->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout* lineLayout = new QHBoxLayout(this);
    lineLayout->setContentsMargins(0, 0, 0, 16);
    KHLineFrame *lineFrame = new KHLineFrame(this);
    lineLayout->addWidget(lineFrame);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelBtn);
    buttonLayout->addSpacing(16);
    buttonLayout->addWidget(m_continueBtn);
    buttonLayout->addSpacing(24);
    buttonLayout->setContentsMargins(0, 0, 0, 16);

    /* 按钮和组件纵向布局 */
    m_vLayout = new QVBoxLayout();
    m_vLayout->addWidget(m_scrollArea);
    m_vLayout->setContentsMargins(24, 0, 0, 0);

    /* 整个窗体布局 */
    m_vMainLayout = new QVBoxLayout();
    m_vMainLayout->setSpacing(0);
    m_vMainLayout->addSpacing(15);
    m_vMainLayout->addLayout(m_vLayout);
    m_vMainLayout->addLayout(lineLayout);
    m_vMainLayout->addLayout(buttonLayout);
    m_vMainLayout->setAlignment(buttonLayout, Qt::AlignRight);
    m_vMainLayout->setContentsMargins(0, 0, 0, 0);

    this->mainWidget()->setLayout(m_vMainLayout);
    UkccCommon::centerToScreen(this);
}

bool PopupFontInfo::isUsingFont(QString family, QString style)
{
    QString fontName = m_styleSettings->get(APPLY_FONT_KEY).toString();
    QString fontStyle;
    if (m_styleSettings->keys().contains(FONT_STYLE_KEY)) {
        fontStyle = m_styleSettings->get(FONT_STYLE).toString();
    } else {
        qDebug() << Q_FUNC_INFO << "org.ukui.style not contains key systemFontStyle";
    }
    if (family == "C059" || family == "Z003") {
        return fontName.contains(family) && (fontStyle == style);
    } else {
        return fontName == family && (fontStyle == style);
    }
}

void PopupFontInfo::slotContinueClick() {
    UkccCommon::buriedSettings("Fonts", "check fontInfo apply", QString("clicked"), m_viewData->name);
    QString style = m_styleCombBox->currentText();
    int index = m_styleCombBox->currentIndex();
    m_viewData->path = m_viewData->pathList.at(index);
    m_viewData->style = m_viewData->styleList.at(index);
    qDebug() << Q_FUNC_INFO << "style index" << index << "style" << style << "path" << m_viewData->path;
    if (index >= 0 && index < m_pathList.size()) {
        emit fontStyleChanged(style, m_pathList.at(index));
    }
    this->window()->close();
}
