// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "wayland");
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "ukui-shell");
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
