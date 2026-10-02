/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2022 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: sundagao <sundagao@kylinos.cn>
 */
#include "input-device-helper.h"

static Display* display = QX11Info::display();

Atom InputDeviceHelper::propertyToAtom(const char* property)
{
    return XInternAtom(display,property,False);
}

bool InputDeviceHelper::supportXinputExtension()
{
    int xi_opcode ,event ,error;
    return XQueryExtension(display, "XInputExtension" , &xi_opcode , &event, &error);
}

Atom InputDeviceHelper::deviceHadProperty(int device, const char *property)
{
    Atom prop = propertyToAtom(property);
    return deviceHadProperty(device, prop);
}

Atom InputDeviceHelper::deviceHadProperty(int device, Atom prop)
{
    int num_props;
    Atom found = None;
    Atom* props = XIListProperties(display,device,&num_props);
    if (prop == None || !props) {
        USD_LOG(LOG_WARNING,"get prop/props is faild");
        return found;
    }
    for (int i = 0; i < num_props; ++i) {
        if (prop == props[i]) {
            found = prop;
        }
    }
    XFree(props);
    return found;
}

QVariantList InputDeviceHelper::getDeviceProp(int device, const char* property)
{
    Atom prop = propertyToAtom(property);
    return getDeviceProp(device, prop);
}

QVariantList InputDeviceHelper::getDeviceProp(int device, Atom prop)
{
    QVariantList valueList;
    Atom           real_type;
    int            real_format;
    unsigned long  nitems;
    unsigned long  bytes_after;
    unsigned char* data = nullptr;
    unsigned char* dataPtr = nullptr;

    if (Success == XIGetProperty(display, device, prop, 0,
                                1000, False, AnyPropertyType,  &real_type,
                                &real_format, &nitems, &bytes_after, &data)) {
        dataPtr = data;
        Atom float_atom = propertyToAtom("FLOAT");
        for (int i = 0; i < nitems; ++i) {
            switch (real_type) {
            case XA_INTEGER:
                switch (real_format) {
                case 8:
                    valueList << *(reinterpret_cast<int8_t*>(dataPtr));
                    break;
                case 16:
                    valueList << *(reinterpret_cast<int16_t*>(dataPtr));
                    break;
                case 32:
                    valueList << *(reinterpret_cast<int32_t*>(dataPtr));
                    break;
                default:
                    break;
                }
                break;
            default:
                if (float_atom == real_type && real_format == 32) {
                    valueList << *(reinterpret_cast<float*>(dataPtr));
                } else {
                    USD_LOG(LOG_DEBUG,"property real type is not expanded. real type :%d",real_type);
                }
                break;
            }
            dataPtr += real_format / 8;
        }
        XFree(data);
    } else {
        USD_LOG(LOG_WARNING,"get device propetry faild .");
    }
    return valueList;
}

void InputDeviceHelper::setDeviceProp(int device, const char *property, QVariantList value)
{
    Atom prop = propertyToAtom(property);
    return setDeviceProp(device, prop, value);
}

void InputDeviceHelper::setDeviceProp(int device, Atom prop ,QVariantList value)
{
    if (prop == None) {
        USD_LOG(LOG_WARNING,"device property is none .");
        return;
    }

    Atom           real_type;
    int            real_format;
    unsigned long  nitems;
    unsigned long  bytes_after;

    union {
        unsigned char *c;
        int16_t *s;
        int32_t *l;
    } data = {nullptr};

    if (Success != XIGetProperty(display, device, prop, 0,
                                0, False, AnyPropertyType,  &real_type,
                                &real_format, &nitems, &bytes_after, &data.c)) {
        USD_LOG(LOG_WARNING,"get device propetry faild .");
        return;
    }
    XFree(data.c);

    Atom float_atom = propertyToAtom("FLOAT");
    int count = value.count();
    data.c = reinterpret_cast<unsigned char*>(calloc(count,sizeof(int32_t)));

    for (int i = 0; i < count; ++i) {
        switch (real_type) {
        //目前设置的属性fomat，以下case都已包含，如有需要可以添加
        case XA_INTEGER:
            switch (real_format) {
            case 8:
                data.c[i] = value.at(i).toInt();
                break;
            case 16:
                data.l[i] = value.at(i).toInt();
                break;
            case 32:
                data.l[i] = value.at(i).toInt();
                break;
            default:
                break;
            }
            break;
        default:
            if (float_atom == real_type && real_format == 32) {
                reinterpret_cast<float*>(data.l)[i] = value.at(i).toFloat();
            }
            break;
        }
    }
    XIChangeProperty (display, device,prop,
                      real_type, real_format, PropModeReplace, data.c, count);
    XSync(display,False);
    free(data.c);
}

int InputDeviceHelper::getDeviceButtonMap(int device, unsigned char** map)
{
    int ndevice;
    int nbuttons = 0;
    XDeviceInfo* deviceInfos = nullptr;
    XDeviceInfo* deviceinfo = nullptr;
    deviceInfos = XListInputDevices(display,&ndevice);

    for (int i = 0; i < ndevice; ++i) {
        if (device == deviceInfos[i].id) {
            deviceinfo = &deviceInfos[i];
        }
    }

    if (!deviceinfo) {
        USD_LOG(LOG_WARNING,"con't find device .");
        XFreeDeviceList(deviceInfos);
        return nbuttons;
    }
    XDevice* dev = XOpenDevice(display,device);
    if (!dev) {
        USD_LOG(LOG_WARNING,"open device %d is faild",dev->device_id);
        return nbuttons;
    }

    for (int i = 0; i < deviceinfo->num_classes; ++i) {
        if (deviceinfo->inputclassinfo->c_class == ButtonClass) {
            nbuttons = reinterpret_cast<XButtonInfoPtr>(deviceinfo->inputclassinfo)->num_buttons;
        }
    }
    *map = reinterpret_cast<unsigned char*>(malloc(sizeof(unsigned char)*nbuttons));
    nbuttons = XGetDeviceButtonMapping(display,dev,*map,nbuttons);
    XCloseDevice(display,dev);
    XFreeDeviceList(deviceInfos);
    return nbuttons;
}

void InputDeviceHelper::setDeviceButtonMap(int device, int nbuttons, unsigned char* map)
{
    XDevice* dev = XOpenDevice(display,device);
    if (!dev) {
        USD_LOG(LOG_WARNING,"open device %d is faild",device);
        return;
    }
    XSetDeviceButtonMapping(display,dev,map,nbuttons);
    XCloseDevice(display,dev);
    XFree(map);
}

void InputDeviceHelper::changePtrFeedbackControl(int device , int threshold, int numerator, int denominator)
{
    XDevice* dev = XOpenDevice(display, device);
    if (!dev) {
        USD_LOG(LOG_WARNING,"open device %d is faild",device);
        return;
    }
    int nFeedbacks;
    XFeedbackState *state = XGetFeedbackControl(display, dev, &nFeedbacks);
    if (!state) {
        USD_LOG(LOG_WARNING,"get feedback states faild .");
        return;
    }

    int id = -1;
    for (int i = 0; i < nFeedbacks; ++i) {
        if (state->c_class == PtrFeedbackClass) {
            id = state->id;
            break;
        }
        state = reinterpret_cast<XFeedbackState *>((reinterpret_cast<char *>(state) + state->length));
    }
    XFreeFeedbackList(state);

    if (-1 == id) {
        USD_LOG(LOG_WARNING,"unable find ptrfeedback .");
        return ;
    }
    XPtrFeedbackControl feedback;
    feedback.c_class    = PtrFeedbackClass;
    feedback.length     = sizeof(XPtrFeedbackControl);
    feedback.id         = id;
    feedback.threshold  = threshold;
    feedback.accelNum   = numerator;
    feedback.accelDenom = denominator;

    XChangeFeedbackControl(display, dev, DvAccelNum | DvAccelDenom | DvThreshold, (XFeedbackControl *)&feedback);
}

//禁用设备
void InputDeviceHelper::disable(int device)
{
    Atom prop = propertyToAtom("Device Enabled");
    QVariantList list;
    list << false;
    setDeviceProp(device, prop, list);
}

//启用设备
void InputDeviceHelper::enabel(int device)
{
    Atom prop = propertyToAtom("Device Enabled");
    QVariantList list;
    list << true;
    setDeviceProp(device, prop, list);
}

