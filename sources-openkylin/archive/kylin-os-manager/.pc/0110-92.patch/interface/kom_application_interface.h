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
#include <QVariant>
#include "kyplugin.h"

enum class ClassType {
    CallbackClickTab = 0,
    CallbackClickPluginProblemFeedback,
    CallbackStartNetCheck,
    // 故障诊断
    kRepairSpecificError,
};



typedef void (*CallbackFun)(ClassType classType, QVariant var);
Q_DECLARE_METATYPE(CallbackFun)

class KomApplicationInterface {
public:
    enum class CommunicationType {
        // 故障诊断
        kRepairSpecificError,
        kDiagnosisSpecificType,

        // 设备管理
        kDisplayHardwareInfo,
        kDisplayDriverManager,
    };

    virtual ~KomApplicationInterface() {}
    virtual std::string name() = 0;
    virtual std::string i18nName() = 0;
    virtual QWidget *createWidget() = 0;
    virtual void init([[maybe_unused]]void (*share)(const char *funcName, ...)) {}
    virtual void communication(enum CommunicationType communication_type, QVariant data) {}
    virtual int sort() {return -1;}
    virtual std::string icon() {return "";}
    virtual std::string description() {return "";}
    CallbackFun getCallbackPtr() {return m_callback;}

protected:
    void registCallback(CallbackFun callback) {
        m_callback = callback;
    }

private:
    CallbackFun m_callback = nullptr;
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
