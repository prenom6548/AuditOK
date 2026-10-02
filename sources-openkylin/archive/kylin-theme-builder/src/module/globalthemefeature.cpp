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
#include "globalthemefeature.h"
#include "globalthemewidget.h"
#include <QGraphicsDropShadowEffect>
#include <KWindowEffects>
#include <QPainterPath>
#include <QGraphicsBlurEffect>

/**
 * @brief CustomGraphicsView 构造函数
 *
 * 创建一个 CustomGraphicsView 对象，并设置相应的参数。
 *
 * @param scene QGraphicsScene 对象指针
 */
CustomGraphicsView::CustomGraphicsView(QGraphicsScene* scene) : QGraphicsView(scene) {
    // 设置视图跟随鼠标的变换锚点
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    // 设置拖拽模式为手滚动模式
    setDragMode(QGraphicsView::ScrollHandDrag);
    // 设置抗锯齿渲染提示，开启抗锯齿
    setRenderHint(QPainter::Antialiasing, true);
    // 设置优化标志，不调整抗锯齿优化和保存画家的状态
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing |
                         QGraphicsView::DontSavePainterState);
    // 设置背景色为透明
    this->setStyleSheet("background-color: transparent;");
    // 设置水平滚动条始终隐藏
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 设置垂直滚动条始终隐藏
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

/**
 * @brief 滚轮事件
 *
 * 当触发滚轮事件时，如果按下 Ctrl 键，则执行缩放操作。根据滚轮滚动的距离计算缩放倍数，并限制最大缩放倍数为 1.1 倍。
 * 如果未按下 Ctrl 键，则调用父类的滚轮事件处理函数。
 *
 * @param event 滚轮事件指针
 */
void CustomGraphicsView::wheelEvent(QWheelEvent* event) {
    // 判断是否按下了控制键
    if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier) {
        // 根据滚轮的滚动距离计算缩放因子
        qreal scaleFactor = std::pow(qreal(2), event->delta() / 240.0);
        // 获取当前的缩放倍数
        qreal currentScale = transform().m11(); 
        // 限制最大为1.1倍
        // 计算目标缩放倍数
        qreal targetScale = currentScale * scaleFactor;
        if (targetScale > 1.1) {
            // 如果目标缩放倍数大于1.1，则缩小到1.1倍
            scaleFactor = 1.1 / currentScale;
        }else if (targetScale < 0.3) {
            // 如果目标缩放倍数小于0.3，则放大到0.3倍
            scaleFactor = 0.3 / currentScale;
        }
        // 执行缩放操作
        scale(scaleFactor, scaleFactor);
    } else {
        // 如果不是控制键，则调用QGraphicsView的滚轮事件处理方法
        QGraphicsView::wheelEvent(event);
    }
}

/**
 * @brief 更新全局图像小部件的半径
 *
 * 根据滑块的值更新全局图像小部件的半径。
 *
 * @param sliderValue 滑块的值
 */
void GlobalImageWidget::updateWidgetRadius(int sliderValue) {

    // 将滑块的值赋给半径变量
    int radius = sliderValue;
    // 将半径值设置给显示控件
    m_showwidget->setRadius(radius);
    // 更新显示控件的显示
    m_showwidget->update();
}

/**
 * @brief 更新按钮颜色
 *
 * 根据给定的颜色更新按钮的颜色。
 *
 * @param color 要更新的按钮颜色
 */
void GlobalImageWidget::updateButtonColor(const QColor& color) {
    // 获取m_showbtn的调色板
    QPalette palette = m_showbtn->palette();

    // 设置调色板中的按钮颜色为传入的color
    palette.setColor(QPalette::Button, color);
    // 将修改后的调色板应用到m_showbtn
    m_showbtn->setPalette(palette);

    // 更新m_showbtn的显示
    m_showbtn->update();
}

/**
 * @brief 更新小部件的背景颜色
 *
 * 根据给定的颜色更新小部件的背景颜色。
 *
 * @param color 背景颜色
 */
void GlobalImageWidget::updateWidgetBackgroundColor(const QColor& color)
{
    // 设置显示小部件的背景颜色
    m_showwidget->setBackgroundColor(color);

}

/**
 * @brief 更新壁纸文件路径
 *
 * 根据传入的封面文件路径更新壁纸文件路径。
 *
 * @param WallpaperFilePath 壁纸文件路径
 */
void GlobalImageWidget::updateWallpaper(const QString &WallpaperFilePath)
{
    m_image = QPixmap();
    m_image = QPixmap(WallpaperFilePath);
    m_image = m_image.scaled(1920, 1080, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (m_image.isNull()) {
        qDebug() << "Failed to load image.";
    }

    m_mergedImage = QPixmap(m_image.size());
    m_mergedImage.fill(Qt::transparent);
    QPainter painter(&m_mergedImage);
    painter.drawPixmap(0, 0, m_image);

    QString overlayImagePath;
    if(m_islight){
        overlayImagePath = ":/resource/background/panel-light.png";
    }else{
        overlayImagePath = ":/resource/background/panel-dark.png";
    }
    QPixmap overlayImage(overlayImagePath);
    if (!overlayImage.isNull()) {
        int x = (m_image.width() - overlayImage.width()) / 2;
        int y = m_image.height() - overlayImage.height();
        painter.drawPixmap(x, y, overlayImage);
    } else {
        qDebug() << "Failed to load overlay image.";
    }
    painter.end();

    m_scene->addPixmap(m_mergedImage);
    // 将原代理窗口删除，并清空场景
    QGraphicsProxyWidget* newproxy = m_proxy;
    m_scene->removeItem(m_proxy);
    m_scene->clear();

    // 将合并后的图像添加到场景中，并设置场景大小为合并图像的大小
    m_scene->addPixmap(m_mergedImage);
    m_scene->setSceneRect(m_mergedImage.rect());

    // 调整视图大小，保持长宽比不变
    graphicsView->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);

    // 将原代理窗口重新添加到场景中
    m_scene->addItem(newproxy);
}

/**
 * @brief 更新覆盖图像
 *
 * 根据指定的覆盖图像路径，更新全局图像小部件的覆盖图像。
 *
 * @param overlayImagePath 覆盖图像路径
 */
void GlobalImageWidget::updateOverlayImage(const QString& overlayImagePath) {
    if(":/resource/background/panel-light.png" == overlayImagePath){
        m_islight = true;
    }else{
        m_islight = false;
    }
    // 加载叠加图像
    QPixmap overlayImage(overlayImagePath);

    // 如果叠加图像不为空
    if (!overlayImage.isNull()) {
        // 创建一个与原图像大小相同的透明图像
        m_mergedImage = QPixmap(m_image.size());
        m_mergedImage.fill(Qt::transparent);

        // 创建一个画板，将原图像画到新的合并图像上
        QPainter painter(&m_mergedImage);
        painter.drawPixmap(0, 0, m_image);

        // 计算叠加图像的位置，使其居中显示
        int x = (m_image.width() - overlayImage.width()) / 2;
        int y = m_image.height() - overlayImage.height();
        painter.drawPixmap(x, y, overlayImage);
        painter.end();

        // 将原代理窗口删除，并清空场景
        QGraphicsProxyWidget* newproxy = m_proxy;
        m_scene->removeItem(m_proxy);
        m_scene->clear();

        // 将合并后的图像添加到场景中，并设置场景大小为合并图像的大小
        m_scene->addPixmap(m_mergedImage);
        m_scene->setSceneRect(m_mergedImage.rect());

        // 调整视图大小，保持长宽比不变
        graphicsView->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);

        // 将原代理窗口重新添加到场景中
        m_scene->addItem(newproxy);
    } else {
        qDebug() << "Failed to load overlay image.";
    }
}

/**
 * @brief 更新比例尺
 *
 * 根据当前图像的大小和缩放级别，更新比例尺并重新设置视图。
 */
void GlobalImageWidget::updatescale()
{
    // 重置视图变换
    graphicsView->resetTransform();

    // 定义中心点坐标为(1500, 600)
    QPointF centerPoint(1500, 600);

    // 将视图中心定位到中心点
    graphicsView->centerOn(centerPoint);

    // 缩放视图，缩放比例为1:1，即不进行缩放
    graphicsView->scale(1, 1);
}

void GlobalImageWidget::updatescale1()
{
    graphicsView->resetTransform();
    QPointF centerPoint(1500, 600);
    graphicsView->centerOn(centerPoint);
    graphicsView->scale(0.3, 0.3);
}

void GlobalImageWidget::maxscale()
{
    graphicsView->resetTransform();

    // 获取当前的转换矩阵
    QTransform currentTransform = graphicsView->transform();

    // 从转换矩阵中提取水平和垂直缩放因子
    qreal currentScaleX = currentTransform.m11();
    qreal currentScaleY = currentTransform.m22();

    // 等比例扩大三倍
    qreal newScaleX = currentScaleX * 0.6;
    qreal newScaleY = currentScaleY * 0.6;

    // 将新的缩放因子应用于graphicsView
    graphicsView->scale(newScaleX, newScaleY);
}

void GlobalImageWidget::miniscale()
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
 * @brief 更新控件半径
 *
 * 根据给定的值更新控件的半径。
 *
 * @param Value 半径值
 */
void GlobalImageWidget::updateControlRadius(int Value)
{
    // 设置显示按钮的半径
    m_showbtn->setRadius(Value);
}

/**
 * @brief 更新透明度
 *
 * 根据指定的透明度值更新图像小部件的透明度。
 *
 * @param transparency 透明度值
 */
void GlobalImageWidget::updateTransparency(int transparency)
{
    // 设置m_showwidget的透明度为传入的参数transparency
    m_showwidget->setTransparency(transparency);
    // 更新m_showwidget的显示
    m_showwidget->update();
}

/**
 * @brief GlobalImageWidget构造函数
 *
 * 创建一个GlobalImageWidget对象
 *
 * @param parent 父QWidget对象
 * @param coverFilePath 封面文件路径
 */
GlobalImageWidget::GlobalImageWidget(QWidget *parent, const QString& coverFilePath)
    : QWidget(parent), m_coverFilePath(coverFilePath) {

    m_scene = new QGraphicsScene(this);
    graphicsView = new CustomGraphicsView(m_scene);
    m_proxy =  new QGraphicsProxyWidget();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(graphicsView);
    setLayout(layout);

    m_image = QPixmap(m_coverFilePath);
    m_image = m_image.scaled(1920, 1080, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (m_image.isNull()) {
        qDebug() << "Failed to load image.";
    }

    m_mergedImage = QPixmap(m_image.size());
    m_mergedImage.fill(Qt::transparent);
    QPainter painter(&m_mergedImage);
    painter.drawPixmap(0, 0, m_image);
    QPixmap overlayImage(":/resource/background/panel-light.png");
    if (!overlayImage.isNull()) {
        int x = (m_image.width() - overlayImage.width()) / 2;
        int y = m_image.height() - overlayImage.height();
        painter.drawPixmap(x, y, overlayImage);
    } else {
        qDebug() << "Failed to load overlay image.";
    }
    painter.end();
    m_islight = true;
    m_scene->addPixmap(m_mergedImage);

    m_scene->setSceneRect(m_image.rect());
    graphicsView->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    graphicsView->show();

    m_showwidget = new RoundedWidget();
    m_showwidget->setFixedSize(700,320);
    m_showwidget->setRadius(6);

    QPalette widgetpalette = m_showwidget->palette();
    widgetpalette.setColor(QPalette::Background, QColor(255, 255, 255));
    m_showwidget->setAutoFillBackground(true);
    m_showwidget->setPalette(widgetpalette);

    QHBoxLayout *buttonlayout = new QHBoxLayout();
    m_showbtn = new RoundedButton(m_showwidget);
    m_showbtn->setFixedWidth(120);
    m_showbtn->setParent(m_showwidget);

    QPalette palette = m_showbtn->palette();

    palette.setColor(QPalette::Button, QColor(55, 144, 250));
    m_showbtn->setPalette(palette);

    QPushButton *disablebtn = new QPushButton(m_showwidget);
    disablebtn->setFixedWidth(120);

    buttonlayout->addStretch(1);
    buttonlayout->addWidget(m_showbtn);
    buttonlayout->addWidget(disablebtn);
    m_showwidget->setLayout(buttonlayout);

    m_proxy->setWidget(m_showwidget);
    m_proxy->setPos(800, 400);

    m_scene->addItem(m_proxy);
    // 设置初始缩放倍数为0.9倍
    qreal initialScale = 0.9;
    graphicsView->scale(initialScale, initialScale);

//    QGraphicsBlurEffect* blurEffect = new QGraphicsBlurEffect(this);
//    blurEffect->setBlurRadius(1);
//    m_showwidget->setGraphicsEffect(blurEffect);
}

/**
 * @brief 全局主题功能构造函数
 *
 * 通过给定的封面文件路径创建一个全局图像部件，并将其添加到全局主题功能中。
 *
 * @param parent 父部件指针
 * @param coverFilePath 封面文件路径
 */
Globalthemefeature::Globalthemefeature(QWidget *parent, const QString& coverFilePath)
    : QWidget(parent) {
    GlobalImageWidget* globalImageWidget = new GlobalImageWidget(this, coverFilePath);
}

RoundedWidget::RoundedWidget(QWidget *parent)
    : QWidget(parent), m_radius(0)
{
    setAttribute(Qt::WA_TranslucentBackground);
//    setRadius(6);
    setTransparency(65);

}

void RoundedWidget::setRadius(int radius)
{
    m_radius = radius;
}

void RoundedWidget::setTransparency(int transparency)
{
    m_transparency = transparency / 100.0;
//    qDebug()<<m_transparency;
//    update();
}

void RoundedWidget::setBackgroundColor(const QColor &color)
{
    backgroundColor = color;
//    update();
}

/**
 * @brief 绘制事件
 *
 * 当需要进行绘制时，会调用该函数。
 *
 * @param event 绘制事件指针
 */
void RoundedWidget::paintEvent(QPaintEvent *event)
{
    // 创建QPainter对象，用于绘制图形
    QPainter painter(this);
    // 开启抗锯齿绘制
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 获取控件的内容矩形，并根据边框宽度进行调整
    QRectF rect = contentsRect().adjusted(borderWidth, borderWidth, -borderWidth, -borderWidth);
    // 计算圆角半径
    int r = std::min(rect.width(), rect.height()) / 2;
    // 取实际圆角半径和计算半径的较小值
    int actualRadius = std::min(m_radius, r);
    // 设置背景色的透明度
    backgroundColor.setAlphaF(m_transparency);

    // 绘制背景
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor);
    painter.drawRoundedRect(rect, actualRadius, actualRadius);

//    // 绘制边框
//    painter.setPen(QPen(borderColor, borderWidth));
//    painter.setBrush(Qt::NoBrush);
//    painter.drawRoundedRect(rect, actualRadius, actualRadius);
}

/**
 * @brief RoundedButton 构造函数
 *
 * 创建一个 RoundedButton 对象，并将其初始化为 QPushButton 类型。
 *
 * @param parent 父对象指针
 */
RoundedButton::RoundedButton(QWidget *parent): QPushButton(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setFixedHeight(40);
    setFlat(true);
    setFocusPolicy(Qt::NoFocus);
}

/**
 * @brief 设置圆角半径
 *
 * 设置圆角按钮的半径。
 *
 * @param radius 半径值
 */
void RoundedButton::setRadius(int radius)
{
    m_radius = radius;
    update();
}

/**
 * @brief 绘制事件处理函数
 *
 * 在绘制事件发生时，执行绘制操作。
 *
 * @param event 绘制事件指针
 */
void RoundedButton::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制圆角矩形背景
    QRectF rect(0, 0, width(), height());
    QBrush backgroundBrush(palette().color(QPalette::Button));
    painter.setBrush(backgroundBrush);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect, m_radius, m_radius);

    // 使用按钮原本的样式绘制文本和图标
    QPushButton::paintEvent(event);
}

EditButton::EditButton(QWidget *parent,QString type) : closeIconRect(0, 0, 0, 0)
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
//    this->setAcceptDrops(true);
    m_type = type;

}

void EditButton::showCloseIcon()
{
    closeIconPixmap = QIcon::fromTheme("window-close-symbolic").pixmap(10, 10);
    this->update();
}

void EditButton::hideCloseIcon()
{
    closeIconPixmap = QPixmap(); // 将关闭图标的pixmap置空
    update(); // 重新绘制以清除关闭图标
}

void EditButton::initializeCloseIconRect()
{
    closeIconRect = QRect(width() - 15, 5, 10, 10);
}

void EditButton::drawCloseIcon(QPainter &painter)
{
    if(!closeIconPixmap.isNull()){
        painter.drawPixmap(closeIconRect, closeIconPixmap);
    }
}

void EditButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);
    QPainter painter(this);
    if (!pixmap.isNull()) {
        painter.drawPixmap(rect(), pixmap);
    }
        drawCloseIcon(painter); // 调用绘制关闭图标的函数
}

void EditButton::enterEvent(QEvent *event)
{
    QPushButton::enterEvent(event);
    showCloseIcon(); // 在这个函数中显示“x”图标
//    subbtn->show();
}

void EditButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    hideCloseIcon(); // 在这个函数中隐藏“x”图标
    subbtn->hide();
}

void EditButton::mousePressEvent(QMouseEvent *event)
{

    if (closeIconRect.contains(event->pos())) {
        emit deleteCustomPic();
        hideCloseIcon(); // 在这个函数中隐藏“x”图标
    } else {
        // 处理其他点击事件
        QPushButton::mousePressEvent(event);
    }

}

void EditButton::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void EditButton::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        for (const QUrl &url : urlList) {
//            qDebug() << "File path:" << url.toLocalFile();

            bool islegal = FileCheck::isLegalWallPaperFile(url.toLocalFile(),m_type);
            if(islegal){
                if(m_type == "cover"){
                    emit droprealcover(url.toLocalFile());
                }else{
                    emit droprealwallpaper(url.toLocalFile());
                }
            }
        }
    }
}

ShowLineEdit::ShowLineEdit(QWidget *parent)
{

}

void ShowLineEdit::paintEvent(QPaintEvent *event)
{
    this->setProperty("setDefaultBrush",QBrush(qApp->palette().color(QPalette::Base)));
    this->setProperty("setHoverBrush",QBrush(qApp->palette().color(QPalette::Base)));

    QLineEdit::paintEvent(event);
}

addPicWidget::addPicWidget(QWidget *parent)
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

void addPicWidget::setType(QString type)
{
    m_type = type;
}

void addPicWidget::paintEvent(QPaintEvent *event)
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

void addPicWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void addPicWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {

        QList<QUrl> urlList = mimeData->urls();
        for (const QUrl &url : urlList) {
//            qDebug() << "File path:" << url.toLocalFile();
            bool islegal = FileCheck::isLegalWallPaperFile(url.toLocalFile(),m_type);
            if(islegal){
                //发信号给widget，换图标
                emit dropRealPic(url.toLocalFile());
            }
        }
    }
}

ShowLineEdit1::ShowLineEdit1(QWidget *parent)
{

}

void ShowLineEdit1::focusOutEvent(QFocusEvent *event)
{
    DataCollect::sendGlobalThemeKdkDataAsync(DataCollect::g_TransparencyWidget);
    QString value = this->text();
    if(value.toInt() < 30){
        value = "30";
    }
    this->setText(value);
}
