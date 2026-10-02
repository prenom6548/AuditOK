// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QPlatformSurfaceEvent>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSocketNotifier>
#include <QTransform>
#include <QVBoxLayout>
#include <QWidget>

#include <qpa/qplatformnativeinterface.h>
#include <wayland-client-protocol.h>
#include <wayland-util.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <drm_fourcc.h>
#include <gbm.h>

#include "linux-dmabuf-v1-client-protocol.h"
#include "qwindow.h"
#include "ukui-background-v1-client-protocol.h"

class Widget; // 前向声明

// 存储 output 信息的结构体
struct OutputInfo {
    uint32_t registry_name; // wl_registry 中的 name
    struct wl_output *output;
    QString output_name; // wl_output 的 name 事件返回的名字
    QString manufacturer;
    QString model;
    QComboBox *combo = nullptr; // 关联的 QComboBox
    Widget *widget = nullptr;   // 关联的 Widget
};

// 存储 buffer 类型
enum BufferType { BUFFER_TYPE_SHM, BUFFER_TYPE_DMABUF };

// 当前 buffer 信息
struct BufferInfo {
    struct wl_buffer *buffer = nullptr;
    int32_t width = 0;
    int32_t height = 0;
    int32_t stride = 0;
    BufferType bufferType = BUFFER_TYPE_SHM;
    // dma-buf 相关资源
    int dmabuf_fd = -1;
    struct gbm_bo *gbm_bo = nullptr;
};

class Widget : public QWidget
{
    Q_OBJECT
  public:
    Widget(QWidget *parent = nullptr, wl_display *display = nullptr,
           QPlatformNativeInterface *nativeInterface = nullptr)
        : QWidget(parent), wlDisplay(display), native(nativeInterface), ukuiManager(nullptr)
    {
        // 初始化 GBM 设备（在构造函数中，不依赖 registry 事件）
        const char *node_paths[] = { "/dev/dri/renderD128", "/dev/dri/renderD129",
                                     "/dev/dri/renderD130" };
        for (const char *path : node_paths) {
            int fd = open(path, O_RDWR | O_CLOEXEC);
            if (fd >= 0) {
                gbmDevice = gbm_create_device(fd);
                if (gbmDevice) {
                    qDebug() << "Created GBM device from" << path;
                    break;
                }
                ::close(fd);
            }
        }

        // 设置窗口背景为灰色
        setStyleSheet("background-color: rgb(236, 233, 233);");

        QHBoxLayout *mainLayout = new QHBoxLayout(this);

        // 左侧：图片预览区域
        QWidget *leftPanel = new QWidget(this);
        QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
        leftLayout->setContentsMargins(0, 0, 0, 0);

        QLabel *previewLabel = new QLabel("图片预览:", this);
        leftLayout->addWidget(previewLabel);

        QScrollArea *scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setMinimumSize(200, 300);

        imagePreview = new QLabel(this);
        imagePreview->setAlignment(Qt::AlignCenter);
        imagePreview->setStyleSheet("background-color: white; border: 1px solid gray;");
        imagePreview->setMinimumSize(180, 240);
        imagePreview->installEventFilter(this); // 安装事件过滤器以监听大小变化
        scrollArea->setWidget(imagePreview);

        leftLayout->addWidget(scrollArea);
        mainLayout->addWidget(leftPanel);

        // 右侧：控制区域
        QWidget *rightPanel = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(rightPanel);
        layout->setContentsMargins(0, 0, 0, 0);
        mainLayout->addWidget(rightPanel);

        // 背景类型下拉框
        QLabel *typeLabel = new QLabel("背景类型:", this);
        layout->addWidget(typeLabel);

        bgTypeCombo = new QComboBox(this);
        bgTypeCombo->addItem("桌面壁纸", UKUI_BACKGROUND_MANAGER_V1_TYPE_WALLPAPER);
        bgTypeCombo->addItem("锁屏壁纸", UKUI_BACKGROUND_MANAGER_V1_TYPE_SCREENLOCK);
        bgTypeCombo->addItem("非法类型测试", 3);
        bgTypeCombo->setCurrentIndex(0);
        layout->addWidget(bgTypeCombo);
        connect(bgTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &Widget::onBackgroundTypeChanged);

        // 输出选择下拉框
        QLabel *outputLabel = new QLabel("选择输出:", this);
        layout->addWidget(outputLabel);

        outputCombo = new QComboBox(this);
        outputCombo->addItem("未选择输出", -1);
        layout->addWidget(outputCombo);
        connect(outputCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &Widget::onOutputChanged);

        // 选择图片文件按钮（带 buffer 类型选择）
        QHBoxLayout *selectFileLayout = new QHBoxLayout();
        QPushButton *selectFileBtn = new QPushButton("1. 选择图片文件", this);
        selectFileLayout->addWidget(selectFileBtn, 1);
        connect(selectFileBtn, &QPushButton::clicked, this, &Widget::onSelectFileClicked);

        QLabel *bufferTypeLabel = new QLabel("Buffer 类型:", this);
        selectFileLayout->addWidget(bufferTypeLabel);

        shmRadio = new QRadioButton("共享内存", this);
        shmRadio->setChecked(true);
        selectFileLayout->addWidget(shmRadio);

        dmaRadio = new QRadioButton("dma-buf", this);
        dmaRadio->setChecked(false);
        dmaRadio->setEnabled(gbmDevice != nullptr); // 根据 gbmDevice 是否初始化决定
        selectFileLayout->addWidget(dmaRadio);

        selectFileLayout->addStretch();
        layout->addLayout(selectFileLayout);

        // 旋转选项复选框
        rotateCheckBox = new QCheckBox("顺时针旋转 90 度", this);
        rotateCheckBox->setChecked(false);
        layout->addWidget(rotateCheckBox);
        connect(rotateCheckBox, &QCheckBox::stateChanged, this, &Widget::onRotateChanged);

        // 创建背景按钮
        createBtn = new QPushButton("2. 创建背景对象 (create_background)", this);
        layout->addWidget(createBtn);
        connect(createBtn, &QPushButton::clicked, this, &Widget::onCreateBackgroundClicked);

        // 设置缓冲区按钮
        setBufferBtn = new QPushButton("3. 设置缓冲区 (set_buffer)", this);
        layout->addWidget(setBufferBtn);
        setBufferBtn->setEnabled(false);
        connect(setBufferBtn, &QPushButton::clicked, this, &Widget::onSetBufferClicked);

        // 销毁背景对象按钮
        destroyBtn = new QPushButton("4. 销毁背景对象 (destroy)", this);
        layout->addWidget(destroyBtn);
        destroyBtn->setEnabled(false);
        connect(destroyBtn, &QPushButton::clicked, this, &Widget::onDestroyBackgroundClicked);

        // 销毁管理器按钮
        QPushButton *destroyManagerBtn = new QPushButton("5. 销毁管理器 (destroy manager)", this);
        layout->addWidget(destroyManagerBtn);
        connect(destroyManagerBtn, &QPushButton::clicked, this, &Widget::onDestroyManagerClicked);

        // 状态标签
        statusLabel = new QLabel("状态：就绪", this);
        statusLabel->setWordWrap(true);
        layout->addWidget(statusLabel);
    }

    ~Widget() override
    {
        if (background) {
            ukui_background_v1_destroy(background);
        }
        if (currentBuffer.buffer) {
            wl_buffer_destroy(currentBuffer.buffer);
        }
        // 清理 dma-buf 资源
        if (currentBuffer.gbm_bo) {
            gbm_bo_destroy(currentBuffer.gbm_bo);
        }
        if (currentBuffer.dmabuf_fd >= 0) {
            ::close(currentBuffer.dmabuf_fd);
        }
        if (ukuiManager) {
            ukui_background_manager_v1_destroy(ukuiManager);
        }
        if (shm) {
            wl_shm_destroy(shm);
        }
        if (linuxDmabuf) {
            zwp_linux_dmabuf_v1_destroy(linuxDmabuf);
        }
        if (gbmDevice) {
            gbm_device_destroy(gbmDevice);
        }
        // 清理 registry 和 socket notifier
        if (socketNotifier) {
            delete socketNotifier;
        }
        if (wlRegistry) {
            wl_registry_destroy(wlRegistry);
        }
        // 清理 outputs
        // wl_output 由 wl_registry 管理，不需要手动销毁
        outputs.clear();
        wl_display_flush(wlDisplay);
    }

    // 设置 wl_registry 监听器数据并启动事件监听
    void setRegistryData(wl_registry *registry)
    {
        wlRegistry = registry;
        wl_registry_add_listener(registry, &registry_listener, this);

        // 设置 socket notifier 以持续监听 Wayland 事件
        int fd = wl_display_get_fd(wlDisplay);
        socketNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
        connect(socketNotifier, &QSocketNotifier::activated, this, &Widget::handleWaylandEvents);
    }

    // 获取 ukui_background_manager_v1 实例
    ukui_background_manager_v1 *getManager() const { return ukuiManager; }

    // 从图片创建 wl_buffer (使用已旋转好的 displayImage)
    wl_buffer *createBufferFromImage(const QImage &image)
    {
        // 销毁旧的 buffer 及相关资源
        if (currentBuffer.buffer) {
            wl_buffer_destroy(currentBuffer.buffer);
            // 清理 dma-buf 资源
            if (currentBuffer.gbm_bo) {
                gbm_bo_destroy(currentBuffer.gbm_bo);
                currentBuffer.gbm_bo = nullptr;
            }
            if (currentBuffer.dmabuf_fd >= 0) {
                ::close(currentBuffer.dmabuf_fd);
                currentBuffer.dmabuf_fd = -1;
            }
            currentBuffer.buffer = nullptr;
        }

        // 根据选择的 buffer 类型创建对应的 buffer
        if (dmaRadio->isChecked()) {
            return createDmabufBufferFromImage(image);
        } else {
            return createShmBufferFromImage(image);
        }
    }

    // 从共享内存创建 buffer (接收已旋转的 QImage)
    wl_buffer *createShmBufferFromImage(const QImage &image)
    {
        if (image.isNull()) {
            qDebug() << "Failed to create buffer: image is null";
            return nullptr;
        }

        // 转换为 ARGB32 格式
        QImage convertedImage = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

        int32_t width = convertedImage.width();
        int32_t height = convertedImage.height();
        int32_t stride = convertedImage.bytesPerLine();
        int32_t size = stride * height;

        // 创建共享内存
        QString template_path = QString("/tmp/wayland-shm-XXXXXX");
        QByteArray template_bytes = template_path.toLocal8Bit();
        char *template_str = template_bytes.data();
        int fd = mkstemp(template_str);
        if (fd < 0) {
            qDebug() << "Failed to create shared memory file";
            return nullptr;
        }
        ::unlink(template_str);

        if (ftruncate(fd, size) < 0) {
            ::close(fd);
            qDebug() << "Failed to resize shared memory file";
            return nullptr;
        }

        void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (data == MAP_FAILED) {
            ::close(fd);
            qDebug() << "Failed to mmap shared memory";
            return nullptr;
        }

        // 复制图像数据到共享内存
        memcpy(data, convertedImage.constBits(), size);
        munmap(data, size);

        // 创建 wl_shm_pool
        struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
        ::close(fd);

        if (!pool) {
            qDebug() << "Failed to create wl_shm_pool";
            return nullptr;
        }

        // 创建 wl_buffer
        struct wl_buffer *buffer =
            wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
        wl_shm_pool_destroy(pool);

        if (!buffer) {
            qDebug() << "Failed to create wl_buffer";
            return nullptr;
        }

        // 保存当前 buffer 信息
        currentBuffer.buffer = buffer;
        currentBuffer.width = width;
        currentBuffer.height = height;
        currentBuffer.stride = stride;
        currentBuffer.bufferType = BUFFER_TYPE_SHM;

        return buffer;
    }

    // 从 dma-buf 创建 buffer (接收已旋转的 QImage)
    wl_buffer *createDmabufBufferFromImage(const QImage &image)
    {
        if (image.isNull()) {
            qDebug() << "Failed to create buffer: image is null";
            return nullptr;
        }

        // 转换为 ARGB32 格式
        QImage convertedImage = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

        int32_t width = convertedImage.width();
        int32_t height = convertedImage.height();

        // 使用 GBM 创建真正的 DMA-BUF
        if (!gbmDevice) {
            qDebug() << "GBM device not available";
            return nullptr;
        }

        // 创建 GBM BO (Buffer Object) - 使用 LINEAR modifier
        struct gbm_bo *bo =
            gbm_bo_create(gbmDevice, width, height, GBM_FORMAT_ARGB8888, GBM_BO_USE_LINEAR);
        if (!bo) {
            qDebug() << "Failed to create GBM BO";
            return nullptr;
        }

        // 获取 DMA-BUF fd
        int fd = gbm_bo_get_fd(bo);
        if (fd < 0) {
            qDebug() << "Failed to get fd from GBM BO";
            gbm_bo_destroy(bo);
            return nullptr;
        }

        // 获取 stride
        uint32_t gbm_stride = gbm_bo_get_stride(bo);

        // 使用 DRM_FORMAT_MOD_LINEAR 作为 modifier
        // 注意：GBM_BO_USE_LINEAR 创建的 BO 应该使用 LINEAR modifier
        // 但为了兼容性，我们显式使用 DRM_FORMAT_MOD_LINEAR
        uint64_t modifier = DRM_FORMAT_MOD_LINEAR;

        // 映射 GBM BO 并复制图像数据
        uint32_t map_stride = 0;
        void *map_data = nullptr;
        void *data =
            gbm_bo_map(bo, 0, 0, width, height, GBM_BO_TRANSFER_WRITE, &map_stride, &map_data);
        if (!data) {
            qDebug() << "Failed to map GBM BO";
            gbm_bo_destroy(bo);
            ::close(fd);
            return nullptr;
        }

        // 复制图像数据到 GBM BO
        // 注意：map_stride 是以字节为单位的
        int32_t image_stride = convertedImage.bytesPerLine();
        const uint8_t *src = convertedImage.constBits();
        uint8_t *dst = static_cast<uint8_t *>(data);
        for (int y = 0; y < height; y++) {
            memcpy(dst + y * map_stride, src + y * image_stride,
                   qMin(map_stride, (uint32_t)image_stride));
        }

        // 取消映射并同步缓存
        gbm_bo_unmap(bo, map_data);

        // 使用 linux-dmabuf-v1 协议创建 buffer
        struct wl_buffer *buffer = nullptr;

        if (linuxDmabuf) {
            // 创建 buffer params 对象
            struct zwp_linux_buffer_params_v1 *params =
                zwp_linux_dmabuf_v1_create_params(linuxDmabuf);

            if (params) {
                // 添加单个平面 (对于 ARGB32 格式，只有一个平面)
                zwp_linux_buffer_params_v1_add(params, fd, 0, 0, gbm_stride, modifier >> 32,
                                               modifier & 0xFFFFFFFF);

                // 创建 buffer (使用 ARGB8888 格式，flags=0 表示非线性)
                buffer = zwp_linux_buffer_params_v1_create_immed(params, width, height,
                                                                 DRM_FORMAT_ARGB8888, 0);

                zwp_linux_buffer_params_v1_destroy(params);
            }
        }

        // fd 由 linux-dmabuf 协议接管，GBM BO 必须保持存活直到 buffer 被释放
        // 将资源引用保存到 currentBuffer 中，以便后续清理
        ::close(fd); // fd 已被 create_immed 接管，可以关闭（内核会引用计数）
        // 但 GBM BO 必须保留引用 - 简化处理：不保存 BO 引用，依赖内核引用计数

        if (!buffer) {
            qDebug() << "Failed to create dma-buf buffer";
            gbm_bo_destroy(bo); // 创建失败时才销毁
            return nullptr;
        }

        // 保存当前 buffer 信息
        currentBuffer.buffer = buffer;
        currentBuffer.width = width;
        currentBuffer.height = height;
        currentBuffer.stride = gbm_stride;
        currentBuffer.bufferType = BUFFER_TYPE_DMABUF;
        currentBuffer.dmabuf_fd = -1; // fd 已交给内核管理
        currentBuffer.gbm_bo = bo;    // 保留 BO 引用

        return buffer;
    }

    // 添加 output
    void addOutput(uint32_t name, struct wl_output *output)
    {
        OutputInfo info;
        info.registry_name = name;
        info.output = output;
        info.output_name = QString();
        info.manufacturer = "Unknown";
        info.model = "Unknown";
        info.combo = outputCombo; // 关联 combo box
        info.widget = this;       // 关联 widget
        outputs.append(info);

        // 绑定 wl_output listener 来获取 name (需要 version 4+)
        static const struct wl_output_listener output_listener = {
            .geometry = output_handle_geometry,
            .mode = output_handle_mode,
            .done = output_handle_done,
            .scale = output_handle_scale,
            .name = output_handle_name,
            .description = output_handle_description,
        };

        wl_output_add_listener(output, &output_listener, &outputs.last());

        // 更新输出选择下拉框
        outputCombo->clear();
        for (int i = 0; i < outputs.size(); ++i) {
            QString displayName = outputs[i].output_name.isEmpty()
                                      ? QString("输出 %1: %2").arg(i).arg(outputs[i].manufacturer)
                                      : outputs[i].output_name;
            outputCombo->addItem(displayName, i);
        }
    }

    // wl_output listener 回调函数
    static void output_handle_geometry(void *data, struct wl_output *wl_output, int32_t x,
                                       int32_t y, int32_t physical_width, int32_t physical_height,
                                       int32_t subpixel, const char *make, const char *model,
                                       int32_t transform)
    {
        // geometry 中的 make/model 也可以用来识别输出
        OutputInfo *info = static_cast<OutputInfo *>(data);
        if (make && strcmp(make, "")) {
            info->manufacturer = QString(make);
        }
        if (model && strcmp(model, "")) {
            info->model = QString(model);
        }
        Q_UNUSED(x);
        Q_UNUSED(y);
        Q_UNUSED(physical_width);
        Q_UNUSED(physical_height);
        Q_UNUSED(subpixel);
        Q_UNUSED(transform);
    }

    static void output_handle_mode(void *data, struct wl_output *wl_output, uint32_t flags,
                                   int32_t width, int32_t height, int32_t refresh)
    {
        // 可选：记录输出模式信息
        Q_UNUSED(data);
        Q_UNUSED(wl_output);
        Q_UNUSED(flags);
        Q_UNUSED(width);
        Q_UNUSED(height);
        Q_UNUSED(refresh);
    }

    static void output_handle_scale(void *data, struct wl_output *wl_output, int32_t factor)
    {
        // 可选：记录 scale 因子
        Q_UNUSED(data);
        Q_UNUSED(wl_output);
        Q_UNUSED(factor);
    }

    static void output_handle_name(void *data, struct wl_output *wl_output, const char *name)
    {
        OutputInfo *info = static_cast<OutputInfo *>(data);
        info->output_name = QString(name);
        qDebug() << "Output name:" << name;
        Q_UNUSED(wl_output);
    }

    static void output_handle_description(void *data, struct wl_output *wl_output,
                                          const char *description)
    {
        OutputInfo *info = static_cast<OutputInfo *>(data);
        QString desc = QString(description);
        info->manufacturer = desc.split(' ').first();
        info->model = desc;
        qDebug() << "Output description:" << description;
        Q_UNUSED(wl_output);
    }

    static void output_handle_done(void *data, struct wl_output *wl_output)
    {
        // v1 done - 更新 UI
        OutputInfo *info = static_cast<OutputInfo *>(data);
        Q_UNUSED(wl_output);

        // 刷新整个 combo box
        if (info->combo && info->widget) {
            info->combo->clear();
            for (int i = 0; i < info->widget->outputs.size(); ++i) {
                QString displayName =
                    info->widget->outputs[i].output_name.isEmpty()
                        ? QString("输出 %1: %2").arg(i).arg(info->widget->outputs[i].manufacturer)
                        : info->widget->outputs[i].output_name;
                info->combo->addItem(displayName, i);
            }
        }
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
                    wlSurface = static_cast<struct wl_surface *>(surfacePtr);
                    if (!wlSurface) {
                        qDebug() << "Failed to retrieve wl_surface.";
                        return QWidget::event(event);
                    }

                    qDebug() << "Successfully retrieved wl_surface pointer:" << wlSurface;
                }
            }
        }

        return QWidget::event(event);
    }

    // 事件过滤器：监听 imagePreview 大小变化
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (obj == imagePreview && event->type() == QEvent::Resize) {
            if (!sourceImage.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(displayImage);
                pixmap = pixmap.scaled(imagePreview->size(), Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
                imagePreview->setPixmap(pixmap);
            }
        }
        return QWidget::eventFilter(obj, event);
    }

  private slots:
    // 槽函数：处理 Wayland 事件（socket notifier 激活时调用）
    void handleWaylandEvents()
    {
        // 先准备读取
        if (wl_display_prepare_read(wlDisplay) != 0) {
            // 有待处理的事件，直接派发
            wl_display_dispatch_pending(wlDisplay);
            return;
        }
        // 读取事件
        wl_display_read_events(wlDisplay);
        // 派发事件
        wl_display_dispatch_pending(wlDisplay);
    }

    // 槽函数：背景类型改变
    void onBackgroundTypeChanged(int index)
    {
        Q_UNUSED(index);
        if (bgTypeCombo) {
            currentBgType = bgTypeCombo->currentData().toInt();
            QString typeStr = currentBgType == UKUI_BACKGROUND_MANAGER_V1_TYPE_WALLPAPER
                                  ? "桌面壁纸"
                                  : "锁屏壁纸";
            statusLabel->setText(QString("状态：选择的背景类型：%1").arg(typeStr));
        }
    }

    // 槽函数：输出改变
    void onOutputChanged(int index)
    {
        if (index >= 0 && index < outputs.size()) {
            currentOutput = outputs[index].output;
            statusLabel->setText(QString("状态：选择的输出索引：%1").arg(index));
        } else {
            currentOutput = nullptr;
            statusLabel->setText("状态：未选择输出");
        }
    }

    // 槽函数：创建背景对象
    void onCreateBackgroundClicked()
    {
        if (!ukuiManager) {
            statusLabel->setText("状态：错误 - 背景管理器不可用");
            return;
        }

        if (!currentOutput) {
            statusLabel->setText("状态：错误 - 未选择输出");
            return;
        }

        if (background) {
            statusLabel->setText("状态：提示 - 背景对象已存在，请先销毁");
            return;
        }

        // 调用 create_background 请求
        // 注意：如果同一 output 和 type 已存在，会触发 ukui_background_exists 错误
        background =
            ukui_background_manager_v1_create_background(ukuiManager, currentBgType, currentOutput);

        if (background) {
            ukui_background_v1_add_listener(background, &background_listener, this);
            statusLabel->setText("状态：背景对象已创建 (create_background 成功)");
            // 如果已经有 buffer，设置缓冲区按钮立即可用
            if (currentBuffer.buffer) {
                setBufferBtn->setEnabled(true);
            }
            destroyBtn->setEnabled(true);
        } else {
            statusLabel->setText("状态：错误 - 创建背景对象失败");
        }
    }

    // 槽函数：设置缓冲区
    void onSetBufferClicked()
    {
        if (!background) {
            statusLabel->setText("状态：错误 - 背景对象未创建");
            return;
        }

        if (!currentBuffer.buffer) {
            statusLabel->setText("状态：错误 - 请先选择图片文件创建 buffer");
            return;
        }

        // 设置 buffer
        ukui_background_v1_set_buffer(background, currentBuffer.buffer);
        statusLabel->setText(QString("状态：成功设置 buffer (尺寸：%1x%2)")
                                 .arg(currentBuffer.width)
                                 .arg(currentBuffer.height));
    }

    // 槽函数：选择图片文件并创建 buffer
    void onSelectFileClicked()
    {
        if (!shm) {
            statusLabel->setText("状态：错误 - wl_shm 不可用");
            return;
        }

        // 弹出文件选择对话框
        QString filePath = QFileDialog::getOpenFileName(
            this, tr("选择图片文件"), QDir::homePath(),
            tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif);;所有文件 (*)"));

        if (filePath.isEmpty()) {
            statusLabel->setText("状态：已取消选择文件");
            return;
        }

        // 加载并保存原始图片
        sourceImage = QImage(filePath);
        if (sourceImage.isNull()) {
            statusLabel->setText(QString("状态：错误 - 无法加载图片：%1").arg(filePath));
            return;
        }

        // 更新图片预览
        updateImagePreview();

        // 使用 displayImage 创建 buffer (已经是旋转后的)
        if (!createBufferFromImage(displayImage)) {
            statusLabel->setText(QString("状态：错误 - 创建 buffer 失败：%1").arg(filePath));
            return;
        }

        selectedImagePath = filePath;
        QString bufferTypeStr =
            currentBuffer.bufferType == BUFFER_TYPE_DMABUF ? "dma-buf" : "共享内存";
        statusLabel->setText(QString("状态：已创建 buffer (图片：%1, 类型：%2, 尺寸：%3x%4)")
                                 .arg(filePath)
                                 .arg(bufferTypeStr)
                                 .arg(currentBuffer.width)
                                 .arg(currentBuffer.height));

        // 如果背景对象已存在，启用设置缓冲区按钮
        if (background) {
            setBufferBtn->setEnabled(true);
        }
    }

    // 槽函数：销毁背景对象
    void onDestroyBackgroundClicked()
    {
        if (!background) {
            statusLabel->setText("状态：提示 - 背景对象不存在");
            return;
        }

        // 调用 destroy 请求
        ukui_background_v1_destroy(background);
        background = nullptr;
        // 重置按钮状态
        setBufferBtn->setEnabled(false); // 设置缓冲区按钮不可用（背景对象已销毁）
        destroyBtn->setEnabled(false);   // 销毁按钮不可用（背景对象已销毁）
        statusLabel->setText("状态：背景对象已销毁 (destroy 请求已发送)");
    }

    // 槽函数：销毁管理器
    void onDestroyManagerClicked()
    {
        if (!ukuiManager) {
            statusLabel->setText("状态：提示 - 管理器不存在");
            return;
        }

        // 先销毁背景对象
        if (background) {
            ukui_background_v1_destroy(background);
            background = nullptr;
        }

        // 销毁管理器
        ukui_background_manager_v1_destroy(ukuiManager);
        ukuiManager = nullptr;
        // 重置按钮状态
        setBufferBtn->setEnabled(false);
        destroyBtn->setEnabled(false);
        statusLabel->setText("状态：管理器已销毁 (destroy 请求已发送)");
    }

    // 槽函数：旋转选项改变
    void onRotateChanged()
    {
        // 重新加载图片并应用旋转
        if (!sourceImage.isNull()) {
            updateImagePreview();
            // 如果已经有 buffer，需要重新创建
            if (!selectedImagePath.isEmpty() && currentBuffer.buffer) {
                if (createBufferFromImage(displayImage)) {
                    statusLabel->setText(QString("状态：已重新创建 buffer (旋转：%1)")
                                             .arg(rotateCheckBox->isChecked() ? "90°" : "0°"));
                } else {
                    statusLabel->setText(
                        QString("状态：错误 - 创建 buffer 失败：%1").arg(selectedImagePath));
                }
            }
        }
    }

    // 更新图片预览
    void updateImagePreview()
    {
        if (sourceImage.isNull()) {
            imagePreview->clear();
            displayImage = QImage();
            return;
        }

        // 根据复选框状态决定是否旋转
        displayImage = sourceImage;
        if (rotateCheckBox->isChecked()) {
            QTransform transform;
            transform.rotate(90); // 顺时针旋转 90 度
            displayImage = displayImage.transformed(transform, Qt::SmoothTransformation);
        }

        // 缩放图片以铺满预览区域 (等比拉伸，宽或高其中一个铺满)
        QPixmap pixmap = QPixmap::fromImage(displayImage);
        pixmap = pixmap.scaled(imagePreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imagePreview->setPixmap(pixmap);
    }

  private:
    // wl_registry 监听器回调
    static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
                                       const char *interface, uint32_t version)
    {
        Widget *widget = static_cast<Widget *>(data);
        if (strcmp(interface, wl_output_interface.name) == 0) {
            // 绑定到 version 4 以获取 name 和 description 事件
            uint32_t output_version = qMin(version, 4u);
            struct wl_output *output = static_cast<struct wl_output *>(
                wl_registry_bind(registry, name, &wl_output_interface, output_version));
            widget->addOutput(name, output);
            qDebug() << "Found wl_output:" << name << "(version:" << output_version << ")";
        } else if (strcmp(interface, wl_shm_interface.name) == 0) {
            // 绑定 wl_shm
            widget->shm = static_cast<struct wl_shm *>(
                wl_registry_bind(registry, name, &wl_shm_interface, 1));
            qDebug() << "Found wl_shm:" << name;
        } else if (strcmp(interface, zwp_linux_dmabuf_v1_interface.name) == 0) {
            // 绑定 linux-dmabuf-v1 (version 3 支持 create_immed)
            uint32_t dmabuf_version = qMin(version, 3u);
            widget->linuxDmabuf = static_cast<struct zwp_linux_dmabuf_v1 *>(
                wl_registry_bind(registry, name, &zwp_linux_dmabuf_v1_interface, dmabuf_version));
            qDebug() << "Found linux-dmabuf-v1:" << name << "(version:" << dmabuf_version << ")";
        } else if (strcmp(interface, ukui_background_manager_v1_interface.name) == 0) {
            // 绑定 ukui_background_manager_v1
            widget->ukuiManager = static_cast<ukui_background_manager_v1 *>(
                wl_registry_bind(registry, name, &ukui_background_manager_v1_interface, 1));
            qDebug() << "Found ukui_background_manager_v1:" << name;
        }
    }

    // 移除指定的 output
    void removeOutput(uint32_t registry_name)
    {
        for (int i = 0; i < outputs.size(); ++i) {
            if (outputs[i].registry_name == registry_name) {
                qDebug() << "Removing output:" << outputs[i].output_name
                         << "registry name:" << registry_name;
                // 如果当前选中的 output 被移除，需要清理相关资源
                if (currentOutput == outputs[i].output) {
                    // 销毁背景对象（因为 output 已无效）
                    if (background) {
                        ukui_background_v1_destroy(background);
                        background = nullptr;
                        setBufferBtn->setEnabled(false);
                        destroyBtn->setEnabled(false);
                    }
                    currentOutput = nullptr;
                    statusLabel->setText("状态：当前输出已移除，背景对象已销毁");
                }
                // wl_output 由 wl_registry 管理，不需要手动销毁
                outputs.removeAt(i);
                break;
            }
        }

        // 更新输出选择下拉框
        outputCombo->clear();
        if (outputs.isEmpty()) {
            outputCombo->addItem("无可用输出", -1);
        } else {
            for (int i = 0; i < outputs.size(); ++i) {
                QString displayName =
                    outputs[i].output_name.isEmpty()
                        ? QString("输出 %1: %2").arg(i).arg(outputs[i].manufacturer)
                        : outputs[i].output_name;
                outputCombo->addItem(displayName, i);
            }
        }
    }

    static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t id)
    {
        Widget *widget = static_cast<Widget *>(data);
        qDebug() << "Removed global:" << id;
        // 查找并移除对应的 output
        widget->removeOutput(id);
    }

    // ukui_background_v1 监听器回调
    static void background_handle_failed(void *data, struct ukui_background_v1 *ukui_background)
    {
        Widget *widget = static_cast<Widget *>(data);
        qDebug() << "Background failed event received.";
        widget->statusLabel->setText("状态：背景对象已失效 (failed 事件)");
        widget->background = nullptr; // 标记为已销毁
        widget->setBufferBtn->setEnabled(false);
        widget->destroyBtn->setEnabled(false);
        ukui_background_v1_destroy(ukui_background);
    }

    static const struct wl_registry_listener registry_listener;
    static const struct ukui_background_v1_listener background_listener;

    wl_display *wlDisplay = nullptr;
    wl_registry *wlRegistry = nullptr;
    QSocketNotifier *socketNotifier = nullptr;
    QPlatformNativeInterface *native = nullptr;
    ukui_background_manager_v1 *ukuiManager = nullptr;
    struct wl_surface *wlSurface = nullptr;
    struct wl_output *currentOutput = nullptr;
    struct ukui_background_v1 *background = nullptr;
    struct wl_shm *shm = nullptr;
    struct zwp_linux_dmabuf_v1 *linuxDmabuf = nullptr;
    struct gbm_device *gbmDevice = nullptr;
    BufferInfo currentBuffer;
    QString selectedImagePath; // 已选择的图片文件路径

    QList<OutputInfo> outputs;
    QComboBox *bgTypeCombo = nullptr;
    QComboBox *outputCombo = nullptr;
    QLabel *statusLabel = nullptr;
    QRadioButton *shmRadio = nullptr;
    QRadioButton *dmaRadio = nullptr;
    QPushButton *createBtn = nullptr;
    QPushButton *setBufferBtn = nullptr;
    QPushButton *destroyBtn = nullptr;
    QCheckBox *rotateCheckBox = nullptr;
    QLabel *imagePreview = nullptr;
    QImage sourceImage;  // 保存原始图片
    QImage displayImage; // 保存当前显示/使用的图片（已旋转）
    int currentBgType = UKUI_BACKGROUND_MANAGER_V1_TYPE_WALLPAPER;
};

// 定义静态成员
const struct wl_registry_listener Widget::registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

const struct ukui_background_v1_listener Widget::background_listener = {
    .failed = background_handle_failed,
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

    qDebug() << "wayland display:" << display;

    // 创建 Widget
    Widget widget(nullptr, display, native);

    // 遍历注册表：获取 outputs、manager 等
    // registry 由 Widget 管理，在 Widget 析构时销毁
    wl_registry *registry = wl_display_get_registry(display);
    widget.setRegistryData(registry);
    wl_display_dispatch(display);
    wl_display_roundtrip(display);

    // 检查 manager 是否可用
    if (!widget.getManager()) {
        qWarning("Failed to find ukui_background_manager_v1 protocol.");
        return -1;
    }

    widget.resize(700, 500);
    widget.setWindowTitle("UKUI Background 协议测试");
    widget.show();

    return app.exec();
}

#include "ukui_background.moc"
