/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */

#include "maininterface.h"



/**
 * @brief MainInterface 构造函数
 *
 * MainInterface 的构造函数，用于初始化界面控件和建立信号连接。
 *
 * @param parent 父控件指针
 */
MainInterface::MainInterface(QWidget *parent) : QWidget(parent)
{
    m_belowwidget = new QWidget(this);
    m_belowwidget->setContentsMargins(0,0,0,0);
    init();

    QVBoxLayout *layout = new QVBoxLayout(this);
    this->setContentsMargins(0,0,0,0);
    layout->addWidget(m_belowwidget);
    layout->setContentsMargins(0,0,0,0);

    this->setLayout(layout);
    connect(m_globalthemewidget, &GlobalThemeWidget::newWallpaperFilePath, this, [&](const QString& path) {
        emit wallpaperPathChanged(path);
    });
    connect(m_globalthemewidget, &GlobalThemeWidget::newCoverFilePath, this, [&](const QString& path) {
        emit coverPathChanged(path);
    });
    connect(m_globalthemewidget, &GlobalThemeWidget::newRadius, this, [&](const int radius) {
        emit radiusChanged(radius);
    });
    connect(m_globalthemewidget, &GlobalThemeWidget::newAccentColor, this, [&](const QColor selectedColor) {
        emit accentColorChanged(selectedColor);
    });
    connect(m_globalthemewidget, &GlobalThemeWidget::newExterior, this, [&](const QColor selectedColor) {
        emit exteriorChanged(selectedColor);
    });
    connect(m_globalthemewidget, &GlobalThemeWidget::newTransparency, this, [&](const int transparency) {
        emit transparencyChanged(transparency);
    });
    connect(m_globalthemewidget, &GlobalThemeWidget::newWindowRadius, this, [&](const int windowFilletSize) {
        emit windowRadiusChanged(windowFilletSize);
    }); 
    connect(m_globalthemewidget, &GlobalThemeWidget::globalthemeChange, this, [&]( QMap<QString, bool> *globalthemeChangeMap){
        emit globalthemeChange(globalthemeChangeMap);
    });
    connect(m_iconthemewidget, &IconThemeWidget::newAppIconsMap, this, [&]( QMap<QString, QString> *appiconsmaps) {
        emit appIconsMapChanged(appiconsmaps);
    });
    connect(m_iconthemewidget, &IconThemeWidget::newSystemIconsMap, this, [&]( QMap<QString, QString> *systemiconsmaps) {
        emit systemIconsMapChanged(systemiconsmaps);
    });
    connect(m_iconthemewidget, &IconThemeWidget::newPlaceIconsMap, this, [&]( QMap<QString, QString> *placeiconsmaps) {
        emit placeIconsMapChanged(placeiconsmaps);
    });
    connect(m_iconthemewidget, &IconThemeWidget::newDeviceIconsMap, this, [&]( QMap<QString, QString> *deviceiconsmaps) {
        emit deviceIconsMapChanged(deviceiconsmaps);
    });
    connect(m_iconthemewidget, &IconThemeWidget::iconthemeChange, this, [&]( QMap<QString, bool> *iconthemeChangeMap){
        emit iconthemeChange(iconthemeChangeMap);
    });
    connect(m_cursorthemewidget, &CursorThemeWidget::newCursorMap, this, [&]( QMap<QString, QString> *cursormaps) {
        emit cursorMapChanged(cursormaps);
    });
    connect(m_cursorthemewidget, &CursorThemeWidget::newTimeCursorMap, this, [&]( QMap<QString, QString> *timecursormaps) {
        emit timeCursorMapChanged(timecursormaps);
    });

    connect(m_cursorthemewidget, &CursorThemeWidget::cursorthemeChange, this, [&]( QMap<QString, bool> *cursorthemeChangeMap){
        emit cursorthemeChange(cursorthemeChangeMap);
    });
    connect(m_plymouththemewidget, &PlymouthThemeWidget::newPlymouthFilePath, this, [&](const QString& path) {
        emit plymouthPathChanged(path);
    });
    connect(m_plymouththemewidget, &PlymouthThemeWidget::refreshPlymouthDir, this, [=](){
        emit refreshPlymouthDir();
    });
    connect(m_grubthemewidget, &GrubThemeWidget::newGrubFilePath, this, [&](const QString& path) {
        emit grubPathChanged(path);
    });

    connect(m_globalthemewidget,&GlobalThemeWidget::build,m_buildbtn,&QPushButton::click);
    connect(m_iconthemewidget,&IconThemeWidget::build,m_buildbtn,&QPushButton::click);
    connect(m_cursorthemewidget,&CursorThemeWidget::build,m_buildbtn,&QPushButton::click);
    connect(m_plymouththemewidget,&PlymouthThemeWidget::build,m_buildbtn,&QPushButton::click);
    connect(m_grubthemewidget,&GrubThemeWidget::build,m_buildbtn,&QPushButton::click);

}

/**
 * @brief 设置是否启用历史记录
 *
 * 根据给定的布尔值设置是否启用历史记录。
 *
 * @param ishistory 是否启用历史记录
 */
void MainInterface::isHistory(bool ishistory)
{
    m_ishistory = ishistory;
}

/**
 * @brief 初始化主界面
 *
 * 初始化主界面，包括左侧栏、主题堆叠小部件等。
 */
void MainInterface::init()
{
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    this->setContentsMargins(0,0,0,0);
    initLeftBar();

    m_themestackedwidget = new QStackedWidget(m_belowwidget);

    m_globalthemewidget = new GlobalThemeWidget(m_themestackedwidget);
    m_iconthemewidget = new IconThemeWidget(m_ishistory,m_themestackedwidget);
    m_cursorthemewidget = new CursorThemeWidget(m_themestackedwidget);
    m_plymouththemewidget = new PlymouthThemeWidget(m_themestackedwidget);
    m_grubthemewidget = new GrubThemeWidget(m_themestackedwidget);

    connect(m_globalthemewidget,&GlobalThemeWidget::wallpaperupdate,this, &MainInterface::wallpaperupdate);
    connect(this, &MainInterface::wallpaperupdate, m_cursorthemewidget, &CursorThemeWidget::wallpaperupdate);
    connect(this, &MainInterface::wallpaperupdate, m_iconthemewidget, &IconThemeWidget::wallpaperupdate);

    m_themestackedwidget->addWidget(m_globalthemewidget);
    m_themestackedwidget->addWidget(m_iconthemewidget);
    m_themestackedwidget->addWidget(m_cursorthemewidget);
    m_themestackedwidget->addWidget(m_plymouththemewidget);
    m_themestackedwidget->addWidget(m_grubthemewidget);

    m_themestackedwidget->setCurrentIndex(1);
    QPushButton *line = new QPushButton();
    line->setDisabled(true);
    line->setFixedWidth(1);
    line->setFixedHeight(2000);
    m_layout = new QHBoxLayout();
    m_layout->addWidget(m_navigationwidget);
    m_layout->addWidget(line);
    m_layout->addWidget(m_themestackedwidget);

    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(0);

    m_belowwidget->setLayout(m_layout);

    connect(m_list, &QListView::clicked, this, [=](const QModelIndex &index){
//        QString type = index.data().toString();
        if(0 == index.row()){
            DataCollect::sendNavigationKdkDataAsync(DataCollect::n_GlobalTheme);
            m_themestackedwidget->setCurrentIndex(0);
        }else if(1 == index.row()){
            DataCollect::sendNavigationKdkDataAsync(DataCollect::n_IconTheme);
            m_themestackedwidget->setCurrentIndex(1);
        }else if(2 == index.row()){
            DataCollect::sendNavigationKdkDataAsync(DataCollect::n_CursorTheme);
            m_themestackedwidget->setCurrentIndex(2);
        }else if(3 == index.row()){
            DataCollect::sendNavigationKdkDataAsync(DataCollect::n_PlymouthTheme);
            m_themestackedwidget->setCurrentIndex(3);
            this->startShowPlymouth();
        }else if(4 == index.row()){
            DataCollect::sendNavigationKdkDataAsync(DataCollect::n_GrubTheme);
            m_themestackedwidget->setCurrentIndex(4);
        }
    });
    connect(m_buildbtn,&QPushButton::clicked,this,[=](){
        m_info = new InfoCreateWidget(nullptr,"create");
        FileProcess getInfo;
        m_info->getFileInfo(getInfo.FileSearch(FileProcess::g_date));
        m_info->show();
        connect(m_info,&InfoCreateWidget::hasGetThemeName,this,[=](){
            emit hasGetThemeName();
        });
//        connect(info,&InfoCreateWidget::readytoBuild,this,&MainInterface::startCopy);
        connect(m_info,&InfoCreateWidget::gohomesignals,this,[=](){
            //发送返回主界面信号给mainwindow
            GlobalThemeWidget::g_themeChange = true;
            emit gohomesignals();
            delete m_info;
        });
    });
////    connect()
//    qDebug()<<this->contentsMargins();
}

/**
 * @brief 初始化左侧工具栏
 *
 * 创建一个QWidget作为左侧工具栏，并设置其最小和最大尺寸。然后创建一个QVBoxLayout来管理左侧工具栏的布局。
 * 创建一个QButtonGroup来管理多个按钮的选中状态。将多个按钮添加到按钮组中，并设置它们为可选中状态。
 * 将全局按钮和构建按钮添加到布局中，并设置构建按钮为普通按钮。最后将布局中的控件添加到左侧工具栏中。
 */
void MainInterface::initLeftBar()
{
    m_navigationwidget = new QWidget(m_belowwidget);
    m_navigationwidget->setMinimumSize(200,620);
    m_navigationwidget->setMaximumWidth(200);
    m_navigation = new kdk::KNavigationBar(m_navigationwidget);

    QVBoxLayout *leftLayout = new QVBoxLayout(m_navigationwidget);

    m_globalbtn = new QStandardItem();
    m_globalbtn->setText(tr("GlobalTheme"));
    m_iconbtn = new QStandardItem();
    m_iconbtn->setText(tr("IconTheme"));
    m_cursorbtn = new QStandardItem();
    m_cursorbtn->setText(tr("CursorTheme"));
    m_plymouthbtn = new QStandardItem();
    m_plymouthbtn->setText(tr("PlymouthTheme"));
    m_grubbtn = new QStandardItem();
    m_grubbtn->setText(tr("GrubTheme"));

    m_model = m_navigation->model();
    m_list = m_navigation->listview();

    m_model->appendRow(m_globalbtn);
    m_model->appendRow(m_iconbtn);
    m_model->appendRow(m_cursorbtn);
    m_model->appendRow(m_plymouthbtn);
    m_model->appendRow(m_grubbtn);
    m_list->setModel(m_model);
    m_list->setMaximumWidth(200);

    m_buildbtn = new QPushButton(m_navigationwidget);
    m_buildbtn->setFont(QFont("Arial" , 12));

    leftLayout->addWidget(m_navigation);
    leftLayout->addStretch();
    leftLayout->addWidget(m_buildbtn);
    m_buildbtn->setVisible(false);
}

void MainInterface::setGlobalTheme()
{
    m_buildbtn->setText(tr("Start Global Theme Building"));
    FileProcess::g_createThemeType = "globalTheme";
    m_themestackedwidget->setCurrentIndex(0);

    m_list->setRowHidden(0,false);
    m_list->setRowHidden(1,false);
    m_list->setRowHidden(2,false);
    m_list->setRowHidden(3,false);
    m_list->setRowHidden(4,false);
    QModelIndex index = m_model->index(0, 0);
    m_list->setCurrentIndex(index);

}

void MainInterface::setIconTheme()
{
    m_buildbtn->setText(tr("Start Icon Theme Building"));
    FileProcess::g_createThemeType = "iconTheme";
    m_themestackedwidget->setCurrentIndex(1);

    m_list->setRowHidden(0,true);
    m_list->setRowHidden(1,false);
    m_list->setRowHidden(2,true);
    m_list->setRowHidden(3,true);
    m_list->setRowHidden(4,true);
    QModelIndex index = m_model->index(1, 0);
    m_list->setCurrentIndex(index);

}

void MainInterface::setCursorTheme()
{
    m_buildbtn->setText(tr("Start Cursor Theme Building"));
    FileProcess::g_createThemeType = "cursorTheme";
    m_themestackedwidget->setCurrentIndex(2);

    m_list->setRowHidden(0,true);
    m_list->setRowHidden(1,true);
    m_list->setRowHidden(2,false);
    m_list->setRowHidden(3,true);
    m_list->setRowHidden(4,true);
    QModelIndex index = m_model->index(2, 0);
    m_list->setCurrentIndex(index);

}

void MainInterface::setPlymouthTheme()
{
    m_buildbtn->setText(tr("Start Plymouth Theme Building"));
    FileProcess::g_createThemeType = "plymouthTheme";
    m_themestackedwidget->setCurrentIndex(3);

    m_list->setRowHidden(0,true);
    m_list->setRowHidden(1,true);
    m_list->setRowHidden(2,true);
    m_list->setRowHidden(3,false);
    m_list->setRowHidden(4,true);
    QModelIndex index = m_model->index(3, 0);
    m_list->setCurrentIndex(index);

}

void MainInterface::setGrubTheme()
{
    m_buildbtn->setText(tr("Start Grub Theme Building"));
    FileProcess::g_createThemeType = "grubTheme";
    m_themestackedwidget->setCurrentIndex(4);

    m_list->setRowHidden(0,true);
    m_list->setRowHidden(1,true);
    m_list->setRowHidden(2,true);
    m_list->setRowHidden(3,true);
    m_list->setRowHidden(4,false);
    QModelIndex index = m_model->index(4, 0);
    m_list->setCurrentIndex(index);

}

void MainInterface::getThemeInfo(const HistoryInfo &InfoData)
{
    if (InfoData.themetype == "globalTheme"){
        setGlobalTheme();
        //发送信号，将info传入widget，并且显示在widget
        m_globalthemewidget->eidtInitWidget(InfoData);
        m_cursorthemewidget->eidtInitWidget(InfoData);
        m_iconthemewidget->eidtInitWidget(InfoData);
        m_plymouththemewidget->eidtInitWidget(InfoData);
        m_grubthemewidget->eidtInitWidget(InfoData);
    }else if(InfoData.themetype == "cursorTheme"){
        setCursorTheme();
        m_cursorthemewidget->eidtInitWidget(InfoData);
    }else if(InfoData.themetype == "iconTheme"){
        setIconTheme();
        m_iconthemewidget->eidtInitWidget(InfoData);
    }else if(InfoData.themetype == "plymouthTheme"){
        setPlymouthTheme();
        m_plymouththemewidget->eidtInitWidget(InfoData);
    }else if(InfoData.themetype == "grubTheme"){
        setGrubTheme();
        m_grubthemewidget->eidtInitWidget(InfoData);
    }

}

/**
 * @brief 获取应用程序图标映射表
 *
 * 返回一个 QMap<QString, QString> 类型的指针，该指针指向应用程序图标映射表。
 *
 * @return 应用程序图标映射表指针
 */
QMap<QString, QString>* MainInterface::getAppIconsMap()
{
    //资源文件的appiconmap
    return m_iconthemewidget->getAppIconsMap();
}

QMap<QString, QString> *MainInterface::getSystemIconsMap()
{
    //资源文件的systemiconmap
    return m_iconthemewidget->getSystemIconsMap();
}

QMap<QString, QString> *MainInterface::getPlaceIconsMap()
{
    return m_iconthemewidget->getPlaceIconsMap();
}

QMap<QString, QString> *MainInterface::getDeviceIconsMap()
{
    return m_iconthemewidget->getDeviceIconsMap();
}

QMap<QString, QString> *MainInterface::getCurosrMap()
{
    //资源文件的cursoriconmap
    return m_cursorthemewidget->getCursorMap();
}

QMap<QString, QString> *MainInterface::getTimeCurosrMap()
{
    //资源文件的timecursoriconmap
    return m_cursorthemewidget->getTimeCursorMap();
}

/**
 * @brief 刷新界面
 *
 * 根据传入的主题类型刷新界面。
 *
 * @param themeType 主题类型
 */
void MainInterface::refresh(QString themeType)
{
    if(themeType == "global"){
        m_globalthemewidget->refresh();
    } else if(themeType == "icon"){
        m_iconthemewidget->refresh();
    } else if(themeType == "cursor"){
        m_cursorthemewidget->refresh();
    } else if(themeType == "plymouth"){
        m_plymouththemewidget->refresh();
    } else if(themeType == "grub"){
        m_grubthemewidget->refresh();
    }
}

void MainInterface::updateButtonGroup(bool isGlobalTheme)
{
    if(isGlobalTheme){
//        m_plymouthbtn->setVisible(false);
//        m_grubbtn->setVisible(false);
//        m_globalbtn->setVisible(true);
//        m_iconbtn->setVisible(true);
//        m_cursorbtn->setVisible(true);
    }else{
//        m_globalbtn->setVisible(false);
//        m_iconbtn->setVisible(false);
//        m_cursorbtn->setVisible(false);
//        m_plymouthbtn->setVisible(true);
//        m_grubbtn->setVisible(true);
    }

}

/**
 * @brief 更新缩略图
 *
 * 将指定的缩略图路径设置为 PlymouthThemeWidget 的缩略图图标。
 *
 * @param thumbnailPath 缩略图路径
 */
void MainInterface::updateThumbnail(QString thumbnailPath)
{
    m_plymouththemewidget->setThumbnailIcon(thumbnailPath);
}

void MainInterface::startShowPlymouth()
{
    m_plymouththemewidget->startShowPlymouth();
}

void MainInterface::useMaximumScale()
{
    m_globalthemewidget->useMaximumScale();
    m_iconthemewidget->useMaximumScale();
    m_cursorthemewidget->useMaximumScale();
    m_plymouththemewidget->useMaximumScale();
    m_grubthemewidget->useMaximumScale();
}

void MainInterface::useRestoreScale()
{
    m_globalthemewidget->useRestoreScale();
    m_iconthemewidget->useRestoreScale();
    m_cursorthemewidget->useRestoreScale();
    m_plymouththemewidget->useRestoreScale();
    m_grubthemewidget->useRestoreScale();
}

void MainInterface::hideNavigation(bool isglobal)
{
    m_navigationwidget->setHidden(!isglobal);
}

void MainInterface::setBuildBtnText(bool isglobal)
{
    m_iconthemewidget->setBuildBtnText(isglobal);
    m_cursorthemewidget->setBuildBtnText(isglobal);
    m_plymouththemewidget->setBuildBtnText(isglobal);
    m_grubthemewidget->setBuildBtnText(isglobal);
}
