/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tipDialog.h"

#include "logger.h"
#include "screenLocator.h"

#include <QAbstractButton>
#include <QCloseEvent>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QScreen>

namespace {

constexpr int kMessageLabelWidth = 380;

void configureMessageLabel(QMessageBox *box)
{
    // QMessageBox 内部文案默认不限宽，长句不换行会导致「1./2.」悬挂缩进失效
    if (QLabel *label = box->findChild<QLabel *>(QStringLiteral("qt_msgbox_label"))) {
        label->setWordWrap(true);
        label->setFixedWidth(kMessageLabelWidth);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    }
}

} // namespace

TipDialog::TipDialog(QWidget *parent)
    : QMessageBox(parent)
{
    // 关闭后由 closeEvent 销毁；不依赖 QMessageBox 默认路径（可能仅 hide）
    setAttribute(Qt::WA_DeleteOnClose, false);
    // 避免关闭弹窗时因「最后一个可退出窗口关闭」导致整个进程退出
    setAttribute(Qt::WA_QuitOnClose, false);
    setIcon(QMessageBox::Warning);
    setStandardButtons(QMessageBox::NoButton);

    m_cancelButton = addButton(tr("Cancel"), QMessageBox::RejectRole);
    m_actionButton = addButton(tr("Settings"), QMessageBox::AcceptRole);
    if (QPushButton *actionBtn = qobject_cast<QPushButton *>(m_actionButton)) {
        setDefaultButton(actionBtn);
    }

    connect(this, &QMessageBox::buttonClicked, this, &TipDialog::onButtonClicked);
}

void TipDialog::updatePosition()
{
    // 跟随活动窗口所在屏幕，与 VoiceInputWindow 一致
    QScreen *screen = ScreenLocator::instance().screenForActiveWindow();
    if (screen == nullptr) {
        VW_LOG(Logger::Level::Warning) << "TipDialog: 无法获取活动窗口所在屏幕，取消窗口位置重置";
        return;
    }

    const QRect screenGeom = screen->geometry();

    const QRect rect(screenGeom.x() + (screenGeom.width() - width()) / 2,
                     screenGeom.y() + (screenGeom.height() - height()) / 2,
                     width(),
                     height());
    setGeometry(rect);

    VW_LOG(Logger::Level::Debug) << "TipDialog: show on screen" << screen->name()
                                 << "geometry" << rect;
}

void TipDialog::closeEvent(QCloseEvent *event)
{
    // 不调用 QMessageBox::closeEvent，避免只 hide 不销毁
    event->accept();
    deleteLater();
}

void TipDialog::showCentered(const QString &message,
                             const QString &actionButtonText,
                             const QString &controlCenterModule)
{
    setTextFormat(Qt::RichText);
    setText(message);
    if (m_actionButton) {
        m_actionButton->setText(actionButtonText);
    }
    m_controlCenterModule = controlCenterModule;

    configureMessageLabel(this);
    adjustSize();
    // 因QMessageBox的特性，显示位置就在屏幕中心，无法做到跟随活动窗口所在屏幕且设置无效，所以暂不再设置位置
    // updatePosition();
    show();
    raise();
    activateWindow();
}

void TipDialog::showTip(const QString &message,
                        const QString &actionButtonText,
                        const QString &controlCenterModule)
{
    showCentered(message, actionButtonText, controlCenterModule);
}

void TipDialog::showMuted()
{
    showCentered(tr("Microphone is muted. Adjust the input volume to use voice input."),
                 tr("Settings"),
                 QStringLiteral("Audio"));
}

void TipDialog::showUnavailable()
{
    showCentered(tr("No available microphone. Voice input is limited. You can check device status in sound device management."),
                 tr("Settings"),
                 QStringLiteral("Audio"));
}

void TipDialog::showNoVoiceModel()
{
    // 悬挂缩进：换行后正文与首行文字左对齐
    const QString message = QStringLiteral(
        "<p style=\"margin-top:0; margin-bottom:8px;\"><b>%1</b></p>"
        "<p style=\"margin-top:0; margin-bottom:6px;\">%2</p>"
        "<p style=\"margin-top:0; margin-bottom:6px; margin-left:20px; text-indent:-20px;\">1.&nbsp;%3</p>"
        "<p style=\"margin-top:0; margin-bottom:6px; margin-left:20px; text-indent:-20px;\">2.&nbsp;%4</p>"
        "<p style=\"margin-top:8px; margin-bottom:0; color:#8C8C8C;\">%5</p>")
        .arg(tr("No voice model is currently available."),
             tr("How to configure:"),
             //: Keep the <b> HTML tags around the settings path and model type names.
             tr("Go to <b>System Settings &gt; AI Model Management &gt; Model Configuration</b> "
                "and add a <b>\"Cloud Model\"</b>, <b>\"Local Model\"</b>, or <b>\"Custom Model\"</b>."),
             tr("If models are already configured on this device, make sure they are enabled."),
             tr("Note: Running a local model may use significant device resources. Please ensure "
                "the device meets performance requirements."));

    showCentered(message, tr("Configure Now"), QStringLiteral("aisubsystem"));
}

void TipDialog::openControlCenter(const QString &module)
{
    if (module.isEmpty()) {
        return;
    }

    const bool started = QProcess::startDetached(
        QStringLiteral("/usr/bin/ukui-control-center"),
        {QStringLiteral("-m"), module});
    if (!started) {
        VW_LOG(Logger::Level::Error) << "Failed to launch ukui-control-center with module:"
                                     << module;
    }
}

void TipDialog::onButtonClicked(QAbstractButton *button)
{
    if (button == m_actionButton) {
        openControlCenter(m_controlCenterModule);
    }
    close();
}
