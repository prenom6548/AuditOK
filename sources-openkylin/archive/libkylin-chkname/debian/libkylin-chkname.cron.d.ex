#
# Regular cron jobs for the libkylin-chkname package
#
0 4	* * *	root	[ -x /usr/bin/libkylin-chkname_maintenance ] && /usr/bin/libkylin-chkname_maintenance
