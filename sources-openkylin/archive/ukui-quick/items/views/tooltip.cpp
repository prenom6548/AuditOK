/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2011 Artur Duque de Souza <asouza@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2024 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
    \qmltype Tooltip
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \brief 提供高级工具提示组件，支持自定义内容和交互。

    Tooltip 是一个功能丰富的工具提示组件，相比简单的文本提示，它支持：
    - 自定义提示内容（可以是任意 QML Item）
    - 自动显示/隐藏机制
    - 可配置的显示延迟和超时
    - 支持交互式提示（鼠标可以移入提示框）
    - 支持跟随鼠标或固定位置显示
    - 支持富文本显示

    \section1 主要特性
    \list
        \li 支持自定义提示内容
        \li 支持纯文本和富文本
        \li 可配置显示延迟和超时时间
        \li 支持交互式提示框
        \li 支持多种弹出位置
        \li 自动检测鼠标进入/离开
    \endlist

    \section1 使用示例

    简单文本提示：
    \qml
    Rectangle {
        width: 100
        height: 100
        Tooltip {
            anchors.fill: parent
            mainText: "这是一个提示"
        }
    }
    \endqml

    自定义内容提示：
    \qml
    Rectangle {
        width: 100
        height: 100
        Tooltip {
            mainItem: Rectangle {
                width: 200
                height: 100
                color: "lightblue"
                Text {
                    anchors.centerIn: parent
                    text: "自定义提示内容"
                }
            }
        }
    }
    \endqml

    交互式提示：
    \qml
    Rectangle {
        width: 100
        height: 100
        Tooltip {
            mainText: "可以点击的提示"
            interactive: true
            timeout: -1  // 不自动隐藏
        }
    }
    \endqml

    \sa TooltipDialog, Dialog
*/

/*!
    \qmlproperty Item Tooltip::mainItem

    提示框中显示的自定义内容项。

    可以设置任意 QML Item 作为提示内容。如果未设置，将使用默认的文本提示样式。
*/

/*!
    \qmlproperty string Tooltip::mainText

    提示框显示的主要文本。

    支持纯文本和富文本（HTML）。如果文本可能是富文本，isRichText 属性会自动设置为 true。
*/

/*!
    \qmlproperty bool Tooltip::containsMouse
    \readonly

    鼠标是否在组件区域内（只读）。

    当鼠标进入组件区域时为 true，离开时为 false。
*/

/*!
    \qmlproperty enumeration Tooltip::location

    提示框的弹出位置。

    可选值：
    \list
        \li Dialog.Floating - 浮动位置（默认）
        \li Dialog.TopEdge - 顶部边缘
        \li Dialog.BottomEdge - 底部边缘
        \li Dialog.LeftEdge - 左侧边缘
        \li Dialog.RightEdge - 右侧边缘
    \endlist

    默认值为 Dialog.Floating。
*/

/*!
    \qmlproperty bool Tooltip::active

    是否启用工具提示。

    当设置为 false 时，鼠标悬停不会显示提示框。
    默认值为 true。
*/

/*!
    \qmlproperty int Tooltip::timeout

    提示框自动隐藏的超时时间（毫秒）。

    设置为 -1 表示不自动隐藏。
    默认值为 10000（10秒）。
*/

/*!
    \qmlproperty bool Tooltip::posFollowCursor

    提示框位置是否跟随鼠标光标。

    \list
        \li true - 提示框跟随鼠标位置显示
        \li false - 提示框在组件附近固定位置显示
    \endlist

    默认值为 true。
*/

/*!
    \qmlproperty int Tooltip::margin

    提示框与组件边缘的间距（像素）。

    仅当 posFollowCursor 为 false 且 location 设置为边缘位置时有效。
    默认值为 0。
*/

/*!
    \qmlproperty bool Tooltip::interactive

    提示框是否可交互。

    \list
        \li false - 鼠标离开组件区域时即隐藏提示框（默认）
        \li true - 鼠标可以移入提示框，提示框不会立即隐藏
    \endlist

    默认值为 false。
*/

/*!
    \qmlproperty bool Tooltip::isRichText
    \readonly

    主文本是否可能是富文本（只读）。

    根据 mainText 的内容自动判断。
*/

/*!
    \qmlmethod void Tooltip::showTooltip()

    显示工具提示。

    手动调用此方法可以立即显示提示框，无需等待延迟时间。
*/

/*!
    \qmlmethod void Tooltip::hideTooltip()

    隐藏工具提示（延迟隐藏）。

    停止显示计时器，并启动隐藏延迟。
*/

/*!
    \qmlmethod void Tooltip::hideImmediately()

    立即隐藏工具提示。

    与 hideTooltip() 不同，此方法会立即隐藏提示框，没有延迟。
*/

/*!
    \qmlsignal Tooltip::aboutToShow()

    提示框即将显示时发出的信号。

    可以在此信号中更新提示内容。
*/

/*!
    \qmlsignal Tooltip::toolTipVisibleChanged(bool toolTipVisible)

    提示框可见性改变时发出的信号。

    \a toolTipVisible 参数表示提示框当前是否可见。
*/

#include "tooltip.h"

#include <QTextDocument>

namespace UkuiQuick {

// ====== Tooltip ======
TooltipDialog *Tooltip::s_dialog = nullptr;
int Tooltip::s_dialogUsers = 0;

Tooltip::Tooltip(QQuickItem *parent)
        : QQuickItem(parent)
        , m_tooltipsEnabledGlobally(true)
        , m_containsMouse(false)
        , m_location(UkuiQuick::Dialog::PopupLocation::Floating)
        , m_active(true)
        , m_timeout(10000)
        , m_usingDialog(false)
        , m_interval(500)
        , m_posFollowCursor(true)
        , m_margin(0)
        , m_interactive(false)
        , m_isRichText(false)
        , m_deactivatedByClick(false)
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setFiltersChildMouseEvents(true);

    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    connect(m_showTimer, &QTimer::timeout, this, [this]() {
        if (!m_deactivatedByClick) {
            showTooltip();
        }
    });
}

Tooltip::~Tooltip()
{
    if (s_dialog && s_dialog->owner() == this) {
        s_dialog->setVisible(false);
    }

    if (m_usingDialog) {
        --s_dialogUsers;
    }

    if (s_dialogUsers == 0) {
        delete s_dialog;
        s_dialog = nullptr;
    }
}

QQuickItem *Tooltip::mainItem() const
{
    return m_mainItem.data();;
}

void Tooltip::setMainItem(QQuickItem *mainItem)
{
    if (m_mainItem.data() != mainItem) {
        m_mainItem = mainItem;

        Q_EMIT mainItemChanged();

        if (!isValid() && s_dialog && s_dialog->owner() == this) {
            s_dialog->setVisible(false);
        }
    }
}

QString Tooltip::mainText() const
{
    return m_mainText;
}

void Tooltip::setMainText(const QString &mainText)
{
    if (mainText == m_mainText) {
        return;
    }

    m_mainText = mainText;
    Q_EMIT mainTextChanged();

    if (m_isRichText != Qt::mightBeRichText(m_mainText)) {
        m_isRichText = !m_isRichText;
        Q_EMIT isRichTextChanged();
    }

    if (!isValid() && s_dialog && s_dialog->owner() == this) {
        s_dialog->setVisible(false);
    }
}

UkuiQuick::Dialog::PopupLocation Tooltip::location() const
{
    return m_location;
}

void Tooltip::setLocation(UkuiQuick::Dialog::PopupLocation location)
{
    if (m_location == location) {
        return;
    }
    m_location = location;
    Q_EMIT locationChanged();
}

bool Tooltip::containsMouse() const
{
    return m_containsMouse;
}

void Tooltip::setContainsMouse(bool contains)
{
    if (m_containsMouse != contains) {
        m_containsMouse = contains;
        Q_EMIT containsMouseChanged();
    }
    if (!contains) {
        tooltipDialogInstance()->dismiss();
    }
}

void Tooltip::setActive(bool active)
{
    if (m_active == active) {
        return;
    }

    m_active = active;
    if (!active) {
        m_showTimer->stop();
        tooltipDialogInstance()->dismiss();
    }
    Q_EMIT activeChanged();
}

void Tooltip::setTimeout(int timeout)
{
    m_timeout = timeout;
}

void Tooltip::showTooltip()
{
    if (!m_active) {
        return;
    }

    Q_EMIT aboutToShow();

    TooltipDialog *dlg = tooltipDialogInstance();

    if (dlg->isVisible()) {
        dlg->setVisible(false);
    }

    if (!mainItem()) {
        setMainItem(dlg->loadDefaultItem());
    }

    // Unset the dialog's old contents before reparenting the dialog.
    dlg->setMainItem(nullptr);

    UkuiQuick::Dialog::PopupLocation location = m_location;
    if (m_location == UkuiQuick::Dialog::PopupLocation::Floating) {
        QQuickItem *p = parentItem();
        while (p) {
            if (p->property("location").isValid()) {
                location = (UkuiQuick::Dialog::PopupLocation)p->property("location").toInt();
                break;
            }
            p = p->parentItem();
        }
    }

    if (mainItem()) {
        mainItem()->setProperty("tooltip", QVariant::fromValue(this));
        mainItem()->setVisible(true);
    }

    connect(dlg, &TooltipDialog::visibleChanged, this, &Tooltip::toolTipVisibleChanged, Qt::UniqueConnection);

    dlg->setHideTimeout(m_timeout);
    dlg->setOwner(this);
    dlg->setLocation(location);
    dlg->setPosFollowCursor(m_posFollowCursor);
    dlg->setMargin(m_margin);
    dlg->setVisualParent(this);
    dlg->setMainItem(mainItem());
    dlg->setInteractive(m_interactive);
    dlg->setVisible(true);
}

void Tooltip::hideTooltip()
{
    m_showTimer->stop();
    tooltipDialogInstance()->dismiss();
}

void Tooltip::hideImmediately()
{
    m_showTimer->stop();
    tooltipDialogInstance()->setVisible(false);
}

bool Tooltip::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        hideTooltip();
    }
    return QQuickItem::childMouseEventFilter(item, event);
}

void Tooltip::hoverEnterEvent(QHoverEvent *event)
{
    setContainsMouse(true);
    m_deactivatedByClick = false;
    if (!m_tooltipsEnabledGlobally || !m_active) {
        return QQuickItem::hoverEnterEvent(event);
    }

    if (!isValid()) {
        return QQuickItem::hoverEnterEvent(event);
    }

    if (tooltipDialogInstance()->isVisible()) {
        // We signal the tooltipmanager that we're "potentially interested,
        // and ask to keep it open for a bit, so other items get the chance
        // to update the content before the tooltip hides -- this avoids
        // flickering
        // It need to be considered only when other items can deal with tooltip area
        if (m_active && !m_deactivatedByClick) {
            tooltipDialogInstance()->keepalive();
            // FIXME: showToolTip needs to be renamed in sync or something like that
            showTooltip();
        }
    } else {
        m_showTimer->start(m_interval);
    }
    QQuickItem::hoverEnterEvent(event);
}

void Tooltip::hoverLeaveEvent(QHoverEvent *event)
{
    Q_UNUSED(event)
    setContainsMouse(false);
    m_showTimer->stop();
}

void Tooltip::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_deactivatedByClick = true;
    hideImmediately();
    event->ignore();
    QQuickItem::mousePressEvent(event);
}
TooltipDialog *Tooltip::tooltipDialogInstance()
{
    if (!s_dialog) {
        s_dialog = new TooltipDialog;
        s_dialogUsers = 1;
    }

    if (!m_usingDialog) {
        s_dialogUsers++;
        m_usingDialog = true;
    }

    return s_dialog;
}

bool Tooltip::isValid() const
{
    return m_mainItem || !mainText().isEmpty();
}

bool Tooltip::posFollowCursor()
{
    return m_posFollowCursor;
}

void Tooltip::setPosFollowCursor(bool follow)
{
    if(m_posFollowCursor == follow) {
        return;
    }
    m_posFollowCursor = follow;
    Q_EMIT posFollowCursorChanged();
}

int Tooltip::margin() const
{
    return m_margin;
}

void Tooltip::setMargin(int margin)
{
    if(m_margin != margin) {
        m_margin = margin;
        Q_EMIT marginChanged();
    }
}

void Tooltip::setInteractive(bool interactive)
{
    if (m_interactive == interactive) {
        return;
    }
    m_interactive = interactive;
    Q_EMIT interactiveChanged();
}
} // UkuiQuick
