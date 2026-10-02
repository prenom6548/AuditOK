"""
Base class for network related tests.

This provides fake wifi devices with mac80211_hwsim and hostapd, test ethernet
devices with veth, utility functions to start wpasupplicant, dnsmasq, get/set
rfkill status, and some utility functions.
"""

__author__ = "Martin Pitt <martin.pitt@ubuntu.com>"
__copyright__ = "(C) 2013-2025 Canonical Ltd."
__license__ = "GPL v2 or later"

import ctypes
import functools
import os
import os.path
import re
import shutil
import subprocess
import sys
import time
from glob import glob

import gi

import base

gi.require_version("NM", "1.0")
from gi.repository import NM, Gio, GLib

# If True, NetworkManager logs directly to stdout, to watch logs in real time
NM_LOG_STDOUT = os.getenv("NM_LOG_STDOUT", False)

# avoid accidentally destroying any real config
os.environ["GSETTINGS_BACKEND"] = "memory"

def run_in_subprocess(fn):
    """Decorator for running fn in a child process"""

    @functools.wraps(fn)
    def wrapped(*args, **kwargs):
        # args[0] is self
        args[0].wrap_process(fn, *args, **kwargs)

    return wrapped

def wait_nm_online():
    tries = 5
    while tries > 0 and subprocess.call(['nm-online', '-qs']) != 0:
        time.sleep(1)
        tries = tries - 1

def set_up_module():
    # unshare the mount namespace, so that our tmpfs mounts are guaranteed to get
    # cleaned up, and don't influence the production system
    libc6 = ctypes.cdll.LoadLibrary("libc.so.6")
    assert (
        libc6.unshare(ctypes.c_int(0x00020000)) == 0
    ), "failed to unshare mount namespace"

    # stop system-wide NetworkManager to avoid interfering with tests
    subprocess.check_call(['systemctl', 'stop', 'NetworkManager.service'])

def tear_down_module():
    # Make sure the management network stays up-and-running.
    if os.path.exists('/etc/systemd/network/20-wired.network'):
        subprocess.check_call(['systemctl', 'restart', 'systemd-networkd.service'])
    else:
        print("WARNING: mgmt network config (20-wired.network) not found. "
              "Skipping restart of systemd-networkd.service ...")


class WifiAuthentication():
    def __init__(self):
        self.wpa_settings = None
        self.wpa_eap_settings = None
        self.setting_name = None
        self.needed_secrets = None

        self.tmpdir = '/tmp/hostapd'
        shutil.rmtree(self.tmpdir, ignore_errors=True)

    @property
    def auth_settings(self):
        return self.wpa_settings

    @property
    def auth_eap_settings(self):
        return self.wpa_eap_settings


class WifiAuthenticationWPAPSK(WifiAuthentication):
    def __init__(self, psk):
        super().__init__()
        self.wpa_settings = NM.SettingWirelessSecurity.new()
        self.wpa_settings.set_property(NM.SETTING_WIRELESS_SECURITY_KEY_MGMT, "wpa-psk")
        self.wpa_settings.set_property(NM.SETTING_WIRELESS_SECURITY_PSK, psk)

        self.setting_name = NM.SETTING_WIRELESS_SECURITY_SETTING_NAME
        self.needed_secrets = [NM.SETTING_WIRELESS_SECURITY_PSK]


class WifiAuthenticationWPAEAP(WifiAuthentication):
    def __init__(self, modes, phase2, identity, password, client_cert=False):
        super().__init__()

        self.make_certs()

        self.wpa_settings = NM.SettingWirelessSecurity.new()
        self.wpa_settings.set_property(NM.SETTING_WIRELESS_SECURITY_KEY_MGMT, "wpa-eap")

        self.wpa_eap_settings = NM.Setting8021x.new()
        self.wpa_eap_settings.set_property(NM.SETTING_802_1X_EAP, modes)

        self.wpa_eap_settings.set_property(NM.SETTING_802_1X_IDENTITY, identity)

        if password:
            self.wpa_eap_settings.set_property(NM.SETTING_802_1X_PASSWORD, password)

        if phase2 == 'tls':
            self.wpa_eap_settings.set_property(NM.SETTING_802_1X_PHASE2_AUTHEAP, phase2)
        else:
            self.wpa_eap_settings.set_property(NM.SETTING_802_1X_PHASE2_AUTH, phase2)

        if client_cert:
            # Certificate paths must start with file:// and be null terminated
            # See src/libnmc-setting/settings-docs.h
            self.wpa_eap_settings.set_property(
                    NM.SETTING_802_1X_CA_CERT,
                    GLib.Bytes(f'file://{self.tmpdir}/pki/ca.crt\0'.encode()))
            self.wpa_eap_settings.set_property(
                    NM.SETTING_802_1X_CLIENT_CERT,
                    GLib.Bytes(f'file://{self.tmpdir}/pki/issued/client.crt\0'.encode()))
            self.wpa_eap_settings.set_property(
                    NM.SETTING_802_1X_PRIVATE_KEY,
                    GLib.Bytes(f'file://{self.tmpdir}/pki/private/client.key\0'.encode()))
            self.wpa_eap_settings.set_property(
                    NM.SETTING_802_1X_PRIVATE_KEY_PASSWORD, 'passw0rd')

        self.setting_name = NM.SETTING_802_1X_SETTING_NAME
        self.needed_secrets = [NM.SETTING_802_1X_PASSWORD, NM.SETTING_802_1X_PRIVATE_KEY_PASSWORD]

    def make_certs(self):
        os.mkdir(self.tmpdir)

        with open(f'{self.tmpdir}/hostapd.eap_user', 'w') as f:
            f.write('''* PEAP,TLS

"account1" MSCHAPV2 "password1" [2]
"account2" MSCHAPV2 "password2" [2]
''')

        # Create a CA, server and client certificates protected by passw0rd and the DH parameters
        create_ca_script = """/usr/share/easy-rsa/easyrsa init-pki
EASYRSA_BATCH=1 /usr/share/easy-rsa/easyrsa build-ca nopass
EASYRSA_PASSOUT=pass:passw0rd EASYRSA_BATCH=1 /usr/share/easy-rsa/easyrsa build-server-full server
EASYRSA_PASSOUT=pass:passw0rd EASYRSA_BATCH=1 /usr/share/easy-rsa/easyrsa build-client-full client
/usr/share/easy-rsa/easyrsa gen-dh
"""

        with open(f'{self.tmpdir}/make_certs.sh', 'w') as f:
            f.write(create_ca_script)

        cmd = ['bash', 'make_certs.sh']
        subprocess.call(cmd, stdout=subprocess.DEVNULL,
                        stderr=subprocess.DEVNULL, cwd=self.tmpdir)


class NetworkTestWifi(base.NetworkTestBase):
    """Wifi functionality for network test cases

    setUp() creates two test wlan devices, one for a simulated access point
    (self.dev_w_ap), the other for a simulated client device
    (self.dev_w_client), and two test ethernet devices (self.dev_e_ap and
    self.dev_e_client).

    Each test should call self.setup_ap() or self.setup_eth() with the desired
    configuration.
    """

    @classmethod
    def setUpClass(klass):
        super().setUpClass()
        klass.SSID = "fake net"
        # check availability of programs, and cleanly skip test if they are not
        # available
        for program in ["wpa_supplicant", "hostapd", "iw"]:
            if shutil.which(program) is None:
                raise SystemError("%s is required for this test suite, but not available" % program)

        klass.orig_country = None
        # ensure we have this so that iw works
        try:
            subprocess.check_call(['modprobe', 'cfg80211'])
            # set regulatory domain "EU", so that we can use 80211.a 5 GHz channels
            out = subprocess.check_output(['iw', 'reg', 'get'], text=True)
            m = re.match(r'^(?:global\n)?country (\S+):', out)
            assert m
            klass.orig_country = m.group(1)
            subprocess.check_call(['iw', 'reg', 'set', 'EU'])
        except Exception:
            print('cfg80211 (wireless) is unavailable, can\'t test', file=sys.stderr)
            raise

    @classmethod
    def tearDownClass(klass):
        if klass.orig_country is not None:
            subprocess.check_call(['iw', 'reg', 'set', klass.orig_country])
        super().tearDownClass()

    @classmethod
    def create_devices(klass):
        """Create Access Point and Client devices with mac80211_hwsim and veth"""
        # TODO: Consider using some trickery, to allow loading modules on the
        # host by name/alias from within a container.
        # https://github.com/weaveworks/weave/issues/3115
        # https://x.com/lucabruno/status/902934379835662336
        # https://github.com/docker-library/docker/blob/master/modprobe.sh  # wokeignore:rule=master
        # https://github.com/torvalds/linux/blob/master/net/core/dev_ioctl.c:dev_load()  # wokeignore:rule=master
        # e.g. via netdev ioctl SIOCGIFINDEX:
        # https://github.com/weaveworks/go-odp/blob/master/odp/dpif.go#L67  # wokeignore:rule=master
        #
        # Or alternatively, port the WiFi testing to virt_wifi, which can be
        # auto-loaded via "ip link add link eth0 name wlan42 type virt_wifi"
        # inside a (privileged) LXC container, as used by autopkgtest.

        if os.path.exists("/sys/module/mac80211_hwsim"):
            raise SystemError("mac80211_hwsim module already loaded")
        # create virtual wlan devs
        before_wlan = set([c for c in os.listdir("/sys/class/net") if c.startswith("wlan")])
        subprocess.check_call(["modprobe", "mac80211_hwsim"])
        # wait 5 seconds for fake devices to appear
        timeout = 50
        while timeout > 0:
            after_wlan = set([c for c in os.listdir("/sys/class/net") if c.startswith("wlan")])
            if len(after_wlan) - len(before_wlan) >= 2:
                break
            timeout -= 1
            time.sleep(0.1)
        else:
            raise SystemError("timed out waiting for fake devices to appear")

        devs = list(after_wlan - before_wlan)
        klass.dev_w_ap = devs[0]
        klass.dev_w_client = devs[1]

        # determine and store MAC addresses
        # Creation of the veths introduces a race with newer versions of
        # systemd, as it  will change the initial MAC address after the device
        # was created and networkd took control. Give it some time, so we read
        # the correct MAC address
        time.sleep(1)
        with open("/sys/class/net/%s/address" % klass.dev_w_ap) as f:
            klass.mac_w_ap = f.read().strip().upper()
        with open("/sys/class/net/%s/address" % klass.dev_w_client) as f:
            klass.mac_w_client = f.read().strip().upper()
        # print('Created fake devices: AP: %s, client: %s' % (klass.dev_w_ap, klass.dev_w_client))
        super().create_devices()

    @classmethod
    def shutdown_devices(klass):
        """Remove test wlan devices"""
        super().shutdown_devices()
        klass.dev_w_ap = None
        klass.dev_w_client = None
        subprocess.check_call(["rmmod", "mac80211_hwsim"])

    def setup_ap(self, hostapd_conf, ipv6_mode):
        """Set up simulated access point

        On self.dev_w_ap, run hostapd with given configuration. Setup dnsmasq
        according to ipv6_mode, see start_dnsmasq().

        This is torn down automatically at the end of the test.
        """
        # give our AP an IP
        subprocess.check_call(["ip", "a", "flush", "dev", self.dev_w_ap])
        if ipv6_mode is not None:
            subprocess.check_call(
                ["ip", "a", "add", "2600::1/64", "dev", self.dev_w_ap]
            )
        else:
            subprocess.check_call(
                ["ip", "a", "add", "192.168.5.1/24", "dev", self.dev_w_ap]
            )

        self.start_hostapd(hostapd_conf)
        self.start_dnsmasq(ipv6_mode, self.dev_w_ap)

    def start_wpasupp(self, conf):
        """Start wpa_supplicant on client interface"""

        w_conf = os.path.join(self.workdir, "wpasupplicant.conf")
        with open(w_conf, "w") as f:
            f.write("ctrl_interface=%s\nnetwork={\n%s\n}\n" % (self.workdir, conf))
        log = os.path.join(self.workdir, "wpasupp.log")
        p = subprocess.Popen(
            [
                "wpa_supplicant",
                "-Dwext",
                "-i",
                self.dev_w_client,
                "-e",
                self.entropy_file,
                "-c",
                w_conf,
                "-f",
                log,
            ],
            stderr=subprocess.PIPE,
        )
        self.addCleanup(p.wait)
        self.addCleanup(p.terminate)
        # TODO: why does this sometimes take so long?
        self.poll_text(log, "CTRL-EVENT-CONNECTED", timeout=200)

    def start_hostapd(self, conf):
        hostapd_conf = os.path.join(self.workdir, "hostapd.conf")
        with open(hostapd_conf, "w") as f:
            f.write("interface=%s\ndriver=nl80211\n" % self.dev_w_ap)
            f.write(conf)

        log = os.path.join(self.workdir, "hostapd.log")
        p = subprocess.Popen(
            ["hostapd", "-e", self.entropy_file, "-f", log, hostapd_conf],
            stdout=subprocess.PIPE,
        )
        self.addCleanup(p.wait)
        self.addCleanup(p.terminate)
        self.poll_text(log, "" + self.dev_w_ap + ": AP-ENABLED")

    def wait_ap(self, timeout):
        """Wait for AccessPoint NM object to appear, and return it"""

        self.assertEventually(
            lambda: len(self.nmdev_w.get_access_points()) > 0,
            "timed out waiting for AP to be detected",
            timeout=timeout,
        )

        return self.nmdev_w.get_access_points()[0]

    def connect_to_ap(self, ap, auth_settings, ipv6_mode, ip6_privacy):
        """Connect to an NMAccessPoint.

        auth_settings should be None for open networks and an instance of
        WifiAuthentication for WEP/WPA-PSK/WPA-EAP.

        ip6_privacy is a NM.SettingIP6ConfigPrivacy flag.

        Return (NMConnection, NMActiveConnection) objects.
        """

        ip4_method = NM.SETTING_IP4_CONFIG_METHOD_DISABLED
        ip6_method = NM.SETTING_IP6_CONFIG_METHOD_IGNORE
        if ipv6_mode is None:
            ip4_method = NM.SETTING_IP4_CONFIG_METHOD_AUTO
        else:
            ip6_method = NM.SETTING_IP6_CONFIG_METHOD_AUTO

        # If we have a secret, supply it to the new connection right away;
        # adding it afterwards with update_secrets() does not work, and we
        # can't implement a SecretAgent as get_secrets() would need to build a
        # map of a map of gpointers to gpointers which is too much for PyGI
        partial_conn = NM.SimpleConnection.new()
        partial_conn.add_setting(NM.SettingIP4Config(method=ip4_method))
        if auth_settings:
            partial_conn.add_setting(auth_settings.auth_settings)
            if isinstance(auth_settings, WifiAuthenticationWPAEAP):
                partial_conn.add_setting(auth_settings.auth_eap_settings)
        if ip6_privacy is not None:
            partial_conn.add_setting(
                NM.SettingIP6Config(ip6_privacy=ip6_privacy, method=ip6_method)
            )

        ml = GLib.MainLoop()
        self.cb_conn = None
        self.cancel = Gio.Cancellable()
        self.timeout_tag = 0

        def add_activate_cb(client, res, data):
            if self.timeout_tag > 0:
                GLib.source_remove(self.timeout_tag)
                self.timeout_tag = 0
            try:
                self.cb_conn = self.nmclient.add_and_activate_connection_finish(res)
            except gi.repository.GLib.Error as e:
                # Check if the error is "Operation was cancelled"
                if e.domain != "g-io-error-quark" or e.code != 19:
                    self.fail(
                        "add_and_activate_connection failed: %s (%s, %d)"
                        % (e.message, e.domain, e.code)
                    )
            ml.quit()

        def timeout_cb():
            self.timeout_tag = -1
            self.cancel.cancel()
            ml.quit()
            return GLib.SOURCE_REMOVE

        self.nmclient.add_and_activate_connection_async(
            partial_conn,
            self.nmdev_w,
            ap.get_path(),
            self.cancel,
            add_activate_cb,
            None,
        )
        self.timeout_tag = GLib.timeout_add_seconds(300, timeout_cb)
        ml.run()
        if self.timeout_tag < 0:
            self.timeout_tag = 0
            self.fail("Main loop for adding connection timed out!")
        self.assertNotEqual(self.cb_conn, None)
        active_conn = self.cb_conn
        self.cb_conn = None

        conn = self.conn_from_active_conn(active_conn)
        self.assertTrue(conn.verify())

        # verify need_secrets()
        needed_secrets = conn.need_secrets()
        if auth_settings is None:
            self.assertEqual(needed_secrets, (None, []))
        else:
            self.assertEqual(
                needed_secrets[0], auth_settings.setting_name
            )
            self.assertEqual(type(needed_secrets[1]), list)
            self.assertGreaterEqual(len(needed_secrets[1]), 1)
            self.assertIn(needed_secrets[1][0], auth_settings.needed_secrets)

        # we are usually ACTIVATING at this point; wait for completion
        # TODO: 5s is not enough, argh slow DHCP client
        self.assertEventually(
            lambda: active_conn.get_state() == NM.ActiveConnectionState.ACTIVATED,
            "timed out waiting for %s to get activated" % active_conn.get_connection(),
            timeout=600,
        )
        self.assertEqual(self.nmdev_w.get_state(), NM.DeviceState.ACTIVATED)
        return (conn, active_conn)

    # libnm-glib has a lot of internal persistent state (private D-BUS
    # connections and such); as it is very brittle and hard to track down
    # all remaining references to any NM* object after a test, we rather
    # run each test in a separate subprocess
    @run_in_subprocess
    def do_test(
        self,
        hostapd_conf,
        ipv6_mode,
        expected_max_bitrate,
        auth_settings=None,
        ip6_privacy=None,
    ):
        """Actual test code, parameterized for the particular test case"""

        self.setup_ap(hostapd_conf, ipv6_mode)
        self.start_nm(self.dev_w_client)

        # on coldplug we expect the AP to be picked out fast
        ap = self.wait_ap(timeout=100)
        self.assertTrue(ap.get_path().startswith("/org/freedesktop/NetworkManager"))
        self.assertEqual(ap.get_mode(), getattr(NM, "80211Mode").INFRA)
        self.assertEqual(ap.get_max_bitrate(), expected_max_bitrate)
        # self.assertEqual(ap.get_flags(), )

        # should not auto-connect
        self.assertEqual(self.filtered_active_connections(), [])

        # connect to that AP
        (conn, active_conn) = self.connect_to_ap(ap, auth_settings, ipv6_mode, ip6_privacy)

        # check NMActiveConnection object
        self.assertIn(
            active_conn.get_uuid(),
            [c.get_uuid() for c in self.filtered_active_connections()],
        )
        self.assertEqual(
            [d.get_udi() for d in active_conn.get_devices()], [self.nmdev_w.get_udi()]
        )

        # check corresponding NMConnection object
        wireless_setting = conn.get_setting_wireless()
        self.assertEqual(wireless_setting.get_ssid().get_data(), self.SSID.encode())
        self.assertEqual(wireless_setting.get_hidden(), False)
        if auth_settings:
            self.assertEqual(
                conn.get_setting_wireless_security().get_name(),
                NM.SETTING_WIRELESS_SECURITY_SETTING_NAME,
            )
        else:
            self.assertEqual(conn.get_setting_wireless_security(), None)
        # for debugging
        # conn.dump()

        # for IPv6, check privacy setting
        if ipv6_mode is not None and ip6_privacy != NM.SettingIP6ConfigPrivacy.UNKNOWN:
            assert (
                ip6_privacy is not None
            ), "for IPv6 tests you need to specify ip6_privacy flag"
            ip6_setting = conn.get_setting_ip6_config()
            self.assertEqual(ip6_setting.props.ip6_privacy, ip6_privacy)

        self.check_low_level_config(self.dev_w_client, ipv6_mode, ip6_privacy)

    def start_nm(self, wait_iface=None, auto_connect=True):
        super().start_nm(wait_iface, auto_connect,
                       managed_devices = [self.dev_w_client, self.dev_e_client])

        # determine device objects
        for d in self.nmclient.get_devices():
            if d.props.interface == self.dev_w_ap:
                self.assertEqual(d.get_device_type(), NM.DeviceType.WIFI)
                self.assertEqual(d.get_driver(), "mac80211_hwsim")
                self.assertEqual(d.get_hw_address(), self.mac_w_ap)
                self.nmdev_w_ap = d
            elif d.props.interface == self.dev_w_client:
                self.assertEqual(d.get_device_type(), NM.DeviceType.WIFI)
                self.assertEqual(d.get_driver(), "mac80211_hwsim")
                # NM ≥ 1.4 randomizes MAC addresses by default, so we can't
                # test for equality, just make sure it's not our AP
                self.assertNotEqual(d.get_hw_address(), self.mac_w_ap)
                self.nmdev_w = d

        self.assertTrue(
            hasattr(self, "nmdev_w_ap"), "Could not determine wifi AP NM device"
        )
        self.assertTrue(
            hasattr(self, "nmdev_w"), "Could not determine wifi client NM device"
        )

        self.process_glib_events()

    def shutdown_connections(self):
        super().shutdown_connections()
        # verify that NM properly deconfigures the WiFi devices
        try:
            self.assert_iface_down(self.dev_w_client, False)
        except AssertionError as e:
            # Log message is hidden by default, when called from an "addCleanup"
            # hook. So let's log it explicitly:
            print(f"AssertionError: {e}")
            raise

    def assert_iface_down(self, iface, validate_dev_w_ap=True):
        """Assert that client interface is down"""

        super().assert_iface_down(iface)

        if iface == self.dev_w_client:
            out = subprocess.check_output(
                ["iw", "dev", iface, "link"], universal_newlines=True
            )
            self.assertIn("Not connected", out)

            # but AP device should never be touched by NM
            if validate_dev_w_ap:
                self.assert_iface_up(self.dev_w_ap)


    def assert_iface_up(self, iface, expected_ip_a=None, unexpected_ip_a=None):
        """Assert that client interface is up"""

        super().assert_iface_up(iface, expected_ip_a, unexpected_ip_a)

        if iface == self.dev_w_client:
            out = subprocess.check_output(
                ["iw", "dev", iface, "link"], universal_newlines=True
            )
            self.assertIn("Connected to " + self.mac_w_ap, out)
            self.assertIn("SSID: " + self.SSID, out)

    @classmethod
    def _rfkill_attribute(klass, interface):
        """Return the path to interface's rfkill soft toggle in sysfs."""

        g = glob("/sys/class/net/%s/phy80211/rfkill*/soft" % interface)
        assert (
            len(g) == 1
        ), 'Did not find exactly one "soft" rfkill attribute for %s: %s' % (
            interface,
            str(g),
        )
        return g[0]

    @classmethod
    def get_rfkill(klass, interface):
        """Get rfkill status of an interface.

        Returns whether the interface is blocked, i. e. "True" for blocked,
        "False" for enabled.
        """
        with open(klass._rfkill_attribute(interface)) as f:
            val = f.read()
        return val == "1"

    @classmethod
    def set_rfkill(klass, interface, block):
        """Set rfkill status of an interface

        Use block==True for disabling ("killswitching") an interface,
        block==False to re-enable.
        """
        with open(klass._rfkill_attribute(interface), "w") as f:
            f.write(block and "1" or "0")
