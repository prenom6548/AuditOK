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
#include "cursorthemewidget.h"

bool CursorThemeWidget::g_themeChange;
CursorThemeWidget::CursorThemeWidget(QWidget *parent) : QWidget(parent)
{
    //是否改动存放记录map
    m_themeChangeMap = new QMap<QString, bool>();
    m_cursorPresenceMap = new QMap<QString, bool>();
    initMaps();
    initTimeMaps();
    initPreviewWidget();
    initRightWidget();

    QPushButton *line = new QPushButton();
    line->setDisabled(true);
    line->setFixedWidth(1);
    line->setFixedHeight(2000);
    m_cursorthemelayout = new QHBoxLayout();
    m_cursorthemelayout->addWidget(m_previewwidget);
    m_cursorthemelayout->addWidget(line);
    m_cursorthemelayout->addWidget(m_rightwidget);

    m_cursorthemelayout->setContentsMargins(0,0,0,0);
    m_cursorthemelayout->setSpacing(0);

    this->setLayout(m_cursorthemelayout);
}

QMap<QString, QString> *CursorThemeWidget::getCursorMap()
{
    return m_iconpathmap;
}

QMap<QString, QString> *CursorThemeWidget::getTimeCursorMap()
{
    return m_timeiconpathmap;
}

void CursorThemeWidget::eidtInitWidget(const HistoryInfo &InfoData)
{
    m_info = InfoData;
    setHistoryCursor("cursor");
    setHistoryCursor("timecursor");
    m_preview->updateIconMap(m_historyiconpathmap);
    m_preview2->updateIconMap(m_historytimeiconpathmap);
    updateCustomPathMap(m_historyiconpathmap,"cursor");
    updateCustomPathMap(m_historytimeiconpathmap,"timecursor");
    if(m_info.wallpaperpath != nullptr){
        m_preview->updateImage(m_info.wallpaperpath);
        m_preview2->updateImage(m_info.wallpaperpath);
    }else{
        m_preview->updateImage(":/resource/background/1-openkylin.jpg");
        m_preview2->updateImage(":/resource/background/1-openkylin.jpg");
    }

}

/**
 * @brief 初始化预览小部件
 *
 * 创建一个新的预览小部件，并对其进行初始化。
 */
void CursorThemeWidget::initPreviewWidget()
{
    m_previewwidget = new BasePreviewWidget(this);
    m_previewwidget->setMinimumSize(495,620);

    m_preview = new CursorImageWidget(m_previewwidget,m_iconpathmap);
    m_preview2 = new CursorImageWidget(m_previewwidget,m_timeiconpathmap);
    m_previewlayout = new QVBoxLayout(m_previewwidget);
    m_preview->setIconMap(m_iconpathmap);
    connect(this,&CursorThemeWidget::wallpaperupdate,  m_preview,[=](const QString& filePath){
        m_preview->updateImage(filePath);
        m_preview2->updateImage(filePath);
    });

    QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *spacer2 = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    spacer->changeSize(m_previewwidget->width(), 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    spacer2->changeSize(m_previewwidget->width(), 20, QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_previewlayout->addItem(spacer);
    m_previewlayout->addWidget(m_preview);
    m_previewlayout->addSpacing(0);
    m_previewlayout->addWidget(m_preview2);
    m_previewlayout->addItem(spacer2);
}

/**
 * @brief 从目录中的文件获取小部件名称列表
 *
 * 从指定的目录中获取所有文件的名称，并返回包含小部件名称的字符串列表。
 *
 * @param directoryPath 目录路径
 *
 * @return 小部件名称列表
 */
QStringList CursorThemeWidget::getWidgetNamesFromFilesInDirectory(const QString& directoryPath) {
    QStringList widgetNames;
    QDir dir(directoryPath);

    QStringList fileNames = dir.entryList(QDir::Files);
    foreach (const QString& fileName, fileNames) {
        QFileInfo fileInfo(fileName);
        QString widgetName = fileInfo.baseName();
        widgetNames.append(widgetName);
    }

    return widgetNames;
}

/**
 * @brief 刷新光标主题控件
 *
 * 刷新光标主题控件，更新预览控件的图标映射和自定义路径映射。
 */
void CursorThemeWidget::refresh()
{
    m_iconpathmap->clear();
    m_timeiconpathmap->clear();
    m_iconpathmap = getResourcesAppIconMap("cursor");
    m_timeiconpathmap = getResourcesAppIconMap("timecursor");
    emit newCursorMap(m_iconpathmap);
    emit newTimeCursorMap(m_timeiconpathmap);
    m_preview->updateIconMap(m_iconpathmap);
    updateCustomPathMap(m_iconpathmap,"cursor");
    m_preview2->updateIconMap(m_timeiconpathmap);
    updateCustomPathMap(m_timeiconpathmap,"timecursor");
    if(m_info.wallpaperpath != nullptr){
        m_preview->updateImage(m_info.wallpaperpath);
        m_preview2->updateImage(m_info.wallpaperpath);
    }else{
        m_preview->updateImage(":/resource/background/1-openkylin.jpg");
        m_preview2->updateImage(":/resource/background/1-openkylin.jpg");
    }

}

void CursorThemeWidget::useMaximumScale()
{
    m_preview->maxscale();
    m_preview2->maxscale();
}

void CursorThemeWidget::useRestoreScale()
{
    m_preview->miniscale();
    m_preview2->miniscale();
}

void CursorThemeWidget::setBuildBtnText(bool isglobal)
{
    if(isglobal){
        m_buildButton->setText(tr("Start Global Theme Building"));
    } else {
        m_buildButton->setText(tr("Start Cursor Theme Building"));
    }
}

/**
 * @brief 获取资源应用程序图标映射
 *
 * 根据指定的类型获取资源应用程序图标映射。
 *
 * @param Type 类型
 *
 * @return 返回 QMap<QString, QString> 指针，指向图标映射
 */
QMap<QString, QString> *CursorThemeWidget::getResourcesAppIconMap(QString Type)
{
    if("cursor" == Type){
        QMap<QString, QString> *iconpathmap = new QMap<QString, QString>;
        QString directoryPath = ":/resource/cursor/";
        QStringList widgetNames = getWidgetNamesFromFilesInDirectory(directoryPath);

        for (int i = 0; i < widgetNames.size(); ++i) {
            QString cursorFileName = directoryPath + widgetNames[i] + ".png";
            iconpathmap->insert(widgetNames[i], cursorFileName);
        }
        return iconpathmap;
    } else if ("timecursor" == Type){
        QMap<QString, QString> * timeiconpathmap = new QMap<QString, QString>;
        QString directoryPath = ":/resource/time-cursor/";
        QStringList widgetNames = getWidgetNamesFromFilesInDirectory(directoryPath);

        for (int i = 0; i < widgetNames.size(); ++i) {
            QString cursorFileName = directoryPath + widgetNames[i] + ".png";
            timeiconpathmap->insert(widgetNames[i], cursorFileName);
        }
        return timeiconpathmap;
    }

}

/**
 * @brief 初始化Map
 *
 * 初始化Map数据，包括获取光标相关资源和应用图标映射。
 */
void CursorThemeWidget::initMaps()
{
    m_iconpathmap = new QMap<QString, QString>;
    m_iconpathmap = getResourcesAppIconMap("cursor");
    m_customiconpathmap= m_iconpathmap;
    m_pathWidgetMap = new QMap<QString, CursorEditWidget*>;
}

/**
 * @brief 初始化时间映射
 *
 * 初始化时间映射，包括获取资源应用程序图标映射和自定义图标路径映射。
 */
void CursorThemeWidget::initTimeMaps()
{
    m_timeiconpathmap = new QMap<QString, QString>;
    m_timeiconpathmap = getResourcesAppIconMap("timecursor");
    m_timecustomiconpathmap= m_timeiconpathmap;
    m_timepathWidgetMap = new QMap<QString, CursorEditWidget*>;
}

/**
 * @brief 初始化右侧小部件
 */
void CursorThemeWidget::initRightWidget()
{
    m_rightwidget = new QWidget(this);
    m_rightwidget->setMinimumSize(400,620);
    m_rightwidget->setMaximumWidth(400);
    initBuildWidget();
    initFormatWidget();
    m_scrollArea = new QScrollArea(m_rightwidget);
    m_scrollArea->setFrameStyle(QFrame::HLine);
    m_scrollArea->setFixedWidth(m_rightwidget->width()-5);
//    m_scrollArea->setMaximumWidth(this->width());
    m_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_scrollArea->setWidgetResizable(true);
    QWidget *viewportWidget = new QWidget(m_scrollArea);
    m_scrollArea->setWidget(viewportWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(viewportWidget);
    viewportWidget->setLayout(mainLayout);

    // 创建有序的键值对列表
    QList<QPair<QString, QString>> sortedList1;
    for (auto it = m_iconpathmap->begin(); it != m_iconpathmap->end(); ++it) {
        sortedList1.append(qMakePair(it.key(), it.value()));
    }

    QList<QPair<QString, QString>> sortedList2;
    for (auto it = m_timeiconpathmap->begin(); it != m_timeiconpathmap->end(); ++it) {
        sortedList2.append(qMakePair(it.key(), it.value()));
    }

    std::sort(sortedList1.begin(), sortedList1.end(), [](const QPair<QString, QString>& pair1, const QPair<QString, QString>& pair2){
        // 按照指定顺序进行排序
        QStringList order = QStringList() << "left_ptr" << "dnd-none" << "hand1" << "hand2" << "right_ptr"
                                          << "dnd-ask"<< "dnd-copy" << "dnd-link"
                                          << "h_double_arrow" << "v_double_arrow" << "ul_angle" << "ur_angle"
                                          << "left_tee" << "bottom_tee"<< "top_tee" << "right_tee" << "cross" << "tcross"
                                          << "sb_up_arrow" << "sb_left_arrow" << "sb_down_arrow" << "sb_right_arrow" << "all-scroll"
                                          << "sb_h_double_arrow" << "sb_v_double_arrow" << "vertical_text" << "xterm"
                                          << "zoom-in" << "zoom-out"
                                          << "circle" << "color-picker" << "pencil" << "dotbox" << "X_cursor" << "plus";
        return order.indexOf(pair1.first) < order.indexOf(pair2.first);
    });
    std::sort(sortedList2.begin(), sortedList2.end(), [](const QPair<QString, QString>& pair1, const QPair<QString, QString>& pair2){

        QStringList order = QStringList()   << "left_ptr_watch_1" << "left_ptr_watch_2" << "left_ptr_watch_3"
                                            << "left_ptr_watch_4" << "left_ptr_watch_5"
                                            << "watch_1" << "watch_2" << "watch_3" << "watch_4" << "watch_5"
                                            << "watch_6" << "watch_7" << "watch_8" << "watch_9" << "watch_10";
        return order.indexOf(pair1.first) < order.indexOf(pair2.first);
    });

    // 遍历sortedList1中的每个元素，创建一个CursorEditWidget对象，并将其添加到主布局中
    for (const auto& pair : sortedList1) {
        const QString& widgetName = pair.first;
        const QString& filePath = pair.second;

        m_themeChangeMap->insert(widgetName,false);
        // 创建一个CursorEditWidget对象，并将其设置为viewportWidget的子控件
        CursorEditWidget* widget = new CursorEditWidget(viewportWidget);
        // 设置widget的默认图标为filePath指定的文件路径
        widget->setdefaulticon(filePath);
        widget->setProperty("widgetname",widgetName);
        QPushButton *line = new QPushButton();
        line->setDisabled(true);
        line->setFixedHeight(1);

        // 将widget添加到主布局中
        mainLayout->addWidget(widget);
        mainLayout->addWidget(line);
        // 将widget插入到m_pathWidgetMap中，以便后续可以通过widgetName找到对应的widget
        m_pathWidgetMap->insert(widgetName, widget);

        connect(widget->m_addiconbutton, &DropButton::clicked, this, [=]() {

            // 通过sender()获取发送信号的对象，将其强制转换为QPushButton指针类型
            DropButton* clickedButton = qobject_cast<DropButton*>(sender());
            // 如果获取的对象是合法的QPushButton对象
            if (clickedButton) {
                // 通过clickedButton的parentWidget()获取widget对象，并将其强制转换为CursorEditWidget指针类型
                CursorEditWidget* clickedWidget = qobject_cast<CursorEditWidget*>(clickedButton->parentWidget());
                // 如果获取的对象是合法的CursorEditWidget对象
                if (clickedWidget) {
                    // 弹出文件选择对话框，让用户选择一个SVG文件
                    QString newFilePath = QFileDialog::getOpenFileName(this, tr("Select SVG file"), QDir::homePath(), tr("SVG file (*.svg)"));
                    // 如果用户选择了文件，且文件路径不为空
                    if (!newFilePath.isEmpty()) {
                        if(FileCheck::isLegalCursorFile(newFilePath)){
                            // 将widgetName和newFilePath插入到m_customiconpathmap中，以便后续可以通过widgetName找到自定义图标路径
                            m_customiconpathmap->insert(widgetName, newFilePath);
                            // 调用m_preview的updateIcon函数，更新预览图标
                            m_preview->updateIcon(widgetName, newFilePath);
                            // 设置widget的自定义图标为newFilePath指定的文件路径
                            clickedWidget->setcustomicon(newFilePath);
                            // 发出newCursorMap信号，通知其他部分更新图标映射
                            emit newCursorMap(m_customiconpathmap);
                            //导入成功图标修改QMap状态true
                            m_themeChangeMap->insert(widgetName, true);
                            // 传递当前主题变化信号
                            emit cursorthemeChange(m_themeChangeMap);
                        }else{
                            qDebug()<<"file is not legal";
                        }
                    }
                }
            }
        });
        connect(widget->m_addiconbutton, &DropButton::droprealicon,this,[=](const QString& path){

            DropButton* clickedButton = qobject_cast<DropButton*>(sender());
            if (clickedButton) {
                CursorEditWidget* clickedWidget = qobject_cast<CursorEditWidget*>(clickedButton->parentWidget());
                if (clickedWidget) {
                    if (!path.isEmpty()) {
                        m_customiconpathmap->insert(widgetName, path);
                        m_preview->updateIcon(widgetName, path);
                        clickedWidget->setcustomicon(path);
                        emit newCursorMap(m_customiconpathmap);
                        m_themeChangeMap->insert(widgetName, true);
                        emit cursorthemeChange(m_themeChangeMap);
                    }
                }
            }
        });
        connect(widget,&CursorEditWidget::deleteCustomIcon,this,[=](){
            QString iconPath = ":/resource/cursor/"+widgetName+".png";
            m_preview->updateIcon(widgetName, iconPath);
            m_customiconpathmap->insert(widgetName, iconPath);
            emit newCursorMap(m_customiconpathmap);
            if (m_info.historytime == nullptr){
                m_themeChangeMap->insert(widgetName, false);
            }else{
                if(m_cursorPresenceMap->value(widgetName)){
                    m_themeChangeMap->insert(widgetName, true);
                    m_cursorPresenceMap->insert(widgetName, false);
                }else{
                    m_themeChangeMap->insert(widgetName, false);
                }
            }
            emit cursorthemeChange(m_themeChangeMap);
        });
    }

        for (const auto& pair : sortedList2) {
        const QString& widgetName = pair.first;
        const QString& filePath = pair.second;

        //导入成功图标修改QMap状态true
        m_themeChangeMap->insert(widgetName, false);
        // 传递当前主题变化信号
        CursorEditWidget* widget = new CursorEditWidget(viewportWidget);
        widget->setdefaulticon(filePath);
        QPushButton *line = new QPushButton();
        line->setDisabled(true);
        line->setFixedHeight(1);
        widget->setProperty("widgetname",widgetName);

        mainLayout->addWidget(widget);
        mainLayout->addWidget(line);
        m_timepathWidgetMap->insert(widgetName, widget);
        connect(widget->m_addiconbutton, &QPushButton::clicked, this, [=]() {

            // 获取发送信号的按钮对象
            QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
            // 判断获取的按钮对象是否为空，如果不为空，则继续执行下面的代码
            if (clickedButton) {
                // 通过按钮对象的parentWidget获取其父窗口对象，假设该父窗口对象为CursorEditWidget
                CursorEditWidget* clickedWidget = qobject_cast<CursorEditWidget*>(clickedButton->parentWidget());
                // 判断获取的CursorEditWidget对象是否为空，如果不为空，则继续执行下面的代码
                if (clickedWidget) {
                    // 使用QFileDialog::getOpenFileName函数打开一个文件选择对话框，选择一个SVG文件
                    QString newFilePath = QFileDialog::getOpenFileName(this, tr("Select SVG file"), QDir::homePath(), tr("SVG file (*.svg)"));
                    // 判断选择的文件路径是否为空，如果不为空，则执行下面的代码
                    if (!newFilePath.isEmpty()) {

                        if(FileCheck::isLegalCursorFile(newFilePath)){
                            // 将选择的文件路径插入到m_timecustomiconpathmap中，假设widgetName为键
                            m_timecustomiconpathmap->insert(widgetName, newFilePath);

                            // 更新m_preview2的对应图标
                            m_preview2->updateIcon(widgetName, newFilePath);

                            // 设置clickedWidget的自定义图标
                            clickedWidget->setcustomicon(newFilePath);

                            // 发出一个新的时间光标映射信号，假设m_timecustomiconpathmap为数据源
                            emit newTimeCursorMap(m_timecustomiconpathmap);

                            //导入成功图标修改QMap状态true
                            m_themeChangeMap->insert(widgetName, true);
                            // 传递当前主题变化信号
                            emit cursorthemeChange(m_themeChangeMap);
                        }
                    }
                }
            }
        });
        connect(widget->m_addiconbutton, &DropButton::droprealicon,this,[=](const QString& path){

            DropButton* clickedButton = qobject_cast<DropButton*>(sender());
            if (clickedButton) {
                CursorEditWidget* clickedWidget = qobject_cast<CursorEditWidget*>(clickedButton->parentWidget());
                if (clickedWidget) {
                    if (!path.isEmpty()) {
                        m_timecustomiconpathmap->insert(widgetName, path);
                        m_preview2->updateIcon(widgetName, path);
                        clickedWidget->setcustomicon(path);
                        emit newTimeCursorMap(m_timecustomiconpathmap);
                        m_themeChangeMap->insert(widgetName, true);
                        emit cursorthemeChange(m_themeChangeMap);
                    }
                }
            }
        });
        connect(widget,&CursorEditWidget::deleteCustomIcon,this,[=](){
            QString iconPath = ":/resource/time-cursor/"+widgetName+".png";
            m_preview2->updateIcon(widgetName, iconPath);
            m_timecustomiconpathmap->insert(widgetName, iconPath);
            emit newTimeCursorMap(m_timecustomiconpathmap);
            if (m_info.historytime == nullptr){
                m_themeChangeMap->insert(widgetName, false);
            }else{
                if(m_cursorPresenceMap->value(widgetName)){
                    m_themeChangeMap->insert(widgetName, true);
                    m_cursorPresenceMap->insert(widgetName, false);
                }else{
                    m_themeChangeMap->insert(widgetName, false);
                }
            }
            emit cursorthemeChange(m_themeChangeMap);
        });
    }

    QVBoxLayout *mainWidgetLayout = new QVBoxLayout();

    QPushButton *line = new QPushButton();
    line->setDisabled(true);
    line->setFixedHeight(1);
    line->setFixedWidth(m_rightwidget->width());

    QPushButton *line1 = new QPushButton();
    line1->setDisabled(true);
    line1->setFixedHeight(1);
    line1->setFixedWidth(m_rightwidget->width());
    mainWidgetLayout->addWidget(m_formatwidget);
    mainWidgetLayout->addWidget(line);
    mainWidgetLayout->addWidget(m_scrollArea);
    mainWidgetLayout->addWidget(line1);
    mainWidgetLayout->addWidget(m_buildwidget);

    m_rightwidget->setLayout(mainWidgetLayout);
}

void CursorThemeWidget::initBuildWidget()
{
    m_buildwidget = new QWidget();
    m_batchimportdlg = new BatchImportDialog();

    m_buildwidget->setFixedHeight(70);
    m_buildButton = new QPushButton(m_buildwidget);
    QPushButton *bacthButton = new QPushButton();
    bacthButton->setText(tr("Batch addition"));
//    buildButton->setFixedSize(346,36);
    m_buildButton->setText(tr("Start Cursor Theme Building"));
    QHBoxLayout *buildlayout = new QHBoxLayout(m_buildwidget);
    buildlayout->addWidget(bacthButton);
    buildlayout->addWidget(m_buildButton);

    connect(m_batchimportdlg,&BatchImportDialog::startimport,this,[=](){
        m_batchimportdlg->close();
        //批量导入不显示错误
        FileProcess::g_isBatchImport = true;
        // 打开文件对话框，选中文件夹，filecheck之后把对应文件update到相应位置上
        QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Batch"), QDir::homePath());
        QStringList fileList;
        QStringList fileList1;
        QList<CursorEditWidget*> childWidgets;
        if(m_info.historytime == nullptr){
            fileList = FileCheck::isLegalBatchIcon(folderPath, m_iconpathmap);
            fileList1 = FileCheck::isLegalBatchIcon(folderPath, m_timeiconpathmap);
        }else{
            fileList = FileCheck::isLegalBatchIcon(folderPath, m_historyiconpathmap);
            fileList1 = FileCheck::isLegalBatchIcon(folderPath, m_historytimeiconpathmap);
        }
        childWidgets = m_scrollArea->viewport()->findChildren<CursorEditWidget*>();

        for (const QString &filePath : fileList) {
            // 检查文件路径是否包含任何部件的名称
            for (CursorEditWidget *widget : childWidgets) {
                QString widgetName = widget->property("widgetname").toString();
                QString fileName = QFileInfo(filePath).baseName();
                if (fileName == widgetName) {
//                    qDebug() << "File Path: " << filePath << " contains Widget Name: " << widgetName;
                    if(FileCheck::isLegalCursorFile(filePath)){

                        if(m_customiconpathmap->contains(widgetName)){
                            m_customiconpathmap->insert(widgetName, filePath);
                            widget->setcustomicon(filePath);
                            m_preview->updateIcon(widgetName, filePath);
                            emit newCursorMap(m_customiconpathmap);
                            emit cursorthemeChange(m_themeChangeMap);
                        }
                    }
                    break;
                }
            }
        }
        for (const QString &filePath : fileList1) {
            // 检查文件路径是否包含任何部件的名称
            for (CursorEditWidget *widget : childWidgets) {
                QString fileName = QFileInfo(filePath).baseName();
                QString widgetName = widget->property("widgetname").toString();
                if (fileName == widgetName) {
//                    qDebug() << "File Path: " << filePath << " contains Widget Name: " << widgetName;
                    if(FileCheck::isLegalCursorFile(filePath)){

                        if(m_timecustomiconpathmap->contains(widgetName)){
                            m_timecustomiconpathmap->insert(widgetName, filePath);
                            m_preview2->updateIcon(widgetName, filePath);
                            widget->setcustomicon(filePath);
                            m_themeChangeMap->insert(widgetName, true);
                            emit newTimeCursorMap(m_timecustomiconpathmap);
                            emit cursorthemeChange(m_themeChangeMap);
                        }
                    }
                    break;
                }
            }
        }
        FileProcess::g_isBatchImport = false;
    });


    connect(bacthButton,&QPushButton::clicked,this,[=](){
        m_batchimportdlg->show();
    });
    connect(m_buildButton,&QPushButton::clicked,this,[=](){
        emit build();
    });    
}

void CursorThemeWidget::initFormatWidget()
{
    m_formatwidget = new QWidget();
    m_formatwidget->setFixedHeight(55);

    QHBoxLayout*layout = new QHBoxLayout(m_formatwidget);
    QLabel *tipslabel = new QLabel(m_formatwidget);
    tipslabel->setText(tr("*The uploaded ICONS are 96 X 96 in SVG format"));
    layout->addWidget(tipslabel);
}

/**
 * @brief 设置历史光标
 *
 * 根据给定的类型设置历史光标的主题。
 *
 * @param type 光标类型
 */
void CursorThemeWidget::setHistoryCursor(QString type)
{
    if("cursor" == type){
        QString directoryPath = m_info.filepath + "/src/cursorTheme/cursor/";
        QDir directory(directoryPath);
        QStringList filters;
        filters << "*.png" << "*.svg";
        QStringList fileList = directory.entryList(filters, QDir::Files);

        m_historyiconpathmap = new QMap<QString, QString>;

        foreach (const QString& fileName, fileList) {
            // 使用文件名构建文件路径
            QString filePath = directoryPath + fileName;
            // 获取文件名（不包括扩展名）
            QString name = fileName.left(fileName.lastIndexOf('.'));
            // 将文件名和文件路径插入到历史图标路径映射表中
            m_historyiconpathmap->insert(name, filePath);
        }
    } else if ("timecursor" == type){
        QString directoryPath = m_info.filepath + "/src/cursorTheme/timecursor/";
        QDir directory(directoryPath);
        QStringList filters;
        filters << "*.png" << "*.svg";
        QStringList fileList = directory.entryList(filters, QDir::Files);

        m_historytimeiconpathmap = new QMap<QString, QString>;

        foreach (const QString& fileName, fileList) {
            // 使用文件名构建文件路径
            QString filePath = directoryPath + fileName;  // 使用文件名构建文件路径
            // 获取文件名（不包括扩展名）
            QString name = fileName.left(fileName.lastIndexOf('.'));
            // 将文件名和文件路径插入到 map 中
            m_historytimeiconpathmap->insert(name, filePath);
        }
    }

}

/**
 * @brief 更新自定义路径映射
 *
 * 根据给定的历史映射和类型，更新自定义路径映射。
 *
 * @param historyMap 历史映射指针
 * @param type 类型
 */
void CursorThemeWidget::updateCustomPathMap(QMap<QString, QString> *historyMap, QString type)
{
    if("cursor" == type){
        m_customiconpathmap->clear();
        m_customiconpathmap = historyMap;
        for (auto it = m_pathWidgetMap->begin(); it != m_pathWidgetMap->end(); ++it) {
            const QString& widgetName = it.key();
            CursorEditWidget* widget = it.value();
            widget->setcustomicon(NULL);

            m_themeChangeMap->insert(widgetName, false);
            m_cursorPresenceMap->insert(widgetName, false);
            if (m_customiconpathmap->contains(widgetName)) {
                const QString& filePath = m_customiconpathmap->value(widgetName);

                // 判断文件类型为svg才显示
                QFileInfo fileInfo(filePath);
                if (fileInfo.suffix().toLower() == "svg") {
                    widget->setcustomicon(filePath);
                    m_preview->updateIcon(widgetName, filePath);
                    m_cursorPresenceMap->insert(widgetName, true);
                }
                else {
                    // 如果文件类型不是svg，直接跳过并不显示
                    continue;
                }
            }
        }
    } else if ("timecursor" == type){
        m_timecustomiconpathmap->clear();
        m_timecustomiconpathmap = historyMap;
        for (auto it = m_timepathWidgetMap->begin(); it != m_timepathWidgetMap->end(); ++it) {
            const QString& widgetName = it.key();
            CursorEditWidget* widget = it.value();
            widget->setcustomicon(NULL);

            m_themeChangeMap->insert(widgetName, false);
            m_cursorPresenceMap->insert(widgetName, false);
            if (m_timecustomiconpathmap->contains(widgetName)) {
                const QString& filePath = m_timecustomiconpathmap->value(widgetName);

                // 判断文件类型为svg才显示
                QFileInfo fileInfo(filePath);
                if (fileInfo.suffix().toLower() == "svg") {
                    widget->setcustomicon(filePath);
                    m_preview2->updateIcon(widgetName, filePath);
                    m_cursorPresenceMap->insert(widgetName, true);
                }
                else {
                    // 如果文件类型不是svg，直接跳过并不显示
                    continue;
                }
            }
        }
    }
}
