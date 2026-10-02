#ifndef ACCEPTCHOOSEDATA_H
#define ACCEPTCHOOSEDATA_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QDBusInterface>
#include "../component/data_checkbox.h"
#include "../component/text_label.h"
class AcceptChooseData : public QWidget
{
    Q_OBJECT
public:
    AcceptChooseData(QWidget *parent = nullptr);
    ~AcceptChooseData();
    void initAppWidget();
    
private:
    void initUI();
    void initConn();
    void configSysData();
    void configUseData();
    void configAppData();   
    
    DataCheckBox *m_checkBoxApp = nullptr;
    DataCheckBox *m_checkBoxUser = nullptr;
    DataCheckBox *m_checkBoxSys = nullptr;
    K::TextLabel *m_lable = nullptr;
    K::TextLabel *m_lableSize = nullptr;

    QPushButton *m_btnBack = nullptr;
    QPushButton *m_btnStart = nullptr;
    
    QHBoxLayout *m_layoutH = nullptr;
    QHBoxLayout *m_layoutHLabel = nullptr;
    QVBoxLayout *m_layoutVMain = nullptr; // 主界面纵向布局
    
    QString m_appDataYamlFile = "/usr/share/kylin-os-manager/replacement/data/appClassification.yaml"; // app的列表
    QString m_configDataYamlFile = "/usr/share/kylin-os-manager/replacement/data/etcPolicy.yaml"; // etc的列表
    const QString confDirPath = "/usr/share/kylin-os-manager/replacement/data/";

signals:
    void sigBackPage();
    void sigChooseData(ChooseDataType type);

public slots:
    void slotBackPage();
    void slotStartMigration();
    void slotChangeBtnStyle();
    void slotMigrationDataSize(qint64 num);
};
#endif