#ifndef LANITEM_H
#define LANITEM_H
#include <QObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>


#include "../component/InfoButton/infobutton.h"
#include "../component/RadioItemButton/radioitembutton.h"
#include "../component/FixLabel/fixlabel.h"
#include "../component/FixLabel/freqlabel.h"

#define MAX_WIDTH_ACTIVATED 190
#define MAX_WIDTH_DEACTIVATED 326

class LanItem : public QFrame
{
    Q_OBJECT
public:
    LanItem(bool isSimple, bool isOobe = false, QWidget *parent = nullptr);

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

    void setNetSpeed(QString strRcv, QString strTx) {
        m_lbLoadDown->setText(strRcv);
        m_lbLoadUp->setText(strTx);
    }

    void startLoading();
    void stopLoading();


Q_SIGNALS:
    void itemClick();
    void infoButtonClick(QString);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);

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

    bool m_loading = false;
    bool m_isAcitve = false;
    bool isSimple;
    bool m_isOobe;

    QString m_uuid;
    QString m_dbusPath;

    bool m_isIn = false;

private Q_SLOTS:
    void updateIcon();
};

#endif // LANITEM_H
