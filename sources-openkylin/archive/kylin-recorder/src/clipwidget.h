#ifndef CLIPWIDGET_H
#define CLIPWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QAudioProbe>
#include <QAudioFormat>
#include <QGraphicsDropShadowEffect>
#include <QColor>
#include <kborderlessbutton.h>

#include "custombutton.h"
#include "clipbutton.h"
#include "mywave.h"
#include "playcontrol.h"
#include "audiofileprocess.h"
#include "scopeframe.h"

#define CLIP_RECTANGLE_COUNT 100 //用于绘制剪裁界面的矩形条个数
class ClipWidget : public QWidget
{
    Q_OBJECT


    friend class TestMainWindow;
public:
    explicit ClipWidget(QString path, QWidget *parent = nullptr);
    ~ClipWidget();

    AudioFileProcess *m_Wavefile;
    int display_Wave();
    void displayBtn(kdk::KBorderlessButton *btn, QString btnTooltip);

private:
    int Arightx; //左按钮的右侧
    int Bleftx;  //右按钮的左侧

    int MaxValue = 0;
    int count = 0;
    int soundVolume = 65; //初始音量为70

    QProcess *process = nullptr; //删除时调用命令
    QString fileClipPath;

    QString oldFileName;
    QString newFileName;
    QString musicDir;
    QString m_itemThemeColor;

    QVector<myWave *> mywave;
    QHBoxLayout *waveLayout = nullptr; //波形图布局

    QLabel *dur_timelb = nullptr;          //音频时间节点(开始/结束)
    CustomButton *clip_playBtn = nullptr;  //剪裁的试听按钮
    CustomButton *clip_pauseBtn = nullptr; //剪裁的暂停按钮
//    QPushButton *cancelButton = nullptr;   //取消
//    QPushButton *finishButton = nullptr;   //完成
    kdk::KBorderlessButton *cancelButton = nullptr;   //取消
    kdk::KBorderlessButton *finishButton = nullptr;   //完成

    QString m_clipPath;                    //录音存储的文件路径
    ClipButton *leftBtn = nullptr;       //左箭头按钮
    ClipButton *rightBtn = nullptr;      //右箭头按钮
    QFrame *cursor = nullptr;            //游标
    QWidget *railWid = nullptr;          //轨道的Wid
    QHBoxLayout *railWidLayout = nullptr;

    QWidget *bottomWid = nullptr;        //底部Wid
    ScopeFrame *scopeFrame = nullptr;    //选中的范围框
    QHBoxLayout *bottomLayout = nullptr; //底部布局

    QFrame *m_splitLine = nullptr;
    QVBoxLayout *m_lineClipLayout = nullptr;//剪裁界面上面分割线
    QVBoxLayout *clipLayout = nullptr; //剪裁界面

    QMenu *menu;
    QAction *actionSave;  //另存为
    QAction *actionCover; //打开此音频所在文件夹


    QString m_timeEditStartTime;
    QString m_timeEditEndTime;
    int clipMovePos = 0; //每隔毫秒数所移动位置
    qint64 m_currentStartPos = 0;
    qint64 m_currentEndPos = 0;
    qint64 m_audioDuration = 0;

    int scopeWidth = 0;

    void initClipWid(); //初始化ClipWid
    void setClipWid();  //设置ClipWid的界面
    int getInterval_Time(QString filePath);

    void resizeEvent(QResizeEvent *event);
    bool generateClipRecord(QString &outputPath);
    void initThemeGsetting();
    void themeStyle(QString themeColor);
signals:

    void cancel_ClipSignal();
    void finish_ClipSignal();

    void wave_AnalysisStart();
    void wave_AnalysisFinish(bool isOk);

    void computeLeftBtnL_To_RightBtnRWidth(int leftBtnPosX, int rightBtnRPosX, int railWidth,
                                           QVector<myWave *> mywave); //发送左按钮左边界到右按钮右边界宽度计算的信号

    void coverUpdateListSignal(QString currentFilePath);
    void addListSignal(QString currentFilePath);

public slots:

    void leftBtn_ReleaseStartPlayer_Slot(int leftButton_absolutePos); //左按钮或右按钮,统一为按钮边界相对位置
    void rightBtn_ReleaseGetEndPositon_Slot(int rightButton_absolutePos);
    void leftBtn_Move_Slot(int pos_X);
    void rightBtn_Move_Slot(int pos_X);

    void cursorMove();

    void cancel_ClipSlot();
    void finish_ClipSlot();

    void play_ClipFile();
    void pause_ClipFile();

    void wave_Analysis_Tips(bool isOk);
    void positionChange(qint64 pos);

    void processFinish(int);
    void toPaintWave(int index, int value);

    void save_NewRecord();
    void cover_CurRecord();
    void itemFontStyle(QFont fontStyle);
    void changeCursorColorSlots();
};

#endif // CLIPWIDGET_H
