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

#include "timezonechooserdialog.h"
#include <QToolTip>
#include <QCompleter>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>
#include <QItemSelectionModel>
#include <locale.h>
#include <libintl.h>
#include <QTimeZone>
#include <QDBusInterface>
#include <kysdk/applications/accessinfohelper.h>
#include <QTimer>
#include "../../../shell/pinyin.h"

const char kTimezoneDomain[] = "installer-timezones";
const char kDefaultLocale[] = "en_US.UTF-8";
const QString zoneTabFile = "/usr/share/zoneinfo/zone.tab";

TimeZoneChooserDialog::TimeZoneChooserDialog(QWidget *parent) :
    KDialog(parent)
{
    initUi();
    initModel();
    initConnect();
}

TimeZoneChooserDialog::~TimeZoneChooserDialog() {

}

void TimeZoneChooserDialog::selectTimezoneRow(int row)
{
    if (!m_timezoneListView || !m_timezoneListView->model()) {
        return;
    }
    const int count = m_timezoneListView->model()->rowCount();
    if (row < 0 || row >= count) {
        return;
    }
    const QModelIndex index = m_timezoneListView->model()->index(row, 0);
    m_timezoneListView->setCurrentIndex(index);
    m_timezoneListView->selectionModel()->select(
        index, QItemSelectionModel::ClearAndSelect);
    m_timezoneListView->scrollTo(index, QAbstractItemView::PositionAtCenter);
}

void TimeZoneChooserDialog::setTitle(uint32_t type) {
    qDebug() << Q_FUNC_INFO << "title type" << type << "select row" << m_currentRow;
    m_searchLineEdit->clear();

    QTimer::singleShot(0, this, [this]() {  // 通过搜索修改时区后，再次打开窗口，如果不加延时，第一次无法定位到对应时区
        qDebug() << Q_FUNC_INFO << "currentRow" << m_currentRow
                 << "rowCount" << m_timezoneListView->model()->rowCount();
        selectTimezoneRow(m_currentRow);
        // 选中行后勿让列表抢走焦点；初始焦点须在搜索框（同添加语言）
        if (m_searchLineEdit) {
            m_searchLineEdit->setFocus(Qt::OtherFocusReason);
        }
    });

    setWindowTitle(tr("Change Timezone"));
    if (type == 1) {
        m_changeTimezone = false;
        setWindowTitle(tr("Add Timezone"));
    } else {
        m_changeTimezone = true;
    }
}

void TimeZoneChooserDialog::showEvent(QShowEvent *e)
{
    // setDefault(true) 的确认键会抢初始焦点；打开时落到搜索框
    if (m_searchLineEdit) {
        m_searchLineEdit->setFocus(Qt::OtherFocusReason);
    }
    KDialog::showEvent(e);
    QTimer::singleShot(0, this, [this]() {
        if (m_searchLineEdit && isVisible()) {
            m_searchLineEdit->setFocus(Qt::OtherFocusReason);
        }
    });
}

void TimeZoneChooserDialog::initUi()
{
    setWindowIcon("ukui-control-center");
    setWindowModality(Qt::ApplicationModal);
    setObjectName(QString::fromUtf8("TimeZoneChooserDialog"));
    setFixedSize(QSize(424, 522));

    m_timezoneChooserframe = new QFrame(this);
    m_timezoneChooserframe->setObjectName(QString::fromUtf8("frame"));
    m_timezoneChooserframe->setFixedSize(QSize(376, 40));
    m_timezoneChooserframe->setFrameShape(QFrame::StyledPanel);
    m_timezoneChooserframe->setFrameShadow(QFrame::Raised);

    QHBoxLayout* horizontalLayout = new QHBoxLayout(m_timezoneChooserframe);
    horizontalLayout->setSpacing(0);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    m_searchLineEdit = new KSearchLineEdit(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_searchLineEdit, "Date", "", "");
    m_searchLineEdit->setClearButtonEnabled(true);
    // KSearchLineEdit 默认为 ClickFocus，须 StrongFocus 才能 Tab 进入
    m_searchLineEdit->setFocusPolicy(Qt::StrongFocus);
    m_searchLineEdit->installEventFilter(this);
    QCompleter* completer = new QCompleter(this);
    m_searchLineEdit->setCompleter(completer);
    m_searchLineEdit->completer()->popup()->setVisible(false);
    m_timezoneChooserframe->layout()->addWidget(m_searchLineEdit);

    m_timezoneLayout = new QVBoxLayout(this);
    m_timezoneLayout->setSpacing(16);
    m_timezoneLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    m_timezoneLayout->setContentsMargins(24, 6, 24, 24);
    m_timezoneLayout->addWidget(m_timezoneChooserframe);

    // QListView：AT-SPI 为 list/list item；QTableView 常落无名 table cell，Tab 不朗读
    m_timezoneListView = new QListView(this);
    m_timezoneListView->setObjectName(QString::fromUtf8("m_timezoneListView"));
    m_timezoneListView->setFixedSize(QSize(376, 330));
    m_timezoneListView->setStyleSheet(
        QStringLiteral("QListView{border: 1px solid palette(midlight);}"
                       "QListView::item{min-height:36px;}"));
    m_timezoneListView->setAlternatingRowColors(true);
    m_timezoneListView->setMouseTracking(true);
    m_timezoneListView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_timezoneListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_timezoneListView->verticalScrollBar()->setProperty("drawScrollBarGroove", true);
    m_timezoneListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_timezoneListView->setUniformItemSizes(true);
    m_timezoneListView->setSpacing(0);
    // Tab 只进列表一次，行间用方向键
    m_timezoneListView->setTabKeyNavigation(false);
    m_timezoneListView->setFocusPolicy(Qt::StrongFocus);
    // 不用 KDK：会写成英文物件名；空格名避免读「列表」机器名，项名走 Display
    m_timezoneListView->setAccessibleName(QStringLiteral(" "));

    m_timezoneMode = new QStandardItemModel(this);
    m_timezoneMode->setColumnCount(1);

    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_timezoneMode);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_timezoneListView->setModel(m_proxyModel);
    m_timezoneListView->setFont(qApp->font());

    m_timezoneLayout->addWidget(m_timezoneListView);

    QSpacerItem* verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_timezoneLayout->addItem(verticalSpacer);

    m_cancelButton = new KPushButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_cancelButton, "Date", "", "");
    m_cancelButton->setText(tr("Cancel"));
    m_cancelButton->setObjectName(QString::fromUtf8("m_cancelButton"));
    m_cancelButton->setMinimumSize(QSize(88, 0));
    m_cancelButton->setMaximumSize(QSize(88, 16777215));
    m_cancelButton->setAutoDefault(false);
    m_cancelButton->setFocusPolicy(Qt::StrongFocus);

    m_confirmButton = new KPushButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_confirmButton, "Date", "", "");
    m_confirmButton->setText(tr("Confirm"));
    m_confirmButton->setObjectName(QString::fromUtf8("yesBtn"));
    m_confirmButton->setMinimumSize(QSize(88, 0));
    m_confirmButton->setMaximumSize(QSize(88, 16777215));
    m_confirmButton->setBackgroundColorHighlight(true);
    m_confirmButton->setAutoDefault(false);
    m_confirmButton->setDefault(true);
    m_confirmButton->setFocusPolicy(Qt::StrongFocus);

    QHBoxLayout* spacerHorizontalLayout = new QHBoxLayout();
    spacerHorizontalLayout->setSpacing(8);
    spacerHorizontalLayout->setObjectName(QString::fromUtf8("spacerHorizontalLayout"));
    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    spacerHorizontalLayout->addItem(horizontalSpacer);

    spacerHorizontalLayout->addWidget(m_cancelButton);
    spacerHorizontalLayout->addWidget(m_confirmButton);

    m_timezoneLayout->addStretch();
    m_timezoneLayout->addLayout(spacerHorizontalLayout);

    mainWidget()->setLayout(m_timezoneLayout);

    // 与 placeholder「搜索」对齐；KDK_EXTEND_ALL_INFO_FORMAT 会写成英文物件名
    m_searchLineEdit->setAccessibleName(m_searchLineEdit->placeholderText());
    m_cancelButton->setAccessibleName(tr("Cancel"));
    m_confirmButton->setAccessibleName(tr("Confirm"));

    // 搜索 → 时区列表 → 取消 → 确认
    QWidget::setTabOrder(m_searchLineEdit, m_timezoneListView);
    QWidget::setTabOrder(m_timezoneListView, m_cancelButton);
    QWidget::setTabOrder(m_cancelButton, m_confirmButton);
}

void TimeZoneChooserDialog::searchLineEditTextChangedSlot(const QString& search) {
    qDebug() << Q_FUNC_INFO << "searchText" << search;
    m_searchLineEdit->completer()->popup()->setVisible(false);
    m_proxyModel->setFilterRole(Qt::UserRole + 1);
    m_proxyModel->setFilterRegExp(search);
    if (search.isEmpty()) {
        QTimer::singleShot(0, this, [this]() {
            qDebug() << Q_FUNC_INFO << "currentRow" << m_currentRow
                     << "rowCount" << m_timezoneListView->model()->rowCount();
            selectTimezoneRow(m_currentRow);
        });
    } else {
        selectTimezoneRow(0);
    }
}

void TimeZoneChooserDialog::confirmButtonClickSlot() {
    QModelIndexList indexList = m_timezoneListView->selectionModel()->selectedIndexes();
    QStringList selectedTimezoneList = {};
    for (const QModelIndex &a : indexList) {
        selectedTimezoneList.append(m_timezoneListView->model()->data(a, Qt::UserRole).toString());
    }
    if (selectedTimezoneList.isEmpty()) {
        return;
    }
    QString selectTimeZone = selectedTimezoneList.at(0);
    if (m_changeTimezone) {
        m_currentRow = m_proxyModel->mapToSource(indexList.at(0)).row();
    }
    qDebug() << Q_FUNC_INFO << "select timezone" << selectTimeZone << "currentRow" << m_currentRow << "changeTimezone" << m_changeTimezone;
    Q_EMIT confirmed(selectTimeZone, windowTitle());
    this->hide();
}

void TimeZoneChooserDialog::initConnect()
{
    connect(m_searchLineEdit, &KSearchLineEdit::textChanged, this, [=](const QString& search) {
        searchLineEditTextChangedSlot(search);
    });

    connect(m_confirmButton, &KPushButton::clicked, this, [=]() {
        confirmButtonClickSlot();
    });

    connect(m_cancelButton, &KPushButton::clicked,this,[=]() {
        this->hide();
    });

    connect(m_timezoneListView, &QListView::entered, this, [=](const QModelIndex &index) {
        const QString mStr = index.data().toString();
        QFontMetrics fontMetrics(font());
        const int fontSize = fontMetrics.width(mStr);
        if (fontSize - 16 > 360) {
            QToolTip::showText(QCursor::pos(), mStr);
        }
    });

    m_fontSetting = new QGSettings("org.ukui.style", QByteArray(), this);
    connect(m_fontSetting, &QGSettings::changed, this, [=](const QString &key) {
        styleSettingChangeSlot(key);
    });
}

void TimeZoneChooserDialog::styleSettingChangeSlot(const QString &key)
{
    if (key != "systemFont" && key != "systemFontSize") {
        return;
    }
    if (m_timezoneListView) {
        m_timezoneListView->setFont(qApp->font());
    }
}

QString TimeZoneChooserDialog::getLocalTimezoneName(QString timezone, QString locale) {
    (void) setlocale(LC_ALL, QString(locale + ".UTF-8").toStdString().c_str());
    const QString local_name(dgettext(kTimezoneDomain,
                                      timezone.toStdString().c_str()));
    int index = local_name.lastIndexOf('/');
    if (index == -1) {
      // Some translations of locale name contains non-standard char.
      index = local_name.lastIndexOf("∕");
    }

    // Reset locale
    (void) setlocale(LC_ALL, kDefaultLocale);
    return (index > -1) ? local_name.mid(index + 1) : local_name;
}

QString TimeZoneChooserDialog::readRile(const QString& filepath) {
    QFile file(filepath);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "ReadFile() failed to open" << filepath;
            return "";
        }
        QTextStream textStream(&file);
        QString res=  textStream.readAll();
        file.close();
        return res;
    } else {
        qWarning() << filepath << " not found" << endl;
        return "";
    }
}

QString TimeZoneChooserDialog::removeDigital(QString input) {
    if ("" == input)
        return "";

    QString value = "";
    QByteArray ba = input.toLocal8Bit();
    char *data = nullptr;
    data = ba.data();
    while (*data) {
        if (!(*data >= '0' && *data <= '9')) {
            value += *data;
        }
        data++;
    }

    return value;
}

void TimeZoneChooserDialog::initModel()
{
    QString locale = QLocale::system().name();
    QString content(this->readRile(zoneTabFile));
    if (!content.contains("Asia/Beijing")){
        content = content + "\n CN	+3955+11626	Asia/Beijing	China Standard Time \n";
    }
    QDBusInterface datetimeInterface("org.ukui.ukcc.session",
                                     "/Datetime",
                                     "org.ukui.ukcc.session.Datetime",
                                     QDBusConnection::sessionBus());
    QStringList list = datetimeInterface.property("timezone").toStringList();
    if (list.size() < 3) {
        qDebug() << Q_FUNC_INFO << "dbus timezone not equal 3, return";
        return;
    }
    QString currentTimezone = list.at(2);
    QStringList blackTimezone = {"Asia/Hong_Kong", "Asia/Macau", "Asia/Taipei"};
    QStringList whiteTimezone = {"Asia/Beijing", "Asia/Shanghai", "Asia/Urumqi"};
    QMap<QString, QString> timezoneNameToTimezoneMap;
    QList<QString> timezoneNameList;
    for (const QString& line : content.split('\n')) {
        if (line.startsWith('#')) {
            continue;
        }
        const QStringList details(line.split('\t'));
        if (details.length() < 3) {
            continue;
        }

        QString timezone = details.at(2);
        if (blackTimezone.contains(timezone) || whiteTimezone.contains(timezone)) {
            qDebug() << Q_FUNC_INFO << "hit black/white timezone" << timezone;
            continue;
        }
        QString timezoneName = getLocalTimezoneName(timezone, locale);
        QString pinyinTxt = QString(removeDigital(Chinese2Pinyin(timezoneName)));
        timezoneNameToTimezoneMap.insert(pinyinTxt, timezone);
        timezoneNameList.append(pinyinTxt);
    }
    int selectRow = 0;
    for (const auto & timezone : whiteTimezone) {  // 先放国内时区
        QString timezoneName = getLocalTimezoneName(timezone, locale);
        const QStringList LINK_SHANGHAI_TIMEZONE = {
            "Asia/Beijing",
            "Asia/Urumqi"
        };
        QString realTimezone = timezone;
        if (LINK_SHANGHAI_TIMEZONE.contains(timezone)) {
            realTimezone = "Asia/Shanghai";
        }
        QTimeZone localTimezone = QTimeZone(realTimezone.toLatin1().data());
        int utcOff = (localTimezone.offsetFromUtc(QDateTime::currentDateTime()))/3600;
        QString gmData;
        if (utcOff >= 0) {
           gmData = QString("（GMT+%1:%2）").arg(utcOff, 2, 10, QLatin1Char('0')).arg(utcOff / 60, 2, 10, QLatin1Char('0'));
        } else {
           gmData = QString("（GMT%1:%2）").arg(utcOff, 3, 10, QLatin1Char('0')).arg(utcOff / 60, 2, 10, QLatin1Char('0'));
        }
        QString rowName = gmData + timezoneName;
        m_timezoneMode->appendRow(new QStandardItem(rowName));
        if (currentTimezone == timezone) {
            selectRow = m_timezoneMode->rowCount() - 1;
            m_currentRow = selectRow;
        }
        QString pinyinTxt = QString(removeDigital(Chinese2Pinyin(timezoneName)));

        m_timezoneMode->setData(m_timezoneMode->index(m_timezoneMode->rowCount() - 1, 0), timezone, Qt::UserRole);
        m_timezoneMode->setData(m_timezoneMode->index(m_timezoneMode->rowCount() - 1, 0), timezoneName + pinyinTxt + timezone, Qt::UserRole + 1);
    }

    std::sort(timezoneNameList.begin(), timezoneNameList.end());
    for (const auto & timezonePyinyin : timezoneNameList) { // 国外时区按首字母排序放列表
        if (timezoneNameToTimezoneMap.find(timezonePyinyin) == timezoneNameToTimezoneMap.end()) {
            continue;
        }
        QString timezone = timezoneNameToTimezoneMap[timezonePyinyin];
        QString timezoneName = getLocalTimezoneName(timezone, locale);
        QString realTimezone = timezone;
        QTimeZone localTimezone = QTimeZone(realTimezone.toLatin1().data());
        int utcOff = (localTimezone.offsetFromUtc(QDateTime::currentDateTime()))/3600;
        QString gmData;
        if (utcOff >= 0) {
           gmData = QString("（GMT+%1:%2）").arg(utcOff, 2, 10, QLatin1Char('0')).arg(utcOff / 60, 2, 10, QLatin1Char('0'));
        } else {
           gmData = QString("（GMT%1:%2）").arg(utcOff, 3, 10, QLatin1Char('0')).arg(utcOff / 60, 2, 10, QLatin1Char('0'));
        }
        QString rowName = gmData + timezoneName;
        m_timezoneMode->appendRow(new QStandardItem(rowName));
        if (currentTimezone == timezone) {
            selectRow = m_timezoneMode->rowCount() - 1;
            m_currentRow = selectRow;
        }
        m_timezoneMode->setData(m_timezoneMode->index(m_timezoneMode->rowCount() - 1, 0), timezone, Qt::UserRole);
        m_timezoneMode->setData(m_timezoneMode->index(m_timezoneMode->rowCount() - 1, 0), timezoneName + timezonePyinyin + timezone, Qt::UserRole + 1);
    }
    selectTimezoneRow(selectRow);
    qDebug() << Q_FUNC_INFO << "select row" << m_timezoneListView->currentIndex().row();
}
