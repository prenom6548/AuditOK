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
#include "globalthemewidget.h"
#include "../fileProcess/filecheck.h"

bool GlobalThemeWidget::g_themeChange;
/**
 * @brief 构造函数
 *
 * 创建一个 GlobalThemeWidget 对象，并设置父窗口。
 *
 * @param parent 父窗口指针
 */
GlobalThemeWidget::GlobalThemeWidget(QWidget *parent) : QWidget(parent)
{
    m_themeChangeMap = new QMap<QString, bool>();
    m_globalPresenceMap = new QMap<QString, bool>();

    this->setContentsMargins(0,0,0,0);
    initPreviewWidget();
    initRightWidget();

    QPushButton *line = new QPushButton();
    line->setDisabled(true);
    line->setFixedWidth(1);
    line->setFixedHeight(2000);
    m_globalthemelayout = new QHBoxLayout();
    m_globalthemelayout->addWidget(m_previewwidget);
    m_globalthemelayout->addWidget(line);
    m_globalthemelayout->addWidget(m_rightwidget);

    m_globalthemelayout->setContentsMargins(0,0,0,0);
    m_globalthemelayout->setSpacing(0);

    this->setLayout(m_globalthemelayout);
}

/**
 * @brief 获取壁纸路径
 *
 * 返回壁纸的路径。
 *
 * @return 壁纸路径
 */
QString GlobalThemeWidget::getWallpaperPath()
{
    return m_wallpaperpath;
}

/**
 * @brief 初始化小部件
 *
 * 根据给定的历史信息编辑小部件的初始化。
 *
 * @param InfoData 历史信息
 */
void GlobalThemeWidget::eidtInitWidget(const HistoryInfo &InfoData)
{
    m_info = InfoData;
    m_info.isEdit = true;
    this->setRadiusSetting(m_info.radius);
    this->setAccentColor(m_info.accentcolor);
    this->setCover(m_info.coverpath);
    this->setWallpaper(m_info.wallpaperpath);
    this->setTransparency(m_info.transparency);
    this->setWindowRadius(m_info.windowradius);
    this->setStyle(m_info.widgetstyle);
    m_preview->updateWallpaper(m_info.wallpaperpath);

    QFile m_newCoverFile(m_info.coverpath);
    QFile m_oldCoverFile(":/resource/background/custom-preview.png");

    m_globalPresenceMap->clear();
    QCryptographicHash m_newCoverhash(QCryptographicHash::Md5);
    m_newCoverhash.addData(&m_newCoverFile);
    QCryptographicHash m_oldCoverhash(QCryptographicHash::Md5);
    m_oldCoverhash.addData(&m_oldCoverFile);
    if(m_newCoverhash.result() != m_oldCoverhash.result()){
        m_globalPresenceMap->insert("cover",true);
    } else {
        m_globalPresenceMap->insert("cover",false);
    }

    QFile m_newWallpaperFile(m_info.wallpaperpath);
    QFile m_oldWallpaperFile(":/resource/background/1-openkylin.jpg");

    m_globalPresenceMap->clear();
    QCryptographicHash m_newWallpaperhash(QCryptographicHash::Md5);
    m_newWallpaperhash.addData(&m_newWallpaperFile);
    QCryptographicHash m_oldWallpaperhash(QCryptographicHash::Md5);
    m_oldWallpaperhash.addData(&m_oldWallpaperFile);
    if(m_newWallpaperhash.result() != m_oldWallpaperhash.result()){
        m_globalPresenceMap->insert("wallpaper",true);
    } else {
        m_globalPresenceMap->insert("wallpaper",false);
    }
}

/**
 * @brief 刷新全局主题界面
 *
 * 刷新全局主题界面，更新设置项。
 */
void GlobalThemeWidget::refresh()
{
    m_info.coverpath = ":/resource/background/custom-preview.png";
    m_info.wallpaperpath = ":/resource/background/1-openkylin.jpg";
    m_info.widgetstyle = "light";
    m_info.radius = 6;
    m_info.accentcolor = "#3790FA";
    m_info.transparency = 65;
    m_info.windowradius = 6;

    this->setRadiusSetting(m_info.radius);
    this->setAccentColor(m_info.accentcolor);
    this->setWindowRadius(m_info.windowradius);
    this->setTransparency(m_info.transparency);
    this->setCover(m_info.coverpath);
    this->setWallpaper(m_info.wallpaperpath);
    this->setStyle(m_info.widgetstyle);
    m_preview->updateWidgetBackgroundColor(QColor(255, 255, 255));
    m_preview->updateOverlayImage(":/resource/background/panel-light.png");
    m_preview->updateWallpaper(":/resource/background/background-light.png");
    m_preview->updatescale1();

    m_globalPresenceMap->clear();
    m_globalPresenceMap->insert("cover",false);
    m_globalPresenceMap->insert("wallpaper",false);
    m_globalPresenceMap->insert("windowcolor",false);
    m_globalPresenceMap->insert("btnradius",false);
    m_globalPresenceMap->insert("highcolor",false);
    m_globalPresenceMap->insert("windowtransparency",false);
    m_globalPresenceMap->insert("windowradius",false);
}

/**
 * @brief 初始化预览小部件
 *
 * 创建一个新的 QWidget 作为预览小部件，并设置其大小和背景颜色。
 * 预览小部件用于展示壁纸的预览图。
 */
void GlobalThemeWidget::initPreviewWidget()
{
    m_previewwidget = new BasePreviewWidget(this);
    m_previewwidget->setMinimumSize(495,620);

    m_preview = new GlobalImageWidget(m_previewwidget,m_wallpaperpath);

    m_previewlayout = new QVBoxLayout(m_previewwidget);
    addspaceritem();
    m_previewwidget->setLayout(m_previewlayout);
}

/**
 * @brief 初始化右侧小部件
 *
 * 初始化右侧小部件，包括滚动区域、视口小部件、各个主题设置小部件等。
 */
void GlobalThemeWidget::initRightWidget()
{
    m_rightwidget = new QWidget(this);
    m_rightwidget->setMinimumSize(400,620);
    m_rightwidget->setMaximumWidth(400);
    m_rightwidget->setBackgroundRole(QPalette::Base);
    m_rightwidget->setContentsMargins(0,0,0,0);

    m_scrollArea = new QScrollArea(m_rightwidget);
    m_scrollArea->setContentsMargins(0,0,0,0);
    m_scrollArea->setFrameStyle(QFrame::HLine);
    m_scrollArea->setMinimumWidth(m_rightwidget->width());
    m_scrollArea->setWidgetResizable(true);

    m_viewportwidget = new QWidget(m_scrollArea);
    m_scrollArea->setWidget(m_viewportwidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(m_viewportwidget);
    m_viewportwidget->setContentsMargins(0,0,0,0);

    initCoverWidget();
    initWallPaperWidget();
    initExteriorWidget();
    initFilletWidget();
    initAccentColorWidget();
    initTransparencyWidget();
    initWindowFilletWidget();
    initbuildWidget();
    QPushButton *line = new QPushButton();
    line->setDisabled(true);
    line->setFixedHeight(1);
    QPushButton *line1 = new QPushButton();
    line1->setDisabled(true);
    line1->setFixedHeight(1);
    QPushButton *line2 = new QPushButton();
    line2->setDisabled(true);
    line2->setFixedHeight(1);
    QPushButton *line3 = new QPushButton();
    line3->setDisabled(true);
    line3->setFixedHeight(1);
    QPushButton *line4 = new QPushButton();
    line4->setDisabled(true);
    line4->setFixedHeight(1);
    QPushButton *line5 = new QPushButton();
    line5->setDisabled(true);
    line5->setFixedHeight(1);

    QPushButton *line6 = new QPushButton();
    line6->setDisabled(true);
    line6->setFixedHeight(1);
    line6->setFixedWidth(m_rightwidget->width());

    mainLayout->addWidget(m_coverwidget);
    mainLayout->addWidget(line);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_wallpaperwidget);
    mainLayout->addWidget(line1);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_exteriorwidget);
    mainLayout->addWidget(line2);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_filletwidget);
    mainLayout->addWidget(line3);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_accentcolorwidget);
    mainLayout->addWidget(line4);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_transparencywidget);
    mainLayout->addWidget(line5);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_windowfilletwidget);
    mainLayout->addStretch(1);

    mainLayout->setContentsMargins(0,0,0,0);
    m_viewportwidget->setLayout(mainLayout);

    QVBoxLayout *mainWidgetLayout = new QVBoxLayout(m_rightwidget);
    mainWidgetLayout->addWidget(m_scrollArea);
    mainWidgetLayout->setSpacing(0);
    mainWidgetLayout->addWidget(line6);
    mainWidgetLayout->addWidget(m_buildwidget);
    mainWidgetLayout->setContentsMargins(0,0,0,0);
    m_rightwidget->setLayout(mainWidgetLayout);

}

/**
 * @brief 初始化CoverWidget
 *
 * 初始化CoverWidget控件，并设置相关属性。
 */
void GlobalThemeWidget::initCoverWidget()
{
    m_coverwidget = new QWidget(m_viewportwidget);
    m_coverwidget->setFixedHeight(240);
    QLabel *title = new QLabel(m_coverwidget);
    title->setText(tr("Overall theme rendering"));

    QVBoxLayout *layout = new QVBoxLayout();

    QWidget *cover = new QWidget(m_coverwidget);
    cover->setFixedHeight(160);
    QHBoxLayout *coverlayout = new QHBoxLayout(cover);
    m_coverbtn = new EditButton(cover,"cover");
    QPixmap pixmap(":/resource/background/custom-preview.png");
    m_coverbtn->setIcon(QIcon(pixmap));
    m_coverbtn->setIconSize(QSize(165,106));
    addPicWidget * a = new addPicWidget();
    a->setType("cover");
    coverlayout->addWidget(m_coverbtn);
    coverlayout->addStretch(1);
    coverlayout->addWidget(a);
    cover->setLayout(coverlayout);
    connect(m_coverbtn, &EditButton::changePic, this, [=]() {
        QString newFilePath = QFileDialog::getOpenFileName(this, tr("Select picture file"), QDir::homePath(), tr("Picture file (*.png)"));
        if (newFilePath.isEmpty()) {
            newFilePath = m_coverpath;
        }else{
            if(!FileCheck::isLegalWallPaperFile(newFilePath,"cover")){
                newFilePath = m_coverpath;
            }
            m_themeChangeMap->insert("cover",true);
            emit globalthemeChange(m_themeChangeMap);
        }
        m_coverpath = newFilePath;
        m_coverbtn->setIcon(QIcon(m_coverpath));
        m_coverbtn->setIconSize(QSize(165,106));
        emit newCoverFilePath(newFilePath);
    });
    connect(m_coverbtn, &EditButton::droprealcover, this, [=](const QString& path) {
        m_themeChangeMap->insert("cover",true);
        emit globalthemeChange(m_themeChangeMap);
        m_coverpath = path;
        m_coverbtn->setIcon(QIcon(m_coverpath));
        m_coverbtn->setIconSize(QSize(165,106));
        emit newCoverFilePath(path);
    });

    connect(m_coverbtn,&EditButton::deleteCustomPic,this,[=](){
        QPixmap pixmap(":/resource/background/custom-preview.png");
        m_coverbtn->setIcon(QIcon(pixmap));
        m_coverbtn->setIconSize(QSize(165,106));
        emit newCoverFilePath(":/resource/background/custom-preview.png");

        if(m_globalPresenceMap->value("cover")){
            m_themeChangeMap->insert("cover",true);
            m_globalPresenceMap->insert("cover",false);
            emit globalthemeChange(m_themeChangeMap);
        }else{
            m_themeChangeMap->insert("cover",false);
            emit globalthemeChange(m_themeChangeMap);
        }
    });

    connect(a,&addPicWidget::buttonclick,m_coverbtn,&EditButton::changePic);
    connect(a,&addPicWidget::dropRealPic,m_coverbtn, &EditButton::droprealcover);

    QLabel *tip = new QLabel(cover);
    tip->setText("<html>*尺寸为 1640 X 1080 的 PNG 图片，<br>*大小不超过 5 MB</html>");
    layout->addStretch(1);
    layout->addWidget(title);
    layout->addWidget(tip);
    layout->setSpacing(0);
    layout->addWidget(cover);
    m_coverwidget->setLayout(layout);

}

/**
 * @brief 初始化桌面壁纸小部件
 *
 * 初始化桌面壁纸小部件，并设置相关属性。
 */
void GlobalThemeWidget::initWallPaperWidget()
{
    m_wallpaperwidget = new QWidget(m_viewportwidget);
    m_wallpaperwidget->setFixedHeight(240);
    QLabel *title = new QLabel(m_wallpaperwidget);
    title->setText(tr("Desktop wallpaper"));
    QVBoxLayout *layout = new QVBoxLayout();
    QWidget *cover = new QWidget(m_wallpaperwidget);
    cover->setFixedHeight(160);
    QHBoxLayout *coverlayout = new QHBoxLayout(cover);
    wallpaperbtn = new EditButton(cover,"wallpaper");
    QLabel *tip = new QLabel(cover);
    tip->setText("<html>*尺寸为 3840 X 2160 的 JPG 图片，<br>*大小不超过 10 MB</html>");
    QPalette palette = tip->palette();
    palette.setColor(QPalette::WindowText, Qt::gray);
    tip->setPalette(palette);

    addPicWidget * a = new addPicWidget();
    a->setType("wallpaper");

    coverlayout->addWidget(wallpaperbtn);
    coverlayout->addStretch(1);
    coverlayout->addWidget(a);
    cover->setLayout(coverlayout);

    QPixmap pixmap(m_wallpaperpath);
    wallpaperbtn->setIcon(QIcon(pixmap));
    wallpaperbtn->setIconSize(QSize(178,122));

    connect(wallpaperbtn, &EditButton::changePic, this, [=]() {
        DataCollect::sendGlobalThemeKdkDataAsync(DataCollect::g_WallPaperWidget);
        QString newFilePath = QFileDialog::getOpenFileName(this, tr("Select picture file"), QDir::homePath(), tr("Picture file (*.jpg)"));

        if (!newFilePath.isEmpty()) {
            if(FileCheck::isLegalWallPaperFile(newFilePath,"wallpaper")){
                m_preview->updateWallpaper(newFilePath);
                emit wallpaperupdate(newFilePath);
                emit newWallpaperFilePath(newFilePath);

                QPixmap pixmap(newFilePath);
                wallpaperbtn->setIcon(QIcon(pixmap));
                wallpaperbtn->setIconSize(QSize(178,122));
                m_themeChangeMap->insert("wallpaper",true);
                emit globalthemeChange(m_themeChangeMap);
            }
        } else {
            qDebug() << "Selected file path:" << newFilePath;
        }
    });
    connect(wallpaperbtn, &EditButton::droprealwallpaper, this, [=](const QString& path) {
        if (!path.isEmpty()) {
            m_preview->updateWallpaper(path);
            emit wallpaperupdate(path);
            emit newWallpaperFilePath(path);

            QPixmap pixmap(path);
            wallpaperbtn->setIcon(QIcon(pixmap));
            wallpaperbtn->setIconSize(QSize(178,122));
            m_themeChangeMap->insert("wallpaper",true);
            emit globalthemeChange(m_themeChangeMap);

        } else {
            qDebug() << "Selected file path:" << path;
        }
    });
    connect(wallpaperbtn,&EditButton::deleteCustomPic,this,[=](){
        QString reswallpaper = ":/resource/background/1-openkylin.jpg";
        m_preview->updateWallpaper(":/resource/background/background-light.png");
        QPixmap pixmap(reswallpaper);
        wallpaperbtn->setIcon(QIcon(pixmap));
        wallpaperbtn->setIconSize(QSize(178,122));
        emit newWallpaperFilePath(reswallpaper);
        emit wallpaperupdate(reswallpaper);

        if(m_globalPresenceMap->value("wallpaper")){
            m_themeChangeMap->insert("wallpaper",true);
            m_globalPresenceMap->insert("wallpaper",false);
            emit globalthemeChange(m_themeChangeMap);
        }else{
            m_themeChangeMap->insert("wallpaper",false);
            emit globalthemeChange(m_themeChangeMap);
        }
    });
    connect(a,&addPicWidget::buttonclick,wallpaperbtn,&EditButton::changePic);
    connect(a,&addPicWidget::dropRealPic,wallpaperbtn, &EditButton::droprealwallpaper);
//    layout->addStretch(1);
    layout->addWidget(title);
    layout->addWidget(tip);
    layout->setSpacing(0);
    layout->addWidget(cover);
        layout->addStretch(1);
    m_wallpaperwidget->setLayout(layout);
}

/**
 * @brief 初始化外观小部件
 *
 * 创建一个新的 QWidget，并设置其固定高度为 65。
 * 创建一个 QLabel，设置其宽度与小部件相同，并设置文本为 "Window appearance"。
 * 创建一个 QHBoxLayout，并添加 QLabel 和 QComboBox 到布局中。
 * 设置 QComboBox 的项目，包括 "light" 和 "dark" 两个选项，分别对应白色和黑色。
 * 连接 QComboBox 的 currentIndexChanged 信号到槽函数，当选项改变时更新预览小部件的背景颜色和覆盖图像，并发送 newExterior 信号。
 */
void GlobalThemeWidget::initExteriorWidget()
{
    m_exteriorwidget = new QWidget(m_viewportwidget);
    m_exteriorwidget->setFixedHeight(65);
    QLabel *title = new QLabel(m_exteriorwidget);
//    title->setFixedWidth(m_exteriorwidget->width());
    title->setText(tr("Window appearance"));
    QHBoxLayout *layout = new QHBoxLayout();
    m_exteriorcombobox = new QComboBox(m_exteriorwidget);
    m_exteriorcombobox->setFixedSize(220,36);
    m_exteriorcombobox->addItem(tr("light"),QColor(255, 255, 255));
    m_exteriorcombobox->addItem(tr("dark"), QColor(29, 29, 29));
    connect(m_exteriorcombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
        DataCollect::sendGlobalThemeKdkDataAsync(DataCollect::g_ExteriorWidget);
        QColor selectedColor = m_exteriorcombobox->itemData(index).value<QColor>();
        QString overlayImage;

        if (index == 0) {
            overlayImage = ":/resource/background/panel-light.png";
        } else if (index == 1) {
            overlayImage = ":/resource/background/panel-dark.png";
        }

        m_preview->updateWidgetBackgroundColor(selectedColor);
        m_preview->updateOverlayImage(overlayImage);
        m_preview->updatescale();

        emit newExterior(selectedColor);

        if(m_info.widgetstyle == "light" && index == 0){
            m_themeChangeMap->insert("windowcolor",false);
            emit globalthemeChange(m_themeChangeMap);
        } else if (m_info.widgetstyle == "light" && index == 1){
            m_themeChangeMap->insert("windowcolor",true);
            emit globalthemeChange(m_themeChangeMap);
        } else if(m_info.widgetstyle == "dark" && index == 0) {
            m_themeChangeMap->insert("windowcolor",true);
            emit globalthemeChange(m_themeChangeMap);
        } else if (m_info.widgetstyle == "dark" && index == 1){
            m_themeChangeMap->insert("windowcolor",false);
            emit globalthemeChange(m_themeChangeMap);
        } else if (m_info.widgetstyle == "" && index == 1){
            m_themeChangeMap->insert("windowcolor",true);
            emit globalthemeChange(m_themeChangeMap);
        } else if (m_info.widgetstyle == "" && index == 0){
            m_themeChangeMap->insert("windowcolor",false);
            emit globalthemeChange(m_themeChangeMap);
        }
    });

    layout->addWidget(title);
    layout->addStretch(1);
    layout->addWidget(m_exteriorcombobox);

    m_exteriorwidget->setLayout(layout);
}

/**
 * @brief 初始化圆角控件小部件
 *
 * 初始化圆角控件小部件，并设置相关属性和布局。
 */
void GlobalThemeWidget::initFilletWidget()
{
    m_filletwidget = new QWidget(m_viewportwidget);
    m_filletwidget->setFixedHeight(65);
    QLabel *title = new QLabel(m_filletwidget);
    title->setFixedWidth(m_filletwidget->width());
    title->setText(tr("Control fillet"));
    QHBoxLayout *layout = new QHBoxLayout();
    m_filletslider = new QSlider(m_filletwidget);
    m_filletslider->setFixedSize(155,36);
    m_filletslider->setOrientation(Qt::Horizontal);
    m_filletslider->setRange(0,18);
    m_filletslider->setSingleStep(1);
    m_filletslider->setTickInterval(1);
    m_filletslider->setValue(6);
    m_preview->updateControlRadius(6);

    ShowLineEdit *lineedit = new ShowLineEdit(m_filletwidget);
    lineedit->setFixedSize(40,36);
    lineedit->setValidator(new QIntValidator(0,18,this));
    connect(lineedit,&QLineEdit::textChanged,[=](QString value){
        int intValue = value.toInt();

        if (intValue < 0) {
            intValue = 0;
        } else if (intValue > 18)
        {
            intValue = 18;
        } else {
            intValue = value.toInt();
        }
        m_filletslider->setValue(intValue);
    });
    connect(lineedit,&ShowLineEdit::editingFinished,[=](){
        DataCollect::sendGlobalThemeKdkDataAsync(DataCollect::g_FilletWidget);
        QString value = lineedit->text();
        if(value.toInt() == 0 && value.length() > 1){
            value = "0";
        }
        lineedit->setText(value);
        m_filletslider->setValue(value.toInt());
    });

    QLabel *showLabel = new QLabel(m_filletwidget);
    showLabel->setDisabled(true);
    showLabel->setFixedSize(26,36);
    int value = 0;
    value = m_filletslider->value();
    lineedit->setText(QString::number(value));
//    showLabel->setText(QString::number(m_filletslider->value())+"px");
    showLabel->setText("px");
    QObject::connect(m_filletslider, &QSlider::valueChanged, [=](int value) {
        lineedit->setText(QString::number(value));
//        showLabel->setText(QString::number(value)+"px");
        m_preview->updatescale();
        m_preview->updateControlRadius(value);

        emit newRadius(value);
        if (value == m_info.radius){
            m_themeChangeMap->insert("btnradius",false);
            emit globalthemeChange(m_themeChangeMap);
        } else {
            m_themeChangeMap->insert("btnradius",true);
            emit globalthemeChange(m_themeChangeMap);
         }

    });

    layout->addWidget(title);
    layout->addStretch(1);
    layout->addWidget(m_filletslider);
    layout->addWidget(lineedit);
    layout->setSpacing(0);
    layout->addWidget(showLabel);

    m_filletwidget->setLayout(layout);
}

/**
 * @brief 初始化强调色控件
 *
 * 初始化强调色控件，并设置相应的布局和功能。
 */
void GlobalThemeWidget::initAccentColorWidget()
{
    m_accentcolorwidget = new QWidget(m_viewportwidget);
    m_accentcolorwidget->setFixedHeight(65);
    QLabel *title = new QLabel(m_accentcolorwidget);
    title->setFixedWidth(m_accentcolorwidget->width());
    title->setText(tr("Accent colour"));
    QHBoxLayout *layout = new QHBoxLayout();
    m_combobox = new QComboBox(m_accentcolorwidget);
    m_combobox->setFixedSize(220,36);
    m_combobox->addItem(tr("daybreakBlue"),QColor(55, 144, 250));
    m_combobox->addItem(tr("jamPurple"), QColor(120, 115, 245));
    m_combobox->addItem(tr("magenta"), QColor(235, 48, 150));
    m_combobox->addItem(tr("sunRed"), QColor(243, 34, 45));
    m_combobox->addItem(tr("sunsetOrange"), QColor(246, 140, 39));
    m_combobox->addItem(tr("dustGold"), QColor(249, 197, 61));
    m_combobox->addItem(tr("polarGreen"), QColor(82, 196, 41));

    connect(m_combobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){

        DataCollect::sendGlobalThemeKdkDataAsync(DataCollect::g_AccentColorWidget);

        QColor selectedColor = m_combobox->itemData(index).value<QColor>();

//        qDebug() << "Selected color:" << selectedColor;

//        qDebug() << "RGB values:" << selectedColor.red() << selectedColor.green() << selectedColor.blue();
        m_preview->updatescale();
        m_preview->updateButtonColor(selectedColor);
        emit newAccentColor(selectedColor);
        if (m_info.accentcolor == nullptr){
            if(selectedColor == QColor(55, 144, 250)){
                m_themeChangeMap->insert("selectedColor",false);
                emit globalthemeChange(m_themeChangeMap);
            } else {
                m_themeChangeMap->insert("selectedColor",true);
                emit globalthemeChange(m_themeChangeMap);
            }
        } else {
            if(selectedColor == m_info.accentcolor){
                m_themeChangeMap->insert("selectedColor",false);
                emit globalthemeChange(m_themeChangeMap);
            } else {
                m_themeChangeMap->insert("selectedColor",true);
                emit globalthemeChange(m_themeChangeMap);
            }
        }
    });

    layout->addWidget(title);
    layout->addStretch(1);
    layout->addWidget(m_combobox);

    m_accentcolorwidget->setLayout(layout);
}

/**
 * @brief 初始化透明度控件
 *
 * 创建一个QWidget作为透明度控件，并设置其固定高度。
 * 创建一个水平布局，添加标题标签和透明度滑块。
 * 设置滑块的相关属性，包括方向、固定宽度、范围、单步长、刻度间隔和固定宽度，并设置初始值为65。
 * 创建一个显示按钮，并设置其大小和文本。
 * 连接滑块的值变化信号与更新透明度的槽函数。
 * 设置透明度控件的布局。
 */
void GlobalThemeWidget::initTransparencyWidget()
{
    m_transparencywidget = new QWidget(m_viewportwidget);
    m_transparencywidget->setFixedHeight(65);
    QHBoxLayout *layout = new QHBoxLayout();
    QLabel *title = new QLabel(tr("Window transparency"));
    m_transparencyslider = new QSlider(m_filletwidget);
    m_transparencyslider->setFixedSize(155,36);
    m_transparencyslider->setOrientation(Qt::Horizontal);
    m_transparencyslider->setRange(30,100);
    m_transparencyslider->setSingleStep(1);
    m_transparencyslider->setTickInterval(1);
    m_transparencyslider->setValue(65);

    ShowLineEdit1 *lineedit = new ShowLineEdit1(m_filletwidget);
    lineedit->setFixedSize(46,36);

    QLabel *showLabel = new QLabel(m_filletwidget);
    showLabel->setDisabled(true);
    showLabel->setFixedSize(20,36);

    int value = 0;
    value = m_transparencyslider->value();
    lineedit->setText(QString::number(value));
    lineedit->setValidator(new QIntValidator(30,100,this));
    connect(lineedit,&QLineEdit::textChanged,[=](QString value){

        int intValue = value.toInt();

        if (intValue < 30) {
            intValue = 30;
        } else if (intValue > 100)
        {
            intValue = 100;
        } else {
            intValue = value.toInt();
        }
        m_transparencyslider->setValue(intValue);
    });

//    showLabel->setText(QString::number(m_transparencyslider->value())+"%");
    showLabel->setText("%");
    QObject::connect(m_transparencyslider, &QSlider::valueChanged, [=](int value) {
        DataCollect::sendGlobalThemeKdkDataAsync(DataCollect::g_TransparencyWidget);
        lineedit->setText(QString::number(value));
        newTransparency(value);
        m_preview->updateTransparency(value);
        if (value == m_info.transparency){
            m_themeChangeMap->insert("windowtransparency",false);
            emit globalthemeChange(m_themeChangeMap);
        } else {
            m_themeChangeMap->insert("windowtransparency",true);
            emit globalthemeChange(m_themeChangeMap);
         }
    });

    layout->addWidget(title);
    layout->addWidget(m_transparencyslider);
    layout->addWidget(lineedit);
    layout->setSpacing(0);
    layout->addWidget(showLabel);

    m_transparencywidget->setLayout(layout);
}

/**
 * @brief 初始化窗户圆角控件
 *
 * 创建一个新的 QWidget，并将其设置为视口小部件的父小部件。
 * 设置透明度小部件的固定高度为 65。
 * 创建一个水平布局并添加标签和下拉框。
 * 下拉框包含 "Large"、"Medium" 和 "Small" 三项，分别对应 12、6 和 0 的整数值。
 * 将下拉框的当前选项设置为 "Medium"，并连接下拉框的 currentIndexChanged 信号。
 * 当选项改变时，更新预览小部件的缩放比例，并更新控件的半径。
 * 发出新窗口半径信号，其中包含选定的半径大小。
 */
void GlobalThemeWidget::initWindowFilletWidget()
{
    m_windowfilletwidget = new QWidget(m_viewportwidget);
    m_transparencywidget->setFixedHeight(65);
    QHBoxLayout *layout = new QHBoxLayout();
    QLabel *title = new QLabel(tr("Window fillet"));

    m_filletcombobox = new QComboBox(m_windowfilletwidget);
    m_filletcombobox->setFixedSize(220,36);
    m_filletcombobox->addItem(tr("Large"), int(12));
    m_filletcombobox->addItem(tr("Medium"), int(6));
    m_filletcombobox->addItem(tr("Small"), int(0));
    m_filletcombobox->setCurrentIndex(1);
    connect(m_filletcombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
        DataCollect::sendGlobalThemeKdkDataAsync(DataCollect::g_WindowFilletWidget);
        // 获取当前选项对应的圆角大小值
        int selectedSize = m_filletcombobox->itemData(index).value<int>();
        // 更新预览部件的缩放比例
        m_preview->updatescale();
        // 更新预览部件的圆角大小
        m_preview->updateWidgetRadius(selectedSize);
        // 发出新的窗口圆角大小信号
        emit newWindowRadius(selectedSize);
        if (selectedSize == m_info.windowradius){
            m_themeChangeMap->insert("windowradius",false);
            emit globalthemeChange(m_themeChangeMap);
        } else {
            m_themeChangeMap->insert("windowradius",true);
            emit globalthemeChange(m_themeChangeMap);
         }
    });
    layout->addWidget(title);
    layout->addWidget(m_filletcombobox);
    m_windowfilletwidget->setLayout(layout);
}

void GlobalThemeWidget::initbuildWidget()
{
    m_buildwidget = new QWidget();
    m_buildwidget->setFixedHeight(70);
    QPushButton *buildButton = new QPushButton();
    buildButton->setFixedSize(346,36);
    buildButton->setText(tr("Start Global Theme Building"));
    QHBoxLayout *buildlayout = new QHBoxLayout(m_buildwidget);
    buildlayout->addWidget(buildButton);
    connect(buildButton,&QPushButton::clicked,this,[=](){
        emit build();
    });
}

/**
 * @brief 添加间距项
 *
 * 在全局主题小部件中添加间距项。
 */
void GlobalThemeWidget::addspaceritem()
{
    QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    QSpacerItem *spacer2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    spacer->changeSize(m_previewwidget->width(), 155, QSizePolicy::Maximum, QSizePolicy::Maximum);
    spacer2->changeSize(m_previewwidget->width(), 159, QSizePolicy::Maximum, QSizePolicy::Maximum);

//    m_previewlayout->addItem(spacer);
    m_previewlayout->addWidget(m_preview);
//    m_previewlayout->addItem(spacer2);
}

void GlobalThemeWidget::setStyle(QString style)
{
    QColor widgetColor;
    if("light" == style){
        widgetColor = QColor(255,255,255);
    }else{
        widgetColor = QColor(29, 29, 29);
    }
    int index = m_exteriorcombobox->findData(widgetColor);
    if (index != -1) {
        m_exteriorcombobox->setCurrentIndex(index);
    }
}

/**
 * @brief 设置圆角设置
 *
 * 将指定的圆角值设置到控件中。
 *
 * @param radius 圆角值
 */
void GlobalThemeWidget::setRadiusSetting(int radius)
{
    m_filletslider->setValue(radius);
}

/**
 * @brief 设置强调色
 *
 * 设置全局主题小部件的强调色。
 *
 * @param accentcolor 强调色的字符串表示形式
 */
void GlobalThemeWidget::setAccentColor(QString accentcolor)
{
    QColor color;
    color.setNamedColor(accentcolor);

    int index = m_combobox->findData(color);
    if (index != -1) {
        m_combobox->setCurrentIndex(index);
    }
}

/**
 * @brief 设置透明度
 *
 * 设置全局主题小部件的透明度。
 *
 * @param transparency 透明度值
 */
void GlobalThemeWidget::setTransparency(int transparency)
{
    // 设置透明度滑块的当前值
    m_transparencyslider->setValue(transparency);
}

/**
 * @brief 设置封面
 *
 * 设置封面按钮的图标为封面路径对应的图标。
 *
 * @param coverpath 封面路径
 */
void GlobalThemeWidget::setCover(QString coverpath)
{
    m_coverbtn->setIcon(QIcon(coverpath));
}

/**
 * @brief 设置壁纸
 *
 * 设置壁纸按钮的图标为壁纸路径对应的图标。
 *
 * @param wallpaperpath 壁纸路径
 */
void GlobalThemeWidget::setWallpaper(QString wallpaperpath)
{
    wallpaperbtn->setIcon(QIcon(wallpaperpath));
}

/**
 * @brief 设置窗口半径
 *
 * 根据给定的窗口半径，设置控件的圆角程度。
 *
 * @param windowradius 窗口半径
 */
void GlobalThemeWidget::setWindowRadius(int windowradius)
{
    QMap<int, int> radiusIndexMap;
    radiusIndexMap.insert(0, 2);
    radiusIndexMap.insert(6, 1);
    radiusIndexMap.insert(12, 0);

    if (radiusIndexMap.contains(windowradius)) {
        int index = radiusIndexMap.value(windowradius);
        m_filletcombobox->setCurrentIndex(index);
    }
}

void GlobalThemeWidget::useMaximumScale()
{
    m_preview->maxscale();
}

void GlobalThemeWidget::useRestoreScale()
{
    m_preview->miniscale();
}
