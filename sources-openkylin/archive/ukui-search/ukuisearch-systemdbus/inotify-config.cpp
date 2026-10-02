/*
* Copyright (C) 2024, KylinSoft Co., Ltd.
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
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "inotify-config.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

namespace {
const char kMaxUserWatchesPath[] = "/proc/sys/fs/inotify/max_user_watches";
const char kMaxUserInstancesPath[] = "/proc/sys/fs/inotify/max_user_instances";
constexpr int kMaxUserWatches = 9999999;
constexpr int kMaxUserInstances = 8192;
constexpr int kMaxUserInstancesStep = 128;

bool readSysctlValue(const char *path, int *value)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream ts(&file);
    bool ok = false;
    const int currentValue = ts.read(512).trimmed().toInt(&ok);
    if (!ok) {
        return false;
    }

    *value = currentValue;
    return true;
}

bool writeSysctlValue(const char *path, int value)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    const QByteArray data = QByteArray::number(value);
    return file.write(data) == data.size();
}
} // namespace

InotifyConfig::InotifyConfig(QObject* parent)
{
    Q_UNUSED(parent)
}

QString InotifyConfig::setInotifyMaxUserWatchesStep2()
{
    int currentValue = 0;
    if (!readSysctlValue(kMaxUserWatchesPath, &currentValue)) {
        return {};
    }
    if (currentValue >= kMaxUserWatches) {
        return QString::number(currentValue);
    }

    if (!writeSysctlValue(kMaxUserWatchesPath, kMaxUserWatches)) {
        return {"setInotifyMaxUserWatchesStep2: write failed"};
    }

    return QStringLiteral("fs.inotify.max_user_watches = %1").arg(kMaxUserWatches);
}

int InotifyConfig::AddInotifyMaxUserInstance(int addNum)
{
    if (addNum <= 0 || addNum > kMaxUserInstancesStep) {
        qWarning() << "AddInotifyMaxUserInstance: invalid addNum" << addNum;
        return -1;
    }

    int currentValue = 0;
    if (!readSysctlValue(kMaxUserInstancesPath, &currentValue)) {
        return -1;
    }

    if (currentValue >= kMaxUserInstances) {
        return currentValue;
    }

    int instances = currentValue + addNum;
    if (instances > kMaxUserInstances) {
        instances = kMaxUserInstances;
    }

    if (!writeSysctlValue(kMaxUserInstancesPath, instances)) {
        qWarning() << "AddInotifyMaxUserInstance: write failed";
        return -1;
    }

    qDebug() << "AddInotifyMaxUserInstance finished:" << instances;
    return instances;
}
