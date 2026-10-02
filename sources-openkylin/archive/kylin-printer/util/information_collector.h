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
#ifndef INFORMATION_COLLECTOR_H
#define INFORMATION_COLLECTOR_H

#include <QString>

namespace kdk::kabase{
    class BuriedPoint;
}

class InformationCollector : public QObject
{
    Q_OBJECT
public:
    enum P {
        ManuallyInstallPrinter, /* 手动安装打印机 */
        SetResolution,          /* 设置分辨率 */
        SetPaperSize,           /* 设置纸张大小 */
        SetPaperType,           /* 设置纸张类型 */
        SetCationSource,        /* 设置纸盒来源 */
        SetDuplexPrint,         /* 设置双面打印 */
        SetInkType,             /* 设置墨水类型 */
        ADDRemovePrinter,       /* 添加删除打印机 */
        SetShareStartup,        /* 设置共享启动 */
        PrintTestPage,          /* 打印测试页 */
        CancelPrintJob,         /* 取消打印任务 */
        DeletePrintJob,         /* 删除打印任务 */
        RePrint,                /* 重新打印 */
        ManualyModifyDrive,     /* 手动修改驱动 */
        Rename,                 /* 重命名 */
    };
    static InformationCollector& getInstance()
    {
        static InformationCollector instance;
        return instance;
    }
    void addPoint(P point) const;
signals:
    void addMessage(const QString &str) const;
    void addMessageMap(const QString &messageType, const QMap<QString, QString> &messageMap) const;

#ifdef TEST_DEBUG
public:
    void test() const;
#endif
private:
    void sendMessage(const QString &key, const QString &value) const;
    void sendMessageMap(const QString &messageType, const QMap<QString, QString> &messageMap) const;
    InformationCollector(/* args */);
    InformationCollector(InformationCollector const&);
    InformationCollector& operator=(InformationCollector const&);
    ~InformationCollector();
};

#endif // INFORMATION_COLLECTOR_H
