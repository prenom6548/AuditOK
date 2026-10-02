/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "inputvalidator.h"

InputValidator::InputValidator(const QRegExp &rx, QObject *parent) :
    QRegExpValidator(rx, parent)
{}

InputValidator::~InputValidator()
{}

QValidator::State InputValidator::validate(QString &input, int &pos) const
{
    QString in = input;
    int index = pos;

    State state = QRegExpValidator::validate(in, index);

    QChar c;
    if (!in.isEmpty())
        c = in.at(index - 1);
    if (State::Acceptable == state)
        emit checked(true, in, index, c);
    else
        emit checked(false, in, index, c);

    return state;
}
