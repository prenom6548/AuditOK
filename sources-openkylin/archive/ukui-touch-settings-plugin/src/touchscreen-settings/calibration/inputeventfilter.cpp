#include "inputeventfilter.h"

#include <QDebug>

#include <xcb/xinput.h>
#include <X11/extensions/XInput2.h>

namespace {
int deviceIdFromEvent(const xcb_input_hierarchy_event_t *event);
}

InputEventFilter::InputEventFilter(QObject* parent)
    : QObject(parent), QAbstractNativeEventFilter() {}

bool InputEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result)

    if (!message) {
        qWarning() << __FILE__ << __FUNCTION__ << __LINE__ << "Message is empty";
        return false;
    }
    if (eventType != "xcb_generic_event_t") {

        qWarning() << __FILE__ << __FUNCTION__ << __LINE__
                   << "Event type is:" << eventType
                   << "But currently only supported xcb_generic_event_t";
        return false;
    }

    xcb_generic_event_t* event = static_cast<xcb_generic_event_t *>(message);
    auto* ge = reinterpret_cast<const xcb_ge_generic_event_t*>(event);
    if (ge->event_type != XI_HierarchyChanged)
        return false;

    auto* he = reinterpret_cast<const xcb_input_hierarchy_event_t *>(event);
    handleHierarchyEvent(he);

    return false;
}

void InputEventFilter::handleHierarchyEvent(const xcb_input_hierarchy_event_t *event)
{
    switch (event->flags) {
    case XCB_INPUT_HIERARCHY_MASK_SLAVE_ADDED:
        emit deviceAdded(deviceIdFromEvent(event));
        break;
    case XCB_INPUT_HIERARCHY_MASK_SLAVE_REMOVED:
        emit deviceRemoved(deviceIdFromEvent(event));
        break;
    case XCB_INPUT_HIERARCHY_MASK_DEVICE_ENABLED:
        emit deviceEnabled(deviceIdFromEvent(event));
        break;
    case XCB_INPUT_HIERARCHY_MASK_DEVICE_DISABLED:
        emit deviceDisabled(deviceIdFromEvent(event));
        break;
    default:
        break;
    }
}

namespace {
int deviceIdFromEvent(const xcb_input_hierarchy_event_t *event)
{
    int num = xcb_input_hierarchy_infos_length(event);
    xcb_input_hierarchy_info_t* infos = xcb_input_hierarchy_infos(event);

    for (int i = 0; i < num; ++i) {
        if (infos[i].flags & event->flags) {
            return infos[i].deviceid;
        }
    }

    qWarning() << __FILE__ << __FUNCTION__ << __LINE__
               << "Can't find deviceid from hierarchy event";
    return -1;
}
} // namespace
