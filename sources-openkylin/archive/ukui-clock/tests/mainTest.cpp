#include <QApplication>
#include <QtTest/QtTest>

#include "testclass.h"
#include "clock.h"
#include "singleApplication.h"

int main(int argc, char *argv[])
{
    TestClass testClass;

    //适配分数缩放
   #if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
       QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
       QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
   #endif
   #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
       QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
   #endif

   //构造单例 利用本地连接qlocalsocket作为判断的公共访问资源
   SingleApplication a(argc, argv);
   //支持主题框架图标
    a.setWindowIcon(QIcon::fromTheme("kylin-alarm-clock"));
   //此属性保存关闭最后一个窗口时应用程序是否隐式退出。默认值为true。
   a.setQuitOnLastWindowClosed(false);

   QTest::qExec(&testClass);

    return 0;
}
