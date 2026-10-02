#include "basedialog.h"
#include "logic/KMBuilderSearch.h"

BaseDialog::BaseDialog(QWidget *parent)
    : QDialog(parent)
{
    initUI();
}

void BaseDialog::initUI()
{
    setWindowTitle("Select Dependency");
    resize(600, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);
    //搜索框
    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLabel *idPackage = new QLabel(tr("ID"));
    searchLineEdit = new QLineEdit(this);
    searchButton = new QPushButton("🔍", this);
    searchLayout->addWidget(idPackage);
    searchLayout->addWidget(searchLineEdit);
    searchLayout->addWidget(searchButton);
    connect(searchButton, &QPushButton::clicked, this, &BaseDialog::onSearchButtonClicked);

    // 初始化表格
    tableWidget = new QTableWidget(this);
    tableWidget->setColumnCount(4); // 3 列：channel, id, version
    tableWidget->setHorizontalHeaderLabels({"Channel", "ID", "Version", "Package List"});
    // 隐藏垂直表头（第一列的序号）
    tableWidget->verticalHeader()->setVisible(false);
    // 设置表格不可编辑
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 设置列宽分配
    int channelWidth = 125; // channel 列固定宽度
    int versionWidth = 125; // version 列固定宽度
    int packageListWidth = 125;  //package list 列固定宽度

    tableWidget->setColumnWidth(0, channelWidth); // channel 列
    tableWidget->setColumnWidth(2, versionWidth); // version 列
    tableWidget->setColumnWidth(3, packageListWidth); //package list 列固定宽度

    // 让第二列（id 列）自由拉伸
    tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
//    tableWidget->horizontalHeader()->setStretchLastSection(true); // 最后一列自动拉伸
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection); // 单选模式
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows); // 选中整行

    layout->addLayout(searchLayout);
    layout->addWidget(tableWidget);

    // 确定按钮
    QPushButton *confirmButton = new QPushButton(tr("Confirm"), this);
    connect(confirmButton, &QPushButton::clicked, this, [this]() {
        // 获取选中的行
        QList<QTableWidgetItem *> selectedItems = tableWidget->selectedItems();
        if (!selectedItems.isEmpty()) {
            int row = selectedItems.first()->row();
            selectedRef.m_channel = tableWidget->item(row, 0)->text();
            selectedRef.m_id = tableWidget->item(row, 1)->text();
            selectedRef.m_version = tableWidget->item(row, 2)->text();
            selectedRef.local = (tableWidget->cellWidget(row, 3) != nullptr);
        }
        accept(); // 关闭对话框
    });

    layout->addWidget(confirmButton);

    setLayout(layout);
}

void BaseDialog::loadData(const QString &kind, const QString &base, const QString &runtime)
{
    // 从接口获取数据
    m_refs = fetchDataFromAPI(kind, base, runtime);
    m_kind = kind;
    displayData(m_refs, kind);
}

void BaseDialog::displayData(const QVector<KMRef> &data, const QString &kind)
{
    //设置标题
    setWindowTitle("Select " + m_kind);

    // 清空表格
    tableWidget->setRowCount(0);

    // 根据 kind 的值决定是否显示最后一列
    if (kind == "depend") {
        tableWidget->setColumnCount(3); // 隐藏最后一列
        tableWidget->setHorizontalHeaderLabels({"Channel", "ID", "Version"});
    } else {
        tableWidget->setColumnCount(4); // 显示所有列
        tableWidget->setHorizontalHeaderLabels({"Channel", "ID", "Version", "Package List"});
    }

    // 填充表格
    for (const KMRef &ref : data) {
        int row = tableWidget->rowCount();
        tableWidget->insertRow(row);

        tableWidget->setItem(row, 0, new QTableWidgetItem(ref.m_channel));
        tableWidget->setItem(row, 1, new QTableWidgetItem(ref.m_id));
        tableWidget->setItem(row, 2, new QTableWidgetItem(ref.m_version));

        if (kind != "depend" && ref.local)
        {
            // 如果 ref.local 为 true，插入按钮
            QPushButton *packageListButton = new QPushButton("Details", this);
            packageListButton->setStyleSheet("color: blue; border: none; text-decoration: underline;");
            connect(packageListButton, &QPushButton::clicked, this, [=]() {
                // 获取当前行的数据
                QString channel = tableWidget->item(row, 0)->text();
                QString id = tableWidget->item(row, 1)->text();
                QString version = tableWidget->item(row, 2)->text();

                // 处理按钮点击事件
                onPackageListButtonClicked(kind, channel, id, version);
            });

            tableWidget->setCellWidget(row, 3, packageListButton); // 将按钮插入到表格中
        } else if (kind != "depend"){
            // 如果 ref.local 为 false，插入文本“N/A”
            QTableWidgetItem *item = new QTableWidgetItem("N/A");
            item->setTextAlignment(Qt::AlignCenter); // 文本居中对齐
            tableWidget->setItem(row, 3, item);
        }
    }
}

QVector<KMRef> BaseDialog::fetchDataFromAPI(const QString &kind, const QString &base, const QString &runtime)
{
    // 从接口获取数据
    QVector<KMRef> data = KMBuilderSearch::search(kind, base, runtime);
    return data;
}

KMRef BaseDialog::getSelectedRef() const
{
    return selectedRef;
}

void BaseDialog::onPackageListButtonClicked(const QString &kind, const QString &channel, const QString &id, const QString &version)
{
    // 打开新的对话框显示包列表
    PackageListDialog *dialog = new PackageListDialog(kind, channel, id, version, this);
    dialog->exec(); // 模态显示对话框
}

void BaseDialog::onSearchButtonClicked()
{
    //获取过滤条件
    QString id = searchLineEdit->text();
    QVector<KMRef> data = KMBuilderApi::filter(m_refs, id); //过滤
    displayData(data, m_kind);
}
