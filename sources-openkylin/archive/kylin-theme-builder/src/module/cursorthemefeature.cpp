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
#include "cursorthemefeature.h"
#include "cursorthemewidget.h"

CursorThemeFeature::CursorThemeFeature(QWidget *parent, const QMap<QString, QString> *iconMap) : QWidget(parent)
{
    CursorImageWidget* cursorImageWidget = new CursorImageWidget(this, iconMap);
}

CursorGraphicsView::CursorGraphicsView(QGraphicsScene* scene) : QGraphicsView(scene)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setRenderHint(QPainter::Antialiasing, true);
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing |QGraphicsView::DontSavePainterState);

    this->setStyleSheet("background-color: transparent;");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void CursorGraphicsView::wheelEvent(QWheelEvent* event)
{
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
        scale(scaleFactor, scaleFactor);
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void CursorImageWidget::updateImage(const QString& imagePath)
{
    // 删除场景中的所有图像项
    graphicsView->scene()->clear();

    // 创建新的背景图像项并添加到场景中
    QPixmap image(imagePath);
    if (image.isNull()) {
        qDebug() << "Failed to load new image.";
        return;
    }
    image = image.scaled(1920, 1080, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QGraphicsPixmapItem* backgroundItem = graphicsView->scene()->addPixmap(image);

    // 创建 QGraphicsBlurEffect 并设置模糊半径
    QGraphicsBlurEffect* blurEffect = new QGraphicsBlurEffect;
    blurEffect->setBlurRadius(10);

    // 将模糊效果应用到 QGraphicsPixmapItem 上
    backgroundItem->setGraphicsEffect(blurEffect);

    // 根据需要重新布局图像项
    int columnCount = 10;
    int row = 1;
    int col = 0;

    // 创建有序的键值对列表
    QList<QPair<QString, QString>> sortedList;
    for (auto it = m_iconMap->begin(); it != m_iconMap->end(); ++it) {
        sortedList.append(qMakePair(it.key(), it.value()));
    }
    std::sort(sortedList.begin(), sortedList.end(), [this](const QPair<QString, QString>& pair1, const QPair<QString, QString>& pair2) {
        QStringList order1 = QStringList() << "left_ptr" << "dnd-none" << "hand1" << "hand2" << "right_ptr"
                                           << "dnd-ask"<< "dnd-copy" << "dnd-link"
                                           << "h_double_arrow" << "v_double_arrow" << "ul_angle" << "ur_angle"
                                           << "left_tee" << "bottom_tee"<< "top_tee" << "right_tee" << "cross" << "tcross"
                                           << "sb_up_arrow" << "sb_left_arrow" << "sb_down_arrow" << "sb_right_arrow" << "all-scroll"
                                           << "sb_h_double_arrow" << "sb_v_double_arrow" << "vertical_text" << "xterm"
                                           << "zoom-in" << "zoom-out"
                                           << "circle" << "color-picker" << "pencil" << "dotbox" << "X_cursor" << "plus";

        QStringList order2 = QStringList() << "watch_1" << "watch_2" << "watch_3" << "watch_4" << "watch_5" << "watch_6" << "watch_7" << "watch_8" << "watch_9" << "watch_10"
                                           << "left_ptr_watch_1" << "left_ptr_watch_2" << "left_ptr_watch_3" << "left_ptr_watch_4" << "left_ptr_watch_5";

        if (m_iconMap->contains("left_ptr_watch_1")) {
            return order2.indexOf(pair1.first) < order2.indexOf(pair2.first);
        } else {
            return order1.indexOf(pair1.first) < order1.indexOf(pair2.first);
        }
    });

    for (const auto& pair : sortedList) {
        // 添加新的图像项到场景中
        const QString& widgetName = pair.first;
        const QString& filePath = pair.second;
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            QGraphicsPixmapItem* item = graphicsView->scene()->addPixmap(pixmap.scaled(128, 128));

            item->setPos(col * 198, row * 198);
            item->setScale(1.0);
            item->setData(0, widgetName);
            item->setData(1, filePath);
            col++;
            if (col >= columnCount) {
                col =  0;
                row++;
            }
        } else {
            qDebug() << "Failed to load image:" << filePath;
        }
    }
    // 适应视图，保持宽高比
    graphicsView->fitInView(graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
    // 获取当前视图转换矩阵
    QTransform currentTransform = graphicsView->transform();
    // 计算场景的中心点坐标，并转换为视图坐标系下的坐标
    QPointF sceneCenter = graphicsView->mapToScene(graphicsView->viewport()->rect().center());
    currentTransform.translate(sceneCenter.x(), sceneCenter.y());
    // 设置新的视图转换矩阵，使视图中心点移动到场景中心点
    graphicsView->setTransform(currentTransform);
    // 计算场景的中心点坐标，并转换为视图坐标系下的坐标
    QPointF viewCenter = graphicsView->mapFromScene(graphicsView->scene()->sceneRect().center());
    // 将视图中心点移动到新的中心点
    graphicsView->centerOn(viewCenter);

    // 更新视口
    graphicsView->viewport()->update();
}

/**
 * @brief 更新图标映射
 *
 * 根据新的图标映射更新光标图像小部件的图标。
 *
 * @param newIconMap 新的图标映射
 */
void CursorImageWidget::updateIconMap(const QMap<QString, QString> *newIconMap)
{
    m_iconMap = newIconMap;
}

/**
 * @brief CursorImageWidget 构造函数
 *
 * 初始化 CursorImageWidget 对象。
 *
 * @param parent 父 QWidget 对象指针
 * @param iconMap 包含图标键值对的 QMap 对象指针
 */
CursorImageWidget::CursorImageWidget(QWidget *parent, const QMap<QString, QString>* iconMap)
    : QWidget(parent), m_iconMap(iconMap)
{

    QGraphicsScene* scene = new QGraphicsScene(this);
    graphicsView = new CursorGraphicsView(scene);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(graphicsView);
    setLayout(layout);

    QPixmap image(":/resource/background/background-light.png");
    if (image.isNull()) {
        qDebug() << "Failed to load image.";
    }

    // 创建 QGraphicsPixmapItem 并添加到 QGraphicsScene
    QGraphicsPixmapItem* backgroundItem = scene->addPixmap(image);

    // 创建 QGraphicsBlurEffect 并设置模糊半径
    QGraphicsBlurEffect* blurEffect = new QGraphicsBlurEffect;
    blurEffect->setBlurRadius(10);  // 设置模糊半径，可以根据需要调整

    // 将模糊效果应用到 QGraphicsPixmapItem 上
    backgroundItem->setGraphicsEffect(blurEffect);
    graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    graphicsView->setScene(scene);
    graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    graphicsView->show();

    int columnCount = 10;
    int row = 1;
    int col = 0;

    // 创建有序的键值对列表
    QList<QPair<QString, QString>> sortedList;
    for (auto it = m_iconMap->begin(); it != m_iconMap->end(); ++it) {
        sortedList.append(qMakePair(it.key(), it.value()));
    }
    std::sort(sortedList.begin(), sortedList.end(), [this](const QPair<QString, QString>& pair1, const QPair<QString, QString>& pair2) {
        QStringList order1 = QStringList() << "left_ptr" << "dnd-none" << "hand1" << "hand2" << "right_ptr"
                                           << "dnd-ask"<< "dnd-copy" << "dnd-link"
                                           << "h_double_arrow" << "v_double_arrow" << "ul_angle" << "ur_angle"
                                           << "left_tee" << "bottom_tee"<< "top_tee" << "right_tee" << "cross" << "tcross"
                                           << "sb_up_arrow" << "sb_left_arrow" << "sb_down_arrow" << "sb_right_arrow" << "all-scroll"
                                           << "sb_h_double_arrow" << "sb_v_double_arrow" << "vertical_text" << "xterm"
                                           << "zoom-in" << "zoom-out"
                                           << "circle" << "color-picker" << "pencil" << "dotbox" << "X_cursor" << "plus";

        QStringList order2 = QStringList() << "watch_1" << "watch_2" << "watch_3" << "watch_4" << "watch_5" << "watch_6" << "watch_7" << "watch_8" << "watch_9" << "watch_10"
                                           << "left_ptr_watch_1" << "left_ptr_watch_2" << "left_ptr_watch_3" << "left_ptr_watch_4" << "left_ptr_watch_5";


        if (m_iconMap->contains("left_ptr_watch_1")) {
            return order2.indexOf(pair1.first) < order2.indexOf(pair2.first);
        } else {
            return order1.indexOf(pair1.first) < order1.indexOf(pair2.first);
        }
    });

    for (const auto& pair : sortedList) {
        const QString& widgetName = pair.first;
        const QString& filePath = pair.second;

        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            QGraphicsPixmapItem* item = scene->addPixmap(pixmap.scaled(128, 128));

            item->setPos(col * 198, row * 198);
            item->setScale(1.0);
            item->setData(0, widgetName);
            item->setData(1, filePath);
            col++;
            if (col >= columnCount) {
                col =  0;
                row++;
            }
        } else {
            qDebug() << "Failed to load image:" << filePath;
        }
    }

    qreal initialScale = 0.3;
    QTransform transform;
    transform.scale(initialScale, initialScale);
    graphicsView->setTransform(transform);
}

/**
 * @brief 更新图标
 *
 * 根据给定的 widgetName 和 newFilePath，更新指定在 QGraphicsView 中的光标图像部件的图标。
 *
 * @param widgetName 部件名称
 * @param newFilePath 新的文件路径
 */
void CursorImageWidget::updateIcon(const QString& widgetName, const QString& newFilePath)
{
    QGraphicsScene* scene = graphicsView->scene();
    QList<QGraphicsItem*> items = scene->items();

    for (QGraphicsItem* item : items) {
        QGraphicsPixmapItem* pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(item);
        if (pixmapItem && pixmapItem->data(0).toString() == widgetName) {

            QPointF oldPosition = pixmapItem->pos();
            scene->removeItem(pixmapItem);
            delete pixmapItem;
            QPixmap newPixmap(newFilePath);
            if (!newPixmap.isNull()) {
                QGraphicsPixmapItem* newPixmapItem = scene->addPixmap(newPixmap.scaled(128, 128));
                newPixmapItem->setData(0, widgetName);
                newPixmapItem->setData(1, newFilePath);
                newPixmapItem->setPos(oldPosition);
            } else {
                qDebug() << "Failed to load image:" << newFilePath;
            }
            return;
        }
    }
}

void CursorImageWidget::setIconMap(QMap<QString, QString> *iconmap)
{
    m_iconMap = iconmap;
}

void CursorImageWidget::maxscale()
{
    graphicsView->resetTransform();

    QTransform currentTransform = graphicsView->transform();

    qreal currentScaleX = currentTransform.m11();
    qreal currentScaleY = currentTransform.m22();

    qreal newScaleX = currentScaleX * 0.6;
    qreal newScaleY = currentScaleY * 0.6;

    graphicsView->scale(newScaleX, newScaleY);
}

void CursorImageWidget::miniscale()
{
    graphicsView->resetTransform();

    QTransform currentTransform = graphicsView->transform();

    qreal currentScaleX = currentTransform.m11();
    qreal currentScaleY = currentTransform.m22();

    qreal newScaleX = currentScaleX * 0.3;
    qreal newScaleY = currentScaleY * 0.3;

    graphicsView->scale(newScaleX, newScaleY);
}


/**
 * @brief 构造函数
 *
 * 创建一个 CursorEditWidget 对象。
 *
 * @param parent 父对象指针
 */
CursorEditWidget::CursorEditWidget(QWidget *parent)
{

    this->setFixedHeight(110);
    m_iconwidgetlayout = new QHBoxLayout();
    m_icondefaultlabel = new QLabel();
    m_icondefaultlabel->setFixedSize(48,48);

    m_icondecustomlabel = new CursorCustomLabel();

    m_addiconbutton = new DropButton();
    m_addiconbutton->setIcon(QIcon::fromTheme("list-add-symbolic"));
    m_addiconbutton->setFixedSize(36,36);
    m_addiconbutton->setVisible(false);

    m_tiplabel = new QLabel();
    m_tiplabel->setWordWrap(true);
    m_tiplabel->setText("<html>格式：svg<br>尺寸：96*96</html>");

    cursorAddIconWidget *a = new cursorAddIconWidget();

    m_iconwidgetlayout->addWidget(m_icondefaultlabel);
    m_iconwidgetlayout->addWidget(m_icondecustomlabel);
    m_iconwidgetlayout->addWidget(a);
    m_iconwidgetlayout->addWidget(m_addiconbutton);
//    m_iconwidgetlayout->addWidget(m_tiplabel);
    this->setLayout(m_iconwidgetlayout);

    connect(m_icondecustomlabel,&CursorCustomLabel::deleteCustomIcon,this,&CursorEditWidget::deleteCustomIcon);

    connect(a,&cursorAddIconWidget::buttonclick,m_addiconbutton,&DropButton::click);
    connect(a,&cursorAddIconWidget::droprealicon,m_addiconbutton,&DropButton::droprealicon);
}

void CursorEditWidget::setdefaulticon(QString iconname)
{
    m_icondefaultlabel->setPixmap(QIcon::fromTheme(iconname).pixmap(48,48));
}

void CursorEditWidget::setcustomicon(QString iconFilePath)
{
    QPixmap pixmap(iconFilePath);
    m_icondecustomlabel->setPixmap(pixmap.scaled(48, 48));
}

CursorCustomLabel::CursorCustomLabel(QWidget *parent): QLabel(parent), pixmap()
{
    this->setFixedSize(48,48);
    initializeCloseIconRect();
}

/**
 * @brief 初始化关闭图标矩形
 *
 * 初始化关闭图标的矩形大小，根据窗口宽度进行设置。
 */
void CursorCustomLabel::initializeCloseIconRect()
{
    closeIconRect = QRect(width() - 15, 5, 10, 10);
}

/**
 * @brief 绘制事件
 *
 * 当需要绘制标签时，调用此函数。
 *
 * @param event QPaintEvent 对象指针，表示绘制事件
 */
void CursorCustomLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event); // 调用父类的 paintEvent 函数，绘制文本

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
void CursorCustomLabel::drawCloseIcon(QPainter &painter)
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
void CursorCustomLabel::setPixmap(const QPixmap &pixmap)
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
void CursorCustomLabel::enterEvent(QEvent *event)
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
void CursorCustomLabel::leaveEvent(QEvent *event)
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
void CursorCustomLabel::mousePressEvent(QMouseEvent *event)
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
void CursorCustomLabel::showCloseIcon()
{
    closeIconPixmap = QIcon::fromTheme("window-close-symbolic").pixmap(10, 10);
    this->update();
}

/**
 * @brief 隐藏关闭图标
 *
 * 将关闭图标的pixmap置空，并重新绘制以清除关闭图标。
 */
void CursorCustomLabel::hideCloseIcon()
{
    closeIconPixmap = QPixmap(); // 将关闭图标的pixmap置空
    update(); // 重新绘制以清除关闭图标
}

cursorAddIconWidget::cursorAddIconWidget(QWidget *parent)
{
    this->setFixedSize(188,48);
    this->setAcceptDrops(true);
    QHBoxLayout *layout = new QHBoxLayout(this);

    QPushButton *button = new QPushButton();
    button->setFixedSize(36,36);
    button->setIcon(QIcon::fromTheme("list-add-symbolic"));
    button->setProperty("isRoundButton", true);

    QLabel *tip = new QLabel();
    tip->setText(tr("Add Icon"));

    layout->addStretch(1);
    layout->addWidget(button);
    layout->addWidget(tip);
    layout->addStretch(1);

    connect(button,&QPushButton::clicked,this,[=](){
        emit buttonclick();
    });
}

void cursorAddIconWidget::paintEvent(QPaintEvent *event)
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

void cursorAddIconWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void cursorAddIconWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        for (const QUrl &url : urlList) {
            qDebug() << "File path:" << url.toLocalFile();
            bool islegal = FileCheck::isLegalCursorFile(url.toLocalFile());
            if(islegal){
                //发信号给widget，换图标
                emit droprealicon(url.toLocalFile());
            }
        }
    }
}
