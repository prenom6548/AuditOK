#ifndef CAMERAINFO_H
#define CAMERAINFO_H
#include <QString>

class CameraInfo {
public:
    CameraInfo();

    QString vendor{QString()}; // 供应商
    QString model{QString()}; // 型号
    QString name{QString()};
    QString interface{QString()};
    QString driver{QString()};
    QString version{QString()};
    QString businfo{QString()};
    QString speed{QString()};
};

#endif //CAMERAINFO_H
