#
# Regular cron jobs for the kylin-screencap package
#
0 4	* * *	root	[ -x /usr/bin/kylin-screencap_maintenance ] && /usr/bin/kylin-screencap_maintenance
