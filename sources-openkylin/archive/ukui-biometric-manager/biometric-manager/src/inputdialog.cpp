/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "inputdialog.h"
#include "ui_inputdialog.h"
#include <QFile>
#include <ukuiwindowhelper.h>

InputDialog::InputDialog(QStringList names, QWidget *parent) : QDialog(parent), ui(new Ui::InputDialog)
{
    ui->setupUi(this);
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);
    QFile qssFile(":/css/assets/promptdialog.qss");
    qssFile.open(QFile::ReadOnly);
    this->setStyleSheet(qssFile.readAll());
    qssFile.close();

    ui->btnClose->setProperty("isWindowButton", 0x2);
    ui->btnClose->setProperty("useIconHighlightEffect", 0x8);
    ui->btnClose->setFixedSize(30, 30);
    ui->btnClose->setIconSize(QSize(16, 16));
    ui->btnClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    ui->btnClose->setFlat(true);
    ui->btnClose->setToolTip(tr("Close"));
    ui->btnOK->setEnabled(false);
    ui->btnOK->setProperty("isImportant", true);
    ui->btnCancel->setProperty("useButtonPalette", true);
    ui->lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    ui->lineEdit->setFocus();
    connect(ui->lineEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        if (text.length() > 32) {
            ui->lblError->setText(tr("feature name is too long"));
        } else if (names.contains(text)) {
            ui->lblError->setText(tr("Duplicate feature name"));
        } else if (text.contains("\n")) { // 按照设计师要求，当复制过来的文本中含有换行符时，过滤掉换行符
            QString removeConst = const_cast<QString &>(text);
            QString removeText = removeConst.remove("\n");
            if (names.contains(removeText)) {
                ui->lblError->setText(tr("Duplicate feature name"));
            } else {
                ui->lblError->setText(tr(""));
            }
        } /*else if(text.startsWith(" ") || text.endsWith("
        ")){//设计师要求与重命名用户名保持一致，去除首尾不能包含空格限制 ui->lblError->setText(tr("Please do not use
        spaces as the beginning or end of the feature name"));
        }*/
        else if (text.isEmpty()) {
            ui->lblError->setText(tr("Empty feature name"));
        } else if (text.contains(" ")) {
            QString textConst = const_cast<QString &>(text);
            if (textConst.trimmed().isEmpty()) {
                ui->lblError->setText(tr("Empty feature name"));
            } else {
                ui->lblError->setText(tr(""));
            }
        } else {
            ui->lblError->setText(tr(""));
        }
        if (!text.isEmpty() && ui->lblError->text().isEmpty()) {
            ui->btnOK->setEnabled(true);
        } else {
            ui->btnOK->setEnabled(false);
        }
    });

    // 去除窗管标题栏
    UkuiWindowHelper *pUkuiWindowHelper = new UkuiWindowHelper(this);
    pUkuiWindowHelper->removeTitleBar();
}

InputDialog::~InputDialog()
{
    delete ui;
}

void InputDialog::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Escape:
            break;
        case Qt::Key_Enter:
            if (ui->btnOK->isEnabled())
                ui->btnOK->clicked();
            break;
        case Qt::Key_Return:
            if (ui->btnOK->isEnabled())
                ui->btnOK->clicked();
            break;
        default:
            QDialog::keyPressEvent(event);
    }
}

void InputDialog::setTitle(const QString &text)
{
    setWindowTitle(text);
}

void InputDialog::setPrompt(const QString &text)
{
    ui->lblPrompt->setText(text);
}

void InputDialog::setError(const QString &text)
{
    ui->lblError->setText(text);
}

void InputDialog::setText(const QString &text)
{
    ui->lineEdit->setText(text);
}

QString InputDialog::getText()
{
    return ui->lineEdit->text();
}

void InputDialog::on_btnClose_clicked()
{
    reject();
}

void InputDialog::on_btnOK_clicked()
{
    Q_EMIT dataChanged(ui->lineEdit->text());
}

void InputDialog::on_btnCancel_clicked()
{
    reject();
}
