// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QPlainTextEdit>
#include <QScreen>
#include <QVBoxLayout>
#include <QWidget>

#include <cstddef>
#include <qpa/qplatformnativeinterface.h>
#include <wayland-client-protocol.h>
#include <wayland-util.h>

#include "qlabel.h"
#include "qslider.h"
#include "qwindow.h"
#include "ukui-blur-v1-client-protocol.h"

class SliderWidget : public QWidget
{
    Q_OBJECT
  public:
    // 构造函数：接受一个描述性文字
    SliderWidget(const QString &description, int value, QWidget *parent) : QWidget(parent)
    {
        // 1. 创建组件
        // 描述性标签
        m_descriptionLabel = new QLabel(description, this);
        m_descriptionLabel->setFixedWidth(120); // 固定宽度让布局更整齐

        // 拖动条 (QSlider)
        m_slider = new QSlider(Qt::Horizontal, this); // 设置为水平方向
        m_slider->setRange(0, 100);                   // 设置范围 0 到 100
        m_slider->setValue(value);                    // 初始值设为 value

        // 数值标签 (QLabel)
        // 初始显示拖动条的当前值
        m_valueLabel = new QLabel(QString::number(m_slider->value()), this);
        m_valueLabel->setFixedWidth(30); // 固定宽度防止布局抖动

        // 2. 布局管理
        // 使用水平布局将三个组件放在同一行
        QHBoxLayout *mainLayout = new QHBoxLayout(this);

        // 按照 描述 | 拖动条 | 数值 的顺序添加
        mainLayout->addWidget(m_descriptionLabel);
        mainLayout->addWidget(m_slider);
        mainLayout->addWidget(m_valueLabel);

        // 设置 widget 的布局
        setLayout(mainLayout);

        // 3. 信号与槽连接
        // 连接拖动条的 valueChanged 信号到槽函数，实现实时更新
        connect(m_slider, &QSlider::valueChanged, this, &SliderWidget::onSliderValueChanged);
    }

  Q_SIGNALS:
    void valueChanged(int value);

  private slots:
    // 槽函数：用于接收 QSlider 的 valueChanged 信号
    void onSliderValueChanged(int value)
    {
        // 当拖动条的值改变时，更新数值标签的文本
        m_valueLabel->setText(QString::number(value));
        emit valueChanged(value);
    }

  private:
    QLabel *m_descriptionLabel; // 描述性文字（例如：“音量”）
    QSlider *m_slider;          // 拖动条
    QLabel *m_valueLabel;       // 实时数值显示
};

class Widget : public QWidget
{
    Q_OBJECT
  public:
    Widget(QWidget *parent = nullptr, wl_display *display = nullptr,
           QPlatformNativeInterface *nativeInterface = nullptr,
           ukui_blur_manager_v1 *manager = nullptr)
        : QWidget(parent), wlDisplay(display), native(nativeInterface), ukuiManager(manager)
    {
        // 1. 设置窗口属性以启用透明背景
        setAttribute(Qt::WA_TranslucentBackground, true);

        QVBoxLayout *layout = new QVBoxLayout(this);
        // 实例化并添加带有不同文字说明的拖动条
        SliderWidget *blurAlphaSlider = new SliderWidget("毛玻璃透明度:", 100, this);
        layout->addWidget(blurAlphaSlider);
        // 2. 信号与槽连接,连接拖动条的 valueChanged 信号到槽函数，实现实时更新
        connect(blurAlphaSlider, &SliderWidget::valueChanged, this, &Widget::onSliderValueChanged);
    }

    ~Widget()
    {
        ukui_blur_manager_v1_destroy(ukuiManager);
        wl_display_flush(wlDisplay);
    }

  protected:
    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::PlatformSurface) {
            QPlatformSurfaceEvent *pe = static_cast<QPlatformSurfaceEvent *>(event);
            if (pe->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
                QWindow *window = this->windowHandle();
                if (window && native) {
                    void *surfacePtr = native->nativeResourceForWindow("surface", window);
                    // 将 void* 转换成 Wayland 协议的 wl_surface* 类型
                    wlSurface = static_cast<struct wl_surface *>(surfacePtr);
                    if (!wlSurface) {
                        qDebug() << "Failed to retrieve wl_surface.";
                        return QWidget::event(event);
                    }

                    // 成功获取到 wl_surface
                    // 可以在这里使用 wlSurface 进行 Wayland 相关的操作
                    qDebug() << "Successfully retrieved wl_surface pointer:" << wlSurface;
                    blur_surface = ukui_blur_manager_v1_get_blur(ukuiManager, wlSurface);
                    ukui_blur_surface_v1_set_region(blur_surface, NULL);
                }
            }
        }

        return QWidget::event(event);
    }

  private slots:
    // 槽函数：用于接收 QSlider 的 valueChanged 信号
    void onSliderValueChanged(int value)
    {
        ukui_blur_surface_v1_set_opacity(blur_surface, wl_fixed_from_double((double)value / 100));
    }

  private:
    wl_display *wlDisplay = nullptr;
    QPlatformNativeInterface *native = nullptr;
    ukui_blur_manager_v1 *ukuiManager = nullptr;
    struct wl_surface *wlSurface = nullptr;
    struct ukui_blur_surface_v1 *blur_surface = nullptr;
};

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
                                   const char *interface, uint32_t version)
{
    ukui_blur_manager_v1 **manager = static_cast<ukui_blur_manager_v1 **>(data);
    if (strcmp(interface, ukui_blur_manager_v1_interface.name) == 0) {
        *manager = static_cast<ukui_blur_manager_v1 *>(
            wl_registry_bind(registry, name, &ukui_blur_manager_v1_interface, 2));
    }
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t id)
{
    // do nothing
}

static struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "wayland");
    QApplication app(argc, argv);

    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    if (!native) {
        return -1;
    }

    wl_display *display = reinterpret_cast<wl_display *>(
        native->nativeResourceForIntegration(QByteArrayLiteral("wl_display")));
    if (!display) {
        qWarning("Failed to get Wayland display.");
        return -1;
    }

    qDebug() << "wayland display" << display;
    ukui_blur_manager_v1 *manager = nullptr;
    wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, &manager);
    wl_display_dispatch(display);
    wl_display_roundtrip(display);
    wl_registry_destroy(registry);

    if (!manager) {
        qWarning("Failed to found kywc security protocol.");
        return -1;
    }

    qDebug() << "ukui_blur manager" << manager;
    Widget widget(nullptr, display, native, manager);
    widget.resize(600, 400);
    widget.show();

    return app.exec();
}

#include "ukui_blur.moc"
