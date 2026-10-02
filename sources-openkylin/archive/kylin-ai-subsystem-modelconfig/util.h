#ifndef UTIL_H
#define UTIL_H

#include <kysdk/applications/klabel.h>
#include <kysdk/kysdk-system/libkysysinfo.h>

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QTimer>
#include <QWidget>

namespace ComputePowerShare {
enum ErrorCode {
    NoError = 0,
    InputParameterError,
    TimeOutError,
    NotSupportComputingPowerSharing,
    ServiceInternalHandingError,
    VerificationCodeError,
    OtherError,
};
}

enum class SystemVersion { Version_2403, Version_2503, Version_V11 };

static SystemVersion curSystemVersion() {
    QString version = kdk_system_get_version_detaile().os_version;
    qInfo() << "System version: " << version;

    if (version == "2403") {
        return SystemVersion::Version_2403;
    } else if (version == "2503") {
        return SystemVersion::Version_2503;
    } else {
        return SystemVersion::Version_V11;
    }
}

static void setKLabelFontColor(kdk::KLabel *klabel, QColor color) {
#ifdef SYSTEM_VERSION_2403
    QPalette palette;
    palette.setColor(QPalette::WindowText, color);
    klabel->setPalette(palette);
#else
    klabel->setFontColor(color);
#endif
}
static void delayedMoveToCenter(QWidget* widget) {
    QTimer::singleShot(0, widget, [widget]() {
        QWidget* parentWidget = widget->parentWidget();
        if (parentWidget) {
            QRect parentGeometry = parentWidget->geometry();
            int x = parentGeometry.x() + (parentGeometry.width() - widget->width()) / 2;
            int y = parentGeometry.y() + (parentGeometry.height() - widget->height()) / 2 - 20;  // 减去标题框的高度
            widget->move(x, y);
        }
    });
}

static void showErrorMessage(const QString& message, QWidget* parent) {
    QMessageBox msgBox(parent);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(message);

    QPushButton *cancelButton = msgBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *sureButton = msgBox.addButton(QObject::tr("Ok"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(sureButton);
    delayedMoveToCenter(&msgBox);
    msgBox.exec();
}

static bool showModelInUseMessage(QWidget* parent) {
    QMessageBox retryMsgBox(parent);
    retryMsgBox.setText(QObject::tr("Sorry, the model is currently executing another task. Please try again after it is completed.."));
    retryMsgBox.setIcon(QMessageBox::Critical);

    QPushButton *cancelButton = retryMsgBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *retryButton = retryMsgBox.addButton(QObject::tr("Retry"), QMessageBox::AcceptRole);
    retryMsgBox.setDefaultButton(retryButton);
    delayedMoveToCenter(&retryMsgBox);
    retryMsgBox.exec();

    if ((QPushButton *)retryMsgBox.clickedButton() == retryButton) {
        return true;
    }
    return false;
}

static bool showConfirmMessage(QWidget* parent) {
    QMessageBox confirmMsgBox(parent);
    confirmMsgBox.setText(QObject::tr("Deleting it will be irreversible. Are you sure you want to delete this model?"));
    confirmMsgBox.setIcon(QMessageBox::Question);

    QPushButton *cancelButton = confirmMsgBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *sureButton = confirmMsgBox.addButton(QObject::tr("Delete"), QMessageBox::AcceptRole);
    confirmMsgBox.setDefaultButton(sureButton);
    delayedMoveToCenter(&confirmMsgBox);
    confirmMsgBox.exec();

    if ((QPushButton *)confirmMsgBox.clickedButton() == sureButton) {
        return true;
    }
    return false;
}
#endif  // UTIL_H
