/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#include "saneobject.h"
#include <QDebug>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

SaneObject * SaneObject::instance = new SaneObject;

#ifdef __cplusplus
extern "C" {
#endif

#define STRIP_HEIGHT 256
#define A4BRY 297 // Some scanner device will less than 297

typedef struct _ScanImage {
    uint8_t *data;
    int width;
    int height;
    int x;
    int y;
} ScanImage;

typedef struct _OptDescriptor {
    int numColorMode;
    int numSource;
    int numResolution;
    int numSizeBrX;
    int numSizeBrY;
    int numSizeTlX;
    int numSizeTlY;
    int numPageWidth;
    int numPageHeight;
    int numPageCount;
} OptDescriptor;


static OptDescriptor g_optDesc = {2, 3, 6, 10, 11, 8, 9, 6, 7, 63};

static double g_saneSizeA4BrY = 297;
static SANE_Handle g_device = nullptr;
static int g_verbose;
static SANE_Byte *g_buf;
static SANE_Int g_BufSize;
static SANE_Device *g_saneDevice = nullptr;


#define SET_1_BIT(n,i) ((1<<(i))|(n))
#define SET_0_BIT(n,i) ((~(1<<(i)))&(n))
#define SET_R_BIT(n,i) ((n)^(1<<(i)))
#define GET_i_BIT(n,i) (((n)>>(i))&1)

static void writePnmHeader (SANE_Frame format, int width, int height, int depth, FILE *ofp)
{
    switch (format) {
    case SANE_FRAME_RED:
    case SANE_FRAME_GREEN:
    case SANE_FRAME_BLUE:
    case SANE_FRAME_RGB:
        fprintf(ofp, "P6\n# SANE data follows\n%d %d\n%d\n",
                width, height, (depth <= 8) ? 255 : 65535);
        qDebug() << "#P6 SANE data follows width = " << width << "height = " << height
                 << "depth = " << ((depth <= 8) ? 255 : 65535);
        break;
    default:
        if (depth == 1) {
            fprintf(ofp, "P4\n# SANE data follows\n%d %d\n", width, height);
            qDebug() << "#P4 SANE data follows width = " << width << "height = " << height;
        } else {
            fprintf(ofp, "P5\n# SANE data follows\n%d %d\n%d\n",
                    width, height, (depth <= 8) ? 255 : 65535);
            qDebug() << "#P5 SANE data follows width = " << width << "height = " << height
                     << "depth = " << ((depth <= 8) ? 255 : 65535);
        }
        break;
    }
}

static void *advance (ScanImage *image)
{
    if (++image->x >= image->width) {
        image->x = 0;
        if ((++image->y >= image->height) || !image->data) {
            size_t old_size = 0, new_size;

            if (image->data)
                old_size = static_cast<size_t>(image->height * image->width);

            image->height += STRIP_HEIGHT;
            new_size = static_cast<size_t>(image->height * image->width);

            if (image->data) {
                image->data = (uint8_t *) realloc (image->data, new_size);
            } else {
                image->data = (uint8_t *) malloc (new_size);
            }

            if (image->data)
                memset (image->data + old_size, 0, new_size - old_size);
        }
    }

    if (!image->data) {
        qDebug() << "Can't allocate image buffer, width = " << image->width
                 << "height = " << image->height;
    }

    return image->data;
}

static SANE_Status onScanning(FILE *ofp)
{
    int i = 0, offset = 0, must_buffer = 0, hundred_percent = 1;
    SANE_Int len;
    bool first_frame = true;
    SANE_Byte min = 0xff, max = 0;
    SANE_Parameters parm;
    SANE_Status status;
    ScanImage image = { nullptr, 0, 0, 0, 0 };
    SANE_Word total_bytes = 0;
    SANE_Int hang_over = -1;
    do {
        if (!first_frame) {
            qDebug() << "sane start!";
            status = sane_start(g_device);
            if (status != SANE_STATUS_GOOD)
                goto cleanup;
        }

        qDebug()<<"sane get parameter";
        status = sane_get_parameters(g_device, &parm);
        qDebug() << "Parm : status = " << sane_strstatus(status)
                 << "format = " << parm.format
                 << "last_frame = " << parm.last_frame
                 << "bytes_per_line = " << parm.bytes_per_line
                 << "pixels_per_line = " << parm.pixels_per_line
                 << "lines = " << parm.lines
                 << "depth = " << parm.depth;

        if (status != SANE_STATUS_GOOD)
            goto cleanup;

        if (first_frame) {
            if (parm.lines >= 0) {
                qDebug() << "ScanImage's size(pixels): " << parm.pixels_per_line << parm.lines
                         << "Bits/pixel: " <<  parm.depth * (SANE_FRAME_RGB == parm.format ? 3 : 1);
            } else {
                qDebug() << "ScanImage's wide pixels: " << parm.pixels_per_line
                         << "Height for bits/pixel: " << parm.depth * (SANE_FRAME_RGB == parm.format ? 3 : 1);
            }

            switch (parm.format) {
            case SANE_FRAME_RED:
            case SANE_FRAME_GREEN:
            case SANE_FRAME_BLUE:
                assert (parm.depth == 8);
                must_buffer = 1;
                offset = parm.format - SANE_FRAME_RED;
                break;

            case SANE_FRAME_RGB:
            case SANE_FRAME_GRAY:
                assert ((parm.depth == 1) || (parm.depth == 8) || (parm.depth == 16));
                if (parm.lines < 0) {
                    must_buffer = 1;
                    offset = 0;
                } else {
                    switch(g_sane_object->output_format)
                    {
                    case OUTPUT_PNM:
                        qDebug()<<"start write file!";
                        writePnmHeader(parm.format, parm.pixels_per_line, parm.lines, parm.depth, ofp);
                        break;
                    }
                }
                break;
            default:
                break;
            }

            if (must_buffer) {
                /**
                    * We're either scanning a multi-frame image or the
                    * scanner doesn't know what the eventual image height
                    * will be (common for hand-held scanners).  In either
                    * case, we need to buffer all data before we can write
                    * the image.
                    */
                image.width = parm.bytes_per_line;
                if (parm.lines >= 0) {
                    image.height = parm.lines - STRIP_HEIGHT + 1;
                } else {
                    image.height = 0;
                }

                image.x = image.width - 1;
                image.y = -1;
                if (!advance (&image)) {
                    status = SANE_STATUS_NO_MEM;
                    goto cleanup;
                }
            }
        } else {
            assert (parm.format >= SANE_FRAME_RED && parm.format <= SANE_FRAME_BLUE);
            offset = parm.format - SANE_FRAME_RED;
            image.x = image.y = 0;
        }

        hundred_percent = parm.bytes_per_line * parm.lines * ((parm.format == SANE_FRAME_RGB || parm.format == SANE_FRAME_GRAY) ? 1 : 3);
        while (status == SANE_STATUS_GOOD) {
            static int i =0;
            double progr;
            qDebug()<< "read file!" << i++;
            status = sane_read(g_device, g_buf, g_BufSize, &len);

            total_bytes += (SANE_Word) len;
            if(hundred_percent == 0){
                hundred_percent = 1;
            }
            progr = ((total_bytes * 100.) / (double) hundred_percent);
            if (progr > 100.)
                progr = 100.;


            if (status != SANE_STATUS_GOOD) {
                if (status != SANE_STATUS_EOF) {
                    goto cleanup;
                }
                break;
            }

            if (must_buffer) {
                qDebug() << "must_buffer = " << must_buffer;
                switch (parm.format) {
                case SANE_FRAME_RED:
                case SANE_FRAME_GREEN:
                case SANE_FRAME_BLUE:
                    for (i = 0; i < len; ++i) {
                        image.data[offset + 3 * i] = g_buf[i];
                        if (!advance (&image)) {
                            status = SANE_STATUS_NO_MEM;
                            goto cleanup;
                        }
                    }
                    offset += 3 * len;
                    break;
                case SANE_FRAME_RGB:
                    for (i = 0; i < len; ++i) {
                        image.data[offset + i] = g_buf[i];
                        if (!advance (&image)) {
                            status = SANE_STATUS_NO_MEM;
                            goto cleanup;
                        }
                    }
                    offset += len;
                    break;
                case SANE_FRAME_GRAY:
                    for (i = 0; i < len; ++i) {
                        image.data[offset + i] = g_buf[i];
                        if (!advance (&image)) {
                            status = SANE_STATUS_NO_MEM;
                            goto cleanup;
                        }
                    }
                    offset += len;
                    break;
                default:
                    break;
                }
            } else { /* ! must_buffer */
                if ((parm.depth != 16)) {
                    fwrite (g_buf, 1, len, ofp);
                } else {
#if !defined(WORDS_BIGENDIAN)
                    int start = 0;
                    /* check if we have saved one byte from the last sane_read */
                    if (hang_over > -1) {
                        if (len > 0) {
                            fwrite (g_buf, 1, 1, ofp);
                            g_buf[0] = (SANE_Byte) hang_over;
                            hang_over = -1;
                            start = 1;
                        }
                    }
                    /* now do the byte-swapping */
                    for (i = start; i < (len - 1); i += 2) {
                        unsigned char LSB;
                        LSB = g_buf[i];
                        g_buf[i] = g_buf[i + 1];
                        g_buf[i + 1] = LSB;
                    }
                    /* check if we have an odd number of bytes */
                    if (((len - start) % 2) != 0) {
                        hang_over = g_buf[len - 1];
                        len--;
                    }
#endif
                    fwrite (g_buf, 1, len, ofp);
                }
            }

            if (g_verbose && parm.depth == 8) {
                for (i = 0; i < len; ++i)
                    if (g_buf[i] >= max) {
                        max = g_buf[i];
                    } else if (g_buf[i] < min) {
                        min = g_buf[i];
                    }
            }
        }
        if(g_sane_object->stopSaneReadFlag == true){
            if(g_sane_object->userInfo.type.compare(QApplication::tr("ADF Duplex"), Qt::CaseInsensitive) == 0 || g_sane_object->userInfo.type.compare("ADF Duplex", Qt::CaseInsensitive) == 0){
                status = status = sane_start(g_device);
                while(status == SANE_STATUS_GOOD){
                    status = sane_read(g_device, g_buf, g_BufSize, &len);
                }
            }

            status = SANE_STATUS_CANCELLED;
            goto cleanup;
        }
        fflush(ofp);
        first_frame = 0;
        qDebug() << "0531info: first_frame value has changed:" << first_frame;
        qDebug() << "0531info: parm.last_frame value:" << parm.last_frame;
    } while (!parm.last_frame);

    if (must_buffer) {
        qDebug() << "must_buffer = " << must_buffer;
        image.height = image.y;

        switch(g_sane_object->output_format) {
        case OUTPUT_PNM:
            writePnmHeader(parm.format, parm.pixels_per_line,
                           image.height, parm.depth, ofp);
            break;
        }

        writePnmHeader (parm.format, parm.pixels_per_line, image.height, parm.depth, ofp);

#if !defined(WORDS_BIGENDIAN)
        if (parm.depth == 16) {
            for (i = 0; i < image.height * image.width; i += 2) {
                unsigned char LSB;
                LSB = image.data[i];
                image.data[i] = image.data[i + 1];
                image.data[i + 1] = LSB;
            }
        }
#endif
        qDebug() << "end scanning status: " << status;
        fwrite (image.data, 1, image.height * image.width, ofp);
    }


    qDebug() << "end scanning status: " << status;

cleanup:
    if (image.data) {
        qDebug() << "free image data!";
        free (image.data);
    }
    qDebug() << "end scanning status: " << sane_strstatus(status);
    return status;

}

static void authCallback (SANE_String_Const resource, SANE_Char *username, SANE_Char *password)
{
    //    qDebug() << "auth_callback" << resource << username << password;
}

void SaneObject::refreshListSlots()
{
    QString name;
    QStringList names;

    saneClose();
    saneExit();
    SANE_Int version_code = 0;
    sane_init(&version_code, authCallback);
    SANE_Status status = sane_get_devices (&g_deviceList, SANE_FALSE);
    GlobalUserSignal::getInstance()->setDeviceList(g_deviceList);

    for (int i = 0; g_deviceList[i]; ++i) {
        qDebug() << "mark-Name-usb: " << g_deviceList[i]->name
                 << "mark-Vendor-usb: " << g_deviceList[i]->vendor
                 << "mark-Model-usb: " << g_deviceList[i]->model
                 << "mark-Type-usb: " << g_deviceList[i]->type;

        name = QString("%1 %2 %3").arg(g_deviceList[i]->vendor).arg(g_deviceList[i]->model).arg(g_deviceList[i]->name);
        names << name;
    }
    setSaneNames(names);

    g_sane_object->setSaneHaveHandle(false);
    g_user_signal->updateSetting();

    QTimer::singleShot(500, this, [this]() {emit g_user_signal->startHotPlugSignal();});
    // 刷新列表完成
    QString msg = tr("Refresh list complete.");
    g_user_signal->warnMsg(msg);
}

SANE_Status doScan(const char *fileName)
{
    SANE_Status status = SANE_STATUS_GOOD;
    g_sane_object->setSaneStatus(status);
    QString text = g_sane_object->userInfo.pageNumber;
    QString text2 = g_sane_object->userInfo.type;
    QString suffix;

    g_sane_object->output_format = OUTPUT_PNM;
    if (g_sane_object->output_format == OUTPUT_JPEG) {
        suffix = QString(".jpg");
    }else{
        suffix = QString(".pnm");
    }
    FILE *ofp = nullptr;
    QString path;
    QString part_path;
    QString save_path;
    g_BufSize = (32 * 1024);
    g_buf = static_cast<SANE_Byte*>(malloc(g_BufSize));
    bool multiscan = 0;
    int pagecount = 1;
    while (status == SANE_STATUS_GOOD || (status == 5 && multiscan)){
        path = fileName;
        QFileInfo pathinfo(path);
        if(text.compare(QApplication::tr("Multiple"), Qt::CaseInsensitive) == 0 || text.compare("Multiple", Qt::CaseInsensitive) == 0 || text.compare("多页扫描", Qt::CaseInsensitive) == 0 || text2.compare(QApplication::tr("ADF Duplex"), Qt::CaseInsensitive) == 0 ||text2.compare("ADF 双面", Qt::CaseInsensitive) == 0){
            multiscan = 1;
            part_path = pathinfo.absolutePath() + "/" + pathinfo.baseName() + "[" + QString::number(pagecount) + "]" + suffix + ".part";
            path = pathinfo.absolutePath() + "/" + pathinfo.baseName() + "[" + QString::number(pagecount) + "]" + suffix;
            save_path = g_sane_object->saveFullScanFileName +  "[" + QString::number(pagecount) + "]" + QString(".") + g_sane_object->userInfo.format;

            g_sane_object->loadFullScanFileNames.append(path);
            g_sane_object->saveFullScanFileNames.append(save_path);
            pagecount++;
        }else{
            part_path = pathinfo.absolutePath() + "/" + pathinfo.baseName() + suffix + ".part";
            path = pathinfo.absolutePath() + "/" + pathinfo.baseName() + suffix;
            save_path = g_sane_object->saveFullScanFileName + QString(".") + g_sane_object->userInfo.format;

            g_sane_object->loadFullScanFileNames.append(path);
            g_sane_object->saveFullScanFileNames.append(save_path);
        }
        qDebug() << "part_path = " << part_path;
        status = sane_start(g_device);

        if(status == SANE_STATUS_NO_DOCS){
            if(g_sane_object->scanPageNumber == 0){
                sane_cancel(g_device);
                return SANE_STATUS_NO_DOCS;
            }
            break;
        }

        if(status == SANE_STATUS_GOOD){
            g_sane_object->scanPageNumber += 1;
        }

        qDebug() << "`sane_start` status: " << sane_strstatus(status);
        if (status != SANE_STATUS_GOOD) {
            qDebug() << "Cannot start scan devices, sane_status = " << status;
            g_user_signal->closeScanDialog();
            break;
        }

        if(g_sane_object->scanPageNumber != 1){
            emit g_sane_object->updatePageNum();
        }

        QFile file(part_path);
        if (!file.open(QIODevice::WriteOnly)) {
            status = SANE_STATUS_ACCESS_DENIED;
            break;
        }else{
            ofp = fdopen(file.handle(), "w");
        }

        status = onScanning(ofp);

        switch(status) {
        case SANE_STATUS_GOOD:
        case SANE_STATUS_EOF: {
            if(QFile(part_path).exists()){
                if (rename(part_path.toLocal8Bit().data(), path.toLocal8Bit().data())) {
                    status = SANE_STATUS_ACCESS_DENIED;
                }
            }
        }
            break;
        default:
            break;
        }
    }
    if(multiscan == 1 && status !=SANE_STATUS_CANCELLED){
        for (int i = g_sane_object->loadFullScanFileNames.length() - 1; i >= 0; --i) {
            if(!QFile(g_sane_object->loadFullScanFileNames[i]).exists()){
                g_sane_object->loadFullScanFileNames.removeAt(i);
            }
        }
        status = SANE_STATUS_EOF;
    }

    if(!(text.compare(QApplication::tr("Multiple"), Qt::CaseInsensitive) == 0 || text.compare("Multiple", Qt::CaseInsensitive) == 0) && multiscan == 1 && status !=SANE_STATUS_CANCELLED && (text2.compare(QApplication::tr("ADF Duplex"), Qt::CaseInsensitive) == 0 ||text2.compare("ADF 双面", Qt::CaseInsensitive) == 0)){
        while (g_sane_object->loadFullScanFileNames.length() > 2) {
            g_sane_object->loadFullScanFileNames.removeLast();
        }
        status = SANE_STATUS_EOF;
    }

    qDebug() << "sane_cancel";
    if (ofp) {
        fclose (ofp);
        ofp = nullptr;
    }

    if (g_buf) {
        free (g_buf);
        g_buf = nullptr;
    }
    sane_cancel(g_device);
    g_sane_object->setSaneStatus(status);
    g_sane_object->isOnScanning = false;

    return status;
}

static void saneInit()
{
    SANE_Int version_code = 0;

    sane_init(&version_code, authCallback);

    qDebug() << "version_code = " << version_code;
}

static SANE_Status saneGetDevices(const SANE_Device ***device_list)
{
    qDebug() << "Get all scan devices, please waiting ...";

    /// This will be crashed unexpectedly, samples can be followed:
    /// 1. Caused by specific drives, such as `lenovo-image-g-series_1.0-16_arm64`, so we need Vendor developers to handle it
    /// 2. This will take a long time

    memset(device_list, 0x00, sizeof(device_list));

    SANE_Status status = sane_get_devices(device_list, SANE_FALSE);

    if (status) {
        qDebug() << "status = " << sane_strstatus(status);
    }
    return status;
}

SANE_Status saneOpen(SANE_Device *device, SANE_Handle *sane_handle)
{
    SANE_Status status = SANE_STATUS_INVAL;

    qDebug() << "name = " << device->name
             << "model = " << device->model
             << "vendor = " << device->vendor
             << "type = " << device->type;

    // For usbRemoved
    char name[512] = {0};
    snprintf(name, 512, "%s %s", device->vendor, device->model);
    g_sane_object->openSaneName = QString(name);
    qDebug() << "Open device name:  " << g_sane_object->openSaneName;

    status = sane_open(device->name, sane_handle);
    if (status) {
        /// status =  Error during device I/O: can be this scanner connected by usb is error,
        /// so check usb connected.
        qDebug() << "status = " << sane_strstatus(status);
        QString message = QApplication::tr("Fail to open the scanner") ;
        qDebug() << "Fail to open the scanner, error code" << QString::number(status);
        QTimer::singleShot(1000, [=](){g_user_signal->warnMsg(message);});
    }else{
        qDebug() << "Open scanner success";
    }

    return status;
}

SANE_Status getOptionColors(SANE_Handle sane_handle, int optnum)
{
    QStringList colors;

    QVector<std::string> colorScanModes;
    colorScanModes.push_back(SANE_VALUE_SCAN_MODE_COLOR);
    colorScanModes.push_back("color"); /* 松下KV-N1058Y for kylin-scanner */
    colorScanModes.push_back("Color");
    colorScanModes.push_back("24bit Color[Fast]"); /* brother4 driver, Brother DCP-1622WE, ADS3600W; simple-scan(#134), kylin-scanner(#74173) */
    colorScanModes.push_back("24bit Color"); /* Seen in the proprietary brother3 driver */
    colorScanModes.push_back("Color - 16 Million Colors"); /*Samsung unified driver. simple-scan(LP: 892915) */
    colorScanModes.push_back("COLOR"); /* deli */

    QVector<std::string> grayScanModes;
    grayScanModes.push_back(SANE_VALUE_SCAN_MODE_GRAY);
    grayScanModes.push_back("Gray");
    grayScanModes.push_back("gray"); /* 松下KV-N1058Y for kylin-scanner */
    grayScanModes.push_back("Grayscale");
    grayScanModes.push_back(SANE_I18N("Grayscale"));
    grayScanModes.push_back("True Gray"); /* Seen in the proprietary brother3 driver, Brother ADS3600W; however, Gray[Error Diffusion] is error gray scan modes */
    grayScanModes.push_back("Grayscale - 256 Levels"); /* Samsung unified driver. simple-scan(LP: 892915) */
    grayScanModes.push_back("GRAY"); /* deli */


    QVector<std::string> lineartScanModes;
    lineartScanModes.push_back(SANE_VALUE_SCAN_MODE_LINEART);
    lineartScanModes.push_back("Lineart");
    lineartScanModes.push_back("LineArt");
    lineartScanModes.push_back(SANE_I18N("LineArt"));
    lineartScanModes.push_back("Black & White");
    lineartScanModes.push_back(SANE_I18N("Black & White"));
    lineartScanModes.push_back("Binary");
    lineartScanModes.push_back(SANE_I18N("Binary"));
    lineartScanModes.push_back("Thresholded");
    lineartScanModes.push_back("Black and White - Line Art"); /* Samsung unified driver. simple-scan(LP: 892915) */
    lineartScanModes.push_back("Black and White - Halftone");
    lineartScanModes.push_back("bw"); /* 松下KV-N1058Y for kylin-scanner */
    lineartScanModes.push_back("BINARY"); /* deli */

    const SANE_Option_Descriptor *opt;
    SANE_Status status = SANE_STATUS_INVAL;

    qDebug() << "Get colors option = " << optnum;

    opt = sane_get_option_descriptor(sane_handle, optnum);

    g_sane_object->colorModesMap.clear();

    for (int i = 0; opt->constraint.string_list[i] != nullptr; ++i) {
        qDebug() << "颜色： " << opt->constraint.string_list[i];
        const char *tmp = *(opt->constraint.string_list + i);
        QVector<std::string>::iterator it;

        status = SANE_STATUS_GOOD;

        it = find(colorScanModes.begin(), colorScanModes.end(), tmp);

        if (it != colorScanModes.end()) {
            if (! g_sane_object->colorModesMap.contains("Color")) {
                // Only the first key matched can be followed, which could avoid duplicate inserts bugs
                g_sane_object->colorModesMap.insert("Color", QString::fromStdString(*it));
                colors << QObject::tr("Color");
            }
        }

        it = find(grayScanModes.begin(), grayScanModes.end(), tmp);
        if ((it != grayScanModes.end()) && (! g_sane_object->colorModesMap.contains("Gray"))) {
            g_sane_object->colorModesMap.insert("Gray", QString::fromStdString(*it));
            colors << QObject::tr("Gray");
        }

        it = find(lineartScanModes.begin(), lineartScanModes.end(), tmp);
        if ((it != lineartScanModes.end()) && (! g_sane_object->colorModesMap.contains("Lineart"))) {
            g_sane_object->colorModesMap.insert("Lineart", QString::fromStdString(*it));
            colors << QObject::tr("Lineart");
        }
    }

    qDebug() << "colors: " << colors;
    QMap<QString, QString>::iterator itMap;
    for (itMap=g_sane_object->colorModesMap.begin(); itMap != g_sane_object->colorModesMap.end(); ++itMap ) {
        qDebug() << "Iterator for colorModesMap: " <<itMap.key() << itMap.value();
    }
    g_sane_object->setSaneColors(colors);
    return status;
}

SANE_Status setOptionPages(SANE_Handle sane_handle, SANE_Int pages){
    SANE_Status status;
    qDebug() << "options: " << g_optDesc.numPageCount
             << "Set page option = " << pages;
    status = sane_control_option(sane_handle,  static_cast<SANE_Int>(g_optDesc.numPageCount),
                                 SANE_ACTION_SET_VALUE, &pages, nullptr);
    if (status != SANE_STATUS_GOOD) {
        qDebug() << "status = " << status << "desc: " << sane_strstatus(status);
        return status;
    }

    qDebug() << "Set countpage success.";

    return status;
}

SANE_Status setOptionColors(SANE_Handle sane_handle, SANE_String val_color)
{
    SANE_Status status = SANE_STATUS_INVAL;

    qDebug() << "option: " << g_optDesc.numColorMode
             << "Set color option = " << val_color;
    int i = 0;
    const SANE_Option_Descriptor *opt;


    qDebug() << "color handle: " << sane_handle;
    opt = sane_get_option_descriptor(sane_handle, g_optDesc.numColorMode);
    for (int i = 0; opt->constraint.string_list[i] != nullptr; ++i) {
        const char *tmp = *(opt->constraint.string_list + i);
        status = SANE_STATUS_GOOD;
        qDebug() << "color strings = " << *(opt->constraint.string_list + i);
    }

    status = sane_control_option(sane_handle,  static_cast<SANE_Int>(g_optDesc.numColorMode),
                                 SANE_ACTION_SET_VALUE, val_color, nullptr);

    if (status != SANE_STATUS_GOOD) {
        qDebug() << "status = " << status << "desc: " << sane_strstatus(status);
        return status;
    }

    qDebug() << "Set color success.";

    return status;
}

SANE_Status getOptionSourcesDefaultFailed()
{
    QStringList typeStringList;
    SANE_Status status = SANE_STATUS_GOOD;

    typeStringList << QObject::tr("Default Type");

    g_sane_object->haveSourceFlag = 0;

    qDebug() << "Default Type: " << typeStringList;

    g_sane_object->setSaneTypes(typeStringList);

    return status;

}

SANE_Status getOptionSources(SANE_Handle sane_handle, int optnum)
{
    QStringList typeStringList;
    SANE_Status status = SANE_STATUS_INVAL;
    const SANE_Option_Descriptor *opt;
    QVector<std::string>::iterator it;

    QVector<std::string> flatbedSources;
    flatbedSources.push_back("Auto");
    flatbedSources.push_back(SANE_I18N("Auto"));
    flatbedSources.push_back("Flatbed");
    flatbedSources.push_back(SANE_I18N("Flatbed"));
    flatbedSources.push_back("FlatBed");
    flatbedSources.push_back("Normal");
    flatbedSources.push_back(SANE_I18N("Normal"));

    QVector<std::string> adfSources;
    adfSources.push_back("Automatic Document Feeder");
    adfSources.push_back(SANE_I18N("Automatic Document Feeder"));
    adfSources.push_back("ADF");
    adfSources.push_back("adf"); /* 松下KV-N1058Y for kylin-scanner*/
    adfSources.push_back("Automatic Document Feeder(left aligned)"); /* Seen in the proprietary brother3 driver */
    adfSources.push_back("utomatic Document Feeder(centrally aligned)"); /* Seen in the proprietary brother3 driver */
    adfSources.push_back("ADF Simplex"); /* Samsung unified driver. LP: # 892915 */
    adfSources.push_back("Simplex");

    QVector<std::string> adfFrontSources;
    adfFrontSources.push_back("ADF Front");
    adfFrontSources.push_back(SANE_I18N("ADF Front"));
    adfFrontSources.push_back(SANE_I18N("Adf-front"));

    QVector<std::string> adfBackSources;
    adfBackSources.push_back("ADF Back");
    adfBackSources.push_back(SANE_I18N("ADF Back"));
    adfBackSources.push_back(SANE_I18N("Adf-back"));

    QVector<std::string> adfDuplexSources;
    adfDuplexSources.push_back("ADF Duplex");
    adfDuplexSources.push_back("Adf-duplex");
    adfDuplexSources.push_back(SANE_I18N("ADF Duplex"));
    adfDuplexSources.push_back("ADF Duplex - Long-Edge Binding"); /* Samsung unified driver. LP: # 892915 */
    adfDuplexSources.push_back("ADF Duplex - Short-Edge Binding");
    adfDuplexSources.push_back("Duplex"); /* HP duplex scan support. LP: #1353599 */
    adfDuplexSources.push_back(SANE_I18N("Duplex")); /* drc240 scanner */
    adfDuplexSources.push_back("Automatic Document Feeder(centrally aligned,Duplex)"); /* Brother duplex scan support. LP: #1343773 */
    adfDuplexSources.push_back("Automatic Document Feeder(left aligned,Duplex)");


    qDebug() << "GetOptionSource: " << optnum;

    opt = sane_get_option_descriptor(sane_handle, optnum);
    if (!opt) {
        qDebug() << "getOptionSources: opt is null!";
        return SANE_STATUS_INVAL;
    }

    qDebug() << "getOptionSources: opt->name = " << (opt->name ? opt->name : "null")
             << "opt->constraint_type = " << opt->constraint_type;

    g_sane_object->sourceModesMap.clear();
    g_sane_object->haveSourceFlag = 0;

    if (opt->constraint_type != SANE_CONSTRAINT_STRING_LIST) {
        qDebug() << "getOptionSources: constraint_type is not STRING_LIST, type = " << opt->constraint_type;
        return SANE_STATUS_INVAL;
    }

    for (int i = 0; opt->constraint.string_list[i] != nullptr; ++i) {
        const char *tmp = *(opt->constraint.string_list + i);
        qDebug() << tmp;
        status = SANE_STATUS_GOOD;

        it = find(flatbedSources.begin(), flatbedSources.end(), tmp);

        if (it != flatbedSources.end()) {
            if (! g_sane_object->sourceModesMap.contains("Flatbed")) {
                g_sane_object->sourceModesMap.insert("Flatbed", QString::fromStdString(*it));
                typeStringList << QObject::tr("Flatbed");

                ++g_sane_object->haveSourceFlag;
                continue;
            }
        }

        it = find(adfSources.begin(), adfSources.end(), tmp);
        if ((it != adfSources.end()) && (! g_sane_object->sourceModesMap.contains("ADF"))) {
            g_sane_object->sourceModesMap.insert("ADF", QString::fromStdString(*it));
            typeStringList << QObject::tr("ADF");

            ++g_sane_object->haveSourceFlag;
            continue;
        }

        it = find(adfFrontSources.begin(), adfFrontSources.end(), tmp);
        if ((it != adfFrontSources.end()) && (! g_sane_object->sourceModesMap.contains("ADF Front"))) {
            g_sane_object->sourceModesMap.insert("ADF Front", QString::fromStdString(*it));
            typeStringList << QObject::tr("ADF Front");

            ++g_sane_object->haveSourceFlag;
            continue;
        }

        it = find(adfBackSources.begin(), adfBackSources.end(), tmp);
        if ((it != adfBackSources.end()) && (! g_sane_object->sourceModesMap.contains("ADF Back"))) {
            g_sane_object->sourceModesMap.insert("ADF Back", QString::fromStdString(*it));
            typeStringList << QObject::tr("ADF Back");

            ++g_sane_object->haveSourceFlag;
            continue;
        }

        it = find(adfDuplexSources.begin(), adfDuplexSources.end(), tmp);
        if ((it != adfDuplexSources.end()) && (! g_sane_object->sourceModesMap.contains("ADF Duplex"))) {
            g_sane_object->sourceModesMap.insert("ADF Duplex", QString::fromStdString(*it));
            typeStringList << QObject::tr("ADF Duplex");

            ++g_sane_object->haveSourceFlag;
            continue;
        }
    }

    qDebug() << "Type: " << typeStringList;
    QMap<QString, QString>::iterator itMap;
    for (itMap=g_sane_object->sourceModesMap.begin(); itMap != g_sane_object->sourceModesMap.end(); ++itMap ) {
        qDebug() << "Iterator for sourceModesMap: " <<itMap.key() << itMap.value();
    }

    g_sane_object->setSaneTypes(typeStringList);

    return status;
}

SANE_Status setOptionSources(SANE_Handle sane_handle, int optnum, SANE_String val_source)
{
    SANE_Status status = SANE_STATUS_GOOD;

    qDebug() << "Set source option = " << val_source;

    status = sane_control_option(sane_handle, optnum,
                                 SANE_ACTION_SET_VALUE, val_source, nullptr);
    if (status != SANE_STATUS_GOOD) {
        qDebug() << "status = " << status << "desc: " << sane_strstatus(status);
        return status;
    }
    return status;
}

static SANE_Status getOptionResolutions(SANE_Handle sane_handle, int optnum)
{
    QStringList resolutions;
    SANE_Status status = SANE_STATUS_INVAL;
    const SANE_Option_Descriptor *opt;

    qDebug() << "Get resolution option = " << optnum;

    opt = sane_get_option_descriptor(sane_handle, optnum);
    int count = *(opt->constraint.word_list);
    for (int i = 1; i <= count; ++i) {
        int res = *(opt->constraint.word_list + i);
        status = SANE_STATUS_GOOD;

        switch (res) {
        case 4800:
            resolutions << QObject::tr("4800 dpi");
            break;
        case 2400:
            resolutions << QObject::tr("2400 dpi");
            break;
        case 1200:
            resolutions << QObject::tr("1200 dpi");
            break;
        case 600:
            resolutions << QObject::tr("600 dpi");
            break;
        case 300:
            resolutions << QObject::tr("300 dpi");
            break;
        case 200:
            resolutions << QObject::tr("200 dpi");
            break;
        case 150:
            resolutions << QObject::tr("150 dpi");
            break;
        case 100:
            resolutions << QObject::tr("100 dpi");
            break;
        case 75:
            resolutions << QObject::tr("75 dpi");
            break;
        default:
            break;
        }
    }
    // reverse sort
    qSort(resolutions.begin(), resolutions.end(), [](const QString & s1, const QString & s2) {
        // Get numbers from QString, and then sort

        int numberS1 = g_sane_object->getNumbersFromQString(s1);
        int numberS2 = g_sane_object->getNumbersFromQString(s2);

        return numberS1 < numberS2;
    });

    // Default resolution
    resolutions.insert(0, QObject::tr("Auto"));

    g_sane_object->setSaneResolutions(resolutions);

    return status;
}


static bool isInRange(int min, int max, int val)
{
    if (val >= min && val <= max) {
        return true;
    } else {
        return false;
    }
}

static SANE_Status getOptionResolutionsRange(int min, int max)
{
    QStringList resolutions;
    SANE_Status status = SANE_STATUS_GOOD;

    if (isInRange(min, max, 4800)) {
        resolutions << QObject::tr("4800 dpi");
    }

    if (isInRange(min, max, 2400)) {
        resolutions << QObject::tr("2400 dpi");
    }

    if (isInRange(min, max, 1200)) {
        resolutions << QObject::tr("1200 dpi");
    }

    if (isInRange(min, max, 600)) {
        resolutions << QObject::tr("600 dpi");
    }

    if (isInRange(min, max, 300)) {
        resolutions << QObject::tr("300 dpi");
    }

    if (isInRange(min, max, 200)) {
        resolutions << QObject::tr("200 dpi");
    }

    if (isInRange(min, max, 150)) {
        resolutions << QObject::tr("150 dpi");
    }

    if (isInRange(min, max, 100)) {
        resolutions << QObject::tr("100 dpi");
    }

    if (isInRange(min, max, 75)) {
        resolutions << QObject::tr("75 dpi");
    }

    qDebug() << "resolutions = " << resolutions;

    // reverse sort
    qSort(resolutions.begin(), resolutions.end(), [](const QString & s1, const QString & s2) {
        // Get numbers from QString, and then sort

        int numberS1 = g_sane_object->getNumbersFromQString(s1);
        int numberS2 = g_sane_object->getNumbersFromQString(s2);

        return numberS1 < numberS2;
    });

    // Default resolution
    resolutions.insert(0, QObject::tr("Auto"));

    g_sane_object->setSaneResolutions(resolutions);

    return status;
}

SANE_Status setOptionResolutions(SANE_Handle sane_handle, SANE_Int val_resolution)
{
    SANE_Status status = SANE_STATUS_GOOD;

    qDebug() << "Set resolution option = " << val_resolution;

    int index = -1;
    if(!g_sane_object->getSaneOptIndex(SANE_NAME_SCAN_RESOLUTION,index)){
        status = sane_control_option(sane_handle, g_optDesc.numResolution,
                                     SANE_ACTION_SET_VALUE, &val_resolution, nullptr);
    }else{
        status = sane_control_option(sane_handle,index,SANE_ACTION_SET_VALUE, &val_resolution, nullptr);
    }
    if (status != SANE_STATUS_GOOD) {
        qDebug() << "status = " << status << "desc: " << sane_strstatus(status);
        return status;
    }

    return status;
}


/**
 * @brief get_option_sizes
 *
 * Get all sizes[8,9,10,11] option: tl-x tl-y br-x br-y (etc.)
 *
 * A3: 0 0 297 420
 * A4: 0 0 210 297 (canon 210=> 0 0 216 297)
 *
 * Because sizes just can read normal, so we could directly to set general sizes
 *
 * @param sane_handle
 * @param optnum
 * @return sane status
 */
SANE_Status getOptionSizes(SANE_Handle sane_handle, int optnum)
{
    const SANE_Option_Descriptor *opt;
    SANE_Status status = SANE_STATUS_GOOD;

    qDebug() << "Get size option = " << optnum;

    opt = sane_get_option_descriptor(sane_handle, optnum);

    for (int i = 0; opt->constraint.word_list[i]; ++i) {
        qDebug() << "sizes = " << *(opt->constraint.word_list + i);
    }
    return status;
}

SANE_Status setOptionSizes(SANE_Handle sane_handle, int optnum, SANE_Int val_size)
{
    SANE_Status status = SANE_STATUS_GOOD;

    qDebug() << "Set size option = "<< optnum << val_size;

    status = sane_control_option(sane_handle, optnum,
                                 SANE_ACTION_SET_VALUE, &val_size, nullptr);

    if (status != SANE_STATUS_GOOD) {
        qDebug() << "status = " << status << "desc: " << sane_strstatus(status);
        return status;
    }

    return status;
}

static const SANE_Option_Descriptor *getOptdescByName(SANE_Handle device, const char *name, int *option_num)
{
    SANE_Int num_dev_options;
    SANE_Status status;

    /* Get the number of options. */
    status = sane_control_option (device, 0,
                                  SANE_ACTION_GET_VALUE, &num_dev_options, nullptr);

    qDebug() << "\n\n\nGet Option name: " << name << status;

    for (*option_num = 0; *option_num < num_dev_options; (*option_num)++) {
        const SANE_Option_Descriptor *opt;

        opt = sane_get_option_descriptor (device, *option_num);
        if (!opt || !opt->name) {
            qDebug() << "Option " << *option_num << " has no name";
            continue;
        }

        qDebug() << "Checking option " << *option_num << ": name=" << opt->name << ", searching for=" << name;

        // Direct match
        if (strcmp(opt->name, name) == 0) {
            qDebug() << "Get option desc for " << *option_num << "opt->name = " << opt->name << "name" << name << "(direct match)";
            return (opt);
        }

        // Special cases: when searching for "source", also check for alternative names
        if (strcmp("source", name) == 0) {
            qDebug() << "Searching for source, checking if opt->name matches alternatives...";
            if (strcmp(opt->name, "ScanMode") == 0) {
                // 适配佳能DR-C240的单双面扫描功能，因为佳能的单双面参数不叫source，叫ScanMode
                qDebug() << "Get option desc for " << *option_num << "opt->name = " << opt->name << "name" << name << "(ScanMode match)";
                return (opt);
            }
        }
    }
    return (nullptr);
}

SANE_Status setOptionSizesReal(SANE_Handle sane_handle, SANE_Int val_size_br_x,
                               SANE_Int val_size_br_y)
{
    SANE_Status status = SANE_STATUS_GOOD;
    qDebug() << "Size bottom-right location(xy) = " << val_size_br_x << val_size_br_y;
    int index = -1;
    const SANE_Option_Descriptor *opt;
    int optnum;
    if(g_sane_object->getSaneOptIndex(SANE_NAME_SCAN_BR_X,index)){
        opt = getOptdescByName(sane_handle, SANE_NAME_SCAN_BR_X, &optnum);
        if(opt->type == SANE_TYPE_INT){
            status = setOptionSizes(sane_handle, index, val_size_br_x);
        }else{
            status = setOptionSizes(sane_handle, index, SANE_FIX(val_size_br_x));
        }

        if (status != SANE_STATUS_GOOD) {
            qDebug() << "status = " << sane_strstatus(status);
            return status;
        }
        index = -1;
    }
    if(g_sane_object->getSaneOptIndex(SANE_NAME_SCAN_BR_Y,index)){

        opt = getOptdescByName(sane_handle, SANE_NAME_SCAN_BR_Y, &optnum);
        if(opt->type == SANE_TYPE_INT){
            status = setOptionSizes(sane_handle, index, val_size_br_y);
        }else{
            status = setOptionSizes(sane_handle, index, SANE_FIX(val_size_br_y));
        }
        if (status != SANE_STATUS_GOOD) {
            qDebug() << "status = " << sane_strstatus(status);
            return status;
        }
        index = -1;
    }
    if(g_sane_object->getSaneOptIndex(SANE_NAME_SCAN_TL_X,index)){
        status = setOptionSizes(sane_handle, index, SANE_FIX(0));
        if (status != SANE_STATUS_GOOD) {
            qDebug() << "status = " << sane_strstatus(status);
            return status;
        }
        index = -1;
    }
    if(g_sane_object->getSaneOptIndex(SANE_NAME_SCAN_TL_Y,index)){
        status = setOptionSizes(sane_handle, index, SANE_FIX(0));
        if (status != SANE_STATUS_GOOD) {
            qDebug() << "status = " << sane_strstatus(status);
            return status;
        }
        index = -1;
    }
    if(g_sane_object->getSaneOptIndex(SANE_NAME_PAGE_WIDTH,index)){
        status = setOptionSizes(sane_handle, index, SANE_FIX(val_size_br_x - 0));
        if (status != SANE_STATUS_GOOD) {
            qDebug() << "status = " << sane_strstatus(status);
            return status;
        }
        index = -1;
    }
    if(g_sane_object->getSaneOptIndex(SANE_NAME_PAGE_HEIGHT,index)){
        status = setOptionSizes(sane_handle, index, SANE_FIX(val_size_br_y - 0));
        if (status != SANE_STATUS_GOOD) {
            qDebug() << "status = " << sane_strstatus(status);
            return status;
        }
        index = -1;
    }
    return status;
}

SANE_Status setOptionSizesAll(SANE_Handle sane_handle, int type)
{
    SANE_Status status = SANE_STATUS_GOOD;

    switch (type) {
    case A2:
        status = setOptionSizesReal(sane_handle, 420, 594);
        break;
    case A3:
        status = setOptionSizesReal(sane_handle, 297, 420);
        break;
    case A4:
        status = setOptionSizesReal(sane_handle, 210, g_saneSizeA4BrY);
        break;
    case A5:
        status = setOptionSizesReal(sane_handle, 148, 210);
        break;
    case A6:
        status = setOptionSizesReal(sane_handle, 105, 144);
        break;
    default:
        status = SANE_STATUS_UNSUPPORTED;
    }

    return status;
}

SANE_Status setOptionPaperSize(SANE_Handle device, char *str, int option_num){
    SANE_Status status = sane_control_option(device, option_num, SANE_ACTION_SET_VALUE, str, nullptr);
    return status;
}

static void *guardsMalloc(size_t size)
{
    unsigned char *ptr;

    ptr = static_cast<unsigned char *>(malloc(size));

    assert(ptr);


    return (ptr);
}

static void guardsFree(void *ptr)
{
    unsigned char *p = static_cast<unsigned char *>(ptr);
    free(p);
}

void displayOptionValue(SANE_Handle device, int optnum)
{
    const SANE_Option_Descriptor *opt;

    opt = sane_get_option_descriptor(device, optnum);

    qDebug() << "Get options: optnum = " << optnum
             << "name = " << opt->name
             << "title = " << opt->title
             << "type = " << opt->type
             << "desc = " << opt->desc
             << "cap = " << opt->cap
             << "size = " << opt->size
             << "unit = " << opt->unit;
}

static SANE_Status getOptionValue(SANE_Handle device, const char *option_name)
{
    QStringList sizes;

    SANE_Status status = SANE_STATUS_GOOD;

    const SANE_Option_Descriptor *opt;
    int optnum;
    static char str[100];

    SANE_String val_string_source;
    SANE_String val_string_color;
    SANE_Word val_size;
    SANE_Word val_resolution;
    opt = getOptdescByName(device, option_name, &optnum);

    if (opt) {
        void *optval;
        optval = guardsMalloc(opt->size);
        /* Get default optval(different format) */
        status = sane_control_option(device, optnum, SANE_ACTION_GET_VALUE, optval, nullptr);

        if (opt->desc) {
            qDebug() << opt->desc;
        }

        qDebug() << "opt->type = " << opt->type;
        switch (opt->type) {
        case SANE_TYPE_INT:
            qDebug() << "type = int" << "size = " << opt->size;

            if (!strcmp(option_name, SANE_NAME_SCAN_RESOLUTION)) {
                val_resolution = *(SANE_Word *)optval;

                g_optDesc.numResolution = optnum;
                if (opt->constraint_type == SANE_CONSTRAINT_WORD_LIST) {
                    status = getOptionResolutions(device, optnum);
                } else if (opt->constraint_type == SANE_CONSTRAINT_RANGE){
                    // For scanner: Fujitsu fi-7140
                    int minResolution = 0;
                    int maxResolution = 0;
                    if (opt->type == SANE_TYPE_FIXED) {
                        qDebug() << "min = " << SANE_UNFIX(opt->constraint.range->min) \
                                 << "max = " << SANE_UNFIX(opt->constraint.range->max) \
                                 << "quant = " << opt->constraint.range->quant;

                        minResolution = SANE_UNFIX(opt->constraint.range->min);
                        maxResolution = SANE_UNFIX(opt->constraint.range->max);
                    } else {
                        qDebug() << "min = " << opt->constraint.range->min \
                                 << "max = " << opt->constraint.range->max \
                                 << "quant = " << SANE_UNFIX(opt->constraint.range->quant);

                        minResolution = opt->constraint.range->min;
                        maxResolution = opt->constraint.range->max;
                    }
                    qDebug() << "minResolution = " << minResolution
                             << "maxResolution = " << maxResolution;

                    status = getOptionResolutionsRange(minResolution, maxResolution);
                }

                qDebug() << "optnum = " << g_optDesc.numResolution
                         << "resolution = " << val_resolution
                         << "constraint_type = " << opt->constraint_type;
            }

            if(!strcmp(option_name, "scan-count")){
                g_optDesc.numPageCount = optnum;
            }
            if(!strcmp(option_name, SANE_NAME_SCAN_BR_X)){
                g_optDesc.numSizeBrX = optnum;
                // Via br_x to decide scan sizes
                int size_range = static_cast<int>( opt->constraint.range->max - opt->constraint.range->min);
                qDebug() << "min = " << opt->constraint.range->min
                         << "max = " << opt->constraint.range->max
                         << "size_range = " << size_range;
                if (val_size >= 420)
                    sizes << "A2";
                if (size_range >= 297)
                    sizes << "A3";
                if (size_range >= 209)
                    sizes << "A4";
                if (size_range >= 148)
                    sizes << "A5";
                if (size_range >= 105)
                    sizes << "A6";

                g_sane_object->setSaneSizes(sizes);
                qDebug() << "size optnum = " << g_optDesc.numSizeBrX
                         << "br_x" << val_size
                         << "constraint_type = " << opt->constraint_type;
            }
            if(!strcmp(option_name, SANE_NAME_SCAN_BR_Y)){
                g_optDesc.numSizeBrY = optnum;
                // Need via br_y to decide scan sizes after br_x, in case of (215.9, 296.926) to A4, which should be A5
                int size_range = static_cast<int>(opt->constraint.range->max - opt->constraint.range->min);
                double min = opt->constraint.range->min;
                double max = opt->constraint.range->max;
                qDebug() << "min = " << min
                         << "max = " << max
                         << "size_range = " << size_range;
                // Judge max(296.926) less than A4BRY(297)
                if ((qCeil(max) == A4BRY) && (qFloor(max) == A4BRY - 1)) {
                    qDebug() << "A4 br_y max = " << max;
                    g_saneSizeA4BrY = max;
                } else {
                    g_saneSizeA4BrY = A4BRY;
                }
                qDebug() << "size optnum = " << g_optDesc.numSizeBrY
                         << "br_y" << val_size
                         << "g_saneSizeA4Bry = " <<g_saneSizeA4BrY
                         << "constraint_type = " << opt->constraint_type;
            }
            break;
        case SANE_TYPE_BOOL:
            if (*(SANE_Word *) optval == SANE_FALSE) {
                strcpy(str, "FALSE");
            } else {
                strcpy(str, "TRUE");
            }
            qDebug() << "type = bool" << "size = " << opt->size << str;
            break;
        case SANE_TYPE_FIXED:
            qDebug() << "type = fixed" << "size = " << opt->size;

            val_size = SANE_UNFIX(*(SANE_Word *) optval);
            if (opt->constraint_type == SANE_CONSTRAINT_RANGE)
                status = getOptionSizes(device, optnum);

            qDebug() << "str_status = " << sane_strstatus(status);

            if (!strcmp(option_name, SANE_NAME_SCAN_TL_X)) {
                qDebug() << "size tl_x = " << val_size << "constraint_type = " << opt->constraint_type;
                g_optDesc.numSizeTlX = optnum;
            } else if (!strcmp(option_name, SANE_NAME_SCAN_TL_Y)) {
                qDebug() << "size tl_y = " << val_size << "constraint_type = " << opt->constraint_type;
                g_optDesc.numSizeTlY = optnum;
            } else if (!strcmp(option_name, SANE_NAME_PAGE_WIDTH)) {
                qDebug() << "size page width = " << val_size << "constraint_type = " << opt->constraint_type;
                g_optDesc.numPageWidth = optnum;
            } else if (!strcmp(option_name, SANE_NAME_PAGE_HEIGHT)) {
                qDebug() << "size page height = " << val_size << "constraint_type = " << opt->constraint_type;
                g_optDesc.numPageHeight = optnum;
            }else if (!strcmp(option_name, SANE_NAME_SCAN_BR_X)) {
                g_optDesc.numSizeBrX = optnum;
                // Via br_x to decide scan sizes
                int size_range = static_cast<int>SANE_UNFIX(opt->constraint.range->max);
                qDebug() << "min = " << SANE_UNFIX(opt->constraint.range->min)
                         << "max = " << SANE_UNFIX(opt->constraint.range->max)
                         << "size_range = " << size_range;
                {
                    if (size_range >= 420)
                        sizes << "A2";
                    if (size_range >= 297)
                        sizes << "A3";
                    if (size_range >= 209)
                        sizes << "A4";
                    if (size_range >= 148)
                        sizes << "A5";
                    if (size_range >= 105)
                        sizes << "A6";
                }
                g_sane_object->setSaneSizes(sizes);
                qDebug() << "size optnum = " << g_optDesc.numSizeBrX
                         << "br_x" << val_size
                         << "constraint_type = " << opt->constraint_type;
            } else if (!strcmp(option_name, SANE_NAME_SCAN_BR_Y)) {
                g_optDesc.numSizeBrY = optnum;
                // Need via br_y to decide scan sizes after br_x, in case of (215.9, 296.926) to A4, which should be A5
                int size_range = static_cast<int>( SANE_UNFIX(opt->constraint.range->max) \
                                                   - SANE_UNFIX(opt->constraint.range->min));
                double min = SANE_UNFIX(opt->constraint.range->min);
                double max = SANE_UNFIX(opt->constraint.range->max);
                qDebug() << "min = " << min
                         << "max = " << max
                         << "size_range = " << size_range;
                // Judge max(296.926) less than A4BRY(297)
                if ((qCeil(max) == A4BRY) && (qFloor(max) == A4BRY - 1)) {
                    qDebug() << "A4 br_y max = " << max;
                    g_saneSizeA4BrY = max;
                } else {
                    g_saneSizeA4BrY = A4BRY;
                }
                qDebug() << "size optnum = " << g_optDesc.numSizeBrY
                         << "br_y" << val_size
                         << "g_saneSizeA4Bry = " <<g_saneSizeA4BrY
                         << "constraint_type = " << opt->constraint_type;
            }

            break;
        case SANE_TYPE_STRING:
            qDebug() << "type = string" << "size = " << opt->size;

            if (!strcmp(option_name, SANE_NAME_SCAN_MODE)) {
                val_string_color = static_cast<SANE_String>(optval);
                g_optDesc.numColorMode = optnum;
                status = getOptionColors(device, optnum);
                qDebug() << "Default optnum = " << g_optDesc.numColorMode
                         << "color = " << val_string_color
                         << "constraint_type = " << opt->constraint_type;
            } else if (!strcmp(option_name, SANE_NAME_SCAN_SOURCE)) {
                val_string_source = static_cast<SANE_String>(optval);
                g_optDesc.numSource = optnum;
                status = getOptionSources(device, optnum);
                qDebug() << "Default optnum = " << g_optDesc.numSource
                         << "source = " << val_string_source
                         << "constraint_type = " << opt->constraint_type;
            } else if(!strcmp(option_name, "paper-size")){
                if(SANE_OPTION_IS_SETTABLE(opt->cap)){
                    setOptionPaperSize(device, QString("user_def").toUtf8().data(), optnum);
                }
            }else{
                // Canon Lide 400 local connect error
                status = SANE_STATUS_INVAL;
            }
            break;
        case SANE_TYPE_BUTTON:
            qDebug() << "type = button" << "size = " << opt->size;
            break;
        case SANE_TYPE_GROUP:
            qDebug() << "type = button" << "size = " << opt->size;
            break;
        default:
            qDebug() << "type = %d" << opt->type << "size = " << opt->size;
            break;
        }

        switch (opt->unit) {
        case SANE_UNIT_NONE:
            qDebug() << "unit = none";
            break;
        case SANE_UNIT_PIXEL:
            qDebug() << "unit = pixel";
            break;
        case SANE_UNIT_BIT:
            qDebug() << "unit = bit";
            break;
        case SANE_UNIT_MM:
            qDebug() << "unit = mm";
            break;
        case SANE_UNIT_DPI:
            qDebug() << "unit = dpi";
            break;
        case SANE_UNIT_PERCENT:
            qDebug() << "unit = percent";
            break;
        case SANE_UNIT_MICROSECOND:
            qDebug() << "unit = microsecond";
            break;
        default:
            qDebug() << "unit = " << opt->unit;
            break;
        }

        switch(opt->constraint_type){
        case SANE_CONSTRAINT_RANGE:
            if (opt->type == SANE_TYPE_FIXED) {
                qDebug() << "min = " << SANE_UNFIX(opt->constraint.range->min) \
                         << "max = " << SANE_UNFIX(opt->constraint.range->max) \
                         << "quant = " << opt->constraint.range->quant;
            } else {
                qDebug() << "min = " << opt->constraint.range->min \
                         << "max = " << opt->constraint.range->max \
                         << "quant = " << SANE_UNFIX(opt->constraint.range->quant);
            }
            break;
        case SANE_CONSTRAINT_WORD_LIST:
            for (int i = 0; i < opt->constraint.word_list[0]; ++i) {
                if (opt->type == SANE_TYPE_INT) {
                    qDebug() << opt->constraint.word_list[i + 1];
                } else {
                    qDebug() << SANE_UNFIX(opt->constraint.word_list[i + 1]);
                }
            }
            break;
        case SANE_CONSTRAINT_STRING_LIST:
            for (int i = 0; opt->constraint.string_list[i] != NULL; ++i) {
                qDebug() << opt->constraint.string_list[i];
            }
            break;
        default:
            qDebug() << "case = %d not found" ;
            break;
        }

        guardsFree(optval);
    } else {
        /* The option does not exists. */
        strcpy_s(str, sizeof(str), "backend default");

        qDebug() << "option_name = " << option_name << "str = " << str;
        if ((strcmp(str, "backend default") == 0)
                && (strcmp(option_name, SANE_NAME_SCAN_SOURCE) == 0)) {
            getOptionSourcesDefaultFailed();
        }
    }

    qDebug() << "option_name = " << option_name << "str = " << str;

    return status;
}

static SANE_Status showAllSaneParameters(SANE_Handle device)
{
    static char str[150];
    char *p = str;

    SANE_Status  status = SANE_STATUS_GOOD;

    *p = 0;

    status = getOptionValue(device, SANE_NAME_SCAN_SOURCE);
    if (status != SANE_STATUS_GOOD) {
        status = SANE_STATUS_INVAL;
        qDebug() << "source parameters error!";
        return status;
    }

    status = getOptionValue(device, SANE_NAME_SCAN_MODE);
    if (status != SANE_STATUS_GOOD) {
        status = SANE_STATUS_INVAL;
        qDebug() << "color mode parameters error!";
        return status;
    }

    status = getOptionValue(device, SANE_NAME_SCAN_RESOLUTION);
    if (status != SANE_STATUS_GOOD) {
        status = SANE_STATUS_INVAL;
        qDebug() << "resolution parameters error!";
        return status;
    }

    getOptionValue(device, "paper-size");

    // Default size coordination, top_left(x, y)
    getOptionValue(device, SANE_NAME_SCAN_TL_X);
    if (status != SANE_STATUS_GOOD) {
        status = SANE_STATUS_INVAL;
        qDebug() << "tl_x parameters error!";
        return status;
    }

    getOptionValue(device, SANE_NAME_SCAN_TL_Y);
    if (status != SANE_STATUS_GOOD) {
        status = SANE_STATUS_INVAL;
        qDebug() << "tl_y parameters error!";
        return status;
    }

    getOptionValue(device, SANE_NAME_PAGE_WIDTH);
    if (status != SANE_STATUS_GOOD) {
        status = SANE_STATUS_INVAL;
        qDebug() << "tl_y parameters error!";
        return status;
    }

    getOptionValue(device, SANE_NAME_PAGE_HEIGHT);
    if (status != SANE_STATUS_GOOD) {
        status = SANE_STATUS_INVAL;
        qDebug() << "tl_y parameters error!";
        return status;
    }
    /* Default size coordination, botton_right(x, y)
     *
     * Refer to backend/sharp.c, for A4 size:
     * s->val[OPT_BR_X].w = SANE_FIX(210);
     * s->val[OPT_BR_Y].w = SANE_FIX(297);
     */
    getOptionValue(device, SANE_NAME_SCAN_BR_X);
    if (status != SANE_STATUS_GOOD) {
        status = SANE_STATUS_INVAL;
        qDebug() << "br_x parameters error!";
        return status;
    }

    status = getOptionValue(device, SANE_NAME_SCAN_BR_Y);
    if (status != SANE_STATUS_GOOD) {
        status = SANE_STATUS_INVAL;
        qDebug() << "br_y parameters error!";
        return status;
    }

    return status;
}

void SaneObject::saneCancel(SANE_Handle sane_handle)
{
    qDebug() << "saneCancel()";

    if (g_sane_object->getSaneHaveHandle()) {
        g_sane_object->setSaneHaveHandle(false);

        qDebug() << "sane_cancel and sane_close";

        sane_cancel(sane_handle);
    }
}

bool SaneObject::detectSaneDevices()
{
    onDetection = true;
    if(saneHaveStart == true){
        sane_exit();
    }
    qDebug() << "detectSaneDevices();";
    QStringList names;
    SANE_Status sane_status;
    QString name;
    saneInit();
    saneHaveStart = true;

    //获取扫描设备列表
    sane_status = saneGetDevices(&g_deviceList);
    GlobalUserSignal::getInstance()->setDeviceList(g_deviceList);

    if (sane_status) {
        qDebug() << "Cannot get scan devices, sane_status = " << sane_status;
        g_sane_object->setSaneStatus(false);
        return false;
    }
    for (int i = 0; g_deviceList[i]; ++i) {
        qDebug() << "mark-Name: " << g_deviceList[i]->name
                 << "mark-Vendor: " << g_deviceList[i]->vendor
                 << "mark-Model: " << g_deviceList[i]->model
                 << "mark-Type: " << g_deviceList[i]->type;
        name = QString("%1 %2 %3").arg(g_deviceList[i]->vendor).arg( g_deviceList[i]->model).arg(g_deviceList[i]->name);

        names << name;
    }
    qDebug() << names;
    g_sane_object->setSaneNames(names);

    if (g_deviceList[0]) {
        g_sane_object->setSaneStatus(true);
        onDetection = false;
        return true;
    }else{
        g_sane_object->setSaneStatus(false);
        qDebug() << "find device set status false";
        onDetection = false;
        return false;
    }
}

void SaneObject::openSaneDevice(int index)
{
    qDebug() << "openSaneDevice";

    QStringList names;
    SANE_Status sane_status;
    QString name;
    do{
        /**
         * Deal with open same scanner device again meeting `SANE_STATUS_DEVICE_BUSY`,
         * Thus, should `sane_close` the open handle before next `sane_open`
         */
        if (g_sane_object->getSaneHaveHandle()) {
            // Avoid SANE_STATUS_BUSY status
            qDebug() << "begin to sane_close()";
            g_sane_object->setSaneHaveHandle(false);
            sane_close(g_sane_object->handle);
        }

        for (int i = 0; g_deviceList[i]; i++) {
            name = QString("%1 %2 %3").arg(g_deviceList[i]->vendor).arg(g_deviceList[i]->model).arg(g_deviceList[i]->name);
            names << name;
        }

        if (names.isEmpty()) {
            qDebug() << "No scan devices !";
        }
        qDebug() << names;
        g_sane_object->setSaneNames(names);

        if (!g_deviceList[0]) {
            qDebug() << "No scan devices !";
            sane_status = SANE_STATUS_UNSUPPORTED;
            break;
        }

        qDebug() << "Open a scan device, plese waiting ...";
        SANE_Handle sane_handle;
        g_saneDevice = const_cast<SANE_Device *>(*(g_deviceList + index));
        g_sane_object->devicemodel = g_saneDevice->model;
        if (!g_saneDevice) {
            qDebug() << "No device connected!";
            sane_status = SANE_STATUS_UNSUPPORTED;
            break;
        }
        SANE_Device tmp;
        tmp.model = g_saneDevice->model;
        tmp.name = g_saneDevice->name;
        tmp.type = g_saneDevice->type;
        tmp.vendor = g_saneDevice->vendor;
        GlobalUserSignal::getInstance()->setDeviceInUse(tmp);

        sane_status = saneOpen(g_saneDevice, &sane_handle);
        if (sane_status) {
            qDebug() << "First open a device failed!";
            g_sane_object->setSaneHaveHandle(false);
            break;
        }
        g_sane_object->handle = sane_handle;
        g_sane_object->setSaneHaveHandle(true);

        sane_status = showAllSaneParameters(sane_handle);
        emit g_user_signal->hotPlugScanCompleteSignal();
    }while(0);

    if (sane_status) {
        g_sane_object->setSaneStatus(false);
        qDebug() << "open device set status false";
    } else {
        g_sane_object->setSaneStatus(true);
    }
}
#ifdef __cplusplus
}
#endif

SaneObject::SaneObject(QObject *parent) : QObject(parent)
{
    devicesInfo.have_handle = false;
    devicesInfo.status = false;
    devicesInfo.haveOpenSaneDevice = false;
    devicesInfo.name << "";
    devicesInfo.type << "";
    devicesInfo.color << "";
    devicesInfo.resolution << "";
    devicesInfo.size << "";

    connect(g_user_signal, &GlobalUserSignal::startUsbHotPlugThreadSignal, this, &SaneObject::getScannerSettings);    // 获取扫描仪的初始化参数
    connect(g_user_signal, &GlobalUserSignal::saneRestartSignal, this, &SaneObject::restartSaneForException);
    connect(g_user_signal, &GlobalUserSignal::openDeviceSignal,this,&SaneObject::openSaneDeviceForPage);
    connect(g_user_signal, &GlobalUserSignal::refreshListSignal, this, &SaneObject::refreshListSlots);
}

SaneObject::~SaneObject()
{
}

void SaneObject::stopSaneRead(bool isStoped)
{
    stopSaneReadFlag = isStoped;
}

SaneObject *SaneObject::getInstance()
{
    if (! instance) {
        instance = new SaneObject;
    }
    return instance;
}

bool SaneObject::getSaneStatus()
{
    return devicesInfo.status;
}

bool SaneObject::getSaneHaveOpenDevice()
{
    return devicesInfo.haveOpenSaneDevice;
}

bool SaneObject::getSaneHaveHandle()
{
    return devicesInfo.have_handle;
}

QStringList SaneObject::getSaneNames()
{
    return devicesInfo.name;
}

QStringList SaneObject::getSaneTypes()
{
    return devicesInfo.type;
}

QStringList SaneObject::getSaneResolutions()
{
    return devicesInfo.resolution;

}

int SaneObject::getNumbersFromQString(const QString &qstring)
{
    QString numbers;
    foreach (QChar ch, qstring) {
        if (ch.isDigit()) {
            numbers += ch;
        }
    }
    return numbers.toInt();
}

QStringList SaneObject::getSaneSizes()
{
    return devicesInfo.size;
}

QStringList SaneObject::getSaneColors()
{
    return devicesInfo.color;
}

void SaneObject::setSaneHaveOpenSaneDevice(bool haveOpenSaneDevice)
{
    devicesInfo.haveOpenSaneDevice = haveOpenSaneDevice;
}

void SaneObject::setSaneHaveHandle(bool have_handle)
{
    devicesInfo.have_handle = have_handle;
}

void SaneObject::setSaneStatus(bool status)
{
    devicesInfo.status = status;
}

void SaneObject::setSaneTypes(QStringList type)
{
    devicesInfo.type = type;
}

void SaneObject::setSaneNames(QStringList name)
{
    devicesInfo.name = name;
}

void SaneObject::setSaneResolutions(QStringList resolution)
{
    devicesInfo.resolution = resolution;
}

void SaneObject::setSaneSizes(QStringList size)
{
    devicesInfo.size = size;
}

void SaneObject::setSaneColors(QStringList color)
{
    devicesInfo.color = color;
}

void SaneObject::saneExit()
{
    sane_exit();
}

void SaneObject::saneClose()
{
    if (g_sane_object->getSaneHaveHandle()) {
//        saneCancel(g_sane_object->handle);
        qDebug() << getSaneNames();
        sane_close(g_sane_object->handle);
        setSaneHaveHandle(false);
    }
    stopSaneRead(true);
}
/*
 * #define SANE_NAME_SCAN_MODE		"mode"
#define SANE_NAME_SCAN_SPEED		"speed"
#define SANE_NAME_SCAN_SOURCE		"source"
#define SANE_NAME_SCAN_TL_X		"tl-x"
#define SANE_NAME_SCAN_TL_Y		"tl-y"
#define SANE_NAME_SCAN_BR_X		"br-x"
#define SANE_NAME_SCAN_BR_Y		"br-y"
#define SANE_NAME_SCAN_RESOLUTION	"resolution"
#define SANE_NAME_SCAN_X_RESOLUTION	"x-resolution"
#define SANE_NAME_SCAN_Y_RESOLUTION	"y-resolution"
#define SANE_NAME_PAGE_WIDTH  		"page-width"
#define SANE_NAME_PAGE_HEIGHT 		"page-height"
*/

void SaneObject::dumpScannerOptions()
{
    m_saneOptions.clear();
    for(auto opt = 0; const SANE_Option_Descriptor *desc = sane_get_option_descriptor(g_sane_object->handle,opt);opt++)
    {
        qDebug ()<<opt << desc->name << desc->desc;
        if(QString::compare(SANE_NAME_SCAN_MODE,desc->name) == 0 ||
                QString::compare(SANE_NAME_SCAN_SOURCE,desc->name) == 0 ||
                QString::compare(SANE_NAME_SCAN_RESOLUTION,desc->name) == 0 ||
                QString::compare(SANE_NAME_SCAN_TL_X,desc->name) == 0 ||
                QString::compare(SANE_NAME_SCAN_TL_Y,desc->name) == 0 ||
                QString::compare(SANE_NAME_SCAN_BR_X,desc->name) == 0 ||
                QString::compare(SANE_NAME_SCAN_BR_Y,desc->name) == 0 ||
                QString::compare(SANE_NAME_PAGE_WIDTH,desc->name) == 0 ||
                QString::compare(SANE_NAME_PAGE_HEIGHT,desc->name) == 0) {
            m_saneOptions.insert(desc->name,opt);
        }
    }
    qDebug() << m_saneOptions;
}
void SaneObject::setSaneAllParametersByUser()
{
    dumpScannerOptions();
    setSanePageNumberByUser();
    setSaneTypeByUser();
    setSaneColorByUser();
    setSaneResolutionByUser();
    setSaneSizeByUser();
    setSaneFormatByUser();
}

void SaneObject::setSanePageNumberByUser()
{
    if(g_sane_object->devicemodel == "HW-3130"){
        QString userPageNum = g_sane_object->userInfo.pageNumber;
        if(userPageNum == "单页扫描" || userPageNum == "Single" || userPageNum == QApplication::tr("Single")){
            setOptionPages(g_sane_object->handle, 1);
        }else{
            setOptionPages(g_sane_object->handle, 0);
        }
    }
}

void SaneObject::setSaneTypeByUser()
{
    SANE_Status status = SANE_STATUS_GOOD;
    std::string strType;
    SANE_String s_type;
    QMap<QString, QString>::iterator it;

    QString type = getSaneTypeByUser(g_sane_object->userInfo.type);
    if (type == "") {
        // avoid crashed unexpected
        return ;
    }

    it = g_sane_object->sourceModesMap.find(type);
    if (it != g_sane_object->sourceModesMap.end()) {
        // `type` is the key of `sourceModesMap`, we need to get the value of `sourceModesMap`,
        qDebug() << "sourceModesMap value: " << it.value();
        strType = it.value().toStdString();
    }

    s_type = const_cast<SANE_String>(strType.c_str());

    status = setOptionSources(g_sane_object->handle, g_optDesc.numSource, s_type);
    if (status != SANE_STATUS_GOOD) {
        qDebug() << "Failed to set option color: " << type
                 << "status = " << sane_strstatus(status);
    }
}

QString SaneObject::getSaneTypeByUser(QString type)
{
    if (QString::compare(type, tr("Flatbed"), Qt::CaseInsensitive) == 0) {
        return QString("Flatbed");
    } else if (QString::compare(type, tr("ADF"), Qt::CaseInsensitive) == 0) {
        return QString("ADF");
    } else if (QString::compare(type, tr("ADF Front"), Qt::CaseInsensitive) == 0) {
        return QString("ADF Front");
    } else if (QString::compare(type, tr("ADF Back"), Qt::CaseInsensitive) == 0) {
        return QString("ADF Back");
    } else if (QString::compare(type, tr("Default Type"), Qt::CaseInsensitive) == 0) {
        return QString("");
    } else if (QString::compare(type, tr("ADF Duplex"), Qt::CaseInsensitive) == 0) {
        return QString("ADF Duplex");
    } else {
        return QString("");
    }
}

void SaneObject::setSaneColorByUser()
{
    SANE_Status status = SANE_STATUS_GOOD;
    std::string strColor;
    SANE_String s_color;
    QMap<QString, QString>::iterator it;

    QString userColor = g_sane_object->userInfo.color;

    QString color = getSaneColorByUser(userColor);
    qDebug() << "userColor: " << color;
    if(color == "NULL"){
        qDebug() << "can not set color!";
        return;
    }
    it = g_sane_object->colorModesMap.find(color);
    if (it != g_sane_object->colorModesMap.end()) {
        strColor = it.value().toStdString();
    }

    s_color = const_cast<SANE_String>(strColor.c_str());


    status = setOptionColors(g_sane_object->handle, s_color);
    if (status != SANE_STATUS_GOOD) {
        qDebug() << "Failed to set option color: " << color
                 << "status = " << sane_strstatus(status);
    }
}

QString SaneObject::getSaneColorByUser(QString color)
{
    if ((QString::compare(color, "Lineart", Qt::CaseInsensitive) == 0)
            || (QString::compare(color, "黑白", Qt::CaseInsensitive) == 0)) {
        return QString("Lineart");
    } else if ((QString::compare(color, "Gray", Qt::CaseInsensitive) == 0)
               || (QString::compare(color, "Gray", Qt::CaseInsensitive) == 0)) {
        return QString("Gray");
    } else if ((QString::compare(color, "Color", Qt::CaseInsensitive) == 0)
               || (QString::compare(color, "彩色", Qt::CaseInsensitive) == 0)) {
        // Cannot match tr("Color"), maybe segment fault
        return QString("Color");
    }else{
        return QString("NULL");
    }
}

void SaneObject::setSaneResolutionByUser()
{
    SANE_Status status = SANE_STATUS_GOOD;
    QString resolution = g_sane_object->userInfo.resolution;
    SANE_Int i_resolution;
    qDebug() << "resolution = " << resolution;
    resolutionValue = getSaneResolutionByUser(resolution);
    i_resolution = static_cast<SANE_Int>(resolutionValue);
    resolution_value = i_resolution;
    status = setOptionResolutions(g_sane_object->handle, i_resolution);
    if (status != SANE_STATUS_GOOD) {
        qDebug() << "Failed to set option resolution: " << resolution
                 << "status = " << sane_strstatus(status);
    }
}

int SaneObject::getSaneResolutionByUser(QString resolution)
{
    if (QString::compare(resolution, QApplication::tr("Auto"), Qt::CaseInsensitive) == 0
            || QString::compare(resolution, "自动", Qt::CaseInsensitive) == 0
            || QString::compare(resolution, "Auto", Qt::CaseInsensitive) == 0) {
        return QString(QString(g_sane_object->getSaneResolutions()[1]).split(" ")[0]).trimmed().toInt();
    } else if (QString::compare(resolution, tr("75 dpi"), Qt::CaseInsensitive) == 0) {
        return 75;
    } else if (QString::compare(resolution, tr("100 dpi"), Qt::CaseInsensitive) == 0){
        return 100;
    } else if (QString::compare(resolution, tr("150 dpi"), Qt::CaseInsensitive) == 0) {
        return 150;
    } else if (QString::compare(resolution, tr("200 dpi"), Qt::CaseInsensitive) == 0) {
        return 200;
    } else if (QString::compare(resolution, tr("300 dpi"), Qt::CaseInsensitive) == 0) {
        return 300;
    } else if (QString::compare(resolution, tr("600 dpi"), Qt::CaseInsensitive) == 0) {
        return 600;
    } else if (QString::compare(resolution, tr("1200 dpi"), Qt::CaseInsensitive) == 0) {
        return 1200;
    } else if (QString::compare(resolution, tr("2400 dpi"), Qt::CaseInsensitive) == 0) {
        return 2400;
    } else if (QString::compare(resolution, tr("4800 dpi"), Qt::CaseInsensitive) == 0) {
        return 4800;
    }
    return 100;
}

void SaneObject::setSaneSizeByUser()
{
    SANE_Status status = SANE_STATUS_GOOD;
    std::string strSize;
    SANE_String s_size;
    int type = 0;

    strSize = g_sane_object->userInfo.size.toStdString();

    s_size = const_cast<SANE_String>(strSize.c_str());
    if (strcmp("A2", s_size) == 0) {
        type = A2;
    } else if (strcmp("A3", s_size) == 0) {
        type = A3;
    } else if (strcmp("A4", s_size) == 0) {
        type = A4;
    } else if (strcmp("A5", s_size) == 0) {
        type = A5;
    } else if (strcmp("A6", s_size) == 0) {
        type = A6;
    }
    status = setOptionSizesAll(g_sane_object->handle, type);
    if (status != SANE_STATUS_GOOD) {
        qDebug() << "Failed to set option size: " << s_size
                 << "status = " << sane_strstatus(status);
    }
}

void SaneObject::setSaneFormatByUser()
{

}

QString SaneObject::getSaneFormatByUser()
{
    QString format = g_sane_object->userInfo.format;

    return format;
}

QString SaneObject::getSaneSaveNameByUser()
{
    QString saveName = g_sane_object->userInfo.saveName;

    return saveName;
}

QString SaneObject::getSaneLoadNameByUser()
{
    QString loadName = g_sane_object->userInfo.loadName;

    return loadName;
}

QString SaneObject::getSaneSaveDirectoryByUser()
{
    QString saveDirectory = g_sane_object->userInfo.saveDirectory;

    return saveDirectory;
}

QString SaneObject::getSaneTmpSaveDirectory()
{
    QString tmpSaveDirectory = g_config_signal->m_scannerPnmImagePath;
    return tmpSaveDirectory;
}

void SaneObject::restartSaneForException(){
    openSaneDevice(openDeviceIndex);
}

void SaneObject::openSaneDeviceForPage(int index)
{    
    openSaneDevice(index);
}

QString SaneObject::fileNameOperation(){
    QString origSaveName = getSaneSaveNameByUser();
    QString saveFullName_tmp;

    QString pageNumber = g_sane_object->userInfo.pageNumber;

    saveFullName_tmp = origSaveName;

    return saveFullName_tmp;
}
QString SaneObject::getFullScanFileNameExceptFormatForSave()
{
    QString saveDirectory = getSaneSaveDirectoryByUser();

    QString saveFullName_tmp = getSaneSaveNameByUser();

    QString saveFullName = saveDirectory + "/" + saveFullName_tmp;
    qDebug() << "saveFullName = " << saveFullName;

    return saveFullName;
}

QString SaneObject::getFullScanFileNameExceptFormatForPnmLoad()
{
    QString tmpSaveDirectory = getSaneTmpSaveDirectory();

    QString loadFullName_tmp = getSaneLoadNameByUser();

    QString loadFullName = tmpSaveDirectory + "/" + loadFullName_tmp;
    qDebug() << "loadFullName = " << loadFullName;

    return loadFullName;
}

/**
 * @brief Sane::startScanning
 * Through parameters setted by user, we could do this:
 *      - 1. Set parameters which are setted by user;
 *      - 2. Do scanning actually.
 *
 * @param info parameters setted by user
 * @return scanning result
 */
int SaneObject::startScanning(UserSelectedInfo info)
{
    SANE_Status status = SANE_STATUS_GOOD;
    if(!g_sane_object->m_ParametersHaveSeted){
        qDebug() << "Setting Parameter...";
        setSaneAllParametersByUser();
        g_sane_object->m_ParametersHaveSeted = true;
    }

    saveFullScanFileName = getFullScanFileNameExceptFormatForSave();
    loadFullScanFileName = getFullScanFileNameExceptFormatForPnmLoad();
    // 获取完成后置空
    g_sane_object->userInfo.loadName = "";

    qDebug() << "Start scanning, please waiting ...";
    g_device = g_sane_object->handle;
    status = doScan(loadFullScanFileName.toStdString().c_str());

    // 存储当前扫描仪配置
    saveScannerSettings();

    qDebug() << "Scan Finish, check status:  " << status;

    qDebug() << "saveText nowSaveName = " << nowSaveName;

    return status;
}

void SaneObject::saveScannerSettings()
{
    ScannerSettings scannerSetting;

    QString scannerName = userInfo.name;   // 需要剔除掉bus相关信息
    QStringList vendorAndModel = scannerName.split(" ");
    scannerName = vendorAndModel[0] + " " + vendorAndModel[1];

    scannerSetting.DeviceName = scannerName;     //设备名称
    scannerSetting.Type = userInfo.type;
    scannerSetting.Pages = userInfo.pageNumber;
    scannerSetting.Type = userInfo.type;
    scannerSetting.Color = userInfo.color;
    scannerSetting.Resolution = userInfo.resolution;
    scannerSetting.PageSize = userInfo.size;
    scannerSetting.Format = userInfo.format;
    scannerSetting.SavePath = userInfo.saveDirectory;

    qDebug() << "----------------------------------";
    qDebug() << "DeviceName" << userInfo.name;
    qDebug() << "Pages" << userInfo.pageNumber;
    qDebug() << "Type" << userInfo.type;
    qDebug() << "Color" << userInfo.color;
    qDebug() << "Resolution" << userInfo.resolution;
    qDebug() << "PageSize" << userInfo.size;
    qDebug() << "Format" << userInfo.format;
    qDebug() << "SavePath" << userInfo.saveDirectory;
    qDebug() << "----------------------------------";

    // scanner
    if(m_scannerSettingMap.isEmpty()){
        m_scannerSettingMap.append(scannerSetting);
    }else{
        bool flag = 0; //用于记录是否在原表中存在
        int position = 0;
        for (int i = 0; i < m_scannerSettingMap.length(); ++i) {
            if(m_scannerSettingMap[i].DeviceName == scannerSetting.DeviceName){
                flag = 1;
                position = i;
            }
        }
        if(flag){
            m_scannerSettingMap.removeAt(position);
            m_scannerSettingMap.append(scannerSetting);
        }else{
            m_scannerSettingMap.append(scannerSetting);
        }

    }
    bool result = convertAndSave();

}

bool SaneObject::convertAndSave()
{
    QJsonArray jsonArray;

    for (const auto &hardwareInfo: m_scannerSettingMap) {
        QJsonObject jsonObject;
        jsonObject["DeviceName"] = hardwareInfo.DeviceName;
        jsonObject["Pages"] = hardwareInfo.Pages;
        jsonObject["Type"] = hardwareInfo.Type;
        jsonObject["Color"] = hardwareInfo.Color;
        jsonObject["Resolution"] = hardwareInfo.Resolution;
        jsonObject["PageSize"] = hardwareInfo.PageSize;
        jsonObject["Format"] = hardwareInfo.Format;
        jsonObject["SavePath"] = hardwareInfo.SavePath;

        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDocument(jsonArray);
    QString path = QDir::homePath() + "/.config/.kylin_scanner_config.json";
    QFile file(path);
    if(file.open(QIODevice::WriteOnly)) {
        file.write(jsonDocument.toJson(QJsonDocument::Indented));
        file.close();
        return true;
    }else{
        qDebug() << "fail to wirte" << path;
        return false;
    }
}

void SaneObject::getScannerSettings()
{
    QString path = QDir::homePath() + "/.config/.kylin_scanner_config.json";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件:" << path;
        return;
    }
    QTextStream in(&file);
    QString jsonString = in.readAll();
    file.close();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject scannerSettingJson = jsonValue.toObject();
        ScannerSettings scannerSetting;
        scannerSetting.DeviceName = scannerSettingJson["DeviceName"].toString();
        scannerSetting.Pages = scannerSettingJson["Pages"].toString();
        scannerSetting.Type = scannerSettingJson["Type"].toString();
        scannerSetting.Color = scannerSettingJson["Color"].toString();
        scannerSetting.Resolution = scannerSettingJson["Resolution"].toString();
        scannerSetting.PageSize = scannerSettingJson["PageSize"].toString();
        scannerSetting.Format = scannerSettingJson["Format"].toString();
        scannerSetting.SavePath = scannerSettingJson["SavePath"].toString();

        bool flag = 0;
        for (int i = 0; i < m_scannerSettingMap.length(); ++i) {
            if(m_scannerSettingMap[i].DeviceName == scannerSetting.DeviceName){
                flag = 1;
            }
        }
        if(!flag){
            m_scannerSettingMap.append(scannerSetting);
        }
    }
    // 当前设备名称
    QString scannerName = userInfo.name;
    QStringList vendorAndModel = scannerName.split(" ");
    scannerName = vendorAndModel[0] + " " + vendorAndModel[1];

    for (int i = 0; i < m_scannerSettingMap.length(); ++i) {
        if(m_scannerSettingMap[i].DeviceName == scannerName){
        // 暂时屏蔽功能等2603再合入
            emit g_user_signal->setScannerTypeSignal(m_scannerSettingMap[i].Type);
            emit g_user_signal->setPageNumberSignal(m_scannerSettingMap[i].Pages);
            emit g_user_signal->setScannerColorSignal(m_scannerSettingMap[i].Color);
            emit g_user_signal->setScannerResolutionSignal(m_scannerSettingMap[i].Resolution);
            emit g_user_signal->setScannerPageSizeSignal(m_scannerSettingMap[i].PageSize);
            emit g_user_signal->setScannerFormatSignal(m_scannerSettingMap[i].Format);
            emit g_user_signal->setScannerSavePathSignal(m_scannerSettingMap[i].SavePath);

            break;
        }
    }
}

bool SaneObject::testScannerIsAlive(QString deviceName)
{
    qDebug() << "deviceName= " << deviceName;
    return true;
}

void SaneObject::hotPlugScanCompleteSlot()
{
    usbAddSlotIsRunning = false;
    usbRemoveSlotIsRunning = false;
}

bool SaneObject::getSaneOptIndex(QString desc,int &index)
{
    auto k = m_saneOptions.find(desc);
    if(k==m_saneOptions.end()){
        return false;
    } else {
        qDebug() << "Found" << k.key() << k.value();
        index = k.value();
        return true;
    }
    return false;
}
