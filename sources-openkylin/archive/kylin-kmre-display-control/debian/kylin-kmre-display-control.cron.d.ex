#
# Regular cron jobs for the kylin-kmre-display-control package
#
0 4	* * *	root	[ -x /usr/bin/kylin-kmre-display-control_maintenance ] && /usr/bin/kylin-kmre-display-control_maintenance
