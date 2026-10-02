#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "custom_qstring_converter.h"


int try_connect(const char *addrname, int port, const char *data)
{
    printf("%s %s\n", addrname, data);
    return 0;
}

int main(int argc, char **argv)
{
    QString ip = "10.41.120.172";
    QString candidate = "PASSTHRU";
    printf ("%d\n", try_connect(QTC(ip), 515, QTC(QString("\2%1\n").arg(candidate))));
    return 0;
}