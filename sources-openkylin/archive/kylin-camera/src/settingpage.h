#ifndef SETTINGPAGE_H
#define SETTINGPAGE_H

#include <QWidget>
//#include "setting.h"
//#include "titlebar.h"
#include <qsettings.h>
#include "settingpagetitle.h"
#include <QGSettings>
#include "defaultgetset.h"
#include <QComboBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "switchbutton.h"
#include <QCameraInfo>
#include "KylinCamera.h"
#include <QStandardItemModel>
#include <QKeyEvent>

struct DeviceName{
    QString fullName;
    QString shortName;
    QString dealName;
};

class SettingPage:public QWidget
{
        Q_OBJECT
public:
    KylinCamera *camera = nullptr;
    static QList<QCameraInfo> *cameras;
    SettingPage(QPoint *p):pos(p){}
    SettingPage(QWidget *parent = nullptr);
    ~SettingPage();

    QString nowThemeStyle;  //lhl debug
    QString currentDir;
    QString currentVideoDir;
    QString currentPhotoFormat;
    QString currentVideoFormat;
    QLabel *currentDirLab = nullptr;
    QLabel *currentVideoDirLab = nullptr;
    QLabel *currentCameraDev = nullptr;
    QLabel *current_videoDev = nullptr;
    QLabel *current_savePhotoFormat = nullptr; //lhl debug
    QLabel *current_savevideoFormat = nullptr;
    QLabel *current_scale = nullptr;
    QVBoxLayout *settingPageLayout = nullptr;
    QGridLayout *gridLayout = nullptr;

    static QSettings *resolution_setting;
    void initConnect();
    void Geometry(QPoint *);
Q_SIGNALS:
//    void dir_change();
//    void video_dir_change();
//    void change_dev(const char *,int scale_index);
//    void change_resolutions(QPair<uint,uint>);
//    void testSig();


public Q_SLOTS:
    void noDeviceSlot(int code);
    void dir_click();
    void video_dir_click();
    void change_item();
    void close();
    void confirm_click();
    void cancel_click();
    void updateShow();
    void isChangeDevice(bool);
    void updateResolution(int x);
    void settingOptionsOnSlot(bool on);
    protected:
    void changeEvent(QEvent *);
        void mouseMoveEvent(QMouseEvent *);
        void mousePressEvent(QMouseEvent *);
        void mouseReleaseEvent(QMouseEvent *);
        virtual void keyPressEvent(QKeyEvent *event);
    private:
        QLabel *titleLab = nullptr;
        QLabel *dirlab = nullptr;
        QLabel *videoDirlab = nullptr;
        QLabel *browselab = nullptr;
        QLabel *scalelab = nullptr;
        QLabel *cameraDevicelab = nullptr;
        QLabel *videoDevicelab = nullptr;
        QLabel *savePhotoFormatlab = nullptr;
        QLabel *savevideoFormatlab = nullptr;
        //功能按钮部分
        //QComboBox *dir;
        QLineEdit *dir = nullptr;
        QLineEdit *videoDir = nullptr;
        QPushButton *browse = nullptr;
        QPushButton *videoBrowse = nullptr;
        QComboBox *scale = nullptr;
        QStringList scalelist;
        QComboBox *cameraDevice = nullptr;
        QComboBox *videoDevice = nullptr;
        QComboBox *savePhotoFormat = nullptr;
        QComboBox *savevideoFormat = nullptr;
        SwitchButton *delaybtn = nullptr;
        SwitchButton *mirrorbtn = nullptr;
        QPushButton *confirm = nullptr;
        QPushButton *cancel = nullptr;

        QVBoxLayout *vlayout = nullptr;
        QHBoxLayout *btnLayout = nullptr;
        QHBoxLayout *dirLayout = nullptr;
        QHBoxLayout *titleLayout = nullptr;
        QHBoxLayout *scaleLayout = nullptr;
        QHBoxLayout *cameraDeviceLayout = nullptr;
        QHBoxLayout *videoDeviceLayout = nullptr;
        QHBoxLayout *savePhotoFormatLayout = nullptr;
        QHBoxLayout *savevideoFormatLayout = nullptr;

        QWidget *dirLayoutWidget = nullptr;
        QWidget *scaleLayoutWidget = nullptr;
        QWidget *cameraDeviceLayoutWidget = nullptr;
        QWidget *savePhotoFormatLayoutWidget = nullptr;
        QWidget *savevideoFormatLayoutWidget = nullptr;

        QPoint *pos = nullptr;
        QPoint m_start;
        QMap<QString,QString> camera_name2dev;
        QMap<QString,QString> audio_name2dev;

        QMap<QString,QString> fullNameArrSysName;
        QMap<QString,QString> sysNameArrEasyNamy;
        QString scale_str;
        QString first;
        SettingPageTitle *settingPageTitle = nullptr;  //lhl add
        QVBoxLayout *settingTitleLayout = new QVBoxLayout;
        static int dev_change;
        static bool press;
        static bool flagFirst;
        DefaultGetSet *gsettingItem = nullptr;
        bool changeEventFlag = false;
        void dealSystemGsettingChange(const QString);
        void elideCameraText();
        QString dirNameELideText(QString str);
        QStandardItemModel *model = nullptr;
        bool settingOnFlag = true;
    };

    #endif // SETTINGPAGE_H
