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
#include "datetimeui.h"
#include <QApplication>
#include <QFontMetrics>
#include <kysdk/applications/accessinfohelper.h>

using namespace kdk;

const QStringList linkShanghaiTzList = {
    "Asia/Beijing",
    "Asia/Urumqi"
};

DatetimeUi::DatetimeUi()
{
    setAttribute(Qt::WA_DeleteOnClose);
    uiLayout            = new QVBoxLayout(this);
    currentDateLayout   = new QVBoxLayout(this);
    otherTimezoneLayout = new QVBoxLayout(this);
    uiLayout->addLayout(currentDateLayout);
    uiLayout->addLayout(otherTimezoneLayout);
    uiLayout->addStretch();
    uiLayout->setSpacing(40);
    uiLayout->setMargin(0);
    currentDateLayout->setSpacing(8);
    initUi();
}

DatetimeUi::~DatetimeUi()
{

}

void DatetimeUi::initUi()
{
    initObject();
    initCurrentDate();
    initOtherTimezone();
    initSignals();

    // KDK_EXTEND 之后设置 accessibleName / setBuddy
    manualRadioButton->setAccessibleName(tr("Manual Time"));
    syncRadioButton->setAccessibleName(tr("Sync Time"));
    changeTZbutton->setAccessibleName(tr("Timezone"));
    addTimezoneButton->setAccessibleName(tr("Add"));
    setNtpLabel->setAccessibleName(tr("Server Address"));
    setNtpLabel->setBuddy(setNtpLineEdit);
    setNtpLineEdit->setAccessibleName(tr("Server Address"));
    setNtpButton->setAccessibleName(tr("Save"));
}

void DatetimeUi::initObject()
{
    // 当前时间
    currentTitleLabel = new KLabel(this); // 标题
    KDK_EXTEND_ALL_INFO_FORMAT(currentTitleLabel, "Datetime", "", "");
    currentTitleLabel->setContentsMargins(16, 0, 0, 0);
    clockWidget       = new ClockWidget(); // 时钟
    timeLabel         = new TimeLabel(this); // 时间
    KDK_EXTEND_ALL_INFO_FORMAT(timeLabel, "Datetime", "", "");
    mDateLabel        = new KLabel(this); // 日期
    KDK_EXTEND_ALL_INFO_FORMAT(mDateLabel, "Datetime", "", "");
    changeTZbutton    = new KPushButton(this); // 修改时区
    KDK_EXTEND_ALL_INFO_FORMAT(changeTZbutton, "Datetime", "", "");
    setTimeModeLabel  = new KLabel(this);  // 设置时间
    KDK_EXTEND_ALL_INFO_FORMAT(setTimeModeLabel, "Datetime", "", "");
    manualRadioButton = new QRadioButton(this); // 手动设置时间
    KDK_EXTEND_ALL_INFO_FORMAT(manualRadioButton, "Datetime", "", "");
    syncRadioButton   = new QRadioButton(this); // 自动同步时间
    KDK_EXTEND_ALL_INFO_FORMAT(syncRadioButton, "Datetime", "", "");
    syncResultLabel   = new KLabel(this); // 同步时间结果
    KDK_EXTEND_ALL_INFO_FORMAT(syncResultLabel, "Datetime", "", "");
    m_fontSetting = new QGSettings("org.ukui.style", QByteArray(), this);
    connect(m_fontSetting, &QGSettings::changed, this, [=](const QString &key) {
        styleSettingChangeSlot(key);
    });
    setDateLabel      = new KLabel(this); // 手动设置日期
    KDK_EXTEND_ALL_INFO_FORMAT(setDateLabel, "Datetime", "", "");
    dategroupWidget   = new DategroupWidget(this);
    timeBtnGroup      = new QButtonGroup(this);
    //~ contents_path /Date/Sync Server
    ntpSerWidget      = new ComboxWidget(tr("Sync Server"), this);
    KDK_EXTEND_ALL_INFO_FORMAT(ntpSerWidget, "Datetime", "", "");
    setNtpLabel       = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(setNtpLabel, "Datetime", "", "");
    setNtpLineEdit    = new QLineEdit(this);
    KDK_EXTEND_ALL_INFO_FORMAT(setNtpLineEdit, "Datetime", "", "");
    setNtpButton      = new KPushButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(setNtpButton, "Datetime", "", "");

    timeBtnGroup->addButton(manualRadioButton, 0);
    timeBtnGroup->addButton(syncRadioButton, 1);
    syncResultLabel->setStyleSheet("QLabel{color: #D9F82929;}");

    otherTimezoneTitleLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(otherTimezoneTitleLabel, "Datetime", "", "");
    otherTimezoneTitleLabel->setContentsMargins(16, 0, 0, 0);
    addTimezoneButton       = new AddButton(this, UkccFrame::BorderRadiusStyle::Around);
    KDK_EXTEND_ALL_INFO_FORMAT(addTimezoneButton, "Datetime", "", "");
    //~ contents_path /Date/Add
    tr("Add");
}

void DatetimeUi::initCurrentDate()
{
    SettingGroup *currentDateGroup  = new SettingGroup(this);
    UkccFrame    *clockFrame        = new UkccFrame(currentDateGroup);
    UkccFrame    *setTimeFrame      = new UkccFrame(currentDateGroup, UkccFrame::BorderRadiusStyle::Around, true);
    UkccFrame    *setDateFrame      = new UkccFrame(currentDateGroup, UkccFrame::BorderRadiusStyle::Around, true);
    UkccFrame    *ntpAddressFrame   = new UkccFrame(currentDateGroup, UkccFrame::BorderRadiusStyle::Around, true);

    //~ contents_path /Date/Current Date
    currentTitleLabel->setText(tr("Current Date"));
    currentDateLayout->addWidget(currentTitleLabel);
    currentDateLayout->addWidget(currentDateGroup);
    currentDateGroup->addWidget(clockFrame);
    currentDateGroup->addWidget(setTimeFrame);
    currentDateGroup->addWidget(setDateFrame);
    currentDateGroup->addWidget(ntpSerWidget);
    currentDateGroup->addWidget(ntpAddressFrame);

    QHBoxLayout *clockLayout      = new QHBoxLayout(clockFrame);
    UkccFrame   *clockRightFrame  = new UkccFrame(clockFrame);
    QVBoxLayout *clockRightLayout = new QVBoxLayout(clockRightFrame);
    UkccFrame   *dataZoneFrame    = new UkccFrame(clockRightFrame);
    QHBoxLayout *dataZoneLayout   = new QHBoxLayout(dataZoneFrame);
    clockLayout->addWidget(clockWidget);
    clockLayout->addWidget(clockRightFrame);
    clockLayout->setSpacing(26);
    clockRightLayout->setMargin(0);
    clockRightLayout->addWidget(timeLabel);
    clockRightLayout->addWidget(dataZoneFrame);
    dataZoneLayout->setMargin(0);
    dataZoneLayout->setSpacing(16);
    dataZoneLayout->addWidget(mDateLabel);
    dataZoneLayout->addWidget(changeTZbutton);
    dataZoneLayout->addStretch();
    clockFrame->setFixedHeight(156);
    clockRightFrame->setFixedHeight(102);
    mDateLabel->setMinimumWidth(280);
    changeTZbutton->setFixedWidth(150);
    //~ contents_path /Date/Timezone
    changeTZbutton->setText(tr("Timezone"));

    QHBoxLayout *setTimeLayout = new QHBoxLayout(setTimeFrame);
    QSpacerItem *setTimeSpacer = new QSpacerItem(10, 24, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    setTimeModeLabel->setFixedWidth(136);
    //~ contents_path /Date/Set Time
    setTimeModeLabel->setText(tr("Set Time"));
    //~ contents_path /Date/Manual Time
    manualRadioButton->setText(tr("Manual Time"));
    //~ contents_path /Date/Sync Time
    syncRadioButton->setText(tr("Sync Time"));
    setTimeLayout->setContentsMargins(16, 0, 0, 0);
    setTimeLayout->setSpacing(16);
    setTimeLayout->addWidget(setTimeModeLabel);
    setTimeLayout->addWidget(manualRadioButton);
    setTimeLayout->addSpacerItem(setTimeSpacer);
    setTimeLayout->addWidget(syncRadioButton);
    setTimeLayout->addWidget(syncResultLabel);
    setTimeLayout->addStretch();

    QHBoxLayout *setDateLayout  = new QHBoxLayout(setDateFrame);
    //~ contents_path /Date/Set Time Manually
    setDateLabel->setText(tr("Set Time Manually"));
    setDateLabel->setFixedWidth(136);
    setDateLayout->setSpacing(16);
    setDateLayout->setContentsMargins(16, 0, 0, 0);
    setDateLayout->addWidget(setDateLabel);
    setDateLayout->addWidget(dategroupWidget);

    connect(timeLabel, &TimeLabel::dateChanged, mDateLabel, [=](){
        updateDate();
    });
    connect(timeBtnGroup, QOverload<int, bool>::of(&QButtonGroup::buttonToggled), this, [=](int id, bool clicked){
        if (clicked && id != toggledBtn) {
            previousToggledBtn = toggledBtn;
            toggledBtn = id;
        }
        if ((id == 0 && clicked) || (id == 1 && !clicked)) {
            ntpAddressFrame->hide();
            ntpSerWidget->hide();
            setDateFrame->show();
        } else {
            setDateFrame->hide();
            ntpSerWidget->show();
            if (ntpSerWidget->comboBox()->currentIndex() == (ntpSerWidget->comboBox()->count() - 1)) {
                ntpAddressFrame->show();
            } else {
                ntpAddressFrame->hide();
            }
        }
    });

    ntpSerWidget->setTitleFixedWidth(136);
    QHBoxLayout *ntpAddresslayout = new QHBoxLayout(ntpAddressFrame);
    ntpAddresslayout->setContentsMargins(16, 8, 16, 8);
    ntpAddresslayout->setSpacing(16);
    ntpAddresslayout->addWidget(setNtpLabel);
    ntpAddresslayout->addWidget(setNtpLineEdit);
    ntpAddresslayout->addWidget(setNtpButton);
    setNtpLabel->setText(tr("Server Address"));
    setNtpLabel->setFixedWidth(136);
    setNtpLineEdit->setPlaceholderText(tr("Required"));
    setNtpButton->setText(tr("Save"));
    setNtpButton->setEnabled(false); // 初始状态不可点击

    connect(ntpSerWidget->comboBox(), &QComboBox::currentTextChanged, this, [=](QString text){
        if (text == tr("Customize") && !ntpSerWidget->isHidden()) {
            setNtpLineEdit->clear();
            ntpAddressFrame->show();
        } else {
            ntpAddressFrame->hide();
        }
    });
    connect(setNtpLineEdit, &QLineEdit::textChanged, this, [=](){
        setNtpLineEdit->blockSignals(true);
        while (setNtpLineEdit->text().front() == " ") {
            setNtpLineEdit->setText(setNtpLineEdit->text().remove(0, 1)); //去掉首空格
        }
        setNtpLineEdit->blockSignals(false);
        setNtpButton->setEnabled(!setNtpLineEdit->text().isEmpty());   //为空时不允许保存
    });
    QDBusConnection::sessionBus().connect("com.kylin.kysdk.DateServer",
                                          "/com/kylin/kysdk/Date",
                                          "com.kylin.kysdk.DateInterface",
                                          "LongDateSignal",
                                          this,
                                          SLOT(updateDateFormat(QString)));
}

void DatetimeUi::setTimeFrameEnable(bool enable) {
    dategroupWidget->setEnabled(enable);
    setDateLabel->setEnabled(enable);
}

void DatetimeUi::initOtherTimezone()
{
    //~ contents_path /Date/Other Timezone
    otherTimezoneTitleLabel->setText(tr("Other Timezone"));
    otherTimezoneLayout->setSpacing(0);
    otherTimezoneLayout->addWidget(otherTimezoneTitleLabel);
    otherTimezoneLayout->addSpacing(8);
    otherTimezoneLayout->addWidget(addTimezoneButton);
    otherTimezoneLayout->addStretch();
}

void DatetimeUi::styleSettingChangeSlot(const QString &key)
{
    if (key != "systemFont" && key != "systemFontSize") {
        return;
    }
    if (!syncResultLabel || syncResultLabel->text().isEmpty()) {
        return;
    }
    syncResultLabel->setFont(qApp->font());
    QFontMetrics fm(syncResultLabel->font());
    syncResultLabel->setFixedWidth(fm.horizontalAdvance(syncResultLabel->text()) + 4);
}

void DatetimeUi::initSignals()
{
    connect(dategroupWidget, &DategroupWidget::dateChanged, this, &DatetimeUi::dateTimeChanged);
    connect(timeBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [=](int id){
        if (id == 0) {
            syncResultLabel->setPixmap(QPixmap(""));
            syncResultLabel->setText("");
            Q_EMIT timeModeChanged(previousToggledBtn, "manual");
        } else {
            Q_EMIT timeModeChanged(previousToggledBtn, "automatic");
        }
    });
    connect(changeTZbutton, &KPushButton::clicked, this, &DatetimeUi::timezoneButtonClicked);
    connect(ntpSerWidget->comboBox(), QOverload<const QString&>::of(&QComboBox::activated), this, [=](QString server){
        if (ntpSerWidget->comboBox()->currentIndex() == 0) {
            server = "default";
            Q_EMIT ntpServerChanged(server);
        } else if (ntpSerWidget->comboBox()->currentIndex() == ntpSerWidget->comboBox()->count() - 1) {

        } else {
            Q_EMIT ntpServerChanged(server);
        }
    });
    connect(setNtpButton, &KPushButton::clicked, this, [=](){
        Q_EMIT ntpServerChanged(setNtpLineEdit->text());
    });
    connect(addTimezoneButton, &AddButton::clicked, this, &DatetimeUi::addTimezoneButtonClicked);
}

void DatetimeUi::updateDate()
{
    updateDateFormat(kdk_system_get_longformat_date());
}

void DatetimeUi::updateDateFormat(QString date)
{
    QString timeAndWeek;
    int longindex = 0;
    QString mLongFormatDate = date;
    if ((mLongFormatDate.at(mLongFormatDate.length() - 4) == ',')
            || ((mLongFormatDate.at(2) < '0' || mLongFormatDate.at(2) > '9')
                && (mLongFormatDate.at(2) < 'a' || mLongFormatDate.at(2) > 'z')))
    {
        longindex = 1;
     }

    QLocale l1ocale = QLocale::system();
    if ("zh_CN" == l1ocale.name()){
        l1ocale = QLocale::Chinese;
    } else if ("bo_CN" == l1ocale.name()){
        l1ocale = QLocale::Tibetan;
    } else if ("mn_MN" == l1ocale.name()) {
        l1ocale = QLocale::Mongolian;
    } else {
        l1ocale = QLocale::English;
    }
    qDebug() << Q_FUNC_INFO << "updateDateFormat date" << date << "longindex" << longindex << "1ocale.name" << l1ocale.name();
    if (longindex) {
        if ("mn_MN" == l1ocale.name()) {
            timeAndWeek = l1ocale.toString(QDateTime::currentDateTime(), tr("MMMM d, yy")).replace("周","星期");
        } else {
            timeAndWeek = l1ocale.toString(QDateTime::currentDateTime(), tr("MMMM d, yy ddd")).replace("周","星期");
        }
    } else {
        if ("mn_MN" == l1ocale.name()) {
            timeAndWeek = l1ocale.toString(QDateTime::currentDateTime(), tr("MMMM dd, yyyy")).replace("周","星期");
        } else {
            timeAndWeek = l1ocale.toString(QDateTime::currentDateTime(), tr("MMMM dd, yyyy ddd")).replace("周","星期");
        }
    }

    if (!timeAndWeek.isEmpty() && !timezoneStr.isEmpty()) {
        mDateLabel->setText(timeAndWeek + "  " + timezoneStr);
    }
}

void DatetimeUi::setTimeMode(QString mode)
{
    if (mode == "manual") {
        manualRadioButton->setChecked(true);
        previousToggledBtn = 0;
        toggledBtn = 0;
    } else {
        syncRadioButton->setChecked(true);
        previousToggledBtn = 1;
        toggledBtn = 1;
    }
}

void DatetimeUi::setSyncResultLabel(int flag)
{
    if (flag == 0) { // 同步中，显示图标
        static quint32 times = 0;
        quint8 picNum = times - qFloor(times/8)*8; //限制在0~7
        QString pixName = QString(":/images/loading%1.svg").arg(picNum+10);
        QPixmap pix(pixName);
        qApp->processEvents();
        syncResultLabel->setText("");
        syncResultLabel->setPixmap(pix);
        syncResultLabel->setFixedWidth(pix.width());
        syncResultLabel->update();
        times++;
    } else if (flag == 1) { // 失败，显示文字
        syncResultLabel->setPixmap(QPixmap(""));
        syncResultLabel->setText(tr("Sync Failed"));
        syncResultLabel->setFontColor(Qt::red);
        syncResultLabel->setFont(qApp->font());
        QFontMetrics fm(syncResultLabel->font());
        syncResultLabel->setFixedWidth(fm.horizontalAdvance(syncResultLabel->text()) + 4);
    } else {
        syncResultLabel->setPixmap(QPixmap("")); //成功，不显示任何内容
        syncResultLabel->setText("");
    }
}

void DatetimeUi::setNtpServers(QStringList sl)
{
    ntpSerWidget->comboBox()->clear();
    sl.insert(0, tr("Default"));
    sl.append(tr("Customize"));
    ntpSerWidget->comboBox()->addItems(sl);
}

void DatetimeUi::setNtpServer(QString s)
{
    if (s.isEmpty()) { // 系统默认
        ntpSerWidget->setCurrentIndex(0);
    } else {
        bool isContains = false;
        for (int i = 1; i < ntpSerWidget->comboBox()->count() - 1; ++i) {
            if (ntpSerWidget->comboBox()->itemText(i) == s) {
                ntpSerWidget->setCurrentIndex(i);
                isContains = true; // 列表中
                break;
            }
        }
        if (!isContains) { // 自定义
            ntpSerWidget->setCurrentIndex(ntpSerWidget->comboBox()->count() - 1);
            setNtpLineEdit->blockSignals(true);
            setNtpLineEdit->setText(s);
            setNtpLineEdit->blockSignals(false);
        }
    }
}

void DatetimeUi::setTimezoneStr(QString str)
{
    timezoneStr = str;
    updateDate();
}

void DatetimeUi::addOtherTimezone(const QString &timezone, const QString &timezoneName)
{
    TimeBtn *timeBtn = nullptr;
    if (linkShanghaiTzList.contains(timezone)) {
        timeBtn = new TimeBtn("Asia/Shanghai",timezone);
    } else {
        timeBtn = new TimeBtn(timezone);
    }
    timeBtn->setParent(this);
    QSpacerItem *spaceIterm = new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Fixed);
    int index = otherTimezoneLayout->count() - 2; // button and Stretch
    otherTimezoneLayout->insertWidget(index, timeBtn);
    otherTimezoneLayout->insertSpacerItem(index + 1, spaceIterm);
    connect(timeBtn, &TimeBtn::deleted, this, [=](){
        otherTimezoneLayout->removeItem(spaceIterm);
        delete spaceIterm;
        Q_EMIT timezoneRemoved(timezone);
    });
}

void DatetimeUi::setAddTimezoneBtnEnabled(bool b)
{
    addTimezoneButton->setEnabled(b);
}

void DatetimeUi::resetDateFormat() {
    dategroupWidget->resetDateEdit(kdk_system_get_shortformat());
}

void DatetimeUi::resetDateTime()
{
    dategroupWidget->updateTime(true);
}
