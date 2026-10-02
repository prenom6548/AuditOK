#include "usersecurityquestions.h"
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QDBusInterface>
#include "ksystemsetting_unit.h"
#include <QDBusMessage>

using namespace std;
namespace KServer {

//bool bindSecurityQuestionsAnswerG(int uid, QString* passwordList,SecurityAnswerListNode *encryptedAnswerlist )
//{
//    for (int i = 0; i < 3; i++){
//        const char * password = passwordList[i].toStdString().c_str();
//        char *hexBlk = NULL;
//        KPR_PKCS5_PBKDF2_HMAC2_v2((unsigned char*)password, strlen(password), &hexBlk);

//        SecurityAnswer answer;
//        memset(&answer, 0, sizeof(SecurityAnswer));
//        answer.userId = uid;
//        answer.questionId = m_uniqueNumbers[i];//
//        strcpy(answer.encryptedAnswer, hexBlk);

//        addAnswerNode(&encryptedAnswerlist, (void *)&answer, sizeof(SecurityAnswer));
//    }

//    return true;
//}

bool finishSecurityQuestionBindG(UniAuthService* pServiceInterface, int uid, QList<AnswerInfo> encryptedAnswerlist){
    qDebug()<<"in finishSecurityQuestionBindG";
    qDebug()<<"uid:"<<uid;
    for (auto ncryptedAnswer : encryptedAnswerlist) {
        qDebug() << ncryptedAnswer.nQuestionId <<"  "<< ncryptedAnswer.strAnswerText;
    }

    if(pServiceInterface==nullptr || encryptedAnswerlist.count() == 0)
        return false;

    //绑定密保和用户
    int nRet = pServiceInterface->BindUserSecurityAnswers(uid, encryptedAnswerlist);
    qDebug() << "Bind ret:" << nRet;

//    QDBusMessage result = pServiceInterface->call(QStringLiteral("BindUserSecurityAnswers"), uid, QVariant::fromValue(encryptedAnswerlist));
//    if (result.type() == QDBusMessage::ErrorMessage) {
//        qDebug() << "BindUserSecurityAnswers error:" << result.errorMessage();
//        return false;
//    }
//    QList<QVariant> varResult = result.arguments();
//    if (varResult.size() > 0) {
//        qDebug() << "BindUserSecurityAnswers :" << varResult.takeFirst().toInt();
//    }else{
//        qDebug() << "BindUserSecurityAnswers error: varResult.size()==0";
//        return false;
//    }

    //检查是否绑定成功
    bool isBound = pServiceInterface->IsUserHasSecurityAnswers(uid);
    qDebug() << "IsUserBound ret:" << isBound;
    return isBound;

//    QDBusMessage result2 = pServiceInterface->call(QStringLiteral("IsUserHasSecurityAnswers"), uid);
//    if (result2.type() == QDBusMessage::ErrorMessage) {
//        qDebug() << "IsUserHasSecurityAnswers error:" << result2.errorMessage();
//        return false;
//    }
//    QList<QVariant> varResult2 = result2.arguments();
//    if (varResult2.size() > 0) {
//        bool bT = varResult2.takeFirst().toBool();
//        qDebug() << "IsUserHasSecurityAnswers :" << bT;
//        return bT;
//    } else {
//        return false;
//    }

////    SecurityAnswerListNode *current = encryptedAnswerlist;
////    while (current != NULL) {
////        qDebug()<<"questionId:"<<current->answer.questionId<<"  encryptedAnswer:"<<current->answer.encryptedAnswer;
//////        printf("questionId: %d, encryptedAnswer: %s\n", current->answer.questionId, (current->answer.encryptedAnswer));
////        current = current->next;
////    }

////    bindSecurityQuestionsAnswerG();
//    if(0 == setUserSecurityAnswers(encryptedAnswerlist)){//将信息录入相关后台
//        qDebug()<<"setUserSecurityAnswers success!";
//    } else{
//        qDebug()<<"setUserSecurityAnswers failed!";
//    }

//    if(1 == IsUserSecurityAnswers(uid)){//检查绑定是否成功
//        qDebug()<<"Userid "<<uid <<"Set security answers success!";
////        WriteSettingToIni("config","security","true");
//        return true;
//    } else{
//        qDebug()<<"Userid "<<uid <<"Set security answers failed!";
////        emit setSecurityAnswersErro();
//        return false;
//    }
}

//bool initSecurityDbus()
//{
//    m_pServiceInterface = new QDBusInterface(KPR_DBUS_SERVICE, \
//                                             KPR_DBUS_PATH, \
//                                             KPR_DBUS_INTERFACE, \
//                                             QDBusConnection::systemBus());

//    int count = 0;
//    QDBusMessage res = QDBusConnection::systemBus().call(m);
//    if(res.type() == QDBusMessage::ReplyMessage) {
//        count = res.arguments().takeFirst().toInt();
//    } else {
//        return false;
//    }
//    if(count > 0) {
//        return true;
//    } else
//        return false;
//}


}
