#ifndef ACCEPTCHOOSEDEVICE_H
#define ACCEPTCHOOSEDEVICE_H

#include <QWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QDBusInterface>
#include "../component/text_label.h"
#include "qzeroconf.h"

class AcceptChooseDevice : public QWidget
{
    Q_OBJECT
public:
    AcceptChooseDevice(QWidget *parent = nullptr);
    ~AcceptChooseDevice();
    void startZeroConf();
    void stopZeroConf();
    QStringList getInfo();
    void showOrHideTip(bool show);
    bool getAcceptDeviceInfo();

protected:
    void paintEvent(QPaintEvent *event);
private:
    void initUI();
        
    K::TextLabel *m_labelInfo = nullptr;
    K::TextLabel *m_labelId = nullptr;
    K::TextLabel *m_labelName = nullptr;
    K::TextLabel *m_labelPwd = nullptr;
    K::TextLabel *m_labelTip = nullptr;
    QComboBox *m_comboBoxId = nullptr;
    QLineEdit *m_editName = nullptr;
    QLineEdit *m_editPwd = nullptr;
    QPushButton *m_btnBack = nullptr;
    QPushButton *m_btnNext = nullptr;

    QVBoxLayout *m_layoutVMain = nullptr; // 主界面纵向布局
    QHBoxLayout *m_layoutHLabel = nullptr;
    QHBoxLayout *m_layoutHBtn = nullptr;
    QHBoxLayout *m_layoutHMain = nullptr;
    QFormLayout *m_layoutF = nullptr;
    
    QZeroConf m_zeroConf;

    QStringList m_list;
    QHash <QString, QStringList> m_hashIp;
    QString m_stringIp;
    QString m_stringName;
    QString m_stringPwd;

    void analyzName(QZeroConfService zcs);
    void comboxChooseId(const QString &text);
};
#endif