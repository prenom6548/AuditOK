/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#include <QApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickView>
#include <QQmlContext>

#include "../output-manager.h"
#include "output-manager-monitor.h"

#include "workspace-demo-controller.h"

int main(int argc, char *argv[])
{
    QString sessionType(qgetenv("XDG_SESSION_TYPE"));
    if(sessionType == "wayland") {
        qputenv("QT_WAYLAND_DISABLE_FIXED_POSITIONS", "true");
    }
    QApplication app(argc, argv);
    QQuickView view;
    UkuiQuick::OutputManager outputManager;
    OutputManagerMonitor outputMonitor(&outputManager);
    auto workspaceDemoController = new WorkspaceDemoController(&view);
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.resize(1500, 950);
    view.engine()->addImportPath(QStringLiteral(TEST_QML_IMPORT_ROOT));
    view.rootContext()->setContextProperty(QStringLiteral("outputMonitor"), &outputMonitor);
    view.rootContext()->setContextProperty(QStringLiteral("WorkspaceDemo"), workspaceDemoController);
    view.setSource(QUrl("qrc:///main.qml"));
    view.show();
    //计算混色
    /*
    QColor back(245, 63, 63, 255);
    QColor fore(0, 0, 0, 0.2 * 255);
    qDebug() << fore.alphaF();
    qreal tiny = 1 - fore.alphaF();
    qreal alpha = fore.alphaF() + back.alphaF() * tiny;

    qreal r = (fore.redF() * fore.alphaF() +
               back.redF() * back.alphaF() * tiny) /
              alpha;
    qreal g = (fore.greenF() * fore.alphaF() +
               back.greenF() * back.alphaF() * tiny) /
              alpha;
    qreal b = (fore.blueF() * fore.alphaF() +
               back.blueF() * back.alphaF() * tiny) /
              alpha;

    qDebug() << QColor::fromRgbF(r, g, b, alpha);
    qDebug() << QColor::fromRgbF(r, g, b, alpha).red();
    qDebug() << QColor::fromRgbF(r, g, b, alpha).green();
    qDebug() << QColor::fromRgbF(r, g, b, alpha).blue();
    qDebug() << QColor::fromRgbF(r, g, b, alpha).alpha();*/
    return QApplication::exec();
}
