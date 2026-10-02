/*
    Copyright (C) 2012  Dan Vratil <dvratil@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "qmlscreen.h"
#include "qmloutputcomponent.h"
#include "qmloutput.h"

#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/config.h>

#include <QTimer>
#include <sys/socket.h>
#include <QtMath>

Q_DECLARE_METATYPE(KScreen::OutputPtr)

QMLScreen::QMLScreen(QQuickItem *parent) :
    QQuickItem(parent)
{
    connect(this, &QMLScreen::widthChanged, this, &QMLScreen::viewSizeChanged);
    connect(this, &QMLScreen::heightChanged, this, &QMLScreen::viewSizeChanged);
}

QMLScreen::~QMLScreen()
{
    qDeleteAll(m_outputMap);
    m_outputMap.clear();
}

KScreen::ConfigPtr QMLScreen::config() const
{
    return m_config;
}

void QMLScreen::setConfig(const KScreen::ConfigPtr &config)
{
    qDeleteAll(m_outputMap);
    m_outputMap.clear();
    m_manuallyMovedOutputs.clear();
    m_connectedOutputsCount = 0;
    m_enabledOutputsCount = 0;

    if (m_config) {
        m_config->disconnect(this);
    }

    m_config = config;
    connect(m_config.data(), &KScreen::Config::outputAdded, this, [this](const KScreen::OutputPtr &output) {
        qDebug() << "add output" << output->name() << m_outputMap;
        m_pendingOutputs.append(output);
        QTimer::singleShot(1000, this, [=] {
            if (!m_pendingOutputs.contains(output)) {
                return;
            }
            m_pendingOutputs.removeAll(output);
            addOutput(output);
            m_isPosChanged = true;
            updateOutputsPlacement();
            m_isPosChanged = false;
            setScreenPos(primaryOutput(), false);
        });
    });
    connect(m_config.data(), &KScreen::Config::outputRemoved, this, [this](int outputId) {
        qDebug() << "remove output" << outputId << m_outputMap;
        QList<KScreen::OutputPtr> tmpPendingOutputs = m_pendingOutputs;
        for (KScreen::OutputPtr output : tmpPendingOutputs) {
            if (output->id() == outputId) {
                m_pendingOutputs.removeAll(output);
            }
        }
        removeOutput(outputId);
    });

    for (const KScreen::OutputPtr &output : m_config->outputs()) {
        addOutput(output);
    }
    setOutputEnabled();
    m_isScreenInit = true;  // 初始化摆放位置时按实际屏幕位置摆放
    qDebug() << Q_FUNC_INFO << "setConfig before updateOutputsPlacement";
    updateOutputsPlacement();
    m_isScreenInit = false;

    QMLOutput* base = primaryOutput();
    qDebug() << Q_FUNC_INFO << "screen isCloneMode" << m_isCloneMode;
    for (QMLOutput *output: outputs()) {
        if (!output || !base || !output->output() || !output->output()->isConnected()) {
            qDebug() << Q_FUNC_INFO << "output is null";
            continue;
        }
        if (m_isCloneMode && output == base) {
            qDebug() << Q_FUNC_INFO << "set primary output isCloneMode true";
            output->setIsCloneMode(true, true);
        } else {
            output->setIsCloneMode(m_isCloneMode, false);
            qDebug() << Q_FUNC_INFO << "set other output isCloneMode" << m_isCloneMode;
        }
    }
    setScreenPos(primaryOutput(), true);  // 初始化或者关闭某个显示器时需重新摆放

    qDebug() << Q_FUNC_INFO << "setConfig after setScreenPos";
}

void QMLScreen::addOutput(const KScreen::OutputPtr &output)
{
    qDebug() << Q_FUNC_INFO << "addOutput start" << output->name() << m_outputMap;
    QMLOutputComponent comp(qmlEngine(this), this);

    QMLOutput *qmloutput = comp.createForOutput(output);
    if (!qmloutput) {
        qWarning() << "Failed to create QMLOutput" << comp.errorString();
        return;
    }
    qmloutput->setObjectName("ukui-control-center_display_output_" + output->name());
    // 在 C++ 侧开启 Tab 焦点，避免在 Output.qml 写 activeFocusOnTab 导致创建失败、预览空白
    qmloutput->setActiveFocusOnTab(output->isConnected());
    connect(output.data(), &KScreen::Output::isConnectedChanged, qmloutput, [qmloutput, output]() {
        if (qmloutput && output) {
            qmloutput->setActiveFocusOnTab(output->isConnected());
        }
    });
    if (isCloneMode()) {
        qmloutput->setVisible(false);
        qDebug() << Q_FUNC_INFO << "isCloneMode addOutput set output visible to false" << output->name() << qmloutput->isVisible() << qmloutput->output()->isEnabled();
    } else {
        qDebug() << Q_FUNC_INFO << "not isCloneMode addOutput set output visible to true" << output->name() << qmloutput->isVisible() << qmloutput->output()->isEnabled();
    }
    for (const KScreen::OutputPtr &curOutput : m_outputMap.keys()) {
        if (curOutput->name() == output->name()) {
            QMLOutput *qmlOutput = m_outputMap.take(curOutput);
            qmlOutput->setParentItem(nullptr);
            qmlOutput->setParent(nullptr);
            qDebug() << Q_FUNC_INFO << "addOutput remove already output name" << output->name();
        }
    }
    m_outputMap.insert(output, qmloutput);

    qmloutput->setParentItem(this);
    qmloutput->setZ(m_outputMap.count());

    connect(output.data(), &KScreen::Output::isConnectedChanged,
            this, &QMLScreen::outputConnectedChanged);
    connect(output.data(), &KScreen::Output::isEnabledChanged,
            this, &QMLScreen::outputEnabledChanged);
    connect(output.data(), &KScreen::Output::posChanged,
            this, &QMLScreen::outputPositionChanged);
    connect(qmloutput, &QMLOutput::yChanged,
            [this, qmloutput]() {
        qmlOutputMoved(qmloutput);
    });
    connect(qmloutput, &QMLOutput::xChanged,
            [this, qmloutput]() {
        qmlOutputMoved(qmloutput);
    });

    // 在这里点击上面小屏幕
    connect(qmloutput, SIGNAL(clicked()),
            this, SLOT(setActiveOutput()));

    connect(qmloutput, SIGNAL(clicked()),
                this, SLOT(getClickedPos()));

    connect(qmloutput, SIGNAL(mouseReleased(bool)),
            this, SLOT(setScreenPos(bool)));

    connect(qmloutput, SIGNAL(rotationChanged(bool)),
            this, SLOT(setScreenPos(bool)));

    connect(qmloutput, SIGNAL(widthChanged(bool)),
            this, SLOT(setScreenPos(bool)));

    connect(qmloutput, SIGNAL(heightChanged(bool)),
            this, SLOT(setScreenPos(bool)));

    connect(qmloutput, &QMLOutput::visibleChanged, this, [=]() {
        visibleChangedSlot();
    });

    qmloutput->updateRootProperties();
    viewSizeChanged();
}

void QMLScreen::visibleChangedSlot() {
    qDebug() << "output visibleChanged";
    m_isPosChanged = true;
    updateOutputsPlacement();
    m_isPosChanged = false;

    viewSizeChanged();
}

void QMLScreen::removeOutput(int outputId)
{
    viewSizeChanged();
    qDebug() << "remove output start" << outputId;
    for (const KScreen::OutputPtr &output : m_outputMap.keys()) {
        if (output->id() == outputId) {
            qDebug() << Q_FUNC_INFO << "removeOutput" << outputId << output->name();
            QMLOutput *qmlOutput = m_outputMap.take(output);
            qmlOutput->setParentItem(nullptr);
            qmlOutput->setParent(nullptr);
            // TODO:bug51346
            // qmlOutput->deleteLater();
            return;
        }
    }
}

int QMLScreen::connectedOutputsCount() const
{
    return m_connectedOutputsCount;
}

int QMLScreen::enabledOutputsCount() const
{
    return m_enabledOutputsCount;
}

QMLOutput *QMLScreen::primaryOutput() const
{
    Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
        if (qmlOutput->output()->isPrimary()) {
            return qmlOutput;
        }
    }

    return nullptr;
}

QList<QMLOutput *> QMLScreen::outputs() const
{
    return m_outputMap.values();
}

void QMLScreen::getClickedPos()
{
    leftPosList.clear();
    // 应该判断所有显示器的坐标是否变化，移动A屏幕可能只会改变A与B屏幕的相对位置，A本身的屏幕位置可能不变(比如(0, 1680))
    Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
        if (qmlOutput->output()->isConnected()) {
            qmlOutput->setIsClicked(true);
            qDebug() << Q_FUNC_INFO << " setclick " << qmlOutput->output()->name() << qmlOutput->isClicked();
            leftPosList.append(qmlOutput->position().x());
            leftPosList.append(qmlOutput->position().y());
        }
    }
}

void QMLScreen::setActiveOutput(QMLOutput *output)
{
    if (!output) {
        return;
    }
    Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
        if (qmlOutput->z() > output->z()) {
            qmlOutput->setZ(qmlOutput->z() - 1);
        }
        // 选中高亮与键盘焦点分离：仅点选/程序切屏改 isSelected，Tab 不改选中
        qmlOutput->setIsSelected(qmlOutput == output);
    }

    output->setZ(m_outputMap.count());
    // 不再 setFocus：Tab 只应出现黑色焦点框，不应因获焦走选中/切配置
    Q_EMIT focusedOutputChanged(output);
}

void QMLScreen::setScreenCenterPos()
{
    // 组成最大矩形四个边的位置，分别对应左上(1)，右下(2)的xy坐标值
    qreal localX1 = -1, localX2 = -1, localY1 = -1, localY2 = -1;
    qreal mX1 = 0, mY1 = 0, mX2 = 0, mY2 = 0; // 矩形中点坐标
    qreal moveX = 0, moveY = 0;// 移动的值
    bool firstFlag = true;
    Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
        if (qmlOutput->output()->isConnected() && qmlOutput->isVisible()) {
            if (firstFlag == true || localX1 > qmlOutput->x()) {
                localX1 = qmlOutput->x();
            }
            if (firstFlag == true || localX2 < qmlOutput->x() + qmlOutput->width()) {
                localX2 = qmlOutput->x() + qmlOutput->width();
            }
            if (firstFlag == true || localY1 > qmlOutput->y()) {
                localY1 = qmlOutput->y();
            }
            if (firstFlag == true || localY2 < qmlOutput->y() + qmlOutput->height()) {
                localY2 = qmlOutput->y() + qmlOutput->height();
            }
            firstFlag = false;
        }
    }

    mX1 = localX1 + (localX2-localX1)/2;
    mY1 = localY1 + (localY2-localY1)/2;

    mX2 = width()/2;
    mY2 = height()/2;

    moveX = mX2 - mX1;
    moveY = mY2 - mY1;

    Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
          if (qmlOutput->output()->isConnected()) {
            qDebug() << Q_FUNC_INFO << "setScreenCenterPos output" << qmlOutput->output()->name() << "isvisible" << qmlOutput->isVisible();
          }
          if (qmlOutput->output()->isConnected() && qmlOutput->isVisible()) {
            qDebug() << Q_FUNC_INFO << " before set position " << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size();
            qmlOutput->blockSignals(true);
            qmlOutput->setX(qmlOutput->x() + moveX);
            qmlOutput->setY(qmlOutput->y() + moveY);
            qmlOutput->blockSignals(false);
            qDebug() << Q_FUNC_INFO << " after set position " << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size();
        }
    }
}

bool QMLScreen::isActualUpDownOutput(QMLOutput *curOutput, QMLOutput *preOutput) {
    if (curOutput == nullptr || preOutput == nullptr) {
        return false;
    }
    bool isUpDown = (curOutput->output()->pos().y() + curOutput->output()->size().height()) == preOutput->output()->pos().y();
    if (!isUpDown) {
        isUpDown = (preOutput->output()->pos().y() + preOutput->output()->size().height()) == curOutput->output()->pos().y();
    }
    qDebug() << Q_FUNC_INFO << "curOutput name" << curOutput->output()->name() << "pos" << curOutput->output()->pos() << curOutput->output()->size();
    qDebug() << Q_FUNC_INFO << "preOutput name" << preOutput->output()->name() << "pos" << preOutput->output()->pos() << preOutput->output()->size();
    qDebug() << Q_FUNC_INFO << "isUpDown" << isUpDown;
    return isUpDown;
}


bool QMLScreen::isUpDownOutput(QMLOutput *curOutput, QMLOutput *preOutput) {
    if (curOutput == nullptr || preOutput == nullptr) {
        return false;
    }
    float width = (std::max(curOutput->x() + curOutput->width(), preOutput->x() + preOutput->width()) -
                   std::min(curOutput->x(), preOutput->x()) -
                   fabs(curOutput->x() - preOutput->x()) -
                   fabs((curOutput->x() + curOutput->width()) - (preOutput->x() + preOutput->width())));
    float height = (std::max(curOutput->y() + curOutput->height(), preOutput->y() + preOutput->height()) -
                    std::min(curOutput->y(), preOutput->y()) -
                    fabs(curOutput->y() - preOutput->y()) -
                    fabs((curOutput->y() + curOutput->height()) - (preOutput->y() + preOutput->height())));
    qDebug() << Q_FUNC_INFO << curOutput->output()->name() << preOutput->output()->name() << "width:" << width << " height:" << height;
    if (width > height) {
        return true;
    }
    return false;
}

void QMLScreen::setCloneMode(bool isCloneMode) {
    m_isCloneMode = isCloneMode;
    qDebug() << Q_FUNC_INFO << "setCloneMode" << isCloneMode;
}

void QMLScreen::setRotation(bool isRotation) {
    m_isRotation = isRotation;
    qDebug() << Q_FUNC_INFO << "rotation" << isRotation;
}

void QMLScreen::setScreenPos(QMLOutput *output, bool isReleased)
{
    qDebug() << Q_FUNC_INFO << "setScreenPos isRelease" << isReleased << "isCloneMode" << m_isCloneMode;

    // 镜像模式下跳过屏幕旋转处理
    if (!output || output->isCloneMode() || isCloneMode()) {
        qDebug() << Q_FUNC_INFO << "isCloneMode setScreenCenterPos" << isCloneMode();
        setScreenCenterPos();
        return;
    }
    output->setSize(QSizeF(output->property("saveWidth").toDouble(), output->property("saveHeight").toDouble()));
    float x1 = 0, y1 = 0;
    float width1 = 0, height1 = 0;
    float x2 = 0, y2 = 0;
    float width2 = 0, height2 = 0;

    x1 = output->x();
    y1 = output->y();
    width1 = output->width();
    height1 = output->height();

    int connectedScreen = 0;
    int enabledConnectedScreen = 0;

    QMLOutput *other = NULL;
    Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
        if (qmlOutput->output()->isConnected()) {
            qDebug() << Q_FUNC_INFO << " start " << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size() << qmlOutput->outputX() << qmlOutput->outputY() << UtilsOfDisplay::sizeToString(qmlOutput->output()->size());
            connectedScreen++;
            if (qmlOutput->output()->isEnabled()) {
                enabledConnectedScreen++;
            }
        }
        if (qmlOutput != output && qmlOutput->output()->isConnected()) {
            qmlOutput->setSize(QSizeF(qmlOutput->property("saveWidth").toDouble(), qmlOutput->property("saveHeight").toDouble()));
            other = qmlOutput;
            x2 = other->x();
            y2 = other->y();
            width2 = other->width();
            height2 = other->height();
        }
    }

    // 预览被裁切时坐标可为负；若此处直接 return，会跳过吸附，竖排旋转后拖动间距会被放大且无法收回。
    // 先平移到非负坐标系，再走后续吸附逻辑，最后由 setScreenCenterPos 居中。
    {
        bool firstPos = true;
        qreal minPosX = 0.0;
        qreal minPosY = 0.0;
        Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
            if (!qmlOutput->output()->isConnected() || !qmlOutput->isVisible()) {
                continue;
            }
            if (firstPos) {
                minPosX = qmlOutput->x();
                minPosY = qmlOutput->y();
                firstPos = false;
            } else {
                minPosX = qMin(minPosX, qmlOutput->x());
                minPosY = qMin(minPosY, qmlOutput->y());
            }
        }
        const qreal shiftX = qMin(minPosX, 0.0);
        const qreal shiftY = qMin(minPosY, 0.0);
        if (!firstPos && (shiftX < 0 || shiftY < 0)) {
            Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
                if (!qmlOutput->output()->isConnected() || !qmlOutput->isVisible()) {
                    continue;
                }
                qmlOutput->blockSignals(true);
                qmlOutput->setX(qmlOutput->x() - shiftX);
                qmlOutput->setY(qmlOutput->y() - shiftY);
                qmlOutput->blockSignals(false);
            }
            x1 -= shiftX;
            y1 -= shiftY;
            x2 -= shiftX;
            y2 -= shiftY;
        }
    }

    if (connectedScreen < 2) {
        setScreenCenterPos();
        return;
    }

    QVector<QMLOutput *> qmlOutputVec;
    qmlOutputVec.reserve(m_outputMap.size());
    Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
        if (!qmlOutput->output()->isConnected()) {
            continue;
        }
        qmlOutputVec.append(qmlOutput);
    }
    // x point sort
    std::sort(qmlOutputVec.begin(), qmlOutputVec.end(),
              [&qmlOutputVec](const QMLOutput* lOutput, const QMLOutput* rOutput){
        if (lOutput->x() != rOutput->x()) {
            return lOutput->x() < rOutput->x();
        }
        if (lOutput->width() != rOutput->width()) {
            return lOutput->width() < rOutput->width();
        }
        if (lOutput->y() != rOutput->y()) {
            return lOutput->y() < rOutput->y();
        }
        if (lOutput->height() != rOutput->height()) {
            return lOutput->height() < rOutput->height();
        }
        return false;
    });

    bool outputVecSizeBeyondTwo = (qmlOutputVec.size() > 2);
    bool enabledOutputBeyondTwo = (enabledConnectedScreen > 2);
    if (outputVecSizeBeyondTwo) {
        qmlOutputVec.append(qmlOutputVec.at(0)); // convert list to a circle
    }

    QMLOutput* preQmlOutput = nullptr;
    QMLOutput* maxXQmlOutput = nullptr;
    qreal lastX = -1.0;
    Q_FOREACH (QMLOutput *qmlOutput, qmlOutputVec) {
        if (!qmlOutput->output()->isConnected() || !qmlOutput->output()->isEnabled()) {
            continue;
        }
        if (preQmlOutput == nullptr) {
            preQmlOutput = qmlOutput;
            continue;
        }
        bool swapOutputFlag = false;
        int outputVecLastIndex = (qmlOutputVec.size() - 1);
        bool dragOutputNotSwap = false;
        if (outputVecSizeBeyondTwo && (qmlOutput == qmlOutputVec.at(outputVecLastIndex))) {
            qDebug() << Q_FUNC_INFO << "last output" << qmlOutput->output()->name() << "drag output" << output->output()->name() << qmlOutputVec;
            if (qmlOutput->output()->name() != output->output()->name()) { // 如果最后一个屏幕不是拖动屏幕才需要交换，如果是拖动屏幕，可以再次正常顺序摆放，不会破坏其他显示器坐标
                QMLOutput* tmpOutput = qmlOutput;
                qmlOutput = preQmlOutput;
                preQmlOutput = tmpOutput;
                swapOutputFlag = true;
            } else {
                dragOutputNotSwap = true;
            }
        }

        if (!swapOutputFlag && !dragOutputNotSwap) {
            if (qmlOutput->x() + qmlOutput->width() < preQmlOutput->x()) {
                qmlOutput->setX(preQmlOutput->x() - qmlOutput->width());
            }
            if (qmlOutput->y() > preQmlOutput->y() + preQmlOutput->height()) {
                qmlOutput->setY(preQmlOutput->y() + preQmlOutput->height());
            }
            if (qmlOutput->x() > preQmlOutput->x() + preQmlOutput->width()) {
                qmlOutput->setX(preQmlOutput->x() + preQmlOutput->width());
            }
            if (qmlOutput->y() + qmlOutput->height() < preQmlOutput->y()) {
                qmlOutput->setY(preQmlOutput->y() - qmlOutput->height());
            }
        }
        // 矩形是否相交
        float maxX = float(std::max(qmlOutput->x(), preQmlOutput->x()));
        float minX = float(std::min(qmlOutput->x() + qmlOutput->width(), preQmlOutput->x() + preQmlOutput->width()));
        float maxY = float(std::max(qmlOutput->y(), preQmlOutput->y()));
        float minY = float(std::min(qmlOutput->y() + qmlOutput->height(), preQmlOutput->y() + preQmlOutput->height()));
        qDebug() << Q_FUNC_INFO << " before set position " << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size();
        qDebug() << Q_FUNC_INFO << " before set position " << preQmlOutput->output()->name() << preQmlOutput->position() << preQmlOutput->size();
        float newMaxX = QString::number(maxX, 'f', 3).toFloat();
        float newMinX = QString::number(minX, 'f', 3).toFloat();
        float newMaxY = QString::number(maxY, 'f', 3).toFloat();
        float newMinY = QString::number(minY, 'f', 3).toFloat();
        qDebug() << Q_FUNC_INFO << " maxX:" << maxX << " minX:" << minX << " maxY:" << maxY << " minY:" << minY;
        qDebug() << Q_FUNC_INFO << " newMaxX:" << newMaxX << " newMinX:" << newMinX << " newMaxY:" << newMaxY << " newMinY:" << newMinY;
        if (newMaxX < newMinX && newMaxY < newMinY) {
            if (swapOutputFlag) {
                qmlOutput->setX(preQmlOutput->x() + preQmlOutput->width());
            } else {
                if (!dragOutputNotSwap && (qmlOutput->output()->name() == output->output()->name() ||
                        preQmlOutput->output()->name() == output->output()->name())) {
                    qDebug() << Q_FUNC_INFO << qmlOutput->output()->name() << qmlOutput->output()->pos() << qmlOutput->output()->size() << preQmlOutput->output()->name() << preQmlOutput->output()->pos() << preQmlOutput->output()->size();
                    if ((m_isRotation && isActualUpDownOutput(qmlOutput, preQmlOutput)) ||
                            (!m_isRotation && isUpDownOutput(qmlOutput, preQmlOutput))) { // 当屏幕旋转90时，点击不保存，需跳过上下相交判断，除非当前实际坐标是上下结构。因为旋转90时，屏幕宽高已按高宽摆放了位置，此时不能按宽高来判断是否上下相交
                        setRotation(false);
                        qDebug() << Q_FUNC_INFO << "outputVecSizeBeyondTwo" << outputVecSizeBeyondTwo << "isRotation" << m_isRotation;
                        if (outputVecSizeBeyondTwo && !m_isRotation && (((qmlOutput->x() < preQmlOutput->x() + preQmlOutput->width()) &&
                                                   (qmlOutput->x() + qmlOutput->width() > preQmlOutput->x() + preQmlOutput->width())) ||
                                             ((preQmlOutput->x() < qmlOutput->x() + qmlOutput->width()) &&
                                                   (preQmlOutput->x() + preQmlOutput->width() > qmlOutput->x() + qmlOutput->width())))) {
                            // 大于2屏时，当拖拽屏包含另一个屏，或者另一个屏包含拖拽屏时，向右摆放
                            qmlOutput->setX(preQmlOutput->x() + preQmlOutput->width());
                            preQmlOutput = qmlOutput;
                            qDebug() << Q_FUNC_INFO << "contains after set position" << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size();
                            continue;
                        }
                        // up
                        if ((qmlOutput->y() + height() > preQmlOutput->y()) &&
                                (qmlOutput->y() < preQmlOutput->y())) {
                            qmlOutput->setY(preQmlOutput->y() - qmlOutput->height());
                            preQmlOutput = qmlOutput;
                            continue;
                        // down
                        } else if ((qmlOutput->y() < preQmlOutput->y()  + preQmlOutput->height()) &&
                                   (qmlOutput->y() + qmlOutput->height() > preQmlOutput->y() + preQmlOutput->height())) {
                            qmlOutput->setY(preQmlOutput->y() + preQmlOutput->height());
                            preQmlOutput = qmlOutput;
                            continue;
                        }
                    }
                }
                // left
                if ((qmlOutput->x() + qmlOutput->width() > preQmlOutput->x()) && (qmlOutput->x() < preQmlOutput->x())) {
                    if (outputVecSizeBeyondTwo && qmlOutput->output()->name() != output->output()->name()) {
                        // 大于2屏时，当前屏幕不是拖拽屏，优先靠右摆放，如果还放左侧，会和preQmlOutput的前一个屏幕重叠，摆放位置从左到右统一摆放的
                        qmlOutput->setX(preQmlOutput->x() + preQmlOutput->width());
                    } else {
                        qmlOutput->setX(preQmlOutput->x() - qmlOutput->width());
                    }
                    qDebug() << Q_FUNC_INFO << "left after set position" << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size();
                // contains
                } else if ((qmlOutput->x() < preQmlOutput->x() + preQmlOutput->width()) &&
                           (qmlOutput->x() + qmlOutput->width() > preQmlOutput->x() + preQmlOutput->width())) {
                    qmlOutput->setX(preQmlOutput->x() + preQmlOutput->width());
                // right
                } else if ((qmlOutput->x() + qmlOutput->width() > preQmlOutput->x()) && (qmlOutput->x() > preQmlOutput->x())) {
                    qmlOutput->setX(preQmlOutput->x() + preQmlOutput->width());
                // up
                } else if ((qmlOutput->y() + height() > preQmlOutput->y()) &&
                    (qmlOutput->y() < preQmlOutput->y())) {
                    qmlOutput->setY(preQmlOutput->y() - qmlOutput->height());
                // down
                } else if ((qmlOutput->y() < preQmlOutput->y()  + preQmlOutput->height()) &&
                           (qmlOutput->y() + qmlOutput->height() > preQmlOutput->y() + preQmlOutput->height())) {
                    qmlOutput->setY(preQmlOutput->y() + preQmlOutput->height());
                }
            }
        } else {
            if (enabledOutputBeyondTwo && newMaxX == newMinX && !isActualUpDownOutput(qmlOutput, preQmlOutput)
                    && (qmlOutput->x() + qmlOutput->width() == preQmlOutput->x())) { // 当前摆放的显示器需在preOutput右侧，否则需要重新摆放
                qmlOutput->setX(preQmlOutput->x() + preQmlOutput->width());
                qDebug() << Q_FUNC_INFO << "equal x after set position" << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size();
            }
        }
        preQmlOutput = qmlOutput;
        if (qmlOutput->x() + qmlOutput->width() > lastX) {
            lastX = qMax(lastX, qmlOutput->x() + qmlOutput->width());
            maxXQmlOutput = qmlOutput;
        }
    }

    if (outputVecSizeBeyondTwo) {
        qmlOutputVec.erase(qmlOutputVec.end() - 1); // erase repeat output
    }

    QMLOutput* nextQmlOutput = nullptr;
    int curOutputIdx = 0;
    int interval = 2;
    Q_FOREACH (QMLOutput *qmlOutput, qmlOutputVec) {
        if (!qmlOutput->output()->isConnected() || !qmlOutput->output()->isEnabled()) {
            continue;
        }
        qDebug() << Q_FUNC_INFO << " after set position" << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size();
        int nextOutputIdx = curOutputIdx + interval;
        curOutputIdx++;
        if (nextOutputIdx > qmlOutputVec.size() - 1) {
            if (qmlOutput->x() + qmlOutput->width() > lastX) {  // continue前需要更新maxXQmlOutput，否则enable=false显示器不会重新摆放
                lastX = qMax(lastX, qmlOutput->x() + qmlOutput->width());
                maxXQmlOutput = qmlOutput;
            }
            continue;
        }
        nextQmlOutput = qmlOutputVec.at(nextOutputIdx);
        if (!nextQmlOutput->output()->isConnected() || !nextQmlOutput->output()->isEnabled()) { // qmlOutputVec存在enable=false的显示器，在此之前都是按enable=true修改位置，此时如果拿enbale=false显示器去判断是否相交会有问题
            if (qmlOutput->x() + qmlOutput->width() > lastX) {
                lastX = qMax(lastX, qmlOutput->x() + qmlOutput->width());
                maxXQmlOutput = qmlOutput;
            }
            continue;
        }
        // compare with interval output
        if ((std::max(qmlOutput->x(), nextQmlOutput->x()) < std::min(qmlOutput->x() + qmlOutput->width(), nextQmlOutput->x() + nextQmlOutput->width())) &&
                std::max(qmlOutput->y(), nextQmlOutput->y()) < std::min(qmlOutput->y() + qmlOutput->height(), nextQmlOutput->y() + nextQmlOutput->height())) {
            qmlOutput->setY(nextQmlOutput->y() + nextQmlOutput->height());
            qDebug() << Q_FUNC_INFO << "interval output after setY" << qmlOutput->output()->name() << qmlOutput->position();
        }
        if (qmlOutput->x() + qmlOutput->width() > lastX) {
            lastX = qMax(lastX, qmlOutput->x() + qmlOutput->width());
            maxXQmlOutput = qmlOutput;
        }
    }
    if (maxXQmlOutput != nullptr) {
        bool hitDisableOutput = false;
        QMLOutput* preQmlOutput = nullptr;
        Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {    // disabled output靠着最右边摆放，后续disable output靠着前一个disable output横向摆放
            if (!qmlOutput->output()->isConnected() || qmlOutput->output()->isEnabled()) {
                continue;
            }
            if (!hitDisableOutput) {
                qDebug() << Q_FUNC_INFO << "first DisableOutput before set position" << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size();
                qmlOutput->setX(maxXQmlOutput->x() + maxXQmlOutput->width());
                if (qmlOutput->y() > maxXQmlOutput->y() + maxXQmlOutput->height()) {
                    qmlOutput->setY(maxXQmlOutput->y() + maxXQmlOutput->height());
                }
                if (qmlOutput->y() + qmlOutput->height() < maxXQmlOutput->y()) {
                    qmlOutput->setY(maxXQmlOutput->y() - qmlOutput->height());
                }
                preQmlOutput = qmlOutput;
                hitDisableOutput = true;
                qDebug() << Q_FUNC_INFO << "first DisableOutput after set position" << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size();
            } else {
                if (preQmlOutput) {
                    qDebug() << Q_FUNC_INFO << "DisableOutput before set position" << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size();
                    qmlOutput->setX(preQmlOutput->x() + preQmlOutput->width());
                    qmlOutput->setY(preQmlOutput->y());
                    qDebug() << Q_FUNC_INFO << "DisableOutput after set position" << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size();
                }
                preQmlOutput = qmlOutput;
            }
        }
    }

    setScreenCenterPos();
    rightPosList.clear();
    Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
        if (qmlOutput->output()->isConnected()) {
            rightPosList.append(qmlOutput->position().x());
            rightPosList.append(qmlOutput->position().y());
        }
    }
    if (isReleased) {
        const bool isEnableOutputRelease = output && output->output()->isEnabled()
                && !m_enableOutput.isEmpty() && output->output()->name() == m_enableOutput;
        if (isEnableOutputRelease) {
            qDebug() << Q_FUNC_INFO << "enable output release emit directly" << m_enableOutput;
            Q_EMIT released();
            return;
        }
        if (leftPosList.size() != rightPosList.size()) {
            Q_EMIT released();
        } else {
            for (int i = 0; i < leftPosList.size(); ++i) {
                if (qFabs(leftPosList.at(i) - rightPosList.at(i)) > 0.001) {
                    Q_EMIT released();
                    break;
                }
            }
        }
    }
}

void QMLScreen::setActiveOutputByCombox(int screenId)
{
    QHash<KScreen::OutputPtr, QMLOutput *>::const_iterator it = m_outputMap.constBegin();
    while (it != m_outputMap.constEnd()) {
        if (screenId == it.key()->id()) {
            setActiveOutput(it.value());
            return;
        }
        it++;
    }
}

QSize QMLScreen::maxScreenSize() const
{
    return m_config->screen()->maxSize();
}

float QMLScreen::outputScale() const
{
    return m_outputScale;
}

void QMLScreen::outputConnectedChanged()
{
    int connectedCount = 0;

    Q_FOREACH (const KScreen::OutputPtr &output, m_outputMap.keys()) {
        if (output->isConnected()) {
            ++connectedCount;
        }
    }

    if (connectedCount != m_connectedOutputsCount) {
        m_connectedOutputsCount = connectedCount;
        Q_EMIT connectedOutputsCountChanged();
        // bug#180957
        //updateOutputsPlacement();
    }
}

void QMLScreen::setOutputEnabled() {
    int enabledCount = 0;
    int connectCount = 0;
    Q_FOREACH (const KScreen::OutputPtr &output, m_outputMap.keys()) {
        if (output->isConnected()) {
            ++connectCount;
        }
        if (output->isEnabled()) {
            ++enabledCount;
        }
    }
    bool isDragEnable = ((connectCount == enabledCount) || enabledCount > 1);
    qDebug() << Q_FUNC_INFO << "isDragEnable" << isDragEnable << "connectCount" << connectCount << "enabledCount" << enabledCount;
    Q_FOREACH (QMLOutput *m_qmlOutput, m_outputMap) {
        if (m_qmlOutput && m_qmlOutput->output()->isConnected()) {
            m_qmlOutput->setDragEnable(isDragEnable);
        }
    }
}

void QMLScreen::outputEnabledChanged()
{
    const KScreen::OutputPtr output(qobject_cast<KScreen::Output *>(sender()), [](void *){
        });
    /*if (output->isEnabled()) {   // bug#206036
        // bug#68442
        QTimer::singleShot(500, this,[=]{
            updateOutputsPlacement();
        });
    }*/

    int enabledCount = 0;
    Q_FOREACH (const KScreen::OutputPtr &output, m_outputMap.keys()) {
        if (output->isEnabled()) {
            ++enabledCount;
        }
    }
    if (!output.isNull()) {
        m_enableOutput = output->name();
        qDebug() << Q_FUNC_INFO << "outputEnabledChanged" << output->name();
    }
    setOutputEnabled();
    if (enabledCount == m_enabledOutputsCount) {
        m_enabledOutputsCount = enabledCount;
        Q_EMIT enabledOutputsCountChanged();
    }
}

void QMLScreen::setCloseOutput(bool status) {
    m_closeOutput = status;
    setOutputEnabled();
    qDebug() << Q_FUNC_INFO << "set close output" << m_closeOutput;
}

void QMLScreen::setExtendMode(bool status) {
    m_extendMode = status;
    qDebug() << Q_FUNC_INFO << "set extend mode" << m_extendMode;
}

void QMLScreen::setIsPosChanged(bool status) {
    m_isPosChanged = status;
    qDebug() << Q_FUNC_INFO << "set is pos changed" << m_isPosChanged;
}

void QMLScreen::outputPositionChanged()
{
    KScreen::Output *output = qobject_cast<KScreen::Output *>(sender());
    if (output == nullptr) {
         qDebug() << Q_FUNC_INFO << " output is null";
         return;
    }
    static const int OUTPUT_POS_CHANGE_COUNT = 2;
    if (m_closeOutput) {
        qDebug() << Q_FUNC_INFO << "close output no need change output position" << output->name();
        ++m_closeOutputCount;
        if (m_closeOutputCount == OUTPUT_POS_CHANGE_COUNT) {
            m_closeOutputCount = 0;
            m_closeOutput = false;
        }
        return;
    }
    bool isClicked = true;
    bool resetClicked = false;
    for (QMLOutput *mOutput: outputs()) {
        if (mOutput->output()->name() == output->name()) {
            isClicked = mOutput->isClicked();
            if (!isClicked) {
                break;
            }
            ++m_outputPosChangedCount;
            if (m_outputPosChangedCount == OUTPUT_POS_CHANGE_COUNT) {
                m_outputPosChangedCount = 0;
                resetClicked = true;
            }
            qDebug() << Q_FUNC_INFO << mOutput->output()->name() << " isClicked " << mOutput->isClicked() << m_outputPosChangedCount;
            break;
        }
    }
    if (resetClicked) {
        for (QMLOutput *mOutput: outputs()) {
            if (mOutput->output()->isConnected()) {
                mOutput->setIsClicked(false);
                qDebug() << Q_FUNC_INFO << mOutput->output()->name() << " resetClicked " << mOutput->isClicked();
            }
        }
    }
    if (isClicked) {
        qDebug() << Q_FUNC_INFO << " output isClicked, skip";
        return;
    }

    m_isPosChanged = true;
    qDebug() << Q_FUNC_INFO << isClicked << resetClicked;
    updateOutputsPlacement(isClicked);
    m_isPosChanged = false;
}

void QMLScreen::qmlOutputMoved(QMLOutput *qmlOutput)
{
    qDebug() << Q_FUNC_INFO << "qmlOutputMoved output" << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size();
    if (qmlOutput->isCloneMode()) {
        qDebug() << Q_FUNC_INFO << "output is cloneMode, return" << qmlOutput->output()->name();
        return;
    }

    if (qFabs(qmlOutput->property("saveWidth").toDouble() - qmlOutput->width()) > 0.001 || qFabs(qmlOutput->property("saveHeight").toDouble() - qmlOutput->height()) > 0.001) {
        qDebug() << Q_FUNC_INFO << "output no need change, return" << qmlOutput->output()->name();
        return;
    }

    if (!m_manuallyMovedOutputs.contains(qmlOutput)) {
        m_manuallyMovedOutputs.append(qmlOutput);
        qDebug() << Q_FUNC_INFO << "qmlOutputMoved m_manuallyMovedOutputs add" << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size() << m_manuallyMovedOutputs.size();
    }

    updateCornerOutputs();

    Q_FOREACH (QMLOutput *m_qmlOutput, m_outputMap) {
        if (!m_qmlOutput->output()->isConnected() || !m_qmlOutput->output()->isEnabled()) {
            continue;
        }
        qDebug() << Q_FUNC_INFO << "output info" << m_qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size() << UtilsOfDisplay::sizeToString(qmlOutput->output()->size());
        float newOutputScale = round(outputScale() * 10000.0f) / 10000.0f;
        int x = qRound(ceil((m_qmlOutput->x() - leftX) / newOutputScale));
        int y = qRound(ceil((m_qmlOutput->y() - topY) / newOutputScale));
        m_qmlOutput->outputPtr()->blockSignals(true);
        qDebug() << Q_FUNC_INFO << "before setOutputX setOutputY" << m_qmlOutput->output()->name() << qmlOutput->position() << m_qmlOutput->outputX() << m_qmlOutput->outputY() << newOutputScale;
        m_qmlOutput->setOutputX(x);
        m_qmlOutput->setOutputY(y);
        qDebug() << Q_FUNC_INFO << "after setOutputX setOutputY" << m_qmlOutput->output()->name() << qmlOutput->position() << m_qmlOutput->outputX() << m_qmlOutput->outputY();
        m_qmlOutput->outputPtr()->blockSignals(false);
    }
}

void QMLScreen::viewSizeChanged()
{
    //TO fix bug#85240
//    updateOutputsPlacement();
    qDebug() << Q_FUNC_INFO << "viewSizeChanged before setScreenCenterPos";
    QTimer::singleShot(0,this,[=]{
        setScreenCenterPos();
    });
}

void QMLScreen::updateCornerOutputs()
{
    // Preview items may be clipped (x/y < 0); do not use -1 as "unset".
    bool first = true;
    leftX = 0.0;
    topY = 0.0;
    Q_FOREACH (QMLOutput *output, m_outputMap) {
        if (!output->output()->isConnected() || !output->output()->isEnabled()) {
            continue;
        }

        if (first || leftX > output->x()) {
            leftX = output->x();
        }

        if (first || topY > output->y()) {
            topY = output->y();
        }
        first = false;
        qDebug() << Q_FUNC_INFO << "updateCornerOutputs leftX" << leftX << "topY" << topY << "output" << output->output()->name() << output->position();
    }
}

void QMLScreen::setOutputScale(float scale)
{
    if (qFuzzyCompare(scale, m_outputScale))
        return;
    m_outputScale = scale;
    emit outputScaleChanged();
}

bool QMLScreen::isCloneMode()
{
    KScreen::OutputPtr output = m_config->primaryOutput();
    if (!output) {
        return false;
    }
    qDebug() << Q_FUNC_INFO << "primary output" << output->name() << output->pos();
    if (m_config->connectedOutputs().count() >= 2) {
        foreach (KScreen::OutputPtr secOutput, m_config->connectedOutputs()) {
            //不能用size为(-1,-1)进行判断，刚插拔时分辨率可能会是(-1,-1)
            if (secOutput->pos() != output->pos() || !secOutput->isEnabled()) {
                qDebug() << "isCloneMode output pos or isEnabled false" << secOutput->name() << "pos" << secOutput->pos() << "primary output pos" << output->pos() << "isEnabled" << secOutput->isEnabled();
                return false;
            }
        }
    } else {
        return false;
    }
    Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
        if (!qmlOutput->output()->isConnected()) {
            continue;
        }
        if (qmlOutput->output()->name() != output->name()) {
            qmlOutput->setVisible(false);
            qmlOutput->setIsCloneMode(true, false);
            qDebug() << Q_FUNC_INFO << "isCloneMode set qmlOutput invisible" << qmlOutput->output()->name() << qmlOutput->isVisible();
        } else {
            qmlOutput->setVisible(true);
            qmlOutput->setIsCloneMode(true, true);
            qDebug() << Q_FUNC_INFO << "isCloneMode set primary qmlOutput visible" << qmlOutput->output()->name() << qmlOutput->isVisible();
        }
    }
    qDebug() << "isCloneMode return true";
    return true;
}

// 画坐标
void QMLScreen::updateOutputsPlacement(bool isClicked)
{
    qDebug() << Q_FUNC_INFO << isCloneMode() << isClicked << "extendMode" << m_extendMode << "isScreenInit" << m_isScreenInit;
    if (width() <= 0 || (!m_extendMode && isCloneMode()))
        return;

    QSizeF initialActiveScreenSize;

    Q_FOREACH (QQuickItem *item, childItems()) {
        QMLOutput *qmlOutput = qobject_cast<QMLOutput *>(item);
        if (!qmlOutput->output()->isConnected() || !qmlOutput->output()->isEnabled()) {
            continue;
        }

        if (qmlOutput->outputX() + qmlOutput->currentOutputWidth()
            > initialActiveScreenSize.width()) {
            initialActiveScreenSize.setWidth(qmlOutput->outputX()
                                             + qmlOutput->currentOutputWidth());
        }
        if (qmlOutput->outputY() + qmlOutput->currentOutputHeight()
            > initialActiveScreenSize.height()) {
            initialActiveScreenSize.setHeight(
                qmlOutput->outputY() + qmlOutput->currentOutputHeight());
        }
    }

    auto initialScale = outputScale();

    auto scale = initialScale;
    qreal lastX = -1.0;
    do {
        auto activeScreenSize = initialActiveScreenSize * scale;

        const QPointF offset((width() - activeScreenSize.width()) / 2.0,
                             (height() - activeScreenSize.height()) / 2.0);

        lastX = -1.0;
        qreal lastY = -1.0;
        QVector<QMLOutput *> qmlOutputVec;
        qmlOutputVec.reserve(m_outputMap.size());
        int isEnabledOutput = 0;
        Q_FOREACH (QQuickItem *item, childItems()) {
            QMLOutput *qmlOutput = qobject_cast<QMLOutput *>(item);
            if (!qmlOutput->output()->isConnected()) {
                continue;
            }
            if (!qmlOutput->output()->isEnabled()) {
                ++isEnabledOutput;
            }
            qmlOutputVec.append(qmlOutput);
        }
        // x point sort
        if (!m_isScreenInit) {
            std::sort(qmlOutputVec.begin(), qmlOutputVec.end(),
                      [&qmlOutputVec](const QMLOutput* lOutput, const QMLOutput* rOutput){
                if (lOutput->outputX() != rOutput->outputX()) {
                    return lOutput->outputX() < rOutput->outputX();
                }
                if (lOutput->width() != rOutput->width()) {
                    return lOutput->width() < rOutput->width();
                }
                if (lOutput->outputY() != rOutput->outputY()) {
                    return lOutput->outputY() < rOutput->outputY();
                }
                if (lOutput->height() != rOutput->height()) {
                    return lOutput->height() < rOutput->height();
                }
                return false;
            });
        }
        bool enableOutputToRight = false;
        qDebug() << Q_FUNC_INFO << "closeOutput" << m_closeOutput << "enableOutput" << m_enableOutput;
        Q_FOREACH (QMLOutput *qmlOutput, qmlOutputVec) {
            if (!qmlOutput->output()->isConnected() || !qmlOutput->output()->isEnabled()
                    || (isClicked && m_manuallyMovedOutputs.contains(qmlOutput))) {
                qDebug() << Q_FUNC_INFO << " continue " << qmlOutput->output()->name() << qmlOutput->position() << !qmlOutput->output()->isConnected()
                         << !qmlOutput->output()->isEnabled() << m_manuallyMovedOutputs.contains(qmlOutput) << isClicked;
                continue;
            }

            if (m_enableOutput == qmlOutput->output()->name()) { // 打开某个显示器需摆放到最右边，同关闭显示器逻辑一致
                qDebug() << Q_FUNC_INFO << "click open output button should place right" << m_enableOutput;
                enableOutputToRight = true;
                continue;
            }

            qmlOutput->blockSignals(true);
            qDebug() << Q_FUNC_INFO << "before set position" << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size()
                     << isClicked << m_manuallyMovedOutputs.contains(qmlOutput);
            qmlOutput->setPosition(QPointF(offset.x() + (qmlOutput->outputX() * scale),
                                           offset.y() + (qmlOutput->outputY() * scale)));
            qDebug() << Q_FUNC_INFO <<"after set position " << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size();

            lastX = qMax(lastX, qmlOutput->position().x() + qmlOutput->width() / initialScale * scale);
            lastY = qMax(lastY, qmlOutput->position().y());
            qDebug() << Q_FUNC_INFO << "lastX" << lastX << "lastY" << lastY;
            qmlOutput->blockSignals(false);
        }
        qDebug() << Q_FUNC_INFO << "isposChanged:" << m_isPosChanged << "m_manuallyMovedOutputs.size:" << m_manuallyMovedOutputs.size();
        if (!m_isPosChanged || isEnabledOutput > 0 || enableOutputToRight) {
            Q_FOREACH (QQuickItem *item, childItems()) {
                QMLOutput *qmlOutput = qobject_cast<QMLOutput *>(item);
                if ((qmlOutput->output()->isConnected() && !qmlOutput->output()->isEnabled()) ||
                        (enableOutputToRight && qmlOutput->output()->isConnected() && m_enableOutput == qmlOutput->output()->name())) {
                    qmlOutput->blockSignals(true);
                    qDebug() << Q_FUNC_INFO << "not in manuallyMovedOutputs, before set position" << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size();
                    qmlOutput->setPosition(QPointF(lastX, lastY));
                    qDebug() << Q_FUNC_INFO << "not in manuallyMovedOutputs, after set position" << qmlOutput->output()->name() << qmlOutput->position() << qmlOutput->size();
                    lastX += qmlOutput->width() / initialScale * scale;
                    qmlOutput->blockSignals(false);
                }
            }
            if (enableOutputToRight) {
                m_enableOutput.clear();
            }
        }
        // calculate the scale dynamically, so all screens fit to the dialog
        if (lastX > width()) {
            scale *= 0.8;
        }
    } while (lastX > width());

    // Use a timer to avoid binding loop on width()
    QTimer::singleShot(0, this, [scale, this] {
        setOutputScale(scale);
    });
}
