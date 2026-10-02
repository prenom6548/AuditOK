#include <QObject>
#include "test_mainwindow.h"
#include "custombutton.h"
#include "clipwidget.h"
#include "clipbutton.h"
#include "customslider.h"
#include "playcontrol.h"
#include "miniwindow.h"
#include "menumodule.h"
#include "recorderdatabase.h"
#include "renamewidget.h"
#include "settings.h"
#include "mywave.h"
#include "tools.h"
#include "mmediaplayer.h"
#include "fileitem.h"
//#include "horizontalorverticalmode.h"
#include "mmediaplaylist.h"
#include "lineedit.h"
#include "save.h"
#include <QApplication>
#include <QThread>
#include <QFileInfo>
#include "mylabel.h"
#include "mythread.h"
#include "log.h"
#include <limits>
#include <cfloat>

#include <iostream>

QString list1 = "./output.mp3";

TestMainWindow::TestMainWindow(QObject *parent)
    : QObject{parent}
{
    QGSettings* gset = new QGSettings("org.kylin-recorder-data.settings");
    gset->set("source", 2);
}
void TestMainWindow::test_construct()
{
    QStringList list, listNull;
    list << "./test.mp3";
    window = new MainWindow(list, nullptr);
    QVERIFY(window != nullptr);
    bool hasVideoPlayed = false;
    QTest::qWaitFor([&]() {
        return hasVideoPlayed;
    }, 5000);
//    QVERIFY(hasVideoPlayed);
std::cout << "-------------test_construct 1" << std::endl;
    window->currentGSettingChangeUI();
    std::cout << "-------------test_construct 2" << std::endl;
    window->receivepause();
    std::cout << "-------------test_construct 3" << std::endl;
//    window->findLeftSlot();
    std::cout << "-------------test_construct 4" << std::endl;
//    window->findRightSlot();
    std::cout << "-------------test_construct 5" << std::endl;
//    window->handleInsertSlot();
    std::cout << "-------------test_construct 6" << std::endl;
    window->handlingSlot();
    std::cout << "-------------test_construct 7" << std::endl;
    window->goset();
    std::cout << "-------------test_construct 8" << std::endl;
    window->updateDateFormat("aaa");
    std::cout << "-------------test_construct 9" << std::endl;
}
void TestMainWindow::test_record()
{
    QStringList list, listNull;
    list << "/home/kylin/gitee/video/20241030_231218_3.mp4";
//    MainWindow window(list, nullptr);
    QVERIFY(window != nullptr);
    QVERIFY(window->m_continueButton != nullptr);
    //测试主界面开始录音按钮
    QVERIFY(window->m_isRecording != true);
//    emit window->m_firstRecordButton->clicked();
    emit window->recordingSignal(1);
    emit window->startThread();
//    emit window->startRecord();
//    QVERIFY(window->m_isRecording != false);
    emit window->m_finishButton->clicked();
    window->m_finish = true;
    emit window->recordingSignal(0);
    emit window->stopRecord();
std::cout << "----------------test_record 1" << std::endl;;
    QVERIFY(window->m_finish != false);
    emit window->pageChange();
    //测试第二界面开始录音按钮
    QVERIFY(window->m_isRecording != true);
    emit window->m_thirdRecordButton->clicked();
    QVERIFY(window->m_isRecording != false);
    emit window->m_continueButton->clicked();
    qDebug() << "1111" << window->m_stratOrPause;
    QVERIFY(window->m_stratOrPause == true);
    std::cout << "----------------test_record 4" << std::endl;

    emit window->m_continueButton->clicked();
    std::cout << "----------------test_record 6" << std::endl;

    QVERIFY(window->m_stratOrPause != true);
    std::cout << "----------------test_record 7" << std::endl;

    emit window->m_pauseButton->clicked();
    std::cout << "----------------test_record 8" << std::endl;

//    emit window->m_flagButton->clicked();
    std::cout << "----------------test_record 9" << std::endl;

    emit window->m_miniButton->clicked();
    std::cout << "----------------test_record 5" << std::endl;

    QVERIFY(window->m_isMax == false);
//    emit window->m_maxButton->clicked();
    std::cout << "----------------test_record 51" << std::endl;

//    QVERIFY(window->m_isMax == true);
    emit window->m_minButton->clicked();
    std::cout << "----------------test_record 52" << std::endl;

    // emit window->m_closeButton->clicked();
    std::cout << "----------------test_record 3" << std::endl;

    emit window->closeSignal();
//    emit window->m_finishButton->clicked();
    emit window->themeColorchange();
    emit window->keySpaceSignal();
    std::cout << "----------------test_record 11" << std::endl;

//    emit window->keyMSignal();
//    emit window->keyCSignal();
//    emit window->keyOSignal();
    std::cout << "----------------test_record 10" << std::endl;

//    emit window->switchPageSignal();
    std::cout << "----------------test_record 101" << std::endl;

//    emit window->stop_clickedSignal();
    std::cout << "----------------test_record 102" << std::endl;

    QGSettings *m_themeData = nullptr;
    m_themeData = new QGSettings(UKUI_THEME_GSETTING_PATH);
    QFont font(m_themeData->get("systemFont").toString(), m_themeData->get("systemFontSize").toInt());
    emit window->itemFontStyleSignal(font);
    std::cout << "----------------test_record 103" << std::endl;
    emit window->themeColorchange();
    std::cout << "----------------test_record 104" << std::endl;
    emit window->m_themeData->changed("styleName");
//    emit window->leftButton->clicked();
//    emit window->rightButton->clicked();
    window->inputDevice_get("11");
    std::cout << "----------------test_record 20" << std::endl;

    window->outputDevice_get("11");
    std::cout << "----------------test_record 21" << std::endl;
    window->onPrepareForShutdown(1);
    std::cout << "----------------test_record 22" << std::endl;
    window->onPrepareForSleep(1);
    std::cout << "----------------test_record 23" << std::endl;
    window->onPrepareForSleep(0);
    std::cout << "----------------test_record 24" << std::endl;
    window->recordPaint(0);
    std::cout << "----------------test_record 25" << std::endl;
    window->updateDisplay();
    std::cout << "----------------test_record 26" << std::endl;
    window->getFilePathList();
    std::cout << "----------------test_record 27" << std::endl;
    window->clearFilePathList();
    std::cout << "----------------test_record 28" << std::endl;
    window->removePathInFilePathList("11");
    std::cout << "----------------test_record 29" << std::endl;
    window->changeVoicePicture();
    std::cout << "----------------test_record 221" << std::endl;
    window->sendFlagView();
    // std::cout << "----------------test_record 222" << std::endl;
    window->showNotify("11");
    std::cout << "----------------test_record 223" << std::endl;
    window->sourceInput(1);
    std::cout << "----------------test_record 224" << std::endl;
    window->sourceInput(2);
    std::cout << "----------------test_record 225" << std::endl;
    window->get_sys_runtime(1);
    std::cout << "----------------test_record 226" << std::endl;
    window->get_sys_runtime(2);
    std::cout << "----------------test_record 01" << std::endl;
    window->changeThemeColorSlot("styleName");
    window->changeThemeColorSlot("systemFont");
    window->changeThemeColorSlot("systemFontSize");
    window->changeThemeColorSlot("themeColor");
    window->changeControlTransSlot("transparency" );
    window->show();
    window->command_Control("-c");
    window->command_Control("");
    window->command_Control("-s");
    window->command_Control("-p");
//    window->command_Control("-f");
    window->themeWindow("ukui-dark");
    window->themeWindow("ukui-white");
    window->themeButton("ukui-dark");
    window->themeButton("ukui-white");
//     QMouseEvent *event1;
//     window->mouseDoubleClickEvent(event1);
//     window->mousePressEvent(event1);
//     window->mouseReleaseEvent(event1);
// //    QKeyEvent *event2;
// //    window->keyPressEvent(event2);
//     window->themeFontStyle("1",13);
     window->themeFontStyle("1",16);
    std::cout << "----------------test_record 100" << std::endl;

}
void TestMainWindow::test_fileitem()
{
    std::cout << "----------------test_fileitem 1" << std::endl;;


    QStringList list, listNull;
//    list << "/home/denghao/2.mp3";
//    MainWindow window(list, nullptr);
    QVERIFY(window != nullptr);

    QFileInfo info(list1);
    if (!info.exists()) {
        std::cout << "--*******************--" << std::endl;
    }

    FileItem fileitem(list1,nullptr);
    std::cout << "----------------test_fileitem 16" << std::endl;;

    QVERIFY(&fileitem != nullptr);
    std::cout << "----------------test_fileitem 15" << std::endl;;

    // emit fileitem.m_playButton->clicked();
    std::cout << "----------------test_fileitem 10" << std::endl;;

    emit fileitem.m_pauseButton->clicked();
    std::cout << "----------------test_fileitem 12" << std::endl;;

//    emit fileitem.m_clipButton->clicked();
    std::cout << "----------------test_fileitem 13" << std::endl;;

//    emit fileitem.m_deleteButton->clicked();
    std::cout << "----------------test_fileitem 14" << std::endl;;

    emit fileitem.m_flagButton->clicked();
    std::cout << "----------------test_fileitem 11" << std::endl;;

//    emit fileitem.m_playSlider->sliderReleasedAt(0);
//    emit fileitem.m_actionSave->triggered();
//    emit fileitem.m_actionOpenFolder->triggered();
//    emit fileitem.m_actionRename->triggered();
//    emit fileitem.m_actionEmptyList->triggered();
    emit fileitem.startdelete(list);
    QDate date(2024,12,17);
    QTime time(16,23);
    QDateTime addtime(date,time);
    emit fileitem.addListItemSignal(list1, addtime);
    emit fileitem.currentPos_PlaySignal(1);
    emit fileitem.updateName(list1);
    fileitem.updateDateFormat("addtime");
    fileitem.sendFlagView();
    fileitem.displayFlag("11");
    std::cout << "----------------test_fileitem 2" << std::endl;;

    // fileitem.addListDisplay("111");
    // fileitem.handleMove();
    // fileitem.handlePause();
//     fileitem.handlePlay();
//     fileitem.updateLabelNameSlots("saaaa");
    // fileitem.getfilePath();
    // fileitem.updateListDisplay("11");
    // fileitem.stateChanged(QMediaPlayer::StoppedState);
    //fileitem.cancel();
    //fileitem.finish();
    std::cout << "----------------test_fileitem 21" << std::endl;;

    fileitem.isOtherClipWidOpen();
    fileitem.themeStyle("ukui-dark");
    fileitem.themeStyle("ukui-white");
    fileitem.currentIndex()==1;
    fileitem.currentIndex()==0;
    fileitem.deleteFile("111");
//    fileitem.rightClickedMenuRequest();
    std::cout << "----------------test_fileitem 3" << std::endl;;

//    fileitem.actionSaveasSlot();
//    fileitem.actionOpenFolderSlot();
//    fileitem.actionRenameSlot();
//    fileitem.actionEmptyListSlot();
//    QMouseEvent *mouseEvent;
//    fileitem.mousePressEvent(mouseEvent);
    std::cout << "----------------test_fileitem 32" << std::endl;;

    QEvent *event2;
    fileitem.leaveEvent(event2);
    fileitem.enterEvent(event2);
    fileitem.processLog();
    QResizeEvent *event3;
    fileitem.resizeEvent(event3);
    std::cout << "----------------test_fileitem 31" << std::endl;;

    QGSettings *m_themeData = nullptr;
    m_themeData = new QGSettings(UKUI_THEME_GSETTING_PATH);
    QFont font(m_themeData->get("systemFont").toString(), m_themeData->get("systemFontSize").toInt());
    fileitem.itemFontStyle(font);
    //剪辑功能
    ClipWidget clipwidget(list1,nullptr);
    QVERIFY(&clipwidget != nullptr);
    CustomButton *custom = new CustomButton("1" , "1" , "1" , nullptr);
    custom->setNormalPixmap("11");
    custom->setEnterPixmap("11");
    std::cout << "----------------test_fileitem 4" << std::endl;;

    custom->setLeavePixmap("11");
    custom->setPressPixmap("11");
    custom->setReleasePixmap("11");
    custom->setButtonEnabled(0,0);
    custom->setButtonEnabled(1,0);
    custom->setButtonEnabled(1,1);
    custom->setButtonEnabled(0,1);
    custom->enterEvent(event2);
    custom->leaveEvent(event2);
//    custom->mousePressEvent(mouseEvent);
//    custom->mouseReleaseEvent(mouseEvent);
     emit clipwidget.clip_playBtn->clicked();
    emit clipwidget.clip_pauseBtn->clicked();
    emit clipwidget.cancelButton->clicked();
    std::cout << "----------------test_fileitem 5" << std::endl;;

//    emit clipwidget.finishButton->clicked();
    emit clipwidget.leftBtn->clicked();
    std::cout << "----------------test_fileitem 51" << std::endl;;

    emit clipwidget.rightBtn->clicked();
    std::cout << "----------------test_fileitem 52" << std::endl;;

    emit clipwidget.cancel_ClipSignal();
    std::cout << "----------------test_fileitem 53" << std::endl;;

    emit clipwidget.finish_ClipSignal();
    std::cout << "----------------test_fileitem 54" << std::endl;;

    emit clipwidget.coverUpdateListSignal(list1);
    std::cout << "----------------test_fileitem 55" << std::endl;;

    emit clipwidget.addListSignal(list1);
    std::cout << "----------------test_fileitem 56" << std::endl;;

//    emit clipwidget.wave_AnalysisStart();
    std::cout << "----------------test_fileitem 57" << std::endl;;

//    emit clipwidget.wave_AnalysisFinish(1);
    std::cout << "----------------test_fileitem 58" << std::endl;;

    emit clipwidget.coverUpdateListSignal(list1);
    std::cout << "----------------test_fileitem 59" << std::endl;;

    emit clipwidget.addListSignal(list1);
//    emit clipwidget.actionSave->triggered();
//    emit clipwidget.actionCover->triggered();
    std::cout << "----------------test_fileitem 6" << std::endl;;

//    clipwidget.display_Wave();
    clipwidget.wave_Analysis_Tips(true);
    clipwidget.toPaintWave(0,0);
    clipwidget.leftBtn_ReleaseStartPlayer_Slot(0);

    clipwidget.rightBtn->move(10, clipwidget.rightBtn->pos().y());
    clipwidget.leftBtn->move(0, clipwidget.leftBtn->pos().y());
    qInfo() << "right:" << clipwidget.rightBtn->pos().rx() << ", left:" << clipwidget.leftBtn->pos().rx();

    clipwidget.leftBtn_ReleaseStartPlayer_Slot(1);
    clipwidget.rightBtn_ReleaseGetEndPositon_Slot(0);
    clipwidget.rightBtn_ReleaseGetEndPositon_Slot(3);
    //clipwidget.generateClipRecord("11");
//    clipwidget.save_NewRecord();
//    clipwidget.cover_CurRecord();
    clipwidget.leftBtn_Move_Slot(0);
    clipwidget.rightBtn_Move_Slot(0);
    clipwidget.cursorMove();

    clipwidget.clipMovePos = 100;
    clipwidget.cursorMove();

    clipwidget.changeCursorColorSlots();
    clipwidget.themeStyle("ukui-dark");
    clipwidget.themeStyle("ukui-white");
    std::cout << "----------------test_fileitem 7" << std::endl;;

//     clipwidget.itemFontStyle(font);
//     //文件播放相关功能
//     emit fileitem.m_pauseButton->clicked();
//     emit fileitem.m_playButton->clicked();
// //    emit fileitem.m_clipButton->clicked();
//     emit fileitem.m_deleteButton->clicked();
//     emit fileitem.m_flagButton->clicked();
//     fileitem.m_playSlider->themeStyle("ukui-dark");
//    fileitem.m_playSlider->mousePressEvent(mouseEvent);
//    fileitem.m_playSlider->mouseReleaseEvent(mouseEvent);
    std::cout << "----------------test_fileitem 8" << std::endl;;


}
void TestMainWindow::test_mpv()
{
    QStringList list, listNull;
//    list << "/home/denghao/2.mp3";
//    MainWindow window(list, nullptr);
    QVERIFY(window != nullptr);
    playcon = new PlayControl;
    QVERIFY(&playcon != nullptr);
    emit playcon->getPlayer()->positionChanged(0);
    emit playcon->getPlayer()->stateChanged(QMediaPlayer::StoppedState);
}
void TestMainWindow::test_menu()
{
    QStringList list, listNull;
//    list << "/home/denghao/2.mp3";
//    MainWindow window(list, nullptr);
    QVERIFY(window != nullptr);
    QWidget patent;
    m_menuModule = new menuModule(&patent);
    QVERIFY(&m_menuModule != nullptr);
    emit m_menuModule->menuModuleClose();
    emit m_menuModule->menuModuleSetThemeStyle("Dark");
    emit m_menuModule->m_menuButton->clicked();
    QAction *action1 = new QAction("Quit" , this);
    m_menuModule->triggerMenu(action1);
    QAction *action2 = new QAction("About" , this);
    m_menuModule->triggerMenu(action2);
    QAction *action3 = new QAction("Help" , this);
    m_menuModule->triggerMenu(action3);
    m_menuModule->triggerThemeMenu(action3);
//    QAction *action4 = new QAction("Settings" , this);
//    m_menuModule->triggerMenu(action4);
//    m_menuModule->triggerThemeMenu(action4);
    QAction *action4 = new QAction("Light" , this);
    m_menuModule->triggerThemeMenu(action4);
    QAction *action5 = new QAction("Dark" , this);
    m_menuModule->triggerThemeMenu(action5);
//    m_menuModule->aboutAction();
    m_menuModule->helpAction();
    m_menuModule->dealSystemGsettingChange("styleName");
    m_menuModule->initAbout();
    m_menuModule->refreshThemeBySystemConf();
    m_menuModule->setThemeDark();
}
void TestMainWindow::test_mini()
{
    QStringList list, listNull;
//    list << "/home/denghao/2.mp3";
//    MainWindow window(list, nullptr);
    QVERIFY(window != nullptr);
    MiniWindow *miniwindow = new MiniWindow;
    QVERIFY(&miniwindow != nullptr);
//    emit miniwindow->m_miniRecordButton->clicked();
    emit miniwindow->m_miniButton->clicked();
//    emit miniwindow->m_miniCloseButton->clicked();
    emit miniwindow->m_miniPauseButton->clicked();
    emit miniwindow->m_miniStartButton->clicked();
//    emit miniwindow->m_miniStopButton->clicked();
    emit miniwindow->startOrPauseSignal();
    miniwindow->initShortcutStyle();
    miniwindow->timeDisplay();
    // QMouseEvent *event1;
    // miniwindow->mousePressEvent(event1);
    // miniwindow->mouseReleaseEvent(event1);
    // miniwindow->mouseMoveEvent(event1);
    // QKeyEvent *event2 = new QKeyEvent(QEvent::KeyPress , Qt::Key_F1 , Qt::NoModifier);
    // miniwindow->keyPressEvent(event2);
    // QKeyEvent *event3 = new QKeyEvent(QEvent::KeyPress , Qt::Key_M , Qt::NoModifier);
    // miniwindow->keyPressEvent(event3);
    // QKeyEvent *event4 = new QKeyEvent(QEvent::KeyPress , Qt::Key_R , Qt::ControlModifier);
    // miniwindow->keyPressEvent(event4);
    miniwindow->changeMiniCoutinuePauseBtnThemeColor("ukui-dark");
    miniwindow->changeMiniCoutinuePauseBtnThemeColor("ukui-white");
}

void TestMainWindow::test_database()
{
    QStringList list, listNull;
//    list << "/home/denghao/2.mp3";
//    MainWindow window(list, nullptr);
    QVERIFY(window != nullptr);
    RecorderDataBase *m_dataBase = new RecorderDataBase;
    QVERIFY(&m_dataBase != nullptr);
    QFileInfo file_info("11.mp3");
    QString flagTime = "16:11";
    QString flagname = "123";
    emit m_dataBase->insertDataSignal(file_info , flagTime);
std::cout << "---------------------test_database 1" << std::endl;
    emit m_dataBase->deleteDataSignal(flagname);
std::cout << "---------------------test_database 2" << std::endl;

    emit m_dataBase->searchDataSignal(file_info);
std::cout << "---------------------test_database 3" << std::endl;

    emit m_dataBase->addRecordSignal(flagname , flagTime);
std::cout << "---------------------test_database 4" << std::endl;

    emit m_dataBase->deleteRecordSignal(flagname);
std::cout << "---------------------test_database 5" << std::endl;

    m_dataBase->updateDatabaseSlot("11" , "mp3");

std::cout << "---------------------test_database 7" << std::endl;

}
void TestMainWindow::test_rename()
{
    QStringList list, listNull;
//    list << "/home/denghao/2.mp3";
//    MainWindow window(list, nullptr);
    QVERIFY(window != nullptr);
    RenameWidget *renamewid = new RenameWidget;
    QVERIFY(&renamewid != nullptr);
    QFileInfo file_info("11");
    QString flagTime = "16:11";
    QString flagname = "123";
    emit renamewid->m_cancelButton->clicked();
    emit renamewid->titleIcon->clicked();
    emit renamewid->m_titleButtonExit->clicked();
    emit renamewid->changeLabel("11");
    renamewid->checkRenameable(&file_info , flagname);
    renamewid->checkLegal(flagname);
    //renamewid->renameSlots();
    renamewid->countLengthSlots();
    renamewid->setLineEditContent("11");
}

void TestMainWindow::test_save()
{
   QStringList list, listNull;
//    list << "/home/denghao/2.mp3";
//    MainWindow window(list, nullptr);
//    QVERIFY(window != nullptr);
   std::cout << "**************************test_save 10" << std::endl;

   Settings *set = new Settings();
   QVERIFY(&set != nullptr);
   emit set->m_closeButton->clicked();
//   emit set->m_alterButton->clicked();
   set->editSlot();
std::cout << "**************************test_save 1" << std::endl;

   QKeyEvent *event;
   QKeyEvent *event2 = new QKeyEvent(QEvent::KeyPress , Qt::Key_F1 , Qt::NoModifier);
   set->keyPressEvent(event2);
   set->saveType(0);
   set->saveType(1);
   set->saveType(2);
   set->recordSource(0);
   set->recordSource(1);
   set->recordSource(2);
//   set->inputEditJudge("");
//   set->inputEditJudge("/home");
std::cout << "**************************test_save 2" << std::endl;
   Tools *tool = new Tools();
   QVERIFY(&tool != nullptr);
   tool->getRecordingFileinfos();
   tool->getRecordingSaveDirectory("/home/kylin");
   tool->formatMillisecond(0);
   tool->formatMillisecond(36000000);
//   tool->inputEditJudge("11");
//   tool->inputEditJudge("");
//   tool->inputEditJudge(";");
std::cout << "**************************test_save 3" << std::endl;
}

void TestMainWindow::test_label()
{
    QStringList list, listNull;
//    list << "/home/denghao/2.mp3";
//    MainWindow window(list, nullptr);
    QVERIFY(window != nullptr);
    MyLabel *label = new MyLabel;
    QVERIFY(&label != nullptr);
    label->setTextLimitShrink("11" , 1);
    label->setTextLimitExpand("11");
    label->fullText();
    label->elideText();
    label->dealMessage("11");
    label->dealMessage("111111111111111111");
    QPaintEvent *event;
    label->paintEvent(event);
    ScopeFrame *scop = new ScopeFrame;
    scop->paintEvent(event);
}
void TestMainWindow::test_all()
{
    QStringList list, listNull;
//    list << "/home/denghao/2.mp3";
//    MainWindow window(list, nullptr);
    QVERIFY(window != nullptr);
//    HorizontalOrVerticalMode horiz;
//    horiz.modeChanged(1);
//    horiz.modeChanged(0);
//    horiz.timeOutSlot();
//    horiz.rotationChanged("11");
//已覆盖代码
    MyThread *thread = new MyThread;
    thread->toConvertWAV("1","2");
    thread->toConvertMp3("1","2");
    thread->uploadStarted();
    thread->audioConversionFinish(1 , QProcess::NormalExit);
    thread->audioConversionFinish(0 , QProcess::NormalExit);
    thread->audioConversionFinish(1 , QProcess::CrashExit);
    thread->audioConversionFinish(0 , QProcess::CrashExit);
    thread->M4a();
    thread->Wav();
    thread->Mp3();
    thread->useVolumeSample(11);
    thread->getFileName();
    thread->getFilePath();
    thread->generateAudio();
    thread->record_pressed();

std::cout << "-----------------------------test_all 1" << std::endl;
    thread->stopBtnPressed();
    thread->setDefaultPath("11");
    thread->readPathCollected();
    std::cout << "-----------------------------test_all 11" << std::endl;

    thread->writePathCollected("11");
    thread->readNumList();
    thread->writeNumList(0);
    std::cout << "-----------------------------test_all 12" << std::endl;

    thread->onChangeCurrentRecordList("11");
    thread->pauseRecord();
    thread->playRecord();
    std::cout << "-----------------------------test_all 13" << std::endl;

    thread->OnSliderValueChanged(0);
    std::cout << "-----------------------------test_all 14" << std::endl;

    thread->fun();
    std::cout << "-----------------------------test_all 15" << std::endl;

    thread->OnReadMore();
std::cout << "-----------------------------test_all 2" << std::endl;

    thread->OpenFolderProcess("11");

//    thread->saveAs("11");
    AudioFileProcess *audiofile = new AudioFileProcess;
    audiofile->wavFile_Analy("/home/denghao/音乐/20241217143010.wav");
    myWave *wave = new myWave(1,nullptr);
    wave->setMaximum(1);
    wave->setMinimum(1);
    wave->setValue(5);
    wave->getValue();
}

void TestMainWindow::unit_log()
{
    std::cout << "-----------------------------------unit_log" << std::endl;

    // 测试不同的日志级别
    kdk::kabase::debug << "Debug message";
    kdk::kabase::info << "Info message";
    kdk::kabase::warning << "Warning message";
    kdk::kabase::error << "Error message";
    kdk::kabase::fatal << "Fatal message";

    // 测试不同类型的数据输出
    kdk::kabase::info << 42;
    kdk::kabase::info << 3.14f;
    kdk::kabase::info << 2.718281828;

    // 测试字符串输出
    kdk::kabase::info << "Hello World";

    // 测试std::string
    std::string stdStr = "Standard string";
    kdk::kabase::info << stdStr;

    // 测试QString
    QString qStr = "QString";
    kdk::kabase::info << qStr;

    // 测试指针输出
    kdk::kabase::info << (void*)nullptr;
    int value = 100;
    kdk::kabase::info << (void*)&value;

    // 测试布尔值输出
    kdk::kabase::info << true;
    kdk::kabase::info << false;

    // 测试边界条件和特殊情况
    kdk::kabase::info << INT_MAX;
    kdk::kabase::info << INT_MIN;
    kdk::kabase::info << FLT_MAX;
    kdk::kabase::info << FLT_MIN;
    kdk::kabase::info << DBL_MAX;
    kdk::kabase::info << DBL_MIN;
    kdk::kabase::info << std::numeric_limits<float>::infinity();
    kdk::kabase::info << std::numeric_limits<double>::quiet_NaN();

    // 测试空值
    kdk::kabase::info << "";
    std::string emptyStdStr = "";
    kdk::kabase::info << emptyStdStr;
    QString emptyQStr = "";
    kdk::kabase::info << emptyQStr;

    // 测试所有日志级别的各种数据类型
    // Debug级别
    kdk::kabase::debug << "Debug string";
    kdk::kabase::debug << 123;
    kdk::kabase::debug << 3.14f;
    kdk::kabase::debug << 2.718281828;
    kdk::kabase::debug << true;
    kdk::kabase::debug << (void*)nullptr;

    // Info级别
    kdk::kabase::info << "Info string";
    kdk::kabase::info << 456;
    kdk::kabase::info << 1.41f;
    kdk::kabase::info << 1.732050808;
    kdk::kabase::info << false;
    kdk::kabase::info << (void*)this;

    // Warning级别
    kdk::kabase::warning << "Warning string";
    kdk::kabase::warning << 789;
    kdk::kabase::warning << 2.71f;
    kdk::kabase::warning << 3.141592654;
    kdk::kabase::warning << true;
    int val = 999;
    kdk::kabase::warning << (void*)&val;

    // Error级别
    kdk::kabase::error << "Error string";
    kdk::kabase::error << 999;
    kdk::kabase::error << 9.99f;
    kdk::kabase::error << 9.999999999;
    kdk::kabase::error << false;
    kdk::kabase::error << (void*)nullptr;

    // Fatal级别
    kdk::kabase::fatal << "Fatal string";
    kdk::kabase::fatal << 111;
    kdk::kabase::fatal << 1.11f;
    kdk::kabase::fatal << 1.111111111;
    kdk::kabase::fatal << true;
    kdk::kabase::fatal << (void*)this;

    std::cout << "-----------------------------------unit_log over" << std::endl;
}

void TestMainWindow::test_tool()
{
//    Tools tool;
//    tool.inputEditJudge("/");
//    tool.inputEditJudge("/home/kylin/hgg/test");
//    tool.inputEditJudge("/home/kylin/hgg/test.*#? \'\\");

}
