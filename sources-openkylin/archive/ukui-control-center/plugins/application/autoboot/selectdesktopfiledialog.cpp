/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2025 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "selectdesktopfiledialog.h"
#include <kysdk/applications/accessinfohelper.h>
#include <kysdk/applications/klineframe.h>
#include <QDir>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QScrollBar>
#include <QItemSelectionModel>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

SelectDesktopFileDialog::SelectDesktopFileDialog(QWidget *parent) :
    KDialog(parent)
{
    initUi();
    initModel();
    initConnect();
    // 默认选中首项，Tab 进入列表时读屏能读到应用名
    if (m_proxyModel && m_proxyModel->rowCount() > 0) {
        const QModelIndex first = m_proxyModel->index(0, 0);
        m_appListView->setCurrentIndex(first);
        updateCurrentRow(first);
    }
}

SelectDesktopFileDialog::~SelectDesktopFileDialog() {

}

void SelectDesktopFileDialog::initUi()
{
    setWindowIcon("ukui-control-center");
    setWindowModality(Qt::ApplicationModal);
    setWindowTitle(tr("Auto Start"));
    setObjectName(QString::fromUtf8("SelectDesktopFileDialog"));
    setFixedSize(QSize(424, 562));

    m_selectDesktopFileframe = new QFrame(this);
    m_selectDesktopFileframe->setObjectName(QString::fromUtf8("frame"));
    m_selectDesktopFileframe->setFixedSize(QSize(376, 28));
    m_selectDesktopFileframe->setFrameShape(QFrame::StyledPanel);
    m_selectDesktopFileframe->setFrameShadow(QFrame::Raised);

    QHBoxLayout* horizontalLayout = new QHBoxLayout(m_selectDesktopFileframe);
    horizontalLayout->setSpacing(0);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    m_titleLable = new KLabel(this);
    m_titleLable->setText(tr("Select the applications to open automatically when you login."));
    KDK_EXTEND_ALL_INFO_FORMAT(m_titleLable, "Autoboot", "", "");
    m_selectDesktopFileframe->layout()->addWidget(m_titleLable);

    m_selectDesktopFileLayout = new QVBoxLayout(this);
    m_selectDesktopFileLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    m_selectDesktopFileLayout->setContentsMargins(24, 0, 24, 0);
    m_selectDesktopFileLayout->addWidget(m_selectDesktopFileframe);

    // 应用列表用 QListView：AT-SPI 为 list/list item，避免 QTableView 读成「表格/表格单元」
    m_appListView = new QListView(this);
    m_appListView->setObjectName(QString::fromUtf8("m_appListView"));
    m_appListView->setFixedSize(QSize(376, 348));
    m_appListView->setFrameShape(QFrame::Panel);
    m_appListView->verticalScrollBar()->setProperty("drawScrollBarGroove", true);
    m_appListView->setMouseTracking(true);
    m_appListView->setSelectionMode(QAbstractItemView::SingleSelection);
    // Tab 不在列表项间循环，便于跳到「选择其他应用程序」；列表内用方向键切换
    m_appListView->setTabKeyNavigation(false);
    m_appListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_appListView->setIconSize(QSize(24, 24));
    m_appListView->setUniformItemSizes(true);
    m_appListView->setSpacing(0);
    m_appListView->setStyleSheet(QStringLiteral("QListView::item{min-height:36px;}"));
    // 不用 KDK_ALL_INFO_FORMAT：会把 accessibleName 写成英文物件名（如 tableview）
    m_appListView->setAccessibleName(QStringLiteral(" "));

    m_selectDesktopFileMode = new QStandardItemModel(this);
    m_selectDesktopFileMode->setColumnCount(1);

    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_selectDesktopFileMode);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_appListView->setModel(m_proxyModel);

    m_toOtherDesktopButton = new KBorderlessButton(tr("Select other applications"), this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_toOtherDesktopButton, "Autoboot", "", "");
    // KBorderlessButton 构造时为 NoFocus，Tab 会跳过；打开 StrongFocus 才能进 Tab 链
    m_toOtherDesktopButton->setFocusPolicy(Qt::StrongFocus);
    m_toOtherDesktopButton->setAutoDefault(false);
    m_toOtherDesktopButton->setDefault(false);
    m_toOtherDesktopButton->setAccessibleName(tr("Select other applications"));
    m_selectDesktopFileLayout->addWidget(m_appListView);
    m_selectDesktopFileLayout->addWidget(m_toOtherDesktopButton);

    QVBoxLayout* m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, -1, 0, -1);

    m_mainLayout->addLayout(m_selectDesktopFileLayout);

    QHBoxLayout* m_lineLayout = new QHBoxLayout(this);
    m_lineLayout->setContentsMargins(0, -1, 0, -1);
    KHLineFrame *lineFrame = new KHLineFrame(this);
    m_lineLayout->addWidget(lineFrame);
    m_mainLayout->addLayout(m_lineLayout);

    QVBoxLayout* m_buttonLayout = new QVBoxLayout(this);
    m_buttonLayout->setContentsMargins(24, 8, 24, 16);
    QSpacerItem* verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_buttonLayout->addItem(verticalSpacer);

    m_cancelButton = new KPushButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_cancelButton, "Autoboot", "", "");
    m_cancelButton->setText(tr("Cancel"));
    m_cancelButton->setObjectName(QString::fromUtf8("m_cancelButton"));
    m_cancelButton->setMinimumSize(QSize(88, 0));
    m_cancelButton->setMaximumSize(QSize(88, 16777215));
    m_cancelButton->setAccessibleName(tr("Cancel"));
    m_cancelButton->setAutoDefault(false);

    m_confirmButton = new KPushButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_confirmButton, "Autoboot", "", "");
    m_confirmButton->setText(tr("Confirm"));
    m_confirmButton->setObjectName(QString::fromUtf8("yesBtn"));
    m_confirmButton->setMinimumSize(QSize(88, 0));
    m_confirmButton->setMaximumSize(QSize(88, 16777215));
    m_confirmButton->setBackgroundColorHighlight(true);
    m_confirmButton->setAccessibleName(tr("Confirm"));
    m_confirmButton->setDefault(true);

    QHBoxLayout* spacerHorizontalLayout = new QHBoxLayout();
    spacerHorizontalLayout->setSpacing(8);
    spacerHorizontalLayout->setObjectName(QString::fromUtf8("spacerHorizontalLayout"));
    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    spacerHorizontalLayout->addItem(horizontalSpacer);

    spacerHorizontalLayout->addWidget(m_cancelButton);
    spacerHorizontalLayout->addWidget(m_confirmButton);

    m_buttonLayout->addStretch();
    m_buttonLayout->addLayout(spacerHorizontalLayout);

    m_mainLayout->addLayout(m_buttonLayout);

    mainWidget()->setLayout(m_mainLayout);

    // 列表 → 选择其他应用程序 → 取消 → 确认
    QWidget::setTabOrder(m_appListView, m_toOtherDesktopButton);
    QWidget::setTabOrder(m_toOtherDesktopButton, m_cancelButton);
    QWidget::setTabOrder(m_cancelButton, m_confirmButton);
}

void SelectDesktopFileDialog::initModel()
{
    const QString desktops = QString::fromLocal8Bit(qgetenv("XDG_DATA_DIRS"));
    qDebug() << Q_FUNC_INFO << "XDG_DATA_DIRS" << desktops;
    const auto list = desktops.split(QLatin1Char(':'));
    QStringList appPathList;
    int row = 0;
    QList<QString> nameList;
    for (const QString &desktop : list) {
        QStringList pathlist = desktop.split(":");
        for (QString path : pathlist) {
            QString desktopDirStr = path + "/applications/";
            if (appPathList.contains(desktopDirStr)) {
                continue;
            }
            QDir desktopDir = QDir(desktopDirStr);
            if (!desktopDir.exists()) {
                qDebug() << Q_FUNC_INFO << "desktopDir not exist, skip" << desktopDirStr;
                continue;
            }
            foreach (QString filename, desktopDir.entryList(QDir::Files | QDir::Dirs)) {
                QString desktopPath = desktopDirStr + filename;
                QFileInfo fileInfo(desktopPath);
                if(fileInfo.isDir() || !filename.endsWith("desktop")) {
                    qDebug() << Q_FUNC_INFO << "not desktop file, skip" << desktopPath;
                    continue;
                }
                QByteArray ba;
                ba = desktopPath.toUtf8();

                GKeyFile *keyfile;
                keyfile = g_key_file_new();
                if (!g_key_file_load_from_file(keyfile, ba.data(), G_KEY_FILE_NONE, NULL)) {
                    g_key_file_free(keyfile);
                    qDebug() << Q_FUNC_INFO << "desktop file parse error, skip" << desktopPath;
                    continue;
                }

                QString mainSection = "Desktop Entry";
                KSharedConfig::Ptr sharedConfig = KSharedConfig::openConfig(desktopPath, KConfig::NoGlobals);
                KConfigGroup cfg(sharedConfig, mainSection);
                QString name = cfg.readEntry("Name");
                QString icon = cfg.readEntry("Icon");
                QString onlyShowIn = cfg.readEntry("OnlyShowIn");
                QString notShowIn = cfg.readEntry("NotShowIn");
                QString nodisplay = cfg.readEntry("NoDisplay");
                if (nodisplay == "true" || notShowIn.contains("UKUI")
                        || (!onlyShowIn.isEmpty() && !onlyShowIn.contains("UKUI"))) {
                    qDebug() << Q_FUNC_INFO << "desktop no show" << desktopPath << "NoDisplay" << nodisplay << "OnlyShowIn" << onlyShowIn << "NotShowIn" << notShowIn;
                    continue;
                }
                if (std::find(nameList.begin(), nameList.end(), name) != nameList.end()) {
                    qDebug() << Q_FUNC_INFO << "name already added, skip" << name << desktopPath;
                    continue;
                }
                nameList.append(name);
                QStandardItem *singleItem = nullptr;
                QFileInfo file = QFileInfo(icon);
                if (file.exists()) {
                    qDebug() << Q_FUNC_INFO << "file exist" << icon << name;
                    QPixmap pixmap = QPixmap(file.filePath());
                    if (!pixmap.isNull()) {
                        singleItem = new QStandardItem(QIcon(pixmap), name);
                    }
                }
                if (!singleItem) {
                    singleItem = new QStandardItem(QIcon::fromTheme(icon), name);
                }
                // 读屏聚焦列表项时只依赖应用名，避免再带表格角色
                singleItem->setData(name, Qt::AccessibleTextRole);
                m_selectDesktopFileMode->setItem(row++, singleItem);
                m_desktopPathList.append(desktopPath);
                qDebug() << "add desktopfile" << desktopPath << "name" << name << "index" << m_selectDesktopFileMode->rowCount() - 1;
            }
            appPathList.append(desktopDirStr);
            qDebug() << Q_FUNC_INFO << "finished add desktop dir" << desktopDirStr;
        }
    }
}

void SelectDesktopFileDialog::confirmButtonClickSlot() {
    if (m_currentRow < 0 || m_currentRow > m_desktopPathList.count() - 1) {
        qDebug() << Q_FUNC_INFO << "currentRow is error, return" << m_currentRow << m_desktopPathList;
        this->hide();
        return;
    }
    QString selectDesktopPath = m_desktopPathList.at(m_currentRow);
    qDebug() << Q_FUNC_INFO << "select desktop" << selectDesktopPath << "current row" << m_currentRow;
    Q_EMIT confirmed(selectDesktopPath);
    this->hide();
}

void SelectDesktopFileDialog::updateCurrentRow(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    m_currentRow = index.row();
    const QString name = index.data(Qt::DisplayRole).toString();
    // Tab 焦点落在列表容器时，读屏读列表 accessibleName；同步为当前选中应用名
    if (!name.isEmpty()) {
        m_appListView->setAccessibleName(name);
    }
    qDebug() << Q_FUNC_INFO << "select desktop index" << m_currentRow << name;
}

void SelectDesktopFileDialog::initConnect()
{
    connect(m_confirmButton, &KPushButton::clicked, this, [=]() {
        confirmButtonClickSlot();
    });

    connect(m_appListView, &QListView::clicked, this, [this](const QModelIndex &index) {
        updateCurrentRow(index);
    });

    // 方向键切换选中时同步行号与无障碍名称
    connect(m_appListView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current, const QModelIndex &) {
        updateCurrentRow(current);
    });

    connect(m_cancelButton, &KPushButton::clicked,this,[=]() {
        this->hide();
    });

    connect(m_toOtherDesktopButton, &KBorderlessButton::clicked, this, [=]() {
        this->hide();
        emit addOtherApp();
    });
}
