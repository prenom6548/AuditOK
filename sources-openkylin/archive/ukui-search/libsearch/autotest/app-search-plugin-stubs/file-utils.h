#ifndef FILE_UTILS_STUB_H
#define FILE_UTILS_STUB_H

#include <QString>

class FileUtils
{
public:
    static QString setAllTextBold(const QString &text)
    {
        return QStringLiteral("<b>") + text + QStringLiteral("</b>");
    }

    static QString getSnippetWithoutKeyword(const QString &text, int length)
    {
        return text.left(length);
    }

    static QString escapeHtml(const QString &text)
    {
        return text.toHtmlEscaped();
    }
};

#endif
