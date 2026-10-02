#ifndef SENDOLDSYSINFO_H
#define SENDOLDSYSINFO_H

#include <QWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QDBusInterface>
#include "../component/text_label.h"
#include "qzeroconf.h"

const QString machineId_path = "/etc/machine-id";

class OldSysInfo : public QWidget
{
    Q_OBJECT
public:
    OldSysInfo(QWidget *parent = nullptr);
    ~OldSysInfo();

protected:
    void paintEvent(QPaintEvent *event);

private:
    void initUI();
    void getOldInfo();
    void createRandomCode(); // 这是一个用来生成验证码的函数
    void changeBtnType();
    bool regularTestCode(QString string);
    void initZeroConf();
    
    K::TextLabel *m_labelId = nullptr;
    K::TextLabel *m_labelName = nullptr;
    K::TextLabel *m_labelPwd = nullptr;
    QLineEdit *m_editId = nullptr;
    QLineEdit *m_editName = nullptr;
    QLineEdit *m_editPwd = nullptr;
    QPushButton *m_btnShareData = nullptr;
    K::TextLabel *m_labelTip = nullptr;

    QVBoxLayout *m_layoutVMain = nullptr; // 主界面纵向布局
    QHBoxLayout *m_layoutHMain = nullptr;
    QFormLayout *m_layoutF = nullptr;

    QDBusInterface *m_dbus = nullptr;

    QString m_code = "";
    int m_num = 8;
    QZeroConf m_zeroConf;
    QString m_nameStr = "";

public slots:
    void slotStarShare();
    void slotStopShare();

};
#endif