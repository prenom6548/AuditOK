#ifndef AI_SEARCH_PLUGIN_FILE_UTILS_STUB_H
#define AI_SEARCH_PLUGIN_FILE_UTILS_STUB_H

#include <QApplication>
#include <QClipboard>
#include <QIcon>
#include <QLabel>
#include <QString>
#include <QStringList>

namespace UkuiSearch {

extern int g_openFileResult;
extern QString g_lastOpenPath;
extern bool g_lastOpenInDir;
extern QString g_lastCopiedPath;

class FileUtils
{
public:
    static QString getHtmlText(const QString &text, const QString &keyword)
    {
        return text.contains(keyword, Qt::CaseInsensitive) ? QString("<b>%1</b>").arg(text) : text;
    }

    static QString setAllTextBold(const QString &name)
    {
        return name;
    }

    static QString wrapData(QLabel *label, const QString &text)
    {
        Q_UNUSED(label)
        return text;
    }

    static QIcon getFileIcon(const QString &uri, bool checkValid = true)
    {
        Q_UNUSED(uri)
        Q_UNUSED(checkValid)
        return QIcon::fromTheme(QStringLiteral("text-x-generic"));
    }

    static QIcon getSettingIcon()
    {
        return QIcon::fromTheme(QStringLiteral("applications-system"));
    }

    static bool isOrUnder(const QString &pathA, const QString &pathB)
    {
        return pathA == pathB || pathA.startsWith(pathB + QLatin1Char('/'));
    }

    static QStringList findMultiToneWords(const QString &hanzi)
    {
        return {hanzi};
    }

    static int openFile(QString &path, bool openInDir = false)
    {
        g_lastOpenPath = path;
        g_lastOpenInDir = openInDir;
        return g_openFileResult;
    }

    static bool copyPath(QString &path)
    {
        g_lastCopiedPath = path;
        if (QApplication::clipboard()) {
            QApplication::clipboard()->setText(path);
        }
        return true;
    }
};

}  // namespace UkuiSearch

#endif
