#include "clockbackend.h"
#include "pinyin.h"

#include <kysdk/applications/ksearchlineedit.h>

#include <QLocale>
#include <QCompleter>
#include <QDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QFile>
#include <QMap>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>
#include <QTimeZone>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QTextStream>
#include <QVBoxLayout>
#include <QPushButton>
#include <algorithm>
#include <cstdlib>
#include <locale.h>
#include <libintl.h>

using namespace kdk;

namespace {
const char kSettingsOrg[] = "org.ukui.desktop-clock-widget";
const char kSettingsApp[] = "DesktopClock";
const char kTimezonePrefix[] = "instances/";
const char kTimezoneDomain[] = "installer-timezones";
const char kDefaultLocale[] = "en_US.UTF-8";
const QString kZoneTabFile = QStringLiteral("/usr/share/zoneinfo/zone.tab");
const QString kEtcTimezoneFile = QStringLiteral("/etc/timezone");
const QStringList kBlackTimezone = {QStringLiteral("Asia/Hong_Kong"), QStringLiteral("Asia/Macau"), QStringLiteral("Asia/Taipei")};
const QStringList kWhiteTimezone = {QStringLiteral("Asia/Beijing"), QStringLiteral("Asia/Shanghai"), QStringLiteral("Asia/Urumqi")};
const QStringList kLinkShanghaiTimezone = {QStringLiteral("Asia/Beijing"), QStringLiteral("Asia/Urumqi")};
const QString kUkccDatetimeService = QStringLiteral("org.ukui.ukcc.session");
const QString kUkccDatetimePath = QStringLiteral("/Datetime");
const QString kUkccDatetimeIface = QStringLiteral("org.ukui.ukcc.session.Datetime");

QString readTextFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.exists())
        return QString();
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    QTextStream textStream(&file);
    return textStream.readAll();
}

QString getLocalTimezoneName(const QString &timezone, const QString &localeName)
{
    (void)setlocale(LC_ALL, QString(localeName + QStringLiteral(".UTF-8")).toStdString().c_str());
    const QString localName = QString::fromUtf8(dgettext(kTimezoneDomain, timezone.toStdString().c_str()));
    int index = localName.lastIndexOf(QLatin1Char('/'));
    if (index == -1)
        index = localName.lastIndexOf(QString::fromUtf8("∕"));
    (void)setlocale(LC_ALL, kDefaultLocale);
    return (index > -1) ? localName.mid(index + 1) : localName;
}

QString zoneForOffset(const QString &zoneId)
{
    if (kLinkShanghaiTimezone.contains(zoneId))
        return QStringLiteral("Asia/Shanghai");
    return zoneId;
}

QString currentTimezoneFromUkccSession()
{
    QDBusInterface datetimeInterface(kUkccDatetimeService,
                                     kUkccDatetimePath,
                                     kUkccDatetimeIface,
                                     QDBusConnection::sessionBus());
    if (!datetimeInterface.isValid())
        return QString();

    const QVariant timezoneVariant = datetimeInterface.property("timezone");
    const QStringList timezoneList = timezoneVariant.toStringList();
    if (timezoneList.size() < 3)
        return QString();

    return timezoneList.at(2).trimmed();
}

QString currentTimezoneId()
{
    const QString ukccTimezone = currentTimezoneFromUkccSession();
    if (!ukccTimezone.isEmpty())
        return ukccTimezone;

    const QString etcTimezone = readTextFile(kEtcTimezoneFile).trimmed();
    if (!etcTimezone.isEmpty())
        return etcTimezone;

    return QString::fromUtf8(QTimeZone::systemTimeZoneId());
}

bool isSupportedTimezoneId(const QString &zoneId)
{
    if (zoneId.isEmpty())
        return true;
    if (kWhiteTimezone.contains(zoneId))
        return true;
    return QTimeZone::isTimeZoneIdAvailable(zoneId.toUtf8());
}

QTimeZone makeTimeZoneForClock(const QString &zoneId)
{
    const QString realZoneId = zoneForOffset(zoneId);
    const QTimeZone zone(realZoneId.toUtf8());
    return zone.isValid() ? zone : QTimeZone::systemTimeZone();
}

QString gmtOffsetText(const QTimeZone &zone, const QDateTime &now)
{
    // 与控制中心 datetime 的时区列表格式保持一致。
    const int utcOff = zone.offsetFromUtc(now) / 3600;
    if (utcOff >= 0) {
        return QStringLiteral("（GMT+%1:%2）")
            .arg(utcOff, 2, 10, QLatin1Char('0'))
            .arg(utcOff / 60, 2, 10, QLatin1Char('0'));
    }
    return QStringLiteral("（GMT%1:%2）")
        .arg(utcOff, 3, 10, QLatin1Char('0'))
        .arg(utcOff / 60, 2, 10, QLatin1Char('0'));
}

QString removeDigital(const QString &input)
{
    QString value;
    value.reserve(input.size());
    for (const QChar &ch : input) {
        if (!ch.isDigit())
            value.append(ch);
    }
    return value;
}
}

ClockBackend::ClockBackend(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &ClockBackend::updateTime);
    m_timer.setInterval(1000);
    m_timer.start();
    updateTime();
}

QString ClockBackend::timeText() const
{
    return m_timeText;
}

QString ClockBackend::timeDisplayText() const
{
    return m_timeDisplayText;
}

QString ClockBackend::dateText() const
{
    return m_dateText;
}

QString ClockBackend::zoneText() const
{
    return m_zoneText;
}

QString ClockBackend::weekText() const
{
    return m_weekText;
}

QString ClockBackend::monthDayText() const
{
    return m_monthDayText;
}

int ClockBackend::hour() const
{
    return m_hour;
}

int ClockBackend::minute() const
{
    return m_minute;
}

int ClockBackend::second() const
{
    return m_second;
}

QString ClockBackend::timezoneId() const
{
    return m_timezoneId;
}

void ClockBackend::setWidgetInstanceId(int instanceId)
{
    if (m_widgetInstanceId == instanceId)
        return;
    m_widgetInstanceId = instanceId;
    loadInstanceTimezone();
    updateTime();
}

bool ClockBackend::isValidTimezoneId(const QString &timezoneId) const
{
    return isSupportedTimezoneId(timezoneId);
}

void ClockBackend::setTimezoneId(const QString &timezoneId)
{
    if (!isValidTimezoneId(timezoneId))
        return;
    if (m_timezoneId == timezoneId)
        return;
    m_timezoneId = timezoneId;
    saveInstanceTimezone();
    Q_EMIT timezoneIdChanged();
    updateTime();
}

void ClockBackend::openTimezoneChooser()
{
    QDialog dialog;
    dialog.setWindowTitle(tr("Change Timezone"));
    dialog.setFixedSize(QSize(424, 522));
    dialog.setWindowModality(Qt::ApplicationModal);

    // 布局与控制中心 TimeZoneChooserDialog 保持一致
    auto *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 6, 24, 24);

    auto *searchFrame = new QFrame(&dialog);
    searchFrame->setFixedSize(QSize(376, 40));
    searchFrame->setFrameShape(QFrame::StyledPanel);
    searchFrame->setFrameShadow(QFrame::Raised);
    auto *searchLayout = new QHBoxLayout(searchFrame);
    searchLayout->setSpacing(0);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    auto *searchEdit = new KSearchLineEdit(searchFrame);
    searchEdit->setClearButtonEnabled(true);
    auto *completer = new QCompleter(&dialog);
    searchEdit->setCompleter(completer);
    searchEdit->completer()->popup()->setVisible(false);
    searchLayout->addWidget(searchEdit);
    mainLayout->addWidget(searchFrame);

    auto *tableView = new QTableView(&dialog);
    tableView->setFixedSize(QSize(376, 330));
    tableView->setStyleSheet(QStringLiteral("QTableView{border: 1px solid palette(midlight);}"));
    tableView->setAlternatingRowColors(true);
    tableView->setMouseTracking(true);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->verticalHeader()->setVisible(false);
    tableView->horizontalHeader()->setVisible(false);
    tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableView->setShowGrid(false);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->verticalHeader()->setDefaultSectionSize(36);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(tableView);
    mainLayout->addStretch();

    auto *model = new QStandardItemModel(&dialog);
    model->setColumnCount(1);
    auto *proxy = new QSortFilterProxyModel(&dialog);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setFilterRole(Qt::UserRole + 1);
    tableView->setModel(proxy);

    // 构建列表，同时找到当前时区对应的源行号
    const QString initialId = m_timezoneId.isEmpty() ? currentTimezoneId() : m_timezoneId;
    int currentRow = 0;
    const QVariantList options = buildTimezoneOptions();
    for (const QVariant &optionVar : options) {
        const QVariantMap option = optionVar.toMap();
        const QString label    = option.value(QStringLiteral("label")).toString();
        const QString zoneId   = option.value(QStringLiteral("id")).toString();
        const QString filter   = option.value(QStringLiteral("filter")).toString();

        auto *item = new QStandardItem(label);
        item->setData(zoneId, Qt::UserRole);
        item->setData(filter, Qt::UserRole + 1);
        model->appendRow(item);
        if (zoneId == initialId)
            currentRow = model->rowCount() - 1;
    }

    // 定位到 currentRow 对应的代理行并滚动
    auto restoreToCurrentRow = [model, proxy, tableView, &currentRow]() {
        if (currentRow < 0 || currentRow >= model->rowCount())
            return;
        const QModelIndex proxyIdx = proxy->mapFromSource(model->index(currentRow, 0));
        if (proxyIdx.isValid()) {
            tableView->selectRow(proxyIdx.row());
            tableView->scrollTo(proxyIdx, QAbstractItemView::PositionAtCenter);
        }
    };
    restoreToCurrentRow();

    // 过滤逻辑与控制中心 TimeZoneChooserDialog 一致
    connect(searchEdit, &KSearchLineEdit::textChanged, &dialog,
            [searchEdit, proxy, tableView, restoreToCurrentRow, &dialog](const QString &text) {
        searchEdit->completer()->popup()->setVisible(false);
        proxy->setFilterRole(Qt::UserRole + 1);
        proxy->setFilterRegExp(text.trimmed());
        if (text.trimmed().isEmpty()) {
            QTimer::singleShot(0, &dialog, [restoreToCurrentRow]() { restoreToCurrentRow(); });
        } else if (proxy->rowCount() > 0) {
            tableView->selectRow(0);
            tableView->scrollTo(proxy->index(0, 0), QAbstractItemView::PositionAtCenter);
        }
    });

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);
    buttonLayout->addStretch();
    auto *cancelButton = new QPushButton(tr("Cancel"), &dialog);
    cancelButton->setFixedWidth(88);
    auto *confirmButton = new QPushButton(tr("Confirm"), &dialog);
    confirmButton->setFixedWidth(88);
    confirmButton->setDefault(true);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(confirmButton);
    mainLayout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(confirmButton, &QPushButton::clicked, &dialog, [&dialog, tableView]() {
        if (!tableView->selectionModel()->hasSelection())
            return;
        dialog.accept();
    });

    if (dialog.exec() != QDialog::Accepted)
        return;

    const QModelIndexList selectedRows = tableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty())
        return;

    const QString selectedId = selectedRows.first().data(Qt::UserRole).toString();
    setTimezoneId(selectedId);
}

void ClockBackend::loadInstanceTimezone()
{
    if (m_widgetInstanceId < 0) {
        if (!m_timezoneId.isEmpty()) {
            m_timezoneId.clear();
            Q_EMIT timezoneIdChanged();
        }
        return;
    }

    QSettings settings(QString::fromUtf8(kSettingsOrg), QString::fromUtf8(kSettingsApp));
    const QString key = QString::fromUtf8(kTimezonePrefix) + QString::number(m_widgetInstanceId) + QStringLiteral("/timezoneId");
    const QString savedId = settings.value(key).toString();
    const QString nextId = isValidTimezoneId(savedId) ? savedId : QString();
    if (m_timezoneId == nextId)
        return;
    m_timezoneId = nextId;
    Q_EMIT timezoneIdChanged();
}

void ClockBackend::saveInstanceTimezone() const
{
    if (m_widgetInstanceId < 0)
        return;

    QSettings settings(QString::fromUtf8(kSettingsOrg), QString::fromUtf8(kSettingsApp));
    const QString key = QString::fromUtf8(kTimezonePrefix) + QString::number(m_widgetInstanceId) + QStringLiteral("/timezoneId");
    if (m_timezoneId.isEmpty())
        settings.remove(key);
    else
        settings.setValue(key, m_timezoneId);
}

void ClockBackend::removeInstanceConfig()
{
    if (m_widgetInstanceId < 0)
        return;
    QSettings settings(QString::fromUtf8(kSettingsOrg), QString::fromUtf8(kSettingsApp));
    const QString key = QString::fromUtf8(kTimezonePrefix) + QString::number(m_widgetInstanceId) + QStringLiteral("/timezoneId");
    settings.remove(key);
}

QDateTime ClockBackend::effectiveNow() const
{
    return QDateTime::currentDateTimeUtc().toTimeZone(effectiveTimeZone());
}

QTimeZone ClockBackend::effectiveTimeZone() const
{
    if (m_timezoneId.isEmpty())
        return QTimeZone::systemTimeZone();
    return makeTimeZoneForClock(m_timezoneId);
}

QVariantList ClockBackend::buildTimezoneOptions() const
{
    QVariantList options;
    QString content = readTextFile(kZoneTabFile);
    if (!content.contains(QStringLiteral("Asia/Beijing")))
        content.append(QStringLiteral("\n CN\t+3955+11626\tAsia/Beijing\tChina Standard Time \n"));

    const QString localeName = QLocale::system().name();
    const QDateTime now = QDateTime::currentDateTime();

    // 与 ukui-control-center TimeZoneChooserDialog::initModel 相同：国外时区按拼音 key 排序
    QMap<QString, QString> timezoneNameToTimezoneMap;
    QList<QString> timezoneNameList;

    const QStringList lines = content.split(QLatin1Char('\n'));
    for (const QString &lineRaw : lines) {
        const QString line = lineRaw.trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
            continue;
        const QStringList details = line.split(QLatin1Char('\t'));
        if (details.size() < 3)
            continue;
        const QString zoneId = details.at(2).trimmed();
        if (zoneId.isEmpty())
            continue;
        if (kBlackTimezone.contains(zoneId) || kWhiteTimezone.contains(zoneId))
            continue;

        const QString timezoneName = getLocalTimezoneName(zoneId, localeName);
        const QString pinyinTxt = removeDigital(Chinese2Pinyin(timezoneName));
        timezoneNameToTimezoneMap.insert(pinyinTxt, zoneId);
        timezoneNameList.append(pinyinTxt);
    }

    std::sort(timezoneNameList.begin(), timezoneNameList.end());

    auto appendOption = [&](const QString &zoneId, const QString &filterMiddle) {
        const QString realZoneId = zoneForOffset(zoneId);
        const QTimeZone zone(realZoneId.toUtf8());
        const QString zoneName = getLocalTimezoneName(zoneId, localeName);
        const QString gmtText = gmtOffsetText(zone, now);

        QVariantMap item;
        item.insert(QStringLiteral("id"), zoneId);
        item.insert(QStringLiteral("label"), gmtText + zoneName);
        item.insert(QStringLiteral("name"), zoneName);
        item.insert(QStringLiteral("filter"), zoneName + filterMiddle + zoneId);
        options.append(item);
    };

    for (const QString &zoneId : kWhiteTimezone) {
        const QString zoneName = getLocalTimezoneName(zoneId, localeName);
        const QString pinyinTxt = removeDigital(Chinese2Pinyin(zoneName));
        appendOption(zoneId, pinyinTxt);
    }

    for (const QString &timezonePinyin : timezoneNameList) {
        if (!timezoneNameToTimezoneMap.contains(timezonePinyin))
            continue;
        const QString zoneId = timezoneNameToTimezoneMap.value(timezonePinyin);
        appendOption(zoneId, timezonePinyin);
    }

    return options;
}

void ClockBackend::updateTime()
{
    const QDateTime now = effectiveNow();
    const QLocale locale;
    const QTimeZone currentZone = now.timeZone();

    // 使用本地化格式，自动适配 12/24 小时制和日期格式
    m_timeText = locale.toString(now.time(), QLocale::ShortFormat);
    // 中等尺寸与预览统一：只显示 HH:mm，不显示上午/下午。
    m_timeDisplayText = now.time().toString(QStringLiteral("HH:mm"));
    m_dateText = locale.toString(now.date(), QLocale::LongFormat);
    m_zoneText = currentZone.displayName(now, QTimeZone::LongName, locale);
    m_weekText = locale.toString(now.date(), QStringLiteral("dddd"));
    m_monthDayText = locale.toString(now.date(), QStringLiteral("M月d日"));
    m_hour = now.time().hour();
    m_minute = now.time().minute();
    m_second = now.time().second();
    if (m_zoneText.isEmpty())
        m_zoneText = currentZone.abbreviation(now);
    if (m_zoneText.isEmpty())
        m_zoneText = QString::fromUtf8(currentZone.id());

    Q_EMIT timeChanged();
}

