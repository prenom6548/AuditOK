// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include "xdgdialogv1.h"

namespace UKUI
{

XdgDialogV1::XdgDialogV1(::xdg_dialog_v1 *object)
    : xdg_dialog_v1(object)
{
}

XdgDialogV1::~XdgDialogV1()
{
    xdg_dialog_v1_destroy(object());
}

XdgDialogWmV1::XdgDialogWmV1(wl_registry *registry, uint32_t id, int version)
    : xdg_wm_dialog_v1(registry, id, qMin(version, 1))
{
}

XdgDialogWmV1::~XdgDialogWmV1()
{
    destroy();
}

XdgDialogV1 *XdgDialogWmV1::getDialog(::xdg_toplevel *toplevel)
{
    return new XdgDialogV1(get_xdg_dialog(toplevel));
}

} // namespace UKUI
