#ifndef UTILS_H
#define UTILS_H

#include <QString>

/**
 * @brief 工具类
 */
class Utils
{
public:
    /**
     * @brief 用事件循环替换sleep，以便事件尽量得到处理
     * @param sec，等待时间，单位ms
     */
    static void wait(uint ms);

    /**
     * @brief 构造路径名
     * @param path
     * @param name
     * @return
     */
    static QString buildFilename(const QString &path, const QString &name);
};

#endif // UTILS_H
