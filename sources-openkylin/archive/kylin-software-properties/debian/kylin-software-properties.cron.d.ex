#
# Regular cron jobs for the kylin-software-properties package
#
0 4	* * *	root	[ -x /usr/bin/kylin-software-properties_maintenance ] && /usr/bin/kylin-software-properties_maintenance
