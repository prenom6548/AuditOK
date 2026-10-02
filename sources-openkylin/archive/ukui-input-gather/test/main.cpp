#include <QCoreApplication>
#include "gtest/gtest.h"
#include "ukuiinputgatherclient.h"

#include <QJsonArray>

void getEvent(Event *event);

int main(int argc, char *argv[])
{

    QCoreApplication a(argc, argv);

    UKUIInputGatherClient client;

    client.setEventCallBack(getEvent);

    client.startToReceiveEvent();

    EXPECT_EQ(client.libinputTabletSwitchState(), false);

    EXPECT_EQ(client.libinputDeviceList().isEmpty(), false);

    return 0;

}

void getEvent(Event *event)
{
    //LCOV_EXCL_START
    //LCOV_EXCL_STOP
}
