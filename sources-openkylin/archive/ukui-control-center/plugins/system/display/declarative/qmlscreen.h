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

#ifndef QMLSCREEN_H
#define QMLSCREEN_H

#include <QQuickItem>

#include <KF5/KScreen/kscreen/output.h>
#include "qmloutput.h"

class QQmlEngine;

namespace KScreen {
class Output;
class Config;
}

class QMLScreen : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QSize maxScreenSize
               READ maxScreenSize
               CONSTANT)

    Q_PROPERTY(int connectedOutputsCount
               READ connectedOutputsCount
               NOTIFY connectedOutputsCountChanged)

    Q_PROPERTY(int enabledOutputsCount
               READ enabledOutputsCount
               NOTIFY enabledOutputsCountChanged)

    Q_PROPERTY(float outputScale
               READ outputScale
               NOTIFY outputScaleChanged)

public:
    explicit QMLScreen(QQuickItem *parent = nullptr);
    ~QMLScreen() override;

    int connectedOutputsCount() const;
    int enabledOutputsCount() const;

    QMLOutput *primaryOutput() const;
    QList<QMLOutput*> outputs() const;

    QSize maxScreenSize() const;

    float outputScale() const;

    KScreen::ConfigPtr config() const;
    void setConfig(const KScreen::ConfigPtr &config);

    void updateOutputsPlacement(bool isClicked = false);

    void setActiveOutput(QMLOutput *output);

    void setScreenPos(QMLOutput *output, bool isReleased);

    bool isUpDownOutput(QMLOutput *curOutput, QMLOutput *preOutput);

    bool isActualUpDownOutput(QMLOutput *curOutput, QMLOutput *preOutput);

    void setScreenCenterPos();

    void setCloseOutput(bool status);

    void setIsPosChanged(bool status);

    void setExtendMode(bool status);

    void removeOutput(int outputId);

    void qmlOutputMoved(QMLOutput *qmlOutput);

    void setRotation(bool isRotation = false);

    void setCloneMode(bool isCloneMode = false);

public Q_SLOTS:
    void setActiveOutput()
    {
        setActiveOutput(qobject_cast<QMLOutput *>(sender()));
    }

    void setActiveOutputByCombox(int screenId);
    void setScreenPos(bool isReleased)
    {
        setScreenPos(qobject_cast<QMLOutput*>(sender()), isReleased);
    }

Q_SIGNALS:
    void connectedOutputsCountChanged();
    void enabledOutputsCountChanged();

    void outputScaleChanged();

    void focusedOutputChanged(QMLOutput *output);

    void released();

private Q_SLOTS:
    void addOutput(const KScreen::OutputPtr &output);

    void outputConnectedChanged();
    void outputEnabledChanged();
    void outputPositionChanged();

    void viewSizeChanged();
    void getClickedPos();

private:
    void updateCornerOutputs();
    void setOutputScale(float scale);

    bool isCloneMode();
    bool isDarkMode();
    void visibleChangedSlot();
    void setOutputEnabled();

    KScreen::ConfigPtr m_config;
    QHash<KScreen::OutputPtr, QMLOutput *> m_outputMap;
    QVector<QMLOutput*> m_manuallyMovedOutputs;
    int m_connectedOutputsCount = 0;
    int m_enabledOutputsCount = 0;

    float m_outputScale = 1.0 / 14.0;// 缩放比例

    qreal leftX = 0.0;
    qreal topY  = 0.0;
    QList<qreal> leftPosList;
    QList<qreal> rightPosList;
    int m_outputPosChangedCount = 0;
    int m_closeOutputCount = 0;

    QList<KScreen::OutputPtr> m_pendingOutputs;
    bool m_closeOutput = false;
    bool m_isPosChanged = false;
    bool m_extendMode = false;
    bool m_isScreenInit = false;
    QString m_enableOutput;
    bool m_isRotation = false;
    bool m_isCloneMode = false;
};

#endif // QMLSCREEN_H
