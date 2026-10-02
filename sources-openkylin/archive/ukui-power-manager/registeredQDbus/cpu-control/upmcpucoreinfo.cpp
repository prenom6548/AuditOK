/*
 * Copyright 2023 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "upmcpucoreinfo.h"

#include "../upmcommonfile.h"
#include "../upmcustomhardwareconfig.h"
#include <QDebug>

UpmCpuCoreInfo::UpmCpuCoreInfo(int cpuCoreId, QObject *parent) : QObject(parent)
{
    m_cpuCoreId = cpuCoreId;

    QString result;
    if (true == UpmCommonFile::read(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                                    CPU_SCALING_AVAILABLE_GOVERNORS, result)) {
        if(UpmCustomHardwareConfig::self()->isD3000M()) {
            m_coreFreqMode[Balance] = "schedutil";
        } else {
            if (result.contains("ondemand")) {
                m_coreFreqMode[Balance] = "ondemand";
            } else if (result.contains("schedutil")) {
                m_coreFreqMode[Balance] = "schedutil";
            }
        }
    }

    if (true == UpmCommonFile::read(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                                    CPU_AVAILABLE_FREQ, result)) {
        m_avaiableFreq = result.replace("\n","").split(0x20, QString::SkipEmptyParts);

        qSort(m_avaiableFreq.begin(), m_avaiableFreq.end(),[](const QString &a, const QString &b){
            return a.toUInt() > b.toInt();
        });//降序
        qDebug()<<m_defaultMaxFreq<<m_defaultMinFreq;
        m_defaultMaxFreq = QString(m_avaiableFreq.at(0)).toUInt();
        m_defaultMinFreq = QString(m_avaiableFreq.at(m_avaiableFreq.count()-1)).toUInt();

    } else {
        if (true == UpmCommonFile::read(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                                        CPU_SCALING_MAX_FREQ, result)) {
            m_defaultMaxFreq = result.toUInt();
        }

        if (true == UpmCommonFile::read(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                                        CPU_SCALING_MIN_FREQ, result)) {
            m_defaultMinFreq = result.toUInt();
        }}

               qDebug()<<m_defaultMaxFreq<<m_defaultMinFreq;
    if (true == UpmCommonFile::read(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                                    CPU_AVAILABLE_GOVERNORS, result)) {
        m_avaiableMode = result.replace("\n","").split(0x20,QString::SkipEmptyParts);
    }
}

int UpmCpuCoreInfo::getCpuCoreOnlineState()
{
    QString result;
    if (true == UpmCommonFile::read(QString(CPU_DEVICES_PATH).arg(m_cpuCoreId),
                                    CPU_CORE_ONLINE, result)) {
        return result.toInt();
    }
    return cpu_core_error;
}

void UpmCpuCoreInfo::setCpuCoreOnlineState(int online)
{
    if (cpu_core_online == online || cpu_core_offline == online) {
        UpmCommonFile::write(QString(CPU_DEVICES_PATH).arg(m_cpuCoreId),
                             CPU_CORE_ONLINE, QString::number(online));
    }
}

void UpmCpuCoreInfo::setCorePolicy(int cpuPolicy)
{
    if (cpuPolicy < Performance || cpuPolicy > Powersave) {
        return ;
    }
    qDebug() << "set cpu:" << m_cpuCoreId << " to " << m_coreFreqMode[cpuPolicy];
    UpmCommonFile::write(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                         CPU_SCALING_GOVERNORS,
                         m_coreFreqMode[cpuPolicy]);
}

void UpmCpuCoreInfo::setCorePolicy(QString cpuPolicy)
{
    if (!m_avaiableMode.contains(cpuPolicy)) {
        qDebug()<<"error" << cpuPolicy << m_avaiableMode;
        return;
    }

    UpmCommonFile::write(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                         CPU_SCALING_GOVERNORS,
                         cpuPolicy);
}

void UpmCpuCoreInfo::setCoreMaxFreq(int cpuFrequency)
{
    if (cpuFrequency > m_defaultMaxFreq) {
        UpmCommonFile::write(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                             CPU_SCALING_MAX_FREQ, QString::number(m_defaultMaxFreq));
    } else if (cpuFrequency < m_defaultMinFreq) {
        UpmCommonFile::write(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                             CPU_SCALING_MAX_FREQ, QString::number(m_defaultMinFreq));
    } else {
        UpmCommonFile::write(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                             CPU_SCALING_MAX_FREQ, QString::number(cpuFrequency));
    }
}

void UpmCpuCoreInfo::setCoreMinFreq(int cpuFrequency)
{
    if (cpuFrequency > m_defaultMaxFreq) {
        UpmCommonFile::write(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                             CPU_SCALING_MIN_FREQ, QString::number(m_defaultMaxFreq));
    } else if (cpuFrequency < m_defaultMinFreq) {
        UpmCommonFile::write(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                             CPU_SCALING_MIN_FREQ, QString::number(m_defaultMinFreq));
    } else {
        UpmCommonFile::write(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                             CPU_SCALING_MIN_FREQ, QString::number(cpuFrequency));
    }
}

int UpmCpuCoreInfo::getDefaultCoreMaxFreq()
{
    return m_defaultMaxFreq;
}

void UpmCpuCoreInfo::setDefaultCoreMaxFreq()
{
    UpmCommonFile::write(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                         CPU_SCALING_MAX_FREQ, QString::number(m_defaultMaxFreq));
}

void UpmCpuCoreInfo::setCoreMaxFreqLevel(const QString &level)
{
    // 读取可用频率
    QString availableFreqStr;
    bool readSuccess = UpmCommonFile::read(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                        CPU_SCALING_AVAILABLE_FREQ, availableFreqStr);
    if (!readSuccess) {
        qInfo("no scaling_available_frequencies file node");
        return;
    }

    std::vector<unsigned long> frequencies;
    QStringList freqList = availableFreqStr.split(" ", QString::SkipEmptyParts);

    foreach (const QString &freq, freqList) {
        bool ok;
        unsigned long value = freq.toULong(&ok);
        if (ok) {
            frequencies.push_back(value);
            qDebug() << "freq:" << value;
        } else {
            qDebug() << "无效的频率值:" << freq;
        }
    }

    std::sort(frequencies.begin(), frequencies.end());
    unsigned long targetFreq = 0;
    if (level.compare("max", Qt::CaseInsensitive) == 0) {
        targetFreq = frequencies.back();
    } else if (level.compare("min", Qt::CaseInsensitive) == 0) {
        targetFreq = frequencies.front();
    } else if (level.compare("medium", Qt::CaseInsensitive) == 0) {
        size_t count = frequencies.size();
        //处理偶数个频率的情况
        if (count % 2 == 0) {
            size_t lowerMidIndex = (count / 2) - 1;
            size_t upperMidIndex = count / 2;
            //中间偏下的频率
            //targetFreq = frequencies[lowerMidIndex];
            //中间偏上的频率
            //targetFreq = frequencies[upperMidIndex];

            //接近两个中间值平均值的频率
            unsigned long midAvg = (frequencies[lowerMidIndex] + frequencies[upperMidIndex]) / 2;
            if (midAvg - frequencies[lowerMidIndex] <= frequencies[upperMidIndex] - midAvg) {
                targetFreq = frequencies[lowerMidIndex];
            } else {
                targetFreq = frequencies[upperMidIndex];
            }
        } else {
            //奇数个值时选择正中间的频率
            size_t midIndex = count / 2;
            targetFreq = frequencies[midIndex];
        }
    }

    qDebug() << "setCoreMaxFreqLevel -> targetFreq:" << targetFreq;

    if (frequencies.size() > 0) {
        bool ret = false;
        ret = UpmCommonFile::write(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                         CPU_SCALING_MAX_FREQ, QString::number(targetFreq));
        if (ret) {
            qInfo() << "setCore:" << m_cpuCoreId << " MaxFreq to " <<targetFreq;
        }
    }
}

void UpmCpuCoreInfo::setCoreMinFreqPercentage(const QString &percentage)
{
        // 读取可用频率
    QString availableFreqStr;
    bool readSuccess = UpmCommonFile::read(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                        CPU_SCALING_AVAILABLE_FREQ, availableFreqStr);
    if (!readSuccess) {
        qInfo("no scaling_available_frequencies file node");
        unsigned long maxFreq = m_defaultMaxFreq;  // 列表已排序，最后一个是最大值
        unsigned long minFreq = m_defaultMinFreq; // 第一个是最小值
        unsigned long step = maxFreq - minFreq;     // 频率范围差值
        // 3. 计算目标频率
        bool ok;
        int percentValue = percentage.toInt(&ok); // 将 QString 转为 int
        if (!ok || percentValue < 1 || percentValue > 100) {
            qWarning() << "Invalid percentage value:" << percentage;
            return;
        }
        // 计算公式：freq_qiu = ((percentage - 1) / 99) (maxFreq - minFreq) + minFreq
        float targetFreqFloat = ((percentValue - 1.0f) / 99.0f) * step + minFreq;
        unsigned long targetFreq = static_cast<unsigned long>(targetFreqFloat);

        bool ret = false;
        ret = UpmCommonFile::write(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                         CPU_SCALING_MIN_FREQ, QString::number(targetFreq));
        if (ret) {
            qInfo() << "setCore:" << m_cpuCoreId << " MinFreq to " <<targetFreq;
        }
    }
    else
    {
        std::vector<unsigned long> frequencies;

        QStringList freqList = availableFreqStr.split(" ", QString::SkipEmptyParts);

        foreach (const QString &freq, freqList) {
            bool ok;
            unsigned long value = freq.toULong(&ok);
            if (ok) {
                frequencies.push_back(value);
                qInfo() << "freq:" << value;
            } else {
                qInfo() << "无效的频率值:" << freq;
            }
        }

        std::sort(frequencies.begin(), frequencies.end());

        unsigned long maxFreq = frequencies.back();  // 列表已排序，最后一个是最大值
        unsigned long minFreq = frequencies.front(); // 第一个是最小值
        unsigned long step = maxFreq - minFreq;     // 频率范围差值
        // 3. 计算目标频率
        bool ok;
        int percentValue = percentage.toInt(&ok); // 将 QString 转为 int
        if (!ok || percentValue < 1 || percentValue > 100) {
            qWarning() << "Invalid percentage value:" << percentage;
            return;
        }
        // 计算公式：freq_qiu = ((percentage - 1) / 99) (maxFreq - minFreq) + minFreq
        float targetFreqFloat = ((percentValue - 1.0f) / 99.0f) * step + minFreq;
        unsigned long targetFreq = static_cast<unsigned long>(targetFreqFloat);
        // 4. 查找最接近的可用频率
        unsigned long nearestFreq = minFreq;
        unsigned long minDiff = std::abs(static_cast<long>(targetFreq - minFreq));
        for (const auto &freq : frequencies) {
            unsigned long diff = std::abs(static_cast<long>(targetFreq - freq));
            if (diff < minDiff) {
                minDiff = diff;
                nearestFreq = freq;
            }
        }
        qInfo() << "Calculated Frequency:" << targetFreq << "Nearest Available Frequency:" << nearestFreq;
        bool ret = false;
        ret = UpmCommonFile::write(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                         CPU_SCALING_MIN_FREQ, QString::number(nearestFreq));
        if (ret) {
            qInfo() << "setCore:" << m_cpuCoreId << " MinFreq to " <<nearestFreq;
        }
    }
}

void UpmCpuCoreInfo::setUserSetSpeed(int cpuFrequency)
{
    if (cpuFrequency > m_defaultMaxFreq) {
        UpmCommonFile::write(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                             CPU_SCALING_SETSPEED, QString::number(m_defaultMaxFreq));
    } else if (cpuFrequency < m_defaultMinFreq) {
        UpmCommonFile::write(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                             CPU_SCALING_SETSPEED, QString::number(m_defaultMinFreq));
    } else {
        UpmCommonFile::write(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                             CPU_SCALING_SETSPEED, QString::number(cpuFrequency));
    }
}


QString UpmCpuCoreInfo::getCpuCorePolicy()
{
    QString result;
    UpmCommonFile::read(QString(CPU_DEVICES_FREQ_PATH).arg(m_cpuCoreId),
                        CPU_SCALING_GOVERNORS, result);
    return result;
}

QStringList UpmCpuCoreInfo::getAllAvailableFreq()
{
    return m_avaiableFreq;
}

QStringList UpmCpuCoreInfo::getAllAvailableGovernors()
{
    return m_avaiableMode;
}
