#!/usr/bin/env python3
# Test NetworkManager on simulated network devices
# For an interactive shell test, run "nm-wifi.py ColdplugWifi.shell", see below

__author__ = "Martin Pitt <martin.pitt@ubuntu.com>"
__copyright__ = "(C) 2013-2025 Canonical Ltd."
__license__ = "GPL v2 or later"

import os
import os.path
import subprocess
import sys
import time
import unittest

import gi

import base
import base_wifi

try:
    from dbusmock import DBusTestCase
except ImportError:
    DBusTestCase = object  # dummy so that the class declaration works

gi.require_version("NM", "1.0")
from gi.repository import NM


class ColdplugWifi(base_wifi.NetworkTestWifi):
    """Wifi: In these tests NM starts after setting up the AP"""

    # not run by default; run "nm-wifi ColdplugWifi.shell" to get this
    @base.run_in_subprocess
    def shell(self):
        """Start AP and NM, then run a shell (for debugging)"""

        self.setup_ap("hw_mode=b\nchannel=1\nssid=" + self.SSID, None)
        self.start_nm(self.dev_w_client)
        print(
            """

client interface: %s, access point interface: %s, AP SSID: "%s"

You can now run commands like "nmcli dev" or "nmcli dev wifi connect '%s'".
Logs are in '%s'. When done, exit the shell.

"""
            % (self.dev_w_client, self.dev_w_ap, self.SSID, self.SSID, self.workdir)
        )
        subprocess.call(["bash", "-i"])

    @base.run_in_subprocess
    def test_no_ap(self):
        """no available access point"""

        self.start_nm(self.dev_w_client)
        # Give the interfaces a bit more time to intialized; it may be needed
        # on ppc64el.
        time.sleep(30)
        self.assertEventually(self.nmclient.networking_get_enabled, timeout=20)

        # state independent properties
        self.assertEqual(self.nmdev_w.props.device_type, NM.DeviceType.WIFI)
        self.assertTrue(self.nmdev_w.props.managed)
        self.assertFalse(self.nmdev_w.props.firmware_missing)
        self.assertTrue(
            self.nmdev_w.props.udi.startswith("/sys/devices/"), self.nmdev_w.props.udi
        )

        # get_version() plausibility check
        out = subprocess.check_output(["nmcli", "--version"], universal_newlines=True)
        cli_version = out.split()[-1]
        self.assertTrue(cli_version[0].isdigit())
        self.assertEqual(self.nmclient.get_version(), cli_version)

        # state dependent properties (disconnected)
        self.assertIn(
            self.nmdev_w.get_state(),
            [NM.DeviceState.DISCONNECTED, NM.DeviceState.UNAVAILABLE],
        )
        self.assertEqual(self.nmdev_w.get_access_points(), [])
        self.assertEqual(self.nmdev_w.get_available_connections(), [])

    def test_open_b_ip4(self):
        """Open network, 802.11b, IPv4"""

        self.do_test("hw_mode=b\nchannel=1\nssid=" + self.SSID, None, 11000)

    def test_open_b_ip6_raonly_tmpaddr(self):
        """Open network, 802.11b, IPv6 with only RA, preferring temp address"""

        self.do_test(
            "hw_mode=b\nchannel=1\nssid=" + self.SSID,
            "ra-only",
            11000,
            ip6_privacy=NM.SettingIP6ConfigPrivacy.PREFER_TEMP_ADDR,
        )

    def test_open_b_ip6_raonly_pubaddr(self):
        """Open network, 802.11b, IPv6 with only RA, preferring public address"""

        self.do_test(
            "hw_mode=b\nchannel=1\nssid=" + self.SSID,
            "ra-only",
            11000,
            ip6_privacy=NM.SettingIP6ConfigPrivacy.PREFER_PUBLIC_ADDR,
        )

    def test_open_b_ip6_raonly_no_pe(self):
        """Open network, 802.11b, IPv6 with only RA, PE disabled"""

        self.do_test(
            "hw_mode=b\nchannel=1\nssid=" + self.SSID,
            "ra-only",
            11000,
            ip6_privacy=NM.SettingIP6ConfigPrivacy.DISABLED,
        )

    def test_open_b_ip6_dhcp(self):
        """Open network, 802.11b, IPv6 with DHCP, preferring temp address"""

        self.do_test(
            "hw_mode=b\nchannel=1\nssid=" + self.SSID,
            "",
            11000,
            ip6_privacy=NM.SettingIP6ConfigPrivacy.UNKNOWN,
        )

    def test_open_g_ip4(self):
        """Open network, 802.11g, IPv4"""

        self.do_test("hw_mode=g\nchannel=1\nssid=" + self.SSID, None, 54000)

    def test_wpa1_ip4(self):
        """WPA1, 802.11g, IPv4"""

        self.do_test(
            """hw_mode=g
channel=1
ssid=%s
wpa=1
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
wpa_passphrase=12345678
"""
            % self.SSID,
            None,
            54000,
            base_wifi.WifiAuthenticationWPAPSK('12345678'),
        )

    def test_wpa2_ip4(self):
        """WPA2, 802.11g, IPv4"""

        self.do_test(
            """hw_mode=g
channel=1
ssid=%s
wpa=2
wpa_key_mgmt=WPA-PSK
wpa_pairwise=CCMP
wpa_passphrase=12345678
"""
            % self.SSID,
            None,
            54000,
            base_wifi.WifiAuthenticationWPAPSK('12345678'),
        )

    def test_wpa2_ip6(self):
        """WPA2, 802.11g, IPv6 with only RA"""

        self.do_test(
            """hw_mode=g
channel=1
ssid=%s
wpa=2
wpa_key_mgmt=WPA-PSK
wpa_pairwise=CCMP
wpa_passphrase=12345678
"""
            % self.SSID,
            "ra-only",
            54000,
            base_wifi.WifiAuthenticationWPAPSK('12345678'),
            ip6_privacy=NM.SettingIP6ConfigPrivacy.PREFER_TEMP_ADDR,
        )

    def test_wpa_enterprise_authentication_ip4(self):
        """WPA2 + 802.1x, 802.11g, IPv4"""

        self.do_test(
            """hw_mode=g
channel=1
ssid=%s
auth_algs=1
eap_server=1
ieee8021x=1
eapol_version=2
wpa=2
wpa_key_mgmt=WPA-EAP
wpa_pairwise=TKIP
rsn_pairwise=CCMP
eap_user_file=/tmp/hostapd/hostapd.eap_user
ca_cert=/tmp/hostapd/pki/ca.crt
server_cert=/tmp/hostapd/pki/issued/server.crt
private_key=/tmp/hostapd/pki/private/server.key
private_key_passwd=passw0rd
dh_file=/tmp/hostapd/pki/dh.pem
ctrl_interface=/var/run/hostapd
ctrl_interface_group=0
"""
            % self.SSID,
            None,
            54000,
            base_wifi.WifiAuthenticationWPAEAP(['peap'], 'mschapv2', 'account1', 'password1')
        )

    def test_wpa_enterprise_authentication_with_client_certificate_ip4(self):
        """WPA2 + 802.1x with client certificate, 802.11g, IPv4"""

        self.do_test(
            """hw_mode=g
channel=1
ssid=%s
auth_algs=1
eap_server=1
ieee8021x=1
eapol_version=2
wpa=2
wpa_key_mgmt=WPA-EAP
wpa_pairwise=TKIP CCMP
rsn_pairwise=TKIP CCMP
eap_user_file=/tmp/hostapd/hostapd.eap_user
ca_cert=/tmp/hostapd/pki/ca.crt
server_cert=/tmp/hostapd/pki/issued/server.crt
private_key=/tmp/hostapd/pki/private/server.key
private_key_passwd=passw0rd
dh_file=/tmp/hostapd/pki/dh.pem
ctrl_interface=/var/run/hostapd
ctrl_interface_group=0
"""
            % self.SSID,
            None,
            54000,
            base_wifi.WifiAuthenticationWPAEAP(['tls'], 'tls', 'client', None, client_cert=True)
        )

    @base.run_in_subprocess
    def test_rfkill(self):
        """shut down connection on killswitch, restore it on unblock"""

        self.setup_ap("hw_mode=b\nchannel=1\nssid=" + self.SSID, None)
        self.start_nm(self.dev_w_client)
        ap = self.wait_ap(timeout=1800)
        (conn, active_conn) = self.connect_to_ap(ap, None, None, None)

        self.assertFalse(self.get_rfkill(self.dev_w_client))
        self.assertFalse(self.get_rfkill(self.dev_w_ap))

        # now block the client interface
        self.set_rfkill(self.dev_w_client, True)
        # disabling should be fast, give it ten seconds
        self.assertEventually(
            lambda: self.nmdev_w.get_state() == NM.DeviceState.UNAVAILABLE, timeout=100
        )

        # dev_w_client should be down now
        self.assert_iface_down(self.dev_w_client)

        # turn it back on
        self.set_rfkill(self.dev_w_client, False)
        # this involves DHCP, use same timeout as for regular connection
        self.assertEventually(
            lambda: self.nmdev_w.get_state() == NM.DeviceState.ACTIVATED, timeout=200
        )

        # dev_w_client should be back up
        self.assert_iface_up(self.dev_w_client, [r"inet 192.168.5.\d+/24"])


class Hotplug(base_wifi.NetworkTestWifi):
    """In these tests APs are set up while NM is already running"""

    @base.run_in_subprocess
    def test_auto_detect_ap(self):
        """new AP is being detected automatically within 30s"""

        self.start_nm()
        self.setup_ap("hw_mode=b\nchannel=1\nssid=" + self.SSID, None)
        ap = self.wait_ap(timeout=300)
        # get_ssid returns a byte array
        self.assertEqual(ap.get_ssid().get_data(), self.SSID.encode())
        self.assertEqual(self.nmdev_w.get_active_access_point(), None)


@unittest.skipIf(
    DBusTestCase is object,
    "WARNING: python-dbusmock not installed, skipping suspend tests; get it from https://pypi.python.org/pypi/python-dbusmock",
)
class Suspend(base_wifi.NetworkTestWifi, DBusTestCase):
    """These tests run under a mock logind on a private system D-BUS"""

    @classmethod
    def setUpClass(klass):
        klass.start_system_bus()
        base_wifi.NetworkTestWifi.setUpClass()

    @classmethod
    def tearDownClass(klass):
        base_wifi.NetworkTestWifi.tearDownClass()
        DBusTestCase.tearDownClass()

    def setUp(self):
        base_wifi.NetworkTestWifi.setUp(self)

        # start mock polkit and logind processes, so that we can
        # intercept/control suspend
        (p_polkit, self.obj_polkit) = self.spawn_server_template(
            "polkitd", {}, stdout=subprocess.PIPE
        )
        # by default we are not concerned about restricting access in the tests
        self.obj_polkit.AllowUnknown(True)
        self.addCleanup(p_polkit.wait)
        self.addCleanup(p_polkit.terminate)

        (p_logind, self.obj_logind) = self.spawn_server_template(
            "logind", {}, stdout=subprocess.PIPE
        )
        self.addCleanup(p_logind.wait)
        self.addCleanup(p_logind.terminate)

        # we have to manually start wpa_supplicant, as D-BUS activation does
        # not happen for the fake D-BUS
        log = os.path.join(self.workdir, "wpasupplicant.log")
        p_wpasupp = subprocess.Popen(
            ["wpa_supplicant", "-u", "-d", "-e", "-K", self.entropy_file, "-f", log]
        )
        self.addCleanup(p_wpasupp.wait)
        self.addCleanup(p_wpasupp.terminate)

    def fixme_test_active_ip4(self):
        """suspend during active IPv4 connection"""

        self.do_test(
            "hw_mode=b\nchannel=1\nssid=" + self.SSID, None, [r"inet 192.168.5.\d+/24"]
        )

    def fixme_test_active_ip6(self):
        """suspend during active IPv6 connection"""

        self.do_test("hw_mode=b\nchannel=1\nssid=" + self.SSID, "ra-only", [r"inet6 2600::"])

    #
    # Common test code
    #

    @base.run_in_subprocess
    def do_test(self, hostapd_conf, ipv6_mode, expected_ip_a):
        """Actual test code, parameterized for the particular test case"""

        self.setup_ap(hostapd_conf, ipv6_mode)
        self.start_nm(self.dev_w_client)
        ap = self.wait_ap(timeout=1800)
        (conn, active_conn) = self.connect_to_ap(ap, None, ipv6_mode, None)

        # send logind signal that we are about to suspend
        self.obj_logind.EmitSignal("", "PrepareForSleep", "b", [True])

        # disabling should be fast, give it one second
        self.assertEventually(
            lambda: self.nmdev_w.get_state() == NM.DeviceState.UNMANAGED, timeout=10
        )
        self.assert_iface_down(self.dev_w_client)

        # send logind signal that we resumed
        self.obj_logind.EmitSignal("", "PrepareForSleep", "b", [False])

        # this involves DHCP, use same timeout as for regular connection
        self.assertEventually(
            lambda: self.nmdev_w.get_state() == NM.DeviceState.ACTIVATED, timeout=100
        )

        # dev_w_client should be back up
        self.assert_iface_up(self.dev_w_client, expected_ip_a)


def setUpModule():
    base.set_up_module()

def tearDownModule():
    base.tear_down_module()


if __name__ == "__main__":
    # avoid unintelligible error messages when not being root
    if os.getuid() != 0:
       raise SystemExit("This integration test suite needs to be run as root")

    # write to stdout, not stderr
    runner = unittest.TextTestRunner(stream=sys.stdout, verbosity=2)
    unittest.main(testRunner=runner)
