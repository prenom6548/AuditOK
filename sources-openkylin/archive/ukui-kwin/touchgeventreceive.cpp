#include "main.h"
#include "platform.h"
#include "touchgeventreceive.h"
#include <cstdio>
#include "X11/extensions/XI2.h"
#include "X11/extensions/XInput2.h"


namespace KWin {

TouchGEventReceive::TouchGEventReceive()
    : m_cursorHidden(false)
{

}

void TouchGEventReceive::toRealData(XIRawEvent *event, QPoint &p, int &detail)
{
    int i, c;
    double *val;
    double pos[2] = {0, 0};
    val = event->valuators.values;
    for (i = 0, c = 0; i < event->valuators.mask_len * 8; i++)
        if (XIMaskIsSet(event->valuators.mask, i) && c < 2){
            pos[c++] = *val++;
        }
//    printf("origin pos = %f, %f, event->valuators.mask_len = %d\n", pos[0], pos[1], event->valuators.mask_len);
//    int x = (int)(1.0 * pos[0] / (1 << 16) * m_width);
//    int y = (int)(1.0 * pos[1] / (1 << 16) * m_height);
    int x = (int)(1.0 * pos[0] / touch_width * m_width);
    int y = (int)(1.0 * pos[1] / touch_height * m_height);
    p.setX(x);
    p.setY(y);
    detail = event->detail;
    return;
}

void TouchGEventReceive::getScreenResolution(int device_id)
{
    Display *dis = XOpenDisplay(0);
    if(!dis){
        return;
    }
    int ndevices = -1;
    XIDeviceInfo *info, *dev;
    info = XIQueryDevice(dis, device_id, &ndevices);
    if(ndevices != 1)
        return;
    dev = &info[0];
    XIAnyClassInfo **classes = dev->classes;
    // 获取触摸屏的触摸分辨率，并不是所有的都默认为 2^16 = 65536
    for (int i = 0; i < dev->num_classes; i++) {
        if(classes[i]->type == XIValuatorClass)
        {
            XIValuatorClassInfo *v = (XIValuatorClassInfo*)classes[i];
            char *name = v->label ?  XGetAtomName(dis, v->label) : NULL;
            if(QString(name).endsWith("X") && QString(name).startsWith("Abs")) {
                touch_width = v->max;
            }
            if(QString(name).endsWith("Y") && QString(name).startsWith("Abs")) {
                touch_height = v->max;
            }
            XFree(name);
        }
    }

    int screen = DefaultScreen(dis);
    m_width = DisplayWidth(dis, screen);
    m_height = DisplayHeight(dis, screen);
//    printf("screenResolution is %d, %d, %lf, %lf\n", m_width, m_height, touch_width, touch_height);
    XCloseDisplay(dis);
}

bool TouchGEventReceive::event(QSharedPointer<XEvent> event)
{
    XGenericEventCookie *cookie = (XGenericEventCookie*)&event->xcookie;
    XIRawEvent *rawData = (XIRawEvent *)cookie->data;

    if(cookie->type == GenericEvent)
    {
        switch (cookie->evtype) {
        case XI_RawTouchBegin:
        {
//            printf("TouchGEventReceive::event() rawData device_id = %d, source_id = %d\n", rawData->deviceid, rawData->sourceid);
            int device_id = rawData->sourceid;
            getScreenResolution(device_id);
            QPoint pos;
            int detail;
            toRealData(rawData, pos, detail);
//            printf("touchBegin pos=(%d,%d)\n", pos.x(), pos.y());
            input()->processTouchDown(detail, pos, 0);

            if(m_cursorHidden)
                break;
            m_cursorHidden = true;
            kwinApp()->platform()->hideCursor();
            break;
        }
        case XI_RawTouchUpdate:
        {
            QPoint pos;
            int detail;
            toRealData(rawData, pos, detail);
            input()->processTouchMotion(detail, pos, 0);
//            printf("touchUpdate pos=(%d,%d)\n", pos.x(), pos.y());
            break;
        }
        case XI_RawTouchEnd:
        {
            QPoint pos;
            int detail;
            toRealData(rawData, pos, detail);
            input()->processTouchUp(detail, 0);
//            printf("touchEnd pos=(%d,%d)\n", pos.x(), pos.y());
            break;
        }
        case XI_RawMotion:
        {
            if(!m_cursorHidden)
                break;
            m_cursorHidden = false;
            kwinApp()->platform()->showCursor();
            break;
        }
        default:
            break;
        }
    }
    return false;
}

bool TouchGEventReceive::event(LEvent event)
{
    switch (event.type) {
    case LIBINPUT_EVENT_TOUCH_DOWN:
    {
        printf("LIBINPUT_EVENT_TOUCH_DOWN\n");
        break;
    }
    default:
        break;
    }
    return false;
}

}

