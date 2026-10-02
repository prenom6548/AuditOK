#include "touchscreenwidget.h"

#include <QDebug>
#include <QGSettings>

#define CONTENTS_MARGINS 0, 0, 0, 0
#define ITEM_MARGINS 16, 0, 16, 0
#define FRAME_MIN_SIZE 550, 60
#define FRAME_MAX_SIZE 16777215, 16777215
#define CONTECT_FRAME_MAX_SIZE 16777215, 60
#define FRAME_MIN_SIZE 550, 60
#define TALL_CONTECT_FRAME_MAX_SIZE 16777215, 80
#define TALL_FRAME_MIN_SIZE 550, 80

#define LABLE_MIN_WIDTH 280
#define LONGLABEL_MIN_WIDTH 490
#define COMBOBOX_MIN_WIDTH 200
#define LINE_MAX_SIZE 16777215, 1
#define LINE_MIN_SIZE 0, 1
#define PUSHBUTTTON_FIXED_WIDTH 96

TouchScreenWidget::TouchScreenWidget(QWidget *parent)
    : QWidget(parent)
{
    m_Vlayout = new QVBoxLayout(this);
    m_Vlayout->setContentsMargins(CONTENTS_MARGINS);
    m_Vlayout->setSpacing(0);
    initUi();
    initConnection();

    this->setLayout(m_Vlayout);
    m_Vlayout->addStretch();
}

void TouchScreenWidget::initUi()
{
    m_hotspotFrame = new QFrame(this);
    m_hotspotFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_hotspotFrame->setMaximumSize(FRAME_MAX_SIZE);
    m_hotspotFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *lyt = new QVBoxLayout(this);
    lyt->setContentsMargins(CONTENTS_MARGINS);
    m_hotspotFrame->setLayout(lyt);

    m_titleLabel = new TitleLabel(this);
    m_titleLabel->setText(tr("TouchCalibrate"));

    setScreenFrame();
    setTouchscreenFrame();
    setDeviceInfoFrame();
    setSettingFrame();
    setCalibrationFrame();

    m_screenAndTouchscreenLine = new kdk::KHLineFrame(this);
    m_touchscreenAndDeviceinfoLine = new kdk::KHLineFrame(this);
    m_deviceinfoAndSettingLine = new kdk::KHLineFrame(this);
    m_settingAndCalibrationLine = new kdk::KHLineFrame(this);
//    if (env::isTabletMode()){
//        qDebug()<<"working in tablet mode";
//        screenFrame->setFixedHeight(64);
//        monitorCombo->setFixedHeight(48);
//        touchScreenFrame->setFixedHeight(64);
//        touchscreenCombo->setFixedHeight(48);
//        deviceInfoFrame->setFixedHeight(64);
//    }

//    if (env::isWayland()) {
        m_settingAndCalibrationLine->hide();
        m_calibrationFrame->hide();
//    }

    lyt->addWidget(m_screenFrame);
    lyt->addWidget(m_screenAndTouchscreenLine);
    lyt->addWidget(m_touchScreenFrame);
    lyt->addWidget(m_touchscreenAndDeviceinfoLine);
    lyt->addWidget(m_deviceInfoFrame);
    lyt->addWidget(m_deviceinfoAndSettingLine);
    lyt->addWidget(m_settingFrame);
    lyt->addWidget(m_settingAndCalibrationLine);
    lyt->addWidget(m_calibrationFrame);
    lyt->setSpacing(0);

    resetFrameSize();

    m_Vlayout->addWidget(m_titleLabel);
    m_Vlayout->addSpacing(8);
    m_Vlayout->addWidget(m_hotspotFrame);
}

void TouchScreenWidget::initConnection() {
    connect(m_monitorCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this]() {
        emit currentOutputChanged(currentOutputDevice());
    });

    connect(m_touchscreenCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this]() {
        Device device = currentTouchDevice();
        m_touchNameContent->setText(device.name);
        emit currentTouchChanged(device);
    });

    connect(m_mapButton, &QPushButton::clicked, this, &TouchScreenWidget::mapButtonClicked);
    connect(m_calibrationButton, &QPushButton::clicked, this, &TouchScreenWidget::calibratButtonClicked);
}

Device TouchScreenWidget::currentTouchDevice() const
{
    QString currentTouchScreen = m_touchscreenCombo->currentText();
    if (currentTouchScreen.isEmpty()) {
        qDebug() << "Current touchscreenCombo text is empty.";
        return Device();
    }

    int deviceId = currentTouchScreen.toInt();
    for (const Device& device : qAsConst(m_touchDevices)) {
        if (deviceId == device.deviceId)
            return device;
    }
    qWarning() << "Can't find touch device:" << deviceId << "from device list";
    return Device();
}

Device TouchScreenWidget::currentOutputDevice() const
{
    QString deviceName = m_monitorCombo->currentText();
    for (const Device& device : qAsConst(m_outputDevices)) {
        if (deviceName == device.name)
            return device;
    }
    qWarning() << "Can't find output device:" << deviceName << "from device list";
    return Device();
}

void TouchScreenWidget::resetTouchList(const QList<Device>& devices)
{
    qDebug() << "TouchCalibrate widget reset touch list";
    m_touchDevices = devices;
    m_touchscreenCombo->clear();

    for (const Device& device : qAsConst(devices)) {
        m_touchscreenCombo->addItem(QString::number(device.deviceId));
    }
}

void TouchScreenWidget::resetOutputList(const QList<Device>& devices)
{
    qDebug() << "TouchCalibrate widget reset output list";
    m_outputDevices = devices;
    m_monitorCombo->clear();

    for (const Device& device : qAsConst(devices)) {
        m_monitorCombo->addItem(device.name);
    }
}

void TouchScreenWidget::setScreenFrame()
{
    m_screenFrame = new QFrame(this);
    m_screenFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_screenFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_screenFrame->setMaximumSize(CONTECT_FRAME_MAX_SIZE);

    QHBoxLayout *screenHLayout = new QHBoxLayout(m_screenFrame);

    m_monitorLabel = new FixLabel(this);
    m_monitorLabel->setText(tr("monitor"));
    m_monitorLabel->setFixedWidth(LABLE_MIN_WIDTH);
    m_monitorCombo = new QComboBox(this);
    m_monitorCombo->setInsertPolicy(QComboBox::NoInsert);
    m_monitorCombo->setMinimumWidth(COMBOBOX_MIN_WIDTH);
    m_monitorCombo->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    screenHLayout->setContentsMargins(ITEM_MARGINS);
    screenHLayout->setSpacing(8);
    screenHLayout->addWidget(m_monitorLabel);
    screenHLayout->addWidget(m_monitorCombo);

    m_screenFrame->setLayout(screenHLayout);
}

void TouchScreenWidget::setTouchscreenFrame()
{
    m_touchScreenFrame = new QFrame(this);
    m_touchScreenFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_touchScreenFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_touchScreenFrame->setMaximumSize(CONTECT_FRAME_MAX_SIZE);

    QHBoxLayout *touchscreenHLayout = new QHBoxLayout(m_touchScreenFrame);

    m_touchLabel = new FixLabel(this);
    m_touchLabel->setText(tr("touch id"));
    m_touchLabel->setFixedWidth(LABLE_MIN_WIDTH);
    m_touchscreenCombo = new QComboBox(this);
    m_touchscreenCombo->setInsertPolicy(QComboBox::NoInsert);
    m_touchscreenCombo->setMinimumWidth(COMBOBOX_MIN_WIDTH);
    m_touchscreenCombo->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    touchscreenHLayout->setContentsMargins(ITEM_MARGINS);
    touchscreenHLayout->setSpacing(8);
    touchscreenHLayout->addWidget(m_touchLabel);
    touchscreenHLayout->addWidget(m_touchscreenCombo);

    m_touchScreenFrame->setLayout(touchscreenHLayout);
}

void TouchScreenWidget::setDeviceInfoFrame()
{
    m_deviceInfoFrame = new QFrame(this);
    m_deviceInfoFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_deviceInfoFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_deviceInfoFrame->setMaximumSize(CONTECT_FRAME_MAX_SIZE);

    QHBoxLayout *deviceInfoHLayout = new QHBoxLayout(m_deviceInfoFrame);

    m_deviceLabel = new FixLabel(this);
    m_deviceLabel->setText(tr("input device"));
    m_deviceLabel->setFixedWidth(LABLE_MIN_WIDTH);

    m_touchNameContent = new QLabel(this);

    deviceInfoHLayout->setContentsMargins(ITEM_MARGINS);
    deviceInfoHLayout->setSpacing(8);
    deviceInfoHLayout->addWidget(m_deviceLabel);
    deviceInfoHLayout->addWidget(m_touchNameContent);

    m_deviceInfoFrame->setLayout(deviceInfoHLayout);
}

void TouchScreenWidget::setSettingFrame()
{
    m_settingFrame = new QFrame(this);
    m_settingFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_settingFrame->setMinimumSize(TALL_FRAME_MIN_SIZE);
    m_settingFrame->setMaximumSize(TALL_CONTECT_FRAME_MAX_SIZE);

    QHBoxLayout *settingHLayout = new QHBoxLayout(m_settingFrame);

    m_settingLabel = new FixLabel(this);
    m_settingLabel->setAlignment(Qt::AlignLeft);
    m_settingLabel->setText(tr("Bind the selected display with touch devices to support touch operations"));
    m_settingLabel->setMinimumWidth(LONGLABEL_MIN_WIDTH);

    m_settingWarningLabel = new FixLabel(this);
    m_settingWarningLabel->setAlignment(Qt::AlignLeft);
    m_settingWarningLabel->setText(tr("The selected display needs to support setting to touch screen to avoid ineffective touch operations."));
    m_settingWarningLabel->setMinimumWidth(LONGLABEL_MIN_WIDTH);
    m_settingWarningLabel->setDisabled(true);
    m_settingWarningLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    QWidget* settingWidget = new QWidget(this);
    QVBoxLayout* settingInfoVLyt = new QVBoxLayout(settingWidget);
    settingInfoVLyt->setContentsMargins(CONTENTS_MARGINS);
    settingInfoVLyt->addStretch();
    settingInfoVLyt->addWidget(m_settingLabel);
    settingInfoVLyt->setSpacing(2);
    settingInfoVLyt->addWidget(m_settingWarningLabel);
    settingInfoVLyt->addStretch();
    settingWidget->setLayout(settingInfoVLyt);
    settingWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    m_mapButton = new FixPushButton(this);
    m_mapButton->setFixedWidth(PUSHBUTTTON_FIXED_WIDTH);
    m_mapButton->setButtonText(tr("map"));

    settingHLayout->setContentsMargins(ITEM_MARGINS);
    settingHLayout->addWidget(settingWidget);
    settingHLayout->addWidget(m_mapButton);

    m_settingFrame->setLayout(settingHLayout);
}

void TouchScreenWidget::setCalibrationFrame()
{
    m_calibrationFrame = new QFrame(this);
    m_calibrationFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_calibrationFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_calibrationFrame->setMaximumSize(CONTECT_FRAME_MAX_SIZE);

    QHBoxLayout *calibrationHLayout = new QHBoxLayout(m_calibrationFrame);

    m_calibrationLabel = new FixLabel(this);
    m_calibrationLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_calibrationLabel->setText(tr("calibrate the touch accuracy of the selected display"));
    m_calibrationLabel->setMinimumWidth(LONGLABEL_MIN_WIDTH);

    m_calibrationButton = new FixPushButton(this);
    m_calibrationButton->setFixedWidth(PUSHBUTTTON_FIXED_WIDTH);
    m_calibrationButton->setButtonText(tr("calibration"));

    calibrationHLayout->setContentsMargins(ITEM_MARGINS);
    calibrationHLayout->addWidget(m_calibrationLabel);
    calibrationHLayout->addWidget(m_calibrationButton);

    m_calibrationFrame->setLayout(calibrationHLayout);
}

void TouchScreenWidget::resetFrameSize()
{
    int height = 0;
    for (int i = 0; i < m_hotspotFrame->layout()->count(); i ++) {
        QWidget *w = m_hotspotFrame->layout()->itemAt(i)->widget();
        if (w != nullptr && !w->isHidden()) {
            height += w->height();
        }
    }
    m_hotspotFrame->setFixedHeight(height + 20);
}

FixPushButton::FixPushButton(QWidget *parent) :
    QPushButton(parent)
{
    const QByteArray id("org.ukui.style");
    QGSettings * fontSetting = new QGSettings(id, QByteArray(), this);
    if(QGSettings::isSchemaInstalled(id)){
        connect(fontSetting, &QGSettings::changed,[=](QString key) {
            if ("systemFont" == key || "systemFontSize" ==key) {
                changedLabelSlot();
            }
        });
    }
}

void FixPushButton::setButtonText(QString text) {

    m_str = text;
    changedLabelSlot();
}

QString FixPushButton::getText(){
    return m_str;
}

void FixPushButton::changedLabelSlot() {
    QFontMetrics  fontMetrics(this->font());
    int fontSize = fontMetrics.width(m_str);
    if (fontSize > 65) {
        setText(fontMetrics.elidedText(m_str, Qt::ElideRight, 65));
        setToolTip(m_str);
    } else {
        setText(m_str);
        setToolTip("");
    }
}
