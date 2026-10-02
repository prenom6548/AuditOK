#include "devicehelper.h"

#include <QDBusInterface>
#include <KF5/KScreen/kscreen/types.h>
#include <KF5/KScreen/kscreen/getconfigoperation.h>
#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/config.h>

#include <QProcess>
#include <X11/extensions/XInput2.h>
#include <X11/Xatom.h>
#include <libudev.h>

static const char* KWIN_DBUS_SERVICE = "org.kde.KWin";
static const char* KWIN_DBUS_INPUT_DEVICE_PATH = "/org/kde/KWin/InputDevice";
static const char* KWIN_DBUS_DEVICE_INTERFACE = "org.kde.KWin.InputDevice";
static const char* KWIN_DBUS_DEVICE_MANAGER_INTERFACE = "org.kde.KWin.InputDeviceManager";

namespace {

QList<Device> getInputDevicesX11()
{
    QList<Device> devices;
    Display* dpy = XOpenDisplay(nullptr);
    int deviceNum = 0;
    XIDeviceInfo* devs = XIQueryDevice(dpy, XIAllDevices, &deviceNum);

    for (int i = 0; i < deviceNum; i++) {
        XIDeviceInfo* dev = &devs[i];
        Device device(dev->name, DeviceType::InputDevice, dev->deviceid);
        devices.append(std::move(device));
    }

    XIFreeDeviceInfo(devs);
    XCloseDisplay(dpy);

    return devices;
}

QString getInputNameX11(int deviceId)
{
    QString inputName;
    QList<Device> devices = getInputDevicesX11();

    for (const Device& device : devices) {
        if(device.deviceId != deviceId) {
            continue;
        }

        inputName = device.name;
    }

    if (inputName.isEmpty()) {
        qWarning() << "The input device name is empty: id = " << deviceId;
    }

    return inputName;
}

QString getInputProductIdX11(int deviceId)
{
    Display* dpy = XOpenDisplay(nullptr);
    if (dpy == nullptr) {
        qWarning() << __FILE__ << __FUNCTION__ << "Open display failed";
        return QString();
    }

    Atom atom = XInternAtom(dpy, "Device Product ID", True);
    Atom actType;
    int actFormat;
    unsigned long nitems, bytesAfter;
    unsigned char *data;
    QString productId = QString();

    int rc = XIGetProperty(dpy, deviceId, atom, 0, 1000, False,
                  XA_INTEGER, &actType, &actFormat,
                  &nitems, &bytesAfter, &data);

    if (rc == Success) {
        QString vId = QString::number(((uint32_t*) data)[0]);
        QString pId = QString::number(((uint32_t*) data)[1]);

        productId = vId + " " + pId;

    } else {
        qWarning() << __FILE__ << __FUNCTION__ << "XIGetProperty failed";
    }

    XFree(data);
    XCloseDisplay(dpy);

    return productId;
}

void mapToOutputX11(int inputDeviceId, const QString& outputName)
{
    QProcess process;
    QStringList arguments = {"--map-to-output", QString::number(inputDeviceId), outputName};
    process.setProgram("xinput");
    process.setArguments(arguments);
    process.start();
    process.waitForFinished();

    QByteArray error = process.readAllStandardError();
    QByteArray message = process.readAllStandardOutput();
    if (!error.isEmpty()) {
        qWarning() << "xinput map-to-output"<< QString::number(inputDeviceId) << outputName
                   << "error:" << error << process.errorString();
    }
    else if (!message.isEmpty()) {
        qWarning() << "xinput map-to-output"<< QString::number(inputDeviceId) << outputName
                   << "error:" << message;
    }
    else {
        qDebug() << "xinput map-to-output"<< inputDeviceId << outputName;
    }
}

void mapToOutputWayland(int inputDeviceId, const QString& outputName)
{
    QDBusInterface interface(KWIN_DBUS_SERVICE,
                             QString(KWIN_DBUS_INPUT_DEVICE_PATH )+ "/event" + QString::number(inputDeviceId),
                             KWIN_DBUS_DEVICE_INTERFACE,
                             QDBusConnection::sessionBus());

    if (!interface.isValid()) {
        qWarning() << "Interface is invalid: service :" << QString(KWIN_DBUS_SERVICE)
                   << "path : " << QString(KWIN_DBUS_INPUT_DEVICE_PATH )+ "/event" + QString::number(inputDeviceId)
                   << "interface: " << QString(KWIN_DBUS_DEVICE_INTERFACE);
        return;
    }
    interface.setProperty("outputName", QVariant(outputName));
}

bool isTouchDevice(XIDeviceInfo *dev)
{
    if(dev == nullptr) {
        return false;
    }
    if(dev->use != XISlavePointer){
        return false;
    }
    if(!dev->enabled) {
        return false;
    }

    bool isTouchDevice = false;
    for (int j = 0; j < dev->num_classes; j++) {
        if (dev->classes[j]->type == XITouchClass) {
            isTouchDevice = true;
            break;
        }
    }
    return isTouchDevice;
}

QList<Device> getX11TouchDevices()
{
    QList<Device> devices;
    Display* dpy = XOpenDisplay(nullptr);
    int deviceNum = 0;
    XIDeviceInfo* devs = XIQueryDevice(dpy, XIAllDevices, &deviceNum);

    for (int i = 0; i < deviceNum; i++) {
        XIDeviceInfo* dev = &devs[i];
        if (!isTouchDevice(dev)) {
            continue;
        }
        Device device(dev->name, DeviceType::InputDevice, dev->deviceid);
        devices.append(std::move(device));
    }

    XIFreeDeviceInfo(devs);
    XCloseDisplay(dpy);

    return devices;
}

KScreen::ConfigPtr getConfigPtr()
{
    KScreen::GetConfigOperation *op = new KScreen::GetConfigOperation;
    if (!op->exec()) {
        qWarning() << op->errorString();
        return KScreen::ConfigPtr();
    }

    return op->config();
}

QString getInputNameWayland(int deviceId)
{
    QDBusInterface interface(KWIN_DBUS_SERVICE,
                             QString(KWIN_DBUS_INPUT_DEVICE_PATH )+ "/event" + QString::number(deviceId),
                             KWIN_DBUS_DEVICE_INTERFACE);

    if (!interface.isValid()) {
        qWarning() << "Interface is invalid: service :" << QString(KWIN_DBUS_SERVICE)
                   << "path : " << QString(KWIN_DBUS_INPUT_DEVICE_PATH )+ "/event" + QString::number(deviceId)
                   << "interface: " << QString(KWIN_DBUS_DEVICE_INTERFACE);
        return QString();
    }

    return interface.property("name").toString();
}

QString getInputProductIdWayland(int deviceId)
{
    QDBusInterface interface(KWIN_DBUS_SERVICE,
                             QString(KWIN_DBUS_INPUT_DEVICE_PATH) + "/event" + QString::number(deviceId),
                             KWIN_DBUS_DEVICE_INTERFACE);

    if (!interface.isValid()) {
        qWarning() << "Interface is invalid: service :" << QString(KWIN_DBUS_SERVICE)
                   << "path : " << QString(KWIN_DBUS_INPUT_DEVICE_PATH )+ "/event" + QString::number(deviceId)
                   << "interface: " << QString(KWIN_DBUS_DEVICE_INTERFACE);
        return QString();
    }

    QVariant PID = interface.property("product");
    QVariant VID = interface.property("vendor");
    QString productId = PID.toString() + " " + VID.toString();

    return productId;
}

bool isWaylandTouchDevice(int deviceId)
{
    QDBusInterface interface(KWIN_DBUS_SERVICE,
                             QString(KWIN_DBUS_INPUT_DEVICE_PATH) + "/event" + QString::number(deviceId),
                             KWIN_DBUS_DEVICE_INTERFACE);

    if (!interface.isValid()) {
        qWarning() << "Interface is invalid: service :" << QString(KWIN_DBUS_SERVICE)
                   << "path : " << QString(KWIN_DBUS_INPUT_DEVICE_PATH )+ "/event" + QString::number(deviceId)
                   << "interface: " << QString(KWIN_DBUS_DEVICE_INTERFACE);
        return false;
    }

    QVariant isTouchDevice = interface.property("touch");
    QVariant isTabletToolDevice = interface.property("tabletTool");
    if (isTouchDevice.toBool() == true
        || isTabletToolDevice.toBool() == true) {
        return true;
    }

    return false;
}

QList<Device> getWaylandTouchDevices()
{
    QList<Device> devices;
    QDBusInterface interface(KWIN_DBUS_SERVICE,
                             KWIN_DBUS_INPUT_DEVICE_PATH,
                             KWIN_DBUS_DEVICE_MANAGER_INTERFACE);

    if (!interface.isValid()) {
        qWarning() << "Interface is invalid: service :" << QString(KWIN_DBUS_SERVICE)
                   << "path : " << QString(KWIN_DBUS_INPUT_DEVICE_PATH )
                   << "interface: " << QString(KWIN_DBUS_DEVICE_MANAGER_INTERFACE);

        return QList<Device>();
    }

    QVariant deviceEvents = interface.property("devicesSysNames");
    QStringList deviceList = deviceEvents.toStringList();

    for (int i = 0; i < deviceList.size(); i++) {
        QString id = deviceList.at(i);
        id.remove("event");
        int deviceId = id.toInt();

        if(!isWaylandTouchDevice(deviceId)) {
            continue;
        }

        QString deviceName = getInputNameWayland(deviceId);
        Device device(deviceName, DeviceType::InputDevice, deviceId);
        devices.append(std::move(device));

    }

    return devices;
}

QString findDeviceNodeFromDeviceIdX11(int deviceId)
{
    Display* dpy = XOpenDisplay(nullptr);
    if (dpy == nullptr) {
        qWarning() << __FILE__ << __FUNCTION__ << "Open display failed";
        return QString();
    }

    Atom atom = XInternAtom(dpy, "Device Node", True);
    Atom actType;
    int actFormat;
    unsigned long nitems, bytesAfter;
    unsigned char *data;
    QString deviceNode = QString();

    int rc = XIGetProperty(dpy, deviceId, atom, 0, 1000, False,
                  AnyPropertyType, &actType, &actFormat,
                  &nitems, &bytesAfter, &data);

    if (rc == Success) {
        deviceNode = reinterpret_cast<const char*>(data);
        deviceNode.remove("/dev/input/");
    } else {
        qWarning() << __FILE__ << __FUNCTION__ << "AnyPropertyType failed";
    }

    XFree(data);
    XCloseDisplay(dpy);

    return deviceNode;
}

QString findDeviceNodeFromDeviceIdWayland(int deviceId)
{
    QString deviceNode = "event" + QString::number(deviceId);
    QDBusInterface interface(KWIN_DBUS_SERVICE,
                             KWIN_DBUS_INPUT_DEVICE_PATH,
                             KWIN_DBUS_DEVICE_MANAGER_INTERFACE);

    if (!interface.isValid()) {
        qWarning() << "Interface is invalid: service :" << QString(KWIN_DBUS_SERVICE)
                   << "path : " << QString(KWIN_DBUS_INPUT_DEVICE_PATH )
                   << "interface: " << QString(KWIN_DBUS_DEVICE_MANAGER_INTERFACE);

        return QString();
    }

    QVariant deviceEvents = interface.property("devicesSysNames");
    QStringList deviceList = deviceEvents.toStringList();

    if (deviceList.indexOf(deviceNode) == -1) {
        return QString();
    }

    return deviceNode;
}

struct udev_device* getUdevDevice(QString deviceNode)
{
    struct udev_device *dev = nullptr;
    struct udev *udev = udev_new();
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);
    struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);

    struct udev_list_entry *devListEntry;
    udev_list_entry_foreach(devListEntry, devices) {
        const char * devicePath = udev_list_entry_get_name(devListEntry);
        QString event = strstr(devicePath, "event");

        if (deviceNode == event) {
            dev = udev_device_new_from_syspath(udev, devicePath);
            break;
        }
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    return dev;
}

} //namespace

QList<Device> DeviceHelper::getTouchDevices()
{
    if (env::isWayland()) {
        return getWaylandTouchDevices();
    }
    return getX11TouchDevices();
}

QString DeviceHelper::getInputName(int deviceId)
{
    if (env::isWayland()) {
        return getInputNameWayland(deviceId);
    }
    return getInputNameX11(deviceId);
}

QString DeviceHelper::getInputProductId(int deviceId)
{
    if (env::isWayland()) {
        return getInputProductIdWayland(deviceId);
    }
    return getInputProductIdX11(deviceId);
}

QString DeviceHelper::findDeviceNodeFromId(int deviceId)
{
    if (env::isWayland()) {
        return findDeviceNodeFromDeviceIdWayland(deviceId);
    }
    return findDeviceNodeFromDeviceIdX11(deviceId);
}

QString DeviceHelper::getInputSerial(int deviceId)
{
    QString deviceNode = findDeviceNodeFromId(deviceId);
    if (deviceNode.isEmpty()) {
        qWarning() << "Not found the deviceNode from id:" << deviceId;
        return QString();
    }

    struct udev_device *dev = getUdevDevice(deviceNode);
    if(dev == nullptr) {
        qWarning() << "Not found the udev device which deviceNode is: " << deviceNode;
        return QString();
    }

    dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
    QString serial = udev_device_get_sysattr_value(dev, "serial");

    udev_device_unref(dev);

    return serial.isEmpty() ? "kydefault" : serial;
}

QList<Device> DeviceHelper::getOutputs()
{
    QList<Device> devices;

    const KScreen::ConfigPtr config = getConfigPtr();
    for (auto &output: config->outputs()) {
        if (!output.data()->isConnected()) {
            continue;
        }

        QString outputName;
        if (env::isWayland()) {
            outputName = output.data()->outputName();
        } else {
            outputName = output.data()->name();
        }

        Device device(outputName, DeviceType::OutputDevice, output.data()->id());
        devices.append(std::move(device));

    }

    return devices;
}

bool DeviceHelper::hasTouchScreen()
{
    if (env::isWayland()) {
        return !getWaylandTouchDevices().isEmpty();
    }
    return !getX11TouchDevices().isEmpty();
}

void DeviceHelper::mapToOutput(int inputDeviceId, const QString& outputName)
{
    if (env::isWayland()) {
        mapToOutputWayland(inputDeviceId, outputName);
        return;
    }
    mapToOutputX11(inputDeviceId, outputName);
}

bool DeviceHelper::isPrimaryOutput(int deviceId)
{
    const KScreen::ConfigPtr config = getConfigPtr();
    KScreen::OutputPtr output = config->output(deviceId);
    return output->isPrimary();
}

QString DeviceHelper::getDeviceSize(int deviceId)
{
    QString deviceNode = findDeviceNodeFromId(deviceId);
    if (deviceNode.isEmpty()) {
        qWarning() << "Not found the deviceNode from deviceId:" << deviceId;
        return QString();
    }

    struct udev_device *dev = getUdevDevice(deviceNode);
    if(dev == nullptr) {
        qWarning() << "Not found the udev device which deviceNode is: " << deviceNode;
        return QString();
    }

    QString width = udev_device_get_property_value(dev, "ID_INPUT_WIDTH_MM");
    QString height = udev_device_get_property_value(dev, "ID_INPUT_HEIGHT_MM");
    QString deviceSize = width + " " + height;

    udev_device_unref(dev);

    return deviceSize;
}
