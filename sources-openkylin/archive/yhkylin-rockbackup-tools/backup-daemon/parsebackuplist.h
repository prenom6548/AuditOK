#ifndef PARSEBACKUPLIST_H
#define PARSEBACKUPLIST_H

#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QList>
#include "../common/mydefine.h"

class ParseBackupList {
public:
    enum ParseResult {
        // xml解析错误
        XML_PARSE_ERR,
        // 失败
        FAIL,
        // 成功
        SUCCESS,
    };

    struct BackupPoint {
        // 备份或还原指定的UUID
        QString m_uuid;
        // 备份名称，用来替换m_comment
        QString m_backupName;
        // 备份时间
        QString m_time;
        // 本地备份还是U盘备份: 0-本地备份；1-移动设备备份；2-异机备份（仅用于业务场景标记，不用于持久化记录）；3-自定义路径备份
        int m_iPosition = -1;
        // 操作类型，如：系统备份, 系统还原
        int m_type = -1;
        // 备份大小
        QString m_size;
        // 备份状态，如是否完成
        QString m_state;
        // 备份用户id
        QString m_userId;
        // 备份机器操作系统
        QString m_os;
        // 备份机器架构
        QString m_arch;
        // 备份机器引导方式
        QString m_archdetect;
        // 备份点所在设备挂载路径（这个暂只在查询中界面展示选择中使用）或自定义的备份路径
        QString m_path;
        // 备份note
        QString m_note;

        bool isNull() { return m_uuid.isEmpty(); }

        bool operator< (const BackupPoint &other);
    };

    ParseBackupList(const QString& xmlPath);

    /**
     * @brief 出厂备份修改backuplist.xml，仅保留出厂备份信息
     * @param factoryBackupUuid，出厂备份的uuid
     * @return ParseResult枚举类型
     * @author zhaominyong
     * @since 2021/06/27
     */
    ParseResult updateForFactoryRestore(const QString &factoryBackupUuid);

    /**
     * @brief 根据comment查找uuid
     * @param comment
     * @return uuid
     */
    QString findUuidByComment(const QString& comment);

    /**
     * @brief 根据Uuid查找备份点信息
     * @param Uuid
     * @return 备份点信息
     */
    BackupPoint findBackupPointByUuid(const QString& Uuid);

    /**
     * @brief 获取xml文件中备份点uuid和backupname的映射
     * @param uuid_name，xml文件中备份点uuid和backupname的映射
     * @return
     */
    void getXmlUuidNameMap(QMap<QString, QString> &uuid_name);

    /**
     * @brief 获取最后一次系统备份
     * @return
     */
    BackupPoint getLastSysBackupPoint();

    /**
     * @brief 获取自定义备份路径列表
     * @param customizePaths
     */
    void getCustomizePaths(QStringList &customizePaths);

    /**
     * @brief 新增备份节点
     * @param backupPoint, 备份点信息
     * @return SUCCESS成功； XML_PARSE_ERR失败
     */
    ParseResult addItem(const BackupPoint& backupPoint);

    /**
     * @brief 更新备份点信息
     * @param backupPoint，新的备份点信息
     * @return SUCCESS成功； XML_PARSE_ERR失败
     */
    ParseResult updateItem(const BackupPoint & backupPoint);

    /**
     * @brief 删除备份点记录
     * @param uuid
     * @return SUCCESS成功； XML_PARSE_ERR失败
     */
    ParseResult deleteItem(const QString& uuid);

    /**
     * @brief 获取备份列表
     * @return
     */
    QList<BackupPoint> getBackupPointList();

    inline QString getXmlPath() { return m_xmlPath; }

private:
    bool InitXml();
    bool isXmlCorrect();
    bool Doc_setContent(QDomDocument& doc);

    void elementNodeToBackupPoint(const QDomElement& eleNode, BackupPoint& backupPoint);
    void backupPointToElementNode(const BackupPoint& backupPoint, QDomDocument& doc, QDomNode& eleNode);
    QDomElement createTextElement(QDomDocument& doc, const QString& tagName, const QString& text);

private:
    QString m_xmlPath;
};

#endif // PARSEBACKUPLIST_H
