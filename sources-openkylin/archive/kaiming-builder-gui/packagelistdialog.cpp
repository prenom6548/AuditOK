#include "packagelistdialog.h"
#include "logic/KMBuilderSearch.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDebug>

PackageListDialog::PackageListDialog(const QString &kind, const QString &channel, const QString &id, const QString &version, QWidget *parent)
    : QDialog(parent), m_kind(kind), m_channel(channel), m_id(id), m_version(version)
{
    initUI();
}

void PackageListDialog::initUI()
{
    setWindowTitle("Package List");
    resize(600, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);

    // 搜索框和搜索按钮
    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLabel *packageNameLable = new QLabel("Package Name");
    searchLineEdit = new QLineEdit(this);
    searchButton = new QPushButton("🔍", this);
    searchLayout->addWidget(packageNameLable);
    searchLayout->addWidget(searchLineEdit);
    searchLayout->addWidget(searchButton);

    // 包列表表格
    packageTableWidget = new QTableWidget(this);
    packageTableWidget->setColumnCount(2); // 假设两列：包名和版本
    packageTableWidget->setHorizontalHeaderLabels({"Package Name", "Version"});
    // 隐藏垂直表头（第一列的序号）
    packageTableWidget->verticalHeader()->setVisible(false);
    packageTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    packageTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 添加到布局
    layout->addLayout(searchLayout);
    layout->addWidget(packageTableWidget);

    setLayout(layout);

    // 连接搜索按钮点击事件
    connect(searchButton, &QPushButton::clicked, this, &PackageListDialog::onSearchButtonClicked);
}

void PackageListDialog::onSearchButtonClicked()
{
    QString packageName = searchLineEdit->text().trimmed();
    loadPackageList(packageName);
}

void PackageListDialog::loadPackageList(const QString &packageName)
{
    // 假设这里调用接口获取数据
     QVector<InstalledPackage> packages = KMBuilderSearch::searchPackage(packageName, m_kind, m_channel, m_id, m_version);

    // 清空表格
    packageTableWidget->setRowCount(0);

    // 填充表格
    for (const auto &package : packages) {
        int row = packageTableWidget->rowCount();
        packageTableWidget->insertRow(row);

        packageTableWidget->setItem(row, 0, new QTableWidgetItem(package.p_name));
        packageTableWidget->setItem(row, 1, new QTableWidgetItem(package.p_version));

    }
}
