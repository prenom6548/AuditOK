#ifndef ACCEPTMIGRATIONAPPDATA_H
#define ACCEPTMIGRATIONAPPDATA_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <QStandardItem>
#include <QCloseEvent>
#include "../component/text_label.h"
#include "kwidget.h"

using namespace kdk;

const QString desktopFilePath = "/fs.old/usr/share/applications/";
const QString desktopListAllowFilePath = "/fs.old/usr/share/applications/replacement_destktop.list";
const QString DesktopGroup = "Desktop Entry";
const QString DesktopKeyNameEN = "Name";
const QString DesktopKeyNameZH = "Name[zh_CN]";
const QString DesktopKeyCommentEN = "Comment";
const QString DesktopKeyCommentZH = "Comment[zh_CN]";
#define ITEM_DATA_CHECKBOX_STATE Qt::UserRole + 7 //复选框状态

class AcceptMigrationAppData : public KWidget
{
    Q_OBJECT
public:
    AcceptMigrationAppData(QWidget *parent = nullptr);
    ~AcceptMigrationAppData();
    void initTreewidgetStyle();


    // 窗口关闭事件
    void closeEvent(QCloseEvent *event);

private:
    void initUI();
    void initTreeWidget();
    void clickSureBtn();
    void clickCannelBtn();
    bool createJsonFile();
    void writeJsonFile();
    void readJsonFile();
    void readSenderDesktopAllowMigrationFile();

    K::TextLabel *m_label = nullptr;
    QPushButton *m_btnSure = nullptr;
    QPushButton *m_btnCannel = nullptr;
    QTreeWidget *m_treeWidget = nullptr;

    QVBoxLayout *m_vlayout = nullptr;
    QTreeWidgetItem *m_currentItem = nullptr;

    QHash <QTreeWidgetItem *, QString> m_itemHash;
    QStringList m_list;
    QStringList m_listApp;
    QStringList m_listNameContains;
    QStringList m_listIncompatible;
    QStringList m_listAllowDesktopFile;

public slots:
    void treeItemChanged(QTreeWidgetItem *item, int column);
signals:
    void sigAppData(QStringList list);


};
#endif