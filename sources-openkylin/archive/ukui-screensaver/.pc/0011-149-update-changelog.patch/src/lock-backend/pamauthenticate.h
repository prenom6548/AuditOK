/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef PAMAUTHENTICATE_H
#define PAMAUTHENTICATE_H

#include <QObject>
#include <QStringList>
#include <security/pam_appl.h>
#include "lightdmhelper.h"
#include <QSharedPointer>

typedef struct pam_message PAM_MESSAGE;
typedef struct pam_response PAM_RESPONSE;

class AuthPamThread;
class QSocketNotifier;
class PamAuthenticate : public QObject
{
    Q_OBJECT
public:
    explicit PamAuthenticate(QSharedPointer<LightDMHelper> helper, QObject *parent = nullptr);

public:
    bool inAuthentication() const;
    bool isAuthenticated() const;
    QString authenticationUser() const;

public Q_SLOTS:
    void authenticate(const QString &username = QString());
    void respond(const QString &response);
    void cancelAuthentication();

Q_SIGNALS:
    void showMessage(QString text, int type);
    void showPrompt(QString text, int type);
    void authenticationComplete();

private Q_SLOTS:
    void onSockRead();
    void onLDMShowMessage(QString strMsg, QLightDM::Greeter::MessageType type);
    void onLDMShowPrompt(QString strPrompt, QLightDM::Greeter::PromptType type);

private:
    void _respond(const struct pam_response *response);

private:
    QSharedPointer<LightDMHelper> m_lightdmHelper = nullptr;
    bool m_isInAuthentication = false;
    bool m_isAuthenticated = false;
    bool m_isOtherUser = false;
    AuthPamThread *m_threadAuthPam = nullptr;
    QSocketNotifier *m_socketNotifier = nullptr;
    int m_nPrompts = 0;
    QStringList m_responseList;
    QList<PAM_MESSAGE> m_messageList;
    int m_fdToParent[2];
    int m_fdToChild[2];
    QString m_strUserName;

    bool hasSendBiometricPam = false;  
};

#endif // PAMAUTHENTICATE_H
