#
# Regular cron jobs for the ostree-maintain-daemon package.
#
0 4	* * *	root	[ -x /usr/bin/ostree-maintain-daemon_maintenance ] && /usr/bin/ostree-maintain-daemon_maintenance
