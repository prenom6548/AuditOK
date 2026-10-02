#ifndef KMBUILDERSTRINGUTILS_H
#define KMBUILDERSTRINGUTILS_H
#include <QString>
#include <QVector>
#include <QSysInfo>
#include <QRegularExpression>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QTableWidget>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>

#include "KMRef.h"
#include "KMStructData.h"

class KMBuilderStringUtils
{
public:
    /**
     * @brief 路径拼接
     * @param str1
     * @param str2
     * @param str3
     * @param str4
     * @return 拼接好的路径
     */
    static QString buildFilename(const QString &str1, const QString &str2);
    static QString buildFilename(const QString &str1, const QString &str2, const QString &str3);
    static QString buildFilename(const QString &str1, const QString &str2, const QString &str3, const QString &str4);
    static QString buildFilename(const QString &str1, const QString &str2, const QString &str3, const QString &str4,const QString &str5);
    static QString buildFilename(const QString &str1, const QString &str2, const QString &str3, const QString &str4, const QString &str5, const QString &str6, const QString &str7);

    /**
     * @brief 校验base与runtime
     * @param base
     * @param runtime
     * @param depend_base
     * @param depend_runtime
     * @return true:符合规则， false:不符合规则
     */
    static bool checkStandard(const QString &base,const QString &runtime, const QString &depend_base, const QString &depend_runtime);

    /**
     * @brief 将字符串格式{channel}:{id}/{version}保存为结构体
     * @param infoValue
     * @return KMRef结构体
     */
    static KMRef getRefFromInfo(const QString &infoValue);
    static KMRef getRefFromValue(const QString &id, const QString &kind, const QString &channel, const QString &name, const QString &version, const QString &module);

    /**
     * @brief vector2数组合并到vector1中
     * @param vector1
     * @param vector2
     */
    static void appendVector(QVector<KMRef>&vector1, QVector<KMRef>&vector2);

    /**
     * @brief 版本号校验
     * @param version
     * @param strict
     * @return true:版本号符合规则；false:版本号不符合规则
     */
    static bool regexVersion(const QString&version, bool strict=false);

    /**
     * @brief 获取当前系统架构
     * @return 当前系统架构
     */
    static QString getArch();

    /**
     * @brief 根据展示格式拼接ref。若depend=false，则返回格式为{channel}:{id}/{version}
     * @param ref
     * @param depend,false:拼接base或runtime，true:拼接depend
     * @return 拼接后的字符串
     */
    static QString buildRef(const KMRef &ref, bool depend = false);

    /**
     * @brief 以{name}/{version}格式拼接已安装软件包结构体
     * @param package
     * @return 拼接后的字符串
     */
    static QString buildInstalledPackage(const InstalledPackage &package);

    /**
     * @brief 文件是否存在
     * @param path:文件路径
     * @return true:存在, false:不存在
     */
    static bool DirIsExits(const QString &path);
    static bool fileIsExits(const QString &path);

    /**
     * @brief 截取字符串数组
     * @param originList
     * @param start
     * @param end
     * @return 截取后的字符串数组
     */
    static QStringList subList(const QStringList &originList, int start, int end);

    /**
     * @brief 用于打包的架构处理，如sw_64 --> sw64等
     * @param platformArch，平台的架构
     * @return 包的架构
     */
    static QString packageArch(const QString& platformArch);


    /**
     * @brief 获取文件时间戳
     * @param 文件路径
     * @return 文件时间戳
     */
    static QDateTime getFileTimestamp(const QString& filePath);

    /**
     * @brief 文件时间戳与当前时间差值
     * @param 文件路径
     * @return 以s为单位的时间戳差值
     */
    static qint64 calculateTimeDifference(const QString& filePath);

    /**
     * @brief 获取路径父目录
     * @param 文件路径
     * @return 文件的父目录
     */
    static QString getpathParent(const QString &filePath);

    /**
     * @brief 获取相对路径
     * @param sourcePath 源路径
     * @param targetPath 目标文件路径
     * @return 相对路径
     */
    static QString getpathRelative(const QString &sourcePath, const QString &targetPath);

    /**
     * @brief getColumnData 获取QTableWidget类型对象某一列的数据
     * @param table  QTableWidget类型对象
     * @param column 某一列数据
     * @return 获取到的字符串数组
     */
    static QStringList getColumnData(QTableWidget* table, int column);

};




#endif // KMBUILDERSTRINGUTILS_H
