#
# Regular cron jobs for the kylin-driver-manager-service package
#
0 4	* * *	root	[ -x /usr/bin/kylin-driver-manager-service_maintenance ] && /usr/bin/kylin-driver-manager-service_maintenance
