#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include "uniauthservice.h"

int TEST1(UniAuthService *uniauthService) {
    // 设置密码提示
    if (uniauthService->SetUserPasswdHint(1001, "我第一只宠物的名字") == 0) {
        qDebug() << "Password hint for UID 1001 set successfully.";
    } else {
        qDebug() << "Failed to set password hint for UID 1001.";
    }

    if (uniauthService->SetUserPasswdHint(1004, "The street I grew up on") == 0) {
        qDebug() << "Password hint for UID 1004 set successfully.";
    } else {
        qDebug() << "Failed to set password hint for UID 1004.";
    }

    // 获取密码提示
    QString password_hint = uniauthService->GetUserPasswdHint(1001);
    qDebug() << "UID: 1001, Password Hint: " << password_hint;

    password_hint = uniauthService->GetUserPasswdHint(1004);
    qDebug() << "UID: 1004, Password Hint: " << password_hint;

    return 0;
}

int TEST2(UniAuthService *uniauthService) {
    // 设置密码提示
    if (uniauthService->SetUserPasswdHint(1001, "我第一只宠物的名字更新测试") == 0) {
        qDebug() << "Password hint for UID 1001 set successfully.";
    } else {
        qDebug() << "Failed to set password hint for UID 1001.";
    }

    if (uniauthService->SetUserPasswdHint(1004, "The street I grew up on update test") == 0) {
        qDebug() << "Password hint for UID 1004 set successfully.";
    } else {
        qDebug() << "Failed to set password hint for UID 1004.";
    }

    // 获取密码提示
    QString password_hint = uniauthService->GetUserPasswdHint(1001);
    qDebug() << "UID: 1001, Password Hint: " << password_hint;

    password_hint = uniauthService->GetUserPasswdHint(1004);
    qDebug() << "UID: 1004, Password Hint: " << password_hint;

    return 0;
}

int TEST3(UniAuthService *uniauthService) {
    // 设置密码提示
    if (uniauthService->SetUserPasswdHint(1000, "我第一只宠物的名字新增测试") == 0) {
        qDebug() << "Password hint for UID 1000 set successfully.";
    } else {
        qDebug() << "Failed to set password hint for UID 1000.";
    }

    if (uniauthService->SetUserPasswdHint(1002, "The street I grew up on error test") == 0) {
        qDebug() << "Password hint for UID 1002 set successfully.";
    } else {
        qDebug() << "Failed to set password hint for UID 1002.";
    }

    // 获取密码提示
    QString password_hint = uniauthService->GetUserPasswdHint(1000);
    qDebug() << "UID: 1000, Password Hint: " << password_hint;

    password_hint = uniauthService->GetUserPasswdHint(1002);
    qDebug() << "UID: 1002, Password Hint: " << password_hint;

    return 0;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UniAuthService *uniauthService = new UniAuthService();

    TEST1(uniauthService);
    TEST2(uniauthService);
    TEST3(uniauthService);

    #if 0
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
    #endif

    uniauthService->deleteLater();
    uniauthService = nullptr;
    return a.exec();
}
