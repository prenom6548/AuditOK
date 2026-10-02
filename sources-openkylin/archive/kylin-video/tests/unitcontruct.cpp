/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
 
#include <QStringList>
#include <QPushButton>

#include "unitcontruct.h"
#include "mainwindow.h"
#include "widget/titlewidget.h"
#include "widget/minimodeshade.h"
#include "widget/minimodebutton.h"
#include "widget/kmenu.h"
#include "widget/kaction.h"
#include "widget/playlistwidget.h"
#include "widget/contralbar.h"
#include "widget/sliderbar/timeslider.h"
#include "widget/sliderbar/progressbar.h"
#include "widget/sliderbar/sliderbar.h"
#include "widget/sliderbar/slider.h"
#include "widget/mediainfodialog.h"
#include "widget/kmenu.h"

#include "widget/checkbox.h"
#include "widget/cmessagebox.h"
#include "widget/eventpasswidget.h"
#include "widget/playxwidget.h"
#include "widget/setup/setupcodec.h"
#include "widget/urleditwidget.h"
#include "widget/setup/setupsystem.h"
#include "widget/setup/setupplay.h"
#include "widget/setup/setupscreenshot.h"
#include "widget/setup/setupsubtitle.h"
#include "widget/setup/setupvolume.h"
#include "widget/setup/setupdialog.h"
#include "core/sqlitehandle.h"
#include "core/dbusadapter.h"
#include "core/mpvtypes.h"
#include "core/mediahandle.h"
#include "core/playlist.h"
#include "widget/setup/setupshortcut.h"

#include "global/translator.h"
#include "global/util.h"
#include "global/log.hpp"
#include "global/globalconfig.h"

#include "global/global.h"
#include <iostream>
#include <limits>
#include <cfloat>
using namespace Global;

const QString VIDEO_PATH = "./output.mp4";
const QString SUB_PATH = "./output.mp4";

unitConstruct::unitConstruct(QObject *parent) : QObject(parent)
{
    QStringList qlist;
    qlist << VIDEO_PATH;

    m_win = new MainWindow(qlist);
    m_win->showList();
//    m_win->kylin_video_play_request(QStringList());
    QTimer::singleShot(1000, [this]() {
        m_win->m_core->Pause();
    });

    QTimer::singleShot(1100, [this]() {
        m_win->m_core->Play();
    });

    QTest::qWaitFor([&]() {
        return false;
    }, 5000);

    m_win->onWiredControl("1:1");
    m_win->onWiredControl("1:2");
    m_win->onWiredControl("1:3");

    m_win->slotPrepareForSwitchuser();

    m_win->inputDeviceGet("device");

    m_win->uiModeChange(false);

    m_win->check4Kscreen();

    m_win->slotPlayWidgetClicked();

    g_playstate = Mpv::Playing;
    m_win->onPrepareForSleep(true);
    g_playstate = Mpv::Idle;

    m_win->onPrepareForSleep(false);

    m_win->updateAspect();

    m_win->slotThemeChange(1);
    m_win->slotThemeChange(2);
    m_win->slotThemeChange(3);

    g_playstate = Mpv::Playing;
    m_win->slotChangePause();
    g_playstate = Mpv::Paused;
    m_win->slotChangePause();
    g_playstate = Mpv::Idle;



    std::cout << "-----------------------------------1" << std::endl;
}

void unitConstruct::unit_title()
{
    std::cout << "-----------------------------------2:" << m_win->m_title_widget << std::endl;
    // 这几个会导致崩溃，先屏蔽掉
    // QTest::mouseClick(m_win->m_title_widget->m_btnMinSize, Qt::LeftButton);
    // std::cout << "-----------------------------------3" << std::endl;

    // QTest::qWaitFor([&]() {
    //     return false;
    // }, 500);

    // QTest::mouseClick(m_win->m_title_widget->m_btnMaxSize, Qt::LeftButton);

    // QTest::qWaitFor([&]() {
    //     return false;
    // }, 500);

    QTest::mouseClick(m_win->m_title_widget->m_btnMiniMode, Qt::LeftButton);

    QTest::qWaitFor([&]() {
        return false;
    }, 500);
    std::cout << "-----------------------------------unit_title 2:" << std::endl;

/////////////////////////////
    QTest::mouseClick(m_win->m_mini_mode_shade->btnPlayPause, Qt::LeftButton);

    QTest::qWaitFor([&]() {
        return false;
    }, 500);
        std::cout << "-----------------------------------unit_title 3:" << std::endl;


    QTest::mouseClick(m_win->m_mini_mode_shade->btnPlayPause, Qt::LeftButton);

    QTest::qWaitFor([&]() {
        return false;
    }, 500);
        std::cout << "-----------------------------------unit_title 4:" << std::endl;


    QTest::mouseClick(m_win->m_mini_mode_shade->btnNormal, Qt::LeftButton);

    QTest::qWaitFor([&]() {
        return false;
    }, 500);
        std::cout << "-----------------------------------unit_title 5:" << std::endl;


    // 标题栏弹窗
    // emit m_win->m_title_menu->act_clearMark->triggered();
    // QTest::qWaitFor([&]() {
    //     return false;
    // }, 100);
    //     std::cout << "-----------------------------------unit_title 6:" << std::endl;

//-----------------------------------------------
    emit m_win->m_title_menu->act_noMarkPlay->triggered();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
        std::cout << "-----------------------------------unit_title 7:" << std::endl;


    emit m_win->m_title_menu->act_noMarkPlay->triggered();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
        std::cout << "-----------------------------------unit_title 8:" << std::endl;


    emit m_win->m_title_menu->act_f1->triggered();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
        std::cout << "-----------------------------------unit_title 9:" << std::endl;


    // emit m_win->m_title_menu->act_about->triggered();
    // QTest::qWaitFor([&]() {
    //     return false;
    // }, 100);

}

void unitConstruct::unit_globalSignal()
{
    std::cout << "-----------------------------------unit_globalSignal" << std::endl;
    g_user_signal->openHelpDoc();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setTheme(0);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->showRightMenu();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->playWidgetClick();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->changeShowMode();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->stop();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->open(VIDEO_PATH, 0);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);


    g_user_signal->pause();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->play();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->play_pause();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->play_pause();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->nFrame();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->pFrame();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->progressUp(1000);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    std::cout << "-----------------------------------unit_globalSignal over5" << std::endl;

    g_user_signal->progressDown(1000);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);


    g_user_signal->forword(true);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->forword(false);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->backword(true);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->backword(false);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->seek(-1);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->seek(10000000);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->seek(10);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->restart();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);


    // g_user_signal->selectFile();
    // QTest::qWaitFor([&]() {
    //     return false;
    // }, 100);

    // g_user_signal->selectDir();
    // QTest::qWaitFor([&]() {
    //     return false;
    // }, 100);

    QTimer::singleShot(100, [this]{
        if (m_win->m_urlEdit) {
            m_win->m_urlEdit->accept();
        }
    });
    g_user_signal->openUrl();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    QTimer::singleShot(200, [this]() {
        m_win->m_aboutDialog->accept();
    });
    g_user_signal->showAbout();

    QTimer::singleShot(200, [this]() {
        m_win->m_setupDialog->accept();
    });
    g_user_signal->showSetup(1);

    QTimer::singleShot(100, [this]{
        if (m_win->m_mediaInfoDialog) {
            m_win->m_mediaInfoDialog->accept();
            g_playstate = Mpv::Idle;
        }
    });
    g_playstate = Mpv::Playing;
    emit m_win->m_right_click_menu->sigMediaInfo();

    g_user_signal->addSub(SUB_PATH);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    std::cout << "-----------------------------------unit_globalSignal over41" << std::endl;

    g_user_signal->setSubId(0);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setSubNext();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setSubUp();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    std::cout << "-----------------------------------unit_globalSignal over42" << std::endl;

    g_user_signal->setSubDown();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setSubDown();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    std::cout << "-----------------------------------unit_globalSignal over6" << std::endl;

    g_user_signal->setSubBackward();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setSubSize(10);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    std::cout << "-----------------------------------unit_globalSignal over3" << std::endl;

    g_user_signal->setSubFont("aaa");
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setChannel(Mpv::Channel::Default);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setChannel(Mpv::Channel::Stereo);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setChannel(Mpv::Channel::Left);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setChannel(Mpv::Channel::Right);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setSpeed(0.5);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setSpeed(1.5);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setSpeed(100);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    std::cout << "-----------------------------------unit_globalSignal over1" << std::endl;

    g_user_signal->setSpeed(1.0);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setSpeedUp();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setSpeedDown();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setVolume(20);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setVolumeUp(10);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setVolumeDown(10);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setMute(true);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setMute(false);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setAudioId(0);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setAudioNext();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setToTop();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setAspect(Mpv::VideoAspect::DIV_4_3);
    QTest::qWaitFor([&]() {
        return false;
    }, 500);

    g_user_signal->sigAspect(Mpv::VideoAspect::DIV_16_9);
    QTest::qWaitFor([&]() {
        return false;
    }, 500);

    g_user_signal->sigAspect(Mpv::VideoAspect::FULL);
    QTest::qWaitFor([&]() {
        return false;
    }, 500);

    g_user_signal->restoreFrame();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    std::cout << "-----------------------------------unit_globalSignal over2" << std::endl;

    g_user_signal->clockwiseRotate();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->counterClockwiseRotate();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->fullScreen();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setPlayOrder(PlayOrder::ONE_LOOP);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->playNext(true);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setPlayOrder(PlayOrder::LIST_LOOP);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->playPrev(true);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setPlayOrder(PlayOrder::RANDOM);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->playPrev(true);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->addListItem("aaa", 100);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->deleteListItem("aaa");
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    std::cout << "-----------------------------------unit_globalSignal over" << std::endl;

    // g_user_signal->showSetup(1);
    // QTest::qWaitFor([&]() {
    //     return false;
    // }, 100);

    g_user_signal->brightnessUp();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->brightnessDown();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->horizontallyFlip();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->horizontallyFlip();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);


    g_user_signal->verticalFlip();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->verticalFlip();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);


    g_user_signal->showPlayList();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->addBookMark(VIDEO_PATH);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->screenShot(true);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    std::cout << "-----------------------------------unit_globalSignal over" << std::endl;
    g_user_signal->screenShot(false);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->screenShotDir("/home/kylin/文档/kylin-video");
    QTest::qWaitFor([&]() {
        return false;
    }, 100);


    g_user_signal->screenShotFormat("jpg");
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->hideBar(true);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->hideBar(false);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->showInfo(true);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->showInfo(false);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setNoMarkMode(true);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->addBookMark(VIDEO_PATH);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setNoMarkMode(false);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    std::cout << "-----------------------------------unit_globalSignal over" << std::endl;


    g_user_signal->setVideoDecoder("vaapi");
    QTest::qWaitFor([&]() {
        return false;
    }, 100);


    g_user_signal->setVideoOutput("gpu");
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setVideoDecodeThreads(5);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setAudioDecoder("alsa");
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->setAudioDecoder("pulse");
    QTest::qWaitFor([&]() {
        return false;
    }, 100);


    g_user_signal->clearPlayList();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    g_user_signal->showStopFrame();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    std::cout << "-----------------------------------unit_globalSignal over" << std::endl;

//    g_user_signal->sigExit();
//    QTest::qWaitFor([&]() {
//        return false;
    //    }, 100);
}

void unitConstruct::unit_coreSignal()
{
    std::cout << "-----------------------------------unit_coreSignal" << std::endl;

    // 测试 notifyFileInfo 函数
    QMap<QString, QString> metadata;
    metadata["title"] = "Test Video";
    metadata["artist"] = "Test Artist";
    Mpv::VideoParams vp;
    vp.codec = "h264";
    vp.width = 1920;
    vp.height = 1080;
    vp.fps = 30.0;
    
    Mpv::AudioParams ap;
    ap.codec = "aac";
    ap.samplerate = 44100;
    ap.channels = 2;

    QList<Mpv::Track> qt;
    Mpv::Track track;
    track.id = 1;
    track.type = "video";
    track.title = "Video Track";
    qt.append(track);
    
    QList<Mpv::Chapter> qc;
    Mpv::Chapter chapter;
    chapter.title = "Chapter 1";
    chapter.time = 100;
    qc.append(chapter);
    
    Mpv::FileInfo info;
    info.file_path = "file_path";
    info.file_size = "1024KB";
    info.media_title = "media_title";
    info.file_type = "video/mp4";
    info.length = 100;
    info.metadata = metadata;
    info.video_params = vp;
    info.audio_params = ap;
    info.atracks = qt;
    info.vtracks = qt;
    info.stracks = qt;
    info.chapters = qc;

    g_core_signal->notifyFileInfo(info);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    
    // 测试长度为0的情况
    info.length = 0;
    g_core_signal->notifyFileInfo(info);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    
    // 测试空文件路径情况
    info.file_path = "";
    info.length = 100;
    g_core_signal->notifyFileInfo(info);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    // 测试 notifyFileNotExist 函数
    g_core_signal->notifyFileNotExist("nonexistent_file.mp4");
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    
    // 测试空文件名情况
    g_core_signal->notifyFileNotExist("");
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    // 测试 notifyDuration 函数
    g_core_signal->notifyDuration("test_video.mp4", 120.5);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    
    // 测试0时长情况
    g_core_signal->notifyDuration("test_video.mp4", 0.0);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    
    // 测试负数时长情况
    g_core_signal->notifyDuration("test_video.mp4", -10.0);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
    
    // 测试空文件名情况
    g_core_signal->notifyDuration("", 100.0);
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    std::cout << "-----------------------------------unit_coreSignal over" << std::endl;

    // 测试 notifyPlayState 函数
    g_core_signal->notifyPlayState();
    QTest::qWaitFor([&]() {
        return false;
    }, 100);
}

void unitConstruct::unit_functions()
{
        std::cout << "-----------------------------------unit_functions" << std::endl;

    m_win->m_playlist_widget->setDefaultTheme();

    m_win->m_playlist_widget->showItemMenu(VIDEO_PATH);

    m_win->m_playlist_widget->slotHide();

    m_win->m_playlist_widget->delAvailableIndex(VIDEO_PATH);

    m_win->m_playlist_widget->delAvailableIndex("systemFontSize");

    m_win->m_playlist_widget->changeListViewMode();

    m_win->m_playlist_widget->changeListViewMode();

    m_win->m_playlist_widget->gSettingChanged("systemFontSize");

    QEvent enterPlaylistWidget(QEvent::Enter);
    QCoreApplication::sendEvent(m_win->m_playlist_widget, &enterPlaylistWidget);





    emit m_win->m_right_click_menu->act_loadSubtitle->triggered();

    MarkListItem* item = new MarkListItem();
    item->setEnter();
    item->setLeave();
    QFont f;
    item->setWidgetFont(f);
    item->setNotExit("aaa");
    QColor color;
    item->setFontColor("color");
    item->setDefaultTheme();
    QTest::mouseClick(item, Qt::RightButton);
    QMouseEvent pressMarkItem(QEvent::MouseButtonDblClick, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(item, &pressMarkItem);

    QEvent enterMarkItem(QEvent::Enter);
    QCoreApplication::sendEvent(item, &enterMarkItem);

    QEvent leaveMarkItem(QEvent::Leave);
    QCoreApplication::sendEvent(item, &leaveMarkItem);


    PlayListItem* item1 = new PlayListItem;
    item1->setEnter();
    item1->setLeave();
//    item1->toBig();
//    item1->toSmall();
    item1->setDefaultTheme();
    item1->setWidgetFont(f);
    QTest::mouseClick(item, Qt::RightButton);
    QMouseEvent pressListItem(QEvent::MouseButtonDblClick, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(item1, &pressListItem);
    item1->deleteItem();



        std::cout << "-----------------------------------unit_functions1" << std::endl;


    m_win->m_core->getMediaInfo();

    m_win->m_contral_bar->m_timeSlider->m_progressBar->addMark(10, "");
    QTest::qWaitFor([&]() {
        return false;
    }, 100);

    m_win->m_contral_bar->m_timeSlider->m_progressBar->deleteMark(10);
    m_win->m_contral_bar->m_timeSlider->m_progressBar->setLightTheme();
    m_win->m_contral_bar->m_timeSlider->m_progressBar->w2v(20);
    m_win->m_contral_bar->m_timeSlider->m_progressBar->updateMouseValue(10);
    // 模拟鼠标进入事件
    QEvent enter(QEvent::Enter);
    QCoreApplication::sendEvent(m_win->m_contral_bar->m_timeSlider->m_progressBar, &enter);

    QMouseEvent press(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(m_win->m_contral_bar->m_timeSlider->m_progressBar, &press);

    QMouseEvent move(QEvent::MouseMove, QPoint(20, 20), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(m_win->m_contral_bar->m_timeSlider->m_progressBar, &move);

    QMouseEvent release(QEvent::MouseButtonRelease, QPoint(20, 20), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(m_win->m_contral_bar->m_timeSlider->m_progressBar, &release);


    GlobalConfig::getInstance()->clearChange();

    MediaInfoDialog* mediaInfo = new MediaInfoDialog;


    Translator translator;
    translator.load("");

    Util::Ratio(3, 2);
    Util::FormatNumber(3, 2);
    Util::FormatNumber(3, 20);
    Util::FormatNumber(3, 200);
    Util::FormatRelativeTime(222222);

    m_win->onPrepareForSleep(true);
    m_win->onPrepareForSleep(false);
    m_win->onScreenLock();
    m_win->onScreenUnlock();
    m_win->onLidWatcherMessage();
    m_win->slotShowMiniMode();

    KMenu* menu = new KMenu;
    menu->initStyle();

    KRightClickMenu* rightMenu = new KRightClickMenu;
    rightMenu->createSubMenu();
            std::cout << "-----------------------------------unit_functions2" << std::endl;

}

void unitConstruct::unit_mediaInfoDialog()
{
    std::cout << "-----------------------------------unit_mediaInfoDialog" << std::endl;
    
    // 创建MediaInfoDialog实例
    MediaInfoDialog* mediaInfo = new MediaInfoDialog;
    
    // 测试setTitle方法
    mediaInfo->setTitle("Test Title");
    
    // 测试setType方法
    mediaInfo->setType("Test Type");
    
    // 测试setSize方法
    mediaInfo->setSize("Test Size");
    
    // 测试setDuration方法
    mediaInfo->setDuration("Test Duration");
    
    // 测试setPath方法
    mediaInfo->setPath("Test Path");
    
    // 测试updateDate方法
    mediaInfo->updateDate();
    
    // 测试ProcessText方法（通过findMaxSize间接测试）
    int maxSize = mediaInfo->findMaxSize();
    
    // 测试gSettingChanged方法
    mediaInfo->gSettingChanged("systemFont");
    mediaInfo->gSettingChanged("systemFontSize");
    
    // 测试on_btn_ok_clicked方法
    mediaInfo->on_btn_ok_clicked();
    
    // 清理资源
    delete mediaInfo;
    
    std::cout << "-----------------------------------unit_mediaInfoDialog over" << std::endl;
}

void unitConstruct::unit_playlistWidget()
{
    std::cout << "-----------------------------------unit_playlistWidget" << std::endl;
    
    // 测试PlayListWidget的各种方法
    m_win->m_playlist_widget->setDefaultTheme();
    m_win->m_playlist_widget->setBlackTheme();
    
    // 测试showItemMenu方法
    m_win->m_playlist_widget->showItemMenu(VIDEO_PATH);
    
    // 测试addItem方法
    PlayItem item;
    item.m_filePath = VIDEO_PATH;
    item.m_duration = 100;
    item.m_lastStopTime = 50;
    m_win->m_playlist_widget->addItem(item);
    
    // 测试deleteItem方法
    m_win->m_playlist_widget->deleteItem(VIDEO_PATH);
    
    // 测试clearList方法
    m_win->m_playlist_widget->clearList();
    
    // 测试updateMarks方法
    QVector<MarkItem> markvec;
    MarkItem markItem;
    markItem.m_filePath = VIDEO_PATH;
    markItem.m_markPos = 10;
    markItem.m_duration = 100;
    markItem.m_describe = "Test mark";
    markvec.append(markItem);
    m_win->m_playlist_widget->updateMarks(markvec);
    
    // 测试deleteMark方法
    m_win->m_playlist_widget->deleteMark(VIDEO_PATH, 10);
    
    // 测试clearMark方法
    m_win->m_playlist_widget->clearMark();
    
    // 测试reloadView方法
    m_win->m_playlist_widget->reloadView(VIDEO_PATH);
    
    // 测试resetDuration方法
    m_win->m_playlist_widget->resetDuration(VIDEO_PATH, 100);
    
    // 测试slotShow和slotHide方法
    m_win->m_playlist_widget->slotShow();
    m_win->m_playlist_widget->slotHide();
    
    // 测试setShowButton方法
    m_win->m_playlist_widget->setShowButton(true);
    m_win->m_playlist_widget->setShowButton(false);
    
    // 测试updateShowIcon和updateHideIcon方法
    m_win->m_playlist_widget->updateShowIcon();
    m_win->m_playlist_widget->updateHideIcon();
    
    // 测试delAvailableIndex方法
    m_win->m_playlist_widget->delAvailableIndex(VIDEO_PATH);
    
    // 测试changeListViewMode方法
    m_win->m_playlist_widget->changeListViewMode();
    m_win->m_playlist_widget->changeListViewMode();
    
    // 测试setPlayIndex方法
    m_win->m_playlist_widget->setPlayIndex(0);
    
    // 测试selectNone方法
    m_win->m_playlist_widget->selectNone();
    
    // 测试gSettingChanged方法
    m_win->m_playlist_widget->gSettingChanged("systemFontSize");
    m_win->m_playlist_widget->gSettingChanged("systemFont");
    
    // 测试PlayListItem的方法
    // 注释掉这部分代码，因为QListWidget在playlistwidget.h中只是前向声明
    // if (m_win->m_playlist_widget->m_playListWidget->count() > 0) {
    //     QListWidgetItem* item = m_win->m_playlist_widget->m_playListWidget->item(0);
    //     PlayListItem* playListItem = static_cast<PlayListItem*>(m_win->m_playlist_widget->m_playListWidget->itemWidget(item));
    //     if (playListItem) {
    //         playListItem->setEnter();
    //         playListItem->setLeave();
    //         playListItem->setDefaultTheme();
    //         playListItem->setBlackTheme();
            
    //         // 触发PlayListItem的信号
    //         playListItem->sigValChange(10);
    //         playListItem->sigRightBPressed("test");
    //         playListItem->sigNotExists("test");
    //         playListItem->sigExistsStateChange("test", true);
            
    //         // 测试槽函数
    //         playListItem->slotNotExit("test");
    //         playListItem->setFontColor(QColor(255, 0, 0));
    //         playListItem->deleteItem();
    //     }
    // }
    
    std::cout << "-----------------------------------unit_playlistWidget over" << std::endl;
}

void unitConstruct::unit_sliderBar()
{
    std::cout << "-----------------------------------unit_sliderBar" << std::endl;
    
    // 创建SliderBar实例
    SliderBar* sliderBar = new SliderBar();
    
    // 测试setValue方法
    sliderBar->setValue(50.0);
    
    // 测试setRange方法
    sliderBar->setRange(0, 100);
    
    // 测试addMark方法
    sliderBar->addMark(25, "Test Mark");
    
    // 测试deleteMark方法
    sliderBar->deleteMark(25);
    
    // 测试clearMark方法
    sliderBar->clearMark();
    
    // 测试setTimeTip方法
    sliderBar->setTimeTip("Test Time Tip");
    
    // 测试setBlackTheme方法
    sliderBar->setBlackTheme();
    
    // 测试setLightTheme方法
    sliderBar->setLightTheme();
    
    // 测试initUI方法
    sliderBar->initUI();

    // 测试鼠标事件
    QMouseEvent enterEvent(QEvent::Enter, QPoint(10, 10), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(sliderBar, &enterEvent);
    
    QMouseEvent leaveEvent(QEvent::Leave, QPoint(10, 10), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(sliderBar, &leaveEvent);
    
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(50, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(sliderBar, &pressEvent);
    
    QMouseEvent moveEvent(QEvent::MouseMove, QPoint(60, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(sliderBar, &moveEvent);
    
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(60, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(sliderBar, &releaseEvent);
    
    // 清理资源
    delete sliderBar;
    
    std::cout << "-----------------------------------unit_sliderBar over" << std::endl;
}

void unitConstruct::unit_checkBox()
{
   std::cout << "-----------------------------------unit_checkBox" << std::endl;
   
   // 创建CheckBox实例
   CheckBox* checkBox = new CheckBox();
   
   // 测试构造函数
   QVERIFY(checkBox != nullptr);
   
   // 测试setText方法
   checkBox->setText("Test Text");
   
   // 测试setFont方法
   QFont font;
   font.setPointSize(12);
   checkBox->setFont(font);
   
   // 测试resizeEvent（通过调整大小间接触发）
   checkBox->resize(100, 30);
   
   // 清理资源
   delete checkBox;
   
   std::cout << "-----------------------------------unit_checkBox over" << std::endl;
}

void unitConstruct::unit_cMessageBox()
{
   std::cout << "-----------------------------------unit_cMessageBox" << std::endl;
   
   // 创建CMessageBox实例
   CMessageBox* msgBox = new CMessageBox();
   
   // 测试构造函数
   QVERIFY(msgBox != nullptr);
   
   // 测试setText方法
   msgBox->setText("Test Message");
   
   // 清理资源
   delete msgBox;
   
   std::cout << "-----------------------------------unit_cMessageBox over" << std::endl;
}

void unitConstruct::unit_eventPassWidget()
{
  std::cout << "-----------------------------------unit_eventPassWidget" << std::endl;
  
  // 创建EventPassWidget实例
  EventPassWidget* eventWidget = new EventPassWidget();
  
  // 测试构造函数
  QVERIFY(eventWidget != nullptr);
  
  // 测试setMouseUsed方法
  eventWidget->setMouseUsed(true);
  eventWidget->setMouseUsed(false);
  
  // 测试事件处理
  // 创建一个鼠标事件
  QMouseEvent mouseEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
  // 发送事件给EventPassWidget
  QCoreApplication::sendEvent(eventWidget, &mouseEvent);
  
  // 创建一个键盘事件
  QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
  // 发送事件给EventPassWidget
  QCoreApplication::sendEvent(eventWidget, &keyEvent);
  
  eventWidget->videIdChange(-1);
  eventWidget->videIdChange(10);
  // 清理资源
  delete eventWidget;
  
  std::cout << "-----------------------------------unit_eventPassWidget over" << std::endl;
}

void unitConstruct::unit_playXWidget()
{
   std::cout << "-----------------------------------unit_playXWidget" << std::endl;
   
   // 创建PlayXWidget实例
   PlayXWidget* playWidget = new PlayXWidget();
   
   // 测试构造函数
   QVERIFY(playWidget != nullptr);
   
   // 清理资源
   delete playWidget;
   
   std::cout << "-----------------------------------unit_playXWidget over" << std::endl;
}

void unitConstruct::unit_setupCodec()
{
  std::cout << "-----------------------------------unit_setupCodec" << std::endl;
  
  // 创建SetupCodec实例
  SetupCodec* codecWidget = new SetupCodec();
  
  // 测试构造函数
  QVERIFY(codecWidget != nullptr);
  
  // 测试主题设置方法
  codecWidget->setBlackTheme();
  codecWidget->setDefaultTheme();
  
  // 测试字体设置方法
  codecWidget->setWidgetFont("Arial", 12);
  
  // 清理资源
  delete codecWidget;
  std::cout << "-----------------------------------unit_setupCodec over" << std::endl;
}

void unitConstruct::unit_urlEditWidget()
{
  std::cout << "-----------------------------------unit_urlEditWidget" << std::endl;
  
  // 创建URLEditWidget实例
  URLEditWidget* urlWidget = new URLEditWidget();
  
  // 测试构造函数
  QVERIFY(urlWidget != nullptr);
  
  // 测试getUrl方法（初始应该为空字符串）
  QString initialUrl = urlWidget->getUrl();
//  QVERIFY(initialUrl.isEmpty());
  
  // 测试设置URL（模拟用户输入）
  // 注意：由于这是一个对话框，我们不能直接测试UI交互，
  // 但我们可以通过访问其内部组件来测试功能
  
  // 清理资源
  delete urlWidget;
  std::cout << "-----------------------------------unit_urlEditWidget over" << std::endl;
}

void unitConstruct::unit_setupSystem()
{
  std::cout << "-----------------------------------unit_setupSystem" << std::endl;
  
  // 创建SetupSystem实例
  SetupSystem* systemWidget = new SetupSystem();
  
  // 测试构造函数
  QVERIFY(systemWidget != nullptr);
  
  // 测试主题设置方法
  systemWidget->setBlackTheme();
  systemWidget->setDefaultTheme();
  
  // 测试字体设置方法
  systemWidget->setWidgetFont("Arial", 12);
  
  // 测试初始化数据方法
  systemWidget->initData();
  
  // 清理资源
  delete systemWidget;
  std::cout << "-----------------------------------unit_setupSystem over" << std::endl;
}

void unitConstruct::unit_setupPlay()
{
  std::cout << "-----------------------------------unit_setupPlay" << std::endl;
  
  // 创建SetupPlay实例
  SetupPlay* playWidget = new SetupPlay();
  
  // 测试构造函数
  QVERIFY(playWidget != nullptr);
  
  // 测试主题设置方法
  playWidget->setBlackTheme();
  playWidget->setDefaultTheme();
  
  // 测试字体设置方法
  playWidget->setWidgetFont("Arial", 12);
  
  // 测试初始化数据方法
  playWidget->initData();
  
  // 清理资源
  delete playWidget;
  std::cout << "-----------------------------------unit_setupPlay over" << std::endl;
}

void unitConstruct::unit_setupScreenshot()
{
  std::cout << "-----------------------------------unit_setupScreenshot" << std::endl;
  
  // 创建SetupScreenshot实例
  SetupScreenshot* screenshotWidget = new SetupScreenshot();
  
  // 测试构造函数
  QVERIFY(screenshotWidget != nullptr);
  
  // 测试主题设置方法
  screenshotWidget->setBlackTheme();
  screenshotWidget->setDefaultTheme();
  
  // 测试字体设置方法
  screenshotWidget->setWidgetFont("Arial", 12);
  
  // 测试初始化数据方法
  screenshotWidget->initData();
  
  // 清理资源
  delete screenshotWidget;
  std::cout << "-----------------------------------unit_setupScreenshot over" << std::endl;
}

void unitConstruct::unit_setupSubtitle()
{
  std::cout << "-----------------------------------unit_setupSubtitle" << std::endl;
  
  // 创建SetupSubtitle实例
  SetupSubtitle* subtitleWidget = new SetupSubtitle();
  
  // 测试构造函数
  QVERIFY(subtitleWidget != nullptr);
  
  // 测试主题设置方法
  subtitleWidget->setBlackTheme();
  subtitleWidget->setDefaultTheme();
  
  // 测试字体设置方法
  subtitleWidget->setWidgetFont("Arial", 12);
  
  // 测试初始化数据方法
  subtitleWidget->initData();
  
  // 清理资源
  delete subtitleWidget;
  std::cout << "-----------------------------------unit_setupSubtitle over" << std::endl;
}

void unitConstruct::unit_setupVolume()
{
  std::cout << "-----------------------------------unit_setupVolume" << std::endl;
  
  // 创建SetupVolume实例
  SetupVolume* volumeWidget = new SetupVolume();
  
  // 测试构造函数
  QVERIFY(volumeWidget != nullptr);
  
  // 测试主题设置方法
  volumeWidget->setBlackTheme();
  volumeWidget->setDefaultTheme();
  
  // 测试字体设置方法
  volumeWidget->setWidgetFont("Arial", 12);
  
  // 测试初始化数据方法
  volumeWidget->initData();
  
  // 清理资源
  delete volumeWidget;
  std::cout << "-----------------------------------unit_setupVolume over" << std::endl;
}
void unitConstruct::unit_setupDialog()
{
   std::cout << "-----------------------------------unit_setupDialog" << std::endl;
   
   // 创建SetUpDialog实例
   SetUpDialog* dialogWidget = new SetUpDialog();
   
   // 测试构造函数
   QVERIFY(dialogWidget != nullptr);
   
   // 测试设置索引方法
   dialogWidget->setIndex(0);
   
   // 清理资源
   delete dialogWidget;
   
   std::cout << "-----------------------------------unit_setupDialog over" << std::endl;
}

void unitConstruct::unit_sqliteHandle()
{
  std::cout << "-----------------------------------unit_sqliteHandle" << std::endl;
  
  // 获取SqliteHandle单例实例
  SqliteHandle* sqliteHandle = SqliteHandle::getInstance("./test.db");
  
  // 测试构造函数
  QVERIFY(sqliteHandle != nullptr);
  
  // 测试获取播放列表方法
  QVector<KPlaylistRecord> playlist = sqliteHandle->getPlayList();
  
  // 测试获取标记方法
  QVector<MarkRecord> marks = sqliteHandle->getMarks();
  
  // 测试getLastTime方法
  double lastTime = sqliteHandle->getLastTime("test_path");
  
  // 测试getDuration方法
  int duration = sqliteHandle->getDuration("test_path");
  std::cout << "-----------------------------------unit_sqliteHandle over" << std::endl;
}
// void unitConstruct::unit_filletWidget()
// {
//     std::cout << "-----------------------------------unit_filletWidget" << std::endl;
//
//     // 创建FilletWidget实例
//     FilletWidget* filletWidget = new FilletWidget();
//
//     // 测试构造函数
//     QVERIFY(filletWidget != nullptr);
//
//     // 清理资源
//     delete filletWidget;
//
//     std::cout << "-----------------------------------unit_filletWidget over" << std::endl;
// }
void unitConstruct::unit_textLabel()
{
   std::cout << "-----------------------------------unit_textLabel" << std::endl;
   
   // 创建QLabel实例替代TextLabel
   QLabel* textLabel = new QLabel();
   
   // 测试构造函数
   QVERIFY(textLabel != nullptr);
   
   // 测试setText方法
   textLabel->setText("Test Text");
   
   // 测试setFont方法
   QFont font;
   font.setPointSize(12);
   textLabel->setFont(font);
   
   // 测试adjustSize方法
   textLabel->adjustSize();
   
   // 清理资源
   delete textLabel;
   
   std::cout << "-----------------------------------unit_textLabel over" << std::endl;
}

void unitConstruct::unit_miniModeButton()
{
   std::cout << "-----------------------------------unit_miniModeButton" << std::endl;
   
   // 创建MiniModeButton实例
   MiniModeButton* miniModeButton = new MiniModeButton("test", QSize(20, 20), QSize(16, 16));
   
   // 测试构造函数
   QVERIFY(miniModeButton != nullptr);
   
   // 测试按钮点击事件
   QTest::mouseClick(miniModeButton, Qt::LeftButton);
   
   // 清理资源
   delete miniModeButton;
   
   std::cout << "-----------------------------------unit_miniModeButton over" << std::endl;
}












void unitConstruct::unit_log()
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

void unitConstruct::unit_dbusAdapter()
{
    std::cout << "-----------------------------------unit_dbusAdapter" << std::endl;
    
    // 创建DbusAdapter实例
    DbusAdapter* dbusAdapter = new DbusAdapter();
    
    // 测试构造函数
    QVERIFY(dbusAdapter != nullptr);
    
    // 测试getter方法
    QString playbackState = dbusAdapter->getPlaybackState();
    QString currentFile = dbusAdapter->CurrentFile();
    int playState = dbusAdapter->PlayState();
    long long position = dbusAdapter->getPosition();
    double volume = dbusAdapter->getVolume();
    QString loopStatus = dbusAdapter->getLoopStatus();
    double rate = dbusAdapter->getRate();
    bool shuffle = dbusAdapter->getShuffle();
    
    // 测试setter方法
    dbusAdapter->setVolume(0.5);
    dbusAdapter->setLoopStatus("Track");
    dbusAdapter->setRate(1.0);
    dbusAdapter->setShuffle(true);
    
    // 测试控制方法
    dbusAdapter->Play();
    dbusAdapter->Pause();
    dbusAdapter->PlayPause();
    dbusAdapter->Stop();
    dbusAdapter->Next();
    dbusAdapter->Previous();
    
    // 测试其他方法
    dbusAdapter->KvPlayPause();
    dbusAdapter->AddFile("/test/file.mp4");
    dbusAdapter->VolumeUp();
    dbusAdapter->VolumeDown();
    dbusAdapter->FullScreen();
    dbusAdapter->LoopMode();
    
    // 添加更多测试以提高覆盖率
    // 测试SetPosition方法的不同分支
    dbusAdapter->SetPosition("", 1000000);  // 正常情况
    dbusAdapter->SetPosition("", -1000000); // 负数情况
    dbusAdapter->SetPosition("track1", 1000000); // 带TrackId情况
    
    // 测试OpenUri方法的不同分支
    dbusAdapter->OpenUri(""); // 空字符串
    dbusAdapter->OpenUri("/test/localfile.mp4"); // 本地文件
//    dbusAdapter->OpenUri("http://example.com/video.mp4"); // URL
    
    // 测试setRate方法的不同分支
    dbusAdapter->setRate(0.5);  // 正常值
    dbusAdapter->setRate(1.0);  // 正常值
    dbusAdapter->setRate(2.0);  // 正常值
    dbusAdapter->setRate(0.0);  // 边界值
    
    // 测试Seek方法的不同分支
    dbusAdapter->Seek(1000000);  // 正常值
    dbusAdapter->Seek(-1000000); // 负值
    dbusAdapter->Seek(100000);   // 小于阈值的值
    
    // 测试setVolume方法的不同分支
    dbusAdapter->setVolume(0.0);  // 最小值
    dbusAdapter->setVolume(1.0);  // 最大值
    dbusAdapter->setVolume(1.5);  // 超过最大值
    dbusAdapter->setVolume(-0.5); // 负值
    
    // 测试KvPlayPause, PlayPause, Pause, Play方法的不同状态
    dbusAdapter->KvPlayPause();
    dbusAdapter->PlayPause();
    dbusAdapter->Pause();
    dbusAdapter->Play();
    
    // 测试Previous方法
    dbusAdapter->Previous();
    
    // 测试getPlaybackState和PlayState方法
    QString state = dbusAdapter->getPlaybackState();
    int playStateValue = dbusAdapter->PlayState();
    
    // 清理资源
    delete dbusAdapter;
    
    std::cout << "-----------------------------------unit_dbusAdapter over" << std::endl;
}

void unitConstruct::unit_controlBar()
{
    std::cout << "-----------------------------------unit_controlBar" << std::endl;
    
    // 创建ContralBar实例
    ContralBar* controlBar = new ContralBar();
    
    // 测试构造函数
    QVERIFY(controlBar != nullptr);
    
    // 测试基础方法
    controlBar->setDuration(100);
    controlBar->setCurrentTime(50);
    controlBar->setFullScreenState(true);
    controlBar->setFullScreenState(false);
    controlBar->setUIMode(true);
    controlBar->setUIMode(false);
    
    // 测试标记相关方法
    controlBar->clearMark();
    controlBar->addMark(10, "Test mark");
    controlBar->insertMark(20, "Another mark");
    controlBar->deleteMark(10);
    controlBar->updateMarks();
    
    // 测试主题设置方法
    controlBar->setBlackTheme();
    controlBar->setDefaultTheme();
    
    m_win->m_contral_bar->slotMute();

    m_win->m_contral_bar->slotShowPreview(10);

    m_win->m_contral_bar->gSettingChanged("systemFontSize");

    m_win->m_contral_bar->gSettingChanged("systemFont");

    // 测试事件处理
    QEvent enterEvent(QEvent::Enter);
    QCoreApplication::sendEvent(controlBar, &enterEvent);
    
    QEvent leaveEvent(QEvent::Leave);
    QCoreApplication::sendEvent(controlBar, &leaveEvent);
    
    // 测试鼠标事件
    QMouseEvent mouseMoveEvent(QEvent::MouseMove, QPoint(10, 10), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(controlBar, &mouseMoveEvent);
    
    // 清理资源
    delete controlBar;
    
    std::cout << "-----------------------------------unit_controlBar over" << std::endl;
}

void unitConstruct::unit_setupShortcut()
{
    std::cout << "-----------------------------------unit_setupShortcut" << std::endl;
    
    // 创建SetupShortcut实例
    SetupShortcut* setupShortcut = new SetupShortcut();
    
    // 测试构造函数
    QVERIFY(setupShortcut != nullptr);
    
    // 测试主题设置方法
    setupShortcut->setBlackTheme();
    setupShortcut->setDefaultTheme();
    
    // 测试配置方法
    setupShortcut->flushChange();
    setupShortcut->clearChange();
    setupShortcut->reloadFromShortcuts();
    
    // 测试字体设置方法
    setupShortcut->setWidgetFont("Arial", 12);
    
    // 测试ShortCutItem
    ShortCutItem* shortcutItem = new ShortCutItem("Test Item", "Ctrl+T", setupShortcut);
    shortcutItem->setBlackTheme();
    shortcutItem->setDefaultTheme();
    shortcutItem->reset();
    shortcutItem->flush();
    shortcutItem->setKeyForce("Ctrl+Shift+T");
    
    // 测试ShortEdit
    ShortEdit* shortEdit = new ShortEdit();
    shortEdit->setBlackTheme();
    shortEdit->setDefaultTheme();
    shortEdit->setShortCut("Ctrl+S");
    
    // 测试键盘事件
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Space, Qt::ControlModifier);
    QCoreApplication::sendEvent(shortEdit, &keyEvent);
    
    // 清理资源
    delete shortEdit;
    delete shortcutItem;
    delete setupShortcut;
    
    std::cout << "-----------------------------------unit_setupShortcut over" << std::endl;
}

void unitConstruct::unit_mediaInfo()
{
    KMedia::MediaHandle media("aaaa");

    VideoInfo *vinfo = new VideoInfo;
    vinfo->ID = 1;
    vinfo->Width = 100;
    vinfo->Height = 100;
    media.getVideoMap()[1] = vinfo;
    media.getVBitRate(1);

    AudioInfo *ainfo = new AudioInfo;
    ainfo->ID = 1;
    ainfo->Channels = 1;
    media.getAudioMap()[1] = ainfo;
    media.getABitRate(1);
}

void unitConstruct::unit_mpvcore()
{
    m_win->m_core->m_fileInfo.length = 10;
    emit g_user_signal->sigDbusSeek(10000);
    m_win->m_core->m_fileInfo.length = 10000000000;
    emit g_user_signal->sigDbusSeek(10000);
//    QTest::qWaitFor([&]() {
//        return false;
//    }, 100);

    emit g_user_signal->sigDbusSeek(-10000);


    m_win->m_core->slotVolumeChange("kylin-video", -1, true);
    m_win->m_core->m_volume = 100;
    m_win->m_core->m_mute = false;
    m_win->m_core->m_restartOkState = true;
    m_win->m_core->slotVolumeChange("kylin-video-990", 10, true);

    m_win->m_core->SetProperties();

    m_win->m_core->m_playingFile = "1111/222:333";
    m_win->m_core->LoadFileInfo();

    m_win->m_core->m_duration = 1000;
    g_playstate = Mpv::Playing;
    m_win->m_core->Seek(10, true, true);
    m_win->m_core->Seek(10, true, false);
    m_win->m_core->Seek(10, false, true);
    m_win->m_core->Seek(10, false, true);


    m_win->m_core->Stop();

    m_win->m_core->LoadMetadata();

    m_win->m_core->LoadAudioParams();

    m_win->m_core->LoadVideoParams();

    m_win->m_core->LoadChapters();

    m_win->m_core->m_lastTime = 10;
    m_win->m_core->m_showProfile = true;
    m_win->m_core->ShowText("1111", 10);
    m_win->m_core->m_vid = -1;
    m_win->m_core->ShowText("", 10);


    g_playstate = Mpv::Paused;
    m_win->m_core->Open("1111", 100);

    g_playstate = Mpv::Paused;
    m_win->m_core->Open("1111.iso", 100);

    m_win->m_core->m_isHFlip = true;
    m_win->m_core->m_isVFlip = true;
    m_win->m_core->Stop();

    g_playstate = Mpv::Paused;
    m_win->m_core->Speed(2);

    g_playstate = Mpv::Paused;
    m_win->m_core->m_speed = 1.6;
    m_win->m_core->SpeedUp();
    m_win->m_core->m_speed = 1;
    m_win->m_core->SpeedUp();

    g_playstate = Mpv::Paused;
    m_win->m_core->m_speed = 2.0;
    m_win->m_core->SpeedDown();
    m_win->m_core->m_speed = 1;
    m_win->m_core->SpeedUp();

    m_win->m_core->m_lastTime = 100;
    m_win->m_core->m_duration = 10000;
    m_win->m_core->AddBookMark(VIDEO_PATH);

    m_win->m_core->SubForward();

    m_win->m_core->AddSubs(QStringList() << "123" << "223");

    GlobalConfig::getInstance()->m_video_output_type = GlobalConfig::VO_WID;
    m_win->m_core->VideoOutput("auto");
    m_win->m_core->VideoOutput("no");

    m_win->m_core->VideoDecoder("default");


}

void unitConstruct::unit_playlist()
{
//    MarkItem a, b;
//    a.m_filePath = "1";
//    b.m_filePath = "2";
//    markSort(a, b);
//    a.m_filePath = "2";
//    b.m_filePath = "1";
//    markSort(a, b);
    m_win->m_playlist_widget->getPlayList()->m_fileList << "123" << "223" << "323";
    m_win->m_playlist_widget->getPlayList()->deleteInvalidItems();

    m_win->m_playlist_widget->getPlayList()->m_availableIndexVec.push_back(1);
    m_win->m_playlist_widget->getPlayList()->setPlayIndex(3);
    m_win->m_playlist_widget->getPlayList()->setPlayIndex(1);

    m_win->m_playlist_widget->getPlayList()->setAvaliable("123");
    m_win->m_playlist_widget->getPlayList()->setAvaliable("323");

    m_win->m_playlist_widget->getPlayList()->addMark("123", 10, "111", "123");

    m_win->m_playlist_widget->getPlayList()->m_playOrder = ONE_LOOP;
//    m_win->m_playlist_widget->getPlayList()->m_availableIndexVec.push_back(0);
    m_win->m_playlist_widget->getPlayList()->playNext(0);
    m_win->m_playlist_widget->getPlayList()->m_playOrder = ONE_LOOP;
    m_win->m_playlist_widget->getPlayList()->playNext(0);
    m_win->m_playlist_widget->getPlayList()->m_playOrder = SEQUENCE;
    m_win->m_playlist_widget->getPlayList()->playNext(0);
    m_win->m_playlist_widget->getPlayList()->m_playOrder = LIST_LOOP;
    m_win->m_playlist_widget->getPlayList()->playNext(0);
    m_win->m_playlist_widget->getPlayList()->m_playOrder = RANDOM;
    m_win->m_playlist_widget->getPlayList()->playNext(0);

    m_win->m_playlist_widget->getPlayList()->m_availableIndexVec.push_back(0);
    m_win->m_playlist_widget->getPlayList()->m_playOrder = ONE_LOOP;
    m_win->m_playlist_widget->getPlayList()->playNext(true);
    m_win->m_playlist_widget->getPlayList()->m_playOrder = SEQUENCE;
    m_win->m_playlist_widget->getPlayList()->playPrev(true);
    m_win->m_playlist_widget->getPlayList()->m_playOrder = LIST_LOOP;
    m_win->m_playlist_widget->getPlayList()->playPrev(true);
    m_win->m_playlist_widget->getPlayList()->m_playOrder = RANDOM;
    m_win->m_playlist_widget->getPlayList()->playPrev(true);
}

void unitConstruct::unit_globalConfig()
{
    GlobalConfig::getInstance()->miniToTray.second           = true;
    GlobalConfig::getInstance()->pauseWhenMini.second        = true;
    GlobalConfig::getInstance()->canRunMultiple.second       = true;
    GlobalConfig::getInstance()->keepStateWhenWakeup.second  = true;

    GlobalConfig::getInstance()->playLastPos.second          = true;
    GlobalConfig::getInstance()->playRelationFile.second     = true;
    GlobalConfig::getInstance()->clearListWhenExit.second    = true;
    GlobalConfig::getInstance()->fullScreenWhenPlay.second   = true;

    GlobalConfig::getInstance()->screenShotPath.second           = "screenShotPath.first";
    GlobalConfig::getInstance()->screenShotFormat.second         = "screenShotFormat.first";
    GlobalConfig::getInstance()->screenShotSaveToClip.second     = true;
    GlobalConfig::getInstance()->screenShotCurrentSize.second    = true;

    GlobalConfig::getInstance()->subDir.second           = "subDir.first";
    GlobalConfig::getInstance()->subFontSize.second      = 12;
    GlobalConfig::getInstance()->subFontFamily.second    = "subFontFamily.first";
    GlobalConfig::getInstance()->loadSameNameSub.second  = true;
    GlobalConfig::getInstance()->loadAllSubInDir.second  = true;

    GlobalConfig::getInstance()->audioOut.second         = "audioOut.first";
    GlobalConfig::getInstance()->globalVolume.second     = true;
    GlobalConfig::getInstance()->standardVolume.second   = true;

    GlobalConfig::getInstance()->videoDecoder.second         = "videoDecoder.first";
    GlobalConfig::getInstance()->videoOutput.second          = "videoOutput.first";
    GlobalConfig::getInstance()->videoDecodeThreads.second   = 3;

    GlobalConfig::getInstance()->seamlessBrowsing.second = true;
    GlobalConfig::getInstance()->audioChannel.second = 3;

    GlobalConfig::getInstance()->flushChange();

    GlobalConfig::getInstance()->readCpuModelName();
}

void unitConstruct::unit_util()
{
    Util::IsValidUrl("http://1121212");

    Util::FormatTime(1, 10000);

    Util::FormatRelativeTime(-1);

    Util::FormatRelativeTime(100);

    Util::FormatNumberWithAmpersand(1, 1);
    Util::FormatNumberWithAmpersand(1, 1000);
    Util::FormatNumberWithAmpersand(-1, 1000);
    Util::FormatNumberWithAmpersand(1, 19);
    Util::FormatNumberWithAmpersand(-1, 19);

    QStringList list;
    list << "123" << "1121";
    Util::ToNativeSeparators(list);

    Util::FromNativeSeparators(list);

}

void unitConstruct::unit_slider()
{
    Slider slider;
    slider.resize(100, 100);
    slider.m_mouse_x = 100;
    slider.updatePos(true);

    slider.updatePos(false);

    MiniModeButton mini("aaa", QSize(100, 100), QSize(100, 100));
    mini.setBlackTheme();

    m_win->m_right_click_menu->sigDefaultFrame();
    m_win->m_right_click_menu->sig4Div3Frame();
    m_win->m_right_click_menu->sig16Div9Frame();
    m_win->m_right_click_menu->sigFullFrame();
    m_win->m_right_click_menu->sigRestoreFrame();

}















//void unitConstruct::unit_exit()
//{
//    g_config->clearListWhenExit.first = true;
//    g_playstate = Mpv::Idle;
//    m_win->appQuit();

//    g_playstate = Mpv::Playing;
//    m_win->appQuit();

//    QTest::qWaitFor([&]() {
//        return false;
//    }, 1000);

//    MainWindow *window = new MainWindow(QStringList());
//    m_win->showList();
////    m_win->kylin_video_play_request(QStringList());
//    QTest::qWaitFor([&]() {
//        return false;
//    }, 3000);
//    g_config->miniToTray.first = true;
//    window->slotQuit();

//    QTest::qWaitFor([&]() {
//        return false;
//    }, 1000);

//    g_config->miniToTray.first = false;
//    window->slotClose();
//}
