/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "promptdialog.h"
#include "ui_promptdialog.h"
#include <QMovie>
#include <QPainter>
#include <QProcess>
#include <QPushButton>
#include <QDBusUnixFileDescriptor>
#include <QToolTip>
#include <QDebug>
#include <QKeyEvent>
#include <QStandardItemModel>
#include <QPainterPath>
#include <QPainter>
#include <pwd.h>
#include <unistd.h>
#include <QImageReader>
#include <QBuffer>
#include <QByteArray>
#include "servicemanager.h"
#include "giodbus.h"
#include <ukuiwindowhelper.h>

#define SS_DBUS_SERVICE "org.ukui.ScreenSaver"
#define SS_DBUS_PATH "/"
#define SS_DBUS_INTERFACE "org.ukui.ScreenSaver"

PromptDialog::PromptDialog(QDBusInterface *service, int bioType, int deviceId, int uid, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PromptDialog)
    , serviceInterface(service)
    , type(bioType)
    , deviceId(deviceId)
    , uid(uid)
    , ops(IDLE)
    , m_hisOps(IDLE)
    , isProcessed(false)
    , opsResult(UNDEFINED)
    , w_timer(nullptr)
    , m_enrollTimer(nullptr)
{
    ui->setupUi(this);
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);
    ui->treeViewResult->setAttribute(Qt::WA_Hover, true);
    ui->treeViewResult->installEventFilter(this);
    ui->btnClose->setFlat(true);
    ui->btnClose->setProperty("isWindowButton", 0x2);
    ui->btnClose->setProperty("useIconHighlightEffect", 0x8);
    ui->btnClose->setFixedSize(30, 30);
    ui->btnClose->setIconSize(QSize(16, 16));
    ui->btnClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    ui->btnClose->setToolTip(tr("Close"));
    ui->Ctbtn->hide();
    ui->Ctbtn->setProperty("useButtonPalette", true);
    ui->Fsbtn->hide();
    ui->Fsbtn->setProperty("isImportant", true);
    ui->cancelBtn->hide();
    ui->cancelBtn->setProperty("useButtonPalette", true);

    /* 设置 CSS */
    QFile qssFile(":/css/assets/promptdialog.qss");
    qssFile.open(QFile::ReadOnly);
    this->setStyleSheet(qssFile.readAll());
    qssFile.close();
    ui->treeViewResult->hide();
    ui->lblImage->setPixmap(getImage(type));
    setTipPixmap();

    movie = new QMovie(getGif(type));
    movie->setScaledSize(QSize(156, 156));

    if (!m_enrollTimer) {
        getEnrollImageList(type);
        m_enrollTimer = new QTimer(this);
        connect(m_enrollTimer, &QTimer::timeout, this, &PromptDialog::switchEnrollImage);
        // 设置初始图片
        m_currentImage = 0;
        updateEnrollImage();
    }

    connect(serviceInterface, SIGNAL(StatusChanged(int, int)), this, SLOT(onStatusChanged(int, int)));

    ServiceManager *sm = ServiceManager::instance();
    connect(sm, &ServiceManager::serviceStatusChanged, this, [&](bool activate) {
        if (!activate) {
            close();
        }
    });

    connect(
        serviceInterface,
        SIGNAL(ProcessChanged(int, QString, int, QString)),
        this,
        SLOT(onProcessChanged(int, QString, int, QString)));

    connect(serviceInterface, SIGNAL(FrameWritten(int)), this, SLOT(onFrameWritten(int)));

    if (connect(serviceInterface, SIGNAL(EnrollFrameWritten(int)), this, SLOT(onEnrollFrameWritten(int)))) {
        m_isUseEnrollFd = true;
        if (!w_timer) {
            w_timer = new QTimer(this);
            w_timer->setInterval(150);
            connect(w_timer, &QTimer::timeout, this, &PromptDialog::updatePixmap);
        }
        m_waitingPixmap = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(24, 24);
        w_timer->start();
        ui->cancelBtn->show();
        ui->Fsbtn->hide();
        ui->lblImage->setPixmap(m_waitingPixmap);
        setTipPixmap();
        qDebug() << "Connect onEnrollFrameWritten Succeed!!";
    }

    QDBusInterface *iface = new QDBusInterface(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        QDBusConnection::systemBus(),
        this);
    connect(iface, SIGNAL(PrepareForSleep(bool)), this, SLOT(onPrepareForSleep(bool)));

    QDBusInterface *interfaceScreensaver
        = new QDBusInterface(SS_DBUS_SERVICE, SS_DBUS_PATH, SS_DBUS_INTERFACE, QDBusConnection::sessionBus(), this);

    connect(interfaceScreensaver, SIGNAL(lock()), this, SLOT(onSysLock()));
    connect(interfaceScreensaver, SIGNAL(unlock()), this, SLOT(onSysUnLock()));
    // 去除窗管标题栏
    UkuiWindowHelper *pUkuiWindowHelper = new UkuiWindowHelper(this);
    pUkuiWindowHelper->removeTitleBar();
}

PromptDialog::~PromptDialog()
{
    delete ui;
}

bool PromptDialog::eventFilter(QObject *editor, QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event); // Tooptip弹出事件，帮助
        QPoint p_Top = helpEvent->globalPos();                    // Tooptip的显示位置，全局坐标
        QPoint p_View
            = ui->treeViewResult->viewport()->mapFromGlobal(QCursor::pos()); // 将鼠标的全局坐标转换为treeview中的坐标
        QModelIndex currentIndex = ui->treeViewResult->indexAt(p_View); // 获取鼠标所指向的Index
        QString name = currentIndex.data(Qt::DisplayRole).toString();
        QToolTip::showText(p_Top, name); // 显示ToolTip
        return true;
    }
    return QWidget::eventFilter(editor, event);
}

void PromptDialog::setTitle(int opsType)
{
    QString title = EnumToString::lowerTransferBioType(type);
    switch (opsType) {
        case ENROLL:
            ui->titleLabel->setText(tr("Enroll ") + title + tr(" feature"));
            title += tr("Enroll ");
            break;
        case VERIFY:
            title += tr("Verify ");
            break;
        case SEARCH:
            title += tr("Search ");
            break;
    }
}

/**
 * @brief 设置要显示的信息
 * @param text
 */
void PromptDialog::setPrompt(const QString &text)
{
    ui->lblPrompt->hide();
    ui->lblPrompt->setText(text);
    ui->lblPrompt->setWordWrap(true);
    ui->lblPrompt->adjustSize();
    ui->lblPrompt->show();
}

void PromptDialog::setIsFace(bool val)
{
    isFace = val;
}

void PromptDialog::setProcessed(bool val)
{
    isProcessed = val;
    if (isProcessed) {
        if (w_timer) {
            w_timer->stop();
        }
        ui->lblImage->setPixmap(QPixmap("/usr/share/ukui-biometric/images/huawei/00.svg"));
        setTipPixmap();
    } else {
        ui->lblImage->setPixmap(getImage(type));
        setTipPixmap();
        if (!movie && type != BIOTYPE_FINGERPRINT) {
            movie = new QMovie(getGif(type));
            movie->setScaledSize(QSize(156, 156));
        }
    }
}

void PromptDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (ui->btnClose->isEnabled()) {
            on_btnClose_clicked();
            accept();
        }
        return;
    }
}

void PromptDialog::on_btnClose_clicked()
{
    close();
}

void PromptDialog::on_Fsbtn_clicked()
{
    close();
}

void PromptDialog::on_cancelBtn_clicked()
{
    close();
}

void PromptDialog::on_Ctbtn_clicked()
{
    resetUI();
    w_timer->start();

    if (isProcessed) {
        if (w_timer) {
            w_timer->stop();
        }
        ui->lblImage->setPixmap(QPixmap("/usr/share/ukui-biometric/images/huawei/00.svg"));
        setTipPixmap();
    }

    int num = 1;
    int freeIndex;
    QList<int> idxList;
    QStringList list = getAllFeatureinfolist(getuid(), 0, -1, type, idxList);
    QString featurename;
    while (1) {
        featurename = EnumToString::lowerTransferBioType(type) + QString::number(num);
        if (!list.contains(featurename))
            break;
        num++;
    }
    num = 0;
    while (1) {
        if (!idxList.contains(num)) {
            freeIndex = num;
            break;
        }
        num++;
    }
    enroll(deviceId, getuid(), freeIndex, featurename);
}

void PromptDialog::resetUI()
{
    ui->Ctbtn->hide();
    ui->Fsbtn->hide();
    ui->cancelBtn->show();
    ui->titleLabel->show();
    ui->lblPrompt->show();
    ui->msgLabel->hide();

    if ((type == BIOTYPE_FINGERPRINT || type == BIOTYPE_IRIS) && !isProcessed) {
        m_enrollTimer->start();
        m_enrollFinish = false;
    }
}

QStringList
PromptDialog::getAllFeatureinfolist(int uid, int indexStart, int indexEnd, int driver_type, QList<int> &idxList)
{
    QStringList list;
    QList<QDBusVariant> qlist;
    FeatureInfo *featureInfo;
    int listsize;
    QDBusMessage result = serviceInterface->call(QStringLiteral("GetAllFeatureList"), uid, indexStart, indexEnd);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetDevList error:" << result.errorMessage();
        return list;
    }
    QList<QVariant> variantList = result.arguments();
    listsize = variantList[0].value<int>();
    variantList[1].value<QDBusArgument>() >> qlist;
    for (int i = 0; i < listsize; i++) {
        featureInfo = new FeatureInfo;
        qlist[i].variant().value<QDBusArgument>() >> *featureInfo;
        list.append(featureInfo->index_name);
        if (driver_type == featureInfo->biotype)
            idxList.append(featureInfo->index);
        delete featureInfo;
    }
    return list;
}

PromptDialog::Result PromptDialog::getResult()
{
    return opsResult;
}

void PromptDialog::setSearchResult(bool isAdmin, const QList<SearchResult> &searchResultList)
{
    QStandardItemModel *model = new QStandardItemModel(ui->treeViewResult);
    if (isAdmin)
        model->setHorizontalHeaderLabels(QStringList{ tr("Serial number"), tr("UserName"), tr("FeatureName") });
    else
        model->setHorizontalHeaderLabels(QStringList{ tr("Serial number"), tr("FeatureName") });

    for (int i = 0; i < searchResultList.size(); i++) {
        SearchResult ret = searchResultList[i];
        QList<QStandardItem *> row;
        row.append(new QStandardItem(QString::number(i + 1)));
        if (isAdmin) {
            struct passwd *pwd = getpwuid(ret.uid);
            row.append(new QStandardItem(QString(pwd->pw_name)));
        }
        row.append(new QStandardItem(ret.indexName));
        model->appendRow(row);
    }
    if (m_enrollImagePaths.count() > 0) {
        QPixmap pixmap;
        pixmap.load(m_enrollImagePaths.at(m_enrollImagePaths.count() - 1));
        ui->lblImage->setPixmap(pixmap);
    }

    ui->treeViewResult->setModel(model);
    ui->treeViewResult->show();
    ui->treeViewResult->resizeColumnToContents(0);
    ui->treeViewResult->header()->setSectionResizeMode(QHeaderView::Fixed);
    ui->treeViewResult->setRootIsDecorated(false); // 序列号居左对齐
    ui->treeViewResult->installEventFilter(this);
    ui->treeViewResult->setAttribute(Qt::WA_Hover, true);
    this->setFixedHeight(height() + 100);
}

QString PromptDialog::getGif(int type)
{
    switch (type) {
        case BIOTYPE_FINGERPRINT:
            return ":/images/assets/fingerprint.gif";
        case BIOTYPE_FINGERVEIN:
            return ":/images/assets/fingervein.gif";
        case BIOTYPE_IRIS:
            return ":/images/assets/iris.gif";
        case BIOTYPE_VOICEPRINT:
            return ":/images/assets/voiceprint.gif";
        case BIOTYPE_FACE:
            return ":/images/assets/face.gif";
    }
    return QString();
}

QString PromptDialog::getImage(int type)
{
    switch (type) {
        case BIOTYPE_FINGERPRINT:
            if (isProcessed) {
                return "/usr/share/ukui-biometric/images/huawei/00.svg";
            } else {
                return "/usr/share/ukui-biometric/images/FingerPrint.png";
            }
        case BIOTYPE_FINGERVEIN:
            return "/usr/share/ukui-biometric/images/FingerVein.png";
        case BIOTYPE_IRIS:
            return "/usr/share/ukui-biometric/images/Iris.png";
        case BIOTYPE_VOICEPRINT:
            return "/usr/share/ukui-biometric/images/VoicePrint.png";
        case BIOTYPE_FACE:
            return "/usr/share/ukui-biometric/images/Face.png";
    }
    return QString();
}

void PromptDialog::getEnrollImageList(int type)
{
    QString path;
    QDir dir;
    switch (type) {
        case BIOTYPE_FINGERPRINT:
            path = ":/images/assets/fingerprint";
            dir.setPath(path);
            m_enrollImagePaths = dir.entryList(QStringList("*.svg"), QDir::Files);
            // 构造完整的文件路径
            for (int i = 0; i < m_enrollImagePaths.size(); ++i) {
                m_enrollImagePaths[i] = path + "/" + m_enrollImagePaths[i];
            }
            return;
            //    case BIOTYPE_FINGERVEIN:
            //        return "/usr/share/ukui-biometric/images/FingerVein.png";
        case BIOTYPE_IRIS:
            path = ":/images/assets/iris";
            dir.setPath(path);
            m_enrollImagePaths = dir.entryList(QStringList("*.svg"), QDir::Files);
            // 构造完整的文件路径
            for (int i = 0; i < m_enrollImagePaths.size(); ++i) {
                m_enrollImagePaths[i] = path + "/" + m_enrollImagePaths[i];
            }
            return;
    }
}

int PromptDialog::enroll(int drvId, int uid, int idx, const QString &idxName)
{
    QList<QVariant> args;
    args << drvId << uid << idx << idxName;

    this->setTitle(ENROLL);
    ui->btnClose->setEnabled(false);

    /*
     * 异步回调参考资料：
     * https://github.com/RalfVB/SQEW-OS/blob/master/src/module/windowmanager/compton.cpp
     */
    serviceInterface->callWithCallback(
        "Enroll", args, this, SLOT(enrollCallBack(const QDBusMessage &)), SLOT(errorCallBack(const QDBusError &)));
    ops = ENROLL;
    m_hisOps = ENROLL;
    m_listEnrollArgs = args;

    return exec();
}

void PromptDialog::enrollCallBack(const QDBusMessage &reply)
{
    int result;
    dup_fd = -1;
    result = reply.arguments()[0].value<int>();
    qDebug() << "Enroll result: " << result;

    ui->btnClose->setEnabled(true);

    QString typeText = EnumToString::transferBioType(type);
    switch (result) {
        case DBUS_RESULT_SUCCESS: { /* 录入成功 */
            opsResult = SUCESS;
            // setPrompt(tr("Enroll successfully"));
            setTipPixmap(SUCESS);
            showClosePrompt(typeText + tr("add successfully"));
            Q_EMIT enrollFinished();
            m_hisOps = IDLE;
            break;
        }
        default:
            opsResult = ERROR;
            handleErrorResult(result);
            setTipPixmap(ERROR);
            break;
    }
    ops = IDLE;
}

int PromptDialog::verify(int drvId, int uid, int idx)
{
    QList<QVariant> args;
    args << drvId << uid << idx;

    this->setTitle(VERIFY);

    serviceInterface->callWithCallback(
        "Verify", args, this, SLOT(verifyCallBack(const QDBusMessage &)), SLOT(errorCallBack(const QDBusError &)));
    ops = VERIFY;
    m_hisOps = VERIFY;
    m_listVerifyArgs = args;

    return exec();
}

void PromptDialog::verifyCallBack(const QDBusMessage &reply)
{
    int result;
    dup_fd = -1;
    result = reply.arguments()[0].value<int>();
    qDebug() << "Verify result: " << result;

    if (result >= 0) {
        opsResult = SUCESS;
        setTipPixmap(SUCESS);
        showClosePrompt(tr("Verify successfully"));
        m_hisOps = IDLE;
    } else if (result == DBUS_RESULT_NOTMATCH) {
        showClosePrompt(tr("Not Match"));
        if (!isProcessed) {
            ui->lblImage->setPixmap(getImage(type));
            setTipPixmap();
        }
        setTipPixmap(ERROR);
        m_hisOps = IDLE;
    } else {
        handleErrorResult(result);
        setTipPixmap(ERROR);
    }

    ops = IDLE;
}

int PromptDialog::search(int drvId, int uid, int idxStart, int idxEnd)
{
    QList<QVariant> args;
    args << drvId << uid << idxStart << idxEnd;
    qDebug() << args << drvId << uid << idxStart << idxEnd;
    this->setTitle(SEARCH);

    serviceInterface->callWithCallback(
        "Search", args, this, SLOT(searchCallBack(const QDBusMessage &)), SLOT(errorCallBack(const QDBusError &)));

    ops = SEARCH;
    m_hisOps = SEARCH;
    m_listSearchArgs = args;

    return exec();
}

void PromptDialog::searchCallBack(const QDBusMessage &reply)
{
    int result;
    dup_fd = -1;
    result = reply.arguments()[0].value<int>();
    qDebug() << "Search result: " << result;

    if (result > 0) {
        setPrompt(tr("Search Result"));
        int count = result;
        QDBusArgument argument = reply.arguments().at(1).value<QDBusArgument>();
        QList<QVariant> variantList;
        argument >> variantList;
        QList<SearchResult> results;
        for (int i = 0; i < count; i++) {
            QDBusArgument arg = variantList.at(i).value<QDBusArgument>();
            SearchResult ret;
            arg >> ret;
            results.append(ret);
        }
        this->setSearchResult(isAdmin(uid), results);
        opsResult = SUCESS;
        m_hisOps = IDLE;
    } else if (result >= DBUS_RESULT_NOTMATCH) {
        showClosePrompt(tr("No matching features Found"));
        setTipPixmap(ERROR);
        m_hisOps = IDLE;
    } else {
        handleErrorResult(result);
        setTipPixmap(ERROR);
    }
    movie->stop();
    if (m_enrollTimer && m_enrollTimer->isActive()) {
        m_enrollTimer->stop();
        m_enrollFinish = true;
    }

    ops = IDLE;
}

void PromptDialog::StopOpsCallBack(const QDBusMessage &reply)
{
    dup_fd = -1;
    int ret = reply.arguments().at(0).toInt();
    accept();
}

void PromptDialog::errorCallBack(const QDBusError &error)
{
    dup_fd = -1;
    m_hisOps = IDLE;
    qDebug() << "DBus Error: " << error.message();
    accept();
}

void PromptDialog::closeEvent(QCloseEvent *event)
{
    qDebug() << " ops = " << ops;
    if (ops != IDLE) {
        QList<QVariant> args;
        args << QVariant(deviceId) << QVariant(5);
        serviceInterface->callWithCallback(
            "StopOps",
            args,
            this,
            SLOT(StopOpsCallBack(const QDBusMessage &)),
            SLOT(errorCallBack(const QDBusError &)));
        setPrompt(tr("In progress, please wait..."));
    } else {
        accept();
    }
}

void PromptDialog::onFrameWritten(int drvId)
{
    if (m_isUseEnrollFd && ops == ENROLL) {
        return;
    }
    if (ops == IDLE) {
        qDebug() << "Not in enroll or verify!";
        return;
    }
    if (drvId != deviceId) {
        qDebug() << "Device not pair!";
        return;
    }
    if (dup_fd == -1) {
        dup_fd = get_server_gvariant_stdout(drvId);
    }

    if (dup_fd < 0)
        return;
    if (w_timer && w_timer->isActive()) {
        w_timer->stop();
    }
    lseek(dup_fd, 0, SEEK_SET);
    char base64_bufferData[1024 * 1024];
    int rc = read(dup_fd, base64_bufferData, 1024 * 1024);
    QImage image;
    if (rc > 0) {
        QByteArray imageData(base64_bufferData, rc);
        QBuffer imgBuffer(&imageData);
        imgBuffer.open(QIODevice::ReadOnly);

        QImageReader imgReader(&imgBuffer);
        image = imgReader.read();
        if (image.isNull()) {
            return;
        }
        image = image.convertToFormat(QImage::Format_RGB888);
    }
    ui->lblImage->setAlignment(Qt::AlignCenter);
    QPixmap faceQImage = PixmapToRound(QPixmap::fromImage(image), 77);
    ui->lblImage->setPixmap(faceQImage);
    setTipPixmap();
}

void PromptDialog::onEnrollFrameWritten(int drvId)
{
    if (!m_isUseEnrollFd || ops != ENROLL) {
        return;
    }
    if (ops == IDLE) {
        qDebug() << "Not in enroll or verify!";
        return;
    }
    if (drvId != deviceId) {
        qDebug() << "Device not pair!";
        return;
    }
    if (dup_fd == -1) {
        dup_fd = get_server_gvariant_stdout(drvId);
    }

    if (dup_fd < 0)
        return;
    if (w_timer && w_timer->isActive()) {
        w_timer->stop();
    }
    lseek(dup_fd, 0, SEEK_SET);
    char base64_bufferData[1024 * 1024];
    int rc = read(dup_fd, base64_bufferData, 1024 * 1024);
    QImage image;
    if (rc > 0) {
        QByteArray imageData(base64_bufferData, rc);
        QBuffer imgBuffer(&imageData);
        imgBuffer.open(QIODevice::ReadOnly);

        QImageReader imgReader(&imgBuffer);
        image = imgReader.read();
        if (image.isNull()) {
            return;
        }
        image = image.convertToFormat(QImage::Format_RGB888);
    }
    QPixmap faceQImage = PixmapToRound(QPixmap::fromImage(image), 77);
    ui->lblImage->setPixmap(faceQImage);
    setTipPixmap();
}

void PromptDialog::onProcessChanged(int drvId, QString aa, int statusType, QString bb)
{
    int count = statusType * 15 / 100;
    QString filename = QString("/usr/share/ukui-biometric/images/huawei/") + (count < 10 ? "0" : "")
                       + QString::number(count) + ".svg";

    ui->lblImage->setPixmap(QPixmap(filename));
    setTipPixmap();
}

void PromptDialog::onStatusChanged(int drvId, int statusType)
{
    if (!(drvId == deviceId && statusType == STATUS_NOTIFY))
        return;

    ui->btnClose->setEnabled(true);

    // 过滤掉当录入时使用生物识别授权接收到的认证的提示信息
    if (ops == ENROLL) {
        QDBusMessage reply = serviceInterface->call("UpdateStatus", drvId);
        if (reply.type() == QDBusMessage::ErrorMessage) {
            qDebug() << "DBUS: " << reply.errorMessage();
            return;
        }
        int devStatus = reply.arguments().at(3).toInt();
        qDebug() << devStatus;

        if (!(devStatus >= 201 && devStatus < 203)) {
            return;
        }

        // 认证结束后，重新获取fd，刷新录入界面
        if (devStatus == 201 && statusType == STATUS_NOTIFY) {
            dup_fd = -1;
        }
    } else if (ops == IDLE) {
        return;
    }

    ui->Fsbtn->hide();
    ui->cancelBtn->show();

    if (!isProcessed && !isFace && type != BIOTYPE_FINGERPRINT && type != BIOTYPE_IRIS) {
        if (w_timer && w_timer->isActive()) {
            w_timer->stop();
        }
        if (movie) {
            if (movie->state() != QMovie::Running) {
                ui->lblImage->setMovie(movie);
                movie->start();
            } else {
                movie->stop();
                ui->lblImage->setMovie(movie);
                movie->start();
            }
        }
        setTipPixmap();
    } else if (!isProcessed && (type == BIOTYPE_FINGERPRINT || type == BIOTYPE_IRIS)) {
        if (w_timer && w_timer->isActive()) {
            w_timer->stop();
        }
        if (m_enrollTimer) {
            m_enrollTimer->start(50);
        }
    }

    QDBusMessage notifyReply = serviceInterface->call("GetNotifyMesg", drvId);
    if (notifyReply.type() == QDBusMessage::ErrorMessage) {
        qDebug() << "DBUS: " << notifyReply.errorMessage();
        return;
    }
    QString prompt = notifyReply.arguments().at(0).toString();
    qDebug() << prompt;

    if (type == 3)
        setPrompt(tr("Keep looking straight at the camera."));
    else
        setPrompt(prompt);
}

void PromptDialog::handleErrorResult(int error)
{
    dup_fd = -1;
    switch (error) {
        case DBUS_RESULT_ERROR: {
            // 操作失败，需要进一步获取失败原因
            QDBusMessage msg = serviceInterface->call("GetOpsMesg", deviceId);
            if (msg.type() == QDBusMessage::ErrorMessage) {
                qDebug() << "UpdateStatus error: " << msg.errorMessage();
                showClosePrompt(tr("D-Bus calling error"));
                return;
            }
            qDebug() << " msg.arguments().at(0).toString() = " << msg.arguments().at(0).toString();
            showClosePrompt(msg.arguments().at(0).toString());
            qDebug() << "GetOpsMesg: deviceId--" << deviceId;
            StatusReslut ret = UpdateStatus(deviceId);
            qDebug() << "StatusReslut:" << ret.result << "," << ret.enable << "," << ret.devNum << "," << ret.devStatus
                     << "," << ret.opsStatus << "," << ret.notifyMessageId;
            // 识别操作超时 304/404 认证超时；8 网络错误；
            if (ret.result == 0) {
                if (ret.opsStatus != OPS_ENROLL_STOP_BY_USER && ret.opsStatus != OPS_IDENTIFY_STOP_BY_USER
                    && ret.opsStatus != OPS_VERIFY_STOP_BY_USER && ret.opsStatus != OPS_SEARCH_STOP_BY_USER
                    && ret.opsStatus != OPS_GET_FLIST_SUCCESS && ret.opsStatus != OPS_OPEN_SUCCESS
                    && ret.opsStatus != OPS_COMM_SUCCESS
                    && ret.opsStatus != OPS_COMM_STOP_BY_USER // 添加一个"打开设备完成"或者"操作成功"的操作结果，在hwe上录入特征时锁屏，解锁后后获取的信息返回是“打开设备完成”或者"操作成功"
                    && ret.opsStatus != OPS_IDENTIFY_MATCH && ret.opsStatus != OPS_TYPE_IDENTIFY && ret.opsStatus != OPS_IDENTIFY_NO_MATCH ) { // v11人脸锁屏解锁后返回的消息是“识别特征匹配”
                    m_hisOps = IDLE; // 主动停止
                }
            } else {
                m_hisOps = IDLE;
            }
            break;
        }
        case DBUS_RESULT_DEVICEBUSY:
            // 设备忙
            showClosePrompt(tr("Device is busy"));
            m_hisOps = IDLE;
            break;
        case DBUS_RESULT_NOSUCHDEVICE:
            // 设备不存在
            showClosePrompt(tr("No such device"));
            m_hisOps = IDLE;
            break;
        case DBUS_RESULT_PERMISSIONDENIED:
            // 没有权限
            on_btnClose_clicked();
            m_hisOps = IDLE;
            break;
    }

    ui->lblPrompt->setStyleSheet("QLabel{color: red;}");
    ui->lblImage->setPixmap(getImage(type));
    setTipPixmap();
    qDebug() << QString("Error:(%1)%2").arg(QString::number(error)).arg(ui->lblPrompt->text());
}

void PromptDialog::setFailed()
{
    switch (ops) {
        case ENROLL:
            showClosePrompt(tr("Failed to enroll"));
            break;
        case VERIFY:
            showClosePrompt(tr("Failed to match"));
            break;
        case SEARCH:
            showClosePrompt(tr("Not Found"));
            break;
        default:
            break;
    }
}

void PromptDialog::showClosePrompt(QString tips)
{
    switch (opsResult) {
        case 0:
            ui->Ctbtn->show();
            break;
        default:
            ui->Ctbtn->hide();
            break;
    }
    if (w_timer && w_timer->isActive()) {
        w_timer->stop();
    }
    ui->msgLabel->setText(tips);
    ui->msgLabel->setWordWrap(true);
    ui->msgLabel->adjustSize();
    ui->msgLabel->show();
    ui->Fsbtn->setText(tr("Finish"));
    ui->Fsbtn->show();
    ui->cancelBtn->hide();
    ui->titleLabel->hide();
    ui->lblPrompt->hide();
}

void PromptDialog::onPrepareForSleep(bool sleep)
{
    /// 系统休眠时，会关闭总线，导致设备不可用，发生错误
    /// 在系统休眠之前停止认证，在系统唤醒后重新开始认证
    bool isChanged = false;
    if (sleep) {
        if (!(m_uSysState & SYSSTATE_SLEEP)) {
            m_uSysState |= SYSSTATE_SLEEP;
            isChanged = true;
        }
    } else {
        if (m_uSysState & SYSSTATE_SLEEP) {
            m_uSysState &= ~SYSSTATE_SLEEP;
            isChanged = true;
        }
    }
    if (isChanged) {
        chkSysState();
    }
}

void PromptDialog::onSysLock()
{
    if (!(m_uSysState & SYSSTATE_LOCK)) {
        m_uSysState |= SYSSTATE_LOCK;
        chkSysState();
    }
}

void PromptDialog::onSysUnLock()
{
    if (m_uSysState & SYSSTATE_LOCK) {
        m_uSysState &= ~SYSSTATE_LOCK;
        chkSysState();
    }
}

void PromptDialog::chkSysState()
{
    qInfo() << "chkSysState:" << m_uSysState << "," << m_hisOps;
    if (m_uSysState) { // 停止
        serviceInterface->call(QStringLiteral("StopOps"), deviceId, 5);
    } else { // 恢复
        if (m_hisOps == IDLE)
            return;
        resetUI();
        ui->treeViewResult->hide();
        ui->lblImage->setPixmap(getImage(type));
        setTipPixmap();
        ui->lblPrompt->setStyleSheet("");
        setPrompt("");
        if (movie && movie->state() == QMovie::Running) {
            movie->stop();
        }
        if (m_enrollTimer && m_enrollTimer->isActive()) {
            m_enrollTimer->stop();
        }
        switch (m_hisOps) {
            case ENROLL: {
                ui->btnClose->setEnabled(false);
                serviceInterface->callWithCallback(
                    "Enroll",
                    m_listEnrollArgs,
                    this,
                    SLOT(enrollCallBack(const QDBusMessage &)),
                    SLOT(errorCallBack(const QDBusError &)));
                ops = m_hisOps;
            } break;
            case VERIFY: {
                serviceInterface->callWithCallback(
                    "Verify",
                    m_listVerifyArgs,
                    this,
                    SLOT(verifyCallBack(const QDBusMessage &)),
                    SLOT(errorCallBack(const QDBusError &)));
                ops = m_hisOps;
            } break;
            case SEARCH: {
                serviceInterface->callWithCallback(
                    "Search",
                    m_listSearchArgs,
                    this,
                    SLOT(searchCallBack(const QDBusMessage &)),
                    SLOT(errorCallBack(const QDBusError &)));
                ops = m_hisOps;
            } break;
            default:
                break;
        }
    }
}

QPixmap PromptDialog::PixmapToRound(const QPixmap &src, int radius)
{
    if (src.isNull()) {
        return QPixmap();
    }

    QPixmap pixmapa(src);
    QPixmap pixmap(radius * 2, radius * 2);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, radius * 2, radius * 2);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, radius * 2, radius * 2, pixmapa);
    return pixmap;
}

void PromptDialog::updatePixmap()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_waitingPixmap = m_waitingPixmap.transformed(matrix, Qt::FastTransformation);
    ui->lblImage->setPixmap(m_waitingPixmap);
    setTipPixmap();
}

void PromptDialog::setTipPixmap(int type)
{
    switch (type) {
        case SUCESS:
            if (m_enrollTimer) {
                m_enrollTimer->stop();
                m_enrollFinish = true;
            }
            ui->lblImage->setPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(QSize(88, 88)));
            ui->lblImage->setFixedSize(88, 88);
            break;
        case ERROR:
            if (m_enrollTimer) {
                m_enrollTimer->stop();
                m_enrollFinish = true;
            }
            ui->lblImage->setPixmap(QIcon::fromTheme("dialog-error").pixmap(QSize(88, 88)));
            ui->lblImage->setFixedSize(88, 88);
            break;
        default:
            ui->lblImage->setFixedSize(156, 156);
            break;
    }
}

StatusReslut PromptDialog::UpdateStatus(int drvid)
{
    StatusReslut status;
    QDBusMessage result = serviceInterface->call(QStringLiteral("UpdateStatus"), drvid);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "UpdateStatus error:" << result.errorMessage();
        status.result = -1;
        return status;
    }

    status.result = result.arguments().at(0).toInt();
    status.enable = result.arguments().at(1).toInt();
    status.devNum = result.arguments().at(2).toInt();
    status.devStatus = result.arguments().at(3).toInt();
    status.opsStatus = result.arguments().at(4).toInt();
    status.notifyMessageId = result.arguments().at(5).toInt();

    return status;
}

void PromptDialog::updateEnrollImage()
{
    if (m_currentImage >= 0 && m_currentImage < m_enrollImagePaths.size()) {
        QPixmap pixmap;
        pixmap.load(m_enrollImagePaths.at(m_currentImage));
        ui->lblImage->setPixmap(pixmap);
        ui->lblImage->setFixedSize(156, 156);
    }
}

void PromptDialog::switchEnrollImage()
{
    if (m_enrollSort) {
        m_currentImage += 1;
        if (m_currentImage == m_enrollImagePaths.size()) {
            m_enrollSort = false;
            //  等待一段时间后重新启动定时器
            m_enrollTimer->stop();
            QTimer::singleShot(100, this, [&, this]() {
                if (!m_enrollFinish) {
                    m_enrollTimer->start();
                }
            });
        }
    } else {
        m_currentImage -= 1;
        if (m_currentImage < 0) {
            m_enrollSort = true;
            //  等待一段时间后重新启动定时器
            m_enrollTimer->stop();
            QTimer::singleShot(100, this, [&, this]() {
                if (!m_enrollFinish) {
                    m_enrollTimer->start();
                }
            });
        }
    }
    updateEnrollImage();
}
