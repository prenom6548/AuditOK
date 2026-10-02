#ifndef FILEITEM_H
#define FILEITEM_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QSlider>
#include <QFrame>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QDesktopServices>
#include <QTextCodec>
#include <QDebug>
#include <QStackedWidget>
#include <QProcess>
#include <QPainter>
#include <QColor>
#include <QToolButton>
#include <QFileSystemWatcher>
#include <QDateTime>

#include "customslider.h"
#include "playcontrol.h"
#include "custombutton.h"
#include "mmediaplayer.h"
#include "mmediaplaylist.h"
#include "mylabel.h"
#include "renamewidget.h"

//class FileItem : public QWidget
class FileItem : public QStackedWidget
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.FileManager1") //调用DBus一定要加这一行

    friend class TestMainWindow;
public:
    explicit FileItem(QString path, QWidget *parent = nullptr);
    ~FileItem();

public:
    int m_movePos = 0; //每隔毫秒数所移动位置
//    void setName_Time_Length(QString nameStr, QString timeStr, QString lengthStr);
    void setName_Time_Length(QString nameStr, QDateTime recordTime, QString lengthStr);
    QString getfilePath();
    static FileItem* s_currentplaying;
    static FileItem* currentPlayingItem() {return s_currentplaying;}
    bool m_mouseState = false; // 用来判断当前鼠标是否在控件中
    bool m_playRecorder = false; //当前是否在播放
    bool m_pauseState = false;  // 用来判断当前是否是暂停状态
    QLabel *m_startRecordTimeLabel = nullptr; //录音时间标签
    MyLabel *m_recordFileNameLabel = nullptr; //录音文件名

private:
    QString m_filePath; //录音存储的文件路径
    QPalette m_pal;
    QProcess *m_processDEL = nullptr;         //删除时调用命令
    QFrame *m_splitLine = nullptr;            //分割线
    CustomSlider *m_playSlider = nullptr;     //播放划块条
    QLabel *m_timelengthLabel = nullptr;      //时间长度标签
    QLabel *m_seatLabel = nullptr;            //占位标签
    QString m_itemThemeColor;                 //主题颜色串
    static QString m_lastSavePath;

    QPushButton *m_pauseButton = nullptr;  //暂停按钮
    QPushButton *m_playButton = nullptr;   //播放按钮
    QPushButton *m_clipButton = nullptr;   //剪裁按钮
    QPushButton *m_deleteButton = nullptr; //删除按钮
    QPushButton *m_flagButton = nullptr;   //标记按钮

    QWidget *m_itemBottomWidget = nullptr;       //列表项的底部Wid，有label和button两组控件
    QWidget *m_fileNameDateTimeWidget = nullptr; //文件名和日期时间的Wid
    QWidget *m_splitLineWidget = nullptr;        //分割线wid

    QHBoxLayout *m_itemBottomLayout = nullptr;       //列表项的底部布局有label和button两组控件
    QVBoxLayout *m_fileNameDateTimeLayout = nullptr; //文件名和录制时间布局
    QVBoxLayout *m_timeLengthLayout = nullptr;       //列表项时间长度布局
    QVBoxLayout *m_itemLayout = nullptr;             // item总布局
    QVBoxLayout *m_splitLineLayout = nullptr;        //分割线layout

    QStackedWidget *m_splitLinestackWidget = nullptr; //分割线堆叠wid
    QStackedWidget *m_buttonStackWidget = nullptr;    //有按钮的堆叠Wid
    //QStackedWidget *m_itemStackWidget = nullptr; //整个item界面的堆叠布局(包括播放、剪裁两个界面)

    QWidget *m_itemWidget = nullptr; // itemWid
    QWidget *m_firstWidget = nullptr;
    QWidget *m_threeButtonWidget = nullptr;     //三个按钮的Wid
    QHBoxLayout *m_threeButtonLayout = nullptr; //列表项的3个按钮布局

    QMenu *m_menu = nullptr;
    QAction *m_actionSave = nullptr;       //另存为
    QAction *m_actionOpenFolder = nullptr; //打开此音频所在文件夹
    QAction *m_actionRename = nullptr;     //重命名
    QAction *m_actionEmptyList = nullptr;

    RenameWidget *m_renameWidget = nullptr;

    QFileSystemWatcher *m_fileWather = nullptr;

    QDateTime m_recordTime;

    bool isCliped = false;

    void initItemWid();       //初始化ItemWid
    void setItemWid();        //设置ItemWid的界面
    void initThemeGsetting(); //配置文件初始化主题
    void themeStyle(QString themeColor);
    void isOtherClipWidOpen(); //是否存在其他剪辑界面，若存在则此方法会将其他收回
    int getInternalMoveTime(); //获取间隔时间

    void processLog(); //命令日志

    void displayFlag(const QString &fileName);
    int textLabelHeight() const;
    int listItemHeight() const;
    void updateListItemHeight();

    void deleteFile(QString savepath);
    void _processStart(QString cmd, QStringList arguments);
    void sendFlagView();

    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    bool eventFilter(QObject *obj, QEvent *event); //事件过滤器
    void resizeEvent(QResizeEvent *event);

    double m_lastPos = 0;
    double m_nowPos = 0;
    double m_internalTime = 0;

    QMediaPlayer::State lastState {QMediaPlayer::StoppedState};
    bool m_nowPosKeep {false};  // 暂停，播放后，由于时间戳存在差异，导致进度条会先回退一下

public slots:
    void handlePlay();                      //控制播放
    void handlePause();                     //控制暂停
    void setSliderPos(qint64 pos);          //设置进度条位置
    void handleMove();                      //控制滑块移动
    void itemFontStyle(QFont fontStyle);    // item字体样式
    void stateChanged(QMediaPlayer::State); //状态的变化
    void currentPos_Play();                 //从当前标记位置播放
    void toFlagFile();
    void actionOpenFolderSlot();    //打开文件所在位置
    void toClipPage(); //切换至剪裁窗口


private slots:
    void rightClickedMenuRequest(); //右击弹出Menu菜单选择另存为和打开文件位置
    void actionSaveasSlot();        //另存为r
    void actionRenameSlot();
    void actionEmptyListSlot();

    void cancel();     //取消剪辑
    void finish();     //剪辑完成
    void updateListDisplay(QString filePath);
    void addListDisplay(QString filePath);
    void updateLabelNameSlots(QString newfilepath);
    void delAudioFile();
    void monitorFileChange();

    void updateDateFormat(QString dateFormat);

signals:
    void startdelete(QStringList path);
//    void addListItemSignal(QString addfilePath, QString addTime);
    void addListItemSignal(QString addfilePath, QDateTime addTime);
    void currentPos_PlaySignal(int);
    void updateName(QString name);
};

#endif // FILEITEM_H
