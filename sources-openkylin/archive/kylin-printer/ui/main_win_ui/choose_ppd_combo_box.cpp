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
#include <QFileDialog>
#include "custom_message_box.h"

#include "find_and_match_ppds.h"
#include "ukui_apt.h"

#include "select_ppd_dialog.h"
#include "base_notify_dialog.h"
#include "choose_ppd_combo_box.h"

ChoosePpdComboBox::ChoosePpdComboBox(QWidget *parent)
    : CustomComboBox(parent)
{
    setParent(parent);
    setDefaultPreItem(tr("Choose PPD"));
    setDefaultAfterItem(tr("Choose from the PPD library"));
    setDefaultAfterItem(tr("Add local PPD"));
    connect(this, &QComboBox::currentTextChanged, [=](const QString &text) {
        if (text.isEmpty()) {
            return;
        }
        if (text == tr("Choose PPD")) {
            emit currentPpdChanged(true, text);
            checkDefaultItem();
        } else if (text == tr("Choose from the PPD library")) {
            emit currentPpdChanged(true, text);
            checkDefaultItem();
            showSelectPpdDialog();
        } else if (text == tr("Add local PPD")) {
            emit currentPpdChanged(true, text);
            checkDefaultItem();
            onChooseClicked();
        } else{
            emit currentPpdChanged(false, text);
        }
    });
}

ChoosePpdComboBox::~ChoosePpdComboBox()
{
}

void ChoosePpdComboBox::onChooseClicked()
{
qDebug() << __FUNCTION__;
    // Popup a new window to choose a valid driver
    QString debName = "";
    QFileDialog dialog(parentWidget() ,tr("Please select a deb package."));
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilters(QStringList() << tr("Deb File(*.deb)") << tr("Kaiming File(*.ok)"));
    dialog.setLabelText(QFileDialog::Accept, tr("Choose"));
    dialog.setLabelText(QFileDialog::Reject, tr("Cancel"));

#ifdef UNIT_TEST
    auto autoCloseTimer = new QTimer(this);
    connect(autoCloseTimer, &QTimer::timeout, &dialog, &QFileDialog::close);
    autoCloseTimer->start(500);
#endif

    if(QDialog::Accepted == dialog.exec()){
      //  QStringList fileName = dialog.selectedFiles();
        QStringList names = dialog.selectedFiles();
        qDebug() << names;
        if(names.size()!=0) {
            debName = names.at(0);
        }
    }
    QPoint windowCenter = QPoint(parentWidget()->mapToGlobal(QPoint(0,0)).x() + parentWidget()->width()/2,parentWidget()->mapToGlobal(QPoint(0,0)).y() + parentWidget()->height()/2);
    //QString debName = QFileDialog::getOpenFileName(this, tr("Please select a deb package."), QDir::currentPath(), "Deb(*.deb)");
    if (debName.compare("") != 0) {
        //安装这个包
        m_installOk = -1;
        BaseNotifyDialog dialog;
        dialog.hide();
        dialog.dialogSetType(BaseNotifyDialogType::WAITTING);
        dialog.labelSetText(tr("Searching driver..."));
        dialog.show();
        dialog.move(windowCenter - dialog.rect().center());
        connect(g_findAndMatchWorkThread, &FindAndMatchPPDs::updateFinished, &dialog, &QDialog::close);
        connect(g_findAndMatchWorkThread, &FindAndMatchPPDs::updateFailed, &dialog, &QDialog::close);
        connect(this, SIGNAL(installEnd()), &dialog, SLOT(close()));

        DebUtilHelper* debHelper = new DebUtilHelper(debName);
        connect(debHelper, &DebUtilHelper::succeed, [=]() {
            qDebug() << "Install succeed";
            m_installOk = 0;
            g_findAndMatchWorkThread->updatePPDsMap();
        });
        connect(debHelper, &DebUtilHelper::failed, [=]() {
            qDebug() << "Install failed";
            m_installOk = 1;
            emit installEnd();
        });
        debHelper->startWorker();
        dialog.exec();
        if (m_installOk == 1) {
            //弹窗
            CustomMessageBox Msg(CustomMessageBox::Warning, tr("Warning"), tr("Install package failed，please retry."), CustomMessageBox::NoButton, this);
            QPushButton *btnYes = Msg.addButton(tr("Yes"), CustomMessageBox::YesRole);
            Msg.exec();
            return;
        }
        showSelectPpdDialog();
    }

}

void ChoosePpdComboBox::showSelectPpdDialog()
{
    m_retPpdName = "";
    m_pSelectPPDDialog.reset(new SelectPpdDialog(parentWidget()));
    connect(m_pSelectPPDDialog.get(),SIGNAL(chosenPPDName(QString)),this,SLOT(updatePPD(QString)));
    m_pSelectPPDDialog.get()->onShow();

}

void ChoosePpdComboBox::updatePPD(QString text)
{
    m_retPpdName = text;
    if (m_retPpdName.compare("") != 0) {
        //更新Combobox
        clear();
        addItem(m_retPpdName);
    }
}

void ChoosePpdComboBox::showPopup()
{
    removeDefaultItem();
    CustomComboBox::showPopup();
}

void ChoosePpdComboBox::hidePopup()
{
    setDefaultPreItem(tr("Choose PPD"));
    CustomComboBox::hidePopup();
}
