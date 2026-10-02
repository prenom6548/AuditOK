#
# Regular cron jobs for the kylin-background-upgrade package
#
0 4	* * *	root	[ -x /usr/bin/kylin-background-upgrade_maintenance ] && /usr/bin/kylin-background-upgrade_maintenance
