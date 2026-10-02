#
# Regular cron jobs for the kylin-kmre-image-data-x64 package
#
0 4	* * *	root	[ -x /usr/bin/kylin-kmre-image-data-x64_maintenance ] && /usr/bin/kylin-kmre-image-data-x64_maintenance
