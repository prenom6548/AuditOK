/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <QFile>

#include <chrono>
#include <thread>

#include <unistd.h>

int main()
{
    const QByteArray markerPath = qgetenv("UKUI_QUICK_LOTTIE_BLOCKING_UNZIP_MARKER");
    if (markerPath.isEmpty()) {
        return 2;
    }

    QFile marker(QString::fromLocal8Bit(markerPath));
    if (!marker.open(QIODevice::WriteOnly | QIODevice::Truncate) ||
        marker.write(QByteArray::number(static_cast<qint64>(getpid()))) <= 0 ||
        !marker.flush()) {
        return 3;
    }
    marker.close();

    for (;;) {
        std::this_thread::sleep_for(std::chrono::hours(1));
    }
}
