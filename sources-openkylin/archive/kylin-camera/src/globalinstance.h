#ifndef GLOBALINSTANCE_H
#define GLOBALINSTANCE_H

#include <QObject>

class GlobalInstance : public QObject
{
    Q_OBJECT
public:
    static GlobalInstance *instance;
    explicit GlobalInstance(QObject *parent = nullptr);    
    static GlobalInstance* getInstance();
    ~GlobalInstance();

    enum   XDG_SESSION_DESKTOP {
        UNKNOWN,
        KYLIN_WLCOM,
        UKUI_WAYLAND,
        UKUI
    };
    XDG_SESSION_DESKTOP session_desktop(){
        if(m_desktop_type == UNKNOWN){
            if(QString(qgetenv("XDG_SESSION_TYPE")) == "kylin-wlcom"){
                m_desktop_type = KYLIN_WLCOM;
            }else if(QString(qgetenv("XDG_SESSION_TYPE")) == "ukui-wayland"){
                m_desktop_type = UKUI_WAYLAND;
            }else if(QString(qgetenv("XDG_SESSION_TYPE")) == "ukui"){
                m_desktop_type = UKUI;
            }
        }
        return m_desktop_type;
    }

Q_SIGNALS:
    void dir_change();
    void video_dir_change();
    void change_dev(const char *);
    void change_resolutions(QPair<uint,uint>);
    void settingPageSig();
    void displayCameraErrorSig(int);
    void settingOptionsOn(bool);
    void optionStatus(bool);
    void sigCameraInfoSet(const char *in_devname, QString scale);

private:
    XDG_SESSION_DESKTOP m_desktop_type = UNKNOWN;
};

#endif // GLOBALINSTANCE_H
