/*
 * libkysdk-system's Library
 *
* Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#include "../../hardware/libkybluetooth.h"
#include "../../hardware/libkyedid.h"
#include "../../hardware/libkyfan.h"
#include "../../hardware/libkybios.h"
#include "../../hardware/libkyboard.h"
#include "../../hardware/libkycpu.h"
#include "../../hardware/libkydisplay.h"
#include "../../hardware/libkync.h"
#include "../../hardware/libkyprinter.h"
#include "../../hardware/libkyusb.h"
#include "../../hardware/libkyhw.h"
#include <stdio.h>
#include <stdlib.h>
#include <gtest/gtest.h>
#include <fcntl.h>
#include "../cpp-stub/src/stub.h"
#include <dbus-1.0/dbus/dbus.h>
#include <sensors/sensors.h>

extern char *strstr_stub (char *__haystack, const char *__needle)
{
    if(strcmp(__needle, "usb_interface") == 0)
    {
        return "1";
    }
    else
    {
        return nullptr;
    }
}

TEST(KdkNcTest, NcInfo)
{
    char **cards = kdk_nc_get_cardlist();
    size_t index = 0;
    printf("所有网卡：\n");
    while (cards[index])
    {
        char *mac = kdk_nc_get_phymac(cards[index]);
        char *ipv4 = kdk_nc_get_private_ipv4(cards[index]);
        char *ipv6 = kdk_nc_get_private_ipv6(cards[index]);
        char *driver = kdk_nc_get_driver(cards[index]);
        char vendor[256] = "\0", product[256] = "\0";
        kdk_nc_get_vendor_and_product(cards[index], vendor, product);
        printf("Card %zd: %s\tStatus: %s\tMac: %s\tIPv4: %s\tIPv6: %s\t Vendor: %s\t Product: %s\t Type: %s\t driver: %s\n",
               index + 1, cards[index], kdk_nc_is_up(cards[index]) == 1 ? "Up" : "Down",
               mac, ipv4, ipv6, vendor, product,
               kdk_nc_is_wireless(cards[index]) ? "wireless" : "ethernet", driver);

        if (mac)
            free(mac);
        if (ipv4)
            free(ipv4);
        if (ipv6)
            free(ipv6);
        if (driver)
            free(driver);

        char *uuid = kdk_nc_get_uuid(cards[index]);
        printf("%s\n", uuid ? uuid : "get uuid faile");
		//if(uuid)
        //	free(uuid);
        
        printf("carrier = %d\n", kdk_nc_get_carrier(cards[index]));

        char *duplex = kdk_nc_get_duplex(cards[index]);
        printf("%s\n", duplex ? duplex : "get duplex failed");
		//if(duplex)
        	//free(duplex);
		printf("11111111\n");
        char *conn = kdk_nc_get_conn_type(cards[index]);
        printf("Conn type = %s\n", conn);

        char *wifi_name = kdk_nc_get_wifi_name(cards[index]);
        printf("wifi name = %s\n", wifi_name);
		if(wifi_name)
        	free(wifi_name);

        char *speed = kdk_nc_get_speed(cards[index]);
        printf("speed = %s\n", speed);
		if(speed)
        	free(speed);

        int rx_packets = kdk_nc_get_rx_packets(cards[index]);
        printf("rx_packets = %d\n", rx_packets);

        int rx_bytes = kdk_nc_get_rx_bytes(cards[index]);
        printf("rx_bytes = %d\n", rx_bytes);

        int rx_errors = kdk_nc_get_rx_errors(cards[index]);
        printf("rx_errors = %d\n", rx_errors);

        int rx_dropped = kdk_nc_get_rx_dropped(cards[index]);
        printf("rx_dropped = %d\n", rx_dropped);

        int fifo_errors = kdk_nc_get_rx_fifo_errors(cards[index]);
        printf("fifo_errors = %d\n", fifo_errors);

        int frame_errors = kdk_nc_get_rx_frame_errors(cards[index]);
        printf("frame_errors = %d\n", frame_errors);

        int tx_packets = kdk_nc_get_tx_packets(cards[index]);
        printf("tx_packets = %d\n", tx_packets);

        int tx_bytes = kdk_nc_get_tx_bytes(cards[index]);
        printf("tx_bytes = %d\n", tx_bytes);

        int tx_errors = kdk_nc_get_tx_errors(cards[index]);
        printf("tx_errors = %d\n", tx_errors);

        int tx_dropped = kdk_nc_get_tx_dropped(cards[index]);
        printf("tx_dropped = %d\n", tx_dropped);

        int tx_fifo_errors = kdk_nc_get_tx_fifo_errors(cards[index]);
        printf("tx_fifo_errors = %d\n", tx_fifo_errors);

        int tx_carrier_errors = kdk_nc_get_tx_carrier_errors(cards[index]);
        printf("tx_carrier_errors = %d\n", tx_carrier_errors);

        int signal_qual = kdk_nc_get_wifi_signal_qual(cards[index]);
        printf("signal_qual = %d\n", signal_qual);

        int signal_level = kdk_nc_get_wifi_signal_level(cards[index]);
        printf("signal_level = %d\n", signal_level);

        int noise = kdk_nc_get_wifi_noise(cards[index]);
        printf("noise = %d\n", noise);

        char **list4 = kdk_nc_get_ipv4(cards[index]);
        int i = 0;
        printf("AllIpV4: ");
        while (list4 && list4[i])
        {
            printf("%s\t", list4[i]);

            char *boardaddr = kdk_nc_get_broadAddr(cards[index], list4[i]);
            printf("boardaddr = %s\n", boardaddr);
            free(boardaddr);

            char *netmask = kdk_nc_get_netmask(cards[index], list4[i]);
            printf("netmask = %s\n", netmask);
            free(netmask);

            free(list4[i++]);
        }
        printf("\n");
        char **list6 = kdk_nc_get_ipv6(cards[index]);
        printf("AllIpV6: ");
        i = 0;
        while (list6 && list6[i])
        {
            printf("%s\t", list6[i]);

            int len = kdk_nc_get_mask_length(0, cards[index], list6[i]);
            printf("netmask_length = %d\n", len);

            int scope = kdk_nc_get_scope(cards[index], list6[i]);
            printf("scope = %d\n", scope);
            
            free(list6[i++]);
        }
        printf("\n");

        free(list4);
        free(list6);
        index++;
    }
    kdk_nc_freeall(cards);

    char **upcards = kdk_nc_get_upcards();
    index = 0;
    printf("工作网卡：\n");
    while (upcards[index])
    {
        printf("Card %zd: %s\n", index + 1, upcards[index]);
        index++;
    }
    kdk_nc_freeall(upcards);

    char **list = kdk_nc_get_cardlist();
    index = 0;
    printf("所有网卡：\n");
    while (list[index])
    {
        char vendor[256] = "\0", product[256] = "\0";
        Stub st;
        char* (*original_strstr)(char*, const char*) = &strstr;
        st.set(original_strstr, strstr_stub);
        kdk_nc_get_vendor_and_product(list[index], vendor, product);
        printf("Vendor: %s\t Product: %s\t\n", vendor, product);
        index++;
    }
    kdk_nc_freeall(list);
}

TEST(KdkBluetoothTest, BluetoothInfo)
{
    int i = 0;
    char *manufacturer = "\0";
    char *version = "\0";
    char *name = "\0";
    char *address = "\0";
    char *link_mode  ="\0"; 
    char *link_policy ="\0";
    char *scomtu = "\0";
    char *alcmtu = "\0";
    char *packettype = "\0";
    char *features = "\0";
    char *bus = "\0";
    int *id = (int *)kdk_bluetooth_get_device_id();
    // if(NULL == id)
    //     return 0;
    // for(; id[i]; i++)
    // {
    //     printf("id = %d\n",id[i]);

        manufacturer = kdk_bluetooth_get_manufacturer(0);
        printf("manufacturer: %s\n", manufacturer);
        free(manufacturer);

        version = kdk_bluetooth_get_dev_version(0);
        printf("version: %s\n", version);
        free(version);

        name = kdk_bluetooth_get_name(0);
        printf("name: %s\n", name);

        address = kdk_bluetooth_get_address(0);
        printf("address: %s\n", address);

        link_mode = kdk_bluetooth_get_link_mode(0);
        printf("link_mode: %s\n", link_mode);

        link_policy = kdk_bluetooth_get_link_policy(0);
        printf("link_policy: %s\n", link_policy);

        scomtu = kdk_bluetooth_get_scomtu(0);
        printf("scomtu: %s\n", scomtu);

        alcmtu = kdk_bluetooth_get_alcmtu(0);
        printf("alcmtu: %s\n", alcmtu);

        packettype = kdk_bluetooth_get_packettype(0);
        printf("packettype: %s\n", packettype);

        features = kdk_bluetooth_get_features(0);
        printf("features: %s\n", features);

        bus = kdk_bluetooth_get_bus(0);
        printf("bus: %s\n", bus);

        char** con_addr = kdk_bluetooth_get_device_addr(0);
        int index = 0;
        if(con_addr != NULL)
        {
            while (con_addr[index])
            {
                printf("addr: %s\n", con_addr[index]);
                index++;
            }
            kdk_bluetooth_freeall(con_addr);
        }

        char** con_name = kdk_bluetooth_get_device_name(0);
        index = 0;
        if(con_name != NULL)
        {
            while (con_name[index])
            {
                printf("name: %s\n", con_name[index]);
                index++;
            }

            kdk_bluetooth_freeall(con_name);
        }

        bool status = kdk_bluetooth_get_connection_status(0);
        printf("status = %d\n", status);

        char** addr_type =  kdk_bluetooth_get_addr_type(0);
        index = 0;
        if(addr_type != NULL)
        {
            while (addr_type[index])
            {
                printf("addr type: %s\n", addr_type[index]);
                index++;
            }

            kdk_bluetooth_freeall(addr_type);
        }
        // kdk_bluetooth_freeall(addr_type);

        kdk_major_class list = kdk_bluetooth_get_major_class(0);
        kdk_major_class major = list;
        // if(major != NULL)
        // {
            while (major)
            {
                printf("addr:%s, type:%d\n", major->addr, major->type);
                major = major->next;
            }

            kdk_bluetooth_free_major_class(major);
        // }

        // free((int*)id[i]);
    // }
    // free((int**)id);
}

int open_stub(const char *__path, int __oflag, ...)
{
    return -1;
}

extern char *getenv_stub(const char *__name)
{
    return "0";
}

char *fgets_stub(char *__restrict __s, int __n, FILE *__restrict __stream)
{
    return nullptr;
}

extern FILE *fopen_stub (const char *__restrict __filename, const char *__restrict __modes)
{
    return nullptr;
}

extern char *realpath_stub (const char *__restrict __name, char *__restrict __resolved) __THROW __wur
{
    return "/kylin";
}

TEST(KdkEdidTest, EdidInfoStub)
{
    Stub st;
    st.set(open, open_stub);
    
    float gamma = kdk_edid_get_gamma("eDP-1");
    printf("gamma = %0.2f\n", gamma);
    //EXPECT_NE(gamma, 0.0);

    float size = kdk_edid_get_size("eDP-1");
    printf("size = %0.1f\n", size);
    //EXPECT_NE(size, 0);

    char *model = kdk_edid_get_model("eDP-1");
    printf("model = %s\n", model);
    free(model);
    //EXPECT_NE(model, nullptr);

    char *area = kdk_edid_get_visible_area("eDP-1");
    printf("area = %s\n", area);
    //EXPECT_NE(area, nullptr);

    char *manufacturer = kdk_edid_get_manufacturer("eDP-1");
    printf("manufacturer = %s\n", manufacturer);
    //EXPECT_NE(manufacturer, nullptr);

    int week = kdk_edid_get_week("eDP-1");
    printf("week = %d\n", week);
    //EXPECT_NE(week, -1);

    int year = kdk_edid_get_year("eDP-1");
    printf("year = %d\n", year);
    //EXPECT_NE(year, -1);

    char *serial = kdk_edid_get_serialNumber("eDP-1");
    printf("serial = %s\n", serial);
    //EXPECT_NE(serial, nullptr);

    ChromaticityCoordinates *red = kdk_edid_get_red_primary("eDP-1");
    if (red)
        printf("redx = %s, redy = %s\n", red->xCoordinate, red->yCoordinate);
    //EXPECT_NE(red->xCoordinate, nullptr);
    //EXPECT_NE(red->yCoordinate, nullptr);

    ChromaticityCoordinates *green = kdk_edid_get_green_primary("eDP-1");
    if (green)
        printf("greenx = %s, greeny = %s\n", green->xCoordinate, green->yCoordinate);
    //EXPECT_NE(green->xCoordinate, nullptr);
    //EXPECT_NE(green->yCoordinate, nullptr);

    ChromaticityCoordinates *blue = kdk_edid_get_blue_primary("eDP-1");
    if (blue)
        printf("bluex = %s, bluey = %s\n", blue->xCoordinate, blue->yCoordinate);
    //EXPECT_NE(blue->xCoordinate, nullptr);
    //EXPECT_NE(blue->yCoordinate, nullptr);

    ChromaticityCoordinates *white = kdk_edid_get_white_primary("eDP-1");
    if (white)
        printf("whitex = %s, whitey = %s\n", white->xCoordinate, white->yCoordinate);
    //EXPECT_NE(white->xCoordinate, nullptr);
    //EXPECT_NE(white->yCoordinate, nullptr);

    st.set(fgets, fgets_stub);
    model = kdk_edid_get_model("eDP-1");
    printf("model = %s\n", model);
    //EXPECT_EQ(model, nullptr);

    st.set(fopen, fopen_stub);

    char *character = kdk_edid_get_character("eDP-1");
    printf("character = %s\n", character);
    //EXPECT_NE(character, nullptr);
    free(character);

    gamma = kdk_edid_get_gamma("eDP-1");
    printf("gamma = %0.2f\n", gamma);
    //EXPECT_EQ(gamma, 0);

    size = kdk_edid_get_size("eDP-1");
    printf("size = %0.1f\n", size);
    //EXPECT_EQ(size, 0);

    model = kdk_edid_get_model("eDP-1");
    printf("model = %s\n", model);
    //EXPECT_EQ(model, nullptr);

    manufacturer = kdk_edid_get_manufacturer("eDP-1");
    printf("manufacturer = %s\n", manufacturer);
    //EXPECT_EQ(manufacturer, nullptr);

    week = kdk_edid_get_week("eDP-1");
    printf("week = %d\n", week);
    //EXPECT_EQ(week, -1);

    year = kdk_edid_get_year("eDP-1");
    printf("year = %d\n", year);
    //EXPECT_EQ(year, -1);

    serial = kdk_edid_get_serialNumber("eDP-1");
    printf("serial = %s\n", serial);
    //EXPECT_EQ(serial, nullptr);

    red = kdk_edid_get_red_primary("eDP-1");

    green = kdk_edid_get_green_primary("eDP-1");

    blue = kdk_edid_get_blue_primary("eDP-1");

    white = kdk_edid_get_white_primary("eDP-1");

    BrightnessInfo *cur_brightness = kdk_edid_get_current_brightness("eDP-1");
    if (cur_brightness)
    {
        printf("brightness = %d\n", cur_brightness->brightness_percentage);
        printf("brightness_value = %d\n", cur_brightness->brightness_value);
        //EXPECT_NE(cur_brightness->brightness_percentage, 0);
        //EXPECT_EQ(cur_brightness->brightness_value, 0);
        kdk_free_brightnessInfo(cur_brightness);
    }
}

TEST(KdkEdidTest, EdidInfoStub1)
{
    Stub st;
    st.set(getenv, getenv_stub);

    char *max_resolution = kdk_edid_get_max_resolution("eDP-1");
    printf("max_resolution = %s\n", max_resolution);
    //EXPECT_NE(max_resolution, nullptr);
    free(max_resolution);

    char *resolution = kdk_edid_get_resolution("eDP-1");
    printf("resolution = %s\n", resolution);
    //EXPECT_NE(resolution, nullptr);
    free(resolution);

    char *Rate = kdk_edid_get_refreshRate("eDP-1");
    printf("Rate = %s\n", Rate);
    //EXPECT_NE(Rate, nullptr);
}

TEST(KdkEdidTest, EdidInfoStub2)
{
    Stub st;
    st.set(fgets, fgets_stub);

    char *model = kdk_edid_get_model("eDP-1");
    printf("model = %s\n", model);
    //EXPECT_NE(model, nullptr);
    free(model);
}

TEST(KdkEdidTest, EdidInfoStub3)
{
    Stub st;
    st.set(realpath, realpath_stub);
    ChromaticityCoordinates *red = kdk_edid_get_red_primary("eDP-1");
    if (red)
        printf("redx = %s, redy = %s\n", red->xCoordinate, red->yCoordinate);
    //EXPECT_NE(red->xCoordinate, nullptr);
    //EXPECT_NE(red->yCoordinate, nullptr);

    ChromaticityCoordinates *green = kdk_edid_get_green_primary("eDP-1");
    if (green)
        printf("greenx = %s, greeny = %s\n", green->xCoordinate, green->yCoordinate);
    //EXPECT_NE(green->xCoordinate, nullptr);
    //EXPECT_NE(green->yCoordinate, nullptr);

    ChromaticityCoordinates *blue = kdk_edid_get_blue_primary("eDP-1");
    if (blue)
        printf("bluex = %s, bluey = %s\n", blue->xCoordinate, blue->yCoordinate);
    //EXPECT_NE(blue->xCoordinate, nullptr);
    //EXPECT_NE(blue->yCoordinate, nullptr);

    ChromaticityCoordinates *white = kdk_edid_get_white_primary("eDP-1");
    if (white)
        printf("whitex = %s, whitey = %s\n", white->xCoordinate, white->yCoordinate);
}

TEST(KdkEdidTest, EdidInfoStub4)
{
    Stub st;
    st.set(fopen, fopen_stub);
    BrightnessInfo *cur_brightness = kdk_edid_get_current_brightness("eDP-1");
    if (cur_brightness)
    {
        printf("brightness = %d\n", cur_brightness->brightness_percentage);
        printf("brightness_value = %d\n", cur_brightness->brightness_value);
        //EXPECT_NE(cur_brightness->brightness_percentage, 0);
        //EXPECT_EQ(cur_brightness->brightness_value, 0);
        kdk_free_brightnessInfo(cur_brightness);
    }
}

TEST(KdkEdidTest, EdidInfo)
{
    char **name = kdk_edid_get_interface();

    for (size_t count = 0; name[count]; count++)
    {
        printf("当前接口 = %s\n", name[count]);
        //EXPECT_NE(name[count], nullptr);

        float gamma = kdk_edid_get_gamma(name[count]);
        printf("gamma = %0.2f\n", gamma);
        //EXPECT_NE(gamma, 0);

        float size = kdk_edid_get_size(name[count]);
        printf("size = %0.1f\n", size);
        //EXPECT_NE(size, 0);

        char *max_resolution = kdk_edid_get_max_resolution(name[count]);
        printf("max_resolution = %s\n", max_resolution);
        //EXPECT_NE(max_resolution, nullptr);
        free(max_resolution);

        char *model = kdk_edid_get_model(name[count]);
        printf("model = %s\n", model);
        //EXPECT_NE(model, nullptr);
        free(model);

        char *area = kdk_edid_get_visible_area(name[count]);
        printf("area = %s\n", area);
        //EXPECT_NE(area, nullptr);
        free(area);

        char *manufacturer = kdk_edid_get_manufacturer(name[count]);
        printf("manufacturer = %s\n", manufacturer);
        //EXPECT_NE(manufacturer, nullptr);
        free(manufacturer);

        int week = kdk_edid_get_week(name[count]);
        printf("week = %d\n", week);
        //EXPECT_NE(week, 0);

        int year = kdk_edid_get_year(name[count]);
        printf("year = %d\n", year);
        //EXPECT_NE(year, 0);

        int primary = kdk_edid_get_primary(name[count]);
        printf("%s\n", primary == 1 ? "是" : "否");
        //EXPECT_NE(primary, 0);

        char *resolution = kdk_edid_get_resolution(name[count]);
        printf("resolution = %s\n", resolution);
        //EXPECT_NE(resolution, nullptr);
        free(resolution);

        char *ratio = kdk_edid_get_ratio(name[count]);
        printf("ratio = %s\n", ratio);
        //EXPECT_NE(ratio, nullptr);
        free(ratio);

        char *character = kdk_edid_get_character(name[count]);
        printf("character = %s\n", character);
        //EXPECT_NE(character, nullptr);
        free(character);

        char *serial = kdk_edid_get_serialNumber(name[count]);
        printf("serial = %s\n", serial);
        //EXPECT_NE(serial, nullptr);
        free(serial);

        BrightnessInfo *max_info = kdk_edid_get_max_brightness(name[count]);
        if (max_info)
        {
            printf("brightness = %d\n", max_info->brightness_percentage);
            printf("brightness_value = %d\n", max_info->brightness_value);
            //EXPECT_NE(max_info->brightness_percentage, 0);
            //EXPECT_EQ(max_info->brightness_value, 0);
        }

        BrightnessInfo *cur_brightness = kdk_edid_get_current_brightness(name[count]);
        if (cur_brightness)
        {
            printf("brightness = %d\n", cur_brightness->brightness_percentage);
            printf("brightness_value = %d\n", cur_brightness->brightness_value);
            //EXPECT_NE(cur_brightness->brightness_percentage, 0);
            //EXPECT_EQ(cur_brightness->brightness_value, 0);
            kdk_free_brightnessInfo(cur_brightness);
        }

        ChromaticityCoordinates *red = kdk_edid_get_red_primary(name[count]);
        if (red)
            printf("redx = %s, redy = %s\n", red->xCoordinate, red->yCoordinate);
        //EXPECT_NE(red->xCoordinate, nullptr);
        //EXPECT_NE(red->yCoordinate, nullptr);

        ChromaticityCoordinates *green = kdk_edid_get_green_primary(name[count]);
        if (green)
            printf("greenx = %s, greeny = %s\n", green->xCoordinate, green->yCoordinate);
        //EXPECT_NE(green->xCoordinate, nullptr);
        //EXPECT_NE(green->yCoordinate, nullptr);

        ChromaticityCoordinates *blue = kdk_edid_get_blue_primary(name[count]);
        if (blue)
            printf("bluex = %s, bluey = %s\n", blue->xCoordinate, blue->yCoordinate);
        //EXPECT_NE(blue->xCoordinate, nullptr);
        //EXPECT_NE(blue->yCoordinate, nullptr);

        ChromaticityCoordinates *white = kdk_edid_get_white_primary(name[count]);
        if (white)
            printf("whitex = %s, whitey = %s\n", white->xCoordinate, white->yCoordinate);
        //EXPECT_NE(white->xCoordinate, nullptr);
        //EXPECT_NE(white->yCoordinate, nullptr);

        //kdk_free_chromaticityCoordinates(white);

        float rawDpiX = kdk_edid_get_rawDpiX(name[count]);
        printf("rawDpiX = %.2f\n", rawDpiX);
        //EXPECT_NE(rawDpiX, 0);

        float rawDpiY = kdk_edid_get_rawDpiY(name[count]);
        printf("rawDpiY = %.2f\n", rawDpiY);
        //EXPECT_NE(rawDpiY, 0);

        char *Rate = kdk_edid_get_refreshRate(name[count]);
        printf("Rate = %s\n", Rate);
        //EXPECT_NE(Rate, nullptr);

        char *rotation = kdk_edid_get_rotation(name[count]);
        printf("rotation = %s\n", rotation);
        //EXPECT_NE(rotation, nullptr);

        // int ret = kdk_edid_set_resolution(name[count], "1920", "1080");
        // printf("ret = %d\n", ret);

        printf("---------------------------------------\n");
    }

    int ret;

    ret = kdk_edid_set_resolution("eDP-1", "1920", "1080");
    printf("ret = %d\n", ret);
    //EXPECT_NE(ret, 0);

    ret = kdk_edid_set_resolution("VGA-0", "1920", "1080");
    printf("ret = %d\n", ret);
    //EXPECT_EQ(ret, 0);
    kdk_edid_freeall(name);
}

const sensors_feature *sensors_get_features_stub(const sensors_chip_name *name, int *nr)
{
    sensors_feature *iter;
    iter->type = SENSORS_FEATURE_FAN;
    return iter;
}

TEST(KdkFanTest, FanInfo)
{
    char **fan = kdk_fan_get_information();
    int index = 0;
    if(fan != NULL)
    {
        while(fan[index])
        {
            printf("%s\n", fan[index]);
            index++;
        }
        kdk_fan_freeall(fan);
    }
}

// TEST(KdkFanTest, FanInfoStub)
// {
//     Stub st;
//     st.set(sensors_get_features, sensors_get_features_stub);
//     char **fan = kdk_fan_get_information();
//     int index = 0;
//     if(fan != NULL)
//     {
//         while(fan[index])
//         {
//             printf("%s\n", fan[index]);
//             index++;
//         }
//         kdk_fan_freeall(fan);
//     }
// }

TEST(KdkBiosTest, BiosInfo)
{
    const char *vendor = kdk_bios_get_vendor();
    const char *version = kdk_bios_get_version();
    printf("vendor: %s\n", vendor);
    printf("version: %s\n",version);
    //EXPECT_STREQ(vendor, "ZD Tech.");
    //EXPECT_STREQ(version, "KunLun BIOS V4.0");
    
    // kdk_bios_free(vendor);
    // kdk_bios_free(version);

    char *smbios_version = kdk_bios_get_smbios_version();
    printf("SMBIOS Version: %s\n", smbios_version);
    //EXPECT_STREQ(smbios_version, "24.3");
    kdk_bios_free(smbios_version);

    char *bios_date = kdk_bios_get_date();
    printf("date: %s\n", bios_date);
    //EXPECT_STREQ(bios_date, "07/16/2020");
    kdk_bios_free(bios_date);

    char *type = kdk_bios_get_type();
    printf("type: %s\n", type);
    //EXPECT_STREQ(type, "UEFI");
    kdk_bios_free(type);
}

DBusMessage *dbus_message_new_method_call_stub(const char *destination, const char *path, const char *iface, const char *method)
{
    return nullptr;
}

TEST(KdkBoardTest, BoardInfo)
{
    const char *name = kdk_board_get_name();
    const char *vendor = kdk_board_get_vendor();
    const char *date = kdk_board_get_date();
    const char *serial = kdk_board_get_serial();
    printf("name : %s", name);
    printf("vendor : %s", vendor);
    printf("date : %s", date);
    printf("serial : %s\n", serial);
    //EXPECT_STREQ(name, "GW-001M1A-FTF\n");
    //EXPECT_STREQ(vendor, "Greatwall\n");
    //EXPECT_STREQ(date, "07/16/2020\n");
    //EXPECT_STREQ(serial, "Base Board Serial#");
    // kdk_board_free(name);
    // kdk_board_free(vendor);
    // kdk_board_free(date);
    // kdk_board_free(serial);
    kdk_board_free(nullptr);

    Stub st;
    st.set(dbus_message_new_method_call, dbus_message_new_method_call_stub);
    serial = kdk_board_get_serial();
    printf("serial : %s\n", serial);
    //EXPECT_STREQ(serial, "Base Board Serial#\n");
}

extern int atoi_stub (const char *__nptr)
{
    return 0;
}

TEST(KdkCpuTest, CpuInfo)
{
    const char *arch = kdk_cpu_get_arch();
    printf("架构：%s\n", arch);
    //EXPECT_STREQ(arch, "aarch64");

    const char *vendor = kdk_cpu_get_vendor();
    printf("生产厂商：%s\n", vendor);
    //EXPECT_STREQ(vendor, "Phytium");

    const char *model = kdk_cpu_get_model();
    printf("CPU 型号：%s\n", model);
    //EXPECT_STREQ(model, "FT-2000/4");

    const char *freq = kdk_cpu_get_freq_MHz();
    printf("CPU 主频：%s MHz\n", freq);
    //EXPECT_STREQ(freq, "2600");

    unsigned int corenums = kdk_cpu_get_corenums();
    printf("CPU 单核核心数量：%u\n", corenums);
    //EXPECT_EQ(corenums, 4);

    const char *virt = kdk_cpu_get_virt();
    printf("CPU 虚拟化支持：%s\n", virt);
    //EXPECT_EQ(virt, nullptr);

    unsigned int process = kdk_cpu_get_process();
    printf("CPU 线程数：%u\n", process);
    //EXPECT_EQ(process, 4);

    float max_freq_MHz = kdk_cpu_get_max_freq_MHz();
    printf("CPU 最大频率：%0.2f MHz\n", max_freq_MHz);
    //EXPECT_EQ(max_freq_MHz, 2600);

    float min_freq_MHz = kdk_cpu_get_min_freq_MHz();
    printf("CPU 最小频率：%0.2f MHz\n", min_freq_MHz);
    //EXPECT_EQ(min_freq_MHz, 1900);


    char *run_time = kdk_cpu_get_running_time();
    printf("CPU 运行时间：%s\n", run_time);
    //EXPECT_NE(run_time, nullptr);
    free(run_time);

    int sockets = kdk_cpu_get_sockets();
    printf("CPU 插槽：%d\n", sockets);
    //EXPECT_EQ(sockets, 2);

    int L1d_cache = kdk_cpu_get_L1d_cache();
    printf("CPU L1缓存（数据）：%d\n", L1d_cache);
    //EXPECT_EQ(L1d_cache, 32);

    int L1i_cache = kdk_cpu_get_L1i_cache();
    printf("CPU L1缓存（指令）：%d\n", L1i_cache);
    //EXPECT_EQ(L1i_cache, 32);

    int L2_cache = kdk_cpu_get_L2_cache();
    printf("CPU L2缓存：%d\n", L2_cache);
    //EXPECT_EQ(L2_cache, 2048);

    int L3_cache = kdk_cpu_get_L3_cache();
    printf("CPU L3缓存：%d\n", L3_cache);
    //EXPECT_EQ(L3_cache, 4096);

    Stub st;
    st.set(atoi, atoi_stub);
    L1d_cache = kdk_cpu_get_L1d_cache();
    printf("CPU L1缓存（数据）：%d\n", L1d_cache);
    //EXPECT_EQ(L1d_cache, 32);

    L1i_cache = kdk_cpu_get_L1i_cache();
    printf("CPU L1缓存（指令）：%d\n", L1i_cache);
    //EXPECT_EQ(L1i_cache, 32);

    L2_cache = kdk_cpu_get_L2_cache();
    printf("CPU L2缓存：%d\n", L2_cache);
    //EXPECT_EQ(L2_cache, 2048);

    L3_cache = kdk_cpu_get_L3_cache();
    printf("CPU L3缓存：%d\n", L3_cache);
}

extern int strncmp_stub (const char *__s1, const char *__s2, size_t __n)
{
    return 1;
}

TEST(KdkCpuTest, CpuInfoStub1)
{
    Stub st;
    st.set(strncmp, strncmp_stub);
    const char *arch = kdk_cpu_get_arch();
    printf("架构：%s\n", arch);
}

TEST(KdkDisplayTest, DisplayInfo)
{
    char *vendor = kdk_display_get_vendor();
    printf("生产厂商：%s\n", vendor);
    //EXPECT_STREQ(vendor, "Advanced Micro Devices, Inc. [AMD/ATI]");
    free(vendor);

    char *product = kdk_display_get_product();
    printf("型号：%s\n", product);
    //EXPECT_STREQ(product, "Oland [Radeon HD 8570 / R5 430 OEM / R7 240/340 / Radeon 520 OEM]");
    free(product);

    char *description = kdk_display_get_description();
    printf("设备类型：%s\n", description);
    //EXPECT_STREQ(description, "VGA compatible controller");
    free(description);

    char *id = kdk_display_get_physical_id();
    printf("物理id：%s\n", id);
    //EXPECT_STREQ(id, "0");
    free(id);

    char *bus = kdk_display_get_bus_info();
    printf("总线地址：%s\n", bus);
    //EXPECT_STREQ(bus, "pci@0000:02:00.0");
    free(bus);

    char *version = kdk_display_get_version();
    printf("设备版本：%s\n", version);
    //EXPECT_STREQ(version, "87");
    free(version);

    char *width = kdk_display_get_width();
    printf("数据宽度：%s\n", width);
    //EXPECT_STREQ(width, "64 bits");
    free(width);

    char *clock = kdk_display_get_clock();
    printf("频率：%s\n", clock);
    //EXPECT_STREQ(clock, "33MHz");
    free(clock);

    char *capabilities = kdk_display_get_capabilities();
    printf("功能：%s\n", capabilities);
    //EXPECT_STREQ(capabilities, "pm pciexpress msi vga_controller bus_master cap_list rom");
    free(capabilities);

    char *configuration = kdk_display_get_configuration();
    printf("配置：%s\n", configuration);
    //EXPECT_STREQ(configuration, "driver=amdgpu latency=0");
    free(configuration);

    char *resources = kdk_display_get_resources();
    printf("资源：%s\n", resources);
    //EXPECT_STREQ(resources, "iomemory:100-ff irq:40 memory:1040000000-107fffffff memory:58000000-5803ffff ioport:1000(size=256) ioport:58040000(size=131072)");
    free(resources);


    Stub st;
    st.set(dbus_message_new_method_call, dbus_message_new_method_call_stub);
    vendor = kdk_display_get_vendor();
    printf("生产厂商：%s\n", vendor);
    //EXPECT_STREQ(vendor, "Advanced Micro Devices, Inc. [AMD/ATI]");
    free(vendor);

    product = kdk_display_get_product();
    printf("型号：%s\n", product);
    //EXPECT_STREQ(product, "Oland [Radeon HD 8570 / R5 430 OEM / R7 240/340 / Radeon 520 OEM]");
    free(product);

    description = kdk_display_get_description();
    printf("设备类型：%s\n", description);
    //EXPECT_STREQ(description, "VGA compatible controller");
    free(description);

    id = kdk_display_get_physical_id();
    printf("物理id：%s\n", id);
    //EXPECT_STREQ(id, "0");
    free(id);

    bus = kdk_display_get_bus_info();
    printf("总线地址：%s\n", bus);
    //EXPECT_STREQ(bus, "pci@0000:02:00.0");
    free(bus);

    version = kdk_display_get_version();
    printf("设备版本：%s\n", version);
    //EXPECT_STREQ(version, "87");
    free(version);

    width = kdk_display_get_width();
    printf("数据宽度：%s\n", width);
    //EXPECT_STREQ(width, "64 bits");
    free(width);

    clock = kdk_display_get_clock();
    printf("频率：%s\n", clock);
    //EXPECT_STREQ(clock, "33MHz");
    free(clock);

    capabilities = kdk_display_get_capabilities();
    printf("功能：%s\n", capabilities);
    //EXPECT_STREQ(capabilities, "pm pciexpress msi vga_controller bus_master cap_list rom");
    free(capabilities);

    configuration = kdk_display_get_configuration();
    printf("配置：%s\n", configuration);
    //EXPECT_STREQ(configuration, "driver=amdgpu latency=0");
    free(configuration);

    resources = kdk_display_get_resources();
    printf("资源：%s\n", resources);
    //EXPECT_STREQ(resources, "iomemory:100-ff irq:40 memory:1040000000-107fffffff memory:58000000-5803ffff ioport:1000(size=256) ioport:58040000(size=131072)");
    free(resources);
}


TEST(KdkPrintTest, PrintInfo)
{
    int index = 0;
    //获取打印机列表
    char **printers = kdk_printer_get_available_list();
    while (printers[index])
    {
        printf("%zd: %s\n", index + 1, printers[index]);

        //取消当前打印机所有打印任务
        kdk_printer_cancel_all_jobs(printers[index]);

        //获取url对应的文件名
        const char *url = "http://www.cztouch.com/upfiles/soft/testpdf.pdf";
        int res = -1;
        char *filename = kdk_printer_get_filename(url); //从完整路径名中解析出文件名称，例如：/home/test/abc.txt,解析完成后为abc.txt
        printf("filename = %s\n", filename);

        //设置打印参数
        kdk_printer_set_options(2, "A4", "lrtb", "two-sided-long-edge");

        //下载
        res = kdk_printer_print_download_remote_file(url, filename);
        printf("[%s] res = %d\n", __FUNCTION__, res);
        printf("[%s] filepath = %s\n", __FUNCTION__, filename);

        //打印
        int job_id = kdk_printer_print_local_file(printers[index], filename);
        if (job_id == 0)
        {   
            //打印失败
            printf("[%s] create print job fail\n", __FUNCTION__);
        }
        else
        {
            //打印成功，等待一会获取任务状态
            sleep(20);
            int status = kdk_printer_get_job_status(printers[index], job_id);
        }

        index++;
        free(filename);
    }
    kdk_printer_freeall(printers);

    index = 0;
    //获取全部打印机列表
    char **printersList = kdk_printer_get_list();
    while (printersList[index])
    {
        int status = kdk_printer_get_status(printersList[index]);
        printf("%zd: %s , %d\n", index + 1, printersList[index], status);
        index++;
    }
    kdk_printer_freeall(printersList);
}

TEST(KdkUsbTest, UsbInfo)
{
    pDriverInfoList list = kdk_usb_get_list();
    if(list)
    {
        pDriverInfoList tmp = list;
        while (tmp)
        {
            printf("Name: %s\t",tmp->data->name);
            printf("Class: %s\t",tmp->data->type);
            printf("PID: %s\t",tmp->data->pid);
            printf("VID: %s\t",tmp->data->vid);
            printf("Serial: %s\t",tmp->data->serialNo);
            printf("DevNode: %s\n",tmp->data->devNode);
            tmp = tmp->next;
        }
        
    }
    kdk_usb_free(list);

    pUsbInfo usb_info = kdk_usb_get_bus_info();
    pUsbInfo tmp = usb_info;
    while(tmp)
    {
        printf("%03u:%03u\n", tmp->busNum, tmp->devNum);
        
        int mount = kdk_usb_get_mount(tmp->busNum,tmp->devNum);
        printf("mount = %d\n", mount);

        char *productName = kdk_usb_get_productName(tmp->busNum,tmp->devNum);
        printf("productName = %s\n", productName);

        char *manufacturerName = kdk_usb_get_manufacturerName(tmp->busNum,tmp->devNum);
        printf("manufacturerName = %s\n", manufacturerName);

        char *version = kdk_usb_get_version(tmp->busNum,tmp->devNum);
        printf("version = %s\n", version);

        int deviceClass = kdk_usb_get_deviceClass(tmp->busNum,tmp->devNum);
        printf("deviceClass = %d\n", deviceClass);

        int deviceSubClass = kdk_usb_get_deviceSubClass(tmp->busNum,tmp->devNum);
        printf("deviceSubClass = %d\n", deviceSubClass);
        
        int deviceProtocol = kdk_usb_get_deviceProtocol(tmp->busNum,tmp->devNum);
        printf("deviceProtocol = %d\n", deviceProtocol);

        char *speed = kdk_usb_get_speed(tmp->busNum,tmp->devNum);
        printf("speed = %s\n", speed);

        pUsbDeviceDescriptor desc = kdk_usb_get_device_descriptor(tmp->busNum,tmp->devNum);
        printf("%d %d %d %d\n",desc->lenth,desc->descriptorType, desc->maxPacketSize0, desc->numConfigurations);
        kdk_usb_free_usb_device_descriptor(desc);

        Stub st;
        st.set(dbus_message_new_method_call, dbus_message_new_method_call_stub);
        productName = kdk_usb_get_productName(tmp->busNum,tmp->devNum);
        printf("productName = %s\n", productName);

        manufacturerName = kdk_usb_get_manufacturerName(tmp->busNum,tmp->devNum);
        printf("manufacturerName = %s\n", manufacturerName);
        tmp = tmp->next;
    }

    kdk_usb_free_usb_info(tmp);
}

TEST(KdkHwinfoTest, HwInfo)
{
    struct HWInfo *keyboard = kdk_hw_get_hwinfo(3);
    printf("KeyBoard Info:\n");
    struct HWInfo *tmp = keyboard;
    while (tmp)
    {
        printf("\tmodel : %s\n", tmp->model);
        printf("\t\tvendor : %s\n", tmp->vendor);
        printf("\t\tdevice : %s\n", tmp->device);
        printf("\t\tdriver : %s\n", tmp->driver);
        printf("\t\trevision : %s\n", tmp->revision);
        printf("\t\tdevicenum : %s\n", tmp->devicenum);
        printf("\t\tclock : %s\n", tmp->clock);
        printf("\t\twidth : %s\n", tmp->width);
        tmp = tmp->next;
    }
    kdk_hw_free_hw(keyboard);

    struct HWInfo *mouse = kdk_hw_get_hwinfo(5);
    printf("Mouse Info:\n");
    tmp = mouse;
    while (tmp)
    {
        printf("\tmodel : %s\n", tmp->model);
        printf("\t\tvendor : %s\n", tmp->vendor);
        printf("\t\tdevice : %s\n", tmp->device);
        printf("\t\tdriver : %s\n", tmp->driver);
        printf("\t\trevision : %s\n", tmp->revision);
        printf("\t\tdevicenum : %s\n", tmp->devicenum);
        printf("\t\tclock : %s\n", tmp->clock);
        printf("\t\twidth : %s\n", tmp->width);
        tmp = tmp->next;
    }
    kdk_hw_free_hw(mouse);

    struct HWInfo *bound = kdk_hw_get_hwinfo(15);
    printf("Bound Info:\n");
    tmp = bound;
    while (tmp)
    {
        printf("\tmodel : %s\n", tmp->model);
        printf("\t\tvendor : %s\n", tmp->vendor);
        printf("\t\tdevice : %s\n", tmp->device);
        printf("\t\tdriver : %s\n", tmp->driver);
        printf("\t\trevision : %s\n", tmp->revision);
        printf("\t\tdevicenum : %s\n", tmp->devicenum);
        printf("\t\tclock : %s\n", tmp->clock);
        printf("\t\twidth : %s\n", tmp->width);
        tmp = tmp->next;
    }
    kdk_hw_free_hw(bound);

    struct HWInfo *cdrom = kdk_hw_get_hwinfo(23);
    printf("CDRom Info:\n");
    tmp = cdrom;
    while (tmp)
    {
        printf("\tmodel : %s\n", tmp->model);
        printf("\t\tvendor : %s\n", tmp->vendor);
        printf("\t\tdevice : %s\n", tmp->device);
        printf("\t\tdriver : %s\n", tmp->driver);
        printf("\t\trevision : %s\n", tmp->revision);
        printf("\t\tdevicenum : %s\n", tmp->devicenum);
        printf("\t\tclock : %s\n", tmp->clock);
        printf("\t\twidth : %s\n", tmp->width);
        tmp = tmp->next;
    }
    kdk_hw_free_hw(cdrom);

    struct HWInfo *camera = kdk_hw_get_hwinfo(27);
    printf("Camera Info:\n");
    tmp = camera;
    while (tmp)
    {
        printf("\tmodel : %s\n", tmp->model);
        printf("\t\tvendor : %s\n", tmp->vendor);
        printf("\t\tdevice : %s\n", tmp->device);
        printf("\t\tdriver : %s\n", tmp->driver);
        printf("\t\trevision : %s\n", tmp->revision);
        printf("\t\tdevicenum : %s\n", tmp->devicenum);
        printf("\t\tclock : %s\n", tmp->clock);
        printf("\t\twidth : %s\n", tmp->width);
        tmp = tmp->next;
    }
    kdk_hw_free_hw(camera);


    struct Power * power = kdk_hw_get_powerinfo();
    printf("on_battery : %s\n", power->on_battery ? "true" : "false");
    printf("lid_is_closed : %s\n", power->lid_is_closed ? "true" : "false");
    printf("lid_is_present : %s\n", power->lid_is_present ? "true" : "false");
    printf("daemon version : %s\n", power->daemon_version);
    printf("critical action : %s\n", power->critical_action);

    struct power_device *device = power->devices;
    while (device)
    {
        printf("name : %s\n", device->name);
        printf("\tnative path : %s\n", device->native_path);
        printf("\tpower supply : %s\n", device->power_supply ? "true" : "false");
        printf("\tupdated : %s\n", device->updated);
        printf("\thas history : %s\n", device->has_history ? "true" : "false");
        printf("\thas statistics : %s\n", device->has_statistics ? "true" : "false");
        printf("\tis persent: %s\n", device->is_persent ? "true" : "false");
        printf("\tis rechargeable : %s\n", device->is_rechargeable ? "true" : "false");
        printf("\tstate : %s\n", device->state);
        printf("\twaring level : %s\n", device->warning_level);
        printf("\tenergy : %0.2f\n", device->energy);
        printf("\tenergy empty : %0.2f\n", device->energy_empty);
        printf("\tenergy full: %0.2f\n", device->energy_full);
        printf("\tenergy full design : %0.2f\n", device->energy_full_design);
        printf("\tenergy rate : %0.2f\n", device->energy_rate);
        printf("\tvoltage : %0.2f\n", device->voltage);
        printf("\ttime to empty : %ld\n", device->time_to_empty);
        printf("\ttime to full : %ld\n", device->time_to_full);
        printf("\tpercentage : %0.2f\n", device->percentage);
        printf("\ttemperature : %0.2f\n", device->temperature);
        printf("\tcapacity : %0.2f\n", device->capacity);
        printf("\ttechnology : %s\n", device->technology);
        printf("\tonline : %s\n", device->online ? "true" : "false");
        printf("\ticon name : %s\n", device->icon_name);
        printf("\tmodel : %s\n", device->model);
        printf("\tbattery_level : %d\n", device->battery_level);
        printf("\ttype : %d\n", device->type);
        device = device->next;
    }
    kdk_hw_free_power_info(power);
}
