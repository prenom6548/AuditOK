/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QProcess>
#include <QTime>
#include <usermanual.h>

#include "menumodule.h"
#include "widgetstyle.h"
#include "windowmanage.hpp"
#include "kmenubutton.h"

menuModule::menuModule(QWidget *parent = nullptr) : QWidget(parent)
{
    init();
}

void menuModule::init()
{
    initAction();
}

void menuModule::initAction()
{
    iconSize = QSize(30, 30);
    menuButton = new QToolButton(this);
    menuButton->setAccessibleName("kylin-calculator_QToolButton_menuButton");
    menuButton->setToolTip(tr("Options"));
    menuButton->setProperty("isWindowButton", 0x1);
    menuButton->setProperty("useIconHighlightEffect", 0x2);
    menuButton->setPopupMode(QToolButton::InstantPopup);
    menuButton->setFixedSize(TITLE_BTN_W, TITLE_BTN_H);
    menuButton->setIconSize(QSize(16, 16));
    menuButton->setAutoRaise(true);
    menuButton->setIcon(QIcon::fromTheme("open-menu-symbolic"));

    kdk::KMenuButton* menuButtonFeedback = new kdk::KMenuButton();
    menuButtonFeedback->setObjectName("kylin-calculator_KMenuButton_menuButtonFeedback");
    m_menu = new QMenu();
    m_menu->setAccessibleName("kylin-calculator_QMenu_m_menu");
    QList<QAction *> actions;
    QAction *actionStandard = new QAction(m_menu);
    actionStandard->setObjectName("kylin-calculator_QAction_actionStandard");
    actionStandard->setText(tr("Standard"));
    QAction *actionScientific = new QAction(m_menu);
    actionScientific->setObjectName("kylin-calculator_QAction_actionScientific");
    actionScientific->setText(tr("Scientific"));
    QAction *actionExchangeRate = new QAction(m_menu);
    actionExchangeRate->setObjectName("kylin-calculator_QAction_actionExchangeRate");
    actionExchangeRate->setText(tr("Exchange Rate"));
    QAction *actionProgrammer = new QAction(m_menu);
    actionProgrammer->setObjectName("kylin-calculator_QAction_actionProgrammer");
    actionProgrammer->setText(tr("Programmer"));

    QAction *separator = new QAction(m_menu);
    separator->setObjectName("kylin-calculator_QAction_separator");
    separator->setSeparator(true);

    QAction *actionTheme = new QAction(m_menu);
    actionTheme->setObjectName("kylin-calculator_QAction_actionTheme");
    actionTheme->setText(tr("Theme"));
    QAction *actionHelp = new QAction(m_menu);
    actionHelp->setObjectName("kylin-calculator_QAction_actionHelp");
    actionHelp->setText(tr("Help"));
    QAction *actionAbout = new QAction(m_menu);
    actionAbout->setObjectName("kylin-calculator_QAction_actionAbout");
    actionAbout->setText(tr("About"));
    QAction *actionFeedback = menuButtonFeedback->feedbackAction();
    actionFeedback->setVisible(true);
    actionFeedback->setText(tr("Feedback"));
    QAction *actionQuit = new QAction(m_menu);
    actionQuit->setObjectName("kylin-calculator_QAction_actionQuit");
    actionQuit->setText(tr("Quit"));
    actions << actionStandard << actionScientific << actionExchangeRate << actionProgrammer
            << separator
            // << actionTheme
            << actionHelp << actionAbout << actionFeedback << actionQuit;

    m_menu->addActions(actions);
    menuButton->setMenu(m_menu);
    connect(m_menu, &QMenu::triggered, this, &menuModule::triggerMenu);
}

void menuModule::triggerMenu(QAction *act)
{
    QString str = act->text();
    if (tr("Quit") == str) {
        emit menuModuleClose();
    kdk_dia_upload_program_exit();
    } else if (tr("About") == str) {
        aboutAction();
    } else if (tr("Help") == str) {
        helpAction();
    } else if (tr("Standard") == str) {
        emit menuModuleChanged(QString("standard"));
    } else if (tr("Scientific") == str) {
        emit menuModuleChanged(QString("scientific"));
    } else if (tr("Exchange Rate") == str) {
        emit menuModuleChanged(QString("exchange rate"));
    } else if (tr("Programmer") == str) {
        emit menuModuleChanged(QString("programmer"));
    }
}

void menuModule::aboutAction()
{
    m_aboutWindow = new kdk::KAboutDialog(this, QIcon::fromTheme("kylin-calculator"), QString(tr("Calculator")),
                                          tr("Version: ") + getVersion());
    m_aboutWindow->setObjectName("kylin-calculator_KAboutDialog_m_aboutWindow");
    m_aboutWindow->setAttribute(Qt::WA_DeleteOnClose);
    m_aboutWindow->setBodyText(
        tr("Calculator is a lightweight calculator based on Qt5, which provides standard calculation, "
           "scientific calculation and exchange rate conversion."));
    m_aboutWindow->setBodyTextVisiable(true);
    m_aboutWindow->setWindowModality(Qt::WindowModal);
    m_aboutWindow->setWindowModality(Qt::ApplicationModal);

    m_aboutWindow->show();
    m_aboutWindow->exec();

    return;
}

void menuModule::helpAction()
{
    //    帮助点击事件处理
    kdk::UserManual userManual;
    if (!userManual.callUserManual("kylin-calculator")) {
        qCritical() << "user manual call fail!";
    }
}

QString menuModule::getVersion()
{
#ifdef NOTKAIMING
    QString version;
    QString command = "dpkg -l kylin-calculator | grep kylin-calculator";
    QProcess process;
    QStringList args;
    args << "-c" << command;
    process.start("bash", args);
    process.waitForFinished();
    process.waitForReadyRead();
    version = process.readAll();
    QStringList fields = version.split(QRegularExpression("[ \t]+"));
    if (fields.size() >= 3)
        version = fields.at(2);
    else
        version = "none";

    return version;
#else
    QString version = QString(qgetenv("APP_VERSION"));
    return version;
#endif

}

void menuModule::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1) {
        emit pullupHelp();
    } else {
        QWidget::keyPressEvent(event);
    }
}
