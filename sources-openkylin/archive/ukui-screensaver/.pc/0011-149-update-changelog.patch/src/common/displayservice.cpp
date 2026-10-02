/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "displayservice.h"
#include <QException>
#include <QDebug>
#include <QProcess>
#include <QStandardPaths>

DisplayService *DisplayService::m_instance = nullptr;
DisplayService::DisplayService(QObject *parent) : QObject(parent)
{
    // 使用usd设置显示参数
    m_strSaveParamPath = QStandardPaths::findExecutable("save-param");
}

DisplayService *DisplayService::instance(QObject *parent)
{
    if (m_instance == nullptr)
        m_instance = new DisplayService(parent);
    return m_instance;
}

bool DisplayService::switchDisplayMode(DisplayMode targetMode)
{
    if (m_strSaveParamPath.isEmpty() || m_strUserName.isEmpty()) {
        //获取显示器列表
        getMonitors();

        //如果当前只有一个显示器就什么也不做
        if (monitors.keys().size() < 2) {
            qDebug() << "only one monitor, doNothing";
            return false;
        }

        QProcess subProcess;
        //模式切换
        try {
            switch (targetMode) {
                case DISPLAY_MODE_ORI: {
                    QString XRANDR_ORIONLY = "xrandr --output " + monitorNames[0] + " --auto";
                    for (int i = 1; i < monitorNames.size(); i++)
                        XRANDR_ORIONLY = XRANDR_ORIONLY + " --output " + monitorNames[i] + " --off";
                    qDebug() << "XRANDR_ORIONLY: " << XRANDR_ORIONLY;

                    subProcess.start(XRANDR_ORIONLY, QIODevice::NotOpen);
                    break;
                }
                case DISPLAY_MODE_CLONE: {
                    //查找最佳克隆分辨率
                    QString BEST_CLONE_MODE;
                    bool find = false;
                    for (auto &mode_0 : monitors[monitorNames[0]]) {
                        for (auto &mode_1 : monitors[monitorNames[1]]) {
                            if (mode_0 == mode_1) {
                                BEST_CLONE_MODE = mode_0;
                                find = true;
                                break;
                            }
                        }
                        if (find)
                            break;
                    }
                    QString XRANDR_CLONE = "xrandr --output " + monitorNames[0] + " --mode " + BEST_CLONE_MODE;
                    for (int i = 1; i < monitorNames.size(); i++)
                        XRANDR_CLONE = XRANDR_CLONE + " --output " + monitorNames[i] + " --mode " + BEST_CLONE_MODE
                                       + " --same-as " + monitorNames[0];
                    qDebug() << "XRANDR_CLONE: " << XRANDR_CLONE;

                    subProcess.start(XRANDR_CLONE, QIODevice::NotOpen);
                    break;
                }
                case DISPLAY_MODE_EXTEND: {
                    QString XRANDR_EXTEND = "xrandr --output " + monitorNames[0] + " --auto";
                    for (int i = 1; i < monitorNames.size(); i++)
                        XRANDR_EXTEND = XRANDR_EXTEND + " --output " + monitorNames[i] + " --right-of "
                                        + monitorNames[i - 1] + " --auto";
                    qDebug() << "XRANDR_EXTEND: " << XRANDR_EXTEND;

                    subProcess.start(XRANDR_EXTEND, QIODevice::NotOpen);
                    break;
                }
                case DISPLAY_MODE_ONLY_OUT: {
                    QString XRANDR_OUTONLY
                        = "xrandr --output " + monitorNames[0] + " --off --output " + monitorNames[1] + " --auto";
                    qDebug() << "XRANDR_OUTONLY: " << XRANDR_OUTONLY;

                    subProcess.start(XRANDR_OUTONLY, QIODevice::NotOpen);
                    break;
                }
            }
            subProcess.waitForFinished();
            return true;
        } catch (const QException &e) {
            qWarning() << e.what();
            return false;
        }
    } else {
        QProcess subProcess;
        try {
            QString usdSaveParam = m_strSaveParamPath + " -u " + m_strUserName;
            qDebug() << "usdSaveParam: " << usdSaveParam;
            subProcess.start(usdSaveParam, QIODevice::NotOpen);
            subProcess.waitForFinished();
            return true;
        } catch (const QException &e) {
            qWarning() << e.what();
            return false;
        }
    }
}

void DisplayService::setOneDisplayMode()
{
    //组装第一个设备名显示命令
    QProcess subProcess;
    try {
        QString oneXrandrCmd = "";
        if (m_strSaveParamPath.isEmpty() || m_strUserName.isEmpty()) {
            //获取显示器列表
            getMonitors();
            if (monitorNames.size() <= 0) {
                return;
            }
            oneXrandrCmd = "xrandr --output " + monitorNames[0] + " --auto";
        } else {
            oneXrandrCmd = m_strSaveParamPath + " -u " + m_strUserName;
        }
        qDebug() << "setOneDisplayMode: " << oneXrandrCmd;
        subProcess.start(oneXrandrCmd);
        subProcess.waitForFinished();
        return;
    } catch (const QException &e) {
        qWarning() << e.what();
        return;
    }
}

void DisplayService::setCurUserName(QString strUserName)
{
    m_strUserName = strUserName;
    if (!m_strSaveParamPath.isEmpty()) {
        QProcess subProcess;
        try {
            QString usdSaveParam = m_strSaveParamPath + " -u " + m_strUserName;
            qDebug() << "setCurUserName usdSaveParam: " << usdSaveParam;
            subProcess.start(usdSaveParam);
            subProcess.waitForFinished();
        } catch (const QException &e) {
            qWarning() << e.what();
        }
    }
}

bool DisplayService::isSaveParamInUsed()
{
    return !(m_strSaveParamPath.isEmpty());
}

void DisplayService::getMonitors()
{
    QProcess subProcess;
    subProcess.setProgram(QStandardPaths::findExecutable("xrandr"));
    subProcess.setArguments({ "-q" });
    subProcess.start(QIODevice::ReadOnly);
    subProcess.waitForFinished();
    QString outputs = subProcess.readAll();
    QStringList lines = outputs.split('\n');
    QString name;
    QVector<QString> modes;
    bool find = false;
    QString lastName;
    monitorNames.clear();
    monitors.clear();
    for (auto &line : lines) {
        if (line.indexOf(" connected") != -1) { //找显示器名称
            name = line.left(line.indexOf(' '));
            monitorNames.push_back(name);
            if (find) //又找到了一个显示器，将上一个显示器的信息存到map
                monitors[lastName] = modes;
            find = true;
            lastName = name;
            modes.clear();
        } else {
            if (line.startsWith(' ')) { //获取分辨率
                QString mode = line.trimmed().split(' ').at(0);
                modes.push_back(mode);
            }
        }
    }
    monitors[name] = modes; //将最后一个显示器的信息存储到map

    qDebug() << "find motinors: " << monitorNames;
}

int DisplayService::isJJW7200()
{
    static int ret = -1;
    char *pAck = NULL;
    char CmdAck[256] = "";
    FILE *pPipe = NULL;

    if (ret != -1) {
        return ret;
    }

    pPipe = popen("lspci | grep -i VGA |grep 7200", "r");
    if (pPipe) {
        pAck = fgets(CmdAck, sizeof(CmdAck) - 1, pPipe);
        if (strlen(CmdAck) > 3) {
            ret = 1;
        } else {
            ret = 0;
        }

        pclose(pPipe);
    } else {
        ret = 0;
    }

    return ret;
}
