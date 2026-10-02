// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QCompleter>
#include <QDebug>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QFontComboBox>
#include <QFontMetrics>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QSocketNotifier>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

#include <qpa/qplatformnativeinterface.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "security-client-protocol.h" // 生成的协议头文件

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
                                   const char *interface, uint32_t version)
{
    kywc_security_configuration_v1 **configuration =
        static_cast<kywc_security_configuration_v1 **>(data);

    if (strcmp(interface, kywc_security_configuration_v1_interface.name) == 0) {
        *configuration = static_cast<kywc_security_configuration_v1 *>(
            wl_registry_bind(registry, name, &kywc_security_configuration_v1_interface, 1));
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

    // ---------- 绑定安全配置协议 ----------
    kywc_security_configuration_v1 *configuration = nullptr;
    wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, &configuration);
    wl_display_dispatch(display);
    wl_display_roundtrip(display);
    wl_registry_destroy(registry);

    if (!configuration) {
        qWarning("Failed to found kywc security configuration protocol.");
        // 继续运行，但配置控件将被禁用
    }

    // ---------- 创建主窗口 ----------
    QWidget window;
    window.setWindowTitle("安全启动器 + 配置");

    QVBoxLayout *mainLayout = new QVBoxLayout(&window);
    mainLayout->setSpacing(10);

    // ============================
    // 1. 启动器区域（垂直固定，不拉伸）
    // ============================
    QGroupBox *launcherGroup = new QGroupBox("程序启动器");
    // 设置垂直策略为 Fixed，防止拉伸
    launcherGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *hLayout = new QHBoxLayout(launcherGroup);
    // 恢复默认边距，但保持紧凑间距
    hLayout->setContentsMargins(-1, -1, -1, -1);
    hLayout->setSpacing(6);

    QLineEdit *lineEdit = new QLineEdit;
    lineEdit->setPlaceholderText("请输入程序路径，如：mate-terminal");
    lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // 补全功能
    QCompleter *completer = new QCompleter(&window);
    QFileSystemModel *fsModel = new QFileSystemModel(completer);
    fsModel->setRootPath("/");
    fsModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    completer->setModel(fsModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    lineEdit->setCompleter(completer);

    QPushButton *browseBtn = new QPushButton("浏览...");
    browseBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QPushButton *launchBtn = new QPushButton("启动");
    launchBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QPushButton *clearBtn = new QPushButton("清空");
    clearBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    hLayout->addWidget(lineEdit);
    hLayout->addWidget(browseBtn);
    hLayout->addWidget(launchBtn);
    hLayout->addWidget(clearBtn);

    // 添加到主布局，拉伸因子为 0，保持固定高度
    mainLayout->addWidget(launcherGroup, 0);

    // ---------- 启动器信号连接 ----------
    QObject::connect(browseBtn, &QPushButton::clicked, [&]() {
        QString path =
            QFileDialog::getOpenFileName(&window, "选择可执行文件", QString(), "所有文件 (*)");
        if (!path.isEmpty()) {
            lineEdit->setText(path);
        }
    });

    QObject::connect(launchBtn, &QPushButton::clicked, [&]() {
        QString program = lineEdit->text().trimmed();
        if (program.isEmpty()) {
            QMessageBox::warning(&window, "提示", "请输入程序路径");
            return;
        }

        QProcess *process = new QProcess;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("LAUNCHED_FROM", argv[0]);
        process->setProcessEnvironment(env);

        process->start(program, QStringList());

        if (!process->waitForStarted(1000)) {
            QMessageBox::critical(&window, "错误", QString("无法启动程序：%1").arg(program));
            delete process;
            return;
        }

        QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                         [process](int exitCode, QProcess::ExitStatus status) {
                             Q_UNUSED(exitCode);
                             Q_UNUSED(status);
                             process->deleteLater();
                         });
        QObject::connect(process, &QProcess::errorOccurred,
                         [process](QProcess::ProcessError error) {
                             Q_UNUSED(error);
                             process->deleteLater();
                         });
    });

    QObject::connect(clearBtn, &QPushButton::clicked, [lineEdit]() { lineEdit->clear(); });
    QObject::connect(lineEdit, &QLineEdit::returnPressed, launchBtn, &QPushButton::click);

    // ============================
    // 2. 安全配置区域（可拉伸，占用剩余空间）
    // ============================
    QGroupBox *securityGroup = new QGroupBox("安全保护配置");
    // 允许垂直拉伸
    securityGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *secLayout = new QVBoxLayout(securityGroup);

    // 2.1 模式选择
    QHBoxLayout *modeLayout = new QHBoxLayout;
    modeLayout->addWidget(new QLabel("保护模式:"));
    QComboBox *modeCombo = new QComboBox;
    modeCombo->addItem("透明", KYWC_SECURITY_CONFIGURATION_V1_PROTECTOR_MODE_FILL_NONE);
    modeCombo->addItem("填充颜色", KYWC_SECURITY_CONFIGURATION_V1_PROTECTOR_MODE_FILL_COLOR);
    modeCombo->addItem("填充图像", KYWC_SECURITY_CONFIGURATION_V1_PROTECTOR_MODE_FILL_IMAGE);
    modeLayout->addWidget(modeCombo);
    secLayout->addLayout(modeLayout);

    // 2.2 保护颜色选择
    QHBoxLayout *colorLayout = new QHBoxLayout;
    colorLayout->addWidget(new QLabel("保护颜色 (RGBA):"));
    QPushButton *colorBtn = new QPushButton("选择颜色");
    colorBtn->setStyleSheet("background-color: rgba(255, 0, 0, 128);");
    QLabel *colorLabel = new QLabel("R=255 G=0 B=0 A=128 (预乘)");
    colorLayout->addWidget(colorBtn);
    colorLayout->addWidget(colorLabel);
    secLayout->addLayout(colorLayout);

    uint32_t currentR = 0xFFFFFFFF;
    uint32_t currentG = 0;
    uint32_t currentB = 0;
    uint32_t currentA = 0x80000000;

    auto updateColorButton = [&]() {
        int r = (currentR >> 24) & 0xFF;
        int g = (currentG >> 24) & 0xFF;
        int b = (currentB >> 24) & 0xFF;
        int a = (currentA >> 24) & 0xFF;
        QString style =
            QString("background-color: rgba(%1, %2, %3, %4);").arg(r).arg(g).arg(b).arg(a);
        colorBtn->setStyleSheet(style);
        colorLabel->setText(QString("R=%1 G=%2 B=%3 A=%4 (预乘)").arg(r).arg(g).arg(b).arg(a));
    };
    updateColorButton();

    QObject::connect(colorBtn, &QPushButton::clicked, [&]() {
        QColor initial((currentR >> 24) & 0xFF, (currentG >> 24) & 0xFF, (currentB >> 24) & 0xFF,
                       (currentA >> 24) & 0xFF);
        QColor color = QColorDialog::getColor(initial, &window, "选择保护颜色",
                                              QColorDialog::ShowAlphaChannel);
        if (!color.isValid())
            return;

        currentR = (uint32_t)((color.red() / 255.0) * 0xFFFFFFFF);
        currentG = (uint32_t)((color.green() / 255.0) * 0xFFFFFFFF);
        currentB = (uint32_t)((color.blue() / 255.0) * 0xFFFFFFFF);
        currentA = (uint32_t)((color.alpha() / 255.0) * 0xFFFFFFFF);

        updateColorButton();
    });

    // 2.3 图像文件选择
    QHBoxLayout *imageLayout = new QHBoxLayout;
    imageLayout->addWidget(new QLabel("图像:"));
    QPushButton *imageBtn = new QPushButton("选择图像文件");
    QLabel *imagePreview = new QLabel("未选择");
    imagePreview->setFixedSize(100, 60);
    imagePreview->setStyleSheet("border: 1px solid gray;");
    imageLayout->addWidget(imageBtn);
    imageLayout->addWidget(imagePreview);
    secLayout->addLayout(imageLayout);

    // 2.4 图像生成设置
    QGroupBox *genSettingsGroup = new QGroupBox("图像生成设置");
    QGridLayout *genGrid = new QGridLayout(genSettingsGroup);

    genGrid->addWidget(new QLabel("字体:"), 0, 0);
    QFontComboBox *fontCombo = new QFontComboBox;
    fontCombo->setCurrentFont(QFont("Arial"));
    genGrid->addWidget(fontCombo, 0, 1);

    genGrid->addWidget(new QLabel("文字大小:"), 0, 2);
    QSpinBox *fontSizeSpin = new QSpinBox;
    fontSizeSpin->setRange(6, 200);
    fontSizeSpin->setValue(48);
    genGrid->addWidget(fontSizeSpin, 0, 3);

    genGrid->addWidget(new QLabel("文字颜色:"), 1, 0);
    QPushButton *textColorBtn = new QPushButton;
    textColorBtn->setFixedWidth(50);
    textColorBtn->setStyleSheet("background-color: white;");
    QColor textColor = Qt::white;
    genGrid->addWidget(textColorBtn, 1, 1);

    genGrid->addWidget(new QLabel("背景颜色:"), 1, 2);
    QPushButton *bgColorBtn = new QPushButton;
    bgColorBtn->setFixedWidth(50);
    bgColorBtn->setStyleSheet("background-color: rgba(255, 0, 0, 200);");
    QColor bgColor(255, 0, 0, 200);
    genGrid->addWidget(bgColorBtn, 1, 3);

    genGrid->addWidget(new QLabel("边距:"), 2, 0);
    QSpinBox *marginSpin = new QSpinBox;
    marginSpin->setRange(0, 100);
    marginSpin->setValue(20);
    genGrid->addWidget(marginSpin, 2, 1);

    QObject::connect(textColorBtn, &QPushButton::clicked, [&]() {
        QColor color = QColorDialog::getColor(textColor, &window, "选择文字颜色");
        if (!color.isValid())
            return;
        textColor = color;
        textColorBtn->setStyleSheet(QString("background-color: rgba(%1,%2,%3,%4);")
                                        .arg(color.red())
                                        .arg(color.green())
                                        .arg(color.blue())
                                        .arg(color.alpha()));
    });

    QObject::connect(bgColorBtn, &QPushButton::clicked, [&]() {
        QColor color = QColorDialog::getColor(bgColor, &window, "选择背景颜色",
                                              QColorDialog::ShowAlphaChannel);
        if (!color.isValid())
            return;
        bgColor = color;
        bgColorBtn->setStyleSheet(QString("background-color: rgba(%1,%2,%3,%4);")
                                      .arg(color.red())
                                      .arg(color.green())
                                      .arg(color.blue())
                                      .arg(color.alpha()));
    });

    secLayout->addWidget(genSettingsGroup);

    // 警示文字输入和生成按钮
    QHBoxLayout *genLayout = new QHBoxLayout;
    genLayout->addWidget(new QLabel("警示文字:"));
    QLineEdit *warningTextEdit = new QLineEdit;
    warningTextEdit->setPlaceholderText("输入文字（如：危险！）");
    QPushButton *generateBtn = new QPushButton("生成图像");
    genLayout->addWidget(warningTextEdit);
    genLayout->addWidget(generateBtn);
    secLayout->addLayout(genLayout);

    QImage selectedImage;

    // 生成图像
    QObject::connect(generateBtn, &QPushButton::clicked, [&]() {
        QString text = warningTextEdit->text().trimmed();
        if (text.isEmpty()) {
            text = "WARNING";
        }

        QFont font = fontCombo->currentFont();
        font.setPointSize(fontSizeSpin->value());
        QFontMetrics fm(font);
        QRect textRect = fm.boundingRect(text);
        int margin = marginSpin->value();
        int width = textRect.width() + margin * 2;
        int height = textRect.height() + margin * 2;
        if (width < 64)
            width = 64;
        if (height < 64)
            height = 64;

        QImage img(width, height, QImage::Format_ARGB32);
        img.fill(bgColor);

        QPainter painter(&img);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setFont(font);
        painter.setPen(textColor);
        painter.drawText(img.rect(), Qt::AlignCenter, text);
        painter.end();

        selectedImage = img;
        QPixmap pix = QPixmap::fromImage(img);
        imagePreview->setPixmap(
            pix.scaled(imagePreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        imagePreview->setToolTip(QString("生成图像: %1 (%2x%3)").arg(text).arg(width).arg(height));
    });

    // 文件选择（覆盖生成的图像）
    QObject::connect(imageBtn, &QPushButton::clicked, [&]() {
        QString path = QFileDialog::getOpenFileName(&window, "选择图像文件", QString(),
                                                    "图像文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
        if (path.isEmpty())
            return;

        QImage img(path);
        if (img.isNull()) {
            QMessageBox::warning(&window, "错误", "无法加载图像文件");
            return;
        }
        selectedImage = img;
        QPixmap pix = QPixmap::fromImage(img);
        imagePreview->setPixmap(
            pix.scaled(imagePreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        imagePreview->setToolTip(QString("%1x%2").arg(img.width()).arg(img.height()));
    });

    // 应用按钮
    QPushButton *applyBtn = new QPushButton("应用设置");
    secLayout->addWidget(applyBtn);

    if (!configuration) {
        securityGroup->setEnabled(false);
        securityGroup->setToolTip(
            "协议接口不可用，请确认合成器支持 kywc_security_configuration_v1");
    }

    // 应用设置
    QObject::connect(applyBtn, &QPushButton::clicked, [&]() {
        if (!configuration) {
            QMessageBox::warning(&window, "错误", "安全配置协议未就绪，无法应用设置");
            return;
        }

        // 1. 颜色
        kywc_security_configuration_v1_set_protector_color(configuration, currentR, currentG,
                                                           currentB, currentA);

        // 2. 图像
        if (!selectedImage.isNull()) {
            QImage argb = selectedImage.convertToFormat(QImage::Format_ARGB32);
            if (argb.isNull()) {
                QMessageBox::warning(&window, "错误", "图像转换失败");
                return;
            }

            int width = argb.width();
            int height = argb.height();
            int stride = argb.bytesPerLine();
            int size = stride * height;

            int fd = memfd_create("kywc-security-image", MFD_CLOEXEC);
            if (fd < 0) {
                QMessageBox::warning(&window, "错误", "创建共享内存失败");
                return;
            }

            if (ftruncate(fd, size) < 0) {
                close(fd);
                QMessageBox::warning(&window, "错误", "调整共享内存大小失败");
                return;
            }

            void *map = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (map == MAP_FAILED) {
                close(fd);
                QMessageBox::warning(&window, "错误", "映射共享内存失败");
                return;
            }
            memcpy(map, argb.constBits(), size);
            munmap(map, size);

            kywc_security_configuration_v1_set_protector_image(configuration, fd, width, height);
        } else {
            int mode = modeCombo->currentData().toInt();
            if (mode == KYWC_SECURITY_CONFIGURATION_V1_PROTECTOR_MODE_FILL_IMAGE) {
                QMessageBox::information(&window, "提示",
                                         "未选择图像，将使用默认图像（如果合成器支持）");
            }
        }

        // 3. 模式（最后设置）
        int mode = modeCombo->currentData().toInt();
        kywc_security_configuration_v1_set_protector_mode(configuration, (uint)mode);

        wl_display_flush(display);
        QMessageBox::information(&window, "成功", "安全设置已发送给合成器");
    });

    // 添加到主布局，拉伸因子为 1，占用剩余垂直空间
    mainLayout->addWidget(securityGroup, 1);

    // ---------- 显示窗口 ----------
    window.show();

    return app.exec();
}
