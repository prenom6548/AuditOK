/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#include "addlanguagedialog.h"
#include <QToolTip>
#include <QCompleter>
#include <QHBoxLayout>
#include <QShowEvent>
#include <QScrollBar>
#include <QItemSelectionModel>
#include <kysdk/applications/accessinfohelper.h>

using namespace kdk;

AddLanguageDialog::AddLanguageDialog(QWidget *parent) :
    KDialog(parent),
    mSearchLineEdit(new KSearchLineEdit(this))
{
    initUi();
    initDict();
    initCompleter();
    initConnect();
}

AddLanguageDialog::~AddLanguageDialog()
{
}

// 初始化添加语言弹窗界面
void AddLanguageDialog::initUi()
{
    setWindowIcon("ukui-control-center");
    setWindowTitle(tr("Add Language"));
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(424, 520);

    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(16);
    m_mainLayout->setContentsMargins(24, 6, 24, 24);

    m_searchFrame = new QFrame(this);
    m_searchFrame->setFixedSize(376, 40);
    m_searchFrame->setFrameShape(QFrame::StyledPanel);
    m_searchFrame->setFrameShadow(QFrame::Raised);

    QHBoxLayout *searchLayout = new QHBoxLayout(m_searchFrame);
    searchLayout->setSpacing(0);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    mSearchLineEdit->setClearButtonEnabled(true);
    // KSearchLineEdit 默认为 ClickFocus，Tab 无法聚焦；StrongFocus 支持键盘 Tab 切换
    mSearchLineEdit->setFocusPolicy(Qt::StrongFocus);
    searchLayout->addWidget(mSearchLineEdit);

    // 语言列表用 QListView：与添加开机启动窗口一致，AT-SPI 为 list/list item，
    // 避免 QTableView 读成「表格/表格单元」。
    m_languageListView = new QListView(this);
    m_languageListView->setObjectName(QString::fromUtf8("m_languageListView"));
    m_languageListView->setMinimumWidth(376);
    m_languageListView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_languageListView->setFrameShape(QFrame::Panel);
    m_languageListView->verticalScrollBar()->setProperty("drawScrollBarGroove", true);
    m_languageListView->setMouseTracking(true);
    m_languageListView->setSelectionMode(QAbstractItemView::SingleSelection);
    // Tab 不在列表项间循环；列表内用方向键切换
    m_languageListView->setTabKeyNavigation(false);
    m_languageListView->setFocusPolicy(Qt::StrongFocus);
    m_languageListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_languageListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_languageListView->setUniformItemSizes(true);
    m_languageListView->setSpacing(0);
    m_languageListView->setAlternatingRowColors(true);
    m_languageListView->setStyleSheet(QStringLiteral("QListView::item{min-height:36px;}"));
    // 不用 KDK_ALL_INFO_FORMAT：会把 accessibleName 写成英文物件名
    m_languageListView->setAccessibleName(QStringLiteral(" "));

    mode = new QStandardItemModel(this);
    mode->setColumnCount(1);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(mode);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_languageListView->setModel(m_proxyModel);

    m_cancelButton = new KPushButton(this);
    m_cancelButton->setText(tr("No"));
    m_cancelButton->setObjectName(QString::fromUtf8("m_cancelButton"));
    m_cancelButton->setMinimumSize(96, 36);
    m_cancelButton->setMaximumWidth(96);
    m_cancelButton->setAutoDefault(false);

    m_confirmButton = new KPushButton(this);
    m_confirmButton->setText(tr("Yes"));
    m_confirmButton->setObjectName(QString::fromUtf8("yesBtn"));
    m_confirmButton->setMinimumSize(96, 36);
    m_confirmButton->setMaximumWidth(96);
    m_confirmButton->setBackgroundColorHighlight(true);
    m_confirmButton->setAutoDefault(false);
    m_confirmButton->setDefault(true);
    m_confirmButton->setEnabled(false);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(16);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_confirmButton);

    m_mainLayout->addWidget(m_searchFrame);
    m_mainLayout->addWidget(m_languageListView, 1);
    m_mainLayout->addLayout(buttonLayout);

    mainWidget()->setLayout(m_mainLayout);

    KDK_EXTEND_ALL_INFO_FORMAT(mSearchLineEdit, "Area", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_cancelButton, "Area", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_confirmButton, "Area", "", "");

    mSearchLineEdit->setAccessibleName(mSearchLineEdit->placeholderText());
    m_cancelButton->setAccessibleName(tr("No"));
    m_confirmButton->setAccessibleName(tr("Yes"));

    // 搜索 → 语言列表 → 否 → 是
    QWidget::setTabOrder(mSearchLineEdit, m_languageListView);
    QWidget::setTabOrder(m_languageListView, m_cancelButton);
    QWidget::setTabOrder(m_cancelButton, m_confirmButton);
}

// 初始化添加语言弹窗信号连接
void AddLanguageDialog::initConnect()
{
    QCompleter *completer = new QCompleter(this);
    mSearchLineEdit->setCompleter(completer);
    mSearchLineEdit->completer()->popup()->setVisible(false);

    connect(m_languageListView, &QListView::entered, this, [=](const QModelIndex &index) {
        const QString mStr = index.data().toString();
        QFontMetrics fontMetrics(font());
        const int fontSize = fontMetrics.width(mStr);
        if (fontSize - 16 > 360) {
            QToolTip::showText(QCursor::pos(), mStr);
        }
    });

    connect(m_confirmButton, &KPushButton::clicked, this, [=]() {
        const QModelIndexList indexList = m_languageListView->selectionModel()->selectedIndexes();
        QStringList selectedLanguageList;
        for (const QModelIndex &index : indexList) {
            selectedLanguageList.append(index.data().toString());
        }
        qDebug() << Q_FUNC_INFO << "select LanguageList" << selectedLanguageList;
        Q_EMIT addShowLanguage(selectedLanguageList);
        hide();
    });

    connect(m_cancelButton, &KPushButton::clicked, this, [=]() {
        hide();
    });

    connect(m_languageListView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &AddLanguageDialog::onSelectionChanged);
}

void AddLanguageDialog::setConfirmEnable(bool enabled)
{
    m_confirmButton->setEnabled(enabled);
}

void AddLanguageDialog::onSelectionChanged(const QItemSelection &selected,
                                           const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)
    const QModelIndexList selectedIndexes = m_languageListView->selectionModel()->selectedIndexes();
    for (const QModelIndex &index : selectedIndexes) {
        if (index.row() >= 0) {
            setConfirmEnable(true);
            return;
        }
    }
    setConfirmEnable(false);
}

void AddLanguageDialog::initCompleter()
{
    connect(mSearchLineEdit, &KSearchLineEdit::textChanged, this, [=](const QString &text) {
        mSearchLineEdit->completer()->popup()->setVisible(false);
        m_proxyModel->setFilterRole(Qt::UserRole);
        m_proxyModel->setFilterRegExp(text);
        // 过滤后重新默认选中首项，与添加开机启动窗口一致
        selectFirstEnabledLanguage();
    });
}

void AddLanguageDialog::selectFirstEnabledLanguage()
{
    if (!m_languageListView || !m_proxyModel) {
        return;
    }
    for (int row = 0; row < m_proxyModel->rowCount(); ++row) {
        const QModelIndex index = m_proxyModel->index(row, 0);
        if (index.isValid() && (index.flags() & Qt::ItemIsEnabled)) {
            m_languageListView->setCurrentIndex(index);
            m_languageListView->selectionModel()->select(
                index, QItemSelectionModel::ClearAndSelect);
            setConfirmEnable(true);
            return;
        }
    }
    m_languageListView->clearSelection();
    setConfirmEnable(false);
}

void AddLanguageDialog::showEvent(QShowEvent *e)
{
    mSearchLineEdit->clear();
    // 默认选中首项，Tab 进入列表时读屏能读到语言名（同 SelectDesktopFileDialog）
    selectFirstEnabledLanguage();
    mSearchLineEdit->setFocus(Qt::OtherFocusReason);
    KDialog::showEvent(e);
}

void AddLanguageDialog::initDict()
{
    if (!dict.isEmpty()) {
        return;
    }
    dict.reserve(25333);
    QFile file(kDictFile);

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    QByteArray content = file.readAll();
    file.close();
    QTextStream stream(&content, QIODevice::ReadOnly);

    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const QStringList items = line.split(QChar(':'));

        if (items.size() == 2) {
            dict.insert(items[0].toInt(nullptr, 16), items[1]);
        }
    }
}

QString AddLanguageDialog::chineseToPinyin(const QString &words)
{
    QString result;
    if (words.contains(QRegExp("[\\x4e00-\\x9fa5]+"))) {
        for (int i = 0; i < words.length(); ++i) {
            const uint key = words.at(i).unicode();
            const auto findResult = dict.find(key);

            if (findResult != dict.end()) {
                result.append(findResult.value());
            } else {
                result.append(words.at(i));
            }
        }
    }
    result = removeDigital(result);
    result.replace("cangyu", "zangyu");
    return result;
}

QString AddLanguageDialog::removeDigital(QString input)
{
    if (input.isEmpty()) {
        return QString();
    }
    QString value;
    QByteArray ba = input.toLocal8Bit();
    char *data = ba.data();
    while (*data) {
        if (!(*data >= '0' && *data <= '9')) {
            value += *data;
        }
        data++;
    }
    return value;
}

void AddLanguageDialog::setRowFlags(const QString &languageName, bool flag)
{
    for (int row = 0; row < mode->rowCount(); ++row) {
        const QModelIndex index = mode->index(row, 0);
        if (index.data().toString() == languageName) {
            QStandardItem *item = mode->itemFromIndex(index);
            if (flag) {
                item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
            } else {
                item->setFlags(item->flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            }
            break;
        }
    }
}

void AddLanguageDialog::addRow(const QString &languageName, bool isAdded)
{
    QStandardItem *item = new QStandardItem(languageName);
    if (isAdded) {
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
    }
    mode->appendRow(item);
    mode->setData(mode->index(mode->rowCount() - 1, 0),
                  languageName + chineseToPinyin(languageName),
                  Qt::UserRole);
}
