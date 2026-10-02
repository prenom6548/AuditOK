#ifndef PRIVATEMODELITEMWIDGET_H
#define PRIVATEMODELITEMWIDGET_H

#include <QWidget>
#include <QGSettings>
#include <QMouseEvent>
#include <QMenu>
#include <QLabel>
#include <QVariant>
#include <QToolButton>
#include <QHBoxLayout>

#include <aiconfig/modelconfig.h>

#define THEME_QT_SCHEMA  "org.ukui.style"

using namespace std;
using namespace kyai::config::model;

class PrivateModelItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PrivateModelItemWidget(AiCapability capability, QString modelName, QString vendor,
                                    bool hasMoreButton = true, QWidget* parent = nullptr);
    ~PrivateModelItemWidget();

    void showSelectedIcon(bool flag);
    AiCapability getAICapability();
    QString getModelName();

signals:
    void sigEdit(AiCapability capability, QString modelName);
    void sigDelete(AiCapability capability, QString modelName);

private:
    void initLayout(bool hasMoreButton);
    void initMenu();
    void initIconLabel();                   // 初始化图标的方法
    void setupThemeListener();              // 设置主题监听
    void updateIconForTheme(const QString& theme);  // 更新图标以适配主题
    QString getCurrentTheme();                      // 获取当前主题
    void updateVendorIcon(QLabel* label, const QString& defaultIconPath,
                          const QString& lightIconPath);  // 更新供应商图标

    QLabel* m_nameLabel = nullptr;          // 模型名称
    QLabel* m_iconLabel = nullptr;          // 模型图标
    QToolButton* m_moreButton = nullptr;    // 更多
    QMenu* m_menu = nullptr;                // 菜单
    QLabel* m_selectedIconLabel = nullptr;  // 已选择状态
    kyai::config::model::AiCapability m_capability;
    QString m_modelName;
    QString m_vendor;  // 模型供应商信息

    QPixmap m_unselectedPixmap;
    QPixmap m_selectedPixmap;

    QGSettings* m_themeSettings = nullptr;  // 主题设置对象
};

#endif // PRIVATEMODELITEMWIDGET_H
