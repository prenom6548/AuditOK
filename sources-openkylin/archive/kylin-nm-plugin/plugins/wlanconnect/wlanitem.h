#ifndef WLANITEM_H
#define WLANITEM_H
#include <QObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <QImage>
#include <QMouseEvent>
#include <QMenu>
#include <QCheckBox>
#include <kylin-nm/kylin-nm-interface.h>

#include "../component/InfoButton/infobutton.h"
#include "../component/RadioItemButton/radioitembutton.h"
#include "../component/FixLabel/fixlabel.h"
#include "../component/FixLabel/freqlabel.h"
#include "../component/FixPushButton/fixpushbutton.h"
#include "kpasswordedit.h"
#include "checkedlineedit.h"
#include "hiddenwifi/enterprisewlanpage.h"
using namespace kdk;

#define FREQ_5GHZ 5000
#define MAX_WIDTH_ACTIVATED 142
#define MAX_WIDTH_DEACTIVATED 270

class WlanItem : public QFrame
{
    Q_OBJECT
public:
    WlanItem(PluginType type, QWidget *parent = nullptr);
public:
    void setName(QString name) {
        titileLabel->setLabelText(name);
    }

    QString getName() {
        return titileLabel->getText();
    }

    void setItemIcon(const QIcon &icon) {
        radioBtn->setButtonIcon(icon);
    }

    inline void updateNameLabel() {
        if (m_isAcitve) {
            titileLabel->setMaximumWidth(MAX_WIDTH_ACTIVATED);
        } else {
            titileLabel->setMaximumWidth(MAX_WIDTH_DEACTIVATED);
        }
        setName(titileLabel->getText());
    }

    inline void setNetSpeedUiVisible(bool visible) {
        m_lbLoadUp->setVisible(visible);
        m_lbLoadDown->setVisible(visible);
        m_lbLoadDownImg->setVisible(visible);
        m_lbLoadUpImg->setVisible(visible);
    }

    void setStatus(bool isAcitve){
        m_isAcitve = isAcitve;
        radioBtn->setActive(isAcitve);
        updateNameLabel();
        setNetSpeedUiVisible(isAcitve);
    }

    bool getStatus(){
        return m_isAcitve;
    }

    bool getIsLoading() {
        return m_loading;
    }

    QString getUuid(){
        return m_uuid;
    }

    void setUuid(QString uuid){
        m_uuid = uuid;
    }

    QString getPath(){
        return m_dbusPath;
    }

    void setPath(QString dbusPath){
        m_dbusPath = dbusPath;
    }

    QString getSecuType(){
        return m_secutype;
    }

    void setSecuType(QString &secuType){
        m_secutype = secuType;
    }

    QString getInterface(){
        return m_interface;
    }

    void setInterface(QString &interface){
        m_interface = interface;
    }

    void setPasswordError(QString pwd) {
        m_pwdLineEdit->setState(LoginFailed);
        m_pwdLineEdit->setText(pwd);
        m_passwordErrorTxt->setVisible(true);
    }

    void setFrequency(uint freq, bool isMix) {
        if (isMix) {
            m_freq->setText("2.4/5G");
            return;
        }
        if (freq < FREQ_5GHZ) {
            m_freq->setText("2.4G");
        } else {
            m_freq->setText("5G");
        }
        return;
    }

    void setNetSpeed(QString strRcv, QString strTx) {
        m_lbLoadDown->setText(strRcv);
        m_lbLoadUp->setText(strTx);
    }

    void showEnterPricePage(QString devName, QWidget *widget);

    void startLoading();
    void stopLoading();

    void setExpend(bool);
    bool getExpend();

    void setRequestId(const QString &id) { m_agentRequestId = id; }
    QString requestId() const { return m_agentRequestId; }

Q_SIGNALS:
    void itemClick();
    void infoButtonClick();
    void connectButtonClick(QString , bool);
    void passwordFocusIn();
    void connectButtonClickForAgent(const QString &requestId, const QString &password, bool autoConnect);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    QTimer *waitTimer = nullptr;
    QList<QIcon> loadIcons;
    int currentIconIndex=0;

    QMenu *m_menu = nullptr;
    RadioItemButton* radioBtn = nullptr;
    InfoButton * infoLabel = nullptr;
    FixLabel * titileLabel = nullptr;
    QLabel *m_lbLoadDown = nullptr;
    QLabel *m_lbLoadUp = nullptr;
    QLabel *m_lbLoadDownImg = nullptr;
    QLabel *m_lbLoadUpImg = nullptr;
    QLabel *m_freq = nullptr;

    PluginType type;
    bool m_isAcitve = false;
    bool m_loading = false;
    bool m_isExpand = false;

    QString m_uuid = "";
    QString m_dbusPath ="";
    //ap类型
    QString m_secutype = "";
    QString m_interface = "";

    QFrame* m_nameFrame = nullptr;
    QFrame* m_expendFrame = nullptr;
    QFrame* m_pwdFrame = nullptr;
    QFrame* m_autoConnectFrame = nullptr;

    KPasswordEdit* m_pwdLineEdit = nullptr;
    QPushButton* m_connectButton = nullptr;
    QCheckBox* m_autoConnectCheckBox = nullptr;
    QLabel* m_autoConnectLabel = nullptr;
    QLabel* m_passwordErrorTxt = nullptr;

    QHBoxLayout* m_autoConnectFrameLyt = nullptr;
    QHBoxLayout* m_pwdFrameLyt = nullptr;

    EnterpriseWlanPage* m_enterPirsePage = nullptr;

    bool m_isIn = false;
    bool m_isFocusIn = false;
    QString m_agentRequestId;  // 当前agent请求ID，为空时表示普通点击

private Q_SLOTS:
    void updateIcon();
//    void onMenuTriggered(QAction *action);
    void onPwdEditorTextChanged();
    void onConnectButtonClicked();

Q_SIGNALS:
    void connectPeapConnect(KyEapMethodPeapInfo info, KyWirelessConnectSetting connSettingInfo);
    void connectTtlsConnect(KyEapMethodTtlsInfo info, KyWirelessConnectSetting connSettingInfo);
    void connectLeapConnect(KyEapMethodLeapInfo info, KyWirelessConnectSetting connSettingInfo);
    void connectPwdConnect(KyEapMethodPwdInfo info, KyWirelessConnectSetting connSettingInfo);
    void connectFastConnect(KyEapMethodFastInfo info, KyWirelessConnectSetting connSettingInfo);
};

#endif // WLANITEM_H
