/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#ifndef KYWC_OUTPUT_MANAGER_H
#define KYWC_OUTPUT_MANAGER_H

#include <QObject>
#include <QList>
#include <KWayland/Client/event_queue.h>

struct kywc_output_v1;
struct kywc_output_mode_v1;
struct kywc_output_manager_v1;
using namespace KWayland::Client;

namespace UkuiQuick::WaylandClient {
namespace Test {
class KywcOutputManagerTest;
}

class KywcOutput;

class KywcOutputMode : public QObject
{
    Q_OBJECT
public:
    ~KywcOutputMode() override;
    void setup(kywc_output_mode_v1 *mode);

    int id() const;
    QSize size() const;
    int refreshRate() const;
    bool isPreferred() const;

Q_SIGNALS:
    void removed();
private:
    friend class KywcOutput;
    friend class Test::KywcOutputManagerTest;
    explicit KywcOutputMode(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

//  KywcOutput class

class KywcOutput : public QObject
{
    Q_OBJECT
public:
    enum class StateMask : quint16
    {
        None = 0,
        Enabled = 1u << 0,
        CurrentMode = 1u << 1,
        Position = 1u << 2,
        Transform = 1u << 3,
        Scale = 1u << 4,
        Power = 1u << 5,
        Brightness = 1u << 6,
        ColorTemp = 1u << 7,
    };
    Q_ENUM(StateMask)
    Q_DECLARE_FLAGS(StateMasks, StateMask)

    ~KywcOutput() override;
    void setup(kywc_output_v1 *output);
    void setEventQueue(EventQueue *queue);

    QString name() const;
    QString make() const;
    QString model() const;
    QString serialNumber() const;
    QString description() const;
    QSize physicalSize() const;
    uint32_t capabilities() const;
    bool isEnabled() const;
    QPoint position() const;
    int transform() const;
    qreal scale() const;
    uint32_t power() const;
    uint32_t brightness() const;
    uint32_t colorTemp() const;
    QString uuid() const;
    QList<KywcOutputMode *> modes() const;
    KywcOutputMode *currentMode() const;

Q_SIGNALS:
    void modeRemoved(int id);
    void modeChanged(const KywcOutputMode *mode);
    void modeAdded(const KywcOutputMode *mode);
    void stateUpdated(StateMasks masks);
    void removed();
private:
    friend class KywcOutputManager;
    friend class KywcOutputMode;
    friend class Test::KywcOutputManagerTest;
    explicit KywcOutput(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KywcOutput::StateMasks)

//  KywcOutputManager class
class KywcOutputManager : public QObject
{
    Q_OBJECT
public:
    explicit KywcOutputManager(QObject *parent = nullptr);
    ~KywcOutputManager() override;

    bool isValid() const;
    void destroy();
    void setup(kywc_output_manager_v1 *manager);
    void setEventQueue(EventQueue *queue);
    EventQueue *eventQueue() const;

    void stop();
    bool isInitialized() const;
    QList<KywcOutput *> outputs() const;
    KywcOutput *primaryOutput() const;

Q_SIGNALS:
    void outputCreated(const KywcOutput *output);
    void outputRemoved(QString uuid);
    void primaryOutputChanged(const KywcOutput *output);
    void removed();
    void initialized();
    void done();

private:
    friend class KywcOutput;
    friend class Test::KywcOutputManagerTest;

    enum class State {
        Unbound,
        Bound,
        Stopping,
        Invalidated
    };

    class Private;
    QScopedPointer<Private> d;
};
}

#endif //KYWC_OUTPUT_MANAGER_H
