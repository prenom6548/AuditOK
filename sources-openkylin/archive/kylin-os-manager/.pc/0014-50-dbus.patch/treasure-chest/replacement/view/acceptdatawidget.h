#ifndef ACCEPTDATAWIDGET_H
#define ACCEPTDATAWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include "acceptchoosedevice.h"
#include "acceptchoosedata.h"
#include "acceptrebootwidget.h"
#include "acceptmigrationdata.h"
#include "acceptdatastatus.h"
#include "kborderlessbutton.h"
#include <QDBusInterface>
class AcceptDataWidget : public QWidget
{
    Q_OBJECT
public:
    AcceptDataWidget(QWidget *parent = nullptr);
    ~AcceptDataWidget();
    void ipAutoDiscovery();

private:
    void initUI();
    void initConn();
    void setRebootWidget();
    void setLayout();
    
    QStackedWidget *m_stackedWid = nullptr;

    AcceptDataStatus *m_acceptStatus = nullptr;
    AcceptChooseDevice *m_accpetChooseWidget = nullptr;
    AcceptChooseData *m_accperChooseData = nullptr;
    AcceptMigrationData *m_acceptMigration = nullptr;
    AcceptRebootWidget *m_acceptReboot = nullptr;
    QWidget *m_rebootWidget = nullptr;
    
    QPushButton *m_btnBack = nullptr;
    QPushButton *m_btnNext = nullptr;
    QPushButton *m_btnCannel = nullptr;
    QPushButton *m_btnReboot = nullptr;
    QPushButton *m_btnTip = nullptr;
    K::TextLabel *m_labelTip = nullptr;
    
    QVBoxLayout *m_layoutVMain = nullptr; // 主界面纵向布局
    QDBusInterface *m_dbus = nullptr;

    bool m_isDataMigration = false;

signals:
    void sigBackPage();

public slots:
    void slotBackPage();
    void slotNextPage();
    void slotMigrationData(ChooseDataType type);
    void slotCannel();
    void slotReboot();
    void ConnSuccess();
    void ConnFile(int errorType);
    void slotMigrationDataInfo(QStringList list);
    void slotMigrationDataSize(qint64 num);
    void slotMigrationDataFinish();

};
#endif