/*
 * 独立进程加载单个 Peony 插件 .so，触发其静态初始化与插件工厂注册路径，
 * 用于覆盖率统计（避免在同一进程内与 extensions-test 已链接的 IDM 代码混载导致不稳定）。
 */
#include <QCoreApplication>
#include <QLibrary>
#include <QFileInfo>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    if (argc < 2) {
        return 2;
    }
    const QString path = QString::fromLocal8Bit(argv[1]);
    if (!QFileInfo::exists(path)) {
        return 3;
    }
    QLibrary lib(path);
    if (!lib.load()) {
        return 1;
    }
    lib.unload();
    return 0;
}
