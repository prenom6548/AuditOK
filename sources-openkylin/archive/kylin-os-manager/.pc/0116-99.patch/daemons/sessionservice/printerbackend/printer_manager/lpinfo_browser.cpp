#include <cups/cups.h>
#include <cups/adminutil.h>
#include <cups/http.h>
#include <cups/cups.h>
#include <cups/ipp.h>
#include <QVariant>
#include <QDebug>
#include "lpinfo_browser.h"
#include "common.h"
#include "udev_devices.h"
#include "ukui_printer.h"
#include "custom_qstring_converter.h"
#include "uri.h"

LpinfoBrowser *LpinfoBrowser::getInstance()
{
    static LpinfoBrowser *instance = nullptr;
    if (nullptr == instance) {
        instance = new LpinfoBrowser();
    }
    return instance;
}

LpinfoBrowser::LpinfoBrowser(/* args */)
{

}

LpinfoBrowser::~LpinfoBrowser()
{
}

// find remote printer
struct printerAttr
{
    const char *name;
    ipp_tag_e tag;
};

printerAttr attrList[] =
{
    {"printer-name",              IPP_TAG_NAME},
    {"printer-type",              IPP_TAG_ENUM},
    {"printer-location",          IPP_TAG_TEXT},
    {"printer-info",              IPP_TAG_TEXT},
    {"printer-make-and-model",    IPP_TAG_TEXT},
    {"printer-state",             IPP_TAG_ENUM},
    {"printer-state-message",     IPP_TAG_TEXT},
    {"printer-state-reasons",     IPP_TAG_KEYWORD},
    {"printer-uri-supported",     IPP_TAG_URI},
    {"device-uri",                IPP_TAG_URI},
    {"device-class",              IPP_TAG_KEYWORD},
    {"device-info",               IPP_TAG_TEXT},
    {"device-make-and-model",     IPP_TAG_TEXT},
    {"device-id",                 IPP_TAG_TEXT},
    {"device-location",           IPP_TAG_TEXT},
    {"printer-is-shared",         IPP_TAG_BOOLEAN},
    {"printer-is-accepting-jobs", IPP_TAG_BOOLEAN},
    {"printer-up-time",           IPP_TAG_INTEGER},
    {"queued-job-count",          IPP_TAG_INTEGER},
};

enum printerAttrCnt
{
    ALL = -1,
    PRINTER_NAME,
    PRINTER_TYPE,
    PRINTER_LOCATION,
    PRINTER_INFO,
    PRINTER_MAKE_AND_MODEL,
    PRINTER_STATE,
    PRINTER_STATE_MESSAGE,
    PRINTER_STATE_REASONS,
    PRINTER_URI_SUPPORTED,
    DEVICE_URI,
    DEVICE_CLASS,
    DEVICE_INFO,
    DEVICE_MAKE_AND_MODEL,
    DEVICE_ID,
    DEVICE_LOCATION,
    PRINTER_IS_SHARED,
    PRINTER_IS_ACCEPTING_JOBS,
    PRINTER_UP_TIME,
    QUEUED_JOB_COUNT
};

QVariant getValueFromAttr(ipp_attribute_t *attr, int i)
{
    QVariant var;
    switch (ippGetValueTag(attr))
    {
    case IPP_TAG_NAME:
    case IPP_TAG_TEXT:
    case IPP_TAG_KEYWORD:
    case IPP_TAG_URI:
    case IPP_TAG_CHARSET:
    case IPP_TAG_MIMETYPE:
    case IPP_TAG_LANGUAGE:
        var = ippGetString (attr, i, NULL);
        break;
    case IPP_TAG_INTEGER:
    case IPP_TAG_ENUM:
        var = ippGetInteger (attr, i);
        break;
    case IPP_TAG_BOOLEAN:
        var = ippGetBoolean (attr, i);
        break;
    case IPP_TAG_RANGE:
        // lower = ippGetRange (attr, i, &upper);
        // val = Py_BuildValue ("(ii)",
		// 	 lower,
		// 	 upper);
        break;

    case IPP_TAG_NOVALUE:
        ;
        break;
    case IPP_TAG_DATE:
        ;
        break;
    case IPP_TAG_RESOLUTION:
    // xres = ippGetResolution(attr, i, &yres, &units);
    // val = Py_BuildValue ("(iii)",
	// 		 xres,
	// 		 yres,
	// 		 units);
        break;
    default:
        printf ("(unknown IPP value tag 0x%x)", ippGetValueTag(attr));
        break;
    }
    if (var.isNull()) {
        qDebug() << "the variant is empty!";
    }
    return var;
}

QVariant getQVarFromAttr(ipp_attribute_t *attr, printerAttrCnt i)
{
    QVariant var;
    if (i == -1
     || !strcmp(ippGetName(attr), attrList[i].name)
     && ippGetValueTag(attr) == attrList[i].tag) {

        var = getValueFromAttr(attr, 0);
        if (var.isNull()) {
            qDebug() << "the variant is empty!";
        }
    }
    return var;
}

QStringList LpinfoBrowser::findRemotePrinter(const QString &host, const int port)
{
    QStringList resList;
    int encryption = (http_encryption_t) cupsEncryption ();
    http_t *http = httpConnectEncrypt (QTC(host), port, (http_encryption_t) encryption);
    if (!http) {
        qDebug ("failed to connect to remote: %s:%d\n", QTC(host), port);
        return resList;
    }
    ipp_t *request = ippNewRequest(CUPS_GET_PRINTERS), *answer;
    ipp_attribute_t *attr;
    const char *attributes[] = {
        attrList[PRINTER_NAME].name,
        attrList[PRINTER_TYPE].name,
        attrList[PRINTER_LOCATION].name,
        attrList[PRINTER_INFO].name,
        attrList[PRINTER_MAKE_AND_MODEL].name,
        attrList[PRINTER_STATE].name,
        attrList[PRINTER_STATE_MESSAGE].name,
        attrList[PRINTER_STATE_REASONS].name,
        attrList[PRINTER_URI_SUPPORTED].name,
        attrList[DEVICE_URI].name,
        attrList[PRINTER_IS_SHARED].name,
    };
    ippAddStrings (request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
		           "requested-attributes",
		           sizeof (attributes) / sizeof (attributes[0]),
		           NULL, attributes);
    answer = cupsDoRequest (http, request, "/");
    if (!answer || ippGetStatusCode (answer) > IPP_OK_CONFLICT) {
        if (answer && ippGetStatusCode (answer) == IPP_NOT_FOUND) {
        // No printers.
            qDebug ("no printers in %s:%d\n",  QTC(host), port);
            ippDelete (answer);
        }
        else
            qDebug("error: %d, %s\n", answer ? ippGetStatusCode (answer) : cupsLastError (), answer ? NULL : cupsLastErrorString ());
        if (answer)
            ippDelete (answer);
        return resList;
    }
    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        char *printer = NULL;
        if (attr && ippGetGroupTag (attr) != IPP_TAG_PRINTER)
            continue;
        if (!attr)
            break;
        QVariant var = getQVarFromAttr(attr, PRINTER_NAME);
        if (!var.isNull())
            resList.append(var.toString());
        var = getQVarFromAttr(attr, PRINTER_URI_SUPPORTED);
        if (!var.isNull())
            resList.append(var.toString());
    }
    ippDelete (answer);
    return resList;
}

static ipp_t *
cupsDoRequestOrDie (http_t *http,
                   ipp_t *request,
                   const char *resource)
{
    ipp_t *answer = cupsDoRequest (http, request, resource);
    if (answer == NULL) {
        qDebug ("failed to send IPP request %d", ippGetOperation (request));
        exit (1);
    }

    if (ippGetStatusCode (answer) > IPP_OK_CONFLICT) {
        qDebug ("IPP request %d failed (%d)", ippGetOperation (request), ippGetStatusCode (answer));
        exit (1);
    }

    return answer;
}

bool deviceidInfoCheck(DeviceInformation &info)
{
    if (info.ieee1284ID.isEmpty()) {
        return false;
    }
    device_id_s id;
    parse_device_id(QTC(info.ieee1284ID), &id);
    if (info.vendor.isEmpty()) {
        info.vendor = id.mfg;
    }
    if (info.model.isEmpty()) {
        info.model = id.mdl;
    }
    if (info.serial.isEmpty()) {
        info.serial = id.sern;
    }

    if (info.vendor.isEmpty() || info.model.isEmpty()) {
        return false;
    }
    return true;
}

QMap<QString, DeviceInformation> LpinfoBrowser::findNetWorkPrinter()
{
    QStringList include_schemes = {
        CUPS_INCLUDE_ALL
    };

    QStringList exclude_schemes = {
        "ipp",
        "beh",
        "http",
        "ipps",
        "https",
        "hp",
        "hpfax",
        "socket",
        "cups-pdf",
        "cups-brf",
        "bluetooth",
        "dnssd",
        "lpd",
        "smb",
    };

    auto res = LpinfoBrowser::findPrinter(include_schemes, exclude_schemes);
    for (auto iterator = res.begin(); iterator != res.end();) {
        // if (!ipv6UriCheck(info.uri)) {
        //     iterator = res.erase(iterator);
        //     continue ;
        // }

        Uri uri(iterator.value().uri);
        iterator.value().importUri(uri);
        if (iterator.value().deviceType == DeviceType::NONE) {
            iterator = res.erase(iterator);
            continue ;
        }

        if (!deviceidInfoCheck(iterator.value())) {
            iterator = res.erase(iterator);
            continue ;
        }

        iterator++;
    }

    return res;
}

QMap<QString, DeviceInformation> LpinfoBrowser::findDirectPrinter()
{
    QStringList include_schemes = {
        "serial",
        "parallel",
        "usb",
        "gutenprint53+usb",
        "cups-brf",
        "cups-pdf"
    };

    QStringList exclude_schemes = {
    };

    auto res = LpinfoBrowser::findPrinter(include_schemes, exclude_schemes);
    for (auto iterator = res.begin(); iterator != res.end();) {
        deviceIdToDeviceInfo(iterator.value());

        if (iterator.value().info.contains("(driverless)")) {
            iterator = res.erase(iterator);
            continue ;
        }

        if (iterator.value().uri == "usb:///") {
            qDebug() << "the uri is usb:///!";
            iterator = res.erase(iterator);
            continue ;
        }

        Uri uri(iterator.value().uri);
        iterator.value().importUri(uri);
        if (iterator.value().deviceType == DeviceType::NONE) {
            iterator = res.erase(iterator);
            continue ;
        }

        deviceIdToDeviceInfo(iterator.value());
        iterator++;
    }


    return res;
}

DeviceInformation LpinfoBrowser::getUsbPrinterDeviceFromDeviceId(const QString &deviceId)
{
    auto res = LpinfoBrowser::findDirectPrinter();
    for (auto iterator = res.begin(); iterator != res.end(); ++iterator) {
        auto value = iterator.value();
        if (deviceId == value.ieee1284ID) {
            return value;
        }
    }
    return DeviceInformation();
}

QMap<QString, DeviceInformation> LpinfoBrowser::findAllPrinter()
{
    QStringList include_schemes = {
    };

    QStringList exclude_schemes = {
    };

    auto res = LpinfoBrowser::findPrinter(include_schemes, exclude_schemes);
    for (auto iterator = res.begin(); iterator != res.end(); ++iterator) {
        auto value = iterator.value();
        deviceIdToDeviceInfo(iterator.value());
    }

    return res;
}

QMap<QString, DeviceInformation> LpinfoBrowser::findPrinter(const QStringList &include_schemes, const QStringList &exclude_schemes)
{
    QMap<QString, DeviceInformation> resmp;

    http_t *cups;
    ipp_t *request, *answer;
    ipp_attribute_t *attr;

    cups = httpConnectEncrypt (cupsServer (), ippPort(), cupsEncryption ());
    request = ippNewRequest (CUPS_GET_DEVICES);

    if (include_schemes.size()) {
        const char **strlist = new const char*[include_schemes.size()];
        CustomQStringConverter c;
        for (int i = 0; i < include_schemes.size(); i++) {
            strlist[i] = c.QStringToChar(include_schemes[i]);
        }
        ippAddStrings (request, IPP_TAG_OPERATION, IPP_TAG_NAME, "include-schemes",
                       include_schemes.size(),
                       NULL, strlist);
        delete[] strlist;
    }

    if (exclude_schemes.size()) {
        const char **strlist = new const char*[exclude_schemes.size()];
        CustomQStringConverter c;
        for (int i = 0; i < exclude_schemes.size(); i++) {
            strlist[i] = c.QStringToChar(exclude_schemes[i]);
        }
        ippAddStrings (request, IPP_TAG_OPERATION, IPP_TAG_NAME, "exclude-schemes",
                       exclude_schemes.size(),
                       NULL, strlist);
        delete[] strlist;
    }

    answer = cupsDoRequestOrDie (cups, request, "/");
    httpClose (cups);

        for (attr = ippFirstAttribute (answer); attr; attr = ippNextAttribute (answer)) {

        while (attr && ippGetGroupTag (attr) != IPP_TAG_PRINTER)
            attr = ippNextAttribute (answer);

        if (!attr)
            break;
        DeviceInformation info;
        for (; attr && ippGetGroupTag (attr) == IPP_TAG_PRINTER; attr = ippNextAttribute (answer)) {
            QVariant var = getQVarFromAttr(attr, DEVICE_URI);
            if (!var.isNull()) {
                info.uri = var.toString();
                continue;
            }
            var = getQVarFromAttr(attr, DEVICE_CLASS);
            if (!var.isNull()) {
                info.devClass = var.toString();
                continue;
            }
            var = getQVarFromAttr(attr, DEVICE_INFO);
            if (!var.isNull()) {
                info.info = var.toString();
                continue;
            }
            var = getQVarFromAttr(attr, DEVICE_MAKE_AND_MODEL);
            if (!var.isNull()) {
                info.makeAndModel = var.toString();
                continue;
            }
            var = getQVarFromAttr(attr, DEVICE_ID);
            if (!var.isNull()) {
                info.ieee1284ID = var.toString();
                continue;
            }
            var = getQVarFromAttr(attr, DEVICE_LOCATION);
            if (!var.isNull()) {
                info.location = var.toString();
                continue;
            }
        }

        if (!resmp.contains(info.uri)) {
            resmp.insert(info.uri, info);
        }
        if (!attr)
            break;
    }

    ippDelete(answer);
    return resmp;
}
