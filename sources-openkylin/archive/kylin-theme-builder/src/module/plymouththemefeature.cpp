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
#include "plymouththemefeature.h"
#include <QDebug>
#include <QIcon>

PlymouthThemeFeature::PlymouthThemeFeature(QWidget *parent) : QWidget(parent)
{

}

/**
 * @brief PlymouthImageWidget 构造函数
 *
 * 创建一个 PlymouthImageWidget 对象，并将其设置为给定父对象的子对象。
 *
 * @param parent 父对象指针
 */
PlymouthImageWidget::PlymouthImageWidget(QWidget *parent)
{
    scene = new QGraphicsScene(this);
    graphicsView = new PlymouthGraphicsView(scene);

    layout = new QVBoxLayout(this);
    layout->addWidget(graphicsView);
    setLayout(layout);
    graphicsView->setScene(scene);

    for (int i = 1; i <= 104; ++i) {
        pixmaps.append(QPixmap(QString(":/resource/plymouth/%1.png").arg(i)));
    }
    QPixmap pixmap(":/resource/plymouth/1.png");
    pixmapItem = new QGraphicsPixmapItem(pixmaps.first());
    qreal x = (scene->width() - pixmap.width()) / 2;
//    qDebug()<<"x= "<<x<<"scene->width()"<<scene->width()<<" pixmap.width()" << pixmap.width();
    qreal y = (scene->height() - pixmap.height()) / 2;
    sceneWidth = scene->width();
    sceneHeight = scene->height();
    pixmapItem->setPos(x, y);
    scene->addItem(pixmapItem);

//    graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    graphicsView->show();
    qreal initialScale = 0.3;
    graphicsView->scale(initialScale, initialScale);

}

/**
 * @brief 更新 Plymouth 图像
 *
 * 根据给定的图像路径更新 Plymouth 图像。
 *
 * @param imagePath 图像路径
 */
void PlymouthImageWidget::updatePlymouth(const QString &imagePath)
{
    pixmaps.clear(); // 清空pixmaps

    QDir dir(imagePath);
    QStringList filters;
    filters << "*.png";
    dir.setNameFilters(filters);
    QStringList fileList = dir.entryList();

    // 对文件列表进行排序
    std::sort(fileList.begin(), fileList.end(), [](const QString& fileName1, const QString& fileName2) {
        // 提取文件名中的数字部分
        QString numberPart1 = fileName1;
        QString numberPart2 = fileName2;
        numberPart1.remove(".png");
        numberPart2.remove(".png");

        // 将数字部分转换为整数进行比较
        int num1 = numberPart1.toInt();
        int num2 = numberPart2.toInt();

        return num1 < num2;
    });

    int numImages = fileList.size();
    for (int i = 0; i < numImages; ++i) {
        pixmaps.append(QPixmap(imagePath + "/" + fileList[i])); // 将新的图片追加到pixmaps中
    }

    // Update the displayed image in the scene
    if (numImages > 0) {
        pixmapItem->setPixmap(pixmaps.first());
        qreal x = (sceneWidth - pixmaps.first().width()) / 2;
        qreal y = (sceneHeight - pixmaps.first().height()) / 2;
        pixmapItem->setPos(x, y);
    }
//    qreal initialScale = 0.3;
//    graphicsView->scale(initialScale, initialScale);
}

/**
 * @brief 显示 Plymouth 图像
 *
 * 通过定时器，依次显示 pixmaps 中的图像，形成动画效果。
 */
void PlymouthImageWidget::showPlymouth()
{
    index = 0;
    QObject::disconnect(&timer, &QTimer::timeout, nullptr, nullptr);
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        if (index < pixmaps.size()) {
            pixmapItem->setPixmap(pixmaps.at(index));
            ++index;
        } else {
            index = 0;
        }
    });
    timer.start(30);

}

void PlymouthImageWidget::maxscale()
{
    graphicsView->resetTransform();

    QTransform currentTransform = graphicsView->transform();

    qreal currentScaleX = currentTransform.m11();
    qreal currentScaleY = currentTransform.m22();

    qreal newScaleX = currentScaleX * 0.9;
    qreal newScaleY = currentScaleY * 0.9;

    graphicsView->scale(newScaleX, newScaleY);
}

void PlymouthImageWidget::miniscale()
{
    graphicsView->resetTransform();

    QTransform currentTransform = graphicsView->transform();

    qreal currentScaleX = currentTransform.m11();
    qreal currentScaleY = currentTransform.m22();

    qreal newScaleX = currentScaleX * 0.3;
    qreal newScaleY = currentScaleY * 0.3;

    graphicsView->scale(newScaleX, newScaleY);
}

PlymouthGraphicsView::PlymouthGraphicsView(QGraphicsScene *scene)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setRenderHint(QPainter::Antialiasing, true);
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing |
                         QGraphicsView::DontSavePainterState);
    this->setStyleSheet("background-color: transparent;");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void PlymouthGraphicsView::wheelEvent(QWheelEvent *event)
{
    if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier) {
        qreal scaleFactor = std::pow(qreal(2), event->delta() / 240.0);
        qreal currentScale = transform().m11(); // 获取当前的缩放倍数
        // 限制最大为1.1倍
        qreal targetScale = currentScale * scaleFactor;
        if (targetScale > 0.9) {
            scaleFactor = 0.9 / currentScale;
        }else if (targetScale < 0.1) {
            scaleFactor = 0.1 / currentScale;
        }

        scale(scaleFactor, scaleFactor);
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void PlymouthGraphicsView::drawBackground(QPainter *painter, const QRectF &rect)
{
    // 绘制全黑背景
    painter->fillRect(rect, Qt::black);
}
/**
 * @brief 构造函数
 *
 * 创建一个 PlymouthCustomLabel 对象，并设置父对象。
 *
 * @param parent 父对象指针
 */
PlymouthCustomLabel::PlymouthCustomLabel(QWidget *parent) : QLabel(parent), pixmap(), closeIconRect(0, 0, 0, 0)
{
    this->setMouseTracking(true);
    this->setFixedSize(108,108);

    initializeCloseIconRect();
}

/**
 * @brief 初始化关闭图标矩形
 *
 * 初始化关闭图标的矩形大小，根据窗口宽度进行设置。
 */
void PlymouthCustomLabel::initializeCloseIconRect()
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
void PlymouthCustomLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    QPainter painter(this);
    QPen pen(Qt::blue);
    pen.setStyle(Qt::DashLine);
    painter.setPen(pen);

    QBrush brush(QColor(qApp->palette().base().color()));
    painter.setBrush(brush);

    QRect roundedRect = rect().adjusted(1, 1, -1, -1);
    QPainterPath path;
    int radius = 0;
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
void PlymouthCustomLabel::drawCloseIcon(QPainter &painter)
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
void PlymouthCustomLabel::setPixmap(const QPixmap &pixmap)
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
void PlymouthCustomLabel::enterEvent(QEvent *event)
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
void PlymouthCustomLabel::leaveEvent(QEvent *event)
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
void PlymouthCustomLabel::mousePressEvent(QMouseEvent *event)
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
void PlymouthCustomLabel::showCloseIcon()
{
    closeIconPixmap = QIcon::fromTheme("window-close-symbolic").pixmap(10, 10);
    this->update();
}

/**
 * @brief 隐藏关闭图标
 *
 * 将关闭图标的pixmap置空，并重新绘制以清除关闭图标。
 */
void PlymouthCustomLabel::hideCloseIcon()
{
    closeIconPixmap = QPixmap(); // 将关闭图标的pixmap置空
    update(); // 重新绘制以清除关闭图标
}
DropPlyButton::DropPlyButton(QWidget *parent) : QPushButton(parent) {
    setAcceptDrops(true);
}

void DropPlyButton::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void DropPlyButton::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        for (const QUrl &url : urlList) {
            emit droprealPly(url.toLocalFile());
        }
    }
}

plymouthAddPicWidget::plymouthAddPicWidget(QWidget *parent)
{
    this->setFixedSize(160,108);
    this->setAcceptDrops(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QHBoxLayout *btnlayout = new QHBoxLayout();
    QPushButton *button = new QPushButton();
    button->setFixedSize(36,36);
    button->setIcon(QIcon::fromTheme("list-add-symbolic"));
    button->setProperty("isRoundButton", true);

    tip = new QLabel();
    tip->setText(tr("Add video"));
    tip->setAlignment(Qt::AlignHCenter);


    btnlayout->addWidget(button);
    layout->setAlignment(this,Qt::AlignHCenter);
    layout->addStretch(1);
    layout->addLayout(btnlayout);
    layout->addWidget(tip);
    layout->addStretch(1);

    connect(button,&QPushButton::clicked,this,[=](){
        emit buttonclick();
    });
    layout->setAlignment(Qt::AlignCenter);
}

void plymouthAddPicWidget::setType(QString type)
{
    m_type = type;
    if(m_type == "pic"){
        tip->setText(tr("Add picture"));
    }else{
        tip->setText(tr("Add video"));
    }
}

void plymouthAddPicWidget::paintEvent(QPaintEvent *event)
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

void plymouthAddPicWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void plymouthAddPicWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {

        QList<QUrl> urlList = mimeData->urls();
        for (const QUrl &url : urlList) {
            emit dropRealPic(url.toLocalFile());
        }
    }
}
