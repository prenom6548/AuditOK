/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <termios.h>
#include <unistd.h>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QTextStream>

namespace
{

constexpr auto kServiceName = "org.fcitx.Fcitx5.Voice";
constexpr auto kObjectPath = "/voice";
constexpr auto kInterfaceName = "org.fcitx.Fcitx5.Voice";

char readKey()
{
    termios oldAttr{};
    termios newAttr{};
    tcgetattr(STDIN_FILENO, &oldAttr);
    newAttr = oldAttr;
    newAttr.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
    tcsetattr(STDIN_FILENO, TCSANOW, &newAttr);

    const int ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldAttr);
    return static_cast<char>(ch);
}

void printMenu(QTextStream &out)
{
    out << "\nVoiceService DBus Test\n";
    out << "1 - CommitString(\"helloworld\")\n";
    out << "2 - HasFocusWindow()\n";
    out << "3 - AppName()\n";
    out << "q - Quit\n";
    out.flush();
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    QTextStream err(stderr);

    QDBusInterface iface(QString::fromLatin1(kServiceName), QString::fromLatin1(kObjectPath), QString::fromLatin1(kInterfaceName),
                         QDBusConnection::sessionBus());

    if (!iface.isValid())
    {
        err << "Failed to connect to VoiceService DBus interface: " << iface.lastError().message() << "\n";
        err << "Make sure fcitx5 is running and the voice module is loaded.\n";
        return 1;
    }

    printMenu(out);

    while (true)
    {
        out << "Press a key: ";
        out.flush();

        const char key = readKey();
        out << key << "\n";

        switch (key)
        {
        case '1':
        {
            QDBusReply<void> reply = iface.call(QStringLiteral("CommitString"), QStringLiteral("helloworld"));
            if (reply.isValid())
            {
                out << "CommitString succeeded\n";
            }
            else
            {
                err << "CommitString failed: " << reply.error().message() << "\n";
            }
            break;
        }
        case '2':
        {
            QDBusReply<bool> reply = iface.call(QStringLiteral("HasFocusWindow"));
            if (reply.isValid())
            {
                out << "HasFocusWindow = " << (reply.value() ? "true" : "false") << "\n";
            }
            else
            {
                err << "HasFocusWindow failed: " << reply.error().message() << "\n";
            }
            break;
        }
        case '3':
        {
            QDBusReply<QString> reply = iface.call(QStringLiteral("AppName"));
            if (reply.isValid())
            {
                out << "AppName = \"" << reply.value() << "\"\n";
            }
            else
            {
                err << "AppName failed: " << reply.error().message() << "\n";
            }
            break;
        }
        case 'q':
        case 'Q':
            out << "Exiting test\n";
            return 0;
        default:
            out << "Unknown key, please enter 1 / 2 / 3 / q\n";
            break;
        }

        out.flush();
        err.flush();
    }
}
