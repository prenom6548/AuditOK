/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "grab-x11.h"
#include <QX11Info>
#include <QDebug>
#include <QString>
#include <stdio.h>
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

#define MAX_PROPERTY_VALUE_LEN 4096

class XServerGraber
{
public:
    XServerGraber()
    {
        xcb_grab_server(QX11Info::connection());
    }
    ~XServerGraber()
    {
        xcb_ungrab_server(QX11Info::connection());
        xcb_flush(QX11Info::connection());
    }
};

static bool grabKeyboard()
{
    int rv = XGrabKeyboard(
        QX11Info::display(), QX11Info::appRootWindow(), True, GrabModeAsync, GrabModeAsync, CurrentTime);
    return (rv == GrabSuccess);
}

static bool grabMouse()
{
#define GRABEVENTS ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask
    int rv = XGrabPointer(
        QX11Info::display(),
        QX11Info::appRootWindow(),
        True,
        GRABEVENTS,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None,
        CurrentTime);
#undef GRABEVENTS

    return (rv == GrabSuccess);
}

bool establishGrab()
{
    XSync(QX11Info::display(), False);
    XServerGraber xserverGraber;

    Q_UNUSED(xserverGraber);

    if (!grabKeyboard())
        return false;

    /*抓取鼠标会导致触摸无效，这里暂时取消掉*/
    /*
        if(!grabMouse()) {
            XUngrabKeyboard(QX11Info::display(), CurrentTime);
            XFlush(QX11Info::display());
            return false;
        }
    */
    return true;
}

bool closeGrab()
{
    // XSync(QX11Info::display(), False);
    XServerGraber xserverGraber;

    Q_UNUSED(xserverGraber);

    XUngrabKeyboard(QX11Info::display(), CurrentTime);
    //    XUngrabPointer(QX11Info::display(), CurrentTime);
    XFlush(QX11Info::display());
    return true;
}

static Bool Window_Has_Property(Display *dpy, Window win, Atom atom)
{
    Atom type_ret;
    int format_ret;
    unsigned char *prop_ret;
    unsigned long bytes_after, num_ret;

    type_ret = None;
    prop_ret = NULL;
    XGetWindowProperty(
        dpy, win, atom, 0, 0, False, AnyPropertyType, &type_ret, &format_ret, &num_ret, &bytes_after, &prop_ret);
    if (prop_ret)
        XFree(prop_ret);

    return (type_ret != None) ? True : False;
}

int getWindowByPid(Window win)
{
    int ret = -1;

    Atom xa_ret_type;
    int ret_format;
    unsigned long ret_nitems;
    unsigned long ret_bytes_after;
    unsigned long tmp_size;
    unsigned char *ret_prop;
    int id = 0;

    Atom xa_prop_name = XInternAtom(QX11Info::display(), "_NET_WM_PID", False); // 取对应字串的AtomID

    if (XGetWindowProperty(
            QX11Info::display(),
            win,
            xa_prop_name,
            0, // 获取窗口属性
            MAX_PROPERTY_VALUE_LEN / 4,
            False,
            XA_CARDINAL,
            &xa_ret_type, // XA_CARDINAL为数值类型
            &ret_format,
            &ret_nitems,
            &ret_bytes_after,
            &ret_prop)
        != Success) // 后五个参数是返回值
    {
        printf("Cannot get %s property.\n", "_NET_WM_PID");
        return -1;
    } else {
        if (ret_prop)
            memcpy(&id, ret_prop, 4); // 类型传换
        qDebug() << "window pid: " << id;
    }
}

static Bool Window_Is_Viewable(Display *dpy, Window win)
{
    Bool ok;
    XWindowAttributes xwa;

    XGetWindowAttributes(dpy, win, &xwa);
    ok = (xwa.c_class == InputOutput) && (xwa.map_state == IsViewable);

    return ok;
}

static Bool getIsPopupMenu(Display *disp, Window win, Atom xa_prop_type, const char *prop_name, ulong *size)
{
    Atom xa_prop_name;
    Atom xa_ret_type;
    int ret_format;
    ulong ret_nitems;
    ulong ret_bytes_after;
    ulong tmp_size;
    unsigned char *ret_prop;
    char *ret;

    xa_prop_name = XInternAtom(disp, prop_name, False);

    qDebug() << "xa_prop_name = " << xa_prop_name;
    if (XGetWindowProperty(
            disp,
            win,
            xa_prop_name,
            0,
            MAX_PROPERTY_VALUE_LEN / 4,
            False,
            xa_prop_type,
            &xa_ret_type,
            &ret_format,
            &ret_nitems,
            &ret_bytes_after,
            &ret_prop)
        != Success) {
        printf("Cannot get %s property.\n", prop_name);
        return false;
    }

    qDebug() << "xa_ret_type = " << xa_ret_type << "ret_format = " << ret_format << "ret_nitems = " << ret_nitems
             << "xa_prop_type = " << xa_prop_type;
    if (xa_ret_type != xa_prop_type) {
        printf("Invalid type of %s property.\n", prop_name);
        XFree(ret_prop);
        return false;
    }

    Atom type, *adata;
    adata = (Atom *)ret_prop;
    int i = 0;
    while (i < (int)ret_nitems) {
        QString windowType = XGetAtomName(QX11Info::display(), adata[i]);
        if (windowType == "_NET_WM_WINDOW_TYPE_POPUP_MENU")
            return true;
        i++;
    }

    qDebug() << "ret = " << ret;
    if (size) {
        *size = tmp_size;
    }

    XFree(ret_prop);
    return false;
}

QString getWindowNameByWid(Window window)
{
    XClassHint ch;
    ch.res_name = NULL;
    ch.res_class = NULL;
    XGetClassHint(QX11Info::display(), window, &ch);

    QString res(ch.res_name);
    if (ch.res_name)
        XFree(ch.res_name);
    if (ch.res_class)
        XFree(ch.res_class);
    qDebug() << "CurFocusWnd:" << res;
    return res;
}

bool checkHasPopupMenu()
{
    Window focus = 0;
    int rev = 0;

    XGetInputFocus(QX11Info::display(), &focus, &rev);
    if (focus != None && focus != PointerRoot) {
        Window root, parent;
        Window *children;
        unsigned int n_children;
        int i;

        Window root_window = DefaultRootWindow(QX11Info::display());

        if (XQueryTree(QX11Info::display(), root_window, &root, &parent, &children, &n_children)) {
            for (i = (int)n_children - 1; i >= 0; i--) {
                if (!Window_Is_Viewable(QX11Info::display(), children[i])) {
                    children[i] = None; /* Don't bother descending into this one */
                    continue;
                }

                if (!Window_Has_Property(
                        QX11Info::display(),
                        children[i],
                        XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False)))
                    continue;

                if (getIsPopupMenu(QX11Info::display(), children[i], XA_ATOM, "_NET_WM_WINDOW_TYPE", NULL)) {
                    return true;
                }

                if (getWindowNameByWid(children[i]) == "ukui-session-tools") {
                    return true;
                }

                continue;
            }
        }
    }
    return false;
}
