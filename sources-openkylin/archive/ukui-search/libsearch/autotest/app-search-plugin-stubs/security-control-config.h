#ifndef SECURITY_CONTROL_CONFIG_STUB_H
#define SECURITY_CONTROL_CONFIG_STUB_H

#include <QObject>

namespace UkuiSearch {

class SecurityControlConfig : public QObject
{
public:
    static SecurityControlConfig &instance()
    {
        static SecurityControlConfig config;
        return config;
    }

    bool isUnderSecurityControl() const { return m_underSecurityControl; }
    bool isWhiteListEnable() const { return m_whiteListEnable; }
    bool isBlackListEnable() const { return m_blackListEnable; }
    QStringList appWhiteList() const { return m_whiteList; }
    QStringList appBlackList() const { return m_blackList; }

    void setPolicy(bool underSecurityControl, bool blackListEnable, bool whiteListEnable,
                   const QStringList &blackList, const QStringList &whiteList)
    {
        m_underSecurityControl = underSecurityControl;
        m_blackListEnable = blackListEnable;
        m_whiteListEnable = whiteListEnable;
        m_blackList = blackList;
        m_whiteList = whiteList;
    }

private:
    explicit SecurityControlConfig(QObject *parent = nullptr) : QObject(parent) {}

    bool m_underSecurityControl = false;
    bool m_whiteListEnable = false;
    bool m_blackListEnable = false;
    QStringList m_whiteList;
    QStringList m_blackList;
};

}

#endif
