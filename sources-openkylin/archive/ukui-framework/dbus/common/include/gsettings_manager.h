#ifndef __ABSTRACT_GSETTINGS_MANAGER_H__
#define __ABSTRACT_GSETTINGS_MANAGER_H__

#include <QDebug>
#include <QObject>
#include <QtDBus/QtDBus>
#include <QDBusContext>
#include <QDBusInterface>
#include "qgsettings.h"

using OnKeyChanged =  std::function<void()>;

#define PEEK_AND_SET(action, value) {\
QString errorInfo;\
    if (calledFromDBus() && !m_pGsettingsManager->peekActionIsValid(action, errorInfo)) {\
    sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support"));\
        return;\
}\
    m_pGsettingsManager->set(action, value);\
    return;}

#define PEEK_AND_SET_AND_RET(action, value, errorValue) {\
QString errorInfo;\
    if (calledFromDBus() && !m_pGsettingsManager->peekActionIsValid(action, errorInfo)) {\
    sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support"));\
        return errorValue;\
}\
    m_pGsettingsManager->set(action, value);}

#define PEEK_SCHEMA(action, ERROR) {\
QString errorInfo;\
    if (calledFromDBus() && !m_pGsettingsManager->peekActionIsValid(action, errorInfo)) {\
    sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support"));\
        return ERROR;\
}}

class GsettingsManager: public QObject
{
    Q_OBJECT
public:
    GsettingsManager(QObject *parent, QString schema, QHash<QString, OnKeyChanged> changedHandlers);
    ~GsettingsManager();

    bool peekActionIsValid(QString action, QString &errorInfo);

    void set(const QString &key, const QVariant &value);

    QVariant get(const QString &key);

    template <typename...Args>
    QDBusMessage callDbus(const QString &name,
                               const QString &path,
                               const QString &interface,
                               const QString &method,
                                Args &&...args);
private Q_SLOTS:
    void onGsettingsChanged(QString key);

protected:
    QHash<QString, OnKeyChanged> m_settingsChangedHandlers;
    UKUI::QGSettings *m_pGsettings = nullptr;
};

#endif
