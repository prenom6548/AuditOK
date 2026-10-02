#ifndef KEYBOARDMAIN_H
#define KEYBOARDMAIN_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QSlider>
#include <QLineEdit>
#include <QGSettings/QGSettings>
#include <QPushButton>
#include <QProcess>
#include <QDebug>
#include <QtDBus/QDBusInterface>
#include <QEvent>
#include "shell/interface.h"
#include "keyboardui.h"
#include "ukcccommon.h"

#include "inputmethodconfigtool.h"

using namespace ukcc;

class UkccArgEvent : public QEvent
{
private:
    QString mstr;

public:
    const static Type TYPE = static_cast<Type>(QEvent::User+0x10);
    explicit UkccArgEvent(QString s) : QEvent(TYPE)
    {
        mstr = s;
    }

    QString str()
    {
        return mstr;
    }
};

class KeyboardMain : public QWidget, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)
public:
    KeyboardMain();
    ~KeyboardMain() Q_DECL_OVERRIDE;

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget *pluginUi() Q_DECL_OVERRIDE;
    const QString name() const Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

private:
    bool            mFirstLoad      = true;
    QString         pluginName      = "";
    int             pluginType      = 0;
    KeyboardUi      *keyBoardWidget = nullptr;
    QDBusInterface  *keyBoardDbus   = nullptr;
    InputMethodConfigTool *inputMethodConfigTool = nullptr;

private:
    bool isCloudDesktop();
    void initConnection();
    void initKeyboardStatus();
    void initKeyRepeat();
    void initDelay();
    void initSpeed();
    void initKeyTips();
    void initShowIconOnTray();
    void initShowIconOnDesktop();
    void initKeyboardSize();
    void initTextSize();
    void initAnimation();
    void initSoundEfects();

    void setKeyboardVisible(bool checked);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

public slots:
    void keyRepeatSlot(bool checked);
    void keyDelaySlot(int value);
    void keySpeedSlot(int value);
    void keyTipsSlot(bool checked);
    void inputMethodSetSlot();
    void fcitxServiceStatusChangedSlot(bool status);
    void fcitxInputMethodChangedSlot(const QString &firstImName);

    void showIconOnTraySlot(bool checked);
    void showIconOnDesktopSlot(bool checked);
    void keyboardSizeSlot(int value);
    void textSizeSlot(int index);
    void animationSlot(bool checked);
    void soundEfectsSlot(bool checked);


    void dataChanged(QString key);
};
#endif // KEYBOARDMAIN_H
