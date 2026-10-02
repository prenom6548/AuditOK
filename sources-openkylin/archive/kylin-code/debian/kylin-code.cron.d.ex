#
# Regular cron jobs for the kylin-code package
#
0 4	* * *	root	[ -x /usr/bin/kylin-code_maintenance ] && /usr/bin/kylin-code_maintenance
