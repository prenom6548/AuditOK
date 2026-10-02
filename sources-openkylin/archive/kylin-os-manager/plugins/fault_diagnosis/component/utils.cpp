#include <cmath>
#include <QDebug>
#include <QWidget>
#include <QFileInfo>
#include <QFile>
#include <QGSettings/QGSettings>
#include "kom-ukui-gsettings.h"
#include "utils.h"

namespace K {

Utils::Utils() {

}

Utils::~Utils() {

}

double Utils::AdaptPixelSizeToPointSize(double pixel_size) {
    static int default_point_size = 10;

    // pixel size 转 point size
    QWidget widget;
    double dpi = widget.logicalDpiX();
    if (dpi < 1)
        dpi = 96.0;
    double point_size = pixel_size * 72 / dpi;

    // 适配 point size 到当前字号
    return point_size / default_point_size * kom::UkuiGsettings::getInstance()->getFontSize().toDouble();
}

QPixmap Utils::ChangePixmapColor(const QPixmap &pixmap, const QColor &color) {
    auto image = pixmap.toImage();
    for (int i = 0; i < image.width(); i++) {
        for (int j = 0; j < image.height(); j++) {
            auto pixel_color = image.pixelColor(i, j);
            if (pixel_color.alpha() > 0) {
                pixel_color.setRed(color.red());
                pixel_color.setGreen(color.green());
                pixel_color.setBlue(color.blue());
                image.setPixelColor(i, j, pixel_color);
            }
        }
    }

    return QPixmap::fromImage(image);
}

QString Utils::ByteToString(quint64 byte) {
    const char *units[] = {"B", "KB", "MB", "GB"};

    size_t unit_index{0};
    auto size = static_cast<double>(byte);
    while (size > 1024 && unit_index < sizeof(units) / sizeof(units[0]) - 1) {
        size /= 1024;
        unit_index++;
    }
    if (size - std::floor(size) >= 0.1) {
        return QString("%1 %2").arg(QString::number(size, 'f', 1), units[unit_index]);
    } else {
        return QString("%1 %2").arg(static_cast<int>(size)).arg(units[unit_index]);
    }
}

quint64 Utils::CalculateDirSize(const QDir &dir) {
    quint64 total_size{0};

    auto entries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    for (const auto &entry: entries) {
        auto absolute_file_path = dir.absoluteFilePath(entry);
        QFileInfo file_info(absolute_file_path);
        if (file_info.isDir()) {
            total_size += CalculateDirSize(QDir(absolute_file_path));
        } else {
            total_size += file_info.size();
        }
    }
    return total_size;
}

void Utils::RemoveDir(const QString &dir_path) {
    QDir dir(dir_path);
    if (!dir.exists()) {
        qWarning() << "Dir not exist.";
        return;
    }

    auto entries = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (const auto &entry: entries) {
        auto absolute_file_path = dir.absoluteFilePath(entry);
        QFileInfo file_info(absolute_file_path);
        if (file_info.isDir()) {
            RemoveDir(absolute_file_path);
        } else {
            if (!QFile::remove(absolute_file_path)) {
                qWarning() << "Remove file [" << absolute_file_path << "] fail.";
            }
        }
    }

    // 删除目录本身
    dir.rmpath(dir_path);
}

enum Utils::Theme Utils::CurrentTheme() {
    QGSettings s("org.ukui.style", "");
    if (s.keys().contains("styleName")) {
        auto theme_name = s.get("styleName").toString();
        if (theme_name == "ukui-default") {
            return Utils::Theme::kUkuiDefault;
        }
        if (theme_name == "ukui-light") {
            return Utils::Theme::kUkuiLight;
        }
        if (theme_name == "ukui-dark") {
            return Utils::Theme::kUkuiDark;
        }
    }

    return Utils::Theme::kUkuiDefault;
}

QColor Utils::CurrentPlaceholderTextColor() {
    QColor color;
    auto theme = CurrentTheme();
    switch (theme) {
        case K::Utils::Theme::kUkuiDark: {
            color.setRed(255);
            color.setGreen(255);
            color.setBlue(255);
            color.setAlphaF(0.35);
            break;
        }
        default: {
            color.setRed(0);
            color.setGreen(0);
            color.setBlue(0);
            color.setAlphaF(0.35);
            break;
        }
    }

    return color;
}

}
