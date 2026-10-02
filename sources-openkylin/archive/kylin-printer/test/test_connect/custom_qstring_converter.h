#ifndef CUSTOM_QSTRING_CONVERTER
#define CUSTOM_QSTRING_CONVERTER

#include <QString>
#include <QByteArrayList>

#define QTC(x) (CustomQStringConverter(x).toChar())


class CustomQStringConverter {
public:
    CustomQStringConverter(const QString &qstr);
    ~CustomQStringConverter();
    char *toChar();
private:
    QByteArray m_buff;
};


#endif // CUSTOM_QSTRING_CONVERTER