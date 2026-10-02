#
# Regular cron jobs for the kylin-kmre-manager package
#
0 4	* * *	root	[ -x /usr/bin/kylin-kmre-manager_maintenance ] && /usr/bin/kylin-kmre-manager_maintenance
