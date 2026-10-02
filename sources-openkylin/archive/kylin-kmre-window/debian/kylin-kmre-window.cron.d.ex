#
# Regular cron jobs for the kylin-kmre-window package
#
0 4	* * *	root	[ -x /usr/bin/kylin-kmre-window_maintenance ] && /usr/bin/kylin-kmre-window_maintenance
