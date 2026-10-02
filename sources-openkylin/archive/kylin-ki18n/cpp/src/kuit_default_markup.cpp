// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-License-Identifier: LGPL-2.0-or-later
// 与上游 ki18n/src/i18n/kuitmarkup.cpp `KuitSetupPrivate::setDefaultMarkup` 对齐（内部顶标签名为 `root`）。

#include "KLocalizedString/klocalizedstring.h"
#include "KLocalizedString/kuitmarkup.h"
#include "KLocalizedString/klazylocalizedstring.h"

#include <QDir>
#include <QHash>
#include <QRegularExpression>
#include <QStringList>

namespace {

QString comboKeyDelim(const QStringList &langs, Kuit::VisualFormat vf)
{
    static QString plainCached;
    static QString richCached;
    static bool initialized = false;
    
    if (!initialized) {
        plainCached = QStringLiteral("+");
        richCached = QStringLiteral("+");
        initialized = true;
    }
    
    return (vf == Kuit::RichText) ? richCached : plainCached;
}

 
QString guiPathDelim(const QStringList &langs, Kuit::VisualFormat vf)
{
    static QString plainCached;
    static QString richCached;
    static bool initialized = false;
    
    if (!initialized) {
        plainCached = QString::fromUtf8("\xe2\x86\x92");
        richCached = QString::fromUtf8("\xe2\x86\x92");
        initialized = true;
    }
    
    return (vf == Kuit::RichText) ? richCached : plainCached;
}

static const QHash<QString, QString> &keyboardKeyNames()
{
    static QHash<QString, QString> m;
    static bool inited = false;
    if (!inited) {
        inited = true;
        m.insert(QStringLiteral("alt"), QStringLiteral("Alt"));
        m.insert(QStringLiteral("altgr"), QStringLiteral("AltGr"));
        m.insert(QStringLiteral("backspace"), QStringLiteral("Backspace"));
        m.insert(QStringLiteral("capslock"), QStringLiteral("CapsLock"));
        m.insert(QStringLiteral("control"), QStringLiteral("Control"));
        m.insert(QStringLiteral("ctrl"), QStringLiteral("Ctrl"));
        m.insert(QStringLiteral("del"), QStringLiteral("Del"));
        m.insert(QStringLiteral("delete"), QStringLiteral("Delete"));
        m.insert(QStringLiteral("down"), QStringLiteral("Down"));
        m.insert(QStringLiteral("end"), QStringLiteral("End"));
        m.insert(QStringLiteral("enter"), QStringLiteral("Enter"));
        m.insert(QStringLiteral("esc"), QStringLiteral("Esc"));
        m.insert(QStringLiteral("escape"), QStringLiteral("Escape"));
        m.insert(QStringLiteral("home"), QStringLiteral("Home"));
        m.insert(QStringLiteral("hyper"), QStringLiteral("Hyper"));
        m.insert(QStringLiteral("ins"), QStringLiteral("Ins"));
        m.insert(QStringLiteral("insert"), QStringLiteral("Insert"));
        m.insert(QStringLiteral("left"), QStringLiteral("Left"));
        m.insert(QStringLiteral("menu"), QStringLiteral("Menu"));
        m.insert(QStringLiteral("meta"), QStringLiteral("Meta"));
        m.insert(QStringLiteral("numlock"), QStringLiteral("NumLock"));
        m.insert(QStringLiteral("pagedown"), QStringLiteral("PageDown"));
        m.insert(QStringLiteral("pageup"), QStringLiteral("PageUp"));
        m.insert(QStringLiteral("pgdown"), QStringLiteral("PgDown"));
        m.insert(QStringLiteral("pgup"), QStringLiteral("PgUp"));
        m.insert(QStringLiteral("pausebreak"), QStringLiteral("PauseBreak"));
        m.insert(QStringLiteral("printscreen"), QStringLiteral("PrintScreen"));
        m.insert(QStringLiteral("prtscr"), QStringLiteral("PrtScr"));
        m.insert(QStringLiteral("return"), QStringLiteral("Return"));
        m.insert(QStringLiteral("right"), QStringLiteral("Right"));
        m.insert(QStringLiteral("scrolllock"), QStringLiteral("ScrollLock"));
        m.insert(QStringLiteral("shift"), QStringLiteral("Shift"));
        m.insert(QStringLiteral("space"), QStringLiteral("Space"));
        m.insert(QStringLiteral("super"), QStringLiteral("Super"));
        m.insert(QStringLiteral("sysreq"), QStringLiteral("SysReq"));
        m.insert(QStringLiteral("tab"), QStringLiteral("Tab"));
        m.insert(QStringLiteral("up"), QStringLiteral("Up"));
        m.insert(QStringLiteral("win"), QStringLiteral("Win"));
        m.insert(QStringLiteral("f1"), QStringLiteral("F1"));
        m.insert(QStringLiteral("f2"), QStringLiteral("F2"));
        m.insert(QStringLiteral("f3"), QStringLiteral("F3"));
        m.insert(QStringLiteral("f4"), QStringLiteral("F4"));
        m.insert(QStringLiteral("f5"), QStringLiteral("F5"));
        m.insert(QStringLiteral("f6"), QStringLiteral("F6"));
        m.insert(QStringLiteral("f7"), QStringLiteral("F7"));
        m.insert(QStringLiteral("f8"), QStringLiteral("F8"));
        m.insert(QStringLiteral("f9"), QStringLiteral("F9"));
        m.insert(QStringLiteral("f10"), QStringLiteral("F10"));
        m.insert(QStringLiteral("f11"), QStringLiteral("F11"));
        m.insert(QStringLiteral("f12"), QStringLiteral("F12"));
    }
    return m;
}

QString tagFormatterShortcut(const QStringList &languages,
                             const QString &,
                             const QHash<QString, QString> &,
                             const QString &text,
                             const QStringList &,
                             Kuit::VisualFormat format)
{
    static const QRegularExpression delimRx(QStringLiteral("[+-]"));
    const QRegularExpressionMatch match = delimRx.match(text);
    QStringList keys;
    if (match.hasMatch()) {
        const QString oldDelim = match.captured(0);
        keys = text.split(oldDelim, Qt::SkipEmptyParts);
    } else {
        keys.append(text);
    }
    for (QString &key : keys) {
        key = key.trimmed();
        const auto it = keyboardKeyNames().constFind(key.toLower());
        if (it != keyboardKeyNames().constEnd()) {
            key = *it;
        }
    }
    return keys.join(comboKeyDelim(languages, format));
}

QString tagFormatterInterface(const QStringList &languages,
                              const QString &,
                              const QHash<QString, QString> &,
                              const QString &text,
                              const QStringList &,
                              Kuit::VisualFormat format)
{
    static const QRegularExpression delimRx(QStringLiteral("\\||->"));
    const QRegularExpressionMatch match = delimRx.match(text);
    if (match.hasMatch()) {
        const QString oldDelim = match.captured(0);
        QStringList parts = text.split(oldDelim, Qt::SkipEmptyParts);
        return parts.join(guiPathDelim(languages, format));
    }
    return text;
}

QString tagFormatterFilename(const QStringList &,
                             const QString &,
                             const QHash<QString, QString> &,
                             const QString &text,
                             const QStringList &,
                             Kuit::VisualFormat format)
{
#ifdef Q_OS_WIN
    if (format == Kuit::RichText) {
        const auto KUIT_CLOSE_XML_REPLACEMENT = QStringLiteral("__kuit_close_xml_tag__");
        const auto KUIT_NOTEXT_XML_REPLACEMENT = QStringLiteral("__kuit_notext_xml_tag__");
        QString result = text;
        result.replace(QStringLiteral("</"), KUIT_CLOSE_XML_REPLACEMENT);
        result.replace(QStringLiteral("/>"), KUIT_NOTEXT_XML_REPLACEMENT);
        result = QDir::toNativeSeparators(result);
        result.replace(KUIT_CLOSE_XML_REPLACEMENT, QStringLiteral("</"));
        result.replace(KUIT_NOTEXT_XML_REPLACEMENT, QStringLiteral("/>"));
        return result;
    }
#else
    Q_UNUSED(format);
#endif
    return QDir::toNativeSeparators(text);
}

void installPattern(KuitSetup *setup,
                    const QString &tag,
                    const QStringList &attribNames,
                    Kuit::VisualFormat format,
                    const KLocalizedString &pattern,
                    Kuit::TagFormatter formatter,
                    int leadingNewlines)
{
    setup->setTagPattern(tag, attribNames, format, pattern, formatter, leadingNewlines);
    if (format == Kuit::PlainText) {
        setup->setTagPattern(tag, attribNames, Kuit::TermText, pattern, formatter, leadingNewlines);
    }
}

void installPattern(KuitSetup *setup,
                    const QString &tag,
                    const QStringList &attribNames,
                    Kuit::VisualFormat format,
                    const QString &pattern,
                    Kuit::TagFormatter formatter,
                    int leadingNewlines)
{
    installPattern(setup, tag, attribNames, format,
                   ki18nc(QByteArray(), pattern.toUtf8()),
                   formatter, leadingNewlines);
}

} // namespace

void ki18n_cpp_install_default_kuit_markup(KuitSetup *setup)
{
    if (!setup) {
        return;
    }

    const QStringList emptyAttrs;

    setup->setTagClass(QStringLiteral("root"), Kuit::StructTag);
    installPattern(setup,
                   QStringLiteral("root"),
                   emptyAttrs,
                   Kuit::PlainText,
                   ki18nc("tag-format-pattern <> plain", "%1"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("root"),
                   emptyAttrs,
                   Kuit::RichText,
                   ki18nc("tag-format-pattern <> rich", "%1"),
                   nullptr,
                   0);

    setup->setTagClass(QStringLiteral("title"), Kuit::StructTag);
    installPattern(setup,
                   QStringLiteral("title"),
                   emptyAttrs,
                   Kuit::PlainText,
                   ki18nc("tag-format-pattern <title> plain", "== %1 =="),
                   nullptr,
                   2);
    installPattern(setup,
                   QStringLiteral("title"),
                   emptyAttrs,
                   Kuit::RichText,
                   ki18nc("tag-format-pattern <title> rich", "<h2>%1</h2>"),
                   nullptr,
                   2);

    setup->setTagClass(QStringLiteral("subtitle"), Kuit::StructTag);
    installPattern(setup,
                   QStringLiteral("subtitle"),
                   emptyAttrs,
                   Kuit::PlainText,
                   ki18nc("tag-format-pattern <subtitle> plain", "~ %1 ~"),
                   nullptr,
                   2);
    installPattern(setup,
                   QStringLiteral("subtitle"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("<h3>%1</h3>"),
                   nullptr,
                   2);

    setup->setTagClass(QStringLiteral("para"), Kuit::StructTag);
    installPattern(setup,
                   QStringLiteral("para"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("%1"),
                   nullptr,
                   2);
    installPattern(setup,
                   QStringLiteral("para"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("<p>%1</p>"),
                   nullptr,
                   2);

    setup->setTagClass(QStringLiteral("list"), Kuit::StructTag);
    installPattern(setup,
                   QStringLiteral("list"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("%1"),
                   nullptr,
                   1);
    installPattern(setup,
                   QStringLiteral("list"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("<ul>%1</ul>"),
                   nullptr,
                   1);

    setup->setTagClass(QStringLiteral("item"), Kuit::StructTag);
    installPattern(setup,
                   QStringLiteral("item"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("  * %1"),
                   nullptr,
                   1);
    installPattern(setup,
                   QStringLiteral("item"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("<li>%1</li>"),
                   nullptr,
                   1);

    installPattern(setup,
                   QStringLiteral("note"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("Note: %1"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("note"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("<i>Note</i>: %1"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("note"),
                   QStringList{QStringLiteral("label")},
                   Kuit::PlainText,
                   ki18nc("tag-format-pattern <note label=> plain\n"
                          "%1 is the text, %2 is the note label",
                          "%2: %1"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("note"),
                   QStringList{QStringLiteral("label")},
                   Kuit::RichText,
                   ki18nc("tag-format-pattern <note label=> rich\n"
                          "%1 is the text, %2 is the note label",
                          "<i>%2</i>: %1"),
                   nullptr,
                   0);

    installPattern(setup,
                   QStringLiteral("warning"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("WARNING: %1"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("warning"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("<b>Warning</b>: %1"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("warning"),
                   QStringList{QStringLiteral("label")},
                   Kuit::PlainText,
                   ki18nc("tag-format-pattern <warning label=> plain\n"
                          "%1 is the text, %2 is the warning label",
                          "%2: %1"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("warning"),
                   QStringList{QStringLiteral("label")},
                   Kuit::RichText,
                   ki18nc("tag-format-pattern <warning label=> rich\n"
                          "%1 is the text, %2 is the warning label",
                          "<b>%2</b>: %1"),
                   nullptr,
                   0);

    installPattern(setup,
                   QStringLiteral("link"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("%1"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("link"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("<a href=\"%1\">%1</a>"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("link"),
                   QStringList{QStringLiteral("url")},
                   Kuit::PlainText,
                   ki18nc("tag-format-pattern <link url=> plain\n"
                          "%1 is the descriptive text, %2 is the URL",
                          "%1 (%2)"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("link"),
                   QStringList{QStringLiteral("url")},
                   Kuit::RichText,
                   ki18nc("tag-format-pattern <link url=> rich\n"
                          "%1 is the descriptive text, %2 is the URL",
                          "<a href=\"%2\">%1</a>"),
                   nullptr,
                   0);

    installPattern(setup,
                   QStringLiteral("filename"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("\u2018%1\u2019"),
                   tagFormatterFilename,
                   0);
    installPattern(setup,
                   QStringLiteral("filename"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("\u2018<tt>%1</tt>\u2019"),
                   tagFormatterFilename,
                   0);

    installPattern(setup,
                   QStringLiteral("application"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("%1"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("application"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("%1"),
                   nullptr,
                   0);

    installPattern(setup,
                   QStringLiteral("command"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("%1"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("command"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("<tt>%1</tt>"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("command"),
                   QStringList{QStringLiteral("section")},
                   Kuit::PlainText,
                   ki18nc("tag-format-pattern <command section=> plain\n"
                          "%1 is the command name, %2 is its man section",
                          "%1(%2)"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("command"),
                   QStringList{QStringLiteral("section")},
                   Kuit::RichText,
                   ki18nc("tag-format-pattern <command section=> rich\n"
                          "%1 is the command name, %2 is its man section",
                          "<tt>%1(%2)</tt>"),
                   nullptr,
                   0);

    installPattern(setup,
                   QStringLiteral("resource"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("\u201c%1\u201d"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("resource"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("\u201c%1\u201d"),
                   nullptr,
                   0);

    installPattern(setup,
                   QStringLiteral("icode"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("\u201c%1\u201d"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("icode"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("<tt>%1</tt>"),
                   nullptr,
                   0);

    installPattern(setup,
                   QStringLiteral("bcode"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("\n%1\n"),
                   nullptr,
                   2);
    installPattern(setup,
                   QStringLiteral("bcode"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("<pre>%1</pre>"),
                   nullptr,
                   2);

    installPattern(setup,
                   QStringLiteral("shortcut"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("%1"),
                   tagFormatterShortcut,
                   0);
    installPattern(setup,
                   QStringLiteral("shortcut"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("<b>%1</b>"),
                   tagFormatterShortcut,
                   0);

    installPattern(setup,
                   QStringLiteral("interface"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("|%1|"),
                   tagFormatterInterface,
                   0);
    installPattern(setup,
                   QStringLiteral("interface"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("<i>%1</i>"),
                   tagFormatterInterface,
                   0);

    installPattern(setup,
                   QStringLiteral("emphasis"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("*%1*"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("emphasis"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("<i>%1</i>"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("emphasis"),
                   QStringList{QStringLiteral("strong")},
                   Kuit::PlainText,
                   QStringLiteral("**%1**"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("emphasis"),
                   QStringList{QStringLiteral("strong")},
                   Kuit::RichText,
                   QStringLiteral("<b>%1</b>"),
                   nullptr,
                   0);

    installPattern(setup,
                   QStringLiteral("placeholder"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("&lt;%1&gt;"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("placeholder"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("&lt;<i>%1</i>&gt;"),
                   nullptr,
                   0);

    installPattern(setup,
                   QStringLiteral("email"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("&lt;%1&gt;"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("email"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("&lt;<a href=\"mailto:%1\">%1</a>&gt;"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("email"),
                   QStringList{QStringLiteral("address")},
                   Kuit::PlainText,
                   ki18nc("tag-format-pattern <email address=> plain\n"
                          "%1 is name, %2 is address",
                          "%1 &lt;%2&gt;"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("email"),
                   QStringList{QStringLiteral("address")},
                   Kuit::RichText,
                   ki18nc("tag-format-pattern <email address=> rich\n"
                          "%1 is name, %2 is address",
                          "<a href=\"mailto:%2\">%1</a>"),
                   nullptr,
                   0);

    installPattern(setup,
                   QStringLiteral("envar"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("$%1"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("envar"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("<tt>$%1</tt>"),
                   nullptr,
                   0);

    installPattern(setup,
                   QStringLiteral("message"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("/%1/"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("message"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("<i>%1</i>"),
                   nullptr,
                   0);

    installPattern(setup,
                   QStringLiteral("nl"),
                   emptyAttrs,
                   Kuit::PlainText,
                   QStringLiteral("%1\n"),
                   nullptr,
                   0);
    installPattern(setup,
                   QStringLiteral("nl"),
                   emptyAttrs,
                   Kuit::RichText,
                   QStringLiteral("%1<br/>"),
                   nullptr,
                   0);
}
