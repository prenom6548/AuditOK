#ifndef USERSECURITYQUESTIONS_H
#define USERSECURITYQUESTIONS_H
#include <QString>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <libintl.h>
#include <locale.h>
//#include "/usr/include/kylin-passwdreset/kylin-passwdreset.h"
#include <kba/kba.h>
#include <QDBusInterface>
//#include "../../plugins/KUserRegister/securityquestions.h"
#include "dbus/uniauthservice.h"

//class QString;
namespace KServer {
//QDBusInterface*             m_pServiceInterface = nullptr;

//bool bindSecurityQuestionsAnswerG(int uid, QString* passwordList,SecurityAnswerListNode *encryptedAnswerlist);//将安全问题答案列表加密

bool finishSecurityQuestionBindG(UniAuthService* pServiceInterface, int uid, QList<AnswerInfo> encryptedAnswerlist);//绑定用户的安全问题和答案

//bool getPresetSecurityQuestion();//

}

#endif // USERSECURITYQUESTIONS_H
