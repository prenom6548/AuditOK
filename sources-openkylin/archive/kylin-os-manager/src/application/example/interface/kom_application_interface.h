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

#ifndef KYLIN_OS_MANAGER_INTERFACE_KOM_APPLICATION_INTERFACE_H
#define KYLIN_OS_MANAGER_INTERFACE_KOM_APPLICATION_INTERFACE_H

#include <QWidget>
#include "kyplugin.h"

class KomApplicationInterface {
public:
	virtual ~KomApplicationInterface() {}
	virtual std::string name() = 0;
    virtual std::string i18nName() = 0;
    virtual QWidget *createWidget() = 0;
    virtual void start() {}
    virtual void stop() {}
    virtual int sort() {return -1;}
    virtual std::string icon() {return "";}
	virtual std::string description() {return "";}
};

class KomApplicationProvider: public kyplugin::Provider {
public:
	virtual KomApplicationInterface *create() const = 0;
	unsigned int getVersion() const {return providerVersion;}

private:
	friend class kyplugin::KyPlugin;
	std::string kypluginGetType() const {return providerType;}

	static const unsigned int providerVersion;
	static const unsigned int providerLowestVersion;
	static const std::string providerType;
};

#endif
