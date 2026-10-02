#include "utils.h"

#include <QEventLoop>
#include <QTimer>

/**
 * @brief 用事件循环替换sleep，以便事件尽量得到处理
 * @param ms，等待时间，单位ms
 */
void Utils::wait(uint ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, SLOT(quit()));
    loop.exec();
}

QString Utils::buildFilename(const QString &path, const QString &name)
{
    QString filename = path + "/" + name;
    filename.replace("//", "/");

    return filename;
}
