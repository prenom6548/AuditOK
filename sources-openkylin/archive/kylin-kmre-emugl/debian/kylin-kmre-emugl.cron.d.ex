#
# Regular cron jobs for the kylin-kmre-emugl package
#
0 4	* * *	root	[ -x /usr/bin/kylin-kmre-emugl_maintenance ] && /usr/bin/kylin-kmre-emugl_maintenance
