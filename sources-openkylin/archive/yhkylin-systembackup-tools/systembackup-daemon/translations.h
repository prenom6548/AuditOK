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

    /**
     * @brief resetLocale 重新设置本地化语言，只能在用户调用接口的时候重新初始化，用户未登录会话的情况下获取不到
     * @param current_uid 当前登录用户id
     * @return
     */
    bool resetLocale(int current_uid);

    QString tr(const QString& src);

private:
    QString zh_CN(const QString& src);
    QString zh_HK(const QString& src);
    QString bo_CN(const QString& src);
    QString mn_MN(const QString& src);
    QString ky_KG(const QString& src);
    QString kk_KZ(const QString& src);
    QString ug_CN(const QString& src);
    QString vi_VN(const QString& src);
    QString ms_MY(const QString& src);
    QString th_TH(const QString& src);
private:
    QString m_locale{"zh_CN"};
};

#endif // TRANSLATIONS_H
