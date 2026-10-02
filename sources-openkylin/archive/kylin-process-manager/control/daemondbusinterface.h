/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DAEMONDBUSINTERFACE_H
#define DAEMONDBUSINTERFACE_H

#include <QDBusInterface>
#include <QObject>

#include <string>
#include <vector>

class DaemonDbusInterface : public QObject
{
    Q_OBJECT
public:
    explicit DaemonDbusInterface(QObject *parent = nullptr);

    double getCpuEnergyConsumption();

    void reclaimProcesses(const std::vector<int> &pids);
    void reclaimProcessGroups(const std::vector<std::string> &groupNames);

    void openPSR();
    void closePSR();
    void setSystemCpuFreq(bool cpufreq);
    void SetSamplingRate(bool samplingrate);
    void SetKernelSched(bool kernelsched);
    void SetDirty(bool dirty);
    void SetNetParams(bool netparams);
    void SetCpuLimit(bool cpulimit);

    void setVRRRefreshRate(std::string mode);
    void reverseVRRRefreshRate();
    void setSystemdUnitRun(const std::string &unitName, bool start);
    void setCurrentScene(const std::vector<std::string> &currScenes);
    void lockGSettingsKey(const std::string &key, bool lock);

Q_SIGNALS:
    void ResourceThresholdWarning(std::string resource, int level);

private Q_SLOTS:
    void onResourceWarning(const QString &resource, int level);

private:
    void asyncCallWithArgumentList(const QString &method, const QList<QVariant> &args);
    QDBusMessage callWithArgumentList(const QString &method, const QList<QVariant> &args);

private:
    QDBusInterface dbusInterface;
};

#endif // DAEMONDBUSINTERFACE_H
