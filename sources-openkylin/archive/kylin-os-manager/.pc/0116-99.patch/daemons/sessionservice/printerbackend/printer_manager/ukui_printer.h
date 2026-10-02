#ifndef UKUI_PRINTER_H
#define UKUI_PRINTER_H

#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QColor>
#include <QVariant>
#include <cups/cups.h>
#include <cups/ppd.h>
#include <vector>
#include <memory>

#include "common.h"

typedef QPair<QPair<QString,QString>,QPair<QString,QString>> conflictPair;

struct ConstrainOption {
    QString name;
    QStringList options;
};
// Set properties enable,shared
enum class UkuiPrinterProperties : int {
    UKUI_PRINTER_PROPERTY_START = 0, // NONE
    UKUI_PRINTER_NAME, // string
    UKUI_PRINTER_DESCRIPTION, // string
    UKUI_PRINTER_LOCATION, // string
    UKUI_PRINTER_DRV_PPD, // string
    UKUI_PRINTER_ENABLE, //"true"/"false"
    UKUI_PRINTER_SHARED, //"true"/"false"
};

enum class UkuiQueueStatus : int {
    UKUI_PRINTER_QUEUE_ACTIVE = 0,
    UKUI_PRINTER_QUEUE_FINISHED,
    UKUI_PRINTER_QUEUE_UNFINISHED,
    UKUI_PRINTER_QUEUE_ALL
};

struct PrintJob {
    int id;
    QString printerName;
    QString user;
    QString title;
    UkuiQueueStatus status;
    int jobState;
    time_t createTime;
    time_t completeTime;
    time_t processingTime;
    int size;
    bool preserved;
};

Q_DECLARE_METATYPE(PrintJob)

struct PrinterAdvancedAttribute {
    QString key;
    ipp_tag_t tag;
    QVariant value;
    PrinterAdvancedAttribute() { tag = IPP_TAG_CUPS_INVALID; }
    PrinterAdvancedAttribute(QString K,ipp_tag_t T,QVariant V) {
		key = K, tag = T, value = V;
	}
};

struct PrinterOptionalAttribute {
    // QString key;
    QVariant value;
    QVariant supportedValue;
};

struct PrinterModifableAttr {
    int state; // Can not modify
    bool isShared;
    bool isAcceptJobs;
    QString printerInfo;
    QString location;
    QString colorMode;
    int copies;
    QString sides;
    QString mediaSize;
};

struct AttrName{
    QString name;
    QString displayName;
    bool operator==(const struct AttrName& src) const {
        return (this->name.compare(src.name)==0)&&(this->displayName.compare(src.displayName)==0);
    }
};

struct SelectableProperty {
    struct AttrName keyword;
    struct AttrName current;
    QList<struct AttrName> supported;
    bool operator==(const SelectableProperty& src) const
    {
        return this->keyword == src.keyword && this->current == src.current && this->supported == src.supported;
    }
};

struct InkProperty {
    InkProperty(){
        color = QColor();
        name = QString();
        type = QString();
        level = 0;
    };
    ~InkProperty(){};
    QColor color;
    QString name;
    QString type;
    int level;
    friend QDebug operator<<(QDebug debug, const InkProperty&);
};

class UkuiUsbPrinter {
public:
    UkuiUsbPrinter()
    { /*qDebug() << "Printer Create...";*/
    }
    ~UkuiUsbPrinter()
    { /*qDebug() << "Printer Destroy..."; */
    }

public:
    // Dest
    QString name = QString(""); // Destination name; RO
    QString printer_uri = QString(""); // Printer Uri : like ipp://localhost:631/printers/xxx RO
    QString device_uri = QString(""); // Device Uri : like usb://xxxx/xxxx?serial=xxxx RO
    QString make_and_model = QString(""); // Printer Model RO
    QString connect_status = QString(""); // RO
    QString location = QString(""); // RW
    QString info = QString("");
    bool is_default = false; // RW
    // PPD
    // Need Save
    QString ppdName = QString("");
    QString vendor = QString(""); // RO
    QString product = QString(""); // RO
    //Need Call checkConflict when attribute changes.
    QList<SelectableProperty> flexibleOptions;
    QList<InkProperty> inkOptions;

    bool operator==(const UkuiUsbPrinter& src) const
    {
        return this->name == src.name && this->printer_uri == src.printer_uri && this->device_uri == src.device_uri
            && this->make_and_model == src.make_and_model && this->connect_status == src.connect_status && this->location == src.location
            && this->is_default == src.is_default && this->ppdName == src.ppdName && this->vendor == src.vendor && this->product == src.product
            && this->flexibleOptions == src.flexibleOptions;
    }
};

class  UkuiPrinterManager : public QObject {
    Q_OBJECT
    // Meyers singleton
public:
    static UkuiPrinterManager& getInstance()
    {
        static UkuiPrinterManager instance;
        return instance;
    }
    void setHostAndPort(QString host, int port);
    // Set ppd and some properties to a specified printer
    bool addPrinter(const QString& usbUri, const QString& printerName, const QString& ppdName, const QString& location, QString& errorString);
    // Remove specified printer
    bool removePrinter(const QString& printerName);
    bool printTestPage(const QString& printerName);
    // Get all printers with cups options
    void enumPrinters(QList<UkuiUsbPrinter>& printers);
    void enumUsbPrinters(QList<UkuiUsbPrinter>& printers);
    bool printerIsInstalled(const QString &uri, bool needEnumPrinters = true, const QList<UkuiUsbPrinter>& printerNameList = QList<UkuiUsbPrinter>());
    CommonStatus printerNameCheck(const QString &printerName = QString(), bool needEnumPrinters = true, const QList<UkuiUsbPrinter>& printerNameList = QList<UkuiUsbPrinter>(), const QString &oldPrinterName = QString());
    //设置成功返回true，设置失败返回false
    bool setDefaultPrinter(QString printerName);
    QString getDefaultPrinter();
    bool setPrinterConfig(const UkuiUsbPrinter& printer);
    // TODO: Duplicate printer, add a new printer with old ppd ,location,description
    bool duplicatePrinter(const QString newName,const QString oldName);
    //开始修改属性
    bool startModifyAttr(const QString &printerName, UkuiUsbPrinter &printer);
    //修改属性，返回冲突属性数量，conflicts是冲突的属性键值对
    int modifyPrinterAttr(const QString &key,const QString &value,QMap<QString,QString> &conflicts);
    //获取和包括<key>属性的PPD冲突结果
    bool getConflictResults(const QString &key,QList<conflictPair> &conflicts);
    //修改位置，不涉及属性冲突检测
    void modifyLocation(const QString &location);
    void modifyInfo(const QString& info);
    //停止修改属性，保存或取消, 保存PPD, 保存Dest信息
    void endModifyAttr(const UkuiUsbPrinter &printer, bool needSave, bool needSaveUri);

    void getPrintQueue(QString printerName, UkuiQueueStatus status, QList<PrintJob>& retJobs);
    bool restartJob(const int &jobId);
    bool cancelJob(const QString &printerName, const int &jobId);
    bool deleteJob(const QString &printerName, const int &jobId);
    bool moveJob(const QString &from, const QString &to, const int &jobId);
    bool moveAllJob(const QString &from, const QString &to);
    bool getJobAttributes(int jobId = 0, PrintJob *job = nullptr, QMap<QString, QVariant> *jobProperties = nullptr);
    bool holdJob(const int &jobId);
    bool releaseJob(const int &jobId);
    bool setJobHoldUntil(const int &jobId, const QString &jobHoldUntil);
    QString generatePrinterName();
/// Not Used
    void getAdvancedPrinterProperties(UkuiUsbPrinter printer, QMap<QString, QVariant>& properties);
    bool modifyPrinterAttributes(UkuiUsbPrinter printer, QMap<QString, QVariant>& newProperties);
    void changeProperties(QStringList& properties);
    // TODO:
    //判断打印机名称是否合法
    bool isPrinterNameValid(QString) { return false; }
    // void getPaperSize(const QString printer_uri, QString& curSize, QStringList& allSize);
    void getPaperSize(const QString printer_uri, SelectableProperty& media, SelectableProperty& sides, SelectableProperty& color);
    void setPaperSize(const QString printer_uri, const QString newSize);
    bool getCupsDebugLogging();
    void setCupsDebugLogging(bool flag);
    bool getPrinterShared(const QString &printerName);
    void setPrinterShared(const QString &printerName, bool flag);
    bool getPrinterEnabled(const QString &printerName);
    void setPrinterEnabled(const QString &printerName, bool flag);
    bool isRemotePrinter(const QString &printerName);
private:
    UkuiPrinterManager(/* args */);
    UkuiPrinterManager(UkuiPrinterManager const&);
    UkuiPrinterManager& operator=(UkuiPrinterManager const&);
    ~UkuiPrinterManager();

    void setLocation(const QString printer_uri, const QString location);
    void setInfo(const QString printer_uri, const QString info);
    void setUri(const QString printer_uri, const QString device_uri);
    void setPrinterPPD(const UkuiUsbPrinter& printer);
    void getPpdInfo(UkuiUsbPrinter& printer);//返回测试
    void getInkInfo(UkuiUsbPrinter& printer);
    bool getOption(ppd_file_t *ppd,ppd_option_t* option, SelectableProperty& property);

    // TODO:Rename Printer, first add a new printer with old ppd,set
    bool renamePrinter(QString oldname, QString newname);
    // Not Used
    // QString: attr name, QVariant attr value
    QVariant getProperty(ipp_attribute_t* attr, int i = 0);
    void generateDefaultOptions();
    // void getAdvancedPrinterProperties(
    //     std::string printer_uri, QMap<QString, QVariant>& properties);
    void getAllProperties(QString printer_uri, QList<PrinterAdvancedAttribute>& printerAttributes);
    void convertProperties(QList<PrinterAdvancedAttribute>, QMap<QString, QVariant>& properties);
    void addIppAttribute(PrinterAdvancedAttribute attribute);
    void getCollection(ipp_attribute_t* attr);
private:
    QString m_host = QString("127.0.0.1");
    QString m_user = QString("");
    QString m_locale = QString("");//语言，默认zh_CN
    http_t* m_http = nullptr;
    int m_port = 631;
    QString m_curFile;//PPD 临时文件
    ppd_file_t* m_ppd = nullptr; //PPD 内容
    bool m_canSave = true;//冲突检测标志位
};
#endif // UKUI_PRINTER_H
