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

#include <QDebug>

#include <libkydiagnostics.h>

#include "common.h"
#include "information_collector.h"

InformationCollector::InformationCollector(/* args */)
{

}

InformationCollector::~InformationCollector(/* args */)
{

}
void InformationCollector::addPoint(P point) const
{
    QString str;
    switch (point)
    {
    case InformationCollector::P::OrderList      : str = "OrderList";      break;
    case InformationCollector::P::UnorderList    : str = "UnorderList";    break;
    case InformationCollector::P::Bold           : str = "Bold";           break;
    case InformationCollector::P::Italics        : str = "Italics";        break;
    case InformationCollector::P::Underline      : str = "Underline";      break;
    case InformationCollector::P::Deleteline     : str = "Deleteline";     break;
    case InformationCollector::P::FontSize       : str = "FontSize";       break;
    case InformationCollector::P::FontColor      : str = "FontColor";      break;
    case InformationCollector::P::InsertPicture  : str = "InsertPicture";  break;
    case InformationCollector::P::InterfaceColor : str = "InterfaceColor"; break;
    case InformationCollector::P::DeleteCurrent  : str = "DeleteCurrent";  break;
    case InformationCollector::P::UiTop          : str = "UiTop";          break;
    case InformationCollector::P::ListMode       : str = "ListMode";       break;
    case InformationCollector::P::IconMode       : str = "IconMode";       break;
    case InformationCollector::P::NewNote        : str = "NewNote";        break;
    case InformationCollector::P::Search         : str = "Search";         break;
    case InformationCollector::P::Delete         : str = "Delete";         break;
    case InformationCollector::P::ModeChange     : str = "ModeChange";     break;
    default:
        return;
    }
    sendMessage("point", str);
}

void InformationCollector::addMessage(const QString &str) const
{
    if (str.isEmpty()) {
        return ;
    }
    sendMessage("message", str);
}

void InformationCollector::sendMessage(const QString &key, const QString &value) const
{
#ifndef TEST_DEBUG
    KBuriedPoint p[1];
    p[0].key = QTC(key);
    p[0].value = QTC(value);
    if (kdk_buried_point(const_cast<char *>("ukui-notebook"), const_cast<char *>("FunctionType"), p, 1)) {
        qWarning() << "InformationCollector::sendMessage : kdk_buried_point failed!";
    }
#endif
}
