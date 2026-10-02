/*
 * Qt5-UKUI's Library
 *
 * 为 KDirTabBar 提供真实文件系统子目录列举（对齐 kdk::KDirTabBar 的 QDirIterator）。
 */

#ifndef KYDIRHELPER_H
#define KYDIRHELPER_H

#include <QObject>
#include <QVariantList>

namespace UKUISDKStyle {

class KyDirHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString homePath READ homePath CONSTANT)

public:
    explicit KyDirHelper(QObject *parent = nullptr);

    QString homePath() const;

    /**
     * @brief 列出 path 下的子目录
     * @return [{ text, uri, icon }, ...]
     */
    Q_INVOKABLE QVariantList listSubDirectories(const QString &path) const;

    /** 去掉末尾多余 '/'（根路径 "/" 保留） */
    Q_INVOKABLE QString normalizePath(const QString &path) const;

private:
    static QString folderIconName(const QString &baseName);
};

} // namespace UKUISDKStyle

#endif // KYDIRHELPER_H
