// Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
// SPDX-License-Identifier: BSD-3-Clause

#ifndef QTSINGLEGUIAPPLICATION_H
#define QTSINGLEGUIAPPLICATION_H

#include <QGuiApplication>

class QtLocalPeer;

#if defined(Q_OS_WIN)
#  if !defined(QT_QTSINGLEAPPLICATION_EXPORT) && !defined(QT_QTSINGLEAPPLICATION_IMPORT)
#    define QT_QTSINGLEAPPLICATION_EXPORT
#  elif defined(QT_QTSINGLEAPPLICATION_IMPORT)
#    if defined(QT_QTSINGLEAPPLICATION_EXPORT)
#      undef QT_QTSINGLEAPPLICATION_EXPORT
#    endif
#    define QT_QTSINGLEAPPLICATION_EXPORT __declspec(dllimport)
#  elif defined(QT_QTSINGLEAPPLICATION_EXPORT)
#    undef QT_QTSINGLEAPPLICATION_EXPORT
#    define QT_QTSINGLEAPPLICATION_EXPORT __declspec(dllexport)
#  endif
#else
#  define QT_QTSINGLEAPPLICATION_EXPORT
#endif

class QT_QTSINGLEAPPLICATION_EXPORT QtSingleGuiApplication : public QGuiApplication
{
    Q_OBJECT

public:
    QtSingleGuiApplication(int &argc, char **argv, bool GUIenabled = true);
    QtSingleGuiApplication(const QString &id, int &argc, char **argv);
#if QT_VERSION < 0x050000
    QtSingleGuiApplication(int &argc, char **argv, Type type);
#  if defined(Q_WS_X11)
    QtSingleGuiApplication(Display* dpy, Qt::HANDLE visual = 0, Qt::HANDLE colormap = 0);
    QtSingleGuiApplication(Display *dpy, int &argc, char **argv, Qt::HANDLE visual = 0, Qt::HANDLE cmap= 0);
    QtSingleGuiApplication(Display* dpy, const QString &appId, int argc, char **argv, Qt::HANDLE visual = 0, Qt::HANDLE colormap = 0);
#  endif // Q_WS_X11
#endif // QT_VERSION < 0x050000

    bool isRunning();
    QString id() const;

    // Obsolete:
    void initialize(bool dummy = true)
        { isRunning(); Q_UNUSED(dummy) }

public Q_SLOTS:
    bool sendMessage(const QString &message, int timeout = 5000);


Q_SIGNALS:
    void messageReceived(const QString &message);


private:
    void sysInit(const QString &appId = QString());
    QtLocalPeer *peer;
};

#endif // QTSINGLEGUIAPPLICATION_H