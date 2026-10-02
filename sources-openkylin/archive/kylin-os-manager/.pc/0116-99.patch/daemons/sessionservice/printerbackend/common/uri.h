#ifndef CUSTOM_URI
#define CUSTOM_URI

#include <QString>
#include <QMap>


enum class DeviceType : int {
    NONE = 0,
    USB,
    IPP,
    SOCKET,
    DRIVERLESS,
    SERIAL_AND_PRALALLEL,
    SMB,
    LPD,
    HTTP,
    IPP_REMOTE,
    DNSSD,
    GUTENPRINT,
    VIRTUAL,
};

const QMap<QString, DeviceType> deviceTypeMap {
    {"none",                DeviceType::NONE},
    {"usb",                 DeviceType::USB},
    {"ipp",                 DeviceType::IPP},
    {"socket",              DeviceType::SOCKET},
    {"driverless",          DeviceType::DRIVERLESS},
    {"serial_and_parallel", DeviceType::SERIAL_AND_PRALALLEL},
    {"smb",                 DeviceType::SMB},
    {"lpd",                 DeviceType::LPD},
    {"virtual",             DeviceType::VIRTUAL},
    {"http",                DeviceType::HTTP},
    {"ipp_remote",          DeviceType::IPP_REMOTE},
    {"dnssd",               DeviceType::DNSSD},
    {"gutenprint",          DeviceType::GUTENPRINT},
};

enum class ProtocolType : int {
    NONE = 0,
    LPD, // lpd://${打印机网络节点}/${打印机服务}
    IPP, // ipp://${IP}:${端口}/ipp/print
    IPPS,
    HTTP, // http://${IP}
    HTTPS,
    SOCKET, // socket://${IP}:${端口}
    DNSSD, // dnssd://${打印机名称}._ipp._tcp.local/?uuid=${UUID}
    SERIAL,
    PARALLEL,
    SMB,
    USB,
    CUPS_PDF,
    CUPS_BRF,
    GUTENPRINT,
};

const QMap<QString, ProtocolType> protocolMap {
    {"none",             ProtocolType::NONE},
    {"lpd",              ProtocolType::LPD},
    {"ipp",              ProtocolType::IPP},
    {"ipps",             ProtocolType::IPPS},
    {"http",             ProtocolType::HTTP},
    {"https",            ProtocolType::HTTPS},
    {"socket",           ProtocolType::SOCKET},
    {"dnssd",            ProtocolType::DNSSD},
    {"serial",           ProtocolType::SERIAL},
    {"parallel",         ProtocolType::PARALLEL},
    {"smb",              ProtocolType::SMB},
    {"usb",              ProtocolType::USB},
    {"cups-pdf",         ProtocolType::CUPS_PDF},
    {"cups-brf",         ProtocolType::CUPS_BRF},
    {"gutenprint53+usb", ProtocolType::GUTENPRINT},
};

class Uri {
private:
    const QString uriSplit = ":/";
public:
    Uri(const QString &str);
    ~Uri();
    QString scheme{QString()};
    ProtocolType protocolType{ProtocolType::NONE};
    DeviceType deviceType{DeviceType::NONE};
    QString driverlessDomain{QString()};
    QString ipAddress{QString()};
private:
    QString m_rawString{QString()};
    DeviceType getDeviceType();
    QString getIpAddress();
};


#endif // CUSTOM_URI