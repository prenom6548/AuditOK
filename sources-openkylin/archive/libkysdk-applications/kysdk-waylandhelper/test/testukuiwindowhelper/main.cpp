#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "wayland");
    qputenv("QT_QPA_PLATFORMTHEME", "ukui");
    // Now ukui-shell is disabled by default, enable it.
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "ukui-shell");
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
