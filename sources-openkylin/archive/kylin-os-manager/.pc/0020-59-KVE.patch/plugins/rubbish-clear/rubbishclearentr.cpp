/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "rubbishclearentr.h"
#include "clearmainwidget.h"
#include "cleanerwidget.h"
#include <QTranslator>
#include <QApplication>
#include "frame.h"

namespace KylinRubbishClear
{

RubbishClearEntr::~RubbishClearEntr() {}

void RubbishClearEntr::init(void (*frameCallback)(const char *funcName, ...))
{
    Frame::setFrameCallback(frameCallback);
}

std::string RubbishClearEntr::name()
{
    return "RubbishClear";
}

std::string RubbishClearEntr::i18nName()
{
    return QObject::tr("RubbishClear").toStdString();
}

std::string RubbishClearEntr::icon()
{
	return "ukui-rubbish-symbolic";
}

int RubbishClearEntr::sort()
{
    return 2;
}

QWidget *RubbishClearEntr::createWidget()
{
    QString tranPath("/usr/share/kylin-os-manager/translations");
    QTranslator *translator = new QTranslator;
    if (translator->load(QLocale(), "kylin-os-manager-rubbish-clear", "_", tranPath)) {
        QApplication::installTranslator(translator);
    } else {
        qWarning() << "ProblemFeedback load translation file fail !";
    }

    CleanerWidget *widget = new CleanerWidget();
    return widget;
}

KomApplicationInterface *PluginProvider::create() const
{
    return new RubbishClearEntr();
}

} // namespace KylinRubbishClear
