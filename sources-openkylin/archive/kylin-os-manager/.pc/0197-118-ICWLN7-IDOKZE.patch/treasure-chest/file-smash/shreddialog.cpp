/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "shreddialog.h"
#include "kalertdialog.h"
#include "buried_point.hpp"
#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>
#include <QToolTip>
#include <QFileDialog>
#include "cmyfiledialog.h"
// ShredDialog::ShredDialog(ShredManager *plugin, QDialog *parent)
//     :QDialog(parent)
ShredDialog::ShredDialog(QWidget *parent) : KWidget(parent), mousePressed(false)
{

    //首先设置大小
    this->setFixedSize(904, 568);
    //设置icon和标题
    this->setIcon("kylin-file-crush");
    this->setWidgetName(tr("File Crush"));
    this->setWindowTitle(tr("File Crush"));
    this->setWindowIcon(QIcon::fromTheme("kylin-file-crush"));

    //设置最大化和最小化不可见
    windowButtonBar()->maximumButton()->setVisible(false);
    //    windowButtonBar()->minimumButton()->setVisible(false);
    windowButtonBar()->menuButton()->setVisible(false);
    sideBar()->hide();
    //设置主布局
    mLabelTipIcon = new QPushButton(this);
    mLabelTipIcon->setFixedSize(24, 24);
    if (QIcon::hasThemeIcon("dialog-info")) {
        mLabelTipIcon->setIcon(QIcon::fromTheme("dialog-info"));
    } else {
        qWarning() << "ShredDialog::ShredDialog 无系统Icon！";
    }

    mLabelTipIcon->setIconSize(QSize(24, 24));
    mLabelTipIcon->setFlat(true);
    mLabelTipIcon->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    m_shredingTip = new KBallonTip();
    m_shredingTip->setWindowFlag(Qt::FramelessWindowHint);
    m_shredingTip->setAttribute(Qt::WA_TranslucentBackground);
    m_shredingTip->setFixedHeight(56);

    m_shredingTip->setText(tr("File shredding, please wait a moment"));
    m_shredingTip->setTipType(TipType::Warning);
    // m_shredingTip->setWindowFlag(Qt::Tool);
    m_shredingTip->setWindowModality(Qt::WindowModal);
    m_shredingTip->setWindowModality(Qt::ApplicationModal);
    m_shredingTip->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    m_shredingTip->hide();

    m_divider = new Divider(this->baseBar());
    m_divider->setFixedSize(856, 352);
    connect(m_divider, &Divider::openFileDialog, this, &ShredDialog::needOpenFileDialog);
    connect(m_divider, &Divider::sendFiles, this, &ShredDialog::addFileToItemShowTip);

    // 添加计时器，无任何触发重新计时，计时1.5秒后关闭弹窗
    m_timer = new QTimer(this);
    m_timer->setInterval(1500);
    connect(m_timer, &QTimer::timeout, this, [=](){
        m_timer->stop();
        m_shredingTip->hide();
    });

    // 使用SDK的拖拽组件
    // m_divider = new KDragWidget(this->baseBar());
    // m_divider->setFixedSize(856, 352);
    // connect(m_divider, &KDragWidget::getPath, [=](QString filePath){
    //     if (m_shredingTip->isHidden()) {
    //         m_shredingTip->setText(tr("Loading file information... "));
    //         m_shredingTip->setTipType(TipType::Warning);
    //         m_shredingTip->show();
    //     }
    //     QTimer::singleShot(500,[=](){ signalShowAddTip(filePath); }) ;
    // });

    m_tableDivider = new TableDivider(this->baseBar());
    connect(m_tableDivider, &TableDivider::sendFiles, this, &ShredDialog::addFileToItemShowTip);
    // connect(m_tableDivider, &TableDivider::sendFiles, this, &ShredDialog::acceptDrogFiles);
    m_fileTable = new QTreeWidget(m_tableDivider);

    initFileTable();
    m_fileTable->move(m_tableDivider->width() / 2 - m_fileTable->width() / 2,
                      m_tableDivider->height() / 2 - m_fileTable->height() / 2);

    mLabelTipText = new QLabel(this);
    QFont font;
    font.setWeight(QFont::Medium);
    mLabelTipText->setFont(font);
    mLabelFilePath = new QLineEdit(this);
    mLabelFilePath->setReadOnly(true);
    mLabelFilePath->setPlaceholderText(tr("No file selected to be shredded"));

    shred_btn = new QPushButton();
    shred_btn->resize(96, 36);
    shred_btn->setObjectName("blackButton");
    shred_btn->setFocusPolicy(Qt::NoFocus);
    //    shred_btn->setProperty("isImportant", true);
    cacel_btn = new QPushButton();
    cacel_btn->resize(96, 36);
    cacel_btn->setObjectName("blackButton");
    cacel_btn->setFocusPolicy(Qt::NoFocus);
    mBtnAddFile = new QPushButton();
    mBtnAddFile->resize(96, 36);
    mBtnAddFile->setFocusPolicy(Qt::NoFocus);
    mBtnAddFile->setProperty("isImportant", true);
    shred_btn->setEnabled(false);
    cacel_btn->setEnabled(false);

    QAction *allSelected = new QAction(tr("select all"), this);
    connect(allSelected, &QAction::triggered, this, &ShredDialog::selectAll);
    //右键菜单
    m_imageMenu = new QMenu(this);
    m_imageMenu->addAction(allSelected);

    tipLabel = new QLabel();
    //    tipLabel->setFixedSize(400,34);
    //    tipLabel->setWordWrap(true);
    tipLabel->setIndent(5);
    tipLabel->setAlignment(Qt::AlignLeft);
    QPalette pal;
    QColor color = palette().color(QPalette::PlaceholderText);
    pal.setColor(QPalette::WindowText, color);
    tipLabel->setPalette(pal);

    QHBoxLayout *layout1 = new QHBoxLayout();
    layout1->setContentsMargins(0, 0, 0, 0);
    layout1->setSpacing(0);
    layout1->setMargin(0);
    layout1->addWidget(mLabelTipIcon);
    layout1->addSpacing(8);
    layout1->addWidget(mLabelTipText);
    layout1->addStretch();

    QHBoxLayout *layout2 = new QHBoxLayout();
    layout2->setContentsMargins(0, 0, 0, 0);
    layout2->setSpacing(0);
    layout2->setMargin(0);
    layout2->addWidget(m_divider);
    layout2->addWidget(m_tableDivider);

    QHBoxLayout *layout3 = new QHBoxLayout();
    layout3->setContentsMargins(0, 0, 0, 0);
    layout3->setSpacing(0);
    layout3->setMargin(0);
    layout3->addWidget(tipLabel);
    layout3->addStretch();

    QHBoxLayout *layout4 = new QHBoxLayout();
    layout4->setContentsMargins(0, 0, 0, 0);
    layout4->setSpacing(0);
    layout4->setMargin(0);
    layout4->addWidget(mBtnAddFile);
    layout4->addSpacing(16);
    layout4->addWidget(cacel_btn);
    layout4->addStretch();
    layout4->addWidget(shred_btn);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(24, 0, 24, 24);
    layout->setSpacing(0);
    //    layout->addStretch();
    layout->addSpacing(5);
    layout->addLayout(layout1);
    layout->addSpacing(12);
    layout->addLayout(layout2);
    layout->addSpacing(8);
    layout->addLayout(layout3);
    layout->addSpacing(14);
    layout->addLayout(layout4);
    //    layout->addStretch();
    baseBar()->setLayout(layout);

    //    progressbar->setVisible(false);

    alertDialog = new KAlertDialog(this);
    connect(alertDialog, &KAlertDialog::sigCloseDialog, [this]() {
        //        qDebug() << "接收到警示框的文件提示信号";
        mLabelFilePath->setText("");
    });
    mLabelFilePath->hide();
    //    m_fileTable->hide();
    m_tableDivider->hide();
    sureDialog = new KSureDialog(this);
    connect(sureDialog, &KSureDialog::sigCloseDialog, this, &ShredDialog::slotSureDialog);
    connect(this, &ShredDialog::signalShowAddFileTip, this, &ShredDialog::acceptDrogFiles);
    connect(this, &ShredDialog::signalShowAddTip, this, &ShredDialog::acceptDrogOrChooseFile); // SDK拖拽组件使用
    // connect(this, &ShredDialog::signalShowAddFileTipStringList, this, &ShredDialog::acceptDrogFilesString);

    this->setLanguage();
    this->initConnect();
    this->moveCenter();
}
void ShredDialog::initFileTable()
{
    m_fileTable->setFrameStyle(QFrame::NoFrame);
    m_fileTable->setColumnCount(2);
    m_fileTable->setFixedSize(856, 352);
    m_fileTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // 将表格变为禁止编辑,取消焦点
    m_fileTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_fileTable->setFocusPolicy(Qt::NoFocus);
    // 设置表格不可选中
    m_fileTable->setSelectionMode(QAbstractItemView::MultiSelection);
    // 设置水平方向的表头标签与垂直方向上的表头标签，注意必须在初始化行列之后进行，否则，没有效果
    QList<QString> header = {tr("File Name"), tr("File Path")};
    m_fileTable->setHeaderLabels(header);
    m_fileTable->header()->setFixedHeight(36);

    //设置每列最小宽度
    m_fileTable->header()->setMinimumSectionSize(100);
    //设置交替背景色
    m_fileTable->setAlternatingRowColors(true);
    int column_width = 364;
    m_fileTable->setColumnWidth(0, column_width);
    m_fileTable->setUniformRowHeights(true);

    m_fileTable->setMouseTracking(true);
    m_fileTable->setRootIsDecorated(false);

    m_fileTable->setContextMenuPolicy(Qt::CustomContextMenu); // 开启右键单击目录
    m_fileTable->setSelectionMode(QAbstractItemView::NoSelection);
}
void ShredDialog::customContextMenu(const QPoint &pos)
{
    //    QTreeWidgetItem *pItem = nullptr;
    //    pItem = m_fileTable->itemAt(pos);      //获取点击的item，没有则为NULL
    //    if (pItem == nullptr) {                // 为空则为空白区域 显示创建菜单
    m_imageMenu->exec(QCursor::pos()); //弹出右键菜单，菜单位置为光标位置
    //    } else {
    //        m_imageMenu->exec(QCursor::pos()); //当前鼠标的位置
    //    }
}

void ShredDialog::selectAll()
{

    if (m_fileTable->topLevelItemCount() > 0) {
        QTreeWidgetItemIterator it(m_fileTable);
        int nodesCount = m_fileTable->topLevelItemCount();
        for (int i = 0; i < nodesCount; ++i) {

            if ((*it)->checkState(0) != Qt::Checked) {
                (*it)->setCheckState(0, Qt::Checked);
            }
            ++it;
        }
    }
}

void ShredDialog::onItemCLicked(QTreeWidgetItem *item, int indexCol)
{
    if (item->checkState(0) == Qt::Checked) {
        item->setCheckState(0, Qt::Unchecked);
    } else {
        item->setCheckState(0, Qt::Checked);
    }
}

QStringList ShredDialog::getAllSelectedFiles()
{
    QStringList selectedFiles;
    if (m_fileTable->topLevelItemCount() > 0) {
        QTreeWidgetItemIterator it(m_fileTable);
        int nodesCount = m_fileTable->topLevelItemCount();
        for (int i = 0; i < nodesCount; ++i) {

            if ((*it)->checkState(0) == Qt::Checked) {
                selectedFiles.append((*it)->whatsThis(0));
            }
            ++it;
        }
    }
    return selectedFiles;
}

void ShredDialog::removeSelectedFiles(QStringList selectedFiles)
{
    QList<int> itemNum;
    if (selectedFiles.length() > 0) {
        for (int i = 0; i < selectedFiles.length(); i++) {
            for (int j = 0; j < m_fileTable->topLevelItemCount(); j++) {
                if (selectedFiles.at(i) == m_fileTable->topLevelItem(j)->whatsThis(0)) {
                    itemNum.append(j);
                }
            }
        }
    }
    qSort(itemNum.begin(), itemNum.end());
    if (itemNum.length() > 0) {
        for (int j = 0; j < itemNum.length(); j++) {
            int index = itemNum.at(j) - j;
            if (index < 0 || index > m_fileTable->topLevelItemCount() - 1) {
                qInfo() << "table take file fail!";
                return;
            }
            m_fileTable->takeTopLevelItem(itemNum.at(j) - j);
        }
    }
}

void ShredDialog::controlUiDisplay()
{
    if (m_fileTable->topLevelItemCount() != 0) {
        if (m_tableDivider->isHidden()) {
            m_tableDivider->show();
            m_divider->show();
        }
    } else {
        if (m_divider->isHidden()) {
            m_divider->show();
            m_tableDivider->hide();
            cacel_btn->setEnabled(false);
            shred_btn->setEnabled(false);
            shred_btn->setProperty("isImportant", false);
        }
    }
}
//判断是否已经添加过了
bool ShredDialog::fileHasAdded(QString filepath)
{
    bool hasAdded = false;
    if (m_fileTable->topLevelItemCount() > 0) {
        QTreeWidgetItemIterator it(m_fileTable);
        int nodesCount = m_fileTable->topLevelItemCount();
        for (int i = 0; i < nodesCount; ++i) {
            if ((*it)->whatsThis(0) == filepath) {
                hasAdded = true;
            }
            ++it;
        }
    }
    return hasAdded;
}

void ShredDialog::changeBtnState()
{
    if (m_fileTable->topLevelItemCount() > 0) {
        cacel_btn->setEnabled(true);
        shred_btn->setEnabled(true);
        shred_btn->setProperty("isImportant", true);
    } else {
        cacel_btn->setEnabled(false);
        shred_btn->setEnabled(false);
        shred_btn->setProperty("isImportant", false);
    }
    if (!mBtnAddFile->isEnabled()) {
        mBtnAddFile->setEnabled(true);
    }
}
void ShredDialog::needOpenFileDialog() {}

void ShredDialog::insertFileData(QFileInfo fileInfo)
{
    // qDebug() << " === insert treewidget ";
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    item->setText(0, fileInfo.fileName());
    item->setText(1, fileInfo.absolutePath());
    item->setIcon(0, QIcon::fromTheme(m_iconName));
    item->setSizeHint(0, QSize(m_fileTable->columnWidth(0), 36));
    m_fileTable->addTopLevelItem(item);
    //    item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    item->setCheckState(0, Qt::Checked); //也可以是其他状态
    item->setWhatsThis(0, fileInfo.absoluteFilePath());
    m_fileTable->update();
    // qDebug() << " === insert treewidget over ";
}

void ShredDialog::addFileToItemShowTip(QList<QUrl> fileList)
{
    if (m_shredingTip->isHidden()) {
        m_shredingTip->setText(tr("Loading file information... "));
        m_shredingTip->setTipType(TipType::Warning);
        m_shredingTip->show();
    }
    QTimer::singleShot(500,[=](){ signalShowAddFileTip(fileList); }) ;
}

// 配合SDK的拖拽文件控件使用
void ShredDialog::acceptDrogOrChooseFile(QString filePath)
{
    // qDebug() << " = 文件列表数据加载中... = ";
    if(m_timer->isActive()) {
        m_timer->stop();
        m_timer->start();
    } else {
        m_timer->start();
    }
    addFileToTable(filePath.toLocal8Bit());
    changeBtnState();
    // qDebug() << " = 文件列表数据加载完成 = ";
}

void ShredDialog::acceptDrogFiles(QList<QUrl> fileList)
{
    // qDebug() << " = 文件列表数据加载中... = ";
    for (int i = 0; i < fileList.length(); i++) {

        QString filePath = fileList.at(i).path().toLocal8Bit();
        addFileToTable(filePath);
    }
    changeBtnState();
    m_shredingTip->hide();
    // qDebug() << " = 文件列表数据加载完成 = ";
}

void ShredDialog::addFileToTable(QString filePath)
{
    // qDebug() << " === " << filePath ;
    if (filePath.startsWith("file://")) {
        filePath = filePath.remove("file://");
    }
    if (filePath == "") {
        qDebug() << "file path is null!";
        return;
    }
    //判断是否重复添加
    if (fileHasAdded(filePath)) {
        qDebug() << "该文件已经添加过" << filePath;
        return;
    }

    QFileInfo file(filePath);
    // if (file.isDir()) {
    //     continue;
    // }
    if (file.exists()) {
        if (m_tableDivider->isHidden()) {
            m_divider->hide();
            m_tableDivider->show();
        }
        //对于含有特殊字符的路径，url特殊字符需转义：%#等。g_file_new_for_uri是never fail，所以不用判空
        GFile *fileg = g_file_new_for_uri(QUrl::fromLocalFile(filePath).toString().toUtf8().constData());
        //参数：1.--file；2.--文件属性 ""代表获取所有属性，access::* --
        //代表可获得的所有boolean属性；3.先设置默认属性；4.和5.nullptr即可
        GFileInfo *fileInfo = g_file_query_info(fileg, "access::*," G_FILE_ATTRIBUTE_ID_FILE,
                                                G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
        //判空-返回false
        if (!fileInfo) {
            changeBtnState();
            qDebug() << "file path is null!";
            return;
        }
        QUrl url = QUrl::fromLocalFile(filePath);
        if (!refreshInfoContents(url)) {
            return;
        } else {
            insertFileData(file);
        }
        g_object_unref(fileInfo);
        g_object_unref(fileg);
    } else {
        qDebug() << "drog file is not exist!" << filePath;
    }
}

bool ShredDialog::refreshInfoContents(QUrl url)
{
    // GFIle
    GError *error;
    GFile *file = g_file_new_for_path(url.path().toLocal8Bit());
    GFileInfo *file_info = g_file_query_info(file, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, &error);
    //判空-返回false
    if (!file_info) {
        qDebug() << "file path is null!";
        return false;
    }
    queryFileType(file_info);
    if (/*m_isDir || */ m_isVolume) {
        return false;
    }
    //获取部分文件属性
    m_isSymbolLink = g_file_info_get_attribute_boolean(file_info, G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK);
    if (g_file_info_has_attribute(file_info, G_FILE_ATTRIBUTE_ACCESS_CAN_READ)) {
        m_canRead = g_file_info_get_attribute_boolean(file_info, G_FILE_ATTRIBUTE_ACCESS_CAN_READ);
    } else {
        m_canRead = true;
    }

    if (g_file_info_has_attribute(file_info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE)) {
        m_canWrite = g_file_info_get_attribute_boolean(file_info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
    } else {
        m_canWrite = true;
    }

    if (g_file_info_has_attribute(file_info, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE)) {
        m_canExcute = g_file_info_get_attribute_boolean(file_info, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE);
    } else {
        m_canExcute = true;
    }

    m_canDelete = g_file_info_get_attribute_boolean(file_info, G_FILE_ATTRIBUTE_ACCESS_CAN_DELETE);

    //获取文件类型
    const char *content_type = g_file_info_get_content_type(file_info);

    //普通文件
    GIcon *icon = g_file_info_get_icon(file_info);
    if (G_IS_THEMED_ICON(icon)) {
        const gchar *const *icon_names = g_themed_icon_get_names(G_THEMED_ICON(icon));
        if (icon_names) {
            auto p = icon_names;
            while (*p) {
                QIcon icon = QIcon::fromTheme(*p);
                if (!icon.isNull()) {
                    m_iconName = QString(*p);
                    break;
                } else {
                    p++;
                }
            }
        }
    }

    // desktop
    if (m_iconName == "application-x-desktop" || QString(content_type) == "application/x-desktop") {
        auto _desktop_file = g_desktop_app_info_new_from_filename(url.path().toUtf8().constData());
        if (_desktop_file) {
            m_iconName = g_desktop_app_info_get_string(_desktop_file, "Icon");
        }
    }
    return true;
}
void ShredDialog::queryFileType(GFileInfo *new_info)
{

    m_isDir = false;
    m_isVolume = false;
    GFileType type = g_file_info_get_file_type(new_info);
    switch (type) {
    case G_FILE_TYPE_DIRECTORY:
        // qDebug()<<"dir";
        m_isDir = true;
        break;
    case G_FILE_TYPE_MOUNTABLE:
        // qDebug()<<"mountable";
        m_isVolume = true;
        break;
    default:
        break;
    }
}
ShredDialog::~ShredDialog()
{
    //    if (shredSettings != NULL)
    //    {
    //        shredSettings->sync();
    //        delete shredSettings;
    //        shredSettings = NULL;
    //    }
}

void ShredDialog::setLanguage()
{
    mLabelTipText->setText(tr("Completely crush files or folders that cannot be deleted"));
    shred_btn->setText(tr("Shred File"));
    cacel_btn->setText(tr("Deselect"));
    mBtnAddFile->setText(tr("Add File"));
    tipLabel->setText(tr("Note:File shredding cannot be cancelled, please exercise caution!"));
    //    barLabel->setText(tr("Shattering..."));
}

void ShredDialog::initConnect()
{
    connect(mBtnAddFile, SIGNAL(clicked()), this, SLOT(onSelectButtonClicked()));
    connect(shred_btn, SIGNAL(clicked()), this, SLOT(onShredButtonClicked()));
    connect(cacel_btn, SIGNAL(clicked()), this, SLOT(onCacelButtonClicked()));
    //    connect(title_bar, SIGNAL(showMinDialog()), this, SLOT(onMinButtonClicked()));
    connect(windowButtonBar()->closeButton(), SIGNAL(clicked()), this, SLOT(onCloseButtonClicked()));
    connect(m_divider, &Divider::openFileDialog, this, &ShredDialog::onSelectButtonClicked);
    connect(m_fileTable, &QTreeWidget::itemPressed, this, &ShredDialog::onItemCLicked);
    connect(m_fileTable, &QTreeWidget::itemDoubleClicked, this, &ShredDialog::onItemCLicked);
    connect(m_fileTable, &QTreeWidget::customContextMenuRequested, this, &ShredDialog::customContextMenu);
    connect(m_fileTable, &QTreeWidget::itemEntered, this, &ShredDialog::setTextToolTips);
}
void ShredDialog::setTextToolTips(QTreeWidgetItem *item, int indexCol)
{
    QString tempText = item->text(indexCol);

    QFontMetrics fontWidth = QFontMetrics(tempText);

    if (fontWidth.width(tempText) > m_fileTable->columnWidth(indexCol) - 10) {
        QToolTip ::showText(QCursor::pos(), tempText);
    } else {
        QToolTip ::showText(QCursor::pos(), "");
    }
}

QStringList ShredDialog::getAllFailFiles(QStringList allFails, bool onlyKeyPath)
{
    QStringList allFailFiles = {};
    if (m_choseKeyPathList.length() > 0) {
        for (int j = 0; j < m_choseKeyPathList.length(); j++) {
            QFileInfo keyfileInfo(m_choseKeyPathList.at(j));
            //if (keyfileInfo.isDir()) {
                //continue;
            //} else {
                m_failDirNum += 1;
                allFailFiles.append(m_choseKeyPathList.at(j));
            //}
        }
    }
    if (onlyKeyPath) {
        return allFailFiles;
    }
    if (allFails.length() > 0) {
        for (int i = 0; i < allFails.length(); i++) {
            QFileInfo fileInfo(allFails.at(i));
            //if (fileInfo.isDir()) {
                //continue;
            //} else {
                allFailFiles.append(allFails.at(i));
            //}
        }
    }


    return allFailFiles;
}

void ShredDialog::showFailDialog(QStringList failFilesPath)
{
    ShredFailInfoDialog *failTipDialog = new ShredFailInfoDialog(this);
    failTipDialog->insertFailContent(failFilesPath);
    failTipDialog->move(this->x() + this->width() / 2 - failTipDialog->width() / 2,
                        this->y() + this->height() / 2 - failTipDialog->height() / 2);
    failTipDialog->exec();
}

void ShredDialog::initDbus()
{
    for (int i = 0; i < 4; i++) {
        getKeyPathFromSys(i);
    }

    QString boxPath = QDir::homePath() + "/.box";

    if (m_keyPath.length() < 1) {
        m_keyPath = m_defaultKeyPath;
    } else {
        if (!m_keyPath.contains(boxPath)) {
            QFileInfo fileinfo(boxPath);
            if (fileinfo.exists()) {
                m_keyPath.append(boxPath);
            }
        }
    }
}

void ShredDialog::getKeyPathFromSys(int sourceType)
{
    QDBusMessage message =
        QDBusMessage::createMethodCall("com.kylin.kysdk.applicationsec", "/com/kylin/kysdk/applicationsec",
                                       "com.kylin.kysdk.applicationsec.classify", "getResourceList");
    message << sourceType;
    QDBusMessage response = QDBusConnection::systemBus().call(message);
    //判断method是否被正确返回
    if (response.type() == QDBusMessage::ReplyMessage) {

        //从返回参数获取返回值
        QJsonParseError jsonError;
        QJsonArray conf_data;
        QByteArray value = response.arguments().takeFirst().toByteArray();
        QJsonDocument json_doc = QJsonDocument::fromJson(value, &jsonError);
        if (jsonError.error == QJsonParseError::NoError) {
            conf_data = json_doc.array();
        } else {
            qWarning() << "Json error:" << sourceType << jsonError.errorString();
        }
        for (int i = 0; i < conf_data.size(); i++) {
            QJsonValue valuea = conf_data.at(i);
            QJsonArray valueArray = valuea.toArray();
            if (valueArray.size() > 0) {
                m_keyPath.append(valueArray.at(0).toString());
            }
        }
    } else {
        QString errorMessage = response.errorMessage();
        qWarning() << "QDBusMessage error " << sourceType << " :" << errorMessage;
    }
}
void ShredDialog::onCloseButtonClicked()
{
    this->close();
}

// void ShredDialog::progressbarFlash()
//{
//     if(i == 100){
//         progressbar->setInvertedAppearance(!(progressbar->invertedAppearance()));
//         i = 0;
//         progressbar->setValue(i);
//     }else{
//         progressbar->setValue(i);
//         i++;
//     }
// }

void ShredDialog::onSelectButtonClicked()
{
    //仅支持文件的filedialog - path
    //    QString fileName = QFileDialog::getOpenFileName(0, tr("Select file"), QDir::homePath(), tr("All Files(*)"));
    //    if (!fileName.isEmpty()) {
    //        mLabelFilePath->setText(fileName);

    //        if (QIcon::hasThemeIcon("dialog-info")) {
    //            mLabelTipIcon->setPixmap(QIcon::fromTheme("dialog-info").pixmap(QSize(24, 24)));
    //        } else {
    //            qWarning() << "ShredDialog::ShredDialog 无系统Icon！";
    //        }
    //        mLabelTipText->setText(tr("彻底粉碎无法删除的文件或文件夹"));
    //    }

    //    //仅支持文件的filedialog - url - 单选
    //    QUrl fileUrl = QFileDialog::getOpenFileUrl(0, tr("Select file"), "file://" + QDir::homePath(), tr("All
    //    Files(*)")); if (!fileUrl.isEmpty()) {
    //        QList<QUrl> fileList;
    //        fileList.append(fileUrl);
    //        acceptDrogFiles(fileList);
    //    }

    //仅支持文件的filedialog - url - 多选
    CMyFileDialog *dialog =new CMyFileDialog( "Custom scan", this);
    dialog->setFileMode(QFileDialog::ExistingFiles);
    QList<QUrl> fileUrlList;
    fileUrlList.clear();
    if(dialog->exec() == QDialog::Accepted){
        fileUrlList = dialog->selectedUrls();
        qDebug() << " ================================= "<< fileUrlList;
        addFileToItemShowTip(fileUrlList);
        dialog->close();
    }
    delete dialog;

    // QList<QUrl> fileUrlList =
    //     QFileDialog::getOpenFileUrls(nullptr, tr("Select file"), "file://" + QDir::homePath(), tr("All files") + " (*)");
    //     qDebug() << " ================================= "<< fileUrlList;
    // if (fileUrlList.length() > 0) {
    //     // acceptDrogFiles(fileUrlList);
    //     addFileToItemShowTip(fileUrlList);
    // }
}
void ShredDialog::slotSureDialog(bool isSure)
{
    if (isSure) {
        kdk::kabase::BuriedPoint buriedPoint;
        if (buriedPoint.functionBuriedPoint(kdk::kabase::AppName::KylinOsManager,
                                            kdk::kabase::BuriedPoint::PT::KylinOsManagerFileShredding)) {
            qCritical() << "buried point fail!pt:BaseInfo";
        }

        myThread = new ShredQThread(getAllSelectedFiles());
        thread = new QThread();
        myThread->moveToThread(thread);
        connect(myThread, &ShredQThread::success, this, [=] {
            qDebug() << "ShredDialog::slotSureDialog success!";
            thread->exit();
            removeSelectedFiles(getAllSelectedFiles());
            m_shredingTip->setText(tr("File shredding completed"));
            m_shredingTip->setTipType(TipType::Normal);
            m_shredingTip->setTipTime(3000);
            m_shredingTip->showInfo();
            changeBtnState();
            controlUiDisplay();
        });

        // connect(myThread, &ShredQThread::failed, this,
        //         [=](QStringList const &failFileName, QStringList const &successFileName) {
        //             qDebug() << "ShredDialog::slotSureDialog failed!" << failFileName;
        //             thread->exit();
        //             removeSelectedFiles(successFileName);
        //             m_shredingTip->hide();
        //             QMessageBox *error_messagebox = new QMessageBox(QMessageBox::Critical, tr("error"),
        //                                                             tr("File shredding failed"), QMessageBox::Ok, this);
        //             error_messagebox->exec();
        //             changeBtnState();
        //             controlUiDisplay();
        //             // myTimer.stop();
        //         });
        connect(myThread, &ShredQThread::failed, this, &ShredDialog::slotShredFileFail);

        connect(thread, &QThread::started, myThread, &ShredQThread::run);

        connect(thread, &QThread::finished, this, [=] {
            thread->deleteLater();
            qDebug() << "ShredQThread thread finished......";
        });
        m_failDirNum = 0;
        m_choseKeyPathList.clear();
        m_allFailFilePath.clear();
        thread->start();
        //开始进行文件粉碎
        m_shredingTip->setText(tr("File shredding, please wait a moment"));
        m_shredingTip->setTipType(TipType::Warning);
        m_shredingTip->show();
        QPoint windowCenter =
            QPoint(this->mapToGlobal(QPoint(0, 0)).x() + this->width() / 2, this->mapToGlobal(QPoint(0, 0)).y());
        // m_shredingTip->move(windowCenter.x() - m_shredingTip->width() / 2, windowCenter.y());
        cacel_btn->setEnabled(false);
        shred_btn->setEnabled(false);
        mBtnAddFile->setEnabled(false);
    }
}

void ShredDialog::slotShredFileFail(QStringList failFileName, QStringList successFileName, QStringList allFailFiles)
{
    qDebug() << "ShredDialog::slotSureDialog failed!" << failFileName;
    thread->exit();
    removeSelectedFiles(successFileName);
    m_shredingTip->hide();
    m_allFailFilePath = getAllFailFiles(allFailFiles);
    QMessageBox *error_messagebox = new QMessageBox(QMessageBox::Critical, tr("error"),
                                                    QString::number(allFailFiles.length()) + QString(tr(" ")) + tr(" File shredding failed"),
                                                    QMessageBox::NoButton, this);
    error_messagebox->addButton(tr("cancel"), QMessageBox::NoRole);
    error_messagebox->addButton(tr("view info"), QMessageBox::YesRole);
    connect(error_messagebox, &QMessageBox::buttonClicked, this, [=](QAbstractButton *button) {
        if (button->text() == tr("cancel")) {
            error_messagebox->close();
        }
        if (button->text() == tr("view info")) {
            error_messagebox->close();
            showFailDialog(m_allFailFilePath);
        }
    });
    error_messagebox->exec();
    changeBtnState();
    controlUiDisplay();
}

void ShredDialog::onShredButtonClicked()
{

    if (getAllSelectedFiles().length() <= 0) {
        if (alertDialog) {
            alertDialog->setAlertText(tr("Please select the file to shred!"));
            alertDialog->exec();
        }
    } else {
        sureDialog->exec();
    }
}

void ShredDialog::onCacelButtonClicked()
{
    mLabelFilePath->clear();
    QList<int> itemNum;
    for (int i = 0; i < m_fileTable->topLevelItemCount(); i++) {
        if (m_fileTable->topLevelItem(i)->checkState(0) == Qt::Checked) {
            itemNum.append(i);
        }
    }
    if (itemNum.length() > 0) {
        for (int j = 0; j < itemNum.length(); j++) {
            int index = itemNum.at(j) - j;
            if (index < 0 || index > m_fileTable->topLevelItemCount() - 1) {
                qInfo() << "table take file fail!";
                return;
            }
            m_fileTable->takeTopLevelItem(itemNum.at(j) - j);
        }
    } else {
        if (alertDialog) {
            alertDialog->setAlertText(tr("Please select the file to cacel!"));
            alertDialog->exec();
        }
    }
    //确定按钮状态
    changeBtnState();
    //确定界面显示
    controlUiDisplay();
}

void ShredDialog::closeEvent(QCloseEvent *event)
{
    event->accept();
    //  Q_EMIT SignalClose();
}

void ShredDialog::moveCenter()
{
    QPoint pos = QCursor::pos();
    QRect primaryGeometry;
    for (QScreen *screen : qApp->screens()) {
        if (screen->geometry().contains(pos)) {
            primaryGeometry = screen->geometry();
        }
    }

    if (primaryGeometry.isEmpty()) {
        primaryGeometry = qApp->primaryScreen()->geometry();
    }

    this->move(primaryGeometry.x() + (primaryGeometry.width() - this->width()) / 2,
               primaryGeometry.y() + (primaryGeometry.height() - this->height()) / 2);
}

void ShredDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->dragPosition = event->globalPos() - frameGeometry().topLeft();
        this->mousePressed = true;
    }
    QWidget::mousePressEvent(event);
}

void ShredDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->mousePressed = false;
    }

    QWidget::mouseReleaseEvent(event);
}

void ShredDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    //    QPainterPath path;
    //    path.setFillRule(Qt::WindingFill);
    //    path.addRoundRect(10,10,this->width()-20,this->height()-20,5,5);
    //    QPainter painter(this);
    //    painter.setRenderHint(QPainter::Antialiasing,true);
    //    painter.fillPath(path,QBrush(Qt::white));
    //    QColor color(0,0,0,50);
    //    for(int i = 0 ; i < 10 ; ++i)
    //    {
    //        QPainterPath path;
    //        path.setFillRule(Qt::WindingFill);
    //        path.addRoundRect(10-i,10-i,this->width()-(10-i)*2,this->height()-(10-i)*2,5,5);
    //        color.setAlpha(150 - qSqrt(i)*50);
    //        painter.setPen(color);
    //        painter.drawPath(path);
    //    }

    QWidget::paintEvent(event);
}

void ShredDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (this->mousePressed) {
        move(event->globalPos() - this->dragPosition);
    }

    QWidget::mouseMoveEvent(event);
}

Divider::Divider(QWidget *parent)
{
    this->setFixedSize(856, 352);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel *tips = new QLabel(this);
    //点击“+”按钮或直接拖拽文件/文件夹到此处
    tips->setText(tr("Click the “+” button or directly drag and drop files here"));
    tips->setEnabled(false);
    KPushButton *btnOPen = new KPushButton(this);
    btnOPen->setFixedSize(56, 56);
    btnOPen->setBorderRadius(btnOPen->width() / 2);
    btnOPen->setTranslucent(true);
    btnOPen->setIcon(QIcon::fromTheme("list-add-symbolic"));
    btnOPen->setIconSize(QSize(24, 24));

    layout->setSpacing(0);
    layout->addSpacing(131);
    layout->addWidget(btnOPen, 0, Qt::AlignCenter);
    layout->addSpacing(11);
    layout->addWidget(tips, 0, Qt::AlignCenter);
    layout->addStretch();
    this->setLayout(layout);
    setAcceptDrops(true);
    connect(btnOPen, &KPushButton::clicked, this, &Divider::openFileDialog);
}

void Divider::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    QColor color = qApp->palette().color(QPalette::Window);
    p.save();
    p.setBrush(color);
    QPen pen;
    QVector<qreal> dashes;
    qreal space = 3;
    qreal line = 3;
    dashes << line << space;
    pen.setDashPattern(dashes);
    QColor colorPen = qApp->palette().color(QPalette::Midlight);
    pen.setColor(colorPen);
    p.setPen(pen);
    p.drawRoundedRect(QRect(0, 0, this->width() - 1, this->height() - 2), 6, 6);
    p.restore();
    return QFrame::paintEvent(event);
}


void Divider::dragEnterEvent(QDragEnterEvent *event)
{

    // 拖动进入事件-屏蔽掉文件夹
    if (event->mimeData()->hasUrls()) // 数据中是否包含URL
    {
        if (event->mimeData()->urls().length() == 1) {

            QString filePath = event->mimeData()->urls().at(0).path().toLocal8Bit();
            if (filePath.startsWith("file://")) {
                filePath = filePath.remove("file://");
            }
            // QFileInfo fileInfo(filePath);
            // if (fileInfo.isDir()) {
            //     event->ignore();
            //     return;
            // }
        }
        event->acceptProposedAction(); // 如果数据中包含URL，就接收动作
    } else {
        event->ignore(); // 如果数据中不包含URL，就忽略该事件
    }
}

void Divider::dropEvent(QDropEvent *event)
{
    // 放下事件
    const QMimeData *mimeData = event->mimeData(); // 获取MIME数据
    if (mimeData->hasUrls())                       // 如果数据中包含URL
    {
        Q_EMIT sendFiles(mimeData->urls());
    }
}

TableDivider::TableDivider(QWidget *parent)
{
    this->setFixedSize(863, 359);
    setAcceptDrops(true);
}

void TableDivider::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    QColor color = qApp->palette().color(QPalette::Base);
    p.save();
    p.setBrush(color);
    QPen pen;
    //    QVector<qreal> dashes;
    //    qreal space = 3;
    //    qreal line = 3;
    //    dashes << line << space;
    //    pen.setDashPattern(dashes);
    QColor colorPen = qApp->palette().color(QPalette::Midlight);
    pen.setColor(colorPen);
    p.setPen(pen);
    p.drawRoundedRect(QRect(0, 0, this->width() - 2, this->height() - 2), 6, 6);
    p.restore();
    return QFrame::paintEvent(event);
}

void TableDivider::dragEnterEvent(QDragEnterEvent *drag_enter_event) {
    const auto *mime_data = drag_enter_event->mimeData();
    if (mime_data->hasUrls()) {
        const auto urls = mime_data->urls();
        if (urls.size() == 1) {
            // QFileInfo file_info(urls.at(0).toLocalFile());
            // if (file_info.isDir()) {
            //     drag_enter_event->ignore();
            //     return;
            // }
        }
        drag_enter_event->acceptProposedAction();
    } else {
        drag_enter_event->ignore();
    }
}

void TableDivider::dropEvent(QDropEvent *drop_event) {
    const auto *mime_data = drop_event->mimeData();
    if (mime_data->hasUrls()) {
        const auto urls = mime_data->urls();
        // bool has_dir = false;
        // for (const auto &u: urls) {
        //     QFileInfo file_info(u.toLocalFile());
        //     if (file_info.isDir()) {
        //         has_dir = true;
        //         break;
        //     }
        // }

        // if (has_dir) {
        //     // TODO: 弹窗提醒用户，目录不被支持已被过滤掉
        // }

        Q_EMIT sendFiles(urls);
    }
}
