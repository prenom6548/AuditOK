#ifndef CUSTOM_QSTRING_CONVERTER
#define CUSTOM_QSTRING_CONVERTER

#include <QString>
#include <QByteArrayList>

class CustomQStringConverter {
public:
    CustomQStringConverter();
    ~CustomQStringConverter();
    char *QStringToChar(const QString &qstr);
private:
    QByteArrayList m_buff;
};


#endif // CUSTOM_QSTRING_CONVERTER