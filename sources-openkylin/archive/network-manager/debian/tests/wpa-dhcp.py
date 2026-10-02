#!/usr/bin/env python3
# Test wpa_supplicant with a DHCP client in various modes

__author__ = 'Martin Pitt <martin.pitt@ubuntu.com>'
__copyright__ = '(C) 2013 Canonical Ltd.'
__license__ = 'GPL v2 or later'

import shutil
import subprocess
import sys
import time
import unittest

import base_wifi


class T(base_wifi.NetworkTestWifi):
    @unittest.expectedFailure
    def test_open_a_ip4(self):
        '''Open network, 802.11a, IPv4'''

        # channel 36 ought to work everywhere
        self.do_test('hw_mode=a\nchannel=36\n\nssid=' + self.SSID,
                     'ssid="%s"\nkey_mgmt=NONE' % self.SSID,
                     None,
                     ['54.0'])

    @unittest.expectedFailure
    def test_open_a_ip6(self):
        '''Open network, 802.11a, IPv6'''

        self.do_test('hw_mode=a\nchannel=36\n\nssid=' + self.SSID,
                     'ssid="%s"\nkey_mgmt=NONE' % self.SSID,
                     '',
                     ['54.0'])

    def test_open_b_ip4(self):
        '''Open network, 802.11b, IPv4'''

        self.do_test('hw_mode=b\nchannel=1\nssid=' + self.SSID,
                     'ssid="%s"\nkey_mgmt=NONE' % self.SSID,
                     None,
                     ['11.0'])

    def test_open_b_ip6_dhcp(self):
        '''Open network, 802.11b, IPv6 with DHCP'''

        self.do_test('hw_mode=b\nchannel=1\nssid=' + self.SSID,
                     'ssid="%s"\nkey_mgmt=NONE' % self.SSID,
                     '',
                     ['11.0'])

    def test_open_b_ip6_raonly(self):
        '''Open network, 802.11b, IPv6 with only RA'''

        self.do_test('hw_mode=b\nchannel=1\nssid=' + self.SSID,
                     'ssid="%s"\nkey_mgmt=NONE' % self.SSID,
                     'ra-only',
                     ['11.0'])

    def test_open_g_ip4(self):
        '''Open network, 802.11g, IPv4'''

        self.do_test('hw_mode=g\nchannel=1\nssid=' + self.SSID,
                     'ssid="%s"\nkey_mgmt=NONE' % self.SSID,
                     None,
                     ['54.0'])

    def test_wpa1_ip4(self):
        '''WPA1, 802.11g, IPv4'''

        self.do_test('''hw_mode=g
channel=1
ssid=%s
wpa=1
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
wpa_passphrase=12345678
''' % self.SSID,
                     '''ssid="%s"
psk="12345678"
key_mgmt=WPA-PSK
proto=WPA
pairwise=TKIP
group=TKIP''' % self.SSID,
                     None,
                     ['54.0',
                      'Pairwise ciphers: TKIP',
                      'Authentication suites: PSK'])

    def test_wpa2_ip4(self):
        '''WPA2, 802.11g, IPv4'''

        self.do_test('''hw_mode=g
channel=1
ssid=%s
wpa=2
wpa_key_mgmt=WPA-PSK
wpa_pairwise=CCMP
wpa_passphrase=12345678
''' % self.SSID,
                     '''ssid="%s"
psk="12345678"
key_mgmt=WPA-PSK
proto=WPA2
pairwise=CCMP
group=CCMP''' % self.SSID,
                     None,
                     ['54.0',
                      'Pairwise ciphers: CCMP',
                      'Authentication suites: PSK'])

    def test_wpa2_ip6(self):
        '''WPA2, 802.11g, IPv6'''

        self.do_test('''hw_mode=g
channel=1
ssid=%s
wpa=2
wpa_key_mgmt=WPA-PSK
wpa_pairwise=CCMP
wpa_passphrase=12345678
''' % self.SSID,
                     '''ssid="%s"
psk="12345678"
key_mgmt=WPA-PSK
proto=WPA2
pairwise=CCMP
group=CCMP''' % self.SSID,
                     '',
                     ['54.0',
                      'Pairwise ciphers: CCMP',
                      'Authentication suites: PSK'])

    #
    # Common for all tests
    #

    def do_test(self, hostapd_conf, wpa_conf, ipv6, exp_iw_scan):
        self.setup_ap(hostapd_conf, ipv6)
        self.check_ssid_avail(exp_iw_scan)
        self.start_wpasupp(wpa_conf)
        self.check_iw_link()
        self.check_communication(ipv6)
        self.check_address(ipv6)

    def check_ssid_avail(self, expected_strings):
        subprocess.check_call(['ip', 'link', 'set', self.dev_w_client, 'up'])
        out = subprocess.check_output(['iw', 'dev', self.dev_w_client, 'scan'],
                                      universal_newlines=True)
        # down it again, wpa_supplicant is supposed to up it by itself
        subprocess.check_call(['ip', 'link', 'set', self.dev_w_client, 'down'])

        self.assertRegex(out, r'SSID: ' + self.SSID)
        for s in expected_strings:
            self.assertRegex(out, s)

    def check_iw_link(self):
        tries = 10
        while tries > 0:
            out = subprocess.check_output(['iw', 'dev', self.dev_w_client, 'link'],
                                          universal_newlines=True)
            if 'SSID' in out:
                break
            tries -= 1
            time.sleep(1)
        else:
            self.fail('timed out on iwconfig showing connected status')

        self.assertRegex(out, r'Connected to ' + self.mac_w_ap)
        self.assertRegex(out, r'SSID: ' + self.SSID)

    def check_communication(self, ipv6_mode):
        '''Verify that communication works between AP and client

        This proves that wpa_supplicant set up a working link. We use a DHCP
        request for IPv4 and IPv6 in DHCP mode. For IPv6 in other (i.
        e. ra-only/slaac) modes, check that dnsmasq received a router solicit
        and sends a router advertisement.
        '''
        if ipv6_mode is not None:
            self.poll_text(self.dnsmasq_log, 'RTR-SOLICIT(%s)' % self.dev_w_ap, timeout=50)
            self.poll_text(self.dnsmasq_log, 'RTR-ADVERT(%s)' % self.dev_w_ap, timeout=5)

        # stop here for non-DHCP modes in IPv6
        if ipv6_mode not in (None, '', 'slaac'):
            return

        if ipv6_mode is not None:
            mode = '-6'
        else:
            mode = '-4'

        # run DHCP client on client
        shutil.rmtree('/var/lib/dhcpcd/', ignore_errors=True)
        out = subprocess.check_output(['dhcpcd', mode, '-w', '-d', '-A', self.dev_w_client],
                                      universal_newlines=True, timeout=10,
                                      stderr=subprocess.STDOUT)
        # stop DHCP client
        subprocess.call(['dhcpcd', mode, '-x', '-q', self.dev_w_client],
                        stderr=subprocess.STDOUT)

        if ipv6_mode is not None:
            self.assertRegex(out, r'adding address 2600::')
            self.assertRegex(out, r'BOUND6', out)
        else:
            self.assertRegex(out, r'acknowledged 192.168.5.\d+ from 192.168.5.1')
            self.assertRegex(out, r'leased 192.168.5.')

    def check_address(self, ipv6_mode):
        '''Verify that the interface got an appropriate address assigned'''

        out = subprocess.check_output(['ip', 'a', 'show', 'dev', self.dev_w_client],
                                      universal_newlines=True)
        self.assertRegex(out, r'state UP')
        if ipv6_mode is None:
            self.assertRegex(out, r'inet 192.168.5.\d+/24')
        else:
            if not ipv6_mode:
                # has global address from our DHCP server
                self.assertRegex(out, r'inet6 2600::[0-9a-z]+/\d')
            else:
                # has address with our prefix and MAC
                self.assertRegex(out, r'inet6 2600::ff:fe00:[0-9a-z]+/64 scope global (?:tentative )?(?:mngtmpaddr )?dynamic')

            # has a link-local address
            self.assertRegex(out, r'inet6 fe80::ff:fe00:[0-9a-z:]+/64 scope link')


# write to stdout, not stderr
runner = unittest.TextTestRunner(stream=sys.stdout, verbosity=2)
unittest.main(testRunner=runner)
