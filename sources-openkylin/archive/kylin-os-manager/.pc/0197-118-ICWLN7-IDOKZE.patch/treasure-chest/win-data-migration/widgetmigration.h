#ifndef WIDGETMIGRATION_H
#define WIDGETMIGRATION_H

#include "dialogmessage.h"
#include "widgetcommon.h"
#include "applogic.h"
#include <QStandardItemModel>
#include <QLineEdit>
#include <QTreeView>
#include <QStandardPaths>

class WidgetMigration : public WidgetCommon
{
    Q_OBJECT

signals:
    void startMigration(const QString &path);

public:
    WidgetMigration(QWidget *parent = nullptr);
    void setModel(QStandardItemModel *itemModel);
    void setAppLogic(AppLogic *appLogic);
    void setCancelHandle(bool *cancelHandle);

private slots:
    void onSavebtnClick();
    void changelineEditSavePath();
    void cancelFileMigrate();
    void connetcResult(int code, QVariant var = QVariant());

private:
    void initUI();
    void initSavePath();
    void initDialog();
    bool eventFilter(QObject *obj, QEvent *e);
    AppLogic *m_appLogic = nullptr;
    DialogMessage *m_dialog = nullptr;
    QLineEdit *m_lineEditSavePath = nullptr;
    QTreeView *m_winDataTreeView = nullptr;
    QStandardItemModel *m_itemModel = nullptr;
    QString m_configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kylin-os-manager/win-data-migration/";
    QString m_configFileName = "win-data-migration.conf";
    QString m_customize = "";
    QPushButton *m_savebtn;
};

#endif // WIDGETMIGRATION_H
