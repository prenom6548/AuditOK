#ifndef FEEDBACK_COMMOM_H
#define FEEDBACK_COMMOM_H

#include <QString>
#include "feedbackmanager.h"

class FeedBackCommon {
public:
    void setArgs(QString path , QString exe , QString restart, QString mode, QString code);
    virtual void run() = 0;

protected:
    void startCollecting(bool agree = false, QString text = "", QString phoneNumber = "", int score = 0);
    QString getAppLogFile();

    QString m_path;
    QString m_exe;
    QString m_resart;
    QString m_mode;
    QString m_code;
    QString m_datetime;
    bool m_isInit = false;
    InternalMode m_internalMode;
};

#endif // FEEDBACK_COMMOM_H
