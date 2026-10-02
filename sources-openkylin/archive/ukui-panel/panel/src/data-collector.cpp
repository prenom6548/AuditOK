/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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

#include "data-collector.h"
#include <QThread>
#include <QTimer>

class DataCollectWrapper : public QObject
 {
    Q_OBJECT
public:
    explicit DataCollectWrapper(QObject *parent = nullptr) : QObject(parent)
     {
     }
    void setPanelPositionEvent(const int &pos)
    {
        auto *property = new KCustomProperty;
        property->key = strdup(QStringLiteral("panelPosition").toLocal8Bit().data());
        property->value = strdup(positionToStr(pos).toLocal8Bit().data());
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
        kdk_dia_append_custom_property(node, property, 1);
        kdk_dia_upload_default(node, strdup(QStringLiteral("adjust_panel_position_event").toLocal8Bit().data()),strdup(QStringLiteral("ukui-panel").toLocal8Bit().data()));
        kdk_dia_data_free(node);
        free(property->key);
        free(property->value);
        delete property;
        functiomExecuted();
    }
    void setPanelSizeEvent(const int &panelSize)
    {
        KCustomProperty *property = new KCustomProperty;
        property->key = strdup(QStringLiteral("panelSize").toLocal8Bit().data());
        property->value = strdup(QString::number(panelSize).toLocal8Bit().data());
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
        kdk_dia_append_custom_property(node, property, 1);
        kdk_dia_upload_default(node, strdup(QStringLiteral("adjust_panel_size_event").toLocal8Bit().data()),strdup(QStringLiteral("ukui-panel").toLocal8Bit().data()));
        kdk_dia_data_free(node);
        free(property->key);
        free(property->value);
        delete property;
        functiomExecuted();
    }
    static QString positionToStr(const int &pos)
    {
        switch (pos)
        {
            case 1:
                return {"Top"};
            case 2:
                return {"Left"};
            case 3:
                return {"Right"};
            case 0:
                return {"Bottom"};
            default: ;
        }

        return {};
    }
Q_SIGNALS:
    void functiomExecuted();
};

class DataCollectorPrivate;
static DataCollectorPrivate *g_dataCollectorPrivate = nullptr;
static std::once_flag flag;
class DataCollectorPrivate : public QThread
{
    Q_OBJECT
public:
    static DataCollectorPrivate *self();
    void startRunning();
Q_SIGNALS:
    void setPanelPositionEvent(const int&pos);

    void setPanelSizeEvent(const int&panelSize);
protected:
    explicit DataCollectorPrivate(QObject* parent = nullptr);
    ~DataCollectorPrivate();
    void run() override {exec();};

private:
    QTimer* m_timer = nullptr;
    DataCollectWrapper *m_wrapper = nullptr;
};

DataCollectorPrivate::DataCollectorPrivate(QObject* parent) : QThread()
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(1000);
    m_wrapper = new DataCollectWrapper();
    m_wrapper->moveToThread(this);

    connect(m_timer, &QTimer::timeout, this, &DataCollectorPrivate::quit);
    connect(this, &DataCollectorPrivate::setPanelSizeEvent, m_wrapper, &DataCollectWrapper::setPanelSizeEvent);
    connect(this, &DataCollectorPrivate::setPanelPositionEvent, m_wrapper, &DataCollectWrapper::setPanelPositionEvent);
}

DataCollectorPrivate::~DataCollectorPrivate()
{
    quit();
    wait();
    if(m_wrapper) {
        delete m_wrapper;
        m_wrapper = nullptr;
    }
    g_dataCollectorPrivate = nullptr;
}

DataCollectorPrivate* DataCollectorPrivate::self()
{
    std::call_once(flag, [ & ] {
        g_dataCollectorPrivate = new DataCollectorPrivate();
    });
    return g_dataCollectorPrivate;
 }

void DataCollectorPrivate::startRunning()
{
    if (!this->isRunning()) {
        start();
    }
    m_timer->start();
}

 void DataCollector::setPanelPositionEvent(const int &pos)
 {
    DataCollectorPrivate::self()->startRunning();
    DataCollectorPrivate::self()->setPanelPositionEvent(pos);
 }

 void DataCollector::setPanelSizeEvent(const int &panelSize)
 {
    DataCollectorPrivate::self()->startRunning();
    DataCollectorPrivate::self()->setPanelSizeEvent(panelSize);
 }
#include "data-collector.moc"
