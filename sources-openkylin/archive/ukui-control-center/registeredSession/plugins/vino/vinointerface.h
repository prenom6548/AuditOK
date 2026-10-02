#ifndef VINOINTERFACE_H
#define VINOINTERFACE_H

#include <ukui-service-interface/serviceobject.h>
#include <QDBusArgument>
#include <QVariant>
#include <QDBusMetaType>
#include "../../universalinterface.h"

class VinoInterface : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.Vino")
    Q_PROPERTY(bool isViewOnly READ getViewOnly)
    Q_PROPERTY(bool isPrompt READ getPromptEnabled)
    Q_PROPERTY(QString method READ getMethod)
    Q_PROPERTY(QString password READ getPassword)
    Q_PROPERTY(bool isActive READ getVinoServerStatus)
    Q_PROPERTY(bool isVino READ getVinoStatus)
    Q_PROPERTY(bool isXrdp READ getXrdpStatus)
    Q_PROPERTY(bool isEnable READ getEnableStatus)

public:
    VinoInterface();

protected:
    bool getViewOnly();
    bool getPromptEnabled();
    QString getMethod();
    QString getPassword();
    bool getVinoServerStatus();
    bool getVinoStatus();
    bool getXrdpStatus();
    bool getEnableStatus();

public Q_SLOTS:
    void setVinoKey(const QString &key, bool value);
    void setVinoKey(const QString &key, QString value);
    void setVinoKey(const QString &key, QByteArray value);

private:
    QString kEnableKey =  "enabled";
    QString kXrdpEnableKey = "xrdp-enabled";
    QString kVinoEnableKey = "vino-enabled";
    QString kVinoViewOnlyKey = "view-only";
    QString kVinoPromptKey = "prompt-enabled";
    QString kAuthenticationKey = "authentication-methods";
    QString kVncPwdKey = "vnc-password";

    QGSettings *mVinoGsettings     = nullptr;
    QGSettings *mUkuiVinoGsettings = nullptr;
    QMap <QString, QString> mGsettingMap;
};

#endif // VINOINTERFACE_H
