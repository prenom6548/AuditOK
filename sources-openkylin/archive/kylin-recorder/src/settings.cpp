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
 *  Authors: baijincheng <baijincheng@kylinos.cn>
 */
#include <QLabel>
#include <QDebug>
#include <QSettings>
#include <QFile>
#include <QtCore/QCoreApplication>
#include <ukuistylehelper/ukuistylehelper.h>

#include "settings.h"
#include "mainwindow.h"
// #include "xatom-helper.h"

Settings::Settings(QWidget *parent) : QMainWindow(parent)
{
    int WIDTH = 420;
    int HEIGHT = 200;
    m_mainWidget = new QWidget();
    m_mainWidget->setAccessibleName("kylin-recorder_settings_QWidget_m_mainWidget");
    m_mainWidget->setWindowFlag(Qt::Tool); //此代码必须在此窗管协议前，否则此模态窗口背景不变灰
    kdk::UkuiStyleHelper::self()->removeHeader(m_mainWidget);

    m_Data = new QGSettings(KYLINRECORDER);
    m_darkData = new QGSettings(UKUI_THEME_GSETTING_PATH);
    // 用户手册功能
    m_DaemonIpcDbus = new DaemonDbus();
    m_mainWidget->setFixedSize(WIDTH, HEIGHT);
    setFocusPolicy(Qt::ClickFocus); // this->setFocusPolicy(Qt::NoFocus);//设置焦点类型
    setWindowTitle(tr("Settings"));
    //显示在活动屏幕中间新方法

    //标题栏设置和布局
    m_setWindowLabel = new QLabel(this);
    m_setWindowLabel->setAccessibleName("kylin-recorder_QLabel_m_setWindowLabel");
    QPushButton *piclb = new QPushButton(this); //窗体左上角图片Label
    piclb->setAccessibleName("kylin-recorder_QPushButton_piclb");
    piclb->setIcon(QIcon::fromTheme("kylin-recorder", QIcon(":/png/png/recording_32.png")));
    piclb->setFixedSize(25, 25);
    piclb->setIconSize(QSize(25, 25)); //重置图标大小
    piclb->setStyleSheet("QPushButton{border:0px;background:transparent;}");
    m_setWindowLabel->setText(tr("Settings"));
    //关闭按钮
    m_closeButton = new QToolButton(this);
    m_closeButton->setAccessibleName("kylin-recorder_settings_QToolButton_m_closeButton");
    m_closeButton->setFixedSize(30, 30);
    m_closeButton->setIcon(QIcon::fromTheme("window-close-symbolic")); //主题库的叉子图标
    m_closeButton->setProperty("isWindowButton", 0x2);
    m_closeButton->setProperty("useIconHighlightEffect", 0x8);
    m_closeButton->setAutoRaise(true);
    m_closeButton->setToolTip(tr("Close"));

    m_alterButton = new kdk::KPushButton(this);
    m_alterButton->setAccessibleName("kylin-recorder_QPushButton_m_alterButton");
    m_alterButton->setFixedSize(85, 36);
    m_alterButton->setText(tr("Alter"));
    m_titleWidget = new QWidget();
    m_titleWidget->setAccessibleName("kylin-recorder_settings_QWidget_m_titleWidget");
    m_titleLayout = new QHBoxLayout();
    m_titleLayout->addWidget(piclb);
    m_titleLayout->addWidget(m_setWindowLabel);
    m_titleLayout->addWidget(m_closeButton);
    m_titleLayout->setContentsMargins(8, 4, 4, 0);
    m_titleWidget->setFixedHeight(35);
    m_titleWidget->setLayout(m_titleLayout);

    m_centerWidget = new QWidget();
    m_centerWidget->setAccessibleName("kylin-recorder_settings_QWidget_m_centerWidget");
    m_fileStoreWidget = new QWidget();  //文件存储wid
    m_fileStoreWidget->setAccessibleName("kylin-recorder_settings_QWidget_m_fileStoreWidget");
    m_fileFormatWidget = new QWidget(); //文件格式wid
    m_fileFormatWidget->setAccessibleName("kylin-recorder_settings_QWidget_m_fileFormatWidget");
    m_sourceWidget = new QWidget();     //录音来源wid
    m_sourceWidget->setAccessibleName("kylin-recorder_settings_QWidget_m_sourceWidget");
    m_centerLayout = new QVBoxLayout();
    m_fileStoreLayout = new QHBoxLayout();  //文件存储Layout
    m_fileFormatLayout = new QHBoxLayout(); //文件格式Layout
    m_sourceLayout = new QHBoxLayout();     //录音来源Layout

    m_storeLabel = new MyLabel(tr("Storage:"));
    m_storeLabel->setAccessibleName("kylin-recorder_QLabel_m_storeLabel");
    m_storeLabel->setFixedWidth(90);
    m_defaultLocation = QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/" + tr("Recorder") + "/";
    m_pathLabel = new MyLabel(this);
    m_pathLabel->setAccessibleName("kylin-recorder_settings_MyLabel_m_pathLabel");
    QString savePath = m_Data->get("path").toString();
    if (savePath == "") {
#ifndef WHITE_TEST
        //配置为空
        qDebug() << "初始配置文件为空自动填充默认路径";
        m_pathLabel->setText(m_defaultLocation);
        m_Data->set("path", m_defaultLocation);
        QDir dir(m_defaultLocation);
        if(!dir.exists()){
            dir.mkdir(m_defaultLocation);
            qDebug() << "Create new folder :" << m_defaultLocation;
        }
        //存在路径
        QFileInfo info(savePath);
        qDebug() <<info.absoluteFilePath()<<info.isWritable();
        if(info.isWritable()) {
            m_pathLabel->setText(savePath);
        } else {
            m_pathLabel->setText(m_defaultLocation);
            m_Data->set("path", m_defaultLocation);
            QDir dir(m_defaultLocation);
            if(!dir.exists()){
                dir.mkdir(m_defaultLocation);
                qDebug() << "Create new folder :" << m_defaultLocation;
            }
        }
#endif
    }
    else{
        m_pathLabel->setText(m_Data->get("path").toString());
    }

//    m_Data->set("recorderpath", "");
//    if (m_Data->get("recorderpath").toString() == "") {
//        m_Data->set("recorderpath", m_defaultLocation);
//    }

    m_pathLabel->setAcceptDrops(false); //禁止拖拽入
    m_pathLabel->setFixedSize(160, 36);
    m_fileStoreLayout->addWidget(m_storeLabel);
    m_fileStoreLayout->addWidget(m_pathLabel);
    m_fileStoreLayout->addWidget(m_alterButton);
    m_fileStoreLayout->setContentsMargins(0, 0, 0, 0);
    m_fileStoreWidget->setLayout(m_fileStoreLayout);

    m_formatLabel = new MyLabel(tr("Format:"));
    m_formatLabel->setAccessibleName("kylin-recorder_QLabel_m_formatLabel");
    m_formatLabel->setFixedWidth(90);
    m_formatDownList = new QComboBox(this);
    m_formatDownList->setAccessibleName("kylin-recorder_QComboBox_m_formatDownList");
    m_formatDownList->setFixedSize(255, 36);

    m_formatDownList->addItem("mp3");
    m_formatDownList->addItem("m4a");
    m_formatDownList->addItem("wav");

    m_fileFormatLayout->addWidget(m_formatLabel);
    m_fileFormatLayout->addWidget(m_formatDownList);
    m_fileFormatLayout->setContentsMargins(0, 0, 0, 0);
    m_fileFormatWidget->setLayout(m_fileFormatLayout);

    m_sourceLabel = new MyLabel(tr("Source:"));
    m_sourceLabel->setAccessibleName("kylin-recorder_QLabel_m_sourceLabel");
    m_sourceLabel->setFixedWidth(90);
    //    sourceLabel->setStyleSheet("font-size:14px;");
    m_sourceDownList = new QComboBox(this);
    m_sourceDownList->setAccessibleName("kylin-recorder_QComboBox_m_sourceDownList");
    //    sourceDownList->setStyleSheet("font-size:14px;");
    m_sourceDownList->setFixedSize(255, 36);
    m_sourceDownList->addItem(tr("Microphone"));
    m_sourceDownList->addItem(tr("System Inside"));

    m_sourceLayout->addWidget(m_sourceLabel);
    m_sourceLayout->addWidget(m_sourceDownList);
    m_sourceLayout->setContentsMargins(0, 0, 0, 0);
    m_sourceWidget->setLayout(m_sourceLayout);

    m_centerLayout->addWidget(m_fileStoreWidget);
    m_centerLayout->addWidget(m_fileFormatWidget);
    m_centerLayout->addWidget(m_sourceWidget);
    m_centerLayout->setContentsMargins(32, 16, 24, 24);
    m_centerWidget->setLayout(m_centerLayout);

    connect(m_closeButton, &QToolButton::clicked, m_mainWidget, &Settings::close);
    connect(m_alterButton, &QPushButton::clicked, this, &Settings::openFileDialog);
    connect(m_formatDownList, SIGNAL(currentIndexChanged(int)), this, SLOT(saveType(int)));
    connect(m_sourceDownList, SIGNAL(currentIndexChanged(int)), this, SLOT(recordSource(int)));

    m_mainLayout = new QVBoxLayout();
    m_mainLayout->addWidget(m_titleWidget);
    m_mainLayout->addWidget(m_centerWidget, 0, Qt::AlignHCenter);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setMargin(1);
    m_mainWidget->setLayout(m_mainLayout);
    m_mainWidget->setAttribute(Qt::WA_ShowModal, true); //模态窗口

    if (m_Data->get("source").toInt() == 1) {
        m_sourceDownList->setCurrentIndex(0);
    } else if (m_Data->get("source").toInt() == 2) //内部
    {
        m_sourceDownList->setCurrentIndex(1);
        qDebug() << "内部";
    } else if (m_Data->get("source").toInt() == 3) //麦克风
    {
        qDebug() << "全部";
    } else {
    }

    if (m_Data->get("type").toInt() == 1) //开始录制格式为mp3
    {
        m_formatDownList->setCurrentIndex(0);
    } else if (m_Data->get("type").toInt() == 2) //开始录制格式为m4a
    {
        m_formatDownList->setCurrentIndex(1);
    } else if (m_Data->get("type").toInt() == 3) //开始录制格式为wav
    {
        m_formatDownList->setCurrentIndex(2);
    }
}

// 实现键盘响应
void Settings::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "键盘事件";
    // F1快捷键打开用户手册
    if (event->key() == Qt::Key_F1) {
        if (!m_DaemonIpcDbus->daemonIsNotRunning()) {
            // F1快捷键打开用户手册，如kylin-recorder
            //由于是小工具类，下面的showGuide参数要填写"kylin-recorder"
            m_DaemonIpcDbus->showGuide("kylin-recorder");
        }
    }
}

//打开文件目录
void Settings::openFileDialog()
{
#ifndef WHITE_TEST
    //选择文件目录
    m_selectDirPath = QFileDialog::getExistingDirectory(m_mainWidget, tr("Select a file storage directory"), "/");
    qDebug() << "选择的文件目录是:" << m_selectDirPath;
    int value = inputEditJudge(m_selectDirPath + "/");
    if (value == 1) {
        //正常时选择目录,要把目录集写入配置文件recorderpath中
        m_pathLabel->setText(m_selectDirPath);
        QString strSaveFilePath = m_Data->get("recorderpath").toString();
        if (strSaveFilePath != "") {
            QString newFilePath = strSaveFilePath + "," + m_selectDirPath + "/";
            QStringList pathList = strSaveFilePath.split(",");
            if (!pathList.contains(m_selectDirPath + "/")) {
                m_Data->set("recorderpath", newFilePath);
            } else {
                qDebug() << "包含路径就不用写了";
            }
        }
        qDebug() << "查看一下存入的路径:" << m_Data->get("recorderpath").toString();
    } else if (value == 0) {
        //命名非法时选择默认路径
        m_Data->set("path", m_defaultLocation);
        m_pathLabel->setText(m_defaultLocation);
    } else {
        //为空时选择默认路径
        m_Data->set("path", m_Data->get("path").toString());
        m_pathLabel->setText(m_Data->get("path").toString());
    }
#endif
}

//编辑label时判断
int Settings::inputEditJudge(QString fileDir)
{
#ifndef WHITE_TEST
    QString str = fileDir;
    if (fileDir != "/") {
        qDebug() << "输入的文件名是:" << fileDir;
        //子串中至少包含"/home/用户名/"
        QString subStr = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/";
        QString path = str.mid(0, str.lastIndexOf("/"));
        QFileInfo fi(path);
        qDebug() << "满足条件的前提" << subStr << "输入的是:" << str << "最后一个/之前的路径:" << path;
        if (str != "") {
            if (!str.contains(subStr) || str.contains(".") || str.contains("#") || str.contains("?") || str.contains(" ")
                || str.contains("\"") || str.contains("'") || str.contains("\\")) {
                //主要是防止非法路径头:至少包含"/home/用户名/"
                QMessageBox::warning(m_mainWidget, tr("Warning"), tr("The directory does not have write permissions, select the user directory to store the files ."));
                return 0;
            } else {
                if (fi.exists()) {
                    QString name = str.split("/").last();
                    int length = name.length();
                    qDebug() << "长度:" << length;
                    if (length <= 20) {
                        m_Data->set("path", fileDir);
                        return 1;
                    } else {
                        //超过20个字符就提示命名不能超过20个字符
                        QMessageBox::warning(m_mainWidget, tr("Warning"),
                                             tr("The file name cannot exceed 20 characters!"));
                        return 0;
                    }

                } else {
                    //主要是防止非法路径尾,比如：用户自己乱写的目录/home/bjc/音乐/h/h显然此路径不存在
                    QMessageBox::warning(m_mainWidget, tr("Warning"), tr("The directory does not have write permissions, select the user directory to store the files ."));
                    return 0;
                }
            }

        } else //当点击关闭时storeEdit会为空
        {
            qDebug() << "若为空则还是选择之前的路径";
            return 2;
        }
    }
#endif
    return 2;
}

//要提示用户不可以在QLineEdit这里修改路径，只能去规定的弹窗中修改防止用户乱改!!!
void Settings::editSlot()
{
    qDebug() << "文本框编辑";
    //    storeEdit->setText(defaultLocation);
    //    QMessageBox::warning(mainWid,tr("Warning"),tr("You cannot edit here!"));
    return;
}

void Settings::saveType(int index)
{
    if (index == 0) {
        m_Data->set("type", 1); // mp3
        qDebug() << "类型改为" << m_Data->get("type").toInt() << " mp3格式";
    } else if (index == 1) {
        m_Data->set("type", 2); // m4a
        qDebug() << "类型改为" << m_Data->get("type").toInt();
    } else if (index == 2) {
        m_Data->set("type", 3); // wav
        qDebug() << "类型改为" << m_Data->get("type").toInt();
    } else {
    }
}

void Settings::recordSource(int index) // mic录音源
{
    if (index == 0) {
        qDebug() << "设置录音源为mic";
        m_Data->set("source", 1);
    } else if (index == 1) {
        m_Data->set("source", 2);
        qDebug() << "设置录音源为系统内部";
    } else {
    }
}
