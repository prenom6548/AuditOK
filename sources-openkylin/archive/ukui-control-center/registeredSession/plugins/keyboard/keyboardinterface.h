#ifndef KEYBOARDINTERFACE_H
#define KEYBOARDINTERFACE_H

#include <ukui-service-interface/serviceobject.h>
#include "../../universalinterface.h"

class KeyBoardInterface : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.KeyBoard")
    Q_PROPERTY(bool repeat READ getKeyRepeat)
    Q_PROPERTY(int delay READ getDelay)
    Q_PROPERTY(int rate READ getRate)
    Q_PROPERTY(bool showLockTip READ getShowLockTip)

    Q_PROPERTY(bool showOnDesktop READ getShowOnDesktop)
    Q_PROPERTY(bool showOnTray READ getShowOnTray)
    Q_PROPERTY(int keyboardSize READ getKeyboardSize)
    Q_PROPERTY(int textSize READ getTextSize)
    Q_PROPERTY(bool animation READ getAnimation)
    Q_PROPERTY(bool soundEffect READ getSoundEffect)

public:
    KeyBoardInterface();
    ~KeyBoardInterface();

    void initConnect();

private:

    QString    KEYREPEAT_KEY       = QString("repeat");
    QString    DELAY_KEY           = QString("delay");
    QString    SPEED_KEY           = QString("rate");

    QString    KEYTIPS_KEY         = QString("showLockTip");

    QString    FLOAT_KEY           = QString("floatButtonEnabled");
    const QString    TRAY_KEY            = QString("");
    QString    SCALE_KEY           = QString("virtualKeyboardScaleFactor");
    const QString    TEXT_SIZE_KEY       = QString("");
    QString    ANIMATION_KEY       = QString("animationEnabled");
    const QString    SOUND_EFFECT_KEY    = QString("");


    QGSettings       *keyboardGsettings  = nullptr;
    QGSettings       *ukccOsdGsettings   = nullptr;
    QGSettings       *ukuiVirKeyBoardGsettings = nullptr;

protected:
    // 对属性的访问函数
    const bool getKeyRepeat();
    const int  getDelay();
    const int  getRate();
    const bool getShowLockTip();

    const bool getShowOnDesktop();
    const bool getShowOnTray();
    const int  getKeyboardSize();
    const int  getTextSize();
    const bool getAnimation();
    const bool getSoundEffect();

public Q_SLOTS:
    // 对属性的设置函数
    void setKeyRepeat(const bool checked);
    void setDelay(const int value);
    void setSpeed(const int value);
    void setKeyTips(const bool checked);
    void setIconOnDesktop(const bool checked);
    void setIconOnTray(const bool checked);
    void setKeyboardSize(const int value);
    void setTextSize(const int value);
    void setAnimation(const bool checked);
    void setSoundEffect(const bool checked);

    void resetKey(QString key);

Q_SIGNALS:
    void changed(QString keyName);
};

#endif // KEYBOARDINTERFACE_H
