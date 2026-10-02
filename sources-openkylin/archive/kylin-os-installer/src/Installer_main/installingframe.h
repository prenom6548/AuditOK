#ifndef INSTALLINGFRAME_H
#define INSTALLINGFRAME_H

#include <QObject>
#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>
#include "../PluginService/ui_unit/arrowtooltip.h"
#include "../PluginService/ui_unit/middleframemanager.h"
#include "./uilt/slidershow.h"
#include "./uilt/showprogressbar.h"
#include "finishedInstall.h"
#include "qtextbrowser.h"
#include "qtextstream.h"
#include "qfilesystemwatcher.h"
#include <myprobar.h>
#include <QStackedWidget>


namespace KInstaller {
class InstallingFrame : public QWidget
{
    Q_OBJECT
public:
    explicit InstallingFrame(QWidget *parent = nullptr);
    ~InstallingFrame();
    void initUI();
    void addStyleSheet();
    void translateStr();
    void initAllConnect();


signals:
    void signalFinished();

public slots:
    void updateProgressValue(int value);
    void findFile();
    void showError();
    void closeW();
    void showMsgDetail();
    void flashMsg();
    void flashMsgDetail();
    void showErrorFrame();
    void clickRestartBtn();

private:
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);
    QStackedWidget* widgets;
public:
    QLabel* littleTitle;
    myprobar *m_installProgressBar;
    QLabel* m_mainTitle;
    QPushButton* m_restartBtn;
    SlideShow* slide;
    ShowProgressBar* progressBarCtrl;

    QHBoxLayout* bnt_box;
    QPushButton *textDetail;
    QVBoxLayout* gridLayout;
    QLabel *spic;
    QLabel *stext;

    QTextBrowser *textMsg;
    QLabel *MsgContext;
    QFile *fi;
    QFile *fiDetail;
    QFileSystemWatcher *mywatcher;
    QFileSystemWatcher *mywatcherDetail;
    qint64 currline=0;
    qint64 currlineDetail=0;
    bool flag=1;
    bool is_faild=0;

};
}

#endif // INSTALLINGFRAME_H
