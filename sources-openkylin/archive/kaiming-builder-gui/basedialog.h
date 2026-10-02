#ifndef BASEDIALOG_H
#define BASEDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QVector>
#include <QLineEdit>

#include "logic/KMBuilderApi.h"
#include "logic/KMRef.h"
#include "packagelistdialog.h"

class BaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BaseDialog(QWidget *parent = nullptr);
    KMRef getSelectedRef() const; // 返回用户选中的 KMRef
    void loadData(const QString &kind, const QString &base = "", const QString &runtime = ""); // 加载数据到表格

private slots:
    void onSearchButtonClicked();

private:
    void initUI(); // 初始化 UI

    QVector<KMRef> fetchDataFromAPI(const QString &kind, const QString &base = "", const QString &runtime = ""); // 根据 kind 获取数据
    void onPackageListButtonClicked(const QString &kind, const QString &channel, const QString &id, const QString &version);
    void displayData(const QVector<KMRef>&data, const QString &kind);  //在table中展示数据

    QTableWidget *tableWidget; // 表格控件
    KMRef selectedRef; // 用户选中的 KMRef

    QLineEdit *searchLineEdit; // 搜索框
    QPushButton *searchButton; // 搜索按钮

private:
    QVector<KMRef> m_refs;
    QString m_kind;
};

#endif // BASEDIALOG_H
