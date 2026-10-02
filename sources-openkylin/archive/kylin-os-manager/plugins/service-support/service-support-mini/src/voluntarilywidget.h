#ifndef VOLUNTARILY_WIDGET_H
#define VOLUNTARILY_WIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QGSettings>
#include <QListWidget>
#include <QCheckBox>
#include <QProcess>
#include <QScrollArea>
#include <QFileSystemWatcher>
#include <kballontip.h>
#include <kwidget.h>
#include <kpressbutton.h>
#include "feedbackcommon.h"
#include "kom-scorewidget.h"

class UploadFileItem : public QWidget
{
    Q_OBJECT
signals:
    void removeUploadFile(const QString &);

public:
    UploadFileItem(const QString &filename);
    QListWidgetItem *getWidgetItem();

protected:
    bool event(QEvent *event);

private:
    void getType();
    void textUpdate();
    enum FileType { Unknow = 0, Image, Video, Package } m_fileType = Unknow;
    QString m_filename;
    QLabel *m_item = nullptr;
    QPushButton *m_closeBtn = nullptr;
    QLabel *m_icon = nullptr;
    QLabel *m_name = nullptr;
    QListWidgetItem *m_widgetItem = nullptr;
};

class VoluntarilyWidget: public kdk::KWidget, public FeedBackCommon {
    Q_OBJECT
public:
    VoluntarilyWidget( QWidget *parent = nullptr);
    void run();

private slots:
    void onScreenRecordFinish(int exit);
    void creatFinish(FeedBackFinishType type, QString str);

private:
    void initUI();
    void onButtonClick();
    void errorMessage(const QString &msg);
    void onScreenCutClicked();
    void getShotImage();
    void addUploadFile(const QString &filename);
    void addUploadFileItem(const QString &filename);
    void onAddFileClicked();
    void onscreenRecordClicked();
    void videoFileChanged(const QString &filename);
    QString getVideoSavePath();
    void showMessageBox(const QString &msg, kdk::TipType type);
    void removeUploadFile(const QString &fileName);
    void addLogFile();
    static double GetSystemScale();

    kdk::KPressButton *m_cardBase;
    QTextEdit *m_detailsEdit = nullptr;
    QLabel *m_detailsEditNumber = nullptr;
    QString m_tmpDetailsEditText;
    QGSettings *m_screenShotSetting = nullptr;
    QFileSystemWatcher m_fileWatcher;
    QStringList m_historyVideoFileList;
    bool isWatcher = false;
    bool m_startShot = false;
    QImage m_lastMimeImage;
    int m_addFileMax = 10;
    QString m_videoConfigPath;
    QProcess *m_screenRecord = nullptr;
    QPushButton *m_screenCut = nullptr;
    QPushButton *m_video = nullptr;
    QPushButton *m_addFile = nullptr;
    QPushButton *m_submit = nullptr;
    QListWidget *m_fileListWidget = nullptr;
    QStringList m_uploadFileNameList;
    QLineEdit *m_phoneNumberEdit = nullptr;
    QLabel *m_phoneNumberErrMsg = nullptr;
    QCheckBox *m_agreeAll = nullptr;
    QLabel *system_information_label_;
    kom::ScoreWidget *m_scoreWidget = nullptr;
    QScrollArea *m_baseScroll = nullptr;

};

#endif // VOLUNTARILY_WIDGET_H
