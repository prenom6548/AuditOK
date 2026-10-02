/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: baijincheng <baijincheng@kylinos.cn>
 */
#ifndef SETTINGS_H
#define SETTINGS_H

#include "save.h"
#include <QApplication>
#include <QMainWindow>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QDesktopWidget>
#include <QRadioButton>
#include <QLineEdit>
#include <QButtonGroup>
#include <QGridLayout>
#include <QSettings>
#include <QGSettings>
#include <QRect>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QAudioDeviceInfo>

#include "daemondbus.h"
#include "lineedit.h"
#include "mylabel.h"
#include <kpushbutton.h>

class Settings : public QMainWindow
{
    Q_OBJECT

    friend class TestMainWindow;
public:
    explicit Settings(QWidget *parent = nullptr);
    QGSettings *m_Data = nullptr;
    QGSettings *m_darkData = nullptr;
    QString m_defaultLocation; //默认路径：/home/<user>/Music/Recorder/

    QWidget *m_mainWidget = nullptr;
    QToolButton *m_closeButton = nullptr;
    // 用户手册功能
    DaemonDbus *m_DaemonIpcDbus = nullptr;


    QLabel *m_setWindowLabel;
    MyLabel *m_storeLabel = nullptr;
    //    LineEdit *storeEdit;//文件存储//重写的Linedit因为QLineEdit没有点击事件
    MyLabel *m_pathLabel = nullptr;        //路径标签
    kdk::KPushButton *m_alterButton = nullptr;  //更改btn
    MyLabel *m_formatLabel = nullptr;       //文件格式
    QComboBox *m_formatDownList = nullptr; //格式下拉列表mp3、wav、m4a
    MyLabel *m_sourceLabel = nullptr;       //来源标签
    QComboBox *m_sourceDownList = nullptr; //录音来源下拉列表mp3、wav、m4a
    void hideTitlebar();



    // 键盘响应事件
    void keyPressEvent(QKeyEvent *event);

private:
    QHBoxLayout *m_titleLayout = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;

    QWidget *m_titleWidget = nullptr;

    QWidget *m_centerWidget = nullptr;     //中心wid4行
    QWidget *m_fileStoreWidget = nullptr;  //文件存储wid
    QWidget *m_fileFormatWidget = nullptr; //文件格式wid
    QWidget *m_sourceWidget = nullptr;     //录音来源wid
    QVBoxLayout *m_centerLayout = nullptr;
    QHBoxLayout *m_fileStoreLayout = nullptr;  //文件存储Layout
    QHBoxLayout *m_fileFormatLayout = nullptr; //文件格式Layout
    QHBoxLayout *m_sourceLayout = nullptr;     //录音来源Layout

    QString m_selectDirPath;

    int inputEditJudge(QString fileDir); //文本框编辑或编程使其改变时要判断非法字符

public slots:

    void openFileDialog(); //点击文本框时触发

    void editSlot();              //文本框编辑时触发
    void saveType(int index);     //选择时保存的类型
    void recordSource(int index); //选中mic录音源

signals:
};

#endif // SETTINGS_H
