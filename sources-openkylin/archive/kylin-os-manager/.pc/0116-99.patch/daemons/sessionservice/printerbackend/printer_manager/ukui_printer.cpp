#include <QLocale>

#include "cupsconnection4ppds.h"
#include "ukui_printer.h"
#include "common.h"
#include <unistd.h>
#include <cupsfilters/ipp.h>
#include <cups/adminutil.h>
#include "lpinfo_browser.h"
#include "information_collector.h"

ConstrainOption inputSlot { "InputSlot", { "MPTRAY", "TRAY1", "AUTO" } };
ConstrainOption colorSpace { "ColorSpace", { "Gray" } };
ConstrainOption pageSize { "PageSize",
    { "Letter", "Legal", "Executive", "FanFoldGermanLegal", "A4", "A5", "A6", "210x270mm", "Env10", "EnvMonarch", "EnvDL", "ENVC5", "ISOB5", "B5", "ISOB6",
        "B6", "4x6", "PostCard", "DoublePostcardRotated", "EnvYou4", "195x270mm", "184x260mm", "197x273mm" } };
ConstrainOption brMediaType { "BrMediaType", { "PLAIN", "THICK", "THICKERPAPER2", "BOND", "ENV", "ENVTHICK", "ENVTHIN", "RECYCLED" } };
ConstrainOption MediaType { "MediaType", { "PLAIN", "THICK", "THICKERPAPER2", "BOND", "ENV", "ENVTHICK", "ENVTHIN", "RECYCLED" } };
ConstrainOption duplex { "Duplex", { "None", "DuplexNoTumble", "DuplexTumble" } };
ConstrainOption resolution { "Resolution", { "300dpi", "600dpi", "2400x600dpi", "1200dpi" } };

//#define g_cupsConnection (CupsConnection4PPDs::getInstance())
const char* baseIppUriFormat = "ipp://%s:%d/printers/";
// canonicalize uri
static void construct_uri(char* buffer, size_t buflen, const char* base, const char* value)
{
    char* d = buffer;
    const unsigned char* s = (const unsigned char*)value;
    if (strlen(base) < buflen) {
        strcpy(buffer, base);
        d += strlen(base);
    } else {
        strncpy(buffer, base, buflen);
        d += buflen;
    }

    while (*s && d < buffer + buflen) {
        if (isalpha(*s) || isdigit(*s) || *s == '-')
            *d++ = *s++;
        else if (*s == ' ') {
            *d++ = '+';
            s++;
        } else {
            if (d + 2 < buffer + buflen) {
                *d++ = '%';
                *d++ = "0123456789ABCDEF"[((*s) & 0xf0) >> 4];
                *d++ = "0123456789ABCDEF"[((*s) & 0x0f)];
                s++;
            } else {
                break;
            }
        }
    }

    if (d < buffer + buflen)
        *d = '\0';
}

static  std::string convertFromQString(const QString &in)
{
    QByteArray data = in.toUtf8();
    int size = data.size();
    char* dest = new char[size+1];
    strncpy(dest,data.data(),size);
    dest[size] = '\0';
    std::string res(dest);
    delete []dest;
    dest = 0;
    qDebug() << QByteArray::fromStdString(res);
    return res;
}

QDebug operator << (QDebug debug, const InkProperty &debugInfo)
{
    debug.noquote();
    QString info = QString("%1,%2,%3,%4").arg(debugInfo.color.name()).arg(debugInfo.name).arg(debugInfo.type).arg(debugInfo.level);
    debug << info;
    return debug;
}

UkuiPrinterManager::UkuiPrinterManager()
{
    // std::cout << "ukuiPrivaluenter Create.." << std::endl;
    m_user = cupsUser();
    m_locale = QLocale::system().name();

    qDebug("Current user:%s\n", m_user.toLocal8Bit().data());
}

UkuiPrinterManager::~UkuiPrinterManager(/* args */) { qDebug() << "ukuiPrinter destroy.."; }
// Add Printer,use ipp request
bool UkuiPrinterManager::addPrinter(const QString& usbUri, const QString& printerName, const QString& ppdName, const QString& location, QString& errorString)
{
    InformationCollector::getInstance().addMessage(QString("add printer %1 :%2 %3 %4").arg(printerName).arg(usbUri).arg(ppdName).arg(location));
    ipp_t* request = ippNewRequest(CUPS_ADD_MODIFY_PRINTER);
    // ipp_t* request = ippNew();
    // ippSetOperation(request, CUPS_ADD_MODIFY_PRINTER);
    ipp_t* answer = nullptr;
    char uri[HTTP_MAX_URI];
    char baseUri[128];
    sprintf(baseUri, baseIppUriFormat, m_host.toLocal8Bit().data(), m_port);
    construct_uri(uri, HTTP_MAX_URI, baseUri, printerName.toLocal8Bit().data());
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "ppd-name", nullptr, ppdName.toLocal8Bit().data());
    if (usbUri.size())
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_URI, "device-uri", nullptr, usbUri.toLocal8Bit().data());
    ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_TEXT, "printer-info", nullptr, printerName.toLocal8Bit().data());
    ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_TEXT, "printer-location", nullptr, location.toLocal8Bit().data());
    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer == nullptr) {
        printf("cupsDoRequest Failed.\n");
        ipp_status_t err = cupsLastError();
        if (err == ipp_status_t::IPP_STATUS_ERROR_CUPS_AUTHENTICATION_CANCELED) {
            int ret = cupsDoAuthentication(m_http, "POST", "/admin/");
            Q_UNUSED(ret)
        }
        qDebug() << "ERROR:" << cupsLastErrorString();
        errorString = QString(cupsLastErrorString());
        return false;
    }
    ipp_status_t status = ippGetStatusCode(answer);
    if (status > IPP_OK_CONFLICT) {
        printf("Error:%s\n", cupsLastErrorString());
        errorString = QString(cupsLastErrorString());
        return false;
    }
    ippDelete(answer);
    answer = nullptr;

    request = ippNewRequest(IPP_ENABLE_PRINTER);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, uri);
    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer != nullptr) {
        ippDelete(answer);
        answer = nullptr;
    }

    request = ippNewRequest(IPP_RESUME_PRINTER);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, uri);
    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer != nullptr) {
        ippDelete(answer);
        answer = nullptr;
    }

    return true;
}

bool UkuiPrinterManager::removePrinter(const QString& printerName)
{
    InformationCollector::getInstance().addMessage(QString("removePrinter") +  QString(printerName));
    ipp_t* request = ippNewRequest(CUPS_DELETE_PRINTER);
    ipp_t* answer = nullptr;
    char uri[HTTP_MAX_URI];
    char baseUri[128];
    sprintf(baseUri, baseIppUriFormat, m_host.toLocal8Bit().data(), m_port);
    construct_uri(uri, HTTP_MAX_URI, baseUri, printerName.toLocal8Bit().data());
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, uri);
    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer == nullptr) {
        printf("cupsDoRequest Failed.\n");
        return false;
    }
    ipp_status_t status = ippGetStatusCode(answer);
    if (status > IPP_OK_CONFLICT) {
        printf("Error:%s\n", cupsLastErrorString());
        ippDelete(answer);
        return false;
    }
    ippDelete(answer);
    return true;
}
void UkuiPrinterManager::addIppAttribute(PrinterAdvancedAttribute attribute)
{
    switch (attribute.tag) {
    default:
        break;
    }
    return;
}
bool UkuiPrinterManager::renamePrinter(QString oldname, QString newname)
{
    InformationCollector::getInstance().addMessage(QString("renamePrinter") +  QString(oldname) + QString(newname));
    duplicatePrinter(newname, oldname);
    removePrinter(oldname);

    return true;
}
#if 1
//使用cups接口打印的测试页，使用ipp获取属性时无法拿到标题
//故修改为使用ipp方案
bool UkuiPrinterManager::printTestPage(const QString& printerName)
{
    InformationCollector::getInstance().addMessage(QString("printTestPage:")+ printerName);
    static const QString fileName = "/usr/share/cups/data/testprint";
    QString resource = QString("/printers/%1").arg(printerName);// "/printers/printername"
    QString title = "Test Page";
    ipp_t *request = ippNewRequest(IPP_PRINT_JOB);
    ipp_t *response;
    ipp_t* answer = nullptr;
    char uri[HTTP_MAX_URI];
    char baseUri[128];
    sprintf(baseUri, baseIppUriFormat, m_host.toLocal8Bit().data(), m_port);
    construct_uri(uri, HTTP_MAX_URI, baseUri, convertFromQString(printerName).c_str());
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,"requesting-user-name", NULL, convertFromQString(m_user).c_str());
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "job-name", NULL, convertFromQString(title).c_str());
    /*
    * Do the request and get back a response...
    */
    if ((response = cupsDoFileRequest(CUPS_HTTP_DEFAULT,
                                      request,
                                      convertFromQString(resource).c_str(),
                                      convertFromQString(fileName).c_str())) != NULL)
    {
      ippDelete(response);
    }

    if (cupsLastError() == IPP_NOT_AUTHORIZED)
    {
        qDebug() << "Error IPP_NOT_AUTHORIZED";
        return false;
    }
    else if (cupsLastError() > IPP_OK_CONFLICT){
        qDebug() << "Error print test failed:"<<cupsLastError() << cupsLastErrorString();
        return false;
    }

    return true;
}
#else
bool UkuiPrinterManager::printTestPage(const QString& printerName, const QString& fileName)
{
    cups_dest_t* allDests = nullptr;
    cups_dest_t currentDest;
    int destCount;
    cups_file_t* fp = cupsFileOpen(fileName.toLocal8Bit().data(), "r");
    if (fp == NULL) {
        printf("Failed to print test page: open printing file failed\n");
        return false;
    }
    destCount = cupsGetDests(&allDests);
    if (destCount == 0) {
        /* code */
        printf("No available printer.\n");
        return false;
    }
    for (int i = 0; i < destCount; i++) {
        if (printerName.compare(allDests[i].name) == 0) {
            currentDest = allDests[i];
        }
    }

    http_t* http; /* Connection to destination */
    cups_dinfo_t* dinfo; /* Destination info */
    unsigned int dFlags = 0;
    ssize_t bytes;
    char buffer[32768];
    http = cupsConnectDest(&currentDest, dFlags, 30000, NULL, NULL, 0, NULL, NULL);
    if (http == NULL)
        return false;

    dinfo = cupsCopyDestInfo(http, &currentDest);
    if (dinfo == NULL)
        return false;

    const char* title;
    if ((title = strrchr(fileName.toLocal8Bit().data(), '/')) != NULL)
        title++;
    else
        title = fileName.toLocal8Bit().data();

    int job_id;
    int num_options = 0;
    cups_option_t* options = nullptr;
    if (cupsCreateDestJob(http, &currentDest, dinfo, &job_id, title, num_options, options) > IPP_STATUS_OK_IGNORED_OR_SUBSTITUTED) {
        /* code */
        cupsFileClose(fp);
        return false;
    }
    if (cupsStartDestDocument(http, &currentDest, dinfo, job_id, title, CUPS_FORMAT_AUTO, 0, NULL, 1) != HTTP_STATUS_CONTINUE) {
        printf("Unable to send document: %s\n", cupsLastErrorString());
        cupsFileClose(fp);
        return false;
    }

    while ((bytes = cupsFileRead(fp, buffer, sizeof(buffer))) > 0) {
        qDebug() << "Cups File Read " << bytes << " bytes.";
        if (cupsWriteRequestData(http, buffer, (size_t)bytes) != HTTP_STATUS_CONTINUE) {
            printf("Unable to write document data: %s\n", cupsLastErrorString());
            break;
        }
    }

    cupsFileClose(fp);

    if (cupsFinishDestDocument(http, const_cast<cups_dest_t*>(&currentDest), dinfo) > IPP_STATUS_OK_IGNORED_OR_SUBSTITUTED) {
        printf("Unable to send document: %s\n", cupsLastErrorString());
        return false;
    }
    puts("Job queued.");
printTestEnd:
    if (dinfo != nullptr) {
        /* code */
        cupsFreeDestInfo(dinfo);
    }

    if (allDests != nullptr) {
        /* code */
        cupsFreeDests(destCount, allDests);
    }
    httpClose(http);
    // Free
    return true;
}
#endif
void UkuiPrinterManager::enumPrinters(QList<UkuiUsbPrinter>& printers)
{
    const char* defaultName = cupsGetDefault2(CUPS_HTTP_DEFAULT);
    cups_dest_t* dests; /* Destinations */
    int num_dests = cupsGetDests(&dests);
    for (auto i = 0; i < num_dests; i++) {
        UkuiUsbPrinter printer;
        printer.name = dests[i].name;
        if (defaultName != nullptr && printer.name.compare(defaultName) == 0) {
            printer.is_default = true;
        }
        //qDebug("Printer Name:%s", printer.name.toLocal8Bit().data());
        QMap<QString, QStringList> inkInfo;
        int minn = 0x3f3f3f3f;
        for (auto k = 0; k < dests[i].num_options; k++) {
            //qDebug("    (%s %s)", dests[i].options[k].name, dests[i].options[k].value);
            if (QString::compare(dests[i].options[k].name, "device-uri") == 0) {
                printer.device_uri = dests[i].options[k].value;
            } else if (QString::compare(dests[i].options[k].name, "printer-uri-supported") == 0) {
                printer.printer_uri = dests[i].options[k].value;
            } else if (QString::compare(dests[i].options[k].name, "printer-make-and-model") == 0) {
                printer.make_and_model = dests[i].options[k].value;
            } else if (QString::compare(dests[i].options[k].name, "printer-state") == 0) {
                if (QString::compare(dests[i].options[k].value, "3") == 0) {
                    printer.connect_status = tr("Idle");
                } else if (QString::compare(dests[i].options[k].value, "4") == 0) {
                    printer.connect_status = tr("Printing");
                } else if (QString::compare(dests[i].options[k].value, "5") == 0) {
                    printer.connect_status = tr("Stopped");
                }
            } else if (QString::compare(dests[i].options[k].name, "printer-location") == 0) {
                printer.location = dests[i].options[k].value;
            } else if (QString::compare(dests[i].options[k].name, "marker-colors") == 0
                    || QString::compare(dests[i].options[k].name, "marker-names") == 0
                    || QString::compare(dests[i].options[k].name, "marker-types") == 0
                    || QString::compare(dests[i].options[k].name, "marker-levels") == 0) {

                QString value = QString(dests[i].options[k].value) + ",";
                QStringList list = value.split(",");
                minn = qMin(minn, list.size());
                inkInfo.insert(dests[i].options[k].name, list);
            }
        }

        if (minn != 0x3f3f3f3f && minn > 0) {
            for (int i = 0; i < minn; i++) {
                InkProperty ink;
                if (inkInfo.contains("marker-colors")) {
                    ink.color.setNamedColor(inkInfo["marker-colors"].at(i));
                }
                if (inkInfo.contains("marker-names")) {
                    ink.name = inkInfo["marker-names"].at(i);
                }
                if (inkInfo.contains("marker-types")) {
                    ink.type = inkInfo["marker-types"].at(i);
                }
                if (inkInfo.contains("marker-levels")) {
                    ink.level = inkInfo["marker-levels"].at(i).toInt();
                }

                if (!ink.color.name().isEmpty() && !ink.name.isEmpty() && ink.level >= 0 && ink.level <= 100) {
                    printer.inkOptions.append(ink);
                }
            }
        }

        if (printer.printer_uri.compare("") != 0) {
            getPpdInfo(printer);
            printers.push_back(printer);
        }
    }
    cupsFreeDests(num_dests, dests);
}

void UkuiPrinterManager::enumUsbPrinters(QList<UkuiUsbPrinter>& printers)
{
    enumPrinters(printers);
    foreach(auto printer, printers) {
        if (!printer.device_uri.contains("usb://")) {
            printers.removeOne(printer);
        }
    }
}

bool UkuiPrinterManager::printerIsInstalled(const QString &uri, bool needEnumPrinters, const QList<UkuiUsbPrinter>& printerNameList)
{
    if (uri.isEmpty()) {
        return false;
    }

    QList<UkuiUsbPrinter> *printersHandle = nullptr;
    QList<UkuiUsbPrinter> printers;
    if (needEnumPrinters == true) {
        UkuiPrinterManager::getInstance().enumPrinters(printers);
        printersHandle = &printers;
    } else {
        printersHandle = const_cast<QList<UkuiUsbPrinter> *>(&printerNameList);
    }

    foreach (auto printer, *printersHandle) {
        if (printer.device_uri == uri) {
            return true;
        }
    }
    return false;
}

CommonStatus UkuiPrinterManager::printerNameCheck(const QString &printerName, bool needEnumPrinters, const QList<UkuiUsbPrinter>& printerNameList, const QString &oldPrinterName)
{
    CommonStatus res;

    if (printerName.isEmpty()) {
        return  res;
    }

    if (printerName == oldPrinterName) {
        res.status = CommonStatusFlag::FAIL;
        return res;
    }

    if (!printerNameIsLegal(printerName)) {
        res.status = CommonStatusFlag::FAIL;
        res.message = tr("Printer Name Cannot Contains '%1', And More Than 0 Letter, Less Than 128 Letters !").arg("/\\'\"?#<>");
        return res;
    }

    QList<UkuiUsbPrinter> *printersHandle = nullptr;
    QList<UkuiUsbPrinter> printers;
    if (needEnumPrinters == true) {
        UkuiPrinterManager::getInstance().enumPrinters(printers);
        printersHandle = &printers;
    } else {
        printersHandle = const_cast<QList<UkuiUsbPrinter> *>(&printerNameList);
    }

    for (auto printer : *printersHandle) {
        if(!printerName.compare(printer.name,Qt::CaseInsensitive)) {
            if(!printerName.compare(printer.name,Qt::CaseSensitive)) {
                res.status = CommonStatusFlag::ERROR_EXIST_SAME_PRINTER;
                res.message = tr("Exist Same Name Printer!");
            } else {
                res.status = CommonStatusFlag::FAIL;
                res.message = tr("Printer name is not case sensitive!");
            }
            return res;
        }
    }

    res.status = CommonStatusFlag::SUCCESS;
    return res;
}

void UkuiPrinterManager::getPrintQueue(QString printerName, UkuiQueueStatus status, QList<PrintJob>& retJobs)
{
    cups_job_t* jobs;
    int whichjob = -1;
    int job_count;
    switch (status) {
    case UkuiQueueStatus::UKUI_PRINTER_QUEUE_ACTIVE:
        whichjob = CUPS_WHICHJOBS_ACTIVE;
        break;
    case UkuiQueueStatus::UKUI_PRINTER_QUEUE_UNFINISHED:
    case UkuiQueueStatus::UKUI_PRINTER_QUEUE_FINISHED:
        whichjob = CUPS_WHICHJOBS_COMPLETED;
        break;
    case UkuiQueueStatus::UKUI_PRINTER_QUEUE_ALL:
    default:
        whichjob = CUPS_WHICHJOBS_ALL;
        break;
    }
    // 这里要注意 jobs顺序是从小到达排列的(即最早的job-->最晚的job)
    job_count = cupsGetJobs(&jobs, printerName.toStdString().c_str(), 0, whichjob);
    for (int i = 0; i < job_count; i++) {
//        qDebug("%d Jobs[%d]  dest:%s title:%s user:%s state:%d", i, jobs[i].id, jobs[i].dest, jobs[i].title, jobs[i].user, jobs[i].state);
        PrintJob job;
        job.printerName = jobs[i].dest;
        job.title       = jobs[i].title;
        job.id          = jobs[i].id;
        job.user        = jobs[i].user;
        job.jobState    = jobs[i].state;
        // 时间可能为0 就会显示成1970xxx
        if (jobs[i].completed_time)
            job.completeTime = jobs[i].completed_time;
        if (jobs[i].creation_time)
            job.createTime = jobs[i].creation_time;
        if (jobs[i].processing_time)
            job.processingTime = jobs[i].processing_time;
        if (jobs[i].state == IPP_JSTATE_PROCESSING) {
            job.status = UkuiQueueStatus::UKUI_PRINTER_QUEUE_ACTIVE;
        } else if (jobs[i].state == IPP_JSTATE_COMPLETED) {
            job.status = UkuiQueueStatus::UKUI_PRINTER_QUEUE_FINISHED;
        } else {
            job.status = UkuiQueueStatus::UKUI_PRINTER_QUEUE_UNFINISHED;
        }
        getJobAttributes(job.id, &job);
        retJobs.push_back(job);
    }
    // perhaps should get "job-state-message" to show how the job is not
    // completed.
}

bool UkuiPrinterManager::restartJob(const int &jobId)
{
    InformationCollector::getInstance().addMessage(QString("restartJob")+ QString(jobId));
    ipp_t *request;
    request = ippNewRequest(IPP_RESTART_JOB);
    QString uri = QString("ipp://localhost/jobs/%1").arg(jobId);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "job-uri", NULL, QTC(uri));
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name", NULL, cupsUser());
    ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/jobs"));
    if (cupsLastError() == IPP_STATUS_ERROR_BAD_REQUEST || cupsLastError() == IPP_STATUS_ERROR_VERSION_NOT_SUPPORTED) {
        return false;
    }
    else if (cupsLastError() > IPP_OK_CONFLICT) {
        qDebug () << cupsLastErrorString();
        return false;
    }
    return true;
}
// cupsCancelJob2 函数第四个参数为purge
// 置成1就是delete 置成0就是cancel
bool UkuiPrinterManager::cancelJob(const QString &printerName, const int &jobId)
{
    InformationCollector::getInstance().addMessage(QString("cancelJob") + QString("printerName") + QString(jobId));
    return (cupsCancelJob2(CUPS_HTTP_DEFAULT, QTC(printerName), jobId, 0) < IPP_STATUS_REDIRECTION_OTHER_SITE);
}

bool UkuiPrinterManager::deleteJob(const QString &printerName, const int &jobId)
{
    InformationCollector::getInstance().addMessage(QString("deleteJob") + QString("printerName") + QString(jobId));
    return (cupsCancelJob2(CUPS_HTTP_DEFAULT, QTC(printerName), jobId, 1) < IPP_STATUS_REDIRECTION_OTHER_SITE);
}

// jobid为0时认为需要move所有job
bool UkuiPrinterManager::moveJob(const QString &from, const QString &to, const int &jobId)
{
    InformationCollector::getInstance().addMessage(QString("moveJob") + QString(from) + QString(to) + QString(jobId));
    ipp_t *request;
    request = ippNewRequest(CUPS_MOVE_JOB);
    if (jobId) {
        QString jobUri = QString("ipp://localhost/jobs/%1").arg(jobId);
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "job-uri", NULL,
                     QTC(jobUri));
    }
    else {
        QString printerUri = QString("ipp://localhost/printers/%1").arg(from);
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL,
                     QTC(printerUri));
    }

    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name",
                 NULL, cupsUser());

    QString printerUri = QString("ipp://localhost/printers/%1").arg(to);
    ippAddString(request, IPP_TAG_JOB, IPP_TAG_URI, "job-printer-uri",
                 NULL, QTC(printerUri));

    ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/jobs"));

    if (cupsLastError() > IPP_OK_CONFLICT) {
        qDebug("moveJob: %s", cupsLastErrorString());
        return false;
    }
    else
        return true;
}

bool UkuiPrinterManager::moveAllJob(const QString &from, const QString &to)
{
    return moveJob(from, to, 0);
}

bool UkuiPrinterManager::getJobAttributes(int jobId, PrintJob *job, QMap<QString, QVariant> *jobProperties)
{
    if (!jobId) {
        return false;
    }
    ipp_t *request, *answer;
    ipp_attribute_t *attr;
    request = ippNewRequest(IPP_GET_JOB_ATTRIBUTES);
    QString uri = QString("ipp://localhost/jobs/%1").arg(jobId);
    ippAddString (request, IPP_TAG_OPERATION, IPP_TAG_URI, "job-uri", NULL, QTC(uri));
    //   ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD, "requested-attributes", NULL, "job-printer-uri");
    answer = cupsDoRequest (CUPS_HTTP_DEFAULT, request, "/");
    // 需要什么拿什么
    for (attr = ippFirstAttribute (answer); attr; attr = ippNextAttribute (answer)) {
        // qDebug() << ippGetName(attr)<< " " <<  ippGetString(attr,0,NULL) << " " << ippGetInteger(attr, 0);
        if (jobProperties) {
            (*jobProperties).insert(ippGetName(attr), getValueFromAttr(attr, 0));
        }
        // qDebug() << ippGetName(attr)<< getValueFromAttr(attr, 0);
        if (job) {
            if (!strcmp(ippGetName(attr), "job-k-octets") && ippGetValueTag(attr) == IPP_TAG_INTEGER) {
                (*job).size = ippGetInteger(attr, 0);
            }
            if (!strcmp(ippGetName(attr), "job-preserved") && ippGetValueTag(attr) == IPP_TAG_BOOLEAN) {
                (*job).preserved = ippGetBoolean(attr, 0);
            }
        }
    }
    if (answer)
        ippDelete (answer);
    return true;
}

bool UkuiPrinterManager::holdJob(const int &jobId)
{
    InformationCollector::getInstance().addMessage(QString("holdJob") +  QString(jobId));
    return this->setJobHoldUntil(jobId, "indefinite");
}

bool UkuiPrinterManager::releaseJob(const int &jobId)
{
    InformationCollector::getInstance().addMessage(QString("releaseJob") +  QString(jobId));
    return this->setJobHoldUntil(jobId, "no-hold");
}

bool UkuiPrinterManager::setJobHoldUntil(const int &jobId, const QString &jobHoldUntil)
{
    ipp_t *request, *answer;
    int num_options = 0;
    cups_option_t *options = NULL;
    request = ippNewRequest(IPP_SET_JOB_ATTRIBUTES);
    QString uri = QString("ipp://localhost/jobs/%1").arg(jobId);
    ippAddString (request, IPP_TAG_OPERATION, IPP_TAG_URI, "job-uri", NULL, QTC(uri));
    ippAddString (request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name", NULL, cupsUser ());
    num_options = cupsAddOption ("job-hold-until", QTC(jobHoldUntil), num_options, &options);
    cupsEncodeOptions (request, num_options, options);
    answer = cupsDoRequest (CUPS_HTTP_DEFAULT, request, "/jobs/");
    if (!answer || ippGetStatusCode (answer) > IPP_OK_CONFLICT) {
        qDebug("set_ipp_error: %d, %s\n", answer ? ippGetStatusCode (answer) : cupsLastError (), answer ? NULL : cupsLastErrorString ());
        if (answer)
            ippDelete (answer);
        qDebug ("<- Connection_setJobHoldUntil() (error)\n");
    return false;
    }
    return true;
}
QString UkuiPrinterManager::generatePrinterName()
{
    QList<UkuiUsbPrinter> printerList;
    enumPrinters(printerList);
    int number = -1;
    for (auto printer : printerList) {
        if (printer.name.indexOf("printer") == 0) {
            number = qMax(number, 0);
        }
        if (printer.name.indexOf("printer_") == 0) {
            number = qMax(number, printer.name.mid(8, -1).toInt());
        }
    }
    number++;
    if (number == 0)
        return QString("printer");
    return QString("printer_%1").arg(number);
}
void UkuiPrinterManager::getAdvancedPrinterProperties(UkuiUsbPrinter printer, QMap<QString, QVariant>& properties)
{
    QList<PrinterAdvancedAttribute> printerAttributes;
    getAllProperties(printer.printer_uri, printerAttributes);
    convertProperties(printerAttributes, properties);
    QMap<QString, PrinterOptionalAttribute> map_attr_with_option;
    for (auto i = 0; i < printerAttributes.size(); i++) {
        if (printerAttributes.at(i).key.endsWith("-default") == true) {
            QString key = printerAttributes.at(i).key.left(printerAttributes.at(i).key.length() - strlen("-default"));
            auto it = map_attr_with_option.find(key);
            if (it == map_attr_with_option.end()) {
                PrinterOptionalAttribute attr;
                attr.value = printerAttributes.at(i).value;
                map_attr_with_option.insert(key, attr);
            } else {
                it.value().value = printerAttributes.at(i).value;
            }
        } else if (printerAttributes.at(i).key.endsWith("-supported") == true) {
            continue;
        } else {
            QString key = printerAttributes.at(i).key;
            auto it = map_attr_with_option.find(key);
            if (it == map_attr_with_option.end()) {
                PrinterOptionalAttribute attr;
                attr.value = printerAttributes.at(i).value;
                map_attr_with_option.insert(key, attr);
            } else {
                it.value().value = printerAttributes.at(i).value;
            }
        }
    }
    for (auto i = 0; i < printerAttributes.size(); i++) {
        if (printerAttributes.at(i).key.endsWith("-supported") == true) {
            QString key = printerAttributes.at(i).key.left(printerAttributes.at(i).key.length() - strlen("-supported"));
            auto it = map_attr_with_option.find(key);
            if (it != map_attr_with_option.end()) {
                it.value().supportedValue = printerAttributes.at(i).value;
            }
        }
    }

    for (auto i = map_attr_with_option.begin(); i != map_attr_with_option.end(); i++) {
        qDebug() << "K:" << i.key() << "V:" << i.value().value << "S:" << i.value().supportedValue;
    }
}

void UkuiPrinterManager::getAllProperties(QString printer_uri, QList<PrinterAdvancedAttribute>& printerAttributes)
{
    ipp_t* request = ippNewRequest(IPP_GET_PRINTER_ATTRIBUTES);
    ipp_t* answer = nullptr;
    ipp_attribute_t* attr = nullptr;
    const char* attributes[] = { "all", nullptr };
    ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD, "requested-attributes", sizeof(attributes) / sizeof(attributes[0]), nullptr, attributes);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, printer_uri.toLocal8Bit().data());
    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/");
    if (answer && ippGetStatusCode(answer) == IPP_NOT_FOUND) {
        // No printers.
        qDebug() << "<- Connection::getPrinters() = {}(no printers)";
        ippDelete(answer);
        return;
    }
    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        while (attr && ippGetGroupTag(attr) != IPP_TAG_PRINTER) {
            attr = ippNextAttribute(answer);
        }

        if (!attr)
            break;

        for (; attr; attr = ippNextAttribute(answer)) {
            PrinterAdvancedAttribute Attr;
            Attr.tag = ippGetValueTag(attr);
            Attr.key = ippGetName(attr);
            if (ippGetCount(attr) > 1) {
                QVariantList list;
                QVariant item;
                for (auto i = 0; i < ippGetCount(attr); i++) {
                    item = getProperty(attr, i);
                    list.push_back(item);
                }
                Attr.value = list;
            } else {
                Attr.value = getProperty(attr);
            }
            if (Attr.key.contains("media") == true) {
                printf("xxx");
            }
            if (Attr.value.isNull() == false) {
                printerAttributes.push_back(Attr);
            }
        }

        if (!attr)
            break;
    }

    ippDelete(answer);
}

bool UkuiPrinterManager::modifyPrinterAttributes(UkuiUsbPrinter printer, QMap<QString, QVariant>& properties)
{
    ipp_t* request = ippNewRequest(CUPS_ADD_MODIFY_PRINTER);
    ipp_t* answer = nullptr;
    Q_UNUSED(request)
    Q_UNUSED(answer)
    char uri[HTTP_MAX_URI];
    char baseUri[128];
    sprintf(baseUri, baseIppUriFormat, m_host.toLocal8Bit().data(), m_port);
    construct_uri(uri, HTTP_MAX_URI, baseUri, printer.name.toLocal8Bit().data());
    QList<PrinterAdvancedAttribute> oldProperties;
    getAllProperties(printer.name, oldProperties);
    for (auto property : oldProperties) {
        auto it = properties.find(property.key);
        Q_UNUSED(it)
    }
    for (auto property : properties) { }
    return false;
}
void UkuiPrinterManager::convertProperties(QList<PrinterAdvancedAttribute> attrs, QMap<QString, QVariant>& properties)
{
    for (auto i = attrs.begin(); i != attrs.end(); i++) {
        properties.insert(i->key, i->value);
    }
}
void UkuiPrinterManager::changeProperties(QStringList& properties)
{
    Q_UNUSED(properties);
    ipp_t* request = ippNewRequest(CUPS_ADD_MODIFY_PRINTER);
    ipp_t* answer = nullptr;
    char uri[HTTP_MAX_URI];
    char baseUri[128];
    sprintf(baseUri, baseIppUriFormat, m_host.toLocal8Bit().data(), m_port);
    construct_uri(uri, HTTP_MAX_URI, baseUri, "Hewlett-Packard-HP-LaserJet-Pro-M706n");
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_TEXT, "printer-info", nullptr, "xHP-LaserJet-Pro-M706");
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_TEXT, "printer-location", nullptr, "lHP-LaserJet-Pro-M706n");
    ippAddBoolean(request, IPP_TAG_OPERATION, "printer-is-shared", true);
    ippAddBoolean(request, IPP_TAG_OPERATION, "printer-is-accepting-jobs", true);
    ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER, "copies-default", 4);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD, "media-default", nullptr, "na_letter_8.5x11in");
    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer != nullptr) {
        ipp_status_t status = ippGetStatusCode(answer);
        Q_UNUSED(status)
        ippDelete(answer);
    }
}
// TODO:设置默认打印机
bool UkuiPrinterManager::setDefaultPrinter(QString printerName)
{
    InformationCollector::getInstance().addMessage(QString("setDefaultPrinter") +  QString(printerName));
    char* instance;
    char* printer = 0;
    if (printerName.compare("") == 0)
        return false;

    cups_dest_t* dests; /* Destinations */

    std::string tmpStr = convertFromQString(printerName);
    printer = const_cast<char*>(tmpStr.c_str());
    int num_dests = cupsGetDests(&dests);
    if (num_dests == 0 || dests == nullptr)
        return false;

    if ((instance = strrchr(printer, '/')) != NULL)
        *instance++ = '\0';

    cups_dest_t* dest = cupsGetDest(printer, instance, num_dests, dests);
    if (dest == nullptr) {
        return false;
    }
    // cupsSetDefaultDest(printer,instance,num_dests,dests);
    for (auto i = 0; i < num_dests; i++) {
        dests[i].is_default = false;
    }
    dest->is_default = true;
    cupsSetDests(num_dests, dests);
    cupsFreeDests(num_dests, dests);
    // cupsSetDefaultDest(printer,instance,num_dests,dests);
    // System Wide Ipp
    // CUPS_SET_DEFAULT
    ipp_t* request = ippNewRequest(CUPS_SET_DEFAULT);
    ipp_t* answer = nullptr;
    char uri[HTTP_MAX_URI];
    char baseUri[128];
    sprintf(baseUri, baseIppUriFormat, m_host.toLocal8Bit().data(), m_port);
    construct_uri(uri, HTTP_MAX_URI, baseUri, printer);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);
    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer == nullptr) {
        printf("cupsDoRequest Failed.\n");
        ipp_status_t err = cupsLastError();
        if (err == ipp_status_t::IPP_STATUS_ERROR_CUPS_AUTHENTICATION_CANCELED) {
            int ret = cupsDoAuthentication(m_http, "POST", "/admin/");
            Q_UNUSED(ret)
        }
        qDebug() << "ERROR:" << cupsLastErrorString();
        return false;
    }
    ipp_status_t status = ippGetStatusCode(answer);
    if (status > IPP_OK_CONFLICT) {
        qDebug("Error:%s\n", cupsLastErrorString());
        return false;
    }
    ippDelete(answer);

    return true;
}

QString UkuiPrinterManager::getDefaultPrinter()
{
    return cupsGetDefault();
}
QVariant UkuiPrinterManager::getProperty(ipp_attribute_t* attr, int i)
{
    QVariant value;
    ipp_tag_t tag;
    ipp_tag_t valTag = tag = ippGetValueTag(attr);
    Q_UNUSED(valTag)
    switch (tag) {
    case IPP_TAG_NAME:
    case IPP_TAG_TEXT:
    case IPP_TAG_CHARSET:
    case IPP_TAG_URI:
    case IPP_TAG_MIMETYPE:
    case IPP_TAG_LANGUAGE:
    case IPP_TAG_KEYWORD: {
        QStringList retList;
        // qDebug() << ippGetName(attr) << ippGetValueTag(attr) << ippGetCount(attr);
        for (auto i = 0; i < ippGetCount(attr); i++) {
            retList.push_back(ippGetString(attr, i, NULL));
            // qDebug() << "   " << retList.at(i);
        }
        value = retList;
    } break;
    case IPP_TAG_BEGIN_COLLECTION: {
        qDebug() << ippGetName(attr) << ippGetValueTag(attr) << ippGetCount(attr);
        getCollection(attr);
    } break;
    case IPP_TAG_END_COLLECTION: {
    } break;
    case IPP_TAG_INTEGER:
    case IPP_TAG_ENUM: {
        QVariantList retList;
        for (auto i = 0; i < ippGetCount(attr); i++) {
            retList.push_back(ippGetInteger(attr, i));
        }
        value = retList;
    } break;
    case IPP_TAG_BOOLEAN: {
        value = ippGetBoolean(attr, i);
    } break;
    case IPP_TAG_RANGE: {
        QVariantList list;
        int lower, upper;
        lower = ippGetRange(attr, i, &upper);
        list.push_back(lower);
        list.push_back(upper);
        value = list;
    } break;
    case IPP_TAG_RESOLUTION: {
        QVariantList list;
        int yres = 0;
        ipp_res_t unit;
        int xres = ippGetResolution(attr, i, &yres, &unit);
        list.push_back(xres);
        list.push_back(yres);
        list.push_back((int)unit);
        value = list;
    } break;
    case IPP_TAG_DATE:
    case IPP_TAG_NOVALUE:
    case IPP_TAG_UNKNOWN:
    default: {
        qDebug() << "Type Not Supported:" << tag << " " << ippGetName(attr);
    }
    }
    return value;
}

void UkuiPrinterManager::getCollection(ipp_attribute_t* attr)
{
    int count = ippGetCount(attr);
    for (auto i = 0; i < count; i++) {
        ipp_t* col = ippGetCollection(attr, i);
        qDebug() << "{";
        for (auto attr = ippFirstAttribute(col); attr; attr = ippNextAttribute(col)) {
            if (ippGetValueTag(attr) == 52) {
                getCollection(attr);
            } else {
                qDebug() << "   " << ippGetName(attr) << ippGetValueTag(attr) << getProperty(attr);
            }
        }
        qDebug() << "}";
    }
}

void UkuiPrinterManager::setLocation(const QString printer_uri, const QString location)
{
    qDebug() << "ukuiPrinter::setLocation";
    ipp_t* request = ippNewRequest(IPP_OP_CUPS_ADD_MODIFY_PRINTER);
    ipp_t* answer = nullptr;
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, printer_uri.toLocal8Bit().data());
    ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_TEXT, "printer-location", nullptr, location.toLocal8Bit().data());
    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer && ippGetStatusCode(answer) != IPP_OK) {
        qDebug() << ippGetStatusCode(answer);
    }
    ippDelete(answer);
}
void UkuiPrinterManager::setInfo(const QString printer_uri, const QString info)
{
    qDebug() << "ukuiPrinter::setLocation";
    ipp_t* request = ippNewRequest(IPP_OP_CUPS_ADD_MODIFY_PRINTER);
    ipp_t* answer = nullptr;
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, printer_uri.toLocal8Bit().data());
    ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_TEXT, "printer-info", nullptr, info.toLocal8Bit().data());
    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer && ippGetStatusCode(answer) != IPP_OK) {
        qDebug() << ippGetStatusCode(answer);
    }
    ippDelete(answer);
}
void UkuiPrinterManager::setUri(const QString printer_uri, const QString device_uri)
{
    qDebug() << "ukuiPrinter::setUri";
    ipp_t* request = ippNewRequest(IPP_OP_CUPS_ADD_MODIFY_PRINTER);
    ipp_t* answer = nullptr;
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, printer_uri.toLocal8Bit().data());
    ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_URI, "device-uri", nullptr, device_uri.toLocal8Bit().data());
    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer && ippGetStatusCode(answer) != IPP_OK) {
        qDebug() << ippGetStatusCode(answer);
    }
    ippDelete(answer);
}

bool UkuiPrinterManager::getOption(ppd_file_t *ppd,ppd_option_t* option, SelectableProperty& property)
{
    ppd_choice_t* choice = ppdFindChoice(option, option->defchoice);
    if (choice == nullptr)
    {
        return false;
    }
    // Selected...
    property.keyword.name = option->keyword;
    QString str = QString("%1.%2").arg(m_locale).arg("Translation");
    ppd_attr_t *attr = ppdFindAttr(ppd,str.toStdString().c_str(),option->keyword);
    if(attr){
        property.keyword.displayName = attr->text;
    }else{
        property.keyword.displayName = option->text;
    }
    property.current.name = choice->choice;
    str = QString("%1.%2").arg(m_locale).arg(option->keyword);
    attr = ppdFindAttr(ppd,str.toStdString().c_str(),choice->choice);
    if(attr){
        property.current.displayName = attr->text;
    }else{
        property.current.displayName = choice->text;
    }
    for (auto i = 0; i < option->num_choices; i++) {
        struct AttrName temp;
        temp.name = option->choices[i].choice;
        attr = ppdFindAttr(ppd,str.toStdString().c_str(),option->choices[i].choice);
        if(attr){
            temp.displayName = attr->text;
        }else{
            temp.displayName = option->choices[i].text;
        }
        property.supported.push_back(temp);
    }
    return true;
}

void UkuiPrinterManager::getInkInfo(UkuiUsbPrinter& printer)
{
    if (printer.device_uri.isEmpty()) {
        return ;
    }

    const char* attributes[] = {
        "marker-change-time",
        "marker-colors",
        "marker-high-levels",
        "marker-levels",
        "marker-low-levels",
        "marker-message",
        "marker-names",
        "marker-types",
    };

    ipp_t *request = ippNewRequest(CUPS_GET_PRINTERS);
    ipp_t *answer;
    ipp_attribute_t *attr;
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, QTC(printer.device_uri));
    ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
        "requested-attributes", sizeof(attributes) / sizeof(attributes[0]), NULL, attributes);

    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/");
    if (!answer || ippGetStatusCode (answer) > IPP_OK_CONFLICT) {
        if (answer && ippGetStatusCode (answer) == IPP_NOT_FOUND) {
        // No printers.
            qDebug ("no printers in CUPS_HTTP_DEFAULT\n");
            ippDelete (answer);
            return ;
        }
        else
            qDebug("error: %d, %s\n", answer ? ippGetStatusCode (answer) : cupsLastError (), answer ? NULL : cupsLastErrorString ());
        if (answer)
            ippDelete (answer);
        return ;
    }
    int minn = 0x3f3f3f3f;
    QMap<QString, PrinterAdvancedAttribute> printerAttributes;
    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        if (attr && ippGetGroupTag (attr) != IPP_TAG_PRINTER)
            continue;
        if (!attr)
            break;
        PrinterAdvancedAttribute Attr;
        Attr.tag = ippGetValueTag(attr);
        Attr.key = ippGetName(attr);
        // getProperty已经获取过一次了，不用自己写循环获取了
        Attr.value = getProperty(attr);
        if (Attr.key == "marker-colors"
         || Attr.key == "marker-names"
         || Attr.key == "marker-types"
         || Attr.key == "marker-levels") {
            minn = qMin(minn, ippGetCount(attr));
        }
        printerAttributes.insert(ippGetName(attr), Attr);
    }
    ippDelete (answer);

    if (minn == 0x3f3f3f3f || minn == 0) {
        return ;
    }

    for (int i = 0; i < minn; i++) {
        InkProperty ink;
        if (printerAttributes.contains("marker-colors"))
            ink.color.setNamedColor(printerAttributes["marker-colors"].value.toList().at(i).toString());
        if (printerAttributes.contains("marker-names"))
            ink.name  = printerAttributes["marker-names"].value.toList().at(i).toString();
        if (printerAttributes.contains("marker-types"))
            ink.type  = printerAttributes["marker-types"].value.toList().at(i).toString();
        if (printerAttributes.contains("marker-levels")) {
            ink.level = printerAttributes["marker-levels"].value.toList().at(i).toInt();
            switch (ink.level)
            {
            case -1:
                qDebug() << "the level is unavailable!";
                break;
            case -2:
                qDebug() << "the level is unknown!";
                break;
            case -3:
                qDebug() << "the level is unknown but has not yet reached capacity!";
                break;
            default:
                break;
            }
        }
        printer.inkOptions.append(ink);
    }
    if (printer.inkOptions.isEmpty()) {
        InformationCollector::getInstance().addMessage("no ink options found!");
    }
    return ;
}

void UkuiPrinterManager::getPpdInfo(UkuiUsbPrinter& printer)
{
    if (printer.name.compare("") == 0) {
        return;
    }
    const char* tmpFileName = cupsGetPPD2(CUPS_HTTP_DEFAULT, printer.name.toLocal8Bit().data());
    ppd_file_t* ppd = ppdOpenFile(tmpFileName);
    ppdMarkDefaults(ppd);
    ppd_attr_t* attr;
    attr = ppdFindAttr(ppd, "ModelName", NULL);
    if (attr != nullptr) {
        printer.product = attr->value;
    }
    attr = ppdFindAttr(ppd, "Manufacturer", NULL);
    if (attr != nullptr) {
        printer.vendor = attr->value;
    }
    ppd_option_t* option = ppdFirstOption(ppd);
    while (option != nullptr) {
        SelectableProperty tmp;
        getOption(ppd,option,tmp);
        printer.flexibleOptions.push_back(tmp);
        option = ppdNextOption(ppd);
    }
    ppdClose(ppd);
    unlink(tmpFileName);
}

#if 1
//
void UkuiPrinterManager::setPrinterPPD(const UkuiUsbPrinter &printer){
    if (m_ppd == nullptr) {
        return;
    }

    const char* filename = cupsGetPPD2(CUPS_HTTP_DEFAULT, printer.name.toLocal8Bit().data());
    if (filename == nullptr) {
        return;
    }
    cups_file_t* orig = cupsFileOpen(filename, "r");
    if (orig == nullptr) {
        return;
    }

    bool needUpdate = false;
    char tempFile[1024];
    cups_file_t* newppd = cupsTempFile2(tempFile, 1024);
    char line[1024];
    while (cupsFileGetLine(orig, line, 1024) != 0) {
        QString strline(line);
        if (strline.startsWith("*Default", Qt::CaseInsensitive) == false) {
            cupsFilePuts(newppd, line);
        }
        else if(strline.indexOf(':') == -1){
            cupsFilePuts(newppd, line);
        }
        else {
            const int defLen = 8;
            int pos = strline.indexOf(':');
            int pos2 = strline.lastIndexOf('\n');
            //*Default<Key>: <Value>
            //获取Key字符串
            QString key = strline.mid(defLen,pos-defLen);
            //获取Value字符串
            QString value = strline.mid(pos+2,pos2-2-pos);
            //qDebug() << key << value;
            ppd_choice_t* choice = ppdFindMarkedChoice(m_ppd, key.toLocal8Bit().data());
            if (choice == nullptr) {
                continue;
            }
            if (value.compare(choice->choice) != 0) {
                needUpdate = true;
                QString newLine = QString("*Default%1: %2\n").arg(key).arg(choice->choice);
                cupsFilePuts(newppd, newLine.toLocal8Bit().data());
            } else
                cupsFilePuts(newppd, line);

        }
    }

    cupsFileClose(newppd);
    cupsFileClose(orig);
    ppdClose(m_ppd);
    m_ppd = nullptr;

    ipp_t* answer = nullptr;
    ipp_t* request = ippNewRequest(CUPS_ADD_MODIFY_PRINTER);
    char uri[HTTP_MAX_URI];
    char baseUri[128];
    sprintf(baseUri, baseIppUriFormat, m_host.toLocal8Bit().data(), m_port);
    construct_uri(uri, HTTP_MAX_URI, baseUri, printer.name.toLocal8Bit().data());
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name", NULL, cupsUser());
    answer = cupsDoFileRequest(CUPS_HTTP_DEFAULT, request, "/admin/", needUpdate ? tempFile : filename);
    unlink(tempFile);
    unlink(filename);
    if (answer != nullptr) {
        ippDelete(answer);
    } else {
        printf("cupsDoRequest Failed.\n");
        ipp_status_t err = cupsLastError();
        Q_UNUSED(err)
        qDebug() << "ERROR:" << cupsLastErrorString();
    }
}

#else
// Open ppd -> mark option -> emit -> close ppd
void UkuiPrinterManager::setPrinterPPD(const UkuiUsbPrinter& printer)
{
    const char* filename = cupsGetPPD2(CUPS_HTTP_DEFAULT, printer.name.toLocal8Bit().data());
    if (filename == nullptr) {
        return;
    }
    cups_file_t* orig = cupsFileOpen(filename, "r");
    if (orig == nullptr) {
        return;
    }
    ppd_file_t* ppd = ppdOpenFile(filename);
    if (ppd == nullptr) {
        return;
    }
    ppdMarkDefaults(ppd);
    bool needUpdate = false;
    char tempFile[1024];
    cups_file_t* newppd = cupsTempFile2(tempFile, 1024);
    char line[1024];
    while (cupsFileGetLine(orig, line, 1024) != 0) {
        QString strline(line);
        if (strline.startsWith("*Default", Qt::CaseInsensitive) == false) {
            cupsFilePuts(newppd, line);
        } else {
            if (printer.duplex.keyword.name.isNull() == false && strline.contains(printer.duplex.keyword.name) == true) {
                ppd_choice_t* choice = ppdFindMarkedChoice(ppd, printer.duplex.keyword.name.toLocal8Bit().data());
                if (choice == nullptr) {
                    continue;
                }
                if (printer.duplex.current.name.compare(choice->choice) != 0) {
                    needUpdate = true;
                    QString newLine = QString("*Default%1: %2\n")
                            .arg(printer.duplex.keyword.name)
                            .arg(printer.duplex.current.name);
                    cupsFilePuts(newppd, newLine.toLocal8Bit().data());
                } else
                    cupsFilePuts(newppd, line);
            } else if (printer.colorMode.keyword.name.isNull() == false && strline.contains(printer.colorMode.keyword.name) == true) {
                ppd_choice_t* choice = ppdFindMarkedChoice(ppd, printer.colorMode.keyword.name.toLocal8Bit().data());
                if (choice == nullptr) {
                    continue;
                }
                if (printer.colorMode.current.name.compare(choice->choice) != 0) {
                    needUpdate = true;
                    QString newLine = QString("*Default%1: %2\n")
                            .arg(printer.colorMode.keyword.name)
                            .arg(printer.colorMode.current.name);
                    cupsFilePuts(newppd, newLine.toLocal8Bit().data());
                } else
                    cupsFilePuts(newppd, line);
            } else if (printer.inputSlot.keyword.name.isNull() == false && strline.contains(printer.inputSlot.keyword.name) == true) {
                ppd_choice_t* choice = ppdFindMarkedChoice(ppd, printer.inputSlot.keyword.name.toLocal8Bit().data());
                if (choice == nullptr) {
                    continue;
                }
                if (printer.inputSlot.current.name.compare(choice->choice) != 0) {
                    needUpdate = true;
                    QString newLine = QString("*Default%1: %2\n")
                            .arg(printer.inputSlot.keyword.name)
                            .arg(printer.inputSlot.current.name);
                    cupsFilePuts(newppd, newLine.toLocal8Bit().data());
                } else
                    cupsFilePuts(newppd, line);
            } else if (printer.mediaType.keyword.name.isNull() == false
                       && strline.contains(printer.mediaType.keyword.name) == true) {
                ppd_choice_t* choice = ppdFindMarkedChoice(ppd, printer.mediaType.keyword.name.toLocal8Bit().data());
                if (choice == nullptr) {
                    continue;
                }
                if (printer.mediaType.current.name.compare(choice->choice) != 0) {
                    needUpdate = true;
                    QString newLine = QString("*Default%1: %2\n")
                            .arg(printer.mediaType.keyword.name)
                            .arg(printer.mediaType.current.name);
                    cupsFilePuts(newppd, newLine.toLocal8Bit().data());
                } else
                    cupsFilePuts(newppd, line);
            } else if (printer.mediaSize.keyword.name.isNull() == false
                       && strline.contains("ImageableArea") == true) {
                ppd_choice_t* choice = ppdFindMarkedChoice(ppd, "PageSize");
                if (choice == nullptr) {
                    continue;
                }
                if (printer.mediaSize.current.name.compare(choice->choice) != 0) {
                    needUpdate = true;
                    QString newLine = QString("*Default%1: %2\n")
                            .arg("ImageableArea")
                            .arg(printer.mediaSize.current.name);
                    cupsFilePuts(newppd, newLine.toLocal8Bit().data());
                } else
                    cupsFilePuts(newppd, line);

            } else if (printer.mediaSize.keyword.name.isNull() == false && strline.contains("PageSize") == true) {
                ppd_choice_t* choice = ppdFindMarkedChoice(ppd, "PageSize");
                if (choice == nullptr) {
                    continue;
                }
                if (printer.mediaSize.current.name.compare(choice->choice) != 0) {
                    needUpdate = true;
                    QString newLine = QString("*Default%1: %2\n")
                            .arg(choice->option->keyword)
                            .arg(printer.mediaSize.current.name);
                    cupsFilePuts(newppd, newLine.toLocal8Bit().data());
                } else
                    cupsFilePuts(newppd, line);

            } else if (printer.mediaSize.keyword.name.isNull() == false && strline.contains("PageRegion") == true) {
                ppd_choice_t* choice = ppdFindMarkedChoice(ppd, "PageSize");
                if (choice == nullptr) {
                    continue;
                }
                if (printer.mediaSize.current.name.compare(choice->choice) != 0) {
                    needUpdate = true;
                    QString newLine = QString("*Default%1: %2\n")
                            .arg("PageRegion")
                            .arg(printer.mediaSize.current.name);
                    cupsFilePuts(newppd, newLine.toLocal8Bit().data());
                } else
                    cupsFilePuts(newppd, line);

            } else if (printer.mediaSize.keyword.name.isNull() == false && strline.contains("PageDimension") == true) {
                ppd_choice_t* choice = ppdFindMarkedChoice(ppd, "PageSize");
                if (choice == nullptr) {
                    continue;
                }
                if (printer.mediaSize.current.name.compare(choice->choice) != 0) {
                    needUpdate = true;
                    QString newLine = QString("*Default%1: %2\n")
                            .arg("PageDimension")
                            .arg(printer.mediaSize.current.name);
                    cupsFilePuts(newppd, newLine.toLocal8Bit().data());
                } else
                    cupsFilePuts(newppd, line);

            } else if (printer.mediaSize.keyword.name.isNull() == false && strline.contains("PaperDimension") == true) {
                ppd_choice_t* choice = ppdFindMarkedChoice(ppd, "PageSize");
                if (choice == nullptr) {
                    continue;
                }
                if (printer.mediaSize.current.name.compare(choice->choice) != 0) {
                    needUpdate = true;
                    QString newLine = QString("*Default%1: %2\n")
                            .arg("PaperDimension")
                            .arg(printer.mediaSize.current.name);
                    cupsFilePuts(newppd, newLine.toLocal8Bit().data());
                } else
                    cupsFilePuts(newppd, line);

            } else {
                bool tmpFlag = false;
                if (printer.flexibleOptions.size())
                {
                    for (auto option : printer.flexibleOptions) {
                        if (option.keyword.name.isNull() == false && strline.contains(option.keyword.name) == true) {
                            QByteArray tmp = option.keyword.name.toLocal8Bit();
                            ppd_choice_t* choice = ppdFindMarkedChoice(ppd, tmp.data());
                            if (choice == nullptr) {
                                tmpFlag = false;
                                break;
                            }
                            if (option.current.name.compare(choice->choice) != 0) {
                                needUpdate = true;
                                QString newLine = QString("*Default%1: %2\n")
                                        .arg(choice->option->keyword)
                                        .arg(option.current.name);
                                cupsFilePuts(newppd, newLine.toLocal8Bit().data());
                                tmpFlag = true;
                            }
                            break;
                        }
                    }
                }
                if(tmpFlag == false)
                {
                    qDebug() << strline;
                    cupsFilePuts(newppd, line);
                }
            }
        }
    }

    cupsFileClose(newppd);
    cupsFileClose(orig);
    ppdClose(ppd);

    ipp_t* answer = nullptr;
    ipp_t* request = ippNewRequest(CUPS_ADD_MODIFY_PRINTER);
    char uri[HTTP_MAX_URI];
    char baseUri[128];
    sprintf(baseUri, baseIppUriFormat, m_host.toLocal8Bit().data(), m_port);
    construct_uri(uri, HTTP_MAX_URI, baseUri, printer.name.toLocal8Bit().data());
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name", NULL, cupsUser());
    answer = cupsDoFileRequest(CUPS_HTTP_DEFAULT, request, "/admin/", needUpdate ? tempFile : filename);
    unlink(tempFile);
    unlink(filename);
    if (answer != nullptr) {
        ippDelete(answer);
    } else {
        printf("cupsDoRequest Failed.\n");
        ipp_status_t err = cupsLastError();
        Q_UNUSED(err)
        qDebug() << "ERROR:" << cupsLastErrorString();
    }
}
#endif
//开始
//传入打印机名称，传出打印机属性
bool UkuiPrinterManager::startModifyAttr(const QString& printerName, UkuiUsbPrinter& printer)
{
    //Get printer info
    if (printerName.compare("") == 0)
        return false;

    const char* defaultName = cupsGetDefault2(CUPS_HTTP_DEFAULT);
    char* instance = 0;
    cups_dest_t* dests; /* Destinations */
    int num_dests = cupsGetDests(&dests);
    if (num_dests == 0 || dests == nullptr)
        return false;
    std::string str = printerName.toStdString();
    char* tmp =  const_cast<char*>(str.c_str());
    if ((instance = strrchr(tmp, '/')) != NULL)
        *instance++ = '\0';

    cups_dest_t* dest = cupsGetDest(tmp, instance, num_dests, dests);
    if (dest == nullptr) {
        qDebug() << "failed to get printer info: " << printerName;
        return false;
    }

    printer.name = dest->name;

    if (defaultName != nullptr && printer.name.compare(defaultName) == 0) {
        printer.is_default = true;
    }
    //qDebug("Printer Name:%s", printer.name.toLocal8Bit().data());
    for (auto k = 0; k < dest->num_options; k++) {
        //qDebug("    (%s %s)", dests[i].options[k].name, dests[i].options[k].value);
        if (QString::compare(dest->options[k].name, "device-uri") == 0) {
            printer.device_uri = dest->options[k].value;
        } else if (QString::compare(dest->options[k].name, "printer-uri-supported") == 0) {
            printer.printer_uri = dest->options[k].value;
        } else if (QString::compare(dest->options[k].name, "printer-make-and-model") == 0) {
            printer.make_and_model = dest->options[k].value;
        } else if (QString::compare(dest->options[k].name, "printer-state") == 0) {
            if (QString::compare(dest->options[k].value, "3") == 0) {
                printer.connect_status = tr("Idle");
            } else if (QString::compare(dest->options[k].value, "4") == 0) {
                printer.connect_status = tr("Printing");
            } else if (QString::compare(dest->options[k].value, "5") == 0) {
                printer.connect_status = tr("Stopped");
            }
        } else if (QString::compare(dest->options[k].name, "printer-location") == 0) {
            printer.location = dest->options[k].value;
        }
    }

    if (printer.printer_uri.compare("") != 0) {
        getPpdInfo(printer);
    }

    m_curFile = cupsGetPPD2(CUPS_HTTP_DEFAULT, printer.name.toLocal8Bit().data());
    std::string tmpStr = convertFromQString(m_curFile);
    m_ppd = ppdOpenFile(const_cast<char*>(tmpStr.c_str()));
    ppdMarkDefaults(m_ppd);

    return true;
}

void UkuiPrinterManager::endModifyAttr(const UkuiUsbPrinter &printer, bool needSave, bool needSaveUri)
{
    if (needSave == true) {
        //属性
        if (m_canSave == true) {
            setPrinterConfig(printer);
        } else {
            qDebug() << "Can not save now...";
        }
    }
    if (needSaveUri == true) {
        if (m_canSave == true) {
            setUri(printer.printer_uri, printer.device_uri);
        } else {
            qDebug() << "Can not save now...";
        }
    }

    //清理现场，把类内变量清除
    //clear m_ppd,名
    if (m_ppd != nullptr) {
        ppdClose(m_ppd);
        m_ppd = nullptr;
    }
    if (m_curFile.compare("") != 0) {
        std::string tmpStr = convertFromQString(m_curFile);

        unlink(const_cast<char*>(tmpStr.c_str()));
        m_curFile = "";
    }
    m_canSave = true;
}
//0 ：无冲突，其他：冲突选项个数
// -1 报错
int UkuiPrinterManager::modifyPrinterAttr(const QString& key, const QString& value, QMap<QString, QString>& conflicts)
{
    conflicts.clear();
    if (m_ppd == nullptr)
        return -1;

    ppdMarkOption(m_ppd, key.toUtf8().data(), value.toUtf8().data());
    //检测冲突
    int num = ppdConflicts(m_ppd);
    if (num != 0) {
        m_canSave = false;
        //枚举所有冲突属性
        for (auto j = 0; j < m_ppd->num_groups; j++) {
            for (auto k = 0; k < m_ppd->groups[j].num_options; k++) {
                ppd_option_t option = m_ppd->groups[j].options[k];
                if (option.conflicted) {
                    for (auto m = 0; m < option.num_choices; m++) {
                        if (option.choices[m].marked) {
                            qDebug() << k << option.keyword << option.choices[m].text;
                            conflicts.insert(option.keyword, option.choices[m].choice);
                        }
                    }
                }
            }
        }
    } else {
        m_canSave = true;
    }
    return num;
}

bool UkuiPrinterManager::getConflictResults(const QString& key, QList<conflictPair>& conflicts)
{
    qDebug() << key;
    conflicts.clear();
    ppd_option_t *option1, *option2;
    ppd_choice_t *choice1, *choice2;
    ppd_const_t constrain;
    if(m_ppd == nullptr)
    {
        qDebug() << "m_ppd is nullptr!";
    }
    for (auto j = 0; j < m_ppd->num_consts; j++) {
        constrain = m_ppd->consts[j];
        option1 = ppdFindOption(m_ppd, constrain.option1);
        option2 = ppdFindOption(m_ppd, constrain.option2);

        if (option1 == nullptr || option2 == nullptr)
            continue;
        qDebug() << "key" << key;
        qDebug() << "option1->keyword:" << option1->keyword;
        qDebug() << "option2->keyword:" << option2->keyword;
        qDebug() << "key.compare(option1->keyword):" << key.compare(option1->keyword);
        qDebug() << "key.compare(option2->keyword):" << key.compare(option2->keyword);
        if (key.compare(option1->keyword) != 0 && key.compare(option2->keyword))
            continue;

        if (constrain.choice1[0] != '\0') {
            choice1 = ppdFindChoice(option1, constrain.choice1);
        } else {
            for (auto i = 0; i < option1->num_choices; i++) {
                if (option1->choices[i].marked != 0) {
                    choice1 = &(option1->choices[i]);
                    break;
                }
            }
        }

        if (constrain.choice2[0] != '\0') {
            choice2 = ppdFindChoice(option2, constrain.choice2);
        } else {
            for (auto i = 0; i < option2->num_choices; i++) {
                if (option2->choices[i].marked != 0) {
                    choice2 = &(option2->choices[i]);
                    break;
                }
            }
        }
        //两个属性都mark的时候
        if (choice1 != nullptr && choice1->marked && choice2 != nullptr && choice2->marked) {
            conflictPair conflict;
            conflict.first.first = option1->keyword;
            conflict.first.second = choice1->choice;
            conflict.second.first = option2->keyword;
            conflict.second.second = choice2->choice;
            conflicts.push_back(conflict);
            //qDebug() << "Conflict Detected:(" << option1->keyword << choice1->choice << ") And (" << option2->keyword << choice2->choice <<")";
        }
    }
    return (conflicts.size() == 0) ? false : true;
}


bool UkuiPrinterManager::setPrinterConfig(const UkuiUsbPrinter& printer)
{
    setPrinterPPD(printer);
    setLocation(printer.printer_uri, printer.location);
    return true;
}
bool UkuiPrinterManager::duplicatePrinter(const QString newName, const QString oldName)
{
    if ((oldName.compare("") == 0) || (newName.compare("") == 0))
        return false;
    //PPD
    const char* filename = cupsGetPPD2(CUPS_HTTP_DEFAULT, oldName.toLocal8Bit().data());
    if (filename == nullptr) {
        return false;
    }
    ipp_t* answer = nullptr;
    ipp_t* request = ippNewRequest(CUPS_ADD_MODIFY_PRINTER);
    char uri[HTTP_MAX_URI];
    char baseUri[128];
    sprintf(baseUri, baseIppUriFormat, m_host.toLocal8Bit().data(), m_port);
    construct_uri(uri, HTTP_MAX_URI, baseUri, newName.toLocal8Bit().data());
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name", NULL, cupsUser());
    answer = cupsDoFileRequest(CUPS_HTTP_DEFAULT, request, "/admin/", filename);
    unlink(filename);
    if (answer != nullptr) {
        ippDelete(answer);
    } else {
        printf("cupsDoRequest Failed.\n");
        ipp_status_t err = cupsLastError();
        Q_UNUSED(err)
        qDebug() << "ERROR:" << cupsLastErrorString();
        return false;
    }
    cups_dest_t* dests; /* Destinations */
    int num_dests = cupsGetDests(&dests);
    if (num_dests == 0 || dests == nullptr)
        return false;

    char* oldInstance = nullptr;
//    char* oldPrinter = nullptr;

//    QByteArray temp = oldName.toUtf8();
    std::string tmpStr = convertFromQString(oldName);

    char* oldPrinter = const_cast<char*>(tmpStr.c_str());


    if ((oldInstance = strrchr(oldPrinter, '/')) != NULL)
        *oldInstance++ = '\0';

    cups_dest_t* oldDest = cupsGetDest(oldPrinter, oldInstance, num_dests, dests);
    char* devUri = const_cast<char*>(cupsGetOption("device-uri", oldDest->num_options, oldDest->options));
    char* location = const_cast<char*>(cupsGetOption("printer-location", oldDest->num_options, oldDest->options));
    char* info = const_cast<char*>(cupsGetOption("printer-info", oldDest->num_options, oldDest->options));
    int isShared = (QString::compare(cupsGetOption("printer-is-shared", oldDest->num_options, oldDest->options), "true", Qt::CaseInsensitive) == 0) ? 1 : 0;
    int isAccepting = (QString::compare(cupsGetOption("printer-is-accepting-jobs", oldDest->num_options, oldDest->options), "true", Qt::CaseInsensitive) == 0) ? 1 : 0;
    int isTemp = (QString::compare(cupsGetOption("printer-is-temporary", oldDest->num_options, oldDest->options), "true", Qt::CaseInsensitive) == 0) ? 1 : 0;
    qDebug() << oldDest->num_options;
    // Need Set uri, location ,description, shared, enable, accepting, op policy,
    //cupsSetDests2(CUPS_HTTP_DEFAULT,num_dests,dests);
    //Use IPP set Attrs, cupsSetDests always failed.
    setUri(uri, devUri);
    setInfo(uri, info);
    setLocation(uri, location);
    request = ippNewRequest(IPP_OP_CUPS_ADD_MODIFY_PRINTER);
    answer = nullptr;
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, uri);
    ippAddBoolean(request, IPP_TAG_OPERATION, "printer-is-shared", isShared);
    ippAddBoolean(request, IPP_TAG_OPERATION, "printer-is-accepting-jobs", isAccepting);
    ippAddBoolean(request, IPP_TAG_OPERATION, "printer-is-temporary", isTemp);

    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer && ippGetStatusCode(answer) != IPP_OK) {
        qDebug() << ippGetStatusCode(answer);
    }
    ippDelete(answer);

    request = ippNewRequest(IPP_RESUME_PRINTER);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, uri);
    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer != nullptr) {
        ippDelete(answer);
        answer = nullptr;
    }
    return true;
}

bool UkuiPrinterManager::getCupsDebugLogging()
{
    int num_settings = 0;
    cups_option_t *settings = NULL;
    http_t	*http;
    if ((http = httpConnectEncrypt(cupsServer(), ippPort(),
                                   cupsEncryption())) == NULL)
    {
    // _cupsLangPrintf(stderr, _("cupsctl: Unable to connect to server: %s"),
    //                 strerror(errno));
    return false;
    }

    if (!cupsAdminGetServerSettings(http, &num_settings, &settings)) {
        // _cupsLangPrintf(stderr, "cupsctl: %s", cupsLastErrorString());
        return false;
    }
    else {
        for (int i = 0; i < num_settings; i++) {
            if (!strcmp(settings[i].name, CUPS_SERVER_DEBUG_LOGGING)) {
                qDebug() << settings[i].value;
                qDebug() << atoi(settings[i].value);
                return atoi(settings[i].value);
            }
        }
    }
    cupsFreeOptions(num_settings, settings);
    return true;
}
void UkuiPrinterManager::setCupsDebugLogging(bool flag)
{
    QString s = flag ? "--debug-logging" : "--no-debug-logging";
    runCommand(QStringList{"cupsctl", s});
    return ;
}


bool UkuiPrinterManager::getPrinterShared(const QString &printerName)
{
    char uri[HTTP_MAX_URI];
    ipp_t *request;
    ipp_t *answer;
    ipp_attribute_t *attr;
    const char *attributes[] = {
    "printer-is-shared",
    };
    request = ippNewRequest (IPP_GET_PRINTER_ATTRIBUTES);
    construct_uri(uri, sizeof (uri), "ipp://localhost/printers/", QTC(printerName));
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);
    ippAddStrings (request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
		           "requested-attributes",
		           sizeof (attributes) / sizeof (attributes[0]),
		           NULL, attributes);
    answer = cupsDoRequest (CUPS_HTTP_DEFAULT, request, "/");
    if (!answer || ippGetStatusCode (answer) > IPP_OK_CONFLICT) {
        if (answer && ippGetStatusCode (answer) == IPP_NOT_FOUND) {
        // No printers.
            qDebug ("Get Enabled failed in %s\n",  QTC(printerName));
            ippDelete (answer);
            return false;
        }
        else
            qDebug("error: %d, %s\n", answer ? ippGetStatusCode (answer) : cupsLastError (), answer ? NULL : cupsLastErrorString ());
        if (answer)
            ippDelete (answer);
        return false;
    }
    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        if (attr && ippGetGroupTag (attr) != IPP_TAG_PRINTER)
            continue;
        if (!attr)
            break;
        qDebug() << ippGetName(attr);
        if (!strcmp(ippGetName(attr), "printer-is-shared") && ippGetValueTag(attr) == IPP_TAG_BOOLEAN) {
            return ippGetBoolean (attr, 0);
        }
    }
    return false;
}

void UkuiPrinterManager::setPrinterShared(const QString &printerName, bool flag)
{
    InformationCollector::getInstance().addMessage(QString("setPrinterShared %1 %2").arg(printerName).arg(flag));
    qDebug() << printerName;
    char uri[HTTP_MAX_URI];
    ipp_t *request;
    ipp_t *answer;
    request = ippNewRequest (CUPS_ADD_MODIFY_PRINTER);
    construct_uri(uri, sizeof (uri), "ipp://localhost/printers/", QTC(printerName));
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);
    for (int i = 0; i < 2; i++) {
        ippAddBoolean(request, IPP_TAG_OPERATION, "printer-is-shared", flag);
        answer = cupsDoRequest (CUPS_HTTP_DEFAULT, request, "/admin/");
        if (answer && ippGetStatusCode (answer) == IPP_NOT_POSSIBLE) {
            // Perhaps it's a class, not a printer.
            ippDelete (answer);
            request = ippNewRequest (CUPS_ADD_MODIFY_CLASS);
            construct_uri(uri, sizeof (uri), "ipp://localhost/printers/", QTC(printerName));
            ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);
        } else break;
    }
    if (!answer || ippGetStatusCode (answer) > IPP_OK_CONFLICT) {
        qDebug() <<  cupsLastErrorString ();
        if (answer)
            ippDelete (answer);
    }
    return ;
}

bool UkuiPrinterManager::getPrinterEnabled(const QString &printerName)
{
    char uri[HTTP_MAX_URI];
    ipp_t *request;
    ipp_t *answer;
    ipp_attribute_t *attr;
    const char *attributes[] = {
        "printer-state",
    };
    request = ippNewRequest (IPP_GET_PRINTER_ATTRIBUTES);
    construct_uri(uri, sizeof (uri), "ipp://localhost/printers/", QTC(printerName));
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);
    ippAddStrings (request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
		           "requested-attributes",
		           sizeof (attributes) / sizeof (attributes[0]),
		           NULL, attributes);
    answer = cupsDoRequest (CUPS_HTTP_DEFAULT, request, "/");
    if (!answer || ippGetStatusCode (answer) > IPP_OK_CONFLICT) {
        if (answer && ippGetStatusCode (answer) == IPP_NOT_FOUND) {
        // No printers.
            qDebug ("Get Enabled failed in %s\n",  QTC(printerName));
            ippDelete (answer);
            return false;
        }
        else
            qDebug("error: %d, %s\n", answer ? ippGetStatusCode (answer) : cupsLastError (), answer ? NULL : cupsLastErrorString ());
        if (answer)
            ippDelete (answer);
        return false;
    }
    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        if (attr && ippGetGroupTag (attr) != IPP_TAG_PRINTER)
            continue;
        if (!attr)
            break;
        if (!strcmp(ippGetName(attr), "printer-state") && ippGetValueTag(attr) == IPP_TAG_ENUM) {
            return ippGetInteger (attr, 0) != IPP_PRINTER_STOPPED;
        }
    }
    return false;
}

void UkuiPrinterManager::setPrinterEnabled(const QString &printerName, bool flag)
{
    char uri[HTTP_MAX_URI];
    ipp_t *request;
    ipp_t *answer;
    ipp_attribute_t *attr;

    request = ippNewRequest (flag ? IPP_RESUME_PRINTER : IPP_PAUSE_PRINTER);
    construct_uri(uri, sizeof (uri), "ipp://localhost/printers/", QTC(printerName));
    ippAddString (request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);
    answer = cupsDoRequest (CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer && ippGetStatusCode (answer) == IPP_NOT_POSSIBLE) {
        // Perhaps it's a class, not a printer.
        ippDelete (answer);
        qDebug() << cupsLastErrorString();
    }
    if (!answer || ippGetStatusCode (answer) > IPP_OK_CONFLICT) {
        qDebug() <<  cupsLastErrorString ();
        if (answer)
            ippDelete (answer);
    }
    return ;
}

bool UkuiPrinterManager::isRemotePrinter(const QString &printerName)
{
    char uri[HTTP_MAX_URI];
    ipp_t *request;
    ipp_t *answer;
    ipp_attribute_t *attr;
    const char *attributes[] = {
        "printer-make-and-model",
    };
    request = ippNewRequest (CUPS_GET_PRINTERS);
    construct_uri(uri, sizeof (uri), "ipp://localhost/printers/", QTC(printerName));
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);
    ippAddStrings (request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
		           "requested-attributes",
		           sizeof (attributes) / sizeof (attributes[0]),
		           NULL, attributes);
    answer = cupsDoRequest (CUPS_HTTP_DEFAULT, request, "/");
    if (!answer || ippGetStatusCode (answer) > IPP_OK_CONFLICT) {
        if (answer && ippGetStatusCode (answer) == IPP_NOT_FOUND) {
        // No printers.
            qDebug ("Get Enabled failed in %s\n",  QTC(printerName));
            ippDelete (answer);
            return false ;
        }
        else
            qDebug("error: %d, %s\n", answer ? ippGetStatusCode (answer) : cupsLastError (), answer ? NULL : cupsLastErrorString ());
        if (answer)
            ippDelete (answer);
        return false;
    }
    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        if (attr && ippGetGroupTag (attr) != IPP_TAG_PRINTER)
            continue;
        if (!attr)
            break;
        if (!strcmp(ippGetName(attr), "printer-make-and-model") && ippGetValueTag(attr) == IPP_TAG_TEXT) {
            qDebug() << ippGetString (attr, 0, nullptr);
            return !strcmp(ippGetString (attr, 0, nullptr), "Remote Printer");
        }
    }
    return false;
}

