#ifndef FUNCTYPECONVERTER_H
#define FUNCTYPECONVERTER_H

#include <QObject>
#include <QMetaEnum>
#include <QMap>

/**
 * @brief 功能类型转换器
 */
class FuncTypeConverter : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 主界面功能类型
     */
    enum FunType{
        BACKUP_SYSTEM,
        RESTORE_SYSTEM,
        BACKUP_DATA,
        RESTORE_DATA,
        OPERATION_LOG,
        GHOST_IMAGE,
        TOTALMODULES,
    };
    Q_ENUM(FunType)

    /**
     * @brief 功能类型关联信息
     */
    struct FunTypeAssociation
    {
        // 功能名称，用于左边功能选择栏中的文本显示
        QString m_funcName;
        // 功能对应的主题图标名称，用于左边功能选择栏中的图标显示
        QString m_themeIconName;
        // 功能对应的窗口类名，用于根据类名动态创建窗口类
        QString m_widgetClassName;
    };

public:
    explicit FuncTypeConverter();
    ~FuncTypeConverter();

public:
    // 枚举值对应的枚举变量名
    QString keyCodeToKeyString(int code);
    // 枚举变量名对应的枚举值
    int keystringTokeycode(QString string);

    /**
     * @brief 功能类型对应的功能名称
     * @param funcType 功能类型
     * @return 功能名称
     */
    QString funcTypeToFuncName(int funcType);

    /**
     * @brief 功能类型对应的主题图标名称
     * @param funcType 功能类型
     * @return 主题图标名称
     */
    QString funcTypeToThemeIconName(int funcType);

    /**
     * @brief 功能类型对应的窗口类名
     * @param funcType
     * @return 窗口类名
     */
    QString funcTypeToWidgetClassName(int funcType);

private:
    QMetaEnum m_metaModule;
    QMap<int, FunTypeAssociation> m_mapAssociation;
};

#endif // FUNCTYPECONVERTER_H
