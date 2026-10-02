#ifndef LANGUAGEFRAME_H
#define LANGUAGEFRAME_H

#include <QMap>
#include "ukccframe.h"
#include "languagefeatureitem.h"
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

class LanguageFrame : public UkccFrame
{
    Q_OBJECT

public:
    LanguageFrame(const QString &languageCode, const QString &showName, QWidget *parent = nullptr);
    ~LanguageFrame() override;
    // 获取语言显示名称
    QString getShowName() const;
    // 获取语言编码
    QString getLanguageCode() const;
    // 设置删除按钮可用状态
    void setDeleteEnabled(bool enabled);
    void reloadIcon();
    // 设置语言特性展开状态
    void setExpanded(bool expanded);
    // 判断语言特性是否展开
    bool isExpanded() const;
    // 添加语言特性项
    LanguageFeatureItem *addLanguageFeature(LanguageFeatureType type,
                                            const QString &featureName,
                                            LanguageFeatureState state = LanguageFeatureState::Installing);
    // 获取指定语言特性项
    LanguageFeatureItem *featureItem(LanguageFeatureType type) const;
    // 判断是否存在语言特性项
    bool hasFeatureItems() const;
    // 更新语言项布局高度
    void updateLayoutHeight();

private:
    // 更新展开按钮图标
    void updateExpandIcon();
    // 通知所属分组刷新外形
    void notifyGroupShapeUpdate();

    UkccFrame *m_headerFrame = nullptr;
    QLabel *nameLabel = nullptr;
    QToolButton *expandBtn = nullptr;
    QToolButton *deleteBtn = nullptr;
    QVBoxLayout *mainLayout = nullptr;
    QMap<LanguageFeatureType, LanguageFeatureItem *> m_featureItems;
    QString m_languageCode;
    bool m_expanded = false;
    bool m_expandClickLocked = false;

Q_SIGNALS:
    // 请求删除当前语言项
    void deleteRequested();
    // 请求安装当前语言特性
    void featureInstallRequested(const QString &languageCode, LanguageFeatureType type);
};

#endif // LANGUAGEFRAME_H
