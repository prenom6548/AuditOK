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
#include "grubthemewidget.h"
#include "../fileProcess/filecheck.h"
#include <QFileDialog>

bool GrubThemeWidget::g_themeChange;
/**
 * @brief GrubThemeWidget 构造函数
 *
 * 创建一个 GrubThemeWidget 对象。
 *
 * @param parent 父对象
 */
GrubThemeWidget::GrubThemeWidget(QWidget *parent) : QWidget(parent)
{
    initPreviewWidget();
    initRightWidget();
    QHBoxLayout *grubthemelayout = new QHBoxLayout();

    QPushButton *line = new QPushButton();
    line->setDisabled(true);
    line->setFixedWidth(1);
    line->setFixedHeight(2000);
    grubthemelayout->addWidget(m_previewwidget);
    grubthemelayout->addWidget(line);
    grubthemelayout->addWidget(m_rightwidget);

    grubthemelayout->setContentsMargins(0,0,0,0);
    grubthemelayout->setSpacing(0);

    this->setLayout(grubthemelayout);
}

/**
 * @brief 初始化预览小部件
 *
 * 创建一个新的QWidget作为预览小部件，并设置其最小大小。
 * 设置预览小部件的背景角色为QPalette::Window。
 * 设置预览小部件的背景颜色为"#F5F5F5"。
 * 设置预览小部件自动填充背景。
 * 创建一个新的GrubImageWidget作为预览图像小部件。
 * 创建一个新的QVBoxLayout作为预览小部件的布局。
 * 添加一个空项。
 * 设置预览小部件的布局。
 */
void GrubThemeWidget::initPreviewWidget()
{
    m_previewwidget = new BasePreviewWidget(this);
    m_previewwidget->setMinimumSize(495,620);

    m_preview = new GrubImageWidget(m_previewwidget);

    m_previewlayout = new QVBoxLayout(m_previewwidget);
    addspaceritem();
    m_previewwidget->setLayout(m_previewlayout);
}

/**
 * @brief 初始化右侧小部件
 *
 * 创建一个新的 QWidget，并将其设置为当前对象的成员变量 m_rightwidget。
 * 设置最小大小为 400x620，最大宽度为 400。
 * 设置背景角色为 QPalette::Base。
 * 调用 initEditWidget() 函数初始化编辑小部件。
 */
void GrubThemeWidget::initRightWidget()
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

    layout->addWidget(m_grubwidget);
    layout->addStretch(1);
    layout->addWidget(line);
    layout->addWidget(m_buildwidget);
}

/**
 * @brief 初始化编辑小部件
 *
 * 创建一个新的 QWidget，设置固定高度，并添加垂直布局。
 * 创建两个 QLabel 和一个 QPushButton，并将它们添加到布局中。
 * 创建 QHBoxLayout，将 QPushButton 和 QLabel 添加到布局中。
 * 连接 QPushButton 的 clicked 信号，当点击按钮时，打开文件对话框选择图片文件。
 * 如果选择的文件合法，则更新背景预览并设置 QPushButton 的图标。
 * 发出 newGrubFilePath 信号以更新新的 Grub 文件路径。
 */
void GrubThemeWidget::initEditWidget()
{
    m_grubwidget = new QWidget(m_rightwidget);
    m_grubwidget->setFixedHeight(250);
    QVBoxLayout *layout = new QVBoxLayout();

    QLabel *tipLabel = new QLabel("Grub背景");
    QWidget *grubWidget = new QWidget();
    QHBoxLayout *widgetLayout = new QHBoxLayout();

    m_showBtn = new GrubEditButton();
    QPixmap pixmap(resourceGrubPath);
    m_showBtn->setIcon(QIcon(pixmap));
    m_showBtn->setIconSize(QSize(178,122));
    QLabel *tipLabel1 = new QLabel();
    tipLabel1->setText("<html>*尺寸为 3840 X 2160 的 PNG 图片，<br>*大小不超过 10 MB</html>");

    grubAddPicWidget *a = new grubAddPicWidget();

    QHBoxLayout *importlayout = new QHBoxLayout();
        importlayout->addStretch(1);
    importlayout->addWidget(m_showBtn);
    importlayout->addStretch(1);
    importlayout->addWidget(a);
    importlayout->addStretch(1);

    connect(m_showBtn, &GrubEditButton::changePic, this, [=]() {
        // 弹出文件选择对话框，选择图片文件
        QString newFilePath = QFileDialog::getOpenFileName(this, tr("Select picture file"), QDir::homePath(), tr("Picture file (*.png)"));

        // 如果选择的文件路径不为空
        if (!newFilePath.isEmpty()) {
            // 判断选择的文件是否是合法的壁纸文件
            if(FileCheck::isLegalWallPaperFile(newFilePath,"grub")){
                // 更新背景预览
                m_preview->updateBackground(newFilePath);
                // 加载图片文件并创建一个QPixmap对象
                QPixmap pixmap(newFilePath);
                // 设置按钮的图标为加载的图片文件对应的图标，并设置图标大小为按钮的大小
                m_showBtn->setIcon(QIcon(pixmap));
                m_showBtn->setIconSize(QSize(178,122));
                // 发出新的Grub文件路径信号
                emit newGrubFilePath(newFilePath);
                g_themeChange = true;
            }
        } else {
            qDebug() << "Selected file path:" << newFilePath;
        }
    });
    connect(m_showBtn, &GrubEditButton::droprealgrub, this, [=](const QString& path) {

        if (!path.isEmpty()) {
            m_preview->updateBackground(path);
            QPixmap pixmap(path);
            m_showBtn->setIcon(QIcon(pixmap));
            m_showBtn->setIconSize(QSize(178,122));
            emit newGrubFilePath(path);
            g_themeChange = true;
        } else {
            qDebug() << "Selected file path:" << path;
        }
    });
    connect(m_showBtn,&GrubEditButton::deleteCustomPic,this,[=](){
        m_preview->updateBackground(resourceGrubPath);
        QPixmap pixmap(resourceGrubPath);
        m_showBtn->setIcon(QIcon(pixmap));
        m_showBtn->setIconSize(QSize(178,122));
        emit newGrubFilePath(resourceGrubPath);
        g_themeChange = false;
    });
    connect(a,&grubAddPicWidget::buttonclick,m_showBtn,&GrubEditButton::changePic);
    widgetLayout->addWidget(m_showBtn);
    widgetLayout->addWidget(tipLabel1);
    grubWidget->setLayout(widgetLayout);

    layout->addWidget(tipLabel);
    layout->addWidget(grubWidget);
    layout->addLayout(importlayout);

    m_grubwidget->setLayout(layout);
}


void GrubThemeWidget::initBuildWidget()
{
    m_buildwidget = new QWidget();
    m_buildwidget->setFixedHeight(70);
    m_buildButton = new QPushButton(m_buildwidget);
    m_buildButton->setFixedSize(346,36);
    m_buildButton->setText(tr("Start Grub Theme Building"));
    QHBoxLayout *buildlayout = new QHBoxLayout(m_buildwidget);
    buildlayout->addWidget(m_buildButton);
    connect(m_buildButton,&QPushButton::clicked,this,[=](){
        emit build();
    });
}

void GrubThemeWidget::eidtInitWidget(const HistoryInfo &InfoData)
{
    QString imagedirpath = InfoData.filepath+"/src/grubTheme/background.png";

    m_preview->updateBackground(imagedirpath);
    QPixmap pixmap(imagedirpath);
    m_showBtn->setIcon(QIcon(pixmap));
    m_showBtn->setIconSize(QSize(178,122));
}

void GrubThemeWidget::refresh()
{
    m_preview->updateBackground(resourceGrubPath);
    QPixmap pixmap(resourceGrubPath);
    m_showBtn->setIcon(QIcon(pixmap));
    m_showBtn->setIconSize(QSize(178,122));
}

void GrubThemeWidget::useMaximumScale()
{
    m_preview->maxscale();
}

void GrubThemeWidget::useRestoreScale()
{
    m_preview->miniscale();
}

void GrubThemeWidget::setBuildBtnText(bool isglobal)
{
    if(isglobal){
        m_buildButton->setText(tr("Start Global Theme Building"));
    } else {
        m_buildButton->setText(tr("Start Grub Theme Building"));
    }
}

void GrubThemeWidget::addspaceritem()
{
    QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    QSpacerItem *spacer2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    spacer->changeSize(m_previewwidget->width(), 155, QSizePolicy::Maximum, QSizePolicy::Maximum);
    spacer2->changeSize(m_previewwidget->width(), 159, QSizePolicy::Maximum, QSizePolicy::Maximum);

//    m_previewlayout->addItem(spacer);
    m_previewlayout->addWidget(m_preview);
//    m_previewlayout->addItem(spacer2);
}



