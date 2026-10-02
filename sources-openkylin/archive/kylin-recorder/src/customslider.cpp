#include "customslider.h"
#include "mainwindow.h"
#include <QColor>
#include <QApplication>

CustomSlider::CustomSlider(QWidget *parent) : QSlider(parent)
{
    m_value = 0;
    m_mousePress = false;
    QString themeColor;
    if (MainWindow::mutual->m_themeData != nullptr) {
        themeColor = MainWindow::mutual->m_themeData->get("styleName").toString();
        connect(MainWindow::mutual->m_themeData, &QGSettings::changed, this, &CustomSlider::changeThemeColorSlot);
    }
    initStyle();
    themeStyle(themeColor);
}

void CustomSlider::initStyle()
{
    this->setOrientation(Qt::Horizontal);
    this->setMaximum(500);
}

void CustomSlider::changeThemeColorSlot(const QString &key)
{
    QString themeColor;
    if (key == "styleName") {
        themeColor = MainWindow::mutual->m_themeData->get("styleName").toString();
        themeStyle(themeColor);
    }
}

void CustomSlider::themeStyle(QString themeColor)
{
    QColor highlight = QApplication::palette().highlight().color();
    QColor shadow = QApplication::palette().midlight().color();
    QString highlightStr = QString("#%1%2%3").arg(highlight.red(),2,16,QChar('0')).arg(highlight.green(),2,16,QChar('0')).arg(highlight.blue(),2,16,QChar('0'));
    QString shadowStr = QString("#%1%2%3").arg(shadow.red(),2,16,QChar('0')).arg(shadow.green(),2,16,QChar('0')).arg(shadow.blue(),2,16,QChar('0'));

    if (themeColor == "ukui-dark" || themeColor == "ukui-black") {
        QString style = QString("QSlider::handle:horizontal{height:6px;background-color:transparent;}"
                                "QSlider::groove:horizontal{left:0px;right:0px;height:3px;background-color:%2;}"
                                "QSlider::add-page:horizontal{background-color:%2;}"
                                "QSlider::sub-page:horizontal{background-color:%1;}").arg(highlightStr).arg(shadowStr);
        this->setStyleSheet(style);
    } else {
        QString style = QString("QSlider::handle:horizontal{height:6px;background-color:transparent;}"
                                "QSlider::groove:horizontal{left:0px;right:0px;height:3px;background-color:%2;}"
                                "QSlider::add-page:horizontal{background-color:%2;}"
                                "QSlider::sub-page:horizontal{background-color:%1;}").arg(highlightStr).arg(shadowStr);
        this->setStyleSheet(style);
    }
}

void CustomSlider::mousePressEvent(QMouseEvent *event)
{
#ifndef WHITE_TEST
    //    this.x:控件原点到界面边缘的x轴距离；
    //    globalPos.x：鼠标点击位置到屏幕边缘的x轴距离；
    //    pos.x：鼠标点击位置到本控件边缘的距离；
    //    this.width:本控件的宽度;
    //注意应先调用父类的鼠标点击处理事件，这样可以不影响拖动的情况
    QSlider::mousePressEvent(event);
    m_mousePress = true;
    //获取鼠标的位置，这里并不能直接从ev中取值（因为如果是拖动的话，鼠标开始点击的位置没有意义了）
    double pos = event->pos().x() / (double)width();
    double value = pos * (maximum() - minimum()) + minimum();
    // value + 0.5 四舍五入
    if (value > maximum()) {
        value = maximum();
    }
    if (value < minimum()) {
        value = minimum();
    }
    m_value = value + 0.5;
    //        setValue(m_value);
    qDebug() << "m_value:" << m_value;
    //向父窗口发送自定义事件event type，这样就可以在父窗口中捕获这个事件进行处理
    QEvent evEvent(static_cast<QEvent::Type>(QEvent::User + 1));
    QCoreApplication::sendEvent(parentWidget(), &evEvent);
#endif
}

void CustomSlider::mouseReleaseEvent(QMouseEvent *event)
{
    QSlider::mouseReleaseEvent(event);
    // qDebug()<<"mouseReleaseEvent"<<m_value;
    m_mousePress = false;
    emit sliderReleasedAt(m_value); //抛出有用信号
}

CustomSlider::~CustomSlider()
{
    this->deleteLater();
}
