/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

#ifndef UKUI_MENU_UKUI_MENU_APPLICATION_H
#define UKUI_MENU_UKUI_MENU_APPLICATION_H

#include <QObject>
#include <keep-alive-client.h>

class QQmlEngine;
class QCommandLineOption;

namespace UkuiMenu {

class MenuMessageProcessor;
class MenuWindow;
class MenuDbusService;

class UkuiMenuApplication : public QObject
{
    Q_OBJECT
public:
    enum Command {
        Active = 0,
        Show,
        Quit,
        Hide
    };
    explicit UkuiMenuApplication(MenuMessageProcessor *processor);
    ~UkuiMenuApplication() override;
    UkuiMenuApplication() = delete;
    UkuiMenuApplication(const UkuiMenuApplication& obj) = delete;
    UkuiMenuApplication(const UkuiMenuApplication&& obj) = delete;

private Q_SLOTS:
    void execCommand(UkuiMenuApplication::Command command);

private:
    static void registerQmlTypes();
    void startUkuiMenu();
    void initQmlEngine();
    void loadMenuUI();

    //注册dubs
    void initDbusService();

    void keepAlive();
private:
    QQmlEngine *m_engine{nullptr};
    MenuWindow *m_mainWindow{nullptr};
    MenuDbusService *m_menuDbusService{nullptr};
    UkuiQuick::HeartBeat::KeepAliveClient *m_keepAliveClient{nullptr};
};

class MenuMessageProcessor : public QObject
{
    Q_OBJECT
public:
    MenuMessageProcessor();
    static bool preprocessMessage(const QStringList& message);
    static QCommandLineOption showUkuiMenu;
    static QCommandLineOption quitUkuiMenu;

public Q_SLOTS:
    void processMessage(const QString &message);

Q_SIGNALS:
    void request(UkuiMenuApplication::Command command);
};

}

#endif //UKUI_MENU_UKUI_MENU_APPLICATION_H
