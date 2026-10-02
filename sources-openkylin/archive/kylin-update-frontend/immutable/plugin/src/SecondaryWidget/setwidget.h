#ifndef SETWIDGET_H
#define SETWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QCheckBox>
#include <QDebug>
#include <QString>
#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QEvent>
#include <QComboBox>
#include <QAbstractItemView>
#include <QDBusPendingCall>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QMessageBox>
#include <QUrl>
#include <QSettings>
#include <QDialog>
#include <QApplication>
class SetWidget : public QDialog
{
    Q_OBJECT
public:

    QMap<QString,QVariant> serverMap;

    QString flag = nullptr;

    QWidget *showAreaWidget = nullptr;
    QScrollArea *scrollArea = nullptr;

    QLabel *titleText = nullptr;
    QPushButton* titleIcon = nullptr;
    QPushButton *titleBtnClose = nullptr;

    QLabel *serverAddressTitleLabel = nullptr;
    QLabel *tipsTextLabel = nullptr;
//    QLabel *ProtocolLabel = nullptr;
    QComboBox *ProtocolComboBox = nullptr;
    QLabel *PortLabel = nullptr;
    QLineEdit *PortEdit = nullptr;
    QLabel *AddressLabel = nullptr;
    QLineEdit *IPEdit = nullptr;



    QPushButton *resetBtn = nullptr;
    QPushButton *cancelBtn = nullptr;
    QPushButton *determineBtn = nullptr;

    QHBoxLayout* initTitleBar();
    QVBoxLayout* initBody();
    QVBoxLayout* initDownloadDir();    //初始化设置界面下载路径模块
    QVBoxLayout* initPackageClean();      //初始化设置界面包清理模块
    QVBoxLayout* initUpdatePackage();      //初始化设置界面包清理模块
    QVBoxLayout* initServerAddress();     //初始化服务器地址
    QHBoxLayout* initButton();
    void getInitialData();       //获取初始数据
    void slotClickComboBox1();    //combox1添加item
    void slotClickLineEdit1();    //combox1添加item
    SetWidget(QWidget* parent = nullptr);

public Q_SLOTS:
    void slotSoftUpdateCheckBox5();  //处理应用更新处按钮的点击
    void slotSoftUpdateCheckBox6();  //处理应用更新处按钮的点击
    void slotRestoreDefaultSettings();     //恢复默认设置
    void slotModifysettings();     //修改设置

};

#endif // SETWIDGET_H
