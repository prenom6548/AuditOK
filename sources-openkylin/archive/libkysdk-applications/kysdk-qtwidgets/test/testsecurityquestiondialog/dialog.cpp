/*
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "dialog.h"
#include <QDebug>

#include "ksecurityquestiondialog.h"
using namespace kdk;

Dialog::Dialog(QWidget *parent)
    : QWidget(parent)
{
    KSecurityQuestionDialog *questionDialog = new KSecurityQuestionDialog();
    questionDialog->setWindowIcon("ukui-control-center");
    questionDialog->setWindowTitle("Security Question");
    questionDialog->setTitleText(tr("Answer the security questions and reset your password"));
    questionDialog->addSecurityQuestionItem(3, true);
    QStringList AA;
    AA << "111"
       << "222"
       << "333"
       << "444";
    questionDialog->initQustionCombox(AA);

    if (questionDialog->answerLabel(1) != nullptr) {
        questionDialog->answerLabel(1)->setText("answerLabel1");
        qDebug() << questionDialog->answerLabel(1)->text();
    }

    if (questionDialog->questionLabel(1) != nullptr) {
        questionDialog->questionLabel(1)->setText("test111");
        qDebug() << questionDialog->questionLabel(1)->text();
    }

    if (questionDialog->answerLineedit(1) != nullptr) {
        qDebug() << questionDialog->answerLineedit(1)->text();
    }

    if (questionDialog->questionCombox(1) != nullptr) {
        qDebug() << questionDialog->questionCombox(1)->currentText();
    }

    if (questionDialog->tipsLabel(1) != nullptr) {
        questionDialog->tipsLabel(1)->setText("Incorrect answer");
        qDebug() << questionDialog->tipsLabel(1)->text();
    }

    connect(questionDialog->cancelButton(), &KPushButton::clicked, this, [=]() {
        questionDialog->close();
    });

    questionDialog->show();
}
