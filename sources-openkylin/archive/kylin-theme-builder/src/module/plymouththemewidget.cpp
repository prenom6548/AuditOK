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
#include "plymouththemewidget.h"
#include <QRadioButton>

bool PlymouthThemeWidget::g_themeChange;
/**
 * @brief PlymouthThemeWidget 构造函数
 *
 * 创建一个 PlymouthThemeWidget 对象，并设置其父对象。
 *
 * @param parent 父对象
 */
PlymouthThemeWidget::PlymouthThemeWidget(QWidget *parent) : QWidget(parent)
{
    initPreviewWidget();
    initRightWidget();
    QHBoxLayout *plymouththemelayout = new QHBoxLayout();

    QPushButton *line = new QPushButton();
    line->setDisabled(true);
    line->setFixedWidth(1);
    line->setFixedHeight(2000);
    plymouththemelayout->addWidget(m_previewwidget);
    plymouththemelayout->addWidget(line);
    plymouththemelayout->addWidget(m_rightwidget);


    plymouththemelayout->setContentsMargins(0,0,0,0);
    plymouththemelayout->setSpacing(0);

    this->setLayout(plymouththemelayout);
}

/**
 * @brief 初始化预览小部件
 *
 * 创建一个新的 QWidget 作为预览小部件，并设置其最小大小。
 * 设置预览小部件的背景角色为 QPalette::Window。
 * 设置预览小部件的调色板，并开启自动填充背景。
 * 创建一个新的 PlymouthImageWidget，并将其添加到预览小部件的垂直布局中。
 * 添加一个空隙项，并设置预览小部件的布局。
 */
void PlymouthThemeWidget::initPreviewWidget()
{
    m_previewwidget = new BasePreviewWidget(this);
    m_previewwidget->setMinimumSize(495,620);

    m_previewwidget->setBackgroundRole(QPalette::Window);

    m_preview = new PlymouthImageWidget(m_previewwidget);

    m_previewlayout = new QVBoxLayout(m_previewwidget);
    addspaceritem();
    m_previewwidget->setLayout(m_previewlayout);
}

/**
 * @brief 初始化右侧小部件
 *
 * 创建一个新的QWidget对象作为右侧小部件，并设置其最小和最大尺寸，以及背景色。
 * 之后初始化编辑小部件。
 */
void PlymouthThemeWidget::initRightWidget()
{
    m_rightwidget = new QWidget(this);
    m_rightwidget->setMinimumSize(400,620);
    m_rightwidget->setMaximumWidth(400);
    m_rightwidget->setBackgroundRole(QPalette::Base);
    m_rightwidget->setAutoFillBackground(true);

    QVBoxLayout *layout = new QVBoxLayout(m_rightwidget);
    layout->setContentsMargins(0,0,0,0);
    initEditWidget();
    initBuildWidget();

    QPushButton *line = new QPushButton();
    line->setDisabled(true);
    line->setFixedHeight(1);
    line->setFixedWidth(m_rightwidget->width());

    layout->addWidget(m_plymouthwidget);
    layout->addStretch(1);
    layout->addWidget(line);
    layout->addWidget(m_buildwidget);
}

/**
 * @brief 初始化编辑小部件
 *
 * 初始化编辑小部件，包括设置固定高度、创建布局、添加控件等。
 */
void PlymouthThemeWidget::initEditWidget()
{
    m_plymouthwidget = new QWidget(m_rightwidget);
    m_plymouthwidget->setFixedHeight(270);
    QVBoxLayout *widgetLayout = new QVBoxLayout(m_plymouthwidget);
    QWidget *checkBoxWidget = new QWidget();
    QHBoxLayout *checkBoxLayout = new QHBoxLayout(checkBoxWidget);
    pic = new QRadioButton();
    pixLabel = new QLabel("图片");
    video = new QRadioButton();
    videoLabel = new QLabel("视频");

    QButtonGroup *btngroup = new QButtonGroup();
    btngroup->addButton(pic);
    btngroup->addButton(video);
    btngroup->setExclusive(true);
    //默认导入视频
    video->setChecked(true);

    checkBoxLayout->addWidget(video);
    checkBoxLayout->addWidget(videoLabel);
    checkBoxLayout->addWidget(pic);
    checkBoxLayout->addWidget(pixLabel);

    checkBoxLayout->addStretch(1);
    QWidget *showWidget = new QWidget();
    QHBoxLayout *showLayout = new QHBoxLayout(showWidget);

    QLabel *defaultLabel = new QLabel();
    defaultLabel->setFixedSize(64,64);
    QPixmap pixmap(":/resource/background/openKylin.svg");
    pixmap = pixmap.scaled(64,64, Qt::KeepAspectRatio);
    defaultLabel->setPixmap(pixmap);
    defaultLabel->setVisible(false);

    m_customLabel= new PlymouthCustomLabel();
    DropPlyButton *addButton = new DropPlyButton();
    addButton->setIcon(QIcon::fromTheme("list-add-symbolic"));
    addButton->setFixedSize(36,36);
    addButton->setVisible(false);
    
    
    plymouthAddPicWidget * a = new plymouthAddPicWidget();

    connect(addButton, &DropPlyButton::droprealPly, this, [=](const QString& path) {
        if(pic->isChecked()){
            if (!path.isEmpty()) {
                if(FileCheck::isLegalPlymouthPicFile(path)){
                    QPixmap pixmap(path);
                    pixmap = pixmap.scaled(64,64, Qt::KeepAspectRatio);
                    m_customLabel->setPixmap(pixmap);
                    emit newPlymouthFilePath(path);

                    g_themeChange = true;
                } else{
                    qDebug()<<"plumouth icon is error";
                }
            }
        }
        else{
            if (!path.isEmpty()) {
                if(FileCheck::isLegalMP4File(path)){
                    QPixmap pixmap(path);
                    pixmap = pixmap.scaled(64,64, Qt::KeepAspectRatio);
                    m_customLabel->setPixmap(pixmap);
                    emit newPlymouthFilePath(path);

                    g_themeChange = true;
                } else{
                    qDebug()<<"plumouth icon is error";
                }
            }
        }

    });
    connect(addButton, &DropPlyButton::clicked, this, [=]() {
        if(pic->isChecked()){
            QString newFilePath = QFileDialog::getOpenFileName(this, tr("Select Png file"), QDir::homePath(), tr("Png file (*.png)"));
            if (!newFilePath.isEmpty()) {
                if(FileCheck::isLegalPlymouthPicFile(newFilePath)){
                    QPixmap pixmap(newFilePath);
                    pixmap = pixmap.scaled(64,64, Qt::KeepAspectRatio);
                    m_customLabel->setPixmap(pixmap);
                    emit newPlymouthFilePath(newFilePath);

                    g_themeChange = true;
                } else{
                    qDebug()<<"plumouth icon is error";
                }
            }
        }
        else{
            QString newFilePath = QFileDialog::getOpenFileName(this, tr("Select MP4 file"), QDir::homePath(), tr("MP4 file (*.mp4)"));
            if (!newFilePath.isEmpty()) {
                if(FileCheck::isLegalMP4File(newFilePath)){
                    QPixmap pixmap(newFilePath);
                    pixmap = pixmap.scaled(64,64, Qt::KeepAspectRatio);
                    m_customLabel->setPixmap(pixmap);
                    emit newPlymouthFilePath(newFilePath);

                    g_themeChange = true;
                } else{
                    qDebug()<<"plumouth icon is error";
                }
            }
        }

    });
    connect(m_customLabel,&PlymouthCustomLabel::deleteCustomIcon,this,[=](){
        m_preview->updatePlymouth(":/resource/plymouth/");
        m_preview->showPlymouth();

        emit refreshPlymouthDir();
        g_themeChange = false;

    });
    QLabel *tipLabel = new QLabel();
    tipLabel->setText("<html>*视频为 MP4 格式<br>*大小不超过 10 MB</html>");
    connect(btngroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton* button) {
        if (button == pic) {
            tipLabel->setText("<html>*尺寸为 400*400 的 PNG 图片<br>*大小不超过 20 KB</html>");
            a->setType("pic");
        } else {
            tipLabel->setText("<html>*视频为 MP4 格式<br>*大小不超过 10 MB</html>");
            a->setType("video");
        }
    });

    m_customLabel->setFixedSize(108,108);

    showLayout->addWidget(defaultLabel);
    showLayout->addWidget(m_customLabel);
    showLayout->addWidget(addButton);
//    showLayout->addStretch(1);
//    showLayout->addWidget(tipLabel);
    showLayout->addWidget(a);

    showWidget->setLayout(showLayout);

    widgetLayout->addWidget(checkBoxWidget);
    widgetLayout->addWidget(tipLabel);
    widgetLayout->addWidget(showWidget);
    m_plymouthwidget->setLayout(widgetLayout);

    connect(a,&plymouthAddPicWidget::buttonclick,addButton,&DropPlyButton::click);
    connect(a,&plymouthAddPicWidget::dropRealPic,addButton,&DropPlyButton::droprealPly);
}

void PlymouthThemeWidget::initBuildWidget()
{
    m_buildwidget = new QWidget();
    m_buildwidget->setFixedHeight(70);
    m_buildButton = new QPushButton(m_buildwidget);
    m_buildButton->setFixedSize(346,36);
    m_buildButton->setText(tr("Start Plymouth Theme Building"));
    QHBoxLayout *buildlayout = new QHBoxLayout(m_buildwidget);
    buildlayout->addWidget(m_buildButton);
    connect(m_buildButton,&QPushButton::clicked,this,[=](){
        emit build();
    });
}

void PlymouthThemeWidget::setThumbnailIcon(QString path)
{
    QPixmap pixmap(path);
    pixmap = pixmap.scaled(64,64, Qt::KeepAspectRatio);
    m_customLabel->setPixmap(pixmap);

    QFileInfo fileInfo(path);
    QString folderName = fileInfo.absolutePath();
    m_imagedirpath = folderName;
    m_preview->updatePlymouth(m_imagedirpath);
}

void PlymouthThemeWidget::startShowPlymouth()
{
    m_preview->showPlymouth();
}

void PlymouthThemeWidget::eidtInitWidget(const HistoryInfo &InfoData)
{
    QString imagedirpath = InfoData.filepath+"/src/plymouthTheme/";

    m_preview->updatePlymouth(imagedirpath);
    m_preview->showPlymouth();

    //加载图像文件，判断是否进行修改
    QImage image1(InfoData.filepath+"/src/plymouthTheme/1.png");
    QImage image2(":/resource/plymouth/1.png");

    if (image1 == image2) {
        m_customLabel->setPixmap(QPixmap());
    } else {
    QPixmap pixmap(InfoData.filepath+"/src/plymouthTheme/40.png");
    pixmap = pixmap.scaled(64,64, Qt::KeepAspectRatio);
    m_customLabel->setPixmap(pixmap);
    }
}

void PlymouthThemeWidget::refresh()
{
    m_preview->updatePlymouth(":/resource/plymouth/");
    m_preview->showPlymouth();
    m_customLabel->setPixmap(QPixmap());
}

void PlymouthThemeWidget::useMaximumScale()
{
    m_preview->maxscale();
}

void PlymouthThemeWidget::useRestoreScale()
{
    m_preview->miniscale();
}

void PlymouthThemeWidget::setBuildBtnText(bool isglobal)
{
    if(isglobal){
        m_buildButton->setText(tr("Start Global Theme Building"));
    } else {
        m_buildButton->setText(tr("Start Plymouth Theme Building"));
    }
}

void PlymouthThemeWidget::addspaceritem()
{
    QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    QSpacerItem *spacer2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    spacer->changeSize(m_previewwidget->width(), 155, QSizePolicy::Maximum, QSizePolicy::Maximum);
    spacer2->changeSize(m_previewwidget->width(), 159, QSizePolicy::Maximum, QSizePolicy::Maximum);

//    m_previewlayout->addItem(spacer);
    m_previewlayout->addWidget(m_preview);
//    m_previewlayout->addItem(spacer2);
}

