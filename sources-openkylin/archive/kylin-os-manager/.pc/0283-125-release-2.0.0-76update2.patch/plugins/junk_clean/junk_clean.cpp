#include "junk_clean.h"
#include <QThread>
#include <QCoreApplication>
#include <QStackedWidget>
#include <QTranslator>
#include <QLocale>
#include <QDebug>
#include <QApplication>
#include <QLibraryInfo>
#include "frame.h"
#include "defines.h"
#include "homepage.h"
#include "mainwindow.h"
#include "clean_up_finish_widget.h"
#include "clean_up_service.h"

namespace junk_clean {

KomApplicationInterface *JunkCleanProvider::create() const {
    return new JunkClean();
}

JunkClean::JunkClean() {

}

JunkClean::~JunkClean() {

}

void JunkClean::init(void (*frame_callback)(const char *, ...)) {
    Frame::setFrameCallback(frame_callback);
}

void JunkClean::communication(enum CommunicationType communication_type, QVariant data) {
    switch (communication_type) {
        case CommunicationType::kRunJunkClean: {
            Q_EMIT sig_StartScan(data);
            break;
        }
        default: {
            break;
        }
    }
}

std::string JunkClean::name() {
    return "JunkClean";
}

std::string JunkClean::i18nName() {
    return QObject::tr("JunkClean").toStdString();
}

std::string JunkClean::icon() {
    return "ukui-rubbish-symbolic";
}

int JunkClean::sort() {
    return 3;
}

QWidget *JunkClean::createWidget() {
    qRegisterMetaType<junk_clean::TypeCleanUpItems>();
    qRegisterMetaType<junk_clean::JunkItem>();
    qRegisterMetaType<junk_clean::CleanTask>();

    // 加载翻译
    auto *translator = new QTranslator;
    if (!translator->load(QLocale(), "junk-clean", "_", "/usr/share/kylin-os-manager/junk-clean/translations")) {
        qCritical() << "Junk clean load translate file fail.";
    } else {
        QApplication::installTranslator(translator);
    }

    auto *qt_translator = new QTranslator;
    if (!qt_translator->load(QLocale(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        qCritical() << "Junk clean load qt translate file fail.";
    } else {
        QApplication::installTranslator(qt_translator);
    }

    auto *sdk_translator = new QTranslator;
    if (!sdk_translator->load(":/translations/gui_" + QLocale::system().name() + ".qm")) {
        qCritical() << "Junk clean load sdk translate file fail.";
    } else {
        QApplication::installTranslator(sdk_translator);
    }

    auto *stack_widget = new QStackedWidget;
    auto *homepage = new Homepage;
    auto *main_window = new MainWindow;
    auto *clean_up_finish_widget = new CleanUpFinishWidget;
    stack_widget->insertWidget(StackWidget::kHomepage, homepage);
    stack_widget->insertWidget(StackWidget::kMainWindow, main_window);
    stack_widget->insertWidget(StackWidget::kCleanFinish, clean_up_finish_widget);
    stack_widget->setCurrentIndex(StackWidget::kHomepage);
    QObject::connect(homepage, &Homepage::sig_StartScan, [stack_widget, main_window]() {
        main_window->StartScan();
        stack_widget->setCurrentIndex(StackWidget::kMainWindow);
    });
    QObject::connect(main_window, &MainWindow::sig_SwitchHomepage, [stack_widget]() {
        stack_widget->setCurrentIndex(StackWidget::kHomepage);
    });
    QObject::connect(main_window, &MainWindow::sig_CleanFinished, [stack_widget, clean_up_finish_widget](quint64 cleaned_size, quint64 cleaned_file_count, quint64 time) {
        clean_up_finish_widget->SetCleanedSize(cleaned_size);
        clean_up_finish_widget->SetCleanedFileCount(cleaned_file_count);
        clean_up_finish_widget->SetCleanTime(time);
        clean_up_finish_widget->checkDiskAgain();
        stack_widget->setCurrentIndex(StackWidget::kCleanFinish);
    });
    QObject::connect(clean_up_finish_widget, &CleanUpFinishWidget::sig_Complete, [stack_widget]() {
        stack_widget->setCurrentIndex(StackWidget::kHomepage);
    });

    QObject::connect(this, &JunkClean::sig_StartScan, this, [homepage, main_window](QVariant data) {
        if (!data.isNull()) {
            const auto flag = data.toString();
            if (flag == "MaxDiskCapacity") {
                main_window->AlwaySelectLog(true);
            }
        }
        homepage->sig_StartScan();
    });
    bool *cancelPtr = main_window->getCancelPtr();
    auto *clean_up_service = new CleanUpService(cancelPtr);
    auto *thread = new QThread;
    clean_up_service->moveToThread(thread);

    QObject::connect(qApp, &QCoreApplication::aboutToQuit, [thread]() {
        thread->quit();
        thread->wait();
    });
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    QObject::connect(thread, &QThread::finished, clean_up_service, &CleanUpService::deleteLater);
    QObject::connect(thread, &QThread::started, main_window, &MainWindow::sig_CleanUpItems);
    QObject::connect(main_window, &MainWindow::sig_CleanUpItems, clean_up_service, &CleanUpService::on_CleanUpItems);
    QObject::connect(clean_up_service, &CleanUpService::sig_CleanUpItemsFinish, main_window, &MainWindow::on_CleanUpItemsFinish);
    QObject::connect(main_window, &MainWindow::sig_Scan, clean_up_service, &CleanUpService::on_Scan);
    QObject::connect(clean_up_service, &CleanUpService::sig_ScanForJunk, main_window, &MainWindow::on_ScanForJunk);
    QObject::connect(clean_up_service, &CleanUpService::sig_ScanFinish, main_window, &MainWindow::on_ScanFinish);
    QObject::connect(main_window, &MainWindow::sig_Clean, clean_up_service, &CleanUpService::on_Clean);
    QObject::connect(clean_up_service, &CleanUpService::sig_CleanForJunk, main_window, &MainWindow::on_CleanForJunk);
    QObject::connect(clean_up_service, &CleanUpService::sig_CleanFinish, main_window, &MainWindow::on_CleanFinish);

    thread->start();

    return stack_widget;
}

}
