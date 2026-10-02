/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2026 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "waylandtouchpadsettingsui.h"

#include <QDBusInterface>
#include <QDebug>
#include <QGSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSignalBlocker>

#include "qdbusmetatype.h"

#include <unistd.h>
#include <kslider.h>
#include <ukcc/interface/ukcccommon.h>

#include "gesturewidget.h"
#include "settingscommon.h"

#define KYLIN_USER_GUIDE_PATH              "/"
#define KYLIN_USER_GUIDE_SERVICE           "com.kylinUserGuide.hotel"
#define KYLIN_USER_GUIDE_INTERFACE         "com.guide.hotel"

const QString DBUS_SERVICE = "com.kylin.Wlcom";
const QString DBUS_OBJECT_PATH = "/com/kylin/Wlcom/GlobalGestures";
const QString DBUS_INTERFACE = "com.kylin.Wlcom.GlobalGestures";

const QByteArray kTouchpadSchemas = "org.ukui.peripherals-touchpad";
const QString kMouseDisableKey = "disableOnExternalMouse";
const QString kTouchpadDisableKey = "touchpadEnabled";
const QString kTypingDisableKey = "disableWhileTyping";
const QString kClickKey = "tapToClick";
const QString kTwoFingerClickKey = "twoFingerClick";
const QString kDoubleClickDragKey = "doubleClickDrag";
const QString kBottomRightClickKey = "bottomRightCornerClickMenu";
const QString kSoftwareEdgeSlidingGestureCapKey = "isZoneGestureAvailable";
const QString kTopEdgeGestureKey = "zoneGestureTopEdgeEnabled";
const QString kLeftEdgeGestureKey = "zoneGestureLeftEdgeEnabled";
const QString kRightEdgeGestureKey = "zoneGestureRightEdgeEnabled";
const QString kScaleGestureKey = "twoFingerPinch";
const QString kScrollSlideKey = "naturalScroll";
const QString kPointerSpeedKey = "motion-acceleration";

const QByteArray kTrackPointSchemas = "org.ukui.peripherals-trackpoint";
const QString kTrackPointDisableKey = "trackpointEnabled";

const QList<QPair<QString, QString>> WaylandTouchpadUI::controlledItems = {
    { kTouchpadDisableKey, CONFIG_KEY_TOUCHPAD_ENABLED },
    { kTypingDisableKey, CONFIG_KEY_DISABLE_WHILE_TYPING },
    { kMouseDisableKey, CONFIG_KEY_DISABLE_ON_EXTERNAL_MOUSE_ENABLED }
};

const QList<QPair<QString, int>> WaylandTouchpadUI::tapGestureFuncItems = {
    { QT_TR_NOOP("None"), 0 },
    { QT_TR_NOOP("Open Search"), 47 },
    { QT_TR_NOOP("Notification Center"), 40 },
    { QT_TR_NOOP("Play/Pause"), 51 },
//    { QT_TR_NOOP("Middle Click"), 43 }
};

const QList<QPair<QString, int>> WaylandTouchpadUI::swipeGestureUpLeftFuncItems = {
    { QT_TR_NOOP("None"), 0 },
    { QT_TR_NOOP("App Switch"), 45 },
    { QT_TR_NOOP("Multi Taskview"), 46 },
    { QT_TR_NOOP("Show Desktop"), 12 },
//    { QT_TR_NOOP("Switch to Left Workspace"), 5 },
//    { QT_TR_NOOP("Switch to Right Workspace"), 6 },
    { QT_TR_NOOP("Switch Workspace"), 49 },
//    { QT_TR_NOOP("Switch to Workspace 1"), 7 },
//    { QT_TR_NOOP("Switch to Workspace 2"), 8 },
//    { QT_TR_NOOP("Switch to Workspace 3"), 9 },
//    { QT_TR_NOOP("Switch to Workspace 4"), 10 },
    { QT_TR_NOOP("Minimize Other Windows"), 14 },
    { QT_TR_NOOP("New Workspace"), 3 },
    { QT_TR_NOOP("Delete Workspace"), 4 },
    { QT_TR_NOOP("Tile Window to the Left"), 24 },
//    { QT_TR_NOOP("Tile Active Window to the Right"), 25 },
//    { QT_TR_NOOP("Follow Gesture to Switch to Left Workspace"), 48 },
//    { QT_TR_NOOP("Follow Gesture to Switch to Right Workspace"), 49 },
    { QT_TR_NOOP("Maximize Window"), 17 },
    { QT_TR_NOOP("Minimize Window"), 18 },
    { QT_TR_NOOP("Next Track"), 53 },
    { QT_TR_NOOP("Previous Track"), 54 },
    { QT_TR_NOOP("Volume Mute"), 57 }
};

const QList<QPair<QString, int>> WaylandTouchpadUI::swipeGestureDownRightFuncItems = {
    { QT_TR_NOOP("None"), 0 },
    { QT_TR_NOOP("App Switch"), 44 },
    { QT_TR_NOOP("Multi Taskview"), 46 },
    { QT_TR_NOOP("Show Desktop"), 12 },
//    { QT_TR_NOOP("Switch to Left Workspace"), 5 },
//    { QT_TR_NOOP("Switch to Right Workspace"), 6 },
    { QT_TR_NOOP("Switch Workspace"), 50 },
//    { QT_TR_NOOP("Switch to Workspace 1"), 7 },
//    { QT_TR_NOOP("Switch to Workspace 2"), 8 },
//    { QT_TR_NOOP("Switch to Workspace 3"), 9 },
//    { QT_TR_NOOP("Switch to Workspace 4"), 10 },
    { QT_TR_NOOP("Minimize Other Windows"), 14 },
    { QT_TR_NOOP("New Workspace"), 3 },
    { QT_TR_NOOP("Delete Workspace"), 4 },
//    { QT_TR_NOOP("Tile Window to the Left"), 24 },
    { QT_TR_NOOP("Tile Window to the Right"), 25 },
//    { QT_TR_NOOP("Follow Gesture to Switch to Left Workspace"), 48 },
//    { QT_TR_NOOP("Follow Gesture to Switch to Right Workspace"), 49 },
    { QT_TR_NOOP("Maximize Window"), 17 },
    { QT_TR_NOOP("Minimize Window"), 18 },
    { QT_TR_NOOP("Next Track"), 53 },
    { QT_TR_NOOP("Previous Track"), 54 },
    { QT_TR_NOOP("Volume Mute"), 57 }
};

typedef QMap<uint, QDBusVariant> GestureConfigMap;
Q_DECLARE_METATYPE(GestureConfigMap)

namespace {

void setupWidgetAccessibility(QWidget *widget, const QString &name, const QString &description = QString())
{
    if (widget == nullptr) {
        return;
    }

    widget->setAccessibleName(name);
    if (!description.isEmpty()) {
        widget->setAccessibleDescription(description);
    }
}

QString switchAccessibleDescription(const QString &settingKey)
{
    if (settingKey == kTouchpadDisableKey) {
        return WaylandTouchpadUI::tr("Enable or disable the touchpad");
    }
    if (settingKey == kMouseDisableKey) {
        return WaylandTouchpadUI::tr("Disable the touchpad while an external mouse is in use");
    }
    if (settingKey == kTypingDisableKey) {
        return WaylandTouchpadUI::tr("Disable the touchpad while typing");
    }
    if (settingKey == kClickKey) {
        return WaylandTouchpadUI::tr("Tap the touchpad to perform a click");
    }
    if (settingKey == kTwoFingerClickKey) {
        return WaylandTouchpadUI::tr("Tap with two fingers to open the context menu");
    }
    if (settingKey == kDoubleClickDragKey) {
        return WaylandTouchpadUI::tr("Double-tap and drag to select multiple items");
    }
    if (settingKey == kBottomRightClickKey) {
        return WaylandTouchpadUI::tr("Click the bottom-right corner of the touchpad for right-click");
    }
    if (settingKey == kTopEdgeGestureKey) {
        return WaylandTouchpadUI::tr("Swipe horizontally along the top edge to adjust video progress");
    }
    if (settingKey == kLeftEdgeGestureKey) {
        return WaylandTouchpadUI::tr("Swipe vertically along the left edge to adjust brightness");
    }
    if (settingKey == kRightEdgeGestureKey) {
        return WaylandTouchpadUI::tr("Swipe vertically along the right edge to adjust volume");
    }
    if (settingKey == kScaleGestureKey) {
        return WaylandTouchpadUI::tr("Pinch with two fingers to zoom in or out");
    }
    if (settingKey == kTrackPointDisableKey) {
        return WaylandTouchpadUI::tr("Enable or disable the trackpoint");
    }

    return WaylandTouchpadUI::tr("Toggle this setting on or off");
}

} // namespace

QDBusArgument &operator<<(QDBusArgument &arg, const GestureConfigMap &map)
{
    int keyTypeId = static_cast<int>(QMetaType::UInt);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    int valueTypeId = QMetaType::fromType<QDBusVariant>().id();
#else
    int valueTypeId = QMetaType::type(QMetaType::typeName(qMetaTypeId<QDBusVariant>()));
#endif
    arg.beginMap(keyTypeId, valueTypeId);

    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        arg.beginMapEntry();
        arg << it.key();
        arg << it.value();
        arg.endMapEntry();
    }
    arg.endMap();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, GestureConfigMap &map)
{
    map.clear();
    arg.beginMap();
    while (!arg.atEnd()) {
        quint32 key;
        QDBusVariant value;
        arg.beginMapEntry();
        arg >> key >> value;
        arg.endMapEntry();
        map.insert(key, value);
    }
    arg.endMap();
    return arg;
}

bool getGestureStatus(quint32 gestureId, quint32 &outCurrentFuncId, bool &outEnabled)
{
    // 1. 初始化D-Bus接口
    const QString methodName = "ListGestures";

    QDBusInterface iface(DBUS_SERVICE, DBUS_OBJECT_PATH, DBUS_INTERFACE,
                         QDBusConnection::sessionBus());

    // 检查接口是否有效
    if (!iface.isValid()) {
        qWarning() << "D-Bus接口无效：" << QDBusConnection::sessionBus().lastError().message();
        return false;
    }

    // 2. 调用ListGestures方法，获取所有手势数据
    QDBusMessage replyMsg = iface.call(methodName);
    if (replyMsg.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "调用ListGestures失败：" << QDBusConnection::sessionBus().lastError().message();
        return false;
    }

    // 3. 解析返回的a{uv}结构
    QList<QVariant> args = replyMsg.arguments();
    if (args.isEmpty()) {
        return false;
    }

    const QDBusArgument &arg = args[0].value<QDBusArgument>();
    QMap<quint32, QDBusVariant> gestureMap;

    arg.beginMap();
    while(!arg.atEnd()) {
        quint32 key;
        QDBusVariant value;
        arg.beginMapEntry();
        arg >> key;
        arg >> value;
        arg.endMapEntry();

        gestureMap.insert(key, value);
    }
    arg.endMap();

    // 4. 查找目标gesture_id是否存在
    if (!gestureMap.contains(gestureId)) {
        qWarning() << "未找到gesture_id：" << gestureId;
        return false;
    }

    // 5. 解析variant中的a{sv}（对应Qt的QVariantMap）
    const QDBusVariant &gestureVariant = gestureMap[gestureId];
    QVariantMap gestureInfo;
    const QDBusArgument &innerArg = gestureVariant.variant().value<QDBusArgument>();

    innerArg.beginMap();
    while(!innerArg.atEnd()) {
        QString key;
        QDBusVariant value;
        innerArg.beginMapEntry();
        innerArg >> key;
        innerArg >> value;
        innerArg.endMapEntry();

        gestureInfo.insert(key, value.variant());
    }
    innerArg.endMap();

    // 6. 提取关键字段（严格匹配字段名和类型）
    // current：当前function_id（u32）
    if (!gestureInfo.contains("current") || !gestureInfo["current"].canConvert<quint32>()) {
        qWarning() << "gesture_id" << gestureId << "缺少current字段或类型错误";
        return false;
    }
    outCurrentFuncId = gestureInfo["current"].toUInt();

    // enabled：启用状态（bool）
    if (!gestureInfo.contains("enabled") || !gestureInfo["enabled"].canConvert<bool>()) {
        qWarning() << "gesture_id" << gestureId << "缺少enabled字段或类型错误";
        return false;
    }
    outEnabled = gestureInfo["enabled"].toBool();

    qInfo() << "解析成功：gesture_id=" << gestureId
            << " current_func_id=" << outCurrentFuncId
            << " enabled=" << outEnabled;
    return true;
}

quint32 updateGestureCurrentFuncIdSimple(quint32 gestureId, quint32 targetCurrentFuncId)
{
    QDBusInterface iface(DBUS_SERVICE, DBUS_OBJECT_PATH, DBUS_INTERFACE, QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        qCritical() << "D-Bus接口无效:" << qPrintable(iface.lastError().message());
        return 1;
    }

    GestureConfigMap gestureConfig;
    QVariantMap singleGestureConfig;
    singleGestureConfig["current"] = targetCurrentFuncId;
    gestureConfig.insert(gestureId, QDBusVariant(singleGestureConfig));

    // 调用ConfigureGestures方法
    QDBusReply<uint> reply = iface.call("ConfigureGestures", QVariant::fromValue(gestureConfig));
    if (reply.isValid()) {
        qDebug() << "调用成功！已为手势ID" << gestureId << "设置current=" << targetCurrentFuncId;
    } else {
        qCritical() << "调用失败:" << qPrintable(reply.error().message());
        qCritical() << "错误详情:" << reply.error();
    }

    quint32 successCount = reply.value();
    qInfo() << "更新手势ID" << gestureId << "的current为" << targetCurrentFuncId
            << "，成功条目数：" << successCount;
    return successCount;
}

WaylandTouchpadUI::WaylandTouchpadUI(QWidget *parent)
    : QWidget(parent)
{
    setupWidgetAccessibility(this, tr("Touchpad settings"), tr("Configure touchpad, trackpoint, and gesture options"));
    qDBusRegisterMetaType<GestureConfigMap>();
    QByteArray schema(kTouchpadSchemas);
    if (QGSettings::isSchemaInstalled(schema)) {
        m_touchpadGsetting = new QGSettings(kTouchpadSchemas, QByteArray(), this);
    }
    else {
        qCritical() << "GSetting org.ukui.peripherals-touchpad don't install";
        m_touchpadGsetting = nullptr;
    }
    m_configMonitor = new ConfigMonitor(CONFIG_FILE_NAME);
    createTouchPadSettingWidget();

    if (m_touchpadGsetting) {
        connect(m_touchpadGsetting, &QGSettings::changed, this, &WaylandTouchpadUI::updateVisible);
    }
    connect(m_configMonitor,&ConfigMonitor::moduleConfigChanged, this, &WaylandTouchpadUI::updateVisible);
    updateVisible();
    playDefaultGif();
}

bool WaylandTouchpadUI::getTouchpadEnabled()
{
    if (m_configMonitor->getModuleControlledByName(CONFIG_KEY_TOUCHPAD_ENABLED)) {
        return m_configMonitor->getModuleConfigByName(CONFIG_KEY_TOUCHPAD_ENABLED).toBool();
    } else {
        return m_touchpadGsetting->get(kTouchpadDisableKey).toBool();
    }
}

void WaylandTouchpadUI::updateVisible()
{
    bool needVisible = getTouchpadEnabled();
    foreach (auto widget, m_needUpdateVisibleWidgetList) {
        if (widget != nullptr) {
            widget->setVisible(needVisible);
        }
    }
}

QWidget* WaylandTouchpadUI::createSingleWidget(const QString& text, QWidget* rightWidget)
{
    QWidget* containerWidget = new QWidget(this);
    containerWidget->installEventFilter(this);
    containerWidget->setMaximumWidth(16777215);
    containerWidget->setFixedHeight(60);
    containerWidget->setFocusPolicy(Qt::NoFocus);

    rightWidget->setParent(containerWidget);

    QHBoxLayout* hLayout = new QHBoxLayout(containerWidget);

    QLabel* label = new QLabel(text, containerWidget);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (rightWidget->focusPolicy() == Qt::NoFocus) {
        rightWidget->setFocusPolicy(Qt::StrongFocus);
    }
    label->setBuddy(rightWidget);
    setupWidgetAccessibility(rightWidget, text);

    hLayout->addSpacing(6);
    if (qobject_cast<kdk::KSwitchButton*>(rightWidget) != nullptr) {
        hLayout->addWidget(label);
        hLayout->addStretch();
        hLayout->addWidget(rightWidget);
    } else {
        hLayout->addWidget(label, 1);
        hLayout->addWidget(rightWidget, 1);
    }
    hLayout->addSpacing(6);

    return containerWidget;
}

QWidget* WaylandTouchpadUI::createSingleSwitchWidget(const QString& settingKey, const QString& uiText, const bool needUpdateVisibility)
{
    if (!m_touchpadGsetting || !m_touchpadGsetting->keys().contains(settingKey)) {
        qWarning() << "gsetting invalid" << settingKey;
        return nullptr;
    }
    kdk::KSwitchButton* btn = new kdk::KSwitchButton();
    QString controlledKey("");
    bool isControlledKey = false;

    for (const auto& pair : WaylandTouchpadUI::controlledItems) {
        if (pair.first == settingKey) {
            isControlledKey = true;
            controlledKey = pair.second;
            break;
        }
    }

    btn->installEventFilter(this);

    if (settingKey == kTouchpadDisableKey) {
          m_touchpadDisableBtn = btn;
    }

    connect(btn, &kdk::KSwitchButton::stateChanged, this, [this, btn, settingKey](bool status) {
        qDebug() <<"stateChanged"  << settingKey << status;
        if (settingKey == kRightEdgeGestureKey && status
                && m_touchpadGsetting->get(V_EDGE_KEY).toBool()) {
            if (!confirmGestureConflict(false)) {
                QSignalBlocker blocker(btn);
                btn->setChecked(false);
                return;
            }
            const int index = m_scrollTypeComBox->findData(V_FINGER_KEY);
            QSignalBlocker comboBlocker(m_scrollTypeComBox);
            m_scrollTypeComBox->setCurrentIndex(index);
            m_previousScrollAreaIndex = index;
            applyScrollingType(V_FINGER_KEY);
            buriedSettings("ScrollArea", "select", V_FINGER_KEY);
        }
        // 双指点击右键：控制 1/2/3 指轻触分别映射的鼠标按键
        // 字母含义：L=左键，R=右键，M=中键，N=无动作；顺序为「1指/2指/3指」
        // 0: LRM — 1指左键 / 2指右键 / 3指中键
        // 1: LMR — 1指左键 / 2指中键 / 3指右键
        // 2: LRN — 1指左键 / 2指右键 / 3指无动作
        // 3: LLL — 1/2/3 指均为左键
        // 4: LLM — 1/2 指左键 / 3指中键
        // 开关开启写入 0(LRM)，关闭写入 4(LLM) 保留1指左键 和 3指中键
        if (settingKey == kTwoFingerClickKey) {
            int value;
            if (status) {
                value = 0;
            } else {
                value = 4;
            }
            m_touchpadGsetting->set(kTwoFingerClickKey, value);
        } else {
            m_touchpadGsetting->set(settingKey, status);
        }
        buriedSettings(settingKey, "settings", status ? "true":"false");
    });

    qDebug() << "createSingleSwitchWidget" << settingKey << uiText <<controlledKey <<  isControlledKey;

    if (settingKey == kTwoFingerClickKey) {
        btn->setChecked(m_touchpadGsetting->get(settingKey).toInt() == 0);
    } else {
        btn->setChecked(m_touchpadGsetting->get(settingKey).toBool());
    }
    connect(m_touchpadGsetting, &QGSettings::changed, this, [this, btn, settingKey](const QString& key) {
        if (key == settingKey) {
            qDebug() << key << "changed" << m_touchpadGsetting->get(settingKey).toBool();
            if (key == kTwoFingerClickKey) {
                btn->setChecked(m_touchpadGsetting->get(key).toInt() == 0);
            } else {
                const bool currentValue = m_touchpadGsetting->get(settingKey).toBool();
                if (btn->isChecked() != currentValue) {
                    btn->setChecked(currentValue);
                }
            }
        }
    });

    if (isControlledKey) {
        btn->setDisabled(m_configMonitor->getModuleControlledByName(controlledKey));
        connect(m_configMonitor, &ConfigMonitor::moduleConfigChanged, this,  [this, btn, controlledKey]() {
            bool isControlled = m_configMonitor->getModuleControlledByName(controlledKey);
            qDebug() << "ConfigChanged" << controlledKey << isControlled;
            btn->setDisabled(isControlled);
        });
    }

    QWidget* widget = createSingleWidget(uiText, btn);
    setupWidgetAccessibility(btn, uiText, switchAccessibleDescription(settingKey));
    if (needUpdateVisibility) {
        m_needUpdateVisibleWidgetList.append(widget);
    }
    return widget;
}

void WaylandTouchpadUI::createTouchPadSettingWidget()
{
    if (!m_touchpadGsetting) {
        qCritical() << "Don't show touchpad settings ui "
                       "because org.ukui.peripherals-touchpad don't install";
        return;
    }

    m_baseWidget = createBaseSettingsWidget();
    m_gestureWidget = createGestureSettingsWidget();
    if (hasTrackPointDevice()) {
        m_trackPointWidget = createTrackPointSettingsWidget();
    }

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setMargin(0);

    mainLayout->addWidget(createTitleWidget(tr("Touchpad")));
    mainLayout->addSpacing(8);
    mainLayout->addWidget(m_baseWidget);
    if (m_trackPointWidget != nullptr) {
        mainLayout->addSpacing(8);
        mainLayout->addWidget(m_trackPointWidget);
    }
    mainLayout->addSpacing(40);
    QWidget* gestureLabel = createTitleWidget(tr("Gesture"));
    mainLayout->addWidget(gestureLabel);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(m_gestureWidget);
    mainLayout->addStretch();

    m_needUpdateVisibleWidgetList.append(gestureLabel);
}

//标题widget
QWidget* WaylandTouchpadUI::createTitleWidget(QString titleText)
{
    QWidget* titleWidget = new QWidget(this);
    QLabel* titleLabel = new QLabel(this);
    titleLabel->setText(titleText);
    setupWidgetAccessibility(titleLabel, titleText);

    QHBoxLayout *titleLayout = new QHBoxLayout(this);
    titleLayout->setContentsMargins(16, 0, 0, 0);
    titleLayout->addWidget(titleLabel);
    titleWidget->setLayout(titleLayout);

    return titleWidget;
}

kdk::KHLineFrame* WaylandTouchpadUI::createBaseLine()
{
    kdk::KHLineFrame* line = new kdk::KHLineFrame(this);
    m_needUpdateVisibleWidgetList.append(line);
    return line;
}

//触控板设置widget
QWidget* WaylandTouchpadUI::createBaseSettingsWidget()
{
    QFrame *baseFrame = new QFrame(this);
    baseFrame->setMaximumSize(16777215, 16777215);
    baseFrame->setFrameShape(QFrame::Shape::Box);
    QVBoxLayout *vLyt = new QVBoxLayout(baseFrame);
    vLyt->setMargin(0);
    vLyt->setSpacing(0);

    vLyt->addWidget(createTouchpadDisableWidget());
    vLyt->addWidget(createBaseLine());

    vLyt->addWidget(createMouseDisableWidget());
    vLyt->addWidget(createBaseLine());

    vLyt->addWidget(createPointerSpeedWidget());
    vLyt->addWidget(createBaseLine());

    vLyt->addWidget(createTypingDisableWidget());

    return baseFrame;
}

QWidget* WaylandTouchpadUI::createTouchpadDisableWidget()
{
    return createSingleSwitchWidget(kTouchpadDisableKey, tr("Touchpad"));
}

QWidget* WaylandTouchpadUI::createMouseDisableWidget()
{
    //~ contents_path /Touchpad/Disable touchpad when using the mouse
    return createSingleSwitchWidget(kMouseDisableKey, tr("Disable touchpad when using the mouse"), true);
}

QWidget* WaylandTouchpadUI::createPointerSpeedWidget()
{
    QWidget *slideWidget = new QWidget();
    QHBoxLayout *slideLayout = new QHBoxLayout(slideWidget);
    slideLayout->setContentsMargins(0,0,0,0);
    slideLayout->setSpacing(0);

    QLabel* pointerSpeedSlowLabel = new QLabel(tr("Slow"), this);
    QLabel* pointerSpeedFastLabel = new QLabel(tr("Fast"), this);
    kdk::KSlider* pointerSpeedSlider = new kdk::KSlider(Qt::Horizontal);
    slideLayout->addWidget(pointerSpeedSlowLabel);
    slideLayout->addWidget(pointerSpeedSlider);
    slideLayout->addWidget(pointerSpeedFastLabel);

    pointerSpeedSlider->setMinimum(11);
    pointerSpeedSlider->setMaximum(80);
    pointerSpeedSlider->setTracking(true);
    pointerSpeedSlider->setValue(m_touchpadGsetting->get(kPointerSpeedKey).toInt()* 10);
    setupWidgetAccessibility(pointerSpeedSlider, tr("Pointer Speed"),
                           tr("Drag to adjust the touchpad pointer speed from slow to fast"));
    setupWidgetAccessibility(pointerSpeedSlowLabel, tr("Slow"));
    setupWidgetAccessibility(pointerSpeedFastLabel, tr("Fast"));

    //~ contents_path /Touchpad/Pointer Speed
    QWidget* w = createSingleWidget(tr("Pointer Speed"), slideWidget);

    connect(pointerSpeedSlider, &QSlider::valueChanged, this, &WaylandTouchpadUI::pointerSpeedSlot);
    connect(m_touchpadGsetting, &QGSettings::changed, this, [this, w, pointerSpeedSlider](const QString &key) {
        if (key == kPointerSpeedKey) {
            int val = m_touchpadGsetting->get(kPointerSpeedKey).toInt();
            if (val < 1) {   //异常数据保护 避免手动更改gsetting配置文件导致鼠标过快或者过慢
                pointerSpeedSlider->blockSignals(true);
                pointerSpeedSlider->setValue(11);
                pointerSpeedSlider->blockSignals(false);
            } else if (val > 8) {
                pointerSpeedSlider->blockSignals(true);
                pointerSpeedSlider->setValue(8 *10);
                pointerSpeedSlider->blockSignals(false);
            }
            else {
                pointerSpeedSlider->blockSignals(true);
                pointerSpeedSlider->setValue(val *10);
                pointerSpeedSlider->blockSignals(false);
            }
        }
    });

    m_needUpdateVisibleWidgetList.append(w);

    return w;
}

QWidget* WaylandTouchpadUI::createTypingDisableWidget()
{
    //~ contents_path /Touchpad/Disable touchpad when typing
    return createSingleSwitchWidget(kTypingDisableKey, tr("Disable touchpad when typing"), true);
}

QWidget* WaylandTouchpadUI::createTrackPointSettingsWidget()
{
    QFrame *trackPointFrame = new QFrame(this);
    trackPointFrame->setMaximumSize(16777215, 16777215);
    trackPointFrame->setFrameShape(QFrame::Shape::Box);
    QVBoxLayout *vLyt = new QVBoxLayout(trackPointFrame);
    vLyt->setMargin(0);
    vLyt->setSpacing(0);

    QWidget* widget = createTrackPointEnabledWidget();
    if (nullptr == widget) {
        return nullptr;
    }
    vLyt->addWidget(widget);

    return trackPointFrame;
}

QWidget* WaylandTouchpadUI::createTrackPointEnabledWidget()
{
    QByteArray schema(kTrackPointSchemas);
    if (!QGSettings::isSchemaInstalled(schema)) {
        qWarning() << "GSetting org.ukui.peripherals-trackpoint don't install";
        return nullptr;
    }

    QGSettings* trackPointGsetting = new QGSettings(schema, QByteArray(), this);
    if (!trackPointGsetting->keys().contains(kTrackPointDisableKey)) {
        qWarning() << "GSetting org.ukui.peripherals-trackpoint don't has key" << kTrackPointDisableKey;
        return nullptr;
    }

    kdk::KSwitchButton* btn = new kdk::KSwitchButton();
    btn->setChecked(trackPointGsetting->get(kTrackPointDisableKey).toBool());

    connect(btn, &kdk::KSwitchButton::stateChanged, this, [this, trackPointGsetting](bool status) {
        trackPointGsetting->set(kTrackPointDisableKey, status);
        buriedSettings(kTrackPointDisableKey, "settings", status ? "true":"false");
    });

    connect(trackPointGsetting, &QGSettings::changed, this, [this, btn, trackPointGsetting](const QString& key) {
        if (key == kTrackPointDisableKey) {
            const bool currentValue = trackPointGsetting->get(kTrackPointDisableKey).toBool();
            if (btn->isChecked() != currentValue) {
                btn->setChecked(currentValue);
            }
        }
    });

    //~ contents_path /Touchpad/TrackPoint
    QWidget* widget = createSingleWidget(tr("TrackPoint"), btn);
    setupWidgetAccessibility(btn, tr("TrackPoint"), switchAccessibleDescription(kTrackPointDisableKey));
    return widget;
}

bool WaylandTouchpadUI::getSoftwareEdgeSlidingGestureCap()
{
    if(m_touchpadGsetting && m_touchpadGsetting->keys().contains(kSoftwareEdgeSlidingGestureCapKey)) {
        return m_touchpadGsetting->get(kSoftwareEdgeSlidingGestureCapKey).toBool();
    }

    return false;
}

QWidget* WaylandTouchpadUI::createGestureSettingsWidget()
{
    QWidget* gestureWidget = new QWidget(this);

    QVBoxLayout* gestureLayout = new QVBoxLayout(gestureWidget);
    gestureLayout->setContentsMargins(0, 0, 0, 0);
    gestureLayout->setMargin(0);

    gestureLayout->addWidget(createGestureBaseSettingsFrame());
    gestureLayout->addSpacing(8);
    if (getSoftwareEdgeSlidingGestureCap()) {
        gestureLayout->addWidget(createEdgeGestureSettingsFrame());
        gestureLayout->addSpacing(8);
    }
    gestureLayout->addWidget(createScrollSettingsFrame());
    gestureLayout->addSpacing(8);
    gestureLayout->addWidget(createGestureConfigWidget());
    gestureLayout->addStretch();

    m_needUpdateVisibleWidgetList.append(gestureWidget);

    return gestureWidget;
}

QWidget* WaylandTouchpadUI::createGestureBaseSettingsFrame()
{
    QFrame *gestureBaseFrame = new QFrame(this);
    gestureBaseFrame->setMaximumSize(16777215, 16777215);
    gestureBaseFrame->setFrameShape(QFrame::Shape::Box);
    QVBoxLayout *vLyt = new QVBoxLayout(gestureBaseFrame);
    vLyt->setMargin(0);
    vLyt->setSpacing(0);

    vLyt->addWidget(createTapToClickSettingWidget());
    vLyt->addWidget(new kdk::KHLineFrame(this));

    vLyt->addWidget(createTwoFingerClickSettingWidget());
    vLyt->addWidget(new kdk::KHLineFrame(this));

    vLyt->addWidget(createTapAndDragSettingWidget());
    vLyt->addWidget(new kdk::KHLineFrame(this));

    vLyt->addWidget(createBottomRightCornerClickMenuSettingWidget());

    return gestureBaseFrame;
}

QWidget* WaylandTouchpadUI::createTapToClickSettingWidget()
{
    //~ contents_path /Touchpad/Touch and click on the touchpad
    return createSingleSwitchWidget(kClickKey, tr("Touch and click on the touchpad"));
}

QWidget* WaylandTouchpadUI::createTwoFingerClickSettingWidget()
{
    //~ contents_path /Touchpad/Two-finger tap for right-click
    return createSingleSwitchWidget(kTwoFingerClickKey, tr("Two-finger tap for right-click"));
}

QWidget* WaylandTouchpadUI::createTapAndDragSettingWidget()
{
    //~ contents_path /Touchpad/Double-click then drag to multi-select
    return createSingleSwitchWidget(kDoubleClickDragKey, tr("Double-click then drag to multi-select"));
}

QWidget* WaylandTouchpadUI::createBottomRightCornerClickMenuSettingWidget()
{
    //~ contents_path /Touchpad/Click the bottom-right corner for right-click
    return createSingleSwitchWidget(kBottomRightClickKey, tr("Click the bottom-right corner for right-click"));
}

QWidget* WaylandTouchpadUI::createEdgeGestureSettingsFrame()
{
    QFrame *edgeGestureFrame = new QFrame(this);
    edgeGestureFrame->setMaximumSize(16777215, 16777215);
    edgeGestureFrame->setFrameShape(QFrame::Shape::Box);
    QVBoxLayout *vLyt = new QVBoxLayout(edgeGestureFrame);
    vLyt->setMargin(0);
    vLyt->setSpacing(0);

    vLyt->addWidget(createTopEdgeGestureSettingWidget());
    vLyt->addWidget(new kdk::KHLineFrame(this));

    vLyt->addWidget(createRightEdgeGestureSettingWidget());
    vLyt->addWidget(new kdk::KHLineFrame(this));

    vLyt->addWidget(createLeftEdgeGestureSettingWidget());

    return edgeGestureFrame;
}

QWidget* WaylandTouchpadUI::createTopEdgeGestureSettingWidget()
{
    //~ contents_path /Touchpad/Scroll horizontally along the top edge with one finger to adjust video playback progress
    return createSingleSwitchWidget(kTopEdgeGestureKey, tr("Scroll horizontally along the top edge with one finger to adjust video playback progress"));
}

QWidget* WaylandTouchpadUI::createLeftEdgeGestureSettingWidget()
{
    //~ contents_path /Touchpad/Scroll vertically along the left edge with one finger to adjust brightness
    return createSingleSwitchWidget(kLeftEdgeGestureKey, tr("Scroll vertically along the left edge with one finger to adjust brightness"));
}

QWidget* WaylandTouchpadUI::createRightEdgeGestureSettingWidget()
{
    //~ contents_path /Touchpad/Scroll vertically along the right edge with one finger to adjust volume
    return createSingleSwitchWidget(kRightEdgeGestureKey, tr("Scroll vertically along the right edge with one finger to adjust volume"));
}

QWidget* WaylandTouchpadUI::createScrollSettingsFrame()
{
    QFrame *edgeGestureFrame = new QFrame(this);
    edgeGestureFrame->setMaximumSize(16777215, 16777215);
    edgeGestureFrame->setFrameShape(QFrame::Shape::Box);
    QVBoxLayout *vLyt = new QVBoxLayout(edgeGestureFrame);
    vLyt->setMargin(0);
    vLyt->setSpacing(0);

    vLyt->addWidget(createScrollAreaWidget());
    vLyt->addWidget(new kdk::KHLineFrame(this));

    vLyt->addWidget(createScrollSlideWidget());
    vLyt->addWidget(new kdk::KHLineFrame(this));

    vLyt->addWidget(createScaleWidget());

    return edgeGestureFrame;
}

//! \todo clean
QWidget* WaylandTouchpadUI::createScrollAreaWidget()
{
    QComboBox* scrollTypeComBox = new QComboBox(this);
    scrollTypeComBox->addItem(tr("Two-finger scrolling in the middle area"), V_FINGER_KEY);
    scrollTypeComBox->addItem(tr("Edge scrolling"), V_EDGE_KEY);
    scrollTypeComBox->addItem(tr("Disable scrolling"), N_SCROLLING);
    scrollTypeComBox->setCurrentIndex(scrollTypeComBox->findData(currentScrollingType()));
    m_scrollTypeComBox = scrollTypeComBox;
    m_previousScrollAreaIndex = scrollTypeComBox->currentIndex();
    if (QString::compare(V_EDGE_KEY, scrollTypeComBox->currentData().toString()) == 0) {
        m_touchpadGsetting->set(H_EDGE_KEY, true);
        m_touchpadGsetting->set(H_FINGER_KEY, false);
    }
    if (QString::compare(V_FINGER_KEY, scrollTypeComBox->currentData().toString()) == 0) {
        m_touchpadGsetting->set(H_EDGE_KEY, false);
        m_touchpadGsetting->set(H_FINGER_KEY, true);
    }
    if (QString::compare(N_SCROLLING, scrollTypeComBox->currentData().toString()) == 0) {
        m_touchpadGsetting->set(H_EDGE_KEY, false);
        m_touchpadGsetting->set(H_FINGER_KEY, false);
    }
    connect(scrollTypeComBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, scrollTypeComBox](int index) {
        QString data = scrollTypeComBox->itemData(index).toString();
        if (QString::compare(data, V_EDGE_KEY) == 0
                && m_touchpadGsetting->keys().contains(kRightEdgeGestureKey)
                && m_touchpadGsetting->get(kRightEdgeGestureKey).toBool()) {
            if (!confirmGestureConflict(true)) {
                QSignalBlocker blocker(scrollTypeComBox);
                scrollTypeComBox->setCurrentIndex(m_previousScrollAreaIndex);
                return;
            }
            if (m_touchpadGsetting->keys().contains(kRightEdgeGestureKey)) {
                m_touchpadGsetting->set(kRightEdgeGestureKey, false);
            }
        }
        applyScrollingType(data);
        buriedSettings("ScrollArea", "select", data);
        m_previousScrollAreaIndex = scrollTypeComBox->currentIndex();
    });

    //~ contents_path /Touchpad/Scrolling area
    QWidget *scrollAreaWidget = createSingleWidget(tr("Scrolling area"), scrollTypeComBox);
    setupWidgetAccessibility(scrollTypeComBox, tr("Scrolling area"),
                             tr("Choose how to scroll on the touchpad"));
    return scrollAreaWidget;
}

QWidget* WaylandTouchpadUI::createScrollSlideWidget()
{
    QComboBox* scrollSlidComBox = new QComboBox();
    scrollSlidComBox->addItem(tr("Natural Scrolling"));
    scrollSlidComBox->addItem(tr("Traditional Scrolling"));
    scrollSlidComBox->setCurrentIndex(m_touchpadGsetting->get(kScrollSlideKey).toBool() ? 0 : 1);

    connect(scrollSlidComBox,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index){
        m_touchpadGsetting->set(kScrollSlideKey, (index == 0));
        buriedSettings(kScrollSlideKey, "settings", (index == 0) ? "true":"false");
    });

    connect(m_touchpadGsetting, &QGSettings::changed, this, [this, scrollSlidComBox](const QString& key) {
        if (key == kScrollSlideKey) {
            const bool currentValue = m_touchpadGsetting->get(kScrollSlideKey).toBool();
                scrollSlidComBox->setCurrentIndex(currentValue ? 0 : 1);
        }
    });

    //~ contents_path /Touchpad/Scroll direction
    QWidget *scrollDirectionWidget = createSingleWidget(tr("Scroll direction"), scrollSlidComBox);
    setupWidgetAccessibility(scrollSlidComBox, tr("Scroll direction"),
                             tr("Choose natural scrolling or traditional scrolling"));
    return scrollDirectionWidget;
}

QWidget* WaylandTouchpadUI::createScaleWidget()
{
    //~ contents_path /Touchpad/Pinch to zoom
    return createSingleSwitchWidget(kScaleGestureKey, tr("Pinch to zoom"));
}

QWidget* WaylandTouchpadUI::createGestureConfigWidget()
{
    QFrame *configShowFrame = new QFrame(this);
    configShowFrame->setMaximumSize(16777215, 16777215);
    configShowFrame->setFrameShape(QFrame::Shape::Box);
    QVBoxLayout *vLyt = new QVBoxLayout(configShowFrame);
    vLyt->setMargin(0);
    vLyt->setSpacing(0);

    vLyt->addWidget(createGestureConfigGuideWidget());
    vLyt->addSpacing(24);
    vLyt->addWidget(createGestureConfigTabWidget());
    vLyt->addSpacing(24);
    vLyt->addWidget(new kdk::KHLineFrame(this));

    vLyt->addWidget(createGestureConfigTapSettingsWidget());
    vLyt->addWidget(new kdk::KHLineFrame(this));

    vLyt->addWidget(createGestureConfigSwipeUpSettingsWidget());
    vLyt->addWidget(new kdk::KHLineFrame(this));

    vLyt->addWidget(createGestureConfigSwipeDownSettingsWidget());
    vLyt->addWidget(new kdk::KHLineFrame(this));

    vLyt->addWidget(createGestureConfigSwipeLeftSettingsWidget());
    vLyt->addWidget(new kdk::KHLineFrame(this));

    vLyt->addWidget(createGestureConfigSwipeRightSettingsWidget());
//    vLyt->addWidget(new kdk::KHLineFrame(this));

    return configShowFrame;
}

QWidget* WaylandTouchpadUI::createGestureConfigGuideWidget()
{
    QWidget* widget = new QWidget(this);
    widget->setMaximumWidth(16777215);
    widget->setFixedHeight(166);

    QHBoxLayout *videoLayout = new QHBoxLayout(widget);
    videoLayout->setContentsMargins(0, 16, 0, 0);
    videoLayout->setSpacing(24);

    QLabel *gestureLabel = new QLabel(widget);
    QLabel *functionLabel = new QLabel(widget);

    videoLayout->addStretch();
    m_gestureMovie = new QMovie(widget);
    m_functionMovie = new QMovie(widget);
    m_gestureMovie->setScaledSize(QSize(262, 150));
    m_functionMovie->setScaledSize(QSize(262, 150));

    videoLayout->addWidget(gestureLabel);
    videoLayout->addWidget(functionLabel);
    videoLayout->addStretch();

    gestureLabel->setMovie(m_gestureMovie);
    functionLabel->setMovie(m_functionMovie);
    setupWidgetAccessibility(gestureLabel, tr("Gesture demonstration"),
                             tr("Animated preview of the selected touchpad gesture"));
    setupWidgetAccessibility(functionLabel, tr("Function demonstration"),
                             tr("Animated preview of the function triggered by the gesture"));

    return widget;
}

class FixedTabBar : public kdk::KTabBar
{
public:
    FixedTabBar(kdk::KTabBarStyle barStyle, QWidget* parent = nullptr) : kdk::KTabBar(barStyle, parent){

    }

protected:
    QSize tabSizeHint(int index) const override
    {
        return QSize(88,36);
    }

    QSize minimumTabSizeHint(int index) const override
    {
        return QSize(88,36);
    }
};

QWidget* WaylandTouchpadUI::createGestureConfigTabWidget()
{
    QWidget* widget = new QWidget(this);
    widget->setMaximumWidth(16777215);
    widget->setFixedHeight(36);

    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    hLayout->setContentsMargins(0, 0, 0, 0);

    FixedTabBar* tabBar = new FixedTabBar(kdk::KTabBarStyle::SegmentDark, widget);

    tabBar->addTab(tr("Three Finger"));
    tabBar->addTab(tr("Four Finger"));
    setupWidgetAccessibility(tabBar, tr("Gesture finger count"),
                             tr("Select three-finger or four-finger gesture configuration"));

    hLayout->addStretch();
    hLayout->addWidget(tabBar);
    hLayout->addStretch();

    connect(tabBar, &FixedTabBar::currentChanged, this, [this](int index){
        m_currentFingerShowType = TouchpadGestureFingerCount(index);
        emit fingerShowTypeChange(index);
    });
    return widget;
}

QWidget* WaylandTouchpadUI::createSwipeGestureWidget(int gestureId3Finger, int gestureId4Finger, const QString& displayText)
{
    // 1. 创建下拉框并添加选项（复用静态列表）
    QComboBox* comBox = new QComboBox();
    if (gestureId3Finger == GESTURE_TOUCHPAD_3FINGER_TAP){
        for (const auto& pair : tapGestureFuncItems) {
            comBox->addItem(tr(pair.first.toUtf8()), pair.second);
        }
    } else if(gestureId3Finger == GESTURE_TOUCHPAD_3FINGER_SWIPE_UP
              || gestureId3Finger == GESTURE_TOUCHPAD_3FINGER_SWIPE_LEFT) {
        for (const auto& pair : swipeGestureUpLeftFuncItems) {
            comBox->addItem(tr(pair.first.toUtf8()), pair.second);
        }
    }
    else {
        for (const auto& pair : swipeGestureDownRightFuncItems) {
            comBox->addItem(tr(pair.first.toUtf8()), pair.second);
        }
    }

    // 2. 封装更新逻辑（仅手势ID动态获取）
    auto updateCombo = [this, comBox, gestureId3Finger, gestureId4Finger]() {
        // 根据手指类型获取对应手势ID
        int gestureId = (m_currentFingerShowType == TouchpadGestureFingerCount::threeFinger) ? gestureId3Finger : gestureId4Finger;

        quint32 funcId = 0;
        bool isEnable = false;
        getGestureStatus(gestureId, funcId, isEnable);
        comBox->setCurrentIndex(comBox->findData(isEnable ? funcId : 0));

        playDefaultGif();
    };

    // 3. 初始化选中状态
    updateCombo();

    // 4. 下拉框选中事件（更新手势配置）
    connect(comBox,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this, comBox, gestureId3Finger, gestureId4Finger](){
        int gestureId = (m_currentFingerShowType == TouchpadGestureFingerCount::threeFinger) ? gestureId3Finger : gestureId4Finger;
        updateGestureCurrentFuncIdSimple(gestureId, comBox->currentData().toUInt());
        QString str = QString("%1 %2").arg("gesture id =")
            .arg(QString::number((m_currentFingerShowType == TouchpadGestureFingerCount::threeFinger) ? gestureId3Finger : gestureId4Finger));
        buriedSettings(str, "setting", comBox->currentText());
    });

    connect(comBox,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::highlighted),
            this, [this, comBox, gestureId3Finger, gestureId4Finger](int index){
        int gestureId = (m_currentFingerShowType == TouchpadGestureFingerCount::threeFinger) ? gestureId3Finger : gestureId4Finger;
        playGif(gestureId, comBox->itemData(index).toInt());
    });

    // 5. 手指类型切换事件（刷新下拉框）
    connect(this, &WaylandTouchpadUI::fingerShowTypeChange, this, updateCombo);

    // 6. 返回最终控件
    QWidget *gestureWidget = createSingleWidget(displayText, comBox);
    setupWidgetAccessibility(comBox, displayText,
                             tr("Choose the action performed by this gesture"));
    if (gestureId3Finger == GESTURE_TOUCHPAD_3FINGER_TAP) {
        m_tapWidget = gestureWidget;
    }
    return gestureWidget;
}

/*
 * 无          0, "None"
 * 打开搜索     46, "Global Search"
 * 通知中心     40, "Notification Center"
 * 播放/暂停    50, "Play"
               51, "Pause"
 * 鼠标中键     43, "Middle Click"
 */
QWidget* WaylandTouchpadUI::createGestureConfigTapSettingsWidget()
{
//    //~ contents_path /Touchpad/Tap
    return createSwipeGestureWidget(GESTURE_TOUCHPAD_3FINGER_TAP,
                                    GESTURE_TOUCHPAD_4FINGER_TAP,
                                    tr("Tap"));
}

/*
 * 无          0, "None"
 * 切换应用     44, "Window Switch"
 * 任务视图     45, "Multi Taskview"
 * 显示桌面     12, "Show Desktop(Minimize All)"
 * 切换桌面     5, "Switch to Left Workspace"
               6, "Switch to Right Workspace"
               7, "Switch to Workspace 1"
               8, "Switch to Workspace 2"
               9, "Switch to Workspace 3"
               10, "Switch to Workspace 4"
               48, "Follow Gesture to Switch to Left Workspace"
               49, "Follow Gesture to Switch to Right Workspace"
 * 除了焦点中的应用之外，隐藏其他对象 14, "Minimize Other Windows"
 * 创建桌面     3, "New Workspace"
 * 删除桌面     4, "Delete Workspace"
 * 将窗口贴靠到左侧/右侧 24, "Tile Active Window to the Left"
                      25, "Tile Active Window to the Right"
 * 最大化窗口    17, "Maximize Active Window"
 * 最小化窗口    18, "Minimize Active Window"
 * 下一首        52, "Next Track"
 * 上一首        53, "Previous Track"
 * 静音          56, "Volume Mute"
 */
QWidget* WaylandTouchpadUI::createGestureConfigSwipeUpSettingsWidget()
{
    //~ contents_path /Touchpad/Swipe Up
    m_swipeUpWidget = createSwipeGestureWidget(GESTURE_TOUCHPAD_3FINGER_SWIPE_UP,
                                    GESTURE_TOUCHPAD_4FINGER_SWIPE_UP,
                                    tr("Swipe Up"));
    return m_swipeUpWidget;
}

QWidget* WaylandTouchpadUI::createGestureConfigSwipeDownSettingsWidget()
{
    //~ contents_path /Touchpad/Swipe Down
    m_swipeDownWidget = createSwipeGestureWidget(GESTURE_TOUCHPAD_3FINGER_SWIPE_DOWN,
                                    GESTURE_TOUCHPAD_4FINGER_SWIPE_DOWN,
                                    tr("Swipe Down"));
    return m_swipeDownWidget;
}

QWidget* WaylandTouchpadUI::createGestureConfigSwipeLeftSettingsWidget()
{
    //~ contents_path /Touchpad/Swipe Left
    m_swipeLeftWidget = createSwipeGestureWidget(GESTURE_TOUCHPAD_3FINGER_SWIPE_LEFT,
                                    GESTURE_TOUCHPAD_4FINGER_SWIPE_LEFT,
                                    tr("Swipe Left"));
    return m_swipeLeftWidget;
}

QWidget* WaylandTouchpadUI::createGestureConfigSwipeRightSettingsWidget()
{
    //~ contents_path /Touchpad/Swipe Right
    m_swipeRightWidget = createSwipeGestureWidget(GESTURE_TOUCHPAD_3FINGER_SWIPE_RIGHT,
                                    GESTURE_TOUCHPAD_4FINGER_SWIPE_RIGHT,
                                    tr("Swipe Right"));
    return m_swipeRightWidget;
}

QString WaylandTouchpadUI::currentScrollingType()
{
    if (m_touchpadGsetting->get(V_EDGE_KEY).toBool()) {
        return V_EDGE_KEY;
    }

    if (m_touchpadGsetting->get(V_FINGER_KEY).toBool()) {
        return V_FINGER_KEY;
    }

    return N_SCROLLING;
}

bool WaylandTouchpadUI::confirmGestureConflict(bool enablingEdgeScroll)
{
    QMessageBox msg(this->topLevelWidget());
    msg.setIcon(QMessageBox::Warning);
    if (enablingEdgeScroll) {
        msg.setText(tr("Gesture function conflict. Continuing the setting will turn off the right-edge scroll volume adjustment function."));
    } else {
        msg.setText(tr("Gesture function conflict. Continuing the setting will restore two-finger scrolling in the middle area."));
    }
    QPushButton *cancelBtn = msg.addButton(tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *acceptBtn = msg.addButton(tr("Enable"), QMessageBox::AcceptRole);
    setupWidgetAccessibility(cancelBtn, tr("Cancel"), tr("Cancel the change and keep current settings"));
    setupWidgetAccessibility(acceptBtn, tr("Enable"), tr("Continue and apply the new setting"));
    acceptBtn->setProperty("useButtonPalette", true);
    msg.setDefaultButton(cancelBtn);
    return msg.exec() == QDialog::Accepted;
}

void WaylandTouchpadUI::applyScrollingType(const QString& data)
{
    if (QString::compare(data, N_SCROLLING) == 0) {
        m_touchpadGsetting->set(V_EDGE_KEY, false);
        m_touchpadGsetting->set(H_EDGE_KEY, false);
        m_touchpadGsetting->set(V_FINGER_KEY, false);
        m_touchpadGsetting->set(H_FINGER_KEY, false);
    } else if (QString::compare(data, V_EDGE_KEY) == 0) {
        m_touchpadGsetting->set(V_EDGE_KEY, true);
        m_touchpadGsetting->set(H_EDGE_KEY, true);
        m_touchpadGsetting->set(V_FINGER_KEY, false);
        m_touchpadGsetting->set(H_FINGER_KEY, false);
    } else if (QString::compare(data, V_FINGER_KEY) == 0) {
        m_touchpadGsetting->set(V_EDGE_KEY, false);
        m_touchpadGsetting->set(H_EDGE_KEY, false);
        m_touchpadGsetting->set(V_FINGER_KEY, true);
        m_touchpadGsetting->set(H_FINGER_KEY, true);
    }
}

void WaylandTouchpadUI::buriedSettings(QString settingsName, QString action, QString value) const
{
    ukcc::UkccCommon::buriedSettings("TouchPad", settingsName, action, value);
}

///* slot functions */
void WaylandTouchpadUI::pointerSpeedSlot(int value)
{
    m_touchpadGsetting->set(kPointerSpeedKey, value/10);
    buriedSettings(kPointerSpeedKey, "settings", QString::number(value));
}

void WaylandTouchpadUI::playGif(uint gestureId, uint funcId)
{
    m_gestureMovie->setFileName(GestureMapper::getGifPathByGestureId(gestureId));
    m_gestureMovie->stop();
    m_gestureMovie->start();

    m_functionMovie->setFileName(FuncMapper::getGifPathByFuncId(funcId));
    m_functionMovie->stop();
    m_functionMovie->start();
}

//void WaylandTouchpadUI::stopGif()
//{
//    m_gestureMovie->stop();
//    m_functionMovie->stop();
////     停止播放后，跳转到第一页
//    m_gestureMovie->jumpToFrame(0);
//    m_functionMovie->jumpToFrame(0);
//}


void WaylandTouchpadUI::showWarningDialog()
{
    int ret;
    QMessageBox msg(this->topLevelWidget());
    msg.setIcon(QMessageBox::Warning);
    msg.setText(tr("No functional mouse detected. Disabling the touchpad will make the computer inoperable. Do you want to disable the touchpad?"));
    QPushButton *cancelBtn = msg.addButton(tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *acceptBtn = msg.addButton(tr("Disable Touchpad"), QMessageBox::AcceptRole);
    setupWidgetAccessibility(cancelBtn, tr("Cancel"), tr("Keep the touchpad enabled"));
    setupWidgetAccessibility(acceptBtn, tr("Disable Touchpad"), tr("Disable the touchpad"));
    acceptBtn->setProperty("useButtonPalette", true);
    msg.setDefaultButton(cancelBtn);
    ret = msg.exec();

    switch (ret) {
    case QDialog::DialogCode::Accepted:
        m_touchpadDisableBtn->setChecked(false);
        break;
    case QDialog::DialogCode::Rejected:
        m_touchpadDisableBtn->setChecked(true);
        break;
    }
}

bool WaylandTouchpadUI::hasTrackPointDevice() const
{
    QDBusInterface interface("org.ukui.SettingsDaemon",
                             "/org/ukui/SettingsDaemon/InputDeviceManager",
                             "org.ukui.SettingsDaemon.InputDeviceManager",
                             QDBusConnection::sessionBus());
    QDBusReply<int> reply = interface.call("GetDevCountByType", "trackpoint");

    if (reply.isValid()) {
        return (reply.value() > 0);
    }
    qWarning() << reply.error().message();
    return true;
}

bool WaylandTouchpadUI::hasMouseDevice() const
{
    QDBusInterface interface("org.ukui.SettingsDaemon",
                             "/org/ukui/SettingsDaemon/InputDeviceManager",
                             "org.ukui.SettingsDaemon.InputDeviceManager",
                             QDBusConnection::sessionBus());
    QDBusReply<int> reply = interface.call("GetDevCountByType", "mouse");

    if (reply.isValid()) {
        return (reply.value() > 0);
    }
    qWarning() << reply.error().message();
    return true;
}

bool WaylandTouchpadUI::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_touchpadDisableBtn
            && m_touchpadDisableBtn->isChecked()
            && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton
                &&!hasMouseDevice()) {
            showWarningDialog();
            return true;
        }
    }
    return false;
}
