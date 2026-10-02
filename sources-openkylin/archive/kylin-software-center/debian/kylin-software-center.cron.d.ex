#
# Regular cron jobs for the kylin-software-center package
#
0 4	* * *	root	[ -x /usr/bin/kylin-software-center_maintenance ] && /usr/bin/kylin-software-center_maintenance
