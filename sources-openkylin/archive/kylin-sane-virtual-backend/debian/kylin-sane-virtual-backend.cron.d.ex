#
# Regular cron jobs for the kylin-sane-virtual-backend package
#
0 4	* * *	root	[ -x /usr/bin/kylin-sane-virtual-backend_maintenance ] && /usr/bin/kylin-sane-virtual-backend_maintenance
