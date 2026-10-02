/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "servicedataworker.h"

#include "servicecommon.h"
#include "kerror.h"
#include "serviceinfo.h"
#include "servicefileinfo.h"
#include "systemd1serviceinterface.h"
#include "systemd1managerinterface.h"
#include "servicemanager.h"

#include <QDebug>
#include <QtConcurrent>

ServiceDataWorker::ServiceDataWorker(QObject *parent)
    : QObject(parent)
{

}

// 刷新服务列表
void ServiceDataWorker::refreshServiceList()
{
    QList<ServiceInfo> list;
    QHash<QString, int> hash;
    KError ke;
    qDebug()<<"POSITION:"<<__FILE__<<","<<__LINE__;
    Systemd1ManagerInterface mgrIf(DBUS_SYSTEMD1_SERVICE_NAME,
                                   DBUS_SYSTEMD1_SERVICE_PATH,
                                   QDBusConnection::systemBus());

    auto serviceFilesResult = mgrIf.ListServiceFiles();
    ke = serviceFilesResult.first;
    if (ke) {
        qDebug() << "ListServiceFiles failed:" << ke.getErrorName() << ke.getErrorMessage();
    }
    ServiceFileInfoList serviceFileList = serviceFilesResult.second;
    qDebug()<<"POSITION:"<<__FILE__<<","<<__LINE__;
    auto servicesResult = mgrIf.ListServices();
    ke = servicesResult.first;
    if (ke) {
        qDebug() << "ListServices failed:" << ke.getErrorName() << ke.getErrorMessage();
    }

    ServiceInfoList serviceList = servicesResult.second;
    for (auto s : serviceList) {
        if (s.getName().endsWith(UNIT_TYPE_SERVICE_SUFFIX)) {
            hash[s.getName()] = 0;
        }
    }

    // 根据服务名称后缀过滤非service类型的服务
    std::function<bool(const ServiceInfo &)> filterServices = [](const ServiceInfo & service) -> bool {
        return service.getName().endsWith(UNIT_TYPE_SERVICE_SUFFIX);
    };

    // 遍历serviceinfo列表并更新元素内容的过程
    std::function<ServiceInfo(const ServiceInfo &) > mapService = [&mgrIf](const ServiceInfo & service) {
    try {
        KError ke1;
        ServiceInfo descService;

        descService = service;
        descService.setSName(service.getName().left(service.getName().lastIndexOf('.')));
        qDebug()<<"Service Name:"<<descService.getSName();
        // 验证service path有效性
        if (service.getServiceObjectPath().isEmpty()) {
            qWarning() << "Empty service object path for service:" << service.getName();
            return descService; // 返回当前构建的服务信息
        }

        Systemd1ServiceInterface servIf(DBUS_SYSTEMD1_SERVICE_NAME,
                                     service.getServiceObjectPath(),
                                     QDBusConnection::systemBus());

        // 服务ID
        auto id = servIf.getId();
        ke1 = id.first;
        if (ke1) {
            qDebug() << "call getId failed:" << ke1.getErrorName() << ke1.getErrorMessage();
        } else {
            descService.setID(id.second);
        }
        qDebug()<<"POSITION:"<<__FILE__<<","<<__LINE__;
        auto bStart = servIf.canStart();
        ke1 = bStart.first;
        if (ke1) {
            qDebug() << "call canStart failed:" << ke1.getErrorName() << ke1.getErrorMessage();
        } else {
            descService.setIsCanStart(bStart.second);
        }

        auto bStop = servIf.canStop();
        ke1 = bStop.first;
        if (ke1) {
            qDebug() << "call canStart failed:" << ke1.getErrorName() << ke1.getErrorMessage();
        } else {
            descService.setIsCanStop(bStop.second);
        }

        auto bReload = servIf.canReload();
        ke1 = bReload.first;
        if (ke1) {
            qDebug() << "call canStart failed:" << ke1.getErrorName() << ke1.getErrorMessage();
        } else {
            descService.setIsCanReload(bReload.second);
        }

        // 主进程id
        auto pid = servIf.getMainPID();
        ke1 = pid.first;
        if (ke1) {
            qDebug() << "getMainPID failed" << ke1.getErrorName() << ke1.getErrorMessage();
        } else {
            descService.setMainPID(pid.second);
        }
        qDebug()<<"POSITION:"<<__FILE__<<","<<__LINE__;
        // 服务状态

        QDBusInterface *iface = new QDBusInterface (DBUS_SYSTEMD1_SERVICE_NAME,
                                  DBUS_SYSTEMD1_SERVICE_PATH,
                                  SYSTEMD1_MANAGER_NAME,
                                  QDBusConnection::systemBus());
        KError m_ke;
        QString m_state = "";
        QDBusMessage reply = iface->call(QDBus::Block, "GetUnitFileState", service.getName());
        if (reply.type() == QDBusMessage::ErrorMessage) {
            m_ke.setCode(KError::kErrorDBus);
            m_ke.setSubCode(iface->lastError().type());
            m_ke.setErrorName(reply.errorName());
            m_ke.setErrorMessage(reply.errorMessage());
        } else {
            if (reply.signature() == "s") {
                m_state = qvariant_cast<QString>(reply.arguments()[0]);
            }
        }
        iface->deleteLater();

        ke1 = m_ke;
        if (ke1) {
            qDebug() << "getUnitFileState failed" << ke1.getErrorName() << ke1.getErrorMessage();
        } else {
           descService.setState(m_state);
        }
        qDebug()<<"POSITION:"<<__FILE__<<","<<__LINE__;
        // 启动模式
        descService.setStartupType(ServiceManager::getServiceStartupMode(
                                 descService.getSName(),
                                 descService.getState()));
        qDebug()<<"POSITION:"<<__FILE__<<","<<__LINE__;
        return descService;
        } catch (const std::exception& e) {
            qWarning() << "Exception in mapService for service:" << service.getName() << ":" << e.what();
            return ServiceInfo(); // 返回默认构造的ServiceInfo
        } catch (...) {
            qWarning() << "Unknown exception in mapService for service:" << service.getName();
            return ServiceInfo(); // 返回默认构造的ServiceInfo
        }
    };
    qDebug()<<"POSITION:"<<__FILE__<<","<<__LINE__;
    // 添加更新信息后的serviceinfo到服务信息列表
    std::function<void(ServiceInfoList &list, const ServiceInfo &service)> addServiceInfoToList;
    addServiceInfoToList = [](ServiceInfoList &list, const ServiceInfo & service) {
        list << service;
    };
    qDebug()<<"POSITION:"<<__FILE__<<","<<__LINE__;
    // 异步过滤并填充serviceinfo信息
    auto filterServiceAsync = QtConcurrent::filter(serviceList, filterServices);
    filterServiceAsync.waitForFinished();
    auto serviceInfoResult = QtConcurrent::mappedReduced<ServiceInfoList>(serviceList, mapService, addServiceInfoToList);
    qDebug()<<"POSITION:"<<__FILE__<<","<<__LINE__;
    // 添加更新信息后的servicefileinfo到服务信息列表
    std::function<void(ServiceInfoList &list, const ServiceInfo &service)> addServiceFileInfoToList;
    addServiceFileInfoToList = [&hash](ServiceInfoList &list, const ServiceInfo &service) {
        if (!hash.contains(service.getID())) {
            list << service;
        }
    };

    // 排除非服务类型的servicefileinfo信息
    std::function<bool(const ServiceFileInfo &)> filterServiceFiles;
    filterServiceFiles = [&hash](const ServiceFileInfo &serviceFile) -> bool {
        auto id = serviceFile.getName().mid(serviceFile.getName().lastIndexOf('/') + 1);
        if (hash.contains(id)) {
            return false;
        } else {
            return serviceFile.getName().endsWith(UNIT_TYPE_SERVICE_SUFFIX);
        }
    };
    qDebug()<<"POSITION:"<<__FILE__<<","<<__LINE__;
    // 根据servicefileinfo获取服务信息
    std::function<ServiceInfo(const ServiceFileInfo &)> mapServiceFiles = [](const ServiceFileInfo &serviceFile) {
    try {
        ServiceInfo descService;
        KError ke2;

        auto id = serviceFile.getName().mid(serviceFile.getName().lastIndexOf('/') + 1);
        auto sname = id;
        sname.chop(strlen(UNIT_TYPE_SERVICE_SUFFIX));

        descService.setSName(sname);
        descService.setState(serviceFile.getStatus());
        descService.setStartupType(ServiceManager::getServiceStartupMode(
                                 descService.getSName(),
                                 descService.getState()));
        if (sname.endsWith('@')) {
            auto desc = ServiceDataWorker::readServiceDescriptionFromServiceFile(serviceFile.getName());
            descService.setDescription(desc);
        } else {
            auto o = Systemd1ServiceInterface::normalizeUnitPath(id);
            // 验证对象路径有效性
            if (o.path().isEmpty()) {
                qWarning() << "Empty object path for service:" << id;
                return descService; // 返回当前构建的服务信息
            }

            Systemd1ServiceInterface svcIf(DBUS_SYSTEMD1_SERVICE_NAME,
                                           o.path(),
                                           QDBusConnection::systemBus());

            // Id
            auto id1 = svcIf.getId();
            ke2 = id1.first;
            if (ke2) {
                qDebug() << "call getId failed:" << ke2.getErrorName() << ke2.getErrorMessage();
            } else {
                descService.setID(id1.second);
            }

            auto desc = svcIf.getDescription();
            ke2 = desc.first;
            if (ke2) {
                qDebug() << "getDescription failed:" << ke2.getErrorName() << ke2.getErrorMessage();
            } else {
                descService.setDescription(desc.second);
            }

            auto actState = svcIf.getActiveState();
            ke2 = actState.first;
            if (ke2) {
                qDebug() << "getActiveState failed:" << ke2.getErrorName() << ke2.getErrorMessage();
            } else {
                descService.setActiveState(actState.second);
            }

            auto loadState = svcIf.getLoadState();
            ke2 = loadState.first;
            if (ke2) {
                qDebug() << "getLoadState failed:" << ke2.getErrorName() << ke2.getErrorMessage();
            } else {
                descService.setLoadState(loadState.second);
            }

            auto subState = svcIf.getSubState();
            ke2 = subState.first;
            if (ke2) {
                qDebug() << "getSubState failed:" << ke2.getErrorName() << ke2.getErrorMessage();
            } else {
                descService.setSubState(subState.second);
            }

            auto mainPID = svcIf.getMainPID();
            ke2 = mainPID.first;
            if (ke2) {
                qDebug() << "getMainPID failed:" << ke2.getErrorName() << ke2.getErrorMessage();
            } else {
                descService.setMainPID(mainPID.second);
            }
        }
        return descService;
    } catch (const std::exception& e) {
        qWarning() << "Exception in mapServiceFiles:" << e.what();
        return ServiceInfo(); // 返回默认构造的ServiceInfo
    } catch (...) {
        qWarning() << "Unknown exception in mapServiceFiles";
        return ServiceInfo(); // 返回默认构造的ServiceInfo
    }
    };
    qDebug()<<"POSITION:"<<__FILE__<<","<<__LINE__;
    auto filterServiceFileAsync = QtConcurrent::filter(serviceFileList, filterServiceFiles);
    filterServiceFileAsync.waitForFinished();
    auto serviceFileInfoResult = QtConcurrent::mappedReduced<ServiceInfoList>(serviceFileList, mapServiceFiles, addServiceFileInfoToList);

    serviceInfoResult.waitForFinished();
    serviceFileInfoResult.waitForFinished();
    qDebug()<<"POSITION:"<<__FILE__<<","<<__LINE__;
    list << serviceInfoResult.result();
    list << serviceFileInfoResult.result();
    Q_EMIT serviceListReady(list);
    qDebug()<<"POSITION:"<<__FILE__<<","<<__LINE__;
}

// 从服务文件中读取服务描述信息
QString ServiceDataWorker::readServiceDescriptionFromServiceFile(const QString &path)
{
    // 修复：避免 toLocal8Bit() 临时对象悬空
    QByteArray localPath = path.toLocal8Bit();
    FILE *fp = fopen(localPath.constData(), "r");
    if (!fp)
        return QString();

    char *line = nullptr;   // getline 自动分配/复用
    size_t cap = 0;         // 当前缓冲区容量
    ssize_t nread;
    QString result;

    while ((nread = getline(&line, &cap, fp)) != -1) {
        // 性能优化1：长度预检，快速跳过非目标短行
        // 性能优化2：strncmp 常数时间前缀匹配，无 sscanf 解析开销
        if (nread >= 12 && strncmp(line, "Description=", 12) == 0) {
            // 去掉尾部换行符
            if (nread > 12 && line[nread - 1] == '\n') {
                line[nread - 1] = '\0';
            }

            // 性能优化3：直接从 getline 缓冲区构造 QString，零中间拷贝
            result = QString::fromLocal8Bit(line + 12);
            break;
        }
    }

    free(line);   // getline 分配的内存必须释放
    fclose(fp);
    return result;
}
