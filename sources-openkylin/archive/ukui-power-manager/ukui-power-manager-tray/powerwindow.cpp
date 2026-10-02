/*
 * Copyright 2021 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "powerwindow.h"
#include "batteryiconhelper.h"
#include "xatom-helper.h"
#include "qdesktopwidget.h"


#include <QApplication>
#include <QTimer>
#include <KWindowEffects>
#include <windowmanager/windowmanager.h>

powerwindow::powerwindow(QWidget *parent) : QWidget(parent),
    m_panelPosition(PanelPosition::panel_bottom)
{
    qDebug() << " power window init start";
    initPlatformType();
    setWindowProperty(); //设置窗口属性
    installEventFilter(this);

    QVariant value;
    if (true == UpmGsettings::self()->getGsettingsConfig(GSETTINGS_KEY_SYSTEM_FONT_SIZE, value)) {
        m_sysFontSize = value.toDouble();
    } else {
        m_sysFontSize = 10;
    }
    if (true == UpmGsettings::self()->getGsettingsConfig(GSETTINGS_KEY_POWER_POLICY_AC, value)) {
        m_powerPolicyAc = value.toInt();
    }
    if (true == UpmGsettings::self()->getGsettingsConfig(GSETTINGS_KEY_POWER_POLICY_BATTERY, value)) {
        m_powerPolicyBattery = value.toInt();
    }

    initUI();            //初始化UI


    connect(UpmGsettings::self(), &UpmGsettings::systemFontSizeChanged, this, [=](QVariant value) {
        m_sysFontSize = value.toDouble();
        setPercentageLabelFontsize();
        if (true == m_acOnlineChange) {
            setPowerStateLabelForAcOnlineChanged();
        } else {
            setPowerStateLabelForBatteryInfoChanged();
        }
    });
    connect(UpmGsettings::self(), &UpmGsettings::styleNameChanged, this, [=](QVariant) {
        updateBatteryIcon();
    });
    connect(UpmGsettings::self(), &UpmGsettings::percentageLowChanged, this, [=](QVariant) {
        updateBatteryIcon();
    });

    connect(UpmGsettings::self(), &UpmGsettings::powerPolicyAcChanged, this, [=](QVariant value) {
        m_powerPolicyAc = value.toInt();
        setSliderValue();
    });
    connect(UpmGsettings::self(), &UpmGsettings::powerPolicyBatteryChanged, this, [=](QVariant value) {
        m_powerPolicyBattery = value.toInt();
        setSliderValue();
    });

    if (true == UpmGsettings::self()->getGsettingsConfig(GSETTINGS_KEY_TRANSPARENCY_KEY, value)) {
        m_transparency = value.toDouble();
    }
    this->update();

    connect(UpmGsettings::self(), &UpmGsettings::transparencyChanged, this, [=](QVariant value) {
        m_transparency = value.toDouble();
        this->update();
    });

    //监听屏幕改变的信号
    connect(QApplication::desktop(), &QDesktopWidget::resized, this, [=]() {
        QTimer::singleShot(1000, this, [=]() {
            resetWindowPosition();
        });
    });
    connect(QApplication::desktop(), &QDesktopWidget::screenCountChanged, this, [=]() {
        resetWindowPosition();
    });
    connect(QApplication::desktop(), &QDesktopWidget::primaryScreenChanged, this, [=]() {
        resetWindowPosition();
    });
}

powerwindow::~powerwindow() {}

void powerwindow::setWindowProperty()
{
    //设置任务栏无显示
    setWindowOpacity(1);
    setAttribute(Qt::WA_TranslucentBackground); //设置窗口背景透明
    setProperty("useSystemStyleBlur", true);    //设置毛玻璃效果
    setProperty("useStyleWindowManager", false);
    m_windowHelper = new UkuiWindowHelper(this);
    m_windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::SystemWindow);

    if(false == m_isWayland) {
        KWindowEffects::enableBlurBehind(this->winId(), true);
        // 添加窗管协议
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    } else {
        resetWindowPosition();
        slideWindowByPanelPosition(m_panelPosition);
        m_windowHelper->setBlurEffect(QRegion(), 0, true);
        m_windowHelper->removeTitleBar();
        m_windowHelper->setSkipTaskBar(true);
        m_windowHelper->setSkipSwitcher(true);
    }
}

void powerwindow::initUI()
{
    this->setFixedSize(420, 265);

    m_firstwidget = new QWidget(this);
    m_topwidget   = new QWidget(this);
    m_lastWidget  = new QWidget(this);
    m_lastWidget->setFixedHeight(56);
    m_statewidget = new QWidget(this);

    m_pmainlayout = new QVBoxLayout(this);
    m_firstlayout = new QHBoxLayout(m_firstwidget);
    m_toplayout   = new QHBoxLayout(m_topwidget);
    m_lastlayout  = new QHBoxLayout(m_lastWidget);
    m_statelayout = new QHBoxLayout(m_statewidget);

    m_iconButton = new QPushButton(this);
    m_iconButton->setFixedSize(QSize(48, 65));
    m_iconButton->setIconSize(QSize(30, 30));
    m_iconButton->setStyle(new CustomStyle);
    m_iconButton->setProperty("useIconHighlightEffect", 0x10);
    m_iconButton->setFlat(true);
    m_iconButton->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_percentageLabel = new QLabel(this);
    setPercentageLabelFontsize();

    m_powerStateLabel = new QLabel(this);

    m_segmentationLine_1 = new Divider(this);
    m_segmentationLine_2 = new Divider(this);

    m_StateSlider = new KSlider(this);
    m_StateSlider->setOrientation(Qt::Orientation::Horizontal);
    m_StateSlider->setSliderType(KSliderType::SingleSelectSlider);
    m_StateSlider->setMaximum(2);
    m_StateSlider->setSingleStep(1);
    m_StateSlider->setPageStep(1);
    m_StateSlider->setFixedWidth(372);
    m_StateSlider->setProperty("needTranslucent", true);
    m_StateSlider->setTranslucent(true);
    setSliderValue();
    connect(m_StateSlider, &KSlider::valueChanged, this, &powerwindow::sliderValueChanged);

    m_enduranceIconButtun = new QToolButton(this);
    m_enduranceIconButtun->setIconSize(QSize(16, 16));
    m_enduranceIconButtun->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_enduranceIconButtun->setProperty("useIconHighlightEffect", 0x2);
    m_enduranceIconButtun->setStyle(new CustomStyle);
    QIcon icon1 = QIcon::fromTheme("ukui-eco-symbolic");
    m_enduranceIconButtun->setIcon(icon1);
    m_enduranceIconButtun->setText(tr("Endurance"));

    m_performanceIconButtun = new QToolButton(this);
    m_performanceIconButtun->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_performanceIconButtun->setProperty("useIconHighlightEffect", 0x2);
    m_performanceIconButtun->setStyle(new CustomStyle);
    QIcon icon2 = QIcon::fromTheme("ukui-performance-symbolic");
    m_performanceIconButtun->setIcon(icon2);
    m_performanceIconButtun->setText(tr("Performance"));

    //电源设置按钮
    m_settingLabel = new KyLable(this);
    m_settingLabel->setText(tr("PowerSettings"));

    m_toplabel = new QLabel(this);
    m_toplabel->setText(tr("PowerMode"));
    m_toplayout->setContentsMargins(24, 0, 24, 0);
    m_toplayout->setSpacing(0);
    m_toplayout->addWidget(m_toplabel);
    m_toplayout->addItem(new QSpacerItem(10, 2));

    //第一行布局
    m_firstlayout->setContentsMargins(18, 0, 24, 0);
    m_firstlayout->setSpacing(0);
    m_firstlayout->addWidget(m_iconButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_firstlayout->addWidget(m_percentageLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_firstlayout->addStretch();
    m_firstlayout->addWidget(m_powerStateLabel, 0, Qt::AlignRight | Qt::AlignVCenter);

    //滑动条下方标识布局
    m_statelayout->setContentsMargins(18, 0, 5, 0);
    m_statelayout->setSpacing(0);
    m_statelayout->addWidget(m_enduranceIconButtun, 0, Qt::AlignLeft | Qt::AlignCenter);
    m_statelayout->addStretch();
    m_statelayout->addWidget(m_performanceIconButtun, 0, Qt::AlignRight);

    //电源设置按钮布局
    m_lastlayout->setContentsMargins(24, 0, 24, 0);
    m_lastlayout->setSpacing(0);
    m_lastlayout->addWidget(m_settingLabel);
    m_lastlayout->addStretch();
    m_settingLabel->setCursor(Qt::PointingHandCursor);

    //总体布局
    m_pmainlayout->setContentsMargins(0, 0, 0, 0);
    m_pmainlayout->setSpacing(0);
    m_pmainlayout->addWidget(m_topwidget);
    m_pmainlayout->addWidget(m_segmentationLine_1);
    m_pmainlayout->addSpacing(12);
    m_pmainlayout->addWidget(m_firstwidget);
    m_pmainlayout->addSpacing(0);
    m_pmainlayout->addWidget(m_StateSlider, 0, Qt::AlignCenter);
    m_pmainlayout->addSpacing(0);
    m_pmainlayout->addWidget(m_statewidget);
    m_pmainlayout->addWidget(m_segmentationLine_2);
    m_pmainlayout->addWidget(m_lastWidget);
    this->setLayout(m_pmainlayout);
}

void powerwindow::initPlatformType()
{
    if ("wayland" == qgetenv("XDG_SESSION_TYPE")) {
       m_isWayland = true;
    }
    qDebug() << "Platform is wayland:" << m_isWayland;
}

void powerwindow::setIconName(QString iconName)
{
    m_iconName = iconName;
    updateBatteryIcon();
}

void powerwindow::updateBatteryIcon()
{
    if (m_iconName.isNull() || nullptr == m_iconButton) {
        return;
    }
    m_iconButton->setIcon(buildBatteryLevelIcon(m_iconName, m_batteryPercentage,
                                                QStringLiteral("(powerWindow)")));
}

void powerwindow::setAcOnlineState(bool acOnlineState)
{
    if (m_acOnlineState == acOnlineState) {
        return;
    }
    m_acOnlineState = acOnlineState;
    if (true == m_acOnlineState) {
        m_toplabel->setText(tr("PowerMode"));
    } else {
        m_toplabel->setText(tr("BatteryMode"));
    }
    setSliderValue();
}

void powerwindow::setBatteryState(int iBatteryState)
{
    if (m_batteryState == iBatteryState) {
        m_batteryStateChanged = false;
    } else {
        m_batteryStateChanged = true;
        m_batteryState = iBatteryState;
        setSliderValue();
    }
}

void powerwindow::setBatteryPercentage(int iBatteryPercentage)
{
    m_batteryPercentage = iBatteryPercentage;
    if (UpmUpowerDBus::self()->hasBattery()) {
        m_percentageLabel->setText(QString("%1%").arg(m_batteryPercentage));
        m_percentageLabel->show();
    } else {
        m_percentageLabel->hide();
    }
}

void powerwindow::setShowPowerLeftTime(int iShowPowerLeftTime)
{
    m_showBatteryLeftTime = iShowPowerLeftTime;
}

void powerwindow::setBatteryLeftTime(qlonglong timeToFull, qlonglong timeToEmpty)
{
    m_timeToFull = timeToFull;
    m_timeToEmpty = timeToEmpty;
}

void powerwindow::setPowerStateLabelForAcOnlineChanged()
{
    // 添加无电池情况的处理
    if (!UpmUpowerDBus::self()->hasBattery()) {
        m_powerStateLabel->setText(tr("No Battery detected"));
        m_acOnlineChange = true;
        return;
    }

    if (true == m_acOnlineState) {
        switch (m_batteryState) {
        case battery_state_unknown:
            qWarning() << "battery state unknown";
            break;
        case battery_state_fully:
            qWarning() << "battery state fully";
            if (100 == m_batteryPercentage) {
                m_powerStateLabel->setText(tr("Fully Charged"));
            } else {
                m_powerStateLabel->setText(tr("Not Charging"));
            }
            break;
        case battery_state_discharging:
            //todo 有时候电池保护或者充满可能会出现该状态，不确定，不是100%时暂时先显示未充电
            qWarning() << "battery state discharging";
            if (100 == m_batteryPercentage) {
                m_powerStateLabel->setText(tr("Fully Charged"));
            } else {
                m_powerStateLabel->setText(tr("Not Charging"));
            }
            break;
        case battery_state_charging:
            m_powerStateLabel->setText(tr("Charging"));
            break;
        case battery_state_pending_charge:
            m_powerStateLabel->setText(tr("Not Charging"));
            break;
        case battery_state_empty:
            //todo
            qWarning() << "battery state empty";
            break;
        case battery_state_last:
            //todo
            qWarning() << "battery state last";
            break;
        default:
            break;
        }
    } else {
        m_powerStateLabel->setText(tr("Discharging"));
    }
    m_acOnlineChange = true;
}

void powerwindow::setPowerStateLabelForBatteryInfoChanged()
{
    // 添加无电池情况的处理
    if (!UpmUpowerDBus::self()->hasBattery()) {
        if (m_acOnlineState) {
            m_powerStateLabel->setText(tr("No Battery detected"));
        }
        m_acOnlineChange = true;
        return;
    }

    QString power_state_text("");
    int hour = 0;
    int minute = 0;
    qint64 batteryLeftTime = 0;

    switch (m_batteryState) {
        case battery_state_unknown:
        case battery_state_fully:
            if (100 == m_batteryPercentage) {
                power_state_text = QString(tr("Fully Charged"));
            }
            break;
        case battery_state_charging:
            batteryLeftTime = m_timeToFull;
            if (false == m_showBatteryLeftTime || 0 == batteryLeftTime
            || true == m_batteryStateChanged || m_batteryPercentage > 95) {
                if (100 == m_batteryPercentage) {
                    power_state_text = QString(tr("Fully Charged"));
                } else {
                    power_state_text = QString(tr("Charging"));
                }
            } else {
                hour = batteryLeftTime / 3600;
                minute = ((batteryLeftTime) % 3600) / 60;
                if (0 == hour) {
                    power_state_text = QString(tr("%1 minutes \nuntil fully charged")).arg(minute);
                } else if (hour > 10) {
                    power_state_text = QString(tr("Charging"));
                } else {
                    if (1 == hour) {
                        if (1 == minute) {
                            power_state_text = QString(tr("%1 hour %2 minute \nuntil fully charged"))
                                    .arg(hour).arg(minute);
                        } else if (0 == minute) {
                            power_state_text = QString(tr("%1 hour \nuntil fully charged")).arg(hour);
                        } else {
                            power_state_text = QString(tr("%1 hour %2 minutes \nuntil fully charged"))
                                    .arg(hour).arg(minute);
                        }
                    } else {
                        if (1 == minute) {
                            power_state_text = QString(tr("%1 hours %2 minute \nuntil fully charged"))
                                    .arg(hour).arg(minute);
                        } else if (0 == minute) {
                            power_state_text = QString(tr("%1 hours \nuntil fully charged")).arg(hour);
                        } else {
                            power_state_text = QString(tr("%1 hours %2 minutes \nuntil fully charged"))
                                    .arg(hour).arg(minute);
                        }
                    }
                }
            }
            break;
        case battery_state_discharging:
            batteryLeftTime = m_timeToEmpty;
            if (false == m_showBatteryLeftTime || 0 == batteryLeftTime || true == m_batteryStateChanged) {
                power_state_text = QString(tr("Discharging"));
            } else {
                hour = batteryLeftTime / 3600;
                minute = ((batteryLeftTime) % 3600) / 60;
                if (0 == hour) {
                    power_state_text = QString(tr("%1 minutes \nremaining")).arg(minute);
                } else if (hour > 20) {
                    power_state_text = QString(tr("Discharging"));
                } else {
                    if (1 == hour) {
                        if (1 == minute) {
                            power_state_text = QString(tr("%1 hour %2 minute \nremaining"))
                                    .arg(hour).arg(minute);
                        } else if (0 == minute) {
                            power_state_text = QString(tr("%1 hour \nremaining"))
                                    .arg(hour);
                        } else {
                            power_state_text = QString(tr("%1 hour %2 minutes \nremaining"))
                                    .arg(hour).arg(minute);
                        }
                    } else {
                        if (1 == minute) {
                            power_state_text = QString(tr("%1 hours %2 minute \nremaining"))
                                    .arg(hour).arg(minute);
                        } else if (0 == minute) {
                            power_state_text = QString(tr("%1 hours \nremaining"))
                                    .arg(hour);
                        } else {
                            power_state_text = QString(tr("%1 hours %2 minutes \nremaining"))
                                    .arg(hour).arg(minute);
                        }
                    }
                }
            }
        break;
    case battery_state_pending_charge:
        power_state_text = QString(tr("Not Charging"));
        break;
        default:
            break;
    }
    m_powerStateLabel->setText(power_state_text);
    m_acOnlineChange = false;
}

void powerwindow::setPercentageLabelFontsize()
{
    QFont font;
    font.setPointSize((int)(m_sysFontSize * 1.5));
    m_percentageLabel->setFont(font);
}

void powerwindow::showPowerWindow()
{
    if (false == m_isWayland) {
        resetWindowPosition();
        KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
        this->show();
        slideWindowByPanelPosition(m_panelPosition);
        this->activateWindow();
    } else {
        this->show();
        resetWindowPosition();
        slideWindowByPanelPosition(m_panelPosition);
    }
}

void powerwindow::resetWindowPosition()
{
    QVariant value;
    int panelSize = 46;

    if (true == UpmGsettings::self()->getGsettingsConfig(GSETTINGS_KEY_PANEL_SIZE, value)) {
        panelSize = value.toInt();
    }
    if (true == UpmGsettings::self()->getGsettingsConfig(GSETTINGS_KEY_PANEL_POSITION, value)) {
        m_panelPosition = value.toInt();
    }

    QScreen* screen = m_windowHelper->getCurrentScreenBlocking();
    if (screen == nullptr) {
        // 如果没有找到屏幕，直接返回，避免访问空指针
        qWarning() << "无法获取当前光标所在屏幕，取消窗口位置重置";
        return;
    }
    QRect availableGeometry = screen->geometry();

    int x = 0;
    int y = 0;
    int margin = 8;

    switch (m_panelPosition) {
        case PanelPosition::panel_top:
            x = availableGeometry.x() + availableGeometry.width() - this->width() - margin;
            y = availableGeometry.y() + panelSize + margin;
            break;

        case PanelPosition::panel_left:
            x = availableGeometry.x() + panelSize + margin;
            y = availableGeometry.y() + availableGeometry.height() - this->height() - margin;
            break;

        case PanelPosition::panel_right:
            x = availableGeometry.x() + availableGeometry.width() - panelSize - this->width() - margin;
            y = availableGeometry.y() + availableGeometry.height() - this->height() - margin;
            break;

        default:
            x = availableGeometry.x() + availableGeometry.width() - this->width() - margin;
            y = availableGeometry.y() + availableGeometry.height() - panelSize - this->height() - margin;
            break;
    }

    m_windowPositionRect = QRect(x, y, this->width(), this->height());
    this->setGeometry(m_windowPositionRect);
}

void powerwindow::slideWindowByPanelPosition(int panelPosition)
{
    if(panelPosition == 0) {
        m_windowHelper->setSlideEffect(UkuiWindowHelper::Position::Bottom, 0, true);
    } else if (panelPosition == 1) {
        m_windowHelper->setSlideEffect(UkuiWindowHelper::Position::Top, 0, true);
    } else if(panelPosition == 2) {
        m_windowHelper->setSlideEffect(UkuiWindowHelper::Position::Left, 0, true);
    } else {
        m_windowHelper->setSlideEffect(UkuiWindowHelper::Position::Right, 0, true);
    }
}

void powerwindow::sliderValueChanged(int value)
{
    qDebug() << "ac online state:" << m_acOnlineState << "slider value:" << value;
    if (value < 0 || value > 2) {
        return ;
    }

    int policy[3] = {EnergySaving, Balance, Performance};

    if (true == m_acOnlineState) {
        UpmGsettings::self()->setGsettingsConfig(GSETTINGS_KEY_POWER_POLICY_AC, policy[value]);
    } else {
        UpmGsettings::self()->setGsettingsConfig(GSETTINGS_KEY_POWER_POLICY_BATTERY, policy[value]);
    }
}

void powerwindow::setSliderValue()
{
    qDebug() << "ac online state:" << m_acOnlineState;

    int sliderPosition[3] = {2, 1, 0};

    if (true == m_acOnlineState) {
        m_StateSlider->setValue(sliderPosition[m_powerPolicyAc]);
    } else {
        m_StateSlider->setValue(sliderPosition[m_powerPolicyBattery]);
    }
}

void powerwindow::paintEvent(QPaintEvent *event)
{
    QStyleOption styleOption;
    styleOption.init(this);
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    QColor color = palette().color(QPalette::Base);
    color.setAlphaF(m_transparency);
    QBrush brush = QBrush(color);
    painter.setBrush(brush);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.drawRect(styleOption.rect);
}

bool powerwindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        if (event->type() == QEvent::WindowDeactivate) {
            hide();
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void powerwindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
    }
    QWidget::keyPressEvent(event);
}
