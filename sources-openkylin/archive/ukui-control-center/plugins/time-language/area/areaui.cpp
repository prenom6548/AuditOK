/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#include "areaui.h"
#include <QEvent>
#include <QIcon>
#include <QMouseEvent>
#include <QTimer>
#include <kysdk/applications/accessinfohelper.h>
#include <kysdk/applications/themeController.h>
#include "fixlabel.h"

#define THEME_QT_SCHEMA "org.ukui.style"
#include <QGSettings/QGSettings>

AreaUi::AreaUi(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    setAttribute(Qt::WA_DeleteOnClose);
    uiLayout       = new QVBoxLayout(this);
    formatLayout   = new QVBoxLayout();
    LanguageLayout = new QVBoxLayout();
    uiLayout->addLayout(formatLayout);
    uiLayout->addLayout(LanguageLayout);
    uiLayout->addStretch();
    uiLayout->setSpacing(40);
    uiLayout->setMargin(0);
    formatLayout->setSpacing(8);
    LanguageLayout->setSpacing(1);
    formatLayout->setMargin(0);
    LanguageLayout->setMargin(0);
    initUi();
}

AreaUi::~AreaUi()
{

}

void AreaUi::initUi()
{
    initLanguageFormat();
    initSystemLanguage();
}

void AreaUi::initLanguageFormat()
{
    formatTitleLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(formatTitleLabel, "Area", "", "");
    formatTitleLabel->setContentsMargins(16, 0, 0, 0);
    formatGroup      = new SettingGroup(this);
    //~ contents_path /Area/Regional Format
    countryWidget    = new ComboxWidget(tr("Regional Format"), formatGroup, UkccFrame::BorderRadiusStyle::Top);
    KDK_EXTEND_ALL_INFO_FORMAT(countryWidget, "Area", "", "");
    //~ contents_path /Area/Calendar
    calendarWidget   = new ComboxWidget(tr("Calendar"), formatGroup, UkccFrame::BorderRadiusStyle::None);
    KDK_EXTEND_ALL_INFO_FORMAT(calendarWidget, "Area", "", "");
    //~ contents_path /Area/First Day Of Week
    dayWidget        = new ComboxWidget(tr("First Day Of Week"), formatGroup, UkccFrame::BorderRadiusStyle::None);
    KDK_EXTEND_ALL_INFO_FORMAT(dayWidget, "Area", "", "");

    //~ contents_path /Area/Short Foramt Date
    shortDateWidget  = new ComboxWidget(tr("Short Foramt Date"), formatGroup, UkccFrame::BorderRadiusStyle::None);
    KDK_EXTEND_ALL_INFO_FORMAT(shortDateWidget, "Area", "", "");

    //~ contents_path /Area/Long Format Date
    longDateWidget   = new ComboxWidget(tr("Long Format Date"), formatGroup, UkccFrame::BorderRadiusStyle::None);
    KDK_EXTEND_ALL_INFO_FORMAT(longDateWidget, "Area", "", "");

    //~ contents_path /Area/Time
    timeWidget       = new ComboxWidget(tr("Time"), formatGroup, UkccFrame::BorderRadiusStyle::None);
    KDK_EXTEND_ALL_INFO_FORMAT(timeWidget, "Area", "", "");

    formatLayout->addWidget(formatTitleLabel);
    formatLayout->addWidget(formatGroup);
    formatGroup->addWidget(countryWidget);
    formatGroup->addWidget(calendarWidget);
    formatGroup->addWidget(dayWidget);
    formatGroup->addWidget(shortDateWidget);
#ifdef Nile
    formatGroup->addWidget(longDateWidget);
#endif
    formatGroup->addWidget(timeWidget);
    //~ contents_path /Area/Language Format
    formatTitleLabel->setText(tr("Language Format"));

    if (UkccCommon::isOpenkylinNile() || UkccCommon::isV11()) {
        m_formatDateFrame = new UkccFrame(formatGroup);
        m_formatDateFrame->setMinimumSize(QSize(550, 0));
        m_formatDateFrame->setMaximumSize(QSize(16777215, 16777215));
        m_formatDateFrame->setFrameShape(QFrame::Box);

        QHBoxLayout* formatDateLayout = new QHBoxLayout();
        formatDateLayout->setContentsMargins(16, 0, 16, 0);
        //~ contents_path /Area/Language Format Example
        m_formatDateNameLabel = new KLabel(m_formatDateFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(m_formatDateNameLabel, "Area", "", "");
        m_formatDateNameLabel->setText(tr("Language Format Example"));
        m_formatDateNameLabel->setFixedSize(214, 59);

        m_formatDateValueLabel = new KLabel(m_formatDateFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(m_formatDateValueLabel, "Area", "", "");

        formatDateLayout->addWidget(m_formatDateNameLabel);
        formatDateLayout->setSpacing(16);
        formatDateLayout->addWidget(m_formatDateValueLabel);

        m_formatDateFrame->setLayout(formatDateLayout);
        formatGroup->addWidget(m_formatDateFrame);
    }
}

void AreaUi::initSystemLanguage()
{
    languageTitleLable = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(languageTitleLable, "Area", "", "");
    languageTitleLable->setContentsMargins(16, 0, 0, 0);
    languageGroup = new SettingGroup(this);
    addLanguageDialog = new AddLanguageDialog(this);
    addInputMehtodDialog = new AddInputMethodDialog(this);
    installLanguageFeaturesDlg = new InstallLanguageFeaturesDialog(this);

    // 系统当前语言
    currentLanguageFrame = new UkccFrame(languageGroup, UkccFrame::BorderRadiusStyle::Top, false);
    currentLanguageFrame->setMinimumSize(550, 0);
    currentLanguageFrame->setMaximumSize(16777215, 16777215);
    QHBoxLayout *currentLanguageLayout = new QHBoxLayout(currentLanguageFrame);
    currentLanguageLayout->setContentsMargins(16, 16, 16, 16);
    currentLanguageLayout->setSpacing(24);

    QVBoxLayout *currentLanguageTextLayout = new QVBoxLayout();
    currentLanguageTextLayout->setSpacing(0);
    currentLanguageTextLayout->setContentsMargins(0, 0, 0, 0);
    FixLabel *currentLanguageTitle = new FixLabel(tr("Current System Language"), currentLanguageFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(currentLanguageTitle, "Area", "", "");
    currentLanguageTitle->setAccessibleName(tr("Current System Language"));
    LightLabel *currentLanguageHint = new LightLabel(
                tr("The language displayed in system windows, menus and web pages."),
                currentLanguageFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(currentLanguageHint, "Area", "", "");
    currentLanguageHint->setWordWrap(true);
    currentLanguageTextLayout->addWidget(currentLanguageTitle);
    currentLanguageTextLayout->addWidget(currentLanguageHint);

    currentLanguageCombo = new QComboBox(currentLanguageFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(currentLanguageCombo, "Area", "currentLanguageCombox", "current system language combox");
    // 行标题由 FixLabel + buddy 提供；勿对 combo 再 setAccessibleName，否则 AT labelled-by 会指向自身
    currentLanguageTitle->setBuddy(currentLanguageCombo);
    currentLanguageCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    currentLanguageLayout->addLayout(currentLanguageTextLayout, 1);
    currentLanguageLayout->addWidget(currentLanguageCombo, 1);

    // 更多系统语言
    moreLanguageFrame = new UkccFrame(languageGroup, UkccFrame::BorderRadiusStyle::None, true);
    QHBoxLayout *moreLanguageLayout = new QHBoxLayout(moreLanguageFrame);
    moreLanguageLayout->setContentsMargins(16, 0, 16, 0);
    FixLabel *moreLanguageLabel = new FixLabel(tr("More System Languages"), moreLanguageFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(moreLanguageLabel, "Area", "", "");
    addLanguageBtn = new KPushButton(moreLanguageFrame);
    addLanguageBtn->setText(tr("Add Language"));
    KDK_OBJ_INFO_FORMAT(addLanguageBtn);
    addLanguageBtn->setBackgroundColorHighlight(true);
    addLanguageBtn->setFixedHeight(36);
    addLanguageBtn->setMinimumWidth(96);
    moreLanguageLayout->addWidget(moreLanguageLabel);
    moreLanguageLayout->addStretch();
    moreLanguageLayout->addWidget(addLanguageBtn);

    LanguageLayout->addWidget(languageTitleLable);
    LanguageLayout->addSpacerItem(new QSpacerItem(8, 7, QSizePolicy::Fixed, QSizePolicy::Fixed));

    languageRestartBar = new LanguageRestartBar(this);
    languageRestartBar->setMinimumSize(550, 36);
    languageRestartBar->setMaximumSize(16777215, 36);
    QHBoxLayout *languageRestartLayout = new QHBoxLayout(languageRestartBar);
    languageRestartLayout->setContentsMargins(16, 0, 16, 0);
    languageRestartLayout->setSpacing(8);

    languageRestartIcon = new KLabel(languageRestartBar);
    languageRestartIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(16, 16));
    languageRestartIcon->setFixedWidth(16);

    languageRestartTipLabel = new KLabel(languageRestartBar);
    languageRestartTipLabel->setText(tr("Some languages require a system restart before the new language can take effect. Restart the system now?"));
    languageRestartTipLabel->setWordWrap(true);

    languageRestartLink = new KLabel(languageRestartBar);
    languageRestartLink->setText(tr("Restart now"));
    languageRestartLink->setCursor(Qt::PointingHandCursor);
    languageRestartLink->installEventFilter(this);

    languageRestartLayout->addWidget(languageRestartIcon);
    languageRestartLayout->addWidget(languageRestartTipLabel, 1);
    languageRestartLayout->addWidget(languageRestartLink);
    languageRestartBar->hide();

    languageRestartSpacer = new QWidget(this);
    languageRestartSpacer->setFixedHeight(10);
    languageRestartSpacer->hide();

    LanguageLayout->addWidget(languageRestartBar);
    LanguageLayout->addWidget(languageRestartSpacer);
    LanguageLayout->addWidget(languageGroup);
    languageGroup->addWidget(currentLanguageFrame);
    languageGroup->addWidget(moreLanguageFrame);
    languageGroup->setOneRadiusStyle(UkccFrame::BorderRadiusStyle::Top);
    languageGroup->setLastRadiusStyle(UkccFrame::BorderRadiusStyle::Bottom);
    //~ contents_path /Area/System Language
    languageTitleLable->setText(tr("System Language"));
    addLanguageDialog->setHidden(true);
    addInputMehtodDialog->setHidden(true);
    installLanguageFeaturesDlg->setHidden(true);

    KDK_EXTEND_ALL_INFO_FORMAT(languageRestartIcon, "Area", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(languageRestartTipLabel, "Area", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(languageRestartLink, "Area", "", "");

    updateLanguageRestartStyle();
    if (QGSettings::isSchemaInstalled(THEME_QT_SCHEMA)) {
        QGSettings *styleSettings = new QGSettings(THEME_QT_SCHEMA, QByteArray(), this);
        connect(styleSettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "styleName" || key == "themeColor" || key == "widgetThemeName") {
                QTimer::singleShot(0, this, &AreaUi::updateLanguageRestartStyle);
            }
        });
    }

    connect(currentLanguageCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [=](int index) {
        if (index < 0) {
            return;
        }
        const QString languageCode = currentLanguageCombo->itemData(index).toString();
        if (languageCode.isEmpty()) {
            return;
        }
        Q_EMIT languageChanged(languageCode);
    });
}

// 更新当前语言下拉框内容
void AreaUi::updateCurrentLanguageCombo(const QStringList &languageCodes,
                                      const QStringList &languageNames,
                                      const QString &currentLanguageCode)
{
    if (!currentLanguageCombo || languageCodes.size() != languageNames.size()) {
        return;
    }
    currentLanguageCombo->blockSignals(true);
    currentLanguageCombo->clear();
    int currentIndex = -1;
    for (int i = 0; i < languageCodes.size(); ++i) {
        currentLanguageCombo->addItem(languageNames.at(i), languageCodes.at(i));
        if (languageCodes.at(i) == currentLanguageCode) {
            currentIndex = i;
        }
    }
    if (currentIndex >= 0) {
        currentLanguageCombo->setCurrentIndex(currentIndex);
    }
    currentLanguageCombo->blockSignals(false);
}

// 设置当前语言选中项
void AreaUi::setCurrentLanguageCode(const QString &languageCode)
{
    if (!currentLanguageCombo) {
        return;
    }
    const int index = currentLanguageCombo->findData(languageCode);
    if (index >= 0 && index != currentLanguageCombo->currentIndex()) {
        currentLanguageCombo->blockSignals(true);
        currentLanguageCombo->setCurrentIndex(index);
        currentLanguageCombo->blockSignals(false);
    }
    for (LanguageFrame *frame : languageFrameList) {
        frame->setDeleteEnabled(frame->getLanguageCode() != languageCode);
    }
}

// 显示语言重启提示
void AreaUi::showLanguageRestartTip()
{
    if (languageRestartBar) {
        languageRestartBar->show();
    }
    if (languageRestartSpacer) {
        languageRestartSpacer->show();
    }
}

// 隐藏语言重启提示
void AreaUi::hideLanguageRestartTip()
{
    if (languageRestartBar) {
        languageRestartBar->hide();
    }
    if (languageRestartSpacer) {
        languageRestartSpacer->hide();
    }
}

// 更新语言重启提示样式
void AreaUi::updateLanguageRestartStyle()
{
    if (languageRestartLink) {
        const QColor accentColor = ThemeController::getCustomColorFromDT("kbrand-normal");
        if (accentColor.isValid()) {
            languageRestartLink->setFontColor(accentColor);
        }
    }
    if (languageRestartBar) {
        languageRestartBar->update();
    }
}

// 添加语言展示项
void AreaUi::addShowLanguage(const QString &languageCode, const QString &languageName, bool isCurrent)
{
    addShowLanguageWithFeatures(languageCode, languageName, isCurrent);
}

// 添加带特性的语言展示项
LanguageFrame *AreaUi::addShowLanguageWithFeatures(const QString &languageCode,
                                                   const QString &languageName,
                                                   bool isCurrent)
{
    for (LanguageFrame *f : languageFrameList) {
        if (f->getLanguageCode() == languageCode) {
            f->setDeleteEnabled(!isCurrent);
            f->setExpanded(false);
            return f;
        }
    }
    LanguageFrame *languageF = new LanguageFrame(languageCode, languageName, languageGroup);
    languageF->setDeleteEnabled(!isCurrent);
    languageF->setExpanded(false);
    languageGroup->addWidget(languageF);
    languageFrameList.append(languageF);
    connect(languageF, &LanguageFrame::deleteRequested, this, [=]() {
        Q_EMIT languageDeleteRequested(languageCode, languageName);
    });
    connect(languageF, &LanguageFrame::featureInstallRequested, this,
            &AreaUi::languageFeatureInstallRequested);
    return languageF;
}

// 折叠所有语言展示项
void AreaUi::collapseAllLanguageFrames()
{
    for (LanguageFrame *frame : languageFrameList) {
        if (frame) {
            frame->setExpanded(false);
        }
    }
}

// 查找指定语言展示项
LanguageFrame *AreaUi::languageFrame(const QString &languageCode) const
{
    for (LanguageFrame *f : languageFrameList) {
        if (f->getLanguageCode() == languageCode) {
            return f;
        }
    }
    return nullptr;
}

// 移除指定语言展示项
void AreaUi::removeLanguageFrame(const QString &languageCode)
{
    LanguageFrame *frame = languageFrame(languageCode);
    if (!frame) {
        return;
    }
    const QString languageName = frame->getShowName();
    languageGroup->removeWidget(frame);
    languageFrameList.removeOne(frame);
    frame->deleteLater();
    languageGroup->updateShape();
    Q_EMIT showLanguageListRemoved(languageCode, languageName);
}

void AreaUi::clearShowLanguage()
{
    qDeleteAll(languageFrameList);
    languageFrameList.clear();
}

void AreaUi::reloadLanguageFrameIcon()
{
    for (LanguageFrame *f : languageFrameList) {
        f->reloadIcon();
    }
}

// 处理重启链接点击事件
bool AreaUi::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == languageRestartLink && event->type() == QEvent::MouseButtonRelease) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            Q_EMIT languageRestartRequested();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AreaUi::settingForIntel()
{
    countryWidget->hide();
    dayWidget->hide();
    languageTitleLable->hide();
    addLanguageBtn->hide();
    languageGroup->hide();
}
