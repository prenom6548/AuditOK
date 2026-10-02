#include "generatetools.h"

#include <QImage>
#include <QPainter>
#include <QPalette>
#include <QTime>
#include <QGuiApplication>
#include <QEventLoop>
#include <QDebug>
#include <QUrl>
#include <qrencode.h>
#include <string>
#include <iostream>
#include <fstream>
#include <uuid/uuid.h>
#include <sys/statvfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const QString USER_NAME = "kylin";
const int PASSWORD_COUNT = 10;
const std::string CONNECTIVITY_ID_FILEPATH = getenv("HOME") + std::string("/.connectivitycache/ConnectivityId");

GenerateTools::GenerateTools()
{
}

QPixmap GenerateTools::getCode(QString info, int width, int height, bool *ok)
{
    QRcode *qrcode = QRcode_encodeString(info.toStdString().c_str(), 2, QR_ECLEVEL_Q, QR_MODE_8, 1);
    if (nullptr == qrcode) {
        qWarning() << "QR code generation failed! The return value of QRcode_encodeString is null.";
        if (ok != nullptr) {
            *ok = false;
        }
        return QPixmap();
    }
    qint32 temp_width = width;
    qint32 temp_height = height;
    qint32 qrcode_width = qrcode->width > 0 ? qrcode->width : 1;
    double scale_x = (double)temp_width / (double)qrcode_width;
    double scale_y = (double)temp_height / (double)qrcode_width;
    QImage image = QImage(temp_width, temp_height, QImage::Format_ARGB32);
    QPainter painter(&image);

    QColor background(Qt::white);
    painter.setBrush(background);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, temp_width, temp_height);
    QColor foreground(Qt::black);  // 二维码颜色
    painter.setBrush(foreground);
    for (qint32 y = 0; y < qrcode_width; y++) {
        for (qint32 x = 0; x < qrcode_width; x++) {
            unsigned char b = qrcode->data[y * qrcode_width + x];
            if (b & 0x01) {
                QRectF r(x * scale_x, y * scale_y, scale_x, scale_y);
                painter.drawRects(&r, 1);
            }
        }
    }

    if (ok != nullptr) {
        *ok = true;
    }
    return QPixmap::fromImage(image);
}

void GenerateTools::getUserPassword(QStringList &list)
{
    char sourceChar[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
                         'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
                         'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
                         'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

    char buffer[PASSWORD_COUNT] = {0};
    srand(time(NULL));

    for (int i = 0; i < PASSWORD_COUNT; i++) {
        buffer[i] = sourceChar[rand() % 62];
    }

    list << USER_NAME << QString::fromLocal8Bit(buffer, PASSWORD_COUNT);
    return;
}

std::string GenerateTools::getUuid()
{
    std::ifstream file(CONNECTIVITY_ID_FILEPATH);
    if (file.good()) {
        // 文件存在
        std::string uuid;
        std::ifstream idfile;
        idfile.open(CONNECTIVITY_ID_FILEPATH, std::ios_base::in);
        if (idfile.is_open()) {
            getline(idfile, uuid);
        } else {
            qWarning() << QString::fromStdString(CONNECTIVITY_ID_FILEPATH) << " read failed";
        }
        idfile.close();
        return uuid;
    } else {
        // 文件不存在
        char uuid_str[37] = {'\0'};
        uuid_t uu;
        uuid_generate(uu);
        uuid_unparse_lower(uu, uuid_str);

        std::ofstream ofs;
        ofs.open(CONNECTIVITY_ID_FILEPATH, std::ios::out);
        if (ofs.is_open()) {
            ofs << uuid_str << std::endl;
        } else {
            qWarning() << QString::fromStdString(CONNECTIVITY_ID_FILEPATH) << " write failed";
        }
        ofs.close();
        return std::string(uuid_str);
    }
}

void GenerateTools::delayMs(int ms)
{
    QTime dieTime = QTime::currentTime().addMSecs(ms);

    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

void GenerateTools::buriedPoint(kabase::BuriedPoint::PT pt)
{
    kabase::BuriedPoint buriedPoint;
    if (buriedPoint.functionBuriedPoint(kabase::AppName::KylinMobileAssistant, pt)) {
        qCritical() << "Error : buried point fail !";
    };
}

QString GenerateTools::getIptoCode(QString ip)
{
    if (ip.isEmpty()) {
        qInfo() << "The IP entered is empty." << ip;
        return QString("");
    }
    QStringList list = ip.split(".");
    QString request;
    for (int i = 0; i < list.count(); i++) {
        int var = list.value(i).toInt();
        QString hex = QString::number(var, 16);
        if (hex.count() < 2) {
            hex.push_front("0");
        }
        request.append(hex);
    }
    request = request.toUpper();
    qInfo() << "Generated projection code " << request;
    return request;
}

QString GenerateTools::getCodetoIp(QString code)
{
    if (code.isEmpty()) {
        qInfo() << "The code entered is empty.";
        return QString("");
    }
    int index = 0;
    QStringList list;
    while (index < code.count()) {
        QString hex = code.mid(index, 2);
        bool ok;
        int dec = hex.toInt(&ok, 16);
        QString num = QString::number(dec);
        list.append(num);
        index = index + 2;
    }
    QString request = list.join(".");
    qInfo() << "IP conversion result is " << request;
    return request;
}

bool GenerateTools::isEnough(qint64 size)
{
    int state = 0;
    struct statvfs vfs;
    qint64 block_size = 0;
    qint64 free_size = 0;

    state = statvfs("/data", &vfs);
    if (state < 0) {
        qCritical() << "read error!";
        return false;
    }

    block_size = vfs.f_bsize;  // 块大小
    free_size = (vfs.f_bavail * block_size) >> 10;

    return free_size > (size >> 10);
}

QString GenerateTools::intSizeToString(qint64 size)
{
    QString text;
    if (size < 1024) {
        text = QString("%1B").arg(QString::number(size / 1.0, 'f', 1));
        return text;
    } else if (size < 1024 * 1024) {
        text = QString("%1KB").arg(QString::number(size / 1024.0, 'f', 1));
        return text;
    } else if (size < 1024 * 1024 * 1024) {
        text = QString("%1MB").arg(QString::number(size / 1024.0 / 1024.0, 'f', 1));
        return text;
    }
    text = QString("%1GB").arg(QString::number(size / 1024.0 / 1024.0 / 1024.0, 'f', 1));

    return text;
}

bool GenerateTools::isPlugKeyboard()
{
    FILE *fp;  // 文件指针

    if (NULL == (fp = popen("lsusb | grep -i keyboard", "r"))) {
        qWarning() << "open lsusb failed!";
        return false;
    }

    while (!feof(fp)) {
        char buffer[BUFSIZ + 1] = "";
        fgets(buffer, sizeof(buffer), fp);
        if (0 == strlen(buffer)) {
            pclose(fp);
            return false;
        } else {
            pclose(fp);
            return true;
        }
    }

    pclose(fp);
    return false;
}