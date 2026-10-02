#include "utils/thememanager.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QColor>
#include <QComboBox>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QList>
#include <QPalette>
#include <QProcess>
#include <QPair>
#include <QProxyStyle>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QWidget>

namespace {

class ComboPopupProxyStyle final : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;

    int styleHint(StyleHint hint,
                  const QStyleOption *option = nullptr,
                  const QWidget *widget = nullptr,
                  QStyleHintReturn *returnData = nullptr) const override
    {
        if (hint == QStyle::SH_ComboBox_Popup) {
            return 0;
        }
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

QString normalizedThemeValue(QString value)
{
    value = value.trimmed();
    if ((value.startsWith(QLatin1Char('\'')) && value.endsWith(QLatin1Char('\''))) ||
        (value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"')))) {
        value = value.mid(1, value.size() - 2);
    }
    return value.trimmed().toLower();
}

QString gsettingsValue(const QString &schema, const QString &key)
{
    QProcess process;
    process.start(QStringLiteral("gsettings"),
                  {QStringLiteral("get"), schema, key},
                  QIODevice::ReadOnly);
    if (!process.waitForFinished(300) || process.exitStatus() != QProcess::NormalExit ||
        process.exitCode() != 0) {
        return QString();
    }
    return normalizedThemeValue(QString::fromLocal8Bit(process.readAllStandardOutput()));
}

bool valueNamesDarkTheme(const QString &value)
{
    return value.contains(QStringLiteral("dark")) ||
           value.contains(QStringLiteral("black")) ||
           value.contains(QStringLiteral("night"));
}

bool valueNamesLightTheme(const QString &value)
{
    return value.contains(QStringLiteral("light")) ||
           value.contains(QStringLiteral("white"));
}

bool currentDesktopIncludes(const QString &desktop)
{
    const QString currentDesktop =
        QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP")).toLower();
    const QString desktopSession =
        QString::fromLocal8Bit(qgetenv("DESKTOP_SESSION")).toLower();
    const QString target = desktop.toLower();
    return currentDesktop.contains(target) || desktopSession.contains(target);
}

bool paletteFilePrefersDark(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    bool inWindowColors = false;
    const QRegularExpression colorRe(QStringLiteral(R"(^\s*BackgroundNormal\s*=\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+))"));
    while (!file.atEnd()) {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (line.startsWith(QLatin1Char('[')) && line.endsWith(QLatin1Char(']'))) {
            inWindowColors = line == QStringLiteral("[Colors:Window]");
            continue;
        }
        if (!inWindowColors) {
            continue;
        }

        const QRegularExpressionMatch match = colorRe.match(line);
        if (match.hasMatch()) {
            const QColor color(match.captured(1).toInt(),
                               match.captured(2).toInt(),
                               match.captured(3).toInt());
            return color.lightness() < 128;
        }
    }

    return false;
}

bool kdePrefersDark()
{
    const QString configHome = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (configHome.isEmpty()) {
        return false;
    }
    return paletteFilePrefersDark(QDir(configHome).filePath(QStringLiteral("kdeglobals")));
}

void installStandardComboDelegate(QComboBox *combo)
{
    if (!combo || combo->property("_kylinStandardItemDelegate").toBool()) {
        return;
    }

    combo->setProperty("_kylinStandardItemDelegate", true);
    if (QAbstractItemView *view = combo->view()) {
        view->setItemDelegate(new QStyledItemDelegate(view));
    }
}

} // namespace

ThemeManager &ThemeManager::instance()
{
    static ThemeManager manager;
    return manager;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
}

void ThemeManager::applySavedTheme(QApplication *app)
{
    m_app = app;
    if (!m_app->property("_kylinComboPopupProxyStyle").toBool()) {
        m_app->setProperty("_kylinComboPopupProxyStyle", true);
        m_app->setStyle(new ComboPopupProxyStyle(m_app->style()));
    }
    m_app->installEventFilter(this);
    m_systemPalette = app->palette();
    if (!m_systemThemeTimer) {
        m_systemThemeTimer = new QTimer(this);
        m_systemThemeTimer->setInterval(5000);
        connect(m_systemThemeTimer, &QTimer::timeout, this, [this]() {
            if (m_mode != FollowSystem || !m_app) {
                return;
            }
            const bool prefersDark = systemPrefersDark();
            if (prefersDark != m_dark) {
                apply();
            }
        });
        m_systemThemeTimer->start();
    }
    QSettings settings;
    const int mode = settings.value(QStringLiteral("General/Theme"), FollowSystem).toInt();
    m_mode = mode >= FollowSystem && mode <= Dark ? static_cast<ThemeMode>(mode) : FollowSystem;
    apply();
}

bool ThemeManager::eventFilter(QObject *watched, QEvent *event)
{
    if (event && (event->type() == QEvent::Polish || event->type() == QEvent::Show)) {
        installStandardComboDelegate(qobject_cast<QComboBox *>(watched));
    }
    return QObject::eventFilter(watched, event);
}

void ThemeManager::setThemeMode(ThemeMode mode)
{
    m_mode = mode;
    QSettings settings;
    settings.setValue(QStringLiteral("General/Theme"), static_cast<int>(m_mode));
    settings.sync();
    apply();
}

bool ThemeManager::systemPrefersDark() const
{
    if (!m_app) {
        return false;
    }

    const QString gnomeColorScheme =
        gsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("color-scheme"));
    const bool isUkui = currentDesktopIncludes(QStringLiteral("ukui"));

    const auto ukuiPrefersDark = []() -> int {
        const QString ukuiStyle =
            gsettingsValue(QStringLiteral("org.ukui.style"), QStringLiteral("style-name"));
        if (valueNamesDarkTheme(ukuiStyle)) {
            return 1;
        }
        if (valueNamesLightTheme(ukuiStyle)) {
            return 0;
        }
        return -1;
    };

    if (isUkui) {
        const int preference = ukuiPrefersDark();
        if (preference != -1) {
            return preference == 1;
        }
    }

    if (gnomeColorScheme.contains(QStringLiteral("prefer-dark"))) {
        return true;
    }
    if (gnomeColorScheme.contains(QStringLiteral("prefer-light"))) {
        return false;
    }

    if (!isUkui) {
        const int preference = ukuiPrefersDark();
        if (preference != -1) {
            return preference == 1;
        }
    }

    const QList<QPair<QString, QString>> themeKeys = {
        {QStringLiteral("org.ukui.interface"), QStringLiteral("gtk-theme")},
        {QStringLiteral("org.ukui.style"), QStringLiteral("widget-theme-name")},
        {QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("gtk-theme")},
        {QStringLiteral("org.mate.interface"), QStringLiteral("gtk-theme")}
    };
    for (const auto &key : themeKeys) {
        const QString value = gsettingsValue(key.first, key.second);
        if (valueNamesDarkTheme(value)) {
            return true;
        }
        if (valueNamesLightTheme(value)) {
            return false;
        }
    }

    if (kdePrefersDark()) {
        return true;
    }

    const QColor window = m_systemPalette.color(QPalette::Window);
    return window.lightness() < 128;
}

void ThemeManager::apply()
{
    if (!m_app) {
        return;
    }

    m_dark = m_mode == Dark || (m_mode == FollowSystem && systemPrefersDark());

    QPalette palette;
    if (m_dark) {
        palette.setColor(QPalette::Window, QColor(QStringLiteral("#17191d")));
        palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#eef1f5")));
        palette.setColor(QPalette::Base, QColor(QStringLiteral("#202329")));
        palette.setColor(QPalette::AlternateBase, QColor(QStringLiteral("#292d34")));
        palette.setColor(QPalette::ToolTipBase, QColor(QStringLiteral("#2f3440")));
        palette.setColor(QPalette::ToolTipText, QColor(QStringLiteral("#eef1f5")));
        palette.setColor(QPalette::Text, QColor(QStringLiteral("#eef1f5")));
        palette.setColor(QPalette::Button, QColor(QStringLiteral("#2a2e36")));
        palette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#eef1f5")));
        palette.setColor(QPalette::BrightText, QColor(QStringLiteral("#ffffff")));
        palette.setColor(QPalette::Highlight, QColor(QStringLiteral("#4a90d9")));
        palette.setColor(QPalette::HighlightedText, QColor(QStringLiteral("#ffffff")));
        palette.setColor(QPalette::Link, QColor(QStringLiteral("#8ab4f8")));
        palette.setColor(QPalette::Mid, QColor(QStringLiteral("#3a3f48")));
        palette.setColor(QPalette::Midlight, QColor(QStringLiteral("#444a54")));
        palette.setColor(QPalette::Dark, QColor(QStringLiteral("#101216")));
        palette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#8a8a8a")));
    } else {
        palette = m_app->style()->standardPalette();
        palette.setColor(QPalette::Window, QColor(QStringLiteral("#ffffff")));
        palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#202124")));
        palette.setColor(QPalette::Base, QColor(QStringLiteral("#ffffff")));
        palette.setColor(QPalette::AlternateBase, QColor(QStringLiteral("#f7f8fa")));
        palette.setColor(QPalette::ToolTipBase, QColor(QStringLiteral("#ffffff")));
        palette.setColor(QPalette::ToolTipText, QColor(QStringLiteral("#202124")));
        palette.setColor(QPalette::Text, QColor(QStringLiteral("#202124")));
        palette.setColor(QPalette::Button, QColor(QStringLiteral("#f0f0f0")));
        palette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#202124")));
        palette.setColor(QPalette::Highlight, QColor(QStringLiteral("#cfe8ff")));
        palette.setColor(QPalette::HighlightedText, QColor(QStringLiteral("#202124")));
        palette.setColor(QPalette::Link, QColor(QStringLiteral("#1565c0")));
        palette.setColor(QPalette::Mid, QColor(QStringLiteral("#d9dce2")));
        palette.setColor(QPalette::Midlight, QColor(QStringLiteral("#eef0f3")));
        palette.setColor(QPalette::Dark, QColor(QStringLiteral("#c4c8d0")));
        palette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#8a8a8a")));
    }

    m_app->setPalette(palette);
    const QString globalStyleSheet = QStringLiteral(R"(
        QWidget {
            background-color: palette(window);
            color: palette(window-text);
        }
        QDialog, QMainWindow, QScrollArea, QStackedWidget {
            background-color: palette(window);
            color: palette(window-text);
        }
        QDialog, QMainWindow {
            border: 1px solid palette(mid);
        }
        QLabel {
            background-color: transparent;
            color: palette(window-text);
        }
        QPushButton {
            background-color: palette(button);
            color: palette(button-text);
            border: 1px solid palette(mid);
            border-radius: 6px;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: palette(midlight);
        }
        QPushButton:pressed {
            background-color: palette(mid);
        }
        kdk--KWindowButtonBar QPushButton,
        kdk--KIconBar QPushButton {
            background-color: transparent;
            border: none;
            border-radius: 0px;
            outline: none;
            padding: 0px;
        }
        kdk--KWindowButtonBar QPushButton:hover,
        kdk--KIconBar QPushButton:hover {
            background-color: palette(midlight);
            border: none;
        }
        kdk--KWindowButtonBar QPushButton:pressed,
        kdk--KIconBar QPushButton:pressed {
            background-color: palette(mid);
            border: none;
        }
        QLineEdit, QTextEdit, QPlainTextEdit, QComboBox, QListWidget, QTreeWidget, QTableWidget {
            background-color: palette(base);
            color: palette(text);
            border: 1px solid palette(mid);
            selection-background-color: palette(highlight);
            selection-color: palette(highlighted-text);
        }
        QComboBox QAbstractItemView {
            background-color: palette(base);
            color: palette(text);
            selection-background-color: palette(highlight);
            selection-color: palette(highlighted-text);
        }
        QAbstractItemView::item:selected {
            background-color: palette(highlight);
            color: palette(highlighted-text);
        }
        QAbstractItemView::item:hover:!selected {
            background-color: palette(midlight);
            color: palette(text);
        }
        QListWidget::item:selected, QTreeWidget::item:selected {
            background-color: palette(highlight);
            color: palette(highlighted-text);
        }
        QListWidget::item:hover, QTreeWidget::item:hover {
            background-color: palette(midlight);
        }
        QScrollBar:vertical, QScrollBar:horizontal {
            background-color: palette(window);
        }
        QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
            background-color: palette(mid);
            border-radius: 3px;
        }
        QSplitter::handle {
            background-color: palette(mid);
        }
        QToolTip {
            background-color: palette(base);
            color: palette(text);
            border: 1px solid palette(mid);
        }
    )");
    m_app->setStyleSheet(QString());
    m_app->setStyleSheet(globalStyleSheet);

    const QList<QWidget *> widgets = m_app->allWidgets();
    for (QWidget *widget : widgets) {
        installStandardComboDelegate(qobject_cast<QComboBox *>(widget));
        widget->setProperty("darkTheme", m_dark);
        const QString localStyle = widget->styleSheet();
        if (!localStyle.isEmpty()) {
            widget->setStyleSheet(QString());
            widget->setStyleSheet(localStyle);
        }
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
        widget->update();
    }
    for (QWidget *window : m_app->topLevelWidgets()) {
        window->updateGeometry();
        window->update();
    }
    QTimer::singleShot(0, m_app, [widgets]() {
        for (QWidget *widget : widgets) {
            if (widget) {
                widget->update();
            }
        }
    });
    emit themeChanged();
}
