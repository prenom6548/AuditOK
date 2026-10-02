#ifndef CURRENTDEVICEINFO_H
#define CURRENTDEVICEINFO_H

#include <QPair>
#include <QList>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QCameraInfo>

struct formatInfo
{
    uint format;
    QString formatString;
    uint width;
    uint height;
    int fps;
};
struct sizeAndfps
{
    QPair<uint,uint> available_size;
    uint fps;
};


class CurrentDeviceInfo
{
public:
    CurrentDeviceInfo();
    static QList<sizeAndfps> available_size;
    static uint available_format;
    static int formatFlag;
    static sizeAndfps current_size;
    static QString current_device_name;
    static QList<QCameraInfo> current_device_list;
    static void sort_resolution();
    static void sort_resolution_tmp();
    static int count;
    static char* current_indevice;
    static QList<sizeAndfps> available_size_tmp;
    static int winHeight;
    static int winWidth;

    //static formatInfo getFormatInfo(v4l2_frmivalenum frmInfo);
    //static QString getFormatString(uint format);
};

#endif // CURRENTDEVICEINFO_H
