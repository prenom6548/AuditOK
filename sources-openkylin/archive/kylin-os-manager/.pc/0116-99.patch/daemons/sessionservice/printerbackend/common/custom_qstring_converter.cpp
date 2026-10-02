#include "custom_qstring_converter.h"


CustomQStringConverter::CustomQStringConverter()
{

}

CustomQStringConverter::~CustomQStringConverter()
{

}

char *CustomQStringConverter::QStringToChar(const QString &qstr)
{
    QByteArray ba = qstr.toLocal8Bit();
    ba += '\0';
    m_buff.append(ba);
    return m_buff.back().data();
}