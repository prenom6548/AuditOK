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
#include <PolkitQt1/Agent/Listener>
#include <PolkitQt1/Agent/Session>
#include <PolkitQt1/Subject>
#include <PolkitQt1/Identity>
#include <PolkitQt1/Details>
#include <PolkitQt1/Authority>
#include <PolkitQt1/ActionDescription>
#include <qdialogbuttonbox.h>

#include <QPoint>
#include <QCursor>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>
#include <QScreen>
#include <fcntl.h>

#include "PolkitListener.h"
#include "mainwindow.h"
#include "generic.h"
#include <libkysysinfo.h>

PolkitListener::PolkitListener(QObject *parent)
    : Listener(parent), inProgress(false), currentIdentity(0), m_managerFullScreen(nullptr)
{
    m_isSupportTableMode = isSupportTableMode();
    m_isMavis = isMavis();
    qDebug() << "isSupportTableMode:" << m_isSupportTableMode;
}

PolkitListener::~PolkitListener() {}

bool PolkitListener::isMavis()
{
    char *prjName = kdk_system_get_projectName();
    if (prjName) {
        QString strFeature = QString(prjName);
        free(prjName);
        if (!strFeature.isEmpty()) {
            if(strFeature == "V10SP1-edu"){
                m_isSupportTableMode = true;
                return true;
            }
        }
    }
    return false;
}

bool PolkitListener::isSupportTableMode()
{
    unsigned int nFeature = kdk_system_get_productFeatures();
    if (nFeature&0x02) {    // 支持平板模式
        return true;
    }
    return false;
}

/* initiateAuthentication message from polkit */
void PolkitListener::initiateAuthentication(
    const QString &actionId,
    const QString &message,
    const QString &iconName,
    const PolkitQt1::Details &details,
    const QString &cookie,
    const PolkitQt1::Identity::List &identities,
    PolkitQt1::Agent::AsyncResult *result)
{
    if (inProgress) {
        result->setError(tr("Another client is already authenticating, please try again later."));
        result->setCompleted();
        qDebug() << "Another client is already authenticating, please try again later.";
        return;
    }

    QStringList usersList;
    QString subjectPid, callerPid;
    PolkitQt1::ActionDescription actionDesc;

    QString username = getenv("USER");
    for(auto identity : identities){
        if(identity.toString().remove("unix-user:") == username)
            usersList.prepend(identity.toString().remove("unix-user:"));
        else
            usersList.append(identity.toString().remove("unix-user:"));
    }

    subjectPid = details.lookup("polkit.subject-pid");
    callerPid = details.lookup("polkit.caller-pid");
    implicitActiveStr = details.lookup("implicit_authorization");

    /* find action description for actionId */
    foreach (const PolkitQt1::ActionDescription &desc, PolkitQt1::Authority::instance()->enumerateActionsSync()) {
        if (actionId == desc.actionId()) {
            actionDesc = desc;
            qDebug() << "Action description has been found";
            break;
        }
    }

    qDebug() << "Initiating authentication";
    qDebug() << "icon name: " << iconName;
    qDebug() << "identities: " << usersList;
    qDebug() << "message: " << message;
    qDebug() << "action id: " << actionId;
    qDebug() << "polkit.subject-pid" << subjectPid;
    qDebug() << "polkit.caller-pid" << callerPid;
    qDebug() << "action: " << actionDesc.description();
    qDebug() << "Vendor:" << actionDesc.vendorName() << actionDesc.vendorUrl();

    this->inProgress = true;

    this->identities = identities;
    this->cookie = cookie;
    this->result = result;
    session.clear();
    if (identities.length() == 1) {
        this->currentIdentity = identities[0];
    } else {
        currentIdentity = identities[0];
    }

    /* Create the polkit window */

    m_managerFullScreen = new FullScreenBackground();
    m_managerFullScreen->initMainWindow(
        iconName, message, usersList, subjectPid, callerPid, actionDesc, m_isMavis, m_isSupportTableMode);

    if (m_managerFullScreen && m_managerFullScreen->getMainWindow()) {
        connect(m_managerFullScreen->getMainWindow(), &MainWindow::accept, this, &PolkitListener::onResponse);
        connect(m_managerFullScreen->getMainWindow(), &MainWindow::canceled, this, [&] {
            if (inProgress && !gainedAuthorization) {
                wasCancelled = true;
                unacknowledged_messages = false;
                if (!session.isNull()) {
                    session.data()->cancel();
                }
                finishObtainPrivilege();
            }
        });

        connect(m_managerFullScreen->getMainWindow(), &MainWindow::switchToBiometric, this, [&] {
            wasSwitchToBiometric = true;
            startAuthentication();
        });

        connect(m_managerFullScreen->getMainWindow(), &MainWindow::yesPrompted, this, [=] {
            result->setCompleted();
            this->gainedAuthorization = true;
            finishObtainPrivilege();
        });

        connect(m_managerFullScreen->getMainWindow(), &MainWindow::restartAuth, this, [&] { startAuthentication(); });

        connect(m_managerFullScreen->getMainWindow(), &MainWindow::userChanged, this, [&](const QString &userName) {
            for (int i = 0; i < this->identities.size(); i++) {
                auto identity = this->identities.at(i);
                if (identity.toString().remove("unix-user:") == userName) {
                    this->currentIdentity = this->identities.at(i);
                    numTries = 0;
                    startAuthentication();
                    break;
                }
            }
        });

        numTries = 0;
        wasCancelled = false;
        wasSwitchToBiometric = false;

        if (implicitActiveStr == "yes_prompt") {
            m_managerFullScreen->getMainWindow()->onShowYesPrompt();
            m_managerFullScreen->getMainWindow()->setFixedSize(m_managerFullScreen->getMainWindow()->width(), 200);
            m_managerFullScreen->getMainWindow()->show();
            m_managerFullScreen->getMainWindow()->activateWindow();
            return;
        }

        m_managerFullScreen->getMainWindow()->userChanged(usersList.at(0));
    }
}

static int get_pam_tally(int *deny, int *unlock_time)
{
    char buf[128];
    FILE *auth_file = NULL;

    if ((auth_file = fopen("/etc/pam.d/common-auth", "r")) == NULL)
        return -1;

    while (fgets(buf, sizeof(buf), auth_file)) {
        if (strnlen(buf,1024) == 0 || buf[0] == '#')
            continue;
        if (!strstr(buf, "deny"))
            continue;

        char *ptr = strtok(buf, " \t");
        while (ptr) {
            if (strncmp(ptr, "deny=", 5) == 0) {
                sscanf(ptr, "deny=%d", deny);
                // gs_debug("-------------------- deny=%d", *deny);
            }
            if (strncmp(ptr, "unlock_time=", 12) == 0) {
                sscanf(ptr, "unlock_time=%d", unlock_time);
                // gs_debug("-------------------- unlock_time=%d", *unlock_time);
            }
            ptr = strtok(NULL, " \t");
        }
        fclose(auth_file);
        auth_file = NULL;
        return 1;
    }
    fclose(auth_file);
    auth_file = NULL;
    return 0;
}

void PolkitListener::finishObtainPrivilege()
{
    /* Number of tries increase only when some user is selected */
    if (currentIdentity.isValid()) {
        numTries++;
    }
    qDebug().noquote() << QString("Finishing obtaining "
                                  "privileges (G:%1, C:%2, D:%3).")
                              .arg(gainedAuthorization)
                              .arg(wasCancelled)
                              .arg(m_managerFullScreen && m_managerFullScreen->getMainWindow());

    if (!gainedAuthorization && !wasCancelled && (m_managerFullScreen && m_managerFullScreen->getMainWindow())) {
        int deny = 0, unlock_time = 0;
        m_managerFullScreen->getMainWindow()->stopDoubleAuth();
        if (!get_pam_tally(&deny, &unlock_time) || (deny == 0 && unlock_time == 0)) {
            // if(!wasSwitchToBiometric){
            if (!unacknowledged_messages)
                m_managerFullScreen->getMainWindow()->setAuthResult(
                    gainedAuthorization, tr("Authentication failure, please try again."));
            //}
            startAuthentication();
            return;
        } else {
            if (!unacknowledged_messages) {
                m_managerFullScreen->getMainWindow()->setAuthResult(gainedAuthorization, tr("Password input error!"));
            }
            startAuthentication();
            return;
        }

        startAuthentication();
        return;
    }

    if (m_managerFullScreen) {
        m_managerFullScreen->closeDialog();
        m_managerFullScreen->deleteLater();
        m_managerFullScreen = nullptr;
    }

    if ((implicitActiveStr == "yes_prompt") && wasCancelled) {
        result->setError("polkit-ukui-authentication-agent-1 cancel");
    }

    if (!session.isNull()) {
        session.data()->result()->setCompleted();
        session.data()->deleteLater();
        session.clear();
    } else {
        result->setCompleted();
    }

    this->inProgress = false;
    implicitActiveStr = "";
    qDebug() << "Finish obtain authorization:" << gainedAuthorization;
    gainedAuthorization = false;
}

void establishToBioPAM()
{
    FILE *file;
    const char *data = "polkit-ukui-authentication-agent-1";

    if ((file = fopen(BIO_COM_FILE, "w")) == NULL) {
        qWarning() << "open communication file failed: " << strerror(errno);
        return;
    }

    if (fputs(data, file) == EOF) {
        qWarning() << "write to communication file error: " << strerror(errno);
    }
    fclose(file);
}

void PolkitListener::startAuthentication()
{
    qDebug() << "start authenticate user " << currentIdentity.toString();
    unacknowledged_messages = false;
    if (!session.isNull()) {
        session.data()->deleteLater();
    }

    /* We will create a new session only when some user is selected */
    if (currentIdentity.isValid()) {

        establishToBioPAM();

        session = new Session(currentIdentity, cookie, result);
        connect(session.data(), SIGNAL(request(QString, bool)), this, SLOT(onShowPrompt(QString, bool)));
        connect(session.data(), SIGNAL(completed(bool)), this, SLOT(onAuthCompleted(bool)));
        connect(session.data(), SIGNAL(showError(QString)), this, SLOT(onShowError(QString)));
        connect(session.data(), SIGNAL(showInfo(QString)), this, SLOT(onShowError(QString)));
        session.data()->initiate();
    }

    if (m_managerFullScreen && m_managerFullScreen->getMainWindow()) {
        m_managerFullScreen->getMainWindow()->clearEdit();
        m_managerFullScreen->getMainWindow()->updateAuthMethod();
        m_managerFullScreen->getMainWindow()->resetSendPassword();
    }
}

void PolkitListener::onShowPrompt(const QString &prompt, bool echo)
{
    qDebug() << "Prompt: " << prompt << "echo: " << echo;
    if (!m_managerFullScreen || !m_managerFullScreen->getMainWindow()) {
        return;
    }
    if (prompt == BIOMETRIC_PAM || prompt == BIOMETRIC_PAM_QRCODE) {
        m_managerFullScreen->getMainWindow()->setDoubleAuth(false);
        m_managerFullScreen->getMainWindow()->switchAuthMode(MainWindow::BIOMETRIC);
    } else if (prompt == BIOMETRIC_PAM_DOUBLE) {
        m_managerFullScreen->getMainWindow()->setDoubleAuth(true);
        m_managerFullScreen->getMainWindow()->switchAuthMode(MainWindow::BIOMETRIC);
        //这时候不需要显示授权弹窗，下一次收到prompt请求的时候再显示
        return;
    } else {
        unacknowledged_messages = false;
        m_managerFullScreen->getMainWindow()->switchAuthMode(MainWindow::PASSWORD);
        m_managerFullScreen->getMainWindow()->setPrompt(prompt, echo);
        m_managerFullScreen->showDialog();
    }
}

//目前返回的pam错误就onShowError，onShowInfo两类
void PolkitListener::onShowError(const QString &text)
{
    qDebug() << "[Polkit]:"
             << "Error:" << text;
    unacknowledged_messages = true;
    if (m_managerFullScreen && m_managerFullScreen->getMainWindow()) {
        QString strText = m_managerFullScreen->getMainWindow()->check_is_pam_message(text);
        m_managerFullScreen->getMainWindow()->setMessage(strText);
    }
}

void PolkitListener::onShowInfo(const QString &text)
{
    qDebug() << "[Polkit]:"
             << "Info:" << text;
    unacknowledged_messages = true;
    if (m_managerFullScreen && m_managerFullScreen->getMainWindow()) {
        QString strText = m_managerFullScreen->getMainWindow()->check_is_pam_message(text);
        m_managerFullScreen->getMainWindow()->setMessage(strText);
    }
}

void PolkitListener::onResponse(const QString &text)
{
    unacknowledged_messages = false;
    session.data()->setResponse(text);
}

void PolkitListener::onAuthCompleted(bool gainedAuthorization)
{
    qDebug() << "completed: " << gainedAuthorization;
    if (m_managerFullScreen)
        m_managerFullScreen->getMainWindow()->resetSendPassword();
    this->gainedAuthorization = gainedAuthorization;
    finishObtainPrivilege();
}

bool PolkitListener::initiateAuthenticationFinish()
{
    qDebug() << "initiateAuthenticationFinish.";
    return true;
}

void PolkitListener::cancelAuthentication()
{
    wasCancelled = true;
    qDebug() << "cancelAuthentication.";
    finishObtainPrivilege();
}
