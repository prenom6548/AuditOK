#include "custom_qstring_converter.h"


CustomQStringConverter::CustomQStringConverter(const QString &qstr)
{
    m_buff = qstr.toLocal8Bit();
    m_buff += '\0';
}

CustomQStringConverter::~CustomQStringConverter()
{

}

char *CustomQStringConverter::toChar()
{
    return m_buff.data();
}