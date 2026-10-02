#ifndef CLOUDMODELITEM_H
#define CLOUDMODELITEM_H

#include <aiconfig/modelconfig.h>
#include <kysdk/applications/klabel.h>

#include <QGSettings>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QToolButton>
#include <QVariant>
#include <QWidget>

#define THEME_QT_SCHEMA  "org.ukui.style"

using namespace std;
using namespace kyai::config::model;

class ModelItem : public QWidget
{
    Q_OBJECT
public:
    explicit ModelItem(AiCapability capability, QString modelName, QString vendor, bool isTrialAccount,
                       QWidget* parent = nullptr);
    ~ModelItem();

    void showSelectedIcon(bool flag);
    AiCapability getAICapability();
    QString getModelName();
    bool isTrialModel();

signals:
    void sigUpdate(AiCapability capability, QString modelName);
    void sigUninstall(AiCapability capability, QString modelName);

private slots:

private:
    void initLayout();
    void initNameLabel();
    void initMoreButton();
    void initSelectedIconLabel();
    void initMenu();
    void initIconLabel();
    void initCertificationLabel();
    void setupThemeListener();              // 设置主题监听
    void updateIconForTheme(const QString &theme);  // 更新图标以适配主题
    QString getCurrentTheme();              // 获取当前主题
    void updateVendorIcon(QLabel* label, const QString &defaultIconPath, const QString &lightIconPath);  // 更新供应商图标
    
    AiCapability m_capability;
    QString m_modelName;
    QString m_vendor;
    bool m_isTrialAccount;

    QLabel* m_iconLabel = nullptr;
    QLabel* m_nameLabel = nullptr;
    kdk::KLabel* m_certificationLabel = nullptr;
    QToolButton* m_moreButton = nullptr;
    QLabel* m_selectedIconLabel = nullptr;
    QMenu* m_menu = nullptr;

    // 添加用于存储选中和未选中状态的Pixmap
    QPixmap m_unselectedPixmap;
    QPixmap m_selectedPixmap;
    
    QGSettings *m_themeSettings = nullptr;  // 主题设置对象
};

#endif  // CLOUDMODELITEM_H