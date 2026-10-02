#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include "uniauthservice.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UniAuthService *uniauthService = new UniAuthService();

    QList<QuestionInfo> listQuestionInfo = uniauthService->GetPresetSecurityQuestions(QLocale::system().name());

    for (auto questionInfo : listQuestionInfo) {
        qDebug() << questionInfo.nQuestionId << questionInfo.strQuestionText;
    }

    QList<AnswerInfo> listAnswerInfo;
    for (int n = 0; n < 3; n++) {
        AnswerInfo answerInfo;
        answerInfo.nQuestionId = n;
        answerInfo.strAnswerText = uniauthService->EncodeSecurityInfo(QString("麒麟%1").arg(n));
        listAnswerInfo.append(answerInfo);
    }
    int nRet = uniauthService->BindUserSecurityAnswers(1000, listAnswerInfo);
    qDebug() << "Bind ret:" << nRet;

    bool isBound = uniauthService->IsUserHasSecurityAnswers(1000);
    qDebug() << "IsUserBound ret:" << isBound;

    listQuestionInfo = uniauthService->GetUserSecurityQuestions(1000, QLocale::system().name());
    for (auto questionInfo : listQuestionInfo) {
        qDebug() << "User Bound Question:" << questionInfo.nQuestionId << questionInfo.strQuestionText;
    }

    QString verifyToken = uniauthService->VerifyUserSecurityAnswers(1000, listAnswerInfo);
    qDebug() << "Verify ret:" << verifyToken;

    if (!verifyToken.isEmpty()) {
        nRet = uniauthService->ChangePassword(1001, "kk123123", verifyToken);
        qDebug() << "ChangePwd ret:" << nRet;
    }

    uniauthService->deleteLater();
    uniauthService = nullptr;
    return a.exec();
}
