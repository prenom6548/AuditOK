
#include "scanfileslistpage.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QToolTip>
#include <QMimeDatabase>
#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <peony-qt/file-utils.h>
#include "peony-qt/file-info.h"
#include "peony-qt/file-info-job.h"

namespace systemcleanup
{

ScanFilesListPage::ScanFilesListPage(QWidget *parent, bool showMd5)
    : QWidget(parent)
    , m_labelSelect(new K::TextLabel(this))
    , m_labelSelectSize(new K::TextLabel(this))
    , m_labelTotalSize(new K::TextLabel(this))
    , m_labelInfo(new K::TextLabel(this))
    , m_tableWidget(new QTreeWidget(this))
    , m_tableHeaderView(new TableHeaderView(Qt::Horizontal, m_tableWidget))
    , m_btnBack(new QPushButton(this))
    , m_btnCleanUp(new QPushButton(this))
    , m_menuTimeInterval(new QMenu(this))
    , m_showMd5(showMd5)
{
    this->setBackgroundRole(QPalette::Base);
    m_strBtnBack = tr("Return");
    m_strBtnClearUp = tr("Clean Up");
    m_strLabelChoosed = tr("Selected ");
    m_strLabelInfo = tr("May contain important files, please delete with caution");
    m_strTableFileName = tr("File Name");
    m_strTableFileType = tr("Format"); 
    m_strTableFileTime = tr("Time");
    m_strTableFileSize = tr("Size");
    m_strTableFileMd5 = tr("MD5");
    m_strMenuTimeIntervalAll = tr("All");
    m_strMenuTimeIntervalLastOneWeek = tr("Last Week");
    m_strMenuTimeIntervalOneWeekAgo = tr("One Week Ago");
    m_strMenuTimeIntervalOneMonthAgo = tr("One Month Ago");
    m_strMenuTimeIntervalThreeMonthAgo = tr("Three Months Ago");
    m_strMenuTreeWidgetRightActionOpenPath = tr("Open File Path");

    m_btnBack->setText(m_strBtnBack);
    m_btnBack->setProperty("useButtonPalette", true);
    m_btnBack->setProperty("isWindowButton", 0x1);
    m_btnBack->setProperty("useIconHighlightEffect", 0x2);

    m_btnCleanUp->setText(m_strBtnClearUp);
    m_btnCleanUp->setProperty("isImportant", true);
    m_btnCleanUp->setEnabled(false);

    m_labelSelect->SetText(m_strLabelChoosed);

    m_labelSelectSize->SetText("0.0B");
    m_labelSelectSize->SetFontColor(QColor("#F68C27"));

    m_labelTotalSize->SetText(" /0.0B");

    m_labelInfo->SetText(m_strLabelInfo);
    m_labelInfo->SetFontColor(QColor("#8C8C8C"));

    QAction *actionAll = new QAction(m_strMenuTimeIntervalAll, this);
    QAction *actionLastOneWeek = new QAction(m_strMenuTimeIntervalLastOneWeek, this);
    QAction *actionOneWeekAgo = new QAction(m_strMenuTimeIntervalOneWeekAgo, this);
    QAction *actionOneMonthAgo = new QAction(m_strMenuTimeIntervalOneMonthAgo, this);
    QAction *actionThreeMonthAgo = new QAction(m_strMenuTimeIntervalThreeMonthAgo, this);
    m_menuTimeInterval->addAction(actionAll);
    m_menuTimeInterval->addAction(actionLastOneWeek);
    m_menuTimeInterval->addAction(actionOneWeekAgo);
    m_menuTimeInterval->addAction(actionOneMonthAgo);
    m_menuTimeInterval->addAction(actionThreeMonthAgo);
    actionAll->setCheckable(true);
    actionLastOneWeek->setCheckable(true);
    actionOneWeekAgo->setCheckable(true);
    actionOneMonthAgo->setCheckable(true);
    actionThreeMonthAgo->setCheckable(true);
    actionAll->setChecked(true);

    QHBoxLayout *hLayoutLabel = new QHBoxLayout;
    hLayoutLabel->setSpacing(0);
    hLayoutLabel->setMargin(0);
    hLayoutLabel->addWidget(m_labelSelect);
    hLayoutLabel->addWidget(m_labelSelectSize);
    hLayoutLabel->addWidget(m_labelTotalSize);
    hLayoutLabel->addSpacing(16);
    hLayoutLabel->addWidget(m_labelInfo);
    hLayoutLabel->addStretch();

    QHBoxLayout *hLayoutBtn = new QHBoxLayout;
    hLayoutBtn->setSpacing(0);
    hLayoutBtn->setMargin(0);
    hLayoutBtn->addStretch();
    hLayoutBtn->addWidget(m_btnBack);
    hLayoutBtn->addSpacing(16);
    hLayoutBtn->addWidget(m_btnCleanUp);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    vLayout->addLayout(hLayoutLabel);
    vLayout->addSpacing(24);
    vLayout->addWidget(m_tableWidget);
    vLayout->addSpacing(24);
    vLayout->addLayout(hLayoutBtn);
    vLayout->setContentsMargins(24, 16, 24, 24);

    this->setLayout(vLayout);

    initTableWidgetUi();
    // testTableWidgetItem();

    connect(m_btnBack, &QPushButton::clicked, this, &ScanFilesListPage::slotBackup);
    connect(m_btnCleanUp, &QPushButton::clicked, this, &ScanFilesListPage::slotCleanUp);
    connect(m_tableWidget, &QTreeWidget::itemEntered, this, &ScanFilesListPage::setTextToolTips);
    // connect(m_tableWidget->header(), &QHeaderView::sectionClicked, this, &ScanFilesListPage::slotTableSort);
    connect(m_tableWidget, &QTreeWidget::itemClicked, [=](QTreeWidgetItem *item){
        if(item->checkState(0)==Qt::Checked){
            if (m_hashChooseFileSize.contains(item)) {
                return;
            }
            m_hashChooseFileSize.insert(item, m_hashItemSize.value(item));
            m_chooseSize = m_chooseSize + m_hashItemSize.value(item);
        } else {
            if (m_hashChooseFileSize.contains(item)) {
                m_chooseSize = m_chooseSize - m_hashChooseFileSize.value(item);
                m_hashChooseFileSize.remove(item);
            }
        }
        QString chooseSizeStr = calculatedSize(m_chooseSize);
        m_labelSelectSize->SetText(chooseSizeStr);
        if (m_hashChooseFileSize.size() == 0) {
            Q_EMIT signalChangeHeaderCheckState(Qt::Unchecked);
            m_btnCleanUp->setEnabled(false);
        } else if(m_hashChooseFileSize.size() == m_hashItemPath.size()) {
            Q_EMIT signalChangeHeaderCheckState(Qt::Checked);
            m_btnCleanUp->setEnabled(true);
        } else {
            Q_EMIT signalChangeHeaderCheckState(Qt::PartiallyChecked);
            m_btnCleanUp->setEnabled(true);
        }

    });
    connect(m_tableHeaderView, &TableHeaderView::signal_headerCheckStateChanged, this, &ScanFilesListPage::slotCheckStateChange);
    connect(this, &ScanFilesListPage::signalChangeHeaderCheckState, m_tableHeaderView, &TableHeaderView::slot_checkStateChange);
    connect(m_tableHeaderView, &QHeaderView::sectionClicked, this, &ScanFilesListPage::slotTableSort);
    connect(m_menuTimeInterval,&QMenu::triggered, this, &ScanFilesListPage::slotMenuTimeIntervalChange);
    connect(m_tableWidget, &QTreeWidget::itemPressed, this, &ScanFilesListPage::slotItemRightClicked);
}

ScanFilesListPage::~ScanFilesListPage()
{
}

void ScanFilesListPage::initTableWidgetUi()
{
    m_tableWidget->setFixedSize(860, 382);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);// 将表格变为禁止编辑,取消焦点
    m_tableWidget->setFocusPolicy(Qt::NoFocus);
    m_tableWidget->setSelectionMode(QAbstractItemView::NoSelection);// 设置表格不可选中
    m_tableWidget->setDragEnabled(false);// 开启拖拽功能
    m_tableWidget->setDragDropMode(QAbstractItemView::NoDragDrop);// 设置控件内部移动
    m_tableWidget->setDefaultDropAction(Qt::IgnoreAction);// 设置移动模式
    m_tableWidget->setMouseTracking(true);
    m_tableWidget->setRootIsDecorated(false);
    m_tableWidget->setAlternatingRowColors(true);//设置交替背景色

    QList<QString> header;
    if (m_showMd5) {
        header = {m_strTableFileName, m_strTableFileType, m_strTableFileTime, m_strTableFileSize, m_strTableFileMd5};
    } else {
        header = {m_strTableFileName, m_strTableFileType, m_strTableFileTime, m_strTableFileSize};
    }
    m_tableWidget->setHeaderLabels(header);
    m_tableWidget->setHeader(m_tableHeaderView);
    if (m_showMd5) {
        // m_tableHeaderView->setSortIndicator(4, Qt::AscendingOrder);
    } else {
        m_tableHeaderView->setSortIndicator(2, Qt::AscendingOrder);
    }
    m_tableHeaderView->setSortIndicatorShown(true);
    m_tableHeaderView->setSectionsClickable(true);
    m_tableHeaderView->setDragEnabled(false);
    m_tableHeaderView->setAcceptDrops(false);
    m_tableHeaderView->setDefaultDropAction(Qt::IgnoreAction);
    m_tableHeaderView->setDragDropMode(QAbstractItemView::NoDragDrop);
    m_tableHeaderView->setSectionsMovable(false);
    m_tableHeaderView->setFixedHeight(40);
    m_tableHeaderView->setTextElideMode(Qt::ElideRight);
    // m_tableHeaderView->setDefaultAlignment(Qt::AlignLeft);
    m_tableHeaderView->setMinimumSectionSize(100); //设置每列最小宽度

    // 设置表头每个item宽度
    if (m_showMd5) {
        m_tableWidget->setColumnWidth(0, 320);
        m_tableWidget->setColumnWidth(1, 140);
        m_tableWidget->setColumnWidth(2, 130);
        m_tableWidget->setColumnWidth(3, 110);
        m_tableWidget->setColumnWidth(4, 150);
    } else {
        m_tableWidget->setColumnWidth(0, 410);
        m_tableWidget->setColumnWidth(1, 140);
        m_tableWidget->setColumnWidth(2, 130);
        m_tableWidget->setColumnWidth(3, 160);
    }

}

void ScanFilesListPage::testTableWidgetItem()
{
    //初始化数据
    //添加数据时，一定要设置行数，否则数据不会展示出来
    m_tableWidget->headerItem()->setFlags(m_tableWidget->headerItem()->flags()  | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
    m_tableWidget->headerItem()->setCheckState(0, Qt::Unchecked);

    for (int i = 0; i < 10; i++ ) {
        
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << QString::number(i) 
                                                                  << QString::number(i + 1) + QString::number(i) 
                                                                  << QString::number(i) + QString::number(i + 1) + QString::number(i)
                                                                  << QString::number(i + 1) + QString::number(i + 1) + QString::number(i));
        item->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
        item->setCheckState(0, Qt::Unchecked);
        m_tableWidget->addTopLevelItem(item);
        item->setSizeHint(0, QSize(m_tableWidget->columnWidth(1), 40));

    }
 
    return;
}

QString ScanFilesListPage::calculatedSize(_Float64 size)
{
    int num = 0;
    QString suffixStr = "B";
    if (size > 1024) {
        for (int i = 0; size > 1024; i++) {
            size = size / 1024;
            num = i + 1;
            if (num == 1) {
                suffixStr = "KB";
            } else if(num == 2) {
                suffixStr = "MB";
            } else if(num == 3) {
                suffixStr = "GB";
                break;
            }
        }
    }
    QString tmpSize = QString::number(size, 'f', 1) + suffixStr;
    return tmpSize;    
}

QString ScanFilesListPage::getTimeDiffDescription(QString pastTime)
{
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    // 将字符串转换为QDate对象
    QDate startDate = QDate::fromString(pastTime, Qt::ISODate);
    QDate endDate = QDate::fromString(currentTime, Qt::ISODate);
    
    // 计算两个日期之间的天数
    int daysBetweenDates = startDate.daysTo(endDate) + 1;
    qDebug() << pastTime << " daysBetweenDates: " << daysBetweenDates;

    if (daysBetweenDates <= 7) {
        return m_strMenuTimeIntervalLastOneWeek; // 最近一周
    } else if (daysBetweenDates < 30) {
        return m_strMenuTimeIntervalOneWeekAgo; // 一周以前
    } else if (daysBetweenDates < 90) {
        return m_strMenuTimeIntervalOneMonthAgo; // 一个月之前
    } else {
        return m_strMenuTimeIntervalThreeMonthAgo; // 三个月之前
    } 
}

QString ScanFilesListPage::getFileType(QString filePath)
{
    auto info = Peony::FileInfo::fromUri("file://" + filePath);
    QString mimeType = info->mimeType();
    if (mimeType.isEmpty()) {
        Peony::FileInfoJob job(info);
        job.querySync();
        mimeType = info->mimeType();
    }
    return info->fileType();
}

void ScanFilesListPage::addTableWidgetItem(QString path, QByteArray md5)
{
    //初始化数据
    m_tableHeaderView->setHeaderCheckBoxCheckEnable(true);
    //添加数据时，一定要设置行数，否则数据不会展示出来
    m_tableWidget->headerItem()->setFlags(m_tableWidget->headerItem()->flags()  | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
    m_tableWidget->headerItem()->setCheckState(0, Qt::Unchecked);

    QFileInfo fileInfo(path);
    // 计算大小
    _Float64 size = fileInfo.size();
    QString fileSize = calculatedSize(size);
    m_totalSize = m_totalSize + size;
    QString totalSize = calculatedSize(m_totalSize);
    m_labelTotalSize->SetText(" / " + totalSize);
    QTreeWidgetItem *item;
    if (m_showMd5) {
        item = new QTreeWidgetItem(QStringList() << fileInfo.fileName()
                                                << getFileType(path)
                                                << getTimeDiffDescription(fileInfo.lastModified().toString("yyyy-MM-dd"))
                                                << fileSize
                                                << QString::fromUtf8(md5));
    } else {
        item = new QTreeWidgetItem(QStringList() << fileInfo.fileName()
                                                << getFileType(path)
                                                << getTimeDiffDescription(fileInfo.lastModified().toString("yyyy-MM-dd"))
                                                << fileSize);
    }
    item->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);

    bool containBool = true;
    if (containBool) {
        item->setCheckState(0, Qt::Unchecked);
    } else {
        item->setCheckState(0, Qt::Checked);
    }
    QIcon fileIcon = QIcon();
    fileIcon = QIcon::fromTheme(Peony::FileUtils::getFileIconName("file://" + path, false),
                                            QIcon::fromTheme("text-x-generic"));
    item->setIcon(0, fileIcon);

    m_tableWidget->addTopLevelItem(item);
    item->setSizeHint(0, QSize(m_tableWidget->columnWidth(1), 40));
    
    // hash 存放 （item path）（item size）
    m_hashItemPath.insert(item, path);
    m_hashItemSize.insert(item, size);
    
    return;
}

void ScanFilesListPage::clearTableWidget()
{
    m_tableHeaderView->setHeaderCheckBoxCheckEnable(false);
    m_hashChooseFileSize.clear();
    m_labelSelectSize->SetText("0.0B");
    m_labelTotalSize->SetText(" / 0.0B");
    m_hashCleanFile.clear();
    m_hashItemPath.clear();
    m_tableWidget->clear();
    m_totalSize = 0;
    m_chooseSize = 0;
    m_searchSize = 0;
    if (m_hashCleanFile.size() > 0) {
        Q_EMIT signalChangeHeaderCheckState(Qt::PartiallyChecked);
        m_btnCleanUp->setEnabled(true);
    } else {
        Q_EMIT signalChangeHeaderCheckState(Qt::Unchecked);
        m_btnCleanUp->setEnabled(false);
    }
}

void ScanFilesListPage::sortTableWidget(int col, bool sortUpToDown)
{
    if (sortUpToDown) {
        m_tableWidget->sortItems(col, Qt::AscendingOrder);
    } else {
        m_tableWidget->sortItems(col, Qt::DescendingOrder);
    }
    
}

void ScanFilesListPage::setTextToolTips(QTreeWidgetItem *item, int indexCol)
{
    QString tempText = item->text(indexCol);
 
    QFont font = item->font(indexCol);
    QFontMetrics fontWidth = QFontMetrics(font);

    if (fontWidth.width(tempText) > m_tableWidget->columnWidth(indexCol) - 65) {
        QToolTip ::showText(QCursor::pos(), tempText);
    } else {
        QToolTip ::showText(QCursor::pos(), "");
    }
}

void ScanFilesListPage::slotTableSort(int col)
{
    if (col == 2) {
        QPoint globalP(m_tableWidget->mapToGlobal(QPoint(0, 0)));
        QPoint movePos(globalP.x() + m_tableHeaderView->sectionPosition(col), globalP.y() + 36);

        m_menuTimeInterval->move(movePos);
        m_menuTimeInterval->exec();
        m_tableWidget->sortItems(col, Qt::AscendingOrder);
        return;
    }
    if(sortUpDown)		//sortUpDown为自定义的一个bool变量，表示升序还是降序
    {
        m_tableWidget->sortItems(col, Qt::AscendingOrder);
        sortUpDown=false;
    } else {
        m_tableWidget->sortItems(col, Qt::DescendingOrder);
        sortUpDown=true;
    }
    m_tableWidget->update();
    this->update();
}

void ScanFilesListPage::slotCleanFileSuccess(QStringList list)
{
    // list 是成功清理的文件列表
    QList<QTreeWidgetItem *> itemList;
    QHash<QTreeWidgetItem *, QString>::const_iterator it = m_hashCleanFile.constBegin();
    while (it != m_hashCleanFile.constEnd()) {
        qDebug() << it.key() << ":" << it.value();
        if (list.contains(it.value())) {
            itemList.append(it.key());
        }
        ++it;
    }
    // 清空 m_hashItemPath 表中的 itemList 值含有 key
    // 3个hash表,文件大小表，文件路径表，勾选item表内容清理。移除item
    for (int i = 0; i < itemList.size(); i++) {
        m_tableWidget->removeItemWidget(itemList.at(i), 0);
        m_tableWidget->takeTopLevelItem(m_tableWidget->indexOfTopLevelItem(itemList[i]));
        m_chooseSize = m_chooseSize - m_hashItemSize.value(itemList[i]);
        m_hashItemPath.remove(itemList[i]);
        m_hashItemSize.remove(itemList[i]);
        m_hashCleanFile.remove(itemList[i]);
    }
    m_tableWidget->update();
    QString chooseSizeStr = calculatedSize(m_chooseSize);
    m_labelSelectSize->SetText(chooseSizeStr);

    if (m_hashCleanFile.size() > 0) {
        Q_EMIT signalChangeHeaderCheckState(Qt::PartiallyChecked);
        m_btnCleanUp->setEnabled(true);
    } else {
        Q_EMIT signalChangeHeaderCheckState(Qt::Unchecked);
        m_btnCleanUp->setEnabled(false);
    }
}

void ScanFilesListPage::slotBackup()
{
    Q_EMIT signalBtnBack();
    return;
}

void ScanFilesListPage::slotCleanUp()
{
    // 获取选中列表
    QStringList cleanFilePath;
    QTreeWidgetItemIterator it(m_tableWidget);
    while (*it) {
        if((*it)->checkState(0)==Qt::Checked){
            cleanFilePath.append(m_hashItemPath.value(*it)); // 选中列表
            m_hashCleanFile.insert(*it, m_hashItemPath.value(*it));
        }
        ++it;
    }
    Q_EMIT signalClearUp(cleanFilePath);
    return;
}

void ScanFilesListPage::slotCheckStateChange(int iState)
{
    QTreeWidgetItemIterator it(m_tableWidget);
    if(iState == Qt::Unchecked){
        m_btnCleanUp->setEnabled(false);
        m_chooseSize = 0;
        m_hashChooseFileSize.clear();
        while (*it) {
            (*it)->setCheckState(0, Qt::Unchecked);
            ++it;
        }
    }else if(iState == Qt::Checked){
        m_btnCleanUp->setEnabled(true);
        while (*it) {
            (*it)->setCheckState(0, Qt::Checked);
            if (!m_hashChooseFileSize.contains((*it))) {
                m_hashChooseFileSize.insert((*it), m_hashItemSize.value((*it)));
                m_chooseSize = m_chooseSize + m_hashItemSize.value((*it));
            }
            ++it;
        }
    }
    QString chooseSizeStr = calculatedSize(m_chooseSize);
    m_labelSelectSize->SetText(chooseSizeStr);

}

void ScanFilesListPage::slotMenuTimeIntervalChange(QAction *action)
{
    if (action->text() == m_strMenuTimeIntervalAll) {
        m_tableWidget->sortByColumn(2, Qt::AscendingOrder);
    }

    int iCount = m_menuTimeInterval->actions().size();
    for(int i = 0; i < iCount; i++){
        m_menuTimeInterval->actions().at(i)->setChecked(false);
    }

    action->setChecked(true);
    
    if(action->text().compare(m_strMenuTimeIntervalAll) == 0){
        slotFilterTreeWidget("");
    }else{
        slotFilterTreeWidget(action->text());
    }

}

// 假设treeWidget是你要筛选的QTreeWidget对象，filterText是你要搜索的文本
void ScanFilesListPage::slotFilterTreeWidget(QString str)
{
    for (int i = 0; i < m_tableWidget->topLevelItemCount(); ++i) {
        if (str == "") {
            m_tableWidget->topLevelItem(i)->setHidden(false);
            QString totalSize = calculatedSize(m_totalSize);
            m_labelTotalSize->SetText(" / " + totalSize);
        } else {
            recursiveFilter(m_tableWidget->topLevelItem(i), str);
        }
    }
}

void ScanFilesListPage::slotItemRightClicked(QTreeWidgetItem *item, int column)
{
    if (qApp->mouseButtons() == Qt::RightButton) {
        
        QTreeWidget* tree = item->treeWidget(); // 获取当前item所在的QTreeWidget
        QMenu* menu = new QMenu(tree);
        QAction* action = new QAction(m_strMenuTreeWidgetRightActionOpenPath); // 打开文件所在路径
        menu->addAction(action);
 
        // 为右键菜单上的QAction创建信号槽，添加对应的功能
        connect(action, &QAction::triggered, this, [=](){
            QString path = m_hashItemPath.value(item);
            QString folderPath = QFileInfo(path).absolutePath();
            QUrl url = QUrl::fromLocalFile(folderPath);
            QDesktopServices::openUrl(url);
        });
 
        // 右键菜单在鼠标点击的位置显示
        menu->exec(QCursor::pos());
    }
}

void ScanFilesListPage::recursiveFilter(QTreeWidgetItem *item, const QString &filterText)
{
    if (!item) return;
 
    bool match = false;
    if (item->text(2).contains(filterText, Qt::CaseInsensitive)) {
        match = true;
    }

    if (!match) {
        item->setHidden(true);
        // todo: 不符合筛选条件的item取消勾选，取消勾选后，需要更新m_chooseSize
        // if (item->checkState(0) == Qt::Checked) {
        //     item->setCheckState(0, Qt::Unchecked);
        //     m_tableWidget->itemClicked(item, 0);
        // }
    } else {
        m_searchSize = m_searchSize + m_hashItemSize.value(item);
        item->setHidden(false);
    }
    QString totalSize = calculatedSize(m_searchSize);
    m_labelTotalSize->SetText(" / " + totalSize);
}
}