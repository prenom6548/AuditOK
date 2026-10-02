/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include "screensaverui.h"
#include "commonfunc.h"
#include <QPluginLoader>
#include <QProcess>
#include <QStandardPaths>

#define DEFAULT_PHOTO_PATH "/usr/share/backgrounds"
#define LOCAL_WALLPAPER_PATH (QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/ukui-control-center/wallpaperData/"))

ScreensaverUi::ScreensaverUi()
{
    setAttribute(Qt::WA_DeleteOnClose);
    uiLayout = new QVBoxLayout(this);
    uiLayout->setSpacing(8);
    uiLayout->setMargin(0);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // 允许自由拉伸
    uiLayout->setSizeConstraint(QLayout::SetMinAndMaxSize ); // 既允许布局根据内容自动调整，又不会阻止窗口拉伸
    uiLayout->setContentsMargins(0, 0, 0, 0); // 移除边距
    initUi();
    sendSignals();
}

ScreensaverUi::~ScreensaverUi()
{

}

void ScreensaverUi::initUi()
{
    initPreview();
    initBasisSettings();
    initThumbnailSaver();
    initWeatherSettings();
    initMusicSettings();
    initPhotoSettings();
    initPhotoSource();
    initSetting();
    uiLayout->addStretch();
}

void ScreensaverUi::initPreview()
{
    screensaverTitle = new KLabel(this);
    screensaverTitle->setContentsMargins(16, 0, 0, 0);
    previewFrame     = new UkccFrame(this);
    previewLayout    = new QHBoxLayout(previewFrame);
    previewLayout->setContentsMargins(16, 8, 8, 8);
    previewWidget    = new PreviewWidget(this);

    uiLayout->addWidget(screensaverTitle);
    uiLayout->addWidget(previewFrame);

    //~ contents_path /Screensaver/Screensaver
    screensaverTitle->setText(tr("Screensaver"));
    previewWidget->setFixedSize(276, 160);
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setContentsMargins(0, 8, 0, 0);
    leftLayout->addWidget(previewWidget);
    leftLayout->addStretch();
    previewLayout->addLayout(leftLayout);

    QHBoxLayout *layout = new QHBoxLayout(previewWidget);
    layout->setMargin(0);
}

void ScreensaverUi::initThumbnailSaver()
{
    m_thumbnailSaverGroup = new SettingGroup(this);
    m_thumbnailSaverGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    m_thumbnailFrame = new UkccFrame(this);
    m_thumbnailFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    UkccFrame *saverFrame = new UkccFrame(m_thumbnailSaverGroup, UkccFrame::Top);
    QHBoxLayout *topLayout = new QHBoxLayout(saverFrame);
    topLayout->setContentsMargins(16, 0, 16, 0);
    KLabel *selectSaverLabel = new KLabel(saverFrame);
    selectSaverLabel->setText(tr("Select Screensaver"));
    topLayout->addWidget(selectSaverLabel);
    topLayout->addStretch();

    m_thumbnailLayout = new FlowLayout(m_thumbnailFrame, 8, 8, 8);
    QWidget *saverWidget = new QWidget();
    QVBoxLayout *saverLayout = new QVBoxLayout(saverWidget);
    saverLayout->setContentsMargins(4, 0, 4, 0);
    saverLayout->setSpacing(4);
    m_defaulSaverWidget = new SaverThumbnailWidget(this, true);
    KLabel *saverLabel = new KLabel(m_thumbnailFrame);
    saverLabel->setFixedWidth(m_defaulSaverWidget->width());
    saverLabel->setAlignment(Qt::AlignCenter);
    saverLabel->setText("UKUI");
    saverLayout->addWidget(m_defaulSaverWidget);
    saverLayout->addWidget(saverLabel);
    m_thumbnailLayout->addWidget(saverWidget);

    QWidget *weatherWidget = new QWidget();
    QVBoxLayout *weatherLayout = new QVBoxLayout(weatherWidget);
    weatherLayout->setContentsMargins(4, 0, 4, 0);
    weatherLayout->setSpacing(4);
    m_weatherThumbnailWidget = new WeatherThumbnailWidget(this, true);
    KLabel *weatherLabel = new KLabel(m_thumbnailFrame);
    weatherLabel->setFixedWidth(m_weatherThumbnailWidget->width());
    weatherLabel->setAlignment(Qt::AlignCenter);
    weatherLabel->setText(tr("Weather"));
    weatherLayout->addWidget(m_weatherThumbnailWidget);
    weatherLayout->addWidget(weatherLabel);
    m_thumbnailLayout->addWidget(weatherWidget);

    QWidget *musicWidget = new QWidget();
    QVBoxLayout *musicLayout = new QVBoxLayout(musicWidget);
    musicLayout->setContentsMargins(4, 0, 4, 0);
    musicLayout->setSpacing(4);
    m_musicWidget = new MusicThumbnailWidget(this, true);
    KLabel *musicLabel = new KLabel(m_thumbnailFrame);
    musicLabel->setFixedWidth(m_musicWidget->width());
    musicLabel->setAlignment(Qt::AlignCenter);
    musicLabel->setText(tr("Music"));
    musicLayout->addWidget(m_musicWidget);
    musicLayout->addWidget(musicLabel);
    m_thumbnailLayout->addWidget(musicWidget);

    QWidget *albunWidget = new QWidget();
    QVBoxLayout *albunLayout = new QVBoxLayout(albunWidget);
    albunLayout->setContentsMargins(4, 0, 4, 0);
    albunLayout->setSpacing(4);
    m_albunWidget = new AlbumThumbnailWidget(this, true);
    KLabel *albunLabel = new KLabel(m_thumbnailFrame);
    albunLabel->setFixedWidth(m_albunWidget->width());
    albunLabel->setAlignment(Qt::AlignCenter);
    albunLabel->setText(tr("Albun"));
    albunLayout->addWidget(m_albunWidget);
    albunLayout->addWidget(albunLabel);
    m_thumbnailLayout->addWidget(albunWidget);

//    mainLayout->addWidget(selectSaverLabel);
//    mainLayout->addLayout(m_thumbnailLayout);

    m_thumbnailSaverGroup->addWidget(saverFrame);
    m_thumbnailSaverGroup->addWidget(m_thumbnailFrame);
    uiLayout->addWidget(m_thumbnailSaverGroup);
}

void ScreensaverUi::initBasisSettings()
{
    m_basisGroup = new KBackgroundGroup(this);
    m_basisGroup->setFixedHeight(275);
    initIdleTimeFrame();
    initScreenlockFrame();
    initSaverDatetimeFrame();
    initBreakTimeFrame();
    previewLayout->addWidget(m_basisGroup);
}

void ScreensaverUi::initIdleTimeFrame()
{
    QWidget   *idleTimeWidget   = new QWidget(m_basisGroup);
    QHBoxLayout *idleTimeLayout = new QHBoxLayout(idleTimeWidget);
    KLabel    *idleTimeLabel  = new KLabel(idleTimeWidget);

    idleTimeLayout->addWidget(idleTimeLabel);

    m_idleTimeBox = new QComboBox(idleTimeWidget);
    idleTimeLayout->addStretch();
    idleTimeLayout->addWidget(m_idleTimeBox);

    //~ contents_path /Screensaver/Idle time
    idleTimeLabel->setText(tr("Idle time"));
    m_basisGroup->addWidget(idleTimeWidget);
}

void ScreensaverUi::initSaverDatetimeFrame()
{
    QWidget   *saverDatetimeWidget   = new QWidget(m_basisGroup);
    QHBoxLayout *saverDatetimeLayout = new QHBoxLayout(saverDatetimeWidget);
    KLabel    *saverDatetimeLabel  = new KLabel(saverDatetimeWidget);

    saverDatetimeLayout->addWidget(saverDatetimeLabel);

    m_saverDatetimeBtn = new KSwitchButton(saverDatetimeWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(m_saverDatetimeBtn, "saverDatetimeWidget", "initSaverDatetimeFrame", "Toggle for showing date and time");
    saverDatetimeLayout->addStretch();
    saverDatetimeLayout->addWidget(m_saverDatetimeBtn);

    //~ contents_path /Screensaver/The date and time are displayed
    saverDatetimeLabel->setText(tr("The date and time are displayed"));
    m_basisGroup->addWidget(saverDatetimeWidget);
}

void ScreensaverUi::initBreakTimeFrame()
{
    QWidget   *showTimeFrame  = new QWidget(m_basisGroup);
    QHBoxLayout *showTimeLayout = new QHBoxLayout(showTimeFrame);
    KLabel    *showTimeLabel  = new KLabel(showTimeFrame);

    showTimeLayout->addWidget(showTimeLabel);
    //添加休息时间显示按钮
    showCustomTimeBtn = new KSwitchButton(showTimeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(showCustomTimeBtn, "showTimeFrame", "initSaverDatetimeFrame", "UKUI display break time switch");
    showUkuiTimeBtn   = new KSwitchButton(showTimeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(showUkuiTimeBtn, "showTimeFrame", "initSaverDatetimeFrame", "Custom mode toggle for showing break time");
    showTimeLayout->addStretch();
    showTimeLayout->addWidget(showCustomTimeBtn);
    showTimeLayout->addWidget(showUkuiTimeBtn);

    //~ contents_path /Screensaver/Show rest time
    showTimeLabel->setText(tr("Show rest time"));
    m_basisGroup->addWidget(showTimeFrame);
}

void ScreensaverUi::initScreenlockFrame()
{
    QWidget   *lockFrame  = new QWidget(m_basisGroup);
    QHBoxLayout *lockLayout = new QHBoxLayout(lockFrame);

    KLabel    *lockLabel  = new KLabel();
    //~ contents_path /Screensaver/Lock screen when screensaver boot
    lockLabel->setText(tr("Lock screen when screensaver boot"));

    lockLayout->addWidget(lockLabel);

    lockButton = new KSwitchButton(lockFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(lockButton, "lockFrame", "initScreenlockFrame", "Toggle for Lock screen when screensaver boot");
    lockLayout->addStretch();
    lockLayout->addWidget(lockButton);

    m_basisGroup->addWidget(lockFrame);
}

void ScreensaverUi::initWeatherSettings()
{
    m_weatherFrame  = new UkccFrame(this, UkccFrame::BorderRadiusStyle::Around, true);
    QHBoxLayout *weatherSettingsLayout = new QHBoxLayout(m_weatherFrame);
    KLabel    *weatherSettingsLabel  = new KLabel();

    weatherSettingsLayout->setContentsMargins(16, 0, 16, 0);
    weatherSettingsLayout->addWidget(weatherSettingsLabel);
    m_weatherSettingBtn = new QPushButton(m_weatherFrame);
    m_weatherSettingBtn->setText(tr("Go to App Settings"));
    m_localLabel = new KLabel(m_weatherFrame);
    m_localLabel->setAlignment(Qt::AlignCenter);
    QPixmap localIcon = QIcon::fromTheme("mark-location-symbolic").pixmap(QSize(16, 16));
    m_localLabel->setPixmap(localIcon);
    m_localLabel->setMinimumWidth(80);
    m_localLabel->setFixedHeight(m_weatherFrame->height());
    weatherSettingsLayout->addStretch();
    weatherSettingsLayout->addWidget(m_localLabel);
    weatherSettingsLayout->addWidget(m_weatherSettingBtn);
    weatherSettingsLabel->setFixedWidth(220);

    //~ contents_path /Screensaver/Show rest time
    weatherSettingsLabel->setText(tr("Current position"));
    uiLayout->addWidget(m_weatherFrame);
}

void ScreensaverUi::initMusicSettings()
{
    m_musicSaverGroup = new SettingGroup(this);

    UkccFrame   *followSystemFrame = new UkccFrame(m_musicSaverGroup, UkccFrame::BorderRadiusStyle::Top, true);
    QHBoxLayout *followSystemLayout           = new QHBoxLayout(followSystemFrame);
    followSystemLayout->setContentsMargins(16, 0, 16, 0);
    followSystemLayout->setSpacing(8);
    //~ contents_path /Screensaver/Prioritize the list of songs from the Music app
    m_followSystemBtn = new QRadioButton(tr("Prioritize the list of songs from the Music app"), followSystemFrame);
    followSystemLayout->addWidget(m_followSystemBtn);
    followSystemLayout->addStretch();

    UkccFrame   *musicSourceFrame = new UkccFrame(m_musicSaverGroup, UkccFrame::BorderRadiusStyle::Bottom, true);
    QHBoxLayout *sourceLayout           = new QHBoxLayout(musicSourceFrame);
    //~ contents_path /Screensaver/Priority playlists
    m_customMusicBtn = new QRadioButton(tr("Priority playlists"), musicSourceFrame);

    m_musicPathLine = new QLineEdit(musicSourceFrame);
    m_musicSourceBtn   = new QPushButton(musicSourceFrame);
    m_musicSourceBtn->setText(tr("Select the folder"));

    sourceLayout->setContentsMargins(16, 0, 16, 0);
    sourceLayout->setSpacing(8);
    sourceLayout->addWidget(m_customMusicBtn);
    sourceLayout->addWidget(m_musicPathLine);
    sourceLayout->addWidget(m_musicSourceBtn);
    m_musicPathLine->setMinimumWidth(252);
    m_musicPathLine->setReadOnly(true);
    m_musicSourceBtn->adjustSize();
    m_musicSourceBtn->raise();

    m_musicSaverGroup->addWidget(followSystemFrame);
    m_musicSaverGroup->addWidget(musicSourceFrame);
    uiLayout->addWidget(m_musicSaverGroup);
}

void ScreensaverUi::initPhotoSettings()
{
    m_PhotoSettingGroup = new SettingGroup(this);

    //~ contents_path /Screensaver/Switch mode
    m_photoSwitchWidget = new RadioButtonWidget(tr("Switch mode"), m_PhotoSettingGroup, UkccFrame::Top);
    m_orderBtn = new QRadioButton(tr("In order"), m_photoSwitchWidget);
    m_randomBtn = new QRadioButton(tr("Random"), m_photoSwitchWidget);
    m_photoSwitchWidget->addButton(m_orderBtn, 0);
    m_photoSwitchWidget->addButton(m_randomBtn, 1, true);

    //~ contents_path /Screensaver/Switching frequency
    m_photoFrequencyWidget    = new ComboxWidget(tr("Switching frequency"), m_PhotoSettingGroup);

    m_PhotoSettingGroup->addWidget(m_photoSwitchWidget);
    m_PhotoSettingGroup->addWidget(m_photoFrequencyWidget);
    uiLayout->addWidget(m_PhotoSettingGroup);
}

void ScreensaverUi::initPhotoSource()
{
    m_photoSourceGroup = new SettingGroup(this);

    UkccFrame *imageSourceFrame = new UkccFrame(m_photoSourceGroup, UkccFrame::Top, false);
    imageSourceFrame->setMinimumHeight(50);
    QHBoxLayout *topLayout = new QHBoxLayout(imageSourceFrame);
    topLayout->setContentsMargins(16, 0, 16, 0);
    KLabel *sourceLabel = new KLabel(m_photoSourceFrame);
    //~ contents_path /Screensaver/Image source
    sourceLabel->setText(tr("Image source"));
    m_photoSourceBtn = new QPushButton(tr("Add image folder"), m_photoSourceFrame);
    topLayout->addWidget(sourceLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_photoSourceBtn);

    m_photoSourceFrame = new UkccFrame(m_photoSourceGroup, UkccFrame::Bottom, false);

    m_photoSourceLayout = new FlowLayout(m_photoSourceFrame, -1, 8);
    m_photoSourceLayout->setContentsMargins(12, 8, 12, 8);

    m_photoSourceGroup->addWidget(imageSourceFrame);
    m_photoSourceGroup->addWidget(m_photoSourceFrame);
    uiLayout->addWidget(m_photoSourceGroup);
}

void ScreensaverUi::initSetting()
{
    screensaverGroup = new SettingGroup(this);
    //~ contents_path /Screensaver/Screensaver program
    programWidget    = new ComboxWidget(tr("Screensaver program"), screensaverGroup);
    customizeGroup   = new SettingGroup(screensaverGroup);

    uiLayout->addWidget(screensaverGroup);
    screensaverGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    screensaverGroup->addWidget(programWidget);
    screensaverGroup->addWidget(customizeGroup);

    initSourceFrame();
    initSwitchModeFrame();
    initSwitchTimeFrame();
    initCustomTextFrame();
    initTextPositionFrame();
}

void ScreensaverUi::initSourceFrame()
{
    UkccFrame   *screensaverSourceFrame = new UkccFrame(customizeGroup, UkccFrame::BorderRadiusStyle::None, true);
    QHBoxLayout *sourceLayout           = new QHBoxLayout(screensaverSourceFrame);
    KLabel    *sourceLabel            = new KLabel();

    sourcePathLine = new QLineEdit();
    sourceButton   = new QPushButton();

    sourceLayout->setContentsMargins(16, 0, 16, 0);
    sourceLayout->addWidget(sourceLabel);
    sourceLayout->addWidget(sourcePathLine);
    sourceLayout->addWidget(sourceButton);
    //~ contents_path /Screensaver/Screensaver source
    sourceLabel->setText(tr("Screensaver source"));
    sourceLabel->setFixedWidth(220);
    sourcePathLine->setMinimumWidth(252);
    sourcePathLine->setReadOnly(true);
    sourceButton->adjustSize();
    sourceButton->setText(tr("Select"));
    sourceButton->raise();

    customizeGroup->addWidget(screensaverSourceFrame);
}

void ScreensaverUi::initSwitchModeFrame()
{
    m_pictureSwitchFrame = new UkccFrame(customizeGroup, UkccFrame::BorderRadiusStyle::Around, true);
    QHBoxLayout  *randomLayout       = new QHBoxLayout(m_pictureSwitchFrame);
    KLabel     *randomLabel        = new KLabel();
    QRadioButton *radioOrder         = new QRadioButton();
    QRadioButton *radioRand          = new QRadioButton();

    switchModeGroup  = new QButtonGroup();

    switchModeGroup->addButton(radioOrder, 0);
    switchModeGroup->addButton(radioRand, 1);

    radioOrder->setFixedWidth(220);
    radioRand->adjustSize();
    radioOrder->setText(tr("Ordinal"));
    radioRand->setText(tr("Random"));

    randomLayout->setContentsMargins(16, 0, 16, 0);
    randomLayout->addWidget(randomLabel);
    randomLayout->addWidget(radioOrder);
    randomLayout->addWidget(radioRand);
    //~ contents_path /Screensaver/Random switching
    randomLabel->setText(tr("Random switching"));
    randomLabel->setFixedWidth(220);
    randomLayout->addStretch();

    customizeGroup->addWidget(m_pictureSwitchFrame);
}

void ScreensaverUi::initSwitchTimeFrame()
{
    m_timeSetFrame  = new UkccFrame(customizeGroup, UkccFrame::BorderRadiusStyle::Around, true);
    QHBoxLayout *timeSetLayout = new QHBoxLayout();
    KLabel    *timeSetLabel  = new KLabel();

    switchTimeComboBox = new QComboBox();

    m_timeSetFrame->setLayout(timeSetLayout);
    timeSetLayout->setContentsMargins(16, 0, 16, 0);
    timeSetLayout->addWidget(timeSetLabel);
    timeSetLayout->addWidget(switchTimeComboBox);
    //~ contents_path /Screensaver/Switching time
    timeSetLabel->setText(tr("Switching time"));
    timeSetLabel->setFixedWidth(220);
    switchTimeComboBox->setMinimumWidth(340);

    customizeGroup->addWidget(m_timeSetFrame);
}

void ScreensaverUi::initCustomTextFrame()
{
    m_showTextFrame  = new UkccFrame(customizeGroup, UkccFrame::BorderRadiusStyle::Around, true);
    QHBoxLayout *showTextLayout = new QHBoxLayout();
    KLabel    *showLabel      = new KLabel();

    textLineEdit = new QLineEdit(); //用户输入文字
    m_showTextFrame->setLayout(showTextLayout);
    showTextLayout->setContentsMargins(16, 6, 15, 8);
    showTextLayout->addWidget(showLabel);
    showTextLayout->addWidget(textLineEdit);
    //~ contents_path /Screensaver/Text(up to 30 characters):
    showLabel->setText(tr("Text(up to 30 characters)"));
    showLabel->setFixedWidth(220);
    textLineEdit->setMaxLength(30);

    customizeGroup->addWidget(m_showTextFrame);
}

void ScreensaverUi::initTextPositionFrame()
{
    m_showTextSetFrame = new UkccFrame(customizeGroup, UkccFrame::BorderRadiusStyle::Around, true);
    QHBoxLayout  *textSetLayout    = new QHBoxLayout(m_showTextSetFrame);
    KLabel     *textSetLabel     = new KLabel(m_showTextSetFrame);
    QRadioButton *radioRandom      = new QRadioButton(this);
    QRadioButton *radioCenter      = new QRadioButton(this);

    textPositionGroup = new QButtonGroup();
    textPositionGroup->addButton(radioRandom, 0);
    textPositionGroup->addButton(radioCenter, 1);

    m_showTextSetFrame->setFixedHeight(60);
    textSetLayout->addWidget(textSetLabel);
    textSetLayout->setContentsMargins(16,0,16,0);
    //~ contents_path /Screensaver/Text position
    textSetLabel->setText(tr("Text position"));
    textSetLabel->setFixedWidth(220);
    radioRandom->setFixedWidth(220);
    radioCenter->adjustSize();

    textSetLayout->addWidget(radioRandom);
    textSetLayout->addWidget(radioCenter);
    textSetLayout->addStretch();
    radioCenter->setText(tr("Centered"));
    radioRandom->setText(tr("Randow(Bubble text)"));

    customizeGroup->addWidget(m_showTextSetFrame);
}

void ScreensaverUi::setIdleTimeOptions(const QStringList &textList, const QList<int> &dataList)
{
    if (textList.size() == dataList.size()) {
        m_idleTimeBox->clear();
        for (int i = 0; i < textList.size(); ++i) {
            m_idleTimeBox->addItem(textList.at(i), dataList.at(i));
        }
    }
}

void ScreensaverUi::setScreensaverOptions(const QStringList &textList, const QStringList &dataList)
{
    if (textList.size() == dataList.size()) {
        programWidget->comboBox()->clear();
        for (int i = 0; i < textList.size(); ++i) {
            programWidget->comboBox()->addItem(textList.at(i), dataList.at(i));
        }
    }
}

void ScreensaverUi::setSwitchTimeOptions(const QStringList &textList, const QList<int> &dataList)
{
    if (textList.size() == dataList.size()) {
        switchTimeComboBox->clear();
        for (int i = 0; i < textList.size(); ++i) {
            switchTimeComboBox->addItem(textList.at(i), dataList.at(i));
        }
    }
}

void ScreensaverUi::setPhotoSwitchTimeOptions(const QStringList &textList, const QList<int> &dataList)
{
    if (textList.size() == dataList.size()) {
        m_photoFrequencyWidget->comboBox()->clear();
        for (int i = 0; i < textList.size(); ++i) {
            m_photoFrequencyWidget->comboBox()->addItem(textList.at(i), dataList.at(i));
        }
    }
}

void ScreensaverUi::setIdleTime(int time)
{
    for (int i = 0; i < m_idleTimeBox->count(); ++i) {
        if (time == m_idleTimeBox->itemData(i)) {
            m_idleTimeBox->blockSignals(true);
            m_idleTimeBox->setCurrentIndex(i);
            m_idleTimeBox->blockSignals(false);
        }
    }
}

void ScreensaverUi::setScreensaver(QString type)
{
    for (int i = 0; i < programWidget->comboBox()->count(); ++i) {
        if (type == programWidget->comboBox()->itemData(i)) {
            programWidget->comboBox()->blockSignals(true);
            programWidget->comboBox()->setCurrentIndex(i);
            programWidget->comboBox()->blockSignals(false);
        }
    }
    if (type == "ukui") {
        m_bCustomSaver = false;
    } else if (type == "custom") {
        m_bCustomSaver = true;
    }
}

void ScreensaverUi::customizeNeedHide(bool hide)
{
    m_bHideCustom = hide;
    if (!m_bCustomSaver) {
        m_thumbnailSaverGroup->setVisible(true);
        customizeGroup->setVisible(false);
        showUkuiTimeBtn->setVisible(true);
        showCustomTimeBtn->setVisible(false);
    } else if (m_bCustomSaver && !m_bHideCustom) {
        m_thumbnailSaverGroup->setVisible(true);
        if (m_subMode == 0) {
            customizeGroup->setVisible(true);
        } else {
            customizeGroup->setVisible(false);
        }
        showUkuiTimeBtn->setVisible(false);
        showCustomTimeBtn->setVisible(true);
    } else if (m_bCustomSaver && m_bHideCustom) {
        customizeGroup->setVisible(false);
        m_thumbnailSaverGroup->setVisible(false);
        showUkuiTimeBtn->setVisible(false);
        showCustomTimeBtn->setVisible(true);
    }
}

void ScreensaverUi::updatePreview(QWidget *w)
{
    for (QObject *o : previewWidget->children()) {
        if (o->objectName() == "screensaverWidget") {
            delete o;
            o = nullptr;
        }
    }
    w->setObjectName("screensaverWidget");
    previewWidget->layout()->addWidget(w);
    previewWidget->update();
}

void ScreensaverUi::closePreview()
{
    if (!previewWidget) {
        return;
    }
    for (QObject *o : previewWidget->children()) {
        if (o->objectName() == "screensaverWidget") {
            delete o;
            o = nullptr;
        }
    }
    previewWidget->update();
}

void ScreensaverUi::setSourcePath(const QString &text)
{
    if (sourcePathLine == nullptr) {
        return;
    }

    QFontMetrics fontMetrics(sourcePathLine->font());
    int fontSize = fontMetrics.width(text);
    if (fontSize > sourcePathLine->width()) {
       sourcePathLine->setText(fontMetrics.elidedText(text, Qt::ElideRight, sourcePathLine->width()));
       sourcePathLine->setToolTip(text);
    } else {
       sourcePathLine->setText(text);
       sourcePathLine->setToolTip("");
    }
}

void ScreensaverUi::setSwitchRandom(const bool &b)
{
    switchModeGroup->blockSignals(true);
    if (b) {
        switchModeGroup->button(1)->setChecked(true);
    } else {
        switchModeGroup->button(0)->setChecked(true);
    }
    switchModeGroup->blockSignals(false);
}

void ScreensaverUi::setSwitchTime(const int &time)
{
    for (int i = 0; i < switchTimeComboBox->count(); ++i) {
        if (time == switchTimeComboBox->itemData(i)) {
            switchTimeComboBox->blockSignals(true);
            switchTimeComboBox->setCurrentIndex(i);
            switchTimeComboBox->blockSignals(false);
        }
    }
}

void ScreensaverUi::setPhotoSwitchTime(const int &time)
{
    for (int i = 0; i < m_photoFrequencyWidget->comboBox()->count(); ++i) {
        if (time == m_photoFrequencyWidget->comboBox()->itemData(i)) {
            m_photoFrequencyWidget->comboBox()->blockSignals(true);
            m_photoFrequencyWidget->comboBox()->setCurrentIndex(i);
            m_photoFrequencyWidget->comboBox()->blockSignals(false);
        }
    }
}

void ScreensaverUi::setCustomText(const QString &text)
{
    textLineEdit->blockSignals(true);
    textLineEdit->setText(text);
    textLineEdit->blockSignals(false);
}

void ScreensaverUi::setTextCentered(const bool &b)
{
    textPositionGroup->blockSignals(true);
    if (b) {
        textPositionGroup->button(1)->setChecked(true);
    } else {
        textPositionGroup->button(0)->setChecked(true);
    }
    textPositionGroup->blockSignals(false);
}

void ScreensaverUi::setShowUkuiTime(const bool &b)
{
    showUkuiTimeBtn->blockSignals(true);
    showUkuiTimeBtn->setChecked(b);
    showUkuiTimeBtn->blockSignals(false);
}

void ScreensaverUi::setShowCustomTime(const bool &b)
{
    showCustomTimeBtn->blockSignals(true);
    showCustomTimeBtn->setChecked(b);
    showCustomTimeBtn->blockSignals(false);
}

void ScreensaverUi::setLockscreen(const bool &b)
{
    lockButton->blockSignals(true);
    lockButton->setChecked(b);
    lockButton->blockSignals(false);
}

void ScreensaverUi::setThumbnailBackground(QString background)
{
    QString thumbnailPath = mapToThumbnailPath(background);
    m_defaulSaverWidget->setBackground(loadFromFile(thumbnailPath));
    QPixmap scaleBackground = scaleBlurPixmap(m_defaulSaverWidget->width(), m_defaulSaverWidget->height(), thumbnailPath);
    m_weatherThumbnailWidget->setBackground(scaleBackground);
    m_musicWidget->setBackground(scaleBackground);
    m_albunWidget->setBackground(scaleBackground);
    m_albunWidget->setDefaultPhoto(loadFromFile(thumbnailPath));
}

void ScreensaverUi::setSubMode(int mode)
{
    m_subMode = mode;
    if (mode == 3) { // 相册模式
        m_photoSourceGroup->setMinimumHeight(200); // 设置一个合理的最小高度
    } else {
        m_photoSourceGroup->setMinimumHeight(0); // 其他模式恢复自动高度
    }

    switch (mode) {
    case 0:
        m_defaulSaverWidget->setSelectStatus(true);
        m_weatherThumbnailWidget->setSelectStatus(false);
        m_musicWidget->setSelectStatus(false);
        m_albunWidget->setSelectStatus(false);

        if (m_bCustomSaver) {
            customizeGroup->setVisible(true);
        }
        programWidget->setVisible(true);
        m_weatherFrame->setVisible(false);
        m_musicSaverGroup->setVisible(false);
        m_PhotoSettingGroup->setVisible(false);
        m_photoSourceGroup->setVisible(false);
        break;
    case 1:
        m_defaulSaverWidget->setSelectStatus(false);
        m_weatherThumbnailWidget->setSelectStatus(true);
        m_musicWidget->setSelectStatus(false);
        m_albunWidget->setSelectStatus(false);

        programWidget->setVisible(false);
        customizeGroup->setVisible(false);
        m_weatherFrame->setVisible(true);
        m_musicSaverGroup->setVisible(false);
        m_PhotoSettingGroup->setVisible(false);
        m_photoSourceGroup->setVisible(false);
        break;
    case 2:
        m_defaulSaverWidget->setSelectStatus(false);
        m_weatherThumbnailWidget->setSelectStatus(false);
        m_musicWidget->setSelectStatus(true);
        m_albunWidget->setSelectStatus(false);

        programWidget->setVisible(false);
        customizeGroup->setVisible(false);
        m_weatherFrame->setVisible(false);
        m_musicSaverGroup->setVisible(true);
        m_PhotoSettingGroup->setVisible(false);
        m_photoSourceGroup->setVisible(false);
        break;
    case 3:
        m_defaulSaverWidget->setSelectStatus(false);
        m_weatherThumbnailWidget->setSelectStatus(false);
        m_musicWidget->setSelectStatus(false);
        m_albunWidget->setSelectStatus(true);

        programWidget->setVisible(false);
        customizeGroup->setVisible(false);
        m_weatherFrame->setVisible(false);
        m_musicSaverGroup->setVisible(false);
        m_PhotoSettingGroup->setVisible(true);
        m_photoSourceGroup->setVisible(true);
        break;
    default:
        break;
    }
}

void ScreensaverUi::setCurrentPosition(QString position)
{
    m_localLabel->setText(position);
    m_localLabel->adjustSize();
}

void ScreensaverUi::setMusicFollowSystem(const bool &b)
{
    m_followSystemBtn->setChecked(b);
    m_customMusicBtn->setChecked(!b);
}

void ScreensaverUi::setMusicPath(QString path)
{
    if (m_musicPathLine == nullptr) {
        return;
    }

    QFontMetrics fontMetrics(m_musicPathLine->font());
    int fontSize = fontMetrics.width(path);
    if (fontSize > m_musicPathLine->width()) {
       m_musicPathLine->setText(fontMetrics.elidedText(path, Qt::ElideRight, m_musicPathLine->width()));
       m_musicPathLine->setToolTip(path);
    } else {
       m_musicPathLine->setText(path);
       m_musicPathLine->setToolTip("");
    }
}

void ScreensaverUi::setPhotoCycleMode(int mode)
{
    if (mode == 0) {
        m_orderBtn->setChecked(true);
        m_randomBtn->setChecked(false);
    } else {
        m_orderBtn->setChecked(false);
        m_randomBtn->setChecked(true);
    }
}

void ScreensaverUi::setSaverDatetimeEnable(const bool &b)
{
    m_saverDatetimeBtn->blockSignals(true);
    m_saverDatetimeBtn->setChecked(b);
    m_saverDatetimeBtn->blockSignals(false);
}

void ScreensaverUi::setPhotoPath(QStringList paths)
{
    QSet<QString> oldSet = m_currentPhotoSource.toSet();
    QSet<QString> newSet = paths.toSet();
    QStringList deleteSource = (oldSet - newSet).toList();
    QStringList addSource = (newSet - oldSet).toList();
    if (!deleteSource.isEmpty()) {
        for (const QString& folderPath : deleteSource) {
            for (int i = 0; i < m_photoSourceLayout->count(); ++i) {
                QLayoutItem* item = m_photoSourceLayout->itemAt(i);
                if (item && item->widget()) {
                    if (item->widget() && folderPath == item->widget()->objectName()) {
                        m_photoSourceLayout->removeItem(item);
                        item->widget()->deleteLater();
                    }
                }
            }
        }
    }

    int bgIndex = addSource.indexOf(DEFAULT_PHOTO_PATH);
    if (bgIndex != -1) {
        addSource.move(bgIndex, 0);
    }

    qDebug() << "m_currentPhotoSource = " << paths;
    if (!addSource.isEmpty()) {
        for (const QString& folderPath : addSource) {
            QString imagePath =getFirstImageFileIterator(folderPath);
            if (!imagePath.isEmpty()) {
                bool bSystemCorver = (folderPath == DEFAULT_PHOTO_PATH) ? true : false;
                QPixmap coverPixmap;
                if (bSystemCorver) {
                    QString thumbnailPath = mapToThumbnailPath(imagePath);
                    coverPixmap = loadFromFile(thumbnailPath);
                } else {
                    coverPixmap = loadFromFile(imagePath);
                }
                QWidget *photoCorverWidget = new QWidget(m_photoSourceFrame);
                PhotoCorver *photoCorver = new PhotoCorver(photoCorverWidget, bSystemCorver, false);
                photoCorver->setBackground(coverPixmap);
                photoCorverWidget->setObjectName(folderPath);
                photoCorver->setObjectName(folderPath);
                connect(photoCorver, &PhotoCorver::photoCorverLeftClicked, this, [=]() {
                    Q_EMIT photoSelectPathChanged(folderPath);
                });
                connect(photoCorver, &PhotoCorver::openFileFolderClicked, this, [=]() {
                    Q_EMIT openFileFolderClicked(folderPath);
                });
                connect(photoCorver, &PhotoCorver::removePathClicked, this, [=]() {
                    if (!bSystemCorver) {
                        Q_EMIT removePhotoPathClicked(folderPath);
                    }
                });
                QVBoxLayout *photoCorverLayout = new QVBoxLayout(photoCorverWidget);
                photoCorverLayout->setContentsMargins(0, 0, 0, 0);
                photoCorverLayout->setSpacing(4);
                KLabel *photoCorverLabel = new KLabel(photoCorverWidget);
                photoCorverLabel->setFixedWidth(photoCorver->width());
                
                if (bSystemCorver) {
                    photoCorverLabel->setText(tr("System wallpaper gallery"));
                    photoCorverLabel->setAlignment(Qt::AlignCenter);//居中
                } else {
                    QDir dir(folderPath);
                    photoCorverLabel->setText(dir.dirName());
                    QFontMetrics fm(photoCorverLabel->font());
                    if (fm.horizontalAdvance(dir.dirName()) <= photoCorverLabel->width()) {
                        photoCorverLabel->setAlignment(Qt::AlignCenter);//居中
                    }
                }
                photoCorverLayout->addWidget(photoCorver);
                photoCorverLayout->addWidget(photoCorverLabel);
                m_photoSourceLayout->addWidget(photoCorverWidget);
            }
        }
    }
    m_currentPhotoSource = paths;
}

void ScreensaverUi::setPhotoSelectPath(QString path)
{
    m_sSelectPhotoSource = path;
    for (int i = 0; i < m_photoSourceLayout->count(); ++i) {
        QLayoutItem* item = m_photoSourceLayout->itemAt(i);
        if (item && item->widget()) {
            if (item->widget() && path == item->widget()->objectName()) {
                PhotoCorver* photoCover = item->widget()->findChild<PhotoCorver*>();
                if (photoCover) {
                    photoCover->setClickStatus(true);
                }
            } else {
                PhotoCorver* photoCover = item->widget()->findChild<PhotoCorver*>();
                if (photoCover) {
                    photoCover->setClickStatus(false);
                }
            }
        }
    }
    if (path == DEFAULT_PHOTO_PATH) {
        path = LOCAL_WALLPAPER_PATH;
    }
    m_albunWidget->setDefaultPath(path);
}

void ScreensaverUi::sendSignals()
{
    connect(m_idleTimeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        Q_EMIT idleTimeChanged(m_idleTimeBox->currentData().toInt());
    });
    connect(programWidget->comboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        QString type = programWidget->comboBox()->currentData().toString();
        Q_EMIT screensaverChanged(type);
        if (type == "ukui") {
            m_bCustomSaver = false;
        } else if (type == "custom") {
            m_bCustomSaver = true;
        }
        customizeNeedHide(m_bHideCustom);
    });
    connect(sourceButton, &QPushButton::clicked, this, [=]() {
        QString filter;
        filter = tr("Wallpaper files(*.jpg *.jpeg *.bmp *.dib *.png *.jfif *.jpe *.gif *.tif *.tiff *.wdp *.svg)");
        QFileDialog fd(this);

        QList<QUrl> usb_list = fd.sidebarUrls();
        int sidebarNum = 8;// 最大添加U盘数，可以自己定义
        QString home_path = QDir::homePath().section("/", -1, -1);
        QString mnt = "/media/" + home_path + "/";
        QDir mntDir(mnt);
        mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList file_list = mntDir.entryInfoList();
        QList<QUrl> mntUrlList;
        for (int i = 0; i < sidebarNum && i < file_list.size(); ++i) {
            QFileInfo fi = file_list.at(i);
            mntUrlList << QUrl("file://" + fi.filePath());
        }

        QFileSystemWatcher m_fileSystemWatcher(&fd);
        m_fileSystemWatcher.addPath("/media/" + home_path + "/");
        connect(&m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, &fd,
                [=, &sidebarNum, &mntUrlList, &usb_list, &fd](const QString path) {
            QDir m_wmntDir(path);
            m_wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
            QFileInfoList m_wfilist = m_wmntDir.entryInfoList();
            mntUrlList.clear();
            for (int i = 0; i < sidebarNum && i < m_wfilist.size(); ++i) {
                QFileInfo m_fi = m_wfilist.at(i);
                mntUrlList << QUrl("file://" + m_fi.filePath());
            }
            fd.setSidebarUrls(usb_list + mntUrlList);
            fd.update();
        });

        connect(&fd, &QFileDialog::finished, &fd, [=, &usb_list, &fd]() {
            fd.setSidebarUrls(usb_list);
        });

        fd.setDirectory(QString(sourcePathLine->text()));
        fd.setAcceptMode(QFileDialog::AcceptOpen);
        fd.setViewMode(QFileDialog::List);
        fd.setNameFilter(filter);
        fd.setFileMode(QFileDialog::Directory);
        fd.setWindowTitle(tr("select custom screensaver dir"));
        fd.setLabelText(QFileDialog::Accept, tr("Select"));
        fd.setLabelText(QFileDialog::LookIn, tr("Position: "));
        fd.setLabelText(QFileDialog::FileName, tr("FileName: "));
        fd.setLabelText(QFileDialog::FileType, tr("FileType: "));
        fd.setLabelText(QFileDialog::Reject, tr("Cancel"));

        fd.setSidebarUrls(usb_list + mntUrlList);

        if (fd.exec() != QDialog::Accepted) {
            return;
        }
        QString selectedfile;
        selectedfile = fd.selectedFiles().first();
        if(selectedfile != "") {  //非点击【取消】时
            setSourcePath(selectedfile);
            Q_EMIT customPathChanged(selectedfile);
        }
    });
    connect(switchModeGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [=](int id) {
        if (id == 0) {
            Q_EMIT switchRandomChanged(false);
        } else {
            Q_EMIT switchRandomChanged(true);
        }
    });
    connect(switchTimeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        Q_EMIT switchTimeChanged(switchTimeComboBox->currentData().toInt());
    });
    connect(m_photoFrequencyWidget->comboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        Q_EMIT photoSwitchTimeChanged(m_photoFrequencyWidget->comboBox()->currentData().toInt());
    });
    connect(textLineEdit, &QLineEdit::textChanged, this, [=](){
        Q_EMIT customTextChanged(textLineEdit->text());
    });
    connect(textPositionGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [=](int id) {
        if (id == 0) {
            Q_EMIT textCenteredChanged(false);
        } else {
            Q_EMIT textCenteredChanged(true);
        }
    });
    connect(showUkuiTimeBtn, &KSwitchButton::stateChanged, this, [=]() {
        Q_EMIT showUkuiTimeChanged(showUkuiTimeBtn->isChecked());
    });
    connect(showCustomTimeBtn, &KSwitchButton::stateChanged, this, [=]() {
        Q_EMIT showCustomChanged(showCustomTimeBtn->isChecked());
    });
    connect(lockButton, &KSwitchButton::stateChanged, this, [=]() {
        Q_EMIT lockscreenChanged(lockButton->isChecked());
    });
    connect(m_saverDatetimeBtn, &KSwitchButton::stateChanged, this, [=]() {
        Q_EMIT saverDatetimeEnableChanged(m_saverDatetimeBtn->isChecked());
    });
    connect(m_defaulSaverWidget, &SaverThumbnailWidget::defaultSaverModeClicked, this, [=]() {
        subModeChanged(0);
        setSubMode(0);
    });
    connect(m_weatherThumbnailWidget, &WeatherThumbnailWidget::weatherModeClicked, this, [=]() {
        subModeChanged(1);
        setSubMode(1);
    });
    connect(m_musicWidget, &MusicThumbnailWidget::musicModeClicked, this, [=]() {
        subModeChanged(2);
        setSubMode(2);
    });
    connect(m_albunWidget, &AlbumThumbnailWidget::albumModeClicked, this, [=]() {
        subModeChanged(3);
        setSubMode(3);
    });
    connect(m_weatherSettingBtn, &QPushButton::clicked, this, [=]() {
        QProcess::startDetached("kylin-weather", QStringList("showmainwindow"));
    });
    connect(m_followSystemBtn, &QRadioButton::clicked, this, [=]() {
        if (m_followSystemBtn->isChecked()) {
            musicFollowSystemChanged(m_followSystemBtn->isChecked());
            m_customMusicBtn->setChecked(!m_followSystemBtn->isChecked());
        } else {
            m_followSystemBtn->setChecked(true);
        }
    });
    connect(m_customMusicBtn, &QRadioButton::clicked, this, [=]() {
        if (m_customMusicBtn->isChecked()) {
            musicFollowSystemChanged(!m_customMusicBtn->isChecked());
            m_followSystemBtn->setChecked(!m_customMusicBtn->isChecked());
        } else {
            m_customMusicBtn->setChecked(true);
        }
    });
    connect(m_orderBtn, &QRadioButton::clicked, this, [=]() {
        int mode = m_orderBtn->isChecked() ? 1 : 0;
        photoCycleModeChanged(!mode);
        m_randomBtn->setChecked(!m_orderBtn->isChecked());
    });
    connect(m_randomBtn, &QRadioButton::clicked, this, [=]() {
        int mode = m_randomBtn->isChecked() ? 1 : 0;
        photoCycleModeChanged(mode);
        m_orderBtn->setChecked(!m_randomBtn->isChecked());
    });
    connect(m_musicSourceBtn, &QPushButton::clicked, this, [=]() {
        setMusicSelectSource();
    });
    connect(m_photoSourceBtn, &QPushButton::clicked, this, [=]() {
        setPhotoSelectSource();
    });
}

void ScreensaverUi::setPhotoSelectSource()
{
    bool bPhotoSourceBtnShow = m_photoSourceLayout->count() > 49 ? true : false;
    if (bPhotoSourceBtnShow) {
        QMessageBox msg(this);
        msg.setIcon(QMessageBox::Icon::Information);
        msg.setIconPixmap(QIcon::fromTheme("dialog-info").pixmap(24,24));
        msg.setText(tr("The maximum limit for the number of images in the folder has been reached; you can remove some before continuing to add more."));
        msg.exec();
    } else {
        QString filter;
        filter = tr("Wallpaper files(*.jpg *.jpeg *.bmp *.dib *.png *.jfif *.jpe *.gif *.tif *.tiff *.wdp *.svg)");
        QFileDialog fd(this);

        QList<QUrl> usb_list = fd.sidebarUrls();
        int sidebarNum = 8;// 最大添加U盘数，可以自己定义
        QString home_path = QDir::homePath().section("/", -1, -1);
        QString mnt = "/media/" + home_path + "/";
        QDir mntDir(mnt);
        mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList file_list = mntDir.entryInfoList();
        QList<QUrl> mntUrlList;
        for (int i = 0; i < sidebarNum && i < file_list.size(); ++i) {
            QFileInfo fi = file_list.at(i);
            mntUrlList << QUrl("file://" + fi.filePath());
        }

        QFileSystemWatcher m_fileSystemWatcher(&fd);
        m_fileSystemWatcher.addPath("/media/" + home_path + "/");
        connect(&m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, &fd,
                [=, &sidebarNum, &mntUrlList, &usb_list, &fd](const QString path) {
            QDir m_wmntDir(path);
            m_wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
            QFileInfoList m_wfilist = m_wmntDir.entryInfoList();
            mntUrlList.clear();
            for (int i = 0; i < sidebarNum && i < m_wfilist.size(); ++i) {
                QFileInfo m_fi = m_wfilist.at(i);
                mntUrlList << QUrl("file://" + m_fi.filePath());
            }
            fd.setSidebarUrls(usb_list + mntUrlList);
            fd.update();
        });

        connect(&fd, &QFileDialog::finished, &fd, [=, &usb_list, &fd]() {
            fd.setSidebarUrls(usb_list);
        });

        QString initialDir = m_sSelectPhotoSource;
        if (initialDir.isEmpty() || !QDir(initialDir).exists()) {
            initialDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        }
        if (!initialDir.isEmpty() && QDir(initialDir).exists()) {
            fd.setDirectory(initialDir);
        }
        fd.setAcceptMode(QFileDialog::AcceptOpen);
        fd.setViewMode(QFileDialog::List);
        fd.setNameFilter(filter);
        fd.setFileMode(QFileDialog::DirectoryOnly);
        fd.setWindowTitle(tr("select custom screensaver dir"));
        fd.setLabelText(QFileDialog::Accept, tr("Select"));
        fd.setLabelText(QFileDialog::LookIn, tr("Position: "));
        fd.setLabelText(QFileDialog::FileName, tr("FileName: "));
        fd.setLabelText(QFileDialog::FileType, tr("FileType: "));
        fd.setLabelText(QFileDialog::Reject, tr("Cancel"));
        fd.setSidebarUrls(usb_list + mntUrlList);
        if (fd.exec() != QDialog::Accepted) {
            return;
        }

        QString selectedfile = fd.selectedFiles().first();
        QString imagePath =getFirstImageFileIterator(selectedfile);
        if (imagePath.isEmpty()) {
            QMessageBox msg(this);
            msg.setIcon(QMessageBox::Icon::Information);
            msg.setIconPixmap(QIcon::fromTheme("dialog-info").pixmap(24,24));
            msg.setText(tr("There are no image resources in the selected folder, please try again!"));
            msg.exec();
            return;
        }
        if(!selectedfile.isEmpty()) {
            Q_EMIT photoSourceChanged(selectedfile);
        }
    }
}

void ScreensaverUi::setMusicSelectSource()
{
    QString filter;
    filter = tr("Music files(*.mp3 *.wav *.ogg *.flac *.aac *.m4a *.wma *.opus *.aiff .ape)");
    QFileDialog fd(this);

    QList<QUrl> usb_list = fd.sidebarUrls();
    int sidebarNum = 8;// 最大添加U盘数，可以自己定义
    QString home_path = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home_path + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList file_list = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < file_list.size(); ++i) {
        QFileInfo fi = file_list.at(i);
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher m_fileSystemWatcher(&fd);
    m_fileSystemWatcher.addPath("/media/" + home_path + "/");
    connect(&m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, &fd,
            [=, &sidebarNum, &mntUrlList, &usb_list, &fd](const QString path) {
        QDir m_wmntDir(path);
        m_wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList m_wfilist = m_wmntDir.entryInfoList();
        mntUrlList.clear();
        for (int i = 0; i < sidebarNum && i < m_wfilist.size(); ++i) {
            QFileInfo m_fi = m_wfilist.at(i);
            mntUrlList << QUrl("file://" + m_fi.filePath());
        }
        fd.setSidebarUrls(usb_list + mntUrlList);
        fd.update();
    });

    connect(&fd, &QFileDialog::finished, &fd, [=, &usb_list, &fd]() {
        fd.setSidebarUrls(usb_list);
    });

    QString initialDir = QString(m_musicPathLine->text());
    if (initialDir.isEmpty() || !QDir(initialDir).exists()) {
        initialDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    }
    if (!initialDir.isEmpty() && QDir(initialDir).exists()) {
        fd.setDirectory(initialDir);
    }
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilter(filter);
    fd.setFileMode(QFileDialog::Directory);
    fd.setWindowTitle(tr("select screensaver music dir"));
    fd.setLabelText(QFileDialog::Accept, tr("Select"));
    fd.setLabelText(QFileDialog::LookIn, tr("Position: "));
    fd.setLabelText(QFileDialog::FileName, tr("FileName: "));
    fd.setLabelText(QFileDialog::FileType, tr("FileType: "));
    fd.setLabelText(QFileDialog::Reject, tr("Cancel"));

    fd.setSidebarUrls(usb_list + mntUrlList);

    if (fd.exec() != QDialog::Accepted) {
        return;
    }
    QString selectedfile;
    selectedfile = fd.selectedFiles().first();
    if(selectedfile != "") {  //非点击【取消】时
        setMusicPath(selectedfile);
        Q_EMIT musicPathChanged(selectedfile);
    }
}
