#ifndef CHANGEUSERGROUP_H
#define CHANGEUSERGROUP_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QListWidget>
#include <QScrollArea>
#include <QLineEdit>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QDBusReply>
#include <QList>
#include <QDebug>
#include <QScrollBar>
#include <polkit-qt5-1/polkitqt1-authority.h>
#include <QCoreApplication>
#include <QCheckBox>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QTableWidget>

#include "creategroupdialog.h"
#include "qtdbus/systemdbusdispatcher.h"
#include "userinfo.h"

struct custom_struct
{
    QString groupname;
    QString passphrase;
    QString groupid;
    QString usergroup;

    friend QDBusArgument &operator<<(QDBusArgument &argument, const custom_struct&mystruct)
    {
        argument.beginStructure();
        argument << mystruct.groupname << mystruct.passphrase << mystruct.groupid << mystruct.usergroup;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, custom_struct&mystruct)
    {
        argument.beginStructure();
        argument >> mystruct.groupname >> mystruct.passphrase >> mystruct.groupid >> mystruct.usergroup;
        argument.endStructure();
        return argument;
    }

};
Q_DECLARE_METATYPE(custom_struct)

typedef struct _UserInfomations {
    QString objpath;
    QString username;
    QString iconfile;
    QString passwd;
    int accounttype;
    int passwdtype;
    bool current;
    bool logined;
    bool autologin;
    bool noPwdLogin;
    qint64 uid;
}UserInfomations;

class FrameBox : public QFrame
{
    Q_OBJECT
public:
    explicit FrameBox(QWidget *parent) {};
    ~FrameBox() {};

protected:
    void paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, true);
        if (qApp->property("kline-normal").canConvert<QBrush>()) {
            QBrush borderColor = qApp->property("kline-normal").value<QBrush>();
            painter.setPen(QPen(borderColor, 1));
        }
        QRect rect = this->rect();
        painter.drawRoundedRect(rect.x(), rect.y(), rect.width(), rect.height(), 0, 0);
        painter.restore();
        QFrame::paintEvent(event);
    }
};

class changeUserGroup : public KDialog
{
    Q_OBJECT

public:
    changeUserGroup(QWidget *parent = nullptr);
    ~changeUserGroup();

    void initUI();
    void initStatus();
    void initConnect();
    bool polkitEdit();
    void setEnableStatus(bool status);

public:
    QList<QSharedPointer<custom_struct>> groupList;

private:
    QVBoxLayout *vlayout = nullptr;

    FrameBox *mUserGroupListFrame;
    QFrame *mAddOrDelUserGroupFrame;
    QFrame *mUserGroupInfoFrame;
    QFrame *mUserGroupTableFrame;

    QPushButton *mAddUserGroupButton;
    QPushButton *mDelUserGroupButton;

    KLabel *mUserGroupLabel;
    KLabel *mUserGroupIdLabel;
    KLabel *mUserGroupNumbersLabel;

    QLineEdit *mUserGroupLineEdit;
    QLineEdit *mUserGroupIdLineEdit;

    FrameBox *mUserGroupNumbersListFrame;

    KPushButton *mCancelButton;
    KPushButton *mConfirmButton;

    QDBusInterface *serviceInterface;

    QList<QSharedPointer<custom_struct>> passwdList;

    SystemDbusDispatcher * sysdispatcher;
    QMap<QString, UserInfomations> allUserInfoMap;
    QTableView *allUserGroupTableView = nullptr;
    QStandardItemModel *mAllUserGroupModel = nullptr;
    QTableView *mUserGroupNumbersTableView = nullptr;
    QStandardItemModel *mUserGroupNumbersModel = nullptr;

    int currentRow = 0;
    bool _nameHasModified;
    bool _idHasModified;
    bool _boxModified;
    bool _deleteable = false;
    bool _addable = false;
    bool _editable;
    bool _deleted = false;
    int m_currentUserType = 1;

private:
    void connectToServer();
    void loadGroupInfo();
    void loadPasswdInfo();
    void loadAllGroup();
    void getUsersList(QString usergroup, QString groupName);
    UserInfomations _acquireUserInfo(QString objpath);
    void refreshCertainBtnStatus();
    void limitInput();
    void nameSetEnabled(bool isEnable);
    void idSetEnabled(bool idSetEnable);
    void refreshList();
    bool setTextDynamic(QLineEdit *lineEdit, QString string);
    void refreshDetialPage();

    void addUserGroupSlot();
    void delUserGroupSlot();
    void cancelUserGroupSlot();
    void saveUserGroupInfoSlot();
    void editUserGroupInfoSlot();
    void needRefreshSlot();

Q_SIGNALS:
    void needRefresh();
    void showDialog();
};
#endif // CHANGEUSERGROUP_H
