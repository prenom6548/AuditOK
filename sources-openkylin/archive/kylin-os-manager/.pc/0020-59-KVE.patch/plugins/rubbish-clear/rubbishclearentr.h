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

#ifndef RUBBISHCLEARENTR_H
#define RUBBISHCLEARENTR_H

#include <kom_application_interface.h>

namespace KylinRubbishClear
{

class RubbishClearEntr : public KomApplicationInterface
{
public:
    virtual ~RubbishClearEntr();
    virtual void init(void (*frameCallback)(const char *funcName, ...)) override;
    virtual std::string name() override;
    virtual std::string i18nName() override;
    virtual std::string icon() override;
    virtual int sort() override;
    virtual QWidget *createWidget() override;
};

class PluginProvider : public KomApplicationProvider
{
public:
    KomApplicationInterface *create() const;
};

} // namespace KylinRubbishClear



#endif // RUBBISHCLEARENTR_H
