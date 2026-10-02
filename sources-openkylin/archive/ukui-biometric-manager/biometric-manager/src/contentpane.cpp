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
#include "contentpane.h"
#include "ui_contentpane.h"
#include <QInputDialog>
#include "promptdialog.h"
#include "inputdialog.h"
#include "messagedialog.h"
#include "configuration.h"
#include "imageutil.h"
#include <QtConcurrent/QtConcurrentRun>
#include <QPoint>
#include <QHoverEvent>
#include <QEvent>
#include <unistd.h>

#define ICON_SIZE 32
#define ICON_TYPE_SCHENA "org.ukui.style"

ContentPane::ContentPane(int uid, DeviceInfo *deviceInfo, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ContentPane)
    , deviceInfo(deviceInfo)
    , currentUid(uid)
    , qtSettings(nullptr)
    , dataModel(nullptr)
{
    ui->setupUi(this);
    prettify();
    /* 向 QDBus 类型系统注册自定义数据类型 */
    registerCustomTypes();
    /* 连接 DBus Daemon */
    serviceInterface = new QDBusInterface(DBUS_SERVICE, DBUS_PATH, DBUS_INTERFACE, QDBusConnection::systemBus());
    serviceInterface->setTimeout(2147483647); /* 微秒 */
    updateWidgetStatus();
    /* 设置数据模型 */
    setModel();
    showDeviceInfo();
    showFeatures();
    // 连接异步删除结果信号
    connect(this, &ContentPane::deleteFeatureResult, this, &ContentPane::onDeleteFeatureResult);
}

ContentPane::~ContentPane()
{
    delete ui;
}

void ContentPane::prettify()
{
    ui->btnDelete->setIconSize(QSize(18, 18));
    ui->btnDelete->setIcon(QIcon::fromTheme("list-remove-symbolic"));
    ui->btnEnroll->setIconSize(QSize(18, 18));
    ui->btnEnroll->setIcon(QIcon::fromTheme("list-add-symbolic"));
    ui->btnSearch->setIconSize(QSize(18, 18));
    ui->btnSearch->setIcon(QIcon::fromTheme("edit-find-symbolic"));
}
/**
 * @brief 设置数据模型
 */
void ContentPane::setModel()
{
    /* 设置 TreeView 的 Model */
    dataModel = new TreeModel(currentUid, BioType(deviceInfo->biotype), this);
    connect(dataModel, &TreeModel::dataChanged, this, [&](const QModelIndex &index, const QVariant &value) {
        renameFeatureName(index, value);
    });
    connect(dataModel, &TreeModel::setFMessage, this, [=](QString text) { messageBox(Information, text, tr("OK")); });

    ui->treeView->setModel(dataModel);
    // ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeView->setFocusPolicy(Qt::NoFocus);
    ui->treeView->setAlternatingRowColors(true);
    ui->treeView->setRootIsDecorated(false); // 序列号居左对齐
    ui->treeView->header()->setSectionResizeMode(QHeaderView::Fixed);
    connect(ui->treeView, &MyTreeView::rightClicked, this, [&](QModelIndex index) { rightClicked(index); });
}

void ContentPane::setDeviceInfo(DeviceInfo *deviceInfo)
{
    this->deviceInfo = deviceInfo;
    updateWidgetStatus();
}

void ContentPane::setDeviceAvailable(int deviceAvailable)
{
    showFeatures();
    deviceInfo->device_available = deviceAvailable;
    updateWidgetStatus();
}

int ContentPane::featuresCount()
{
    if (dataModel)
        return dataModel->rowCount();
    return 0;
}

void ContentPane::updateWidgetStatus()
{
    ui->btnEnroll->setEnabled(deviceInfo->device_available > 0);
    ui->btnDelete->setEnabled(deviceInfo->device_available > 0);
    //    ui->btnVerify->setEnabled(deviceInfo->device_available > 0);
    ui->btnSearch->setEnabled(deviceInfo->device_available > 0);
    //    ui->btnClean->setEnabled(deviceInfo->device_available > 0);
    ui->treeView->setEnabled(deviceInfo->device_available > 0);

    if (deviceInfo->device_shortname == "gdxfp") {
        //        ui->btnVerify->setEnabled(false);
        ui->btnSearch->setEnabled(false);
    }
}

void ContentPane::updateButtonUsefulness()
{
    bool enable = !!dataModel->rowCount();
    ui->btnDelete->setEnabled(enable);
    //	ui->btnVerify->setEnabled(enable);
    ui->btnSearch->setEnabled(enable);
    //    ui->btnClean->setEnabled(enable);

    if (deviceInfo->device_shortname == "gdxfp") {
        // ui->btnVerify->setEnabled(false);
        ui->btnSearch->setEnabled(false);
    }
}

/**
 * @brief 检测某种生物识别设备是否存在
 * @param biotype
 * @return
 */
bool ContentPane::deviceIsAvailable()
{
    return deviceInfo->device_available > 0;
}

void ContentPane::showDeviceInfo() {}

/**
 * @brief 显示生物识别数据列表
 * @param biotype
 */
void ContentPane::showFeatures()
{
    if (isChangeName) {
        isChangeName = false;
        return;
    }
    dataModel->removeAll();
    if (!deviceIsAvailable())
        return;
    setCursor(Qt::WaitCursor);
    QList<QVariant> args;

    args << QVariant((isAdmin(currentUid) ? -1 : currentUid)) << QVariant(0) << QVariant(-1);
    serviceInterface->callWithCallback(
        "GetAllFeatureList", args, this, SLOT(showFeaturesCallback(QDBusMessage)), SLOT(errorCallback(QDBusError)));
}

#include <QDBusVariant>

bool compareBarData(const QDBusVariant &feature1, const QDBusVariant &feature2)
{
    std::shared_ptr<FeatureInfo> featureInfo1 = std::make_shared<FeatureInfo>();
    std::shared_ptr<FeatureInfo> featureInfo2 = std::make_shared<FeatureInfo>();

    feature1.variant().value<QDBusArgument>() >> *featureInfo1;
    feature2.variant().value<QDBusArgument>() >> *featureInfo2;
    if (featureInfo1->index_name < featureInfo2->index_name) {
        return true;
    }
    return false;
}

/**
 * @brief 特征列表返回后回调函数进行显示
 * @param callbackReply
 */
void ContentPane::showFeaturesCallback(QDBusMessage callbackReply)
{
    dataModel->removeAll();

    QList<QDBusVariant> qlist;
    std::shared_ptr<FeatureInfo> featureInfo;
    int listsize;

    QList<QVariant> variantList = callbackReply.arguments();
    listsize = variantList[0].value<int>();
    variantList[1].value<QDBusArgument>() >> qlist;
    qSort(qlist.begin(), qlist.end(), compareBarData);
    for (int i = 0; i < listsize; i++) {
        featureInfo = std::make_shared<FeatureInfo>();
        qlist[i].variant().value<QDBusArgument>() >> *featureInfo;
        if (deviceInfo && deviceInfo->device_shortname == featureInfo->device_shortname
            && deviceInfo->biotype == featureInfo->biotype) {
            dataModel->appendData(featureInfo.get());
        }
    }
    dataModel->setMySort();
    dataModel->updateSerialNum();
    setCursor(Qt::ArrowCursor);

    updateButtonUsefulness();
}

QString ContentPane::inputFeatureName(bool isNew)
{
    QStringList names = getAllFeaturelist(getuid());
    InputDialog *inputDialog = new InputDialog(names, this);
    if (isNew) {
        inputDialog->setTitle(tr("New Feature"));
        inputDialog->setPrompt(EnumToString::lowerTransferBioType(deviceInfo->biotype) + tr("name:"));
    } else {
        inputDialog->setTitle(tr("Rename Feature"));
        inputDialog->setPrompt(
            tr("Please input a new") + EnumToString::lowerTransferBioType(deviceInfo->biotype) + tr("name:"));
    }
    connect(inputDialog, &InputDialog::dataChanged, this, [&](const QString &text) { inputDialog->accept(); });
    QString featureName = QString();
    if (inputDialog->exec() != QDialog::Rejected)
        featureName = inputDialog->getText();

    delete inputDialog;
    if (featureName.contains("\n")) { // 按照设计师要求，当复制过来的文本中含有换行符时，过滤掉换行符
        featureName = featureName.remove("\n");
    }
    return featureName;
}

QStringList ContentPane::getAllFeaturelist(int uid, int indexStart, int indexEnd)
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
        delete featureInfo;
    }
    return list;
}

QStringList
ContentPane::getAllFeatureinfolist(int uid, int indexStart, int indexEnd, int driver_type, QList<int> &idxList)
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

/**
 * @brief 录入
 */
void ContentPane::on_btnEnroll_clicked()
{
    if (!deviceInfo) {
        return;
    }
    if (deviceInfo->biotype == BIOTYPE_FACE) {
        m_pRiskTip = new QMessageBox(this);
        m_pRiskTip->setWindowTitle(tr(""));
        m_pRiskTip->setText(tr("'Face recognition' does not support live body detection, and the verification method "
                               "is risky. Do you want to continue?"));
        m_pRiskTip->setIcon(QMessageBox::Warning);
        QPushButton *btnContinue = m_pRiskTip->addButton(tr("Continue"), QMessageBox::AcceptRole);
        btnContinue->setProperty("isImportant", true);
        btnContinue->setObjectName("msgContinueBtn");
        QPushButton *btnCancel = m_pRiskTip->addButton(tr("Cancel"), QMessageBox::RejectRole);
        btnCancel->setProperty("useButtonPalette", true);
        btnCancel->setObjectName("msgCancelBtn");
        m_pRiskTip->exec();

        if (m_pRiskTip->clickedButton() != btnContinue) {
            delete m_pRiskTip;
            m_pRiskTip = nullptr;
            return;
        }
        delete m_pRiskTip;
        m_pRiskTip = nullptr;
    }
    int num = 1;
    QDBusReply<int> reply = serviceInterface->call("StopOps", QVariant(deviceInfo->device_id), QVariant(3000));
    QList<int> idxList;
    QStringList list = getAllFeatureinfolist(getuid(), 0, -1, deviceInfo->biotype, idxList);
    QString featurename;
    while (1) {
        featurename = EnumToString::lowerTransferBioType(deviceInfo->biotype) + QString::number(num);
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
    indexName = featurename;
    if (indexName.isEmpty())
        return;

    qDebug() << "Enroll: devicceid--" << deviceInfo->device_id << "uid--" << currentUid << " index--" << freeIndex
             << " indexName--" << indexName;
    promptDialog = new PromptDialog(serviceInterface, deviceInfo->biotype, deviceInfo->device_id, currentUid, this);
    connect(promptDialog, &PromptDialog::enrollFinished, this, [=]() { showFeatures(); });
    if (deviceInfo->device_shortname == "gdxfp")
        promptDialog->setProcessed(true);

    if (deviceInfo->biotype == BIOTYPE_FACE)
        promptDialog->setIsFace(true);

    isShowDialog = true;
    promptDialog->enroll(deviceInfo->device_id, currentUid, freeIndex, indexName);
    qDebug() << "Enroll result: ----- " << promptDialog->getResult();
    // if(promptDialog->getResult() == PromptDialog::SUCESS) {
    showFeatures();
    //}
    delete promptDialog;
    promptDialog = nullptr;

    updateButtonUsefulness();
    ui->btnEnroll->clearFocus();

    isShowDialog = false;
}

bool ContentPane::getIsShowDialog()
{
    return isShowDialog;
}

FeatureInfo *ContentPane::createNewFeatureInfo()
{
    FeatureInfo *featureInfo = new FeatureInfo;
    featureInfo->uid = currentUid;
    featureInfo->biotype = deviceInfo->biotype;
    featureInfo->device_shortname = deviceInfo->device_shortname;
    featureInfo->index = freeIndex;
    featureInfo->index_name = indexName;
    return featureInfo;
}

/**
 * @brief DBus 异步操作 errorMethod
 * @param error
 */
void ContentPane::errorCallback(QDBusError error)
{
    setCursor(Qt::ArrowCursor);
    qDebug() << "DBUS:" << error.message();
}

bool ContentPane::confirmDelete(bool all)
{
    QString text, title;
    if (all) {
        text = tr("Confirm whether clean all the features?");
        title = tr("Confirm Clean");
    } else {
        text = tr("Confirm whether delete the features selected?");
        title = tr("Confirm Delete");
    }
    messageBox(Question, tr("Confirm whether delete the features selected?"), tr("OK"));
}

void ContentPane::messageBox(int type, QString msg, QString text)
{
    QMessageBox MBox;
    MBox.setText(msg);
    QPushButton *agreeBtn = MBox.addButton(text, QMessageBox::AcceptRole);
    agreeBtn->setProperty("isImportant", true);
    switch (type) {
        case Question:
            MBox.setIcon(QMessageBox::Question);
            break;
        case Information:
            MBox.setIcon(QMessageBox::Information);
            break;
        case Warning:
            MBox.setIcon(QMessageBox::Warning);
            break;
        case Critical:
            MBox.setIcon(QMessageBox::Critical);
            break;
        default:
            MBox.setIcon(QMessageBox::NoIcon);
            break;
    }
    MBox.exec();
}

/**
 * @brief 删除生物特征
 */
void ContentPane::on_btnDelete_clicked()
{
    QModelIndexList selectedIndexList = ui->treeView->selectionModel()->selectedRows(0);
    if (selectedIndexList.size() <= 0) {
        messageBox(Information, tr("Please select the feature you want to delete."), tr("OK"));
        return;
    }

    auto bound
        = std::stable_partition(selectedIndexList.begin(), selectedIndexList.end(), [&](const QModelIndex &index) {
              return index.parent() != QModelIndex();
          });
    auto cmp = [&](const QModelIndex &ia, const QModelIndex &ib) { return ia.row() > ib.row(); };
    std::sort(selectedIndexList.begin(), bound, cmp);
    std::sort(bound, selectedIndexList.end(), cmp);

    Q_EMIT deleteFeatureState(0);

    // 异步删除特征
    auto *watcher = new QFutureWatcher<bool>();
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher]() {
        if (watcher->result()) {
            qDebug() << "clean Succeed!!";
        } else {
            qDebug() << "clean failed!!";
        }
        watcher->deleteLater();
    });
    QFuture<bool> fu = QtConcurrent::run([=]() {
        auto ret = this->cleanFeature(selectedIndexList);
        return ret;
    });
    // 运行异步任务
    watcher->setFuture(fu);
}

bool ContentPane::cleanFeature(QModelIndexList selectedIndexList)
{
    unsigned deleteSuccessCount = 0;
    for (auto index : selectedIndexList) {
        int idxStart, idxEnd;
        int uid = index.data(TreeModel::UidRole).toInt();
        bool recursive = !ui->treeView->isExpanded(index) && (index.child(0, 0) != QModelIndex());
        if (recursive) {
            // 如果没有展开则删除该用户的所有特征
            idxStart = 0;
            idxEnd = -1;
        } else {
            idxStart = idxEnd = index.data(Qt::UserRole).toInt();
        }
        qDebug() << "Delete: uid--" << uid << " index--" << idxStart << idxEnd;
        isShowDialog = true;
        QDBusPendingReply<int> reply = serviceInterface->call("Clean", deviceInfo->device_id, uid, idxStart, idxEnd);
        reply.waitForFinished();
        if (reply.isError()) {
            qWarning() << "DBUS:" << reply.error();
            isShowDialog = false;
            Q_EMIT deleteFeatureResult(-1, "");
            return false;
        }
        int result = reply.argumentAt(0).value<int>();
        if (result == DBUS_RESULT_PERMISSIONDENIED) {
            // 第一次未授权直接返回删除失败
            isShowDialog = false;
            Q_EMIT deleteFeatureResult(-2, "");
            return false;
        } else if (result != DBUS_RESULT_SUCCESS) {
            Q_EMIT deleteFeatureResult(-3, tr("Fail to delete! ") + getErrorMessage(DELETE, result));
        } else {
            // 删除成功不做提示
            deleteSuccessCount += 1;
        }
    }
    if (deleteSuccessCount > 0) {
        Q_EMIT deleteFeatureResult(0, "");
    } else {
        Q_EMIT deleteFeatureResult(1, "");
    }

    isShowDialog = false;
    return true;
}

void ContentPane::onDeleteFeatureResult(int nRet, QString strTip)
{
    if (nRet == 0) {
        showFeatures();
        updateButtonUsefulness();
    }
    if (!strTip.isEmpty()) {
        messageBox(Critical, strTip, tr("OK"));
    } else {
        Q_EMIT deleteFeatureState(1);
        ui->btnDelete->clearFocus();
    }
}

/**
 * @brief 生物特征搜索
 */
void ContentPane::on_btnSearch_clicked()
{
    promptDialog = new PromptDialog(serviceInterface, deviceInfo->biotype, deviceInfo->device_id, currentUid, this);
    if (deviceInfo->biotype == BIOTYPE_FACE)
        promptDialog->setIsFace(true);

    promptDialog->search(deviceInfo->device_id, currentUid, 0, -1);
    ui->btnSearch->clearFocus();
    delete promptDialog;
    promptDialog = nullptr;
}

/**
 * @brief 双击重命名
 */
void ContentPane::on_treeView_doubleClicked(const QModelIndex &index)
{
    int column = index.column();

    if (isAdmin(currentUid)) {
        if (column != 2) // 管理员模式双击第三列（特征名称列）重命名
            return;
    } else {
        if (column != 1) // 非管理员模式双击第二列
            return;
    }

    int idx = index.data(TreeModel::IndexRole).toInt();
    int uid = index.data(TreeModel::UidRole).toInt();
    QString idxName = index.data().toString();

    QString newName = inputFeatureName(false);
    if (newName.isEmpty())
        return;

    qDebug() << "Rename " << idx << idxName << " to " << newName;

    QDBusReply<int> result = serviceInterface->call("Rename", deviceInfo->device_id, uid, idx, newName);

    QString resultMessage;
    int type;
    if (result == DBUS_RESULT_SUCCESS) {
        dataModel->setData(index, newName);
        dataModel->setMySort();
        dataModel->updateSerialNum();
        resultMessage = tr("Rename Successfully");
        // messageBox(NoIcon,resultMessage,tr("OK"));
    } else {
        resultMessage = getErrorMessage(RENAME, result);
        // messageBox(Warning,resultMessage,tr("OK"));
    }
}

void ContentPane::rightClicked(QModelIndex index)
{
    int idx = index.data(TreeModel::IndexRole).toInt();
    int uid = index.data(TreeModel::UidRole).toInt();
    QString idxName = index.data().toString();

    QString newName = inputFeatureName(false);
    if (newName.isEmpty())
        return;

    qDebug() << "Rename " << idx << idxName << " to " << newName;

    QDBusReply<int> result = serviceInterface->call("Rename", deviceInfo->device_id, uid, idx, newName);

    QString resultMessage;
    int type;
    if (result == DBUS_RESULT_SUCCESS) {
        dataModel->setData(index, newName);
        dataModel->setMySort();
        dataModel->updateSerialNum();
    } else {
        resultMessage = getErrorMessage(RENAME, result);
    }
}

void ContentPane::renameFeatureName(QModelIndex index, QVariant value)
{
    QString text = value.toString();
    int idx = index.data(TreeModel::IndexRole).toInt();
    int uid = index.data(TreeModel::UidRole).toInt();
    QString idxName = index.data().toString();
    // qDebug() << "Rename " << idx <<idxName << " to " << text;
    QDBusReply<int> result = serviceInterface->call("Rename", deviceInfo->device_id, uid, idx, text);
    isChangeName = true;
    QString resultMessage;
    int type;
    if (result == DBUS_RESULT_SUCCESS) {
        // dataModel->setData(index, newName);
        resultMessage = tr("Rename Successfully");
        type = MessageDialog::Normal;
    } else {

        messageBox(Warning, resultMessage, tr("OK"));
    }
}

void ContentPane::setMessage(QString message)
{
    MessageDialog msgDialog(0, "", "", this);
    msgDialog.setWindowTitle(tr("Rename Result"));
    msgDialog.setMessage(message);
    msgDialog.exec();
}

QString ContentPane::getErrorMessage(int type, int result)
{
    QString errorMessage;

    switch (result) {
        case DBUS_RESULT_ERROR: {
            // 操作失败，需要进一步获取失败原因
            QDBusMessage msg = serviceInterface->call("GetNotifyMesg", deviceInfo->device_id);
            if (msg.type() == QDBusMessage::ErrorMessage) {
                qWarning() << QString("GetNotifyMesg(%1)").arg(deviceInfo->device_id) << msg.errorMessage();
                return tr("DBus calling error");
            }

            errorMessage = msg.arguments().at(0).toString();
            break;
        }
        case DBUS_RESULT_DEVICEBUSY:
            // 设备忙
            errorMessage = (tr("Device is busy"));
            break;
        case DBUS_RESULT_NOSUCHDEVICE:
            // 设备不存在
            errorMessage = (tr("No such device"));
            break;
        case DBUS_RESULT_PERMISSIONDENIED:
            // 没有权限
            errorMessage = (tr("Permission denied"));
            break;
    }
    return errorMessage;
}

void ContentPane::clearSubWidgets()
{
    if (m_inputDialog) {
        m_inputDialog->close();
    }
    if (promptDialog) {
        promptDialog->hide();
        promptDialog->close();
    }
    //  QMessageBox在exec()时，如果父窗口被关闭，会导致messageBox多次释放，因此在窗口关闭之前把messageBox关闭
    for (auto child : children()) {
        QMessageBox *msgBox = qobject_cast<QMessageBox *>(child);
        if (msgBox) {
            msgBox->reject();
        }
    }
}

void ContentPane::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    clearSubWidgets();
}
