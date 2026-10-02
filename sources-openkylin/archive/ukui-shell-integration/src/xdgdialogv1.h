// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#pragma once

#include "ukui-xdg-dialog-v1.h"

namespace UKUI
{

class XdgDialogV1 : public xdg_dialog_v1
{
public:
    XdgDialogV1(::xdg_dialog_v1 *object);
    ~XdgDialogV1() override;
};

class XdgDialogWmV1 : public xdg_wm_dialog_v1
{
public:
    XdgDialogWmV1(wl_registry *registry, uint32_t id, int version);
    ~XdgDialogWmV1() override;
    XdgDialogV1 *getDialog(::xdg_toplevel *toplevel);
};

} // namespace UKUI
