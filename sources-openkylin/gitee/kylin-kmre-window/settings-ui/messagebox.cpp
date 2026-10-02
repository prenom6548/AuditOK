/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "messagebox.h"
#include "global.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

namespace KylinUI {
namespace MessageBox {

#ifdef UKUI_WAYLAND

MessageBox::MessageBox(QString title, QString text, QWidget *parent, MessageType type) :
    Dialog(parent)
{
    msg_type = type;
    initLayout();
    setData(title, text);
}

MessageBox::~MessageBox()
{
    delete lab_titleIcon;
    delete lab_title;
    delete lab_text;
    delete lab_textIcon;
    delete btn_ok;
    delete box_hb;
    delete box_hm;
    delete box_ht;
    delete box_v;
}

void MessageBox::setData(QString title, QString text)
{
    QIcon icMsg;
    QIcon icTitle = QIcon::fromTheme("kmre");
    if (icTitle.isNull()) {
        icTitle = QIcon(":/res/kmre.svg");
    }

    switch (msg_type) {
    case INFORMATION:
        icMsg = QIcon::fromTheme("ukui-dialog-information");
        btn_cancel->hide();
        break;
    case QUESTION:
        icMsg = QIcon::fromTheme("ukui-dialog-help");
        break;
    case WARNING:
        icMsg = QIcon::fromTheme("ukui-dialog-warning");
        btn_cancel->hide();
        break;
    case ABOUT:
        icMsg = QIcon::fromTheme("about");
        btn_cancel->hide();
        break;
    default:
        break;
    }

    lab_title->setText(title);
    lab_titleIcon->setPixmap(icTitle.pixmap(lab_titleIcon->size()));
    lab_text->setText(text);
    lab_textIcon->setPixmap(icMsg.pixmap(lab_textIcon->size()));
}

void MessageBox::initLayout()
{
    setFixedSize(WIDGET_WIDTH, WIDGET_HEIGHT);

    box_ht = new QHBoxLayout;
    box_hm = new QHBoxLayout;
    box_hb = new QHBoxLayout;
    box_v = new QVBoxLayout(this);

    box_v->addLayout(box_ht);
    box_v->addSpacing(10);
    box_v->addLayout(box_hm);
    box_v->addStretch();
    box_v->addLayout(box_hb);

    lab_titleIcon = new QLabel;
    lab_titleIcon->setFixedSize(TITLE_ICON_WIDTH, TITLE_ICON_WIDTH);
    lab_title = new QLabel;

    box_ht->addWidget(lab_titleIcon);
    box_ht->addWidget(lab_title);
    box_ht->addStretch();

    lab_text = new QLabel;
    lab_text->setWordWrap(true);
    lab_text->adjustSize();
    lab_text->setMinimumWidth(WIDGET_WIDTH - TEXT_ICON_WIDTH - 2*24);
    lab_text->setMinimumHeight(50);
    lab_text->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab_textIcon = new QLabel;
    lab_textIcon->setFixedSize(TEXT_ICON_WIDTH, TEXT_ICON_WIDTH);
    box_hm->addSpacing(24);
    box_hm->addWidget(lab_textIcon, 0, Qt::AlignVCenter);
    box_hm->addWidget(lab_text, 0, Qt::AlignVCenter);
    box_hm->addStretch();

    btn_cancel = new QPushButton;
    btn_cancel->setFocusPolicy(Qt::NoFocus);
    btn_cancel->setFixedSize(80, 30);
    btn_cancel->setText(tr("Cancel"));

    btn_ok = new QPushButton;
    btn_ok->setFocusPolicy(Qt::NoFocus);
    btn_ok->setFixedSize(80, 30);
    btn_ok->setText(tr("Ok"));

    connect(btn_cancel, &QPushButton::clicked, this, &MessageBox::reject);
    connect(btn_ok, &QPushButton::clicked, this, &MessageBox::accept);
    box_hb->addStretch();
    box_hb->addWidget(btn_cancel);
    box_hb->addWidget(btn_ok);
    box_hb->setContentsMargins(0, 0, 24, 8);
}
#endif
void information(QWidget *parent, QString title, QString text, bool block)
{
#ifdef UKUI_WAYLAND
    MessageBox box(title, text, parent, KylinUI::MessageBox::INFORMATION);
    box.exec();
#else
    if ((!block) && (Global::isKylinOSPro() || Global::isOpenKylin())) {
        QMessageBox::information(parent, title, text, QMessageBox::Ok);
    }
    else {
        QMessageBox* Msginfo = new QMessageBox(QMessageBox::Warning, title, text, QMessageBox::Ok, parent);
        Msginfo->button(QMessageBox::Ok)->setText(QObject::tr("Ok"));
        Msginfo->exec();
    }
#endif
}

void critical(QWidget *parent, QString title, QString text, bool block)
{
#ifdef UKUI_WAYLAND
    MessageBox box(title, text, parent, KylinUI::MessageBox::CRITICAL);
    box.exec();
#else
    if ((!block) && (Global::isKylinOSPro() || Global::isOpenKylin())) {
        QMessageBox::critical(parent, title, text, QMessageBox::Ok);
    }
    else {
        QMessageBox* Msginfo = new QMessageBox(QMessageBox::Critical, title, text, QMessageBox::Ok, parent);
        Msginfo->button(QMessageBox::Ok)->setText(QObject::tr("Ok"));
        Msginfo->exec();
    }
#endif
}

void warning(QWidget *parent, QString title, QString text, bool block)
{
#ifdef UKUI_WAYLAND
    MessageBox box(title, text, parent, KylinUI::MessageBox::WARNING);
    box.exec();
#else
    if ((!block) && (Global::isKylinOSPro() || Global::isOpenKylin())) {
        QMessageBox::warning(parent, title, text, QMessageBox::Ok);
    }
    else {
        QMessageBox* Msginfo = new QMessageBox(QMessageBox::Warning, title, text, QMessageBox::Ok, parent);
        Msginfo->button(QMessageBox::Ok)->setText(QObject::tr("Ok"));
        Msginfo->exec();
    }
#endif
}

bool question(QWidget *parent, QString title, QString text)
{
#ifdef UKUI_WAYLAND
    MessageBox box(title, text, parent, KylinUI::MessageBox::QUESTION);
    return (box.exec() == QDialog::Accepted);
#else
    if (Global::isKylinOSPro() || Global::isOpenKylin()) {
        QMessageBox::StandardButton result = QMessageBox::question(parent, title, text);
        return (result == QMessageBox::Yes);
    }
    else {
        QMessageBox* Msginfo = new QMessageBox(QMessageBox::Question, title, text, QMessageBox::Yes|QMessageBox::No, parent);
        Msginfo->button(QMessageBox::Yes)->setText(QObject::tr("Yes"));
        Msginfo->button(QMessageBox::No)->setText(QObject::tr("No"));
        return (Msginfo->exec() == QMessageBox::Yes);
    }
#endif
}

bool restartEnv(QWidget *parent)
{
#ifdef UKUI_WAYLAND
    MessageBox box(QObject::tr("Tips"), QObject::tr("Modify successfully!The modification takes effect only after the service is restarted. Do you want to restart the service?"), parent, KylinUI::MessageBox::QUESTION);
    return (box.exec() == QDialog::Accepted);
#else
    QMessageBox* Msginfo = new QMessageBox(QMessageBox::Warning, QObject::tr("Tips"), QObject::tr("Modify successfully!The modification takes effect only after the service is restarted. Do you want to restart the service?"), NULL, parent);
    QPushButton *okBtn = Msginfo->addButton(QObject::tr("restart now"), QMessageBox::AcceptRole);
    QPushButton *cancelBtn = Msginfo->addButton(QObject::tr("restart later"), QMessageBox::RejectRole);
    Msginfo->exec();
    return(Msginfo->clickedButton() == okBtn);
#endif
}
}
}
