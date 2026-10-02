#include <cups/cups.h>
#include <cups/http.h>
#include <errno.h>
#include <fcntl.h>
#include <libudev.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <syslog.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <glib.h>
#include <dirent.h>
#include <libudev.h>

#include <QString>
#include <QStringList>
#include <QDebug>

#include "udevDervices.h"

int main(int argc, char **argv)
{
    DeviceInformation devinfo;
    do_add(argv[2], nullptr, devinfo);
}