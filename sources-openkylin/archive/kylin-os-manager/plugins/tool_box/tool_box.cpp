#include "tool_box.h"
#include <QApplication>
#include <QTranslator>
#include <QDebug>
#include <QLocale>
#include "frame.h"
#include "mainwindow.h"

namespace tool_box {

KomApplicationInterface *ToolBoxProvider::create() const {
    return new ToolBox();
}

ToolBox::ToolBox() {

}

ToolBox::~ToolBox() {

}

void ToolBox::init(void (*frame_callback)(const char *, ...)) {
    Frame::setFrameCallback(frame_callback);
}

std::string ToolBox::name() {
    return "ToolBox";
}

std::string ToolBox::i18nName() {
    return QObject::tr("ToolBox").toStdString();
}

std::string ToolBox::icon() {
    return "ukui-full-backup-symbolic";
}

int ToolBox::sort() {
    return 6;
}

QWidget *ToolBox::createWidget() {
    // 加载翻译
    QString tranPath("/usr/share/kylin-os-manager/plugins/tool-box/translations");
    QTranslator *translator = new QTranslator;
    if (translator->load(QLocale(), "tool-box", "_", tranPath)) {
        QApplication::installTranslator(translator);
    } else {
        qWarning() << "Tool box load translation file fail";
    }

    // 加载 sdk 翻译
    auto locale = QLocale::system().name();
    auto *sdk_translator = new QTranslator;
    if (!sdk_translator->load(":/translations/gui_" + locale + ".qm")) {
        qWarning() << "Tool box load sdk translation file fail";
    } else {
        QApplication::installTranslator(sdk_translator);
    }

    auto *mainwindow = new MainWindow;
    return mainwindow;
}

}
