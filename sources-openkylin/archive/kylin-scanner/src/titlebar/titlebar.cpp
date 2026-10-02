/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#include "titlebar.h"
#include "ui_titlebar.h"
#include "../about/custom_about_dialog.h"
#include <QMessageBox>
#include <kysdk/applications/gsettingmonitor.h>
#include <QTimer>
#include <kysdk/applications/usermanual.h>

TitleBar::TitleBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TitleBar),
    m_menu(new QMenu(this))
{
    ui->setupUi(this);
    ui->m_logoBtn->setProperty("useIconHighlightEffect", 0x0);
    ui->m_logoBtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->m_logoBtn->setFlat(true);
    ui->m_logoBtn->setCheckable(false);
    ui->m_logoBtn->setStyleSheet("QPushButton{border:0px; border-radius:4px; background:transparent;}");

    ui->m_closeBtn->setAttribute(Qt::WA_AlwaysShowToolTips, true);
    ui->m_maxBtn->setAttribute(Qt::WA_AlwaysShowToolTips, true);
    ui->m_minBtn->setAttribute(Qt::WA_AlwaysShowToolTips, true);
    ui->m_OptionBtn->setAttribute(Qt::WA_AlwaysShowToolTips, true);

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    this->setWindowTitle("Scanner");
    if (QGSettings::isSchemaInstalled(UKUI_THEME_GSETTING_PATH)) {
        m_themeData = new QGSettings(UKUI_THEME_GSETTING_PATH);
    }

    // appname
    QFont appnameFont(m_themeData->get("systemFont").toString(), m_themeData->get("systemFontSize").toFloat());
    ui->m_appnameLabel->setFont(appnameFont);

    m_menu->addAction(tr("Refresh List"), this, [=](){
        if(!g_sane_object->isOnScanning && g_sane_object->onDetection == false){
            // 1、关闭热插拔
            emit g_user_signal->setExitFlagTrueSignal();
            // 2、检测设备 & 打开热插拔
            // 正在刷新列表，请等待刷新成功信息...
            QString msg = QApplication::tr("Refreshing list. Please wait for the refresh success information...");
            g_user_signal->warnMsg(msg);
            g_user_signal->setScanIconDisableSignal();
            QTimer::singleShot(2000, this, [this]() {emit g_user_signal->refreshListSignal();});
        }else{
            // 弹窗告诉用户扫描期间不支持识别设备
            QString msg = QApplication::tr("Scanner is on detecting...");
            g_user_signal->warnMsg(msg);
        }
    });

    m_menu->addAction(tr("Help"), this, [=]() {
        showHelpDialog();
    });//, QKeySequence(Qt::Key_F1)

    m_menu->addAction(tr("About"), this, [=]() {
        CustomAboutDialog dialog(window());
        dialog.exec();
    });

    m_menu->addAction(tr("Exit"), [=]() {
        qDebug() << "window close.";
        if (g_user_signal->exitWindowWithSaveFlag) {
            int saveFlag = warnCloseWindow();
            if (saveFlag == 1) {
                qDebug() << "close window save as";
                g_user_signal->closeWindowSaveAs(true);
            }
        }
        g_user_signal->exitApplication();
    } );
    ui->m_OptionBtn->setMenu(m_menu);
    connect(m_themeData, &QGSettings::changed, this, &TitleBar::fontSizeChanged);
}

TitleBar::~TitleBar()
{
    delete ui;
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (window()->isMaximized()) {
            updateMaxButtonStatus(false);
            window()->showNormal();
        } else if (! window()->isFullScreen()) {
            updateMaxButtonStatus(true);
            window()->showMaximized();
        }
    }
    QWidget::mouseDoubleClickEvent(event);
}

void TitleBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 0, 0);
    QStyleOption opt;
    opt.init(this);

    QColor mainColor;
    mainColor = opt.palette.color(QPalette::Base);
    p.fillPath(rectPath,QBrush(mainColor));
    p.end();
}

void TitleBar::updateMaxButtonStatus(bool isMax)
{
    if (isMax) {
        ui->m_maxBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
        ui->m_maxBtn->setToolTip(tr("Restore"));
    } else {
        ui->m_maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        ui->m_maxBtn->setToolTip(tr("Maximize"));
    }
}

void TitleBar::on_m_minBtn_clicked()
{
    g_user_signal->minimumWindowSignal();
}

void TitleBar::on_m_maxBtn_clicked(bool checked)
{
    qDebug() << "checked: " << checked;

    g_user_signal->maximumWindowSignal();
}

void TitleBar::on_m_closeBtn_clicked()
{
    qDebug() << "window close.";
    if (g_user_signal->exitWindowWithSaveFlag) {
        int saveFlag = warnCloseWindow();
        if (saveFlag == 1) {
            qDebug() << "close window save as";
            g_user_signal->closeWindowSaveAs(true);
        }
    }else{
        g_user_signal->exitApplication();
    }

}

void TitleBar::showHelpDialog()
{
    kdk::UserManual userManualTest;
    if (!userManualTest.callUserManual("kylin-scanner")) {
        qCritical() << "user manual call fail!";
    }
    return;
}

int TitleBar::warnCloseWindow()
{

    QMessageBox *msgBox = new QMessageBox();
    msgBox->setModal(Qt::WindowModal);
    msgBox->setAttribute(Qt::WA_ShowModal, true);
    connect(msgBox, &QMessageBox::finished, msgBox, &QMessageBox::deleteLater);
    msgBox->setText(tr("The current file is not saved. Do you want to save it?"));
    msgBox->setIcon(QMessageBox::Warning);
    msgBox->setWindowTitle(tr("Scanner"));
    msgBox->setContextMenuPolicy(Qt::NoContextMenu);
    QAbstractButton *noButton = (QAbstractButton *)msgBox->addButton(tr("Straight &Exit"), QMessageBox::NoRole);
    QAbstractButton *yesButton =  (QAbstractButton *)msgBox->addButton(tr("&Save Exit"), QMessageBox::YesRole);

    msgBox->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    QWidget *widget = nullptr;
    QWidgetList widgetList = QApplication::allWidgets();
    for (int i=0; i<widgetList.length(); ++i) {
        if (widgetList.at(i)->objectName() == "MainWindow") {
            widget = widgetList.at(i);
        }
    }
    if (widget) {
        msgBox->setParent(widget);
        QRect rect = widget->geometry();
        int x = rect.x() + rect.width()/2 - msgBox->width()/2;
        int y = rect.y() + rect.height()/2 - msgBox->height()/2;
        msgBox->move(x,y);
    }

    msgBox->exec();

    if (msgBox->clickedButton() == yesButton) {
        msgBox->deleteLater();
        qDebug() << "Accepeted !";
        return 1;
    } else if (msgBox->clickedButton() == noButton) {
        g_user_signal->exitApplication();
        msgBox->deleteLater();
        qDebug() << "Rejected !";
        return 0;
    }
    return 0;

}

void TitleBar::fontSizeChanged()
{
    float systemFontSize = kdk::GsettingMonitor::getSystemFontSize().toFloat();
    QString fontType = m_themeData->get("systemFont").toString();
    QFont font(fontType, systemFontSize);
    ui->m_appnameLabel->setFont(font);
}

