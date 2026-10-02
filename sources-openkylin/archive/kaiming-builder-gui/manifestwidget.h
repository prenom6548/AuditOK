#ifndef MANIFESTWIDGET_H
#define MANIFESTWIDGET_H

#include <QWidget>
#include <QScopedPointer>

#include <packageframe.h>
#include <baseinfoframe.h>
#include <setupinfoframe.h>
#include "logic/KMBuilderApi.h"
//#include "permissiondialog.h"
#include "permissionwidget.h"

class ManifestWidgetPrivate;

enum OperationFlag {
    New,
    Load
};

/**
 * @brief 清单配置界面
 */
class ManifestWidget : public QWidget
{
    Q_OBJECT
public:
    ManifestWidget(QWidget *parent = nullptr);
    virtual ~ManifestWidget();

    bool fetchSources();

    void setOperationFlag(OperationFlag flag);

    void setEditingIndex(int index); // 设置当前编辑的组件索引
    int editingIndex() const;        // 获取当前编辑的组件索引
    void getManifest();
    void clearAll(const QString &path, const QString &id);
    void load(const KMBuilderManifest &manifest, const QString &fileName); // 加载数据

    const KMBuilderManifest::Permission& getPermissionData();
    void setPermissionData(const KMBuilderManifest::Permission& permisson);

signals:
    void buildButtonClickedForNew(QString id);
    void buildButtonClickedForLoad(QString fileName);
    void addComponentClicked();
    void editComponentClicked(int index, const KMBuilderManifest::Component &component);

    void back();
    void next();
public slots:
   void handleComponentSaved(const KMBuilderManifest::Component &component);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    Q_DECLARE_PRIVATE(ManifestWidget)
    QScopedPointer<ManifestWidgetPrivate> d_ptr;
};

#endif // MANIFESTWIDGET_H
