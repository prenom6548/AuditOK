#ifndef TRANSLATIONS_H
#define TRANSLATIONS_H

#include <QString>
#include "../common/singleton.h"

// 使用Qt的ts翻译机制，尝试了多次未能生效，原因未知，因为时间原因先暂时代码翻译，后续优化
class Translations : public Singleton<Translations>
{
public:
    Translations(token);
    ~Translations();

    void setlocale(const QString& locale) {
        m_locale = locale;
    }

    QString tr(const QString& src);

private:
    QString zh_CN(const QString& src);
    QString zh_HK(const QString& src);
    QString bo_CN(const QString& src);
    QString mn_MN(const QString& src);

private:
    QString m_locale{"zh_CN"};
};

#endif // TRANSLATIONS_H
