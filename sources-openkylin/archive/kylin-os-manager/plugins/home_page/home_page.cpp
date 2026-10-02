#include "home_page.h"
#include <QTranslator>
#include <QLocale>
#include <QApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <frame.h>
#include "mainwindow.h"

namespace home_page {

KomApplicationInterface *HomePageProvider::create() const {
    return new HomePage;
}

HomePage::HomePage() {

}

HomePage::~HomePage() {

}

void HomePage::init(void (*frame_callback)(const char *function_name, ...)) {
    Frame::setFrameCallback(frame_callback);
}

std::string HomePage::name() {
    return "HomePage";
}

std::string HomePage::i18nName() {
    return QObject::tr("HomePage").toStdString();
}

std::string HomePage::icon() {
    return "go-home-symbolic";
}

int HomePage::sort() {
    return 1;
}

QWidget *HomePage::createWidget() {
    // 加载翻译
    auto *translator = new QTranslator;
    if (translator->load(QLocale(), "home-page", "_", "/usr/share/kylin-os-manager/plugins/home_page/translations/")) {
        qApp->installTranslator(translator);
    } else {
        qCritical() << "Home page load translate file fail";
    }

    auto *qt_translator = new QTranslator;
    if (qt_translator->load(QLocale(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        qApp->installTranslator(qt_translator);
    } else {
        qCritical() << "Home page load qt translate file fail";
    }

    auto *sdk_translator = new QTranslator;
    if (sdk_translator->load(":/translations/gui_" + QLocale::system().name() + ".qm")) {
        qApp->installTranslator(sdk_translator);
    } else {
        qCritical() << "Home page load sdk translate file fail";
    }

    return new MainWindow;
}

}
