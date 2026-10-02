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
#include "setupsubtitle.h"

#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFontDatabase>
#include <QFileSystemWatcher>

#include "global/global.h"
#include "../checkbox.h"

using namespace Global;

SetupSubtitle::SetupSubtitle(QWidget *parent) :
    ThemeWidget(parent)
{
    initLayout();

    QFontDatabase fdb;
    for (QString it : fdb.families()) {
        if (!it.toLower().contains("ubuntu")) {
            m_familyComboBox->addItem(it);
        }
    }

    // 字体选择列表 先自己设置一些
    QStringList size_list;
    size_list << "26" << "32" << "42" << "52" << "64" << "76";
    m_sizeComboBox->addItems(size_list);

    initConnect();
    m_allName->hide();

    m_subPathEdit->setReadOnly(true);
    m_subPathEdit->setCursor(Qt::IBeamCursor);

    connectThemeSetting();
}

SetupSubtitle::~SetupSubtitle()
{
}

void SetupSubtitle::initData()
{
    m_sameName->setChecked(g_config->loadSameNameSub.first);
    m_allName->setChecked(g_config->loadAllSubInDir.first);
    // 如果默认字幕路径为空的话设置为家目录
    if (g_config->subDir.first == "") {
        g_config->subDir.second = QDir::homePath();
        g_config->flushChange();
    }
    m_subPathEdit->setText(g_config->subDir.first);

    // 加载字体
    QString font_family = g_config->subFontFamily.first;
    int font_size = g_config->subFontSize.first;
    QFont f;
    if(font_family.length() > 0)
        m_familyComboBox->setCurrentText(font_family);
    else
        m_familyComboBox->setCurrentText(f.defaultFamily());

    if(font_size == 0)
        m_sizeComboBox->setCurrentText(QString("%1").arg(f.pointSize()));
    else
        m_sizeComboBox->setCurrentText(QString("%1").arg(font_size));
}

void SetupSubtitle::setBlackTheme()
{
    m_title1Label->setStyleSheet(QString("color:rgb(255,255,255);"));
    m_title2Label->setStyleSheet(QString("color:rgb(255,255,255);"));

//    m_familyComboBox->setStyleSheet(QString("color:rgb(249,249,249);background-color:rgb(64,64,64);"));
//    m_sizeComboBox->setStyleSheet(QString("color:rgb(249,249,249);background-color:rgb(64,64,64);"));
//    m_subPathEdit->setStyleSheet(QString("color:rgb(249,249,249);background-color:rgb(64,64,64);"));
    m_browserButton->setStyleSheet(QString("color:rgb(249,249,249);background-color:rgb(64,64,64);"));
    m_allName->setStyleSheet(QString("color:rgb(249,249,249);"));
    m_sameName->setStyleSheet(QString("color:rgb(249,249,249);"));
    m_label1->setStyleSheet(QString("color:rgb(249,249,249);"));
    m_label2->setStyleSheet(QString("color:rgb(249,249,249);"));
    m_label3->setStyleSheet(QString("color:rgb(249,249,249);"));
}

void SetupSubtitle::setDefaultTheme()
{
    m_title1Label->setStyleSheet(QString("color:rgb(38,38,38);"));
    m_title2Label->setStyleSheet(QString("color:rgb(38,38,38);"));

//    m_familyComboBox->setStyleSheet(QString("color:rgb(38,38,38);background-color:rgb(240,240,240);"));
//    m_sizeComboBox->setStyleSheet(QString("color:rgb(38,38,38);background-color:rgb(240,240,240);"));
//    m_subPathEdit->setStyleSheet(QString("color:rgb(38,38,38);background-color:rgb(240,240,240);"));
    m_browserButton->setStyleSheet("QPushButton{background-color:rgb(224,224,224);color:rgb(38,38,38);}"
                                  "QPushButton:hover{color:rgb(255,255,255);}");
    m_allName->setStyleSheet(QString("color:rgb(38,38,38);"));
    m_sameName->setStyleSheet(QString("color:rgb(38,38,38);"));
    m_label1->setStyleSheet(QString("color:rgb(38,38,38);"));
    m_label2->setStyleSheet(QString("color:rgb(38,38,38);"));
    m_label3->setStyleSheet(QString("color:rgb(38,38,38);"));
}

void SetupSubtitle::setWidgetFont(QString family, int size)
{
    QFont f(family);
    f.setPointSize(size + 2);
    m_title1Label->setFont(f);
    m_title2Label->setFont(f);

    f.setPointSize(size);
    m_label1->setFont(f);
    m_label2->setFont(f);
    m_label3->setFont(f);
    m_familyComboBox->setFont(f);
    m_allName->setFont(f);
    m_sameName->setFont(f);
    m_sizeComboBox->setFont(f);
    m_browserButton->setFont(f);
    m_subPathEdit->setFont(f);
}

void SetupSubtitle::initLayout()
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(30, 10, 24, 8);
    lay->setSpacing(0);

    m_title1Label = new QLabel(tr("Sub loading"));
    m_title1Label->setAccessibleName("kylin-video_QLabel_m_title1Label");
    lay->addWidget(m_title1Label);

    m_sameName = new CheckBox;
    m_sameName->setAccessibleName("kylin-video_CheckBox_m_sameName");
    m_sameName->setText(tr("Auto loading subtitles with the same name"));
    lay->addWidget(m_sameName);

    m_allName = new CheckBox;
    m_allName->setAccessibleName("kylin-video_CheckBox_m_allName");
    m_allName->setText(tr("Auto loading other subtitles in the same folder"));
    lay->addWidget(m_allName);

    {
        m_hboxLayout1 = new QHBoxLayout;
        m_hboxLayout1->setObjectName("kylin-video_QHBoxLayout_m_hboxLayout1");
        m_hboxLayout1->setContentsMargins(0, 5, 0, 5);
        m_hboxLayout1->setSpacing(10);

        m_label1 = new QLabel(tr("Sub Path"));
        m_label1->setAccessibleName("kylin-video_QLabel_m_label1");
        m_label1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_hboxLayout1->addWidget(m_label1);

        m_subPathEdit = new QLineEdit;
        m_subPathEdit->setAccessibleName("kylin-video_setupsubtitle_QLineEdit_m_subPathEdit");
        m_hboxLayout1->addWidget(m_subPathEdit);

        m_browserButton = new QPushButton(tr("browser"));
        m_browserButton->setAccessibleName("kylin-video_QPushButton_m_browserButton");
        m_hboxLayout1->addWidget(m_browserButton);

        lay->addLayout(m_hboxLayout1);
    }

    lay->addSpacing(20);

    m_title2Label = new QLabel(tr("Font Style"));
    m_title2Label->setAccessibleName("kylin-video_QLabel_m_title2Label");
    m_title2Label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lay->addWidget(m_title2Label);

    {
        m_hboxLayout2 = new QHBoxLayout;
        m_hboxLayout2->setObjectName("kylin-video_QHBoxLayout_m_hboxLayout2");
        m_hboxLayout2->setContentsMargins(0, 5, 0, 5);
        m_hboxLayout2->setSpacing(8);

        m_label2 = new QLabel(tr("Family"));
        m_label2->setAccessibleName("kylin-video_QLabel_m_label2");
        m_label2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_hboxLayout2->addWidget(m_label2);

        m_familyComboBox = new QComboBox;
        m_familyComboBox->setAccessibleName("kylin-video_setupsubtitle_QComboBox_m_familyComboBox");
        m_hboxLayout2->addWidget(m_familyComboBox);

        m_hboxLayout2->setStretchFactor(m_familyComboBox, 1);

        lay->addLayout(m_hboxLayout2);
    }

    {
        m_hboxLayout3 = new QHBoxLayout;
        m_hboxLayout3->setObjectName("kylin-video_QHBoxLayout_m_hboxLayout3");
        m_hboxLayout3->setContentsMargins(0, 5, 0, 5);
        m_hboxLayout3->setSpacing(8);

        m_label3 = new QLabel(tr("Size"));
        m_label3->setAccessibleName("kylin-video_QLabel_m_label3");
        m_label3->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_hboxLayout3->addWidget(m_label3);

        m_sizeComboBox = new QComboBox;
        m_sizeComboBox->setAccessibleName("kylin-video_setupsubtitle_QComboBox_m_sizeComboBox");
        m_hboxLayout3->addWidget(m_sizeComboBox);

        m_hboxLayout3->setStretchFactor(m_sizeComboBox, 1);

        lay->addLayout(m_hboxLayout3);
    }

    lay->addStretch();
}

void SetupSubtitle::initConnect()
{
    connect(m_sameName, &QCheckBox::clicked, [&](bool checked){
        g_config->loadSameNameSub.second = checked;
    });
    connect(m_allName, &QCheckBox::clicked, [&](bool checked){
        g_config->loadAllSubInDir.second = checked;
    });
    connect(m_browserButton, &QPushButton::clicked, [&](){
        // 打开文件夹
        QString url;

        {
            QFileDialog fd(parentWidget());
            fd.setModal(true);
            QList<QUrl> list = fd.sidebarUrls();
            int sidebarNum = 8;
            QString home = QDir::homePath().section("/", -1, -1);
            QString mnt = "/media/" + home + "/";
            QDir mntDir(mnt);
            mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
            QFileInfoList filist = mntDir.entryInfoList();
            QList<QUrl> mntUrlList;
            for(int i=0; i < sidebarNum && i < filist.size(); i++) {
                QFileInfo fi = filist.at(i);
                //华为990、9a0需要屏蔽最小系统挂载的目录
                if (fi.fileName() == "2691-6AB8")
                     continue;
                mntUrlList << QUrl("file://" + fi.filePath());
            }
            QFileSystemWatcher fsw(&fd);
            fsw.addPath("/media/" + home + "/");
            connect(&fsw, &QFileSystemWatcher::directoryChanged, &fd, [=, &sidebarNum, &mntUrlList, &list, &fd](const QString path){
                QDir wmnDir(path);
                wmnDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
                QFileInfoList wfilist = wmnDir.entryInfoList();
                mntUrlList.clear();
                for(int i=0; i < sidebarNum && i < wfilist.size(); ++i) {
                    QFileInfo fi = wfilist.at(i);
                    //华为990、9a0需要屏蔽最小系统挂载的目录
                    if (fi.fileName() == "2691-6AB8")
                         continue;
                    mntUrlList << QUrl("file://" + fi.filePath());
                }
                fd.setSidebarUrls(list + mntUrlList);
                fd.update();
            });
            connect(&fd, &QFileDialog::finished, &fd, [=, &list, &fd](){
                fd.setSidebarUrls(list);
            });
            fd.setSidebarUrls(list + mntUrlList);
            fd.setDirectory(QDir(g_config->subDir.second));
            fd.setWindowTitle(tr("Video Player Choose a directory"));
            fd.setFileMode(QFileDialog::DirectoryOnly);

            g_shortcut->makeAllInvalid();
            if(fd.exec() == QFileDialog::Accepted)
            {
                url = fd.selectedUrls().at(0).toString();
                if(url.startsWith("file"))
                    url.remove(0, 7);
            }
            g_shortcut->makeAllValid();
        }

        if(url.length() > 0) {
            m_subPathEdit->setText(url);
            g_config->subDir.second = url;
            QFontMetrics fontWidth(m_subPathEdit->font());//得到每个字符的宽度
            QString show_name = fontWidth.elidedText(g_config->subDir.second, Qt::ElideMiddle, m_subPathEdit->width() - m_subPathEdit->font().pointSizeF() * 1.5);
            m_subPathEdit->setText(show_name);
            m_subPathEdit->setToolTip(show_name == g_config->subDir.second ?
                                        "" : g_config->subDir.second);
        }
    });
    connect(m_familyComboBox, &QComboBox::currentTextChanged, [&](QString family){
        g_config->subFontFamily.second = family;
    });
    connect(m_sizeComboBox, &QComboBox::currentTextChanged, [&](QString size){
        g_config->subFontSize.second = size.toInt();
    });
}

void SetupSubtitle::paintEvent(QPaintEvent *e)
{
    return QWidget::paintEvent(e);
}
