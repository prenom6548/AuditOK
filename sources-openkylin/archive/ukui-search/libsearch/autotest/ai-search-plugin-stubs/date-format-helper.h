#ifndef AI_SEARCH_PLUGIN_DATE_FORMAT_HELPER_STUB_H
#define AI_SEARCH_PLUGIN_DATE_FORMAT_HELPER_STUB_H

#include <QString>

static const QString DEFAULT_DATETIME_FORMAT = "yyyy/MM/dd hh:mm:ss";

class DateFormat
{
public:
    static DateFormat &getInstance()
    {
        static DateFormat instance;
        return instance;
    }

    QString format() const
    {
        if (m_dateFormat == DEFAULT_DATETIME_FORMAT) {
            return QString();
        }
        return m_dateFormat;
    }

    void updateDateFormat(const QString &format)
    {
        m_dateFormat = format + QStringLiteral(" hh:mm:ss");
    }

private:
    DateFormat() = default;

    QString m_dateFormat = DEFAULT_DATETIME_FORMAT;
};

#endif
