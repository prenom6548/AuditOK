#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QTextCodec>
#include <QTranslator>
#include "logic/KMBuilderManifest.h"
#include "logic/KMBuilderSearch.h"

/**
 * @brief 应用初始化
 * @param a
 * @note  new出来的两个QTranslator不用删除，需要整个生命周期内有效，应用结束由操作系统收回即可
 */
void initApp(QApplication& a)
{
    // 前端向后端传递QString参数，若参数中含有中文则保证不会乱码
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    QString locale = QLocale::system().name();

    // QT自身标准的翻译
#ifndef QT_NO_TRANSLATION
    QTranslator *selfTransOfQt = new QTranslator();
    if (selfTransOfQt->load(QLocale(), QString("qt"), QString("_"), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        a.installTranslator(selfTransOfQt);
    else
        qDebug() << "load qt translator file failed!";
#endif

    // 部分sdk控件用到翻译文件
    // TODO

    // 应用内的翻译
    QTranslator *translator = new QTranslator();
    QString translatorResourceName(":/language/qt_");
        translatorResourceName += locale;
        translatorResourceName += ".qm";
        if (!translator->load(translatorResourceName)) //qtcreator启动后看到在资源目录下
            qDebug() << translatorResourceName << "load translator file failed!";
        else {
            a.installTranslator(translator);
            qDebug() << translatorResourceName << "load translator file success!";
        }
}

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QApplication a(argc, argv);
    a.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    initApp(a);
    MainWindow w;
    w.show();
    return a.exec();
}
