#include <QDebug>
#include <QList>
#include "libinput/device.h"
#include "libinputeventdispatch.h"
#include "x11globaleventfilter.h"


namespace KWin {

LibinputEventDispatch* LibinputEventDispatch::_self = nullptr;

LibinputEventDispatch* LibinputEventDispatch::self()
{
    if(!_self)
        _self = new LibinputEventDispatch;
    return _self;
}

LibinputEventDispatch::LibinputEventDispatch():
    m_connection(nullptr)
{
    qRegisterMetaType<LEvent>("LEvent");
    qRegisterMetaType<UEvent>("UEvent");
    qRegisterMetaType<LTouchEvent>("LTouchEvent");
    qRegisterMetaType<LPointerEvent>("LPointerEvent");
}

void LibinputEventDispatch::init()
{
    printf("enter libinput event dispactc init\n");
    m_logind = KWin::LogindIntegration::create(this);
    QObject::connect(m_logind, &KWin::LogindIntegration::connectedChanged,
        [this]() {
            printf("enter KWin::LogindIntegration::connectedChanged\n");
            if (m_logind->isConnected()) {
                printf("enter logind->takeControl();\n");
                m_logind->takeControl();
            }
        }
    );

    QObject::connect(m_logind, &KWin::LogindIntegration::hasSessionControlChanged,
        [this](bool valid) {
            printf("enter hasSessionControlChanged valid=%s\n", valid ? "true" : "false");
            if (!valid) {
                return;
            }
            m_connection = LibInput::Connection::create(this);
            if(!m_connection)
            {
                printf("LibInput::Connection::create falied!\n");
                qDebug() << "create libinput connection failed!";
                return;
            }
            QObject::connect(m_connection, &LibInput::Connection::eventsRead, [=](){
                    m_connection->processEvents();
                }
            );

            m_connection->setup();

            connect(m_connection, &LibInput::Connection::touchDown, this,
                    [=](qint32 id, const QPointF &absolutePos, quint32 time, KWin::LibInput::Device *device){
                LEvent event;
                event.type = LIBINPUT_EVENT_TOUCH_DOWN;
                event.event.touch_event.id = id;
                event.event.touch_event.pos = absolutePos;
                event.event.touch_event.time = time;
                event.event.touch_event.device = LDevice(device->screenId());
                printf("LibinputEventDispatch to dispatch!\n");
                dispatch(event);
            });

    });
}

void LibinputEventDispatch::registerGlobalEventFilter(X11GlobalEventFilter *filter)
{
    m_globalEventFilter.append(filter);
}

void LibinputEventDispatch::unregisterGlobalEventFilter(X11GlobalEventFilter *filter)
{
    m_globalEventFilter.removeOne(filter);
}

void LibinputEventDispatch::dispatch(LEvent event)
{
    for(auto &filter : m_globalEventFilter)
    {
        filter->event(event);
    }
}

}
