#ifndef UTILSFORUSERINFO_H
#define UTILSFORUSERINFO_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QPainter>
#include <QPainterPath>
#include <QToolButton>

#include <kysdk/applications/klineframe.h>
#include <kysdk/applications/klabel.h>
#include <kysdk/applications/kpushbutton.h>
using namespace kdk;

#include "ukcccommon.h"
using namespace ukcc;
#include "ukccframe.h"

class QFrame;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;

class UtilsForUserinfo : public QObject
{
    Q_OBJECT
public:
    explicit UtilsForUserinfo(bool isExisMfa = false, QObject *parent = nullptr);

public:
    UkccFrame * buildItemForUsers();

    void refreshUserLogo(QString logo);
    void refreshUserNickname(QString name);
    void refreshUserType(int type);
    void refreshDelStatus(bool enabled);
    void refreshTypeStatus(bool enabled);
    void refreshPwdStatus(bool enabled);
    void setObjectPathData(QString op);

    void setMoreBtnVisible(bool visible);
    void setDelVisible(bool visible);
    void setPwdVisible(bool visible);
    void setTypeVisible(bool visible);

    QToolButton *getUtilsToolBtn();

public:
    UkccFrame * m_pMainItemFrame;
    QString m_username;
    KLabel * logoBtn;
    KPushButton * changeTypeBtn;
    KPushButton * changePwdBtn;
    KPushButton * delUserBtn;

    KLabel * nickNameLabel;
    KLabel * typeLabel;

    QVBoxLayout * mainUserVerLayout;
    QVBoxLayout * subUserInfoVerLayout;
    QHBoxLayout * mUserInfoHorLayout;

    QToolButton *m_pMoreBtn = nullptr;
    QAction *m_pChangePwdAct = nullptr;
    QAction *m_pChangeTypeAct = nullptr;
    QAction *m_pDeleteAct = nullptr;
    QAction *m_pBindOtpAct = nullptr;
    QAction *m_pBindUkeyAct = nullptr;
    bool m_isServerVersion = false;
    bool m_isExistMfa = false;

private:
    QString _accountTypeIntToString(int type);
    QPixmap makeRoundLogo(QString logo, int wsize, int hsize, int radius);

    void initUI();
    void initConnections();

public slots:
    void userPropertyChangedSlot(QString, QMap<QString, QVariant>, QStringList);

signals:
    void changePwdBtnPressed();
    void changeTypeBtnPressed();
    void deleteUserBtnPressed();
    void bindOtpActPressed();
    void bindUkeyActPressed();
};

#endif // UTILSFORUSERINFO_H
