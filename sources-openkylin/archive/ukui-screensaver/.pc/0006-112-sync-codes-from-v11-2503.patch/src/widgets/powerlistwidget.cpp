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
    return MyListWidget::eventFilter(obj, event);
}

void PowerListWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint mousePos = event->pos();
    QList<PowerItemWidget*> itemList = this->findChildren<PowerItemWidget*>();
    for (PowerItemWidget* item : itemList)
    {
        if (item->geometry().contains(mousePos))
        {
            //  如果是点击的功能按钮，不发信号，防止点击锁屏按钮时不进入锁屏
            return QListWidget::mouseReleaseEvent(event);;
        }
    }
    //  只有点击空白处，才需要发送信号
    qDebug() << "Mouse press on list widget";
    Q_EMIT powerWidgetClicked();
    return QListWidget::mouseReleaseEvent(event);
}

void PowerListWidget::setPowerType(int type)
{
    m_powerType = type;
    qDebug() << "m_powerType = " << m_powerType;
    updateBtnShowFlag();
    onUpdateListInfo();
    updateWidgetSize();
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

    m_curFontSize = m_modelLockDialog->getCurFontSize();
    m_ptToPx = m_modelLockDialog->getPtToPx();
}

void PowerListWidget::initConnections()
{
    connect(this, &PowerListWidget::itemClicked, this, &PowerListWidget::onListItemClicked);
}
void PowerListWidget::updateBtnShowFlag()
{
    m_powerBtnNum = 0;
    for (int i = 0; i < sizeof(powerBtnList) / sizeof(powerBtnList[0]); i++) {
        if (powerBtnList[i].canFuncName == QString("CanSwitchUser")) {
            powerBtnList[i].m_show_flag = (m_powerType == SESSION && m_modelLockDialog->normalUserCount() > 1);
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }

        if (powerBtnList[i].canFuncName == QString("CanLockScreen")) {
            powerBtnList[i].m_show_flag = (m_powerType == SESSION && m_modelLockDialog->getCanLockScreen());
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }

        if (powerBtnList[i].canFuncName == QString("CanLogout")) {
            powerBtnList[i].m_show_flag = (m_powerType == SESSION && m_modelLockDialog->getCanLogout());
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }

        if (powerBtnList[i].canFuncName == QString("CanPowerOff")) {
            powerBtnList[i].m_show_flag = m_modelLockDialog->getCanPowerOff();
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }

        if (powerBtnList[i].canFuncName == QString("CanSuspend")) {
            powerBtnList[i].m_show_flag = m_modelLockDialog->getCanSuspend();
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }

        if (powerBtnList[i].canFuncName == QString("CheckSystemUpgradeReboot")) {
            powerBtnList[i].m_show_flag = (m_powerType == SESSION && m_modelLockDialog->checkSystemUpgrade());
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }

        if (powerBtnList[i].canFuncName == QString("CheckSystemUpgradeShutdown")) {
            powerBtnList[i].m_show_flag = (m_powerType == SESSION && m_modelLockDialog->checkSystemUpgrade());
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }

        if (powerBtnList[i].canFuncName == QString("CanReboot")) {
            powerBtnList[i].m_show_flag = m_modelLockDialog->getCanReboot();
            if (powerBtnList[i].m_show_flag)
                m_powerBtnNum += 1;
        }

        if (powerBtnList[i].canFuncName == QString("CanHibernate")) {
            powerBtnList[i].m_show_flag = m_modelLockDialog->getCanHibernate();
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

        PowerItemWidget *btnWdg = new PowerItemWidget(this);
        btnWdg->setObjectName(powerBtnList[i].setFuncName);
        QListWidgetItem *btnItem = new QListWidgetItem();
        btnItem->setSizeHint(QSize(powerBtnList[i].m_item_width, powerBtnList[i].m_item_height));

        this->insertItem(this->count(), btnItem);
        this->setItemWidget(btnItem, btnWdg);
        btnWdg->setText(powerBtnList[i].m_strName);
        btnWdg->setToolTip(powerBtnList[i].m_strToolTip);
        QString strIconTheme = "";
        if (powerBtnList[i].setFuncName == "Reboot") {
            strIconTheme = "ukui-system-restart-symbolic";
        } else if (powerBtnList[i].setFuncName == "PowerOff") {
            strIconTheme = "system-shutdown-symbolic";
        } else if (powerBtnList[i].setFuncName == "Logout") {
            strIconTheme = "system-logout-symbolic";
        } else if (powerBtnList[i].setFuncName == "Hibernate") {
            strIconTheme = "ukui-hibernate-symbolic";
        } else if (powerBtnList[i].setFuncName == "Suspend") {
            strIconTheme = "ukui-sleep-symbolic";
        } else if (powerBtnList[i].setFuncName == "SwitchUser") {
            strIconTheme = "ukui-stock-people-symbolic";
        } else if (powerBtnList[i].setFuncName == "LockScreen") {
            strIconTheme = "system-lock-screen-symbolic";
        };
        btnWdg->setIcon(drawSymbolicColoredPixmap(
            QIcon::fromTheme(strIconTheme, QIcon(powerBtnList[i].m_strIcon))
                .pixmap(powerBtnList[i].m_icon_width, powerBtnList[i].m_icon_width),
            "white"));
        btnWdg->setFontSize((16 + m_curFontSize) * m_ptToPx);
    }
}

void PowerListWidget::onListItemClicked(QListWidgetItem *item)
{
    QWidget *widget = itemWidget(item);
    PowerItemWidget *currentItem = qobject_cast<PowerItemWidget *>(widget);
    for (int i = 0; i < sizeof(powerBtnList) / sizeof(powerBtnList[0]); i++) {
        if (powerBtnList[i].setFuncName == currentItem->objectName()) {
            if (powerBtnList[i].setFuncName == "Hibernate" || powerBtnList[i].setFuncName == "Suspend") {
                QStringList sleepLockcheck = m_modelLockDialog->getSleepLockcheck();
                if (!sleepLockcheck.isEmpty()) {
                    Q_EMIT showInhibitWarning(sleepLockcheck, powerBtnList[i].m_inhibitType, false);
                    break;
                }
            }
            if (powerBtnList[i].setFuncName == "Reboot" || powerBtnList[i].setFuncName == "PowerOff"
                || powerBtnList[i].setFuncName == "Logout") {
                QStringList shutdownLockcheck = m_modelLockDialog->getShutdownLockcheck();
                if (shutdownLockcheck.count() > 0) {
                    Q_EMIT showInhibitWarning(shutdownLockcheck, powerBtnList[i].m_inhibitType, false);
                    break;
                } else if (
                    m_powerType == SAVER && m_modelLockDialog->getLoggedInUsersCount() >= 1
                    && powerBtnList[i].setFuncName != "Logout") {
                    Q_EMIT sureShutDown(powerBtnList[i].m_inhibitType, false);
                    break;
                } else if (m_modelLockDialog->getLoggedInUsersCount() > 1 && powerBtnList[i].setFuncName != "Logout") {
                    Q_EMIT mulUsersLogined(powerBtnList[i].m_inhibitType, false);
                    break;
                }
            }
            // qWarning() << __LINE__ << "=================" << powerBtnList[i].setFuncName << __FUNCTION__;
            if (powerBtnList[i].setFuncName == "LockScreen") {
                Q_EMIT lockScreenClicked();
            } else if (powerBtnList[i].setFuncName == "Suspend" || powerBtnList[i].setFuncName == "Hibernate") {
                Q_EMIT suspendClicked();
                QTimer::singleShot(
                    100, this, [=]() { Q_EMIT m_modelLockDialog->setPowerManager(powerBtnList[i].setFuncName); });
            } else if (powerBtnList[i].setFuncName == "SwitchUser") {
                //                Q_EMIT lockScreenClicked();
                //                Q_EMIT m_modelLockDialog->setPowerManager(powerBtnList[i].setFuncName);
                Q_EMIT switchuserClicked();
            } else {
                Q_EMIT m_modelLockDialog->setPowerManager(powerBtnList[i].setFuncName);
            }
            break;
        }
    }
}

void PowerListWidget::updateWidgetSize()
{
    QScreen *screen = QApplication::primaryScreen();
    if (!screen) {
        return;
    }
    QRect rect = screen->geometry();
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
        refreshTranslate();
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
