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
#include "iconwidgetfeature.h"

MainInterFaceFeature::MainInterFaceFeature(QWidget *parent) : QWidget(parent)
{

}
TypeButton::TypeButton(QWidget *parent): QPushButton(parent)
{
    this->setFixedSize(184,36);
   setCheckable(true);
}

/**
 * @brief 绘制事件
 *
 * 当按钮被绘制时调用此函数。
 *
 * @param event 绘制事件指针
 */
void TypeButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

//    if (isChecked()) {
//        QPainter painter(this);
//        QIcon icon = QIcon::fromTheme("");
//        QPixmap pixmap = icon.pixmap(QSize(16, 16));
//        QRect pixmapRect(rect().right() - pixmap.width(), (rect().height() - pixmap.height()) / 2, pixmap.width(), pixmap.height());
//        painter.drawPixmap(pixmapRect, pixmap);
//    }
}

/**
 * @brief 构造函数
 *
 * 创建一个 QGraphicsView 实例，并设置相关参数。
 *
 * @param scene QGraphicsScene 对象指针，用于显示在视图中的场景
 */
IconGraphicsView::IconGraphicsView(QGraphicsScene* scene) : QGraphicsView(scene)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setRenderHint(QPainter::Antialiasing, true);
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing |QGraphicsView::DontSavePainterState);

    this->setStyleSheet("background-color: transparent;");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

/**
 * @brief 滚轮事件
 *
 * 当发生滚轮事件时，该函数将被调用。如果按下控制键，则通过缩放视图来处理滚轮事件；否则，将滚轮事件传递给基类 QGraphicsView 处理。
 *
 * @param event 滚轮事件指针
 */
void IconGraphicsView::wheelEvent(QWheelEvent* event)
{
//    if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier) {
//        qreal scaleFactor = std::pow(qreal(2), event->delta() / 240.0);
//        scale(scaleFactor, scaleFactor);
//    } else {
//        QGraphicsView::wheelEvent(event);
//    }

    if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier) {
        qreal scaleFactor = std::pow(qreal(2), event->delta() / 240.0);
        qreal currentScale = transform().m11(); // 获取当前的缩放倍数

        // 限制最大为1.1倍
        qreal targetScale = currentScale * scaleFactor;
        if (targetScale > 1.1) {
            scaleFactor = 1.1 / currentScale;
        }else if (targetScale < 0.3) {
            scaleFactor = 0.3 / currentScale;
        }

        scale(scaleFactor, scaleFactor);
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

/**
 * @brief ImageWidget构造函数
 *
 * ImageWidget的构造函数，用于初始化ImageWidget对象。
 *
 * @param parent QWidget对象指针，指向ImageWidget的父窗口
 * @param iconMap QMap<QString, QString>对象指针，用于存储图标与路径的对应关系
 */
ImageWidget::ImageWidget(QWidget *parent, const QMap<QString, QString>* iconMap)
    : QWidget(parent), m_iconMap(iconMap)
{

    QGraphicsScene* scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(Qt::transparent);
    graphicsView = new IconGraphicsView(scene);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(graphicsView);
    setLayout(layout);


     // 获取图标映射表的大小
    int count = m_iconMap->size();

    // 如果图标映射表的大小等于11，加载特定的背景图片并添加到场景中
    if(count == 11){
        image = new QPixmap(":/resource/background/controlcenter-light.png");
        if (image->isNull()) {
            qDebug() << "Failed to load image.";
        }
    }else{
        image = new QPixmap(":/resource/background/background-light.png");
        if (image->isNull()) {
            qDebug() << "Failed to load image.";
        }
    }
    // 将场景添加到视图，并设置视图显示的场景
    scene->addPixmap(*image);
    graphicsView->setScene(scene);
    // 根据视图的大小调整场景的大小，保持宽高比不变
    graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    // 显示视图
    graphicsView->show();

//    // 如果图标映射表的大小等于11，则进行特定的图片加载和添加操作
//    if (count == 11) {
//        // 获取图标映射表的所有键值对
//        QList<QString> keys = m_iconMap->keys();
//        // 根据特定的顺序创建键值的数组
//        QString positions[11] = { keys[0], keys[9], keys[4], keys[5], keys[6],
//                                  keys[3], keys[10], keys[8], keys[1], keys[7], keys[2] };
//        // 定义图片的位置数组
//        int posX[3] = { 208, 879, 1550 };
//        int posY[4] = { 275, 535, 795, 1055 };
//        int index = 0;
//         QMap<QString,QString>* systemicon_name_CH = new QMap<QString,QString>;
//        systemicon_name_CH = initSystemIconName(systemicon_name_CH,keys);
//        // 遍历位置数组，加载并添加图片到场景中
//        for (int i = 0; i < 4; ++i) {
//            for (int j = 0; j < 3; ++j) {
//                // 检查索引是否超出范围
//                if (index >= 11) {
//                    break;  // 处理超出索引范围的情况
//                }
//                // 根据键值获取图片路径，并加载图片
//                QString pixmapPath = m_iconMap->value(positions[index]);
//                QPixmap pixmap(pixmapPath);
//                QGraphicsPixmapItem* item = scene->addPixmap(pixmap.scaled(100, 100));
//                // 设置图形的可选择和可聚焦属性，以触发选择和焦点事件
//                item->setFlag(QGraphicsItem::ItemIsSelectable); // 启用选择事件
//                item->setFlag(QGraphicsItem::ItemIsFocusable);  // 启用焦点事件
//                // 将键值和图片路径保存到图形的数据中，以便后续使用
//                item->setData(0, keys[index]);
//                item->setData(1, pixmapPath);
//                // 设置图形的位置
//                item->setPos(posX[j], posY[i]);

//                // 添加文字项
//                QGraphicsTextItem* textItem = scene->addText(systemicon_name_CH->value(keys[index]));
//                textItem->setPos(posX[j], posY[i] + 110);
//                textItem->setDefaultTextColor(Qt::black);
//                QRectF textRect = textItem->boundingRect();
//                textItem->setPos(posX[j] + (128 - textRect.width()) / 2 - 15, posY[i] + 110);

//                // 增加索引的值，准备处理下一个图片的加载和添加操作
//                ++index;
//            }
//        }
//    }else{ // 如果图标映射表的大小不等于11，则进行常规的图片加载和添加操作
//            // 定义列数和行数，以及初始的列和行索引值
//            int columnCount = 10;
//            int row = 1;
//            int col = 2;
//            // 遍历图标映射表的所有键值对，加载并添加图片到场景中
//            for (auto it = m_iconMap->begin(); it != m_iconMap->end(); ++it) {
//                const QString& widgetName = it.key(); // 获取键值对应的名称，用于后续数据的保存和处理中用到此处名称的部分。
//            // 获取当前迭代器it的值
//            const QString& filePath = it.value();

//            // 根据文件路径加载QPixmap对象，这将加载文件并创建一个QPixmap对象
//            QPixmap pixmap(filePath);

//            // 判断图片是否成功加载，如果QPixmap对象为空，说明加载失败
//            if (!pixmap.isNull()) {
//                // 如果图片加载成功，则将图片添加到QGraphicsScene中，并按照128x128的尺寸进行缩放
//                QGraphicsPixmapItem* item = scene->addPixmap(pixmap.scaled(128, 128));

//                // 设置该项可以被选择，即响应用户的点击选择事件
//                item->setFlag(QGraphicsItem::ItemIsSelectable);

//                // 设置该项在场景中的位置
//                item->setPos(col * 160, row * 160);
//                item->setScale(1.0);

//                // 为该项设置一些自定义的数据，这些数据可以通过调用QGraphicsItem::data方法来获取
//                item->setData(0, widgetName); // 将widgetName数据保存到该项的第一个数据槽中
//                item->setData(1, filePath);  // 将filePath数据保存到该项的第二个数据槽中

//                // 增加列计数器
//                col++;

//                // 如果列计数器超过了指定的列数，就重新开始行计数器，同时增加一行
//                if (col >= columnCount) {
//                    col = 2; // 重新开始列计数器，但这次从第二列开始计数
//                    row++;  // 增加一行计数器
//                } else {
//                    qDebug() << "Failed to load image:" << filePath;
//                }
//            }
//        }



//    }
    // 设置初始缩放倍数为0.9倍
    qreal initialScale = 2;
    graphicsView->scale(initialScale, initialScale);
}

/**
 * @brief 更新图标映射
 *
 * 更新图标映射，并重新布局图标。
 *
 * @param newIconMap 新的图标映射
 */
void ImageWidget::updateIconMap(const QMap<QString, QString>* newIconMap, QString type)
{
    m_iconMap = newIconMap;

    // 清空场景
    QGraphicsScene* scene = graphicsView->scene();
    scene->clear();

//    // 判断新的iconMap大小
//    int count = m_iconMap->size();
    if (type == "systemicon") {
        // 更新背景图片
        image = new QPixmap(":/resource/background/controlcenter-light.png");
        if (image->isNull()) {
            qDebug() << "Failed to load image.";
        }
        scene->addPixmap(*image);

        // 重新布局图标
        QList<QString> keys = m_iconMap->keys();
        QString positions[11] = { keys[0], keys[9], keys[4], keys[5], keys[6],
                                  keys[3], keys[10], keys[8], keys[1], keys[7], keys[2] };
        //FIX ME:第三个了历史记录无法打开
        int posX[3] = { 208, 879, 1550 };
        int posY[4] = { 275, 535, 795, 1055 };
        int index = 0;
        QMap<QString,QString>* systemicon_name_CH = new QMap<QString,QString>;
        systemicon_name_CH = initSystemIconName(systemicon_name_CH,keys);

        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (index >= 11) {
                    break;  // 处理超出索引范围的情况
                }

                QString pixmapPath = m_iconMap->value(positions[index]);
                QPixmap pixmap(pixmapPath);
                QGraphicsPixmapItem* item = scene->addPixmap(pixmap.scaled(100, 100));
                item->setFlag(QGraphicsItem::ItemIsSelectable); // 启用选择事件
                item->setFlag(QGraphicsItem::ItemIsFocusable);  // 启用焦点事件
                item->setData(0, keys[index]);
                item->setData(1, pixmapPath);
                item->setPos(posX[j], posY[i]);
                // 添加文字项
                QGraphicsTextItem* textItem = scene->addText(systemicon_name_CH->value(keys[index]));
                textItem->setPos(posX[j], posY[i] + 110);
                textItem->setDefaultTextColor(Qt::black);
                QRectF textRect = textItem->boundingRect();
                textItem->setPos(posX[j] + (128 - textRect.width()) / 2 - 15, posY[i] + 110);

                ++index;
            }
        }
    } else if(type == "appicon") {
        // 更新背景图片
        image = new QPixmap(":/resource/background/background-light.png");
        if (image->isNull()) {
            qDebug() << "Failed to load image.";
        }
        scene->addPixmap(*image);
        QMap<QString,QString>* appicon_name_CH = new QMap<QString,QString>;
        QList<QString> keys = m_iconMap->keys();
        appicon_name_CH = initIconName(appicon_name_CH,keys);
        // 重新布局图标
        int columnCount = 10;
        int row = 1;
        int col = 2;
        for (auto it = m_iconMap->begin(); it != m_iconMap->end(); ++it) {
            const QString& widgetName = it.key();
            const QString& filePath = it.value();

            QPixmap pixmap(filePath);
            if (!pixmap.isNull()) {
                QGraphicsPixmapItem* item = scene->addPixmap(pixmap.scaled(128, 128));
                item->setFlag(QGraphicsItem::ItemIsSelectable); // 启用选择事件
                item->setFlag(QGraphicsItem::ItemIsFocusable);  // 启用焦点事件
                item->setPos(col * 160, row * 160);
                item->setScale(1.0);
                item->setData(0, widgetName);
                item->setData(1, filePath);

                QGraphicsTextItem* textItem = scene->addText(appicon_name_CH->value(it.key()));
                textItem->setPos(col * 160, row * 160 + 128);
                QRectF textRect = textItem->boundingRect();
                textItem->setPos(col * 160 + (128 - textRect.width()) / 2, row * 160 + 120);

                col++;
                if (col >= columnCount) {
                    col = 2;
                    row++;
                }
            } else {
                qDebug() << "Failed to load image:" << filePath;
            }
        }
    }else if(type == "placeicon"){
        // 更新背景图片
        image = new QPixmap(":/resource/background/background-light.png");
        if (image->isNull()) {
            qDebug() << "Failed to load image.";
        }
        scene->addPixmap(*image);
        // 创建有序的键值对列表
        QList<QPair<QString, QString>> sortedList1;
        for (auto it = m_iconMap->begin(); it != m_iconMap->end(); ++it) {
            sortedList1.append(qMakePair(it.key(), it.value()));
        }

        std::sort(sortedList1.begin(), sortedList1.end(), [](const QPair<QString, QString>& pair1, const QPair<QString, QString>& pair2){
            // 按照指定顺序进行排序
            QStringList order = QStringList() << "document-open-recent" << "folder-documents" << "folder-download" << "folder-dropbox" << "folder-music"
                                              << "folder-open"<< "folder-pictures" << "folder-publicshare"
                                              << "folder-templates" << "folder-videos" << "inode-directory" << "user-desktop"
                                              << "user-home" << "user-trash-full"<< "user-trash"
                                              << "computer" << "cupsprinter" << "drive-harddisk" << "drive-removable-media-usb" << "input-keyboard"
                                              << "input-mouse" << "scanner";
            return order.indexOf(pair1.first) < order.indexOf(pair2.first);
        });
        // 重新布局图标
        int columnCount = 10;
        int row = 1;
        int col = 2;
        for (const auto& pair : sortedList1) {
            const QString& widgetName = pair.first;
            const QString& filePath = pair.second;
            const QString& translatedWidgetName = tr(widgetName.toUtf8().constData());

            QPixmap pixmap(filePath);
            if (!pixmap.isNull()) {
                QGraphicsPixmapItem* item = scene->addPixmap(pixmap.scaled(128, 128));
                item->setFlag(QGraphicsItem::ItemIsSelectable); // 启用选择事件
                item->setFlag(QGraphicsItem::ItemIsFocusable);  // 启用焦点事件
                item->setPos(col * 160, row * 160);
                item->setScale(1.0);
                item->setData(0, widgetName);
                item->setData(1, filePath);

                QGraphicsTextItem* textItem = scene->addText(translatedWidgetName);
                textItem->setPos(col * 160, row * 160 + 128);
                QRectF textRect = textItem->boundingRect();
                textItem->setPos(col * 160 + (128 - textRect.width()) / 2, row * 160 + 120);

                col++;
                if (col >= columnCount) {
                    col = 2;
                    row++;
                }
                if (widgetName == "user-trash") {
                    col = 2;
                    row++;
                }
            } else {
                qDebug() << "Failed to load image:" << filePath;
            }
        }
    }

    // 更新视图
    graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

/**
 * @brief 更新图像
 *
 * 根据给定的图像路径更新图像。
 *
 * @param imagePath 图像路径
 */
void ImageWidget::updateImage(const QString& imagePath)
{
    // 如果已经有图片存在，则删除并置空
    if (image != nullptr) {
        delete image;
        image = nullptr;
    }

    // 根据给定的路径加载图片
    image = new QPixmap(imagePath);
    *image = image->scaled(1920, 1080, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (image->isNull()) {
        qDebug() << "Failed to load image.";
    }

    // 获取图形视图的对象，并清空场景中的内容
    QGraphicsScene* scene = graphicsView->scene();
    scene->clear();

    // 在场景中添加图片
    scene->addPixmap(*image);

    // 定义列数和初始行列位置
    int columnCount = 10;
    int row = 1;
    int col = 2;

    // 遍历图标映射表，逐个添加图标到场景中
    for (auto it = m_iconMap->begin(); it != m_iconMap->end(); ++it) {
        const QString& widgetName = it.key();  // 获取键值对应的widgetName
        const QString& filePath = it.value();  // 获取键值对应的文件路径

        // 根据文件路径加载图标
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            // 缩放图标大小为128x128，并添加到场景中
            QGraphicsPixmapItem* item = scene->addPixmap(pixmap.scaled(128, 128));

            // 设置图标的初始位置和缩放比例，并将widgetName和文件路径保存到图标的数据中
            item->setPos(col * 160, row * 160);
            item->setScale(1.0);
            item->setData(0, widgetName);
            item->setData(1, filePath);

            // 更新列的位置，如果超过列数则重新开始行，行数加1
            col++;
            if (col >= columnCount) {
                col = 2;
                row++;
            }
        } else {
            qDebug() << "Failed to load image:" << filePath;
        }
    }

    // 根据场景的边界自适应视图大小，保持视图比例不变
    graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

/**
 * @brief 更新图标
 *
 * 根据给定的 widgetName 和 newFilePath，更新图标。
 *
 * @param widgetName 控件名称
 * @param newFilePath 新的文件路径
 */
void ImageWidget::updateIcon(const QString& widgetName, const QString& newFilePath)
{
    QGraphicsScene* scene = graphicsView->scene();
    QList<QGraphicsItem*> items = scene->items();
//    for (int i = 0; i < items.size(); ++i) {
//        QGraphicsItem* item = items.at(i);
//        // 输出item的内容，例如位置和类型等信息
//        qDebug() << "Item" << i << ":"
//                 << "Position:" << item->pos()
//                 << "Type:" << item->type();
//    }
    // 遍历所有的图形项
    for (QGraphicsItem* item : items) {
        // 尝试将当前图形项转换为图形位图项
        QGraphicsPixmapItem* pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(item);
        // 如果图形位图项存在，并且它的数据（索引为0）与widgetName相同
        if (pixmapItem && pixmapItem->data(0).toString() == widgetName) {
            // 保存图形位图项的旧位置
            QPointF oldPosition = pixmapItem->pos();
            // 从场景中移除图形位图项
            scene->removeItem(pixmapItem);
            // 删除图形位图项
            delete pixmapItem;
            // 加载新的位图（从newFilePath路径）
            QPixmap newPixmap(newFilePath);
            // 如果新的位图不为空
            if (!newPixmap.isNull()) {
                // 如果widgetName包含"kylin-setting"字符串
                if(widgetName.contains("kylin-setting")){
                    // 添加新的位图（缩放为100x100）到场景中，并设置数据（索引为0为widgetName, 索引为1为newFilePath）
                    QGraphicsPixmapItem* newPixmapItem = scene->addPixmap(newPixmap.scaled(100, 100));
                    newPixmapItem->setData(0, widgetName);
                    newPixmapItem->setData(1, newFilePath);
                    // 设置新的位图的位置为旧位置
                    newPixmapItem->setPos(oldPosition);
                }else{
                    // 添加新的位图（缩放为128x128）到场景中，并设置数据（索引为0为widgetName, 索引为1为newFilePath）
                    QGraphicsPixmapItem* newPixmapItem = scene->addPixmap(newPixmap.scaled(128, 128));
                    newPixmapItem->setData(0, widgetName);
                    newPixmapItem->setData(1, newFilePath);
                    // 设置新的位图的位置为旧位置
                    newPixmapItem->setPos(oldPosition);
                }
            } else {
                 qDebug() << "Failed to load image:" << newFilePath;
             }
             return;
         }
     }

}

void ImageWidget::maxscale()
{
    graphicsView->resetTransform();

    QTransform currentTransform = graphicsView->transform();

    qreal currentScaleX = currentTransform.m11();
    qreal currentScaleY = currentTransform.m22();

    qreal newScaleX = currentScaleX * 0.6;
    qreal newScaleY = currentScaleY * 0.6;

    graphicsView->scale(newScaleX, newScaleY);
}

void ImageWidget::miniscale()
{
    graphicsView->resetTransform();

    QTransform currentTransform = graphicsView->transform();

    qreal currentScaleX = currentTransform.m11();
    qreal currentScaleY = currentTransform.m22();

    qreal newScaleX = currentScaleX * 0.3;
    qreal newScaleY = currentScaleY * 0.3;

    graphicsView->scale(newScaleX, newScaleY);
}

QMap<QString, QString> *ImageWidget::initIconName(QMap<QString, QString> *name_CH, QList<QString> keys)
{
    name_CH->insert(keys[0],tr("biometric-manager"));
    name_CH->insert(keys[1],tr("box-manager"));
    name_CH->insert(keys[2],tr("burner"));
    name_CH->insert(keys[3],tr("indicator-china-weather"));
    name_CH->insert(keys[4],tr("ksc-defender"));
    name_CH->insert(keys[5],tr("kylin-alarm-clock"));
    name_CH->insert(keys[6],tr("kylin-assistant"));
    name_CH->insert(keys[7],tr("kylin-calculator"));
    name_CH->insert(keys[8],tr("kylin-camera"));
    name_CH->insert(keys[9],tr("kylin-connectivity"));
    name_CH->insert(keys[10],tr("kylin-font-viewer"));
    name_CH->insert(keys[11],tr("kylin-installer"));
    name_CH->insert(keys[12],tr("kylin-ipmsg"));
    name_CH->insert(keys[13],tr("kylin-log-viewer"));
    name_CH->insert(keys[14],tr("kylin-music"));
    name_CH->insert(keys[15],tr("kylin-new-function-introduction"));
    name_CH->insert(keys[16],tr("kylin-notebook"));
    name_CH->insert(keys[17],tr("kylin-os-manager"));
    name_CH->insert(keys[18],tr("kylin-photo-viewer"));
    name_CH->insert(keys[19],tr("kylin-printer"));
    name_CH->insert(keys[20],tr("kylin-recorder"));
    name_CH->insert(keys[21],tr("kylin-screenshot"));
    name_CH->insert(keys[22],tr("kylin-service-support"));
    name_CH->insert(keys[23],tr("kylin-software-center"));
    name_CH->insert(keys[24],tr("kylin-usb-creator"));
    name_CH->insert(keys[25],tr("kylin-user-guide"));
    name_CH->insert(keys[26],tr("kylin-video"));
    name_CH->insert(keys[27],tr("system-file-manager"));
    name_CH->insert(keys[28],tr("ukui-control-center"));
    name_CH->insert(keys[29],tr("ukui-system-monitor"));
    name_CH->insert(keys[30],tr("utilities-terminal"));
    name_CH->insert(keys[31],tr("yhkylin-backup-tools"));

    return name_CH;
}

QMap<QString, QString> *ImageWidget::initSystemIconName(QMap<QString, QString> *name_CH, QList<QString> keys)
{
    name_CH->insert(keys[0],tr("kylin-settings-account"));
    name_CH->insert(keys[1],tr("kylin-settings-system"));
    name_CH->insert(keys[2],tr("kylin-settings-devices"));
    name_CH->insert(keys[3],tr("kylin-settings-network"));
    name_CH->insert(keys[4],tr("kylin-settings-personalized"));
    name_CH->insert(keys[5],tr("kylin-settings-update"));
    name_CH->insert(keys[6],tr("kylin-settings-datetime"));
    name_CH->insert(keys[7],tr("kylin-settings-security"));
    name_CH->insert(keys[8],tr("kylin-settings-application"));
    name_CH->insert(keys[9],tr("kylin-settings-search"));
    name_CH->insert(keys[10],tr("kylin-settings-commoninfo"));

    return name_CH;
}

QMap<QString, QString> *ImageWidget::initPlaceIconName(QMap<QString, QString> *name_CH, QList<QString> keys)
{
    name_CH->insert(keys[0],tr("document-open-recent"));
    name_CH->insert(keys[1],tr("folder-documents"));
    name_CH->insert(keys[2],tr("folder-download"));
    name_CH->insert(keys[3],tr("folder-dropbox"));
    name_CH->insert(keys[4],tr("folder-music"));
    name_CH->insert(keys[5],tr("folder-open"));
    name_CH->insert(keys[6],tr("folder-pictures"));
    name_CH->insert(keys[7],tr("folder-publicshare"));
    name_CH->insert(keys[8],tr("folder-templates"));
    name_CH->insert(keys[9],tr("folder-videos"));
    name_CH->insert(keys[10],tr("inode-directory"));
    name_CH->insert(keys[11],tr("user-desktop"));
    name_CH->insert(keys[12],tr("user-home"));
    name_CH->insert(keys[13],tr("user-trash-full"));
    name_CH->insert(keys[14],tr("user-trash"));
    return name_CH;
}

QMap<QString, QString> *ImageWidget::initDeviceIconName(QMap<QString, QString> *name_CH, QList<QString> keys)
{
    name_CH->insert(keys[0],tr("computer"));
    name_CH->insert(keys[1],tr("cupsprinter"));
    name_CH->insert(keys[2],tr("drive-harddisk"));
    name_CH->insert(keys[3],tr("drive-removable-media-usb"));
    name_CH->insert(keys[4],tr("input-keyboard"));
    name_CH->insert(keys[5],tr("input-mouse"));
    name_CH->insert(keys[6],tr("scanner"));
    return name_CH;
}

/**
 * @brief 构造函数
 *
 * 创建一个图标编辑器部件，作为父窗口的子部件进行显示。
 *
 * @param parent 父窗口指针
 */
IconEditWidget::IconEditWidget(QWidget *parent)
{
    QVBoxLayout *widgetLayout = new QVBoxLayout(this);

    // 创建一个 QFrame 作为边框容器
    QFrame *frame = new QFrame(this);
    frame->setFrameShape(QFrame::NoFrame);  // 设置边框样式

    QVBoxLayout *innerLayout = new QVBoxLayout();
    m_label = new QLabel();
    QWidget *editwidget = new QWidget();

    QHBoxLayout *m_iconwidgetlayout = new QHBoxLayout();
    m_icondefaultlabel = new QLabel();
    m_icondefaultlabel->setFixedSize(48,48);

    m_icondecustomlabel = new CustomLabel();

    m_addiconbutton = new DropButton();
    m_addiconbutton->setIcon(QIcon::fromTheme("list-add-symbolic"));
    m_addiconbutton->setFixedSize(36,36);
    m_addiconbutton->setVisible(false);
    m_addwidget = new addIconWidget();
//    QLabel *m_tiplabel = new QLabel();
//    m_tiplabel->setWordWrap(true);
//    m_tiplabel->setText("<html>格式：svg<br>尺寸：96*96</html>");

    m_iconwidgetlayout->addWidget(m_icondefaultlabel);
    m_iconwidgetlayout->addWidget(m_icondecustomlabel);
    m_iconwidgetlayout->addWidget(m_addiconbutton);
    m_iconwidgetlayout->addWidget(m_addwidget);
//    m_iconwidgetlayout->addWidget(m_tiplabel);
    editwidget->setLayout(m_iconwidgetlayout);

    innerLayout->addWidget(m_label);
    innerLayout->addWidget(editwidget);

    frame->setLayout(innerLayout);
    widgetLayout->addWidget(frame);
    connect(m_icondecustomlabel,&CustomLabel::deleteCustomIcon,this,&IconEditWidget::deleteCustomIcon);

    connect(m_addwidget,&addIconWidget::buttonclick,m_addiconbutton,&DropButton::click);
    connect(m_addwidget,&addIconWidget::droprealicon,m_addiconbutton,&DropButton::droprealicon);
}

/**
 * @brief 设置默认图标
 *
 * 设置默认图标的函数，通过传入图标名称来设置默认图标。
 *
 * @param iconname 图标名称
 */
void IconEditWidget::setdefaulticon(QString iconname)
{
    m_icondefaultlabel->setPixmap(QIcon::fromTheme(iconname).pixmap(48,48));
}

/**
 * @brief 设置默认图标名称
 *
 * 设置默认图标名称，将标签的文本更新为指定的图标名称。
 *
 * @param iconname 图标名称
 */
void IconEditWidget::setdefaulticonname(QString iconname)
{
    m_label->setText(iconname);
}

/**
 * @brief 设置自定义图标
 *
 * 通过指定的图标文件路径设置自定义图标。
 *
 * @param iconFilePath 图标文件路径
 */
void IconEditWidget::setcustomicon(QString iconFilePath)
{
    QPixmap pixmap(iconFilePath);
    m_icondecustomlabel->setPixmap(pixmap.scaled(48, 48));
}

/**
 * @brief 构造函数
 *
 * 创建一个 CustomLabel 对象，并设置父对象。
 *
 * @param parent 父对象指针
 */
CustomLabel::CustomLabel(QWidget *parent) : QLabel(parent), pixmap(), closeIconRect(0, 0, 0, 0)
{
    this->setMouseTracking(true);
    this->setFixedSize(48,48);
    initializeCloseIconRect();
}

/**
 * @brief 初始化关闭图标矩形
 *
 * 初始化关闭图标的矩形大小，根据窗口宽度进行设置。
 */
void CustomLabel::initializeCloseIconRect()
{
    closeIconRect = QRect(width() - 15, 5, 10, 10);
}

/**
 * @brief 自定义标签的绘制事件
 *
 * 在绘制事件发生时，执行标签的绘制操作。
 *
 * @param event 绘制事件指针
 */
void CustomLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    QPainter painter(this);
    QPen pen(Qt::blue);
    pen.setStyle(Qt::DashLine);
    painter.setPen(pen);

    QBrush brush(QColor(qApp->palette().window().color()));
    painter.setBrush(brush);

    QRect roundedRect = rect().adjusted(1, 1, -1, -1);
    QPainterPath path;
    int radius = 6;
    path.addRoundedRect(roundedRect, radius, radius);
    painter.drawPath(path);

    if (!pixmap.isNull()) {
        painter.drawPixmap(rect(), pixmap);
    }
        drawCloseIcon(painter); // 调用绘制关闭图标的函数
}

/**
 * @brief 绘制关闭图标
 *
 * 使用指定的画笔在给定的位置绘制关闭图标。
 *
 * @param painter 画笔对象
 */
void CustomLabel::drawCloseIcon(QPainter &painter)
{
    if (!closeIconPixmap.isNull()) {
        painter.drawPixmap(closeIconRect, closeIconPixmap);
    }
}

/**
 * @brief 设置 QPixmap
 *
 * 设置 CustomLabel 的 QPixmap。
 *
 * @param pixmap QPixmap 对象
 */
void CustomLabel::setPixmap(const QPixmap &pixmap)
{
    this->pixmap = pixmap;
    update();
}

/**
 * @brief 进入事件处理函数
 *
 * 当鼠标进入 CustomLabel 控件时，该函数将被调用。
 *
 * @param event 进入事件指针
 */
void CustomLabel::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);
    if(!this->pixmap.isNull()){
        showCloseIcon(); // 在这个函数中显示“x”图标
    }else{
        qDebug()<<"pixmap is null";
    }

}

/**
 * @brief 处理鼠标离开事件
 *
 * 当鼠标离开控件时，该函数将被调用。在此函数中，调用 QWidget::leaveEvent() 处理鼠标离开事件，并隐藏“x”图标。
 *
 * @param event 鼠标事件指针
 */
void CustomLabel::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    hideCloseIcon(); // 在这个函数中隐藏“x”图标
}

/**
 * @brief 鼠标按下事件处理函数
 *
 * 当鼠标按下时，该函数会处理事件。如果点击的位置在关闭图标矩形内，则将图标显示为空，并隐藏关闭图标。
 * 否则，处理其他点击事件。
 *
 * @param event 鼠标事件指针
 */
void CustomLabel::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if (closeIconRect.contains(event->pos())) {
        setPixmap(QPixmap()); // 点击“x”时将 Icon 显示为空
        hideCloseIcon(); // 在这个函数中隐藏“x”图标
        emit deleteCustomIcon();
    } else {
        // 处理其他点击事件
    }
}

/**
 * @brief 显示关闭图标
 *
 * 将关闭图标的Pixmap从主题中加载，并更新控件的显示。
 */
void CustomLabel::showCloseIcon()
{
    closeIconPixmap = QIcon::fromTheme("window-close-symbolic").pixmap(10, 10);
    this->update();
}

/**
 * @brief 隐藏关闭图标
 *
 * 将关闭图标的pixmap置空，并重新绘制以清除关闭图标。
 */
void CustomLabel::hideCloseIcon()
{
    closeIconPixmap = QPixmap(); // 将关闭图标的pixmap置空
    update(); // 重新绘制以清除关闭图标
}


DropButton::DropButton(QWidget *parent) : QPushButton(parent) {
    setAcceptDrops(true);
}

void DropButton::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void DropButton::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        for (const QUrl &url : urlList) {
//            qDebug() << "File path:" << url.toLocalFile();
            bool islegal = FileCheck::isLegalIconFile(url.toLocalFile());
            if(islegal){
                //发信号给widget，换图标
                emit droprealicon(url.toLocalFile());
            }
        }
    }
}

addIconWidget::addIconWidget(QWidget *parent)
{
    this->setFixedSize(188,48);
    this->setAcceptDrops(true);
    QHBoxLayout *layout = new QHBoxLayout(this);

    QPushButton *button = new QPushButton();
    button->setFixedSize(36,36);
    button->setIcon(QIcon::fromTheme("list-add-symbolic"));
    button->setProperty("isRoundButton", true);

    QLabel *tip = new QLabel();
    tip->setText(tr("Add icon"));

    layout->addStretch(1);
    layout->addWidget(button);
    layout->addWidget(tip);
    layout->addStretch(1);

    connect(button,&QPushButton::clicked,this,[=](){
        emit buttonclick();
    });
}

void addIconWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    QPen *m_pen = new QPen();
    m_pen->setColor(QColor(Qt::gray));
    m_pen->setStyle(Qt::DashLine);
    painter.setPen(*m_pen);

    int radius = 7;
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(1,1,-1,-1), radius, radius);

    painter.drawPath(path);
}

void addIconWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void addIconWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        for (const QUrl &url : urlList) {
//            qDebug() << "File path:" << url.toLocalFile();
            bool islegal = FileCheck::isLegalIconFile(url.toLocalFile());
            if(islegal){
                //发信号给widget，换图标
                emit droprealicon(url.toLocalFile());
            }
        }
    }
}
