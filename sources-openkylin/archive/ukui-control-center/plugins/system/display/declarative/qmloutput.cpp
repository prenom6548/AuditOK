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

#include "qmloutput.h"
#include "qmlscreen.h"

#include <KF5/KScreen/kscreen/output.h>

#include <QStandardItem>
#include <QStandardItemModel>
#include <qquickitem.h>
#include <QRect>

const static int sMargin = 0;
const static int sSnapArea = 20;
const static int sSnapAlignArea = 6;

Q_DECLARE_METATYPE(KScreen::ModePtr)

bool operator>(const QSize &sizeA, const QSize &sizeB)
{
    return ((sizeA.width() > sizeB.width()) && (sizeA.height() > sizeB.height()));
}

QMLOutput::QMLOutput(QQuickItem *parent):
    QQuickItem(parent),
    m_screen(nullptr),
    m_cloneOf(nullptr),
    m_isCloneMode(false),
    m_isDarkMode(false),
    m_allowResetSize(true),
    m_dragEnable(true),
    m_isClicked(false),
    m_isSelected(false) {
}

KScreen::Output* QMLOutput::output() const
{
    return m_output.data();
}

KScreen::OutputPtr QMLOutput::outputPtr() const
{
    return m_output;
}

void QMLOutput::setOutputPtr(const KScreen::OutputPtr &output)
{
    Q_ASSERT(m_output.isNull());

    m_output = output;
    Q_EMIT outputChanged();

    connect(m_output.data(), &KScreen::Output::rotationChanged,
            this, &QMLOutput::updateRootProperties);
    connect(m_output.data(), &KScreen::Output::currentModeIdChanged,
            this, &QMLOutput::currentModeIdChanged);
    connect(m_output.data(), &KScreen::Output::scaleChanged,
            this, &QMLOutput::currentModeIdChanged);
            
}

bool QMLOutput::isClicked()
{
    return m_isClicked;
}

void QMLOutput::setIsClicked(bool isClicked)
{
    qDebug() << Q_FUNC_INFO << " set clicked " << isClicked;
    m_isClicked = isClicked;
    emit isClickedChanged();
}

bool QMLOutput::isSelected() const
{
    return m_isSelected;
}

void QMLOutput::setIsSelected(bool selected)
{
    if (m_isSelected == selected) {
        return;
    }
    m_isSelected = selected;
    Q_EMIT isSelectedChanged();
}

QMLScreen *QMLOutput::screen() const
{
    return m_screen;
}

void QMLOutput::setScreen(QMLScreen *screen)
{
    Q_ASSERT(m_screen == nullptr);

    m_screen = screen;
    Q_EMIT screenChanged();
}

void QMLOutput::setCloneOf(QMLOutput* other)
{
    if (m_cloneOf == other) {
        return;
    }

    m_cloneOf = other;
    Q_EMIT cloneOfChanged();
}

QMLOutput* QMLOutput::cloneOf() const
{
    return m_cloneOf;
}

int QMLOutput::currentOutputHeight() const
{
    if (!m_output) {
        return 0;
    }

    KScreen::ModePtr mode = m_output->currentMode();
    if (!mode) {
        if (m_output->isConnected()) {
            mode = bestMode();
            if (!mode) {
                return 1000;
            }
            m_output->setCurrentModeId(mode->id());
        } else {
            return 1000;
        }
    }
    return mode->size().height() / m_output->scale();
}

int QMLOutput::currentOutputWidth() const
{
    if (!m_output) {
        return 0;
    }

    KScreen::ModePtr mode = m_output->currentMode();
    if (!mode) {
        if (m_output->isConnected()) {
            mode = bestMode();
            if (!mode) {
                return 1000;
            }
            m_output->setCurrentModeId(mode->id());
        } else {
            return 1000;
        }
    }
    return mode->size().width() / m_output->scale();
}

void QMLOutput::currentModeIdChanged()
{
    if (!m_output) {
        return;
    }

    Q_EMIT currentOutputSizeChanged();
}


int QMLOutput::outputX() const
{
    return m_output->pos().x();
}

void QMLOutput::setOutputX(int x)
{
    if (m_output->pos().rx() == x) {
        return;
    }

    QPoint pos = m_output->pos();
    pos.setX(x);
    m_output->setPos(pos);
    Q_EMIT outputXChanged();
}

int QMLOutput::outputY() const
{
    return m_output->pos().y();
}

void QMLOutput::setOutputY(int y)
{
    if (m_output->pos().ry() == y) {
        return;
    }

    QPoint pos = m_output->pos();
    pos.setY(y);
    m_output->setPos(pos);
    Q_EMIT outputYChanged();
}

void QMLOutput::setAllowResetSize(bool t_allowResetSize)
{
    if (t_allowResetSize != m_allowResetSize) {
        m_allowResetSize = t_allowResetSize;
        Q_EMIT allowResetSizeChanged();
    }
}

bool QMLOutput::dragEnable() const
{
    return m_dragEnable;
}

void QMLOutput::setDragEnable(bool dragStatus)
{
    m_dragEnable = dragStatus;
    qDebug() << Q_FUNC_INFO << "setDragEnable" << dragStatus;
    Q_EMIT dragEnableChanged();
}

bool QMLOutput::allowResetSize() const
{
    return m_allowResetSize;
}

bool QMLOutput::isCloneModeShow() const
{
    return m_cloneModeShow;
}

bool QMLOutput::isCloneMode() const
{
    return m_isCloneMode;
}

void QMLOutput::setIsCloneMode(bool isCloneMode, bool cloneModeShow)
{
    m_cloneModeShow = cloneModeShow;

    m_isCloneMode = isCloneMode;
    Q_EMIT isCloneModeChanged();
}

bool QMLOutput::isDarkMode() const
{
    return m_isDarkMode;
}

void QMLOutput::setIsDarkMode(bool isDarkMode)
{
    m_isDarkMode = isDarkMode;
    Q_EMIT isDarkModeChanged();
}

QString QMLOutput::friendlyOutputName()
{
    return UtilsOfDisplay::outputName(outputPtr());
}

KScreen::ModePtr QMLOutput::bestMode() const
{
    if (!m_output) {
        return KScreen::ModePtr();
    }

    KScreen::ModeList modes = m_output->modes();
    KScreen::ModePtr bestMode;
    Q_FOREACH (const KScreen::ModePtr &mode, modes) {
        if (!bestMode || (mode->size() > bestMode->size())) {
            bestMode = mode;
        }
    }

    return bestMode;
}

bool QMLOutput::collidesWithOutput(QObject *other)
{
    QQuickItem* otherItem = qobject_cast<QQuickItem*>(other);
    return boundingRect().intersects(otherItem->boundingRect());
}

/* Transformation of an item (rotation of the MouseArea) is only visual.
 * The coordinates and dimensions are still the same (when you rotated
 * 100x500 rectangle by 90 deg, it will still be 100x500, although
 * visually it will be 500x100).
 *
 * This method calculates the real-visual coordinates and dimensions of
 * the MouseArea and updates root item to match them. This makes snapping
 * work correctly regardless off visual rotation of the output
 */

//旋转时计算坐标更改方向
void QMLOutput::updateRootProperties()
{
    //qDebug()<<"updateRootProperties----->"<<endl;
    const float transformedWidth = (m_output->isHorizontal() ? currentOutputWidth() : currentOutputHeight()) * m_screen->outputScale();
    const float transformedHeight = (m_output->isHorizontal() ? currentOutputHeight() : currentOutputWidth()) * m_screen->outputScale();

    const float transformedX = x() + (width() / 2.0) - (transformedWidth / 2.0);
    const float transformedY = y() + (height() / 2.0) - (transformedHeight / 2.0);

    //qDebug()<<"transformedWidth: "<<transformedWidth<<"transformedHeight: "<<transformedHeight<<endl;

//    setPosition(QPointF(transformedX, transformedY));  //why ?
    setSize(QSizeF(transformedWidth, transformedHeight));
}
