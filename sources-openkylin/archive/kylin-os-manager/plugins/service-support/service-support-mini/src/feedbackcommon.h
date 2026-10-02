#ifndef FEEDBACK_COMMOM_H
#define FEEDBACK_COMMOM_H

#include <QString>
#include "feedbackmanager.h"

class FeedBackCommon {
public:
    FeedBackCommon();
    void setArgs(QString path , QString exe , QString restart, QString mode, QString code);
    QString ExecName() const;
    QString ExecIcon() const;
    virtual void run() = 0;

protected:
    void startCollecting(bool agree = false, QString text = "", QString phoneNumber = "", int score = 0);
    QString getAppLogFile();
    QString GetDesktopPath() const;

    QString m_path;
    QString m_exe;
    QString m_resart;
    QString m_mode;
    QString m_code;
    QString m_datetime;
    bool m_isInit = false;
    InternalMode m_internalMode;
    /**
     * @brief 部分 desktop 的名字与二进制的名字不一致，此处对这部分二进制做一下映射。
     * @note <二进制名, desktop 名>
     */
    QHash<QString, QString> exec_desktop_map_;
    quint64 annex_size_max_limit_;    // 附件最大大小限制
};

#endif
