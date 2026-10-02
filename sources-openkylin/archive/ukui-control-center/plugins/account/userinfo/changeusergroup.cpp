/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "changeusergroup.h"
#include <kysdk/applications/accessinfohelper.h>
#include <kysdk/applications/klineframe.h>
using namespace kdk;

#ifdef signals
#undef signals
#endif
#include <glib.h>
#include <gio/gio.h>

changeUserGroup::changeUserGroup(QWidget *parent)
    : KDialog(parent)
{
    this->setWindowTitle(tr("User Group"));
    setWindowIcon("ukui-control-center");
    this->setFixedSize(600, 520);

    initUI();
    initStatus();
    initConnect();
}

changeUserGroup::~changeUserGroup()
{
}

void changeUserGroup::initUI()
{
    vlayout = new QVBoxLayout(this);
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(24, 16, 24, 24);

    QHBoxLayout *hUserGrouplayout = new QHBoxLayout;
    hUserGrouplayout->setContentsMargins(0, 0, 0, 0);
    hUserGrouplayout->setSpacing(8);

    // 左侧所有用户组
    QVBoxLayout *vUserGroupLeftLayout = new QVBoxLayout;
    vUserGroupLeftLayout->setSpacing(0);
    vUserGroupLeftLayout->setContentsMargins(1,1,0,0);
    mUserGroupListFrame = new FrameBox(this);
    mUserGroupListFrame->setContentsMargins(0,0,0,0);
    mUserGroupListFrame->setObjectName("mUserGroupListFrame");
    mUserGroupListFrame->setFixedSize(240, 388);

    // 左侧用户组
    mUserGroupTableFrame = new QFrame(this);
    QHBoxLayout *layout = new QHBoxLayout(mUserGroupTableFrame);
    layout->setMargin(0);
    // 初始化用户组列表
    mUserGroupTableFrame->setFixedSize(238, 325);
    allUserGroupTableView = new QTableView(this);
    mUserGroupTableFrame->layout()->addWidget(allUserGroupTableView);
    allUserGroupTableView->setFixedHeight(325);
    allUserGroupTableView->setStyleSheet("QTableView{border: 0px solid palette(midlight);}");
    mAllUserGroupModel = new QStandardItemModel(this);

    allUserGroupTableView->setModel(mAllUserGroupModel);
    allUserGroupTableView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    allUserGroupTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    allUserGroupTableView->setAlternatingRowColors(true);
    allUserGroupTableView->setMouseTracking(true);
    allUserGroupTableView->verticalHeader()->setVisible(false);//隐藏垂直表头
    allUserGroupTableView->horizontalHeader()->setVisible(false);//隐藏水平表头
    allUserGroupTableView->setShowGrid(false);//不显示表格线
    allUserGroupTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//水平表格自动拉伸
    allUserGroupTableView->verticalHeader()->setDefaultSectionSize(36);//行高默认为36
    this->setWindowModality(Qt::ApplicationModal); //模态

    allUserGroupTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑
    mAllUserGroupModel->setColumnCount(1);

    // 左侧添加删除按钮
    mAddUserGroupButton =new QPushButton(this);
    mAddUserGroupButton->setFixedSize(36, 36);
    KDK_EXTEND_ALL_INFO_FORMAT(mAddUserGroupButton, "UserInfo", "", "");
    mAddUserGroupButton->setAccessibleName(tr("Add User Group"));
    QIcon mAddIcon = QIcon::fromTheme("list-add-symbolic");
    mAddUserGroupButton->setIcon(mAddIcon);
    mDelUserGroupButton = new QPushButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mDelUserGroupButton, "UserInfo", "", "");
    mDelUserGroupButton->setAccessibleName(tr("Del User Group"));
    mDelUserGroupButton->setFixedSize(36, 36);
    QIcon mDelIcon = QIcon::fromTheme("window-minimize-symbolic");
    mDelUserGroupButton->setIcon(mDelIcon);

    QHBoxLayout *addOrDelUserGroupHLayout = new QHBoxLayout();
    addOrDelUserGroupHLayout->setContentsMargins(10, 8, 8, 8);
    addOrDelUserGroupHLayout->addWidget(mAddUserGroupButton);
    addOrDelUserGroupHLayout->addSpacing(8);
    addOrDelUserGroupHLayout->addWidget(mDelUserGroupButton);
    addOrDelUserGroupHLayout->setAlignment(Qt::AlignVCenter);
    addOrDelUserGroupHLayout->addStretch();

    KHLineFrame *listAndAddLine = new KHLineFrame();
    vUserGroupLeftLayout->addWidget(mUserGroupTableFrame);
    vUserGroupLeftLayout->addWidget(listAndAddLine);
    vUserGroupLeftLayout->addLayout(addOrDelUserGroupHLayout);
    mUserGroupListFrame->setLayout(vUserGroupLeftLayout);

    // 右侧用户组编辑页
    QVBoxLayout *vUserGroupRightLayout = new QVBoxLayout;
    vUserGroupRightLayout->setContentsMargins(0, 0, 0, 0);
    vUserGroupRightLayout->setSpacing(0);
    mUserGroupInfoFrame = new QFrame(this);
    mUserGroupInfoFrame->setFixedSize(288, 388);
    mUserGroupInfoFrame->setObjectName("mUserGroupInfoFrame");

    // 用户组
    QHBoxLayout *userGroupHlayout = new QHBoxLayout;
    userGroupHlayout->setContentsMargins(0, 0, 0, 0);
    mUserGroupLabel = new KLabel(this);
    mUserGroupLabel->setText(tr("Group"));
    mUserGroupLabel->setFixedWidth(90);
    mUserGroupLineEdit = new QLineEdit(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mUserGroupLineEdit, "UserInfo", "", "");
    mUserGroupLineEdit->setAccessibleName(tr("Group"));
    mUserGroupLineEdit->setFixedWidth(190);
    userGroupHlayout->addWidget(mUserGroupLabel);
    userGroupHlayout->addWidget(mUserGroupLineEdit);

    // 用户ID
    QHBoxLayout *userGroupIdHlayout = new QHBoxLayout;
    userGroupIdHlayout->setContentsMargins(0, 0, 0, 0);
    mUserGroupIdLabel = new KLabel(this);
    mUserGroupIdLabel->setText(tr("GID"));
    QString language = QLocale::system().name();
    // 维吾尔 哈萨克斯 柯尔克孜 语言控件翻转
    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
       mUserGroupIdLabel->setAlignment(Qt::AlignRight);
    }
    mUserGroupIdLabel->setFixedWidth(90);
    mUserGroupIdLineEdit = new QLineEdit(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mUserGroupIdLineEdit, "UserInfo", "", "");
    mUserGroupIdLineEdit->setAccessibleName(tr("GID"));
    mUserGroupIdLineEdit->setFixedWidth(190);
    userGroupIdHlayout->addWidget(mUserGroupIdLabel);
    userGroupIdHlayout->addWidget(mUserGroupIdLineEdit);

    // 组成员
    QHBoxLayout *userGroupNumbersHlayout = new QHBoxLayout;
    userGroupNumbersHlayout->setContentsMargins(0, 0, 0, 0);

    mUserGroupNumbersListFrame = new FrameBox(this);
    mUserGroupNumbersListFrame->setFixedSize(190, 268);
    QHBoxLayout *hlayout = new QHBoxLayout(mUserGroupNumbersListFrame);
    hlayout->setContentsMargins(0,0,0,0);
    mUserGroupNumbersLabel = new KLabel(this);
    mUserGroupNumbersLabel->setText(tr("GNum"));
    mUserGroupNumbersLabel->setFixedSize(90, 271);
    mUserGroupNumbersLabel->setAlignment(Qt::AlignTop);
    userGroupNumbersHlayout->addWidget(mUserGroupNumbersLabel);
    userGroupNumbersHlayout->addWidget(mUserGroupNumbersListFrame);

    vUserGroupRightLayout->addLayout(userGroupHlayout);
    vUserGroupRightLayout->addSpacing(24);
    vUserGroupRightLayout->addLayout(userGroupIdHlayout);
    vUserGroupRightLayout->addSpacing(24);
    vUserGroupRightLayout->addLayout(userGroupNumbersHlayout);
    mUserGroupInfoFrame->setLayout(vUserGroupRightLayout);
    mUserGroupInfoFrame->setContentsMargins(0, 0, 0, 0);

    hUserGrouplayout->addWidget(mUserGroupListFrame);
    hUserGrouplayout->addStretch();
    hUserGrouplayout->addWidget(mUserGroupInfoFrame);

    // 初始化组成员列表
    mUserGroupNumbersTableView = new QTableView(this);
    mUserGroupNumbersListFrame->layout()->addWidget(mUserGroupNumbersTableView);
    mUserGroupNumbersTableView->setFixedSize(190, 268);
    mUserGroupNumbersModel = new QStandardItemModel(this);

    mUserGroupNumbersTableView->setModel(mUserGroupNumbersModel);
    mUserGroupNumbersTableView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    mUserGroupNumbersTableView->setAlternatingRowColors(true);
    mUserGroupNumbersTableView->setMouseTracking(true);
    mUserGroupNumbersTableView->verticalHeader()->setVisible(false);//隐藏垂直表头
    mUserGroupNumbersTableView->horizontalHeader()->setVisible(false);//隐藏水平表头
    mUserGroupNumbersTableView->setShowGrid(false);//不显示表格线
    mUserGroupNumbersTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//水平表格自动拉伸
    mUserGroupNumbersTableView->verticalHeader()->setDefaultSectionSize(36);//行高默认为36
    this->setWindowModality(Qt::ApplicationModal); //模态

    mUserGroupNumbersTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑
    mUserGroupNumbersModel->setColumnCount(1);

    // cancel and save
    QHBoxLayout *cancelAndConfirmHLayout = new QHBoxLayout;
    cancelAndConfirmHLayout->setContentsMargins(0, 0, 0, 0);
    cancelAndConfirmHLayout->setSpacing(16);
    mCancelButton = new KPushButton(this);
    mCancelButton->setText(tr("Cancel"));
    mCancelButton->setFixedWidth(110);
    mConfirmButton = new KPushButton(this);
    mConfirmButton->setText(tr("Save"));
    mConfirmButton->setFixedWidth(110);
    mConfirmButton->setBackgroundColorHighlight(true);
    cancelAndConfirmHLayout->addStretch();
    cancelAndConfirmHLayout->addWidget(mCancelButton);
    cancelAndConfirmHLayout->addWidget(mConfirmButton);

    vlayout->addLayout(hUserGrouplayout);
    vlayout->addSpacing(24);
    vlayout->addLayout(cancelAndConfirmHLayout);

    mainWidget()->setLayout(vlayout);
}

void changeUserGroup::initStatus()
{
    connectToServer();
    loadGroupInfo();
    loadPasswdInfo();
    loadAllGroup();
    limitInput();
    nameSetEnabled(false);

    // 初始化右侧用户组信息默认显示第一组
    if (!groupList.isEmpty()) {
        mUserGroupLineEdit->setText(groupList.at(0)->groupname);
        mUserGroupIdLineEdit->setText(groupList.at(0)->groupid);
        getUsersList(groupList.at(0)->usergroup,groupList.at(0)->groupname);
        for(int j = 0; j < passwdList.size(); j++){
            _deleteable = true;
            if (passwdList.at(j)->groupid == groupList.at(0)->groupid) {
                _deleteable = false;
                idSetEnabled(false);
                break;
            }
        }
    }
    mDelUserGroupButton->setEnabled(_deleteable);

    allUserGroupTableView->selectRow(0);
    _nameHasModified = false;
    _idHasModified = false;
    _boxModified = false;

    refreshCertainBtnStatus();
}

void changeUserGroup::initConnect()
{
    connect(mAddUserGroupButton, &QPushButton::clicked, this, &changeUserGroup::addUserGroupSlot);
    connect(mDelUserGroupButton, &QPushButton::clicked, this, &changeUserGroup::delUserGroupSlot);
    connect(mCancelButton, &QPushButton::clicked, this, &changeUserGroup::cancelUserGroupSlot);
    connect(mConfirmButton, &QPushButton::clicked, this, &changeUserGroup::saveUserGroupInfoSlot);
    connect(this, &changeUserGroup::needRefresh, this, &changeUserGroup::needRefreshSlot);

    connect(mUserGroupLineEdit,&QLineEdit::textChanged,[=](QString txt){
        refreshCertainBtnStatus();
    });
    connect(mUserGroupIdLineEdit,&QLineEdit::textChanged,[=](QString txt){
        refreshCertainBtnStatus();
    });
    connect(mUserGroupIdLineEdit, &QLineEdit::textEdited,[=](){
        for (int j = 0; j < this->groupList.size(); j++){
            if(mUserGroupIdLineEdit->text() == this->groupList.at(j)->groupid){
                _idHasModified = false;
                return;
            }
        }
        _idHasModified = true;
    });
    connect(mUserGroupLineEdit, &QLineEdit::textEdited,[=](){
        _nameHasModified = true;
    });
}

void changeUserGroup::connectToServer()
{
    serviceInterface = new QDBusInterface("org.ukui.groupmanager",
                                          "/org/ukui/groupmanager",
                                          "org.ukui.groupmanager.interface",
                                          QDBusConnection::systemBus());
    if (!serviceInterface->isValid())
    {
        qDebug() << "fail to connect to service";
        qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
        return;
    }
    // 将以后所有DBus调用的超时设置为 milliseconds
    serviceInterface->setTimeout(2147483647); // -1 为默认的25s超时
    QTimer::singleShot(100, this, [=]{emit showDialog();});
}

void changeUserGroup::setEnableStatus(bool status)
{
    m_currentUserType = 0;
    mAddUserGroupButton->setEnabled(status);
    mDelUserGroupButton->setEnabled(status);
    mUserGroupInfoFrame->setEnabled(status);
}

void changeUserGroup::loadGroupInfo()
{
    qDebug() << "loadGroupInfo";
    groupList.clear();
    QDBusMessage msg = serviceInterface->call("getGroup");
    if(msg.type() == QDBusMessage::ErrorMessage) {
        printf("get group info fail.\n");
    }
    QDBusArgument argument = msg.arguments().at(0).value<QDBusArgument>();
    QList<QVariant> infos;
    argument >> infos;

    // 使用QList<QSharedPointer<custom_struct>> 自动管理内存
    for (int i = 0; i < infos.size(); i++) {
        auto dbus_struct = QSharedPointer<custom_struct>::create();
        infos.at(i).value<QDBusArgument>() >> *dbus_struct;
        groupList.append(dbus_struct);
    }
    // 无需手动释放，离开作用域时自动清理
}

void changeUserGroup::loadPasswdInfo()
{
    qDebug() << "loadPasswdInfo";
    QDBusMessage msg = serviceInterface->call("getPasswd");
    if(msg.type() == QDBusMessage::ErrorMessage) {
        printf("get passwd info fail.\n");
    }
    QDBusArgument argument = msg.arguments().at(0).value<QDBusArgument>();
    QList<QVariant> infos;
    argument >> infos;

    // 使用QList<QSharedPointer<custom_struct>> 自动管理内存
    for (int i = 0; i < infos.size(); i++) {
        auto dbus_struct = QSharedPointer<custom_struct>::create();
        infos.at(i).value<QDBusArgument>() >> *dbus_struct;
        passwdList.append(dbus_struct);
    }
    // 无需手动释放，离开作用域时自动清理
}

void changeUserGroup::limitInput()
{
    QIntValidator *intValidator = new QIntValidator;
    //QRegExp rx("^[a-zA-z]+$");// 首字符为字母
    QRegExp rx("[a-zA-z]{32}");
    QRegExpValidator *regValidator = new QRegExpValidator(rx);
    //intValidator->setRange(0, 65535);
    intValidator->setBottom(0);
    // 整形输入限制
    mUserGroupIdLineEdit->setValidator(intValidator);
    // 字母输入限制
    mUserGroupLineEdit->setValidator(regValidator);
    // 字符长度限制
    //ui->lineEdit_name->setMaxLength(4);
}

void changeUserGroup::nameSetEnabled(bool isEnable)
{
    mUserGroupLineEdit->setEnabled(isEnable);
}

void changeUserGroup::idSetEnabled(bool idSetEnable)
{
    mUserGroupIdLineEdit->setEnabled(idSetEnable);
}


void changeUserGroup::loadAllGroup()
{
    for(int i = 0; i < groupList.size(); i++){
//        singleWidget->setUpdateable(true);
        _editable = true;

        QStandardItem *singleItem = new QStandardItem(QIcon::fromTheme("stock-people-symbolic"), groupList.at(i)->groupname);
        singleItem->setAccessibleDescription("group list in" + groupList.at(i)->groupname);
        singleItem->setAccessibleText("ukui-control-center_QStandardItem_singleItem_" + groupList.at(i)->groupname);
        mAllUserGroupModel->setItem(i, singleItem);
    }

    connect(allUserGroupTableView,&QTableView::clicked,this,[=](const QModelIndex &index){
        currentRow = index.row();
        refreshDetialPage();
        UkccCommon::buriedSettings(QString("Userinfo"), this->windowTitle(), QString("clicked"), groupList.at(currentRow)->groupname);
    });
//    ui->listWidget->setSortingEnabled(true);
}


bool changeUserGroup::setTextDynamic(QLineEdit *lineEdit, QString string)
{

    bool isOverLength = false;
    QFontMetrics fontMetrics(lineEdit->font());
    int labelLength= lineEdit->width();
    int fontLength = fontMetrics.width(string);
    QString str = string;
    qDebug() << "fontLength:" << fontLength << ";labelLength:" << labelLength;
    if (fontLength > labelLength-15) {
        str = fontMetrics.elidedText(string, Qt::ElideRight, labelLength-15);
        isOverLength = true;
    }
    lineEdit->setText(str);
    return isOverLength;

}

void changeUserGroup::getUsersList(QString usergroup, QString groupName)
{
    QStringList allUsers;
    sysdispatcher = new SystemDbusDispatcher(this);

    QStringList objectpaths = sysdispatcher->list_cached_users();
    allUserInfoMap.clear();
    disconnect(mUserGroupNumbersModel, &QStandardItemModel::itemChanged , this, nullptr);
    if (mUserGroupNumbersModel != nullptr) {
        mUserGroupNumbersModel->clear();
    }

    //root
    if (!getuid()){
        UserInfomations root;
        root.username = g_get_user_name();
        root.current = true;
        root.logined = true;
        root.autologin = false;
        root.uid = 0;
        root.accounttype = ADMINISTRATOR;
        //        root.iconfile = DEFAULTFACE;
        allUserInfoMap.insert(root.username, root);
    }
    for (QString objectpath : objectpaths){
        UserInfomations user;
        user = _acquireUserInfo(objectpath);
        allUserInfoMap.insert(user.username, user);
    }
    for (QVariant tmp : allUserInfoMap.keys()){
        allUsers << tmp.toString();

    }
    QStringList usersList = allUsers;
    QStringList usergroupList = usergroup.split(",");

    for(int i = 0; i < usersList.size(); i++){
        QStandardItem *item = new QStandardItem(usersList.at(i));
        item->setCheckable(true);
        item->setCheckState(Qt::Unchecked);
        item->setEditable(false);
        mUserGroupNumbersModel->setItem(i,item);

        if(usersList.at(i) == groupName){
            item->setCheckState(Qt::Checked);
            item->setCheckable(false);
        } else {
            for (int j = 0; j < usergroupList.size(); j ++){
                if(usergroupList.at(j) == usersList.at(i)){
                    item->setCheckState(Qt::Checked);
                }
            }
        }

        // sudo组中当前用户不可退出
        if (groupName == "sudo" && usersList.at(i) == g_get_user_name()) {
            item->setCheckable(false);
            item->setEnabled(false);
        }
    }
    connect(mUserGroupNumbersModel, &QStandardItemModel::itemChanged, this, [=](QStandardItem *item){
        qDebug() << "checkbox clicked";
        _boxModified = true;
        refreshCertainBtnStatus();
    });
}

UserInfomations changeUserGroup::_acquireUserInfo(QString objpath)
{
    UserInfomations user;

    //默认值
    user.current = false;
    user.logined = false;
    user.autologin = false;

    QDBusInterface * iproperty = new QDBusInterface("org.freedesktop.Accounts",
                                            objpath,
                                            "org.freedesktop.DBus.Properties",
                                            QDBusConnection::systemBus());
    QDBusReply<QMap<QString, QVariant> > reply = iproperty->call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()){
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        user.username = propertyMap.find("UserName").value().toString();
        if (user.username == QString(g_get_user_name())) {
            user.current = true;
            user.logined = true;
        }
    }
    else
        qDebug() << "reply failed";

    delete iproperty;
    iproperty = nullptr;

    return user;
}

void changeUserGroup::refreshCertainBtnStatus(){
    if (mUserGroupLineEdit->text().isEmpty() ||
            mUserGroupIdLineEdit->text().isEmpty())
        mConfirmButton->setEnabled(false);
    else
        mConfirmButton->setEnabled(_nameHasModified || _idHasModified || _boxModified || _deleted || _addable);
}

void changeUserGroup::refreshList()
{
    qDebug() << "refresh list";
    mAllUserGroupModel->removeRows(0,mAllUserGroupModel->rowCount());

    loadGroupInfo();
    loadAllGroup();
}

bool changeUserGroup::polkitEdit()
{
    PolkitQt1::Authority::Result result;
    //PolkitQt1::SystemBusNameSubject subject(message().service());

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.ukui.groupmanager.action.edit",
                PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
                PolkitQt1::Authority::AllowUserInteraction);
    if (result == PolkitQt1::Authority::Yes) { //认证通过
        qDebug() << QString("operation authorized");
        return true;
    } else {
        qDebug() << QString("not authorized");
        return false;
    }
}

void changeUserGroup::refreshDetialPage()
{
    bool idSetEnable = true;
    if (setTextDynamic(mUserGroupLineEdit, groupList.at(currentRow)->groupname)) {
        mUserGroupLineEdit->setToolTip(groupList.at(currentRow)->groupname);
    }
    mUserGroupIdLineEdit->setText(groupList.at(currentRow)->groupid);
    getUsersList(groupList.at(currentRow)->usergroup,groupList.at(currentRow)->groupname);

    // 判断id是否可修改
    for(int j = 0; j < passwdList.size(); j++){
        _deleteable = true;
        if(passwdList.at(j)->groupid == groupList.at(currentRow)->groupid){
            _deleteable = false;
            idSetEnable = false;
            if (currentRow == 0) {
                idSetEnabled(idSetEnable);
            }
            break;
        }
    }

    // 管理员用户删除用户组按钮才可以高亮
    mDelUserGroupButton->setEnabled(_deleteable && m_currentUserType == 1);

    idSetEnabled(idSetEnable);
    _nameHasModified = false;
    _idHasModified = false;
    _boxModified = false;
}

// slots function
void changeUserGroup::addUserGroupSlot()
{
    CreateGroupDialog *dialog = new CreateGroupDialog(this);
    KPushButton *certainBtn = dialog->certainBtnComponent();
    QLineEdit *lineId = dialog->lineIdComponent();
    QLineEdit *lineName = dialog->lineNameComponent();

    connect(certainBtn, &KPushButton::clicked, this, [=](){
        for (int j = 0; j < groupList.size(); j++){
            if(lineId->text() == groupList.at(j)->groupid){
                QMessageBox invalid(QMessageBox::Question, tr("Tips"), tr("Invalid Id!"));
                invalid.setIcon(QMessageBox::Warning);
                invalid.setStandardButtons(QMessageBox::Ok);
                invalid.setButtonText(QMessageBox::Ok, QString(tr("OK")));
                invalid.exec();
                return;
            }
            if(lineName->text() == groupList.at(j)->groupname){
                QMessageBox invalid(QMessageBox::Question, tr("Tips"), tr("Invalid Group Name!"));
                invalid.setIcon(QMessageBox::Warning);
                invalid.setStandardButtons(QMessageBox::Ok);
                invalid.setButtonText(QMessageBox::Ok, QString(tr("OK")));
                invalid.exec();
                return;
            }
        }

        QDBusReply<bool> reply = serviceInterface->call("add",lineName->text(),lineId->text());
        if (reply) {
            // use the returned value
            qDebug() << "get call value" << reply.value();
            _addable = true;
            refreshCertainBtnStatus();
            refreshList();
            allUserGroupTableView->scrollToBottom();
            currentRow = mAllUserGroupModel->rowCount()-1;
            allUserGroupTableView->selectRow(currentRow);
            refreshDetialPage();
        } else {
            // call failed. Show an error condition.
            qDebug() << "call failed" << reply.error();
        }

        dialog->close();
    });
    dialog->exec();
}

void changeUserGroup::delUserGroupSlot()
{
    QMessageBox *delUserGroupMessage = new QMessageBox(this);
    delUserGroupMessage->setWindowModality(Qt::ApplicationModal);
    delUserGroupMessage->setIcon(QMessageBox::Warning);
    delUserGroupMessage->setText(tr("Whether delete the group: “%1” ?").arg(mAllUserGroupModel->index(currentRow,0).data().toString()));
    delUserGroupMessage->setInformativeText(tr("Which will make some file components in the file system invalid!"));
    QPushButton *cancelBtn = delUserGroupMessage->addButton(tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *confirmBtn = delUserGroupMessage->addButton(tr("Confirm"), QMessageBox::AcceptRole);

    delUserGroupMessage->exec();

    if (delUserGroupMessage->clickedButton() == confirmBtn) {
        QDBusReply<bool> reply = serviceInterface->call("del", allUserGroupTableView->currentIndex().data().toString());
        if (reply){
            // use the returned value
            qDebug() << "current index" << allUserGroupTableView->currentIndex();
            mAllUserGroupModel->removeRow(currentRow);
            allUserGroupTableView->scrollToBottom();
            mDelUserGroupButton->setEnabled(false);
            _deleted = true;
            refreshCertainBtnStatus();
            refreshList();
            currentRow = currentRow-1;
            allUserGroupTableView->selectRow(currentRow);
            refreshDetialPage();
        } else {
            // call failed. Show an error condition.
            qDebug() << "call failed" << reply.error();
        }
    }
}

void changeUserGroup::cancelUserGroupSlot()
{
    this->close();
}

void changeUserGroup::saveUserGroupInfoSlot()
{
    QStringList addUserList;
    QStringList delUserList;
    for (int i = 0; i < mUserGroupNumbersTableView->model()->rowCount(); i++) {
        if(_idHasModified){
            for (int j = 0; j < this->groupList.size(); j++){
                if(mUserGroupIdLineEdit->text() == this->groupList.at(j)->groupid){
                    QMessageBox invalid(QMessageBox::Question, tr("Tips"), tr("Invalid Id!"));
                    invalid.setIcon(QMessageBox::Warning);
                    invalid.setStandardButtons(QMessageBox::Ok);
                    invalid.setButtonText(QMessageBox::Ok, QString(tr("OK")));
                    invalid.exec();
                    return;
                }
            }
        }

        if (mUserGroupNumbersModel->item(i)->checkState()) {
            qDebug() << "---------i = " << i << "--------checked = " << mUserGroupNumbersModel->item(i)->text();
            addUserList << mUserGroupNumbersModel->item(i)->text();

        } else {
            delUserList << mUserGroupNumbersModel->item(i)->text();
            qDebug() << "---------i = " << i << "--------unchecked = " << mUserGroupNumbersModel->item(i)->text();

        }
    }
    QString groupName = mUserGroupLineEdit->text();
    if (mUserGroupLineEdit->toolTip() != nullptr) {
        groupName = mUserGroupLineEdit->toolTip();
    }


    QDBusReply<bool> setReply = this->serviceInterface->call("set",
                                    groupName,mUserGroupIdLineEdit->text());
    if (setReply){
        // use the returned value
        qDebug() << "set get call value" << setReply.value();
        QDBusReply<bool> addUserReply = this->serviceInterface->call("addUserToGroup",
                                        groupName, addUserList);
        if (addUserReply.isValid()){
            // use the returned value
            qDebug() << "addUserToGroup get call value" << addUserReply.value();
        } else {
            // call failed. Show an error condition.
            qDebug() << "addUserToGroup call failed" << addUserReply.error();
        }
        QDBusReply<bool> delUserReply = this->serviceInterface->call("delUserFromGroup",
                                        groupName, delUserList);
        if (delUserReply.isValid()){
            // use the returned value
            qDebug() << "delUserFromGroup get call value" << delUserReply.value();
        } else {
            // call failed. Show an error condition.
            qDebug() << "delUserFromGroup call failed" << delUserReply.error();
        }
        emit needRefresh();

        close();
    } else {
        // call failed. Show an error condition.
        qDebug() << "set call failed" << setReply.error();
    }

}

void changeUserGroup::editUserGroupInfoSlot()
{

}

void changeUserGroup::needRefreshSlot()
{
    refreshList();
}
