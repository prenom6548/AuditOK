#ifndef PACKAGELISTDIALOG_H
#define PACKAGELISTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>

class PackageListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PackageListDialog(const QString &kind, const QString &channel, const QString &id, const QString &version, QWidget *parent = nullptr);

private slots:
    void onSearchButtonClicked();

private:
    void initUI();
    void loadPackageList(const QString &packageName);

    QString m_kind;
    QString m_channel;
    QString m_id;
    QString m_version;

    QLineEdit *searchLineEdit; // 搜索框
    QTableWidget *packageTableWidget; // 包列表表格
    QPushButton *searchButton; // 搜索按钮
};

#endif // PACKAGELISTDIALOG_H
