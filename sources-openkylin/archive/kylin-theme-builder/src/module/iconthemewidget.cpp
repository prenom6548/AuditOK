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
#include "iconthemewidget.h"


bool IconThemeWidget::g_themeChange;
/**
 * @brief 构造函数
 *
 * 用于创建 IconThemeWidget 实例的构造函数。
 *
 * @param isHistory 是否为历史图标主题
 * @param parent 父 QWidget
 */
IconThemeWidget::IconThemeWidget(bool isHistory, QWidget *parent) : QWidget(parent)
{
    //是否存在导入素材QMap
    m_themeChangeMap = new QMap<QString, bool>();
    //历史记录中图标是否存在QMap
    m_iconPresenceMap = new QMap<QString, bool>();
    initMaps();
    initSystemMaps();
    initPlacesMaps();
    initDevicesMaps();

    initPreviewWidget();
    initRightWidget();
    QPushButton *line = new QPushButton();
    line->setDisabled(true);
    line->setFixedWidth(1);
    line->setFixedHeight(2000);
    QHBoxLayout * m_iconthemelayout = new QHBoxLayout();
    m_iconthemelayout->addWidget(m_previewwidget);
    m_iconthemelayout->addWidget(line);
    m_iconthemelayout->addWidget(m_rightwidget);

    m_iconthemelayout->setContentsMargins(0,0,0,0);
    m_iconthemelayout->setSpacing(0);

    this->setLayout(m_iconthemelayout);
}

/**
 * @brief 初始化预览小部件
 *
 * 创建一个QWidget作为预览小部件，并设置其最小大小、背景颜色等属性。
 * 创建一个QStackedWidget作为图片预览的容器。
 * 创建两个ImageWidget分别用于显示自定义图标和系统图标，并更新图标路径映射。
 * 将两个ImageWidget添加到QStackedWidget中。
 * 将QStackedWidget添加到布局中。
 * 连接壁纸更新信号，当壁纸更新时，更新预览小部件的图片。
 */
void IconThemeWidget::initPreviewWidget()
{
    m_previewwidget = new BasePreviewWidget(this);
    m_previewwidget->setMinimumSize(550,620);

    m_previewstack = new QStackedWidget(m_previewwidget);

    m_preview = new ImageWidget(m_previewwidget,m_iconpathmap);
//    m_preview->updateIconMap(m_iconpathmap);
    m_systempreview = new ImageWidget(m_previewwidget,m_systemiconpathmap);

    m_placepreview = new ImageWidget(m_previewwidget,m_placesiconpathmap);

    m_previewstack->addWidget(m_preview);
    m_previewstack->addWidget(m_systempreview);
    m_previewstack->addWidget(m_placepreview);
    QVBoxLayout *layout = new QVBoxLayout(m_previewwidget);

    QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *spacer2 = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    spacer->changeSize(m_previewwidget->width(), 155, QSizePolicy::Fixed, QSizePolicy::Fixed);
    spacer2->changeSize(m_previewwidget->width(), 159, QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(this,&IconThemeWidget::wallpaperupdate,  m_preview,[=](const QString& filePath){
        m_preview->updateImage(filePath);
    });
//    layout->addItem(spacer);
    layout->addWidget(m_previewstack);
//    layout->addItem(spacer2);

    m_previewwidget->setLayout(layout);
}

/**
 * @brief 初始化编辑小部件
 *
 * 初始化编辑小部件，并创建相应的控件和布局。
 */
void IconThemeWidget::initEditWidget()
{

    m_editwidget = new QWidget(m_stackedwidget);
    m_scrollArea = new QScrollArea(m_rightwidget);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    m_scrollArea->setFrameStyle(QFrame::HLine);
    m_scrollArea->setMinimumSize(m_editwidget->size());
    m_scrollArea->setWidgetResizable(true);
    QWidget *viewportWidget = new QWidget(m_scrollArea);
    m_scrollArea->setWidget(viewportWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(viewportWidget);
    viewportWidget->setLayout(mainLayout);
    QMap<QString,QString>* appicon_name_CH = new QMap<QString,QString>;
    QList<QString> keys = m_iconpathmap->keys();
    appicon_name_CH = m_preview->initIconName(appicon_name_CH,keys);

    for (auto it = m_iconpathmap->begin(); it != m_iconpathmap->end(); ++it) {
        const QString& widgetName = it.key();
        const QString& filePath = it.value();
        // 遍历每个图标插入默认false未导入状态
        m_themeChangeMap->insert(widgetName, false);
        IconEditWidget* widget = new IconEditWidget(viewportWidget);
        widget->setdefaulticon(filePath);
        widget->setdefaulticonname(appicon_name_CH->value(it.key()));
        widget->setProperty("widgetname",widgetName);
        QPushButton *line = new QPushButton();
        line->setDisabled(true);
        line->setFixedHeight(1);

        mainLayout->addWidget(widget);
        mainLayout->addWidget(line);
        m_pathWidgetMap->insert(widgetName, widget);

        connect(widget->m_addiconbutton, &DropButton::clicked, this, [=]() {
            DropButton* clickedButton = qobject_cast<DropButton*>(sender());

            if (clickedButton) {
                IconEditWidget* clickedWidget = nullptr;
                QWidget *widget = clickedButton->parentWidget();
                while (widget && !widget->inherits("IconEditWidget")) {
                    widget = widget->parentWidget();
                }
                clickedWidget = qobject_cast<IconEditWidget*>(widget);

                if (clickedWidget) {
                    QString newFilePath = QFileDialog::getOpenFileName(this, tr("Select SVG file"), QDir::homePath(), tr("SVG file (*.svg)"));

                    if (!newFilePath.isEmpty()) {
                        if(FileCheck::isLegalIconFile(newFilePath)){
                            m_customiconpathmap->insert(widgetName, newFilePath);
                            clickedWidget->setcustomicon(newFilePath);
                            m_preview->updateIcon(widgetName, newFilePath);
                            //导入成功图标修改QMap状态true
                            m_themeChangeMap->insert(widgetName, true);
                            // 传递当前主题变化信号
                            emit iconthemeChange(m_themeChangeMap);
                        }
                    }
                    emit newAppIconsMap(m_customiconpathmap);
                }
            }
        });
        connect(widget->m_addiconbutton,&DropButton::droprealicon,this,[=](const QString& path){
            DropButton* clickedButton = qobject_cast<DropButton*>(sender());
            if (clickedButton) {
                IconEditWidget* clickedWidget = nullptr;
                QWidget *widget = clickedButton->parentWidget();
                while (widget && !widget->inherits("IconEditWidget")) {
                    widget = widget->parentWidget();
                }
                clickedWidget = qobject_cast<IconEditWidget*>(widget);
                if (clickedWidget) {
                    if (!path.isEmpty()) {
                        m_customiconpathmap->insert(widgetName, path);
                        clickedWidget->setcustomicon(path);
                        m_preview->updateIcon(widgetName, path);
                        m_themeChangeMap->insert(widgetName, true);
                        emit iconthemeChange(m_themeChangeMap);
                    }
                    emit newAppIconsMap(m_customiconpathmap);
                }
            }
        });
        connect(widget,&IconEditWidget::deleteCustomIcon,this,[=](){
            QString iconPath = ":/resource/appicons/"+widgetName+".png";
            m_preview->updateIcon(widgetName, iconPath);
            m_customiconpathmap->insert(widgetName, iconPath);
            emit newAppIconsMap(m_customiconpathmap);
            if (m_info.historytime == nullptr){
                m_themeChangeMap->insert(widgetName, false);
            }else{
                if(m_iconPresenceMap->value(widgetName)){
                    m_themeChangeMap->insert(widgetName, true);
                    m_iconPresenceMap->insert(widgetName, false);
                }else{
                    m_themeChangeMap->insert(widgetName, false);
                }
            }
            emit iconthemeChange(m_themeChangeMap);
        });
    }

    QVBoxLayout *mainWidgetLayout = new QVBoxLayout(m_editwidget);
    mainWidgetLayout->addWidget(m_scrollArea);
    m_editwidget->setLayout(mainWidgetLayout);
}

/**
 * @brief 更新自定义图标路径映射
 *
 * 清空现有的 m_customiconpathmap，将 historyMap 中的数据复制到 m_customiconpathmap，
 * 并更新相应的图标和预览。
 *
 * @param historyMap 自定义图标路径历史的 QMap 指针
 */
void IconThemeWidget::updateCustomIconPathMap(QMap<QString, QString>*historyMap)
{
    // 清空现有的 m_customiconpathmap
    m_customiconpathmap->clear();

    // 将 historyMap 中的数据复制到 m_customiconpathmap
    m_customiconpathmap = historyMap;

    // 更新相应的图标和预览
    for (auto it = m_pathWidgetMap->begin(); it != m_pathWidgetMap->end(); ++it) {
        const QString& widgetName = it.key();
        IconEditWidget* widget = it.value();

        // 遍历每个图标插入默认false未导入状态以及插入无图标内容false状态
        m_themeChangeMap->insert(widgetName, false);
        m_iconPresenceMap->insert(widgetName, false);
        widget->setcustomicon(NULL);
        // 检查 m_customiconpathmap 中是否存在当前的 widgetName
        if (m_customiconpathmap->contains(widgetName)) {
            const QString& filePath = m_customiconpathmap->value(widgetName);

            // 判断文件类型为svg才显示
            QFileInfo fileInfo(filePath);
            if (fileInfo.suffix().toLower() == "svg") {
                widget->setcustomicon(filePath);
                m_preview->updateIcon(widgetName, filePath);
                //插入图标内容存在状态true
                m_iconPresenceMap->insert(widgetName, true);
            } else {
                // 如果文件类型不是svg，直接跳过并不显示
                continue;
            }
        }
    }
}

/**
 * @brief 更新自定义系统图标路径映射
 *
 * 根据给定的历史图标路径映射，更新自定义系统图标路径映射。
 *
 * @param historyMap 历史图标路径映射指针
 */
void IconThemeWidget::updateCustomSyetemIconPathMap(QMap<QString, QString>*historyMap)
{
    m_systemcustomiconpathmap->clear();

    m_systemcustomiconpathmap = historyMap;

    for (auto it = m_systempathWidgetMap->begin(); it != m_systempathWidgetMap->end(); ++it) {
        const QString& widgetName = it.key();
        IconEditWidget* widget = it.value();
        m_themeChangeMap->insert(widgetName, false);
        m_iconPresenceMap->insert(widgetName, false);
        widget->setcustomicon(NULL);

        if (m_systemcustomiconpathmap->contains(widgetName)) {
            m_themeChangeMap->insert(widgetName, false);
            m_iconPresenceMap->insert(widgetName, false);
            const QString& filePath = m_systemcustomiconpathmap->value(widgetName);

            // 判断文件类型为svg才显示
            QFileInfo fileInfo(filePath);
            if (fileInfo.suffix().toLower() == "svg") {
                widget->setcustomicon(filePath);
                m_preview->updateIcon(widgetName, filePath);
                m_iconPresenceMap->insert(widgetName, true);
            }
            else {
                // 如果文件类型不是svg，直接跳过并不显示
                continue;
            }
        }
    }
}

void IconThemeWidget::updateCustomPlaceIconPathMap(QMap<QString, QString> *historyMap)
{
    m_placescustomiconpathmap->clear();

    m_placescustomiconpathmap = historyMap;

    for (auto it = m_placespathWidgetMap->begin(); it != m_placespathWidgetMap->end(); ++it) {
        const QString& widgetName = it.key();
        IconEditWidget* widget = it.value();
        m_themeChangeMap->insert(widgetName, false);
        m_iconPresenceMap->insert(widgetName, false);
        widget->setcustomicon(NULL);

        if (m_placescustomiconpathmap->contains(widgetName)) {
            m_themeChangeMap->insert(widgetName, false);
            m_iconPresenceMap->insert(widgetName, false);
            const QString& filePath = m_placescustomiconpathmap->value(widgetName);

            // 判断文件类型为svg才显示
            QFileInfo fileInfo(filePath);
            if (fileInfo.suffix().toLower() == "svg") {
                widget->setcustomicon(filePath);
                m_preview->updateIcon(widgetName, filePath);
                m_iconPresenceMap->insert(widgetName, true);
            }
            else {
                // 如果文件类型不是svg，直接跳过并不显示
                continue;
            }
        }
    }
}

void IconThemeWidget::updateCustomDeviceIconPathMap(QMap<QString, QString> *historyMap)
{
    m_devicescustomiconpathmap->clear();

    m_devicescustomiconpathmap = historyMap;

    for (auto it = m_devicespathWidgetMap->begin(); it != m_devicespathWidgetMap->end(); ++it) {
        const QString& widgetName = it.key();
        IconEditWidget* widget = it.value();
        m_themeChangeMap->insert(widgetName, false);
        m_iconPresenceMap->insert(widgetName, false);
        widget->setcustomicon(NULL);

        if (m_devicescustomiconpathmap->contains(widgetName)) {
            m_themeChangeMap->insert(widgetName, false);
            m_iconPresenceMap->insert(widgetName, false);
            const QString& filePath = m_devicescustomiconpathmap->value(widgetName);

            // 判断文件类型为svg才显示
            QFileInfo fileInfo(filePath);
            if (fileInfo.suffix().toLower() == "svg") {
                widget->setcustomicon(filePath);
                m_preview->updateIcon(widgetName, filePath);
                m_iconPresenceMap->insert(widgetName, true);
            }
            else {
                // 如果文件类型不是svg，直接跳过并不显示
                continue;
            }
        }
    }
}

/**
 * @brief 初始化系统编辑控件
 *
 * 初始化系统编辑控件，并创建相应的子控件。
 */
void IconThemeWidget::initSystemEditWidget()
{
    m_systemeditwidget = new QWidget(m_stackedwidget);
    m_systemscrollArea = new QScrollArea(m_rightwidget);
//    m_systemscrollArea->setFrameStyle(QFrame::HLine);
    m_systemscrollArea->setWidgetResizable(true);
    m_systemscrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_systemscrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QWidget *viewportWidget = new QWidget(m_systemscrollArea);
    m_systemscrollArea->setWidget(viewportWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(viewportWidget);
    viewportWidget->setLayout(mainLayout);

    QList<QPair<QString, QString>> sortedList1;
    for (auto it = m_systemiconpathmap->begin(); it != m_systemiconpathmap->end(); ++it) {
        sortedList1.append(qMakePair(it.key(), it.value()));
    }
    std::sort(sortedList1.begin(), sortedList1.end(), [](const QPair<QString, QString>& pair1, const QPair<QString, QString>& pair2){
        // 按照指定顺序进行排序
        QStringList order = QStringList() << "kylin-settings-account" << "kylin-settings-system" << "kylin-settings-devices" << "kylin-settings-network" << "kylin-settings-personalized"
                                          << "kylin-settings-datetime"<< "kylin-settings-update" << "kylin-settings-security"
                                          << "kylin-settings-application" << "kylin-settings-search" << "kylin-settings-commoninfo";
        return order.indexOf(pair1.first) < order.indexOf(pair2.first);
    });



    QMap<QString,QString>* appicon_name_CH = new QMap<QString,QString>;
    QList<QString> keys;
    keys << "kylin-settings-account" << "kylin-settings-system" << "kylin-settings-devices" << "kylin-settings-network" << "kylin-settings-personalized"
                          << "kylin-settings-datetime"<< "kylin-settings-update" << "kylin-settings-security"
                          << "kylin-settings-application" << "kylin-settings-search" << "kylin-settings-commoninfo";;
    appicon_name_CH = m_preview->initSystemIconName(appicon_name_CH,keys);
    for (const auto& pair : sortedList1) {
        qDebug()<<pair.first<<"-------------------------------------"<<pair.second;
        const QString& widgetName = pair.first;
        const QString& filePath = pair.second;
        IconEditWidget* widget = new IconEditWidget(viewportWidget);
        widget->setdefaulticon(filePath);
        widget->setdefaulticonname(appicon_name_CH->value(pair.first));
//        widget->setdefaulticonname(tr(widgetName));
        widget->setProperty("widgetname",widgetName);
        QPushButton *line = new QPushButton();
        line->setDisabled(true);
        line->setFixedHeight(1);

        mainLayout->addWidget(widget);
        mainLayout->addWidget(line);

        m_systempathWidgetMap->insert(widgetName, widget);

        // 当widget中的"添加图标"按钮被点击时，执行下面的函数。
        connect(widget->m_addiconbutton, &QPushButton::clicked, this, [=]() {

            // 获取发出信号的按钮，并将其转换为QPushButton*类型。
            QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
            // 检查转换是否成功。如果成功，继续执行下面的代码。
            if (clickedButton) {
                // 定义一个IconEditWidget类型的指针变量，并将其初始化为nullptr。
                IconEditWidget* clickedWidget = nullptr;
                // 获取点击的按钮的父窗口，并将其作为QWidget类型保存。
                QWidget *widget = clickedButton->parentWidget();
                
                // 循环遍历父窗口，直到找到一个窗口是IconEditWidget类型的窗口。
                while (widget && !widget->inherits("IconEditWidget")) {
                    widget = widget->parentWidget();
                }
                // 将QWidget类型转换为IconEditWidget类型，并将结果保存到clickedWidget变量中。
                clickedWidget = qobject_cast<IconEditWidget*>(widget);
                // 检查转换是否成功。如果成功，继续执行下面的代码。
                if (clickedWidget) {
                    
                    // 显示一个文件选择对话框，让用户选择一个SVG文件。将文件路径保存到newFilePath变量中。
                    QString newFilePath = QFileDialog::getOpenFileName(this, tr("Select SVG file"), QDir::homePath(), tr("SVG file (*.svg)"));
                    if (!newFilePath.isEmpty()) {
                        //check
                        if(FileCheck::isLegalIconFile(newFilePath)){
                            // 将新的文件路径插入到m_systemcustomiconpathmap中，键为widgetName。
                            m_systemcustomiconpathmap->insert(widgetName, newFilePath);
                            // 设置clickedWidget的自定义图标为新的文件路径。
                            clickedWidget->setcustomicon(newFilePath);
                            
                            // 输出选择的文件路径和对应的widgetName。
//                            qDebug() << "Selected file path:" << newFilePath<< "Corresponding widgetName:" << widgetName;
                            // 更新系统预览图标的显示。
                            m_systempreview->updateIcon(widgetName, newFilePath);
                        }
                    }
                    // 发出一个新的系统图标映射信号。
                    emit newSystemIconsMap(m_systemcustomiconpathmap);
                }
            }
        });
        connect(widget->m_addiconbutton,&DropButton::droprealicon,this,[=](const QString& path){
            DropButton* clickedButton = qobject_cast<DropButton*>(sender());
            if (clickedButton) {
                IconEditWidget* clickedWidget = nullptr;
                QWidget *widget = clickedButton->parentWidget();
                while (widget && !widget->inherits("IconEditWidget")) {
                    widget = widget->parentWidget();
                }
                clickedWidget = qobject_cast<IconEditWidget*>(widget);
                if (clickedWidget) {
                    if (!path.isEmpty()) {
                        m_systemcustomiconpathmap->insert(widgetName, path);
                        clickedWidget->setcustomicon(path);
                        m_systempreview->updateIcon(widgetName, path);
                    }
                    emit newSystemIconsMap(m_systemcustomiconpathmap);
                }
            }
        });
        connect(widget,&IconEditWidget::deleteCustomIcon,this,[=](){
            QString iconPath = ":/resource/systemicons/"+widgetName+".png";
            m_systempreview->updateIcon(widgetName, iconPath);
            m_systemcustomiconpathmap->insert(widgetName, iconPath);
            emit newSystemIconsMap(m_systemcustomiconpathmap);
        });
    }

    QVBoxLayout *mainWidgetLayout = new QVBoxLayout(m_systemeditwidget);
    mainWidgetLayout->addWidget(m_systemscrollArea);
    m_systemeditwidget->setLayout(mainWidgetLayout);
}

void IconThemeWidget::initPlaceandDeviceEditWidget()
{
    m_placeeditwidget = new QWidget(m_stackedwidget);
    m_placescrollArea = new QScrollArea(m_rightwidget);
    m_placescrollArea->setWidgetResizable(true);
    m_placescrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_placescrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QWidget *viewportWidget = new QWidget(m_placescrollArea);
    m_placescrollArea->setWidget(viewportWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(viewportWidget);
    viewportWidget->setLayout(mainLayout);
    QMap<QString,QString>* placeicon_name_CH = new QMap<QString,QString>;
    QList<QString> placekeys = m_placesiconpathmap->keys();
    placeicon_name_CH = m_preview->initPlaceIconName(placeicon_name_CH,placekeys);
    for (auto it = m_placesiconpathmap->begin(); it != m_placesiconpathmap->end(); ++it) {
        const QString& widgetName = it.key();
        const QString& filePath = it.value();
        IconEditWidget* widget = new IconEditWidget(viewportWidget);
        widget->setdefaulticon(filePath);
        widget->setdefaulticonname(placeicon_name_CH->value(it.key()));
        widget->setProperty("widgetname",widgetName);
        QPushButton *line = new QPushButton();
        line->setDisabled(true);
        line->setFixedHeight(1);

        mainLayout->addWidget(widget);
        mainLayout->addWidget(line);
        m_placespathWidgetMap->insert(widgetName, widget);

        connect(widget->m_addiconbutton, &QPushButton::clicked, this, [=]() {
            QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
            if (clickedButton) {
                IconEditWidget* clickedWidget = nullptr;
                QWidget *widget = clickedButton->parentWidget();
                while (widget && !widget->inherits("IconEditWidget")) {
                    widget = widget->parentWidget();
                }
                clickedWidget = qobject_cast<IconEditWidget*>(widget);
                if (clickedWidget) {

                    QString newFilePath = QFileDialog::getOpenFileName(this, tr("Select SVG file"), QDir::homePath(), tr("SVG file (*.svg)"));
                    if (!newFilePath.isEmpty()) {
                        //check
                        if(FileCheck::isLegalIconFile(newFilePath)){
                            m_placescustomiconpathmap->insert(widgetName, newFilePath);
                            clickedWidget->setcustomicon(newFilePath);

//                            qDebug() << "Selected file path:" << newFilePath<< "Corresponding widgetName:" << widgetName;
                            m_placepreview->updateIcon(widgetName, newFilePath);
                        }
                    }
                    emit newPlaceIconsMap(m_placescustomiconpathmap);
                }
            }
        });
        connect(widget->m_addiconbutton,&DropButton::droprealicon,this,[=](const QString& path){
            DropButton* clickedButton = qobject_cast<DropButton*>(sender());
            if (clickedButton) {
                IconEditWidget* clickedWidget = nullptr;
                QWidget *widget = clickedButton->parentWidget();
                while (widget && !widget->inherits("IconEditWidget")) {
                    widget = widget->parentWidget();
                }
                clickedWidget = qobject_cast<IconEditWidget*>(widget);
                if (clickedWidget) {
                    if (!path.isEmpty()) {
                        m_placescustomiconpathmap->insert(widgetName, path);
                        clickedWidget->setcustomicon(path);
                        m_placepreview->updateIcon(widgetName, path);
                    }
                    emit newPlaceIconsMap(m_placescustomiconpathmap);
                }
            }
        });
        connect(widget,&IconEditWidget::deleteCustomIcon,this,[=](){
            QString iconPath = ":/resource/places/"+widgetName+".png";
            m_placepreview->updateIcon(widgetName, iconPath);
            m_placescustomiconpathmap->insert(widgetName, iconPath);
            emit newPlaceIconsMap(m_placescustomiconpathmap);
        });
    }

    QMap<QString,QString>* deviceicon_name_CH = new QMap<QString,QString>;
    QList<QString> devicekeys = m_devicesiconpathmap->keys();
    deviceicon_name_CH = m_preview->initDeviceIconName(deviceicon_name_CH,devicekeys);

    for (auto it = m_devicesiconpathmap->begin(); it != m_devicesiconpathmap->end(); ++it) {
        const QString& widgetName = it.key();
        const QString& filePath = it.value();
        IconEditWidget* widget = new IconEditWidget(viewportWidget);
        widget->setdefaulticon(filePath);
        widget->setdefaulticonname(deviceicon_name_CH->value(it.key()));
        widget->setProperty("widgetname",widgetName);
        QPushButton *line = new QPushButton();
        line->setDisabled(true);
        line->setFixedHeight(1);

        mainLayout->addWidget(widget);
        mainLayout->addWidget(line);

        m_devicespathWidgetMap->insert(widgetName, widget);

        connect(widget->m_addiconbutton, &QPushButton::clicked, this, [=]() {

            QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
            if (clickedButton) {
                IconEditWidget* clickedWidget = nullptr;
                QWidget *widget = clickedButton->parentWidget();

                while (widget && !widget->inherits("IconEditWidget")) {
                    widget = widget->parentWidget();
                }
                clickedWidget = qobject_cast<IconEditWidget*>(widget);
                if (clickedWidget) {

                    QString newFilePath = QFileDialog::getOpenFileName(this, tr("Select SVG file"), QDir::homePath(), tr("SVG file (*.svg)"));
                    if (!newFilePath.isEmpty()) {
                        if(FileCheck::isLegalIconFile(newFilePath)){
                            m_devicescustomiconpathmap->insert(widgetName, newFilePath);
                            clickedWidget->setcustomicon(newFilePath);
//                            qDebug() << "Selected file path:" << newFilePath<< "Corresponding widgetName:" << widgetName;

//                            m_placepreview->updateIcon(widgetName, newFilePath);
                        }
                    }
                    emit newDeviceIconsMap(m_devicescustomiconpathmap);
                }
            }
        });
        connect(widget->m_addiconbutton,&DropButton::droprealicon,this,[=](const QString& path){
            DropButton* clickedButton = qobject_cast<DropButton*>(sender());
            if (clickedButton) {
                IconEditWidget* clickedWidget = nullptr;
                QWidget *widget = clickedButton->parentWidget();
                while (widget && !widget->inherits("IconEditWidget")) {
                    widget = widget->parentWidget();
                }
                clickedWidget = qobject_cast<IconEditWidget*>(widget);
                if (clickedWidget) {
                    if (!path.isEmpty()) {
                        m_devicescustomiconpathmap->insert(widgetName, path);
                        clickedWidget->setcustomicon(path);
                        m_placepreview->updateIcon(widgetName, path);
                    }
                    emit newDeviceIconsMap(m_devicescustomiconpathmap);
                }
            }
        });
        connect(widget,&IconEditWidget::deleteCustomIcon,this,[=](){
            QString iconPath = ":/resource/devices/"+widgetName+".png";
            m_placepreview->updateIcon(widgetName, iconPath);
            m_devicescustomiconpathmap->insert(widgetName, iconPath);
            emit newDeviceIconsMap(m_devicescustomiconpathmap);
        });
    }
    QVBoxLayout *mainWidgetLayout = new QVBoxLayout(m_placeeditwidget);
    mainWidgetLayout->addWidget(m_placescrollArea);
    m_placeeditwidget->setLayout(mainWidgetLayout);
}


void IconThemeWidget::initBatchEditWidget()
{
    m_batchwidget = new QWidget(m_rightwidget);

    QPushButton *bacthButton = new QPushButton(m_batchwidget);
    bacthButton->setText(tr("Batch addition"));
    m_buildButton = new QPushButton(m_batchwidget);
    m_buildButton->setText(tr("Start Icon Theme Building"));

    m_batchimportdlg = new BatchImportDialog();
    QHBoxLayout *layout = new QHBoxLayout(m_batchwidget);
    layout->addWidget(bacthButton);

    layout->addWidget(m_buildButton);

    m_batchwidget->setFixedHeight(60);

    connect(m_batchimportdlg,&BatchImportDialog::startimport,this,[=](){
        m_batchimportdlg->close();
        FileProcess::g_isBatchImport = true;

        // 打开文件对话框，选中文件夹，filecheck之后把对应文件update到相应位置上
        QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Batch"), QDir::homePath());
        QStringList fileList;
        QStringList fileList1;
        QList<IconEditWidget*> childWidgets;
        if(m_stackedwidget->currentIndex() == 0){
            if(m_info.historytime == nullptr){
                fileList = FileCheck::isLegalBatchIcon(folderPath, m_iconpathmap);
                childWidgets = m_scrollArea->viewport()->findChildren<IconEditWidget*>();
            }else{
                fileList = FileCheck::isLegalBatchIcon(folderPath, m_historyiconmap);
                childWidgets = m_scrollArea->viewport()->findChildren<IconEditWidget*>();
            }

        } else if (m_stackedwidget->currentIndex() == 1){
            if(m_info.historytime == nullptr){
                fileList = FileCheck::isLegalBatchIcon(folderPath, m_systemiconpathmap);
                childWidgets = m_systemscrollArea->viewport()->findChildren<IconEditWidget*>();
            }else{
                fileList = FileCheck::isLegalBatchIcon(folderPath, m_historysystemiconmap);
                childWidgets = m_systemscrollArea->viewport()->findChildren<IconEditWidget*>();
            }
        } else if(m_stackedwidget->currentIndex() == 2){
            if(m_info.historytime == nullptr){
                fileList = FileCheck::isLegalBatchIcon(folderPath, m_placesiconpathmap);
                fileList1 = FileCheck::isLegalBatchIcon(folderPath, m_devicesiconpathmap);
            }else{
                fileList = FileCheck::isLegalBatchIcon(folderPath, m_historyplacesiconmap);
                fileList1 = FileCheck::isLegalBatchIcon(folderPath, m_historydevicesiconmap);
            }

            childWidgets = m_placescrollArea->viewport()->findChildren<IconEditWidget*>();
        }

        for (const QString &filePath : fileList) {
            // 检查文件路径是否包含任何部件的名称
            for (IconEditWidget *widget : childWidgets) {
                QString fileName = QFileInfo(filePath).baseName();
                QString widgetName = widget->property("widgetname").toString();
                if (fileName == widgetName) {
//                    qDebug() << "File Path: " << filePath << " contains Widget Name: " << widgetName;
                    if(FileCheck::isLegalIconFile(filePath)){
                        if(m_stackedwidget->currentIndex() == 0){
                            m_customiconpathmap->insert(widgetName, filePath);
                            widget->setcustomicon(filePath);
                            m_preview->updateIcon(widgetName, filePath);
                            m_themeChangeMap->insert(widgetName, true);
                            emit iconthemeChange(m_themeChangeMap);
                            emit newAppIconsMap(m_customiconpathmap);
                        }else if(m_stackedwidget->currentIndex() == 1){
                            m_systemcustomiconpathmap->insert(widgetName, filePath);
                            widget->setcustomicon(filePath);
                            m_systempreview->updateIcon(widgetName, filePath);
                            m_themeChangeMap->insert(widgetName, true);
                            emit iconthemeChange(m_themeChangeMap);
                            emit newSystemIconsMap(m_systemcustomiconpathmap);
                        }else if(m_stackedwidget->currentIndex() == 2){
                            if(m_placescustomiconpathmap->contains(widgetName)){
                                m_placescustomiconpathmap->insert(widgetName, filePath);
                            }else if(m_devicescustomiconpathmap->contains(widgetName)){
                                m_devicescustomiconpathmap->insert(widgetName, filePath);
                            }
                            widget->setcustomicon(filePath);
                            m_placepreview->updateIcon(widgetName, filePath);
                            m_themeChangeMap->insert(widgetName, true);
                            emit iconthemeChange(m_themeChangeMap);
                            emit newPlaceIconsMap(m_placescustomiconpathmap);
                            emit newDeviceIconsMap(m_devicescustomiconpathmap);
                        }

                    }
                    break;
                }
            }
        }
        for (const QString &filePath : fileList1) {
            QString fileName = QFileInfo(filePath).baseName();
            for (IconEditWidget *widget : childWidgets) {
                QString widgetName = widget->property("widgetname").toString();
                if (fileName == widgetName) {
//                    qDebug() << "File Path: " << filePath << " contains Widget Name: " << widgetName;
                    if(FileCheck::isLegalIconFile(filePath)){
                        if(m_stackedwidget->currentIndex() == 2){
                            m_devicescustomiconpathmap->insert(widgetName, filePath);
                            widget->setcustomicon(filePath);
                            m_placepreview->updateIcon(widgetName, filePath);
                            emit newDeviceIconsMap(m_devicescustomiconpathmap);
                        }
                    }
                    break;
                }
            }
        }
        FileProcess::g_isBatchImport = false;
        //FIX m_systempreview sorterror
        if(m_info.historytime == nullptr){
            m_systempreview->updateIconMap(m_systemiconpathmap,"systemicon");
        }else{
            m_systempreview->updateIconMap(m_historysystemiconmap,"systemicon");
        }

    });
//    connect(bacthButton,&DropBatchButton::droprealbatchicon,this,[=](const QString& folderPath){
//        // 打开文件对话框，选中文件夹，filecheck之后把对应文件update到相应位置上
////        QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Batch"), QDir::homePath());
//        QStringList fileList;
//        QStringList fileList1;

//        QList<IconEditWidget*> childWidgets;
//        if(m_stackedwidget->currentIndex() == 0){
//            fileList = FileCheck::isLegalBatchIcon(folderPath, m_iconpathmap);
//            childWidgets = m_scrollArea->viewport()->findChildren<IconEditWidget*>();
//        } else if (m_stackedwidget->currentIndex() == 1){
//            fileList = FileCheck::isLegalBatchIcon(folderPath, m_systemiconpathmap);
//            childWidgets = m_systemscrollArea->viewport()->findChildren<IconEditWidget*>();
//        } else if(m_stackedwidget->currentIndex() == 2){
//            fileList = FileCheck::isLegalBatchIcon(folderPath, m_placesiconpathmap);
//            fileList1 = FileCheck::isLegalBatchIcon(folderPath, m_devicesiconpathmap);
//            childWidgets = m_placescrollArea->viewport()->findChildren<IconEditWidget*>();
//        }

//        for (const QString &filePath : fileList) {
//            QString fileName = QFileInfo(filePath).baseName();
//            // 检查文件路径是否包含任何部件的名称
//            for (IconEditWidget *widget : childWidgets) {
//                QString widgetName = widget->property("widgetname").toString();
//                if (fileName == widgetName) {
//                    qDebug() << "File Path: " << filePath << " contains Widget Name: " << widgetName;
//                    if(FileCheck::isLegalIconFile(filePath)){
//                        if(m_stackedwidget->currentIndex() == 0){
//                            m_customiconpathmap->insert(widgetName, filePath);
//                            widget->setcustomicon(filePath);
//                            m_preview->updateIcon(widgetName, filePath);
//                            m_themeChangeMap->insert(widgetName, true);
//                            emit iconthemeChange(m_themeChangeMap);
//                            emit newAppIconsMap(m_customiconpathmap);
//                        }else if(m_stackedwidget->currentIndex() == 1){
//                            m_systemcustomiconpathmap->insert(widgetName, filePath);
//                            widget->setcustomicon(filePath);
//                            m_systempreview->updateIcon(widgetName, filePath);
//                            emit newSystemIconsMap(m_systemcustomiconpathmap);
//                        }else if(m_stackedwidget->currentIndex() == 2){
//                            m_placescustomiconpathmap->insert(widgetName, filePath);
//                            m_devicescustomiconpathmap->insert(widgetName, filePath);
//                            widget->setcustomicon(filePath);
//                            m_placepreview->updateIcon(widgetName, filePath);
//                            emit newPlaceIconsMap(m_placescustomiconpathmap);
//                            emit newDeviceIconsMap(m_devicescustomiconpathmap);
//                        }
//                    }
//                    break;
//                }
//            }
//        }
//        for (const QString &filePath : fileList1) {
//            QString fileName = QFileInfo(filePath).baseName();
//            for (IconEditWidget *widget : childWidgets) {
//                QString widgetName = widget->property("widgetname").toString();
//                if (fileName == widgetName) {
//                    qDebug() << "File Path: " << filePath << " contains Widget Name: " << widgetName;
//                    if(FileCheck::isLegalIconFile(filePath)){
//                        if(m_stackedwidget->currentIndex() == 2){
//                            m_devicescustomiconpathmap->insert(widgetName, filePath);
//                            widget->setcustomicon(filePath);
//                            m_placepreview->updateIcon(widgetName, filePath);
//                            emit newDeviceIconsMap(m_devicescustomiconpathmap);
//                        }
//                    }
//                    break;
//                }
//            }
//        }
//    });
    connect(bacthButton,&QPushButton::clicked,this,[=](){
        m_batchimportdlg->show();
    });
    connect(m_buildButton,&QPushButton::clicked,this,[=](){
        emit build();
    });
}

/**
 * @brief 从目录中获取文件列表的widget名称
 *
 * 从指定的目录中获取文件列表，并返回包含文件名称的widget名称列表。
 *
 * @param directoryPath 目录路径
 *
 * @return widget名称列表
 */
QStringList IconThemeWidget::getWidgetNamesFromFilesInDirectory(const QString& directoryPath) {
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
 * @brief 初始化应用程序图标地图
 *
 * 初始化应用程序图标地图。
 */
void IconThemeWidget::initMaps()
{
    getResourcesAppIconMap("appicon");

    m_customiconpathmap = m_iconpathmap;
    m_pathWidgetMap = new QMap<QString, IconEditWidget*>;
}

/**
 * @brief 获取资源应用程序图标映射
 *
 * 根据给定的类型获取资源应用程序图标映射。
 *
 * @param Type 图标类型
 *
 * @return 返回 QMap<QString, QString> 指针，指向应用程序图标映射
 */
QMap<QString, QString> *IconThemeWidget::getResourcesAppIconMap(QString Type)
{
    if("appicon" == Type){
        m_iconpathmap = new QMap<QString, QString>;
        QString directoryPath = ":/resource/appicons/";
        QStringList widgetNames = getWidgetNamesFromFilesInDirectory(directoryPath);
        for (int i = 0; i < widgetNames.size(); ++i) {
            QString cursorFileName = directoryPath + widgetNames[i] + ".png";
            m_iconpathmap->insert(widgetNames[i], cursorFileName);
        }
        return m_iconpathmap;
    }else if("systemicon" == Type){
        m_systemiconpathmap = new QMap<QString, QString> ;
        QString directoryPath = ":/resource/systemicons/";
        QStringList widgetNames = getWidgetNamesFromFilesInDirectory(directoryPath);

        for (int i = 0; i < widgetNames.size(); ++i) {
            QString cursorFileName = directoryPath + widgetNames[i] + ".png";
            m_systemiconpathmap->insert(widgetNames[i], cursorFileName);
        }
        return m_systemiconpathmap;
    }else if("placesicon" == Type){
        m_placesiconpathmap = new QMap<QString, QString> ;
        QString directoryPath = ":/resource/places/";
        QStringList widgetNames = getWidgetNamesFromFilesInDirectory(directoryPath);

        for (int i = 0; i < widgetNames.size(); ++i) {
            QString cursorFileName = directoryPath + widgetNames[i] + ".png";
            m_placesiconpathmap->insert(widgetNames[i], cursorFileName);
        }
        return m_placesiconpathmap;
    }else if("devicesicon" == Type){
        m_devicesiconpathmap = new QMap<QString, QString> ;
        QString directoryPath = ":/resource/devices/";
        QStringList widgetNames = getWidgetNamesFromFilesInDirectory(directoryPath);

        for (int i = 0; i < widgetNames.size(); ++i) {
            QString cursorFileName = directoryPath + widgetNames[i] + ".png";
            m_devicesiconpathmap->insert(widgetNames[i], cursorFileName);
        }
        return m_devicesiconpathmap;
    }

}

/**
 * @brief 初始化系统图标主题
 *
 * 初始化系统图标主题，包括获取系统应用图标映射和系统自定义图标路径映射，并创建新的 QMap 对象用于存储图标编辑小部件。
 */
void IconThemeWidget::initSystemMaps()
{
    //解决历史记录三次之后第单数历史记录窗出现段错误
    getResourcesAppIconMap("systemicon");

    m_systemcustomiconpathmap= m_systemiconpathmap;
    m_systempathWidgetMap = new QMap<QString, IconEditWidget*>;
}

/**
 * @brief 初始化系统图标主题
 *
 * 初始化文件夹图标主题，包括获取系统应用图标映射和系统自定义图标路径映射，并创建新的 QMap 对象用于存储图标编辑小部件。
 */
void IconThemeWidget::initPlacesMaps()
{
    getResourcesAppIconMap("placesicon");

    m_placescustomiconpathmap = m_placesiconpathmap;
    m_placespathWidgetMap = new QMap<QString, IconEditWidget*>;
}

/**
 * @brief 初始化系统图标主题
 *
 * 初始化设备图标主题，包括获取系统应用图标映射和系统自定义图标路径映射，并创建新的 QMap 对象用于存储图标编辑小部件。
 */
void IconThemeWidget::initDevicesMaps()
{
    getResourcesAppIconMap("devicesicon");

    m_devicescustomiconpathmap = m_devicesiconpathmap;
    m_devicespathWidgetMap = new QMap<QString, IconEditWidget*>;
}

/**
 * @brief 初始化右侧小部件
 *
 * 初始化右侧的子小部件，包括更换图标按钮、编辑图标和系统图标编辑页面以及堆叠式控件。
 */
void IconThemeWidget::initRightWidget()
{
    m_rightwidget = new QWidget(this);
    m_rightwidget->setMinimumSize(400,620);
    m_rightwidget->setMaximumWidth(400);


    m_rightwidgetlayout = new QVBoxLayout(m_rightwidget);

    QWidget *m_changebuttonwidget = new QWidget(m_rightwidget);
    m_changebuttonwidget->setMinimumSize(400,70);
    QHBoxLayout *layout = new QHBoxLayout();

    m_tabbar = new kdk::KTabBar();
    m_tabbar->setTabBarStyle(kdk::KTabBarStyle::SegmentDark);
    m_tabbar->addTab(tr("AppIcons"));
    m_tabbar->addTab(tr("SystemIcons"));
    m_tabbar->addTab(tr("PlaceIcons"));

    layout->addWidget(m_tabbar);
    layout->setContentsMargins(0,0,15,0);
    m_changebuttonwidget->setLayout(layout);

    m_stackedwidget = new QStackedWidget(m_rightwidget);

    initEditWidget();
    initSystemEditWidget();
    initPlaceandDeviceEditWidget();

    initBatchEditWidget();
    m_stackedwidget->addWidget(m_editwidget);
    m_stackedwidget->addWidget(m_systemeditwidget);
    m_stackedwidget->addWidget(m_placeeditwidget);

    m_stackedwidget->setCurrentIndex(0);

    connect(m_tabbar, &QTabBar::tabBarClicked, [=](int tabIndex){
        m_stackedwidget->setCurrentIndex(tabIndex);
        m_previewstack->setCurrentIndex(tabIndex);
    });

    QPushButton *line = new QPushButton();
    line->setDisabled(true);
    line->setFixedHeight(1);
    line->setFixedWidth(m_rightwidget->width());

    QLabel *tip = new QLabel();
    tip->setText(tr("*The uploaded ICONS are 256 X 256 in SVG format"));

    m_rightwidgetlayout->addWidget(m_changebuttonwidget);
    m_rightwidgetlayout->addWidget(tip);
    m_rightwidgetlayout->addWidget(m_stackedwidget);
    m_rightwidgetlayout->addWidget(line);
    m_rightwidgetlayout->addWidget(m_batchwidget);

    m_rightwidget->setLayout(m_rightwidgetlayout);
}

/**
 * @brief 获取应用图标的 QMap 对象
 *
 * 返回一个 QMap 对象，该对象包含了应用图标的键值对。
 * 用于刷新
 * @return 应用图标的 QMap 对象指针
 */
QMap<QString, QString>* IconThemeWidget::getAppIconsMap()
{
    getResourcesAppIconMap("appicon");
    return m_iconpathmap;
}

/**
 * @brief 获取系统图标映射表
 *
 * 获取系统图标映射表，返回一个 QMap 对象，其中键为图标名称，值为图标路径。
 * 用于刷新
 * @return 系统图标映射表
 */
QMap<QString, QString> *IconThemeWidget::getSystemIconsMap()
{
    getResourcesAppIconMap("systemicon");
    return m_systemiconpathmap;
}

QMap<QString, QString> *IconThemeWidget::getPlaceIconsMap()
{
    getResourcesAppIconMap("placesicon");
    return m_placesiconpathmap;
}

QMap<QString, QString> *IconThemeWidget::getDeviceIconsMap()
{
    getResourcesAppIconMap("devicesicon");
    return m_devicesiconpathmap;
}

/**
 * @brief 设置应用图标的浏览历史记录
 *
 * 根据指定的目录路径，浏览并加载该目录下所有的应用图标文件，并将它们保存到浏览历史记录中。
 */
void IconThemeWidget::setHistoryAppIcon()
{
    QString directoryPath = m_info.filepath + "/src/iconTheme/appicon/";
//    QStringList widgetNames = getWidgetNamesFromFilesInDirectory(directoryPath);
    QDir directory(directoryPath);
    QStringList filters;
    filters << "*.png" << "*.svg";
    QStringList fileList = directory.entryList(filters, QDir::Files);

    m_historyiconmap = new QMap<QString, QString>;


    foreach (const QString& fileName, fileList) {
        QString filePath = directoryPath + fileName;  // 使用文件名构建文件路径
        QString name = fileName.left(fileName.lastIndexOf('.'));
        m_historyiconmap->insert(name, filePath);
    }
}

/**
 * @brief 设置历史系统图标
 *
 * 从指定路径加载系统图标，并存储在历史系统图标映射中。
 */
void IconThemeWidget::setHistorySystemIcon()
{
    QString directoryPath = m_info.filepath + "/src/iconTheme/systemicon/";
    QDir directory(directoryPath);
    QStringList filters;
    filters << "*.png" << "*.svg";
    QStringList fileList = directory.entryList(filters, QDir::Files);

    m_historysystemiconmap = new QMap<QString, QString>;


    foreach (const QString& fileName, fileList) {
        QString filePath = directoryPath + fileName;  // 使用文件名构建文件路径
        QString name = fileName.left(fileName.lastIndexOf('.'));
        m_historysystemiconmap->insert(name, filePath);
    }
}

void IconThemeWidget::setHistoryPlaceIcon()
{
    QString directoryPath = m_info.filepath + "/src/iconTheme/placeicon/";
    QDir directory(directoryPath);
    QStringList filters;
    filters << "*.png" << "*.svg";
    QStringList fileList = directory.entryList(filters, QDir::Files);

    m_historyplacesiconmap = new QMap<QString, QString>;


    foreach (const QString& fileName, fileList) {
        QString filePath = directoryPath + fileName;  // 使用文件名构建文件路径
        QString name = fileName.left(fileName.lastIndexOf('.'));
        m_historyplacesiconmap->insert(name, filePath);
    }
}

void IconThemeWidget::setHistoryDeviceIcon()
{
    QString directoryPath = m_info.filepath + "/src/iconTheme/deviceicon/";
    QDir directory(directoryPath);
    QStringList filters;
    filters << "*.png" << "*.svg";
    QStringList fileList = directory.entryList(filters, QDir::Files);

    m_historydevicesiconmap = new QMap<QString, QString>;


    foreach (const QString& fileName, fileList) {
        QString filePath = directoryPath + fileName;  // 使用文件名构建文件路径
        QString name = fileName.left(fileName.lastIndexOf('.'));
        m_historydevicesiconmap->insert(name, filePath);
    }
}

/**
 * @brief 初始化小部件
 *
 * 使用给定的历史信息数据初始化小部件。
 *
 * @param InfoData 历史信息数据对象
 */
void IconThemeWidget::eidtInitWidget(const HistoryInfo &InfoData)
{
    m_info = InfoData;

    setHistoryAppIcon();
    setHistorySystemIcon();
    setHistoryPlaceIcon();
    setHistoryDeviceIcon();

    m_preview->updateIconMap(m_historyiconmap,"appicon");
    m_systempreview->updateIconMap(m_historysystemiconmap,"systemicon");
    QMap<QString, QString> placesMap = *m_historyplacesiconmap;
    QMap<QString, QString> devicesMap = *m_historydevicesiconmap;
    QMap<QString, QString> mergedMap = placesMap.unite(devicesMap);
    m_placepreview->updateIconMap(&mergedMap,"placeicon");

    updateCustomIconPathMap(m_historyiconmap);
    updateCustomSyetemIconPathMap(m_historysystemiconmap);
    updateCustomPlaceIconPathMap(m_historyplacesiconmap);
    updateCustomDeviceIconPathMap(m_historydevicesiconmap);

    if(m_info.wallpaperpath != nullptr){
        m_preview->updateImage(m_info.wallpaperpath);
    }else{
        m_preview->updateImage(":/resource/background/1-openkylin.jpg");
    }

}

/**
 * @brief 刷新图标主题部件
 *
 * 更新图标主题部件的预览图标映射和自定义图标路径映射。
 */
void IconThemeWidget::refresh()
{
    m_preview->updateIconMap(getResourcesAppIconMap("appicon"),"appicon");
    updateCustomIconPathMap(m_iconpathmap);
    m_systempreview->updateIconMap(getResourcesAppIconMap("systemicon"),"systemicon");
    updateCustomSyetemIconPathMap(m_systemiconpathmap);

    getResourcesAppIconMap("placesicon");
    getResourcesAppIconMap("devicesicon");
    QMap<QString, QString> placesMap = *m_placesiconpathmap;
    QMap<QString, QString> devicesMap = *m_devicesiconpathmap;
    QMap<QString, QString> mergedMap = placesMap.unite(devicesMap);
    m_placepreview->updateIconMap(&mergedMap,"placeicon");
    updateCustomPlaceIconPathMap(m_placesiconpathmap);
    updateCustomDeviceIconPathMap(m_devicesiconpathmap);
//    m_placepreview->updateIconMap(getResourcesAppIconMap("placesicon"),"placeicon");
//    updateCustomPlaceIconPathMap(m_placesiconpathmap);
//    m_placepreview->updateIconMap(getResourcesAppIconMap("devicesicon"),"deviceicon");

}

void IconThemeWidget::useMaximumScale()
{
    m_preview->maxscale();
}

void IconThemeWidget::useRestoreScale()
{
    m_preview->miniscale();
}

void IconThemeWidget::setBuildBtnText(bool isglobal)
{
    if(isglobal){
        m_buildButton->setText(tr("Start Global Theme Building"));
    } else {
        m_buildButton->setText(tr("Start Icon Theme Building"));
    }

}
