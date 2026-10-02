#ifndef DEVICEHELPER_H
#define DEVICEHELPER_H

#include <QString>
#include "common.h"

namespace DeviceHelper {

QList<Device> getOutputs();
QList<Device> getTouchDevices();

QString getInputName(int deviceId);
QString getInputProductId(int deviceId);
QString getInputSerial(int deviceId);

bool hasTouchScreen();
void mapToOutput(int inputDeviceId, const QString& outputName);

bool isPrimaryOutput(int deviceId);
QString findDeviceNodeFromId(int deviceId);
QString getDeviceSize(int deviceId);
}

#endif // DEVICEHELPER_H
