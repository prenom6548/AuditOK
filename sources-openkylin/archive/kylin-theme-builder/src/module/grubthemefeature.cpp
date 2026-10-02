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
#include "grubthemefeature.h"
#include <QGraphicsBlurEffect>
GrubThemeFeature::GrubThemeFeature(QWidget *parent) : QWidget(parent)
{

}

GrubGraphicsView::GrubGraphicsView(QGraphicsScene *scene)
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

void GrubGraphicsView::wheelEvent(QWheelEvent *event)
{
    if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier) {
        qreal scaleFactor = std::pow(qreal(2), event->delta() / 240.0);
        qreal currentScale = transform().m11(); // 获取当前的缩放倍数
        // 限制最大为1.1倍
        qreal targetScale = currentScale * scaleFactor;
        if (targetScale > 1.0) {
            scaleFactor = 1.0 / currentScale;
        }else if (targetScale < 0.3) {
            scaleFactor = 0.3 / currentScale;
        }

        scale(scaleFactor, scaleFactor);
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

/**
 * @brief GrubImageWidget 构造函数
 *
 * 创建一个 GrubImageWidget 对象。
 *
 * @param parent 父对象
 */
GrubImageWidget::GrubImageWidget(QWidget *parent)
{
    m_scene = new QGraphicsScene(this);
    graphicsView = new GrubGraphicsView(m_scene);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(graphicsView);
    setLayout(layout);

    graphicsView->setScene(m_scene);

    // 加载并显示背景图片
    QPixmap pixmap(":/resource/background/1-warty-final-ubuntukylin.jpg");
    m_backgroundItem = new QGraphicsPixmapItem(pixmap);
    m_backgroundItem->setData(Qt::UserRole, "background");
    m_scene->addItem(m_backgroundItem);

    // 创建毛玻璃特效并应用于背景图元
    QGraphicsBlurEffect* blurEffect = new QGraphicsBlurEffect;
    blurEffect->setBlurRadius(10);
    m_backgroundItem->setGraphicsEffect(blurEffect);

    // 设置图元居中显示
    QSize viewSize = graphicsView->size();
    QPointF center(viewSize.width() / 2, viewSize.height() / 2);
    m_backgroundItem->setPos(center.x() - pixmap.width() / 2, center.y() - pixmap.height() / 2);


    // 添加新的图元
    m_overlayPixmap.load(":/resource/background/grub-listview.png");
    m_overlayItem = new QGraphicsPixmapItem(m_overlayPixmap);
    m_scene->addItem(m_overlayItem);
    // 放大图元
    qreal scaleRatio = 2.0; // 放大倍数
    m_overlayItem->setScale(scaleRatio);
    // 计算新的图元位置
    qreal scaledWidth = m_overlayPixmap.width() * scaleRatio; // 放大后的宽度
    qreal scaledHeight = m_overlayPixmap.height() * scaleRatio; // 放大后的高度
    m_overlayItem->setPos(center.x() - scaledWidth / 2, center.y() - scaledHeight / 2);


    graphicsView->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    graphicsView->show();

    // 设置初始缩放倍数为0.9倍
    qreal initialScale = 0.1;
    graphicsView->scale(initialScale, initialScale);
}

void GrubImageWidget::maxscale()
{
    graphicsView->resetTransform();

    QTransform currentTransform = graphicsView->transform();

    qreal currentScaleX = currentTransform.m11();
    qreal currentScaleY = currentTransform.m22();

    qreal newScaleX = currentScaleX * 0.6;
    qreal newScaleY = currentScaleY * 0.6;

    graphicsView->scale(newScaleX, newScaleY);
}

void GrubImageWidget::miniscale()
{
    graphicsView->resetTransform();

    QTransform currentTransform = graphicsView->transform();

    qreal currentScaleX = currentTransform.m11();
    qreal currentScaleY = currentTransform.m22();

    qreal newScaleX = currentScaleX * 0.3;
    qreal newScaleY = currentScaleY * 0.3 ;

    graphicsView->scale(newScaleX, newScaleY);
}

/**
 * @brief 更新背景图片
 *
 * 根据给定的图片路径，加载新的背景图片并更新界面显示。
 *
 * @param imagePath 图片路径
 */
void GrubImageWidget::updateBackground(const QString &imagePath)
{
    // 加载新的背景图片
    QPixmap pixmap(imagePath);

    if (pixmap.isNull()) {
//        qDebug() << "Failed to load background image.";·
        return;
    }
    pixmap = pixmap.scaled(1920, 1080, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 移除旧的背景图元
    m_scene->removeItem(m_backgroundItem);
    delete m_backgroundItem;

    // 创建并添加新的背景图元
    m_backgroundItem = new QGraphicsPixmapItem(pixmap);
    m_backgroundItem->setData(Qt::UserRole, "background");
    m_scene->addItem(m_backgroundItem);

    // 创建毛玻璃特效并应用于新的背景图元
    QGraphicsBlurEffect* blurEffect = new QGraphicsBlurEffect;
    blurEffect->setBlurRadius(10);
    m_backgroundItem->setGraphicsEffect(blurEffect);

    // 设置图元居中显示
    QRectF viewRect = graphicsView->viewport()->rect();
    QPointF center(viewRect.center());
    m_backgroundItem->setPos(center.x() - pixmap.width() / 2, center.y() - pixmap.height() / 2);

    // 创建并添加新的overlay图元
    QGraphicsPixmapItem* newOverlayItem = new QGraphicsPixmapItem(m_overlayPixmap);
    m_scene->addItem(newOverlayItem);

    // 移除旧的overlay图元
    m_scene->removeItem(m_overlayItem);
    delete m_overlayItem;

    // 更新overlayItem指针
    m_overlayItem = newOverlayItem;
    // 设置新的overlay图元位置
    qreal scaleRatio = 1.0; // 放大倍数
    qreal scaledWidth = m_overlayPixmap.width() * scaleRatio; // 放大后的宽度
    qreal scaledHeight = m_overlayPixmap.height() * scaleRatio; // 放大后的高度
    newOverlayItem->setPos(center.x() - scaledWidth / 2, center.y() - scaledHeight / 2);
    graphicsView->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    graphicsView->show();

    // 设置初始缩放倍数为0.9倍
    qreal initialScale = 0.4;
    graphicsView->resetTransform();
    graphicsView->scale(initialScale, initialScale);
}

GrubEditButton::GrubEditButton(QWidget *parent) : closeIconRect(0, 0, 0, 0)
{
    this->setMouseTracking(true);
    this->setFixedSize(160,100);
    initializeCloseIconRect();
    subbtn = new QPushButton(this);
    subbtn->hide();
    subbtn->move(this->geometry().center() - subbtn->rect().center());
    connect(subbtn,&QPushButton::clicked,this,[=](){
        emit changePic();
    });
    subbtn->setProperty("useButtonPalette", true);
    subbtn->setText(tr("Import"));
    this->setAcceptDrops(true);
}

void GrubEditButton::showCloseIcon()
{
    closeIconPixmap = QIcon::fromTheme("window-close-symbolic").pixmap(10, 10);
    this->update();
}

void GrubEditButton::hideCloseIcon()
{
    closeIconPixmap = QPixmap(); // 将关闭图标的pixmap置空
    update(); // 重新绘制以清除关闭图标
}

void GrubEditButton::initializeCloseIconRect()
{
    closeIconRect = QRect(width() - 15, 5, 10, 10);
}

void GrubEditButton::drawCloseIcon(QPainter &painter)
{
    if(!closeIconPixmap.isNull()){
        painter.drawPixmap(closeIconRect, closeIconPixmap);
    }
}

void GrubEditButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);
    QPainter painter(this);
    if (!pixmap.isNull()) {
        painter.drawPixmap(rect(), pixmap);
    }
        drawCloseIcon(painter); // 调用绘制关闭图标的函数
}

void GrubEditButton::enterEvent(QEvent *event)
{
    QPushButton::enterEvent(event);
    showCloseIcon(); // 在这个函数中显示“x”图标
//    subbtn->show();
}

void GrubEditButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    hideCloseIcon(); // 在这个函数中隐藏“x”图标
    subbtn->hide();
}

void GrubEditButton::mousePressEvent(QMouseEvent *event)
{

    if (closeIconRect.contains(event->pos())) {
        emit deleteCustomPic();
        hideCloseIcon(); // 在这个函数中隐藏“x”图标
    } else {
        // 处理其他点击事件
        QPushButton::mousePressEvent(event);
    }

}

void GrubEditButton::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void GrubEditButton::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        for (const QUrl &url : urlList) {
//            qDebug() << "File path:" << url.toLocalFile();

            bool islegal = FileCheck::isLegalWallPaperFile(url.toLocalFile(),"grub");
            if(islegal){
                emit droprealgrub(url.toLocalFile());
            }
        }
    }
}

grubAddPicWidget::grubAddPicWidget(QWidget *parent)
{
    this->setFixedSize(160,108);
    this->setAcceptDrops(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QHBoxLayout *btnlayout = new QHBoxLayout();
    QPushButton *button = new QPushButton();
    button->setFixedSize(36,36);
    button->setIcon(QIcon::fromTheme("list-add-symbolic"));
    button->setProperty("isRoundButton", true);

    QLabel *tip = new QLabel();
    tip->setText(tr("Add picture"));
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

void grubAddPicWidget::paintEvent(QPaintEvent *event)
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

void grubAddPicWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void grubAddPicWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {

        QList<QUrl> urlList = mimeData->urls();
        for (const QUrl &url : urlList) {
//            qDebug() << "File path:" << url.toLocalFile();
            bool islegal = FileCheck::isLegalWallPaperFile(url.toLocalFile(),"grub");
            if(islegal){
                //发信号给widget，换图标
                emit dropRealPic(url.toLocalFile());
            }
        }
    }
}
