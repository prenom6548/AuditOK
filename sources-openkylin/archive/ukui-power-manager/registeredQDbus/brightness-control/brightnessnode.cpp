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

#include <QDir>
#include <QMap>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

#include "brightnessnode.h"
#include "hardware-reader/hardwarereader.h"
#include "dmi_chassis_type.h"
/*
 * 根据笔记本概率性无法设置亮度的问题，由于使用log5qt的机制，无法记录提权
 * 进程的日志，暂时无法分析，只能根据最大亮度为0，以及不支持设置亮度的情况。
 * 制定如下策略：
 * 1.先判断设备是否支持亮度。
 * 2.读取设备的亮度信息。
 * 3.如果支持亮度，但是亮度读取失败则重试若干次。
 */
BrightnessNode::BrightnessNode(QObject *parent) :
    m_maxbrightness(0),
    m_hasEffectiveNode(false),
    m_isBrightnessEnhancementSupported(false),
    QObject(parent)
{
    m_isCanAdjustBright = isBrightnessAdjustable();

    if (true == m_isCanAdjustBright) {
        QString brightnessNode = UpmCustomHardwareConfig::self()->getBrightnessNode();
        initBrightnessNodeInfo(brightnessNode);
        initBrightnessEnhancementCapability();
        if (0 >= m_maxbrightness) {
            m_BrightnessTimer = new QTimer(this);
            m_BrightnessTimer->setSingleShot(false);
            QObject::connect(m_BrightnessTimer, &QTimer::timeout, this, &BrightnessNode::onBrightnessTimeOut);
            m_BrightnessTimer->start(1000);
        }
    }

    m_brightnessAdjustTimer = new QTimer(this);
    connect(m_brightnessAdjustTimer, &QTimer::timeout, this, &BrightnessNode::setSmootBrightness);
}

BrightnessNode::~BrightnessNode() {}

void BrightnessNode::initBrightnessNodeInfo(QString &brightnessNode)
{
    QDir brightnessNodeDir(BRIGHTNESS_NODE_PATH);

    static QMap<QString,QString> nodeTypeMap;


    if (!brightnessNode.isNull()) {
        m_effectiveNodePath = BRIGHTNESS_NODE_PATH + brightnessNode;
        if (!QFileInfo::exists(m_effectiveNodePath)) {
            qWarning() << "the configured brightness node does not exist:" << m_effectiveNodePath;
            // 配置的节点不存在时，不应直接返回；继续走后续自动探测以回退到可用节点
            m_hasEffectiveNode = false;
            m_effectiveNodePath.clear();
            m_maxbrightness = 0;
        } else {
            m_hasEffectiveNode = true;
            m_maxbrightness = getBrightnessNodeFileContent(m_effectiveNodePath, "/max_brightness").toInt();

            if (0 < m_maxbrightness) {
                return;
            }
            qWarning() << "max brightness get error!";
        }
    }



    m_nodeDirs.clear();
    m_nodeDirs = brightnessNodeDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (int i = 0; i < m_nodeDirs.size(); ++i) {
        nodeTypeMap.insert(getBrightnessNodeFileContent(BRIGHTNESS_NODE_PATH + m_nodeDirs.at(i), "/type"),
                        BRIGHTNESS_NODE_PATH + m_nodeDirs.at(i));
    }

    qWarning() << "all nodeDir" << nodeTypeMap;
    QStringList nodeTypes = {"firmware", "platform", "raw"};//优先级.
    for (int i = 0; i < nodeTypes.size(); ++i) {
        if (nodeTypeMap.keys().contains(nodeTypes[i])) {
            m_effectiveNodePath = nodeTypeMap[nodeTypes[i]];
            m_hasEffectiveNode = true;
            m_maxbrightness = getBrightnessNodeFileContent(m_effectiveNodePath, "/max_brightness").toInt();
            break;
        }
    }
}


bool BrightnessNode::isBrightnessAdjustable()
{
    QFile file("/sys/class/dmi/id/chassis_type");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open chassis_type file";
        return false;
    }

    QString chassisTypeStr = file.readAll().trimmed();
    file.close();

    bool ok = false;
    int chassisTypeInt = chassisTypeStr.toInt(&ok);
    if (!ok) {
        qWarning() << "Invalid chassis_type format:" << chassisTypeStr;
        return false;
    }

    switch (static_cast<DmiChassisType>(chassisTypeInt)) {
        case DmiChassisType::Laptop:
        case DmiChassisType::Notebook:
        case DmiChassisType::AllInOne:
        case DmiChassisType::Tablet:
        case DmiChassisType::Convertible:
        case DmiChassisType::Detachable:
            return true;
        default:
            qDebug() << "Device type not supported for brightness:" << chassisTypeInt;
            return false;
    }
}

QString BrightnessNode::getBrightnessNodeFileContent(const QString &strNodePath, const QString &strNodeFile)
{
    QFile nodeFile(strNodePath + strNodeFile);
    if (!nodeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "open file error :" << strNodePath + strNodeFile;
        return "";
    }

    QString fileContent = nodeFile.readLine();
    nodeFile.close();
    qWarning() << "read" << nodeFile.fileName() << fileContent;
    return fileContent.remove('\n');
}

bool BrightnessNode::canSetBrightness()
{
    return m_isCanAdjustBright;
}

int BrightnessNode::getMaxBrightness()
{
    return m_maxbrightness;
}

int BrightnessNode::getCurrentBrightness()
{
    return getBrightnessNodeFileContent(m_effectiveNodePath, "/brightness").toInt();
}

QString BrightnessNode::setCurrentBrightness(int iBrightness)
{
    if (!m_hasEffectiveNode) {
        return "no effective node";
    }

    m_targetBrightness = iBrightness;

    m_currentBrightness = getCurrentBrightness();

    if (m_currentBrightness == m_targetBrightness) {
        return "not need set";
    }

    m_brightnessStep = qMax(5, int(m_maxbrightness * 0.05));
    if (!m_brightnessAdjustTimer->isActive()) {
        m_brightnessAdjustTimer->start(10);
    }

    return QString("%1 set brightness %2").arg(m_effectiveNodePath).arg(iBrightness);
}

void BrightnessNode::onBrightnessTimeOut()
{
    static QString brightnessNode = UpmCustomHardwareConfig::self()->getBrightnessNode();
    static int getBrightnessNodeTimes = 0;

    initBrightnessNodeInfo(brightnessNode);
    qWarning() << "node path :" << m_effectiveNodePath;
    if (getBrightnessNodeTimes++ > 3 ||
            true == m_hasEffectiveNode ||
            m_maxbrightness > 0) {
        qWarning()<<"effectNode"<<m_hasEffectiveNode;
        qWarning()<<"maxBrightness"<<m_maxbrightness;
        m_BrightnessTimer->stop();
    }
}

bool BrightnessNode::isBrightnessEnhancementSupported()
{
    return m_isBrightnessEnhancementSupported;
}

void BrightnessNode::initBrightnessEnhancementCapability()
{
    // BrightnessEnhancement‌ 判断是否支持亮度增强
    const QStringList bn_enhancementSupported  = {"LXKT-FTD3-X1", "LXKT-FTD3-N8"};

    if (m_isCanAdjustBright
            && bn_enhancementSupported.contains(HardwareReader::instance()->getDMIInfo("board_name"), Qt::CaseInsensitive)
            && EdidComparator::isSameScreen())
    {
        m_isBrightnessEnhancementSupported = true;
        qInfo() << "brightness enhancement supported and maxBrightness =" << m_maxbrightness;
    }

}

void BrightnessNode::setSmootBrightness()
{
    if (m_currentBrightness < m_targetBrightness) {
        m_currentBrightness = qMin(m_currentBrightness + m_brightnessStep, m_targetBrightness);
    } else if (m_currentBrightness > m_targetBrightness) {
        m_currentBrightness = qMax(m_currentBrightness - m_brightnessStep, m_targetBrightness);
    } else {
        m_brightnessAdjustTimer->stop();
        return;
    }

    QFile brightnessFile(m_effectiveNodePath + "/brightness");
    if (!brightnessFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open brightness file:" << m_effectiveNodePath + "/brightness";
        return;
    }
    QTextStream in(&brightnessFile);
    in << m_currentBrightness << endl;
    brightnessFile.close();
}