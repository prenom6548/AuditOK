#
# Regular cron jobs for the kylin-web-app-engine-2.0.0.0 package
#
0 4	* * *	root	[ -x /usr/bin/kylin-web-app-engine-2.0.0.0_maintenance ] && /usr/bin/kylin-web-app-engine-2.0.0.0_maintenance
