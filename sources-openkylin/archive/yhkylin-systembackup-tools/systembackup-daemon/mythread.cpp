/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "mythread.h"

MyThread::MyThread(QObject *parent) :
    QThread(parent)
{}

MyThread::~MyThread()
{}
