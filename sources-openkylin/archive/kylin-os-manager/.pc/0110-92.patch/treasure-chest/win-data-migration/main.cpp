#include "widgetconnect.h"
#include <QTranslator>
#include <QScreen>
#include <QTranslator>
#include <QLibraryInfo>
#include <singleapplication.h>
#include <log.hpp>
int main(int argc, char *argv[])
{
#ifndef DEBUG_MODE
    qInstallMessageHandler(kdk::kabase::Log::logOutput);
#endif
    // 修改分辨率的显示
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
    // 单例检查
    kdk::QtSingleApplication a(argc, argv);
    if (a.isRunning()) {
        qDebug() << "is running";
        a.sendMessage("running , 4000");
        return 0;
    }
    // QLocale locale("bo_CN");
    QLocale locale;
    // 加载应用翻译文件
    QString projectName = "win-data-migration";
    QString localeFileCrush = "translations";
    QString kylinFileCrush = "/usr/share/kylin-os-manager/" + projectName + "/translations";
    QTranslator trans_global;
    if (trans_global.load(locale, projectName, "_", localeFileCrush)) {
        a.installTranslator(&trans_global);
    } else {
        if (!trans_global.load(QLocale(), projectName, "_", kylinFileCrush)) {
            qWarning() << "main Load global translations file" << QLocale() << "failed!";
        } else {
            a.installTranslator(&trans_global);
        }
    }
    // 加载Qt翻译文件
    QTranslator trans_qt;
    QString qtTransPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    if (!trans_qt.load(locale, "qt", "_", qtTransPath)) {
        qWarning() << "main Load qt translations file" << QLocale() << "failed!";
    } else {
        a.installTranslator(&trans_qt);
    }
    // 加载sdk控件翻译
    QTranslator trans;
    if (trans.load(":/translations/gui_" + locale.name() + ".qm")) {
        a.installTranslator(&trans);
    }
    // 翻译
    Constant::getTranslations();
    // 实例化主界面
    WidgetConnect w;
    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    w.move(availableGeometry.width() / 2 - w.width() / 2, availableGeometry.height() / 2 - w.height() / 2);
    a.setActivationWindow(&w);
    a.setApplicationName(QApplication::tr("Win Data Migration"));
    a.setWindowIcon(QIcon::fromTheme("kylin-win-data-migration"));
    w.show();
    return a.exec();
}
