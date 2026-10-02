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
#include "changefeaturename.h"
#include "ui_changefeaturename.h"
#include <ukuiwindowhelper.h>
#include <QDebug>
#include <kysdk/applications/accessinfohelper.h>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

ChangeFeatureName::ChangeFeatureName(QStringList featurenames, QWidget *parent)
    : KDialog(parent), ui(new Ui::ChangeFeatureName), names(featurenames)
{
    // 去除窗管标题栏
    UkuiWindowHelper *pUkuiWindowHelper = new UkuiWindowHelper(this);
    pUkuiWindowHelper->removeTitleBar();
    setWindowIcon("ukui-control-center");
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    initInfoFormat();

    ui->saveBtn->setEnabled(false);
    ui->lineEdit->setFocus();
    ui->cancelBtn->setProperty("useButtonPalette", true);
    ui->saveBtn->setProperty("isImportant", true);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, [=](QString txt) {
        if (names.contains(txt)) {
            ui->tipLabel->setText(tr("Duplicate feature name"));
        } else if (txt.contains("\n")) { // 按照设计师要求，当复制过来的文本中含有换行符时，过滤掉换行符
            QString removeText = txt.remove("\n");
            if (names.contains(removeText)) {
                ui->tipLabel->setText(tr("Duplicate feature name"));
            } else {
                ui->tipLabel->setText(tr(""));
            }
        } else if (txt.isEmpty()) {
            ui->tipLabel->setText(tr("Empty feature name"));
        } else if (txt.length() > 32) {
            ui->tipLabel->setText(tr("feature name is too long"));
        } else if (txt.contains(" ")) {
            QString textConst = const_cast<QString &>(txt);
            if (textConst.trimmed().isEmpty()) {
                ui->tipLabel->setText(tr("Empty feature name"));
            } else {
                ui->tipLabel->setText(tr(""));
            }
        } else {
            ui->tipLabel->setText(tr(""));
        }
        if (!txt.isEmpty() && ui->tipLabel->text().isEmpty()) {
            ui->saveBtn->setEnabled(true);
        } else {
            ui->saveBtn->setEnabled(false);
        }
    });
    connect(ui->cancelBtn, &QPushButton::clicked, [=] { close(); });
    connect(ui->saveBtn, &QPushButton::clicked, [=] {
        emit sendNewName(ui->lineEdit->text());
        close();
    });
    this->mainWidget()->setLayout(ui->frame->layout());
}

ChangeFeatureName::~ChangeFeatureName()
{
    delete ui;
}

void ChangeFeatureName::initInfoFormat()
{
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->lineEdit, "Login Options", "ChangeFeatureName", "new name lineEdit of ChangeFeatureName dailog");
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->renameLabel, "Login Options", "ChangeFeatureName", "rename label of ChangeFeatureName dailog");
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->tipLabel, "Login Options", "ChangeFeatureName", "tip label of ChangeFeatureName dailog");
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->cancelBtn, "Login Options", "ChangeFeatureName", "cancel button of ChangeFeatureName dailog");
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->saveBtn, "Login Options", "ChangeFeatureName", "confirm button of ChangeFeatureName dailog");
}

void ChangeFeatureName::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Escape:
            break;
        case Qt::Key_Enter:
            if (ui->saveBtn->isEnabled())
                ui->saveBtn->clicked();
            break;
        case Qt::Key_Return:
            if (ui->saveBtn->isEnabled())
                ui->saveBtn->clicked();
            break;
        default:
            QDialog::keyPressEvent(event);
    }
}

void ChangeFeatureName::setTitle(QString titleName)
{
    setWindowTitle(titleName + tr(" Rename"));
    ui->renameLabel->setText(titleName + tr(" name"));
    QFont font;
    QString str = ElideText(font, 95, ui->renameLabel->text());
    if (str != ui->renameLabel->text()) {
        ui->renameLabel->setToolTip(ui->renameLabel->text());
        ui->renameLabel->setText(str);
    }

    setWindowTitle(titleName + tr(" Rename"));
}

QString ChangeFeatureName::ElideText(QFont font, int width, QString strInfo)
{
    QFontMetrics fontMetrics(font);
    // 如果当前字体下，字符串长度大于指定宽度
    if (fontMetrics.width(strInfo) > width) {
        strInfo = QFontMetrics(font).elidedText(strInfo, Qt::ElideRight, width);
    }
    return strInfo;
}
