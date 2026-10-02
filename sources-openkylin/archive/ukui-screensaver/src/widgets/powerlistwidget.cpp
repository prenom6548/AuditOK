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
#include <QKeyEvent>
#include <QScreen>
#include <QApplication>
#include "powerlistwidget.h"
#include "commonfunc.h"
#include <QGSettings>

#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

PowerListWidget::PowerListWidget(LockDialogModel *model, QWidget *parent)
    : m_modelLockDialog(model), MyListWidget(parent)
{
    //    updateBtnShowFlag();
    initUI();
    initConnections();
}

PowerListWidget::~PowerListWidget() {}

bool PowerListWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) && m_lastRow == -1) {
            return true;
        }

        if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) {
            return true;
        }

        MyListWidget *listWidget = qobject_cast<MyListWidget *>(obj);
        if (listWidget) {
            if (listWidget && listWidget->hasFocus()) { // Tab键切入时，设置焦点状态
                listWidget->setCurrentRow(listWidget->currentRow(), QItemSelectionModel::SelectionFlag::SelectCurrent);
                listWidget->currentItemChanged(listWidget->currentItem(), nullptr);
            } else { // Tab键切出时，清空焦点状态
                listWidget->setCurrentRow(listWidget->currentRow(), QItemSelectionModel::SelectionFlag::Clear);
            }
        }
    } else if (event->type() == QEvent::MouseButtonRelease) { // 电源管理界面点击后清空选中状态
        MyListWidget *listWidget = qobject_cast<MyListWidget *>(obj);
        if (listWidget)
            this->setCurrentRow(0, QItemSelectionModel::SelectionFlag::Clear);
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) && m_lastRow == -1) {
            return true;
        }

        if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) {
            return true;
        }
    }
    return MyListWidget::eventFilter(obj, event);
}

void PowerListWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint mousePos = event->pos();
    QListWidgetItem *clickedItem = this->itemAt(mousePos);

    if (!clickedItem) {
        // 点击在列表的完全空白区域
        qDebug() << "Clicked on blank area of list, closing power widget";
        Q_EMIT powerWidgetClicked();
    }
}

void PowerListWidget::setPowerType(int type)
{
    m_powerType = type;
    qDebug() << "m_powerType = " << m_powerType;
    updateBtnShowFlag();
    onUpdateListInfo();
    updateWidgetSize(m_rectListWidget);
}

void PowerListWidget::initUI()
{
    setFlow(QListWidget::LeftToRight);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setProperty("isWrapping", QVariant(true));
    setResizeMode(QListView::Adjust);
    QString powerListStyle = "QListWidget{background:rgba(255, 255, 255, 0%); border: 0px;}"
                             "QListWidget::item{background:rgba(255, 255, 255, 0%);}";
    setStyleSheet(powerListStyle);
    this->verticalScrollBar()->setStyleSheet(
        "QScrollBar{ background: transparent; margin-top:3px;margin-bottom:3px ; }"
        "QScrollBar:vertical{width: 6px;background: transparent;border-radius:3px;}"
        "QScrollBar::handle:vertical{width: 6px; background: rgba(255,255,255, 40); border-radius:3px;}"
        "QScrollBar::handle:vertical:hover{width: 6px; background: rgba(255,255,255, 60); border-radius:3px;}"
        "QScrollBar::add-line:vertical{width:0px;height:0px}"
        "QScrollBar::sub-line:vertical{width:0px;height:0px}");
    //    powerBtnList[0].func = &PowerListWidget::setSystemSuspend1();
}

void PowerListWidget::initConnections()
{
    connect(this, &PowerListWidget::itemClicked, this, &PowerListWidget::onListItemClicked);
}

void PowerListWidget::updateBtnShowFlag()
{
    m_powerBtnNum = 0;

    const auto upgradeInfo = Q_EMIT m_modelLockDialog->checkSystemUpgrade();
    bool isCheckHidden = false;
    if (upgradeInfo.first) {
        m_upgradeTimeRestart = upgradeInfo.second;
        m_upgradeTimeShutdown = upgradeInfo.second;
        isCheckHidden = Q_EMIT m_modelLockDialog->checkHideUpdate();
    }
    for (int i = 0; i < sizeof(powerBtnList) / sizeof(powerBtnList[0]); i++) {
        if (powerBtnList[i].canFuncName == QString("CanSwitchUser")) {
            powerBtnList[i].m_show_flag
                = (m_powerType == SESSION && m_modelLockDialog->normalUserCount() > 1
                   && m_modelLockDialog->getLockCanSwitchUser() && canAction("switchuser"));
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }
        bool canDoAction = false;
        if (powerBtnList[i].canFuncName == QString("CanLockScreen")) {
            canDoAction = Q_EMIT m_modelLockDialog->getCanLockScreen();
            powerBtnList[i].m_show_flag = (m_powerType == SESSION && canDoAction && canAction("lockscreen"));
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }

        if (powerBtnList[i].canFuncName == QString("CanLogout")) {
            canDoAction = Q_EMIT m_modelLockDialog->getCanLogout();
            powerBtnList[i].m_show_flag = (m_powerType == SESSION && canDoAction && canAction("logout"));
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }

        if (powerBtnList[i].canFuncName == QString("CanPowerOff")) {
            canDoAction = Q_EMIT m_modelLockDialog->getCanPowerOff();
            powerBtnList[i].m_show_flag = (!isCheckHidden) && canDoAction && canAction("shutdown");
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }

        if (powerBtnList[i].canFuncName == QString("CanSuspend")) {
            canDoAction = Q_EMIT m_modelLockDialog->getCanSuspend();
            powerBtnList[i].m_show_flag = canDoAction && canAction("suspend");
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }

        if (powerBtnList[i].canFuncName == QString("CheckSystemUpgradeReboot")) {
            canDoAction = upgradeInfo.first;
            qDebug() << "upgrade info (reboot)" << canDoAction << m_upgradeTimeRestart;
            powerBtnList[i].m_show_flag = (m_powerType == SESSION && canDoAction);
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }

        if (powerBtnList[i].canFuncName == QString("CheckSystemUpgradeShutdown")) {
            canDoAction = upgradeInfo.first;
            qDebug() << "upgrade info (shutdown)" << canDoAction << m_upgradeTimeShutdown;
            powerBtnList[i].m_show_flag = (m_powerType == SESSION && canDoAction);
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }

        if (powerBtnList[i].canFuncName == QString("CanReboot")) {
            canDoAction = Q_EMIT m_modelLockDialog->getCanReboot();
            powerBtnList[i].m_show_flag = (!isCheckHidden) && canDoAction && canAction("restart");
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }

        if (powerBtnList[i].canFuncName == QString("CanHibernate")) {
            canDoAction = Q_EMIT m_modelLockDialog->getCanHibernate();
            powerBtnList[i].m_show_flag = canDoAction && canAction("hibernate");
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }
    }
    qInfo() << "m_powerBtnNum = " << m_powerBtnNum;
}

void PowerListWidget::onUpdateListInfo()
{
    this->clear();

    for (int i = 0; i < sizeof(powerBtnList) / sizeof(powerBtnList[0]); i++) {
        // qDebug() << powerBtnList[i].m_strName << powerBtnList[i].setFuncName << powerBtnList[i].m_show_flag;
        if (!powerBtnList[i].m_show_flag)
            continue;

        PowerItemWidget *btnWdg = new PowerItemWidget(powerBtnList[i].setFuncName,this);
        btnWdg->setObjectName(powerBtnList[i].setFuncName);
        connect(btnWdg, &PowerItemWidget::iconClicked, [this, btnWdg]() {
            for (int row = 0; row < this->count(); ++row) {
                if (this->itemWidget(this->item(row)) == btnWdg) {
                    qDebug() << "Icon clicked, triggering power action for row:" << row;
                    Q_EMIT itemClicked(this->item(row));
                    break;
                }
            }
        });
        //item内，icon外区域退出会话管理界面
        connect(btnWdg, &PowerItemWidget::nonIconAreaClicked, [this]() {
            Q_EMIT powerWidgetClicked();
        });
        KDK_EXTEND_ALL_INFO_FORMAT(btnWdg, "PowerBtnWdg", QString("%1_InitialPowerItem").arg(i), "some power button of tools");
        QListWidgetItem *btnItem = new QListWidgetItem();
        btnItem->setSizeHint(QSize(powerBtnList[i].m_item_width, powerBtnList[i].m_item_height));
        this->insertItem(this->count(), btnItem);
        this->setItemWidget(btnItem, btnWdg);
        if (powerBtnList[i].setFuncName == QStringLiteral("UpgradeThenRestart")) {
            btnWdg->setUpgradeTimeText(m_upgradeTimeRestart);
        }
        if (powerBtnList[i].setFuncName == QStringLiteral("UpgradeThenShutdown")) {
            btnWdg->setUpgradeTimeText(m_upgradeTimeShutdown);
        }
        btnWdg->setText(powerBtnList[i].m_strName);
        btnWdg->setToolTip(powerBtnList[i].m_strToolTip);
        QString strIconTheme = "";
        if (powerBtnList[i].setFuncName == "Reboot") {
            strIconTheme = "system-reboot-symbolic";
        } else if (powerBtnList[i].setFuncName == "PowerOff") {
            strIconTheme = "system-shutdown-symbolic";
        } else if (powerBtnList[i].setFuncName == "Logout") {
            strIconTheme = "system-logout-symbolic";
        } else if (powerBtnList[i].setFuncName == "Hibernate") {
            strIconTheme = "ukui-hibernate-symbolic";
        } else if (powerBtnList[i].setFuncName == "Suspend") {
            strIconTheme = "ukui-sleep-symbolic";
        } else if (powerBtnList[i].setFuncName == "SwitchUser") {
            strIconTheme = "system-users-symbolic";
        } else if (powerBtnList[i].setFuncName == "LockScreen") {
            strIconTheme = "system-lock-screen-symbolic";
        };
        btnWdg->setIcon(drawSymbolicColoredPixmap(
            QIcon::fromTheme(strIconTheme, QIcon(powerBtnList[i].m_strIcon))
                .pixmap(powerBtnList[i].m_icon_width, powerBtnList[i].m_icon_width),
            "white"));
        btnWdg->setFontSize((16 + m_modelLockDialog->getCurFontSize()) * m_modelLockDialog->getPtToPx());
    }
}

void PowerListWidget::onListItemClicked(QListWidgetItem *item)
{
    QWidget *widget = itemWidget(item);
    m_modelLockDialog->updateLoggedInUsersCount();
    PowerItemWidget *currentItem = qobject_cast<PowerItemWidget *>(widget);
    if (currentItem) {
        for (int i = 0; i < sizeof(powerBtnList) / sizeof(powerBtnList[0]); i++) {
            if (powerBtnList[i].setFuncName == currentItem->objectName()) {
                if (m_powerType == SAVER && m_modelLockDialog->getLoggedInUsersCount() >= 1
                    && (powerBtnList[i].setFuncName == "Reboot" || powerBtnList[i].setFuncName == "PowerOff")
                    && m_modelLockDialog->getShutdownLockcheck().isEmpty()) {
                    Q_EMIT sureShutDown(powerBtnList[i].m_inhibitType);
                    break;
                }
                if (powerBtnList[i].setFuncName == "LockScreen") {
                    Q_EMIT lockScreenClicked();
                } else if (powerBtnList[i].setFuncName == "Suspend" || powerBtnList[i].setFuncName == "Hibernate") {
                    Q_EMIT suspendClicked(powerBtnList[i].setFuncName);
                } else if (powerBtnList[i].setFuncName == "SwitchUser") {
                    Q_EMIT switchuserClicked();
                } else {
                    if (!m_modelLockDialog->getIsLegacy() &&
                            (powerBtnList[i].setFuncName == "Reboot" || powerBtnList[i].setFuncName == "PowerOff" || powerBtnList[i].setFuncName == "Logout")) {
                        Q_EMIT doPowerAction(powerBtnList[i].setFuncName, true);
                    } else {
                        Q_EMIT doPowerAction(powerBtnList[i].setFuncName);
                    }
                }
                break;
            }
        }
    }
}

void PowerListWidget::updateWidgetSize(const QRect &geometry)
{
    QRect rect = geometry;
    if (rect.isEmpty()) {
        rect = m_rectListWidget;
    }
    m_rectListWidget = rect;
    if (BTN_ITEM_SIZE_WIDTH * m_powerBtnNum < rect.width()) {
        if (BTN_ITEM_SIZE_HEIGHT < this->maxHeight()) {
            setFixedSize(QSize((BTN_ITEM_SIZE_WIDTH + 3) * m_powerBtnNum, BTN_ITEM_SIZE_HEIGHT));
            m_scrollIsShow = false;
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        } else {
            setFixedSize(QSize((BTN_ITEM_SIZE_WIDTH + 3) * m_powerBtnNum, this->maxHeight()));
            m_scrollIsShow = true;
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        }
    } else { // 宽度超过屏幕宽度，需要换行显示，取item宽度的倍数设为总宽度，防止显示不居中
        if (rect.width() > 0) {
            int pHeight = BTN_ITEM_SIZE_HEIGHT * (BTN_ITEM_SIZE_WIDTH * m_powerBtnNum / rect.width() + 1);
            if (pHeight < this->maxHeight()) {
                setFixedSize(QSize(
                    rect.width() / BTN_ITEM_SIZE_WIDTH * (BTN_ITEM_SIZE_WIDTH + 3),
                    BTN_ITEM_SIZE_HEIGHT * (BTN_ITEM_SIZE_WIDTH * m_powerBtnNum / rect.width() + 1)));
                m_scrollIsShow = false;
                setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            } else if (
                (rect.width() / BTN_ITEM_SIZE_WIDTH * BTN_ITEM_SIZE_WIDTH + 10) > rect.width()) { // 需要显示滚动条
                setFixedSize(
                    QSize((rect.width() / BTN_ITEM_SIZE_WIDTH - 1) * BTN_ITEM_SIZE_WIDTH + 10, this->maxHeight()));
                m_scrollIsShow = true;
                setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            } else {
                setFixedSize(QSize(rect.width() / BTN_ITEM_SIZE_WIDTH * BTN_ITEM_SIZE_WIDTH + 10, this->maxHeight()));
                m_scrollIsShow = true;
                setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            }
        }
    }
}

void PowerListWidget::setSystemPowerStatus(QString statusName)
{
    qWarning() << __LINE__ << __FUNCTION__ << statusName;
}

void PowerListWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        if (!m_modelLockDialog->isSessionTools()) { // 首次通过快捷键打开会话管理器界面会收到changeEvent信号导致clear当前list出现焦点丢失问题
            refreshTranslate();
        }
    }
    return QListWidget::changeEvent(event);
}

void PowerListWidget::refreshTranslate()
{
    for (int i = 0; i < sizeof(powerBtnList) / sizeof(powerBtnList[0]); i++) {
        if (powerBtnList[i].setFuncName == "Hibernate") {
            powerBtnList[i].m_strName = tr("Hibernate");
            powerBtnList[i].m_strToolTip = tr("Turn off your computer, but the app stays open, When the computer is "
                                              "turned on, it can be restored to the state you left");
        } else if (powerBtnList[i].setFuncName == "Suspend") {
            powerBtnList[i].m_strName = tr("Suspend");
            powerBtnList[i].m_strToolTip = tr("The computer stays on, but consumes less power, The app stays open and "
                                              "can quickly wake up and revert to where you left off");
        } else if (powerBtnList[i].setFuncName == "Reboot") {
            powerBtnList[i].m_strName = tr("Restart");
            powerBtnList[i].m_strToolTip = tr("Close all apps, and then restart your computer");
        } else if (powerBtnList[i].setFuncName == "PowerOff") {
            powerBtnList[i].m_strName = tr("Shut Down");
            powerBtnList[i].m_strToolTip = tr("Close all apps, and then shut down your computer");
        } else if (powerBtnList[i].setFuncName == "Logout") {
            powerBtnList[i].m_strName = tr("Log Out");
            powerBtnList[i].m_strToolTip
                = tr("The current user logs out of the system, terminates the session, and returns to the login page");
        } else if (powerBtnList[i].setFuncName == "SwitchUser") {
            powerBtnList[i].m_strName = tr("SwitchUser");
        } else if (powerBtnList[i].setFuncName == "LockScreen") {
            powerBtnList[i].m_strName = tr("LockScreen");
        } else if (powerBtnList[i].setFuncName == "UpgradeThenRestart") {
            powerBtnList[i].m_strName = tr("UpgradeThenRestart");
        } else if (powerBtnList[i].setFuncName == "UpgradeThenShutdown") {
            powerBtnList[i].m_strName = tr("UpgradeThenShutdown");
        }
    }
    onUpdateListInfo();
}

bool PowerListWidget::canAction(const QString power)
{
    if (!QGSettings::isSchemaInstalled("org.ukui.SettingsDaemon.plugins.globalManager")) {
        return true;
    }

    QGSettings globalSettings("org.ukui.SettingsDaemon.plugins.globalManager");

    QStringList hideList = globalSettings.get("disable-shutdown-option").toStringList();
    if (hideList.contains(power)) {
        return false;
    }

    return true;
}

void PowerListWidget::setDefaultFocus()
{
    QStringList priorityList;
    if (m_modelLockDialog->isSessionTools()) {
        priorityList << "LockScreen" << "PowerOff" << "Reboot" << "Suspend"
                     << "Hibernate" << "Logout" << "SwitchUser";
    } else {
        priorityList << "PowerOff" << "Reboot" << "Suspend" << "Hibernate";
    }

    int targetRow = -1;
    for (const QString &funcName : priorityList) {
        for (int i = 0; i < this->count(); i++) {
            QWidget *itemWidget = this->itemWidget(this->item(i));
            if (itemWidget && itemWidget->objectName() == funcName) {
                targetRow = i;
                break;
            }
        }
        if (targetRow >= 0)
            break;
    }

    if (targetRow < 0)
        targetRow = 0;

    m_lastRow = targetRow;
    this->setCurrentRow(targetRow, QItemSelectionModel::SelectionFlag::SelectCurrent);
    Q_EMIT currentItemChanged(this->item(targetRow), nullptr);
}
