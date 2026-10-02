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
 * Authors: sundagao <sundagao@kylinos.cn>
 */

#ifndef UKUI_QUICK_OUTPUT_MANAGER_H
#define UKUI_QUICK_OUTPUT_MANAGER_H

#include <QObject>
#include <QList>
#include <QPoint>
#include <QSize>
#include <QString>

#include <cstdint>
namespace UkuiQuick
{
class OutputMode;
class Output;
class OutputManager : public QObject
{
    Q_OBJECT
public:
    explicit OutputManager(QObject *parent = nullptr);
    ~OutputManager() override;

    void stop() const;
    [[nodiscard]] bool isAvailable() const;
    [[nodiscard]] bool isInitialized() const;
    // 对象所有权在OutputManager中，最好使用QPointer缓存，
    // 收到outputRemoved() 或者 availableChanged(false) 信号后，停止使用output对象。
    [[nodiscard]] QList<Output*> outputs() const;
    [[nodiscard]] Output* primaryOutput() const;

Q_SIGNALS:
    void outputAdded(Output *output);
    void outputRemoved(const QString &uuid);
    void primaryOutputChanged(Output *output);
    void initialized();
    void availableChanged(bool available);
    void configurationUpdated();

private:
    class Private;
    Private* d;
};

class OutputMode : public QObject
{
    Q_OBJECT
    friend class Output;
public:
    ~OutputMode() override;

    [[nodiscard]] int id() const;
    [[nodiscard]] QSize size() const;
    [[nodiscard]] int refreshRate() const;
    [[nodiscard]] bool preferred() const;
private:
    explicit OutputMode(QObject *parent = nullptr);

    class Private;
    Private* d;
};

class Output : public QObject
{
    Q_OBJECT
    friend class OutputManager;
public:
    enum class Capability {
        Power = 1 << 0,      // power(dpms)
        Brightness = 1 << 1, // 亮度调节
        ColorTemperature = 1 << 2, // 色温调节
    };
    Q_ENUM(Capability)
    Q_DECLARE_FLAGS(Capabilities, Capability)

    enum class Transform : qint8 {
        Normal = 0,
        Rotate90 = 1,
        Rotate180 = 2,
        Rotate270 = 3,
        Flipped = 4,
        Flipped90 = 5,
        Flipped180 = 6,
        Flipped270 = 7,
    };
    Q_ENUM(Transform)

    ~Output() override;

    [[nodiscard]] QString uuid() const;
    [[nodiscard]] QString name() const;
    [[nodiscard]] QString make() const;
    [[nodiscard]] QString model() const;
    [[nodiscard]] QString serialNumber() const;
    [[nodiscard]] QString description() const;
    [[nodiscard]] QSize physicalSize() const;
    [[nodiscard]] QPoint position() const;
    [[nodiscard]] Capabilities capabilities() const;
    [[nodiscard]] bool isEnabled() const;
    [[nodiscard]] Transform transform() const;
    [[nodiscard]] qreal scale() const;
    [[nodiscard]] uint32_t power() const;
    [[nodiscard]] uint32_t brightness() const;
    [[nodiscard]] uint32_t colorTemp() const;
    [[nodiscard]] OutputMode *currentMode() const;
    [[nodiscard]] QList<OutputMode *> modes() const;
Q_SIGNALS:
    void modesChanged();
    void positionChanged(const QPoint &position);
    void enabledChanged(bool enabled);
    void transformChanged(Transform transform);
    void scaleChanged(qreal scale);
    void powerChanged(uint32_t power);
    void brightnessChanged(uint32_t brightness);
    void colorTempChanged(uint32_t colorTemp);
    void currentModeChanged(OutputMode *currentMode);
private:
    explicit Output(QObject *parent = nullptr);

    class Private;
    Private* d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Output::Capabilities)
}
#endif //UKUI_QUICK_OUTPUT_MANAGER_H
