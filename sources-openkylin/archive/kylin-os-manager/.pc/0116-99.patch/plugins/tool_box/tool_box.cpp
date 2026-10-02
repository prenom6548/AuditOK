#include "tool_box.h"
#include <QApplication>
#include <QTranslator>
#include <QDebug>
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
    return 100;
}

QWidget *ToolBox::createWidget() {
    QString tranPath("/usr/share/kylin-os-manager/plugins/tool-box/translations");
    QTranslator *translator = new QTranslator;
    if (translator->load(QLocale(), "kylin-os-manager-tool-box", "_", tranPath)) {
        QApplication::installTranslator(translator);
    } else {
        qWarning() << "ProblemFeedback load translation file fail !";
    }
    auto *mainwindow = new MainWindow;
    return mainwindow;
}

}
