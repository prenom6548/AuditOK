#
# Regular cron jobs for the kylin-kmre-daemon package
#
0 4	* * *	root	[ -x /usr/bin/kylin-kmre-daemon_maintenance ] && /usr/bin/kylin-kmre-daemon_maintenance
